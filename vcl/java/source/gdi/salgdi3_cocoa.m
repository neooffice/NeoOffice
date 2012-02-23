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

#import <Cocoa/Cocoa.h>

#include "salgdi3_cocoa.h"
#include "../java/VCLEventQueue_cocoa.h"

#ifndef USE_CORETEXT_TEXT_RENDERING

@interface NSFont (ATSFontRef)
- (ATSFontRef)_atsFontID;
@end

ATSFontRef NSFont_getATSFontRef( NSFont *pNSFont )
{
	ATSFontRef aRet = (ATSFontRef)nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		if ( [pNSFont respondsToSelector:@selector(_atsFontID)] )
		{
			aRet = [pNSFont _atsFontID];
			if ( aRet )
			{
				FSRef aFile;
				if ( ATSFontGetFileReference( aRet, &aFile ) != noErr )
					aRet = (ATSFontRef)nil;
			}
		}
	}

	[pPool release];

	return aRet;
}

CFStringRef NSFont_familyName( NSFont *pNSFont )
{
	CFStringRef aRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSString *pFamilyName = [pNSFont familyName];
		if ( pFamilyName )
			aRet = CFStringCreateCopy( NULL, (CFStringRef)pFamilyName );
	}

	[pPool release];

	return aRet;
}

#endif	// !USE_CORETEXT_TEXT_RENDERING

NSFont *NSFont_findFontWithStyle( NSFont *pNSFont, BOOL bBold, BOOL bItalic )
{
	NSFont *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager )
		{
			int nWeight = [pFontManager weightOfFont:pNSFont];
			NSFontTraitMask nTraits = [pFontManager traitsOfFont:pNSFont] & ( NSItalicFontMask | NSBoldFontMask | NSExpandedFontMask | NSCondensedFontMask | NSCompressedFontMask );
			if ( bBold || bItalic )
			{
				if ( bBold )
				{
					nTraits |= NSBoldFontMask;
	
					// Fix bug 1128 by ensuring that the weight is at least 9
					if ( nWeight < 9 )
						nWeight = 9;
				}
				if ( bItalic )
					nTraits |= NSItalicFontMask;

				NSFontManager_acquire();
				NSFont *pNewNSFont = [pFontManager fontWithFamily:[pNSFont familyName] traits:nTraits weight:nWeight size:[pNSFont pointSize]];
				NSFontManager_release();

				if ( pNewNSFont && pNewNSFont != pNSFont )
				{
					[pNewNSFont retain];
					pRet = pNewNSFont;
				}
			}
		}
	}

	[pPool release];

	return pRet;
}

NSArray *NSFontManager_getAllFonts()
{
	NSArray *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSFontManager *pFontManager = [NSFontManager sharedFontManager];
	if ( pFontManager )
	{
		NSArray *pFontNames = [pFontManager availableFonts];
		if ( pFontNames )
		{
			unsigned nCount = [pFontNames count];
			if ( nCount )
			{
				NSMutableArray *pFontArray = [NSMutableArray arrayWithCapacity:nCount];
				if ( pFontArray)
				{
					unsigned i = 0;
					for ( ; i < nCount; i++ )
					{
						NSFont *pCurrentFont = [NSFont fontWithName:(NSString *)[pFontNames objectAtIndex:i] size:(float)12];
						if ( pCurrentFont )
						{
							// Fix bug 3097 by using the printer font when the
							// font is a bitmap font
							pCurrentFont = [pCurrentFont printerFont];
							if ( pCurrentFont )
								[pFontArray addObject:pCurrentFont];
						}
					}

					[pFontArray retain];
					pRet = pFontArray;
				}
			}
		}
	}

	[pPool release];

	return pRet;
}

BOOL NSFontManager_isFixedPitch( NSFont *pNSFont )
{
	BOOL bRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager && [pFontManager traitsOfFont:(NSFont *)pNSFont] & NSFixedPitchFontMask )
			bRet = YES;
	}

	[pPool release];

	return bRet;
}

BOOL NSFontManager_isItalic( NSFont *pNSFont )
{
	BOOL bRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager && [pFontManager traitsOfFont:(NSFont *)pNSFont] & NSItalicFontMask )
			bRet = YES;
	}

	[pPool release];

	return bRet;
}

int NSFontManager_widthOfFont( NSFont *pNSFont )
{
	int nRet = 0;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager )
		{
			NSFontTraitMask nTraits = [pFontManager traitsOfFont:(NSFont *)pNSFont];
			if ( nTraits & NSCompressedFontMask )
				nRet = 1;
			else if ( nTraits & NSCondensedFontMask )
				nRet = 3;
			else if ( nTraits & NSNarrowFontMask )
				nRet = 4;
			else if ( nTraits & NSExpandedFontMask )
				nRet = 7;
			else
				nRet = 5;
		}
	}

	[pPool release];

	return nRet;
}

int NSFontManager_weightOfFont( NSFont *pNSFont )
{
	int nRet = 0;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager )
		{
			int nWeight = [pFontManager weightOfFont:(NSFont *)pNSFont];

			// Convert from NSFont weights to FontWeight values
			if ( nWeight <= 1 )
				nRet = 1;
			else if ( nWeight <= 6 )
				nRet = nWeight;
			else if ( nWeight <= 8 )
				nRet = 7;
			else if ( nWeight <= 9 )
				nRet = 8;
			else if ( nWeight <= 12 )
				nRet = 9;
			else
				nRet = 10;
		}
	}

	[pPool release];

	return nRet;
}
