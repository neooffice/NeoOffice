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

@interface RunSpellCheckerArgs : NSObject
{
	NSArray*				mpArgs;
	NSObject*				mpResult;
}
+ (id)argsWithArgs:(NSArray *)pArgs;
- (NSArray *)args;
- (void)dealloc;
- (id)initWithArgs:(NSArray *)pArgs;
- (NSObject *)result;
- (void)setResult:(NSObject *)pResult;
@end

@implementation RunSpellCheckerArgs

+ (id)argsWithArgs:(NSArray *)pArgs
{
	RunSpellCheckerArgs *pRet = [[RunSpellCheckerArgs alloc] initWithArgs:(NSArray *)pArgs];
	[pRet autorelease];
	return pRet;
}

- (NSArray *)args
{
	return mpArgs;
}

- (void)dealloc
{
	if ( mpArgs )
		[mpArgs release];

	if ( mpResult )
		[mpResult release];

	[super dealloc];
}

- (id)initWithArgs:(NSArray *)pArgs
{
	[super init];

	mpResult = nil;
	mpArgs = pArgs;
	if ( mpArgs )
		[mpArgs retain];

	return self;
}

- (NSObject *)result
{
	return mpResult;
}

- (void)setResult:(NSObject *)pResult
{
	if ( mpResult )
		[mpResult release];

	mpResult = pResult;

	if ( mpResult )
		[mpResult retain];
}

@end

@interface RunSpellChecker : NSObject
+ (id)create;
- (void)checkSpellingOfString:(RunSpellCheckerArgs *)pArgs;
- (void)getGuesses:(RunSpellCheckerArgs *)pArgs;
- (void)getLocales:(RunSpellCheckerArgs *)pArgs;
@end

@implementation RunSpellChecker

+ (id)create
{
	RunSpellChecker *pRet = [[RunSpellChecker alloc] init];
	[pRet autorelease];
	return pRet;
}

- (void)checkSpellingOfString:(RunSpellCheckerArgs *)pArgs
{
    NSArray *pArgArray = [pArgs args];
    if ( !pArgArray || [pArgArray count] < 2 )
        return;

    NSString *pString = (NSString *)[pArgArray objectAtIndex:0];
	if ( !pString )
		return;

    NSString *pLocale = (NSString *)[pArgArray objectAtIndex:1];
	if ( !pLocale )
		return;

	@try
	{
		NSSpellChecker *pChecker = [NSSpellChecker sharedSpellChecker];
		if ( pChecker && [pChecker setLanguage:pLocale] )
		{
			NSRange aRange = [pChecker checkSpellingOfString:pString startingAt:0];
			if ( aRange.location != NSNotFound && aRange.length > 0 )
				[pArgs setResult:[NSNumber numberWithBool:NO]];
		}
	}
	@catch ( NSException *pExc )
	{
		NSLog( @"%@", [pExc reason] );
	}
}

- (void)getGuesses:(RunSpellCheckerArgs *)pArgs
{
    NSArray *pArgArray = [pArgs args];
    if ( !pArgArray || [pArgArray count] < 2 )
        return;

    NSString *pString = (NSString *)[pArgArray objectAtIndex:0];
	if ( !pString )
		return;

    NSString *pLocale = (NSString *)[pArgArray objectAtIndex:1];
	if ( !pLocale )
		return;

	@try
	{
		NSSpellChecker *pChecker = [NSSpellChecker sharedSpellChecker];
		if ( pChecker && [pChecker setLanguage:pLocale] )
		{
			NSArray *pArray = [pChecker guessesForWord:pString];
			if ( pArray && [pArray count] )
				[pArgs setResult:pArray];
		}
	}
	@catch ( NSException *pExc )
	{
		NSLog( @"%@", [pExc reason] );
	}
}

- (void)getLocales:(RunSpellCheckerArgs *)pArgs
{
    NSArray *pArgArray = [pArgs args];
    if ( !pArgArray || [pArgArray count] < 1 )
        return;

    NSMutableSet *pLocales = (NSMutableSet *)[pArgArray objectAtIndex:0];
	if ( !pLocales )
		return;

	@try
	{
		NSSpellChecker *pChecker = [NSSpellChecker sharedSpellChecker];
		if ( pChecker )
		{
			NSBundle *pBundle = [NSBundle bundleWithPath:@"/System/Library/Services/AppleSpell.service"];
			if ( pBundle )
				[pLocales addObjectsFromArray:[pBundle localizations]];
			pBundle = [NSBundle mainBundle];
			if ( pBundle )
				[pLocales addObjectsFromArray:[pBundle localizations]];

			NSMutableArray *pRet = [NSMutableArray arrayWithCapacity:[pLocales count]];
			if ( pRet )
			{
				NSArray *pLocaleArray = [pLocales allObjects];
				if ( pLocaleArray )
				{
					unsigned nCount = [pLocales count];
					unsigned i = 0;
					for ( ; i < nCount; i++ )
					{
						NSString *pLocale = (NSString *)[pLocaleArray objectAtIndex:i];
						if ( pLocale && [pChecker setLanguage:(NSString *)pLocale] )
							[pRet addObject:pLocale];
					}
				}
			}

			[pArgs setResult:pRet];
		}
	}
	@catch ( NSException *pExc )
	{
		NSLog( @"%@", [pExc reason] );
	}
}

@end

BOOL NSSpellChecker_checkSpellingOfString( CFStringRef aString, CFStringRef aLocale )
{
	BOOL bRet = YES;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( aString && aLocale )
	{
		RunSpellCheckerArgs *pArgs = [RunSpellCheckerArgs argsWithArgs:[NSArray arrayWithObjects:(NSString *)aString, (NSString *)aLocale, nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		RunSpellChecker *pRunSpellChecker = [RunSpellChecker create];
		[pRunSpellChecker performSelectorOnMainThread:@selector(checkSpellingOfString:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSNumber *pRet = (NSNumber *)[pArgs result];
		if ( pRet )
			bRet = [pRet boolValue];
	}

	[pPool release];

	return bRet;
}

CFArrayRef NSSpellChecker_getGuesses( CFStringRef aString, CFStringRef aLocale )
{
	CFMutableArrayRef aRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( aString && aLocale )
	{
		RunSpellCheckerArgs *pArgs = [RunSpellCheckerArgs argsWithArgs:[NSArray arrayWithObjects:(NSString *)aString, (NSString *)aLocale, nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		RunSpellChecker *pRunSpellChecker = [RunSpellChecker create];
		[pRunSpellChecker performSelectorOnMainThread:@selector(getGuesses:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSArray *pRet = (NSArray *)[pArgs result];
		if ( pRet )
		{
			unsigned int nCount = [pRet count];
			aRet = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
			if ( aRet )
			{
				unsigned i = 0;
				for ( ; i < nCount; i++ )
				{
					NSString *pString = [pRet objectAtIndex:i];
					if ( pString )
					{
						CFStringRef aString = CFStringCreateCopy( NULL, (CFStringRef)pString );
						if ( aString )
						{
							CFArrayAppendValue( aRet, aString );
							CFRelease( aString );
						}
					}
				}
			}
		}
	}

	[pPool release];

	return aRet;
}

CFArrayRef NSSpellChecker_getLocales( CFArrayRef aAppLocales )
{
	CFMutableArrayRef aRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	RunSpellCheckerArgs *pArgs = [RunSpellCheckerArgs argsWithArgs:[NSArray arrayWithObject:( aAppLocales ? [NSMutableSet setWithArray:(NSArray *)aAppLocales] : [NSMutableSet setWithCapacity:64] )]];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	RunSpellChecker *pRunSpellChecker = [RunSpellChecker create];
	[pRunSpellChecker performSelectorOnMainThread:@selector(getLocales:) withObject:pArgs waitUntilDone:YES modes:pModes];
	NSArray *pRet = (NSArray *)[pArgs result];
	if ( pRet )
	{
		unsigned int nCount = [pRet count];
		aRet = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
		if ( aRet )
		{
			unsigned i = 0;
			for ( ; i < nCount; i++ )
			{
				NSString *pString = [pRet objectAtIndex:i];
				if ( pString )
				{
					CFStringRef aString = CFStringCreateCopy( NULL, (CFStringRef)pString );
					if ( aString )
					{
						CFArrayAppendValue( aRet, aString );
						CFRelease( aString );
					}
				}
			}
		}
	}

	[pPool release];

	return aRet;
}
