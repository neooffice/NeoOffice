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

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include "java/saldata.hxx"

#include "salinst_cocoa.h"

// Uncomment the following line to implement the panel:shouldEnableURL:
// delegate selector. Note: implementing that selector will cause hanging in
// Open dialogs after a Save panel has been displayed on Mac OS X 10.9
// #define USE_SHOULDENABLEURL_DELEGATE_SELECTOR

typedef sal_Bool Application_isRunningInSandbox_Type();

static ::osl::Mutex aCurrentInstanceSecurityURLCacheMutex;
static NSMutableDictionary *pCurrentInstanceSecurityURLCacheDictionary = nil;
static BOOL bIsCppUnitTesterInitialized = NO;
static BOOL bIsCppUnitTester = NO;
static BOOL bIsRunningInSandbox = YES;
static Application_isRunningInSandbox_Type *pApplication_isRunningInSandbox = NULL;

using namespace osl;

static void AcquireSecurityScopedURL( NSURL *pURL, BOOL bMustShowDialogIfNoBookmark, BOOL bResolveAliasURLs, NSString *pTitle, NSMutableArray *pSecurityScopedURLs );

@interface VCLRequestSecurityScopedURL : NSObject <NSOpenSavePanelDelegate>
{
	BOOL					mbCancelled;
	BOOL					mbFinished;
	NSOpenPanel*			mpOpenPanel;
	NSURL*					mpSecurityScopedURL;
	NSString*				mpTitle;
	NSURL*					mpURL;
}
+ (id)createWithURL:(NSURL *)pURL title:(NSString *)pTitle;
- (void)cancel:(id)pObject;
- (void)dealloc;
- (void)destroy:(id)pObject;
- (BOOL)finished;
- (id)initWithURL:(NSURL *)pURL title:(NSString *)pTitle;
#ifdef USE_SHOULDENABLEURL_DELEGATE_SELECTOR
- (BOOL)panel:(id)pSender shouldEnableURL:(NSURL *)pURL;
#else	// USE_SHOULDENABLEURL_DELEGATE_SELECTOR
- (BOOL)panel:(id)pSender validateURL:(NSURL *)pURL error:(NSError **)ppError;
#endif	// USE_SHOULDENABLEURL_DELEGATE_SELECTOR
- (void)panel:(id)pSender didChangeToDirectoryURL:(NSURL *)pURL;
- (void)panel:(id)pObject willExpand:(BOOL)bExpanding;
- (void)requestSecurityScopedURL:(id)pObject;
- (NSURL *)securityScopedURL;
- (void)setResult:(NSInteger)nResult;
@end

static BOOL ImplIsCppUnitTester()
{
	if ( !bIsCppUnitTesterInitialized )
	{
		bIsCppUnitTesterInitialized = YES;

		OUString sFile;
		if ( osl_getExecutableFile( &sFile.pData ) == osl_Process_E_None )
		{
			sFile = sFile.copy( sFile.lastIndexOf( '/' ) + 1 );
			if ( sFile == "cppunittester" )
				bIsCppUnitTester = YES;
		}
	}

	return bIsCppUnitTester;
}

static BOOL IsURLReadableOrWritable( NSURL *pURL )
{
	BOOL bRet = NO;

	if ( pURL )
	{
		NSNumber *pReadable = nil;
		NSNumber *pWritable = nil;
		if ( ( [pURL getResourceValue:&pReadable forKey:NSURLIsReadableKey error:nil] && pReadable && [pReadable boolValue] ) || ( [pURL getResourceValue:&pWritable forKey:NSURLIsWritableKey error:nil] && pWritable && [pWritable boolValue] ) )
			bRet = YES;
	}

	return bRet;
}

static BOOL IsCurrentInstanceCacheSecurityURL( NSURL *pURL, BOOL bExists )
{
	BOOL bRet = NO;

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
						// Fix unexpected display of native open dialog when
						// the URL has been deleted or renamed after caching
						// the URL by assuming all the URL is still accessible
						bRet = YES;
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
static BOOL IsInIgnoreURLs( NSString *pURLString )
{
	BOOL bRet = NO;

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
				@"/.DocumentRevisions-V100",
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

static NSURL *ResolveAliasURL( NSURL *pURL, BOOL bMustShowDialogIfNoBookmark, NSString *pTitle, NSMutableArray *pSecurityScopedURLs )
{
	NSURL *pRet = nil;

	if ( pURL )
	{
		if ( pSecurityScopedURLs )
			AcquireSecurityScopedURL( pURL, bMustShowDialogIfNoBookmark, NO, pTitle, pSecurityScopedURLs );

		NSData *pData = [NSURL bookmarkDataWithContentsOfURL:pURL error:nil];
		if ( pData )
		{
			BOOL bStale = NO;
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

static void AcquireSecurityScopedURL( NSURL *pURL, BOOL bMustShowDialogIfNoBookmark, BOOL bResolveAliasURLs, NSString *pTitle, NSMutableArray *pSecurityScopedURLs )
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
						// Abort if the parent folder is "/", if more than the
						// last component is missing, or if only the last
						// component is missing and the URL is in the current
						// instance cache
						if ( i == 1 || i < nCount - 1 || ( i == nCount - 1 && IsCurrentInstanceCacheSecurityURL( pURL, NO ) ) )
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
					// Don't display open panel if we are not running in the
					// sandbox
					if ( !pApplication_isRunningInSandbox )
						pApplication_isRunningInSandbox = (Application_isRunningInSandbox_Type *)dlsym( RTLD_MAIN_ONLY, "Application_isRunningInSandbox" );
					if ( bIsRunningInSandbox && pApplication_isRunningInSandbox && !pApplication_isRunningInSandbox() )
						bIsRunningInSandbox = NO;

					// Check if there are any cached security scoped bookmarks
					// for this URL or any of its parent folders. Fix slowness
					// during saving by suppressing the open panel when not
					// running in the sandbox.
					BOOL bShowOpenPanel = bIsRunningInSandbox && !ImplIsCppUnitTester();
					NSUserDefaults *pUserDefaults = [NSUserDefaults standardUserDefaults];
					NSURL *pTmpURL = pURL;
					BOOL bSecurityScopedURLFound = NO;
					while ( pUserDefaults && pTmpURL && !bSecurityScopedURLFound )
					{
						NSString *pKey = [pTmpURL absoluteString];
						if ( pKey && [pKey length] )
						{
							NSObject *pBookmarkData = [pUserDefaults objectForKey:pKey];
							if ( pBookmarkData && [pBookmarkData isKindOfClass:[NSData class]] )
							{
								BOOL bStale = NO;
								NSURL *pSecurityScopedURL = [NSURL URLByResolvingBookmarkData:(NSData *)pBookmarkData options:NSURLBookmarkResolutionWithoutUI | NSURLBookmarkResolutionWithoutMounting | NSURLBookmarkResolutionWithSecurityScope relativeToURL:nil bookmarkDataIsStale:&bStale error:nil];
								if ( !bStale && pSecurityScopedURL )
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
						// Don't lock mutex as we expect callbacks to this
						// object from a different thread while the dialog is
						// showing
						sal_uLong nCount = Application::ReleaseSolarMutex();

						// Ignore any AWT events while the open dialog is
						// showing to emulate a modal dialog
						VCLRequestSecurityScopedURL *pVCLRequestSecurityScopedURL = [VCLRequestSecurityScopedURL createWithURL:pURL title:pTitle];
						NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
						[pVCLRequestSecurityScopedURL performSelectorOnMainThread:@selector(requestSecurityScopedURL:) withObject:pVCLRequestSecurityScopedURL waitUntilDone:YES modes:pModes];

						NSURL *pSecurityScopedURL = [pVCLRequestSecurityScopedURL securityScopedURL];
						if ( pSecurityScopedURL && [pSecurityScopedURL startAccessingSecurityScopedResource] )
						{
							bSecurityScopedURLFound = YES;
							[pSecurityScopedURLs addObject:pSecurityScopedURL];
						}

						[pVCLRequestSecurityScopedURL performSelectorOnMainThread:@selector(destroy:) withObject:pVCLRequestSecurityScopedURL waitUntilDone:YES modes:pModes];

						Application::AcquireSolarMutex( nCount );
					}
				}
			}
		}
	}
}

@implementation VCLRequestSecurityScopedURL

+ (id)createWithURL:(NSURL *)pURL title:(NSString *)pTitle
{
	VCLRequestSecurityScopedURL *pRet = [[VCLRequestSecurityScopedURL alloc] initWithURL:pURL title:pTitle];
	[pRet autorelease];
	return pRet;
}

- (void)cancel:(id)pObject
{
	(void)pObject;

	if ( mpOpenPanel && !mbCancelled && !mbFinished )
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

- (void)dealloc
{
	[self destroy:self];

	[super dealloc];
}

- (void)destroy:(id)pObject
{
	(void)pObject;

	if ( !mbFinished )
		[self cancel:self];

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
}

- (BOOL)finished
{
	return mbFinished;
}

- (id)initWithURL:(NSURL *)pURL title:(NSString *)pTitle
{
	[super init];

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

	return self;
}

#ifdef USE_SHOULDENABLEURL_DELEGATE_SELECTOR
- (BOOL)panel:(id)pSender shouldEnableURL:(NSURL *)pURL
#else	// USE_SHOULDENABLEURL_DELEGATE_SELECTOR
- (BOOL)panel:(id)pSender validateURL:(NSURL *)pURL error:(NSError **)ppError
#endif	// USE_SHOULDENABLEURL_DELEGATE_SELECTOR
{
	(void)pSender;

#ifndef USE_SHOULDENABLEURL_DELEGATE_SELECTOR
	if ( ppError )
		*ppError = nil;
#endif	// !USE_SHOULDENABLEURL_DELEGATE_SELECTOR

	if ( pURL )
	{
		pURL = [pURL URLByStandardizingPath];
		if ( pURL )
			pURL = [pURL URLByResolvingSymlinksInPath];
	}

	BOOL bRet = ( pURL && mpURL && [pURL isFileURL] && [pURL isEqual:mpURL] );

#ifndef USE_SHOULDENABLEURL_DELEGATE_SELECTOR
	if ( !bRet )
		[self cancel:self];
#endif	// !USE_SHOULDENABLEURL_DELEGATE_SELECTOR

	return bRet;
}

- (void)panel:(id)pSender didChangeToDirectoryURL:(NSURL *)pURL
{
	(void)pSender;

	if ( mpOpenPanel && !mbCancelled && !mbFinished && mpURL )
	{
		// Fix exception on OS X 10.10 by ensuring the parameter is really an
		// NSURL instance
		if ( pURL && [pURL isKindOfClass:[NSURL class]] && [pURL isFileURL] )
		{
			pURL = [pURL URLByStandardizingPath];
			if ( pURL )
				pURL = [pURL URLByResolvingSymlinksInPath];
		}

		if ( !pURL || ![pURL isEqual:mpURL] )
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
}

- (void)panel:(id)pObject willExpand:(BOOL)bExpanding
{
	// Stop exceptions from being logged on Mac OS X 10.9
	(void)pObject;
	(void)bExpanding;
}

- (void)requestSecurityScopedURL:(id)pObject
{
	(void)pObject;

	// Do not allow recursion or reuse
	if ( mbCancelled || mbFinished || mpOpenPanel || mpSecurityScopedURL || !mpURL )
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
				{
					[mpOpenPanel setTitle:mpTitle];
					[mpOpenPanel setPrompt:mpTitle];
				}

				[mpOpenPanel setDelegate:self];
				[self setResult:[mpOpenPanel runModal]];
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

- (void)setResult:(NSInteger)nResult
{
	if ( mpOpenPanel && !mbFinished )
	{
		@try
		{
			// When running in the sandbox, native file dialog calls may
			// throw exceptions if the PowerBox daemon process is killed
			if ( nResult == NSModalResponseOK )
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
								// Fix bug reported in the following NeoOffice
								// forum topic by not using the
								// NSURLBookmarkResolutionWithoutUI or
								// NSURLBookmarkResolutionWithoutMounting flags:
								// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=64379#64379
								NSData *pData = [pDirURL bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope includingResourceValuesForKeys:nil relativeToURL:nil error:nil];
								if ( pData )
								{
									BOOL bStale = NO;
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

	if ( mpOpenPanel )
	{
		[mpOpenPanel release];
		mpOpenPanel = nil;
	}

	// Post an event to wakeup the VCL event thread if the VCL
	// event dispatch thread is in a potentially long wait
	Application_postWakeUpEvent();
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
	(void)pObject;

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		mpModalWindow = [pApp modalWindow];
		if ( !mpModalWindow )
		{
			// Eliminate temporary hang on macOS 11 by not requesting ordered
			// windows
			[pApp enumerateWindowsWithOptions:0 usingBlock:^(NSWindow *pWindow, BOOL *bStop) {
				if ( bStop )
					*bStop = NO;

				if ( pWindow && [pWindow isSheet] && [pWindow isVisible] )
				{
					mpModalWindow = pWindow;
					if ( bStop )
						*bStop = YES;
				}
			}];
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

void NSApplication_dispatchPendingEvents( sal_Bool bInNativePrintDrag, sal_Bool bWait )
{
	if ( CFRunLoopGetCurrent() != CFRunLoopGetMain() )
		return;

	// Do not dispatch any native events in a native drag session as it causes
	// the [NSView dragImage:at:offset:event:pasteboard:source:slideBack:]
	// selector to never return
	if ( bInNativePrintDrag )
	{
		BOOL bModalWindow = NO;

		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		// Fix freeze when in a Writer document with lengthy table cells and
		// Writer tries to display the repagination dialog after unchecking the
		// "print selection only" checkbox in the print panel's accessory view
		// by aborting the current native modal window
		NSApplication *pApp = [NSApplication sharedApplication];
		if ( pApp )
		{
			NSWindow *pNSWindow = [NSApp modalWindow];
			if ( pNSWindow )
			{
				bModalWindow = YES;
				[pNSWindow cancelOperation:pNSWindow];
				[pNSWindow orderOut:pNSWindow];
				[pApp abortModal];
			}
		}

		[pPool release];

		// Do not return if we aborted a native modal window as the modal
		// window will not abort until we dispatch some native events
		if ( !bModalWindow )
			return;
	}

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		// Fix excessive CPU usage when this is called from by adding a slight
		// wait if there are no pending events
		NSDate *pDate = [NSDate date];
		if ( pDate && bWait )
			pDate = [NSDate dateWithTimeInterval:0.05f sinceDate:pDate];

		NSEvent *pEvent;
		while ( ( pEvent = [pApp nextEventMatchingMask:NSEventMaskAny untilDate:pDate inMode:( [pApp modalWindow] ? NSModalPanelRunLoopMode : NSDefaultRunLoopMode ) dequeue:YES] ) != nil )
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
		return sal_False;

	JavaSalFrame *pFocusFrame = NULL;

	// Get the active document window
	vcl::Window *pWindow = Application::GetActiveTopWindow();
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
	if ( !Application::IsShutDown() )
	{
		JavaSalEvent *pUserEvent = new JavaSalEvent( SALEVENT_WAKEUP, NULL, NULL );
		JavaSalEventQueue::postCachedEvent( pUserEvent );
		pUserEvent->release();
	}
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
						AcquireSecurityScopedURL( pURL, (BOOL)bMustShowDialogIfNoBookmark, YES, pDialogTitle && [pDialogTitle isKindOfClass:[NSString class]] ? (NSString *)pDialogTitle : nil, pSecurityScopedURLs );
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

void Application_cacheSecurityScopedURLFromOUString( const OUString *pNonSecurityScopedURL )
{
	if ( ImplGetSVData() && ImplGetSVData()->mpDefInst && pNonSecurityScopedURL && pNonSecurityScopedURL->getLength() )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSString *pString = [NSString stringWithCharacters:pNonSecurityScopedURL->getStr() length:pNonSecurityScopedURL->getLength()];
		if ( pString )
			Application_cacheSecurityScopedURL( [NSURL URLWithString:pString] );

		[pPool release];
	}
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
				// Fix bug reported in the following NeoOffice forum topic by
				// not using the NSURLBookmarkResolutionWithoutUI or
				// NSURLBookmarkResolutionWithoutMounting flags:
				// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=64379#64379
				NSData *pData = [pURL bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope includingResourceValuesForKeys:nil relativeToURL:nil error:nil];
				if ( pData )
				{
					BOOL bStale = NO;
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
				if ( pURL && [pURL isKindOfClass:[NSURL class]] )
					[(NSURL *)pURL stopAccessingSecurityScopedResource];
			}
		}

		[pSecurityScopedURLs release];
	}

	[pPool release];
}
