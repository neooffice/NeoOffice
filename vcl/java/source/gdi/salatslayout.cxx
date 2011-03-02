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

#include <dlfcn.h>
#include <sys/sysctl.h>
#include <unicode/ubidi.h>

#ifndef _SV_SALATSLAYOUT_HXX
#include <salatslayout.hxx>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_OUTFONT_HXX
#include <vcl/outfont.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#define MAXEXTRACHARS 100
#ifdef USE_SUBPIXEL_TEXT_RENDERING
#define UNITS_PER_PIXEL 1000
#else	// USE_SUBPIXEL_TEXT_RENDERING
#define UNITS_PER_PIXEL 1
#endif	// USE_SUBPIXEL_TEXT_RENDERING

typedef OSStatus ATSUCalculateBaselineDeltas_Type( ATSUStyle aStyle, BslnBaselineClass nBaselineClass, BslnBaselineRecord aBaselineDeltas );
typedef OSStatus ATSUCreateAndCopyStyle_Type( ATSUStyle aStyle, ATSUStyle *pStyle );
typedef OSStatus ATSUCreateFontFallbacks_Type( ATSUFontFallbacks *pFontFallback );
typedef OSStatus ATSUCreateStyle_Type( ATSUStyle *pStyle );
typedef OSStatus ATSUCreateTextLayoutWithTextPtr_Type( ConstUniCharArrayPtr aText, UniCharArrayOffset nTextOffset, UniCharCount nTextLength, UniCharCount nTextTotalLength, ItemCount nNumberOfRuns, const UniCharCount aRunLengths[], ATSUStyle aStyles[], ATSUTextLayout *pTextLayout );
typedef OSStatus ATSUDirectGetLayoutDataArrayPtrFromTextLayout_Type( ATSUTextLayout aTextLayout, UniCharArrayOffset nLineOffset, ATSUDirectDataSelector nDataSelector, void *pLayoutDataArrayPtr[], ItemCount *pLayoutDataCount );
typedef OSStatus ATSUDirectReleaseLayoutDataArrayPtr_Type( ATSULineRef aLineRef, ATSUDirectDataSelector nDataSelector, void *pLayoutDataArrayPtr[] );
typedef OSStatus ATSUDisposeFontFallbacks_Type( ATSUFontFallbacks aFontFallbacks );
typedef OSStatus ATSUDisposeStyle_Type( ATSUStyle aStyle );
typedef OSStatus ATSUDisposeTextLayout_Type( ATSUTextLayout aTextLayout );
typedef OSStatus ATSUGetGlyphBounds_Type( ATSUTextLayout aTextLayout, ATSUTextMeasurement fTextBasePointX, ATSUTextMeasurement fTextBasePointY, UniCharArrayOffset nBoundsCharStart, UniCharCount nBoundsCharLength, UInt16 nTypeOfBounds, ItemCount nMaxNumberOfBounds, ATSTrapezoid aGlyphBounds[], ItemCount *pActualNumberOfBounds );
typedef OSStatus ATSUGetRunStyle_Type( ATSUTextLayout aTextLayout, UniCharArrayOffset nOffset, ATSUStyle *pStyle, UniCharArrayOffset *pRunStart, UniCharCount *pRunLength );
typedef OSStatus ATSUGlyphGetCubicPaths_Type( ATSUStyle aATSUStyle, GlyphID nGlyphID, ATSCubicMoveToUPP aMoveToProc, ATSCubicLineToUPP aLineToProc, ATSCubicCurveToUPP aCurveToProc, ATSCubicClosePathUPP aClosePathProc, void *pCallbackDataPtr, OSStatus *pCallbackResult );
typedef OSStatus ATSUGlyphGetIdealMetrics_Type( ATSUStyle nATSUStyle, ItemCount nNumOfGlyphs, GlyphID aGlyphIDs[], ByteOffset nInputOffset, ATSGlyphIdealMetrics aIdealMetrics[] );
typedef OSStatus ATSUGlyphGetScreenMetrics_Type( ATSUStyle aATSUStyle, ItemCount nNumOfGlyphs, GlyphID aGlyphIDs[], ByteOffset nInputOffset, MacOSBoolean bForcingAntiAlias, MacOSBoolean bAntiAliasSwitch, ATSGlyphScreenMetrics aScreenMetrics[] );
typedef OSStatus ATSUMatchFontsToText_Type( ATSUTextLayout aTextLayout, UniCharArrayOffset nTextStart, UniCharCount nTextLength, ATSUFontID *pFontID, UniCharArrayOffset *pChangedOffset, UniCharCount *pChangedLength );
typedef OSStatus ATSUMeasureTextImage_Type( ATSUTextLayout aTextLayout, UniCharArrayOffset nLineOffset, UniCharCount nLineLength, ATSUTextMeasurement fLocationX, ATSUTextMeasurement fLocationY, Rect *pTextImageRect );
typedef OSStatus ATSUSetAttributes_Type( ATSUStyle aStyle, ItemCount nAttributeCount, const ATSUAttributeTag aTag[], const ByteCount aValueSize[], const ATSUAttributeValuePtr aValue[] );
typedef OSStatus ATSUSetFontFeatures_Type( ATSUStyle aStyle, ItemCount nFeatureCount, const ATSUFontFeatureType aType[], const ATSUFontFeatureSelector aSelector[] );
typedef OSStatus ATSUSetLayoutControls_Type( ATSUTextLayout aTextLayout, ItemCount nAttributeCount, const ATSUAttributeTag aTag[], const ByteCount aValueSize[], const ATSUAttributeValuePtr aValue[] );
typedef OSStatus ATSUSetObjFontFallbacks_Type( ATSUFontFallbacks aFontFallbacks, ItemCount nFontFallbacksCount, const ATSUFontID aFonts[], ATSUFontFallbackMethod nFontFallbackMethod );
typedef OSStatus ATSUSetRunStyle_Type( ATSUTextLayout aTextLayout, ATSUStyle aStyle, UniCharArrayOffset nRunStart, UniCharCount nRunLength );
typedef ATSFontRef FMGetATSFontRefFromFont_Type( FMFont nFont );
typedef FMFont FMGetFontFromATSFontRef_Type( ATSFontRef aFont );

static bool	bATSUIInitialized = false;
static ATSUCalculateBaselineDeltas_Type *pATSUCalculateBaselineDeltas = NULL;
static ATSUCreateAndCopyStyle_Type *pATSUCreateAndCopyStyle = NULL;
static ATSUCreateFontFallbacks_Type *pATSUCreateFontFallbacks = NULL;
static ATSUCreateStyle_Type *pATSUCreateStyle = NULL;
static ATSUCreateTextLayoutWithTextPtr_Type *pATSUCreateTextLayoutWithTextPtr = NULL;
static ATSUDirectGetLayoutDataArrayPtrFromTextLayout_Type *pATSUDirectGetLayoutDataArrayPtrFromTextLayout = NULL;
static ATSUDirectReleaseLayoutDataArrayPtr_Type *pATSUDirectReleaseLayoutDataArrayPtr = NULL;
static ATSUDisposeFontFallbacks_Type *pATSUDisposeFontFallbacks = NULL;
static ATSUDisposeStyle_Type *pATSUDisposeStyle = NULL;
static ATSUDisposeTextLayout_Type *pATSUDisposeTextLayout = NULL;
static ATSUGetGlyphBounds_Type *pATSUGetGlyphBounds = NULL;
static ATSUGetRunStyle_Type *pATSUGetRunStyle = NULL;
static ATSUGlyphGetCubicPaths_Type *pATSUGlyphGetCubicPaths = NULL;
static ATSUGlyphGetIdealMetrics_Type *pATSUGlyphGetIdealMetrics = NULL;
static ATSUGlyphGetScreenMetrics_Type *pATSUGlyphGetScreenMetrics = NULL;
static ATSUMatchFontsToText_Type *pATSUMatchFontsToText = NULL;
static ATSUMeasureTextImage_Type *pATSUMeasureTextImage = NULL;
static ATSUSetAttributes_Type *pATSUSetAttributes = NULL;
static ATSUSetFontFeatures_Type *pATSUSetFontFeatures = NULL;
static ATSUSetLayoutControls_Type *pATSUSetLayoutControls = NULL;
static ATSUSetObjFontFallbacks_Type *pATSUSetObjFontFallbacks = NULL;
static ATSUSetRunStyle_Type *pATSUSetRunStyle = NULL;
static FMGetATSFontRefFromFont_Type *pFMGetATSFontRefFromFont = NULL;
static FMGetFontFromATSFontRef_Type *pFMGetFontFromATSFontRef = NULL;

static const String aGeezaPro( RTL_CONSTASCII_USTRINGPARAM( "Geeza Pro" ) );
static const String aHelvetica( RTL_CONSTASCII_USTRINGPARAM( "Helvetica" ) );
static const String aHiraginoKakuGothicProW3( RTL_CONSTASCII_USTRINGPARAM( "Hiragino Kaku Gothic Pro W3" ) );
static const String aHiraginoMinchoProW3( RTL_CONSTASCII_USTRINGPARAM( "Hiragino Mincho Pro W3" ) );
static const String aOpenSymbol( RTL_CONSTASCII_USTRINGPARAM( "OpenSymbol" ) );
static const String aTimesNewRoman( RTL_CONSTASCII_USTRINGPARAM( "Times New Roman" ) );
static const String aTimesRoman( RTL_CONSTASCII_USTRINGPARAM( "Times Roman" ) );

inline long Float32ToLong( Float32 f ) { return (long)( f + 0.5 ); }

inline bool IsNonprintingChar( sal_Unicode nChar ) { return ( nChar == 0x00b6 || nChar == 0x00b7 ); }

struct ImplATSLayoutDataHash {
	int					mnLen;
#ifdef USE_CORETEXT_TEXT_RENDERING
	ATSFontRef			mnFontID;
#else	// USE_CORETEXT_TEXT_RENDERING
	ATSUFontID			mnFontID;
#endif	// USE_CORETEXT_TEXT_RENDERING
	float				mfFontSize;
	double				mfFontScaleX;
	bool				mbAntialiased;
	bool				mbRTL;
	bool				mbVertical;
	sal_Unicode*		mpStr;
	sal_Int32			mnStrHash;
	bool				mbOwnsStr;

						ImplATSLayoutDataHash( const sal_Unicode *pStr, int nLen, int nMinCharPos, int nEndCharPos, int nFlags, ::vcl::com_sun_star_vcl_VCLFont *pVCLFont );
						~ImplATSLayoutDataHash();
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
#ifdef USE_CORETEXT_TEXT_RENDERING
	CTFontRef			maFont;
	CTTypesetterRef		maTypesetter;
	CTLineRef			maLine;
	CGGlyph*			mpGlyphs;
	int*				mpGlyphsToChars;
#endif	// USE_CORETEXT_TEXT_RENDERING
	ATSUTextLayout		maLayout;
#ifdef USE_CORETEXT_TEXT_RENDERING
	CFIndex				mnGlyphCount;
#else	// USE_CORETEXT_TEXT_RENDERING
	ItemCount			mnGlyphCount;
	ATSLayoutRecord*	mpGlyphDataArray;
#endif	// USE_CORETEXT_TEXT_RENDERING
	int*				mpCharsToGlyphs;
	int*				mpCharsToChars;
	long*				mpGlyphAdvances;
	ATSUStyle			maVerticalFontStyle;
	long				mnBaselineDelta;
	bool				mbValid;
	bool				mbGlyphBounds;
	Rectangle			maGlyphBounds;
	::std::hash_map< GlyphID, Point >	maVerticalGlyphTranslations;
	::std::hash_map< GlyphID, long >	maNativeGlyphWidths;

#ifndef USE_CORETEXT_TEXT_RENDERING
	static ATSFontRef			GetATSFontRefFromNativeFont( sal_IntPtr nFont );
	static ATSUFontID			GetNativeFontFromATSFontRef( ATSFontRef aFont );
#endif	// !USE_CORETEXT_TEXT_RENDERING
	static void					ClearLayoutDataCache();
	static void					SetFontFallbacks();
	static ImplATSLayoutData*	GetLayoutData( const sal_Unicode *pStr, int nLen, int nMinCharPos, int nEndCharPos, int nFlags, int nFallbackLevel, ::vcl::com_sun_star_vcl_VCLFont *pVCLFont, const SalATSLayout *pCurrentLayout );

						ImplATSLayoutData( ImplATSLayoutDataHash *pLayoutHash, int nFallbackLevel, ::vcl::com_sun_star_vcl_VCLFont *pVCLFont, const SalATSLayout *pCurrentLayout );
						~ImplATSLayoutData();

	void				Destroy();
	const Rectangle&	GetGlyphBounds();
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

static bool ATSUIInitialize()
{
	if ( !bATSUIInitialized )
	{
		void *pLib = dlopen( NULL, RTLD_LAZY | RTLD_LOCAL );
		if ( pLib )
		{
			pATSUCalculateBaselineDeltas = (ATSUCalculateBaselineDeltas_Type *)dlsym( pLib, "ATSUCalculateBaselineDeltas" );
			pATSUCreateAndCopyStyle = (ATSUCreateAndCopyStyle_Type *)dlsym( pLib, "ATSUCreateAndCopyStyle" );
			pATSUCreateFontFallbacks = (ATSUCreateFontFallbacks_Type *)dlsym( pLib, "ATSUCreateFontFallbacks" );
			pATSUCreateStyle = (ATSUCreateStyle_Type *)dlsym( pLib, "ATSUCreateStyle" );
			pATSUCreateTextLayoutWithTextPtr = (ATSUCreateTextLayoutWithTextPtr_Type *)dlsym( pLib, "ATSUCreateTextLayoutWithTextPtr" );
			pATSUDirectGetLayoutDataArrayPtrFromTextLayout = (ATSUDirectGetLayoutDataArrayPtrFromTextLayout_Type *)dlsym( pLib, "ATSUDirectGetLayoutDataArrayPtrFromTextLayout" );
			pATSUDirectReleaseLayoutDataArrayPtr = (ATSUDirectReleaseLayoutDataArrayPtr_Type *)dlsym( pLib, "ATSUDirectReleaseLayoutDataArrayPtr" );
			pATSUDisposeFontFallbacks = (ATSUDisposeFontFallbacks_Type *)dlsym( pLib, "ATSUDisposeFontFallbacks" );
			pATSUDisposeStyle = (ATSUDisposeStyle_Type *)dlsym( pLib, "ATSUDisposeStyle" );
			pATSUDisposeTextLayout = (ATSUDisposeTextLayout_Type *)dlsym( pLib, "ATSUDisposeTextLayout" );
			pATSUGetGlyphBounds = (ATSUGetGlyphBounds_Type *)dlsym( pLib, "ATSUGetGlyphBounds" );
			pATSUGetRunStyle = (ATSUGetRunStyle_Type *)dlsym( pLib, "ATSUGetRunStyle" );
			pATSUGlyphGetCubicPaths = (ATSUGlyphGetCubicPaths_Type *)dlsym( pLib, "ATSUGlyphGetCubicPaths" );
			pATSUGlyphGetIdealMetrics = (ATSUGlyphGetIdealMetrics_Type *)dlsym( pLib, "ATSUGlyphGetIdealMetrics" );
			pATSUGlyphGetScreenMetrics = (ATSUGlyphGetScreenMetrics_Type *)dlsym( pLib, "ATSUGlyphGetScreenMetrics" );
			pATSUMatchFontsToText = (ATSUMatchFontsToText_Type *)dlsym( pLib, "ATSUMatchFontsToText" );
			pATSUMeasureTextImage = (ATSUMeasureTextImage_Type *)dlsym( pLib, "ATSUMeasureTextImage" );
			pATSUSetAttributes = (ATSUSetAttributes_Type *)dlsym( pLib, "ATSUSetAttributes" );
			pATSUSetFontFeatures = (ATSUSetFontFeatures_Type *)dlsym( pLib, "ATSUSetFontFeatures" );
			pATSUSetLayoutControls = (ATSUSetLayoutControls_Type *)dlsym( pLib, "ATSUSetLayoutControls" );
			pATSUSetObjFontFallbacks = (ATSUSetObjFontFallbacks_Type *)dlsym( pLib, "ATSUSetObjFontFallbacks" );
			pATSUSetRunStyle = (ATSUSetRunStyle_Type *)dlsym( pLib, "ATSUSetRunStyle" );
			pFMGetATSFontRefFromFont = (FMGetATSFontRefFromFont_Type *)dlsym( pLib, "FMGetATSFontRefFromFont" );
			pFMGetFontFromATSFontRef = (FMGetFontFromATSFontRef_Type *)dlsym( pLib, "FMGetFontFromATSFontRef" );

			dlclose( pLib );
		}

#ifdef DEBUG
		fprintf( stderr, "pATSUCalculateBaselineDeltas: %p\n", pATSUCalculateBaselineDeltas );
		fprintf( stderr, "pATSUCreateAndCopyStyle: %p\n", pATSUCreateAndCopyStyle );
		fprintf( stderr, "pATSUCreateFontFallbacks: %p\n", pATSUCreateFontFallbacks );
		fprintf( stderr, "pATSUCreateStyle: %p\n", pATSUCreateStyle );
		fprintf( stderr, "pATSUCreateTextLayoutWithTextPtr: %p\n", pATSUCreateTextLayoutWithTextPtr );
		fprintf( stderr, "pATSUDirectGetLayoutDataArrayPtrFromTextLayout: %p\n", pATSUDirectGetLayoutDataArrayPtrFromTextLayout );
		fprintf( stderr, "pATSUDirectReleaseLayoutDataArrayPtr: %p\n", pATSUDirectReleaseLayoutDataArrayPtr );
		fprintf( stderr, "pATSUDisposeFontFallbacks: %p\n", pATSUDisposeFontFallbacks );
		fprintf( stderr, "pATSUDisposeStyle: %p\n", pATSUDisposeStyle );
		fprintf( stderr, "pATSUDisposeTextLayout: %p\n", pATSUDisposeTextLayout );
		fprintf( stderr, "pATSUGetGlyphBounds: %p\n", pATSUGetGlyphBounds );
		fprintf( stderr, "pATSUGetRunStyle: %p\n", pATSUGetRunStyle );
		fprintf( stderr, "pATSUGlyphGetCubicPaths: %p\n", pATSUGlyphGetCubicPaths );
		fprintf( stderr, "pATSUGlyphGetIdealMetrics: %p\n", pATSUGlyphGetIdealMetrics );
		fprintf( stderr, "pATSUGlyphGetScreenMetrics: %p\n", pATSUGlyphGetScreenMetrics );
		fprintf( stderr, "pATSUMatchFontsToText: %p\n", pATSUMatchFontsToText );
		fprintf( stderr, "pATSUMeasureTextImage: %p\n", pATSUMeasureTextImage );
		fprintf( stderr, "pATSUSetAttributes: %p\n", pATSUSetAttributes );
		fprintf( stderr, "pATSUSetFontFeatures: %p\n", pATSUSetFontFeatures );
		fprintf( stderr, "pATSUSetLayoutControls: %p\n", pATSUSetLayoutControls );
		fprintf( stderr, "pATSUSetObjFontFallbacks: %p\n", pATSUSetObjFontFallbacks );
		fprintf( stderr, "pATSUSetRunStyle: %p\n", pATSUSetRunStyle );
		fprintf( stderr, "pFMGetATSFontRefFromFont: %p\n", pFMGetATSFontRefFromFont );
		fprintf( stderr, "pFMGetFontFromATSFontRef: %p\n", pFMGetFontFromATSFontRef );
#endif	// DEBUG

		bATSUIInitialized = true;
	}

	return ( pATSUCalculateBaselineDeltas && pATSUCreateAndCopyStyle && pATSUCreateFontFallbacks && pATSUCreateStyle && pATSUCreateTextLayoutWithTextPtr && pATSUDirectGetLayoutDataArrayPtrFromTextLayout && pATSUDirectReleaseLayoutDataArrayPtr && pATSUDisposeFontFallbacks && pATSUDisposeStyle && pATSUDisposeTextLayout && pATSUGetGlyphBounds && pATSUGetRunStyle && pATSUGlyphGetCubicPaths && pATSUGlyphGetIdealMetrics && pATSUGlyphGetScreenMetrics && pATSUMatchFontsToText && pATSUMeasureTextImage && pATSUSetAttributes && pATSUSetFontFeatures && pATSUSetLayoutControls && pATSUSetObjFontFallbacks && pATSUSetRunStyle && pFMGetATSFontRefFromFont && pFMGetFontFromATSFontRef );
}

// ============================================================================

ImplATSLayoutDataHash::ImplATSLayoutDataHash( const sal_Unicode *pStr, int nLen, int nMinCharPos, int nEndCharPos, int nFlags, com_sun_star_vcl_VCLFont *pVCLFont ) :
	mnLen( nEndCharPos - nMinCharPos ),
#ifdef USE_CORETEXT_TEXT_RENDERING
	mnFontID( (ATSFontRef)pVCLFont->getNativeFont() ),
#else	// USE_CORETEXT_TEXT_RENDERING
	mnFontID( (ATSUFontID)pVCLFont->getNativeFont() ),
#endif	// USE_CORETEXT_TEXT_RENDERING
	mfFontSize( pVCLFont->getSize() ),
	mfFontScaleX( pVCLFont->getScaleX() ),
	mbAntialiased( pVCLFont->isAntialiased() ),
	mbRTL( nFlags & SAL_LAYOUT_BIDI_RTL ),
	mbVertical( nFlags & SAL_LAYOUT_VERTICAL ),
	mpStr( (sal_Unicode *)( pStr + nMinCharPos ) ),
	mnStrHash( rtl_ustr_hashCode_WithLength( mpStr, mnLen ) ),
	mbOwnsStr( false )
{
}

// ----------------------------------------------------------------------------

ImplATSLayoutDataHash::~ImplATSLayoutDataHash()
{
	if ( mbOwnsStr && mpStr )
		rtl_freeMemory( mpStr );
}

// ============================================================================

bool ImplATSLayoutDataHashEquality::operator()( const ImplATSLayoutDataHash *p1, const ImplATSLayoutDataHash *p2 ) const
{
	return ( p1->mnLen == p2->mnLen &&
		p1->mnFontID == p2->mnFontID &&
		p1->mfFontSize == p2->mfFontSize &&
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

#ifndef USE_CORETEXT_TEXT_RENDERING

ATSFontRef ImplATSLayoutData::GetATSFontRefFromNativeFont( sal_IntPtr nFont )
{
	if ( ATSUIInitialize() )
		return pFMGetATSFontRefFromFont( (ATSUFontID)nFont );
	else
		return NULL;
}

// ----------------------------------------------------------------------------

ATSUFontID ImplATSLayoutData::GetNativeFontFromATSFontRef( ATSFontRef aFont )
{
	if ( ATSUIInitialize() )
		return (ATSUFontID)pFMGetFontFromATSFontRef( aFont );
	else
		return kATSUInvalidFontID;
}

#endif	// !USE_CORETEXT_TEXT_RENDERING

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
		if ( ATSUIInitialize() )
			pATSUDisposeFontFallbacks( maFontFallbacks );
		maFontFallbacks = NULL;
	}
}

// ----------------------------------------------------------------------------

void ImplATSLayoutData::SetFontFallbacks()
{
	if ( maFontFallbacks )
	{
		if ( ATSUIInitialize() )
			pATSUDisposeFontFallbacks( maFontFallbacks );
		maFontFallbacks = NULL;
	}

	// Initialize font fallbacks list if necessary
	if ( ATSUIInitialize() && pATSUCreateFontFallbacks( &maFontFallbacks ) == noErr )
	{
		SalData *pSalData = GetSalData();
		ItemCount nCount = pSalData->maNativeFontMapping.size();
		ItemCount nActualCount = 0;

		ATSUFontID aATSUFonts[ nCount ];
		for ( ::std::hash_map< sal_IntPtr, JavaImplFontData* >::const_iterator it = pSalData->maNativeFontMapping.begin(); it != pSalData->maNativeFontMapping.end(); ++it )
			aATSUFonts[ nActualCount++ ] = it->first;

		if ( !nActualCount || pATSUSetObjFontFallbacks( maFontFallbacks, nActualCount, aATSUFonts, kATSUSequentialFallbacksExclusive ) != noErr )
		{
			if ( maFontFallbacks )
			{
				pATSUDisposeFontFallbacks( maFontFallbacks );
				maFontFallbacks = NULL;
			}
		}
	}
}

// ----------------------------------------------------------------------------

ImplATSLayoutData *ImplATSLayoutData::GetLayoutData( const sal_Unicode *pStr, int nLen, int nMinCharPos, int nEndCharPos, int nFlags, int nFallbackLevel, com_sun_star_vcl_VCLFont *pVCLFont, const SalATSLayout *pCurrentLayout )
{
	ImplATSLayoutData *pLayoutData = NULL;

	if ( !ATSUIInitialize() )
		return pLayoutData;

	ImplATSLayoutDataHash *pLayoutHash = new ImplATSLayoutDataHash( pStr, nLen, nMinCharPos, nEndCharPos, nFlags, pVCLFont );

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
		pLayoutHash->mbOwnsStr = true;
		pLayoutHash->mpStr = (sal_Unicode *)rtl_allocateMemory( pLayoutHash->mnLen * sizeof( sal_Unicode ) );
		memcpy( pLayoutHash->mpStr, pStr + nMinCharPos, pLayoutHash->mnLen * sizeof( sal_Unicode ) );
		pLayoutData = new ImplATSLayoutData( pLayoutHash, nFallbackLevel, pVCLFont, pCurrentLayout );

		if ( !pLayoutData )
		{
			delete pLayoutHash;
			return NULL;
		}

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
#ifdef USE_CORETEXT_TEXT_RENDERING
	maFont( NULL ),
	maTypesetter( NULL ),
	maLine( NULL ),
	mpGlyphs( NULL ),
#endif	// USE_CORETEXT_TEXT_RENDERING
	maLayout( NULL ),
	mnGlyphCount( 0 ),
#ifndef USE_CORETEXT_TEXT_RENDERING
	mpGlyphDataArray( NULL ),
#endif	// !USE_CORETEXT_TEXT_RENDERING
	mpCharsToGlyphs( NULL ),
	mpCharsToChars( NULL ),
	mpGlyphAdvances( NULL ),
	maVerticalFontStyle( NULL ),
	mnBaselineDelta( 0 ),
	mbValid( false ),
	mbGlyphBounds( false )
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
	if ( pATSUCreateStyle( &maFontStyle ) != noErr )
	{
		Destroy();
		return;
	}

	// Fix bug 1595 by allowing rare ligatures
	ATSUFontFeatureType aType;
	ATSUFontFeatureSelector aSelector;
	aType = kDiacriticsType;
	aSelector = kDecomposeDiacriticsSelector;
	if ( pATSUSetFontFeatures( maFontStyle, 1, &aType, &aSelector ) != noErr )
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
	float fSize = mpHash->mfFontSize;
	float fAdjustedSize;
	if ( (long)( mpHash->mfFontSize * mpHash->mnLen * 4 ) > 0x00007fff )
		fAdjustedSize = (float)( 0x00007fff / ( mpHash->mnLen * 4 ) );
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

	ATSUVerticalCharacterType nHorizontal = kATSUStronglyHorizontal;
	nTags[3] = kATSUVerticalCharacterTag;
	nBytes[3] = sizeof( ATSUVerticalCharacterType );
	nVals[3] = &nHorizontal;

	if ( pATSUSetAttributes( maFontStyle, 4, nTags, nBytes, nVals ) != noErr )
	{
		Destroy();
		return;
	}

	if ( mpHash->mbVertical )
	{
		if ( pATSUCreateAndCopyStyle( maFontStyle, &maVerticalFontStyle ) == noErr && maVerticalFontStyle )
		{
			ATSUVerticalCharacterType nVertical = kATSUStronglyVertical;
			nTags[0] = kATSUVerticalCharacterTag;
			nBytes[0] = sizeof( ATSUVerticalCharacterType );
			nVals[0] = &nVertical;

			if ( pATSUSetAttributes( maVerticalFontStyle, 1, nTags, nBytes, nVals ) != noErr )
			{
				Destroy();
				return ;
			}
		}
	}

#ifdef USE_CORETEXT_TEXT_RENDERING
	maFont = CTFontCreateWithPlatformFont( mpHash->mnFontID, mpHash->mfFontSize, NULL, NULL );
	if ( !maFont )
	{
		Destroy();
		return;
	}

	CFStringRef aString =  CFStringCreateWithCharactersNoCopy( NULL, mpHash->mpStr, mpHash->mnLen, kCFAllocatorNull );
	if ( !aString )
	{
		Destroy();
		return;
	}

	CFAttributedStringRef aAttrString = CFAttributedStringCreate( NULL, aString, NULL );
	CFRelease( aString );
	if ( !aAttrString )
	{
		Destroy();
		return;
	}

	CFMutableAttributedStringRef aMutableAttrString = CFAttributedStringCreateMutableCopy( NULL, 0, aAttrString );
	CFRelease( aAttrString );
	if ( !aMutableAttrString )
	{
		Destroy();
		return;
	}

	CFAttributedStringSetAttribute( aMutableAttrString, CFRangeMake( 0, mpHash->mnLen ), kCTFontAttributeName, maFont );

	CTParagraphStyleSetting aSetting;
	CTWritingDirection nCTDirection;
	if ( mpHash->mbRTL )
		nCTDirection = kCTWritingDirectionRightToLeft;
	else
		nCTDirection = kCTWritingDirectionLeftToRight;
	aSetting.spec = kCTParagraphStyleSpecifierBaseWritingDirection;
	aSetting.valueSize = sizeof( CTWritingDirection );
	aSetting.value = &nCTDirection;

	CTParagraphStyleRef aParaStyle = CTParagraphStyleCreate( &aSetting, 1 );
	if ( !aParaStyle )
	{
		CFRelease( aMutableAttrString );
		Destroy();
		return;
	}

	CFAttributedStringSetAttribute( aMutableAttrString, CFRangeMake( 0, mpHash->mnLen ), kCTParagraphStyleAttributeName, aParaStyle );
	CFRelease( aParaStyle );

	if ( mpHash->mbVertical )
	{
		for ( int i = 0; i < mpHash->mnLen; i++ )
		{
			if ( GetVerticalFlags( mpHash->mpStr[ i ] ) & GF_ROTMASK )
				CFAttributedStringSetAttribute( aMutableAttrString, CFRangeMake( i, 1 ), kCTVerticalFormsAttributeName, kCFBooleanTrue );
		}

		float fAscent = fabs( CTFontGetAscent( maFont ) );
		float fDescent = fabs( CTFontGetDescent( maFont ) );
		mnBaselineDelta = Float32ToLong( ( ( ( fAscent + fDescent ) / 2 ) - fDescent ) * UNITS_PER_PIXEL );
	}

	maTypesetter = CTTypesetterCreateWithAttributedString ( aMutableAttrString );
	CFRelease( aMutableAttrString );
	if ( !maTypesetter )
	{
		Destroy();
		return;
	}

	CTLineRef maLine = CTTypesetterCreateLine( maTypesetter, CFRangeMake( 0, 0 ) );
	if ( !maLine )
	{
		Destroy();
		return;
	}

	CFArrayRef aLineGlyphRuns = CTLineGetGlyphRuns( maLine );
	if ( !aLineGlyphRuns )
	{
		Destroy();
		return;
	}
#endif	// USE_CORETEXT_TEXT_RENDERING

	if ( pATSUCreateTextLayoutWithTextPtr( mpHash->mpStr, kATSUFromTextBeginning, kATSUToTextEnd, mpHash->mnLen, 1, (const UniCharCount *)&mpHash->mnLen, &maFontStyle, &maLayout ) != noErr )
	{
		Destroy();
		return;
	}

	if ( maVerticalFontStyle )
	{
		for ( int i = 0; i < mpHash->mnLen; i++ )
		{
			if ( GetVerticalFlags( mpHash->mpStr[ i ] ) & GF_ROTMASK && pATSUSetRunStyle( maLayout, maVerticalFontStyle, i, 1 ) != noErr )
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

	if ( pATSUSetLayoutControls( maLayout, 2, nTags, nBytes, nVals ) != noErr )
	{
		Destroy();
		return;
	}

#ifdef USE_CORETEXT_TEXT_RENDERING
	CFIndex nLineGlyphRuns = CFArrayGetCount( aLineGlyphRuns );
	CFIndex nGlyphsProcessed = 0;
	CFIndex nCurrentGlyphRun;
	CFIndex nBufSize;
	for ( nCurrentGlyphRun = 0; nCurrentGlyphRun < nLineGlyphRuns; nCurrentGlyphRun++ )
	{
		CTRunRef aGlyphRun = (CTRunRef)CFArrayGetValueAtIndex( aLineGlyphRuns, nCurrentGlyphRun );
		if ( aGlyphRun )
			nGlyphsProcessed += CTRunGetGlyphCount( aGlyphRun );
	}

	mnGlyphCount = nGlyphsProcessed;
#else	// USE_CORETEXT_TEXT_RENDERING
	// Fix bug 2919 by still producing a valid text layout even if no glyphs
	// can be retrieved. Also, fix bug 3063 by not holding onto the
	// ATSLayoutRecord and, instead, making our own private copy.
	ByteCount nBufSize;
	ATSLayoutRecord *pGlyphDataArray = NULL;
	pATSUDirectGetLayoutDataArrayPtrFromTextLayout( maLayout, 0, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent, (void **)&pGlyphDataArray, &mnGlyphCount );
	if ( pGlyphDataArray )
	{
		if ( mnGlyphCount )
		{
			nBufSize = mnGlyphCount * sizeof( ATSLayoutRecord );
			mpGlyphDataArray = (ATSLayoutRecord *)rtl_allocateMemory( nBufSize );
			if ( mpGlyphDataArray )
				memcpy( mpGlyphDataArray, pGlyphDataArray, nBufSize );

			pATSUDirectReleaseLayoutDataArrayPtr( NULL, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent, (void **)&pGlyphDataArray );
		}
	}
#endif	// USE_CORETEXT_TEXT_RENDERING

	// Cache mapping of characters to glyphs
	nBufSize = mpHash->mnLen * sizeof( int );
	mpCharsToGlyphs = (int *)rtl_allocateMemory( nBufSize );

	CFIndex i;
	for ( i = 0; i < mpHash->mnLen; i++ )
		mpCharsToGlyphs[ i ] = -1;
#ifdef USE_CORETEXT_TEXT_RENDERING
	nBufSize = mnGlyphCount * sizeof( CGGlyph );
	mpGlyphs = (CGGlyph *)rtl_allocateMemory( nBufSize );
	memset( mpGlyphs, 0, nBufSize );

	nBufSize = mnGlyphCount * sizeof( long );
	mpGlyphsToChars = (int *)rtl_allocateMemory( nBufSize );
	for ( i = 0; i < mnGlyphCount; i++ )
		mpGlyphsToChars[ i ] = -1;

	nGlyphsProcessed = 0;
	for ( nCurrentGlyphRun = 0; nCurrentGlyphRun < nLineGlyphRuns; nCurrentGlyphRun++ )
	{
		CTRunRef aGlyphRun = (CTRunRef)CFArrayGetValueAtIndex( aLineGlyphRuns, nCurrentGlyphRun );
		if ( aGlyphRun )
		{
			CFIndex nGlyphRunCount = CTRunGetGlyphCount( aGlyphRun );
			CFRange aRange = CTRunGetStringRange( aGlyphRun );
			if ( nGlyphRunCount && aRange.location != kCFNotFound && aRange.length > 0 )
			{
				CFIndex aIndices[ nGlyphRunCount ];
				CGGlyph aGlyphs[ nGlyphRunCount ];
				CTRunGetStringIndices( aGlyphRun, CFRangeMake( 0, 0 ), aIndices );
				CTRunGetGlyphs( aGlyphRun, CFRangeMake( 0, 0 ), aGlyphs );
				i = nGlyphsProcessed;
				CFIndex j = 0;
				for ( ; j < nGlyphRunCount; i++, j++ )
				{
					CFIndex nIndex = aIndices[ j ];
					mpGlyphs[ i ] = aGlyphs[ j ];
					mpGlyphsToChars[ i ] = nIndex;
					if ( mpCharsToGlyphs[ nIndex ] < 0 || i < mpCharsToGlyphs[ nIndex ] )
						mpCharsToGlyphs[ nIndex ] = i;
				}

				nGlyphsProcessed += nGlyphRunCount;
			}
		}
	}
#else	// USE_CORETEXT_TEXT_RENDERING
	for ( i = 0; i < (int)mnGlyphCount && mpGlyphDataArray; i++ )
	{
		int nIndex = mpGlyphDataArray[ i ].originalOffset / 2;
		if ( mpCharsToGlyphs[ nIndex ] < 0 || i < mpCharsToGlyphs[ nIndex ] )
			mpCharsToGlyphs[ nIndex ] = i;
	}
#endif	// USE_CORETEXT_TEXT_RENDERING

	// Cache mapping of characters to glyph character indices
	nBufSize = mpHash->mnLen * sizeof( int );
	mpCharsToChars = (int *)rtl_allocateMemory( nBufSize );

	for ( i = 0; i < mpHash->mnLen; i++ )
		mpCharsToChars[ i ] = -1;
#ifdef USE_CORETEXT_TEXT_RENDERING
	for ( i = 0; i < mpHash->mnLen; i++ )
	{
		int nIndex = mpCharsToGlyphs[ i ];
		if ( nIndex >= 0 )
			mpCharsToChars[ i ] = mpGlyphsToChars[ nIndex ];
	}
#else	// USE_CORETEXT_TEXT_RENDERING
	if ( mpHash->mbRTL )
	{
		i = 0;
		for ( int j = mpHash->mnLen - 1; j >= 0 && i < (int)mnGlyphCount && mpGlyphDataArray; j-- )
		{
			unsigned int nIndex = mpGlyphDataArray[ i ].originalOffset / 2;
			mpCharsToChars[ j ] = nIndex;
			for ( ; i < (int)mnGlyphCount && ( mpGlyphDataArray[ i ].originalOffset / 2 ) == nIndex; i++ )
				;
		}
	}
	else
	{
		i = 0;
		for ( int j = 0; j < mpHash->mnLen && i < (int)mnGlyphCount && mpGlyphDataArray; j++ )
		{
			unsigned int nIndex = mpGlyphDataArray[ i ].originalOffset / 2;
			mpCharsToChars[ j ] = nIndex;
			for ( ; i < (int)mnGlyphCount && ( mpGlyphDataArray[ i ].originalOffset / 2 ) == nIndex; i++ )
				;
		}
	}
#endif	// USE_CORETEXT_TEXT_RENDERING

	// Cache glyph widths
	nBufSize = mnGlyphCount * sizeof( long );
	mpGlyphAdvances = (long *)rtl_allocateMemory( nBufSize );
	memset( mpGlyphAdvances, 0, nBufSize );

	// Fix bug 448 by eliminating subpixel advances.
	mfFontScaleY = fSize / fAdjustedSize;
	int nLastNonSpacingIndex = -1;
	int nLastNonSpacingGlyph = -1;
#ifdef USE_CORETEXT_TEXT_RENDERING
	nGlyphsProcessed = 0;
	for ( nCurrentGlyphRun = 0; nCurrentGlyphRun < nLineGlyphRuns; nCurrentGlyphRun++ )
	{
		CTRunRef aGlyphRun = (CTRunRef)CFArrayGetValueAtIndex( aLineGlyphRuns, nCurrentGlyphRun );
		if ( aGlyphRun )
		{
			CFIndex nGlyphRunCount = CTRunGetGlyphCount( aGlyphRun );
			CFRange aRange = CTRunGetStringRange( aGlyphRun );
			if ( nGlyphRunCount && aRange.location != kCFNotFound && aRange.length > 0 )
			{
				// Determine if this is a vertical run
				bool bVerticalRun = false;
				CFDictionaryRef aDict = CTRunGetAttributes( aGlyphRun );
				if ( aDict )
				{
					const CFBooleanRef aValue = (const CFBooleanRef)CFDictionaryGetValue( aDict, kCTVerticalFormsAttributeName );
					if ( aValue == kCFBooleanTrue )
						bVerticalRun = true;
				}

				CFIndex aIndices[ nGlyphRunCount ];
				CGPoint aPositions[ nGlyphRunCount ];
				CGGlyph aGlyphs[ nGlyphRunCount ];
				CTRunGetStringIndices( aGlyphRun, CFRangeMake( 0, 0 ), aIndices );
				CTRunGetPositions( aGlyphRun, CFRangeMake( 0, 0 ), aPositions );
				CTRunGetGlyphs( aGlyphRun, CFRangeMake( 0, 0 ), aGlyphs );
				i = nGlyphsProcessed;
				CFIndex j = 0;
				for ( ; j < nGlyphRunCount; i++, j++ )
				{
					CFIndex nIndex = aIndices[ j ];
					if ( j == nGlyphRunCount - 1 )
					{
						if ( bVerticalRun )
							mpGlyphAdvances[ i ] += Float32ToLong( CTFontGetAdvancesForGlyphs( maFont, kCTFontVerticalOrientation, &aGlyphs[ j ], NULL, 1 ) * UNITS_PER_PIXEL );
						else
							mpGlyphAdvances[ i ] += Float32ToLong( CTFontGetAdvancesForGlyphs( maFont, kCTFontHorizontalOrientation, &aGlyphs[ j ], NULL, 1 ) * UNITS_PER_PIXEL );
					}
					else
					{
						if ( bVerticalRun )
							mpGlyphAdvances[ i ] += Float32ToLong( ( aPositions[ j ].y - aPositions[ j + 1 ].y ) * mpHash->mfFontScaleX * UNITS_PER_PIXEL );
						else
							mpGlyphAdvances[ i ] += Float32ToLong( ( aPositions[ j + 1 ].x - aPositions[ j ].x ) * mpHash->mfFontScaleX * UNITS_PER_PIXEL );
					}
#else	// USE_CORETEXT_TEXT_RENDERING
	for ( i = 0; i < (int)mnGlyphCount && mpGlyphDataArray; i++ )
	{
		int nIndex = mpGlyphDataArray[ i ].originalOffset / 2;
		if ( i == (int)mnGlyphCount - 1 )
		{
			ATSTrapezoid aTrapezoid;
			if ( pATSUGetGlyphBounds( maLayout, 0, 0, i, 1, kATSUseFractionalOrigins, 1, &aTrapezoid, NULL ) == noErr )
				mpGlyphAdvances[ i ] += Float32ToLong( Fix2X( aTrapezoid.upperRight.x - aTrapezoid.upperLeft.x ) * mpHash->mfFontScaleX * mfFontScaleY * UNITS_PER_PIXEL );
		}
		else
		{
			mpGlyphAdvances[ i ] += Float32ToLong( Fix2X( mpGlyphDataArray[ i + 1 ].realPos - mpGlyphDataArray[ i ].realPos ) * mpHash->mfFontScaleX * mfFontScaleY * UNITS_PER_PIXEL );
		}
#endif	// USE_CORETEXT_TEXT_RENDERING

		// Make sure that ligature glyphs get all of the width and that their
		// attached spacing glyphs have zero width so that the OOo code will
		// force the cursor to the end of the ligature instead of the beginning.
		// Fix bug 3621 by treating negative width glyphs like ligatured glyphs.
		long nWidthAdjust = 0;
#ifdef USE_CORETEXT_TEXT_RENDERING
		if ( ( mpGlyphs[ i ] == 0xffff || mpGlyphAdvances[ i ] < 0 ) && !IsNonprintingChar( mpHash->mpStr[ nIndex ] ) && !pCurrentLayout->IsSpacingGlyph( mpHash->mpStr[ nIndex ] | GF_ISCHAR ) )
#else	// USE_CORETEXT_TEXT_RENDERING
		if ( ( mpGlyphDataArray[ i ].glyphID == 0xffff || mpGlyphAdvances[ i ] < 0 ) && !IsNonprintingChar( mpHash->mpStr[ nIndex ] ) && !pCurrentLayout->IsSpacingGlyph( mpHash->mpStr[ nIndex ] | GF_ISCHAR ) )
#endif	// USE_CORETEXT_TEXT_RENDERING
		{
			if ( nLastNonSpacingGlyph >= 0 && nLastNonSpacingGlyph != i && nLastNonSpacingIndex != nIndex )
			{
				mpGlyphAdvances[ nLastNonSpacingGlyph ] += mpGlyphAdvances[ i ];
				if ( mpGlyphAdvances[ nLastNonSpacingGlyph ] < 0 )
				{
					nWidthAdjust = mpGlyphAdvances[ nLastNonSpacingGlyph ];
					mpGlyphAdvances[ nLastNonSpacingGlyph ] = 0;
				}

				mpGlyphAdvances[ i ] = 0;
			}
		}
		else if ( IsNonprintingChar( mpHash->mpStr[ nIndex ] ) || pCurrentLayout->IsSpacingGlyph( mpHash->mpStr[ nIndex ] | GF_ISCHAR ) )
		{
			nLastNonSpacingIndex = -1;
			nLastNonSpacingGlyph = -1;
		}
		else
		{
			nLastNonSpacingIndex = nIndex;
			nLastNonSpacingGlyph = i;
		}

		if ( mpGlyphAdvances[ i ] < 0 )
		{
			nWidthAdjust += mpGlyphAdvances[ i ];
			mpGlyphAdvances[ i ] = 0;
		}
		else if ( nWidthAdjust && mpGlyphAdvances[ i ] > 0 )
		{
			mpGlyphAdvances[ i ] += nWidthAdjust;
			if ( mpGlyphAdvances[ i ] < 0 )
			{
				nWidthAdjust = mpGlyphAdvances[ i ];
				mpGlyphAdvances[ i ] = 0;
			}
			else
			{
				nWidthAdjust = 0;
			}
		}
#ifdef USE_CORETEXT_TEXT_RENDERING
				}

				nGlyphsProcessed += nGlyphRunCount;
			}
		}
#endif	// USE_CORETEXT_TEXT_RENDERING
	}

#ifndef USE_CORETEXT_TEXT_RENDERING
	if ( maVerticalFontStyle )
	{
		BslnBaselineRecord aBaseline;
		memset( aBaseline, 0, sizeof( BslnBaselineRecord ) );
		if ( pATSUCalculateBaselineDeltas( maVerticalFontStyle, kBSLNRomanBaseline, aBaseline ) == noErr )
			mnBaselineDelta = Float32ToLong( Fix2X( aBaseline[ kBSLNIdeographicCenterBaseline ] ) * mfFontScaleY * UNITS_PER_PIXEL );
		if ( !mnBaselineDelta )
		{
			ATSFontMetrics aFontMetrics;
			ATSFontRef aFont = mpHash->mnFontID;
			ATSFontRef aFont = ImplATSLayoutData::GetATSFontRefFromNativeFont( mpHash->mnFontID );
			if ( ATSFontGetHorizontalMetrics( aFont, kATSOptionFlagsDefault, &aFontMetrics ) == noErr )
				mnBaselineDelta = Float32ToLong( ( ( ( fabs( aFontMetrics.descent ) + fabs( aFontMetrics.ascent ) ) / 2 ) - fabs( aFontMetrics.descent ) ) * fSize * UNITS_PER_PIXEL );
		}
	}
#endif	// !USE_CORETEXT_TEXT_RENDERING

	// Find positions that require fallback fonts
	bool bUseFontFallbacksList = false;
	UniCharArrayOffset nCurrentPos = 0;
	UniCharCount nOffset;
	ATSUFontID nFontID;
	for ( ; ; )
	{
		bool bNeedFontFallbackList = false;
		OSStatus nErr = pATSUMatchFontsToText( maLayout, nCurrentPos, kATSUToTextEnd, &nFontID, &nCurrentPos, &nOffset );
		if ( nErr == kATSUFontsNotMatched )
		{
			bNeedFontFallbackList = true;
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

			UniCharArrayOffset nOffsetPos = nCurrentPos + nOffset;
			for ( ; nCurrentPos < nOffsetPos; nCurrentPos++ )
				mpNeedFallback[ nCurrentPos ] = true;

			// Update font for next pass through
			if ( !mpFallbackFont )
			{
				SalData *pSalData = GetSalData();
				::std::hash_map< sal_IntPtr, JavaImplFontData* >::const_iterator it = pSalData->maNativeFontMapping.find( (sal_IntPtr)nFontID );
				if ( it != pSalData->maNativeFontMapping.end() )
					mpFallbackFont = new com_sun_star_vcl_VCLFont( it->second->maVCLFontName, mpHash->mfFontSize, mpVCLFont->getOrientation(), mpHash->mbAntialiased, mpHash->mbVertical, mpHash->mfFontScaleX );
				else
					bNeedFontFallbackList = true;
			}
		}
		else
		{
			break;
		}

		// Fix problem rendering Cherokee characters without causing bug
		// bug 3031 to reoccur by setting the font fallbacks list and
		// restarting the matching process
		if ( bNeedFontFallbackList && !bUseFontFallbacksList && ImplATSLayoutData::maFontFallbacks )
		{
			nTags[0] = kATSULineFontFallbacksTag;
			nBytes[0] = sizeof( ATSUFontFallbacks );
			nVals[0] = &ImplATSLayoutData::maFontFallbacks;

			if ( pATSUSetLayoutControls( maLayout, 1, nTags, nBytes, nVals ) == noErr )
			{
				bUseFontFallbacksList = true;
				nCurrentPos = 0;
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
			}
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

const Rectangle& ImplATSLayoutData::GetGlyphBounds()
{
	if ( !mbGlyphBounds )
	{
		Rect aRect;
		if ( pATSUMeasureTextImage( maLayout, kATSUFromTextBeginning, kATSUToTextEnd, 0, 0, &aRect ) == noErr )
		{
			maGlyphBounds = Rectangle( Point( Float32ToLong( aRect.left * mpHash->mfFontScaleX * mfFontScaleY ), Float32ToLong( aRect.top * mfFontScaleY ) ), Size( Float32ToLong( ( aRect.right - aRect.left ) * mpHash->mfFontScaleX * mfFontScaleY ), Float32ToLong( ( aRect.bottom - aRect.top ) * mfFontScaleY ) ) );
			maGlyphBounds.Justify();
		}

		mbGlyphBounds = true;
	}

	return maGlyphBounds;
}

// ----------------------------------------------------------------------------

void ImplATSLayoutData::Destroy()
{
	if ( mpHash )
	{
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
		pATSUDisposeStyle( maFontStyle );
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

#ifdef USE_CORETEXT_TEXT_RENDERING
	if ( maFont )
	{
		CFRelease( maFont );
		maFont = NULL;
	}

	if ( maTypesetter )
	{
		CFRelease( maTypesetter );
		maTypesetter = NULL;
	}

	if ( maLine )
	{
		CFRelease( maLine );
		maLine = NULL;
	}

	if ( mpGlyphs )
	{
		rtl_freeMemory( mpGlyphs );
		mpGlyphs = NULL;
	}

	if ( mpGlyphsToChars )
	{
		rtl_freeMemory( mpGlyphsToChars );
		mpGlyphsToChars = NULL;
	}
#endif	// USE_CORETEXT_TEXT_RENDERING

	if ( maLayout )
	{
		pATSUDisposeTextLayout( maLayout );
		maLayout = NULL;
	}

	mnGlyphCount = 0;

	if ( mpCharsToChars )
	{
		rtl_freeMemory( mpCharsToChars );
		mpCharsToChars = NULL;
	}

#ifndef USE_CORETEXT_TEXT_RENDERING
	if ( mpGlyphDataArray )
	{
		rtl_freeMemory( mpGlyphDataArray );
		mpGlyphDataArray = NULL;
	}
#endif	// !USE_CORETEXT_TEXT_RENDERING

	if ( mpCharsToGlyphs )
	{
		rtl_freeMemory( mpCharsToGlyphs );
		mpCharsToGlyphs = NULL;
	}

	if ( mpGlyphAdvances )
	{
		rtl_freeMemory( mpGlyphAdvances );
		mpGlyphAdvances = NULL;
	}

	if ( maVerticalFontStyle )
	{
		pATSUDisposeStyle( maVerticalFontStyle );
		maVerticalFontStyle = NULL;
	}

	mnBaselineDelta = 0;
	mbValid = false;
	mbGlyphBounds = false;
	maGlyphBounds.SetEmpty();
	maVerticalGlyphTranslations.clear();
	maNativeGlyphWidths.clear();
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

	Point aPoint( Float32ToLong( pPoint->x * UNITS_PER_PIXEL ), Float32ToLong( pPoint->y * UNITS_PER_PIXEL ) );
	pPolygonList->push_back( Polygon( 1, &aPoint ) );

	return noErr;
}

// ----------------------------------------------------------------------------

static OSStatus SalATSCubicLineToCallback( const Float32Point *pPoint, void *pData )
{
	::std::list< Polygon > *pPolygonList = (::std::list< Polygon > *)pData;

	pPolygonList->back().Insert( pPolygonList->back().GetSize(), Point( Float32ToLong( pPoint->x * UNITS_PER_PIXEL ), Float32ToLong( pPoint->y * UNITS_PER_PIXEL ) ) );

	return noErr;
}

// ----------------------------------------------------------------------------

static OSStatus SalATSCubicCurveToCallback( const Float32Point *pStart, const Float32Point *pOffCurve, const Float32Point *pEnd, void *pData )
{
	::std::list< Polygon > *pPolygonList = (::std::list< Polygon > *)pData;

	Point aStart( Float32ToLong( pStart->x * UNITS_PER_PIXEL ), Float32ToLong( pStart->y * UNITS_PER_PIXEL ) );
	Point aOffCurve( Float32ToLong( pOffCurve->x * UNITS_PER_PIXEL ), Float32ToLong( pOffCurve->y * UNITS_PER_PIXEL ) );
	Point aEnd( Float32ToLong( pEnd->x * UNITS_PER_PIXEL ), Float32ToLong( pEnd->y * UNITS_PER_PIXEL ) );

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
	if ( nFallbackLevel && rArgs.mnFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING && ATSUIInitialize() )
		return NULL;
	else
		return new SalATSLayout( this, nFallbackLevel );
}

// ============================================================================

#ifndef USE_CORETEXT_TEXT_RENDERING

ATSFontRef SalATSLayout::GetATSFontRefFromNativeFont( sal_IntPtr nFont )
{
	return ImplATSLayoutData::GetATSFontRefFromNativeFont( nFont );
}

// ----------------------------------------------------------------------------

sal_IntPtr SalATSLayout::GetNativeFontFromATSFontRef( ATSFontRef aFont )
{
	return (sal_IntPtr)ImplATSLayoutData::GetNativeFontFromATSFontRef( aFont );
}

#endif	// !USE_CORETEXT_TEXT_RENDERING

// ----------------------------------------------------------------------------

void SalATSLayout::ClearLayoutDataCache()
{
	ImplATSLayoutData::ClearLayoutDataCache();
}

// ----------------------------------------------------------------------------

void SalATSLayout::SetFontFallbacks()
{
	ImplATSLayoutData::SetFontFallbacks();
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
	SetUnitsPerPixel( UNITS_PER_PIXEL );

	if ( mnFallbackLevel )
	{
		::std::hash_map< int, com_sun_star_vcl_VCLFont* >::const_iterator it = mpGraphics->maFallbackFonts.find( mnFallbackLevel );
		if ( it != mpGraphics->maFallbackFonts.end() && mnFallbackLevel < MAX_FALLBACK )
			mpVCLFont = new com_sun_star_vcl_VCLFont( it->second );
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
			nWidth = rArgs.mpDXArray[ rArgs.mnEndCharPos - rArgs.mnMinCharPos - 1 ] * UNITS_PER_PIXEL;
		else if ( rArgs.mnLayoutWidth )
			nWidth = rArgs.mnLayoutWidth * UNITS_PER_PIXEL;
		else
			nWidth = mnOrigWidth;

		// Fix bug 2882 by ensuring that the glyph scale is never zero
		if ( nWidth > 0 && nWidth < mnOrigWidth )
			mfGlyphScaleX = (float)nWidth / mnOrigWidth;
	}

	if ( rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN && ! ( rArgs.mnFlags & SAL_LAYOUT_VERTICAL ) )
		ApplyAsianKerning( rArgs.mpStr, rArgs.mnLength );

#ifdef USE_CORETEXT_TEXT_RENDERING
	if ( rArgs.mnFlags & SAL_LAYOUT_KASHIDA_JUSTIFICATON && rArgs.mpDXArray && mpKashidaLayoutData && mpKashidaLayoutData->mnGlyphCount )
		KashidaJustify( mpKashidaLayoutData->mpGlyphs[ 0 ], mpKashidaLayoutData->mpGlyphAdvances[ 0 ] );
#else	// USE_CORETEXT_TEXT_RENDERING
	if ( rArgs.mnFlags & SAL_LAYOUT_KASHIDA_JUSTIFICATON && rArgs.mpDXArray && mpKashidaLayoutData && mpKashidaLayoutData->mpGlyphDataArray )
		KashidaJustify( mpKashidaLayoutData->mpGlyphDataArray[ 0 ].glyphID, mpKashidaLayoutData->mpGlyphAdvances[ 0 ] );
#endif	// USE_CORETEXT_TEXT_RENDERING
}

// ----------------------------------------------------------------------------

bool SalATSLayout::LayoutText( ImplLayoutArgs& rArgs )
{
	Destroy();

	bool bRet = false;
	rArgs.mnFlags |= SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;

	if ( !mpVCLFont )
		return bRet;

	com_sun_star_vcl_VCLFont *pSymbolFallbackFont = NULL;
	bool bUseNativeFallback = false;
	int nEstimatedGlyphs = 0;

	// Aggregate runs
	bool bRunRTL;
	int nMinCharPos;
	int nEndCharPos;
	if ( !mnFallbackLevel )
	{
		// Fix bug 2841 by ensuring that we process only full runs. Fix bug
		// 3497 by limiting the amount of extra characters that we include.
		bool bDeleteArgs = false;
		nMinCharPos = rArgs.mnMinCharPos;
		nEndCharPos = rArgs.mnEndCharPos;
		int nMaxPosChange = MAXEXTRACHARS;
		while ( nMaxPosChange && nMinCharPos && !IsNonprintingChar( rArgs.mpStr[ nMinCharPos - 1 ] ) && !IsSpacingGlyph( rArgs.mpStr[ nMinCharPos - 1 ] | GF_ISCHAR ) )
		{
			nMinCharPos--;
			nMaxPosChange--;
			bDeleteArgs = true;
		}
		nMaxPosChange = MAXEXTRACHARS;
		while ( nMaxPosChange && nEndCharPos < rArgs.mnLength && !IsNonprintingChar( rArgs.mpStr[ nEndCharPos ] ) && !IsSpacingGlyph( rArgs.mpStr[ nEndCharPos ] | GF_ISCHAR ) )
		{
			nEndCharPos++;
			nMaxPosChange--;
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
					for ( ; i > nMinCharPos && !IsNonprintingChar( pArgs->mpStr[ i - 1 ] ) && !IsSpacingGlyph( pArgs->mpStr[ i - 1 ] | GF_ISCHAR ); i-- )
						;
					for ( ; i > nMinCharPos && ( IsNonprintingChar( pArgs->mpStr[ i - 1 ] ) || IsSpacingGlyph( pArgs->mpStr[ i - 1 ] | GF_ISCHAR ) ); i-- )
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
					for ( ; i < nEndCharPos && !IsNonprintingChar( pArgs->mpStr[ i ] ) && !IsSpacingGlyph( pArgs->mpStr[ i ] | GF_ISCHAR ); i++ )
						;
					for ( ; i < nEndCharPos && ( IsNonprintingChar( pArgs->mpStr[ i ] ) || IsSpacingGlyph( pArgs->mpStr[ i ] | GF_ISCHAR ) ); i++ )
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

		// Check if this run will need Kashida justification. Fix bug 3149
		// by always checking if Arabic font support is needed even if
		// kashidas won't be needed.
		if ( bRunRTL )
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
#ifdef USE_CORETEXT_TEXT_RENDERING
					if ( mpKashidaLayoutData->mpNeedFallback && mpKashidaLayoutData->mpFallbackFont )
#else	// USE_CORETEXT_TEXT_RENDERING
					if ( !mpKashidaLayoutData->mpGlyphDataArray || ( mpKashidaLayoutData->mpNeedFallback && mpKashidaLayoutData->mpFallbackFont ) )
#endif	// USE_CORETEXT_TEXT_RENDERING
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
						for ( int i = 0; i < (int)mpKashidaLayoutData->mnGlyphCount; i++ )
						{
#ifdef USE_CORETEXT_TEXT_RENDERING
							if ( !mpKashidaLayoutData->mpGlyphs[ i ] )
#else	// USE_CORETEXT_TEXT_RENDERING
							if ( !mpKashidaLayoutData->mpGlyphDataArray[ i ].glyphID )
#endif	// USE_CORETEXT_TEXT_RENDERING
							{
								bHasArabicFontSupport = false;
								break;
							}
#ifdef USE_CORETEXT_TEXT_RENDERING
							else if ( i && mpKashidaLayoutData->mpGlyphs[ i ] == mpKashidaLayoutData->mpGlyphs[ i - 1 ] )
#else	// USE_CORETEXT_TEXT_RENDERING
							else if ( i && mpKashidaLayoutData->mpGlyphDataArray[ i ].glyphID == mpKashidaLayoutData->mpGlyphDataArray[ i - 1 ].glyphID )
#endif	// USE_CORETEXT_TEXT_RENDERING
							{
								bHasArabicFontSupport = false;
								break;
							}
#ifdef USE_CORETEXT_TEXT_RENDERING
							else if ( mpKashidaLayoutData->mpGlyphs[ i ] == 0xffff )
#else	// USE_CORETEXT_TEXT_RENDERING
							else if ( mpKashidaLayoutData->mpGlyphDataArray[ i ].glyphID == 0xffff )
#endif	// USE_CORETEXT_TEXT_RENDERING
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
							if ( mpKashidaLayoutData->mpFallbackFont )
							{
								delete mpKashidaLayoutData->mpFallbackFont;
								mpKashidaLayoutData->mpFallbackFont = NULL;
							}

							SalData *pSalData = GetSalData();

							::std::map< String, JavaImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( aGeezaPro );
							if ( it != pSalData->maFontNameMapping.end() )
							{
								com_sun_star_vcl_VCLFont *pVCLFont = new com_sun_star_vcl_VCLFont( it->second->maVCLFontName, mpVCLFont->getSize(), mpVCLFont->getOrientation(), mpVCLFont->isAntialiased(), mpVCLFont->isVertical(), mpVCLFont->getScaleX() );
								if ( pVCLFont )
								{
									if ( pVCLFont->getNativeFont() != mpVCLFont->getNativeFont() )
										mpKashidaLayoutData->mpFallbackFont = pVCLFont;
									else
										delete pVCLFont;
								}
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
				if ( nIndex >= 0 && !IsNonprintingChar( pLayoutData->mpHash->mpStr[ nIndex ] ) && !IsSpacingGlyph( pLayoutData->mpHash->mpStr[ nIndex ] | GF_ISCHAR ) )
				{
					int i = pLayoutData->mpCharsToGlyphs[ nIndex ];
#ifdef USE_CORETEXT_TEXT_RENDERING
					if ( i >= 0 && pLayoutData->mpGlyphs[ i ] == 0xffff )
#else	// USE_CORETEXT_TEXT_RENDERING
					if ( i >= 0 && pLayoutData->mpGlyphDataArray && pLayoutData->mpGlyphDataArray[ i ].glyphID == 0xffff )
#endif	// USE_CORETEXT_TEXT_RENDERING
					{
						nMinFallbackCharPos = nMinCharPos;
						bRelayout = true;
					}
				}
			}

			if ( nEndCharPos < nEndFallbackCharPos )
			{
				int nIndex = pLayoutData->mpCharsToChars[ nEndCharPos - nMinFallbackCharPos - ( bRunRTL ? 1 : 0 ) ];
				if ( nIndex >= 0 && !IsNonprintingChar( pLayoutData->mpHash->mpStr[ nIndex ] ) && !IsSpacingGlyph( pLayoutData->mpHash->mpStr[ nIndex ] | GF_ISCHAR ) )
				{
					int i = pLayoutData->mpCharsToGlyphs[ nIndex ];
#ifdef USE_CORETEXT_TEXT_RENDERING
					if ( i >= 0 && pLayoutData->mpGlyphs[ i ] == 0xffff )
#else	// USE_CORETEXT_TEXT_RENDERING
					if ( i >= 0 && pLayoutData->mpGlyphDataArray && pLayoutData->mpGlyphDataArray[ i ].glyphID == 0xffff )
#endif	// USE_CORETEXT_TEXT_RENDERING
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
						::std::hash_map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.find( nChar );
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
 
#ifdef USE_CORETEXT_TEXT_RENDERING
				for ( CFIndex i = pCurrentLayoutData->mpCharsToGlyphs[ nIndex ]; i >= 0 && i < pCurrentLayoutData->mnGlyphCount && pCurrentLayoutData->mpGlyphsToChars[ i ] == nIndex; i++ )
				{
					long nGlyphWidth = pCurrentLayoutData->mpGlyphAdvances[ i ];
					sal_Int32 nGlyph = (sal_Int32)pCurrentLayoutData->mpGlyphs[ i ];
#else	// USE_CORETEXT_TEXT_RENDERING
				for ( ItemCount i = pCurrentLayoutData->mpCharsToGlyphs[ nIndex ]; i >= 0 && i < pCurrentLayoutData->mnGlyphCount && pCurrentLayoutData->mpGlyphDataArray && (int)( pCurrentLayoutData->mpGlyphDataArray[ i ].originalOffset / 2 ) == nIndex; i++ )
				{
					long nGlyphWidth = pCurrentLayoutData->mpGlyphAdvances[ i ];
					sal_Int32 nGlyph = pCurrentLayoutData->mpGlyphDataArray[ i ].glyphID;
#endif	// USE_CORETEXT_TEXT_RENDERING

					// Fix bug 3588 by setting fallback glyph IDs to zero
					if ( nGlyph && pLayoutData->mpNeedFallback && pLayoutData->mpNeedFallback[ nIndex ] )
						nGlyph = 0;

					if ( !nGlyph )
					{
						if ( nChar < 0x0500 )
						{
							// Fix bug 3087 if there is no fallback font and it
							// is a European or Cyrillic character by using a
							// font that we can render those ranges nicely
							if ( !pSymbolFallbackFont )
							{
								SalData *pSalData = GetSalData();

								::std::map< String, JavaImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( mpGraphics->mpFontData->meFamily == FAMILY_ROMAN ? aTimesRoman : aHelvetica );
								if ( it != pSalData->maFontNameMapping.end() )
								{
									pSymbolFallbackFont = new com_sun_star_vcl_VCLFont( it->second->maVCLFontName, mpVCLFont->getSize(), mpVCLFont->getOrientation(), mpVCLFont->isAntialiased(), mpVCLFont->isVertical(), mpVCLFont->getScaleX() );
									if ( pSymbolFallbackFont->getNativeFont() == mpVCLFont->getNativeFont() )
									{
										delete pSymbolFallbackFont;
										pSymbolFallbackFont = NULL;
									}
								}
							}

							rArgs.NeedFallback( nCharPos, bRunRTL );
							rArgs.mnFlags &= ~SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
						}
						else if ( nChar >= 0x0590 && nChar < 0x0600 )
						{
							// If there is no fallback font and it is a Hebrew
							// character, use the Raanana font
							if ( !pSymbolFallbackFont )
							{
								SalData *pSalData = GetSalData();

								::std::map< String, JavaImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( aTimesNewRoman );
								if ( it != pSalData->maFontNameMapping.end() )
								{
									pSymbolFallbackFont = new com_sun_star_vcl_VCLFont( it->second->maVCLFontName, mpVCLFont->getSize(), mpVCLFont->getOrientation(), mpVCLFont->isAntialiased(), mpVCLFont->isVertical(), mpVCLFont->getScaleX() );
									if ( pSymbolFallbackFont->getNativeFont() == mpVCLFont->getNativeFont() )
									{
										delete pSymbolFallbackFont;
										pSymbolFallbackFont = NULL;
									}
								}
							}

							rArgs.NeedFallback( nCharPos, bRunRTL );
							rArgs.mnFlags &= ~SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
						}
						else if ( nChar >= 0xe000 && nChar < 0xf900 )
						{
							// If there is no fallback font and it is a Private
							// Use Area character, use the symbol font
							if ( !pSymbolFallbackFont )
							{
								SalData *pSalData = GetSalData();

								::std::map< String, JavaImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( aOpenSymbol );
								if ( it != pSalData->maFontNameMapping.end() )
								{
									pSymbolFallbackFont = new com_sun_star_vcl_VCLFont( it->second->maVCLFontName, mpVCLFont->getSize(), mpVCLFont->getOrientation(), mpVCLFont->isAntialiased(), mpVCLFont->isVertical(), mpVCLFont->getScaleX() );
									if ( pSymbolFallbackFont->getNativeFont() == mpVCLFont->getNativeFont() )
									{
										delete pSymbolFallbackFont;
										pSymbolFallbackFont = NULL;
									}
								}
							}

							rArgs.NeedFallback( nCharPos, bRunRTL );
							rArgs.mnFlags &= ~SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
						}
						else if ( nChar >= 0x3000 && ( nChar < 0x3100 || ( nChar >= 0x31f0 && nChar < 0x3200 ) || ( nChar >= 0x3300 && nChar < 0x4dc0 ) || ( nChar >= 0x4e00 && nChar < 0xa000 ) || ( nChar >= 0xfe30 && nChar < 0xfe50 ) || ( nChar >= 0xff00 && nChar < 0xfff0 ) ) )
						{
							// Fix bugs 2772 and 3097 if there is no fallback
							// font and it is a Japanese character by using a
							// font that we can render those ranges nicely
							if ( !pSymbolFallbackFont )
							{
								SalData *pSalData = GetSalData();

								::std::map< String, JavaImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( mpGraphics->mpFontData->meFamily == FAMILY_ROMAN ? aHiraginoMinchoProW3 : aHiraginoKakuGothicProW3 );
								if ( it != pSalData->maFontNameMapping.end() )
								{
									pSymbolFallbackFont = new com_sun_star_vcl_VCLFont( it->second->maVCLFontName, mpVCLFont->getSize(), mpVCLFont->getOrientation(), mpVCLFont->isAntialiased(), mpVCLFont->isVertical(), mpVCLFont->getScaleX() );
									if ( pSymbolFallbackFont->getNativeFont() == mpVCLFont->getNativeFont() )
									{
										delete pSymbolFallbackFont;
										pSymbolFallbackFont = NULL;
									}
								}
							}

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
					// Prevent display of zero glyphs in fallback levels where
					// we know that there is a valid fallback font
					else if ( !nGlyph && mnFallbackLevel < MAX_FALLBACK - 1 && ( pSymbolFallbackFont || pFallbackFont ) )
					{
						// Fix bug 3504 by not changing the glyph flags for
						// zero glyphs
						nGlyphWidth = 0;
						if ( !bFirstGlyph )
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

					// Mark known nonprinting characters
					if ( IsNonprintingChar( nChar ) )
						nGlyphFlags |= GlyphItem::IS_NONPRINTING_CHAR;

					AppendGlyph( GlyphItem( nCharPos, nGlyph, aPos, nGlyphFlags, nGlyphWidth ) );

					aPos.X() += nGlyphWidth;
					bFirstGlyph = false;
					bRet = true;
				}
			}

			if ( bFirstGlyph )
			{
				AppendGlyph( GlyphItem( nCharPos, 0x0020 | GF_ISCHAR, aPos, bRunRTL ? GlyphItem::IS_RTL_GLYPH : 0, 0 ) );
				bRet = true;
			}
		}

		maLayoutData.push_back( pLayoutData );
		maLayoutMinCharPos.push_back( nMinFallbackCharPos );
	}

	mnOrigWidth = aPos.X();

	// Set fallback font
	if ( pFallbackFont || pSymbolFallbackFont || ! ( rArgs.mnFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING ) )
	{
		SalData *pSalData = GetSalData();

		// If this is the first fallback, first try using a font that most
		// closely matches the currently requested font
		JavaImplFontData *pHighScoreFontData = NULL;
		::std::hash_map< sal_IntPtr, JavaImplFontData* >::const_iterator nfit = pSalData->maNativeFontMapping.find( pSymbolFallbackFont ? pSymbolFallbackFont->getNativeFont() : 0 );
		if ( nfit != pSalData->maNativeFontMapping.end() )
			pHighScoreFontData = nfit->second;
		
		if ( !pHighScoreFontData && !bUseNativeFallback && !mnFallbackLevel && ( !mpKashidaLayoutData || !mpKashidaLayoutData->mpFallbackFont ) )
		{
			::std::hash_map< sal_IntPtr, JavaImplFontData* >::const_iterator it = pSalData->maNativeFontMapping.find( pFallbackFont ? pFallbackFont->getNativeFont() : 0 );
			if ( it == pSalData->maNativeFontMapping.end() || it->second->GetFamilyType() != mpGraphics->mnFontFamily || it->second->GetWeight() != mpGraphics->mnFontWeight || ( it->second->GetSlant() == ITALIC_OBLIQUE || it->second->GetSlant() == ITALIC_NORMAL ? true : false ) != mpGraphics->mbFontItalic || it->second->GetPitch() != mpGraphics->mnFontPitch )
			{
				USHORT nHighScore = 0;
				sal_IntPtr nNativeFont = mpVCLFont->getNativeFont();
				for ( it = pSalData->maNativeFontMapping.begin(); it != pSalData->maNativeFontMapping.end(); ++it )
				{
					if ( it->first == nNativeFont )
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

		int nNextLevel = mnFallbackLevel + 1;
		::std::hash_map< int, com_sun_star_vcl_VCLFont* >::iterator it = mpGraphics->maFallbackFonts.find( nNextLevel );
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
			mpGraphics->maFallbackFonts[ nNextLevel ] = new com_sun_star_vcl_VCLFont( pHighScoreFontData->maVCLFontName, mpVCLFont->getSize(), mpVCLFont->getOrientation(), mpVCLFont->isAntialiased(), mpVCLFont->isVertical(), mpVCLFont->getScaleX() );
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
	sal_GlyphId aGlyphArray[ nMaxGlyphs ];
	sal_Int32 aDXArray[ nMaxGlyphs ];
	int aCharPosArray[ nMaxGlyphs ];

	Point aPos;
	JavaSalGraphics& rJavaGraphics = (JavaSalGraphics&)rGraphics;
	bool bPrinter = ( rJavaGraphics.mpPrinter ? true : false );
	int nFetchGlyphCount = nMaxGlyphs;
	for ( int nStart = 0; ; )
	{
		int nOldStart = nStart;
		int nTotalGlyphCount = GetNextGlyphs( nFetchGlyphCount, aGlyphArray, aPos, nStart, aDXArray, aCharPosArray );

		if ( !nTotalGlyphCount )
			break;

		// The GenericSalLayout class should return glyph runs with the same
		// rotation mask
		sal_Int32 nGlyphOrientation = aGlyphArray[ 0 ] & GF_ROTMASK;
		if ( nGlyphOrientation && nFetchGlyphCount > 1 )
		{
			nFetchGlyphCount = 1;
			nStart = nOldStart;
			continue;
		}
		else if ( !nGlyphOrientation && nFetchGlyphCount < nMaxGlyphs )
		{
			nFetchGlyphCount = nMaxGlyphs;
			nStart = nOldStart;
			continue;
		}

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
			if ( nStartGlyph < nTotalGlyphCount )
			{
				if ( nGlyphOrientation )
				{
					nGlyphCount++;
					aCurrentPos.Y() += aDXArray[ nStartGlyph ];
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

			for ( i = nStartGlyph; i < nCurrentGlyph; i++ )
				aGlyphArray[ i ] &= GF_IDXMASK;

			float fTranslateX = (float)nTranslateX / UNITS_PER_PIXEL;
			float fTranslateY = (float)nTranslateY / UNITS_PER_PIXEL;

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
						pSizes[ i ].width = (float)aDXArray[ i + nStartGlyph ] / UNITS_PER_PIXEL;
						pSizes[ i ].height = 0.0f;
					}
				}

				rJavaGraphics.mpVCLGraphics->drawGlyphBuffer( aStartPos.X(), aStartPos.Y(), nGlyphCount, pGlyphs, pSizes, mpVCLFont, rJavaGraphics.mnTextColor, GetOrientation(), nGlyphOrientation, fTranslateX, fTranslateY, mfGlyphScaleX, rJavaGraphics.maNativeClipPath ? CGPathCreateCopy( rJavaGraphics.maNativeClipPath ) : NULL );
			}
			else
			{
				float aAdvances[ nGlyphCount ];
				for ( i = 0; i < nGlyphCount; i++ )
					aAdvances[ i ] = (float)aDXArray[ i + nStartGlyph ] / UNITS_PER_PIXEL;

				rJavaGraphics.mpVCLGraphics->drawGlyphs( aStartPos.X(), aStartPos.Y(), nGlyphCount, aGlyphArray + nStartGlyph, aAdvances, mpVCLFont, rJavaGraphics.mnTextColor, GetOrientation(), nGlyphOrientation, fTranslateX, fTranslateY, mfGlyphScaleX );
			}
		}
	}
}

// ----------------------------------------------------------------------------

bool SalATSLayout::GetBoundRect( SalGraphics& rGraphics, Rectangle& rRect ) const
{
GenericSalLayout::GetBoundRect( rGraphics, rRect );
	rRect.SetEmpty();

	Rectangle aRect;
	for ( std::vector< ImplATSLayoutData* >::const_iterator it = maLayoutData.begin(); it != maLayoutData.end(); ++it )
	{
		Rectangle aGlyphBounds( (*it)->GetGlyphBounds() );
		if ( aGlyphBounds.IsEmpty() )
			continue;
		if ( !aRect.IsEmpty() )
			aGlyphBounds.setX( aGlyphBounds.Left() + aRect.Left() + aRect.GetWidth() );
		aRect.Union( aGlyphBounds );
	}

	if ( !aRect.IsEmpty() )
	{
		// Fix bug 3578 by moving the rectangle to the layout's draw position
		aRect += GetDrawPosition( Point( 0, 0 ) );
		aRect.setWidth( Float32ToLong( (float)aRect.GetWidth() * mfGlyphScaleX ) );
	}

	// Fix bug 2191 by always returning true so that the OOo code doesn't
	// exeecute its "draw the glyph and see which pixels are black" code
	rRect = aRect;
	return true;
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
	sal_GlyphId aGlyphArray[ nMaxGlyphs ];
	sal_Int32 aDXArray[ nMaxGlyphs ];
	int aCharPosArray[ nMaxGlyphs ];

	Point aPos;
	unsigned int nRunIndex = 0;
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

		// Check if this is a kashida glyph
#ifdef USE_CORETEXT_TEXT_RENDERING
		if ( mpKashidaLayoutData && (CGGlyph)( aGlyphArray[ 0 ] & GF_IDXMASK ) == mpKashidaLayoutData->mpGlyphs[ 0 ] )
#else	// USE_CORETEXT_TEXT_RENDERING
		if ( mpKashidaLayoutData && mpKashidaLayoutData->mpGlyphDataArray && (sal_Int32)( aGlyphArray[ 0 ] & GF_IDXMASK ) == mpKashidaLayoutData->mpGlyphDataArray[ 0 ].glyphID )
#endif	// USE_CORETEXT_TEXT_RENDERING
		{
			pCurrentLayoutData = mpKashidaLayoutData;
			nIndex = mpKashidaLayoutData->mpHash->mnLen - 1;
		}
		// Check if this is a mirrored or ideographic space character
		else if ( pCurrentLayoutData->mpHash->mbRTL )
		{
			sal_Unicode nChar = pCurrentLayoutData->mpHash->mpStr[ nIndex ];
			::std::hash_map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.find( nChar );
			if ( mit != maMirroredLayoutData.end() )
			{
				pCurrentLayoutData = mit->second;
				nIndex = 0;
			}
		}

#ifdef USE_CORETEXT_TEXT_RENDERING
		for ( CFIndex i = pCurrentLayoutData->mpCharsToGlyphs[ nIndex ]; i >= 0 && i < pCurrentLayoutData->mnGlyphCount && pCurrentLayoutData->mpGlyphsToChars[ i ] == nIndex; i++ )
		{
			CGGlyph nGlyph = pCurrentLayoutData->mpGlyphs[ i ];
			if ( (CGGlyph)( aGlyphArray[ 0 ] & GF_IDXMASK ) != nGlyph )
				continue;
#else	// USE_CORETEXT_TEXT_RENDERING
		for ( ItemCount i = pCurrentLayoutData->mpCharsToGlyphs[ nIndex ]; i >= 0 && i < pCurrentLayoutData->mnGlyphCount && pCurrentLayoutData->mpGlyphDataArray && (int)( pCurrentLayoutData->mpGlyphDataArray[ i ].originalOffset / 2 ) == nIndex; i++ )
		{
			sal_Int32 nGlyph = pCurrentLayoutData->mpGlyphDataArray[ i ].glyphID;
			if ( (sal_Int32)( aGlyphArray[ 0 ] & GF_IDXMASK ) != nGlyph )
				continue;
#endif	// USE_CORETEXT_TEXT_RENDERING

			// Fix bug 2390 by ignoring the value of nErr passed by reference
			::std::list< Polygon > aPolygonList;
			ATSUStyle aCurrentStyle = NULL;
			UniCharArrayOffset nRunStart;
			UniCharCount nRunLen;
			OSStatus nErr;
#ifdef USE_CORETEXT_TEXT_RENDERING
			if ( pATSUGetRunStyle( pCurrentLayoutData->maLayout, nIndex, &aCurrentStyle, &nRunStart, &nRunLen ) != noErr || !aCurrentStyle || pATSUGlyphGetCubicPaths( aCurrentStyle, pCurrentLayoutData->mpGlyphs[ i ], pATSCubicMoveToUPP, pATSCubicLineToUPP, pATSCubicCurveToUPP, pATSCubicClosePathUPP, (void *)&aPolygonList, &nErr ) != noErr )
#else	// USE_CORETEXT_TEXT_RENDERING
			if ( pATSUGetRunStyle( pCurrentLayoutData->maLayout, nIndex, &aCurrentStyle, &nRunStart, &nRunLen ) != noErr || !aCurrentStyle || pATSUGlyphGetCubicPaths( aCurrentStyle, pCurrentLayoutData->mpGlyphDataArray[ i ].glyphID, pATSCubicMoveToUPP, pATSCubicLineToUPP, pATSCubicCurveToUPP, pATSCubicClosePathUPP, (void *)&aPolygonList, &nErr ) != noErr )
#endif	// USE_CORETEXT_TEXT_RENDERING
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

			aPolyPolygon.Move( aPos.X() * UNITS_PER_PIXEL, aPos.Y() * UNITS_PER_PIXEL );

			sal_Int32 nGlyphOrientation = aGlyphArray[ 0 ] & GF_ROTMASK;
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
		for ( ::std::hash_map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.begin(); mit != maMirroredLayoutData.end(); ++mit )
			mit->second->Release();
		maMirroredLayoutData.clear();
	}

	mnOrigWidth = 0;
	mfGlyphScaleX = 1.0;
}

// ----------------------------------------------------------------------------

ImplATSLayoutData *SalATSLayout::GetVerticalGlyphTranslation( sal_Int32 nGlyph, int nCharPos, long& nX, long& nY ) const
{
	ImplATSLayoutData *pRet = NULL;

	nX = 0;
	nY = 0;

	if ( !maLayoutData.size() )
		return pRet;

	unsigned int nRunIndex = 0;
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
		::std::hash_map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.find( nChar );
		if ( mit != maMirroredLayoutData.end() )
		{
			pRet = mit->second;
			nIndex = 0;
		}
	}

	sal_Int32 nGlyphOrientation = nGlyph & GF_ROTMASK;

	if ( pRet->maVerticalFontStyle && nGlyphOrientation & GF_ROTMASK )
	{
		GlyphID nGlyphID = (GlyphID)( nGlyph & GF_IDXMASK );

		::std::hash_map< GlyphID, Point >::const_iterator it = pRet->maVerticalGlyphTranslations.find( nGlyphID );
		if ( it == pRet->maVerticalGlyphTranslations.end() )
		{
#ifdef USE_CORETEXT_TEXT_RENDERING
			CGGlyph aGlyph = (CGGlyph)nGlyphID;
			CGSize aTranslation = CGSizeMake( 0, 0 );
			CTFontGetVerticalTranslationsForGlyphs( pRet->maFont, &aGlyph, &aTranslation, 1 );
			nX = Float32ToLong( aTranslation.width * UNITS_PER_PIXEL );
			if ( nGlyphOrientation == GF_ROTL )
				nX += pRet->mnBaselineDelta;
			else
				nX -= pRet->mnBaselineDelta;
			nY = Float32ToLong( aTranslation.height * -1 * UNITS_PER_PIXEL );
#else	// USE_CORETEXT_TEXT_RENDERING
			ATSGlyphScreenMetrics aVerticalMetrics;
			ATSGlyphScreenMetrics aHorizontalMetrics;
			if ( pATSUGlyphGetScreenMetrics( pRet->maVerticalFontStyle, 1, &nGlyphID, sizeof( GlyphID ), pRet->mpHash->mbAntialiased, pRet->mpHash->mbAntialiased, &aVerticalMetrics ) == noErr && pATSUGlyphGetScreenMetrics( pRet->maFontStyle, 1, &nGlyphID, sizeof( GlyphID ), pRet->mpHash->mbAntialiased, pRet->mpHash->mbAntialiased, &aHorizontalMetrics ) == noErr )
			{
				nX = Float32ToLong( ( aVerticalMetrics.topLeft.x - aHorizontalMetrics.topLeft.x ) * pRet->mfFontScaleY * UNITS_PER_PIXEL );
				if ( nGlyphOrientation == GF_ROTL )
					nX += pRet->mnBaselineDelta;
				else
					nX -= pRet->mnBaselineDelta;
				nY = Float32ToLong( ( aHorizontalMetrics.topLeft.y - aVerticalMetrics.topLeft.y ) * pRet->mfFontScaleY * UNITS_PER_PIXEL );
			}
#endif	// USE_CORETEXT_TEXT_RENDERING
			pRet->maVerticalGlyphTranslations[ nGlyphID ] = Point( nX, nY );
		}
		else
		{
			nX = it->second.X();
			nY = it->second.Y();
		}
	}

	return pRet;
}

// ----------------------------------------------------------------------------

sal_Int32 SalATSLayout::GetNativeGlyphWidth( sal_Int32 nGlyph, int nCharPos ) const
{
	sal_Int32 nRet = 0;

	if ( !maLayoutData.size() )
		return nRet;

	unsigned int nRunIndex = 0;
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
		return nRet;

	int nIndex = pLayoutData->mpCharsToChars[ nCharPos - nMinCharPos ];
	if ( nIndex < 0 )
		return nRet;

	// Check if this is a mirrored character
	sal_Unicode nChar = pLayoutData->mpHash->mpStr[ nIndex ];
	if ( pLayoutData->mpHash->mbRTL )
	{
		::std::hash_map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.find( nChar );
		if ( mit != maMirroredLayoutData.end() )
		{
			pLayoutData = mit->second;
			nIndex = 0;
		}
	}

	GlyphID nGlyphID = (GlyphID)( nGlyph & GF_IDXMASK );

	::std::hash_map< GlyphID, long >::const_iterator it = pLayoutData->maNativeGlyphWidths.find( nGlyphID );
	if ( it == pLayoutData->maNativeGlyphWidths.end() )
	{
#ifdef USE_CORETEXT_TEXT_RENDERING
		CGGlyph aGlyph = (CGGlyph)nGlyphID;
		nRet = Float32ToLong( CTFontGetAdvancesForGlyphs( pLayoutData->maFont, kCTFontHorizontalOrientation, &aGlyph, NULL, 1 ) * UNITS_PER_PIXEL );
#else	// USE_CORETEXT_TEXT_RENDERING
		ATSGlyphIdealMetrics aIdealMetrics;
		if ( pATSUGlyphGetIdealMetrics( pLayoutData->maFontStyle, 1, &nGlyphID, sizeof( GlyphID ), &aIdealMetrics ) == noErr )
			nRet = Float32ToLong( aIdealMetrics.advance.x * pLayoutData->mfFontScaleY * UNITS_PER_PIXEL );
#endif	// USE_CORETEXT_TEXT_RENDERING
		pLayoutData->maNativeGlyphWidths[ nGlyphID ] = nRet;
	}
	else
	{
		nRet = it->second;
	}

	return nRet;
}
