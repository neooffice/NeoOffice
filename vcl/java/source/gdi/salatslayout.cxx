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
	static ::std::hash_map< ImplATSLayoutDataHash*, ImplATSLayoutData*, ImplHash, ImplHashEquality >	maLayoutCache;
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
	int*				mpCharsToGlyphs;
	long*				mpCharAdvances;
	ATSUStyle			maVerticalFontStyle;
	long				mnBaselineDelta;
	bool				mbValid;

	static ImplATSLayoutData*	GetLayoutData( ImplLayoutArgs& rArgs, int nFallbackLevel, ::vcl::com_sun_star_vcl_VCLFont *pVCLFont );

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
	return ( p1->mnFallbackLevel == p2->mnFallbackLevel &&
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

::std::hash_map< ImplATSLayoutDataHash*, ImplATSLayoutData*, ImplHash, ImplHashEquality > ImplATSLayoutData::maLayoutCache;

// ----------------------------------------------------------------------------

::std::list< ImplATSLayoutData* > ImplATSLayoutData::maLayoutCacheList;

// ----------------------------------------------------------------------------

ImplATSLayoutData *ImplATSLayoutData::GetLayoutData( ImplLayoutArgs& rArgs, int nFallbackLevel, com_sun_star_vcl_VCLFont *pVCLFont )
{
	ImplATSLayoutData *pLayoutData = NULL;

	ImplATSLayoutDataHash *pLayoutHash = new ImplATSLayoutDataHash();
	pLayoutHash->mnFallbackLevel = nFallbackLevel;
	pLayoutHash->mnLen = rArgs.mnLength;
	pLayoutHash->mnFontID = (ATSUFontID)pVCLFont->getNativeFont();
	pLayoutHash->mnFontSize = pVCLFont->getSize();
	pLayoutHash->mfFontScaleX = pVCLFont->getScaleX();
	pLayoutHash->mbAntialiased = pVCLFont->isAntialiased();
	pLayoutHash->mbRTL = ( rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL );
	pLayoutHash->mbVertical = ( rArgs.mnFlags & SAL_LAYOUT_VERTICAL );
	pLayoutHash->mpStr = (sal_Unicode *)rArgs.mpStr;
	pLayoutHash->mnStrHash = rtl_ustr_hashCode_WithLength( pLayoutHash->mpStr, pLayoutHash->mnLen );

	// Search cache for matching layout
	::std::hash_map< ImplATSLayoutDataHash*, ImplATSLayoutData*, ImplHash, ImplHashEquality >::const_iterator it = maLayoutCache.find( pLayoutHash );
	if ( it != maLayoutCache.end() )
		pLayoutData = it->second;

	if ( !pLayoutData )
	{
		// Copy the string so that we can cache it
		pLayoutHash->mpStr = (sal_Unicode *)rtl_allocateMemory( pLayoutHash->mnLen * sizeof( sal_Unicode ) );
		memcpy( pLayoutHash->mpStr, rArgs.mpStr, pLayoutHash->mnLen * sizeof( sal_Unicode ) );

		pLayoutData = new ImplATSLayoutData( rArgs, pLayoutHash, nFallbackLevel, pVCLFont );

		if ( !pLayoutData->IsValid() )
		{
			pLayoutData->Release();
			return NULL;
		}

		maLayoutCache[ pLayoutData->mpHash ] = pLayoutData;
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

	// Fix bug 449 by turning off Unicode composition
	ATSUFontFeatureType aTypes[3];
	ATSUFontFeatureSelector aSelectors[3];
	aTypes[0] = kUnicodeDecompositionType;
	aTypes[1] = kUnicodeDecompositionType;
	aTypes[2] = kUnicodeDecompositionType;
	aSelectors[0] = kCanonicalCompositionOffSelector;
	aSelectors[1] = kCompatibilityCompositionOffSelector;
	aSelectors[2] = kTranscodingCompositionOffSelector;
	if ( ATSUSetFontFeatures( maFontStyle, 3, aTypes, aSelectors ) != noErr )
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

	if ( rArgs.mnFlags & SAL_LAYOUT_VERTICAL )
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

	// Break lines that are more than 32K pixels long to avoid messing up
	// the metrics that ATSUGetGlyphBounds() returns
	if ( ATSUBatchBreakLines( maLayout, kATSUFromTextBeginning, kATSUToTextEnd, Long2Fix( 32768 ), NULL ) != noErr )
	{
		Destroy();
		return;
	}

	// Cache glyph widths
	nBufSize = mpHash->mnLen * sizeof( long );
	mpCharAdvances = (long *)rtl_allocateMemory( nBufSize );
	memset( mpCharAdvances, 0, nBufSize );

	int i;
	ATSTrapezoid aTrapezoid;
	for ( i = 0; i < mpHash->mnLen; i++ )
	{
		// Fix bug 448 by eliminating subpixel advances
		if ( ATSUGetGlyphBounds( maLayout, 0, 0, i, 1, kATSUseFractionalOrigins, 1, &aTrapezoid, NULL ) == noErr )
			mpCharAdvances[ i ] = Float32ToLong( Fix2X( aTrapezoid.upperRight.x - aTrapezoid.upperLeft.x ) * mpHash->mfFontScaleX );
	}

	// Cache mapping of characters to glyphs
	nBufSize = mpHash->mnLen * sizeof( int );
	mpCharsToGlyphs = (int *)rtl_allocateMemory( nBufSize );

	for ( i = 0; i < mpHash->mnLen; i++ )
		mpCharsToGlyphs[ i ] = -1;
	for ( i = 0; i < mnGlyphCount; i++ )
	{
		int nCharPos = mpGlyphInfoArray->glyphs[ i ].charIndex;
		if ( mpCharsToGlyphs[ nCharPos ] < 0 || i < mpCharsToGlyphs[ nCharPos ] )
			mpCharsToGlyphs[ nCharPos ] = i;
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
				::std::map< void*, ImplFontData* >::const_iterator it = pSalData->maNativeFontMapping.find( (void *)nFontID );
				if ( it != pSalData->maNativeFontMapping.end() )
				{
					com_sun_star_vcl_VCLFont *pVCLFont = (com_sun_star_vcl_VCLFont *)it->second->mpSysData;
					mpFallbackFont = pVCLFont->deriveFont( mpHash->mnFontSize, mpVCLFont->isBold(), mpVCLFont->isItalic(), mpVCLFont->getOrientation(), mpHash->mbAntialiased, mpHash->mbVertical, mpHash->mfFontScaleX );
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
	return new SalATSLayout( this, nFallbackLevel );
}

// ============================================================================

SalATSLayout::SalATSLayout( SalGraphics *pGraphics, int nFallbackLevel ) :
	mpGraphics( pGraphics ),
	mnFallbackLevel( nFallbackLevel ),
	mpVCLFont( NULL ),
	mpLayoutData( NULL )
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

bool SalATSLayout::LayoutText( ImplLayoutArgs& rArgs )
{
	Destroy();

	if ( !mpVCLFont )
		return false;

	mpLayoutData = ImplATSLayoutData::GetLayoutData( rArgs, mnFallbackLevel, mpVCLFont );
	if ( !mpLayoutData )
		return false;

	// Create fallback runs
	if ( mpLayoutData->mpNeedFallback && mpLayoutData->mpFallbackFont )
	{
		bool bPosRTL;
		int nCharPos = -1;
		rArgs.ResetPos();
		while ( rArgs.GetNextPos( &nCharPos, &bPosRTL ) )
		{
			if ( mpLayoutData->mpNeedFallback[ nCharPos ] )
				rArgs.NeedFallback( nCharPos, bPosRTL );
		}

		int nNextLevel = mnFallbackLevel + 1;
		::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator it = mpGraphics->maGraphicsData.maFallbackFonts.find( nNextLevel );
		if ( it != mpGraphics->maGraphicsData.maFallbackFonts.end() )
			delete it->second;
		mpGraphics->maGraphicsData.maFallbackFonts[ nNextLevel ] = new com_sun_star_vcl_VCLFont( mpLayoutData->mpFallbackFont->getJavaObject() );
	}
	else
	{
		rArgs.mnFlags |= SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
	}

	// Calculate and cache glyph advances
	bool bPosRTL;
	Point aPos( 0, 0 );
	int nCharPos = -1;
	rArgs.ResetPos();
	while ( rArgs.GetNextPos( &nCharPos, &bPosRTL ) )
	{
		sal_Unicode nChar = mpLayoutData->mpHash->mpStr[ nCharPos ];
		long nCharWidth = mpLayoutData->mpCharAdvances[ nCharPos ];

		bool bFirstGlyph = true;
		for ( int i = mpLayoutData->mpCharsToGlyphs[ nCharPos ]; i < mpLayoutData->mnGlyphCount && mpLayoutData->mpGlyphInfoArray->glyphs[ i ].charIndex == nCharPos; i++ )
		{
			long nGlyph = mpLayoutData->mpGlyphInfoArray->glyphs[ i ].glyphID;

			if ( mpLayoutData->maVerticalFontStyle )
				nGlyph |= GetVerticalFlags( nChar );

			// Mark whitespace glyphs
			if ( mpLayoutData->mpGlyphInfoArray->glyphs[ i ].glyphID == 0xffff || IsSpacingGlyph( nChar | GF_ISCHAR ) || mpLayoutData->mpGlyphInfoArray->glyphs[ i ].layoutFlags & kATSGlyphInfoTerminatorGlyph )
				nGlyph = 0x0020 | GF_ISCHAR;

			int nGlyphFlags = bFirstGlyph ? 0 : GlyphItem::IS_IN_CLUSTER;
			if ( bPosRTL )
				nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;

			AppendGlyph( GlyphItem( nCharPos, nGlyph, aPos, nGlyphFlags, nCharWidth ) );

			aPos.X() += nCharWidth;
			bFirstGlyph = false;
		}

		if ( bFirstGlyph )
			AppendGlyph( GlyphItem( nCharPos, 0x0020 | GF_ISCHAR, aPos, bPosRTL ? GlyphItem::IS_RTL_GLYPH : 0, 0 ) );
	}

	return ( nCharPos >= 0 );
}

// ----------------------------------------------------------------------------

void SalATSLayout::DrawText( SalGraphics& rGraphics ) const
{
	if ( !mpLayoutData )
		return;

	int nMaxGlyphs( mpLayoutData->mnGlyphCount );
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
				nTranslateX = nX + mpLayoutData->mnBaselineDelta;
				nTranslateY = Float32ToLong( nY * mpLayoutData->mpHash->mfFontScaleX );
			}
			else
			{
				nTranslateX = nX - mpLayoutData->mnBaselineDelta;
				nTranslateY = Float32ToLong( ( aDXArray[ 0 ] - nY ) * mpLayoutData->mpHash->mfFontScaleX );
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

	if ( !mpLayoutData )
		return bRet;

	int nMaxGlyphs( 1 );
	long aGlyphArray[ nMaxGlyphs ];
	long aDXArray[ nMaxGlyphs ];
	int aCharPosArray[ nMaxGlyphs ];

	Point aPos;
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

		for ( int i = mpLayoutData->mpCharsToGlyphs[ aCharPosArray[ 0 ] ]; i < mpLayoutData->mnGlyphCount && mpLayoutData->mpGlyphInfoArray->glyphs[ i ].charIndex == aCharPosArray[ 0 ]; i++ )
		{
			long nGlyph = mpLayoutData->mpGlyphInfoArray->glyphs[ i ].glyphID;
			if ( ( aGlyphArray[ 0 ] & GF_IDXMASK ) != nGlyph )
				continue;

			::std::list< Polygon > aPolygonList;
			OSStatus nErr;
			if ( ATSUGlyphGetCubicPaths( mpLayoutData->mpGlyphInfoArray->glyphs[ i ].style, mpLayoutData->mpGlyphInfoArray->glyphs[ i ].glyphID, SalATSCubicMoveToCallback, SalATSCubicLineToCallback, SalATSCubicCurveToCallback, SalATSCubicClosePathCallback, (void *)&aPolygonList, &nErr ) != noErr )
				continue;

			PolyPolygon aPolyPolygon;
			while ( aPolygonList.size() )
			{
				aPolyPolygon.Insert( aPolygonList.front() );
				aPolygonList.pop_front();
			}

			aPolyPolygon.Move( aPos.X(), aPos.Y() );

			// Sync coordinates to glyph metrics
			ATSGlyphScreenMetrics aScreenMetrics;
			if ( ATSUGlyphGetScreenMetrics( mpLayoutData->mpGlyphInfoArray->glyphs[ i ].style, 1, &mpLayoutData->mpGlyphInfoArray->glyphs[ i ].glyphID, sizeof( GlyphID ), false, false, &aScreenMetrics ) == noErr )
				aPolyPolygon.Move( Float32ToLong( aScreenMetrics.topLeft.x ) - aPolyPolygon.GetBoundRect().nLeft, ( Float32ToLong( aScreenMetrics.topLeft.y ) + aPolyPolygon.GetBoundRect().nTop ) * -1 );

			if ( mpLayoutData->maVerticalFontStyle )
			{
				long nGlyphOrientation = aGlyphArray[ 0 ] & GF_ROTMASK;
				if ( nGlyphOrientation )
				{
					if ( nGlyphOrientation == GF_ROTL )
					{
						aPolyPolygon.Rotate( Point( 0, 0 ), 900 );
						aPolyPolygon.Move( aPolyPolygon.GetBoundRect().nLeft * -1, mpLayoutData->mnBaselineDelta * -1 );
					}
					else
					{
						aPolyPolygon.Rotate( Point( 0, 0 ), 2700 );
						aPolyPolygon.Move( aDXArray[ 0 ] - aPolyPolygon.GetBoundRect().nRight, mpLayoutData->mnBaselineDelta * -1 );
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
	if ( mpLayoutData )
	{
		mpLayoutData->Release();
		mpLayoutData = NULL;
	}
}

// ----------------------------------------------------------------------------

long SalATSLayout::GetBaselineDelta() const
{
	return ( mpLayoutData ? mpLayoutData->mnBaselineDelta : 0 );
}

// ----------------------------------------------------------------------------

void SalATSLayout::GetVerticalGlyphTranslation( long nGlyph, long& nX, long& nY ) const
{
	nX = 0;
	nY = 0;

	if ( !mpLayoutData )
		return;

	long nGlyphOrientation = nGlyph & GF_ROTMASK;

	if ( mpLayoutData->maVerticalFontStyle && nGlyphOrientation & GF_ROTMASK )
	{
		GlyphID nGlyphID = (GlyphID)( nGlyph & GF_IDXMASK );

		ATSGlyphScreenMetrics aVerticalMetrics;
		ATSGlyphScreenMetrics aHorizontalMetrics;
		if ( ATSUGlyphGetScreenMetrics( mpLayoutData->maVerticalFontStyle, 1, &nGlyphID, sizeof( GlyphID ), mpLayoutData->mpHash->mbAntialiased, mpLayoutData->mpHash->mbAntialiased, &aVerticalMetrics ) == noErr && ATSUGlyphGetScreenMetrics( mpLayoutData->maFontStyle, 1, &nGlyphID, sizeof( GlyphID ), mpLayoutData->mpHash->mbAntialiased, mpLayoutData->mpHash->mbAntialiased, &aHorizontalMetrics ) == noErr )
		{
			nX = Float32ToLong( aVerticalMetrics.topLeft.x - aHorizontalMetrics.topLeft.x );
			nY = Float32ToLong( aHorizontalMetrics.topLeft.y - aVerticalMetrics.topLeft.y );
		}
	}
}
