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

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <rtl/process.h>
#include <tools/color.hxx>
#include <vcl/fntstyle.hxx>
#include <vcl/fontcharmap.hxx>
#include <vcl/unohelp.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include "outdev.h"
#include "PhysicalFontCollection.hxx"
#include "sallayout.hxx"
#include "impfont.hxx"
#include "java/salatslayout.hxx"
#include "java/saldata.hxx"
#include "java/salgdi.h"
#include "java/salinst.h"
#include "quartz/utils.h"

#include "salgdi3_cocoa.h"

static void ImplFontListChanged();

static bool bFontListChangedObserverAdded = false;
static bool bNativeFontsLoaded = false;

using namespace basegfx;

@interface VCLLoadNativeFonts : NSObject
+ (id)create;
- (void)loadNativeFonts:(id)pObject;
@end

@implementation VCLLoadNativeFonts

+ (id)create
{
	VCLLoadNativeFonts *pRet = [[VCLLoadNativeFonts alloc] init];
	[pRet autorelease];
	return pRet;
}

- (void)loadNativeFonts:(id)pObject
{
	(void)pObject;

	ImplFontListChanged();
}

@end

// ============================================================================

static void ImplFontListChangedCallback( CFNotificationCenterRef, void*, CFStringRef, const void*, CFDictionaryRef )
{
	static bool bInCallback = false;

	if ( bInCallback )
		return;

	bInCallback = true;

	// Consume any duplicate notifications that are already in the queue
	while ( CFRunLoopRunInMode( kCFRunLoopDefaultMode, 0.1f, true ) == kCFRunLoopRunHandledSource )
		;

	// Queue font list update
	if ( !Application::IsShutDown() )
	{
		comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
			Application::PostUserEvent( LINK( nullptr, JavaPhysicalFontFace, RunNativeFontsTimer ) );
		rSolarMutex.release();
	}

	bInCallback = false;
}

// ----------------------------------------------------------------------------

static void ImplCachePlainFontMappings( NSFont *pNSFont )
{
	if ( !pNSFont )
		return;

	SalData *pSalData = GetSalData();
	sal_IntPtr nNativeFont = (sal_IntPtr)pNSFont;
	::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* >::const_iterator nfit = pSalData->maNativeFontMapping.find( nNativeFont );
	if ( nfit == pSalData->maNativeFontMapping.end() )
		return;

	JavaPhysicalFontFace *pFontData = nfit->second;

	NSFont *pPlainFont = NSFont_findPlainFont( pNSFont );
	if ( pPlainFont )
	{
		CFStringRef aPlainPSString = CTFontCopyPostScriptName( (CTFontRef)pPlainFont );
		if ( aPlainPSString )
		{
			OUString aPlainPSName = GetOUString( aPlainPSString );
			CFRelease( aPlainPSString );
			if ( aPlainPSName.getLength() )
			{
				::std::unordered_map< OUString, JavaPhysicalFontFace*, OUStringHash >::const_iterator jfnit = pSalData->maJavaFontNameMapping.find( aPlainPSName );
				if ( jfnit != pSalData->maJavaFontNameMapping.end() )
				{
					pSalData->maPlainFamilyNativeFontMapping[ nNativeFont ] = jfnit->second;

					sal_IntPtr nPlainNativeFont = jfnit->second->GetFontId();
					if ( pFontData->GetItalic() == ITALIC_OBLIQUE || pFontData->GetItalic() == ITALIC_NORMAL )
						pSalData->maItalicNativeFontMapping[ nPlainNativeFont ][ nNativeFont ] = pFontData;
					else
						pSalData->maUnitalicNativeFontMapping[ nPlainNativeFont ][ nNativeFont ] = pFontData;
				}
			}
		}

		[pPlainFont release];
	}
}

// ----------------------------------------------------------------------------

static void ImplFontListChanged()
{
	static bool bInLoad = false;

	if ( bInLoad )
		return;

	bInLoad = true;

	if ( !Application::IsShutDown() )
	{
		comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			SalData *pSalData = GetSalData();

			// Clean out caches
			SalATSLayout::ClearLayoutDataCache();
			JavaImplFont::clearNativeFonts();
			JavaPhysicalFontFace::ClearNativeFonts();
			for ( ::std::map< OUString, JavaPhysicalFontFace* >::const_iterator dfnit = pSalData->maFontNameMapping.begin(); dfnit != pSalData->maFontNameMapping.end(); ++dfnit )
				delete dfnit->second;
			pSalData->maFontNameMapping.clear();
			pSalData->maJavaFontNameMapping.clear();
			pSalData->maNativeFontMapping.clear();
			pSalData->maPlainFamilyNativeFontMapping.clear();
			pSalData->maItalicNativeFontMapping.clear();
			pSalData->maUnitalicNativeFontMapping.clear();
			pSalData->maSystemFont = vcl::Font();
			pSalData->maLabelFont = vcl::Font();
			pSalData->maMenuFont = vcl::Font();
			pSalData->maTitleBarFont = vcl::Font();

			if ( !Application::IsShutDown() )
			{
				NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

				// Update cached fonts
				NSArray *pFonts = NSFontManager_getAllFonts();
				if ( pFonts )
				{
					const OUString aCourier( "Courier" );
					const OUString aGillSansPS( "GillSans" );
					const OUString aGillSansBoldPS( "GillSans-Bold" );
					const OUString aGillSansBoldItalicPS( "GillSans-BoldItalic" );
					const OUString aGillSansItalicPS( "GillSans-Italic" );
					const OUString aLastResort( "LastResort" );
					const OUString aMincho( "Mincho" );
					const OUString aMing( "Ming" );
					const OUString aMyungjo( "Myungjo" );
					const OUString aRoman( "Roman" );
					const OUString aSans( "Sans" );
					const OUString aSerif( "Serif" );
					const OUString aSong( "Song" );
					const OUString aSung( "Sung" );
					const OUString aNeoSymbol( "Neo Symbol" );
					const OUString aNeo3Symbol( "Neo3Symbol" );
					const OUString aNeo4Symbol( "Neo4Symbol" );
					const OUString aOpenSymbol( "OpenSymbol" );
					const OUString aRegular( " Regular" );
					const OUString aStarSymbol( "StarSymbol" );
					const OUString aTimes( "Times" );
					const OUString aTimesNewRomanPS( "TimesNewRomanPSMT" );
					const OUString aTimesNewRomanBoldPS( "TimesNewRomanPS-BoldMT" );
					const OUString aTimesNewRomanBoldItalicPS( "TimesNewRomanPS-BoldItalicMT" );
					const OUString aTimesNewRomanItalicPS( "TimesNewRomanPS-ItalicMT" );
					const OUString aTimesRoman( "Times Roman" );

					unsigned int i = 0;
					unsigned int nCount = [pFonts count];

					sal_uInt32 nActualCount = 0;
					for ( i = 0; i < nCount; i++ )
					{
						NSFont *pNSFont = [pFonts objectAtIndex:i];
						if ( !pNSFont )
							continue;
						CTFontRef aFont = (CTFontRef)pNSFont;

						// Get font attributes
						FontWeight nWeight = NSFontManager_weightOfFont( pNSFont );
						FontItalic nItalic = ( NSFontManager_isItalic( pNSFont ) ? ITALIC_NORMAL : ITALIC_NONE );
						FontWidth nWidth = NSFontManager_widthOfFont( pNSFont );
						FontPitch nPitch = ( NSFontManager_isFixedPitch( pNSFont ) ? PITCH_FIXED : PITCH_VARIABLE );

						CFStringRef aPSString = CTFontCopyPostScriptName( aFont );
						OUString aPSName;
						if ( aPSString )
						{
							aPSName = GetOUString( aPSString );
							CFRelease( aPSString );
						}

						if ( !aPSName.getLength() )
							continue;

						// Get the font family name
						CFStringRef aFamilyString = CTFontCopyFamilyName( aFont );
						OUString aFamilyName;
						if ( aFamilyString )
						{
							aFamilyName = GetOUString( aFamilyString );
							CFRelease( aFamilyString );
						}

						sal_IntPtr nNativeFont = (sal_IntPtr)aFont;

						OUString aDisplayName;
						CFStringRef aDisplayString = CTFontCopyFullName( aFont );
						if ( aDisplayString )
						{
							aDisplayName = GetOUString( aDisplayString );
							CFRelease( aDisplayString );
						}

						if ( !aDisplayName.getLength() )
							continue;

						FontAttributes aAttributes;
						aAttributes.AddMapName( aPSName );

						sal_Int32 nColon = aDisplayName.indexOf( (sal_Unicode)':' );
						if ( nColon >= 0 )
						{
							aDisplayName = OUString( aDisplayName.getStr(), nColon );
							aAttributes.AddMapName( aDisplayName );
						}

						// Ignore empty font names or font names that start
						// with a "."
						if ( !aDisplayName.getLength() || aDisplayName.startsWith( "." ) )
							continue;

						if ( aDisplayName == aOpenSymbol || aDisplayName == aStarSymbol || aDisplayName == aNeoSymbol || aDisplayName == aNeo3Symbol )
						{
							// Don't allow Sun's symbol fonts our older
							// NeoOffice fonts to override our symbol font
							continue;
						}
						else if ( aDisplayName == aNeo4Symbol )
						{
							aDisplayName = OUString( aOpenSymbol );
							aAttributes.AddMapName( "Symbol" );
							aAttributes.AddMapName( aNeo4Symbol );
						}
						else if ( aDisplayName == aLastResort )
						{
							// Ignore this Java font as it will mess up
							// our font fallback process
							continue;
						}
						else if ( aDisplayName == aTimesRoman )
						{
							aAttributes.AddMapName( aTimes );
						}
						else if ( aDisplayName == aFamilyName + aRegular )
						{
							// Fix bug 3668 by adding family name to map
							// for "regular" fonts
							aAttributes.AddMapName( aFamilyName );
						}
						else if ( aPSName == aGillSansPS )
						{
							aAttributes.AddMapName( "Calibri" );
						}
						else if ( aPSName == aGillSansBoldPS )
						{
							aAttributes.AddMapName( "Calibri Bold" );
							aAttributes.AddMapName( "Calibri-Bold" );
						}
						else if ( aPSName == aGillSansBoldItalicPS )
						{
							aAttributes.AddMapName( "Calibri Bold Italic" );
							aAttributes.AddMapName( "Calibri-BoldItalic" );
						}
						else if ( aPSName == aGillSansItalicPS )
						{
							aAttributes.AddMapName( "Calibri Italic" );
							aAttributes.AddMapName( "Calibri-Italic" );
						}
						else if ( aPSName == aTimesNewRomanPS )
						{
							aAttributes.AddMapName( "Cambria" );
						}
						else if ( aPSName == aTimesNewRomanBoldPS )
						{
							aAttributes.AddMapName( "Cambria Bold" );
							aAttributes.AddMapName( "Cambria-Bold" );
						}
						else if ( aPSName == aTimesNewRomanBoldItalicPS )
						{
							aAttributes.AddMapName( "Cambria Bold Italic" );
							aAttributes.AddMapName( "Cambria-BoldItalic" );
						}
						else if ( aPSName == aTimesNewRomanItalicPS )
						{
							aAttributes.AddMapName( "Cambria Italic" );
							aAttributes.AddMapName( "Cambria-Italic" );
						}

						// Skip the font if we already have it
						::std::map< OUString, JavaPhysicalFontFace* >::iterator it = pSalData->maFontNameMapping.find( aDisplayName );
						if ( it != pSalData->maFontNameMapping.end() )
							continue;

						// Determine pitch and family type
						FontFamily nFamily;
						if ( nPitch == PITCH_FIXED )
							nFamily = FAMILY_MODERN;
						else if ( aPSName.indexOf( aSans ) >= 0 )
							nFamily = FAMILY_SWISS;
						else if ( aPSName.indexOf( aCourier ) >= 0 || aPSName.indexOf( aMincho ) >= 0 || aPSName.indexOf( aMing ) >= 0 || aPSName.indexOf( aMyungjo ) >= 0 || aPSName.indexOf( aRoman ) >= 0 || aPSName.indexOf( aSerif ) >= 0 || aPSName.indexOf( aTimes ) >= 0 || aPSName.indexOf( aSong ) >= 0 || aPSName.indexOf( aSung ) >= 0 )
							nFamily = FAMILY_ROMAN;
						else
							nFamily = FAMILY_SWISS;

						aAttributes.SetFamilyName( aDisplayName );
						aAttributes.SetWeight( nWeight );
						aAttributes.SetItalic( nItalic );
						aAttributes.SetFamilyType( nFamily );
						aAttributes.SetPitch( nPitch );
						aAttributes.SetWidthType( nWidth );
						aAttributes.SetSymbolFlag( false );
						aAttributes.SetQuality( 0 );

						JavaPhysicalFontFace *pFontData = new JavaPhysicalFontFace( aAttributes, aPSName, nNativeFont, aFamilyName );

						// Check fonts that were previously marked as bad
						::std::map< OUString, OUString >::iterator bfnit = JavaPhysicalFontFace::maBadNativeFontNameMap.find( aPSName );
 						if ( bfnit != JavaPhysicalFontFace::maBadNativeFontNameMap.end() )
						{
							if ( JavaPhysicalFontFace::IsBadFont( pFontData, false ) )
							{
								delete pFontData;
								continue;
							}
							else
							{
								JavaPhysicalFontFace::maBadNativeFontNameMap.erase( bfnit );
							}
						}

						pSalData->maFontNameMapping[ aDisplayName ] = pFontData;

						// Multiple native fonts can map to the same font
						// due to disabling and reenabling of fonts with
						// the same name. Also, note that multiple font
						// names can map to a single native font so do not
						// rely on the native font to look up the font name.
						pSalData->maNativeFontMapping[ nNativeFont ] = pFontData;
						pSalData->maJavaFontNameMapping[ aPSName ] = pFontData;

						nActualCount++;
					}

					// Cache matching bold, italic, and bold italic fonts
					for ( i = 0; i < nCount; i++ )
					{
						NSFont *pNSFont = [pFonts objectAtIndex:i];
						if ( !pNSFont )
							continue;

						// Fix bug 3031 by caching the plain variant of the
						// font family
						ImplCachePlainFontMappings( pNSFont );
					}

					[pFonts release];
				}

				// Cache system font
				CTFontRef aSystemFont = (CTFontRef)[NSFont systemFontOfSize:0];
				if ( aSystemFont )
				{
					CFStringRef aDisplayString = CTFontCopyFullName( aSystemFont );
					if ( aDisplayString )
					{
						OUString aDisplayName = GetOUString( aDisplayString );
						CFRelease( aDisplayString );
						if ( aDisplayName.getLength() )
							pSalData->maSystemFont = vcl::Font( aDisplayName, Size( 0, (long)( CTFontGetSize( aSystemFont ) + 0.5f ) ) );
					}
				}

				// Cache label font
				CTFontRef aLabelFont = (CTFontRef)[NSFont labelFontOfSize:0];
				if ( aLabelFont )
				{
					CFStringRef aDisplayString = CTFontCopyFullName( aLabelFont );
					if ( aDisplayString )
					{
						OUString aDisplayName = GetOUString( aDisplayString );
						CFRelease( aDisplayString );
						if ( aDisplayName.getLength() )
							pSalData->maLabelFont = vcl::Font( aDisplayName, Size( 0, (long)( CTFontGetSize( aLabelFont ) + 0.5f ) ) );
					}
				}

				// Cache menu font
				CTFontRef aMenuFont = (CTFontRef)[NSFont menuFontOfSize:0];
				if ( aMenuFont )
				{
					CFStringRef aDisplayString = CTFontCopyFullName( aMenuFont );
					if ( aDisplayString )
					{
						OUString aDisplayName = GetOUString( aDisplayString );
						CFRelease( aDisplayString );
						if ( aDisplayName.getLength() )
							pSalData->maMenuFont = vcl::Font( aDisplayName, Size( 0, (long)( CTFontGetSize( aMenuFont ) + 0.5f ) ) );
					}
				}

				// Cache titlebar font
				CTFontRef aTitleBarFont = (CTFontRef)[NSFont titleBarFontOfSize:0];
				if ( aTitleBarFont )
				{
					CFStringRef aDisplayString = CTFontCopyFullName( aTitleBarFont );
					if ( aDisplayString )
					{
						OUString aDisplayName = GetOUString( aDisplayString );
						CFRelease( aDisplayString );
						if ( aDisplayName.getLength() )
							pSalData->maTitleBarFont = vcl::Font( aDisplayName, Size( 0, (long)( CTFontGetSize( aTitleBarFont ) + 0.5f ) ) );
					}
				}

				[pPool release];
			}

			// Fix bug 3095 by handling font change notifications
			if ( !bFontListChangedObserverAdded )
			{
				CFNotificationCenterAddObserver( CFNotificationCenterGetLocalCenter(), NULL, ImplFontListChangedCallback, kCTFontManagerRegisteredFontsChangedNotification, NULL, CFNotificationSuspensionBehaviorCoalesce );
				bFontListChangedObserverAdded = true;
			}

			OutputDevice::ImplUpdateAllFontData( true );

			rSolarMutex.release();
		}
	}

	bNativeFontsLoaded = true;
	bInLoad = false;
}

// -----------------------------------------------------------------------

static const JavaPhysicalFontFace *ImplGetFontVariant( const JavaPhysicalFontFace *pFontData, FontWeight nWeight, bool bItalic, FontWidth nWidthType )
{
	if ( !pFontData || nWeight <= WEIGHT_DONTKNOW || nWeight > WEIGHT_BLACK )
		return pFontData;

	int nWeightDiff = pFontData->GetWeight() - nWeight;
	if ( !nWeightDiff && bItalic == ( pFontData->GetItalic() == ITALIC_OBLIQUE || pFontData->GetItalic() == ITALIC_NORMAL ) )
		return pFontData;

	if ( nWidthType <= WIDTH_DONTKNOW || nWidthType > WIDTH_ULTRA_EXPANDED )
		nWidthType = WIDTH_NORMAL;

	SalData *pSalData = GetSalData();
	const ::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* > *pFontVariants = NULL;
	if ( bItalic )
	{
		::std::unordered_map< sal_IntPtr, ::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* > >::const_iterator nfit = pSalData->maItalicNativeFontMapping.find( pFontData->GetFontId() );
		if ( nfit != pSalData->maItalicNativeFontMapping.end() )
		{
			pFontVariants = &nfit->second;
		}
		else
		{
			// Fix bug 1813 by using unitalic variant if this variant is NULL
			nfit = pSalData->maUnitalicNativeFontMapping.find( pFontData->GetFontId() );
			if ( nfit != pSalData->maUnitalicNativeFontMapping.end() )
				pFontVariants = &nfit->second;
		}
	}
	else
	{
		::std::unordered_map< sal_IntPtr, ::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* > >::const_iterator nfit = pSalData->maUnitalicNativeFontMapping.find( pFontData->GetFontId() );
		if ( nfit != pSalData->maUnitalicNativeFontMapping.end() )
		{
			pFontVariants = &nfit->second;
		}
		else
		{
			// Fix bug 1813 by using italic variant if this variant is NULL
			nfit = pSalData->maItalicNativeFontMapping.find( pFontData->GetFontId() );
			if ( nfit != pSalData->maItalicNativeFontMapping.end() )
				pFontVariants = &nfit->second;
		}
	}

	if ( pFontVariants )
	{
		const JavaPhysicalFontFace *pBestFontData = pFontData;
		bool bBestItalic = ( pBestFontData->GetItalic() == ITALIC_OBLIQUE || pBestFontData->GetItalic() == ITALIC_NORMAL );
		int nBestAbsWeightDiff = abs( pBestFontData->GetWeight() - nWeight );
		int nBestAbsWidthTypeDiff = ( pBestFontData->GetWidthType() - nWidthType );

		const JavaPhysicalFontFace *pBestWidthFontData = ( pFontData->GetWidthType() == nWidthType ? pFontData : NULL );
		bool bBestWidthItalic = ( pBestWidthFontData ? ( pBestWidthFontData->GetItalic() == ITALIC_OBLIQUE || pBestWidthFontData->GetItalic() == ITALIC_NORMAL ) : bItalic );
		int nBestWidthAbsWeightDiff = ( pBestWidthFontData ? abs( pBestWidthFontData->GetWeight() - nWeight ) : FontWeight_FORCE_EQUAL_SIZE );

		for ( ::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* >::const_iterator fvit = pFontVariants->begin(); fvit != pFontVariants->end(); ++fvit )
		{
			bool bCurrentItalic = ( fvit->second->GetItalic() == ITALIC_OBLIQUE || fvit->second->GetItalic() == ITALIC_NORMAL );
			int nCurrentAbsWeightDiff = abs( fvit->second->GetWeight() - nWeight );
			int nCurrentAbsWidthTypeDiff = abs( fvit->second->GetWidthType() - nWidthType );
			if ( ( bBestItalic != bItalic && bCurrentItalic == bItalic ) || nBestAbsWeightDiff > nCurrentAbsWeightDiff || ( nBestAbsWeightDiff == nCurrentAbsWeightDiff && nBestAbsWidthTypeDiff > nCurrentAbsWidthTypeDiff ) )
			{
				pBestFontData = fvit->second;
				bBestItalic = bCurrentItalic;
				nBestAbsWeightDiff = nCurrentAbsWeightDiff;
				nBestAbsWidthTypeDiff = nCurrentAbsWidthTypeDiff;
			}

			if ( !nCurrentAbsWidthTypeDiff && ( ( pBestWidthFontData && bBestWidthItalic != bItalic && bCurrentItalic == bItalic ) || nBestWidthAbsWeightDiff > nCurrentAbsWeightDiff ) )
			{
				pBestWidthFontData = fvit->second;
				bBestWidthItalic = bCurrentItalic;
				nBestWidthAbsWeightDiff = nCurrentAbsWeightDiff;
			}
		}

		// Use the same font face if it has a weight that is close enough
		if ( pBestWidthFontData && nBestWidthAbsWeightDiff <= 1 )
			pFontData = pBestWidthFontData;
		else
			pFontData = pBestFontData;
	}

	return pFontData;
}

// =======================================================================

::std::map< sal_IntPtr, sal_IntPtr > JavaPhysicalFontFace::maBadNativeFontCheckedMap;

// -----------------------------------------------------------------------

::std::map< sal_IntPtr, sal_IntPtr > JavaPhysicalFontFace::maBadNativeFontIDMap;

// -----------------------------------------------------------------------

::std::map< OUString, OUString > JavaPhysicalFontFace::maBadNativeFontNameMap;

// -----------------------------------------------------------------------

void JavaPhysicalFontFace::ClearNativeFonts()
{
	maBadNativeFontCheckedMap.clear();
	maBadNativeFontIDMap.clear();
}

// -----------------------------------------------------------------------

void JavaPhysicalFontFace::HandleBadFont( const JavaPhysicalFontFace *pFontData )
{
	if ( !pFontData )
		return;

	// Fix bug 3446 by reloading native fonts without any known bad fonts
	bool bReloadFonts = false;
	::std::map< sal_IntPtr, sal_IntPtr >::const_iterator bit = maBadNativeFontIDMap.find( pFontData->mnNativeFontID );
	if ( bit == maBadNativeFontIDMap.end() )
	{
		bReloadFonts = true;
		maBadNativeFontIDMap[ pFontData->mnNativeFontID ] = pFontData->mnNativeFontID;
		maBadNativeFontNameMap[ pFontData->maFontName ] = pFontData->maFontName;
	}

	// Find any fonts that have the same family as the current font and mark
	// those as bad fonts
	SalData *pSalData = GetSalData();
	for ( ::std::map< OUString, JavaPhysicalFontFace* >::const_iterator it = pSalData->maFontNameMapping.begin(); it != pSalData->maFontNameMapping.end(); ++it )
	{
		if ( it->second->maFamilyName == pFontData->maFamilyName )
		{
			bit = maBadNativeFontIDMap.find( it->second->mnNativeFontID );
			if ( bit == maBadNativeFontIDMap.end() )
			{
				bReloadFonts = true;
				maBadNativeFontIDMap[ it->second->mnNativeFontID ] = it->second->mnNativeFontID;
				maBadNativeFontNameMap[ it->second->maFontName ] = it->second->maFontName;
			}
		}
	}

	// Fix bug 3576 by updating the fonts after all currently queued
	// event are dispatched
	if ( bReloadFonts )
		Application::PostUserEvent( LINK( nullptr, JavaPhysicalFontFace, RunNativeFontsTimer ) );
}

// -----------------------------------------------------------------------

bool JavaPhysicalFontFace::IsBadFont( const JavaPhysicalFontFace *pFontData, bool bHandleIfBadFont )
{
	bool bRet = false;

	if ( !pFontData )
		return bRet;

	::std::map< sal_IntPtr, sal_IntPtr >::const_iterator cit = maBadNativeFontCheckedMap.find( pFontData->mnNativeFontID );
	if ( cit == maBadNativeFontCheckedMap.end() )
	{
		maBadNativeFontCheckedMap[ pFontData->mnNativeFontID ] = pFontData->mnNativeFontID;

		CFStringRef aPSString = CTFontCopyPostScriptName( (CTFontRef)pFontData->mnNativeFontID );
		if ( aPSString )
		{
			if ( CFStringGetLength( aPSString ) )
			{
				NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

				NSBundle *pBundle = [NSBundle mainBundle];
				if ( pBundle && pBundle.bundlePath )
				{
					NSString *pCommandPath = [NSString stringWithFormat:@"%@/Contents/program/checknativefont", pBundle.bundlePath];
					if ( pCommandPath && !access( [pCommandPath UTF8String], R_OK | X_OK ) )
					{
						char *pCommandArgs[ 3 ];
						pCommandArgs[ 0 ] = (char *)[pCommandPath UTF8String];
						pCommandArgs[ 1 ] = (char *)[(NSString *)aPSString UTF8String];
						pCommandArgs[ 2 ] = NULL;

						// Execute the checknativefont command in child process
						pid_t pid = fork();
						if ( !pid )
						{
							close( 0 );
							execvp( [pCommandPath UTF8String], pCommandArgs );
							_exit( 0 );
						}
						else
						{
							// Get child process status
							int status;
							while ( waitpid( pid, &status, 0 ) > 0 && EINTR == errno )
								usleep( 10 );

							// If the child process crashes, it is a bad font.
							// Check for termination from any signal, not just
							// SIGSEGV.
							if ( EINTR != errno && WIFSIGNALED( status ) )
							{
								bRet = true;

								if ( bHandleIfBadFont )
								{
									SalData *pSalData = GetSalData();
									::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* >::const_iterator nit = pSalData->maNativeFontMapping.find( pFontData->mnNativeFontID );
									if ( nit != pSalData->maNativeFontMapping.end() )
										JavaPhysicalFontFace::HandleBadFont( nit->second );
								}
							}
						}
					}
				}

				[pPool release];
			}

			CFRelease( aPSString );
		}
	}
	else
	{
		::std::map< sal_IntPtr, sal_IntPtr >::const_iterator bit = maBadNativeFontIDMap.find( pFontData->mnNativeFontID );
		if ( bit != maBadNativeFontIDMap.end() )
			bRet = true;
	}

	return bRet;
}

// -----------------------------------------------------------------------

IMPL_STATIC_LINK( JavaPhysicalFontFace, RunNativeFontsTimer, void*, /* pCallData */, void )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	sal_uLong nCount = Application::ReleaseSolarMutex();
	VCLLoadNativeFonts *pVCLLoadNativeFonts = [VCLLoadNativeFonts create];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLLoadNativeFonts performSelectorOnMainThread:@selector(loadNativeFonts:) withObject:pVCLLoadNativeFonts waitUntilDone:YES modes:pModes];
	Application::AcquireSolarMutex( nCount );

	[pPool release];
}

// -----------------------------------------------------------------------

JavaPhysicalFontFace::JavaPhysicalFontFace( const FontAttributes& rAttributes, const OUString& rFontName, sal_IntPtr nNativeFontID, const OUString& rFamilyName ) : PhysicalFontFace( rAttributes ), maFontName( rFontName ), mnNativeFontID( nNativeFontID ), maFamilyName( rFamilyName ), mpParent( NULL )
{
	if ( mnNativeFontID )
		CFRetain( (CTFontRef)mnNativeFontID );

	// [ed] 11/1/04 Scalable fonts should always report their width and height
	// as zero. The single size zero causes higher-level font elements to treat
	// fonts as infinitely scalable and provide lists of default font sizes.
	// The size of zero matches the unx implementation. Bug 196.
	SetBitmapSize( 0, 0 );
}

// -----------------------------------------------------------------------

JavaPhysicalFontFace::~JavaPhysicalFontFace()
{
	if ( mpParent )
		mpParent->maChildren.remove( this );

	if ( mnNativeFontID )
		CFRelease( (CTFontRef)mnNativeFontID );

	if ( maChildren.size() )
	{
		// Copy list as deleting a child will try to remove the child from
		// the list
		::std::list< JavaPhysicalFontFace* > aChildren( maChildren );
		maChildren.clear();
		while ( aChildren.size() )
		{
			delete aChildren.front();
			aChildren.pop_front();
		}
	}
}

// -----------------------------------------------------------------------

LogicalFontInstance* JavaPhysicalFontFace::CreateFontInstance( FontSelectPattern& rData ) const
{
    return new LogicalFontInstance( rData );
}

// -----------------------------------------------------------------------

PhysicalFontFace* JavaPhysicalFontFace::Clone() const
{
	return new JavaPhysicalFontFace( *this, maFontName, mnNativeFontID, maFamilyName );
}

// -----------------------------------------------------------------------

sal_IntPtr JavaPhysicalFontFace::GetFontId() const
{
	return mnNativeFontID;
}

// =======================================================================

void JavaSalGraphics::SetTextColor( SalColor nSalColor )
{
	mnTextColor = nSalColor | 0xff000000;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::SetFont( FontSelectPattern* pFont, int nFallbackLevel )
{
	if ( !pFont || !pFont->mpFontData )
		return;

	SalData *pSalData = GetSalData();

	const JavaPhysicalFontFace *pFontData = NULL;

	// System font names returned by NSFont are usually excluded from the
	// font list in JavaSalGraphics::GetDevFontList() but such fonts are set
	// as the default fonts in JavaSalFrame::UpdateSettings() so, if the
	// requested font name matches an excluded font, use the excluded font
	::std::map< OUString, JavaPhysicalFontFace* >::const_iterator fnit = pSalData->maFontNameMapping.find( pFont->GetFamilyName() );
	if ( fnit != pSalData->maFontNameMapping.end() && ( !fnit->second->maFamilyName.getLength() || fnit->second->maFamilyName.startsWith( "." ) ) )
		pFontData = fnit->second;

	if ( !pFontData )
	{
		pFontData = dynamic_cast<const JavaPhysicalFontFace *>( pFont->mpFontData );
		if ( !pFontData )
		{
			if ( pSalData->maJavaFontNameMapping.size() )
			{
				pFontData = pSalData->maJavaFontNameMapping.begin()->second;
			}
			else
			{
				// We should never get here as there should always be at least
				// one font
				return;
			}
		}
	}

	if ( nFallbackLevel )
	{
		// Retrieve the fallback font if one has been set by a text layout
		::std::unordered_map< int, JavaImplFont* >::const_iterator ffit = maFallbackFonts.find( nFallbackLevel );
		if ( ffit != maFallbackFonts.end() )
		{
			sal_IntPtr nNativeFont = ffit->second->getNativeFont();
			::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* >::const_iterator it = pSalData->maNativeFontMapping.find( nNativeFont );
			if ( it != pSalData->maNativeFontMapping.end() )
				pFontData = it->second;
		}
	}

	if ( !nFallbackLevel )
	{
		mnFontFamily = pFont->GetFamilyType();
		mnFontWeight = pFont->GetWeight();
		mbFontItalic = ( pFont->GetItalic() == ITALIC_OBLIQUE || pFont->GetItalic() == ITALIC_NORMAL );
		mnFontPitch = pFont->GetPitch();

		// Cache font data's width type, as the request width type will always
		// be medium
		mnFontWidthType = pFontData->GetWidthType();
	}

	// Fix bugs 1813, 2964, 2968, 2971, and 2972 by trying to find a matching
	// bold and/or italic font even if we are in a fallback level
	const JavaPhysicalFontFace *pOldFontData = pFontData;
	bool bAddBold = ( mnFontWeight > WEIGHT_MEDIUM && pFontData->GetWeight() <= WEIGHT_MEDIUM );
	bool bAddItalic = ( mbFontItalic && pFontData->GetItalic() != ITALIC_OBLIQUE && pFontData->GetItalic() != ITALIC_NORMAL );
	FontWeight nSetWeight = mnFontWeight;
	bool bSetItalic = mbFontItalic;
	if ( nFallbackLevel || bAddBold || bAddItalic )
	{
		if ( bAddBold || bAddItalic )
		{
			// If we are forcing the font to a bold or italic variant but not
			// both, do not allow the other variant to change
			if ( !bAddBold )
				nSetWeight = pFontData->GetWeight();
			if ( !bAddItalic )
				bSetItalic = ( pFontData->GetItalic() == ITALIC_OBLIQUE || pFontData->GetItalic() == ITALIC_NORMAL );
		}

		// Remove any bold or italic variants so that we don't get drifting to
		// bold or italic in fallback levels where none was requested by
		// matching bold or italic variants within the same font family
		::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* >::const_iterator pfit = pSalData->maPlainFamilyNativeFontMapping.find( pFontData->GetFontId() );
		if ( pfit != pSalData->maPlainFamilyNativeFontMapping.end() )
			pFontData = pfit->second;
	}

	pFontData = ImplGetFontVariant( pFontData, nSetWeight, bSetItalic, mnFontWidthType );

	int nNativeFont = pFontData->GetFontId();
	if ( nNativeFont != pOldFontData->GetFontId() )
	{
		if ( nFallbackLevel )
		{
			// Avoid selecting a font that has already been used
			for ( ::std::unordered_map< int, JavaImplFont* >::const_iterator ffit = maFallbackFonts.begin(); ffit != maFallbackFonts.end(); ++ffit )
			{
				if ( ffit->first < nFallbackLevel && ffit->second->getNativeFont() == nNativeFont )
				{
					pFontData = pOldFontData;
					break;
				}
			}
		}
	}
	else
	{
		pFontData = pOldFontData;
	}

	// Check that the font still exists as it might have been disabled or
	// removed by the ATS server
	::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* >::const_iterator nfit = pSalData->maNativeFontMapping.find( pFont->mpFontData->GetFontId() );
	if ( nfit == pSalData->maNativeFontMapping.end() )
	{
		::std::unordered_map< OUString, JavaPhysicalFontFace*, OUStringHash >::const_iterator jfnit = pSalData->maJavaFontNameMapping.find( pFontData->maFontName );
		if ( jfnit != pSalData->maJavaFontNameMapping.end() )
		{
			pFontData = jfnit->second;
		}
		else if ( pSalData->maJavaFontNameMapping.size() )
		{
			pFontData = pSalData->maJavaFontNameMapping.begin()->second;
		}
		else
		{
			// We should never get here as there should always be at least one
			// font
			return;
		}
	}

	::std::unordered_map< int, JavaImplFont* >::iterator ffit = maFallbackFonts.find( nFallbackLevel );
	if ( ffit != maFallbackFonts.end() )
	{
		delete ffit->second;
		maFallbackFonts.erase( ffit );
	}

	maFallbackFonts[ nFallbackLevel ] = new JavaImplFont( pFontData->maFontName, pFont->mfExactHeight, pFont->mnOrientation, !pFont->mbNonAntialiased, pFont->mbVertical, pFont->mnWidth ? (double)pFont->mnWidth / (double)pFont->mfExactHeight : 1.0 );
	maFallbackFontSizes[ nFallbackLevel ] = Size( pFont->mnWidth, pFont->mnHeight );

	// Update the native font as Java may be using a different font
	pFontData->mnNativeFontID = maFallbackFonts[ nFallbackLevel ]->getNativeFont();

	if ( !nFallbackLevel )
	{
		// Set font data for graphics device
		if ( mpFontData )
			delete mpFontData;
		mpFontData = (JavaPhysicalFontFace *)pFontData->Clone();

		// Set font for graphics device
		sal_IntPtr nOldNativeFont = 0;
		if ( mpFont )
		{
			nOldNativeFont = mpFont->getNativeFont();
			delete mpFont;
		}
		mpFont = new JavaImplFont( maFallbackFonts[ nFallbackLevel ] );

		// Fix bug 3446 by checking if the new font is a bad font
		if ( mpFont->getNativeFont() != nOldNativeFont )
		{
			// If the font is a bad font, select a different font
			ImplFontMetricDataRef xMetricData( new ImplFontMetricData( *pFont ) );
			GetFontMetric( xMetricData, 0 );
			::std::map< sal_IntPtr, sal_IntPtr >::const_iterator bit = JavaPhysicalFontFace::maBadNativeFontIDMap.find( mpFont->getNativeFont() );
			if ( bit != JavaPhysicalFontFace::maBadNativeFontIDMap.end() )
			{
				for ( ::std::unordered_map< OUString, JavaPhysicalFontFace*, OUStringHash >::const_iterator jfnit = pSalData->maJavaFontNameMapping.begin(); jfnit != pSalData->maJavaFontNameMapping.end(); ++jfnit )
				{
					pFontData = ImplGetFontVariant( jfnit->second, mnFontWeight, mbFontItalic, mnFontWidthType );

					// Reset font
					delete maFallbackFonts[ nFallbackLevel ];
					delete mpFont;
					delete mpFontData;
					maFallbackFonts[ nFallbackLevel ] = new JavaImplFont( pFontData->maFontName, pFont->mfExactHeight, pFont->mnOrientation, !pFont->mbNonAntialiased, pFont->mbVertical, pFont->mnWidth ? (double)pFont->mnWidth / (double)pFont->mfExactHeight : 1.0 );
					mpFont = new JavaImplFont( maFallbackFonts[ nFallbackLevel ] );
					mpFontData = (JavaPhysicalFontFace *)pFontData->Clone();

					GetFontMetric( xMetricData, 0 );
					bit = JavaPhysicalFontFace::maBadNativeFontIDMap.find( mpFont->getNativeFont() );
					if ( bit == JavaPhysicalFontFace::maBadNativeFontIDMap.end() )
						break;
				}
			}
		}

		// Clone the new font data and make it a child of the requested font
		// data so that it will eventually get deleted
		if ( pFont->mpFontData != pFontData )
		{
			JavaPhysicalFontFace *pChildFontData = (JavaPhysicalFontFace *)pFontData->Clone();
			if ( pChildFontData )
			{
				const JavaPhysicalFontFace *pJavaFontData = dynamic_cast<const JavaPhysicalFontFace *>( pFont->mpFontData );
				if ( pJavaFontData )
				{
					// Fix stack overflow crash due to excessively long chains
					// of child fonts by replacing the last chained font
					// instead of adding to the chain
					if ( pJavaFontData->mpParent )
					{
						delete pJavaFontData;
					}
					else
					{
						pChildFontData->mpParent = pJavaFontData;
						pJavaFontData->maChildren.push_back( pChildFontData );
					}
				}

				pFont->mpFontData = pChildFontData;
			}
		}
	}
	else
	{
		// No need to clone as the select data is merely temporary data in
		// fallback levels
		pFont->mpFontData = pFontData;
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::GetFontMetric( ImplFontMetricDataRef& rMetric, int /* nFallbackLevel */ )
{
	float fFontSize = 0;
	if ( mpFont )
	{
		fFontSize = mpFont->getSize();

		// Fix bug 3446 by only overriding the width if it is positive. Apply
		// font scale to be consistent with OOo's Aqua implementation.
		long nWidth = (long)( ( mpFont->getSize() * mpFont->getScaleX() ) + 0.5 );
		if ( nWidth >= 0 )
			rMetric->SetWidth( nWidth );
		rMetric->SetOrientation( mpFont->getOrientation() );
	}

	if ( mpFontData )
	{
		if ( rMetric->GetWidth() )
		{
			// Fix scaling of the line height in Writer by creating a font with
			// the same font size, not the scaled width
			CTFontRef aFont = CTFontCreateCopyWithAttributes( (CTFontRef)mpFontData->mnNativeFontID, fFontSize, NULL, NULL );
			if ( aFont )
			{
				// Mac OS X seems to overstate the leading for some fonts
				// (usually CJK fonts like Hiragino) so fix fix bugs 2827 and
				// 2847 by adding combining the leading with descent
				rMetric->SetAscent( (long)( CTFontGetAscent( aFont ) + 0.5 ) );
				// Fix bug 2881 by handling cases where font does not have
				// negative descent
				long nDescent = (long)( CTFontGetLeading( aFont ) + fabs( CTFontGetDescent( aFont ) ) + 0.5 );
				if ( nDescent < 0 )
					nDescent = 0;
				rMetric->SetDescent( nDescent );

				CFRelease( aFont );
			}
			else
			{
				// Fix bug 3446 by treating a font that don't have horizontal
				// metrics as a bad font
				JavaPhysicalFontFace::HandleBadFont( mpFontData );
			}

			if ( rMetric->GetAscent() < 1 )
			{
				long nAscent = rMetric->GetWidth() - rMetric->GetDescent();
				if ( nAscent < 1 )
				{
					nAscent = rMetric->GetDescent();
					rMetric->SetDescent( 0 );
				}
				rMetric->SetAscent( nAscent );
			}
		}

		rMetric->SetFamilyName( mpFontData->GetFamilyName() );
		rMetric->SetStyleName( mpFontData->GetStyleName() );
		rMetric->SetWeight( mpFontData->GetWeight() );
		rMetric->SetFamilyType( mpFontData->GetFamilyType() );
		rMetric->SetItalic( mpFontData->GetItalic() );
		rMetric->SetPitch( mpFontData->GetPitch() );
		rMetric->SetSymbolFlag( mpFontData->IsSymbolFont() );
	}
	else
	{
		rMetric->SetFamilyName( OUString() );
		rMetric->SetStyleName( OUString() );
		rMetric->SetWeight( WEIGHT_NORMAL );
		rMetric->SetFamilyType( FAMILY_DONTKNOW );
		rMetric->SetItalic( ITALIC_NONE );
		rMetric->SetPitch( PITCH_VARIABLE );
		rMetric->SetSymbolFlag( false );
	}

	rMetric->SetInternalLeading( 0 );
	rMetric->SetExternalLeading( 0 );
	rMetric->SetSlant( 0 );

	// Fix missing kashidas by setting the font's minimum kashida width to a
	// non-zero width
	rMetric->SetMinKashida( 1 );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::GetDevFontList( PhysicalFontCollection* pList )
{
	// Only run the timer once since loading fonts is extremely expensive
	if ( !bNativeFontsLoaded )
	{
		// Invoke the native shutdown cancelled handler
		JavaSalEventQueue::setShutdownDisabled( sal_True );
		LINK( nullptr, JavaPhysicalFontFace, RunNativeFontsTimer ).Call( NULL );
		JavaSalEventQueue::setShutdownDisabled( sal_False );
	}

	SalData *pSalData = GetSalData();

	// Iterate through fonts and add each to the font list
	for ( ::std::map< OUString, JavaPhysicalFontFace* >::const_iterator it = pSalData->maFontNameMapping.begin(); it != pSalData->maFontNameMapping.end(); ++it )
	{
		// Ignore empty family names or family names that start with a "."
		if ( it->second->maFamilyName.getLength() && !it->second->maFamilyName.startsWith( "." ) )
			pList->Add( it->second->Clone() );
	}
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::GetGlyphBoundRect( const GlyphItem& rIndex, tools::Rectangle& rRect )
{
	rRect = tools::Rectangle( Point( 0, 0 ), Size( 0, 0 ) );

	JavaImplFont *pFont = NULL;

	if ( !rIndex.mnFallbackLevel )
	{
		pFont = mpFont;
	}
	else
	{
		// Retrieve the fallback font if one has been set by a text layout
		::std::unordered_map< int, JavaImplFont* >::const_iterator ffit = maFallbackFonts.find( rIndex.mnFallbackLevel );
		if ( ffit != maFallbackFonts.end() )
			pFont = ffit->second;
	}

	if ( pFont )
	{
		SalATSLayout::GetGlyphBounds( rIndex.maGlyphId, pFont, rRect );
		rRect.Justify();
	}

	// Fix bug 2191 by always returning true so that the OOo code doesn't
	// exeecute its "draw the glyph and see which pixels are black" code
	return true;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::GetGlyphOutline( const GlyphItem& /* rIndex */, basegfx::B2DPolyPolygon& rPolyPoly )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::GetGlyphOutline not implemented\n" );
#endif
	rPolyPoly.clear();
	return false;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::AddTempDevFont( PhysicalFontCollection* /* pList */, const OUString& /* rFileURL */, const OUString& /* rFontName */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::AddTempDevFont not implemented\n" );
#endif
	return false;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::CreateFontSubset( const OUString& /* rToFile */, const PhysicalFontFace* /* pFont */, const sal_GlyphId* /* pGlyphIDs */, const sal_uInt8* /* pEncoding */, sal_Int32* /* pWidths */, int /* nGlyphs */, FontSubsetInfo& /* rInfo */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::CreateFontSubset not implemented\n" );
#endif
	return false;
}

// -----------------------------------------------------------------------

const void* JavaSalGraphics::GetEmbedFontData( const PhysicalFontFace* /* pFont */, long* /* pDataLen */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::GetEmbedFontData not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::FreeEmbedFontData( const void* /* pData */, long /* nLen */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::FreeEmbedFontData not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalGraphics::GetGlyphWidths( const PhysicalFontFace* /* pFont */, bool /* bVertical */, std::vector< sal_Int32 >& rWidths, Ucs2UIntMap& rUnicodeEnc )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::GetGlyphWidths not implemented\n" );
#endif
	rWidths.clear();
	rUnicodeEnc.clear();
}

// -----------------------------------------------------------------------

const FontCharMapRef JavaSalGraphics::GetFontCharMap() const
{
	return FontCharMap::GetDefaultMap( false );
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::GetFontCapabilities( vcl::FontCapabilities& /* rFontCapabilities */ ) const
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::GetFontCapabilities not implemented\n" );
#endif
	return false;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::ClearDevFontCache()
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::ClearDevFontCache not implemented\n" );
#endif
}
