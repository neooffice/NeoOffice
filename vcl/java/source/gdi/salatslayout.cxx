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

#include <sys/sysctl.h>
#include <hash_map>
#include <unicode/ubidi.h>

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

struct ImplATSLayoutDataHashHash
{
	size_t				operator()( const ImplATSLayoutDataHash *x ) const { return (size_t)x->mnStrHash; }
};

struct ImplATSLayoutDataHashEquality
{
	bool				operator()( const ImplATSLayoutDataHash *p1, const ImplATSLayoutDataHash *p2 ) const;
};

struct ImplATSLayoutData {
	static ::std::hash_map< ImplATSLayoutDataHash*, ImplATSLayoutData*, ImplATSLayoutDataHashHash, ImplATSLayoutDataHashEquality >	maLayoutCache;
	static ::std::list< ImplATSLayoutData* >	maLayoutCacheList;
	static int			mnLayoutCacheSize;
	static ATSUFontFallbacks	maFontFallbacks;

	mutable int			mnRefCount;
	ImplATSLayoutDataHash*	mpHash;
	::vcl::com_sun_star_vcl_VCLFont*	mpVCLFont;
	ATSUStyle			maFontStyle;
	float				mfFontScaleY;
	bool*				mpNeedFallback;
	::vcl::com_sun_star_vcl_VCLFont*	mpFallbackFont;
	ATSUTextLayout		maLayout;
	ItemCount			mnGlyphCount;
	ATSLayoutRecord*	mpGlyphDataArray;
	int*				mpCharsToChars;
	int*				mpCharsToGlyphs;
	long*				mpCharAdvances;
	ATSUStyle			maVerticalFontStyle;
	long				mnBaselineDelta;
	bool				mbValid;

	static void					ClearLayoutDataCache();
	static ImplATSLayoutData*	GetLayoutData( const sal_Unicode *pStr, int nLen, int nMinCharPos, int nEndCharPos, int nFlags, int nFallbackLevel, ::vcl::com_sun_star_vcl_VCLFont *pVCLFont, const SalATSLayout *pCurrentLayout );

						ImplATSLayoutData( ImplATSLayoutDataHash *pLayoutHash, int nFallbackLevel, ::vcl::com_sun_star_vcl_VCLFont *pVCLFont, const SalATSLayout *pCurrentLayout );
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

bool ImplATSLayoutDataHashEquality::operator()( const ImplATSLayoutDataHash *p1, const ImplATSLayoutDataHash *p2 ) const
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

::std::hash_map< ImplATSLayoutDataHash*, ImplATSLayoutData*, ImplATSLayoutDataHashHash, ImplATSLayoutDataHashEquality > ImplATSLayoutData::maLayoutCache;

// ----------------------------------------------------------------------------

::std::list< ImplATSLayoutData* > ImplATSLayoutData::maLayoutCacheList;

// ----------------------------------------------------------------------------

int ImplATSLayoutData::mnLayoutCacheSize = 0;

// ----------------------------------------------------------------------------

ATSUFontFallbacks ImplATSLayoutData::maFontFallbacks = NULL;

// ----------------------------------------------------------------------------

void ImplATSLayoutData::ClearLayoutDataCache()
{
	mnLayoutCacheSize = 0;
	maLayoutCache.clear();

	while ( maLayoutCacheList.size() )
	{
		maLayoutCacheList.back()->Release();
		maLayoutCacheList.pop_back();
	}

	if ( maFontFallbacks )
	{
		ATSUDisposeFontFallbacks( maFontFallbacks );
		maFontFallbacks = NULL;
	}
}

// ----------------------------------------------------------------------------

ImplATSLayoutData *ImplATSLayoutData::GetLayoutData( const sal_Unicode *pStr, int nLen, int nMinCharPos, int nEndCharPos, int nFlags, int nFallbackLevel, com_sun_star_vcl_VCLFont *pVCLFont, const SalATSLayout *pCurrentLayout )
{
	ImplATSLayoutData *pLayoutData = NULL;

	ImplATSLayoutDataHash *pLayoutHash = new ImplATSLayoutDataHash();
	pLayoutHash->mnLen = nEndCharPos - nMinCharPos;
	pLayoutHash->mnFontID = (ATSUFontID)pVCLFont->getNativeFont();
	pLayoutHash->mnFontSize = pVCLFont->getSize();
	pLayoutHash->mfFontScaleX = pVCLFont->getScaleX();
	pLayoutHash->mbAntialiased = pVCLFont->isAntialiased();
	pLayoutHash->mbRTL = ( nFlags & SAL_LAYOUT_BIDI_RTL );
	pLayoutHash->mbVertical = ( nFlags & SAL_LAYOUT_VERTICAL );

	pLayoutHash->mpStr = (sal_Unicode *)( pStr + nMinCharPos );
	pLayoutHash->mnStrHash = rtl_ustr_hashCode_WithLength( pLayoutHash->mpStr, pLayoutHash->mnLen );

	// Search cache for matching layout
	::std::hash_map< ImplATSLayoutDataHash*, ImplATSLayoutData*, ImplATSLayoutDataHashHash, ImplATSLayoutDataHashEquality >::const_iterator it = maLayoutCache.find( pLayoutHash );
	if ( it != maLayoutCache.end() )
	{
		pLayoutData = it->second;
		delete pLayoutHash;
		pLayoutHash = NULL;
	}

	if ( !pLayoutData )
	{
		// Copy the string so that we can cache it
		pLayoutHash->mpStr = (sal_Unicode *)rtl_allocateMemory( pLayoutHash->mnLen * sizeof( sal_Unicode ) );
		memcpy( pLayoutHash->mpStr, pStr + nMinCharPos, pLayoutHash->mnLen * sizeof( sal_Unicode ) );
		pLayoutData = new ImplATSLayoutData( pLayoutHash, nFallbackLevel, pVCLFont, pCurrentLayout );

		if ( !pLayoutData )
			return NULL;

		if ( !pLayoutData->IsValid() )
		{
			pLayoutData->Release();
			return NULL;
		}

		// Limit cache size
		static int nLayoutCacheSize = 0;
		static int nTargetCacheSize = 0;
		if ( mnLayoutCacheSize > nLayoutCacheSize )
		{
			if ( !nLayoutCacheSize )
			{
				// Set the layout cache size based on physical memory
				int pMib[2];
				size_t nMinMem = 256 * 1024 * 1024;
				size_t nMaxMem = nMinMem * 2;
				size_t nUserMem = 0;
				size_t nUserMemLen = sizeof( nUserMem );
				pMib[0] = CTL_HW;
				pMib[1] = HW_USERMEM;
				if ( !sysctl( pMib, 2, &nUserMem, &nUserMemLen, NULL, 0 ) )
					nUserMem /= 2;
				if ( nUserMem > nMaxMem )
					nUserMem = nMaxMem;
				else if ( nUserMem < nMinMem )
					nUserMem = nMinMem;
				nLayoutCacheSize = nUserMem / ( 1024 * 8 );
				nTargetCacheSize = (int)( nLayoutCacheSize * 0.8 );
			}

			while ( mnLayoutCacheSize > nTargetCacheSize )
			{
				mnLayoutCacheSize -= maLayoutCacheList.back()->mpHash->mnLen;
				maLayoutCache.erase( maLayoutCacheList.back()->mpHash );
				maLayoutCacheList.back()->Release();
				maLayoutCacheList.pop_back();
			}
		}

		mnLayoutCacheSize += pLayoutData->mpHash->mnLen;
		maLayoutCache.insert( ::std::hash_map< ImplATSLayoutDataHash*, ImplATSLayoutData*, ImplATSLayoutDataHashHash, ImplATSLayoutDataHashEquality >::value_type( pLayoutData->mpHash, pLayoutData ) );
		maLayoutCacheList.push_front( pLayoutData );
	}

	if ( pLayoutData )
		pLayoutData->Reference();

	return pLayoutData;
}

// ----------------------------------------------------------------------------

ImplATSLayoutData::ImplATSLayoutData( ImplATSLayoutDataHash *pLayoutHash, int nFallbackLevel, com_sun_star_vcl_VCLFont *pVCLFont, const SalATSLayout *pCurrentLayout ) :
	mnRefCount( 1 ),
	mpHash( pLayoutHash ),
	mpVCLFont( NULL ),
	maFontStyle( NULL ),
	mfFontScaleY( 1.0f ),
	mpNeedFallback( NULL ),
	mpFallbackFont( NULL ),
	maLayout( NULL ),
	mnGlyphCount( 0 ),
	mpGlyphDataArray( NULL ),
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

	mpVCLFont = new com_sun_star_vcl_VCLFont( pVCLFont );
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

	ATSUAttributeTag nTags[4];
	ByteCount nBytes[4];
	ATSUAttributeValuePtr nVals[4];

	// Set font
	nTags[0] = kATSUFontTag;
	nBytes[0] = sizeof( ATSUFontID );
	nVals[0] = &mpHash->mnFontID;

	// The OOo code will often layout fonts at unrealistically large sizes so
	// we need to use a more reasonably sized font or else we will exceed the
	// 32K Fixed data type limit that the ATSTrapezoid struct uses so we
	// preemptively limit font size to a size that is most likely to fit the
	// within the 32K limit
	long nSize = mpHash->mnFontSize;
	long nAdjustedSize;
	if ( mpHash->mnFontSize * mpHash->mnLen * 2 > 0x00007fff )
		nAdjustedSize = 0x00007fff / ( mpHash->mnLen * 2 );
	else
		nAdjustedSize = nSize;
	Fixed fCurrentSize = Long2Fix( nAdjustedSize );
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

	ATSUVerticalCharacterType nHorizontal = kATSUStronglyHorizontal;
	nTags[3] = kATSUVerticalCharacterTag;
	nBytes[3] = sizeof( ATSUVerticalCharacterType );
	nVals[3] = &nHorizontal;

	if ( ATSUSetAttributes( maFontStyle, 4, nTags, nBytes, nVals ) != noErr )
	{
		Destroy();
		return;
	}

	if ( mpHash->mbVertical )
	{
		if ( ATSUCreateAndCopyStyle( maFontStyle, &maVerticalFontStyle ) == noErr && maVerticalFontStyle )
		{
			ATSUVerticalCharacterType nVertical = kATSUStronglyVertical;
			nTags[0] = kATSUVerticalCharacterTag;
			nBytes[0] = sizeof( ATSUVerticalCharacterType );
			nVals[0] = &nVertical;

			if ( ATSUSetAttributes( maVerticalFontStyle, 1, nTags, nBytes, nVals ) != noErr )
			{
				Destroy();
				return ;
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

	// Initialize font fallbacks list if necessary
	if ( !ImplATSLayoutData::maFontFallbacks )
	{
		if ( ATSUCreateFontFallbacks( &ImplATSLayoutData::maFontFallbacks ) == noErr )
		{
			SalData *pSalData = GetSalData();
			ItemCount nCount = pSalData->maNativeFontMapping.size();
			ItemCount nActualCount = 0;

			ATSUFontID aATSUFonts[ nCount ];
			for ( ::std::map< sal_IntPtr, JavaImplFontData* >::const_iterator it = pSalData->maNativeFontMapping.begin(); it != pSalData->maNativeFontMapping.end(); ++it )
				aATSUFonts[ nActualCount++ ] = it->first;

			if ( !nActualCount || ATSUSetObjFontFallbacks( ImplATSLayoutData::maFontFallbacks, nActualCount, aATSUFonts, kATSUSequentialFallbacksExclusive ) != noErr )
			{
				if ( ImplATSLayoutData::maFontFallbacks )
				{
					ATSUDisposeFontFallbacks( ImplATSLayoutData::maFontFallbacks );
					ImplATSLayoutData::maFontFallbacks = NULL;
				}
			}
		}
	}

	// Set the font fallbacks list
	if ( ImplATSLayoutData::maFontFallbacks )
	{
		nTags[0] = kATSULineFontFallbacksTag;
		nBytes[0] = sizeof( ATSUFontFallbacks );
		nVals[0] = &ImplATSLayoutData::maFontFallbacks;

		if ( ATSUSetLayoutControls( maLayout, 1, nTags, nBytes, nVals ) != noErr )
		{
			Destroy();
			return;
		}
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
		nAdjustedSize /= 2;
		fCurrentSize = Long2Fix( nAdjustedSize );
		nTags[0] = kATSUSizeTag;
		nBytes[0] = sizeof( Fixed );
		nVals[0] = &fCurrentSize;

		if ( nAdjustedSize < 1 || ATSUSetAttributes( maFontStyle, 1, nTags, nBytes, nVals ) != noErr || ( maVerticalFontStyle && ATSUSetAttributes( maVerticalFontStyle, 1, nTags, nBytes, nVals ) != noErr ) || ATSUGetGlyphBounds( maLayout, 0, 0, kATSUFromTextBeginning, kATSUToTextEnd, kATSUseFractionalOrigins, 1, &aTrapezoid, NULL ) != noErr )
		{
			Destroy();
			return;
		}
	}

	// Fix bug 2919 by still producing a valid text layout even if no glyphs
	// can be retrieved
	ATSUDirectGetLayoutDataArrayPtrFromTextLayout( maLayout, 0, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent, (void **)&mpGlyphDataArray, &mnGlyphCount );

	// Cache mapping of characters to glyph character indices
	ByteCount nBufSize = mpHash->mnLen * sizeof( int );
	mpCharsToChars = (int *)rtl_allocateMemory( nBufSize );

	int i;
	for ( i = 0; i < mpHash->mnLen; i++ )
		mpCharsToChars[ i ] = -1;
	if ( mpHash->mbRTL )
	{
		i = 0;
		for ( int j = mpHash->mnLen - 1; j >= 0 && i < mnGlyphCount && mpGlyphDataArray; j-- )
		{
			int nIndex = mpGlyphDataArray[ i ].originalOffset / 2;
			mpCharsToChars[ j ] = nIndex;
			for ( ; i < mnGlyphCount && ( mpGlyphDataArray[ i ].originalOffset / 2 ) == nIndex; i++ )
				;
		}
	}
	else
	{
		i = 0;
		for ( int j = 0; j < mpHash->mnLen && i < mnGlyphCount && mpGlyphDataArray; j++ )
		{
			int nIndex = mpGlyphDataArray[ i ].originalOffset / 2;
			mpCharsToChars[ j ] = nIndex;
			for ( ; i < mnGlyphCount && ( mpGlyphDataArray[ i ].originalOffset / 2 ) == nIndex; i++ )
				;
		}
	}

	// Cache mapping of characters to glyphs
	nBufSize = mpHash->mnLen * sizeof( int );
	mpCharsToGlyphs = (int *)rtl_allocateMemory( nBufSize );

	for ( i = 0; i < mpHash->mnLen; i++ )
		mpCharsToGlyphs[ i ] = -1;
	for ( i = 0; i < mnGlyphCount && mpGlyphDataArray; i++ )
	{
		int nIndex = mpGlyphDataArray[ i ].originalOffset / 2;
		if ( mpCharsToGlyphs[ nIndex ] < 0 || i < mpCharsToGlyphs[ nIndex ] )
			mpCharsToGlyphs[ nIndex ] = i;
	}

	// Cache glyph widths
	nBufSize = mpHash->mnLen * sizeof( long );
	mpCharAdvances = (long *)rtl_allocateMemory( nBufSize );
	memset( mpCharAdvances, 0, nBufSize );

	// Fix bug 448 by eliminating subpixel advances.
	mfFontScaleY = (float)nSize / nAdjustedSize;
	int nLastNonSpacingIndex = -1;
	for ( i = 0; i < mnGlyphCount && mpGlyphDataArray; i++ )
	{
		int nIndex = mpGlyphDataArray[ i ].originalOffset / 2;
		if ( i == mnGlyphCount - 1 )
		{
			if ( ATSUGetGlyphBounds( maLayout, 0, 0, i, 1, kATSUseFractionalOrigins, 1, &aTrapezoid, NULL ) == noErr )
				mpCharAdvances[ nIndex ] += Float32ToLong( Fix2X( aTrapezoid.upperRight.x - aTrapezoid.upperLeft.x ) * mpHash->mfFontScaleX * mfFontScaleY );
		}
		else
		{
			mpCharAdvances[ nIndex ] += Float32ToLong( Fix2X( mpGlyphDataArray[ i + 1 ].realPos - mpGlyphDataArray[ i ].realPos ) * mpHash->mfFontScaleX * mfFontScaleY );
		}

		// Make sure that ligature glyphs get all of the width and that their
		// attached spacing glyphs have zero width so that the OOo code will
		// force the cursor to the end of the ligature instead of the beginning
		long nWidthAdjust = 0;
		if ( mpGlyphDataArray[ i ].glyphID == 0xffff && !pCurrentLayout->IsSpacingGlyph( mpHash->mpStr[ nIndex ] | GF_ISCHAR ) )
		{
			if ( nLastNonSpacingIndex >= 0 && nLastNonSpacingIndex != nIndex )
			{
				mpCharAdvances[ nLastNonSpacingIndex ] += mpCharAdvances[ nIndex ] - 1;
				if ( mpCharAdvances[ nLastNonSpacingIndex ] < 0 )
				{
					nWidthAdjust = mpCharAdvances[ nLastNonSpacingIndex ];
					mpCharAdvances[ nLastNonSpacingIndex ] = 0;
				}

				mpCharAdvances[ nIndex ] = 0;
			}
		}
		else if ( pCurrentLayout->IsSpacingGlyph( mpHash->mpStr[ nIndex ] | GF_ISCHAR ) )
		{
			nLastNonSpacingIndex = -1;
		}
		else
		{
			nLastNonSpacingIndex = nIndex;
		}

		if ( mpCharAdvances[ nIndex ] < 0 )
		{
			nWidthAdjust += mpCharAdvances[ nIndex ];
			mpCharAdvances[ nIndex ] = 0;
		}
		else if ( nWidthAdjust && mpCharAdvances[ nIndex ] > 0 )
		{
			mpCharAdvances[ nIndex ] += nWidthAdjust;
			if ( mpCharAdvances[ nIndex ] < 0 )
			{
				nWidthAdjust = mpCharAdvances[ nIndex ];
				mpCharAdvances[ nIndex ] = 0;
			}
			else
			{
				nWidthAdjust = 0;
			}
		}
	}

	if ( maVerticalFontStyle )
	{
		BslnBaselineRecord aBaseline;
		memset( aBaseline, 0, sizeof( BslnBaselineRecord ) );
		if ( ATSUCalculateBaselineDeltas( maVerticalFontStyle, kBSLNRomanBaseline, aBaseline ) == noErr )
			mnBaselineDelta = Float32ToLong( Fix2X( aBaseline[ kBSLNIdeographicCenterBaseline ] ) * mfFontScaleY );
		if ( !mnBaselineDelta )
		{
			ATSFontMetrics aFontMetrics;
			ATSFontRef aFont = FMGetATSFontRefFromFont( mpHash->mnFontID );
			if ( ATSFontGetHorizontalMetrics( aFont, kATSOptionFlagsDefault, &aFontMetrics ) == noErr )
				mnBaselineDelta = Float32ToLong( ( ( ( fabs( aFontMetrics.descent ) + fabs( aFontMetrics.ascent ) ) / 2 ) - fabs( aFontMetrics.descent ) ) * nSize );
		}
	}

	// Find positions that require fallback fonts
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
				::std::map< sal_IntPtr, JavaImplFontData* >::const_iterator it = pSalData->maNativeFontMapping.find( (sal_IntPtr)nFontID );
				if ( it != pSalData->maNativeFontMapping.end() )
				{
					mpFallbackFont = new com_sun_star_vcl_VCLFont( it->second->maVCLFontName, mpHash->mnFontSize, mpVCLFont->getOrientation(), mpHash->mbAntialiased, mpHash->mbVertical, mpHash->mfFontScaleX, 0 );
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

	if ( mpCharsToChars )
	{
		rtl_freeMemory( mpCharsToChars );
		mpCharsToChars = NULL;
	}

	if ( mpGlyphDataArray )
	{
		ATSUDirectReleaseLayoutDataArrayPtr( NULL, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent, (void **)&mpGlyphDataArray );
		mpGlyphDataArray = NULL;
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
	mpKashidaLayoutData( NULL ),
	mnOrigWidth( 0 ),
	mfGlyphScaleX( 1.0 )
{
	if ( mnFallbackLevel )
	{
		::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator it = mpGraphics->maFallbackFonts.find( mnFallbackLevel );
		if ( it != mpGraphics->maFallbackFonts.end() )
		{
			mpVCLFont = new com_sun_star_vcl_VCLFont( it->second );

			// Prevent infinite fallback
			sal_IntPtr nNativeFont = mpVCLFont->getNativeFont();	
			if ( mpVCLFont )
			{
				for ( ::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator ffit = mpGraphics->maFallbackFonts.begin(); ffit != mpGraphics->maFallbackFonts.end(); ++ffit )
				{
					// It is OK to reuse the original font if a force fallback
					// font is in use
					if ( ffit->first < mnFallbackLevel && ffit->second->getNativeFont() == nNativeFont )
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
		mpVCLFont = new com_sun_star_vcl_VCLFont( mpGraphics->mpVCLFont );
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

	// Fix bug 2133 by scaling width of characters if the new width is narrower
	// than the original width. Fix bug 2652 by only applying this fix when
	// there is only a single character in the layout.
	mfGlyphScaleX = 1.0;
	if ( rArgs.mnEndCharPos - rArgs.mnMinCharPos == 1 )
	{
		long nWidth;
		if ( rArgs.mpDXArray )
			nWidth = rArgs.mpDXArray[ rArgs.mnEndCharPos - rArgs.mnMinCharPos - 1 ];
		else if ( rArgs.mnLayoutWidth )
			nWidth = rArgs.mnLayoutWidth;
		else
			nWidth = mnOrigWidth;

		// Fix bug 2882 by ensuring that the glyph scale is never zero
		if ( nWidth > 0 && nWidth < mnOrigWidth )
			mfGlyphScaleX = (float)nWidth / mnOrigWidth;
	}

	if ( rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN && ! ( rArgs.mnFlags & SAL_LAYOUT_VERTICAL ) )
		ApplyAsianKerning( rArgs.mpStr, rArgs.mnLength );

	if ( rArgs.mnFlags & SAL_LAYOUT_KASHIDA_JUSTIFICATON && rArgs.mpDXArray && mpKashidaLayoutData && mpKashidaLayoutData->mpGlyphDataArray )
		KashidaJustify( mpKashidaLayoutData->mpGlyphDataArray[ 0 ].glyphID, mpKashidaLayoutData->mpCharAdvances[ mpKashidaLayoutData->mpHash->mnLen - 1 ] );
}

// ----------------------------------------------------------------------------

bool SalATSLayout::LayoutText( ImplLayoutArgs& rArgs )
{
	Destroy();

	bool bRet = false;
	rArgs.mnFlags |= SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;

	if ( !mpVCLFont )
		return bRet;

	bool bNeedSymbolFallback = false;
	bool bUseNativeFallback = false;
	int nEstimatedGlyphs = 0;

	// Aggregate runs
	bool bRunRTL;
	int nMinCharPos;
	int nEndCharPos;
	if ( !mnFallbackLevel )
	{
		// Fix bug 2841 by ensuring that we process only full runs
		bool bDeleteArgs = false;
		nMinCharPos = rArgs.mnMinCharPos;
		nEndCharPos = rArgs.mnEndCharPos;
		while ( nMinCharPos && !IsSpacingGlyph( rArgs.mpStr[ nMinCharPos - 1 ] | GF_ISCHAR ) )
		{
			nMinCharPos--;
			bDeleteArgs = true;
		}
		while ( nEndCharPos < rArgs.mnLength && !IsSpacingGlyph( rArgs.mpStr[ nEndCharPos ] | GF_ISCHAR ) )
		{
			nEndCharPos++;
			bDeleteArgs = true;
		}

		ImplLayoutArgs *pArgs = &rArgs;

		// If SAL_LAYOUT_BIDI_STRONG is set, we need to verify that the extra
		// characters are of the same direction otherwise typing LTR characters
		// in RTL text will be misrendered
		if ( bDeleteArgs && rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG )
		{
			// The ImplLayoutArgs class does not do BIDI analysis in this case
			// so we can be assured that the entire string is a single run
			bool bIsStrongRTL = ( rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL );

			bool bStrongRunRTL;
			int nStrongMinCharPos;
			int nStrongEndCharPos;

			if ( nMinCharPos < rArgs.mnMinCharPos )
			{
				ImplLayoutArgs aMinArgs( rArgs.mpStr, rArgs.mnLength, nMinCharPos, rArgs.mnMinCharPos, rArgs.mnFlags & ~( SAL_LAYOUT_BIDI_STRONG | SAL_LAYOUT_DISABLE_GLYPH_PROCESSING ) );
				aMinArgs.ResetPos();
				while ( aMinArgs.GetNextRun( &nStrongMinCharPos, &nStrongEndCharPos, &bStrongRunRTL ) )
				{
					if ( nStrongEndCharPos == rArgs.mnMinCharPos )
					{
						if ( bStrongRunRTL == bIsStrongRTL && nStrongMinCharPos < rArgs.mnMinCharPos )
							nMinCharPos = nStrongMinCharPos;
						else
							bDeleteArgs = false;
					}
				}
			}

			if ( nEndCharPos > rArgs.mnEndCharPos )
			{
				ImplLayoutArgs aEndArgs( rArgs.mpStr, rArgs.mnLength, rArgs.mnEndCharPos, nEndCharPos, rArgs.mnFlags & ~( SAL_LAYOUT_BIDI_STRONG | SAL_LAYOUT_DISABLE_GLYPH_PROCESSING ) );
				aEndArgs.ResetPos();
				while ( aEndArgs.GetNextRun( &nStrongMinCharPos, &nStrongEndCharPos, &bStrongRunRTL ) )
				{
					if ( nStrongMinCharPos == rArgs.mnEndCharPos )
					{
						if ( bStrongRunRTL == bIsStrongRTL && nEndCharPos > rArgs.mnEndCharPos )
							nEndCharPos = nStrongEndCharPos;
						else
							bDeleteArgs = false;
					}
				}
			}
		}

		if ( bDeleteArgs )
		{
			pArgs = new ImplLayoutArgs( rArgs.mpStr, rArgs.mnLength, nMinCharPos, nEndCharPos, rArgs.mnFlags & ~SAL_LAYOUT_DISABLE_GLYPH_PROCESSING );
			if ( !pArgs )
			{
				pArgs = &rArgs;
				bDeleteArgs = false;
			}
		}

		mpGraphics->maFallbackRuns.Clear();
		pArgs->ResetPos();
		while ( pArgs->GetNextRun( &nMinCharPos, &nEndCharPos, &bRunRTL ) )
		{
			if ( nEndCharPos <= rArgs.mnMinCharPos || nMinCharPos >= rArgs.mnEndCharPos )
				continue;

			// Significantly improve cache hit rate by splitting runs into
			// their component words
			if ( bRunRTL )
			{
				int nStart = nEndCharPos;
				while ( nStart > nMinCharPos )
				{
					int i = nStart;
					for ( ; i > nMinCharPos && !IsSpacingGlyph( pArgs->mpStr[ i - 1 ] | GF_ISCHAR ); i-- )
						;
					for ( ; i > nMinCharPos && IsSpacingGlyph( pArgs->mpStr[ i - 1 ] | GF_ISCHAR ); i-- )
						;
					mpGraphics->maFallbackRuns.AddRun( i, nStart, bRunRTL );
					nEstimatedGlyphs += nStart - i;
					nStart = i;
				}
			}
			else
			{
				int nStart = nMinCharPos;
				while ( nStart < nEndCharPos )
				{
					int i = nStart;
					for ( ; i < nEndCharPos && !IsSpacingGlyph( pArgs->mpStr[ i ] | GF_ISCHAR ); i++ )
						;
					for ( ; i < nEndCharPos && IsSpacingGlyph( pArgs->mpStr[ i ] | GF_ISCHAR ); i++ )
						;
					mpGraphics->maFallbackRuns.AddRun( nStart, i, bRunRTL );
					nEstimatedGlyphs += i - nStart;
					nStart = i;
				}
			}

		}

		if ( bDeleteArgs )
			delete pArgs;
	}

	bool bFallbackRunRTL;
	int nMinFallbackCharPos;
	int nEndFallbackCharPos;
	rArgs.ResetPos();
	while ( rArgs.GetNextRun( &nMinCharPos, &nEndCharPos, &bRunRTL ) )
	{
		mpGraphics->maFallbackRuns.ResetPos();
		while ( mpGraphics->maFallbackRuns.GetRun( &nMinFallbackCharPos, &nEndFallbackCharPos, &bFallbackRunRTL ) )
		{
			mpGraphics->maFallbackRuns.NextRun();

			int nMaxMinCharPos = nMinCharPos;
			int nMinEndCharPos = nEndCharPos;
			bool bExactMatch = true;
			if ( nMaxMinCharPos < nMinFallbackCharPos )
			{
				nMaxMinCharPos = nMinFallbackCharPos;
				bExactMatch = false;
			}
			if ( nMinEndCharPos > nEndFallbackCharPos )
			{
				nMinEndCharPos = nEndFallbackCharPos;
				bExactMatch = false;
			}

			if ( nMaxMinCharPos < nMinEndCharPos )
			{
				maRuns.AddRun( nMaxMinCharPos, nMinEndCharPos, bRunRTL );
				nEstimatedGlyphs += nMinEndCharPos - nMaxMinCharPos;

				if ( bExactMatch )
					break;
			}
		}
	}

	SetGlyphCapacity( (int)( nEstimatedGlyphs * 1.1 ) );

	com_sun_star_vcl_VCLFont *pFallbackFont = NULL;
	Point aPos;
	maRuns.ResetPos();
	mpGraphics->maFallbackRuns.ResetPos();
	while ( maRuns.GetRun( &nMinCharPos, &nEndCharPos, &bRunRTL ) )
	{
		maRuns.NextRun();

		// Check if this run will need Kashida justification
		if ( bRunRTL && ( rArgs.mpDXArray || rArgs.mnLayoutWidth ) )
		{
			bool bNeedArabicFontSupport = false;
			for ( int i = nMinCharPos; i < nEndCharPos; i++ )
			{
				if ( ( rArgs.mpStr[ i ] >= 0x0600 && rArgs.mpStr[ i ] < 0x0900 ) || ( rArgs.mpStr[ i ] >= 0xfb50 && rArgs.mpStr[ i ] < 0xfe00 ) || ( rArgs.mpStr[ i ] >= 0xfe70 && rArgs.mpStr[ i ] < 0xff00 ) )
				{
					bNeedArabicFontSupport = true;
					break;
				}
			}

			if ( bNeedArabicFontSupport )
			{
				if ( !mpKashidaLayoutData )
				{
					sal_Unicode aArabicTest[ 3 ];
					aArabicTest[ 0 ] = 0x0634;
					aArabicTest[ 1 ] = 0x0634;
					aArabicTest[ 2 ] = 0x0640;
					mpKashidaLayoutData = ImplATSLayoutData::GetLayoutData( aArabicTest, 3, 0, 3, rArgs.mnFlags | SAL_LAYOUT_BIDI_STRONG | SAL_LAYOUT_BIDI_RTL, mnFallbackLevel, mpVCLFont, this );
				}

				if ( mpKashidaLayoutData )
				{
					bool bHasArabicFontSupport = true;
					if ( !mpKashidaLayoutData->mpGlyphDataArray || ( mpKashidaLayoutData->mpNeedFallback && mpKashidaLayoutData->mpFallbackFont ) )
					{
						bHasArabicFontSupport = false;
					}
					else
					{
						// Fix bug 2757 by detecting when a font cannot support
						// Arabic text layout. The characters in our layout
						// should always product different glyphs for each
						// character so if any are the same, the font does not
						// support Arabic properly.
						for ( int i = 0; i < mpKashidaLayoutData->mnGlyphCount; i++ )
						{
							if ( !mpKashidaLayoutData->mpGlyphDataArray[ i ].glyphID )
							{
								bHasArabicFontSupport = false;
								break;
							}
							else if ( i && mpKashidaLayoutData->mpGlyphDataArray[ i ].glyphID == mpKashidaLayoutData->mpGlyphDataArray[ i - 1 ].glyphID )
							{
								bHasArabicFontSupport = false;
								break;
							}
							else if ( mpKashidaLayoutData->mpGlyphDataArray[ i ].glyphID == 0xffff )
							{
								break;
							}
						}
					}

					if ( !bHasArabicFontSupport )
					{
						for ( int i = nMinCharPos; i < nEndCharPos; i++ )
							rArgs.NeedFallback( i, bRunRTL );

						if ( !pFallbackFont )
						{
							// If there is no fallback font but the font really
							// does not support Arabic (e.g. non-AAT fonts like
							// STIXihei), assign Geeza Pro as this layout's
							// fallback font
							if ( !mpKashidaLayoutData->mpFallbackFont )
							{
								SalData *pSalData = GetSalData();

								sal_IntPtr nNativeFont = mpVCLFont->getNativeFont();
								::std::map< String, JavaImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( String( RTL_CONSTASCII_USTRINGPARAM( "Geeza Pro" ) ) );
								if ( it != pSalData->maFontNameMapping.end() && (int)it->second->GetFontId() != nNativeFont )
									mpKashidaLayoutData->mpFallbackFont = new com_sun_star_vcl_VCLFont( it->second->maVCLFontName, mpVCLFont->getSize(), mpVCLFont->getOrientation(), mpVCLFont->isAntialiased(), mpVCLFont->isVertical(), mpVCLFont->getScaleX(), 0 );
							}

							if ( mpKashidaLayoutData->mpFallbackFont )
								pFallbackFont = mpKashidaLayoutData->mpFallbackFont;
						}

						rArgs.mnFlags &= ~SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
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

		if ( !mpGraphics->maFallbackRuns.GetRun( &nMinFallbackCharPos, &nEndFallbackCharPos, &bFallbackRunRTL ) )
			continue;

		// Turn off direction analysis
		int nRunFlags = rArgs.mnFlags | SAL_LAYOUT_BIDI_STRONG;
		if ( bRunRTL )
			nRunFlags |= SAL_LAYOUT_BIDI_RTL;
		else
			nRunFlags &= ~SAL_LAYOUT_BIDI_RTL;

		ImplATSLayoutData *pLayoutData = ImplATSLayoutData::GetLayoutData( rArgs.mpStr, rArgs.mnLength, nMinFallbackCharPos, nEndFallbackCharPos, nRunFlags, mnFallbackLevel, mpVCLFont, this );
		if ( !pLayoutData )
			continue;

		// Check for typographical ligatures at the requested run boundaries
		// in fallback runs as we should not allow ligatures because it is 
		// highly unlikely that the first layout had ligatures at these
		// character positions
		bool bRelayout = false;
		if ( mnFallbackLevel )
		{
			if ( nMinCharPos > nMinFallbackCharPos )
			{
				int nIndex = pLayoutData->mpCharsToChars[ nMinCharPos - nMinFallbackCharPos - ( bRunRTL ? 1 : 0 ) ];
				if ( nIndex >= 0 && !IsSpacingGlyph( pLayoutData->mpHash->mpStr[ nIndex ] | GF_ISCHAR ) )
				{
					int i = pLayoutData->mpCharsToGlyphs[ nIndex ];
					if ( i >= 0 && pLayoutData->mpGlyphDataArray && pLayoutData->mpGlyphDataArray[ i ].glyphID == 0xffff )
					{
						nMinFallbackCharPos = nMinCharPos;
						bRelayout = true;
					}
				}
			}

			if ( nEndCharPos < nEndFallbackCharPos )
			{
				int nIndex = pLayoutData->mpCharsToChars[ nEndCharPos - nMinFallbackCharPos - ( bRunRTL ? 1 : 0 ) ];
				if ( nIndex >= 0 && !IsSpacingGlyph( pLayoutData->mpHash->mpStr[ nIndex ] | GF_ISCHAR ) )
				{
					int i = pLayoutData->mpCharsToGlyphs[ nIndex ];
					if ( i >= 0 && pLayoutData->mpGlyphDataArray && pLayoutData->mpGlyphDataArray[ i ].glyphID == 0xffff )
					{
						nEndFallbackCharPos = nEndCharPos;
						bRelayout = true;
					}
				}
			}
		}

		if ( bRelayout )
		{
			pLayoutData->Release();
			pLayoutData = ImplATSLayoutData::GetLayoutData( rArgs.mpStr, rArgs.mnLength, nMinFallbackCharPos, nEndFallbackCharPos, nRunFlags, mnFallbackLevel, mpVCLFont, this );
			if ( !pLayoutData )
				continue;
		}

		// Create fallback runs
		if ( pLayoutData->mpNeedFallback && pLayoutData->mpFallbackFont )
		{
			for ( int i = nMinCharPos; i < nEndCharPos; i++ )
			{
				if ( pLayoutData->mpNeedFallback[ i - nMinFallbackCharPos ] )
					rArgs.NeedFallback( i, bRunRTL );
			}

			if ( !pFallbackFont )
				pFallbackFont = pLayoutData->mpFallbackFont;
			rArgs.mnFlags &= ~SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
		}

		// Calculate and cache glyph advances. Fix bug 2893 by not allowing
		// zero character widths
		int nCharPos = ( bRunRTL ? nEndCharPos - 1 : nMinCharPos );
		for ( ; bRunRTL ? nCharPos >= nMinCharPos : nCharPos < nEndCharPos; bRunRTL ? nCharPos-- : nCharPos++ )
		{
			bool bFirstGlyph = true;
			long nCharWidth = 0;
			int nIndex = pLayoutData->mpCharsToChars[ nCharPos - nMinFallbackCharPos ];
			if ( nIndex >= 0 )
			{
				ImplATSLayoutData *pCurrentLayoutData = pLayoutData;

				sal_Unicode nChar = pCurrentLayoutData->mpHash->mpStr[ nIndex ];

				if ( bRunRTL )
				{
					// Fix bugs 1637 and 1797 by laying out mirrored characters
					// separately
					sal_Unicode nMirroredChar = (sal_Unicode)GetMirroredChar( nChar );
					if ( nMirroredChar != nChar )
					{
						::std::map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.find( nChar );
						if ( mit == maMirroredLayoutData.end() )
						{
							sal_Unicode aMirrored[ 1 ];
							aMirrored[ 0 ] = nMirroredChar;
							pCurrentLayoutData = ImplATSLayoutData::GetLayoutData( aMirrored, 1, 0, 1, ( rArgs.mnFlags & ~SAL_LAYOUT_BIDI_RTL ) | SAL_LAYOUT_BIDI_STRONG, mnFallbackLevel, mpVCLFont, this );
							if ( pCurrentLayoutData )
							{
								if ( pCurrentLayoutData->mpNeedFallback && pCurrentLayoutData->mpFallbackFont )
								{
									pCurrentLayoutData->Release();
									pCurrentLayoutData = pLayoutData;
									rArgs.NeedFallback( nCharPos, bRunRTL );

									if ( !pFallbackFont )
										pFallbackFont = pCurrentLayoutData->mpFallbackFont;
									rArgs.mnFlags &= ~SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
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

				for ( int i = pCurrentLayoutData->mpCharsToGlyphs[ nIndex ]; i >= 0 && i < pCurrentLayoutData->mnGlyphCount && pCurrentLayoutData->mpGlyphDataArray && ( pCurrentLayoutData->mpGlyphDataArray[ i ].originalOffset / 2 ) == nIndex; i++ )
				{
					long nGlyph = pCurrentLayoutData->mpGlyphDataArray[ i ].glyphID;
					if ( !nGlyph )
					{
						if ( nChar >= 0xe000 && nChar < 0xf900 )
						{
							// If there is no fallback font and it is a Private
							// Use Area character, use the symbol font
							bNeedSymbolFallback = true;
							rArgs.NeedFallback( nCharPos, bRunRTL );
							rArgs.mnFlags &= ~SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
						}
						else if ( GetVerticalFlags( nChar ) != GF_NONE )
						{
							// Fix bug 2772 by always using the fallback font
							// picked by the native APIs when rotatable
							// characters are used
							bUseNativeFallback = true;
							if ( !pFallbackFont )
								pFallbackFont = pCurrentLayoutData->mpFallbackFont;
							rArgs.NeedFallback( nCharPos, bRunRTL );
							rArgs.mnFlags &= ~SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
						}
						else if ( pCurrentLayoutData->mpFallbackFont )
						{
							// Fix bug 2091 by suppressing zero glyphs if there
							// is a fallback font
							if ( !pFallbackFont )
								pFallbackFont = pCurrentLayoutData->mpFallbackFont;
							rArgs.NeedFallback( nCharPos, bRunRTL );
							rArgs.mnFlags &= ~SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
						}
					}

					// Fix bugs 810, 1806, 1927, and 2089 by treating all
					// 0x0000ffff glyphs as spaces
					if ( nGlyph >= 0x0000ffff )
					{
						if ( bFirstGlyph )
							nGlyph = 0x0020 | GF_ISCHAR;
						else
							continue;
					}

					// Fix bug 2512 without breaking fix for bug 2453 by
					// allowing spacing glyphs to go through but marking when
					// glyph 3 is not a spacing glyph
					if ( nGlyph == 3 && mbSpecialSpacingGlyph && !IsSpacingGlyph( nChar | GF_ISCHAR ) )
						mbSpecialSpacingGlyph = false;

					if ( pCurrentLayoutData->maVerticalFontStyle )
						nGlyph |= GetVerticalFlags( nChar );

					int nGlyphFlags = bFirstGlyph ? 0 : GlyphItem::IS_IN_CLUSTER;
					if ( bRunRTL )
					{
						nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;

						// Note characters that we can append kashidas onto
						if ( bFirstGlyph && rArgs.mnFlags & SAL_LAYOUT_KASHIDA_JUSTIFICATON && nCharPos > nMinFallbackCharPos )
						{
							int nPreviousIndex = pLayoutData->mpCharsToChars[ nCharPos - nMinFallbackCharPos - 1 ];
							if ( nPreviousIndex >= 0 )
							{
								UJoiningType nTypeLeft = (UJoiningType)u_getIntPropertyValue( nChar, UCHAR_JOINING_TYPE );
								UJoiningType nTypeRight = (UJoiningType)u_getIntPropertyValue( pCurrentLayoutData->mpHash->mpStr[ nPreviousIndex ], UCHAR_JOINING_TYPE );
								if ( ( nTypeLeft == U_JT_RIGHT_JOINING || nTypeLeft == U_JT_DUAL_JOINING ) && ( nTypeRight == U_JT_LEFT_JOINING || nTypeRight == U_JT_DUAL_JOINING || nTypeRight == U_JT_TRANSPARENT ) )
									nGlyphFlags |= GlyphItem::IS_KASHIDA_ALLOWED_AFTER_GLYPH;
							}
						}
					}

					AppendGlyph( GlyphItem( nCharPos, nGlyph, aPos, nGlyphFlags, nCharWidth ) );

					if ( bFirstGlyph )
					{
						aPos.X() += nCharWidth;
						nCharWidth = 0;
						bFirstGlyph = false;
						bRet = true;
					}
				}
			}

			if ( bFirstGlyph )
			{
				AppendGlyph( GlyphItem( nCharPos, 0x0020 | GF_ISCHAR, aPos, bRunRTL ? GlyphItem::IS_RTL_GLYPH : 0, nCharWidth ) );
				aPos.X() += nCharWidth;
				bRet = true;
			}
		}

		maLayoutData.push_back( pLayoutData );
		maLayoutMinCharPos.push_back( nMinFallbackCharPos );
	}

	mnOrigWidth = aPos.X();

	// Set fallback font
	if ( pFallbackFont || bNeedSymbolFallback || ! ( rArgs.mnFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING ) )
	{
		// If this is the first fallback, first try using a font that most
		// closely matches the currently requested font
		JavaImplFontData *pHighScoreFontData = NULL;
		if ( ( !mnFallbackLevel || bNeedSymbolFallback ) && ( !mpKashidaLayoutData || !mpKashidaLayoutData->mpFallbackFont ) )
		{
			SalData *pSalData = GetSalData();

			sal_IntPtr nNativeFont = mpVCLFont->getNativeFont();
			if ( bNeedSymbolFallback )
			{
				::std::map< String, JavaImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( String( RTL_CONSTASCII_USTRINGPARAM( "OpenSymbol" ) ) );
				if ( it != pSalData->maFontNameMapping.end() && (int)it->second->GetFontId() != nNativeFont )
					pHighScoreFontData = it->second;
			}

			if ( !pHighScoreFontData && !bUseNativeFallback )
			{
				::std::map< sal_IntPtr, JavaImplFontData* >::const_iterator it = pSalData->maNativeFontMapping.find( pFallbackFont ? pFallbackFont->getNativeFont() : 0 );
				if ( it == pSalData->maNativeFontMapping.end() || it->second->GetFamilyType() != mpGraphics->mnFontFamily || it->second->GetWeight() != mpGraphics->mnFontWeight || ( it->second->GetSlant() == ITALIC_OBLIQUE || it->second->GetSlant() == ITALIC_NORMAL ? true : false ) != mpGraphics->mbFontItalic || it->second->GetPitch() != mpGraphics->mnFontPitch )
				{
					USHORT nHighScore = 0;
					for ( it = pSalData->maNativeFontMapping.begin(); it != pSalData->maNativeFontMapping.end(); ++it )
					{
						if ( (int)it->first == nNativeFont )
							continue;

						USHORT nScore = ( ( it->second->GetSlant() == ITALIC_OBLIQUE || it->second->GetSlant() == ITALIC_NORMAL ? true : false ) == mpGraphics->mbFontItalic ? 8 : 0 );
						nScore += ( it->second->GetWeight() == mpGraphics->mnFontWeight ? 4 : 0 );
						nScore += ( it->second->GetFamilyType() == mpGraphics->mnFontFamily ? 2 : 0 );
						nScore += ( it->second->GetPitch() == mpGraphics->mnFontPitch ? 1 : 0 );
						if ( nScore == 15 )
						{
							pHighScoreFontData = it->second;
							break;
						}
						else if ( nHighScore < nScore )
						{
							pHighScoreFontData = it->second;
							nHighScore = nScore;
						}
					}
				}
			}
		}

		int nNextLevel = mnFallbackLevel + 1;
		::std::map< int, com_sun_star_vcl_VCLFont* >::iterator it = mpGraphics->maFallbackFonts.find( nNextLevel );
		if ( it != mpGraphics->maFallbackFonts.end() )
		{
			delete it->second;
			mpGraphics->maFallbackFonts.erase( it );
		}

		// Use the kashida fallback font first so that we are assured of
		// rendering a kashida if needed
		if ( mpKashidaLayoutData && mpKashidaLayoutData->mpFallbackFont )
			mpGraphics->maFallbackFonts[ nNextLevel ] = new com_sun_star_vcl_VCLFont( mpKashidaLayoutData->mpFallbackFont );
		else if ( pHighScoreFontData )
			mpGraphics->maFallbackFonts[ nNextLevel ] = new com_sun_star_vcl_VCLFont( pHighScoreFontData->maVCLFontName, mpVCLFont->getSize(), mpVCLFont->getOrientation(), mpVCLFont->isAntialiased(), mpVCLFont->isVertical(), mpVCLFont->getScaleX(), 0 );
		else if ( pFallbackFont )
			mpGraphics->maFallbackFonts[ nNextLevel ] = new com_sun_star_vcl_VCLFont( pFallbackFont );
		else
			rArgs.mnFlags |= SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
	}

	return bRet;
}

// ----------------------------------------------------------------------------

void SalATSLayout::DrawText( SalGraphics& rGraphics ) const
{
	int nMaxGlyphs = 256;
	long aGlyphArray[ nMaxGlyphs ];
	long aDXArray[ nMaxGlyphs ];
	int aCharPosArray[ nMaxGlyphs ];

	Point aPos;
	JavaSalGraphics& rJavaGraphics = (JavaSalGraphics&)rGraphics;
	bool bPrinter = ( rJavaGraphics.mpPrinter ? true : false );
	for ( int nStart = 0; ; )
	{
		int nTotalGlyphCount = GetNextGlyphs( nMaxGlyphs, aGlyphArray, aPos, nStart, aDXArray, aCharPosArray );

		if ( !nTotalGlyphCount )
			break;

		Point aCurrentPos( aPos );
		int nCurrentGlyph = 0;
		while ( nCurrentGlyph < nTotalGlyphCount )
		{
			int i;
			int nStartGlyph = nCurrentGlyph;
			int nGlyphCount = 0;

			// Skip spacing glyphs
			for ( ; nStartGlyph < nTotalGlyphCount && aGlyphArray[ nStartGlyph ] & GF_ISCHAR; nStartGlyph++ )
				aCurrentPos.X() += aDXArray[ nStartGlyph ];

			// Determine glyph count but only allow one glyph at a time for
			// rotated glyphs
			Point aStartPos( aCurrentPos );
			long nGlyphOrientation = aGlyphArray[ nStartGlyph ] & GF_ROTMASK;
			if ( nStartGlyph < nTotalGlyphCount )
			{
				if ( nGlyphOrientation )
				{
					nGlyphCount = 1;
					aCurrentPos.X() += aDXArray[ nStartGlyph ];
				}
				else
				{
					for ( i = nStartGlyph; i < nTotalGlyphCount && ! ( aGlyphArray[ i ] & GF_ISCHAR ); i++ )
					{
						nGlyphCount++;
						aCurrentPos.X() += aDXArray[ i ];
					}
				}
			}

			nCurrentGlyph = nStartGlyph + nGlyphCount;
			if ( !nGlyphCount )
				continue;

			long nTranslateX = 0;
			long nTranslateY = 0;

			if ( nGlyphOrientation )
			{
				long nX;
				long nY;
				ImplATSLayoutData *pFoundLayout = GetVerticalGlyphTranslation( aGlyphArray[ nStartGlyph ], aCharPosArray[ nStartGlyph ], nX, nY );
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
						nTranslateY = Float32ToLong( pFoundLayout->mpHash->mfFontScaleX * ( aDXArray[ nStartGlyph ] - nY ) );
					else
						nTranslateY = nY;
				}
			}

			for ( i = nStartGlyph; i < nGlyphCount; i++ )
				aGlyphArray[ i ] &= GF_IDXMASK;

			if ( bPrinter )
			{
				// Don't delete the CGGlyph buffer and let the Java native
				// method print the buffer directly
				CGGlyph *pGlyphs = (CGGlyph *)rtl_allocateMemory( nGlyphCount * sizeof( CGGlyph ) );
				if ( pGlyphs )
				{
					for ( i = 0; i < nGlyphCount; i++ )
						pGlyphs[ i ] = (CGGlyph)aGlyphArray[ i + nStartGlyph ];
				}

				// Don't delete the CGSize buffer and let the Java native
				// method print the buffer directly
				CGSize *pSizes = (CGSize *)rtl_allocateMemory( nGlyphCount * sizeof( CGSize ) );
				if ( pSizes )
				{
					for ( i = 0; i < nGlyphCount; i++ )
					{
						pSizes[ i ].width = (float)aDXArray[ i + nStartGlyph ];
						pSizes[ i ].height = 0.0f;
					}
				}

				rJavaGraphics.mpVCLGraphics->drawGlyphBuffer( aStartPos.X(), aStartPos.Y(), nGlyphCount, pGlyphs, pSizes, mpVCLFont, rJavaGraphics.mnTextColor, GetOrientation(), nGlyphOrientation, nTranslateX, nTranslateY, mfGlyphScaleX );
			}
			else
			{
				rJavaGraphics.mpVCLGraphics->drawGlyphs( aStartPos.X(), aStartPos.Y(), nGlyphCount, aGlyphArray + nStartGlyph, aDXArray + nStartGlyph, mpVCLFont, rJavaGraphics.mnTextColor, GetOrientation(), nGlyphOrientation, nTranslateX, nTranslateY, mfGlyphScaleX );
			}
		}
	}
}

// ----------------------------------------------------------------------------

bool SalATSLayout::GetOutline( SalGraphics& rGraphics, B2DPolyPolygonVector& rVector ) const
{
	bool bRet = false;

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

		if ( aGlyphArray[ 0 ] & GF_ISCHAR )
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

		// Check if this is a mirrored or ideographic space character
		sal_Unicode nChar = pCurrentLayoutData->mpHash->mpStr[ nIndex ];
		if ( pCurrentLayoutData->mpHash->mbRTL )
		{
			::std::map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.find( nChar );
			if ( mit != maMirroredLayoutData.end() )
			{
				pCurrentLayoutData = mit->second;
				nIndex = 0;
			}
		}

		for ( int i = pCurrentLayoutData->mpCharsToGlyphs[ nIndex ]; i >= 0 && i < pCurrentLayoutData->mnGlyphCount && pCurrentLayoutData->mpGlyphDataArray && ( pCurrentLayoutData->mpGlyphDataArray[ i ].originalOffset / 2 ) == nIndex; i++ )
		{
			long nGlyph = pCurrentLayoutData->mpGlyphDataArray[ i ].glyphID;
			if ( ( aGlyphArray[ 0 ] & GF_IDXMASK ) != nGlyph )
				continue;

			// Fix bug 2390 by ignoring the value of nErr passed by reference
			::std::list< Polygon > aPolygonList;
			ATSUStyle aCurrentStyle = NULL;
			UniCharArrayOffset nRunStart;
			UniCharCount nRunLen;
			OSStatus nErr;
			if ( ATSUGetRunStyle( pCurrentLayoutData->maLayout, nIndex, &aCurrentStyle, &nRunStart, &nRunLen ) != noErr || !aCurrentStyle || ATSUGlyphGetCubicPaths( aCurrentStyle, pCurrentLayoutData->mpGlyphDataArray[ i ].glyphID, pATSCubicMoveToUPP, pATSCubicLineToUPP, pATSCubicCurveToUPP, pATSCubicClosePathUPP, (void *)&aPolygonList, &nErr ) != noErr )
				continue;

			PolyPolygon aPolyPolygon;
			while ( aPolygonList.size() )
			{
				aPolygonList.front().Scale( pCurrentLayoutData->mfFontScaleY, pCurrentLayoutData->mfFontScaleY );
				aPolyPolygon.Insert( aPolygonList.front() );
				aPolygonList.pop_front();
			}

			// Fix bug 2537 by ignoring unusual bounds
			Rectangle aRect = aPolyPolygon.GetBoundRect();
			if ( aRect.GetWidth() <= 0 || aRect.GetHeight() <= 0 )
				continue;

			long nTranslateX = 0;
			long nTranslateY = 0;

			aPolyPolygon.Move( aPos.X(), aPos.Y() );

			long nGlyphOrientation = aGlyphArray[ 0 ] & GF_ROTMASK;
			if ( pCurrentLayoutData->maVerticalFontStyle )
			{
				long nX;
				long nY;
				GetVerticalGlyphTranslation( aGlyphArray[ 0 ], aCharPosArray[ 0 ], nX, nY );
				if ( nGlyphOrientation == GF_ROTL )
				{
					nTranslateX = nX;
					nTranslateY = nY;
				}
				else if ( nGlyphOrientation == GF_ROTR )
				{
					aPolyPolygon.Rotate( aPos, 1800 );
					nTranslateX = nX;
					nTranslateY = nY;
				}
				else
				{
					aPolyPolygon.Rotate( aPos, 2700 );
					nTranslateX = nX;
					nTranslateY = nY;
				}
			}

			aPolyPolygon.Move( nTranslateX, nTranslateY );

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

	mnOrigWidth = 0;
	mfGlyphScaleX = 1.0;
}

// ----------------------------------------------------------------------------

ImplATSLayoutData *SalATSLayout::GetVerticalGlyphTranslation( long nGlyph, int nCharPos, long& nX, long& nY ) const
{
	ImplATSLayoutData *pRet = NULL;

	nX = 0;
	nY = 0;

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
	if ( pRet->mpHash->mbRTL )
	{
		::std::map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.find( nChar );
		if ( mit != maMirroredLayoutData.end() )
		{
			pRet = mit->second;
			nIndex = 0;
		}
	}

	long nGlyphOrientation = nGlyph & GF_ROTMASK;

	if ( pRet->maVerticalFontStyle && nGlyphOrientation & GF_ROTMASK )
	{
		GlyphID nGlyphID = (GlyphID)( nGlyph & GF_IDXMASK );

		ATSGlyphScreenMetrics aVerticalMetrics;
		ATSGlyphScreenMetrics aHorizontalMetrics;
		if ( ATSUGlyphGetScreenMetrics( pRet->maVerticalFontStyle, 1, &nGlyphID, sizeof( GlyphID ), pRet->mpHash->mbAntialiased, pRet->mpHash->mbAntialiased, &aVerticalMetrics ) == noErr && ATSUGlyphGetScreenMetrics( pRet->maFontStyle, 1, &nGlyphID, sizeof( GlyphID ), pRet->mpHash->mbAntialiased, pRet->mpHash->mbAntialiased, &aHorizontalMetrics ) == noErr )
		{
			nX = Float32ToLong( ( aVerticalMetrics.topLeft.x - aHorizontalMetrics.topLeft.x ) * pRet->mfFontScaleY );
			if ( nGlyphOrientation == GF_ROTL )
				nX += pRet->mnBaselineDelta;
			else
				nX -= pRet->mnBaselineDelta;
			nY = Float32ToLong( ( aHorizontalMetrics.topLeft.y - aVerticalMetrics.topLeft.y ) * pRet->mfFontScaleY );
		}
	}

	return pRet;
}
