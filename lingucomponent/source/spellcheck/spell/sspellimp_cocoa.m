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
 *		 - GNU General Public License Version 2.1
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2007 Planamesa Inc.
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
#import "sspellimp_cocoa.h"

BOOL NSSpellChecker_checkSpellingOfString( CFStringRef aString, CFStringRef aLocale )
{
	BOOL bRet = YES;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSSpellChecker *pChecker = [NSSpellChecker sharedSpellChecker];

	if ( aString && aLocale && pChecker && [pChecker setLanguage:(NSString *)aLocale] )
	{
		NSRange aRange = [pChecker checkSpellingOfString:(NSString *)aString startingAt:0];
		if ( aRange.location != NSNotFound && aRange.length > 0 )
			bRet = NO;
	}

	[pPool release];

	return bRet;
}

CFMutableArrayRef NSSpellChecker_getGuesses( CFStringRef aString, CFStringRef aLocale )
{
	CFMutableArrayRef aRet = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSSpellChecker *pChecker = [NSSpellChecker sharedSpellChecker];

	if ( aRet && aString && aLocale && pChecker && [pChecker setLanguage:(NSString *)aLocale] )
	{
		NSArray *pArray = [pChecker guessesForWord:(NSString *)aString];
		if ( pArray )
		{
			unsigned nCount = [pArray count];
			unsigned i = 0;
			for ( ; i < nCount; i++ )
			{
				NSString *pGuess = (NSString *)[pArray objectAtIndex:i];
				if ( pGuess )
					CFArrayAppendValue( aRet, (CFStringRef)pGuess );
			}
		}
	}

	[pPool release];

	return aRet;
}

CFMutableArrayRef NSSpellChecker_getLocales()
{
	CFMutableArrayRef aRet = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSSpellChecker *pChecker = [NSSpellChecker sharedSpellChecker];
	NSMutableSet *pLocales = [NSMutableSet setWithCapacity:64];

	if ( aRet && pChecker && pLocales )
	{
		NSBundle *pBundle = [NSBundle bundleWithPath:@"/System/Library/Services/AppleSpell.service"];
		if ( pBundle )
			[pLocales addObjectsFromArray:[pBundle localizations]];
		pBundle = [NSBundle mainBundle];
		if ( pBundle )
			[pLocales addObjectsFromArray:[pBundle localizations]];

		NSArray *pLocaleArray = [pLocales allObjects];
		if ( pLocaleArray )
		{
			unsigned nCount = [pLocales count];
			unsigned i = 0;
			for ( ; i < nCount; i++ )
			{
				NSString *pLocale = (NSString *)[pLocaleArray objectAtIndex:i];
				if ( pLocale && [pChecker setLanguage:(NSString *)pLocale] )
				{
					CFStringRef aLocale = CFLocaleCreateCanonicalLocaleIdentifierFromString( kCFAllocatorDefault, (CFStringRef)pLocale );
					if ( aLocale )
					{
						CFArrayAppendValue( aRet, aLocale );
						CFRelease( aLocale );
					}
				}
			}
		}
	}

	[pPool release];

	return aRet;
}
