/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU General Public License Version 2.1
 *
 *  Sun Microsystems Inc., December, 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *  
 *  =================================================
 *  Modified July 2004 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Original source obtained from patch submitted to OpenOffice.org in issue
 *  23283 (see http://qa.openoffice.org/issues/show_bug.cgi?id=23283).
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

#include <hash_map>
#include <unicode/uscript.h>

#ifndef _SV_SALATSLAYOUT_HXX
#include <salatslayout.hxx>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

// Fix bug 1418 by setting the cache size very small as the OOo 2.0.x code
// lays out entire documents in the background and the cache will get full
// very quickly
#define LAYOUT_CACHE_MAX_SIZE 256

inline long Float32ToLong( Float32 f ) { return (long)( f + 0.5 ); }

struct ImplATSLayoutDataHash {
	int					mnLen;
	ATSUFontID			mnFontID;
	long				mnFontSize;
	double				mfFontScaleX;
	bool				mbAntialiased;
	bool				mbRTL;
	bool				mbVertical;
	sal_Unicode*		mpStr;
	sal_Int32			mnStrHash;
};

struct ImplHash
{
	size_t				operator()( const ImplATSLayoutDataHash *x ) const { return (size_t)x->mnStrHash; }
};

struct ImplHashEquality
{
	bool				operator()( const ImplATSLayoutDataHash *p1, const ImplATSLayoutDataHash *p2 ) const;
};

struct ImplATSLayoutData {
	static ::std::hash_multimap< ImplATSLayoutDataHash*, ImplATSLayoutData*, ImplHash, ImplHashEquality >	maLayoutCache;
	static ::std::list< ImplATSLayoutData* >	maLayoutCacheList;

	mutable int			mnRefCount;
	ImplATSLayoutDataHash*	mpHash;
	::vcl::com_sun_star_vcl_VCLFont*	mpVCLFont;
	ATSUStyle			maFontStyle;
	bool*				mpNeedFallback;
	::vcl::com_sun_star_vcl_VCLFont*	mpFallbackFont;
	ATSUTextLayout		maLayout;
	int					mnGlyphCount;
	ATSUGlyphInfoArray*	mpGlyphInfoArray;
	int*				mpCharsToChars;
	int*				mpCharsToGlyphs;
	long*				mpCharAdvances;
	ATSUStyle			maVerticalFontStyle;
	long				mnBaselineDelta;
	bool				mbValid;

	static void					ClearLayoutDataCache();
	static ImplATSLayoutData*	GetLayoutData( ImplLayoutArgs& rArgs, int nFallbackLevel, ::vcl::com_sun_star_vcl_VCLFont *pVCLFont );

						ImplATSLayoutData( ImplLayoutArgs& rArgs, ImplATSLayoutDataHash *pLayoutHash, int nFallbackLevel, ::vcl::com_sun_star_vcl_VCLFont *pVCLFont );
						~ImplATSLayoutData();

	void				Destroy();
	bool				IsValid() const { return mbValid; }
	void				Reference() const;
	void				Release() const;
};

static ATSCubicMoveToUPP pATSCubicMoveToUPP = NULL;
static ATSCubicLineToUPP pATSCubicLineToUPP = NULL;
static ATSCubicCurveToUPP pATSCubicCurveToUPP = NULL;
static ATSCubicClosePathUPP pATSCubicClosePathUPP = NULL;

using namespace basegfx;
using namespace osl;
using namespace rtl;
using namespace vcl;

// ============================================================================

bool ImplHashEquality::operator()( const ImplATSLayoutDataHash *p1, const ImplATSLayoutDataHash *p2 ) const
{
	return ( p1->mnLen == p2->mnLen &&
		p1->mnFontID == p2->mnFontID &&
		p1->mnFontSize == p2->mnFontSize &&
		p1->mfFontScaleX == p2->mfFontScaleX &&
		p1->mbAntialiased == p2->mbAntialiased &&
		p1->mbRTL == p2->mbRTL &&
		p1->mbVertical == p2->mbVertical &&
		p1->mnStrHash == p2->mnStrHash &&
		rtl_ustr_compare_WithLength( p1->mpStr, p1->mnLen, p2->mpStr, p2->mnLen ) == 0 );
}

// ============================================================================

::std::hash_multimap< ImplATSLayoutDataHash*, ImplATSLayoutData*, ImplHash, ImplHashEquality > ImplATSLayoutData::maLayoutCache;

// ----------------------------------------------------------------------------

::std::list< ImplATSLayoutData* > ImplATSLayoutData::maLayoutCacheList;

// ----------------------------------------------------------------------------

void ImplATSLayoutData::ClearLayoutDataCache()
{
	maLayoutCache.clear();

	while ( maLayoutCacheList.size() )
	{
		maLayoutCacheList.back()->Release();
		maLayoutCacheList.pop_back();
	}
}

// ----------------------------------------------------------------------------

ImplATSLayoutData *ImplATSLayoutData::GetLayoutData( ImplLayoutArgs& rArgs, int nFallbackLevel, com_sun_star_vcl_VCLFont *pVCLFont )
{
	ImplATSLayoutData *pLayoutData = NULL;

	ImplATSLayoutDataHash *pLayoutHash = new ImplATSLayoutDataHash();
	pLayoutHash->mnLen = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
	pLayoutHash->mnFontID = (ATSUFontID)pVCLFont->getNativeFont();
	pLayoutHash->mnFontSize = pVCLFont->getSize();
	pLayoutHash->mfFontScaleX = pVCLFont->getScaleX();
	pLayoutHash->mbAntialiased = pVCLFont->isAntialiased();
	pLayoutHash->mbRTL = ( rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL );
	pLayoutHash->mbVertical = ( rArgs.mnFlags & SAL_LAYOUT_VERTICAL );

	// Copy the string so that we can cache it and add leading and trailing
	// spaces so that ATSUGetGlyphInfo() will not fail as described in
	// bug 554
	pLayoutHash->mpStr = (sal_Unicode *)rtl_allocateMemory( pLayoutHash->mnLen * sizeof( sal_Unicode ) );
	memcpy( pLayoutHash->mpStr, rArgs.mpStr + rArgs.mnMinCharPos, pLayoutHash->mnLen * sizeof( sal_Unicode ) );
	pLayoutHash->mnStrHash = rtl_ustr_hashCode_WithLength( pLayoutHash->mpStr, pLayoutHash->mnLen );

	// Search cache for matching layout
	::std::hash_multimap< ImplATSLayoutDataHash*, ImplATSLayoutData*, ImplHash, ImplHashEquality >::const_iterator it = maLayoutCache.find( pLayoutHash );
	if ( it != maLayoutCache.end() )
	{
		pLayoutData = it->second;
		rtl_freeMemory( pLayoutHash->mpStr );
		delete pLayoutHash;
		pLayoutHash = NULL;
	}

	if ( !pLayoutData )
	{
		pLayoutData = new ImplATSLayoutData( rArgs, pLayoutHash, nFallbackLevel, pVCLFont );

		if ( !pLayoutData )
			return NULL;

		if ( !pLayoutData->IsValid() )
		{
			pLayoutData->Release();
			return NULL;
		}

		maLayoutCache.insert( ::std::hash_multimap< ImplATSLayoutDataHash*, ImplATSLayoutData*, ImplHash, ImplHashEquality >::value_type( pLayoutData->mpHash, pLayoutData ) );
		maLayoutCacheList.push_front( pLayoutData );

		// Limit cache size
		if ( maLayoutCache.size() > LAYOUT_CACHE_MAX_SIZE )
		{
			maLayoutCache.erase( maLayoutCacheList.back()->mpHash );
			maLayoutCacheList.back()->Release();
			maLayoutCacheList.pop_back();
		}
	}

	if ( pLayoutData )
		pLayoutData->Reference();

	return pLayoutData;
}

// ----------------------------------------------------------------------------

ImplATSLayoutData::ImplATSLayoutData( ImplLayoutArgs& rArgs, ImplATSLayoutDataHash *pLayoutHash, int nFallbackLevel, com_sun_star_vcl_VCLFont *pVCLFont ) :
	mnRefCount( 1 ),
	mpHash( pLayoutHash ),
	mpVCLFont( NULL ),
	maFontStyle( NULL ),
	mpNeedFallback( NULL ),
	mpFallbackFont( NULL ),
	maLayout( NULL ),
	mnGlyphCount( 0 ),
	mpGlyphInfoArray( NULL ),
	mpCharsToChars( NULL ),
	mpCharsToGlyphs( NULL ),
	mpCharAdvances( NULL ),
	maVerticalFontStyle( NULL ),
	mnBaselineDelta( 0 ),
	mbValid( false )
{
	if ( !mpHash )
	{
		Destroy();
		return;
	}

	mpVCLFont = new com_sun_star_vcl_VCLFont( pVCLFont->getJavaObject(), pVCLFont->getNativeFont() );
	if ( !mpVCLFont )
	{
		Destroy();
		return;
	}

	// Create font style
	if ( ATSUCreateStyle( &maFontStyle ) != noErr )
	{
		Destroy();
		return;
	}

	// Fix bug 1595 by allowing rare ligatures
	ATSUFontFeatureType aType;
	ATSUFontFeatureSelector aSelector;
	aType = kDiacriticsType;
	aSelector = kDecomposeDiacriticsSelector;
	if ( ATSUSetFontFeatures( maFontStyle, 1, &aType, &aSelector ) != noErr )
	{
		Destroy();
		return;
	}

	ATSUAttributeTag nTags[3];
	ByteCount nBytes[3];
	ATSUAttributeValuePtr nVals[3];

	// Set font
	nTags[0] = kATSUFontTag;
	nBytes[0] = sizeof( ATSUFontID );
	nVals[0] = &mpHash->mnFontID;

	// The OOo code will often layout fonts at unrealistically large sizes so
	// we need to use a more reasonably sized font or else we will exceed the
	// 32K Fixed data type limit that the ATSTrapezoid struct uses so we
	// preemptively limit font size to 100 like in the OOo 1.1.x code.
	float fSize = (float)mpHash->mnFontSize;
	float fAdjustedSize;
	if ( fSize > 100 )
		fAdjustedSize = 100;
	else
		fAdjustedSize = fSize;
	Fixed fCurrentSize = X2Fix( fAdjustedSize );
	nTags[1] = kATSUSizeTag;
	nBytes[1] = sizeof( Fixed );
	nVals[1] = &fCurrentSize;

	// Set antialiasing
	ATSStyleRenderingOptions nOptions;
	if ( mpHash->mbAntialiased )
		nOptions = kATSStyleApplyAntiAliasing;
	else
		nOptions = kATSStyleNoAntiAliasing;
	nTags[2] = kATSUStyleRenderingOptionsTag;
	nBytes[2] = sizeof( ATSStyleRenderingOptions );
	nVals[2] = &nOptions;

	if ( ATSUSetAttributes( maFontStyle, 3, nTags, nBytes, nVals ) != noErr )
	{
		Destroy();
		return;
	}

	if ( mpHash->mbVertical )
	{
		if ( ATSUCreateAndCopyStyle( maFontStyle, &maVerticalFontStyle ) == noErr )
		{
			ATSUVerticalCharacterType nVertical;
			if ( maVerticalFontStyle )
				nVertical = kATSUStronglyVertical;
			else
				nVertical = kATSUStronglyHorizontal;
			nTags[0] = kATSUVerticalCharacterTag;
			nBytes[0] = sizeof( ATSUVerticalCharacterType );
			nVals[0] = &nVertical;

			if ( ATSUSetAttributes( maVerticalFontStyle, 1, nTags, nBytes, nVals ) != noErr )
			{
				Destroy();
				return ;
			}

			BslnBaselineRecord aBaseline;
			memset( aBaseline, 0, sizeof( BslnBaselineRecord ) );
			if ( ATSUCalculateBaselineDeltas( maVerticalFontStyle, kBSLNRomanBaseline, aBaseline ) == noErr )
				mnBaselineDelta = Fix2Long( aBaseline[ kBSLNIdeographicCenterBaseline ] );
			if ( !mnBaselineDelta )
			{
				ATSFontMetrics aFontMetrics;
				ATSFontRef aFont = FMGetATSFontRefFromFont( mpHash->mnFontID );
				if ( ATSFontGetHorizontalMetrics( aFont, kATSOptionFlagsDefault, &aFontMetrics ) == noErr )
					mnBaselineDelta = Float32ToLong( ( ( ( fabs( aFontMetrics.descent ) + fabs( aFontMetrics.ascent ) ) / 2 ) - fabs( aFontMetrics.descent ) ) * mpVCLFont->getSize() );
			}
		}
	}

	if ( ATSUCreateTextLayoutWithTextPtr( mpHash->mpStr, kATSUFromTextBeginning, kATSUToTextEnd, mpHash->mnLen, 1, (const UniCharCount *)&mpHash->mnLen, &maFontStyle, &maLayout ) != noErr )
	{
		Destroy();
		return;
	}

	if ( maVerticalFontStyle )
	{
		for ( int i = 0; i < mpHash->mnLen; i++ )
		{
			if ( GetVerticalFlags( mpHash->mpStr[ i ] ) & GF_ROTMASK && ATSUSetRunStyle( maLayout, maVerticalFontStyle, i, 1 ) != noErr )
			{
				Destroy();
				return;
			}
		}
	}

	MacOSBoolean nDirection;
	if ( mpHash->mbRTL )
		nDirection = kATSURightToLeftBaseDirection;
	else
		nDirection = kATSULeftToRightBaseDirection;
	nTags[0] = kATSULineDirectionTag;
	nBytes[0] = sizeof( MacOSBoolean );
	nVals[0] = &nDirection;
	ATSLineLayoutOptions nLineOptions = kATSLineKeepSpacesOutOfMargin;
	nTags[1] = kATSULineLayoutOptionsTag;
	nBytes[1] = sizeof( ATSLineLayoutOptions );
	nVals[1] = &nLineOptions;

	if ( ATSUSetLayoutControls( maLayout, 2, nTags, nBytes, nVals ) != noErr )
	{
		Destroy();
		return;
	}

	// Fix bug 652 by forcing ATSUI to layout the text before we request any
	// glyph data
	ATSTrapezoid aTrapezoid;
	if ( ATSUGetGlyphBounds( maLayout, 0, 0, kATSUFromTextBeginning, kATSUToTextEnd, kATSUseFractionalOrigins, 1, &aTrapezoid, NULL ) != noErr )
	{
		Destroy();
		return;
	}

	// If the font is still too large that the text length exceeds the 32K
	// Fixed data type limit and becomes negative, reduce the font size until
	// we find a small enough size that works
	while ( Fix2X( aTrapezoid.upperRight.x ) < 0 )
	{
		// Reset font size
		fAdjustedSize /= 10;
		fCurrentSize = X2Fix( fAdjustedSize );
		nTags[0] = kATSUSizeTag;
		nBytes[0] = sizeof( Fixed );
		nVals[0] = &fCurrentSize;

		if ( fAdjustedSize < 1 || ATSUSetAttributes( maFontStyle, 1, nTags, nBytes, nVals ) != noErr || ATSUGetGlyphBounds( maLayout, 0, 0, kATSUFromTextBeginning, kATSUToTextEnd, kATSUseFractionalOrigins, 1, &aTrapezoid, NULL ) != noErr )
		{
			Destroy();
			return;
		}
	}

	ByteCount nBufSize;
	if ( ATSUGetGlyphInfo( maLayout, kATSUFromTextBeginning, kATSUToTextEnd, &nBufSize, NULL ) != noErr )
	{
		Destroy();
		return;
	}

	mpGlyphInfoArray = (ATSUGlyphInfoArray *)rtl_allocateMemory( nBufSize );

	ByteCount nRetSize = nBufSize;
	if ( ATSUGetGlyphInfo( maLayout, kATSUFromTextBeginning, kATSUToTextEnd, &nRetSize, mpGlyphInfoArray ) != noErr || nRetSize != nBufSize )
	{
		Destroy();
		return;
	}

	mnGlyphCount = mpGlyphInfoArray->numGlyphs;

	// Cache mapping of characters to glyph character indices
	nBufSize = mpHash->mnLen * sizeof( int );
	mpCharsToChars = (int *)rtl_allocateMemory( nBufSize );

	int i;
	for ( i = 0; i < mpHash->mnLen; i++ )
		mpCharsToChars[ i ] = -1;
	if ( mpHash->mbRTL )
	{
		i = 0;
		for ( int j = mpHash->mnLen - 1; j >= 0 && i < mnGlyphCount; j-- )
		{
			int nIndex = mpGlyphInfoArray->glyphs[ i ].charIndex;
			mpCharsToChars[ j ] = nIndex;
			for ( ; i < mnGlyphCount && mpGlyphInfoArray->glyphs[ i ].charIndex == nIndex; i++ )
				;
		}
	}
	else
	{
		i = 0;
		for ( int j = 0; j < mpHash->mnLen && i < mnGlyphCount; j++ )
		{
			int nIndex = mpGlyphInfoArray->glyphs[ i ].charIndex;
			mpCharsToChars[ j ] = nIndex;
			for ( ; i < mnGlyphCount && mpGlyphInfoArray->glyphs[ i ].charIndex == nIndex; i++ )
				;
		}
	}

	// Cache mapping of characters to glyphs
	nBufSize = mpHash->mnLen * sizeof( int );
	mpCharsToGlyphs = (int *)rtl_allocateMemory( nBufSize );

	for ( i = 0; i < mpHash->mnLen; i++ )
		mpCharsToGlyphs[ i ] = -1;
	for ( i = 0; i < mnGlyphCount; i++ )
	{
		int nIndex = mpGlyphInfoArray->glyphs[ i ].charIndex;
		if ( mpCharsToGlyphs[ nIndex ] < 0 || i < mpCharsToGlyphs[ nIndex ] )
			mpCharsToGlyphs[ nIndex ] = i;
	}

	// Cache glyph widths
	nBufSize = mpHash->mnLen * sizeof( long );
	mpCharAdvances = (long *)rtl_allocateMemory( nBufSize );
	memset( mpCharAdvances, 0, nBufSize );

	// Fix bug 448 by eliminating subpixel advances.
	float nScaleY = fSize / fAdjustedSize;
	UniCharArrayOffset nNextCaretPos = 0;
	for ( i = 0; i < mpHash->mnLen; i = nNextCaretPos )
	{
		if ( ATSUNextCursorPosition( maLayout, i, kATSUByCharacterCluster, &nNextCaretPos ) != noErr )
			nNextCaretPos = i + 1;

		// Make sure that all characters have a width greater than zero as
		// OOo can get confused by zero width characters
		if ( mpHash->mbRTL )
		{
			if ( ATSUGetGlyphBounds( maLayout, 0, 0, i, nNextCaretPos - i, kATSUseFractionalOrigins, 1, &aTrapezoid, NULL ) == noErr )
				mpCharAdvances[ i ] += Float32ToLong( Fix2X( aTrapezoid.upperRight.x - aTrapezoid.upperLeft.x ) * mpHash->mfFontScaleX * nScaleY );

			if ( mpCharAdvances[ i ] < 1 )
			{
				if ( nNextCaretPos < mpHash->mnLen )
					mpCharAdvances[ nNextCaretPos ] += mpCharAdvances[ i ] - 1;
				mpCharAdvances[ i ] = 1;
			}

			for ( int j = i + 1; j < nNextCaretPos; j++ )
			{
				if ( nNextCaretPos < mpHash->mnLen )
					mpCharAdvances[ nNextCaretPos ] += mpCharAdvances[ j ] - 1;
				mpCharAdvances[ j ] = 1;
			}
		}
		else
		{
			long nClusterWidth = 0;
			for ( int j = nNextCaretPos - 1; j >= i; j-- )
			{
				long nWidth = 0;
				if ( j != mpCharsToChars[ j ] )
				{
					if ( ATSUGetGlyphBounds( maLayout, 0, 0, j, 1, kATSUseFractionalOrigins, 1, &aTrapezoid, NULL ) == noErr )
						nWidth = Float32ToLong( Fix2X( aTrapezoid.upperRight.x - aTrapezoid.upperLeft.x ) * mpHash->mfFontScaleX * nScaleY ) + nClusterWidth;
				}
				else
				{
					if ( ATSUGetGlyphBounds( maLayout, 0, 0, j, nNextCaretPos - j, kATSUseFractionalOrigins, 1, &aTrapezoid, NULL ) == noErr )
						nWidth = Float32ToLong( Fix2X( aTrapezoid.upperRight.x - aTrapezoid.upperLeft.x ) * mpHash->mfFontScaleX * nScaleY );
				}

				if ( nWidth > nClusterWidth )
				{
					mpCharAdvances[ j ] = nWidth - nClusterWidth;
					nClusterWidth = nWidth;
				}
				else
				{
					mpCharAdvances[ j ] = 1;
				}
			}
		}
	}

	if ( fAdjustedSize != fSize )
	{
		// Reset font size
		fCurrentSize = X2Fix( fSize );
		nTags[0] = kATSUSizeTag;
		nBytes[0] = sizeof( Fixed );
		nVals[0] = &fCurrentSize;

		if ( ATSUSetAttributes( maFontStyle, 1, nTags, nBytes, nVals ) != noErr )
		{
			Destroy();
			return;
		}
	}

	// Find positions that require fallback fonts
	mpNeedFallback = NULL;
	UniCharArrayOffset nCurrentPos = 0;
	UniCharCount nOffset;
	ATSUFontID nFontID;
	for ( ; ; )
	{
		OSStatus nErr = ATSUMatchFontsToText( maLayout, nCurrentPos, kATSUToTextEnd, &nFontID, &nCurrentPos, &nOffset );
		if ( nErr == kATSUFontsNotMatched )
		{
			nCurrentPos += nOffset;
		}
		else if ( nErr == kATSUFontsMatched )
		{
			if ( !mpNeedFallback )
			{
				nBufSize = mpHash->mnLen * sizeof( bool );
				mpNeedFallback = (bool *)rtl_allocateMemory( nBufSize );
				memset( mpNeedFallback, 0, nBufSize );
			}

			int nOffsetPos = nCurrentPos + nOffset;
			for ( ; nCurrentPos < nOffsetPos; nCurrentPos++ )
				mpNeedFallback[ nCurrentPos ] = true;

			// Update font for next pass through
			if ( !mpFallbackFont )
			{
				SalData *pSalData = GetSalData();
				::std::map< int, JavaImplFontData* >::const_iterator it = pSalData->maNativeFontMapping.find( (int)nFontID );
				if ( it != pSalData->maNativeFontMapping.end() )
				{
					mpFallbackFont = new com_sun_star_vcl_VCLFont( it->second->maVCLFontName, mpHash->mnFontSize, mpVCLFont->getOrientation(), mpHash->mbAntialiased, mpHash->mbVertical, mpHash->mfFontScaleX, 0 );
				}
				else
				{
					rtl_freeMemory( mpNeedFallback );
					mpNeedFallback = NULL;
				}
			}
		}
		else
		{
			break;
		}
	}

	mbValid = true;
}

// ----------------------------------------------------------------------------

ImplATSLayoutData::~ImplATSLayoutData()
{
	Destroy();
}

// ----------------------------------------------------------------------------

void ImplATSLayoutData::Destroy()
{
	if ( mpHash )
	{
		if ( mpHash->mpStr )
			rtl_freeMemory( mpHash->mpStr );
		delete mpHash;
		mpHash = NULL;
	}

	if ( mpVCLFont )
	{
		delete mpVCLFont;
		mpVCLFont = NULL;
	}

	if ( maFontStyle )
	{
		ATSUDisposeStyle( maFontStyle );
		maFontStyle = NULL;
	}

	if ( mpNeedFallback )
	{
		rtl_freeMemory( mpNeedFallback );
		mpNeedFallback = NULL;
	}

	if ( mpFallbackFont )
	{
		delete mpFallbackFont;
		mpFallbackFont = NULL;
	}

	if ( maLayout )
	{
		ATSUDisposeTextLayout( maLayout );
		maLayout = NULL;
	}

	mnGlyphCount = 0;

	if ( mpGlyphInfoArray )
	{
		rtl_freeMemory( mpGlyphInfoArray );
		mpGlyphInfoArray = NULL;
	}

	if ( mpCharsToChars )
	{
		rtl_freeMemory( mpCharsToChars );
		mpCharsToChars = NULL;
	}

	if ( mpCharsToGlyphs )
	{
		rtl_freeMemory( mpCharsToGlyphs );
		mpCharsToGlyphs = NULL;
	}

	if ( mpCharAdvances )
	{
		rtl_freeMemory( mpCharAdvances );
		mpCharAdvances = NULL;
	}

	if ( maVerticalFontStyle )
	{
		ATSUDisposeStyle( maVerticalFontStyle );
		maVerticalFontStyle = NULL;
	}

	mnBaselineDelta = 0;
	mbValid = false;
}

// ----------------------------------------------------------------------------

void ImplATSLayoutData::Reference() const
{ 
	++mnRefCount;
}

// ----------------------------------------------------------------------------

void ImplATSLayoutData::Release() const
{
	if ( --mnRefCount > 0 )
		return;

	// const_cast because some compilers violate ANSI C++ spec
	delete const_cast<ImplATSLayoutData*>( this );
}

// ============================================================================

static OSStatus SalATSCubicMoveToCallback( const Float32Point *pPoint, void *pData )
{
	::std::list< Polygon > *pPolygonList = (::std::list< Polygon > *)pData;

	Point aPoint( Float32ToLong( pPoint->x ), Float32ToLong( pPoint->y ) );
	pPolygonList->push_back( Polygon( 1, &aPoint ) );

	return noErr;
}

// ----------------------------------------------------------------------------

static OSStatus SalATSCubicLineToCallback( const Float32Point *pPoint, void *pData )
{
	::std::list< Polygon > *pPolygonList = (::std::list< Polygon > *)pData;

	pPolygonList->back().Insert( pPolygonList->back().GetSize(), Point( Float32ToLong( pPoint->x ), Float32ToLong( pPoint->y ) ) );

	return noErr;
}

// ----------------------------------------------------------------------------

static OSStatus SalATSCubicCurveToCallback( const Float32Point *pStart, const Float32Point *pOffCurve, const Float32Point *pEnd, void *pData )
{
	::std::list< Polygon > *pPolygonList = (::std::list< Polygon > *)pData;

	Point aStart( Float32ToLong( pStart->x ), Float32ToLong( pStart->y ) );
	Point aOffCurve( Float32ToLong( pOffCurve->x ), Float32ToLong( pOffCurve->y ) );
	Point aEnd( Float32ToLong( pEnd->x ), Float32ToLong( pEnd->y ) );

	USHORT nSize = pPolygonList->back().GetSize();
	pPolygonList->back().Insert( nSize++, aStart, POLY_CONTROL );
	pPolygonList->back().Insert( nSize++, aOffCurve, POLY_CONTROL );
	pPolygonList->back().Insert( nSize, aEnd );

	return noErr;
}

// ----------------------------------------------------------------------------

static OSStatus SalATSCubicClosePathCallback( void *pData )
{
	::std::list< Polygon > *pPolygonList = (::std::list< Polygon > *)pData;

	USHORT nSize = pPolygonList->back().GetSize();
	if ( nSize > 1 )
		pPolygonList->back().Insert( nSize, Point( pPolygonList->back().GetPoint( 0 ) ) );

	return noErr;
}

// ============================================================================

SalLayout *JavaSalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
	if ( nFallbackLevel && rArgs.mnFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING )
		return NULL;
	else
		return new SalATSLayout( this, nFallbackLevel );
}

// ============================================================================

void SalATSLayout::ClearLayoutDataCache()
{
	ImplATSLayoutData::ClearLayoutDataCache();
}

// ----------------------------------------------------------------------------

SalATSLayout::SalATSLayout( JavaSalGraphics *pGraphics, int nFallbackLevel ) :
	mpGraphics( pGraphics ),
	mnFallbackLevel( nFallbackLevel ),
	mpVCLFont( NULL ),
	mpKashidaLayoutData( NULL )
{
	if ( mnFallbackLevel )
	{
		::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator it = mpGraphics->maFallbackFonts.find( mnFallbackLevel );
		if ( it != mpGraphics->maFallbackFonts.end() )
		{
			int nNativeFont = it->second->getNativeFont();	
			mpVCLFont = new com_sun_star_vcl_VCLFont( it->second->getJavaObject(), nNativeFont );

			// Prevent infinite fallback
			if ( mpVCLFont )
			{
				for ( ::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator ffit = mpGraphics->maFallbackFonts.begin(); ffit != mpGraphics->maFallbackFonts.end(); ++ffit )
				{
					if ( ffit->second->getNativeFont() == nNativeFont && ffit->first < mnFallbackLevel )
					{
						delete mpVCLFont;
						mpVCLFont = NULL;
						break;
					}
				}
			}
		}
	}
	else
	{
		mpVCLFont = new com_sun_star_vcl_VCLFont( mpGraphics->mpVCLFont->getJavaObject(), mpGraphics->mpVCLFont->getNativeFont() );
	}
}

// ----------------------------------------------------------------------------

SalATSLayout::~SalATSLayout()
{
	Destroy();

	if ( mpVCLFont )
		delete mpVCLFont;
}

// ----------------------------------------------------------------------------

void SalATSLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
	GenericSalLayout::AdjustLayout( rArgs );

	if ( rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN && ! ( rArgs.mnFlags & SAL_LAYOUT_VERTICAL ) )
		ApplyAsianKerning( rArgs.mpStr, rArgs.mnLength );

	if ( rArgs.mnFlags & SAL_LAYOUT_KASHIDA_JUSTIFICATON && rArgs.mpDXArray && mpKashidaLayoutData )
		KashidaJustify( mpKashidaLayoutData->mpGlyphInfoArray->glyphs[ 0 ].glyphID, mpKashidaLayoutData->mpCharAdvances[ 0 ] );
}

// ----------------------------------------------------------------------------

bool SalATSLayout::LayoutText( ImplLayoutArgs& rArgs )
{
	Destroy();

	bool bRet = false;
	rArgs.mnFlags |= SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;

	if ( !mpVCLFont )
		return bRet;

	// Aggregate runs
	bool bRunRTL;
	int nMinCharPos;
	int nEndCharPos;
	rArgs.ResetPos();
	if ( !mnFallbackLevel )
	{
		mpGraphics->maFallbackRuns.Clear();
		while ( rArgs.GetNextRun( &nMinCharPos, &nEndCharPos, &bRunRTL ) )
		{
			mpGraphics->maFallbackRuns.AddRun( nMinCharPos, nEndCharPos, bRunRTL );
			maRuns.AddRun( nMinCharPos, nEndCharPos, bRunRTL );
		}
	}
	else
	{
		bool bFallbackRunRTL;
		int nMinFallbackCharPos;
		int nEndFallbackCharPos;
		bool bLastFallbackRunRTL;
		int nMinLastFallbackCharPos;
		int nEndLastFallbackCharPos;
		while ( rArgs.GetNextRun( &nMinCharPos, &nEndCharPos, &bRunRTL ) )
		{
			mpGraphics->maFallbackRuns.ResetPos();
			while ( mpGraphics->maFallbackRuns.GetRun( &nMinFallbackCharPos, &nEndFallbackCharPos, &bFallbackRunRTL ) )
			{
				mpGraphics->maFallbackRuns.NextRun();
				if ( nMinCharPos >= nMinFallbackCharPos && nEndCharPos <= nEndFallbackCharPos )
				{
					maRuns.AddRun( nMinCharPos, nEndCharPos, bRunRTL );
					break;
				}
			}
		}
	}

	Point aPos;
	maRuns.ResetPos();
	mpGraphics->maFallbackRuns.ResetPos();
	while ( maRuns.GetRun( &nMinCharPos, &nEndCharPos, &bRunRTL ) )
	{
		maRuns.NextRun();

		// Check if this run will need Kashida justification
		if ( bRunRTL && ( rArgs.mpDXArray || rArgs.mnLayoutWidth ) && !mpKashidaLayoutData )
		{
			UErrorCode nErrorCode = U_ZERO_ERROR;
			UScriptCode eScriptCode = uscript_getScript( rArgs.mpStr[ nMinCharPos ], &nErrorCode );
			if ( eScriptCode == USCRIPT_ARABIC || eScriptCode == USCRIPT_SYRIAC )
			{
				sal_Unicode aKashida[ 1 ];
				aKashida[ 0 ] = 0x0640;
				ImplLayoutArgs aKashidaArgs( aKashida, 1, 0, 1, rArgs.mnFlags | SAL_LAYOUT_BIDI_STRONG | SAL_LAYOUT_BIDI_RTL );

				mpKashidaLayoutData = ImplATSLayoutData::GetLayoutData( aKashidaArgs, mnFallbackLevel, mpVCLFont );
				if ( mpKashidaLayoutData )
				{
					if ( mpKashidaLayoutData->mpNeedFallback || mpKashidaLayoutData->mnGlyphCount != 1 )
					{
						mpKashidaLayoutData->Release();
						mpKashidaLayoutData = NULL;
					}
					else
					{
						rArgs.mnFlags |= SAL_LAYOUT_KASHIDA_JUSTIFICATON;
					}
				}
			}
		}

		if ( !mpGraphics->maFallbackRuns.PosIsInRun( nMinCharPos ) )
		{
			mpGraphics->maFallbackRuns.ResetPos();
			while ( !mpGraphics->maFallbackRuns.PosIsInRun( nMinCharPos ) )
				mpGraphics->maFallbackRuns.NextRun();
		}

		bool bFallbackRunRTL;
		int nMinFallbackCharPos;
		int nEndFallbackCharPos;
		if ( !mpGraphics->maFallbackRuns.GetRun( &nMinFallbackCharPos, &nEndFallbackCharPos, &bFallbackRunRTL ) )
			return false;

		// Turn off direction analysis
		int nRunFlags = rArgs.mnFlags | SAL_LAYOUT_BIDI_STRONG;
		if ( bRunRTL )
			nRunFlags |= SAL_LAYOUT_BIDI_RTL;
		else
			nRunFlags &= ~SAL_LAYOUT_BIDI_RTL;

		ImplLayoutArgs aArgs( rArgs.mpStr, rArgs.mnLength, nMinCharPos, nEndCharPos, nRunFlags );
		ImplLayoutArgs aFallbackArgs( rArgs.mpStr, rArgs.mnLength, nMinFallbackCharPos, nEndFallbackCharPos, nRunFlags );

		ImplATSLayoutData *pLayoutData = ImplATSLayoutData::GetLayoutData( aFallbackArgs, mnFallbackLevel, mpVCLFont );
		if ( !pLayoutData )
			return false;

		// Create fallback runs
		if ( pLayoutData->mpNeedFallback && pLayoutData->mpFallbackFont )
		{
			bool bPosRTL;
			int nCharPos = -1;
			aArgs.ResetPos();
			while ( aArgs.GetNextPos( &nCharPos, &bPosRTL ) )
			{
				if ( pLayoutData->mpNeedFallback[ nCharPos - aFallbackArgs.mnMinCharPos ] )
					rArgs.NeedFallback( nCharPos, bPosRTL );
			}

			int nNextLevel = mnFallbackLevel + 1;
			::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator it = mpGraphics->maFallbackFonts.find( nNextLevel );
			if ( it != mpGraphics->maFallbackFonts.end() )
				delete it->second;
			mpGraphics->maFallbackFonts[ nNextLevel ] = new com_sun_star_vcl_VCLFont( pLayoutData->mpFallbackFont->getJavaObject(), pLayoutData->mpFallbackFont->getNativeFont() );
			rArgs.mnFlags &= ~SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
		}

		// Calculate and cache glyph advances
		bool bPosRTL;
		int nCharPos = -1;
		aArgs.ResetPos();
		while ( aArgs.GetNextPos( &nCharPos, &bPosRTL ) )
		{
			bool bFirstGlyph = true;
			long nCharWidth = 1;
			int nIndex = pLayoutData->mpCharsToChars[ nCharPos - aFallbackArgs.mnMinCharPos ];
			if ( nIndex >= 0 )
			{
				ImplATSLayoutData *pCurrentLayoutData = pLayoutData;

				sal_Unicode nChar = pCurrentLayoutData->mpHash->mpStr[ nIndex ];

				if ( bPosRTL )
				{
					// Fix bugs 1637 and 1797 by laying out mirrored characters
					// separately
					sal_Unicode nMirroredChar = GetMirroredChar( nChar );
					if ( nMirroredChar != nChar )
					{
						::std::map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.find( nChar );
						if ( mit == maMirroredLayoutData.end() )
						{
							sal_Unicode aMirrored[ 1 ];
							aMirrored[ 0 ] = nMirroredChar;
							ImplLayoutArgs aMirroredArgs( aMirrored, 1, 0, 1, ( rArgs.mnFlags & ~SAL_LAYOUT_BIDI_RTL ) | SAL_LAYOUT_BIDI_STRONG );
							pCurrentLayoutData = ImplATSLayoutData::GetLayoutData( aMirroredArgs, mnFallbackLevel, mpVCLFont );
							if ( pCurrentLayoutData )
							{
								if ( pCurrentLayoutData->mpNeedFallback )
								{
									pCurrentLayoutData->Release();
									pCurrentLayoutData = pLayoutData;
									rArgs.NeedFallback( nCharPos, bPosRTL );
								}
								else
								{
									maMirroredLayoutData[ nChar ] = pCurrentLayoutData;
								}
							}
						}
						else
						{
							pCurrentLayoutData = mit->second;
						}

						if ( pCurrentLayoutData != pLayoutData )
							nIndex = 0;
					}
				}

				nCharWidth = pCurrentLayoutData->mpCharAdvances[ nIndex ];
				for ( int i = pCurrentLayoutData->mpCharsToGlyphs[ nIndex ]; i >= 0 && i < pCurrentLayoutData->mnGlyphCount && pCurrentLayoutData->mpGlyphInfoArray->glyphs[ i ].charIndex == nIndex; i++ )
				{
					long nGlyph = pCurrentLayoutData->mpGlyphInfoArray->glyphs[ i ].glyphID;
					if ( pCurrentLayoutData->maVerticalFontStyle )
						nGlyph |= GetVerticalFlags( nChar );

					int nGlyphFlags = bFirstGlyph ? 0 : GlyphItem::IS_IN_CLUSTER;
					if ( bPosRTL )
						nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;

					AppendGlyph( GlyphItem( nCharPos, nGlyph, aPos, nGlyphFlags, nCharWidth ) );

					if ( bFirstGlyph )
					{
						aPos.X() += nCharWidth;
						nCharWidth = 0;
						bFirstGlyph = false;
					}
				}
			}

			if ( bFirstGlyph )
			{
				AppendGlyph( GlyphItem( nCharPos, 0x0020 | GF_ISCHAR, aPos, bPosRTL ? GlyphItem::IS_RTL_GLYPH : 0, nCharWidth ) );
				aPos.X() += nCharWidth;
			}
		}

		if ( nCharPos >= 0 )
			bRet = true;

		maLayoutData.push_back( pLayoutData );
		maLayoutMinCharPos.push_back( aFallbackArgs.mnMinCharPos );
	}

	return bRet;
}

// ----------------------------------------------------------------------------

void SalATSLayout::DrawText( SalGraphics& rGraphics ) const
{
	if ( !maLayoutData.size() )
		return;

	int nMaxGlyphs = maLayoutData.front()->mnGlyphCount;
	long aGlyphArray[ nMaxGlyphs ];
	long aDXArray[ nMaxGlyphs ];
	int aCharPosArray[ nMaxGlyphs ];

	Point aPos;
	for ( int nStart = 0; ; )
	{
		int nGlyphCount = GetNextGlyphs( nMaxGlyphs, aGlyphArray, aPos, nStart, aDXArray, aCharPosArray );

		if ( !nGlyphCount )
			break;

		int i;
		for ( i = 0; i < nGlyphCount && IsSpacingGlyph( aGlyphArray[ i ] ); i++ )
			;
		if ( i )
		{
			while ( i < nGlyphCount && aCharPosArray[ i ] == aCharPosArray[ 0 ] )
				i++;
			if ( i < nGlyphCount )
				nStart = aCharPosArray[ i ];
			continue;
		}

		for ( i = 0 ; i < nGlyphCount && !IsSpacingGlyph( aGlyphArray[ i ] ); i++ )
			;
		if ( i < nGlyphCount )
		{
			nGlyphCount = i;
			if ( nGlyphCount )
				nStart = aCharPosArray[ i ];
			else
				continue;
		}

		long nTranslateX = 0;
		long nTranslateY = 0;

		long nGlyphOrientation = aGlyphArray[ 0 ] & GF_ROTMASK;
		if ( nGlyphOrientation )
		{
			nStart -= nGlyphCount - 1;
			nGlyphCount = 1;

			long nX;
			long nY;
			ImplATSLayoutData *pFoundLayout = GetVerticalGlyphTranslation( aGlyphArray[ 0 ], aCharPosArray[ 0 ], nX, nY );
			if ( nGlyphOrientation == GF_ROTL )
			{
				nTranslateX = nX;
				if ( pFoundLayout )
					nTranslateY = Float32ToLong( pFoundLayout->mpHash->mfFontScaleX * nY );
				else
					nTranslateY = nY;
			}
			else
			{
				nTranslateX = nX;
				if ( pFoundLayout )
					nTranslateY = Float32ToLong( pFoundLayout->mpHash->mfFontScaleX * ( aDXArray[ 0 ] - nY ) );
				else
					nTranslateY = nY;
			}
		}

		JavaSalGraphics& rJavaGraphics = (JavaSalGraphics&)rGraphics;
		int nLastStart = 0;
		int nLastX = 0;
		long *pGlyphArray = aGlyphArray;
		long *pDXArray = aDXArray;
		for ( i = 0; i < nGlyphCount; i++ )
		{
			nLastX += aDXArray[ i ];
			aGlyphArray[ i ] &= GF_IDXMASK;
			if ( aGlyphArray[ i ] >= 0x0000ffff )
			{
				if ( i - nLastStart )
					rJavaGraphics.mpVCLGraphics->drawGlyphs( aPos.X(), aPos.Y(), i - nLastStart, pGlyphArray, pDXArray, mpVCLFont, rJavaGraphics.mnTextColor, GetOrientation(), nGlyphOrientation, nTranslateX, nTranslateY );
				nLastStart = i + 1;
				pGlyphArray = aGlyphArray + nLastStart;
				pDXArray = aDXArray + nLastStart;
				aPos.X() += nLastX;
				nLastX = 0;
				continue;
			}
		}

		if ( i - nLastStart )
			rJavaGraphics.mpVCLGraphics->drawGlyphs( aPos.X(), aPos.Y(), i - nLastStart, pGlyphArray, pDXArray, mpVCLFont, rJavaGraphics.mnTextColor, GetOrientation(), nGlyphOrientation, nTranslateX, nTranslateY );
	}
}

// ----------------------------------------------------------------------------

bool SalATSLayout::GetOutline( SalGraphics& rGraphics, B2DPolyPolygonVector& rVector ) const
{
	bool bRet = false;

	if ( !maLayoutData.size() )
		return bRet;

	if ( !pATSCubicMoveToUPP )
		pATSCubicMoveToUPP = NewATSCubicMoveToUPP( SalATSCubicMoveToCallback );
	if ( !pATSCubicLineToUPP )
		pATSCubicLineToUPP = NewATSCubicLineToUPP( SalATSCubicLineToCallback );
	if ( !pATSCubicCurveToUPP )
		pATSCubicCurveToUPP = NewATSCubicCurveToUPP( SalATSCubicCurveToCallback );
	if ( !pATSCubicClosePathUPP )
		pATSCubicClosePathUPP = NewATSCubicClosePathUPP( SalATSCubicClosePathCallback );
	if ( !pATSCubicMoveToUPP || !pATSCubicLineToUPP || !pATSCubicCurveToUPP  || !pATSCubicClosePathUPP )
		return bRet;

	int nMaxGlyphs( 1 );
	long aGlyphArray[ nMaxGlyphs ];
	long aDXArray[ nMaxGlyphs ];
	int aCharPosArray[ nMaxGlyphs ];

	Point aPos;
	int nRunIndex = 0;
	ImplATSLayoutData *pLayoutData = maLayoutData[ nRunIndex ];
	int nMinCharPos = maLayoutMinCharPos[ nRunIndex ];
	maRuns.ResetPos();
	for ( int nStart = 0; ; )
	{
		int nGlyphCount = GetNextGlyphs( nMaxGlyphs, aGlyphArray, aPos, nStart, aDXArray, aCharPosArray );

		if ( !nGlyphCount )
			break;

		if ( IsSpacingGlyph( aGlyphArray[ 0 ] ) || ( aGlyphArray[ 0 ] & GF_IDXMASK ) >= 0x0000ffff )
		{
			bRet = true;
			continue;
		}

		if ( !maRuns.PosIsInRun( aCharPosArray[ 0 ] ) )
		{
			nRunIndex = 0;
			pLayoutData = maLayoutData[ nRunIndex ];
			nMinCharPos = maLayoutMinCharPos[ nRunIndex ];
			maRuns.ResetPos();
			while ( !maRuns.PosIsInRun( aCharPosArray[ 0 ] ) )
			{
				maRuns.NextRun();
				nRunIndex++;
				if ( nRunIndex < maLayoutData.size() )
				{
					pLayoutData = maLayoutData[ nRunIndex ];
					nMinCharPos = maLayoutMinCharPos[ nRunIndex ];
				}
				else
				{
					pLayoutData = NULL;
					break;
				}
			}
		}

		if ( !pLayoutData )
			continue;

		int nIndex = pLayoutData->mpCharsToChars[ aCharPosArray[ 0 ] - nMinCharPos ];
		if ( nIndex < 0 )
			continue;

		ImplATSLayoutData *pCurrentLayoutData = pLayoutData;

		// Check if this is a mirrored character
		sal_Unicode nChar = pCurrentLayoutData->mpHash->mpStr[ nIndex ];
		::std::map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.find( nChar );
		if ( mit != maMirroredLayoutData.end() )
		{
			pCurrentLayoutData = mit->second;
			nIndex = 0;
		}

		for ( int i = pCurrentLayoutData->mpCharsToGlyphs[ nIndex ]; i >= 0 && i < pCurrentLayoutData->mnGlyphCount && pCurrentLayoutData->mpGlyphInfoArray->glyphs[ i ].charIndex == nIndex; i++ )
		{
			long nGlyph = pCurrentLayoutData->mpGlyphInfoArray->glyphs[ i ].glyphID;
			if ( ( aGlyphArray[ 0 ] & GF_IDXMASK ) != nGlyph )
				continue;

			::std::list< Polygon > aPolygonList;
			OSStatus nErr;
			if ( ATSUGlyphGetCubicPaths( pCurrentLayoutData->mpGlyphInfoArray->glyphs[ i ].style, pCurrentLayoutData->mpGlyphInfoArray->glyphs[ i ].glyphID, pATSCubicMoveToUPP, pATSCubicLineToUPP, pATSCubicCurveToUPP, pATSCubicClosePathUPP, (void *)&aPolygonList, &nErr ) != noErr || nErr != noErr )
				continue;

			PolyPolygon aPolyPolygon;
			while ( aPolygonList.size() )
			{
				aPolyPolygon.Insert( aPolygonList.front() );
				aPolygonList.pop_front();
			}

			aPolyPolygon.Move( aPos.X(), aPos.Y() );

			if ( pCurrentLayoutData->maVerticalFontStyle )
			{
				long nGlyphOrientation = aGlyphArray[ 0 ] & GF_ROTMASK;
				if ( nGlyphOrientation )
				{
					if ( nGlyphOrientation == GF_ROTL )
					{
						aPolyPolygon.Rotate( Point( 0, 0 ), 900 );
						aPolyPolygon.Move( aPolyPolygon.GetBoundRect().nLeft * -1, pCurrentLayoutData->mnBaselineDelta * -1 );
					}
					else
					{
						aPolyPolygon.Rotate( Point( 0, 0 ), 2700 );
						aPolyPolygon.Move( aDXArray[ 0 ] - aPolyPolygon.GetBoundRect().nRight, pCurrentLayoutData->mnBaselineDelta * -1 );
					}
				}
			}

			rVector.push_back( aPolyPolygon.getB2DPolyPolygon() );
			bRet = true;
			break;
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------------

void SalATSLayout::Destroy()
{
	maRuns.Clear();

	while ( maLayoutData.size() )
	{
		maLayoutData.back()->Release();
		maLayoutData.pop_back();
	}

	maLayoutMinCharPos.clear();

	if ( mpKashidaLayoutData )
	{
		mpKashidaLayoutData->Release();
		mpKashidaLayoutData = NULL;
	}

	if ( maMirroredLayoutData.size() )
	{
		for ( ::std::map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.begin(); mit != maMirroredLayoutData.end(); ++mit )
			mit->second->Release();
		maMirroredLayoutData.clear();
	}
}

// ----------------------------------------------------------------------------

ImplATSLayoutData *SalATSLayout::GetVerticalGlyphTranslation( long nGlyph, int nCharPos, long& nX, long& nY ) const
{
	ImplATSLayoutData *pRet = NULL;

	nX = 0;
	nY = 0;

	if ( !maLayoutData.size() )
		return pRet;

	int nRunIndex = 0;
	ImplATSLayoutData *pLayoutData = maLayoutData[ nRunIndex ];
	int nMinCharPos = maLayoutMinCharPos[ nRunIndex ];
	maRuns.ResetPos();
	while ( !maRuns.PosIsInRun( nCharPos ) )
	{
		maRuns.NextRun();
		nRunIndex++;
		if ( nRunIndex < maLayoutData.size() )
		{
			pLayoutData = maLayoutData[ nRunIndex ];
			nMinCharPos = maLayoutMinCharPos[ nRunIndex ];
		}
		else
		{
			pLayoutData = NULL;
			break;
		}
	}

	if ( !pLayoutData )
		return pRet;

	int nIndex = pLayoutData->mpCharsToChars[ nCharPos - nMinCharPos ];
	if ( nIndex < 0 )
		return pRet;

	pRet = pLayoutData;

	// Check if this is a mirrored character
	sal_Unicode nChar = pRet->mpHash->mpStr[ nIndex ];
	::std::map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.find( nChar );
	if ( mit != maMirroredLayoutData.end() )
	{
		pRet = mit->second;
		nIndex = 0;
	}

	long nGlyphOrientation = nGlyph & GF_ROTMASK;

	if ( maLayoutData.front()->maVerticalFontStyle && nGlyphOrientation & GF_ROTMASK )
	{
		GlyphID nGlyphID = (GlyphID)( nGlyph & GF_IDXMASK );

		ATSGlyphScreenMetrics aVerticalMetrics;
		ATSGlyphScreenMetrics aHorizontalMetrics;
		if ( ATSUGlyphGetScreenMetrics( pRet->maVerticalFontStyle, 1, &nGlyphID, sizeof( GlyphID ), pRet->mpHash->mbAntialiased, pRet->mpHash->mbAntialiased, &aVerticalMetrics ) == noErr && ATSUGlyphGetScreenMetrics( pRet->maFontStyle, 1, &nGlyphID, sizeof( GlyphID ), pRet->mpHash->mbAntialiased, pRet->mpHash->mbAntialiased, &aHorizontalMetrics ) == noErr )
		{
			nX = Float32ToLong( aVerticalMetrics.topLeft.x - aHorizontalMetrics.topLeft.x );
			if ( nGlyphOrientation == GF_ROTL )
				nX += pRet->mnBaselineDelta;
			else
				nX -= pRet->mnBaselineDelta;
			nY = Float32ToLong( aHorizontalMetrics.topLeft.y - aVerticalMetrics.topLeft.y );
		}
	}

	return pRet;
}
