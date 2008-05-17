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
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 Planamesa Inc.
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
 ************************************************************************/

#define _SV_SALGDI3_CXX

#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALATSLAYOUT_HXX
#include <salatslayout.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_H
#include <salinst.h>
#endif
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif
#ifndef _SV_IMPFONT_HXX
#include <impfont.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <rtl/process.h>
#endif
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#include "salgdi3_cocoa.h"

#ifndef DLLPOSTFIX
#error DLLPOSTFIX must be defined in makefile.mk
#endif

#define DOSTRING( x )			#x
#define STRING( x )				DOSTRING( x )

typedef void NativeShutdownCancelledHandler_Type();

static EventLoopTimerUPP pLoadNativeFontsTimerUPP = NULL;
static ::osl::Condition aLoadNativeFontsCondition;
static ::vos::OModule aShutdownCancelledHandlerModule;
static NativeShutdownCancelledHandler_Type *pShutdownCancelledHandler = NULL;

using namespace basegfx;
using namespace rtl;
using namespace vcl;
using namespace vos;

// ============================================================================

static void ImplFontListChangedCallback( ATSFontNotificationInfoRef aInfo, void *pData )
{
	if ( !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			SalData *pSalData = GetSalData();

			// Clean out caches
			for ( ::std::map< String, JavaImplFontData* >::const_iterator fnit = pSalData->maFontNameMapping.begin(); fnit != pSalData->maFontNameMapping.end(); ++fnit )
				delete fnit->second;
			pSalData->maFontNameMapping.clear();
			pSalData->maNativeFontMapping.clear();
			pSalData->maJavaFontNameMapping.clear();
			pSalData->maJavaNativeFontMapping.clear();
			SalATSLayout::ClearLayoutDataCache();

			if ( !Application::IsShutDown() )
			{
				VCLThreadAttach t;
				if ( t.pEnv )
				{
					// Update cached fonts
					long *pFonts = NSFontManager_getAllFonts();
					if ( pFonts )
					{
						const OUString aGothic( OUString::createFromAscii( "Gothic" ) );
						const OUString aLastResort( OUString::createFromAscii( "LastResort" ) );
						const OUString aRoman( OUString::createFromAscii( "Roman" ) );
						const OUString aSans( OUString::createFromAscii( "Sans" ) );
						const OUString aSerif( OUString::createFromAscii( "Serif" ) );
						const OUString aSymbol( OUString::createFromAscii( "Symbol" ) );
						const OUString aNeoSymbol( OUString::createFromAscii( "Neo Symbol" ) );
						const OUString aOpenSymbol( OUString::createFromAscii( "OpenSymbol" ) );
						const OUString aStarSymbol( OUString::createFromAscii( "StarSymbol" ) );
						const OUString aTimes( OUString::createFromAscii( "Times" ) );
						const OUString aTimesRoman( OUString::createFromAscii( "Times Roman" ) );
						for ( int i = 0; pFonts[ i ]; i++ )
						{
							void *pNSFont = (void *)pFonts[ i ];

							ATSFontRef aFont = NSFont_getATSFontRef( pNSFont );
							if ( !aFont )
								continue;

							// Get font attributes
							FontWeight nWeight = (FontWeight)NSFontManager_weightOfFont( pNSFont );
							FontItalic nItalic = ( NSFontManager_isItalic( pNSFont ) ? ITALIC_NORMAL : ITALIC_NONE );
							FontWidth nWidth = (FontWidth)NSFontManager_widthOfFont( pNSFont );
							FontPitch nPitch = ( NSFontManager_isFixedPitch( pNSFont ) ? PITCH_FIXED : PITCH_VARIABLE );

							CFStringRef aPSString;
							if ( ATSFontGetPostScriptName( aFont, kATSOptionFlagsDefault, &aPSString ) != noErr )
								continue;

							OUString aPSName;
							if ( aPSString )
							{
								CFIndex nPSLen = CFStringGetLength( aPSString );
								CFRange aPSRange = CFRangeMake( 0, nPSLen );
								sal_Unicode pPSBuffer[ nPSLen + 1 ];
								CFStringGetCharacters( aPSString, aPSRange, pPSBuffer );
								pPSBuffer[ nPSLen ] = 0;
								CFRelease( aPSString );
								aPSName = OUString( pPSBuffer );
							}

							if ( !aPSName.getLength() )
								continue;

							int pNativeFont = (int)FMGetFontFromATSFontRef( aFont );
							if ( (ATSUFontID)pNativeFont == kATSUInvalidFontID )
								continue;

							// Get the ATS font name as the Cocoa name on some
							// Mac OS X versions adds extraneous words
							CFStringRef aDisplayString;
							if ( ATSFontGetName( aFont, kATSOptionFlagsDefault, &aDisplayString ) != noErr )
								continue;

							CFIndex nDisplayLen = CFStringGetLength( aDisplayString );
							CFRange aDisplayRange = CFRangeMake( 0, nDisplayLen );
							sal_Unicode pDisplayBuffer[ nDisplayLen + 1 ];
							CFStringGetCharacters( aDisplayString, aDisplayRange, pDisplayBuffer );
							pDisplayBuffer[ nDisplayLen ] = 0;
							CFRelease( aDisplayString );

							OUString aMapName;
							OUString aDisplayName( pDisplayBuffer );
							sal_Int32 nColon = aDisplayName.indexOf( (sal_Unicode)':' );
							if ( nColon >= 0 )
							{
								aMapName = aDisplayName;
								aDisplayName = OUString( aDisplayName.getStr(), nColon );
							}

							// Ignore empty font names or font names that start
							// with a "."
							if ( !aDisplayName.getLength() || aDisplayName.toChar() == (sal_Unicode)'.' )
								continue;

							if ( aDisplayName == aOpenSymbol || aDisplayName == aStarSymbol )
							{
								// Don't allow Sun's symbol fonts to override
								// our symbol font
								continue;
							}
							else if ( aDisplayName == aNeoSymbol )
							{
								aDisplayName = OUString( aOpenSymbol );
								aMapName = aSymbol + OUString::createFromAscii( ";" ) + aNeoSymbol;
							}
							else if ( aDisplayName == aLastResort )
							{
								// Ignore this Java font as it will mess up
								// our font fallback process
								continue;
							}
							else if ( aDisplayName == aTimesRoman )
							{
								aMapName = aTimes;
							}

							String aXubMapName( aMapName );
							String aXubDisplayName( aDisplayName );

							// Skip the font if we already have it
							::std::map< String, JavaImplFontData* >::iterator it = pSalData->maFontNameMapping.find( aXubDisplayName );
							if ( it != pSalData->maFontNameMapping.end() )
								continue;

							ImplDevFontAttributes aAttributes;

							// Determine pitch and family type
							FontFamily nFamily;
							if ( nPitch == PITCH_FIXED )
								nFamily = FAMILY_MODERN;
							else if ( aPSName.indexOf( aSans ) >= 0 )
								nFamily = FAMILY_SWISS;
							else if ( aPSName.indexOf( aRoman ) >= 0 || aPSName.indexOf( aSerif ) >= 0 || aPSName.indexOf( aTimes ) >= 0 || aPSName.indexOf( aGothic ) >= 0 )
								nFamily = FAMILY_ROMAN;
							else
								nFamily = FAMILY_SWISS;

							aAttributes.maName = aXubDisplayName;
							aAttributes.meWeight = nWeight;
							aAttributes.meItalic = nItalic;
							aAttributes.meFamily = nFamily;
							aAttributes.mePitch = nPitch;
							aAttributes.meWidthType = nWidth;
							aAttributes.mbSymbolFlag = false;
							aAttributes.maMapNames = aXubMapName;
							aAttributes.mnQuality = 0;
							aAttributes.mbOrientation = true;
							aAttributes.mbDevice = false;
							aAttributes.mbSubsettable = true;
							aAttributes.mbEmbeddable = false;

							JavaImplFontData *pFontData = new JavaImplFontData( aAttributes, aPSName, pNativeFont );
							pSalData->maFontNameMapping[ aXubDisplayName ] = pFontData;

							// Multiple native fonts can map to the same font
							// due to disabling and reenabling of fonts with
							// the same name. Also, note that multiple font
							// names can map to a single native font so do not
							// rely on the native font to look up the font name.
							pSalData->maNativeFontMapping[ pNativeFont ] = pFontData;
							pSalData->maJavaFontNameMapping[ aPSName ] = pFontData;
						}

						NSFontManager_releaseAllFonts( pFonts );
					}
				}

				// Reset any cached VCLFont instances
				for ( ::std::list< JavaSalGraphics* >::const_iterator git = pSalData->maGraphicsList.begin(); git != pSalData->maGraphicsList.end(); ++git )
				{
					com_sun_star_vcl_VCLFont *pCurrentFont = (*git)->mpVCLFont;
					if ( pCurrentFont )
					{
						OUString aFontName( pCurrentFont->getName() );
						::std::map< OUString, JavaImplFontData* >::const_iterator it = pSalData->maJavaFontNameMapping.find( aFontName );
						if ( it != pSalData->maJavaFontNameMapping.end() )
						{
							(*git)->mpVCLFont = new com_sun_star_vcl_VCLFont( it->second->maVCLFontName, pCurrentFont->getSize(), pCurrentFont->getOrientation(), pCurrentFont->isAntialiased(), pCurrentFont->isVertical(), pCurrentFont->getScaleX(), 0 );
							delete pCurrentFont;
						}

						for ( ::std::map< int, com_sun_star_vcl_VCLFont* >::iterator ffit = (*git)->maFallbackFonts.begin(); ffit != (*git)->maFallbackFonts.end(); ++ffit )
						{
							pCurrentFont = ffit->second;
							if ( pCurrentFont )
							{
								OUString aCurrentFontName( pCurrentFont->getName() );
								::std::map< OUString, JavaImplFontData* >::const_iterator mit = pSalData->maJavaFontNameMapping.find( aCurrentFontName );
								if ( mit != pSalData->maJavaFontNameMapping.end() )
								{
									ffit->second = new com_sun_star_vcl_VCLFont( mit->second->maVCLFontName, pCurrentFont->getSize(), pCurrentFont->getOrientation(), pCurrentFont->isAntialiased(), pCurrentFont->isVertical(), pCurrentFont->getScaleX(), 0 );
									delete pCurrentFont;
								}
							}
						}
					}
				}
			}

			rSolarMutex.release();
		}
	}
}

// -----------------------------------------------------------------------

static void LoadNativeFontsTimerCallback( EventLoopTimerRef aTimer, void *pData )
{
	static bool bInLoad = false;

	if ( bInLoad )
		return;

	bInLoad = true;

	ImplFontListChangedCallback( NULL, NULL );

	// Release any waiting thread
	aLoadNativeFontsCondition.set();

	bInLoad = false;
}

// =======================================================================

JavaImplFontData::JavaImplFontData( const ImplDevFontAttributes& rAttributes, OUString aVCLFontName, sal_IntPtr nATSUFontID, int nJavaFontID ) : ImplFontData( rAttributes, 0 ), maVCLFontName( aVCLFontName ), mnATSUFontID( nATSUFontID ), mnJavaFontID( nJavaFontID )
{

	// [ed] 11/1/04 Scalable fonts should always report their width and height
	// as zero. The single size zero causes higher-level font elements to treat
	// fonts as infinitely scalable and provide lists of default font sizes.
	// The size of zero matches the unx implementation. Bug 196.
	SetBitmapSize( 0, 0 );
}

// -----------------------------------------------------------------------

JavaImplFontData::~JavaImplFontData()
{
}

// -----------------------------------------------------------------------

ImplFontEntry* JavaImplFontData::CreateFontInstance( ImplFontSelectData& rData ) const
{
    return new ImplFontEntry( rData );
}

// -----------------------------------------------------------------------

ImplFontData* JavaImplFontData::Clone() const
{
	return new JavaImplFontData( *this, maVCLFontName, mnATSUFontID, mnJavaFontID );
}

// -----------------------------------------------------------------------

sal_IntPtr JavaImplFontData::GetFontId() const
{
	if ( !mnJavaFontID )
	{
		com_sun_star_vcl_VCLFont aVCLFont( maVCLFontName, GetHeight(), 0, sal_True, sal_False, GetWidth() ? (double)GetWidth() / (double)GetHeight() : 1.0, 0 );
		mnJavaFontID = aVCLFont.getNativeFont();
	}

	return mnJavaFontID;
}

// =======================================================================

void JavaSalGraphics::SetTextColor( SalColor nSalColor )
{
	mnTextColor = nSalColor | 0xff000000;
}

// -----------------------------------------------------------------------

USHORT JavaSalGraphics::SetFont( ImplFontSelectData* pFont, int nFallbackLevel )
{
	if ( !pFont )
		return 0;

	SalData *pSalData = GetSalData();
	JavaImplFontData *pFontData = (JavaImplFontData *)pFont->mpFontData;

	if ( nFallbackLevel )
	{
		// Retrieve the fallback font if one has been set by a text layout
		::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator ffit = maFallbackFonts.find( nFallbackLevel );
		if ( ffit != maFallbackFonts.end() )
		{
			int pNativeFont = ffit->second->getNativeFont();
			::std::map< int, JavaImplFontData* >::const_iterator it = pSalData->maNativeFontMapping.find( pNativeFont );
			if ( it != pSalData->maNativeFontMapping.end() )
				pFontData = it->second;
		}
	}

	// Fix bugs 1813, 2964, 2968, 2971, and 2972 by tryng to find a matching
	// bold and/or italic font even if we are in a fallback level
	BOOL bAddBold = ( pFont->GetWeight() > WEIGHT_MEDIUM && pFontData->GetWeight() <= WEIGHT_MEDIUM );
	BOOL bAddItalic = ( ( pFont->GetSlant() == ITALIC_OBLIQUE || pFont->GetSlant() == ITALIC_NORMAL ) && pFontData->GetSlant() != ITALIC_OBLIQUE && pFontData->GetSlant() != ITALIC_NORMAL );
	if ( bAddBold || bAddItalic )
	{
		JavaImplFontData *pOldFontData = pFontData;
		BOOL bBold = ( pFont->GetWeight() > WEIGHT_MEDIUM );
		BOOL bItalic = ( pFont->GetSlant() == ITALIC_OBLIQUE || pFont->GetSlant() == ITALIC_NORMAL );
		OUString aFontName( pFontData->maVCLFontName );
		CFStringRef aString = CFStringCreateWithCharactersNoCopy( NULL, aFontName.getStr(), aFontName.getLength(), kCFAllocatorNull );
		CFStringRef aMatchedString = NSFontManager_findFontNameWithStyle( aString, bBold, bItalic, pFont->mnHeight );
		if ( aMatchedString )
		{
			CFRange aRange = CFStringFind( aMatchedString, CFSTR( ":" ), 0 );
			CFIndex nLen;
			if ( aRange.location != kCFNotFound )
				nLen = aRange.location;
			else
				nLen = CFStringGetLength( aMatchedString );
			aRange = CFRangeMake( 0, nLen );
			sal_Unicode pBuffer[ nLen + 1 ];
			CFStringGetCharacters( aMatchedString, aRange, pBuffer );
			pBuffer[ nLen ] = 0;
			aFontName = OUString( pBuffer );
			CFRelease( aMatchedString );
		}

		String aXubFontName( aFontName );
		::std::map< String, JavaImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( aXubFontName );
		if ( it != pSalData->maFontNameMapping.end() && ( !bAddBold || it->second->meWeight > WEIGHT_MEDIUM ) && ( !bAddItalic || it->second->meItalic == ITALIC_OBLIQUE || it->second->meItalic == ITALIC_NORMAL ) )
		{
			pFontData = it->second;
		}
		else if ( bAddBold && bAddItalic )
		{
			// Try with bold only
			aMatchedString = NSFontManager_findFontNameWithStyle( aString, bBold, FALSE, pFont->mnHeight );
			if ( aMatchedString )
			{
				CFRange aRange = CFStringFind( aMatchedString, CFSTR( ":" ), 0 );
				CFIndex nLen;
				if ( aRange.location != kCFNotFound )
					nLen = aRange.location;
				else
					nLen = CFStringGetLength( aMatchedString );
				aRange = CFRangeMake( 0, nLen );
				sal_Unicode pBuffer[ nLen + 1 ];
				CFStringGetCharacters( aMatchedString, aRange, pBuffer );
				pBuffer[ nLen ] = 0;
				aFontName = OUString( pBuffer );
				CFRelease( aMatchedString );
			}

			aXubFontName = XubString( aFontName );
			it = pSalData->maFontNameMapping.find( aXubFontName );
			if ( it != pSalData->maFontNameMapping.end() && it->second->meWeight > WEIGHT_MEDIUM )
			{
				pFontData = it->second;
			}
			else
			{
				// Try with italic only
				aMatchedString = NSFontManager_findFontNameWithStyle( aString, FALSE, bItalic, pFont->mnHeight );
				if ( aMatchedString )
				{
					CFRange aRange = CFStringFind( aMatchedString, CFSTR( ":" ), 0 );
					CFIndex nLen;
					if ( aRange.location != kCFNotFound )
						nLen = aRange.location;
					else
						nLen = CFStringGetLength( aMatchedString );
					aRange = CFRangeMake( 0, nLen );
					sal_Unicode pBuffer[ nLen + 1 ];
					CFStringGetCharacters( aMatchedString, aRange, pBuffer );
					pBuffer[ nLen ] = 0;
					aFontName = OUString( pBuffer );
					CFRelease( aMatchedString );
				}

				aXubFontName = XubString( aFontName );
				it = pSalData->maFontNameMapping.find( aXubFontName );
				if ( it != pSalData->maFontNameMapping.end() && ( it->second->meItalic == ITALIC_OBLIQUE || it->second->meItalic == ITALIC_NORMAL ) )
					pFontData = it->second;
			}
		}

		if ( aString )
			CFRelease( aString );

		// Avoid selecting a font that has already been used
		if ( nFallbackLevel && pOldFontData != pFontData )
		{
			::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator ffit = maFallbackFonts.find( 0 );
			if ( ffit != maFallbackFonts.end() && ffit->second->getNativeFont() == pFontData->GetFontId() )
				pFontData = pOldFontData;
		}
	}

	::std::map< int, com_sun_star_vcl_VCLFont* >::iterator ffit = maFallbackFonts.find( nFallbackLevel );
	if ( ffit != maFallbackFonts.end() )
	{
		delete ffit->second;
		maFallbackFonts.erase( ffit );
	}

	maFallbackFonts[ nFallbackLevel ] = new com_sun_star_vcl_VCLFont( pFontData->maVCLFontName, pFont->mnHeight, pFont->mnOrientation, !pFont->mbNonAntialiased, pFont->mbVertical, pFont->mnWidth ? (double)pFont->mnWidth / (double)pFont->mnHeight : 1.0, 0 );

	if ( !nFallbackLevel )
	{
		// Set font for graphics device
		if ( mpVCLFont )
			delete mpVCLFont;
		mpVCLFont = new com_sun_star_vcl_VCLFont( maFallbackFonts[ nFallbackLevel ] );

		mnFontFamily = pFont->GetFamilyType();
		mnFontWeight = pFont->GetWeight();    
		mbFontItalic = ( pFont->GetSlant() == ITALIC_OBLIQUE || pFont->GetSlant() == ITALIC_NORMAL );
		mnFontPitch = pFont->GetPitch();
	}

	return 0;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::GetFontMetric( ImplFontMetricData* pMetric )
{
	SalData *pSalData = GetSalData();

	JavaImplFontData *pData;
	::std::map< String, JavaImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( pMetric->maName );
	if ( it != pSalData->maFontNameMapping.end() )
		pData = it->second;
	else
 		pData = NULL;

	if ( mpVCLFont )
	{
		pMetric->mnWidth = mpVCLFont->getSize();
		pMetric->mnOrientation = mpVCLFont->getOrientation();
	}
	else
	{
		pMetric->mnWidth = 0;
		pMetric->mnOrientation = 0;
	}

	if ( pData && pMetric->mnWidth )
	{
		ATSFontMetrics aFontMetrics;
		ATSFontRef aFont = FMGetATSFontRefFromFont( pData->mnATSUFontID );
		if ( ATSFontGetHorizontalMetrics( aFont, kATSOptionFlagsDefault, &aFontMetrics ) == noErr ) {
			// Mac OS X seems to overstate the leading for some fonts (usually
			// CJK fonts like Hiragino) so fix fix bugs 2827 and 2847 by
			// adding combining the leading with descent
			pMetric->mnAscent = (long)( ( aFontMetrics.ascent * pMetric->mnWidth ) + 0.5 );
			if ( pMetric->mnAscent < 1 )
				pMetric->mnAscent = 1;
			// Fix bug 2881 by handling cases where font does not have negative
			// descent
			pMetric->mnDescent = (long)( ( ( aFontMetrics.leading + fabs( aFontMetrics.descent ) ) * pMetric->mnWidth ) + 0.5 );
			if ( pMetric->mnDescent < 0 )
				pMetric->mnDescent = 0;
		}
		else
		{
			pMetric->mnAscent = 0;
			pMetric->mnDescent = 0;
		}

		pMetric->mbDevice = pData->mbDevice;
		pMetric->mbScalableFont = true;
		pMetric->maName = pData->GetFamilyName();
		pMetric->maStyleName = pData->GetStyleName();
		pMetric->meWeight = pData->GetWeight();
		pMetric->meFamily = pData->GetFamilyType();
		pMetric->meItalic = pData->GetSlant();
		pMetric->mePitch = pData->GetPitch();
		pMetric->mbSymbolFlag = pData->IsSymbolFont();
	}
	else
	{
		pMetric->mnAscent = 0;
		pMetric->mnDescent = 0;
		pMetric->mnSlant = 0;

		pMetric->mbDevice = false;
		pMetric->mbScalableFont = false;
		pMetric->maName = String();
		pMetric->maStyleName = String();
		pMetric->meWeight = WEIGHT_NORMAL;
		pMetric->meFamily = FAMILY_DONTKNOW;
		pMetric->meItalic = ITALIC_NONE;
		pMetric->mePitch = PITCH_VARIABLE;
		pMetric->mbSymbolFlag = false;
	}

	pMetric->mnIntLeading = 0;
	pMetric->mnExtLeading = 0;
	pMetric->mbKernableFont = false;
	pMetric->mnSlant = 0;
}

// -----------------------------------------------------------------------

ULONG JavaSalGraphics::GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs )
{
	return 0;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::GetDevFontList( ImplDevFontList* pList )
{
	SalData *pSalData = GetSalData();

	// Only run the timer once since loading fonts is extremely expensive
	if ( !pLoadNativeFontsTimerUPP )
	{
		pSalData->mpEventQueue->setShutdownDisabled( sal_True );

		// Load libsfx and invoke the native shutdown cancelled handler
		if ( !pShutdownCancelledHandler )
		{
			OUString aLibName = OUString::createFromAscii( "libsfx" );
			aLibName += OUString::valueOf( (sal_Int32)SUPD, 10 );
			aLibName += OUString::createFromAscii( STRING( DLLPOSTFIX ) );
			aLibName += OUString( RTL_CONSTASCII_USTRINGPARAM( ".dylib" ) );
			if ( aShutdownCancelledHandlerModule.load( aLibName ) )
				pShutdownCancelledHandler = (NativeShutdownCancelledHandler_Type *)aShutdownCancelledHandlerModule.getSymbol( OUString::createFromAscii( "NativeShutdownCancelledHandler" ) );
		}

		if ( pShutdownCancelledHandler )
			pShutdownCancelledHandler();

		pLoadNativeFontsTimerUPP = NewEventLoopTimerUPP( LoadNativeFontsTimerCallback );
		if ( pLoadNativeFontsTimerUPP )
		{
			if ( GetCurrentEventLoop() != GetMainEventLoop() )
			{
				aLoadNativeFontsCondition.reset();
				InstallEventLoopTimer( GetMainEventLoop(), 0.001, kEventDurationForever, pLoadNativeFontsTimerUPP, NULL, NULL );
				ULONG nCount = Application::ReleaseSolarMutex();
				aLoadNativeFontsCondition.wait();
				Application::AcquireSolarMutex( nCount );
			}
			else
			{
				LoadNativeFontsTimerCallback( NULL, NULL );
			}
		}

		pSalData->mpEventQueue->setShutdownDisabled( sal_False );
	}

	// Iterate through fonts and add each to the font list
	for ( ::std::map< String, JavaImplFontData* >::const_iterator it = pSalData->maFontNameMapping.begin(); it != pSalData->maFontNameMapping.end(); ++it )
		pList->Add( it->second->Clone() );
}

// -----------------------------------------------------------------------

BOOL JavaSalGraphics::GetGlyphBoundRect( long nIndex, Rectangle& rRect )
{
	rRect.SetEmpty();

	com_sun_star_vcl_VCLFont *pVCLFont = NULL;

	int nFallbackLevel = nIndex >> GF_FONTSHIFT;
	if ( !nFallbackLevel )
	{
		pVCLFont = mpVCLFont;
	}
	else
	{
		// Retrieve the fallback font if one has been set by a text layout
		::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator ffit = maFallbackFonts.find( nFallbackLevel );
		if ( ffit != maFallbackFonts.end() )
			pVCLFont = ffit->second;
	}

	if ( pVCLFont )
		rRect = mpVCLGraphics->getGlyphBounds( nIndex & GF_IDXMASK, pVCLFont, nIndex & GF_ROTMASK );

	// Fix bug 2191 by always returning true so that the OOo code doesn't
	// exeecute its "draw the glyph and see which pixels are black" code
	return true;
}

// -----------------------------------------------------------------------

BOOL JavaSalGraphics::GetGlyphOutline( long nIndex, B2DPolyPolygon& rPolyPoly )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::GetGlyphOutline not implemented\n" );
#endif
	rPolyPoly.clear();
	return FALSE;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::GetDevFontSubstList( OutputDevice* pOutDev )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::GetDevFontSubstList not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::AddTempDevFont( ImplDevFontList* pList, const String& rFileURL, const String& rFontName )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::AddTempDevFont not implemented\n" );
#endif
	return false;
}

// -----------------------------------------------------------------------

BOOL JavaSalGraphics::CreateFontSubset( const rtl::OUString& rToFile,
                                    ImplFontData* pFont, long* pGlyphIDs,
                                    sal_uInt8* pEncoding, sal_Int32* pWidths,
                                    int nGlyphs, FontSubsetInfo& rInfo )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::CreateFontSubset not implemented\n" );
#endif
	return FALSE;
}

// -----------------------------------------------------------------------

const void* JavaSalGraphics::GetEmbedFontData( ImplFontData* pFont,
                                           const sal_Unicode* pUnicodes,
                                           sal_Int32* pWidths,
                                           FontSubsetInfo& rInfo,
                                           long* pDataLen )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::GetEmbedFontData not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::FreeEmbedFontData not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

const std::map< sal_Unicode, sal_Int32 >* JavaSalGraphics::GetFontEncodingVector(
                ImplFontData* pFont,
                const std::map< sal_Unicode, rtl::OString >** pNonEncoded )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::GetFontEncodingVector not implemented\n" );
#endif
	if ( pNonEncoded )
		*pNonEncoded = NULL;
	return NULL;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::DrawServerFontLayout( const ServerFontLayout& )
{
}

// -----------------------------------------------------------------------

ImplFontCharMap* JavaSalGraphics::GetImplFontCharMap() const
{
	return ImplFontCharMap::GetDefaultMap();
}
