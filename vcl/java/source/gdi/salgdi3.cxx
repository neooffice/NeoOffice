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

#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALATSLAYOUT_HXX
#include <salatslayout.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <rtl/process.h>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#include "salgdi3_cocoa.h"

static EventLoopTimerUPP pLoadNativeFontsTimerUPP = NULL;
static ::osl::Condition aLoadNativeFontsCondition;

using namespace rtl;
using namespace utl;
using namespace vcl;
using namespace vos;

// ============================================================================

static void ImplFontListChangedCallback( ATSFontNotificationInfoRef aInfo, void *pData )
{
	if ( !Application::IsShutDown() )
	{
		// We need to let any pending timers run so that we don't deadlock
		IMutex& rSolarMutex = Application::GetSolarMutex();
		bool bAcquired = false;
		while ( !Application::IsShutDown() )
		{
			if ( rSolarMutex.tryToAcquire() )
			{
				bAcquired = true;
				break;
			}

			ReceiveNextEvent( 0, NULL, 0, false, NULL );
			OThread::yield();
		}

		if ( bAcquired )
		{
			SalData *pSalData = GetSalData();

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
					java_lang_Object *pFonts = com_sun_star_vcl_VCLFont::getAllFonts();
					if ( pFonts )
					{
						jobjectArray pArray = (jobjectArray)pFonts->getJavaObject();
						jsize nFonts = t.pEnv->GetArrayLength( pArray );
						for ( jsize i = 0; i < nFonts; i++ )
						{
							jobject tempObj = t.pEnv->GetObjectArrayElement( pArray, i );
							if ( !tempObj )
								continue;

							com_sun_star_vcl_VCLFont *pVCLFont = new com_sun_star_vcl_VCLFont( tempObj );
							if ( !pVCLFont )
								continue;

							OUString aFontName( pVCLFont->getName() );
							CFStringRef aString = CFStringCreateWithCharactersNoCopy( NULL, aFontName.getStr(), aFontName.getLength(), kCFAllocatorNull );
							if ( !aString )
							{
								delete pVCLFont;
								continue;
							}

							void *pNSFont = NSFont_create( aString, pVCLFont->getSize() );
							CFRelease( aString );
							if ( !pNSFont )
							{
								delete pVCLFont;
								continue;
							}

							ATSFontRef aFont = NSFont_getATSFontRef( pNSFont );
							if ( !aFont )
							{
								NSFont_release( pNSFont );
								delete pVCLFont;
								continue;
							}

							ATSFontRef aRealFont = aFont;
							CFStringRef aRealPSString;
							if ( ATSFontGetPostScriptName( aRealFont, kATSOptionFlagsDefault, &aRealPSString ) != noErr )
							{
								NSFont_release( pNSFont );
								delete pVCLFont;
								continue;
							}

							OUString aFontPSName( pVCLFont->getPSName() );
							CFStringRef aPSString = CFStringCreateWithCharactersNoCopy( NULL, aFontPSName.getStr(), aFontPSName.getLength(), kCFAllocatorNull );
							if ( aPSString )
							{
								if ( CFStringCompare( aPSString, aRealPSString, 0 ) )
									aRealFont = ATSFontFindFromPostScriptName( aPSString, kATSOptionFlagsDefault );
								CFRelease( aRealPSString );
							}
							else
							{
								aRealFont = NULL;
							}

							if ( !aRealFont )
							{
								NSFont_release( pNSFont );
								delete pVCLFont;
								continue;
							}
							
							void *pNativeFont = (void *)FMGetFontFromATSFontRef( aRealFont );
							if ( (ATSUFontID)pNativeFont == kATSUInvalidFontID )
							{
								NSFont_release( pNSFont );
								delete pVCLFont;
								continue;
							}

							pVCLFont->setNativeFont( pNativeFont );

							// Get the ATS font name as the Cocoa name on some
							// Mac OS X versions adds extraneous words
							CFStringRef aDisplayString;
							if ( ATSFontGetName( aFont, kATSOptionFlagsDefault, &aDisplayString ) != noErr )
							{
								NSFont_release( pNSFont );
								delete pVCLFont;
								continue;
							}

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
							{
								NSFont_release( pNSFont );
								delete pVCLFont;
								continue;
							}

							XubString aXubMapName( aMapName );
							XubString aXubDisplayName( aDisplayName );
							ImplFontData *pData;
							::std::map< XubString, ImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( aXubDisplayName );
							if ( it != pSalData->maFontNameMapping.end() )
							{
								// Delete old font data
								pData = it->second;
								delete (com_sun_star_vcl_VCLFont *)pData->mpSysData;
							}
							else
							{
								pData = new ImplFontData();
								pSalData->maFontNameMapping[ aXubDisplayName ] = pData;
							}

							pData->mpSysData = (void *)( new com_sun_star_vcl_VCLFont( pVCLFont->getJavaObject() ) );

							// Multiple native fonts can map to the same font
							// due to disabling and reenabling of fonts with
							// the same name. Also, note that multiple font
							// names can map to a single native font so do not
							// rely on the native font to look up the font name.
							pSalData->maNativeFontMapping[ pNativeFont ] = pData;
							pSalData->maJavaFontNameMapping[ aFontName ] = pData;

							// Determine pitch and family type
							FontPitch nPitch = ( NSFontManager_isFixedPitch( pNSFont ) ? PITCH_FIXED : PITCH_VARIABLE );
							FontFamily nFamily;
							if ( nPitch == PITCH_FIXED )
								nFamily = FAMILY_MODERN;
							else if ( aFontName.indexOf( aSans ) >= 0 )
								nFamily = FAMILY_SWISS;
							else if ( aFontName.indexOf( aRoman ) >= 0 || aFontName.indexOf( aSerif ) >= 0 || aFontName.indexOf( aTimes ) >= 0 )
								nFamily = FAMILY_ROMAN;
							else
								nFamily = FAMILY_SWISS;

							pData->mpNext = NULL;
							pData->maName = aXubDisplayName;
							pData->maMapNames = aXubMapName;
							// [ed] 11/1/04 Scalable fonts should always report
							// their width and height as zero. The single size
							// zero causes higher-level font elements to treat
							// fonts as infinitely scalable and provide lists of
							// default font sizes. The size of zero matches the
							// unx implementation. Bug 196.
							pData->mnWidth = 0;
							pData->mnHeight = 0;
							pData->meFamily = nFamily;
							pData->meCharSet = RTL_TEXTENCODING_UNICODE;
							pData->mePitch = nPitch;
							pData->meWidthType = (FontWidth)NSFontManager_widthOfFont( pNSFont );
							pData->meWeight = (FontWeight)NSFontManager_weightOfFont( pNSFont );
							pData->meItalic = ( NSFontManager_isItalic( pNSFont ) ? ITALIC_NORMAL : ITALIC_NONE );
							pData->meType = TYPE_SCALABLE;
							pData->mnVerticalOrientation = 0;
							pData->mbOrientation = TRUE;
							pData->mbDevice = FALSE;
							pData->mnQuality = 0;
							pData->mbSubsettable = TRUE;
							pData->mbEmbeddable = FALSE;

							NSFont_release( pNSFont );
							delete pVCLFont;
						}

						delete pFonts;
					}
				}

				// Reset any cached VCLFont instances
				for ( ::std::list< SalGraphics* >::const_iterator git = pSalData->maGraphicsList.begin(); git != pSalData->maGraphicsList.end(); ++git )
				{
					com_sun_star_vcl_VCLFont *pCurrentFont = (*git)->maGraphicsData.mpVCLFont;
					if ( pCurrentFont )
					{
						OUString aFontName( pCurrentFont->getName() );
						::std::map< OUString, ImplFontData* >::const_iterator it = pSalData->maJavaFontNameMapping.find( aFontName );
						if ( it != pSalData->maJavaFontNameMapping.end() )
						{
							com_sun_star_vcl_VCLFont *pVCLFont = (com_sun_star_vcl_VCLFont *)it->second->mpSysData;
							(*git)->maGraphicsData.mpVCLFont = pVCLFont->deriveFont( pCurrentFont->getSize(), pCurrentFont->getOrientation(), pCurrentFont->isAntialiased(), pCurrentFont->isVertical(), pCurrentFont->getScaleX() );
							delete pCurrentFont;
						}

						for ( ::std::map< int, com_sun_star_vcl_VCLFont* >::iterator ffit = (*git)->maGraphicsData.maFallbackFonts.begin(); ffit != (*git)->maGraphicsData.maFallbackFonts.end(); ++ffit )
						{
							pCurrentFont = ffit->second;
							if ( pCurrentFont )
							{
								OUString aFontName( pCurrentFont->getName() );
								::std::map< OUString, ImplFontData* >::const_iterator it = pSalData->maJavaFontNameMapping.find( aFontName );
								if ( it != pSalData->maJavaFontNameMapping.end() )
								{
									com_sun_star_vcl_VCLFont *pVCLFont = (com_sun_star_vcl_VCLFont *)it->second->mpSysData;
									ffit->second = pVCLFont->deriveFont(pCurrentFont->getSize(), pCurrentFont->getOrientation(), pCurrentFont->isAntialiased(), pCurrentFont->isVertical(), pCurrentFont->getScaleX() );
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
	OUString aUserStr;
	OUString aUserPath;
	if ( Bootstrap::locateUserInstallation( aUserStr ) == Bootstrap::PATH_EXISTS && osl_getSystemPathFromFileURL( aUserStr.pData, &aUserPath.pData ) == osl_File_E_None )
	{
		ByteString aFontDir( aUserPath.getStr(), RTL_TEXTENCODING_UTF8 );
		if ( aFontDir.Len() )
		{
			aFontDir += ByteString( "/user/fonts", RTL_TEXTENCODING_UTF8 );
			FSRef aFontPath;
			FSSpec aFontSpec;
			if ( FSPathMakeRef( (const UInt8 *)aFontDir.GetBuffer(), &aFontPath, 0 ) == noErr && FSGetCatalogInfo( &aFontPath, kFSCatInfoNone, NULL, NULL, &aFontSpec, NULL) == noErr )
				ATSFontActivateFromFileSpecification( &aFontSpec, kATSFontContextGlobal, kATSFontFormatUnspecified, NULL, kATSOptionFlagsDefault, NULL );
		}
	}

	// Activate the fonts in the "share/fonts/truetype" directory
	OUString aExecStr;
	OUString aExecPath;
	if ( osl_getExecutableFile( &aExecStr.pData ) == osl_Process_E_None && osl_getSystemPathFromFileURL( aExecStr.pData, &aExecPath.pData ) == osl_File_E_None )
	{
		ByteString aFontDir( aExecPath.getStr(), RTL_TEXTENCODING_UTF8 );
		if ( aFontDir.Len() )
		{
			DirEntry aFontDirEntry( aFontDir );
			aFontDirEntry.ToAbs();
			aFontDir = ByteString( aFontDirEntry.GetPath().GetFull(), RTL_TEXTENCODING_UTF8 );
			aFontDir += ByteString( "/../share/fonts/truetype", RTL_TEXTENCODING_UTF8 );
			FSRef aFontPath;
			FSSpec aFontSpec;
			if ( FSPathMakeRef( (const UInt8 *)aFontDir.GetBuffer(), &aFontPath, 0 ) == noErr && FSGetCatalogInfo( &aFontPath, kFSCatInfoNone, NULL, NULL, &aFontSpec, NULL) == noErr )
				ATSFontActivateFromFileSpecification( &aFontSpec, kATSFontContextGlobal, kATSFontFormatUnspecified, NULL, kATSOptionFlagsDefault, NULL );
		}
	}

	ImplFontListChangedCallback( NULL, NULL );

	// Release any waiting thread
	aLoadNativeFontsCondition.set();

	bInLoad = false;
}

// =======================================================================

void SalGraphics::SetTextColor( SalColor nSalColor )
{
	maGraphicsData.mnTextColor = nSalColor | 0xff000000;
}

// -----------------------------------------------------------------------

USHORT SalGraphics::SetFont( ImplFontSelectData* pFont, int nFallbackLevel )
{
	SalData *pSalData = GetSalData();

	// Don't change the font for fallback levels as we need the first font
	// to properly determine the fallback font
	if ( !nFallbackLevel )
	{
		BOOL bAddBold = ( pFont->meWeight > WEIGHT_MEDIUM && pFont->mpFontData->meWeight <= WEIGHT_MEDIUM );
		BOOL bAddItalic = ( ( pFont->meItalic == ITALIC_OBLIQUE || pFont->meItalic == ITALIC_NORMAL ) && pFont->mpFontData->meItalic != ITALIC_OBLIQUE && pFont->mpFontData->meItalic != ITALIC_NORMAL );
		if ( bAddBold || bAddItalic )
		{
			BOOL bBold = ( pFont->meWeight > WEIGHT_MEDIUM );
			BOOL bItalic = ( pFont->meItalic == ITALIC_OBLIQUE || pFont->meItalic == ITALIC_NORMAL );
			com_sun_star_vcl_VCLFont *pVCLFont = (com_sun_star_vcl_VCLFont *)pFont->mpFontData->mpSysData;
			OUString aFontName = pVCLFont->getName();
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

			XubString aXubFontName( aFontName );
			::std::map< XubString, ImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( aXubFontName );
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

				XubString aXubFontName( aFontName );
				::std::map< XubString, ImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( aXubFontName );
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

					XubString aXubFontName( aFontName );
					::std::map< XubString, ImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( aXubFontName );
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
		::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator ffit = maGraphicsData.maFallbackFonts.find( nFallbackLevel );
		if ( ffit != maGraphicsData.maFallbackFonts.end() )
		{
			void *pNativeFont = ffit->second->getNativeFont();
			::std::map< void*, ImplFontData* >::const_iterator it = pSalData->maNativeFontMapping.find( pNativeFont );
			if ( it != pSalData->maNativeFontMapping.end() )
				pFont->mpFontData = it->second;
		}
	}

	pFont->maFoundName = pFont->mpFontData->maName;

	if ( !nFallbackLevel )
	{
		// Set font for graphics device
		if ( maGraphicsData.mpVCLFont )
			delete maGraphicsData.mpVCLFont;
		com_sun_star_vcl_VCLFont *pVCLFont = (com_sun_star_vcl_VCLFont *)pFont->mpFontData->mpSysData;
		maGraphicsData.mpVCLFont = pVCLFont->deriveFont( pFont->mnHeight, pFont->mnOrientation, !pFont->mbNonAntialiased, pFont->mbVertical, pFont->mnWidth ? (double)pFont->mnWidth / (double)pFont->mnHeight : 1.0 );
	}

	return 0;
}

// -----------------------------------------------------------------------

void SalGraphics::GetFontMetric( ImplFontMetricData* pMetric )
{
	ImplFontData *pData = NULL;
	SalData *pSalData = GetSalData();
	::std::map< XubString, ImplFontData* >::const_iterator it = pSalData->maFontNameMapping.find( pMetric->maName );
	if ( it != pSalData->maFontNameMapping.end() )
		pData = it->second;

	if ( maGraphicsData.mpVCLFont )
	{
		pMetric->mnAscent = maGraphicsData.mpVCLFont->getAscent();
		pMetric->mnDescent = maGraphicsData.mpVCLFont->getDescent();
		pMetric->mnLeading = maGraphicsData.mpVCLFont->getLeading();
		pMetric->mnOrientation = maGraphicsData.mpVCLFont->getOrientation();
		pMetric->mnWidth = maGraphicsData.mpVCLFont->getSize();
	}
	else
	{
		pMetric->mnAscent = 0;
		pMetric->mnDescent = 0;
		pMetric->mnLeading = 0;
		pMetric->mnOrientation = 0;
		pMetric->mnWidth = 0;
	}

	if ( pData )
	{
		pMetric->meCharSet = pData->meCharSet;
		pMetric->meFamily = pData->meFamily;
		pMetric->meItalic = pData->meItalic;
		pMetric->maName = pData->maName;
		pMetric->mePitch = pData->mePitch;
		pMetric->meWeight = pData->meWeight;
	}
	else
	{
		pMetric->meCharSet = RTL_TEXTENCODING_UNICODE;
		pMetric->meFamily = FAMILY_DONTKNOW;
		pMetric->meItalic = ITALIC_NONE;
		pMetric->maName = OUString();
		pMetric->mePitch = PITCH_VARIABLE;
		pMetric->meWeight = WEIGHT_NORMAL;
	}

	pMetric->mbDevice = FALSE;
	pMetric->mnFirstChar = 0;
	pMetric->mnLastChar = 255;
	pMetric->mnSlant = 0;
	pMetric->meType = TYPE_SCALABLE;
}

// -----------------------------------------------------------------------

ULONG SalGraphics::GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs )
{
	if ( !maGraphicsData.mpVCLFont )
		return 0;
	
	ImplKernPairData *pPair = pKernPairs;
	for ( ULONG i = 0; i < nPairs; i++ )
	{
		pPair->mnKern = maGraphicsData.mpVCLFont->getKerning( pPair->mnChar1, pPair->mnChar2 );
		pPair++;
	}

	return nPairs;
}

// -----------------------------------------------------------------------

ULONG SalGraphics::GetFontCodeRanges( sal_uInt32* pCodePairs ) const
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::GetFontCodeRanges not implemented\n" );
#endif
	return 0;
}

// -----------------------------------------------------------------------

void SalGraphics::GetDevFontList( ImplDevFontList* pList )
{
	SalData *pSalData = GetSalData();

	if ( !pLoadNativeFontsTimerUPP )
	{
		pLoadNativeFontsTimerUPP = NewEventLoopTimerUPP( LoadNativeFontsTimerCallback );

		// Only run the timer once since loading fonts is extremely expensive
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
	}

	// Iterate through fonts and add each to the font list
	for ( ::std::map< XubString, ImplFontData* >::const_iterator it = pSalData->maFontNameMapping.begin(); it != pSalData->maFontNameMapping.end(); ++it )
	{
		// Set default values
		ImplFontData *pFontData = new ImplFontData();
		pFontData->mpNext = it->second->mpNext;
		pFontData->mpSysData = it->second->mpSysData;
		pFontData->maName = it->second->maName;
		pFontData->mnWidth = it->second->mnWidth;
		pFontData->mnHeight = it->second->mnHeight;
		pFontData->meFamily = it->second->meFamily;
		pFontData->meCharSet = it->second->meCharSet;
		pFontData->mePitch = it->second->mePitch;
		pFontData->meWidthType = it->second->meWidthType;
		pFontData->meWeight = it->second->meWeight;
		pFontData->meItalic = it->second->meItalic;
		pFontData->meType = it->second->meType;
		pFontData->mnVerticalOrientation = it->second->mnVerticalOrientation;
		pFontData->mbOrientation = it->second->mbOrientation;
		pFontData->mbDevice = it->second->mbDevice;
		pFontData->mnQuality = it->second->mnQuality;
		pFontData->mbSubsettable = it->second->mbSubsettable;
		pFontData->mbEmbeddable = it->second->mbEmbeddable;

		// Add to list
		pList->Add( pFontData );
	}
}

// -----------------------------------------------------------------------

BOOL SalGraphics::GetGlyphBoundRect( long nIndex, Rectangle& rRect,
                                     const OutputDevice *pOutDev )
{
	if ( maGraphicsData.mpVCLFont )
	{
		rRect = maGraphicsData.mpVCLGraphics->getGlyphBounds( nIndex & GF_IDXMASK, maGraphicsData.mpVCLFont, nIndex & GF_ROTMASK );
		return TRUE;
	}
	else
	{
		rRect.SetEmpty();
		return FALSE;
	}
}

// -----------------------------------------------------------------------

BOOL SalGraphics::GetGlyphOutline( long nIndex, PolyPolygon& rPolyPoly,
                                   const OutputDevice *pOutDev )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::GetGlyphOutline not implemented\n" );
#endif
	rPolyPoly.Clear();
	return FALSE;
}

// -----------------------------------------------------------------------

void SalGraphics::GetDevFontSubstList( OutputDevice* pOutDev )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::GetDevFontSubstList not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

ImplFontData* SalGraphics::AddTempDevFont( const String& rFontFileURL,
                                           const String& rFontName )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::AddTempDevFont not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

void SalGraphics::RemovingFont( ImplFontData* )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::RemovingFont not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

BOOL SalGraphics::CreateFontSubset( const rtl::OUString& rToFile,
                                    ImplFontData* pFont, long* pGlyphIDs,
                                    sal_uInt8* pEncoding, sal_Int32* pWidths,
                                    int nGlyphs, FontSubsetInfo& rInfo )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::CreateFontSubset not implemented\n" );
#endif
	return FALSE;
}

// -----------------------------------------------------------------------

const void* SalGraphics::GetEmbedFontData( ImplFontData* pFont,
                                           const sal_Unicode* pUnicodes,
                                           sal_Int32* pWidths,
                                           FontSubsetInfo& rInfo,
                                           long* pDataLen )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::GetEmbedFontData not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

void SalGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::FreeEmbedFontData not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

const std::map< sal_Unicode, sal_Int32 >* SalGraphics::GetFontEncodingVector(
                ImplFontData* pFont,
                const std::map< sal_Unicode, rtl::OString >** pNonEncoded )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::GetFontEncodingVector not implemented\n" );
#endif
	if ( pNonEncoded )
		*pNonEncoded = NULL;
	return NULL;
}
