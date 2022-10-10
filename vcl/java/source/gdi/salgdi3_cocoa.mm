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
 *  Patrick Luby, July 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 Planamesa Inc.
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

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include "quartz/utils.h"

#include "salgdi3_cocoa.h"

static void AddFontToFontsDictWithAltName( NSFont *pFont, NSMutableDictionary *pFontDict, NSString *pAltName, BOOL bLocalization )
{
	if ( !pFont || !pFontDict )
		return;

	NSString *pKey = nil;
	if ( pAltName )
	{
		if ( [pAltName length] && ![pFontDict valueForKey:pAltName] )
			pKey = pAltName;
	}
	else
	{
		NSString *pFontName = [pFont fontName];
		if ( pFontName && [pFontName length] && ![pFontDict valueForKey:pFontName] )
			pKey = pFontName;
	}

	if ( !pKey )
		return;

	// Fix bug 3097 by using the printer font when the font is a bitmap font
	NSFont *pPrinterFont = [pFont printerFont];
	if ( pPrinterFont )
		[pFontDict setObject:pPrinterFont forKey:pKey];

	// Prevent infinite recursion
	if ( bLocalization )
		return;

	CFStringRef aUILang = CreateCFString( Application::GetSettings().GetUILanguageTag().getLanguage() );
	if ( aUILang )
	{
		CFStringRef aLang = nullptr;
		CFStringRef aLocalizedName = CTFontCopyLocalizedName( (CTFontRef)pFont, pAltName ? kCTFontFamilyNameKey : kCTFontFullNameKey, &aLang );
		if ( aLocalizedName )
		{
			if ( aLang && CFStringCompare( aUILang, aLang, 0 ) == kCFCompareEqualTo )
				AddFontToFontsDictWithAltName( pFont, pFontDict, (NSString *)aLocalizedName, YES );

			CFRelease( aLocalizedName );
		}

		CFRelease( aUILang );
	}
}

static void AddFontToFontsDict( NSFont *pFont, NSMutableDictionary *pFontDict )
{
	AddFontToFontsDictWithAltName( pFont, pFontDict, nil, NO );
}

NSFont *NSFont_findPlainFont( NSFont *pNSFont )
{
	NSFont *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager )
		{
			// Find matching unbolded, unitalicized, medium weight font
			NSFont *pNewNSFont = [pFontManager fontWithFamily:[pNSFont familyName] traits:( NSUnboldFontMask | NSUnitalicFontMask ) weight:5 size:[pNSFont pointSize]];
			if ( pNewNSFont && pNewNSFont != pNSFont )
			{
				[pNewNSFont retain];
				pRet = pNewNSFont;
			}
		}
	}

	[pPool release];

	return pRet;
}

NSDictionary *NSFontManager_getAllFonts()
{
	NSDictionary *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSFontManager *pFontManager = [NSFontManager sharedFontManager];
	if ( pFontManager )
	{
		NSArray<NSString *> *pFontNames = [pFontManager availableFonts];
		NSArray<NSString *> *pFamilyNames = [pFontManager availableFontFamilies];
		if ( pFontNames && pFamilyNames )
		{
			NSDictionary *pEmptyDict = [NSDictionary dictionary];
			NSMutableDictionary *pFontDict = [NSMutableDictionary dictionaryWithCapacity:( [pFontNames count] + [pFamilyNames count] ) * 2];
			if ( pEmptyDict && pFontDict )
			{
				CGFloat aSystemFontSizes[ 7 ];
				aSystemFontSizes[ 0 ] = [NSFont systemFontSize];
				aSystemFontSizes[ 1 ] = [NSFont smallSystemFontSize];
				aSystemFontSizes[ 2 ] = [NSFont labelFontSize];
				aSystemFontSizes[ 3 ] = [NSFont systemFontSizeForControlSize:NSControlSizeMini];
				aSystemFontSizes[ 4 ] = [NSFont systemFontSizeForControlSize:NSControlSizeRegular];
				aSystemFontSizes[ 5 ] = [NSFont systemFontSizeForControlSize:NSControlSizeSmall];
				aSystemFontSizes[ 6 ] = [NSFont systemFontSizeForControlSize:NSControlSizeLarge];
				for ( size_t i = 0; i < sizeof( aSystemFontSizes ) / sizeof( CGFloat ); i++ )
				{
					CGFloat fFontSize = aSystemFontSizes[ i ];

					// System fonts
					AddFontToFontsDict( [NSFont systemFontOfSize:fFontSize], pFontDict );
					AddFontToFontsDict( [NSFont boldSystemFontOfSize:fFontSize], pFontDict );
					AddFontToFontsDict( [NSFont labelFontOfSize:fFontSize], pFontDict );
					AddFontToFontsDict( [NSFont messageFontOfSize:fFontSize], pFontDict );
					AddFontToFontsDict( [NSFont menuBarFontOfSize:fFontSize], pFontDict );
					AddFontToFontsDict( [NSFont menuFontOfSize:fFontSize], pFontDict );
					AddFontToFontsDict( [NSFont controlContentFontOfSize:fFontSize], pFontDict );
					AddFontToFontsDict( [NSFont titleBarFontOfSize:fFontSize], pFontDict );
					AddFontToFontsDict( [NSFont paletteFontOfSize:fFontSize], pFontDict );
					AddFontToFontsDict( [NSFont toolTipsFontOfSize:fFontSize], pFontDict );

					// System fonts by weight
					AddFontToFontsDict( [NSFont systemFontOfSize:fFontSize weight:NSFontWeightUltraLight], pFontDict );
					AddFontToFontsDict( [NSFont systemFontOfSize:fFontSize weight:NSFontWeightThin], pFontDict );
					AddFontToFontsDict( [NSFont systemFontOfSize:fFontSize weight:NSFontWeightLight], pFontDict );
					AddFontToFontsDict( [NSFont systemFontOfSize:fFontSize weight:NSFontWeightRegular], pFontDict );
					AddFontToFontsDict( [NSFont systemFontOfSize:fFontSize weight:NSFontWeightMedium], pFontDict );
					AddFontToFontsDict( [NSFont systemFontOfSize:fFontSize weight:NSFontWeightSemibold], pFontDict );
					AddFontToFontsDict( [NSFont systemFontOfSize:fFontSize weight:NSFontWeightBold], pFontDict );
					AddFontToFontsDict( [NSFont systemFontOfSize:fFontSize weight:NSFontWeightHeavy], pFontDict );
					AddFontToFontsDict( [NSFont systemFontOfSize:fFontSize weight:NSFontWeightBlack], pFontDict );

					// Monospaced system fonts by weight
					AddFontToFontsDict( [NSFont monospacedSystemFontOfSize:fFontSize weight:NSFontWeightUltraLight], pFontDict );
					AddFontToFontsDict( [NSFont monospacedSystemFontOfSize:fFontSize weight:NSFontWeightThin], pFontDict );
					AddFontToFontsDict( [NSFont monospacedSystemFontOfSize:fFontSize weight:NSFontWeightLight], pFontDict );
					AddFontToFontsDict( [NSFont monospacedSystemFontOfSize:fFontSize weight:NSFontWeightRegular], pFontDict );
					AddFontToFontsDict( [NSFont monospacedSystemFontOfSize:fFontSize weight:NSFontWeightMedium], pFontDict );
					AddFontToFontsDict( [NSFont monospacedSystemFontOfSize:fFontSize weight:NSFontWeightSemibold], pFontDict );
					AddFontToFontsDict( [NSFont monospacedSystemFontOfSize:fFontSize weight:NSFontWeightBold], pFontDict );
					AddFontToFontsDict( [NSFont monospacedSystemFontOfSize:fFontSize weight:NSFontWeightHeavy], pFontDict );
					AddFontToFontsDict( [NSFont monospacedSystemFontOfSize:fFontSize weight:NSFontWeightBlack], pFontDict );

					// Monospaced digit system fonts by weight
					AddFontToFontsDict( [NSFont monospacedDigitSystemFontOfSize:fFontSize weight:NSFontWeightUltraLight], pFontDict );
					AddFontToFontsDict( [NSFont monospacedDigitSystemFontOfSize:fFontSize weight:NSFontWeightThin], pFontDict );
					AddFontToFontsDict( [NSFont monospacedDigitSystemFontOfSize:fFontSize weight:NSFontWeightLight], pFontDict );
					AddFontToFontsDict( [NSFont monospacedDigitSystemFontOfSize:fFontSize weight:NSFontWeightRegular], pFontDict );
					AddFontToFontsDict( [NSFont monospacedDigitSystemFontOfSize:fFontSize weight:NSFontWeightMedium], pFontDict );
					AddFontToFontsDict( [NSFont monospacedDigitSystemFontOfSize:fFontSize weight:NSFontWeightSemibold], pFontDict );
					AddFontToFontsDict( [NSFont monospacedDigitSystemFontOfSize:fFontSize weight:NSFontWeightBold], pFontDict );
					AddFontToFontsDict( [NSFont monospacedDigitSystemFontOfSize:fFontSize weight:NSFontWeightHeavy], pFontDict );

					// Preferred text style fonts
					AddFontToFontsDict( [NSFont preferredFontForTextStyle:NSFontTextStyleBody options:pEmptyDict], pFontDict );
					AddFontToFontsDict( [NSFont preferredFontForTextStyle:NSFontTextStyleCallout options:pEmptyDict], pFontDict );
					AddFontToFontsDict( [NSFont preferredFontForTextStyle:NSFontTextStyleCaption1 options:pEmptyDict], pFontDict );
					AddFontToFontsDict( [NSFont preferredFontForTextStyle:NSFontTextStyleCaption2 options:pEmptyDict], pFontDict );
					AddFontToFontsDict( [NSFont preferredFontForTextStyle:NSFontTextStyleFootnote options:pEmptyDict], pFontDict );
					AddFontToFontsDict( [NSFont preferredFontForTextStyle:NSFontTextStyleHeadline options:pEmptyDict], pFontDict );
					AddFontToFontsDict( [NSFont preferredFontForTextStyle:NSFontTextStyleSubheadline options:pEmptyDict], pFontDict );
					AddFontToFontsDict( [NSFont preferredFontForTextStyle:NSFontTextStyleLargeTitle options:pEmptyDict], pFontDict );
					AddFontToFontsDict( [NSFont preferredFontForTextStyle:NSFontTextStyleTitle1 options:pEmptyDict], pFontDict );
					AddFontToFontsDict( [NSFont preferredFontForTextStyle:NSFontTextStyleTitle2 options:pEmptyDict], pFontDict );
					AddFontToFontsDict( [NSFont preferredFontForTextStyle:NSFontTextStyleTitle3 options:pEmptyDict], pFontDict );

					// All other fonts
					for ( NSString *pFontName : pFontNames )
					{
						// Suppress CoreText messages when compiled on
						// macOS 10.15 or higher by excluding font names that
						// start with "."
						if ( pFontName && [pFontName length] && ![pFontName hasPrefix:@"."] )
						{
							NSFont *pFont = [NSFont fontWithName:pFontName size:fFontSize];
							if ( pFont )
								AddFontToFontsDict( pFont, pFontDict );
						}
					}

					// Fix LibreOffice bug 145563 by creating a separate
					// font with the family name set to the display name so
					// that when opening a document saved by OpenOffice or
					// LibreOffice, we can match the font. Also, the user
					// can select the font with family name so that
					// OpenOffice and LibreOffice can match the font when
					// opening a document saved by NeoOffice.
					for ( NSString *pFamilyName : pFamilyNames )
					{
						// Suppress CoreText messages when compiled on
						// macOS 10.15 or higher by excluding font names that
						// start with "."
						if ( pFamilyName && [pFamilyName length] && ![pFamilyName hasPrefix:@"."] )
						{
							NSFont *pFont = [NSFont fontWithName:pFamilyName size:fFontSize];
							if ( pFont )
							{
								NSString *pFontName = [pFont fontName];
								if ( pFontName && ![pFontName hasPrefix:pFamilyName] )
									AddFontToFontsDictWithAltName( pFont, pFontDict, pFamilyName, NO );
							}
						}
					}
				}

				pRet = pFontDict;
				[pRet retain];
			}
		}
	}

	[pPool release];

	return pRet;
}

sal_Bool NSFontManager_isFixedPitch( NSFont *pNSFont )
{
	sal_Bool bRet = sal_False;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager && [pFontManager traitsOfFont:(NSFont *)pNSFont] & NSFixedPitchFontMask )
			bRet = sal_True;
	}

	[pPool release];

	return bRet;
}

sal_Bool NSFontManager_isItalic( NSFont *pNSFont )
{
	sal_Bool bRet = sal_False;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager && [pFontManager traitsOfFont:(NSFont *)pNSFont] & NSItalicFontMask )
			bRet = sal_True;
	}

	[pPool release];

	return bRet;
}

FontWidth NSFontManager_widthOfFont( NSFont *pNSFont )
{
	FontWidth nRet = WIDTH_DONTKNOW;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager )
		{
			NSFontTraitMask nTraits = [pFontManager traitsOfFont:(NSFont *)pNSFont];
			if ( nTraits & NSCompressedFontMask )
				nRet = WIDTH_ULTRA_CONDENSED;
			else if ( nTraits & NSCondensedFontMask )
				nRet = WIDTH_CONDENSED;
			else if ( nTraits & NSNarrowFontMask )
				nRet = WIDTH_SEMI_CONDENSED;
			else if ( nTraits & NSExpandedFontMask )
				nRet = WIDTH_EXPANDED;
			else
				nRet = WIDTH_NORMAL;
		}
	}

	[pPool release];

	return nRet;
}

FontWeight NSFontManager_weightOfFont( NSFont *pNSFont )
{
	FontWeight nRet = WEIGHT_DONTKNOW;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager )
		{
			int nWeight = [pFontManager weightOfFont:(NSFont *)pNSFont];

			// Convert from NSFont weights to FontWeight values
			if ( nWeight <= 1 )
				nRet = WEIGHT_THIN;
			else if ( nWeight <= 6 )
				nRet = (FontWeight)nWeight;
			else if ( nWeight <= 8 )
				nRet = WEIGHT_SEMIBOLD;
			else if ( nWeight <= 9 )
				nRet = WEIGHT_BOLD;
			else if ( nWeight <= 12 )
				nRet = WEIGHT_ULTRABOLD;
			else
				nRet = WEIGHT_BLACK;
		}
	}

	[pPool release];

	return nRet;
}
