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
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
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

#define LAYOUT_CACHE_MAX_SIZE 4096

inline long Float32ToLong( Float32 f ) { return (long)( f + 0.5 ); }

struct ImplATSLayoutDataHash {
	int					mnFallbackLevel;
	int					mnBeginChars;
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
	int					mnFallbackLevel;
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
	static ImplATSLayoutData*	GetLayoutData( ImplLayoutArgs& rArgs, int nFallbackLevel, ::vcl::com_sun_star_vcl_VCLFont *pVCLFont, int nBeginChars, int nEndChars );

						ImplATSLayoutData( ImplLayoutArgs& rArgs, ImplATSLayoutDataHash *pLayoutHash, int nFallbackLevel, ::vcl::com_sun_star_vcl_VCLFont *pVCLFont );
						~ImplATSLayoutData();

	void				Destroy();
	bool				IsValid() const { return mbValid; }
	void				Reference() const;
	void				Release() const;
};

using namespace osl;
using namespace rtl;
using namespace vcl;

// ============================================================================

bool ImplHashEquality::operator()( const ImplATSLayoutDataHash *p1, const ImplATSLayoutDataHash *p2 ) const
{
	// HACK: Don't compare leading and trailing spaces as they might not be
	// valid memory
	return ( p1->mnFallbackLevel == p2->mnFallbackLevel &&
		p1->mnBeginChars == p2->mnBeginChars &&
		p1->mnLen == p2->mnLen &&
		p1->mnFontID == p2->mnFontID &&
		p1->mnFontSize == p2->mnFontSize &&
		p1->mfFontScaleX == p2->mfFontScaleX &&
		p1->mbAntialiased == p2->mbAntialiased &&
		p1->mbRTL == p2->mbRTL &&
		p1->mbVertical == p2->mbVertical &&
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

ImplATSLayoutData *ImplATSLayoutData::GetLayoutData( ImplLayoutArgs& rArgs, int nFallbackLevel, com_sun_star_vcl_VCLFont *pVCLFont, int nBeginChars, int nEndChars )
{
	ImplATSLayoutData *pLayoutData = NULL;

	ImplATSLayoutDataHash *pLayoutHash = new ImplATSLayoutDataHash();
	pLayoutHash->mnFallbackLevel = nFallbackLevel;
	pLayoutHash->mnBeginChars = nBeginChars;
	pLayoutHash->mnLen = rArgs.mnEndCharPos - rArgs.mnMinCharPos + pLayoutHash->mnBeginChars + nEndChars;
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
	memcpy( pLayoutHash->mpStr, rArgs.mpStr + rArgs.mnMinCharPos - pLayoutHash->mnBeginChars, pLayoutHash->mnLen * sizeof( sal_Unicode ) );
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
	mnFallbackLevel( nFallbackLevel ),
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

	mpVCLFont = new com_sun_star_vcl_VCLFont( pVCLFont->getJavaObject() );
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

	// Fix bug 449 by turning off Unicode composition and turn off rare
	// ligatures to be more consistent with TextEdit's layout behavior
	ATSUFontFeatureType aTypes[2];
	ATSUFontFeatureSelector aSelectors[2];
	aTypes[0] = kDiacriticsType;
	aSelectors[0] = kDecomposeDiacriticsSelector;
	aTypes[1] = kLigaturesType;
	aSelectors[1] = kRareLigaturesOffSelector;
	if ( ATSUSetFontFeatures( maFontStyle, 2, aTypes, aSelectors ) != noErr )
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

	// Set font size
	Fixed nSize = Long2Fix( mpHash->mnFontSize );
	nTags[1] = kATSUSizeTag;
	nBytes[1] = sizeof( Fixed );
	nVals[1] = &nSize;

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
				mnBaselineDelta = ( ( mpVCLFont->getDescent() + mpVCLFont->getAscent() ) / 2 ) - mpVCLFont->getDescent();
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

	long nFullWidth = Float32ToLong( Fix2X( aTrapezoid.upperRight.x - aTrapezoid.upperLeft.x ) * mpHash->mfFontScaleX );

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
	int nLastValidCharPos = -1;
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
				mpCharAdvances[ i ] += Float32ToLong( Fix2X( aTrapezoid.upperRight.x - aTrapezoid.upperLeft.x ) * mpHash->mfFontScaleX );

			if ( mpCharAdvances[ i ] < 1 )
			{
				if ( nNextCaretPos < mpHash->mnLen )
					mpCharAdvances[ nNextCaretPos ] += mpCharAdvances[ i ] - 1;
				mpCharAdvances[ i ] = 1;
			}

			bool bValidChar = true;
			for ( int j = mpCharsToGlyphs[ i ]; j >= 0 && j < mnGlyphCount && mpGlyphInfoArray->glyphs[ j ].charIndex == i; j++ )
			{
				if ( mpGlyphInfoArray->glyphs[ j ].glyphID == 0xffff || mpGlyphInfoArray->glyphs[ j ].layoutFlags & kATSGlyphInfoTerminatorGlyph )
				{
					bValidChar = false;
					if ( nNextCaretPos < mpHash->mnLen )
						mpCharAdvances[ nNextCaretPos ]--;
					if ( nLastValidCharPos >= 0 )
						mpCharAdvances[ nLastValidCharPos ] += mpCharAdvances[ i ];
					mpCharAdvances[ i ] = 1;
					break;
				}
			}

			if ( bValidChar )
				nLastValidCharPos = i;

			for ( j = i + 1; j < nNextCaretPos; j++ )
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
						nWidth = Float32ToLong( Fix2X( aTrapezoid.upperRight.x - aTrapezoid.upperLeft.x ) * mpHash->mfFontScaleX ) + nClusterWidth;
				}
				else
				{
					if ( ATSUGetGlyphBounds( maLayout, 0, 0, j, nNextCaretPos - j, kATSUseFractionalOrigins, 1, &aTrapezoid, NULL ) == noErr )
						nWidth = Float32ToLong( Fix2X( aTrapezoid.upperRight.x - aTrapezoid.upperLeft.x ) * mpHash->mfFontScaleX );
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

			if ( mpHash->mbRTL && nOffsetPos < mpHash->mnLen && mpHash->mpStr[ nOffsetPos ] == 0x0020 )
				mpNeedFallback[ nOffsetPos ] = true;

			// Update font for next pass through
			if ( !mpFallbackFont )
			{
				SalData *pSalData = GetSalData();
				::std::map< void*, ImplFontData* >::const_iterator it = pSalData->maNativeFontMapping.find( (void *)nFontID );
				if ( it != pSalData->maNativeFontMapping.end() )
				{
					com_sun_star_vcl_VCLFont *pVCLFont = (com_sun_star_vcl_VCLFont *)it->second->mpSysData;
					mpFallbackFont = pVCLFont->deriveFont( mpHash->mnFontSize, mpVCLFont->getOrientation(), mpHash->mbAntialiased, mpHash->mbVertical, mpHash->mfFontScaleX );
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

SalLayout *SalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
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

SalATSLayout::SalATSLayout( SalGraphics *pGraphics, int nFallbackLevel ) :
	mpGraphics( pGraphics ),
	mnFallbackLevel( nFallbackLevel ),
	mpVCLFont( NULL ),
	mpKashidaLayoutData( NULL )
{
	if ( mnFallbackLevel )
	{
		::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator it = mpGraphics->maGraphicsData.maFallbackFonts.find( mnFallbackLevel );
		if ( it != mpGraphics->maGraphicsData.maFallbackFonts.end() )
			mpVCLFont = new com_sun_star_vcl_VCLFont( it->second->getJavaObject() );
	}
	else
	{
		mpVCLFont = new com_sun_star_vcl_VCLFont( mpGraphics->maGraphicsData.mpVCLFont->getJavaObject() );
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

	if ( !mnFallbackLevel )
		mpGraphics->maGraphicsData.maFallbackRuns.Clear();

	// Aggregate runs
	bool bRunRTL;
	int nMinCharPos;
	int nEndCharPos;
	bool bLastRunRTL;
	int nLastMinCharPos;
	int nLastEndCharPos;
	rArgs.ResetPos();
	if ( rArgs.GetNextRun( &nLastMinCharPos, &nLastEndCharPos, &bLastRunRTL ) )
	{
		while ( rArgs.GetNextRun( &nMinCharPos, &nEndCharPos, &bRunRTL ) )
		{
			if ( bRunRTL == bLastRunRTL && nMinCharPos <= nLastEndCharPos && nEndCharPos >= nLastMinCharPos )
			{
				if ( nMinCharPos < nLastMinCharPos )
					nLastMinCharPos = nMinCharPos;
				if ( nEndCharPos > nLastEndCharPos )
					nLastEndCharPos = nEndCharPos;
				continue;
			}

			maRuns.AddRun( nLastMinCharPos, nLastEndCharPos, bLastRunRTL );
			if ( !mnFallbackLevel )
				mpGraphics->maGraphicsData.maFallbackRuns.AddRun( nLastMinCharPos, nLastEndCharPos, bLastRunRTL );
			bLastRunRTL = bRunRTL;
			nLastMinCharPos = nMinCharPos;
			nLastEndCharPos = nEndCharPos;
		}

		maRuns.AddRun( nLastMinCharPos, nLastEndCharPos, bLastRunRTL );
		if ( !mnFallbackLevel )
			mpGraphics->maGraphicsData.maFallbackRuns.AddRun( nLastMinCharPos, nLastEndCharPos, bLastRunRTL );
	}

	Point aPos;
	bool bFallbackRunRTL;
	int nMinFallbackCharPos;
	int nEndFallbackCharPos;
	maRuns.ResetPos();
	mpGraphics->maGraphicsData.maFallbackRuns.ResetPos();
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

				mpKashidaLayoutData = ImplATSLayoutData::GetLayoutData( aKashidaArgs, mnFallbackLevel, mpVCLFont, 0, 0 );
				if ( mpKashidaLayoutData )
				{
					if ( mpKashidaLayoutData->mpNeedFallback )
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

		if ( mnFallbackLevel )
		{
			// Find run that this run is a fallback of
			bool bFallbackRunFound = true;
			if ( !mpGraphics->maGraphicsData.maFallbackRuns.PosIsInRun( nMinCharPos ) )
			{
				mpGraphics->maGraphicsData.maFallbackRuns.ResetPos();
				if ( !mpGraphics->maGraphicsData.maFallbackRuns.GetRun( &nMinFallbackCharPos, &nEndFallbackCharPos, &bFallbackRunRTL ) )
					bFallbackRunFound = false;
				while ( bFallbackRunFound && !mpGraphics->maGraphicsData.maFallbackRuns.PosIsInRun( nMinCharPos ) )
				{
					mpGraphics->maGraphicsData.maFallbackRuns.NextRun();
					if ( !mpGraphics->maGraphicsData.maFallbackRuns.GetRun( &nMinFallbackCharPos, &nEndFallbackCharPos, &bFallbackRunRTL ) )
						bFallbackRunFound = false;
				}
			}
			else if ( !mpGraphics->maGraphicsData.maFallbackRuns.GetRun( &nMinFallbackCharPos, &nEndFallbackCharPos, &bFallbackRunRTL ) )
				bFallbackRunFound = false;

			if ( !bFallbackRunFound )
				continue;
		}
		else
		{
			nMinFallbackCharPos = nMinCharPos;
			nEndFallbackCharPos = nEndCharPos;
		}

		int nBeginChars = nMinCharPos - nMinFallbackCharPos;
		int nEndChars = nEndFallbackCharPos - nEndCharPos;

		// Turn off direction analysis
		int nRunFlags = rArgs.mnFlags | SAL_LAYOUT_BIDI_STRONG;
		if ( bRunRTL )
			nRunFlags |= SAL_LAYOUT_BIDI_RTL;
		else
			nRunFlags &= ~SAL_LAYOUT_BIDI_RTL;

		// The OOo code will layout substrings and when this happens for Arabic
		// strings, the OOo code seems to expect that medial forms will be used
		// for the last character so we need to force the use of a trailing
		// character in such cases
		if ( bRunRTL && !nEndChars && nEndCharPos < rArgs.mnLength )
		{
			ImplLayoutArgs aDirAnalysisArgs( rArgs.mpStr, rArgs.mnLength, rArgs.mnMinCharPos, rArgs.mnLength, nRunFlags & ~SAL_LAYOUT_BIDI_STRONG );
			while ( aDirAnalysisArgs.GetNextRun( &nMinFallbackCharPos, &nEndFallbackCharPos, &bFallbackRunRTL ) )
			{
				if ( nMinFallbackCharPos <= nMinCharPos && nEndFallbackCharPos >= nEndCharPos )
				{
					nBeginChars = nMinCharPos - nMinFallbackCharPos;
					nEndChars = nEndFallbackCharPos - nEndCharPos;
					break;
				}
			}
		}

		ImplLayoutArgs aArgs( rArgs.mpStr, rArgs.mnLength, nMinCharPos, nEndCharPos, nRunFlags );

		ImplATSLayoutData *pLayoutData = ImplATSLayoutData::GetLayoutData( aArgs, mnFallbackLevel, mpVCLFont, nBeginChars, nEndChars );
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
				if ( pLayoutData->mpNeedFallback[ nCharPos - aArgs.mnMinCharPos + pLayoutData->mpHash->mnBeginChars ] )
					rArgs.NeedFallback( nCharPos, bPosRTL );
			}

			int nNextLevel = mnFallbackLevel + 1;
			::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator it = mpGraphics->maGraphicsData.maFallbackFonts.find( nNextLevel );
			if ( it != mpGraphics->maGraphicsData.maFallbackFonts.end() )
				delete it->second;
			mpGraphics->maGraphicsData.maFallbackFonts[ nNextLevel ] = new com_sun_star_vcl_VCLFont( pLayoutData->mpFallbackFont->getJavaObject() );
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
			int nIndex = pLayoutData->mpCharsToChars[ nCharPos - aArgs.mnMinCharPos + pLayoutData->mpHash->mnBeginChars ];
			if ( nIndex >= 0 )
			{
				sal_Unicode nChar = pLayoutData->mpHash->mpStr[ nIndex ];
				nCharWidth = pLayoutData->mpCharAdvances[ nIndex ];

				for ( int i = pLayoutData->mpCharsToGlyphs[ nIndex ]; i >= 0 && i < pLayoutData->mnGlyphCount && pLayoutData->mpGlyphInfoArray->glyphs[ i ].charIndex == nIndex; i++ )
				{
					long nGlyph = pLayoutData->mpGlyphInfoArray->glyphs[ i ].glyphID;

					if ( pLayoutData->maVerticalFontStyle )
						nGlyph |= GetVerticalFlags( nChar );

					// Mark whitespace glyphs
					if ( IsSpacingGlyph( nChar | GF_ISCHAR ) || pLayoutData->mpGlyphInfoArray->glyphs[ i ].glyphID == 0xffff || pLayoutData->mpGlyphInfoArray->glyphs[ i ].layoutFlags & kATSGlyphInfoTerminatorGlyph )
						nGlyph = 0x0020 | GF_ISCHAR;

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

	Point aPos;
	for ( int nStart = 0; ; )
	{
		int nGlyphCount = GetNextGlyphs( nMaxGlyphs, aGlyphArray, aPos, nStart, aDXArray, NULL );

		if ( !nGlyphCount )
			break;

		int i;
		for ( i = 0; i < nGlyphCount && IsSpacingGlyph( aGlyphArray[ i ] ); i++ )
			;
		if ( i )
		{
			nStart -= nGlyphCount - i;
			continue;
		}

		for ( i = 0; i < nGlyphCount && !IsSpacingGlyph( aGlyphArray[ i ] ); i++ )
			;
		nGlyphCount = i;

		if ( !nGlyphCount )
			break;

		long nTranslateX = 0;
		long nTranslateY = 0;

		long nGlyphOrientation = aGlyphArray[ 0 ] & GF_ROTMASK;
		if ( nGlyphOrientation )
		{
			nStart -= nGlyphCount - 1;
			nGlyphCount = 1;

			long nX;
			long nY;
			GetVerticalGlyphTranslation( aGlyphArray[ 0 ], nX, nY );
			if ( nGlyphOrientation == GF_ROTL )
			{
				nTranslateX = nX;
				nTranslateY = Float32ToLong( nY * maLayoutData.front()->mpHash->mfFontScaleX );
			}
			else
			{
				nTranslateX = nX;
				nTranslateY = Float32ToLong( ( aDXArray[ 0 ] - nY ) * maLayoutData.front()->mpHash->mfFontScaleX );
			}
		}

		for ( i = 0; i < nGlyphCount; i++ )
			aGlyphArray[ i ] &= GF_IDXMASK;

		rGraphics.maGraphicsData.mpVCLGraphics->drawGlyphs( aPos.X(), aPos.Y(), nGlyphCount, aGlyphArray, aDXArray, mpVCLFont, rGraphics.maGraphicsData.mnTextColor, GetOrientation(), nGlyphOrientation, nTranslateX, nTranslateY );
	}
}

// ----------------------------------------------------------------------------

bool SalATSLayout::GetOutline( SalGraphics& rGraphics, PolyPolyVector& rVector ) const
{
	bool bRet = false;

	if ( !maLayoutData.size() )
		return bRet;

	int nMaxGlyphs( 1 );
	long aGlyphArray[ nMaxGlyphs ];
	long aDXArray[ nMaxGlyphs ];
	int aCharPosArray[ nMaxGlyphs ];

	Point aPos;
	int nRunIndex = 0;
	ImplATSLayoutData *pLayoutData = maLayoutData[ nRunIndex ];
	maRuns.ResetPos();
	for ( int nStart = 0; ; )
	{
		int nGlyphCount = GetNextGlyphs( nMaxGlyphs, aGlyphArray, aPos, nStart, aDXArray, aCharPosArray );

		if ( !nGlyphCount )
			break;

		if ( IsSpacingGlyph( aGlyphArray[ 0 ] ) )
		{
			bRet = true;
			continue;
		}

		if ( !maRuns.PosIsInRun( aCharPosArray[ 0 ] ) )
		{
			nRunIndex = 0;
			pLayoutData = maLayoutData[ nRunIndex ];
			maRuns.ResetPos();
			while ( !maRuns.PosIsInRun( aCharPosArray[ 0 ] ) )
			{
				maRuns.NextRun();
				nRunIndex++;
				pLayoutData = maLayoutData[ nRunIndex ];
			}
		}

		int nIndex = pLayoutData->mpCharsToChars[ aCharPosArray[ 0 ] - mnMinCharPos + pLayoutData->mpHash->mnBeginChars ];
		if ( nIndex < 0 )
			continue;

		for ( int i = pLayoutData->mpCharsToGlyphs[ nIndex ]; i >= 0 && i < pLayoutData->mnGlyphCount && pLayoutData->mpGlyphInfoArray->glyphs[ i ].charIndex == nIndex; i++ )
		{
			long nGlyph = pLayoutData->mpGlyphInfoArray->glyphs[ i ].glyphID;
			if ( ( aGlyphArray[ 0 ] & GF_IDXMASK ) != nGlyph )
				continue;

			::std::list< Polygon > aPolygonList;
			OSStatus nErr;
			if ( ATSUGlyphGetCubicPaths( pLayoutData->mpGlyphInfoArray->glyphs[ i ].style, pLayoutData->mpGlyphInfoArray->glyphs[ i ].glyphID, SalATSCubicMoveToCallback, SalATSCubicLineToCallback, SalATSCubicCurveToCallback, SalATSCubicClosePathCallback, (void *)&aPolygonList, &nErr ) != noErr )
				continue;

			PolyPolygon aPolyPolygon;
			while ( aPolygonList.size() )
			{
				aPolyPolygon.Insert( aPolygonList.front() );
				aPolygonList.pop_front();
			}

			aPolyPolygon.Move( aPos.X(), aPos.Y() );

			if ( pLayoutData->maVerticalFontStyle )
			{
				long nGlyphOrientation = aGlyphArray[ 0 ] & GF_ROTMASK;
				if ( nGlyphOrientation )
				{
					if ( nGlyphOrientation == GF_ROTL )
					{
						aPolyPolygon.Rotate( Point( 0, 0 ), 900 );
						aPolyPolygon.Move( aPolyPolygon.GetBoundRect().nLeft * -1, pLayoutData->mnBaselineDelta * -1 );
					}
					else
					{
						aPolyPolygon.Rotate( Point( 0, 0 ), 2700 );
						aPolyPolygon.Move( aDXArray[ 0 ] - aPolyPolygon.GetBoundRect().nRight, pLayoutData->mnBaselineDelta * -1 );
					}
				}
			}

			rVector.push_back( aPolyPolygon );
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

	if ( mpKashidaLayoutData )
	{
		mpKashidaLayoutData->Release();
		mpKashidaLayoutData = NULL;
	}
}

// ----------------------------------------------------------------------------

long SalATSLayout::GetBaselineDelta() const
{
	return ( maLayoutData.size() ? maLayoutData.front()->mnBaselineDelta : 0 );
}

// ----------------------------------------------------------------------------

void SalATSLayout::GetVerticalGlyphTranslation( long nGlyph, long& nX, long& nY ) const
{
	nX = 0;
	nY = 0;

	if ( !maLayoutData.size() )
		return;

	long nGlyphOrientation = nGlyph & GF_ROTMASK;

	if ( maLayoutData.front()->maVerticalFontStyle && nGlyphOrientation & GF_ROTMASK )
	{
		GlyphID nGlyphID = (GlyphID)( nGlyph & GF_IDXMASK );

		ATSGlyphScreenMetrics aVerticalMetrics;
		ATSGlyphScreenMetrics aHorizontalMetrics;
		if ( ATSUGlyphGetScreenMetrics( maLayoutData.front()->maVerticalFontStyle, 1, &nGlyphID, sizeof( GlyphID ), maLayoutData.front()->mpHash->mbAntialiased, maLayoutData.front()->mpHash->mbAntialiased, &aVerticalMetrics ) == noErr && ATSUGlyphGetScreenMetrics( maLayoutData.front()->maFontStyle, 1, &nGlyphID, sizeof( GlyphID ), maLayoutData.front()->mpHash->mbAntialiased, maLayoutData.front()->mpHash->mbAntialiased, &aHorizontalMetrics ) == noErr )
		{
			nX = Float32ToLong( aVerticalMetrics.topLeft.x - aHorizontalMetrics.topLeft.x );
			if ( nGlyphOrientation == GF_ROTL )
				nX += GetBaselineDelta();
			else
				nX -= GetBaselineDelta();
			nY = Float32ToLong( aHorizontalMetrics.topLeft.y - aVerticalMetrics.topLeft.y );
		}
	}
}
