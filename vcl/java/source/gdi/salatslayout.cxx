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

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <unicode/ubidi.h>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

#include "java/salatslayout.hxx"
#include "java/saldata.hxx"
#include "java/salgdi.h"
#include "java/salinst.h"
#include "quartz/utils.h"

#define MAXEXTRACHARS 100
#ifdef USE_SUBPIXEL_TEXT_RENDERING
#define UNITS_PER_PIXEL 1000
#else	// USE_SUBPIXEL_TEXT_RENDERING
#define UNITS_PER_PIXEL 1
#endif	// USE_SUBPIXEL_TEXT_RENDERING

static const OUString aAlBayanPlain( "Al Bayan Plain" );
static const OUString aAppleSymbols( "Apple Symbols" );
static const OUString aArialUnicodeMS( "Arial Unicode MS" );
static const OUString aBanglaMN( "Bangla MN" );
static const OUString aBanglaSangamMN( "Bangla Sangam MN" );
static const OUString aDevanagariMT( "Devanagari MT" );
static const OUString aDevanagariSangamMN( "Devanagari Sangam MT" );
static const OUString aGeezaPro( "Geeza Pro" );
static const OUString aGeezaProRegular( "Geeza Pro Regular" );
static const OUString aGujaratiMN( "Gujarati MN" );
static const OUString aGujaratiSangamMN( "Gujarati Sangam MN" );
static const OUString aGurmukhiMN( "Gurmukhi MN" );
static const OUString aGurmukhiSangamMN( "Gurmukhi Sangam MN" );
static const OUString aHeitiSCMedium( "Heiti SC Medium" );
static const OUString aHelvetica( "Helvetica" );
static const OUString aHiraginoKakuGothicProW3( "Hiragino Kaku Gothic Pro W3" );
static const OUString aHiraginoMinchoProW3( "Hiragino Mincho Pro W3" );
static const OUString aLucidaGrande( "Lucida Grande" );
static const OUString aOpenSymbol( "OpenSymbol" );
static const OUString aOriyaMN( "Oriya MN" );
static const OUString aOriyaSangamMN( "Oriya Sangam MN" );
static const OUString aTamilMN( "Tamil MN" );
static const OUString aTamilSangamMN( "Tamil Sangam MN" );
static const OUString aTeluguMN( "Telugu MN" );
static const OUString aTeluguSangamMN( "Telugu Sangam MN" );
static const OUString aTimesNewRoman( "Times New Roman" );
static const OUString aTimesRoman( "Times Roman" );

inline long Float32ToLong( Float32 f ) { return static_cast< long >( f + 0.5 ); }

inline bool IsNonprintingChar( sal_Unicode nChar ) { return ( nChar == 0x00b6 || nChar == 0x00b7 ); }

inline bool IsSurrogatePairChar( sal_Unicode nChar ) { return ( nChar >= 0xd800 && nChar <= 0xdfff ); }

struct SAL_DLLPRIVATE ImplATSLayoutDataHash {
	int					mnLen;
	CTFontRef			mnFontID;
	float				mfFontSize;
	double				mfFontScaleX;
	bool				mbAntialiased;
	bool				mbRTL;
	bool				mbVertical;
	sal_Unicode*		mpStr;
	sal_Int32			mnStrHash;
	bool				mbOwnsStr;

						ImplATSLayoutDataHash( const sal_Unicode *pStr, int nLen, int nMinCharPos, int nEndCharPos, SalLayoutFlags nFlags, JavaImplFont *pFont );
						~ImplATSLayoutDataHash();
};

struct SAL_DLLPRIVATE ImplATSLayoutDataHashHash
{
	size_t				operator()( const ImplATSLayoutDataHash *x ) const { return static_cast< size_t >( x->mnStrHash ); }
};

struct SAL_DLLPRIVATE ImplATSLayoutDataHashEquality
{
	bool				operator()( const ImplATSLayoutDataHash *p1, const ImplATSLayoutDataHash *p2 ) const;
};

struct SAL_DLLPRIVATE ImplATSLayoutData {
	static ::std::unordered_map< ImplATSLayoutDataHash*, ImplATSLayoutData*, ImplATSLayoutDataHashHash, ImplATSLayoutDataHashEquality >	maLayoutCache;
	static ::std::list< ImplATSLayoutData* >	maLayoutCacheList;
	static int			mnLayoutCacheSize;
	static sal_uInt32	mnSharedContextData;
	static CGContextRef	maSharedContext;

	mutable int			mnRefCount;
	ImplATSLayoutDataHash*	mpHash;
	JavaImplFont*		mpFont;
	bool*				mpNeedFallback;
	::std::list< JavaImplFont* >	maFallbackFontList;
	::std::map< int, JavaImplFont* >	maFallbackFontMap;
	JavaImplFont*		mpFallbackFont;
	CTFontRef			maFont;
	CTTypesetterRef		maTypesetter;
	CTLineRef			maLine;
	CGGlyph*			mpGlyphs;
	int*				mpGlyphsToChars;
	CFIndex				mnGlyphCount;
	int*				mpCharsToGlyphs;
	int*				mpCharsToChars;
	long*				mpGlyphAdvances;
	long				mnBaselineDelta;
	bool				mbValid;
	::std::unordered_map< GlyphID, Point >	maVerticalGlyphTranslations;
	::std::unordered_map< GlyphID, long >	maNativeGlyphWidths;

	static CGContextRef			GetSharedContext();
	static void					ClearLayoutDataCache();
	static ImplATSLayoutData*	GetLayoutData( const sal_Unicode *pStr, int nLen, int nMinCharPos, int nEndCharPos, SalLayoutFlags nFlags, int nFallbackLevel, JavaImplFont *pFont, const SalATSLayout *pCurrentLayout );

						ImplATSLayoutData( ImplATSLayoutDataHash *pLayoutHash, int nFallbackLevel, JavaImplFont *pFont, const SalATSLayout *pCurrentLayout );
protected:
	virtual				~ImplATSLayoutData();

public:
	void				Destroy();
	JavaImplFont*		GetFallbackFont( int nCharPos );
	bool				IsValid() const { return mbValid; }
	void				Reference() const;
	void				Release() const;
};

class SAL_DLLPRIVATE JavaSalGraphicsDrawGlyphsOp : public JavaSalGraphicsOp
{
	CGPoint					maStartPoint;
	int						mnGlyphCount;
	CGGlyph*				mpGlyphs;
	CGPoint*				mpPositions;
	CTFontRef				mnFontID;
	float					mfFontSize;
	bool					mbAntialiased;
	SalColor				mnColor;
	float					mfRotateAngle;
	DeviceCoordinate		mfTranslateX;
	DeviceCoordinate		mfTranslateY;
	float					mfScaleX;
	float					mfScaleY;

public:
							JavaSalGraphicsDrawGlyphsOp( const CGPathRef aFrameClipPath, const CGPathRef aNativeClipPath, const CGPoint aStartPoint, int nGlyphCount, const GlyphItem **pGlyphs, const DeviceCoordinate *pAdvances, JavaImplFont *pFont, SalColor nColor, int nOrientation, int nGlyphOrientation, DeviceCoordinate fTranslateX, DeviceCoordinate fTranslateY, float fGlyphScaleX );
	virtual					~JavaSalGraphicsDrawGlyphsOp();

	virtual	void			drawOp( JavaSalGraphics *pGraphics, CGContextRef aContext, CGRect aBounds );
};

using namespace basegfx;
using namespace osl;
using namespace vcl;

// ============================================================================

ImplATSLayoutDataHash::ImplATSLayoutDataHash( const sal_Unicode *pStr, int /* nLen */, int nMinCharPos, int nEndCharPos, SalLayoutFlags nFlags, JavaImplFont *pFont ) :
	mnLen( nEndCharPos - nMinCharPos ),
	mnFontID( reinterpret_cast< CTFontRef >( pFont->getNativeFont() ) ),
	mfFontSize( pFont->getSize() ),
	mfFontScaleX( pFont->getScaleX() ),
	mbAntialiased( pFont->isAntialiased() ),
	mbRTL( nFlags & SalLayoutFlags::BiDiRtl ),
	mbVertical( nFlags & SalLayoutFlags::Vertical ),
	mpStr( const_cast< sal_Unicode* >( pStr + nMinCharPos ) ),
	mnStrHash( rtl_ustr_hashCode_WithLength( mpStr, mnLen ) ),
	mbOwnsStr( false )
{
	if ( mnFontID )
		CFRetain( static_cast< CTFontRef >( mnFontID ) );
}

// ----------------------------------------------------------------------------

ImplATSLayoutDataHash::~ImplATSLayoutDataHash()
{
	if ( mnFontID )
		CFRelease( static_cast< CTFontRef >( mnFontID ) );

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

::std::unordered_map< ImplATSLayoutDataHash*, ImplATSLayoutData*, ImplATSLayoutDataHashHash, ImplATSLayoutDataHashEquality > ImplATSLayoutData::maLayoutCache;

// ----------------------------------------------------------------------------

::std::list< ImplATSLayoutData* > ImplATSLayoutData::maLayoutCacheList;

// ----------------------------------------------------------------------------

int ImplATSLayoutData::mnLayoutCacheSize = 0;

// ----------------------------------------------------------------------------

sal_uInt32 ImplATSLayoutData::mnSharedContextData = 0;

// ----------------------------------------------------------------------------

CGContextRef ImplATSLayoutData::maSharedContext = nullptr;

// ----------------------------------------------------------------------------

CGContextRef ImplATSLayoutData::GetSharedContext()
{
	if ( !maSharedContext )
	{
		CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
		if ( aColorSpace )
		{
			maSharedContext = CGBitmapContextCreate( &mnSharedContextData, 1, 1, 8, sizeof( mnSharedContextData ), aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little );
			CGColorSpaceRelease( aColorSpace );
		}
	}

	return maSharedContext;
}

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
}

// ----------------------------------------------------------------------------

ImplATSLayoutData *ImplATSLayoutData::GetLayoutData( const sal_Unicode *pStr, int nLen, int nMinCharPos, int nEndCharPos, SalLayoutFlags nFlags, int nFallbackLevel, JavaImplFont *pFont, const SalATSLayout *pCurrentLayout )
{
	ImplATSLayoutData *pLayoutData = nullptr;

	ImplATSLayoutDataHash *pLayoutHash = new ImplATSLayoutDataHash( pStr, nLen, nMinCharPos, nEndCharPos, nFlags, pFont );

	// Search cache for matching layout
	::std::unordered_map< ImplATSLayoutDataHash*, ImplATSLayoutData*, ImplATSLayoutDataHashHash, ImplATSLayoutDataHashEquality >::const_iterator it = maLayoutCache.find( pLayoutHash );
	if ( it != maLayoutCache.end() )
	{
		pLayoutData = it->second;
		delete pLayoutHash;
		pLayoutHash = nullptr;
	}

	if ( !pLayoutData )
	{
		pLayoutHash->mbOwnsStr = true;
		pLayoutHash->mpStr = static_cast< sal_Unicode* >( rtl_allocateMemory( pLayoutHash->mnLen * sizeof( sal_Unicode ) ) );
		memcpy( pLayoutHash->mpStr, pStr + nMinCharPos, pLayoutHash->mnLen * sizeof( sal_Unicode ) );
		pLayoutData = new ImplATSLayoutData( pLayoutHash, nFallbackLevel, pFont, pCurrentLayout );

		if ( !pLayoutData )
		{
			delete pLayoutHash;
			return nullptr;
		}

		if ( !pLayoutData->IsValid() )
		{
			pLayoutData->Release();
			return nullptr;
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
				if ( !sysctl( pMib, 2, &nUserMem, &nUserMemLen, nullptr, 0 ) )
					nUserMem /= 2;
				if ( nUserMem > nMaxMem )
					nUserMem = nMaxMem;
				else if ( nUserMem < nMinMem )
					nUserMem = nMinMem;
				nLayoutCacheSize = nUserMem / ( 1024 * 8 );
				nTargetCacheSize = static_cast< int >( nLayoutCacheSize * 0.8 );
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
		maLayoutCache.insert( ::std::unordered_map< ImplATSLayoutDataHash*, ImplATSLayoutData*, ImplATSLayoutDataHashHash, ImplATSLayoutDataHashEquality >::value_type( pLayoutData->mpHash, pLayoutData ) );
		maLayoutCacheList.push_front( pLayoutData );
	}

	if ( pLayoutData )
		pLayoutData->Reference();

	return pLayoutData;
}

// ----------------------------------------------------------------------------

ImplATSLayoutData::ImplATSLayoutData( ImplATSLayoutDataHash *pLayoutHash, int /* nFallbackLevel */, JavaImplFont *pFont, const SalATSLayout *pCurrentLayout ) :
	mnRefCount( 1 ),
	mpHash( pLayoutHash ),
	mpFont( nullptr ),
	mpNeedFallback( nullptr ),
	mpFallbackFont( nullptr ),
	maFont( nullptr ),
	maTypesetter( nullptr ),
	maLine( nullptr ),
	mpGlyphs( nullptr ),
	mpGlyphsToChars( nullptr ),
	mnGlyphCount( 0 ),
	mpCharsToGlyphs( nullptr ),
	mpCharsToChars( nullptr ),
	mpGlyphAdvances( nullptr ),
	mnBaselineDelta( 0 ),
	mbValid( false )
{
	if ( !mpHash || !mpHash->mnFontID )
	{
		Destroy();
		return;
	}

	mpFont = new JavaImplFont( pFont );
	if ( !mpFont )
	{
		Destroy();
		return;
	}

	maFont = CTFontCreateCopyWithAttributes( static_cast< CTFontRef >( mpHash->mnFontID ), mpHash->mfFontSize, nullptr, nullptr );
	if ( !maFont )
	{
		Destroy();
		return;
	}

	CFStringRef aString = CFStringCreateWithCharactersNoCopy( nullptr, reinterpret_cast< const UniChar* >( mpHash->mpStr ), mpHash->mnLen, kCFAllocatorNull );
	if ( !aString )
	{
		Destroy();
		return;
	}

	CFAttributedStringRef aAttrString = CFAttributedStringCreate( nullptr, aString, nullptr );
	CFRelease( aString );
	if ( !aAttrString )
	{
		Destroy();
		return;
	}

	CFMutableAttributedStringRef aMutableAttrString = CFAttributedStringCreateMutableCopy( nullptr, 0, aAttrString );
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
			// Fix hanging on OS X 10.8 by not using vertical forms for
			// characters in the surrogate pair range
			if ( GetVerticalFlags( mpHash->mpStr[ i ] ) & GF_ROTMASK && !IsSurrogatePairChar( mpHash->mpStr[ i ] ) )
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

	maLine = CTTypesetterCreateLine( maTypesetter, CFRangeMake( 0, 0 ) );
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

	CFIndex nLineGlyphRuns = CFArrayGetCount( aLineGlyphRuns );
	CFIndex nGlyphsProcessed = 0;
	CFIndex nCurrentGlyphRun;
	CFIndex nBufSize;
	for ( nCurrentGlyphRun = 0; nCurrentGlyphRun < nLineGlyphRuns; nCurrentGlyphRun++ )
	{
		CTRunRef aGlyphRun = static_cast< CTRunRef >( CFArrayGetValueAtIndex( aLineGlyphRuns, nCurrentGlyphRun ) );
		if ( aGlyphRun )
			nGlyphsProcessed += CTRunGetGlyphCount( aGlyphRun );
	}

	mnGlyphCount = nGlyphsProcessed;

	// Cache mapping of characters to glyphs
	nBufSize = mpHash->mnLen * sizeof( int );
	mpCharsToGlyphs = static_cast< int* >( rtl_allocateMemory( nBufSize ) );

	CFIndex i;
	for ( i = 0; i < mpHash->mnLen; i++ )
		mpCharsToGlyphs[ i ] = -1;
	nBufSize = mnGlyphCount * sizeof( CGGlyph );
	mpGlyphs = static_cast< CGGlyph* >( rtl_allocateMemory( nBufSize ) );
	memset( mpGlyphs, 0, nBufSize );

	nBufSize = mnGlyphCount * sizeof( long );
	mpGlyphsToChars = static_cast< int* >( rtl_allocateMemory( nBufSize ) );
	for ( i = 0; i < mnGlyphCount; i++ )
		mpGlyphsToChars[ i ] = -1;

	nGlyphsProcessed = 0;
	for ( nCurrentGlyphRun = 0; nCurrentGlyphRun < nLineGlyphRuns; nCurrentGlyphRun++ )
	{
		CTRunRef aGlyphRun = static_cast< CTRunRef >( CFArrayGetValueAtIndex( aLineGlyphRuns, nCurrentGlyphRun ) );
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

	// Cache mapping of characters to glyph character indices
	nBufSize = mpHash->mnLen * sizeof( int );
	mpCharsToChars = static_cast< int* >( rtl_allocateMemory( nBufSize ) );

	for ( i = 0; i < mpHash->mnLen; i++ )
		mpCharsToChars[ i ] = -1;
	if ( mpHash->mbRTL )
	{
		i = 0;
		for ( int j = mpHash->mnLen - 1; j >= 0 && i < static_cast< int >( mnGlyphCount ); j-- )
		{
			// Detect characters that have no glyphs
			if ( mpCharsToGlyphs[ j ] < 0 )
				continue;

			int nIndex = mpGlyphsToChars[ i ];
			mpCharsToChars[ j ] = nIndex;
			for ( ; i < static_cast< int >( mnGlyphCount ) && mpGlyphsToChars[ i ] == nIndex; i++ )
				;
		}
	}
	else
	{
		i = 0;
		for ( int j = 0; j < mpHash->mnLen && i < static_cast< int >( mnGlyphCount ); j++ )
		{
			// Detect characters that have no glyphs
			if ( mpCharsToGlyphs[ j ] < 0 )
				continue;

			int nIndex = mpGlyphsToChars[ i ];
			mpCharsToChars[ j ] = nIndex;
			for ( ; i < static_cast< int >( mnGlyphCount ) && mpGlyphsToChars[ i ] == nIndex; i++ )
				;
		}
	}

	// Cache glyph widths
	nBufSize = mnGlyphCount * sizeof( long );
	mpGlyphAdvances = static_cast< long* >( rtl_allocateMemory( nBufSize ) );
	memset( mpGlyphAdvances, 0, nBufSize );

	int nLastNonSpacingIndex = -1;
	int nLastNonSpacingGlyph = -1;
	nGlyphsProcessed = 0;
	for ( nCurrentGlyphRun = 0; nCurrentGlyphRun < nLineGlyphRuns; nCurrentGlyphRun++ )
	{
		CTRunRef aGlyphRun = static_cast< CTRunRef >( CFArrayGetValueAtIndex( aLineGlyphRuns, nCurrentGlyphRun ) );
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
					const CFBooleanRef aValue = static_cast< const CFBooleanRef >( CFDictionaryGetValue( aDict, kCTVerticalFormsAttributeName ) );
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
							mpGlyphAdvances[ i ] += Float32ToLong( CTFontGetAdvancesForGlyphs( maFont, kCTFontOrientationVertical, &aGlyphs[ j ], nullptr, 1 ) * mpHash->mfFontScaleX * UNITS_PER_PIXEL );
						else
							mpGlyphAdvances[ i ] += Float32ToLong( CTFontGetAdvancesForGlyphs( maFont, kCTFontOrientationHorizontal, &aGlyphs[ j ], nullptr, 1 ) * mpHash->mfFontScaleX * UNITS_PER_PIXEL );
					}
					else
					{
						if ( bVerticalRun )
							mpGlyphAdvances[ i ] += Float32ToLong( ( aPositions[ j ].y - aPositions[ j + 1 ].y ) * mpHash->mfFontScaleX * UNITS_PER_PIXEL );
						else
							mpGlyphAdvances[ i ] += Float32ToLong( ( aPositions[ j + 1 ].x - aPositions[ j ].x ) * mpHash->mfFontScaleX * UNITS_PER_PIXEL );
					}

					// Make sure that ligature glyphs get all of the width and
					// that their attached spacing glyphs have zero width so
					// that the OOo code will force the cursor to the end of
					// the ligature instead of the beginning. Fix bug 3621 by
					// treating negative width glyphs like ligatured glyphs.
					long nWidthAdjust = 0;
					if ( ( mpGlyphs[ i ] == 0xffff || mpGlyphAdvances[ i ] < 0 ) && !IsNonprintingChar( mpHash->mpStr[ nIndex ] ) && !pCurrentLayout->IsSpacingGlyph( mpHash->mpStr[ nIndex ] | GF_ISCHAR ) )
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
				}

				nGlyphsProcessed += nGlyphRunCount;
			}
		}
	}

	// Find positions that require fallback fonts
	for ( nCurrentGlyphRun = 0; nCurrentGlyphRun < nLineGlyphRuns; nCurrentGlyphRun++ )
	{
		CTRunRef aGlyphRun = static_cast< CTRunRef >( CFArrayGetValueAtIndex( aLineGlyphRuns, nCurrentGlyphRun ) );
		if ( aGlyphRun )
		{
			CFIndex nGlyphRunCount = CTRunGetGlyphCount( aGlyphRun );
			CFRange aRange = CTRunGetStringRange( aGlyphRun );
			if ( nGlyphRunCount && aRange.location != kCFNotFound && aRange.length > 0 )
			{
				CFDictionaryRef aDict = CTRunGetAttributes( aGlyphRun );
				if ( aDict )
				{
					// Fix bug 3666 while still using
					// kCTVerticalFormsAttributeName attribute by detecting
					// only assuming there is a fallback font when the font's
					// PostScript names are different
					CTFontRef aFont = static_cast< CTFontRef >( CFDictionaryGetValue( aDict, kCTFontAttributeName ) );
					CFStringRef aFallbackFontPSName = aFont ? CTFontCopyPostScriptName( aFont ) : nullptr;
					CFStringRef aFontPSName = CTFontCopyPostScriptName( maFont );
					bool bHasFallbackFont = ( aFallbackFontPSName && ( !aFontPSName || CFStringCompare( aFallbackFontPSName, aFontPSName, 0 ) != kCFCompareEqualTo ) );
					if ( aFallbackFontPSName )
						CFRelease( aFallbackFontPSName );
					if ( aFontPSName )
						CFRelease( aFontPSName );

					if ( !aFont )
					{
						// No fallback font exists so don't bother looking
					}
					else if ( bHasFallbackFont )
					{
						if ( !mpNeedFallback )
						{
							nBufSize = mpHash->mnLen * sizeof( bool );
							mpNeedFallback = static_cast< bool* >( rtl_allocateMemory( nBufSize ) );
							memset( mpNeedFallback, 0, nBufSize );
						}

						CFIndex j;
						for ( j = 0; j < aRange.length; j++ )
							mpNeedFallback[ aRange.location + j ] = true;

						// Update font for next pass through
						JavaImplFont *pFallbackFont = nullptr;
						SalData *pSalData = GetSalData();
						if ( aFont )
						{
							OUString aPSName;
							CFStringRef aPSString = CTFontCopyPostScriptName( aFont );
							if ( aPSString )
							{
								aPSName = GetOUString( aPSString );
								CFRelease( aPSString );
							}

							::std::unordered_map< OUString, JavaPhysicalFontFace*, OUStringHash >::const_iterator ffit = pSalData->maJavaFontNameMapping.find( aPSName );
							if ( ffit != pSalData->maJavaFontNameMapping.end() )
								pFallbackFont = new JavaImplFont( ffit->second->maFontName, mpHash->mfFontSize, mpFont->getOrientation(), mpHash->mbAntialiased, mpHash->mbVertical, mpHash->mfFontScaleX );
						}

						if ( !pFallbackFont )
						{
							// Look through our application font list for
							// a font that has glyphs for the current char
							for ( ::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* >::const_iterator nit = pSalData->maNativeFontMapping.begin(); !pFallbackFont && nit != pSalData->maNativeFontMapping.end(); nit++ )
							{
								for ( j = 0; j < aRange.length; j++ )
								{
									UniChar nChar = mpHash->mpStr[ aRange.location + j ];
									CGGlyph nGlyph;
									if ( CTFontGetGlyphsForCharacters( reinterpret_cast< CTFontRef >( nit->first ), &nChar, &nGlyph, 1 ) )
									{
										pFallbackFont = new JavaImplFont( nit->second->maFontName, mpHash->mfFontSize, mpFont->getOrientation(), mpHash->mbAntialiased, mpHash->mbVertical, mpHash->mfFontScaleX );
										break;
									}
								}
							}
						}

						if ( pFallbackFont )
						{
							maFallbackFontList.push_back( pFallbackFont );
							if ( !mpFallbackFont )
								mpFallbackFont = pFallbackFont;
						}
						else
						{
							if ( mpNeedFallback )
							{
								rtl_freeMemory( mpNeedFallback );
								mpNeedFallback = nullptr;
							}
						}

						for ( j = 0; j < aRange.length; j++ )
							maFallbackFontMap[ aRange.location + j ] = pFallbackFont;
					}
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

JavaImplFont *ImplATSLayoutData::GetFallbackFont( int nCharPos )
{
	JavaImplFont *pRet = nullptr;

	::std::map< int, JavaImplFont* >::const_iterator it = maFallbackFontMap.find( nCharPos );
	if ( it != maFallbackFontMap.end() )
		pRet = it->second;

	return pRet;
}

// ----------------------------------------------------------------------------

void ImplATSLayoutData::Destroy()
{
	if ( mpHash )
	{
		delete mpHash;
		mpHash = nullptr;
	}

	if ( mpFont )
	{
		delete mpFont;
		mpFont = nullptr;
	}

	if ( mpNeedFallback )
	{
		rtl_freeMemory( mpNeedFallback );
		mpNeedFallback = nullptr;
	}

	while ( maFallbackFontList.size() )
	{
		delete maFallbackFontList.back();
		maFallbackFontList.pop_back();
	}

	maFallbackFontMap.clear();
	mpFallbackFont = nullptr;

	if ( maFont )
	{
		CFRelease( maFont );
		maFont = nullptr;
	}

	if ( maTypesetter )
	{
		CFRelease( maTypesetter );
		maTypesetter = nullptr;
	}

	if ( maLine )
	{
		CFRelease( maLine );
		maLine = nullptr;
	}

	if ( mpGlyphs )
	{
		rtl_freeMemory( mpGlyphs );
		mpGlyphs = nullptr;
	}

	if ( mpGlyphsToChars )
	{
		rtl_freeMemory( mpGlyphsToChars );
		mpGlyphsToChars = nullptr;
	}

	mnGlyphCount = 0;

	if ( mpCharsToChars )
	{
		rtl_freeMemory( mpCharsToChars );
		mpCharsToChars = nullptr;
	}

	if ( mpCharsToGlyphs )
	{
		rtl_freeMemory( mpCharsToGlyphs );
		mpCharsToGlyphs = nullptr;
	}

	if ( mpGlyphAdvances )
	{
		rtl_freeMemory( mpGlyphAdvances );
		mpGlyphAdvances = nullptr;
	}

	mnBaselineDelta = 0;
	mbValid = false;
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
	delete const_cast< ImplATSLayoutData* >( this );
}

// ============================================================================

static void SalCGPathApplier( void *pInfo, const CGPathElement *pElement )
{
	::std::list< tools::Polygon > *pPolygonList = static_cast< ::std::list< tools::Polygon >* >( pInfo );

	switch ( pElement->type )
	{
		case kCGPathElementMoveToPoint:
		{
			Point aPoint( Float32ToLong( pElement->points[ 0 ].x * UNITS_PER_PIXEL ), Float32ToLong( pElement->points[ 0 ].y * -1 * UNITS_PER_PIXEL ) );
			pPolygonList->push_back( tools::Polygon( 1, &aPoint ) );
			break;
		}
		case kCGPathElementAddLineToPoint:
		{
			pPolygonList->back().Insert( pPolygonList->back().GetSize(), Point( Float32ToLong( pElement->points[ 0 ].x * UNITS_PER_PIXEL ), Float32ToLong( pElement->points[ 0 ].y * -1 * UNITS_PER_PIXEL ) ) );
			break;
		}
		case kCGPathElementAddQuadCurveToPoint:
		{
			sal_uInt16 nSize = pPolygonList->back().GetSize();
			if ( nSize )
			{
				Point aStart( pPolygonList->back().GetPoint( nSize - 1 ) );
				Point aOffCurve( Float32ToLong( pElement->points[ 0 ].x * UNITS_PER_PIXEL ), Float32ToLong( pElement->points[ 0 ].y * -1 * UNITS_PER_PIXEL ) );
				Point aEnd( Float32ToLong( pElement->points[ 1 ].x * UNITS_PER_PIXEL ), Float32ToLong( pElement->points[ 1 ].y * -1 * UNITS_PER_PIXEL ) );

				pPolygonList->back().Insert( nSize, aStart );
				pPolygonList->back().SetFlags( nSize++, PolyFlags::Control );
				pPolygonList->back().Insert( nSize, aOffCurve );
				pPolygonList->back().SetFlags( nSize++, PolyFlags::Control );
				pPolygonList->back().Insert( nSize, aEnd );
			}
			break;
		}
		case kCGPathElementAddCurveToPoint:
		{
			Point aStart( Float32ToLong( pElement->points[ 0 ].x * UNITS_PER_PIXEL ), Float32ToLong( pElement->points[ 0 ].y * -1 * UNITS_PER_PIXEL ) );
			Point aOffCurve( Float32ToLong( pElement->points[ 1 ].x * UNITS_PER_PIXEL ), Float32ToLong( pElement->points[ 1 ].y * -1 * UNITS_PER_PIXEL ) );
			Point aEnd( Float32ToLong( pElement->points[ 2 ].x * UNITS_PER_PIXEL ), Float32ToLong( pElement->points[ 2 ].y * -1 * UNITS_PER_PIXEL ) );
			sal_uInt16 nSize = pPolygonList->back().GetSize();
			pPolygonList->back().Insert( nSize, aStart );
			pPolygonList->back().SetFlags( nSize++, PolyFlags::Control );
			pPolygonList->back().Insert( nSize, aOffCurve );
			pPolygonList->back().SetFlags( nSize++, PolyFlags::Control );
			pPolygonList->back().Insert( nSize, aEnd );
			break;
		}
		case kCGPathElementCloseSubpath:
		{
			sal_uInt16 nSize = pPolygonList->back().GetSize();
			if ( nSize > 1 )
				pPolygonList->back().Insert( nSize, Point( pPolygonList->back().GetPoint( 0 ) ) );
			break;
		}
	}
}

// ============================================================================

JavaSalGraphicsDrawGlyphsOp::JavaSalGraphicsDrawGlyphsOp( const CGPathRef aFrameClipPath, const CGPathRef aNativeClipPath, const CGPoint aStartPoint, int nGlyphCount, const GlyphItem **pGlyphs, const DeviceCoordinate *pAdvances, JavaImplFont *pFont, SalColor nColor, int nOrientation, int nGlyphOrientation, DeviceCoordinate fTranslateX, DeviceCoordinate fTranslateY, float fGlyphScaleX ) :
	JavaSalGraphicsOp( aFrameClipPath, aNativeClipPath ),
	maStartPoint( aStartPoint ),
	mnGlyphCount( nGlyphCount ),
	mpGlyphs( nullptr ),
	mpPositions( nullptr ),
	mnFontID( reinterpret_cast< CTFontRef >( pFont->getNativeFont() ) ),
	mfFontSize( pFont->getSize() ),
	mbAntialiased( pFont->isAntialiased() ),
	mnColor( nColor ),
	mfRotateAngle( 0.0f ),
	mfTranslateX( fTranslateX ),
	mfTranslateY( fTranslateY ),
	mfScaleX( 1.0f ),
	mfScaleY( 1.0f )
{
	// Fix bug 2673 by applying font scale here instead of in the native method
	nGlyphOrientation &= GF_ROTMASK;
	if ( nGlyphOrientation & GF_ROTMASK )
	{
		float fRotateDegrees;
		if ( nGlyphOrientation == GF_ROTL )
			fRotateDegrees = 90.0f;
		else
			fRotateDegrees = -90.0f;
		mfRotateAngle += fRotateDegrees * M_PI / 180;
		// Fix scaling the wrong axis by doing all scaling vertically
		mfScaleY *= static_cast< float >( pFont->getScaleX() ) * fGlyphScaleX;
	}
	else
	{
		mfScaleX *= static_cast< float >( pFont->getScaleX() ) * fGlyphScaleX;
	}

	if ( mnGlyphCount > 0 )
	{
		mpGlyphs = static_cast< CGGlyph* >( rtl_allocateMemory( mnGlyphCount * sizeof( CGGlyph ) ) );
		if ( mpGlyphs )
		{
			for ( int i = 0; i < mnGlyphCount; i++ )
				mpGlyphs[ i ] = static_cast< CGGlyph >( pGlyphs[ i ]->maGlyphId & GF_IDXMASK );
		}

		mpPositions = static_cast< CGPoint* >( rtl_allocateMemory( ( mnGlyphCount + 1 )* sizeof( CGPoint ) ) );
		if ( mpPositions )
		{
			mpPositions[ 0 ].x = 0.0f;
			mpPositions[ 0 ].y = 0.0f;
			for ( int i = 1; i <= mnGlyphCount; i++ )
			{
				// Fix font scaling bug reported in 11/13/2015 e-mail to
				// to elcapitanbugs@neooffice.org by unscaling the glyph
				// positions by the same amount of X scale in the context's
				// text matrix
				mpPositions[ i ].x = mpPositions[ i - 1 ].x + ( ( pAdvances[ i - 1 ] / mfScaleX ) / UNITS_PER_PIXEL );
				mpPositions[ i ].y = 0.0f;
			}
		}
	}

	if ( mnFontID )
		CFRetain( mnFontID );

	// Calculate glyph rotation and scale
	if ( nOrientation )
		mfRotateAngle += ( static_cast< float >( nOrientation ) / 10 ) * M_PI / 180;
}

// ----------------------------------------------------------------------------

JavaSalGraphicsDrawGlyphsOp::~JavaSalGraphicsDrawGlyphsOp()
{
	if ( mpGlyphs )
		rtl_freeMemory( mpGlyphs );

	if ( mpPositions )
		rtl_freeMemory( mpPositions );

	if ( mnFontID )
		CFRelease( mnFontID );
}

// ----------------------------------------------------------------------------

void JavaSalGraphicsDrawGlyphsOp::drawOp( JavaSalGraphics *pGraphics, CGContextRef aContext, CGRect aBounds )
{
	if ( !pGraphics || !aContext || !mpGlyphs || !mpPositions )
		return;

	CGRect aDrawBounds = aBounds;
	if ( maFrameClipPath )
		aDrawBounds = CGRectIntersection( aDrawBounds, CGPathGetBoundingBox( maFrameClipPath ) );
	if ( maNativeClipPath )
		aDrawBounds = CGRectIntersection( aDrawBounds, CGPathGetBoundingBox( maNativeClipPath ) );
	if ( CGRectIsEmpty( aDrawBounds ) )
		return;

	CGColorRef aColor = CreateCGColorFromSalColor( mnColor );
	if ( aColor )
	{
		if ( CGColorGetAlpha( aColor ) )
		{
			CTFontRef aFont = CTFontCreateCopyWithAttributes( mnFontID, mfFontSize, nullptr, nullptr );
			if ( aFont )
			{
				// Text draw bounds is never XOR'd so don't pass any bounds
				aContext = saveClipXORGState( pGraphics, aContext );
				if ( aContext )
				{
					// Enable or disable font antialiasing
					CGContextSetAllowsAntialiasing( aContext, mbAntialiased );

					CGContextTranslateCTM( aContext, maStartPoint.x, maStartPoint.y );
					CGContextRotateCTM( aContext, mfRotateAngle );
					CGContextTranslateCTM( aContext, mfTranslateX * mfScaleX, mfTranslateY * mfScaleY );

					// Fix bug 2674 by setting all translation, rotation, and
					// scaling in the CGContext and not in the text matrix.
					// Fix bug 2957 by moving the glyph scale back into the
					// font transform. Fix vertical scaling when scaling the
					// Apple Color Emoji font by greater than 100% without
					// causing bug 2957 to reoccur by drawing using
					// CTFontDrawGlyphs() and scaling in the CGContext
					// instead of in the text matrix.
					CGContextScaleCTM( aContext, mfScaleX, mfScaleY );

					CGContextSetFillColorWithColor( aContext, aColor );
					CGContextSetStrokeColorWithColor( aContext, aColor );

					CTFontDrawGlyphs( aFont, mpGlyphs, mpPositions, mnGlyphCount, aContext );

					// Calculate rough draw bounds including any transformations
					if ( pGraphics->mpFrame )
					{
						float fWidth = 0;
						for ( int i = 0; i <= mnGlyphCount; i++ )
						{
							if ( fWidth < mpPositions[ i ].x )
								fWidth = mpPositions[ i ].x;
						}
						fWidth += mfFontSize * 4;

						float fPadding = mfFontSize * 2;
						CGRect aUntransformedBounds = CGRectMake( maStartPoint.x, maStartPoint.y, fWidth + ( fabs( mfTranslateX ) * mfScaleX ) + fPadding, ( fabs( mfTranslateY ) * mfScaleY ) + ( fPadding * 2 ) );

						// If there is any rotation, expand bounds to cover all
						// possible rotation positions just to be safe
						if ( mfRotateAngle != 0 )
						{
							float fExtent = ( aUntransformedBounds.size.width > aUntransformedBounds.size.height ? aUntransformedBounds.size.width : aUntransformedBounds.size.height );
							aUntransformedBounds = CGRectMake( aUntransformedBounds.origin.x - fExtent, aUntransformedBounds.origin.y - fExtent, fExtent * 2, fExtent * 2 );
						}
						else
						{
							aUntransformedBounds.origin.x -= fPadding;
							aUntransformedBounds.origin.y -= fPadding;
							aUntransformedBounds.size.width += fPadding;
							aUntransformedBounds.size.height += fPadding;
						}

						aDrawBounds = CGRectIntersection( aDrawBounds, aUntransformedBounds );
					}

					restoreClipXORGState();

					if ( pGraphics->mpFrame )
						pGraphics->addNeedsDisplayRect( aDrawBounds, mfLineWidth );
				}

				CFRelease( aFont );
			}
		}

		CGColorRelease( aColor );
	}
}

// ============================================================================

SalLayout *JavaSalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
	if ( nFallbackLevel && rArgs.mnFlags & SalLayoutFlags::DisableGlyphProcessing )
		return nullptr;

	// Fix crash in the CppunitTest_chart2_export unit by returning NULL if
	// this graphic's font has not been set
	JavaImplFont *pFont = nullptr;
	if ( nFallbackLevel )
	{
		::std::unordered_map< int, JavaImplFont* >::const_iterator it = maFallbackFonts.find( nFallbackLevel );
		if ( it != maFallbackFonts.end() && nFallbackLevel < MAX_FALLBACK )
			pFont = it->second;
	}
	else if ( mpFont )
	{
		pFont = mpFont;
	}

	if ( pFont )
		return new SalATSLayout( this, nFallbackLevel, pFont );
	else
		return nullptr;
}

// ============================================================================

void SalATSLayout::GetGlyphBounds( sal_Int32 nGlyph, JavaImplFont *pFont, tools::Rectangle &rRect )
{
	rRect = tools::Rectangle( Point( 0, 0 ), Size( 0, 0 ) );

	if ( pFont )
	{
		CTFontRef aFont = CTFontCreateCopyWithAttributes( reinterpret_cast< CTFontRef >( pFont->getNativeFont() ), pFont->getSize(), nullptr, nullptr );
		if ( aFont )
		{
			CGGlyph nGlyphID = static_cast< CGGlyph >( nGlyph & GF_IDXMASK );
			CGRect aRect = CTFontGetBoundingRectsForGlyphs( aFont, kCTFontOrientationDefault, &nGlyphID, nullptr, 1 );
			if ( !CGRectIsEmpty( aRect ) )
			{
				double fFontScaleX = pFont->getScaleX();
				if ( nGlyph & GF_ROTMASK )
					rRect = tools::Rectangle( Point( Float32ToLong( aRect.origin.x ), Float32ToLong( ( aRect.origin.y + aRect.size.height ) * fFontScaleX * -1 ) ), Size( Float32ToLong( aRect.size.width ), Float32ToLong( aRect.size.height * fFontScaleX ) ) );
				else
					rRect = tools::Rectangle( Point( Float32ToLong( aRect.origin.x * fFontScaleX ), Float32ToLong( ( aRect.origin.y + aRect.size.height ) * -1 ) ), Size( Float32ToLong( aRect.size.width * fFontScaleX ), Float32ToLong( aRect.size.height ) ) );
				rRect.Justify();
			}

			CFRelease( aFont );
		}
	}
}

// ----------------------------------------------------------------------------

void SalATSLayout::ClearLayoutDataCache()
{
	ImplATSLayoutData::ClearLayoutDataCache();
}

// ----------------------------------------------------------------------------

SalATSLayout::SalATSLayout( JavaSalGraphics *pGraphics, int nFallbackLevel, JavaImplFont *pFont ) :
	mpGraphics( pGraphics ),
	mnFallbackLevel( nFallbackLevel ),
	mpFont( new JavaImplFont( pFont ) ),
	mpKashidaLayoutData( nullptr ),
	mfOrigWidth( 0 ),
	mfGlyphScaleX( 1.0 )
{
	SetUnitsPerPixel( UNITS_PER_PIXEL );
}

// ----------------------------------------------------------------------------

SalATSLayout::~SalATSLayout()
{
	Destroy();

	if ( mpFont )
		delete mpFont;
}

// ----------------------------------------------------------------------------

void SalATSLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
	GenericSalLayout::AdjustLayout( rArgs );

	// Fix bug 2133 by scaling width of characters if the new width is narrower
	// than the original width. Fix glyph scaling when drawing a
	// non-proportionally resized embedded spreadsheet during a slideshow
	// without causing bug 2652 to reoccur by using the position array only
	// when the layout width is zero.
	mfGlyphScaleX = 1.0;
	DeviceCoordinate fWidth;
	if ( rArgs.mnLayoutWidth )
		fWidth = rArgs.mnLayoutWidth * UNITS_PER_PIXEL;
	else
		fWidth = mfOrigWidth;

	// Fix bug 2882 by ensuring that the glyph scale is never zero
	if ( fWidth > 0 && mfOrigWidth > 0 )
		mfGlyphScaleX = fWidth / mfOrigWidth;

	if ( rArgs.mnFlags & SalLayoutFlags::KerningAsian && ! ( rArgs.mnFlags & SalLayoutFlags::Vertical ) )
		ApplyAsianKerning( rArgs.mrStr );

	if ( rArgs.mnFlags & SalLayoutFlags::KashidaJustification && rArgs.mpDXArray && mpKashidaLayoutData && mpKashidaLayoutData->mnGlyphCount )
		KashidaJustify( mpKashidaLayoutData->mpGlyphs[ 0 ], mpKashidaLayoutData->mpGlyphAdvances[ 0 ] );
}

// ----------------------------------------------------------------------------

bool SalATSLayout::LayoutText( ImplLayoutArgs& rArgs )
{
	Destroy();

	bool bRet = false;
	rArgs.mnFlags |= SalLayoutFlags::DisableGlyphProcessing;

	if ( !mpFont )
		return bRet;

	JavaImplFont *pSymbolFallbackFont = nullptr;
	bool bUseNativeFallback = false;

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
		while ( nMaxPosChange && nMinCharPos && !IsNonprintingChar( rArgs.mrStr[ nMinCharPos - 1 ] ) && !IsSpacingGlyph( rArgs.mrStr[ nMinCharPos - 1 ] | GF_ISCHAR ) )
		{
			nMinCharPos--;
			nMaxPosChange--;
			bDeleteArgs = true;
		}
		nMaxPosChange = MAXEXTRACHARS;
		while ( nMaxPosChange && nEndCharPos < rArgs.mrStr.getLength() && !IsNonprintingChar( rArgs.mrStr[ nEndCharPos ] ) && !IsSpacingGlyph( rArgs.mrStr[ nEndCharPos ] | GF_ISCHAR ) )
		{
			nEndCharPos++;
			nMaxPosChange--;
			bDeleteArgs = true;
		}

		ImplLayoutArgs *pArgs = &rArgs;

		// If SalLayoutFlags::BiDiStrong is set, we need to verify that the extra
		// characters are of the same direction otherwise typing LTR characters
		// in RTL text will be misrendered
		if ( bDeleteArgs && rArgs.mnFlags & SalLayoutFlags::BiDiStrong )
		{
			// The ImplLayoutArgs class does not do BIDI analysis in this case
			// so we can be assured that the entire string is a single run
			bool bIsStrongRTL = static_cast< bool >( rArgs.mnFlags & SalLayoutFlags::BiDiRtl );

			bool bStrongRunRTL;
			int nStrongMinCharPos;
			int nStrongEndCharPos;

			if ( nMinCharPos < rArgs.mnMinCharPos )
			{
				// Improve BIDI analysis speed by trimming layout args string
				int nMinArgsLen = rArgs.mnMinCharPos - nMinCharPos;
				OUString aMinArgsStr = rArgs.mrStr.copy( nMinCharPos, nMinArgsLen );
				ImplLayoutArgs aMinArgs( aMinArgsStr, 0, nMinArgsLen, rArgs.mnFlags & ~static_cast< SalLayoutFlags >( SalLayoutFlags::BiDiStrong | SalLayoutFlags::DisableGlyphProcessing ), rArgs.maLanguageTag, nullptr );
				aMinArgs.ResetPos();
				while ( aMinArgs.GetNextRun( &nStrongMinCharPos, &nStrongEndCharPos, &bStrongRunRTL ) )
				{
					nStrongMinCharPos += nMinCharPos;
					nStrongEndCharPos += nMinCharPos;
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
				// Improve BIDI analysis speed by trimming layout args string
				int nEndArgsLen = nEndCharPos - rArgs.mnEndCharPos;
				OUString aEndArgsStr = rArgs.mrStr.copy( rArgs.mnEndCharPos, nEndArgsLen );
				ImplLayoutArgs aEndArgs( aEndArgsStr, 0, nEndArgsLen, rArgs.mnFlags & ~static_cast< SalLayoutFlags >( SalLayoutFlags::BiDiStrong | SalLayoutFlags::DisableGlyphProcessing ), rArgs.maLanguageTag, nullptr );
				aEndArgs.ResetPos();
				while ( aEndArgs.GetNextRun( &nStrongMinCharPos, &nStrongEndCharPos, &bStrongRunRTL ) )
				{
					nStrongMinCharPos += rArgs.mnEndCharPos;
					nStrongEndCharPos += rArgs.mnEndCharPos;
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
			pArgs = new ImplLayoutArgs( rArgs.mrStr, nMinCharPos, nEndCharPos, rArgs.mnFlags & ~SalLayoutFlags::DisableGlyphProcessing, rArgs.maLanguageTag, nullptr );
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
					for ( ; i > nMinCharPos && !IsNonprintingChar( pArgs->mrStr[ i - 1 ] ) && !IsSpacingGlyph( pArgs->mrStr[ i - 1 ] | GF_ISCHAR ); i-- )
						;
					for ( ; i > nMinCharPos && ( IsNonprintingChar( pArgs->mrStr[ i - 1 ] ) || IsSpacingGlyph( pArgs->mrStr[ i - 1 ] | GF_ISCHAR ) ); i-- )
						;
					mpGraphics->maFallbackRuns.AddRun( i, nStart, bRunRTL );
					nStart = i;
				}
			}
			else
			{
				int nStart = nMinCharPos;
				while ( nStart < nEndCharPos )
				{
					int i = nStart;
					for ( ; i < nEndCharPos && !IsNonprintingChar( pArgs->mrStr[ i ] ) && !IsSpacingGlyph( pArgs->mrStr[ i ] | GF_ISCHAR ); i++ )
						;
					for ( ; i < nEndCharPos && ( IsNonprintingChar( pArgs->mrStr[ i ] ) || IsSpacingGlyph( pArgs->mrStr[ i ] | GF_ISCHAR ) ); i++ )
						;
					mpGraphics->maFallbackRuns.AddRun( nStart, i, bRunRTL );
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

				if ( bExactMatch )
					break;
			}
		}
	}

	JavaImplFont *pFallbackFont = nullptr;
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
				if ( ( rArgs.mrStr[ i ] >= 0x0600 && rArgs.mrStr[ i ] < 0x0900 ) || ( rArgs.mrStr[ i ] >= 0xfb50 && rArgs.mrStr[ i ] < 0xfe00 ) || ( rArgs.mrStr[ i ] >= 0xfe70 && rArgs.mrStr[ i ] < 0xff00 ) )
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
					mpKashidaLayoutData = ImplATSLayoutData::GetLayoutData( aArabicTest, 3, 0, 3, rArgs.mnFlags | SalLayoutFlags::BiDiStrong | SalLayoutFlags::BiDiRtl, mnFallbackLevel, mpFont, this );
				}

				if ( mpKashidaLayoutData )
				{
					bool bHasArabicFontSupport = true;
					if ( !mpKashidaLayoutData->mnGlyphCount || ( mpKashidaLayoutData->mpNeedFallback && mpKashidaLayoutData->mpFallbackFont ) )
					{
						bHasArabicFontSupport = false;
					}
					else
					{
						// Fix bug 2757 by detecting when a font cannot support
						// Arabic text layout. The characters in our layout
						// should always produce different glyphs for each
						// character so if any are the same, the font does not
						// support Arabic properly.
						for ( int i = 0; i < static_cast< int >( mpKashidaLayoutData->mnGlyphCount ); i++ )
						{
							if ( !mpKashidaLayoutData->mpGlyphs[ i ] )
							{
								bHasArabicFontSupport = false;
								break;
							}
							else if ( i && mpKashidaLayoutData->mpGlyphs[ i ] == mpKashidaLayoutData->mpGlyphs[ i - 1 ] )
							{
								bHasArabicFontSupport = false;
								break;
							}
							else if ( mpKashidaLayoutData->mpGlyphs[ i ] == 0xffff )
							{
								break;
							}
						}
					}

					if ( !bHasArabicFontSupport )
					{
						if ( !pFallbackFont )
						{
							// If there is no fallback font but the font really
							// does not support Arabic (e.g. non-AAT fonts like
							// STIXihei), assign Geeza Pro as this layout's
							// fallback font
							if ( !mpKashidaLayoutData->mpFallbackFont )
							{
								SalData *pSalData = GetSalData();

								::std::map< OUString, JavaPhysicalFontFace* >::const_iterator it = pSalData->maFontNameMapping.find( aGeezaPro );
								if ( it == pSalData->maFontNameMapping.end() )
								{
									it = pSalData->maFontNameMapping.find( aGeezaProRegular );
									if ( it == pSalData->maFontNameMapping.end() )
										it = pSalData->maFontNameMapping.find( aAlBayanPlain );
								}
								if ( it != pSalData->maFontNameMapping.end() )
								{
									JavaImplFont *pFont = new JavaImplFont( it->second->maFontName, mpFont->getSize(), mpFont->getOrientation(), mpFont->isAntialiased(), mpFont->isVertical(), mpFont->getScaleX() );
									if ( pFont )
									{
										if ( pFont->getNativeFont() != mpFont->getNativeFont() )
											mpKashidaLayoutData->mpFallbackFont = pFont;
										else
											delete pFont;
									}
								}
							}

							if ( mpKashidaLayoutData->mpFallbackFont )
								pFallbackFont = mpKashidaLayoutData->mpFallbackFont;
						}

						if ( pFallbackFont )
						{
							for ( int i = nMinCharPos; i < nEndCharPos; i++ )
								rArgs.NeedFallback( i, bRunRTL );
							rArgs.mnFlags &= ~SalLayoutFlags::DisableGlyphProcessing;
						}
					}
					else
					{
						rArgs.mnFlags |= SalLayoutFlags::KashidaJustification;
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
		SalLayoutFlags nRunFlags = rArgs.mnFlags | SalLayoutFlags::BiDiStrong;
		if ( bRunRTL )
			nRunFlags |= SalLayoutFlags::BiDiRtl;
		else
			nRunFlags &= ~SalLayoutFlags::BiDiRtl;

		ImplATSLayoutData *pLayoutData = ImplATSLayoutData::GetLayoutData( rArgs.mrStr.getStr(), rArgs.mrStr.getLength(), nMinFallbackCharPos, nEndFallbackCharPos, nRunFlags, mnFallbackLevel, mpFont, this );
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
					if ( i >= 0 && pLayoutData->mpGlyphs[ i ] == 0xffff )
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
					if ( i >= 0 && pLayoutData->mpGlyphs[ i ] == 0xffff )
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
			pLayoutData = ImplATSLayoutData::GetLayoutData( rArgs.mrStr.getStr(), rArgs.mrStr.getLength(), nMinFallbackCharPos, nEndFallbackCharPos, nRunFlags, mnFallbackLevel, mpFont, this );
			if ( !pLayoutData )
				continue;
		}

		// Create fallback runs
		if ( pLayoutData->mpNeedFallback && pLayoutData->mpFallbackFont )
		{
			for ( int i = nMinCharPos; i < nEndCharPos; i++ )
			{
				if ( pLayoutData->mpNeedFallback[ i - nMinFallbackCharPos ] )
				{
					if ( !pFallbackFont )
						pFallbackFont = pLayoutData->GetFallbackFont( i - nMinFallbackCharPos );
					rArgs.NeedFallback( i, bRunRTL );
				}
			}

			if ( !pFallbackFont )
				pFallbackFont = pLayoutData->mpFallbackFont;

			rArgs.mnFlags &= ~SalLayoutFlags::DisableGlyphProcessing;
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
					sal_Unicode nMirroredChar = static_cast< sal_Unicode >( GetMirroredChar( nChar ) );
					if ( nMirroredChar != nChar )
					{
						::std::unordered_map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.find( nChar );
						if ( mit == maMirroredLayoutData.end() )
						{
							sal_Unicode aMirrored[ 1 ];
							aMirrored[ 0 ] = nMirroredChar;
							pCurrentLayoutData = ImplATSLayoutData::GetLayoutData( aMirrored, 1, 0, 1, ( rArgs.mnFlags & ~SalLayoutFlags::BiDiRtl ) | SalLayoutFlags::BiDiStrong, mnFallbackLevel, mpFont, this );
							if ( pCurrentLayoutData )
							{
								if ( pCurrentLayoutData->mpNeedFallback && pCurrentLayoutData->mpFallbackFont )
								{
									pCurrentLayoutData->Release();
									pCurrentLayoutData = pLayoutData;
									rArgs.NeedFallback( nCharPos, bRunRTL );

									if ( !pFallbackFont )
										pFallbackFont = pCurrentLayoutData->mpFallbackFont;
									rArgs.mnFlags &= ~SalLayoutFlags::DisableGlyphProcessing;
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
 
				for ( CFIndex i = pCurrentLayoutData->mpCharsToGlyphs[ nIndex ]; i >= 0 && i < pCurrentLayoutData->mnGlyphCount && pCurrentLayoutData->mpGlyphsToChars[ i ] == nIndex; i++ )
				{
					long nGlyphWidth = pCurrentLayoutData->mpGlyphAdvances[ i ];
					sal_Int32 nGlyph = static_cast< sal_Int32 >( pCurrentLayoutData->mpGlyphs[ i ] );

					// Fix bug 3588 by setting fallback glyph IDs to zero
					if ( nGlyph && pLayoutData->mpNeedFallback && pLayoutData->mpNeedFallback[ nIndex ] )
						nGlyph = 0;

					if ( !nGlyph )
					{
						if ( nChar < 0x0500 )
						{
							// Fix bug 3087 if there is no fallback font and it
							// is a European or Cyrillic character by using a
							// known font that we can render those ranges
							if ( !pSymbolFallbackFont )
							{
								SalData *pSalData = GetSalData();

								::std::map< OUString, JavaPhysicalFontFace* >::const_iterator it = pSalData->maFontNameMapping.find( mpGraphics->mpFontData->GetFamilyType() == FAMILY_ROMAN ? aTimesRoman : aHelvetica );
								if ( it != pSalData->maFontNameMapping.end() )
								{
									pSymbolFallbackFont = new JavaImplFont( it->second->maFontName, mpFont->getSize(), mpFont->getOrientation(), mpFont->isAntialiased(), mpFont->isVertical(), mpFont->getScaleX() );
									if ( pSymbolFallbackFont->getNativeFont() == mpFont->getNativeFont() )
									{
										delete pSymbolFallbackFont;
										pSymbolFallbackFont = nullptr;
									}
								}
							}

							rArgs.NeedFallback( nCharPos, bRunRTL );
							rArgs.mnFlags &= ~SalLayoutFlags::DisableGlyphProcessing;
						}
						else if ( nChar >= 0x0590 && nChar < 0x0600 )
						{
							// If there is no fallback font and it is a Hebrew
							// character, use a font that can render Hebrew
							if ( !pSymbolFallbackFont )
							{
								SalData *pSalData = GetSalData();

								::std::map< OUString, JavaPhysicalFontFace* >::const_iterator it = pSalData->maFontNameMapping.find( aTimesNewRoman );
								if ( it == pSalData->maFontNameMapping.end() )
									it = pSalData->maFontNameMapping.find( aLucidaGrande );
								if ( it != pSalData->maFontNameMapping.end() )
								{
									pSymbolFallbackFont = new JavaImplFont( it->second->maFontName, mpFont->getSize(), mpFont->getOrientation(), mpFont->isAntialiased(), mpFont->isVertical(), mpFont->getScaleX() );
									if ( pSymbolFallbackFont->getNativeFont() == mpFont->getNativeFont() )
									{
										delete pSymbolFallbackFont;
										pSymbolFallbackFont = nullptr;
									}
								}
							}

							rArgs.NeedFallback( nCharPos, bRunRTL );
							rArgs.mnFlags &= ~SalLayoutFlags::DisableGlyphProcessing;
						}
						else if ( nChar >= 0x0900 && nChar < 0x0c80 )
						{
							// If there is no fallback font and it is an Indic
							// character, use a font that can render Indic
							if ( !pSymbolFallbackFont )
							{
								SalData *pSalData = GetSalData();

								OUString aFirstFontName;
								OUString aSecondFontName;
								if ( nChar < 0x0980 )
								{
									aFirstFontName = aDevanagariMT;
									aSecondFontName = aDevanagariSangamMN;
								}
								else if ( nChar < 0x0a00 )
								{
									aFirstFontName = aBanglaMN;
									aSecondFontName = aBanglaSangamMN;
								}
								else if ( nChar < 0x0a80 )
								{
									aFirstFontName = aGurmukhiMN;
									aSecondFontName = aGurmukhiSangamMN;
								}
								else if ( nChar < 0x0b00 )
								{
									aFirstFontName = aGujaratiMN;
									aSecondFontName = aGujaratiSangamMN;
								}
								else if ( nChar < 0x0b80 )
								{
									aFirstFontName = aOriyaMN;
									aSecondFontName = aOriyaSangamMN;
								}
								else if ( nChar < 0x0c00 )
								{
									aFirstFontName = aTamilMN;
									aSecondFontName = aTamilSangamMN;
								}
								else if ( nChar < 0x0c80 )
								{
									aFirstFontName = aTeluguMN;
									aSecondFontName = aTeluguSangamMN;
								}

								::std::map< OUString, JavaPhysicalFontFace* >::const_iterator it = pSalData->maFontNameMapping.find( aFirstFontName );
								if ( it == pSalData->maFontNameMapping.end() )
									it = pSalData->maFontNameMapping.find( aSecondFontName );
								if ( it != pSalData->maFontNameMapping.end() )
								{
									pSymbolFallbackFont = new JavaImplFont( it->second->maFontName, mpFont->getSize(), mpFont->getOrientation(), mpFont->isAntialiased(), mpFont->isVertical(), mpFont->getScaleX() );
									if ( pSymbolFallbackFont->getNativeFont() == mpFont->getNativeFont() )
									{
										delete pSymbolFallbackFont;
										pSymbolFallbackFont = nullptr;
									}
								}
							}

							rArgs.NeedFallback( nCharPos, bRunRTL );
							rArgs.mnFlags &= ~SalLayoutFlags::DisableGlyphProcessing;
						}
						else if ( nChar >= 0x2600 && nChar < 0x2700 )
						{
							// If there is no fallback font and it is a
							// miscellaneous symbol character, use a known font
							// other than the Apple Color Emoji font that can
							// render those ranges
							if ( !pSymbolFallbackFont )
							{
								SalData *pSalData = GetSalData();

								::std::map< OUString, JavaPhysicalFontFace* >::const_iterator it = pSalData->maFontNameMapping.find( aAppleSymbols );
								if ( it == pSalData->maFontNameMapping.end() )
									it = pSalData->maFontNameMapping.find( aArialUnicodeMS );
								if ( it != pSalData->maFontNameMapping.end() )
								{
									pSymbolFallbackFont = new JavaImplFont( it->second->maFontName, mpFont->getSize(), mpFont->getOrientation(), mpFont->isAntialiased(), mpFont->isVertical(), mpFont->getScaleX() );
									if ( pSymbolFallbackFont->getNativeFont() == mpFont->getNativeFont() )
									{
										delete pSymbolFallbackFont;
										pSymbolFallbackFont = nullptr;
									}
								}
							}

							rArgs.NeedFallback( nCharPos, bRunRTL );
							rArgs.mnFlags &= ~SalLayoutFlags::DisableGlyphProcessing;
						}
						else if ( nChar >= 0xe000 && nChar < 0xf900 )
						{
							// If there is no fallback font and it is a Private
							// Use Area character, use the symbol font
							if ( !pSymbolFallbackFont )
							{
								SalData *pSalData = GetSalData();

								::std::map< OUString, JavaPhysicalFontFace* >::const_iterator it = pSalData->maFontNameMapping.find( aOpenSymbol );
								if ( it != pSalData->maFontNameMapping.end() )
								{
									pSymbolFallbackFont = new JavaImplFont( it->second->maFontName, mpFont->getSize(), mpFont->getOrientation(), mpFont->isAntialiased(), mpFont->isVertical(), mpFont->getScaleX() );
									if ( pSymbolFallbackFont->getNativeFont() == mpFont->getNativeFont() )
									{
										delete pSymbolFallbackFont;
										pSymbolFallbackFont = nullptr;
									}
								}
							}

							rArgs.NeedFallback( nCharPos, bRunRTL );
							rArgs.mnFlags &= ~SalLayoutFlags::DisableGlyphProcessing;
						}
						else if ( nChar >= 0x3000 && ( nChar < 0x3100 || ( nChar >= 0x31f0 && nChar < 0x3200 ) || ( nChar >= 0x3300 && nChar < 0x4dc0 ) || ( nChar >= 0x4e00 && nChar < 0xa000 ) || ( nChar >= 0xf900 && nChar < 0xfaff ) || ( nChar >= 0xfe30 && nChar < 0xfe50 ) || ( nChar >= 0xff00 && nChar < 0xfff0 ) ) )
						{
							// Fix bugs 2772 and 3097 if there is no fallback
							// font and it is a CJK character by using a
							// font that can render those ranges
							if ( !pSymbolFallbackFont )
							{
								SalData *pSalData = GetSalData();

								::std::map< OUString, JavaPhysicalFontFace* >::const_iterator it = pSalData->maFontNameMapping.find( mpGraphics->mpFontData->GetFamilyType() == FAMILY_ROMAN ? aHiraginoMinchoProW3 : aHiraginoKakuGothicProW3 );
								if ( it == pSalData->maFontNameMapping.end() )
									it = pSalData->maFontNameMapping.find( aHeitiSCMedium );
								if ( it != pSalData->maFontNameMapping.end() )
								{
									pSymbolFallbackFont = new JavaImplFont( it->second->maFontName, mpFont->getSize(), mpFont->getOrientation(), mpFont->isAntialiased(), mpFont->isVertical(), mpFont->getScaleX() );
									if ( pSymbolFallbackFont->getNativeFont() == mpFont->getNativeFont() )
									{
										delete pSymbolFallbackFont;
										pSymbolFallbackFont = nullptr;
									}
								}
							}

							rArgs.NeedFallback( nCharPos, bRunRTL );
							rArgs.mnFlags &= ~SalLayoutFlags::DisableGlyphProcessing;
						}
						else
						{
							// Fix bug 2091 by suppressing zero glyphs if there
							// is a fallback font
							if ( !pFallbackFont )
							{
								pFallbackFont = pCurrentLayoutData->GetFallbackFont( nIndex );
								if ( !pFallbackFont )
									pFallbackFont = pCurrentLayoutData->mpFallbackFont;
							}

							if ( pFallbackFont )
							{
								rArgs.NeedFallback( nCharPos, bRunRTL );
								rArgs.mnFlags &= ~SalLayoutFlags::DisableGlyphProcessing;
							}
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

					// Prevent drawing of zero glyphs on top of fallback glyphs
					// that occurs with case like Hiragino Mincho Pro character
					// 0x301E by not applying vertical flags to zero glyphs
					if ( nGlyph && pCurrentLayoutData->mpHash->mbVertical )
						nGlyph |= GetVerticalFlags( nChar );

					int nGlyphFlags = bFirstGlyph ? 0 : GlyphItem::IS_IN_CLUSTER;
					if ( pCurrentLayoutData->mpHash->mbVertical )
						nGlyphFlags |= GlyphItem::IS_VERTICAL;

					// Mark spacing characters for LibO code
					if ( IsSpacingGlyph( nGlyph ) )
						nGlyphFlags |= GlyphItem::IS_SPACING;

					if ( bRunRTL )
					{
						nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;

						// Note characters that we can append kashidas onto
						if ( bFirstGlyph && rArgs.mnFlags & SalLayoutFlags::KashidaJustification && nIndex )
						{
							// Fix reoccurrence of bug 823 by setting the
							// previous index from the current index instead of
							// the char position
							int nPreviousIndex = nIndex - 1;
							if ( nPreviousIndex >= 0 )
							{
								UJoiningType nTypeLeft = static_cast< UJoiningType >( u_getIntPropertyValue( nChar, UCHAR_JOINING_TYPE ) );
								UJoiningType nTypeRight = static_cast< UJoiningType >( u_getIntPropertyValue( pCurrentLayoutData->mpHash->mpStr[ nPreviousIndex ], UCHAR_JOINING_TYPE ) );
								if ( ( nTypeLeft == U_JT_RIGHT_JOINING || nTypeLeft == U_JT_DUAL_JOINING ) && ( nTypeRight == U_JT_LEFT_JOINING || nTypeRight == U_JT_DUAL_JOINING || nTypeRight == U_JT_TRANSPARENT ) )
									nGlyphFlags |= GlyphItem::ALLOW_KASHIDA;
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
				sal_Int32 nGlyph = 0x0020 | GF_ISCHAR;
				int nGlyphFlags = bRunRTL ? GlyphItem::IS_RTL_GLYPH : 0;

				// Mark spacing characters for LibO code
				if ( IsSpacingGlyph( nGlyph ) )
					nGlyphFlags |= GlyphItem::IS_SPACING;

				AppendGlyph( GlyphItem( nCharPos, nGlyph, aPos, nGlyphFlags, 0 ) );
				bRet = true;
			}
		}

		maLayoutData.push_back( pLayoutData );
		maLayoutMinCharPos.push_back( nMinFallbackCharPos );
	}

	mfOrigWidth = aPos.X();

	// Set fallback font
	if ( pFallbackFont || pSymbolFallbackFont || ! ( rArgs.mnFlags & SalLayoutFlags::DisableGlyphProcessing ) )
	{
		SalData *pSalData = GetSalData();

		// If this is the first fallback, first try using a font that most
		// closely matches the currently requested font
		JavaPhysicalFontFace *pHighScoreFontData = nullptr;
		::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* >::const_iterator nfit = pSalData->maNativeFontMapping.find( pSymbolFallbackFont ? pSymbolFallbackFont->getNativeFont() : 0 );
		if ( nfit != pSalData->maNativeFontMapping.end() )
			pHighScoreFontData = nfit->second;
		
		if ( !pHighScoreFontData && !bUseNativeFallback && !mnFallbackLevel && ( !mpKashidaLayoutData || !mpKashidaLayoutData->mpFallbackFont ) )
		{
			::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* >::const_iterator it = pSalData->maNativeFontMapping.find( pFallbackFont ? pFallbackFont->getNativeFont() : 0 );
			if ( it == pSalData->maNativeFontMapping.end() || it->second->GetFamilyType() != mpGraphics->mnFontFamily || it->second->GetWeight() != mpGraphics->mnFontWeight || ( it->second->GetItalic() == ITALIC_OBLIQUE || it->second->GetItalic() == ITALIC_NORMAL ? true : false ) != mpGraphics->mbFontItalic || it->second->GetPitch() != mpGraphics->mnFontPitch )
			{
				sal_uInt16 nHighScore = 0;
				sal_IntPtr nNativeFont = mpFont->getNativeFont();
				for ( it = pSalData->maNativeFontMapping.begin(); it != pSalData->maNativeFontMapping.end(); ++it )
				{
					if ( it->first == nNativeFont )
						continue;

					sal_uInt16 nScore = ( ( it->second->GetItalic() == ITALIC_OBLIQUE || it->second->GetItalic() == ITALIC_NORMAL ? true : false ) == mpGraphics->mbFontItalic ? 8 : 0 );
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
		::std::unordered_map< int, JavaImplFont* >::iterator it = mpGraphics->maFallbackFonts.find( nNextLevel );
		if ( it != mpGraphics->maFallbackFonts.end() )
		{
			delete it->second;
			mpGraphics->maFallbackFonts.erase( it );
		}

		// Use the kashida fallback font first so that we are assured of
		// rendering a kashida if needed
		if ( mpKashidaLayoutData && mpKashidaLayoutData->mpFallbackFont )
			mpGraphics->maFallbackFonts[ nNextLevel ] = new JavaImplFont( mpKashidaLayoutData->mpFallbackFont );
		else if ( pHighScoreFontData )
			mpGraphics->maFallbackFonts[ nNextLevel ] = new JavaImplFont( pHighScoreFontData->maFontName, mpFont->getSize(), mpFont->getOrientation(), mpFont->isAntialiased(), mpFont->isVertical(), mpFont->getScaleX() );
		else if ( pFallbackFont )
			mpGraphics->maFallbackFonts[ nNextLevel ] = new JavaImplFont( pFallbackFont );
		else
			rArgs.mnFlags |= SalLayoutFlags::DisableGlyphProcessing;
	}

	return bRet;
}

// ----------------------------------------------------------------------------

void SalATSLayout::DrawText( SalGraphics& rGraphics ) const
{
	const int nMaxGlyphs = 256;
	const GlyphItem *pGlyphs[ nMaxGlyphs ] = { nullptr };
	DeviceCoordinate aDXArray[ nMaxGlyphs ];

	Point aPos;
	JavaSalGraphics& rJavaGraphics = static_cast< JavaSalGraphics& >( rGraphics );
	int nFetchGlyphCount = nMaxGlyphs;
	for ( int nStart = 0; ; )
	{
		int nOldStart = nStart;
		int nTotalGlyphCount = GetNextGlyphs( nFetchGlyphCount, pGlyphs, aPos, nStart, aDXArray );

		if ( !nTotalGlyphCount )
			break;

		// The GenericSalLayout class should return glyph runs with the same
		// rotation mask
		sal_Int32 nGlyphOrientation = pGlyphs[ 0 ]->maGlyphId & GF_ROTMASK;
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
			for ( ; nStartGlyph < nTotalGlyphCount && pGlyphs[ nStartGlyph ]->maGlyphId & GF_ISCHAR; nStartGlyph++ )
				aCurrentPos.X() += Float32ToLong( aDXArray[ nStartGlyph ] );

			// Determine glyph count but only allow one glyph at a time for
			// rotated glyphs
			Point aStartPos( aCurrentPos );
			if ( nStartGlyph < nTotalGlyphCount )
			{
				if ( nGlyphOrientation )
				{
					nGlyphCount++;
					aCurrentPos.Y() += Float32ToLong( aDXArray[ nStartGlyph ] );
				}
				else
				{
					for ( i = nStartGlyph; i < nTotalGlyphCount && ! ( pGlyphs[ i ]->maGlyphId & GF_ISCHAR ); i++ )
					{
						nGlyphCount++;
						aCurrentPos.X() += Float32ToLong( aDXArray[ i ] );
					}
				}
			}

			nCurrentGlyph = nStartGlyph + nGlyphCount;
			if ( !nGlyphCount )
				continue;

			DeviceCoordinate fTranslateX = 0;
			DeviceCoordinate fTranslateY = 0;

			if ( nGlyphOrientation )
			{
				// Don't apply font scale to fix vertical misplacement when
				// using scaled text
				DeviceCoordinate fX;
				DeviceCoordinate fY;
				GetVerticalGlyphTranslation( pGlyphs[ nStartGlyph ]->maGlyphId, pGlyphs[ nStartGlyph ]->mnCharPos, fX, fY );
				if ( nGlyphOrientation == GF_ROTL )
				{
					fTranslateX = fX;
					fTranslateY = fY;
				}
				else
				{
					fTranslateX = fX;
					fTranslateY = aDXArray[ nStartGlyph ] - fY;
				}
			}

			fTranslateX /= UNITS_PER_PIXEL;
			fTranslateY /= UNITS_PER_PIXEL * -1.0f;

			CGPoint aUnflippedStartPoint = UnflipFlippedPoint( CGPointMake( static_cast< float >( aStartPos.X() ), static_cast< float >( aStartPos.Y() ) ), rJavaGraphics.maNativeBounds );
			rJavaGraphics.addUndrawnNativeOp( new JavaSalGraphicsDrawGlyphsOp( rJavaGraphics.maFrameClipPath, rJavaGraphics.maNativeClipPath, aUnflippedStartPoint, nGlyphCount, pGlyphs + nStartGlyph, aDXArray + nStartGlyph, mpFont, rJavaGraphics.mnTextColor, GetOrientation(), nGlyphOrientation, fTranslateX, fTranslateY, mfGlyphScaleX ) );
		}
	}
}

// ----------------------------------------------------------------------------

bool SalATSLayout::GetBoundRect( SalGraphics& /* rGraphics */, tools::Rectangle& rRect ) const
{
	rRect.SetEmpty();

	const int nMaxGlyphs = 256;
	const GlyphItem *pGlyphs[ nMaxGlyphs ] = { nullptr };
	DeviceCoordinate aDXArray[ nMaxGlyphs ];

	Point aPos;
	int nFetchGlyphCount = nMaxGlyphs;
	for ( int nStart = 0; ; )
	{
		int nOldStart = nStart;
		int nTotalGlyphCount = GetNextGlyphs( nFetchGlyphCount, pGlyphs, aPos, nStart, aDXArray );

		if ( !nTotalGlyphCount )
			break;

		// The GenericSalLayout class should return glyph runs with the same
		// rotation mask
		sal_Int32 nGlyphOrientation = pGlyphs[ 0 ]->maGlyphId & GF_ROTMASK;
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
			for ( ; nStartGlyph < nTotalGlyphCount && pGlyphs[ nStartGlyph ]->maGlyphId & GF_ISCHAR; nStartGlyph++ )
				aCurrentPos.X() += Float32ToLong( aDXArray[ nStartGlyph ] );

			// Determine glyph count but only allow one glyph at a time for
			// rotated glyphs
			Point aStartPos( aCurrentPos );
			if ( nStartGlyph < nTotalGlyphCount )
			{
				if ( nGlyphOrientation )
				{
					nGlyphCount++;
					aCurrentPos.Y() += Float32ToLong( aDXArray[ nStartGlyph ] );
				}
				else
				{
					for ( i = nStartGlyph; i < nTotalGlyphCount && ! ( pGlyphs[ i ]->maGlyphId & GF_ISCHAR ); i++ )
					{
						nGlyphCount++;
						aCurrentPos.X() += Float32ToLong( aDXArray[ i ] );
					}
				}
			}

			nCurrentGlyph = nStartGlyph + nGlyphCount;
			if ( !nGlyphCount )
				continue;

			DeviceCoordinate fTranslateX = 0;
			DeviceCoordinate fTranslateY = 0;

			if ( nGlyphOrientation )
			{
				// Don't apply font scale to fix vertical misplacement when
				// using scaled text
				DeviceCoordinate fX;
				DeviceCoordinate fY;
				GetVerticalGlyphTranslation( pGlyphs[ nStartGlyph ]->maGlyphId, pGlyphs[ nStartGlyph ]->mnCharPos, fX, fY );
				if ( nGlyphOrientation == GF_ROTL )
				{
					fTranslateX = fX;
					fTranslateY = fY;
				}
				else
				{
					fTranslateX = fX;
					fTranslateY = aDXArray[ nStartGlyph ] - fY;
				}
			}

			fTranslateX /= UNITS_PER_PIXEL;
			fTranslateY /= UNITS_PER_PIXEL;
			float fScale = static_cast< float >( mpFont->getScaleX() ) * mfGlyphScaleX;

			// Fix bug 3578 by using layout's draw position
			CGPoint aBoundPoint = CGPointMake( aStartPos.X(), aStartPos.Y() );
			CGRect aBoundRect = CGRectNull;
			CTFontRef aFont = CTFontCreateCopyWithAttributes( reinterpret_cast< CTFontRef >( mpFont->getNativeFont() ), mpFont->getSize(), nullptr, nullptr );
			if ( aFont )
			{
				for ( i = nStartGlyph; i < nCurrentGlyph; i++ )
				{
					CGGlyph nGlyphID = static_cast< CGGlyph >( pGlyphs[ i ]->maGlyphId & GF_IDXMASK );
					CGRect aGlyphRect = CTFontGetBoundingRectsForGlyphs( aFont, kCTFontOrientationDefault, &nGlyphID, nullptr, 1 );
					if ( !CGRectIsEmpty( aGlyphRect ) )
					{
						if ( nGlyphOrientation == GF_ROTL )
							aGlyphRect = CGRectMake( ( fTranslateY - aGlyphRect.origin.y - aGlyphRect.size.height ) * fScale, ( fTranslateX + aGlyphRect.origin.x + aGlyphRect.size.width ) * -1, aGlyphRect.size.height * fScale, aGlyphRect.size.width );
						else if ( nGlyphOrientation == GF_ROTR )
						{
							aGlyphRect = CGRectMake( ( ( fTranslateY * -1 ) + aGlyphRect.origin.y ) * fScale, fTranslateX + aGlyphRect.origin.x, aGlyphRect.size.height * fScale, aGlyphRect.size.width );
						}
						else
							aGlyphRect = CGRectMake( ( fTranslateX + aGlyphRect.origin.x ) * fScale, fTranslateY + ( ( aGlyphRect.origin.y + aGlyphRect.size.height ) * -1 ), aGlyphRect.size.width * fScale, aGlyphRect.size.height );

						aGlyphRect.origin.x += aBoundPoint.x;
						aGlyphRect.origin.y += aBoundPoint.y;
						aBoundRect = CGRectUnion( aBoundRect, aGlyphRect );
					}

					aBoundPoint.x += aDXArray[ i ] / UNITS_PER_PIXEL;
				}

				if ( !CGRectIsEmpty( aBoundRect ) )
				{
					tools::Rectangle aRect( Point( Float32ToLong( aBoundRect.origin.x ), Float32ToLong( aBoundRect.origin.y ) ), Size( Float32ToLong( aBoundRect.size.width ), Float32ToLong( aBoundRect.size.height ) ) );
					aRect.Justify();
					rRect.Union( aRect );
				}

				CFRelease( aFont );
			}
		}
	}

	// Fix bug 2191 by always returning true so that the OOo code doesn't
	// execute its "draw the glyph and see which pixels are black" code
	return true;
}

// ----------------------------------------------------------------------------

bool SalATSLayout::GetOutline( SalGraphics& /* rGraphics */, B2DPolyPolygonVector& rVector ) const
{
	const int nMaxGlyphs = 256;
	const GlyphItem *pGlyphs[ nMaxGlyphs ] = { nullptr };
	DeviceCoordinate aDXArray[ nMaxGlyphs ];

	Point aPos;
	int nFetchGlyphCount = nMaxGlyphs;
	for ( int nStart = 0; ; )
	{
		int nOldStart = nStart;
		int nTotalGlyphCount = GetNextGlyphs( nFetchGlyphCount, pGlyphs, aPos, nStart, aDXArray );

		if ( !nTotalGlyphCount )
			break;

		// The GenericSalLayout class should return glyph runs with the same
		// rotation mask
		sal_Int32 nGlyphOrientation = pGlyphs[ 0 ]->maGlyphId & GF_ROTMASK;
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
			for ( ; nStartGlyph < nTotalGlyphCount && pGlyphs[ nStartGlyph ]->maGlyphId & GF_ISCHAR; nStartGlyph++ )
				aCurrentPos.X() += Float32ToLong( aDXArray[ nStartGlyph ] );

			// Determine glyph count but only allow one glyph at a time for
			// rotated glyphs
			Point aStartPos( aCurrentPos );
			if ( nStartGlyph < nTotalGlyphCount )
			{
				if ( nGlyphOrientation )
				{
					nGlyphCount++;
					aCurrentPos.Y() += Float32ToLong( aDXArray[ nStartGlyph ] );
				}
				else
				{
					for ( i = nStartGlyph; i < nTotalGlyphCount && ! ( pGlyphs[ i ]->maGlyphId & GF_ISCHAR ); i++ )
					{
						nGlyphCount++;
						aCurrentPos.X() += Float32ToLong( aDXArray[ i ] );
					}
				}
			}

			nCurrentGlyph = nStartGlyph + nGlyphCount;
			if ( !nGlyphCount )
				continue;

			DeviceCoordinate fTranslateX = 0;
			DeviceCoordinate fTranslateY = 0;

			if ( nGlyphOrientation )
			{
				// Don't apply font scale to fix vertical misplacement when
				// using scaled text
				DeviceCoordinate fX;
				DeviceCoordinate fY;
				GetVerticalGlyphTranslation( pGlyphs[ nStartGlyph ]->maGlyphId, pGlyphs[ nStartGlyph ]->mnCharPos, fX, fY );
				if ( nGlyphOrientation == GF_ROTL )
				{
					fTranslateX = fX;
					fTranslateY = fY;
				}
				else
				{
					fTranslateX = fX;
					fTranslateY = aDXArray[ nStartGlyph ] - fY;
				}
			}

			double fScale = mpFont->getScaleX() * mfGlyphScaleX;

			// Calculate position of glyphs relative to 0, 0
			Point aBoundPos( aStartPos - GetDrawPosition() );
			CGPoint aBoundPoint = CGPointMake( aBoundPos.X() * UNITS_PER_PIXEL, aBoundPos.Y() * UNITS_PER_PIXEL );
			CTFontRef aFont = CTFontCreateCopyWithAttributes( reinterpret_cast< CTFontRef >( mpFont->getNativeFont() ), mpFont->getSize(), nullptr, nullptr );
			if ( aFont )
			{
				for ( i = nStartGlyph; i < nCurrentGlyph; i++ )
				{
					// Fix bug 2390 by ignoring the value of nErr passed by
					// reference
					CGGlyph nGlyphID = static_cast< CGGlyph >( pGlyphs[ i ]->maGlyphId & GF_IDXMASK );
					CGPathRef aPath = CTFontCreatePathForGlyph( aFont, nGlyphID, nullptr );
					if ( aPath )
					{
						::std::list< tools::Polygon > aPolygonList;
						CGPathApply( aPath, static_cast< void* >( &aPolygonList ), SalCGPathApplier );
						CGPathRelease( aPath );

						tools::PolyPolygon aPolyPolygon;
						while ( aPolygonList.size() )
						{
							aPolyPolygon.Insert( aPolygonList.front() );
							aPolygonList.pop_front();
						}

						// Fix bug 2537 by ignoring unusual bounds
						tools::Rectangle aRect = aPolyPolygon.GetBoundRect();
						if ( aRect.GetWidth() <= 0 || aRect.GetHeight() <= 0 )
							continue;

						// Do not apply any rotation for vertical glyphs as the
						// OOo code will rotate the polypolygon
						aPolyPolygon.Move( fTranslateX, fTranslateY );

						if ( nGlyphOrientation )
							aPolyPolygon.Scale( 1.0f, fScale );
						else
							aPolyPolygon.Scale( fScale, 1.0f );

						// Move to position after scaling of glyph
						aPolyPolygon.Move( Float32ToLong( aBoundPoint.x ), Float32ToLong( aBoundPoint.y ) );

						rVector.push_back( aPolyPolygon.getB2DPolyPolygon() );
					}

					aBoundPoint.x += aDXArray[ i ];
				}

				CFRelease( aFont );
			}
		}
	}

	// Always return true so that the OOo code doesn't execute its
	// "draw the glyph and see which pixels are black" code
	return true;
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
		mpKashidaLayoutData = nullptr;
	}

	if ( maMirroredLayoutData.size() )
	{
		for ( ::std::unordered_map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.begin(); mit != maMirroredLayoutData.end(); ++mit )
			mit->second->Release();
		maMirroredLayoutData.clear();
	}

	mfOrigWidth = 0;
	mfGlyphScaleX = 1.0;
}

// ----------------------------------------------------------------------------

ImplATSLayoutData *SalATSLayout::GetVerticalGlyphTranslation( sal_Int32 nGlyph, int nCharPos, DeviceCoordinate& fX, DeviceCoordinate& fY ) const
{
	ImplATSLayoutData *pRet = nullptr;

	fX = 0;
	fY = 0;

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
			pLayoutData = nullptr;
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
		::std::unordered_map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.find( nChar );
		if ( mit != maMirroredLayoutData.end() )
		{
			pRet = mit->second;
			nIndex = 0;
		}
	}

	sal_Int32 nGlyphOrientation = nGlyph & GF_ROTMASK;

	if ( pRet->mpHash->mbVertical && nGlyphOrientation & GF_ROTMASK )
	{
		GlyphID nGlyphID = static_cast< GlyphID >( nGlyph & GF_IDXMASK );

		::std::unordered_map< GlyphID, Point >::const_iterator it = pRet->maVerticalGlyphTranslations.find( nGlyphID );
		if ( it == pRet->maVerticalGlyphTranslations.end() )
		{
			CGGlyph aGlyph = static_cast< CGGlyph >( nGlyphID );
			CGSize aTranslation = CGSizeMake( 0, 0 );
			CTFontGetVerticalTranslationsForGlyphs( pRet->maFont, &aGlyph, &aTranslation, 1 );

			fX = aTranslation.width * UNITS_PER_PIXEL;
			if ( nGlyphOrientation == GF_ROTL )
				fX += pRet->mnBaselineDelta;
			else
				fX -= pRet->mnBaselineDelta;
			fY = aTranslation.height * -1 * UNITS_PER_PIXEL;
		}
		else
		{
			fX = it->second.X();
			fY = it->second.Y();
		}
	}

	return pRet;
}

// ----------------------------------------------------------------------------

DeviceCoordinate SalATSLayout::GetNativeGlyphWidth( sal_Int32 nGlyph, int nCharPos ) const
{
	DeviceCoordinate nRet = 0;

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
			pLayoutData = nullptr;
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
		::std::unordered_map< sal_Unicode, ImplATSLayoutData* >::const_iterator mit = maMirroredLayoutData.find( nChar );
		if ( mit != maMirroredLayoutData.end() )
		{
			pLayoutData = mit->second;
			nIndex = 0;
		}
	}

	GlyphID nGlyphID = static_cast< GlyphID >( nGlyph & GF_IDXMASK );

	::std::unordered_map< GlyphID, long >::const_iterator it = pLayoutData->maNativeGlyphWidths.find( nGlyphID );
	if ( it == pLayoutData->maNativeGlyphWidths.end() )
	{
		CGGlyph aGlyph = static_cast< CGGlyph >( nGlyphID );
		nRet = CTFontGetAdvancesForGlyphs( pLayoutData->maFont, kCTFontOrientationHorizontal, &aGlyph, nullptr, 1 ) * UNITS_PER_PIXEL;
		pLayoutData->maNativeGlyphWidths[ nGlyphID ] = nRet;
	}
	else
	{
		nRet = it->second;
	}

	return nRet;
}
