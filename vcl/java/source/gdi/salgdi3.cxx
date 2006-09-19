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
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#include "salgdi3_cocoa.h"

#define DOSTRING( x )			#x
#define STRING( x )				DOSTRING( x )

typedef void NativeShutdownCancelledHandler_Type();

static EventLoopTimerUPP pLoadNativeFontsTimerUPP = NULL;
static ::osl::Condition aLoadNativeFontsCondition;
static ::vos::OModule aShutdownCancelledHandlerModule;
static NativeShutdownCancelledHandler_Type *pShutdownCancelledHandler = NULL;

using namespace basegfx;
using namespace rtl;
using namespace utl;
using namespace vcl;
using namespace vos;

// ============================================================================

static void ImplLoadNativeFont( OUString aPath )
{
	if ( !aPath.getLength() )
		return;

	oslDirectory aDir;
	if ( osl_openDirectory( aPath.pData, &aDir ) == osl_File_E_None )
	{
		oslDirectoryItem aDirItem;
		while ( osl_getNextDirectoryItem( aDir, &aDirItem, 16 ) == osl_File_E_None )
		{
			oslFileStatus aStatus;
			memset( &aStatus, 0, sizeof( oslFileStatus ) );
			if ( osl_getFileStatus( aDirItem, &aStatus, osl_FileStatus_Mask_FileURL ) == osl_File_E_None )
				ImplLoadNativeFont( OUString( aStatus.ustrFileURL ) );

			osl_releaseDirectoryItem( aDirItem );
		}
	}
	else
	{
		OUString aSysPath;
		if ( osl_getSystemPathFromFileURL( aPath.pData, &aSysPath.pData ) == osl_File_E_None )
		{
			FSRef aFontPath;
			FSSpec aFontSpec;
			OString aUTF8Path( aSysPath.getStr(), aSysPath.getLength(), RTL_TEXTENCODING_UTF8 );
			if ( FSPathMakeRef( (const UInt8 *)aUTF8Path.getStr(), &aFontPath, 0 ) == noErr && FSGetCatalogInfo( &aFontPath, kFSCatInfoNone, NULL, NULL, &aFontSpec, NULL ) == noErr )
			{
				ATSFontActivateFromFileSpecification( &aFontSpec, kATSFontContextGlobal, kATSFontFormatUnspecified, NULL, kATSOptionFlagsDefault, NULL );
				ReceiveNextEvent( 0, NULL, 0, false, NULL );
			}
		}
	}
}

// -----------------------------------------------------------------------

static void ImplFontListChangedCallback( ATSFontNotificationInfoRef aInfo, void *pData )
{
	if ( !Application::IsShutDown() )
	{
		// We need to let any pending timers run so that we don't deadlock
		IMutex& rSolarMutex = Application::GetSolarMutex();
		bool bAcquired = false;
		TimeValue aDelay;
		aDelay.Seconds = 0;
		aDelay.Nanosec = 10;
		while ( !Application::IsShutDown() )
		{
			if ( rSolarMutex.tryToAcquire() )
			{
				bAcquired = true;
				break;
			}

			ReceiveNextEvent( 0, NULL, 0, false, NULL );
			OThread::wait( aDelay );
		}

		if ( bAcquired )
		{
			SalData *pSalData = GetSalData();

			pSalData->maJavaNativeFontMapping.clear();
			SalATSLayout::ClearLayoutDataCache();

			if ( !Application::IsShutDown() )
			{
				VCLThreadAttach t;
				if ( t.pEnv )
				{
					// Update cached fonts
					OUString aRoman( OUString::createFromAscii( "Roman" ) );
					OUString aSans( OUString::createFromAscii( "Sans" ) );
					OUString aSerif( OUString::createFromAscii( "Serif" ) );
					OUString aTimes( OUString::createFromAscii( "Times" ) );
					OUString aTimesRoman( OUString::createFromAscii( "Times Roman" ) );
					long *pFonts = NSFontManager_getAllFonts();
					if ( pFonts )
					{
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
							OUString aMapName;
							OUString aDisplayName( pDisplayBuffer );
							sal_Int32 nColon = aDisplayName.indexOf( (sal_Unicode)':' );
							if ( nColon >= 0 )
							{
								aMapName = aDisplayName;
								aDisplayName = OUString( aDisplayName.getStr(), nColon );
							}

							if ( aDisplayName == aTimesRoman )
								aMapName = aTimes;

							CFRelease( aDisplayString );

							// Ignore empty font names or font names that start
							// with a "."
							if ( !aDisplayName.getLength() || aDisplayName.toChar() == (sal_Unicode)'.' )
								continue;

							String aXubMapName( aMapName );
							String aXubDisplayName( aDisplayName );

							// Delete old font data
							::std::map< String, JavaImplFontData* >::iterator it = pSalData->maFontNameMapping.find( aXubDisplayName );
							if ( it != pSalData->maFontNameMapping.end() )
							{
								delete it->second;
								pSalData->maFontNameMapping.erase( it );
							}
							::std::map< int, JavaImplFontData* >::iterator nit = pSalData->maNativeFontMapping.find( pNativeFont );
							if ( nit != pSalData->maNativeFontMapping.end() )
								pSalData->maNativeFontMapping.erase( nit );
							::std::map< OUString, JavaImplFontData* >::iterator jit = pSalData->maJavaFontNameMapping.find( aPSName );
							if ( jit != pSalData->maJavaFontNameMapping.end() )
								pSalData->maJavaFontNameMapping.erase( jit );

							ImplDevFontAttributes aAttributes;

							// Determine pitch and family type
							FontFamily nFamily;
							if ( nPitch == PITCH_FIXED )
								nFamily = FAMILY_MODERN;
							else if ( aPSName.indexOf( aSans ) >= 0 )
								nFamily = FAMILY_SWISS;
							else if ( aPSName.indexOf( aRoman ) >= 0 || aPSName.indexOf( aSerif ) >= 0 || aPSName.indexOf( aTimes ) >= 0 )
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

							JavaImplFontData *pData = new JavaImplFontData( aAttributes, aPSName, pNativeFont );
							pSalData->maFontNameMapping[ aXubDisplayName ] = pData;

							// Multiple native fonts can map to the same font
							// due to disabling and reenabling of fonts with
							// the same name. Also, note that multiple font
							// names can map to a single native font so do not
							// rely on the native font to look up the font name.
							pSalData->maNativeFontMapping[ pNativeFont ] = pData;
							pSalData->maJavaFontNameMapping[ aPSName ] = pData;
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
							(*git)->mpVCLFont = new com_sun_star_vcl_VCLFont( it->second->maVCLFontName, pCurrentFont->getSize(), pCurrentFont->getOrientation(), pCurrentFont->isAntialiased(), pCurrentFont->isVertical(), pCurrentFont->getScaleX(), 0, pCurrentFont->isBold(), pCurrentFont->isItalic() );
							delete pCurrentFont;
						}

						for ( ::std::map< int, com_sun_star_vcl_VCLFont* >::iterator ffit = (*git)->maFallbackFonts.begin(); ffit != (*git)->maFallbackFonts.end(); ++ffit )
						{
							pCurrentFont = ffit->second;
							if ( pCurrentFont )
							{
								OUString aFontName( pCurrentFont->getName() );
								::std::map< OUString, JavaImplFontData* >::const_iterator it = pSalData->maJavaFontNameMapping.find( aFontName );
								if ( it != pSalData->maJavaFontNameMapping.end() )
								{
									ffit->second = new com_sun_star_vcl_VCLFont( it->second->maVCLFontName, pCurrentFont->getSize(), pCurrentFont->getOrientation(), pCurrentFont->isAntialiased(), pCurrentFont->isVertical(), pCurrentFont->getScaleX(), 0, pCurrentFont->isBold(), pCurrentFont->isItalic() );
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

	// Activate the fonts in the "user/fonts" directory
	OUString aUserPath;
	if ( Bootstrap::locateUserInstallation( aUserPath ) == Bootstrap::PATH_EXISTS )
	{
		if ( aUserPath.getLength() )
		{
			aUserPath += OUString::createFromAscii( "/user/fonts" );
			ImplLoadNativeFont( aUserPath );
		}
	}

	// Activate the fonts in the "share/fonts/truetype" directory
	OUString aBasePath;
	if ( Bootstrap::locateBaseInstallation( aBasePath ) == Bootstrap::PATH_EXISTS )
	{
		if ( aBasePath.getLength() )
		{
			aBasePath += OUString::createFromAscii( "/share/fonts/truetype" );
			ImplLoadNativeFont( aBasePath );
		}
	}

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
		com_sun_star_vcl_VCLFont aVCLFont( maVCLFontName, GetHeight(), 0, sal_True, sal_False, GetWidth() ? (double)GetWidth() / (double)GetHeight() : 1.0, 0, GetWeight() > WEIGHT_MEDIUM ? sal_True : sal_False, ( GetSlant() == ITALIC_OBLIQUE || GetSlant() == ITALIC_NORMAL ) ? sal_True : sal_False );
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
	{
		if ( mpVCLFont )
		{
			delete mpVCLFont;
			mpVCLFont = NULL;
		}
		return 0;
	}
	
	SalData *pSalData = GetSalData();

	// Don't change the font for fallback levels as we need the first font
	// to properly determine the fallback font
	if ( !nFallbackLevel )
	{
		BOOL bAddBold = ( pFont->GetWeight() > WEIGHT_MEDIUM && pFont->mpFontData->GetWeight() <= WEIGHT_MEDIUM );
		BOOL bAddItalic = ( ( pFont->GetSlant() == ITALIC_OBLIQUE || pFont->GetSlant() == ITALIC_NORMAL ) && pFont->mpFontData->GetSlant() != ITALIC_OBLIQUE && pFont->mpFontData->GetSlant() != ITALIC_NORMAL );
		if ( bAddBold || bAddItalic )
		{
			BOOL bBold = ( pFont->GetWeight() > WEIGHT_MEDIUM );
			BOOL bItalic = ( pFont->GetSlant() == ITALIC_OBLIQUE || pFont->GetSlant() == ITALIC_NORMAL );
			OUString aFontName( ((JavaImplFontData *)pFont->mpFontData)->maVCLFontName );
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
				pFont->mpFontData = it->second;
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

				String aXubFontName( aFontName );
				::std::map< String, JavaImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( aXubFontName );
				if ( it != pSalData->maFontNameMapping.end() && it->second->meWeight > WEIGHT_MEDIUM )
				{
					pFont->mpFontData = it->second;
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

					String aXubFontName( aFontName );
					::std::map< String, JavaImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( aXubFontName );
					if ( it != pSalData->maFontNameMapping.end() && ( it->second->meItalic == ITALIC_OBLIQUE || it->second->meItalic == ITALIC_NORMAL ) )
						pFont->mpFontData = it->second;
				}
			}

			if ( aString )
				CFRelease( aString );
		}
	}
	else
	{
		// Retrieve the fallback font if one has been set by a text layout
		::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator ffit = maFallbackFonts.find( nFallbackLevel );
		if ( ffit != maFallbackFonts.end() )
		{
			int pNativeFont = ffit->second->getNativeFont();
			::std::map< int, JavaImplFontData* >::const_iterator it = pSalData->maNativeFontMapping.find( pNativeFont );
			if ( it != pSalData->maNativeFontMapping.end() )
				pFont->mpFontData = it->second;
		}
	}

	pFont->maSearchName = pFont->mpFontData->maName;

	if ( !nFallbackLevel )
	{
		// Set font for graphics device
		if ( mpVCLFont )
			delete mpVCLFont;
		mpVCLFont = new com_sun_star_vcl_VCLFont( ((JavaImplFontData *)pFont->mpFontData)->maVCLFontName, pFont->mnHeight, pFont->mnOrientation, !pFont->mbNonAntialiased, pFont->mbVertical, pFont->mnWidth ? (double)pFont->mnWidth / (double)pFont->mnHeight : 1.0, 0, pFont->GetWeight() > WEIGHT_MEDIUM ? sal_True : sal_False, ( pFont->GetSlant() == ITALIC_OBLIQUE || pFont->GetSlant() == ITALIC_NORMAL ) ? sal_True : sal_False );
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
		pMetric->mnAscent = mpVCLFont->getAscent();
		pMetric->mnDescent = mpVCLFont->getDescent();
		pMetric->mnIntLeading = mpVCLFont->getLeading();
		pMetric->mnOrientation = mpVCLFont->getOrientation();
	}
	else
	{
		pMetric->mnWidth = 0;
		pMetric->mnAscent = 0;
		pMetric->mnDescent = 0;
		pMetric->mnIntLeading = 0;
		pMetric->mnOrientation = 0;
	}

	if ( pData )
	{
		pMetric->mbDevice = pData->mbDevice;
		pMetric->mbScalableFont = true;
		pMetric->mbKernableFont = true;
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
		pMetric->mbDevice = false;
		pMetric->mbScalableFont = false;
		pMetric->mbKernableFont = false;
		pMetric->maName = String();
		pMetric->maStyleName = String();
		pMetric->meWeight = WEIGHT_NORMAL;
		pMetric->meFamily = FAMILY_DONTKNOW;
		pMetric->meItalic = ITALIC_NONE;
		pMetric->mePitch = PITCH_VARIABLE;
		pMetric->mbSymbolFlag = false;
	}

	pMetric->mnExtLeading = 0;
	pMetric->mnSlant = 0;
}

// -----------------------------------------------------------------------

ULONG JavaSalGraphics::GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs )
{
	if ( !mpVCLFont )
		return 0;
	
	ImplKernPairData *pPair = pKernPairs;
	for ( ULONG i = 0; i < nPairs; i++ )
	{
		pPair->mnKern = mpVCLFont->getKerning( pPair->mnChar1, pPair->mnChar2 );
		pPair++;
	}

	return nPairs;
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

	return !rRect.IsEmpty();
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
	return NULL;
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
