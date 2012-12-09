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
 *  Patrick Luby, September 2005
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

#include <dlfcn.h>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>
#undef check

#include <vcl/svdata.hxx>

#include "salinst_cocoa.h"

#ifndef NSURLBookmarkCreationWithSecurityScope
#define NSURLBookmarkCreationWithSecurityScope ( 1UL << 11 )
#endif	// !NSURLBookmarkCreationWithSecurityScope

#ifndef NSURLBookmarkResolutionWithSecurityScope
#define NSURLBookmarkResolutionWithSecurityScope ( 1UL << 10 )
#endif	// !NSURLBookmarkResolutionWithSecurityScope

typedef NSString* const NSURLIsReadableKey_Type;
typedef NSString* const NSURLIsWritableKey_Type;

static NSURLIsReadableKey_Type *pNSURLIsReadableKey = NULL;
static NSURLIsWritableKey_Type *pNSURLIsWritableKey = NULL;

static NSURL *AcquireSecurityScopedURL( const NSURL *pURL, MacOSBOOL bMustShowDialogIfNoBookmark, MacOSBOOL bResolveAliasURLs, const NSString *pTitle );
static void ReleaseSecurityScopedURL( NSURL *pURL );

@interface NSURL (VCLURL)
- (MacOSBOOL)startAccessingSecurityScopedResource;
- (void)stopAccessingSecurityScopedResource;
@end

@interface VCLRequestSecurityScopedURL : NSObject
{
	NSOpenPanel*			mpOpenPanel;
	NSURL*					mpSecurityScopedURL;
	NSString*				mpTitle;
	NSURL*					mpURL;
}
+ (id)createWithURL:(NSURL *)pURL title:(NSString *)pTitle;
- (id)initWithURL:(NSURL *)pURL title:(NSString *)pTitle;
- (MacOSBOOL)panel:(id)pSender shouldEnableURL:(NSURL *)pURL;
- (void)panel:(id)pSender didChangeToDirectoryURL:(NSURL *)pURL;
- (void)requestSecurityScopedURL:(id)pObject;
- (NSURL *)securityScopedURL;
@end

static NSURL *ResolveAliasURL( const NSURL *pURL, MacOSBOOL bMustShowDialogIfNoBookmark, const NSString *pTitle )
{
	NSURL *pRet = nil;

	if ( pURL )
	{
		NSURL *pSecurityScopedURL = AcquireSecurityScopedURL( pURL, bMustShowDialogIfNoBookmark, NO, pTitle );
		NSData *pData = [NSURL bookmarkDataWithContentsOfURL:pURL error:nil];
		if ( pData )
		{
			MacOSBOOL bStale = NO;
			pURL = [NSURL URLByResolvingBookmarkData:pData options:NSURLBookmarkResolutionWithoutUI | NSURLBookmarkResolutionWithoutMounting relativeToURL:nil bookmarkDataIsStale:&bStale error:nil];
			if ( !bStale && pURL )
			{
				pURL = [pURL URLByStandardizingPath];
				if ( pURL )
				{
					pURL = [pURL URLByResolvingSymlinksInPath];
					if ( pURL )
					{
						// Recurse to check if the URL is also an alias
						NSURL *pRecursedURL = ResolveAliasURL( pURL, bMustShowDialogIfNoBookmark, pTitle );
						if ( pRecursedURL )
							pRet = pRecursedURL;
						else
							pRet = pURL;
					}
				}
			}
		}

		if ( pSecurityScopedURL )
			ReleaseSecurityScopedURL( pSecurityScopedURL );
	}

	return pRet;
}

static NSURL *AcquireSecurityScopedURL( const NSURL *pURL, MacOSBOOL bMustShowDialogIfNoBookmark, MacOSBOOL bResolveAliasURLs, const NSString *pTitle )
{
	NSURL *pRet = nil;

	if ( pURL && [pURL isFileURL] )
	{
		// Iterate through path and resolve any aliases
		NSArray *pPathComponents = [pURL pathComponents];
		if ( pPathComponents && [pPathComponents count] )
		{
			pURL = [NSURL fileURLWithPath:[pPathComponents objectAtIndex:0]];
			if ( pURL )
			{
				NSUInteger nCount = [pPathComponents count];
				NSUInteger i = 1;
				for ( ; i < nCount ; i++ )
				{
					pURL = [pURL URLByAppendingPathComponent:[pPathComponents objectAtIndex:i]];
					if ( pURL )
						pURL = [pURL URLByResolvingSymlinksInPath];

					// If the path does not exist, use partial URL
					if ( !pURL || ![pURL checkResourceIsReachableAndReturnError:nil] )
						break;

					if ( bResolveAliasURLs )
					{
						NSNumber *pAlias = nil;
						if ( [pURL getResourceValue:&pAlias forKey:NSURLIsAliasFileKey error:nil] && pAlias && [pAlias boolValue] )
						{
							NSURL *pResolvedURL = ResolveAliasURL( pURL, bMustShowDialogIfNoBookmark, pTitle );
							if ( pResolvedURL )
								pURL = pResolvedURL;
						}
					}
				}

				if ( pURL )
				{
					// Check if there are any cached security scoped bookmarks
					// for this URL or any of its parent folders
					MacOSBOOL bShowOpenPanel = YES;
					NSUserDefaults *pUserDefaults = [NSUserDefaults standardUserDefaults];
					NSURL *pTmpURL = pURL;
					while ( pUserDefaults && pTmpURL && !pRet )
					{
						NSString *pKey = [pTmpURL absoluteString];
						if ( pKey )
						{
							NSObject *pBookmarkData = [pUserDefaults objectForKey:pKey];
							if ( pBookmarkData && [pBookmarkData isKindOfClass:[NSData class]] )
							{
								MacOSBOOL bStale = NO;
								NSURL *pSecurityScopedURL = [NSURL URLByResolvingBookmarkData:(NSData *)pBookmarkData options:NSURLBookmarkResolutionWithSecurityScope relativeToURL:nil bookmarkDataIsStale:&bStale error:nil];
								if ( !bStale && pSecurityScopedURL && [pSecurityScopedURL respondsToSelector:@selector(startAccessingSecurityScopedResource)] )
								{
									if ( [pSecurityScopedURL startAccessingSecurityScopedResource] )
										pRet = pSecurityScopedURL;

									bShowOpenPanel = NO;
									break;
								}
							}
						}

						NSURL *pOldTmpURL = pTmpURL;
						pTmpURL = [pTmpURL URLByDeletingLastPathComponent];
						if ( pTmpURL )
						{
							pTmpURL = [pTmpURL URLByStandardizingPath];
							if ( pTmpURL )
							{
								pTmpURL = [pTmpURL URLByResolvingSymlinksInPath];
								if ( pTmpURL && [pTmpURL isEqual:pOldTmpURL] )
									pTmpURL = nil;
							}
						}
					}

					if ( bShowOpenPanel && !bMustShowDialogIfNoBookmark )
					{
						if ( !pNSURLIsReadableKey )
							pNSURLIsReadableKey = (NSURLIsReadableKey_Type *)dlsym( RTLD_DEFAULT, "NSURLIsReadableKey" );
						if ( !pNSURLIsWritableKey )
							pNSURLIsWritableKey = (NSURLIsWritableKey_Type *)dlsym( RTLD_DEFAULT, "NSURLIsWritableKey" );
						if ( pNSURLIsReadableKey && pNSURLIsWritableKey && *pNSURLIsReadableKey && *pNSURLIsWritableKey )
						{
							NSNumber *pReadable = nil;
							NSNumber *pWritable = nil;
							if ( ( [pURL getResourceValue:&pReadable forKey:*pNSURLIsReadableKey error:nil] && pReadable && [pReadable boolValue] ) || ( [pURL getResourceValue:&pWritable forKey:*pNSURLIsWritableKey error:nil] && pWritable && [pWritable boolValue] ) )
								bShowOpenPanel = NO;
						}
					}

					if ( bShowOpenPanel && !pRet )
					{
						VCLRequestSecurityScopedURL *pVCLRequestSecurityScopedURL = [VCLRequestSecurityScopedURL createWithURL:pURL title:pTitle];
						NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
						[pVCLRequestSecurityScopedURL performSelectorOnMainThread:@selector(requestSecurityScopedURL:) withObject:pVCLRequestSecurityScopedURL waitUntilDone:YES modes:pModes];
						NSURL *pSecurityScopedURL = [pVCLRequestSecurityScopedURL securityScopedURL];
						if ( pSecurityScopedURL && [pSecurityScopedURL respondsToSelector:@selector(startAccessingSecurityScopedResource)] && [pSecurityScopedURL startAccessingSecurityScopedResource] )
							pRet = pSecurityScopedURL;
					}
				}
			}
		}
	}

	return pRet;
}

static void ReleaseSecurityScopedURL( NSURL *pURL )
{
	if ( pURL )
	{
		if ( [pURL respondsToSelector:@selector(stopAccessingSecurityScopedResource)] )
			[pURL stopAccessingSecurityScopedResource];
		[pURL release];
	}
}

@implementation VCLRequestSecurityScopedURL

+ (id)createWithURL:(NSURL *)pURL title:(NSString *)pTitle
{
	VCLRequestSecurityScopedURL *pRet = [[VCLRequestSecurityScopedURL alloc] initWithURL:pURL title:pTitle];
	[pRet autorelease];
	return pRet;
}

- (void)dealloc
{
	if ( mpSecurityScopedURL )
		[mpSecurityScopedURL release];

	if ( mpTitle )
		[mpTitle release];

	if ( mpURL )
		[mpURL release];

	[super dealloc];
}

- (id)initWithURL:(NSURL *)pURL title:(NSString *)pTitle
{
	[super init];

	mpOpenPanel = nil;
	mpSecurityScopedURL = nil;

	mpTitle = pTitle;
	if ( mpTitle )
		[mpTitle retain];

	mpURL = nil;
	if ( pURL && [pURL isFileURL] )
	{
		pURL = [pURL URLByStandardizingPath];
		if ( pURL )
		{
			pURL = [pURL URLByResolvingSymlinksInPath];
			if ( pURL )
			{
				mpURL = pURL;
				[mpURL retain];
			}
		}
	}

	return self;
}

- (MacOSBOOL)panel:(id)pSender shouldEnableURL:(NSURL *)pURL
{
	if ( pURL )
	{
		pURL = [pURL URLByStandardizingPath];
		if ( pURL )
			pURL = [pURL URLByResolvingSymlinksInPath];
	}

	return ( pURL && mpURL && [pURL isFileURL] && [pURL isEqual:mpURL] );
}

- (void)panel:(id)pSender didChangeToDirectoryURL:(NSURL *)pURL
{
	if ( mpURL && mpOpenPanel && ( !pURL || ![pURL isEqual:mpURL] ) )
		[mpOpenPanel setDirectoryURL:mpURL];
}

- (void)requestSecurityScopedURL:(id)pObject
{
	if ( mpOpenPanel || mpSecurityScopedURL || !mpURL )
		return;

	// Check if URL is a directory otherwise use parent directory
	NSURL *pURL = mpURL;
	NSNumber *pDir = nil;
	if ( ![pURL getResourceValue:&pDir forKey:NSURLIsDirectoryKey error:nil] || !pDir || ![pDir boolValue] )
	{
		pURL = [pURL URLByDeletingLastPathComponent];
		if ( pURL )
		{
			pURL = [pURL URLByStandardizingPath];
			if ( pURL )
			{
				pURL = [pURL URLByResolvingSymlinksInPath];
				if ( pURL )
				{
					pDir = nil;
					if ( ![pURL getResourceValue:&pDir forKey:NSURLIsDirectoryKey error:nil] || !pDir || ![pDir boolValue] )
						pURL = nil;
				}
			}
		}
	}

	if ( mpURL != pURL )
	{
		[mpURL release];
		mpURL = pURL;
		if ( mpURL )
			[mpURL retain];
	}

	if ( mpURL )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		mpOpenPanel = [NSOpenPanel openPanel];
		if ( mpOpenPanel )
		{
			[mpOpenPanel setAllowsMultipleSelection:NO];
			[mpOpenPanel setDirectoryURL:mpURL];
			[mpOpenPanel setCanChooseDirectories:YES];
			[mpOpenPanel setCanChooseFiles:NO];
			[mpOpenPanel setDelegate:self];
			if ( mpTitle && [mpTitle length] )
			{
				[mpOpenPanel setPrompt:mpTitle];
				[mpOpenPanel setTitle:mpTitle];
			}

			if ( [mpOpenPanel runModal] == NSFileHandlingPanelOKButton )
			{
				NSArray *pURLs = [mpOpenPanel URLs];
				if ( pURLs && [pURLs count] )
				{
					// There should only be one selected URL
					NSURL *pDirURL = [pURLs objectAtIndex:0];
					if ( pDirURL && [pDirURL isFileURL] )
					{
						Application_cacheSecurityScopedURL( pDirURL );

						pDirURL = [pDirURL URLByStandardizingPath];
						if ( pDirURL )
						{
							pDirURL = [pDirURL URLByResolvingSymlinksInPath];
							if ( pDirURL )
							{
								NSData *pData = [pDirURL bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope includingResourceValuesForKeys:nil relativeToURL:nil error:nil];
								if ( pData )
								{
									MacOSBOOL bStale = NO;
									NSURL *pResolvedURL = [NSURL URLByResolvingBookmarkData:pData options:NSURLBookmarkResolutionWithSecurityScope relativeToURL:nil bookmarkDataIsStale:&bStale error:nil];
									if ( pResolvedURL && !bStale && [pResolvedURL isFileURL] )
									{
										pResolvedURL = [pResolvedURL URLByStandardizingPath];
										if ( pResolvedURL )
										{
											pResolvedURL = [pResolvedURL URLByResolvingSymlinksInPath];
											if ( pResolvedURL )
											{
												mpSecurityScopedURL = pResolvedURL;
												[mpSecurityScopedURL retain];
											}
										}
									}
								}
							}
						}
					}
				}
			}

			[mpOpenPanel setDelegate:nil];
			mpOpenPanel = nil;
		}

		[pPool release];
	}
}

- (NSURL *)securityScopedURL
{
	return mpSecurityScopedURL;
}

@end

@interface GetModalWindow : NSObject
{
	NSWindow*			mpModalWindow;
}
+ (id)create;
- (void)getModalWindow:(id)pObject;
- (id)init;
- (NSWindow *)modalWindow;
@end

@implementation GetModalWindow

+ (id)create
{
	GetModalWindow *pRet = [[GetModalWindow alloc] init];
	[pRet autorelease];
	return pRet;
}

- (void)getModalWindow:(id)pObject
{
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		mpModalWindow = [pApp modalWindow];
		if ( !mpModalWindow )
		{
			NSArray *pWindows = [pApp windows];
			if ( pWindows )
			{
				unsigned int nCount = [pWindows count];
				unsigned int i = 0;
				for ( ; i < nCount ; i++ )
				{
					NSWindow *pWindow = [pWindows objectAtIndex:i];
					if ( [pWindow isSheet] && [pWindow isVisible] )
					{
						mpModalWindow = pWindow;
						break;
					}
				}
			}
		}
	}
}

- (id)init
{
	[super init];
 
	mpModalWindow = nil;
 
	return self;
}

- (NSWindow *)modalWindow
{
	return mpModalWindow;
}

@end

void NSApplication_dispatchPendingEvents()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSEvent *pEvent;
		while ( ( pEvent = [pApp nextEventMatchingMask:NSAnyEventMask untilDate:nil inMode:( [pApp modalWindow] ? NSModalPanelRunLoopMode : NSDefaultRunLoopMode ) dequeue:YES] ) != nil )
			[pApp sendEvent:pEvent];
	}

	[pPool release];
}

id NSApplication_getModalWindow()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSWindow *pModalWindow = nil;

	GetModalWindow *pGetModalWindow = [GetModalWindow create];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pGetModalWindow performSelectorOnMainThread:@selector(getModalWindow:) withObject:pGetModalWindow waitUntilDone:YES modes:pModes];
	pModalWindow = [pGetModalWindow modalWindow];

	[pPool release];

	return pModalWindow;
}

id Application_acquireSecurityScopedURL( const char *pPath, unsigned char bMustShowDialogIfNoBookmark, const char *pDialogTitle )
{
	id pRet = nil;

	if ( ImplGetSVData() && ImplGetSVData()->mpDefInst && pPath && strlen( pPath ) )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSString *pString = [NSString stringWithUTF8String:pPath];
		if ( pString )
		{
			NSURL *pURL = [NSURL fileURLWithPath:pString];
			if ( pURL )
			{
				pURL = [pURL URLByStandardizingPath];
				if ( pURL )
				{
					pURL = [pURL URLByResolvingSymlinksInPath];
					if ( pURL )
					{
						pRet = AcquireSecurityScopedURL( pURL, (MacOSBOOL)bMustShowDialogIfNoBookmark, YES, pDialogTitle ? [NSString stringWithUTF8String:pDialogTitle] : nil );
						if ( pRet )
							[pRet retain];
					}
				}
			}
		}

		[pPool release];
	}

	return pRet;
}

void Application_cacheSecurityScopedURL( id pNonSecurityScopedURL )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSURL *pURL = (NSURL *)pNonSecurityScopedURL;
	if ( pURL && [pURL isKindOfClass:[NSURL class]] && [pURL isFileURL] )
	{
		pURL = [pURL URLByStandardizingPath];
		if ( pURL )
		{
			pURL = [pURL URLByResolvingSymlinksInPath];
			if ( pURL )
			{
				NSData *pData = [pURL bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope includingResourceValuesForKeys:nil relativeToURL:nil error:nil];
				if ( pData )
				{
					MacOSBOOL bStale = NO;
					NSURL *pResolvedURL = [NSURL URLByResolvingBookmarkData:pData options:NSURLBookmarkResolutionWithSecurityScope relativeToURL:nil bookmarkDataIsStale:&bStale error:nil];
					if ( pResolvedURL && !bStale && [pResolvedURL isFileURL] )
					{
						pResolvedURL = [pResolvedURL URLByStandardizingPath];
						if ( pResolvedURL )
						{
							pResolvedURL = [pResolvedURL URLByResolvingSymlinksInPath];
							if ( pResolvedURL )
							{
								NSUserDefaults *pUserDefaults = [NSUserDefaults standardUserDefaults];
								NSString *pKey = [pResolvedURL absoluteString];
								if ( pUserDefaults && pKey )
									[pUserDefaults setObject:pData forKey:pKey];
							}
						}
					}
				}
			}
		}
	}

	[pPool release];
}

void Application_releaseSecurityScopedURL( id pURL )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pURL && [pURL isKindOfClass:[NSURL class]] )
		ReleaseSecurityScopedURL( (NSURL *)pURL );

	[pPool release];
}
