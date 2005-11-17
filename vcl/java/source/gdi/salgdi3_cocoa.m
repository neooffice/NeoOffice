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
 *  Copyright 2005 by Patrick Luby (patrick.luby@planamesa.com)
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
#import "salgdi3_cocoa.h"

// Note: many of the exported functions in this file do not use the
// performOnMainThread selector but only because the code that calls these
// functions is run in a native timer from the native event dispatch thread.

@interface FindFontNameWithStyle : NSObject
{
    BOOL				mbBold;
    CFStringRef			maFontName;
    CFStringRef			maFoundFontName;
    BOOL				mbItalic;
	float				mfSize;
}
- (void)findFontNameWithStyle:(id)pObject;
- (CFStringRef)foundFontName;
- (id)initWithFontName:(CFStringRef)aFontName bold:(BOOL)bBold italic:(BOOL)bItalic size:(float)fSize;
@end

@implementation FindFontNameWithStyle
- (void)findFontNameWithStyle:(id)pObject
{
	NSFontManager *pFontManager = [NSFontManager sharedFontManager];
	if ( pFontManager )
	{
		NSFont *pNSFont = [NSFont fontWithName:(NSString *)maFontName size:mfSize];
		if ( pNSFont )
		{
			int nWeight = [pFontManager weightOfFont:pNSFont];
			NSFontTraitMask nTraits = ( [pFontManager traitsOfFont:pNSFont] & ( NSBoldFontMask | NSItalicFontMask ) );
			if ( mbBold )
			{
				nTraits |= NSBoldFontMask;

				// Fix bug 1128 by ensuring that the weight is at least 9
				if ( nWeight < 9 )
					nWeight = 9;
			}
			if ( mbItalic )
				nTraits |= NSItalicFontMask;
			NSFont *pNewNSFont = [pFontManager fontWithFamily:[pNSFont familyName] traits:nTraits weight:nWeight size:mfSize];
			if ( pNewNSFont && pNewNSFont != pNSFont )
			{
				ATSFontRef aFont = NSFont_getATSFontRef( pNewNSFont );
				if ( aFont )
					ATSFontGetName( aFont, kATSOptionFlagsDefault, &maFoundFontName );
			}
		}
	}
}

- (CFStringRef)foundFontName
{
	return maFoundFontName;
}

- (id)initWithFontName:(CFStringRef)aFontName bold:(BOOL)bBold italic:(BOOL)bItalic size:(float)fSize
{
	[super init];

	mbBold = bBold;
	maFontName = aFontName;
	maFoundFontName = nil;
	mbItalic = bItalic;
	mfSize = fSize;

	return self;
}
@end

id NSFont_create( CFStringRef aFontName, long nSize )
{
	NSFont *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( aFontName && nSize )
	{
		pRet = [NSFont fontWithName:(NSString *)aFontName size:(float)nSize];
		if ( pRet )
			pRet = [pRet retain];
	}

	[pPool release];

	return pRet;
}

ATSFontRef NSFont_getATSFontRef( id pNSFont )
{
	ATSFontRef aRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		CFStringRef aPSName = (CFStringRef)[(NSFont *)pNSFont fontName];
		if ( [pNSFont respondsToSelector:@selector(_atsFontID)] )
		{
			aRet = (ATSFontRef)[pNSFont _atsFontID];
			if ( aRet )
			{
				CFStringRef aString;
				if ( ATSFontGetPostScriptName( aRet, kATSOptionFlagsDefault, &aString ) == noErr )
				{
					// In some cases, _atsFontID may return a different font
					// so ignore if names don't match
					if ( CFStringCompare( aPSName, aString, 0 ) )
						aRet = nil;
					CFRelease( aString );
				}
			}
		}

		if ( !aRet )
			aRet = ATSFontFindFromPostScriptName( aPSName, kATSOptionFlagsDefault );
	}

	[pPool release];

	return aRet;
}

void NSFont_release( id pNSFont )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
		[(NSFont *)pNSFont release];

	[pPool release];
}

CFStringRef NSFontManager_findFontNameWithStyle( CFStringRef aFontName, BOOL bBold, BOOL bItalic, long nSize )
{
	CFStringRef aRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( aFontName && nSize )
	{
		FindFontNameWithStyle *pFindFontNameWithStyle = [[FindFontNameWithStyle alloc] initWithFontName:aFontName bold:bBold italic:bItalic size:(float)nSize];
		[pFindFontNameWithStyle performSelectorOnMainThread:@selector(findFontNameWithStyle:) withObject:pFindFontNameWithStyle waitUntilDone:YES];
		aRet = [pFindFontNameWithStyle foundFontName];
	}

	[pPool release];

	return aRet;
}

BOOL NSFontManager_isFixedPitch( id pNSFont )
{
	BOOL bRet = FALSE;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
		bRet = [pNSFont isFixedPitch];

	[pPool release];

	return bRet;
}

BOOL NSFontManager_isItalic( id pNSFont )
{
	BOOL bRet = FALSE;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager )
			bRet = ( [pFontManager traitsOfFont:(NSFont *)pNSFont] & NSItalicFontMask );
	}

	[pPool release];

	return bRet;
}

int NSFontManager_widthOfFont( id pNSFont )
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
int NSFontManager_weightOfFont( id pNSFont )
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
