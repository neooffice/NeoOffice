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
 *  Patrick Luby, May 2011
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2011 Planamesa Inc.
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

#import <dlfcn.h>
#import <pwd.h>

#include <com/sun/star/embed/ElementModes.hpp>
#include <osl/file.h>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syswin.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#import <objc/objc-runtime.h>
#include <postmac.h>
#import "topfrm_cocoa.hxx"

#define PDF_BUF_SIZE ( 128 * 1024 )

typedef void Application_cacheSecurityScopedURL_Type( id pURL );

static Application_cacheSecurityScopedURL_Type *pApplication_cacheSecurityScopedURL = NULL;
static NSString *pNoTranslationValue = @" ";

using namespace com::sun::star;

static BOOL HasNativeVersion( vcl::Window *pWindow )
{
	SfxViewFrame *pFrame = SfxViewFrame::GetFirst();
	while ( pFrame )
	{
		SfxViewFrame *pTopViewFrame = pFrame->GetTopViewFrame();
		if ( pTopViewFrame && pTopViewFrame->GetFrame().GetTopWindow_Impl() == pWindow )
		{
			SfxObjectShell *pDoc = pTopViewFrame->GetObjectShell();
			if ( pDoc )
			{
				SfxMedium *pMedium = pDoc->GetMedium();
				if ( pMedium )
				{
					if ( !pMedium->GetBaseURL( true ).getLength() )
						return NO;
				}
			}

			return YES;
		}

		pFrame = SfxViewFrame::GetNext( *pFrame );
	}

	return NO;
}

static BOOL IsValidMoveToURL( NSURL *pURL )
{
	if ( !pURL )
		return NO;

	NSString *pURLString = [pURL absoluteString];
	if ( !pURLString || ![pURLString length] )
		return NO;

	NSURL *pRevisionsURL = [NSURL fileURLWithPath:@"/.DocumentRevisions-V100"];
	if ( pRevisionsURL )
	{
		NSString *pRevisionsURLString = [pRevisionsURL absoluteString];
		if ( pRevisionsURLString && [pRevisionsURLString length] )
		{
			NSRange aRange = [pURLString rangeOfString:pRevisionsURLString];
			if ( !aRange.location && aRange.length )
				return NO;
		}
	}

	NSArray *pCachesFolders = NSSearchPathForDirectoriesInDomains( NSCachesDirectory, NSUserDomainMask, NO );
	NSString *pRealHomeFolder = nil;
	struct passwd *pPasswd = getpwuid( getuid() );
	if ( pPasswd )
		pRealHomeFolder = [NSString stringWithUTF8String:pPasswd->pw_dir];
	if ( pCachesFolders && pRealHomeFolder && [pRealHomeFolder length] )
	{
		NSUInteger nCount = [pCachesFolders count];
		NSUInteger i = 0;
		for ( ; i < nCount; i++ )
		{
			NSString *pFolder = [pCachesFolders objectAtIndex:i];
			if ( pFolder && [pFolder length] )
			{
				pFolder = [[pFolder stringByAppendingPathComponent:@"com.apple.bird"] stringByReplacingOccurrencesOfString:@"~" withString:pRealHomeFolder];
				if ( pFolder && [pFolder length] )
				{
					NSURL *pFolderURL = [NSURL fileURLWithPath:pFolder];
					if ( pFolderURL )
					{
						NSString *pFolderURLString = [pFolderURL absoluteString];
						if ( pFolderURLString && [pFolderURLString length] )
						{
							NSRange aRange = [pURLString rangeOfString:pFolderURLString];
							if ( !aRange.location && aRange.length )
								return NO;
						}
					}
				}
			}
		}
	}

	return YES;
}

static BOOL IsValidMoveToPath( NSString *pPath )
{
	if ( !pPath || ![pPath length] )
		return NO;

	return IsValidMoveToURL( [NSURL fileURLWithPath:pPath] );
}

static const NSString *pWritableTypeEntries[] = {
	#ifdef PRODUCT_NAME
	@PRODUCT_NAME" Chart Document",
	@PRODUCT_NAME" Database Document",
	@PRODUCT_NAME" Document",
	@PRODUCT_NAME" Drawing Document",
	@PRODUCT_NAME" Drawing Template",
	@PRODUCT_NAME" Extension",
	@PRODUCT_NAME" Formula Document",
	@PRODUCT_NAME" HTML Document",
	@PRODUCT_NAME" HTML Template",
	@PRODUCT_NAME" Image Document",
	@PRODUCT_NAME" Image Template",
	@PRODUCT_NAME" Master Document",
	@PRODUCT_NAME" MathML Document",
	@PRODUCT_NAME" Presentation Document",
	@PRODUCT_NAME" Presentation Template",
	@PRODUCT_NAME" Spreadsheet Document",
	@PRODUCT_NAME" Spreadsheet Template",
	@PRODUCT_NAME" Text Document",
	@PRODUCT_NAME" Text Template"
#endif	// PRODUCT_NAME
};

static NSArray *pWritableTypes = nil;
static OUString aRevertToSavedLocalizedString;
static OUString aSaveAVersionLocalizedString;

@class NSDocumentVersion;

@interface NSObject (NSDocumentRevisionsController)
+ (id)sharedController;
- (BOOL)isVisualizing;
@end

@interface NSDocument (SFXDocument)
- (BOOL)_preserveContentsIfNecessaryAfterWriting:(BOOL)bAfter toURL:(NSURL *)pURL forSaveOperation:(NSUInteger)nSaveOperation version:(NSDocumentVersion **)ppVersion error:(NSError **)ppError;
- (void)poseAsMakeWindowControllers;
@end

@interface SFXDocument : NSDocument
{
	SfxViewFrame*			mpFrame;
	BOOL					mbInSetDocumentModified;
	BOOL					mbRelinquished;
	NSLock*					mpRelinquishedLock;
	NSWindowController*		mpWinController;
	NSWindow*				mpWindow;
}
+ (BOOL)autosavesInPlace;
+ (BOOL)isInVersionBrowser;
- (void)browseDocumentVersions:(id)pObject;
- (void)close;
- (void)dealloc;
- (NSDocument *)duplicateAndReturnError:(NSError **)ppError;
- (void)duplicateDocument:(id)pObject;
- (void)duplicateDocumentAndWaitForRevertCall:(BOOL)bWait;
- (BOOL)hasUnautosavedChanges;
- (id)initWithContentsOfURL:(NSURL *)pURL frame:(SfxViewFrame *)pFrame window:(NSWindow *)pWindow ofType:(NSString *)pTypeName error:(NSError **)ppError;
- (BOOL)isRelinquished;
- (void)moveToURL:(NSURL *)pURL completionHandler:(void (^)(NSError *))aCompletionHandler;
- (BOOL)readFromURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)ppError;
- (void)relinquishPresentedItem:(BOOL)bWriter reacquirer:(void (^)(void (^reacquirer)(void)))aReacquirer;
- (void)relinquishPresentedItemToReader:(void (^)(void (^reacquirer)(void)))aReader;
- (void)relinquishPresentedItemToWriter:(void (^)(void (^reacquirer)(void)))aWriter;
- (void)reloadFrame:(NSNumber *)pSilent;
- (void)restoreStateWithCoder:(NSCoder *)pCoder;
- (void)revertDocumentToSaved:(id)pObject;
- (BOOL)revertToContentsOfURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)ppError;
- (void)setDocumentModified:(BOOL)bModified;
- (BOOL)setRelinquished:(BOOL)bRelinquished;
- (void)updateChangeCount:(NSDocumentChangeType)nChangeType;
- (NSArray *)writableTypesForSaveOperation:(NSSaveOperationType)nSaveOperation;
- (BOOL)writeToURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)ppError;
@end

@interface SFXUndoManager : NSUndoManager
{
	SFXDocument*			mpDoc;
}
+ (id)createWithDocument:(SFXDocument *)pDoc;
- (void)dealloc;
- (id)initWithDocument:(SFXDocument *)pDoc;
- (void)undo;
@end

static NSMutableDictionary *pFrameDict = nil;
static ::osl::Mutex aFrameDictMutex;

static OUString NSStringToOUString( NSString *pString )
{
	OUString aRet;

	if ( pString )
	{
		NSUInteger nLen = [pString length];
		if ( nLen )
		{
			sal_Unicode aBuf[ nLen + 1 ];
			[pString getCharacters:aBuf];
			aBuf[ nLen ] = 0;
			aRet = OUString( aBuf );
		}
	}

	return aRet;
}

static SFXDocument *GetDocumentForFrame( SfxViewFrame *pFrame )
{
	SFXDocument *pRet = nil;

	osl::MutexGuard aGuard( aFrameDictMutex );

	if ( pFrame && pFrameDict )
	{
		NSNumber *pKey = [NSNumber numberWithUnsignedLong:(unsigned long)pFrame];
		if ( pKey )
			pRet = [pFrameDict objectForKey:pKey];
	}

	return pRet;
}

static void SetDocumentForFrame( SfxViewFrame *pFrame, SFXDocument *pDoc )
{
	if ( !pFrame )
		return;

	osl::MutexGuard aGuard( aFrameDictMutex );

	if ( !pFrameDict )
	{
		pFrameDict = [NSMutableDictionary dictionaryWithCapacity:10];
		if ( pFrameDict )
			[pFrameDict retain];
	}

	if ( pFrameDict )
	{
		NSNumber *pKey = [NSNumber numberWithUnsignedLong:(unsigned long)pFrame];
		if ( pKey )
		{
			// If we are replacing an existing document, close the old document
			// otherwise it will never get released by its controllers
			SFXDocument *pOldDoc = [pFrameDict objectForKey:pKey];
			if ( pOldDoc )
				[pOldDoc close];

			if ( pDoc )
				[pFrameDict setObject:pDoc forKey:pKey];
			else
				[pFrameDict removeObjectForKey:pKey];
		}
	}
}

#include <premac.h>
#import <Quartz/Quartz.h>
#include <postmac.h>

@interface SFXQLPreviewItem : NSObject <QLPreviewItem>
{
	NSURL*					mpURL;
}
@property(readonly) NSURL *previewItemURL;
- (void)dealloc;
- (id)initWithURL:(NSURL *)pURL;
@end

@implementation SFXQLPreviewItem

@dynamic previewItemURL;

- (void)dealloc
{
	if ( mpURL )
		[mpURL release];
	
	[super dealloc];
}

- (id)initWithURL:(NSURL *)pURL
{
	[super init];

	mpURL = pURL;
	if ( mpURL )
		[mpURL retain];

	return self;
}

- (NSURL *)previewItemURL
{
	return mpURL;
}

@end

static NSRect aLastVersionBrowserDocumentFrame = NSZeroRect;

@implementation SFXDocument

+ (BOOL)autosavesInPlace
{
	return NSDocument_versionsSupported();
}

+ (BOOL)isInVersionBrowser
{
	NSBundle *pBundle = [NSBundle bundleForClass:[NSDocument class]];
	if ( pBundle )
	{
		Class aClass = [pBundle classNamed:@"NSDocumentRevisionsController"];
		if ( aClass && class_getClassMethod( aClass, @selector(sharedController) ) )
		{
			id pController = [aClass sharedController];
			if ( pController && [pController respondsToSelector:@selector(isVisualizing)] )
				return [pController isVisualizing];
		}
	}

	return NO;
}

- (void)browseDocumentVersions:(id)pObject
{
	aLastVersionBrowserDocumentFrame = ( mpWindow ? [NSWindow contentRectForFrameRect:[mpWindow frame] styleMask:[mpWindow styleMask]] : NSZeroRect );

	[super browseDocumentVersions:pObject];
}

- (void)close
{
	// Fix unexpected closing of the old document's window when dragging an
	// existing spreadsheet's icon from the titlebar into the spreadsheet's
	// content area by removing all window controllers before closing the old
	// document
	NSArray *pWinControllers = [self windowControllers];
	while ( pWinControllers && [pWinControllers count] )
	{
		NSWindowController *pWinController = (NSWindowController *)[pWinControllers objectAtIndex:0];
		if ( pWinController )
			[self removeWindowController:pWinController];

		pWinControllers = [self windowControllers];
	}

	// Fix short hang when closing a document when compiled on macOS 11 by
	// removing the document immediately before closing it
	NSDocumentController *pDocController = [NSDocumentController sharedDocumentController];
	if ( pDocController )
		[pDocController removeDocument:self];

	[super close];

	// Set undo manager to nil otherwise the document will never be released
	[self setUndoManager:nil];
}

- (void)dealloc
{
	// Release our custom undo manager
	[self setUndoManager:nil];

	if ( mpRelinquishedLock )
		[mpRelinquishedLock release];

	if ( mpWinController )
	{
		[self removeWindowController:mpWinController];
		[mpWinController release];
	}

	if ( mpWindow )
		[mpWindow release];

	NSDocumentController *pDocController = [NSDocumentController sharedDocumentController];
	if ( pDocController )
		[pDocController removeDocument:self];

	[super dealloc];
}

- (NSDocument *)duplicateAndReturnError:(NSError **)ppError
{
	if ( ppError )
		*ppError = nil;

	// This selector gets invoked by the duplicate button in the "this document
	// is locked" alert that appears when you change content in a locked
	// document so wait for a revert call
	[self duplicateDocumentAndWaitForRevertCall:YES];

	return self;
}

- (void)duplicateDocument:(id)pObject
{
	(void)pObject;

	// This selector gets invoked by the duplicate menu item in the titlebar's
	// menu so don't wait for a revert call and execute immediately
	[self duplicateDocumentAndWaitForRevertCall:NO];
}

- (void)duplicateDocumentAndWaitForRevertCall:(BOOL)bWait
{
	if ( NSDocument_versionsSupported() && !Application::IsShutDown() )
	{
		comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			SFXDocument *pDoc = GetDocumentForFrame( mpFrame );
			if ( pDoc == self )
				SFXDocument_duplicate( mpFrame, bWait, NO );
		}
		rSolarMutex.release();
	}
}

- (BOOL)hasUnautosavedChanges
{
	// Don't allow NSDocument to do the autosaving
	return NO;
}

- (id)initWithContentsOfURL:(NSURL *)pURL frame:(SfxViewFrame *)pFrame window:(NSWindow *)pWindow ofType:(NSString *)pTypeName error:(NSError **)ppError
{
	[super initWithContentsOfURL:pURL ofType:pTypeName error:ppError];

	mpFrame = pFrame;
	mbInSetDocumentModified = NO;
	mbRelinquished = NO;
	mpRelinquishedLock = [[NSLock alloc] init];
	mpWinController = nil;
	mpWindow = pWindow;
	if ( mpWindow )
	{
		[mpWindow retain];

		NSDocumentController *pDocController = [NSDocumentController sharedDocumentController];
		if ( pDocController )
		{
			mpWinController = [[NSWindowController alloc] initWithWindow:mpWindow];
			if ( mpWinController )
			{
				[self addWindowController:mpWinController];

				// Adding to document controller is slow when compiled on
				// macOS 11 so don't block document loading in the LibO code
				[pDocController performSelector:@selector(addDocument:) withObject:self afterDelay:0];
			}
		}
	}

	// Set our own custom undo manager
	[self setUndoManager:[SFXUndoManager createWithDocument:self]];

	return self;
}

- (BOOL)isRelinquished
{
	BOOL bRet = NO;

	if ( mpRelinquishedLock )
		[mpRelinquishedLock lock];

	bRet = mbRelinquished;

	if ( mpRelinquishedLock )
		[mpRelinquishedLock unlock];

	return bRet;
}

- (void)moveToURL:(NSURL *)pURL completionHandler:(void (^)(NSError *))aCompletionHandler
{
	// Fix bug reported in the following NeoOffice forum by detecting when the
	// user has moved or renamed the file:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8619
	[super moveToURL:pURL completionHandler:^(NSError *pError) {
		if ( aCompletionHandler )
			aCompletionHandler( pError );

		if ( !pError && pURL )
		{
			if ( !pApplication_cacheSecurityScopedURL )
				pApplication_cacheSecurityScopedURL = (Application_cacheSecurityScopedURL_Type *)dlsym( RTLD_DEFAULT, "Application_cacheSecurityScopedURL" );
			if ( pApplication_cacheSecurityScopedURL )
				pApplication_cacheSecurityScopedURL( pURL );

			if ( ![self isRelinquished] )
			{
				comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
				rSolarMutex.acquire();
				if ( !Application::IsShutDown() )
				{
					SFXDocument *pDoc = GetDocumentForFrame( mpFrame );
					if ( pDoc == self )
						SFXDocument_documentHasMoved( mpFrame, NSStringToOUString( [pURL absoluteString] ) );
				}
				rSolarMutex.release();
			}
		}
	}];
}

- (BOOL)readFromURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)ppError
{
	(void)pURL;
	(void)pTypeName;

	if ( ppError )
		*ppError = nil;

	return YES;
}

- (void)relinquishPresentedItem:(BOOL)bWriter reacquirer:(void (^)(void (^reacquirer)(void)))aReacquirer
{
	if ( !aReacquirer )
		return;

	// Attempt to fix the deadlock reported in the
	// testing/elcapitanbugs_emails/20160913 e-mail by always executing this
	// selector on the main thread
	if ( CFRunLoopGetCurrent() != CFRunLoopGetMain() )
	{
		[self continueAsynchronousWorkOnMainThreadUsingBlock:^() {
			[self relinquishPresentedItem:bWriter reacquirer:aReacquirer];
		}];
		return;
	}

	BOOL bOldIsRelinquished = [self setRelinquished:YES];

	SfxObjectShell *pObjSh = NULL;
	sal_Bool bOldEnableSetModified = sal_False;
	NSDate *pModDate = nil;
	id pFileID = nil;
	NSURL *pURL = [self fileURL];

	comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
	rSolarMutex.acquire();

	if ( !Application::IsShutDown() )
	{
		SFXDocument *pDoc = GetDocumentForFrame( mpFrame );
		if ( pDoc == self )
		{
			pObjSh = mpFrame->GetObjectShell();
			while ( pObjSh && !pObjSh->IsLoadingFinished() )
			{
				sal_uLong nCount = Application::ReleaseSolarMutex();
				osl::Thread::yield();
				Application::AcquireSolarMutex( nCount );

				pDoc = GetDocumentForFrame( mpFrame );
				if ( pDoc == self )
					pObjSh = mpFrame->GetObjectShell();
				else
					pObjSh = NULL;
			}

			// Block editing and saving
			if ( pObjSh && !Application::IsShutDown() )
			{
				bOldEnableSetModified = pObjSh->IsEnableSetModified();
				pObjSh->EnableSetModified( sal_False );
			}
		}
	}

	pURL = [self fileURL];
	if ( pURL )
	{
		pURL = [pURL URLByStandardizingPath];
		if ( pURL )
		{
			pURL = [pURL URLByResolvingSymlinksInPath];
			if ( pURL )
			{
				if ( [pURL checkResourceIsReachableAndReturnError:nil] )
				{
					[pURL getResourceValue:&pModDate forKey:NSURLContentModificationDateKey error:nil];
					[pURL getResourceValue:&pFileID forKey:NSURLFileResourceIdentifierKey error:nil];
				}

				if ( bWriter )
				{
					NSString *pURLPath = [pURL path];
					if ( pURLPath )
						osl_setLockedFilesLock( [pURLPath UTF8String], sal_False );
				}
			}
		}
	}

	rSolarMutex.release();

	aReacquirer(^{
		comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();

		sal_Bool bRelocked = !bWriter;
		NSDate *pNewModDate = nil;
		id pNewFileID = nil;
		NSURL *pNewURL = [self fileURL];
		if ( pNewURL )
		{
			pNewURL = [pNewURL URLByStandardizingPath];
			if ( pNewURL )
			{
				pNewURL = [pNewURL URLByResolvingSymlinksInPath];
				if ( pNewURL )
				{
					NSString *pNewURLPath = [pNewURL path];
					if ( pNewURLPath )
						bRelocked = osl_setLockedFilesLock( [pNewURLPath UTF8String], sal_True );

					if ( [pNewURL checkResourceIsReachableAndReturnError:nil] )
					{
						[pNewURL getResourceValue:&pNewModDate forKey:NSURLContentModificationDateKey error:nil];
						[pNewURL getResourceValue:&pNewFileID forKey:NSURLFileResourceIdentifierKey error:nil];
					}
				}
			}
		}

		if ( !Application::IsShutDown() )
		{
			SFXDocument *pNewDoc = GetDocumentForFrame( mpFrame );
			if ( pNewDoc == self )
			{
				SfxObjectShell *pNewObjSh = mpFrame->GetObjectShell();
				while ( pNewObjSh && !pNewObjSh->IsLoadingFinished() )
				{
					sal_uLong nCount = Application::ReleaseSolarMutex();
					osl::Thread::yield();
					Application::AcquireSolarMutex( nCount );

					pNewDoc = GetDocumentForFrame( mpFrame );
					if ( pNewDoc == self )
						pNewObjSh = mpFrame->GetObjectShell();
					else
						pNewObjSh = NULL;
				}

				if ( pNewObjSh && !Application::IsShutDown() )
				{
					pNewObjSh->EnableSetModified( bOldEnableSetModified);

					BOOL bDeleted = NO;
					BOOL bMoved = NO;
					BOOL bChanged = NO;

					if ( !pNewURL || !pNewModDate || !pNewFileID )
					{
						bDeleted = YES;
					}
					else if ( !pURL || !pModDate || !pFileID )
					{
						bChanged = YES;
						bMoved = YES;
					}
					else if ( pURL && pNewURL)
					{
						if ( [pURL isEqual:pNewURL] && pModDate && pNewModDate && ![pModDate isEqual:pNewModDate] )
							bChanged = YES;
						else if ( ![pURL isEqual:pNewURL] && pFileID && pNewFileID && [pFileID isEqual:pNewFileID] )
							bMoved = YES;
					}

					if ( !bDeleted && pNewURL && ( !pURL || ![pURL isEqual:pNewURL] ) && !IsValidMoveToURL( pNewURL ) )
						bDeleted = YES;

					if ( bDeleted )
					{
						SFXDocument_documentHasBeenDeleted( mpFrame );
					}
					else
					{
						if ( bMoved && pNewURL )
						{
							if ( !pApplication_cacheSecurityScopedURL )
								pApplication_cacheSecurityScopedURL = (Application_cacheSecurityScopedURL_Type *)dlsym( RTLD_DEFAULT, "Application_cacheSecurityScopedURL" );
							if ( pApplication_cacheSecurityScopedURL )
								pApplication_cacheSecurityScopedURL( pNewURL );
							SFXDocument_documentHasMoved( mpFrame, NSStringToOUString( [pNewURL absoluteString] ) );
						}

						if ( bChanged || !bRelocked )
							[self reloadFrame:[NSNumber numberWithBool:NO]];
					}
				}
			}
		}

		rSolarMutex.release();

		[self setRelinquished:bOldIsRelinquished];
	});
}

- (void)relinquishPresentedItemToReader:(void (^)(void (^reacquirer)(void)))aReader
{
	[self relinquishPresentedItem:NO reacquirer:aReader];
}

- (void)relinquishPresentedItemToWriter:(void (^)(void (^reacquirer)(void)))aWriter
{
	[self relinquishPresentedItem:YES reacquirer:aWriter];
}

- (void)reloadFrame:(NSNumber *)pSilent
{
	if ( NSDocument_versionsSupported() && !Application::IsShutDown() )
	{
		if ( [SFXDocument isInVersionBrowser] )
		{
			[self performSelector:@selector(reloadFrame:) withObject:pSilent afterDelay:0];
		}
		else
		{
			comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
			rSolarMutex.acquire();
			// Fix Mac App Store crash by not reloading if SfxGetpApp() returns
			// NULL as that means we are already in DeInitVCL()
			if ( !Application::IsShutDown() && SfxGetpApp() )
			{
				SFXDocument *pDoc = GetDocumentForFrame( mpFrame );
				if ( pDoc == self )
				{
					sal_Bool bSilent = sal_True;
					if ( pSilent && ![pSilent boolValue] )
						bSilent = sal_False;
					SFXDocument_reload( mpFrame, bSilent );
				}
			}
			rSolarMutex.release();
		}
	}
}

- (void)restoreStateWithCoder:(NSCoder *)pCoder
{
	(void)pCoder;

	// Don't allow NSDocument to do the restoration
}

- (void)revertDocumentToSaved:(id)pObject
{
	(void)pObject;

	if ( [SFXDocument isInVersionBrowser] )
		return;

	[self browseDocumentVersions:pObject];
}

- (BOOL)revertToContentsOfURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)ppError
{
	(void)pURL;
	(void)pTypeName;

	if ( ppError )
		*ppError = nil;

	[self reloadFrame:nil];

	return YES;
}

- (void)setDocumentModified:(BOOL)bModified
{
	if ( mbInSetDocumentModified )
		return;

	mbInSetDocumentModified = YES;

	// Remove any modified state set on the window before it was attached to
	// this document
	if ( mpWindow )
		[mpWindow setDocumentEdited:NO];

	if ( bModified )
	{
		if ( [self checkAutosavingSafetyAndReturnError:nil] )
			[self updateChangeCount:NSChangeDone];
	}
	else
	{
		[self updateChangeCount:NSChangeCleared];
	}

	mbInSetDocumentModified = NO;
}

- (BOOL)setRelinquished:(BOOL)bRelinquished
{
	BOOL bRet = NO;

	if ( mpRelinquishedLock )
		[mpRelinquishedLock lock];

	bRet = mbRelinquished;
	mbRelinquished = bRelinquished;

	if ( mpRelinquishedLock )
		[mpRelinquishedLock unlock];

	return bRet;
}

- (void)updateChangeCount:(NSDocumentChangeType)nChangeType
{
	BOOL bIsEdited = [self isDocumentEdited];

	[super updateChangeCount:nChangeType];

	if ( !mbInSetDocumentModified && nChangeType == NSChangeDone && !bIsEdited && [self isDocumentEdited] )
	{
		comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			SFXDocument *pDoc = GetDocumentForFrame( mpFrame );
			if ( pDoc == self )
				SFXDocument_documentHasBeenModified( mpFrame );
		}
		rSolarMutex.release();
	}
}

- (NSArray *)writableTypesForSaveOperation:(NSSaveOperationType)nSaveOperation
{
	(void)nSaveOperation;

	if ( !pWritableTypes )
	{
		unsigned int nCount = sizeof( pWritableTypeEntries ) / sizeof( NSString* );
		if ( nCount )
			pWritableTypes = [NSArray arrayWithObjects:pWritableTypeEntries count:nCount];
		else
			pWritableTypes = [NSArray array];

		if ( pWritableTypes )
			[pWritableTypes retain];
	}

	return pWritableTypes;
}

- (BOOL)writeToURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)ppError
{
	(void)pURL;
	(void)pTypeName;

	if ( ppError )
		*ppError = nil;

	return NO;
}

@end

@interface PDFView (SFXPDFView)
- (void)setAllowsDragging:(BOOL)bFlag;
@end

@interface SFXDocumentRevision : SFXDocument
{
	PDFView*				mpPDFView;
	QLPreviewView*			mpQLPreviewView;
}
- (void)dealloc;
- (void)destroy;
- (id)init;
- (void)makeWindowControllers;
@end

@implementation SFXDocumentRevision

- (void)dealloc
{
	[self destroy];

	[super dealloc];
}

- (void)destroy
{
	if ( mpPDFView )
	{
		[mpPDFView removeFromSuperview];
		// Stop crashing when exiting the versions browser on macOS 10.12 by
		// setting the document to nil before releasing
		[mpPDFView setDocument:nil];
		[mpPDFView release];
	}

	if ( mpQLPreviewView )
	{
		[mpQLPreviewView removeFromSuperview];
		// Stop crashing when exiting the versions browser on macOS 10.12 by
		// setting the preview item to nil before releasing
		mpQLPreviewView.previewItem = nil;
		[mpQLPreviewView release];
	}
}

- (id)init
{
	[super init];

	mpPDFView = nil;
	mpQLPreviewView = nil;

	return self;
}

- (void)makeWindowControllers
{
	[self destroy];

	[super makeWindowControllers];

	NSMutableData *pPDFData = nil;
	NSURL *pFileURL = [self fileURL];
	if ( pFileURL )
	{
		if ( !pApplication_cacheSecurityScopedURL )
			pApplication_cacheSecurityScopedURL = (Application_cacheSecurityScopedURL_Type *)dlsym( RTLD_DEFAULT, "Application_cacheSecurityScopedURL" );
		if ( pApplication_cacheSecurityScopedURL )
			pApplication_cacheSecurityScopedURL( pFileURL );

		OUString aFileURL( NSStringToOUString( [pFileURL absoluteString] ) );
		if ( aFileURL.getLength() )
		{
			SfxMedium aMedium( aFileURL, STREAM_STD_READ );
			uno::Reference< io::XInputStream > xPDFInputStream;
		try
			{
				uno::Reference< embed::XStorage > xStorage( aMedium.GetStorage() );
				if ( xStorage.is() )
				{
					uno::Reference< embed::XStorage > xThumbnails = xStorage->openStorageElement( "Thumbnails", embed::ElementModes::READ );
					if ( xThumbnails.is() )
					{
						uno::Reference< io::XStream > xPDFStream = xThumbnails->openStreamElement( "thumbnail.pdf", embed::ElementModes::READ );
						if ( xPDFStream.is() )
						{
							uno::Reference< io::XInputStream > xPDFInputStream = xPDFStream->getInputStream();
							if ( xPDFInputStream.is() )
							{
								pPDFData = [NSMutableData dataWithCapacity:PDF_BUF_SIZE];
								if ( pPDFData )
								{
									static const sal_uInt32 nBytes = 4096;
									sal_Int32 nBytesRead;
									uno::Sequence< ::sal_Int8 > aBytes( nBytes );
									while ( ( nBytesRead = xPDFInputStream->readBytes( aBytes, nBytes ) ) > 0 )
										[pPDFData appendBytes:aBytes.getConstArray() length:nBytesRead];
								}
							}
						}
					}
				}
			}
			catch ( ... )
			{
			}

			if ( xPDFInputStream.is() )
			{
				try
				{
					xPDFInputStream->closeInput();
				}
				catch ( ... )
				{
				}
			}

			aMedium.CloseAndRelease();
		}
	}

	NSUInteger nStyleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable;
	if ( NSIsEmptyRect( aLastVersionBrowserDocumentFrame ) )
	{
		NSApplication *pApp = [NSApplication sharedApplication];
		NSDocumentController *pDocController = [NSDocumentController sharedDocumentController];
		if ( pApp && pDocController )
		{
			NSArray *pWindows = [pApp windows];
			if ( pWindows )
			{
				NSDocument *pDoc = [pDocController currentDocument];
				NSUInteger nCount = [pWindows count];
				NSUInteger i = 0;
				for ( ; i < nCount; i++ )
				{
					NSWindow *pWindow = [pWindows objectAtIndex:i];
					if ( pWindow )
					{
						NSRect aContentRect = [NSWindow contentRectForFrameRect:[pWindow frame] styleMask:[pWindow styleMask]];
						if ( !NSIsEmptyRect( aContentRect ) )
						{
							aLastVersionBrowserDocumentFrame = aContentRect;
							if ( pDoc && [pDocController documentForWindow:pWindow] == pDoc )
								break;
						}
					}
				}
			}
		}
	}

	NSWindow *pWindow = [[NSWindow alloc] initWithContentRect:aLastVersionBrowserDocumentFrame styleMask:nStyleMask backing:NSBackingStoreBuffered defer:YES];
	if ( pWindow )
	{
		[pWindow autorelease];

		NSWindowController *pWinController = [[NSWindowController alloc] initWithWindow:pWindow];
		if ( pWinController )
		{
			[pWinController autorelease];

			[self addWindowController:pWinController];

			if ( pPDFData )
			{
				PDFDocument *pPDFDoc = [[PDFDocument alloc] initWithData:pPDFData];
				if ( pPDFDoc )
				{
					[pPDFDoc autorelease];

					mpPDFView = [[PDFView alloc] initWithFrame:[[pWindow contentView] frame]];
					if ( mpPDFView )
					{
						[mpPDFView setDocument:pPDFDoc];
						[mpPDFView setAcceptsDraggedFiles:NO];
						if ( [mpPDFView respondsToSelector:@selector(setAllowsDragging:)] )
							[mpPDFView setAllowsDragging:NO];
						[mpPDFView setAutoScales:YES];
						[mpPDFView setDisplaysPageBreaks:NO];
						[pWindow setContentView:mpPDFView];
					}
				}
			}

			if ( !mpPDFView && pFileURL )
			{
				SFXQLPreviewItem *pQLItem = [[SFXQLPreviewItem alloc] initWithURL:pFileURL];
				if ( pQLItem )
				{
					[pQLItem autorelease];

					NSRect aQLFrame = [[pWindow contentView] frame];
					aQLFrame.origin.x = 0;
					aQLFrame.origin.y = 0;
					mpQLPreviewView = [[QLPreviewView alloc] initWithFrame:aQLFrame];
					if ( mpQLPreviewView )
					{
						[mpQLPreviewView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
						[[pWindow contentView] setAutoresizesSubviews:YES];
						[[pWindow contentView] addSubview:mpQLPreviewView];
						mpQLPreviewView.previewItem = pQLItem;
					}
				}
			}
		}
	}
}

@end

@implementation SFXUndoManager

+ (id)createWithDocument:(SFXDocument *)pDoc
{
	SFXUndoManager *pRet = [[SFXUndoManager alloc] initWithDocument:pDoc];
	[pRet autorelease];
	return pRet;
}

- (void)dealloc
{
	if ( mpDoc )
		[mpDoc release];

	[super dealloc];
}

- (id)initWithDocument:(SFXDocument *)pDoc
{
	[super init];

	mpDoc = pDoc;
	if ( mpDoc )
		[mpDoc retain];

	return self;
}

- (void)undo
{
	[super undo];

	// This selector is called when the document is locked and the user cancels
	// unlocking so revert any changes
	if ( mpDoc )
		[mpDoc reloadFrame:nil];
}

@end

@interface NSBundle (RunSFXDocument)
+ (NSBundle *)bundleWithURL:(NSURL *)pURL;
@end

@interface RunSFXDocument : NSObject
{
	SFXDocument*			mpDoc;
	SfxViewFrame*			mpFrame;
	BOOL					mbReadOnly;
	NSString*				mpRevertToSavedLocalizedString;
	BOOL					mbSaved;
	NSString*				mpTitle;
	NSURL*					mpURL;
	NSView*					mpView;
}
+ (id)create;
+ (id)createWithFrame:(SfxViewFrame *)pFrame;
+ (id)createWithFrame:(SfxViewFrame *)pFrame view:(NSView *)pView URL:(NSURL *)pURL readOnly:(BOOL)bReadOnly;
- (void)createDocument:(id)pObject;
- (void)dealloc;
- (SFXDocument *)document;
- (BOOL)documentIsRelinquished;
- (void)getDocument:(id)pObject;
- (id)initWithFrame:(SfxViewFrame *)pFrame view:(NSView *)pView URL:(NSURL *)pURL readOnly:(BOOL)bReadOnly;
- (void)revertDocumentToSaved:(id)pObject;
- (NSString *)revertToSavedLocalizedString;
- (void)saveVersionOfDocument:(id)pObject;
- (void)setDocumentModified:(id)pObject;
- (NSString *)title;
@end

@implementation RunSFXDocument

+ (id)create
{
	RunSFXDocument *pRet = [[RunSFXDocument alloc] initWithFrame:nil view:nil URL:nil readOnly:YES];
	[pRet autorelease];
	return pRet;
}

+ (id)createWithFrame:(SfxViewFrame *)pFrame
{
	RunSFXDocument *pRet = [[RunSFXDocument alloc] initWithFrame:pFrame view:nil URL:nil readOnly:YES];
	[pRet autorelease];
	return pRet;
}

+ (id)createWithFrame:(SfxViewFrame *)pFrame view:(NSView *)pView URL:(NSURL *)pURL readOnly:(BOOL)bReadOnly
{
	RunSFXDocument *pRet = [[RunSFXDocument alloc] initWithFrame:pFrame view:pView URL:pURL readOnly:bReadOnly];
	[pRet autorelease];
	return pRet;
}

- (void)createDocument:(id)pObject
{
	(void)pObject;

	if ( mpFrame && mpView )
	{
		NSWindow *pWindow = [mpView window];
		if ( pWindow )
		{
			if ( mbReadOnly || !NSDocument_versionsSupported() )
			{
				[pWindow setRepresentedURL:mpURL];
			}
			else if ( [pWindow isVisible] )
			{
				SFXDocument *pOldDoc = GetDocumentForFrame( mpFrame );
				if ( pOldDoc )
				{
					// If the URL has changed, disconnect the document from
					// the old URL's version history
					NSURL *pOldURL = [pOldDoc fileURL];
					if ( !mpURL || !pOldURL || ![pOldURL isEqual:mpURL] )
					{
						[pOldDoc setFileURL:mpURL];

 						// Fix bug reported in the following NeoOffice forum
						// post by preserving the edited status of the document:
						// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=64685#64685
						BOOL bIsEdited = [pOldDoc isDocumentEdited];
						[pOldDoc updateChangeCount:NSChangeCleared];
						if ( mpDoc )
							[self setDocumentModified:[NSNumber numberWithBool:bIsEdited]];
					}
				}
				else if ( mpURL )
				{
					NSError *pError = nil;
					SFXDocument *pDoc = [[SFXDocument alloc] initWithContentsOfURL:mpURL frame:mpFrame window:pWindow ofType:@"" error:&pError];
					SetDocumentForFrame( mpFrame, pDoc );
					// The document will be retained by SetDocumentForFrame()
					[pDoc release];
				}
			}
		}
	}
}

- (void)dealloc
{
	if ( mpDoc )
		[mpDoc release];
	if ( mpRevertToSavedLocalizedString )
		[mpRevertToSavedLocalizedString release];
	if ( mpTitle )
		[mpTitle release];
	if ( mpURL )
		[mpURL release];
	if ( mpView )
		[mpView release];

	[super dealloc];
}

- (SFXDocument *)document
{
	return mpDoc;
}

- (BOOL)documentIsRelinquished
{
	if ( mpDoc )
		return [mpDoc isRelinquished];
	else
		return NO;
}

- (void)getDocument:(id)pObject
{
	(void)pObject;

	if ( !mpDoc )
	{
		mpDoc = GetDocumentForFrame( mpFrame );
		if ( mpDoc )
			[mpDoc retain];
	}
}

- (id)initWithFrame:(SfxViewFrame *)pFrame view:(NSView *)pView URL:(NSURL *)pURL readOnly:(BOOL)bReadOnly
{
	[super init];

	mpDoc = nil;
	mpFrame = pFrame;
	mbReadOnly = bReadOnly;
	mpRevertToSavedLocalizedString = nil;
	mbSaved = NO;
	mpTitle = nil;
	mpURL = pURL;
	if ( mpURL )
		[mpURL retain];
	mpView = pView;
	if ( mpView )
		[mpView retain];

	return self;
}

- (void)release:(id)pObject
{
	(void)pObject;

	SetDocumentForFrame( mpFrame, nil );
}

- (void)revertDocumentToSaved:(id)pObject
{
	(void)pObject;

	SFXDocument *pDoc = GetDocumentForFrame( mpFrame );
	if ( pDoc )
		[pDoc revertDocumentToSaved:self];
}

- (NSString *)revertToSavedLocalizedString
{
	return mpRevertToSavedLocalizedString;
}

- (void)saveVersionOfDocument:(id)pObject
{
	(void)pObject;

	SFXDocument *pDoc = GetDocumentForFrame( mpFrame );

	// Fix crashing bug reported in the following NeoOffice forum post by
	// checking for nil file URLs:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=64664#64664
	if ( pDoc && [pDoc respondsToSelector:@selector(_preserveContentsIfNecessaryAfterWriting:toURL:forSaveOperation:version:error:)] && [pDoc fileURL] )
	{
		NSDocumentVersion *pNewVersion = nil;
		NSError *pError = nil;
		mbSaved = [pDoc _preserveContentsIfNecessaryAfterWriting:YES toURL:[pDoc fileURL] forSaveOperation:NSSaveOperation version:&pNewVersion error:&pError];
	}
}

- (void)setDocumentModified:(id)pObject
{
	[self getDocument:pObject];
	if ( mpDoc )
	{
		if ( pObject && [pObject isKindOfClass:[NSNumber class]] && [(NSNumber *)pObject boolValue] )
			[mpDoc setDocumentModified:YES];
		else
			[mpDoc setDocumentModified:NO];
	}
}

- (NSString *)title
{
	return mpTitle;
}

@end

OUString *NSDocument_revertToSavedLocalizedString( vcl::Window *pWindow )
{
	if ( !pWindow || !NSDocument_versionsEnabled() || !HasNativeVersion( pWindow ) )
	{
		aRevertToSavedLocalizedString = "";
	}
	else if ( !aRevertToSavedLocalizedString.getLength() )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSString *pKey = @"Browse All Versions\\U2026";
		NSString *pTable = @"Document";
		NSString *pLocalizedString = nil;
		NSBundle *pBundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/AppKit.framework"];
		if ( pBundle )
		{
			pLocalizedString = [pBundle localizedStringForKey:pKey value:pNoTranslationValue table:pTable];
			if ( pLocalizedString && [pLocalizedString length] && ![pLocalizedString isEqualToString:pNoTranslationValue] )
				aRevertToSavedLocalizedString = NSStringToOUString( pLocalizedString );
		}

		[pPool release];
	}

	return &aRevertToSavedLocalizedString;
}

OUString *NSDocument_saveAVersionLocalizedString( vcl::Window *pWindow )
{
	if ( !pWindow || !NSDocument_versionsEnabled() || !HasNativeVersion( pWindow ) )
	{
		aSaveAVersionLocalizedString = "";
	}
	else if ( !aSaveAVersionLocalizedString.getLength() )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSString *pKey = @"Save a Version";
		NSString *pAltKey = @"Save";
		NSString *pTable = @"Document";
		NSString *pLocalizedString = nil;
		NSBundle *pBundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/AppKit.framework"];
		if ( pBundle )
		{
			pLocalizedString = [pBundle localizedStringForKey:pKey value:pNoTranslationValue table:pTable];
			if ( pLocalizedString && [pLocalizedString length] && ![pLocalizedString isEqualToString:pNoTranslationValue] )
			{
				aSaveAVersionLocalizedString = NSStringToOUString( pLocalizedString );
			}
			else
			{
				pLocalizedString = [pBundle localizedStringForKey:pAltKey value:pNoTranslationValue table:pTable];
				if ( pLocalizedString && [pLocalizedString length] && ![pLocalizedString isEqualToString:pNoTranslationValue] )
					aSaveAVersionLocalizedString = NSStringToOUString( pLocalizedString );
			}
		}

		[pPool release];
	}

	return &aSaveAVersionLocalizedString;
}

sal_Bool NSDocument_isValidMoveToPath( OUString aPath )
{
	if ( !aPath.getLength() )
		return sal_False;
	else if ( !NSDocument_versionsSupported() )
		return sal_True;

	sal_Bool bRet = sal_True;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSString *pPath = [NSString stringWithCharacters:aPath.getStr() length:aPath.getLength()];
	if ( pPath )
		bRet = IsValidMoveToPath( pPath );

	[pPool release];

	return bRet;
}

sal_Bool NSDocument_versionsEnabled()
{
	sal_Bool bRet = NSDocument_versionsSupported();

	// Check if user has explicitly disabled versions
	if ( bRet )
	{
		CFPropertyListRef aPref = CFPreferencesCopyAppValue( CFSTR( "DisableVersions" ), kCFPreferencesCurrentApplication );
		if ( aPref )
		{
			if ( CFGetTypeID( aPref ) == CFBooleanGetTypeID() && (CFBooleanRef)aPref == kCFBooleanTrue )
				bRet = sal_False;
			CFRelease( aPref );
		}
	}

	return bRet;

}

sal_Bool NSDocument_versionsSupported()
{
#ifdef USE_NATIVE_VERSIONS
	return ( class_getInstanceMethod( [NSDocument class], @selector(_preserveContentsIfNecessaryAfterWriting:toURL:forSaveOperation:version:error:) ) ? sal_True : sal_False );
#else	// USE_NATIVE_VERSIONS
	return sal_False;
#endif	// USE_NATIVE_VERSIONS
}

void SFXDocument_createDocument( SfxViewFrame *pFrame, NSView *pView, CFURLRef aURL, sal_Bool bReadOnly )
{
	if ( !NSDocument_versionsSupported() )
		return;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pFrame && pView )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		RunSFXDocument *pRunSFXDocument = [RunSFXDocument createWithFrame:pFrame view:pView URL:(NSURL *)aURL readOnly:bReadOnly];
		[pRunSFXDocument performSelectorOnMainThread:@selector(createDocument:) withObject:pRunSFXDocument waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

sal_Bool SFXDocument_documentIsReliquished( SfxViewFrame *pFrame )
{
	sal_Bool bRet = sal_False;

	if ( !NSDocument_versionsSupported() )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pFrame )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		RunSFXDocument *pRunSFXDocument = [RunSFXDocument createWithFrame:pFrame];
		[pRunSFXDocument performSelectorOnMainThread:@selector(getDocument:) withObject:pRunSFXDocument waitUntilDone:YES modes:pModes];
		bRet = [pRunSFXDocument documentIsRelinquished];
	}

	[pPool release];

	return bRet;
}

sal_Bool SFXDocument_hasDocument( SfxViewFrame *pFrame )
{
	sal_Bool bRet = sal_False;

	if ( !NSDocument_versionsSupported() )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pFrame )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		RunSFXDocument *pRunSFXDocument = [RunSFXDocument createWithFrame:pFrame];
		[pRunSFXDocument performSelectorOnMainThread:@selector(getDocument:) withObject:pRunSFXDocument waitUntilDone:YES modes:pModes];
		bRet = ( [pRunSFXDocument document] ? YES : NO );
	}

	[pPool release];

	return bRet;
}

void SFXDocument_releaseDocument( SfxViewFrame *pFrame )
{
	if ( !NSDocument_versionsSupported() )
		return;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pFrame )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		RunSFXDocument *pRunSFXDocument = [RunSFXDocument createWithFrame:pFrame];
		[pRunSFXDocument performSelectorOnMainThread:@selector(release:) withObject:pRunSFXDocument waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

void SFXDocument_revertDocumentToSaved( SfxViewFrame *pFrame )
{
	if ( !NSDocument_versionsSupported() )
		return;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	RunSFXDocument *pRunSFXDocument = [RunSFXDocument createWithFrame:pFrame];
	sal_uLong nCount = Application::ReleaseSolarMutex();
	[pRunSFXDocument performSelectorOnMainThread:@selector(revertDocumentToSaved:) withObject:pRunSFXDocument waitUntilDone:YES modes:pModes];
	Application::AcquireSolarMutex( nCount );

	[pPool release];
}

void SFXDocument_saveVersionOfDocument( SfxViewFrame *pFrame )
{
	if ( !NSDocument_versionsSupported() )
		return;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	RunSFXDocument *pRunSFXDocument = [RunSFXDocument createWithFrame:pFrame];
	[pRunSFXDocument performSelectorOnMainThread:@selector(saveVersionOfDocument:) withObject:pRunSFXDocument waitUntilDone:YES modes:pModes];

	[pPool release];
}

sal_Bool SFXDocument_setDocumentModified( SfxViewFrame *pFrame, sal_Bool bModified )
{
	sal_Bool bRet = sal_False;

	if ( !NSDocument_versionsSupported() )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSNumber *pNumber = [NSNumber numberWithBool:bModified];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	RunSFXDocument *pRunSFXDocument = [RunSFXDocument createWithFrame:pFrame];
	[pRunSFXDocument performSelectorOnMainThread:@selector(setDocumentModified:) withObject:pNumber waitUntilDone:YES modes:pModes];
	bRet = ( [pRunSFXDocument document] ? sal_True : sal_False );

	[pPool release];

	return bRet;
}
