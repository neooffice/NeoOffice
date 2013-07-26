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

#include <saldata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vos/mutex.hxx>

#include "salinst_cocoa.h"

#ifndef NSURLBookmarkCreationWithSecurityScope
#define NSURLBookmarkCreationWithSecurityScope ( 1UL << 11 )
#endif	// !NSURLBookmarkCreationWithSecurityScope

#ifndef NSURLBookmarkResolutionWithSecurityScope
#define NSURLBookmarkResolutionWithSecurityScope ( 1UL << 10 )
#endif	// !NSURLBookmarkResolutionWithSecurityScope

// Uncomment the following line to implement the panel:shouldEnableURL:
// delegate selector. Note: implementing that selector will cause hanging in
// Open dialogs after a Save panel has been displayed on Mac OS X 10.9
// #define USE_SHOULDENABLEURL_DELEGATE_SELECTOR

typedef NSString* const NSURLIsReadableKey_Type;
typedef NSString* const NSURLIsWritableKey_Type;

static ::osl::Mutex aCurrentInstanceSecurityURLCacheMutex;
static NSMutableDictionary *pCurrentInstanceSecurityURLCacheDictionary = nil;
static NSURLIsReadableKey_Type *pNSURLIsReadableKey = NULL;
static NSURLIsWritableKey_Type *pNSURLIsWritableKey = NULL;

using namespace osl;
using namespace vos;

static void AcquireSecurityScopedURL( const NSURL *pURL, MacOSBOOL bMustShowDialogIfNoBookmark, MacOSBOOL bResolveAliasURLs, const NSString *pTitle, NSMutableArray *pSecurityScopedURLs );

@interface NSURL (VCLURL)
- (MacOSBOOL)startAccessingSecurityScopedResource;
- (void)stopAccessingSecurityScopedResource;
@end

@interface VCLRequestSecurityScopedURL : NSObject
{
    NSWindow*				mpAttachedSheet;
	MacOSBOOL				mbCancelled;
	MacOSBOOL				mbFinished;
	NSOpenPanel*			mpOpenPanel;
	NSURL*					mpSecurityScopedURL;
	NSString*				mpTitle;
	NSURL*					mpURL;
	NSWindow*				mpWindow;
	MacOSBOOL				mbWindowOwner;
}
+ (id)createWithURL:(NSURL *)pURL title:(NSString *)pTitle window:(NSWindow *)pWindow;
- (void)cancel:(id)pObject;
- (void)checkForErrors:(id)pObject;
- (void)dealloc;
- (void)destroy:(id)pObject;
- (MacOSBOOL)finished;
- (id)initWithURL:(NSURL *)pURL title:(NSString *)pTitle window:(NSWindow *)pWindow;
#ifdef USE_SHOULDENABLEURL_DELEGATE_SELECTOR
- (MacOSBOOL)panel:(id)pSender shouldEnableURL:(NSURL *)pURL;
#else	// USE_SHOULDENABLEURL_DELEGATE_SELECTOR
- (MacOSBOOL)panel:(id)pSender validateURL:(NSURL *)pURL error:(NSError *)ppError;
#endif	// USE_SHOULDENABLEURL_DELEGATE_SELECTOR
- (void)panel:(id)pSender didChangeToDirectoryURL:(NSURL *)pURL;
- (void)panel:(id)pObject willExpand:(MacOSBOOL)bExpanding;
- (void)requestSecurityScopedURL:(id)pObject;
- (NSURL *)securityScopedURL;
@end

static MacOSBOOL IsURLReadableOrWritable( NSURL *pURL )
{
	MacOSBOOL bRet = NO;

	if ( pURL )
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
				bRet = YES;
		}
	}

	return bRet;
}

static MacOSBOOL IsCurrentInstanceCacheSecurityURL( NSURL *pURL, MacOSBOOL bExists )
{
	MacOSBOOL bRet = NO;

	if ( pURL )
	{
		NSString *pKey = [pURL absoluteString];
		if ( pKey && [pKey length] )
		{
			MutexGuard aGuard( aCurrentInstanceSecurityURLCacheMutex );

			if ( pCurrentInstanceSecurityURLCacheDictionary )
			{
				NSNumber *pValue = [pCurrentInstanceSecurityURLCacheDictionary objectForKey:pKey];
				if ( pValue && [pValue isKindOfClass:[NSNumber class]] )
				{
					if ( bExists )
					{
						bRet = IsURLReadableOrWritable( pURL );

						// Recache to force check if the file now exists
						Application_cacheSecurityScopedURL( pURL );
					}
					else
					{
						bRet = ![pValue boolValue];
					}
				}
			}
		}
	}

	return bRet;
}

// Improve performance by checking if the URL is within the home, main bundle,
// temporary, or one of the safe system folders. These folders do not need a
// security scoped URL and the majority of calls to this function are for paths
// in these folders.
static MacOSBOOL IsInIgnoreURLs( NSString *pURLString )
{
	MacOSBOOL bRet = NO;

	if ( !pURLString )
		return NO;

	static NSArray *pIgnoreURLStrings = nil;

	if ( !pIgnoreURLStrings )
	{
		MutexGuard aGuard( aCurrentInstanceSecurityURLCacheMutex );

		if ( !pIgnoreURLStrings )
		{
			// Readable system folders listed in the following URL's
			// "Container Directories and File System Access :: PowerBox and
			// File System Access Outside of Your Container" section:
			// http://developer.apple.com/library/mac/#documentation/Security/Conceptual/AppSandboxDesignGuide/AppSandboxInDepth/AppSandboxInDepth.html
			NSArray *pReadableSystemFolders = [NSArray arrayWithObjects:
				@"/bin",
				@"/sbin",
				@"/usr/bin",
				@"/usr/lib",
				@"/usr/sbin",
				@"/usr/share",
				@"/System",
				nil];

			// Unreadable system folders
			NSArray *pUnreadableSystemFolders = [NSArray arrayWithObjects:
				@"/.vol",
				@"/dev",
				@"/etc",
				@"/private",
				@"/tmp",
				@"/var",
				nil];

			// Applications folder
			NSArray *pApplicationFolders = NSSearchPathForDirectoriesInDomains( NSApplicationDirectory, NSLocalDomainMask, NO );

			// Libary folder
			NSArray *pLibraryFolders = NSSearchPathForDirectoriesInDomains( NSLibraryDirectory, NSLocalDomainMask, NO );

			NSMutableArray *pTmpPaths = [NSMutableArray arrayWithCapacity:
				( pReadableSystemFolders ? [pReadableSystemFolders count] : 0 ) +
				( pUnreadableSystemFolders ? [pUnreadableSystemFolders count] : 0 ) +
				( pApplicationFolders ? [pApplicationFolders count] : 0 ) +
				( pLibraryFolders ? [pLibraryFolders count] : 0 )];
			if ( pTmpPaths )
			{
				[pTmpPaths addObjectsFromArray:pReadableSystemFolders];
				[pTmpPaths addObjectsFromArray:pUnreadableSystemFolders];
				[pTmpPaths addObjectsFromArray:pApplicationFolders];
				[pTmpPaths addObjectsFromArray:pLibraryFolders];

				NSMutableArray *pTmpURLs = [NSMutableArray arrayWithCapacity:[pTmpPaths count] + 3];
				if ( pTmpURLs )
				{
					NSUInteger nCount = [pTmpPaths count];
					NSUInteger i = 0;
					for ( ; i < nCount; i++ )
					{
						NSString *pTmpPath = [pTmpPaths objectAtIndex:i];
						if ( pTmpPath )
						{
							NSURL *pTmpURL = [NSURL fileURLWithPath:pTmpPath isDirectory:YES];
							if ( pTmpURL )
								[pTmpURLs addObject:pTmpURL];
						}
					}

					// Add home folder
					NSString *pHomeDir = NSHomeDirectory();
					if ( pHomeDir )
					{
						NSURL *pTmpURL = [NSURL fileURLWithPath:pHomeDir isDirectory:YES];
						if ( pTmpURL )
							[pTmpURLs addObject:pTmpURL];
					}

					// Add main bundle folder
					NSBundle *pMainBundle = [NSBundle mainBundle];
					if ( pMainBundle )
					{
						NSURL *pTmpURL = [pMainBundle bundleURL];
						if ( pTmpURL )
							[pTmpURLs addObject:pTmpURL];
					}

					// Add temporary folder
					NSString *pTemporaryDir = NSTemporaryDirectory();
					if ( pTemporaryDir )
					{
						NSURL *pTmpURL = [NSURL fileURLWithPath:pTemporaryDir isDirectory:YES];
						if ( pTmpURL )
							[pTmpURLs addObject:pTmpURL];
					}

					// Convert URLs to absolute strings
					nCount = [pTmpURLs count];
					pTmpPaths = [NSMutableArray arrayWithCapacity:nCount];
					if ( pTmpPaths )
					{
						for ( i = 0; i < nCount; i++ )
						{
							NSURL *pTmpURL = [pTmpURLs objectAtIndex:i];
							if ( pTmpURL )
							{
								pTmpURL = [pTmpURL URLByStandardizingPath];
								if ( pTmpURL )
								{
									pTmpURL = [pTmpURL URLByResolvingSymlinksInPath];
									if ( pTmpURL )
									{
										NSString *pTmpURLString = [pTmpURL absoluteString];
										if ( pTmpURLString && [pTmpURLString length] )
											[pTmpPaths addObject:pTmpURLString];
									}
								}
							}
						}

						pIgnoreURLStrings = [NSArray arrayWithArray:pTmpPaths];
						if ( pIgnoreURLStrings )
							[pIgnoreURLStrings retain];
					}
				}
			}
		}
	}

	if ( pIgnoreURLStrings )
	{
		NSUInteger nCount = [pIgnoreURLStrings count];
		NSUInteger i = 0;
		for ( ; i < nCount; i++ )
		{
			NSString *pTmpURLString = [pIgnoreURLStrings objectAtIndex:i];
			if ( pTmpURLString )
			{
				NSRange aTmpRange = [pURLString rangeOfString:pTmpURLString];
				if ( !aTmpRange.location && aTmpRange.length )
				{
					bRet = YES;
					break;
				}
			}
		}
	}

	return bRet;
}

static NSURL *ResolveAliasURL( const NSURL *pURL, MacOSBOOL bMustShowDialogIfNoBookmark, const NSString *pTitle, NSMutableArray *pSecurityScopedURLs )
{
	NSURL *pRet = nil;

	if ( pURL )
	{
		if ( pSecurityScopedURLs )
			AcquireSecurityScopedURL( pURL, bMustShowDialogIfNoBookmark, NO, pTitle, pSecurityScopedURLs );

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
						NSURL *pRecursedURL = ResolveAliasURL( pURL, bMustShowDialogIfNoBookmark, pTitle, pSecurityScopedURLs );
						if ( pRecursedURL )
							pRet = pRecursedURL;
						else
							pRet = pURL;
					}
				}
			}
		}
	}

	return pRet;
}

static void AcquireSecurityScopedURL( const NSURL *pURL, MacOSBOOL bMustShowDialogIfNoBookmark, MacOSBOOL bResolveAliasURLs, const NSString *pTitle, NSMutableArray *pSecurityScopedURLs )
{
	if ( pURL && [pURL isFileURL] && pSecurityScopedURLs )
	{
		NSString *pURLString = [pURL absoluteString];
		if ( !pURLString || ![pURLString length] )
			return;

		if ( IsInIgnoreURLs( pURLString ) )
			return;

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
					if ( !pURL )
						break;

					// If the path does not exist, use partial URL
					pURL = [pURL URLByResolvingSymlinksInPath];
					if ( !pURL || ![pURL checkResourceIsReachableAndReturnError:nil] )
					{
						// Abort if the parent folder is "/" or if only last
						// component is missing and the URL is in the current
						// instance cache
						if ( i == 1 || ( i == nCount - 1 && IsCurrentInstanceCacheSecurityURL( pURL, NO ) ) )
							pURL = nil;

						break;
					}

					if ( bResolveAliasURLs )
					{
						NSNumber *pAlias = nil;
						if ( [pURL getResourceValue:&pAlias forKey:NSURLIsAliasFileKey error:nil] && pAlias && [pAlias boolValue] )
						{
							NSURL *pResolvedURL = ResolveAliasURL( pURL, bMustShowDialogIfNoBookmark, pTitle, pSecurityScopedURLs );
							if ( pResolvedURL )
								pURL = pResolvedURL;
						}
					}

					// Ignore current instance cached security URLs
					if ( IsCurrentInstanceCacheSecurityURL( pURL, YES ) )
					{
						pURL = nil;
						break;
					}
				}

				if ( pURL )
				{
					// Check if there are any cached security scoped bookmarks
					// for this URL or any of its parent folders
					MacOSBOOL bShowOpenPanel = YES;
					NSUserDefaults *pUserDefaults = [NSUserDefaults standardUserDefaults];
					NSURL *pTmpURL = pURL;
					MacOSBOOL bSecurityScopedURLFound = NO;
					while ( pUserDefaults && pTmpURL && !bSecurityScopedURLFound )
					{
						NSString *pKey = [pTmpURL absoluteString];
						if ( pKey && [pKey length] )
						{
							NSObject *pBookmarkData = [pUserDefaults objectForKey:pKey];
							if ( pBookmarkData && [pBookmarkData isKindOfClass:[NSData class]] )
							{
								MacOSBOOL bStale = NO;
								NSURL *pSecurityScopedURL = [NSURL URLByResolvingBookmarkData:(NSData *)pBookmarkData options:NSURLBookmarkResolutionWithoutUI | NSURLBookmarkResolutionWithoutMounting | NSURLBookmarkResolutionWithSecurityScope relativeToURL:nil bookmarkDataIsStale:&bStale error:nil];
								if ( !bStale && pSecurityScopedURL && [pSecurityScopedURL respondsToSelector:@selector(startAccessingSecurityScopedResource)] )
								{
									if ( [pSecurityScopedURL startAccessingSecurityScopedResource] )
									{
										bSecurityScopedURLFound = YES;
										[pSecurityScopedURLs addObject:pSecurityScopedURL];
									}

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

					if ( bShowOpenPanel && !bMustShowDialogIfNoBookmark && IsURLReadableOrWritable( pURL ) )
						bShowOpenPanel = NO;

					if ( bShowOpenPanel && !bSecurityScopedURLFound && !Application::IsShutDown() )
					{
						IMutex &rSolarMutex = Application::GetSolarMutex();
						rSolarMutex.acquire();
						if ( !Application::IsShutDown() )
						{
							NSWindow *pNSWindow = nil;
							if ( Application_beginModalSheet( &pNSWindow ) )
							{
								// Ignore any AWT events while the open dialog
								// is showing to emulate a modal dialog
								VCLRequestSecurityScopedURL *pVCLRequestSecurityScopedURL = [VCLRequestSecurityScopedURL createWithURL:pURL title:pTitle window:pNSWindow];
								NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
								[pVCLRequestSecurityScopedURL performSelectorOnMainThread:@selector(requestSecurityScopedURL:) withObject:pVCLRequestSecurityScopedURL waitUntilDone:YES modes:pModes];
								while ( ![pVCLRequestSecurityScopedURL finished] )
								{
									[pVCLRequestSecurityScopedURL performSelectorOnMainThread:@selector(checkForErrors:) withObject:pVCLRequestSecurityScopedURL waitUntilDone:YES modes:pModes];
									Application::Yield();
								}

								NSURL *pSecurityScopedURL = [pVCLRequestSecurityScopedURL securityScopedURL];
								if ( pSecurityScopedURL && [pSecurityScopedURL respondsToSelector:@selector(startAccessingSecurityScopedResource)] && [pSecurityScopedURL startAccessingSecurityScopedResource] )
								{
									bSecurityScopedURLFound = YES;
									[pSecurityScopedURLs addObject:pSecurityScopedURL];
								}

								[pVCLRequestSecurityScopedURL performSelectorOnMainThread:@selector(destroy:) withObject:pVCLRequestSecurityScopedURL waitUntilDone:YES modes:pModes];

								Application_endModalSheet();
							}
						}

						rSolarMutex.release();
					}
				}
			}
		}
	}
}

@implementation VCLRequestSecurityScopedURL

+ (id)createWithURL:(NSURL *)pURL title:(NSString *)pTitle window:(NSWindow *)pWindow
{
	VCLRequestSecurityScopedURL *pRet = [[VCLRequestSecurityScopedURL alloc] initWithURL:pURL title:pTitle window:pWindow];
	[pRet autorelease];
	return pRet;
}

- (void)cancel:(id)pObject;
{
	if ( !mbCancelled && mpOpenPanel )
	{
		// Prevent crashing by only allowing cancellation to be requested once
		mbCancelled = YES;

		@try
		{
			// When running in the sandbox, native file dialog calls may
			// throw exceptions if the PowerBox daemon process is killed
			[mpOpenPanel cancel:pObject];
		}
		@catch ( NSException *pExc )
		{
			if ( pExc )
				NSLog( @"%@", [pExc callStackSymbols] );
		}
	}
}

- (void)checkForErrors:(id)pObject
{
	// Detect if the sheet window has been closed without any call to the
	// completion handler
	if ( !mbFinished && ( !mpAttachedSheet || !mpWindow || [mpWindow attachedSheet] != mpAttachedSheet ) )
		[self cancel:self];
}

- (void)dealloc
{
	[self destroy:self];

	[super dealloc];
}

- (void)destroy:(id)pObject
{
	if ( !mbFinished )
		[self cancel:self];

	if ( mpAttachedSheet )
	{
		[mpAttachedSheet release];
		mpAttachedSheet = nil;
	}

	if ( mpOpenPanel && mbFinished )
	{
		@try
		{
			// When running in the sandbox, native file dialog calls may
			// throw exceptions if the PowerBox daemon process is killed
			[mpOpenPanel setDelegate:nil];
		}
		@catch ( NSException *pExc )
		{
			if ( pExc )
				NSLog( @"%@", [pExc callStackSymbols] );
		}

		[mpOpenPanel release];
		mpOpenPanel = nil;
	}

	if ( mpSecurityScopedURL )
	{
		[mpSecurityScopedURL release];
		mpSecurityScopedURL = nil;
	}

	if ( mpTitle )
	{
		[mpTitle release];
		mpTitle = nil;
	}

	if ( mpURL )
	{
		[mpURL release];
		mpURL = nil;
	}

	if ( mpWindow )
	{
		if ( mbWindowOwner )
			[mpWindow close];
		[mpWindow release];
		mpWindow = nil;
	}
}

- (MacOSBOOL)finished
{
	return mbFinished;
}

- (id)initWithURL:(NSURL *)pURL title:(NSString *)pTitle window:(NSWindow *)pWindow
{
	[super init];

	mpAttachedSheet = nil;
	mbCancelled = NO;
	mbFinished = NO;
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

	mpWindow = pWindow;
	if ( mpWindow )
		[mpWindow retain];

	mbWindowOwner = NO;

	return self;
}

#ifdef USE_SHOULDENABLEURL_DELEGATE_SELECTOR
- (MacOSBOOL)panel:(id)pSender shouldEnableURL:(NSURL *)pURL
#else	// USE_SHOULDENABLEURL_DELEGATE_SELECTOR
- (MacOSBOOL)panel:(id)pSender validateURL:(NSURL *)pURL error:(NSError *)ppError
#endif	// USE_SHOULDENABLEURL_DELEGATE_SELECTOR
{
	if ( pURL )
	{
		pURL = [pURL URLByStandardizingPath];
		if ( pURL )
			pURL = [pURL URLByResolvingSymlinksInPath];
	}

	MacOSBOOL bRet = ( pURL && mpURL && [pURL isFileURL] && [pURL isEqual:mpURL] );

#ifndef USE_SHOULDENABLEURL_DELEGATE_SELECTOR
	if ( !bRet )
		[self cancel:self];
#endif	// !USE_SHOULDENABLEURL_DELEGATE_SELECTOR

	return bRet;
}

- (void)panel:(id)pSender didChangeToDirectoryURL:(NSURL *)pURL
{
	if ( pURL )
	{
		pURL = [pURL URLByStandardizingPath];
		if ( pURL )
			pURL = [pURL URLByResolvingSymlinksInPath];
	}

	if ( mpURL && mpOpenPanel && ( !pURL || ![pURL isEqual:mpURL] ) )
	{
		@try
		{
			// When running in the sandbox, native file dialog calls may
			// throw exceptions if the PowerBox daemon process is killed
#ifdef USE_SHOULDENABLEURL_DELEGATE_SELECTOR
			[mpOpenPanel setDirectoryURL:mpURL];
#else	// USE_SHOULDENABLEURL_DELEGATE_SELECTOR
			[mpOpenPanel cancel:self];
#endif	// USE_SHOULDENABLEURL_DELEGATE_SELECTOR
		}
		@catch ( NSException *pExc )
		{
			if ( pExc )
				NSLog( @"%@", [pExc callStackSymbols] );
		}
	}
}

- (void)panel:(id)pObject willExpand:(MacOSBOOL)bExpanding
{
	// Stop exceptions from being logged on Mac OS X 10.9
}

- (void)requestSecurityScopedURL:(id)pObject
{
	// Do not allow recursion or reuse
	if ( mpAttachedSheet || mbCancelled || mbFinished || mpOpenPanel || mpSecurityScopedURL || !mpURL )
		return;

	if ( !mpWindow || ![mpWindow canBecomeKeyWindow] || ( ![mpWindow isVisible] && ![mpWindow isMiniaturized] ) )
	{
		if ( mpWindow )
			[mpWindow release];

		NSRect aContentRect = NSMakeRect( 0, 0, 400, 25 );
		NSScreen *pScreen = [NSScreen mainScreen];
		if ( pScreen )
		{
			NSRect aFrame = [pScreen visibleFrame];
			aContentRect.origin.x = aFrame.origin.x + ( ( aFrame.size.width - aContentRect.size.width ) / 2 );
			if ( aContentRect.origin.x < aFrame.origin.x )
				aContentRect.origin.x = aFrame.origin.x;
			aContentRect.origin.y = aFrame.origin.y + ( aFrame.size.height * 0.75f );
			if ( aContentRect.origin.y < aFrame.origin.y )
				aContentRect.origin.y = aFrame.origin.y;
		}

		mpWindow = [[NSWindow alloc] initWithContentRect:aContentRect styleMask:NSTitledWindowMask | NSClosableWindowMask backing:NSBackingStoreBuffered defer:YES];
		if ( mpWindow )
		{
			mbWindowOwner = YES;

			NSBundle *pBundle = [NSBundle mainBundle];
			if ( pBundle )
			{
				NSDictionary *pDict = [pBundle infoDictionary];
				if ( pDict )
				{
					NSString *pName = [pDict valueForKey:@"CFBundleName"];
					if ( pName && [pName isKindOfClass:[NSString class]] )
						[mpWindow setTitle:pName];
				}
			}

			[mpWindow setReleasedWhenClosed:NO];
			[mpWindow makeKeyAndOrderFront:self];
		}
	}

	// We cannot display the panel if there is no window or the window already
	// has an attached sheet
	if ( !mpWindow || [mpWindow attachedSheet] )
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
		@try
		{
			// When running in the sandbox, native file dialog calls may
			// throw exceptions if the PowerBox daemon process is killed
			mpOpenPanel = [NSOpenPanel openPanel];
			if ( mpOpenPanel )
			{
				[mpOpenPanel retain];
				[mpOpenPanel setAllowsMultipleSelection:NO];
				[mpOpenPanel setDirectoryURL:mpURL];
				[mpOpenPanel setCanChooseDirectories:YES];
				[mpOpenPanel setCanChooseFiles:NO];
				if ( mpTitle && [mpTitle length] )
					[mpOpenPanel setTitle:mpTitle];

				NSWindow *pOldAttachedSheet = [mpWindow attachedSheet];
				[mpOpenPanel setDelegate:self];

				// Retain self to ensure that we don't release it before the
				// completion handler executes
				[self retain];
				[mpOpenPanel beginSheetModalForWindow:mpWindow completionHandler:^(NSInteger nRet) {
					if ( mpOpenPanel )
					{
						@try
						{
							// When running in the sandbox, native file dialog
							// calls may throw exceptions if the PowerBox
							// daemon process is killed
							if ( nRet == NSFileHandlingPanelOKButton )
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
													NSURL *pResolvedURL = [NSURL URLByResolvingBookmarkData:pData options:NSURLBookmarkResolutionWithoutUI | NSURLBookmarkResolutionWithoutMounting | NSURLBookmarkResolutionWithSecurityScope relativeToURL:nil bookmarkDataIsStale:&bStale error:nil];
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
						}
						@catch ( NSException *pExc )
						{
							if ( pExc )
								NSLog( @"%@", [pExc callStackSymbols] );
						}
					}

					mbFinished = YES;

					if ( mpAttachedSheet )
					{
						[mpAttachedSheet release];
						mpAttachedSheet = nil;
					}

					if ( mpOpenPanel )
					{
						[mpOpenPanel release];
						mpOpenPanel = nil;
					}

					// Post an event to wakeup the VCL event thread if the VCL
					// event dispatch thread is in a potentially long wait
					Application_postWakeUpEvent();

					[self release];
				}];

				NSWindow *pAttachedSheet = [mpWindow attachedSheet];
				if ( pAttachedSheet && pAttachedSheet != pOldAttachedSheet )
				{
					mpAttachedSheet = pAttachedSheet;
					if ( mpAttachedSheet )
						[mpAttachedSheet retain];
				}
				else
				{
					mbFinished = YES;
				}
			}
		}
		@catch ( NSException *pExc )
		{
			mbFinished = YES;
			if ( pExc )
				NSLog( @"%@", [pExc callStackSymbols] );
		}
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
				NSUInteger nCount = [pWindows count];
				NSUInteger i = 0;
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
		// Fix excessive CPU usage when this is called from by adding a slight
		// wait if there are no pending events
		NSDate *pDate = [NSDate date];
		if ( pDate )
			pDate = [NSDate dateWithTimeInterval:0.05f sinceDate:pDate];

		NSEvent *pEvent;
		while ( ( pEvent = [pApp nextEventMatchingMask:NSAnyEventMask untilDate:pDate inMode:( [pApp modalWindow] ? NSModalPanelRunLoopMode : NSDefaultRunLoopMode ) dequeue:YES] ) != nil )
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

sal_Bool Application_beginModalSheet( id *pNSWindowForSheet )
{
	SalData *pSalData = GetSalData();

	// Do not allow more than one window to display a modal sheet
	if ( pSalData->mbInNativeModalSheet || !pNSWindowForSheet )
		return false;

	JavaSalFrame *pFocusFrame = NULL;

	// Get the active document window
	Window *pWindow = Application::GetActiveTopWindow();
	if ( pWindow )
		pFocusFrame = (JavaSalFrame *)pWindow->ImplGetFrame();

	if ( !pFocusFrame )
		pFocusFrame = pSalData->mpFocusFrame;

	// Fix bug 3294 by not attaching to utility windows
	while ( pFocusFrame && ( pFocusFrame->IsFloatingFrame() || pFocusFrame->IsUtilityWindow() || pFocusFrame->mbShowOnlyMenus ) )
		pFocusFrame = pFocusFrame->mpParent;

	// Fix bug 1106. If the focus frame is not set or is not visible, find the
	// first visible non-floating, non-utility frame.
	if ( !pFocusFrame || !pFocusFrame->mbVisible )
	{
		pFocusFrame = NULL;
		for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
		{
			if ( (*it)->mbVisible && !(*it)->IsFloatingFrame() && !(*it)->IsUtilityWindow() && !(*it)->mbShowOnlyMenus )
			{
				pFocusFrame = *it;
				break;
			}
		}
	}

	pSalData->mbInNativeModalSheet = true;
	pSalData->mpNativeModalSheetFrame = pFocusFrame;

	if ( pFocusFrame )
	{
		pSalData->mpNativeModalSheetFrame = pFocusFrame;
		*pNSWindowForSheet = pFocusFrame->GetNativeWindow();
	}
	else
	{
		*pNSWindowForSheet = nil;
	}

	return sal_True;
}

void Application_endModalSheet()
{
	SalData *pSalData = GetSalData();
	pSalData->mbInNativeModalSheet = false;
	pSalData->mpNativeModalSheetFrame = NULL;
}

void Application_postWakeUpEvent()
{
	JavaSalEvent *pUserEvent = new JavaSalEvent( SALEVENT_USEREVENT, NULL, NULL );
	JavaSalEventQueue::postCachedEvent( pUserEvent );
	pUserEvent->release();
}

id Application_acquireSecurityScopedURLFromOUString( const OUString *pNonSecurityScopedURL, unsigned char bMustShowDialogIfNoBookmark, const OUString *pDialogTitle )
{
	id pRet = nil;

	if ( ImplGetSVData() && ImplGetSVData()->mpDefInst && pNonSecurityScopedURL && pNonSecurityScopedURL->getLength() )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSString *pString = [NSString stringWithCharacters:pNonSecurityScopedURL->getStr() length:pNonSecurityScopedURL->getLength()];
		if ( pString )
			pRet = Application_acquireSecurityScopedURLFromNSURL( [NSURL URLWithString:pString], bMustShowDialogIfNoBookmark, pDialogTitle && pDialogTitle->getLength() ? [NSString stringWithCharacters:pDialogTitle->getStr() length:pDialogTitle->getLength()] : nil );

		[pPool release];
	}

	return pRet;
}

id Application_acquireSecurityScopedURLFromNSURL( const id pNonSecurityScopedURL, unsigned char bMustShowDialogIfNoBookmark, const id pDialogTitle )
{
	id pRet = nil;

	if ( ImplGetSVData() && ImplGetSVData()->mpDefInst && pNonSecurityScopedURL )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		if ( [pNonSecurityScopedURL isKindOfClass:[NSURL class]] )
		{
			NSURL *pURL = (NSURL *)pNonSecurityScopedURL;
			if ( [pURL isFileURL] )
			{
				pURL = [pURL URLByStandardizingPath];
				if ( pURL )
				{
					pURL = [pURL URLByResolvingSymlinksInPath];
					if ( pURL )
					{
						NSMutableArray *pSecurityScopedURLs = [NSMutableArray arrayWithCapacity:2];
						AcquireSecurityScopedURL( pURL, (MacOSBOOL)bMustShowDialogIfNoBookmark, YES, pDialogTitle, pSecurityScopedURLs );
						if ( pSecurityScopedURLs && [pSecurityScopedURLs count] )
						{
							pRet = pSecurityScopedURLs;
							[pRet retain];
						}
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
			if ( pURL && !IsInIgnoreURLs( [pURL absoluteString] ) )
			{
				NSData *pData = [pURL bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope includingResourceValuesForKeys:nil relativeToURL:nil error:nil];
				if ( pData )
				{
					MacOSBOOL bStale = NO;
					NSURL *pResolvedURL = [NSURL URLByResolvingBookmarkData:pData options:NSURLBookmarkResolutionWithoutUI | NSURLBookmarkResolutionWithoutMounting | NSURLBookmarkResolutionWithSecurityScope relativeToURL:nil bookmarkDataIsStale:&bStale error:nil];
					if ( pResolvedURL && !bStale && [pResolvedURL isFileURL] )
					{
						pResolvedURL = [pResolvedURL URLByStandardizingPath];
						if ( pResolvedURL )
						{
							pResolvedURL = [pResolvedURL URLByResolvingSymlinksInPath];
							if ( pResolvedURL )
							{
								NSUserDefaults *pUserDefaults = [NSUserDefaults standardUserDefaults];
								if ( pUserDefaults )
								{
									NSString *pKey = [pResolvedURL absoluteString];
									if ( pKey && [pKey length] )
									{
										[pUserDefaults setObject:pData forKey:pKey];
										[pUserDefaults synchronize];
									}
								}
							}
						}
					}
				}

				NSString *pKey = [pURL absoluteString];
				if ( pKey && [pKey length] )
				{
					// URLs from save panel and versions browser, even if
					// non-existent are good for the life of the current
					// instance. Note that it appears that the path is
					// accessible even if the path's file reference changes.
					MutexGuard aGuard( aCurrentInstanceSecurityURLCacheMutex );

					if ( !pCurrentInstanceSecurityURLCacheDictionary )
					{
						pCurrentInstanceSecurityURLCacheDictionary = [NSMutableDictionary dictionaryWithCapacity:10];
						if ( pCurrentInstanceSecurityURLCacheDictionary )
							[pCurrentInstanceSecurityURLCacheDictionary retain];
					}

					if ( pCurrentInstanceSecurityURLCacheDictionary )
						[pCurrentInstanceSecurityURLCacheDictionary setObject:[NSNumber numberWithBool:[pURL checkResourceIsReachableAndReturnError:nil]] forKey:pKey];
				}
			}
		}
	}

	[pPool release];
}

void Application_releaseSecurityScopedURL( id pSecurityScopedURLs )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pSecurityScopedURLs )
	{
		if ( [pSecurityScopedURLs isKindOfClass:[NSArray class]] )
		{
			NSArray *pArray = (NSArray *)pSecurityScopedURLs;
			NSUInteger nCount = [pArray count];
			NSUInteger i = 0;
			for ( ; i < nCount; i++ )
			{
				NSURL *pURL = [pArray objectAtIndex:i];
				if ( pURL && [pURL isKindOfClass:[NSURL class]] && [pURL respondsToSelector:@selector(stopAccessingSecurityScopedResource)] )
					[(NSURL *)pURL stopAccessingSecurityScopedResource];
			}
		}

		[pSecurityScopedURLs release];
	}

	[pPool release];
}
