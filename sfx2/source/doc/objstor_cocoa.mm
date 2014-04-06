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
 *  Patrick Luby, April 2014
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2014 Planamesa Inc.
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

#include <osl/file.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#import <objc/objc-runtime.h>
#include <postmac.h>

#import "objstor_cocoa.h"

// Uncomment the following line to limit file coordination to only iCloud URLs
// #define FILE_COORDINATION_FOR_ICLOUD_ONLY

#ifndef NSFileCoordinatorWritingOptions
enum {
	NSFileCoordinatorWritingForDeleting = 1 << 0,
	NSFileCoordinatorWritingForReplacing = 1 << 3,
	NSFileCoordinatorWritingForMoving = 1 << 1,
	NSFileCoordinatorWritingForMerging = 1 << 2
};
typedef NSUInteger NSFileCoordinatorWritingOptions;
#endif	// !NSFileCoordinatorWritingOptions

using namespace rtl;
using namespace vos;

@interface NSFileManager (RunSFXFileCoordinator)
- (BOOL)isUbiquitousItemAtURL:(NSURL *)pURL;
@end

static NSURL *NSURLFromOUString( OUString aURL )
{
	NSURL *pRet = nil;

	if ( aURL.indexOf( OUString( RTL_CONSTASCII_USTRINGPARAM( "file://" ) ) ) == 0 )
	{
		// Exclude our own temporary files to avoid degrading performance
		OUString aPath;
		if ( ::osl::FileBase::getSystemPathFromFileURL( aURL, aPath ) == ::osl::FileBase::E_None && aPath.getLength() && aPath.indexOf( ::utl::TempFile::GetTempNameBaseDirectory() ) != 0 )
		{
			NSString *pString = [NSString stringWithCharacters:aPath.getStr() length:aPath.getLength()];
			if ( pString )
			{
				pRet = [NSURL fileURLWithPath:pString];
				if ( pRet )
				{
					pRet = [pRet URLByStandardizingPath];
					if ( pRet )
					{
						pRet = [pRet URLByResolvingSymlinksInPath];
#ifdef FILE_COORDINATION_FOR_ICLOUD_ONLY
						if ( pRet )
						{
							NSFileManager *pFileManager = [NSFileManager defaultManager];
							if ( !pFileManager || ![pFileManager respondsToSelector:@selector(isUbiquitousItemAtURL:)] || ![pFileManager isUbiquitousItemAtURL:pRet] )
								pRet = nil;
						}
#endif	// FILE_COORDINATION_FOR_ICLOUD_ONLY
					}
				}
			}
		}
	}

	return pRet;
}

static NSURL *NSURLFromSfxMedium( SfxMedium *pMedium )
{
	NSURL *pRet = nil;

	if ( pMedium )
		pRet = NSURLFromOUString( pMedium->GetBaseURL( true ) );

	return pRet;
}

@interface NSObject (NSFileCoordinator)
- (id)initWithFilePresenter:(id)pFilePresenter;
- (void)coordinateWritingItemAtURL:(NSURL *)pURL options:(NSFileCoordinatorWritingOptions)nOptions error:(NSError **)ppOutError byAccessor:(void (^)(NSURL *pNewURL))aWriter;
- (void)coordinateWritingItemAtURL:(NSURL *)pURL1 options:(NSFileCoordinatorWritingOptions)nOptions1 writingItemAtURL:(NSURL *)pURL2 options:(NSFileCoordinatorWritingOptions)nOptions2 error:(NSError **)ppOutError byAccessor:(void (^)(NSURL *pNewURL1, NSURL *pNewURL2))aWriter;
@end

@interface NSDocument (RunSFXFileCoordinator)
- (void)performSynchronousFileAccessUsingBlock:(void (^)(void))aBlock;
@end

@interface RunSFXFileCoordinator : NSObject
{
	SfxObjectShell*			mpObjShell;
	const String*			mpFileName;
	const String*			mpFilterName;
	const SfxItemSet*		mpSet;
	SfxMedium*				mpMedium;
	BOOL*					mpCommit;
	sal_Bool				mbResult;
}
+ (id)createWithObjectShell:(SfxObjectShell *)pObjShell fileName:(const String *)pFileName filterName:(const String *)pFilterName itemSet:(const SfxItemSet *)pSet medium:(SfxMedium *)pMedium commit:(BOOL *)pCommit;
- (NSDocument *)documentForURL:(NSURL *)pURL;
- (NSObject *)fileCoordinator;
- (id)initWithObjectShell:(SfxObjectShell *)pObjShell fileName:(const String *)pFileName filterName:(const String *)pFilterName itemSet:(const SfxItemSet *)pSet medium:(SfxMedium *)pMedium commit:(BOOL *)pCommit;
- (void)objectShellDoSave:(id)pObject;
- (void)objectShellDoSaveAs:(id)pObject;
- (void)objectShellDoSaveObjectAs:(id)pObject;
- (void)objectShellDoSave_Impl:(id)pObject;
- (void)objectShellPreDoSaveAs_Impl:(id)pObject;
- (void)objectShellSave_Impl:(id)pObject;
- (sal_Bool)writeToURL1:(NSURL *)pURL1 writeToURL2:(NSURL *)pURL2 writer:(sal_Bool (^)())aWriter;
@end

@implementation RunSFXFileCoordinator

+ (id)createWithObjectShell:(SfxObjectShell *)pObjShell fileName:(const String *)pFileName filterName:(const String *)pFilterName itemSet:(const SfxItemSet *)pSet medium:(SfxMedium *)pMedium commit:(BOOL *)pCommit
{
	RunSFXFileCoordinator *pRet = [[RunSFXFileCoordinator alloc] initWithObjectShell:pObjShell fileName:pFileName filterName:pFilterName itemSet:pSet medium:pMedium commit:pCommit];
	[pRet autorelease];
	return pRet;
}

- (NSDocument *)documentForURL:(NSURL *)pURL
{
	NSDocument *pRet = nil;

	if ( pURL )
	{
		NSDocumentController *pDocController = [NSDocumentController sharedDocumentController];
		if ( pDocController )
		{
			NSDocument *pDoc = [pDocController documentForURL:pURL];
			if ( pDoc && [pDoc respondsToSelector:@selector(performSynchronousFileAccessUsingBlock:)] )
				pRet = pDoc;
		}
	}

	return pRet;
}

- (NSObject *)fileCoordinator
{
	NSObject *pRet = nil;

	NSBundle *pBundle = [NSBundle bundleForClass:[NSBundle class]];
	if ( pBundle )
	{
		Class aClass = [pBundle classNamed:@"NSFileCoordinator"];
		if ( aClass )
		{
			NSObject *pObject = [aClass alloc];
			if ( pObject && [pObject respondsToSelector:@selector(initWithFilePresenter:)] && [pObject respondsToSelector:@selector(coordinateWritingItemAtURL:options:error:byAccessor:)] && [pObject respondsToSelector:@selector(coordinateWritingItemAtURL:options:writingItemAtURL:options:error:byAccessor:)] )
			{
				pRet = [pObject initWithFilePresenter:nil];
				if ( pRet )
					[pRet autorelease];
			}
		}
	}

	return pRet;
}

- (id)initWithObjectShell:(SfxObjectShell *)pObjShell fileName:(const String *)pFileName filterName:(const String *)pFilterName itemSet:(const SfxItemSet *)pSet medium:(SfxMedium *)pMedium commit:(BOOL *)pCommit
{
	[super init];

	mpObjShell = pObjShell;
	mpFileName = pFileName;
	mpFilterName = pFilterName;
	mpSet = pSet;
	mpMedium = pMedium;
	mpCommit = pCommit;
	mbResult = sal_False;

	return self;
}

- (void)objectShellDoSave:(id)pObject
{
	mbResult = sal_False;

	if ( !mpObjShell )
		return;

	IMutex& rSolarMutex = Application::GetSolarMutex();
	rSolarMutex.acquire();
	if ( !Application::IsShutDown() )
	{
		sal_Bool (^aBlock)() = ^() {
			return mpObjShell->DoSave();
		};

		mpObjShell->GetMedium()->CheckForMovedFile( mpObjShell );
		NSURL *pURL = NSURLFromSfxMedium( mpObjShell->GetMedium() );
		mbResult = [self writeToURL1:pURL writeToURL2:nil writer:aBlock];
	}
	rSolarMutex.release();
}

- (void)objectShellDoSaveAs:(id)pObject
{
	mbResult = sal_False;

	if ( !mpObjShell || !mpMedium )
		return;

	IMutex& rSolarMutex = Application::GetSolarMutex();
	rSolarMutex.acquire();
	if ( !Application::IsShutDown() )
	{
		sal_Bool (^aBlock)() = ^() {
			return mpObjShell->DoSaveAs( *mpMedium );
		};

		mpObjShell->GetMedium()->CheckForMovedFile( mpObjShell );
		mpMedium->CheckForMovedFile( mpObjShell );
		NSURL *pURL1 = NSURLFromSfxMedium( mpObjShell->GetMedium() );
		NSURL *pURL2 = NSURLFromSfxMedium( mpMedium );
		mbResult = [self writeToURL1:pURL1 writeToURL2:pURL2 writer:aBlock];
	}
	rSolarMutex.release();
}

- (void)objectShellDoSaveObjectAs:(id)pObject
{
	mbResult = sal_False;

	if ( !mpObjShell || !mpMedium || !mpCommit )
		return;

	IMutex& rSolarMutex = Application::GetSolarMutex();
	rSolarMutex.acquire();
	if ( !Application::IsShutDown() )
	{
		sal_Bool (^aBlock)() = ^() {
			return mpObjShell->DoSaveObjectAs( *mpMedium, *mpCommit );
		};

		mpObjShell->GetMedium()->CheckForMovedFile( mpObjShell );
		mpMedium->CheckForMovedFile( mpObjShell );
		NSURL *pURL1 = NSURLFromSfxMedium( mpObjShell->GetMedium() );
		NSURL *pURL2 = NSURLFromSfxMedium( mpMedium );
		mbResult = [self writeToURL1:pURL1 writeToURL2:pURL2 writer:aBlock];
	}
	rSolarMutex.release();
}

- (void)objectShellDoSave_Impl:(id)pObject
{
	mbResult = sal_False;

	if ( !mpObjShell )
		return;

	IMutex& rSolarMutex = Application::GetSolarMutex();
	rSolarMutex.acquire();
	if ( !Application::IsShutDown() )
	{
		sal_Bool (^aBlock)() = ^() {
			return mpObjShell->DoSave_Impl( mpSet );
		};

		mpObjShell->GetMedium()->CheckForMovedFile( mpObjShell );
		NSURL *pURL = NSURLFromSfxMedium( mpObjShell->GetMedium() );
		mbResult = [self writeToURL1:pURL writeToURL2:nil writer:aBlock];
	}
	rSolarMutex.release();
}

- (void)objectShellPreDoSaveAs_Impl:(id)pObject
{
	mbResult = sal_False;

	if ( !mpObjShell || !mpFileName || !mpFilterName )
		return;

	IMutex& rSolarMutex = Application::GetSolarMutex();
	rSolarMutex.acquire();
	if ( !Application::IsShutDown() )
	{
		sal_Bool (^aBlock)() = ^() {
			return mpObjShell->PreDoSaveAs_Impl( *mpFileName, *mpFilterName, (SfxItemSet *)mpSet );
		};

		mpObjShell->GetMedium()->CheckForMovedFile( mpObjShell );
		NSURL *pURL1 = NSURLFromSfxMedium( mpObjShell->GetMedium() );
		NSURL *pURL2 = NSURLFromOUString( OUString( *mpFileName ) );
		mbResult = [self writeToURL1:pURL1 writeToURL2:pURL2 writer:aBlock];
	}
	rSolarMutex.release();
}

- (void)objectShellSave_Impl:(id)pObject
{
	mbResult = sal_False;

	if ( !mpObjShell )
		return;

	IMutex& rSolarMutex = Application::GetSolarMutex();
	rSolarMutex.acquire();
	if ( !Application::IsShutDown() )
	{
		sal_Bool (^aBlock)() = ^() {
			return mpObjShell->Save_Impl( mpSet );
		};

		mpObjShell->GetMedium()->CheckForMovedFile( mpObjShell );
		NSURL *pURL = NSURLFromSfxMedium( mpObjShell->GetMedium() );
		mbResult = [self writeToURL1:pURL writeToURL2:nil writer:aBlock];
	}
	rSolarMutex.release();
}

- (sal_Bool)result
{
	return mbResult;
}

- (sal_Bool)writeToURL1:(NSURL *)pURL1 writeToURL2:(NSURL *)pURL2 writer:(sal_Bool (^)())aWriter
{
	__block sal_Bool bRet = sal_False;

	if ( !aWriter )
		return bRet;

	__block BOOL bBlockExecuted = NO;
	__block void (^aBlock)() = ^() {
		bBlockExecuted = YES;
		bRet = aWriter();
	};

	NSDocument *pDoc1 = [self documentForURL:pURL1];
	NSDocument *pDoc2 = [self documentForURL:pURL2];
	if ( !pDoc1 && pDoc2 )
	{
		// Swap URLs and documents
		pDoc1 = pDoc2;
		pDoc2 = nil;
		pURL2 = pURL1;
		pURL1 = nil;
	}
	else if ( !pURL1 )
	{
		// Shift second URLs and documents to first position
		pURL1 = pURL2;
		pURL2 = nil;
		pDoc1 = pDoc2;
		pDoc2 = nil;
	}

	if ( pDoc1 )
	{
		[pDoc1 performSynchronousFileAccessUsingBlock:^() {
			if ( pDoc2 )
			{
				[pDoc2 performSynchronousFileAccessUsingBlock:aBlock];
			}
			else
			{
				NSObject *pFileCoordinator = [self fileCoordinator];
				if ( pFileCoordinator )
				{
					NSError *pError = nil;
					[pFileCoordinator coordinateWritingItemAtURL:pURL2 options:NSFileCoordinatorWritingForReplacing error:&pError byAccessor:^(NSURL *pNewURL) {
						aBlock();
					}];
				}
			}
		}];
	}

	if ( !bBlockExecuted && pURL1 )
	{
		NSObject *pFileCoordinator = [self fileCoordinator];
		if ( pFileCoordinator )
		{
			NSError *pError = nil;
			if ( pURL2 )
			{
				[pFileCoordinator coordinateWritingItemAtURL:pURL1 options:NSFileCoordinatorWritingForReplacing writingItemAtURL:pURL2 options:NSFileCoordinatorWritingForReplacing error:&pError byAccessor:^(NSURL *pNewURL1, NSURL *pNewURL2) {
					aBlock();
				}];
			}
			else
			{
				[pFileCoordinator coordinateWritingItemAtURL:pURL1 options:NSFileCoordinatorWritingForReplacing error:&pError byAccessor:^(NSURL *pNewURL) {
					aBlock();
				}];
			}
		}
	}

	if ( !bBlockExecuted )
		aBlock();

	return bRet;
}

@end

sal_Bool NSFileCoordinator_objectShellDoSave( SfxObjectShell *pObjShell )
{
	sal_Bool bRet = sal_False;

	if ( !pObjShell )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	pObjShell->GetMedium()->CheckForMovedFile( pObjShell );
	NSURL *pURL = NSURLFromSfxMedium( pObjShell->GetMedium() );
	RunSFXFileCoordinator *pRunSFXFileCoordinator = [RunSFXFileCoordinator createWithObjectShell:pObjShell fileName:NULL filterName:NULL itemSet:NULL medium:NULL commit:NO];
	if ( pURL )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		ULONG nCount = Application::ReleaseSolarMutex();
		[pRunSFXFileCoordinator performSelectorOnMainThread:@selector(objectShellDoSave:) withObject:pRunSFXFileCoordinator waitUntilDone:YES modes:pModes];
		Application::AcquireSolarMutex( nCount );
	}
	else
	{
		[pRunSFXFileCoordinator objectShellDoSave:pRunSFXFileCoordinator];
	}
	bRet = [pRunSFXFileCoordinator result];

	[pPool release];

	return bRet;
}

sal_Bool NSFileCoordinator_objectShellDoSaveAs( SfxObjectShell *pObjShell, SfxMedium *pMedium )
{
	sal_Bool bRet = sal_False;

	if ( !pObjShell || !pMedium )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	pObjShell->GetMedium()->CheckForMovedFile( pObjShell );
	pMedium->CheckForMovedFile( pObjShell );
	NSURL *pURL1 = NSURLFromSfxMedium( pObjShell->GetMedium() );
	NSURL *pURL2 = NSURLFromSfxMedium( pMedium );
	RunSFXFileCoordinator *pRunSFXFileCoordinator = [RunSFXFileCoordinator createWithObjectShell:pObjShell fileName:NULL filterName:NULL itemSet:NULL medium:pMedium commit:NO];
	if ( pURL1 || pURL2 )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		ULONG nCount = Application::ReleaseSolarMutex();
		[pRunSFXFileCoordinator performSelectorOnMainThread:@selector(objectShellDoSaveAs:) withObject:pRunSFXFileCoordinator waitUntilDone:YES modes:pModes];
		Application::AcquireSolarMutex( nCount );
	}
	else
	{
		[pRunSFXFileCoordinator objectShellDoSaveAs:pRunSFXFileCoordinator];
	}
	bRet = [pRunSFXFileCoordinator result];

	[pPool release];

	return bRet;
}

sal_Bool NSFileCoordinator_objectShellDoSaveObjectAs( SfxObjectShell *pObjShell, SfxMedium *pMedium, BOOL *pCommit )
{
	sal_Bool bRet = sal_False;

	if ( !pObjShell || !pMedium || !pCommit )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	pObjShell->GetMedium()->CheckForMovedFile( pObjShell );
	pMedium->CheckForMovedFile( pObjShell );
	NSURL *pURL1 = NSURLFromSfxMedium( pObjShell->GetMedium() );
	NSURL *pURL2 = NSURLFromSfxMedium( pMedium );
	RunSFXFileCoordinator *pRunSFXFileCoordinator = [RunSFXFileCoordinator createWithObjectShell:pObjShell fileName:NULL filterName:NULL itemSet:NULL medium:pMedium commit:pCommit];
	if ( pURL1 || pURL2 )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		ULONG nCount = Application::ReleaseSolarMutex();
		[pRunSFXFileCoordinator performSelectorOnMainThread:@selector(objectShellDoSaveObjectAs:) withObject:pRunSFXFileCoordinator waitUntilDone:YES modes:pModes];
		Application::AcquireSolarMutex( nCount );
	}
	else
	{
		[pRunSFXFileCoordinator objectShellDoSaveObjectAs:pRunSFXFileCoordinator];
	}
	bRet = [pRunSFXFileCoordinator result];

	[pPool release];

	return bRet;
}

sal_Bool NSFileCoordinator_objectShellDoSave_Impl( SfxObjectShell *pObjShell, const SfxItemSet* pSet )
{
	sal_Bool bRet = sal_False;

	if ( !pObjShell )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	pObjShell->GetMedium()->CheckForMovedFile( pObjShell );
	NSURL *pURL = NSURLFromSfxMedium( pObjShell->GetMedium() );
	RunSFXFileCoordinator *pRunSFXFileCoordinator = [RunSFXFileCoordinator createWithObjectShell:pObjShell fileName:NULL filterName:NULL itemSet:pSet medium:NULL commit:NO];
	if ( pURL )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		ULONG nCount = Application::ReleaseSolarMutex();
		[pRunSFXFileCoordinator performSelectorOnMainThread:@selector(objectShellDoSave_Impl:) withObject:pRunSFXFileCoordinator waitUntilDone:YES modes:pModes];
		Application::AcquireSolarMutex( nCount );
	}
	else
	{
		[pRunSFXFileCoordinator objectShellDoSave_Impl:pRunSFXFileCoordinator];
	}
	bRet = [pRunSFXFileCoordinator result];

	[pPool release];

	return bRet;
}

sal_Bool NSFileCoordinator_objectShellPreDoSaveAs_Impl( SfxObjectShell *pObjShell, const String *pFileName, const String *pFilterName, SfxItemSet *pSet )
{
	sal_Bool bRet = sal_False;

	if ( !pObjShell || !pFileName || !pFilterName )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	pObjShell->GetMedium()->CheckForMovedFile( pObjShell );
	NSURL *pURL1 = NSURLFromSfxMedium( pObjShell->GetMedium() );
	NSURL *pURL2 = NSURLFromOUString( OUString( *pFileName ) );
	RunSFXFileCoordinator *pRunSFXFileCoordinator = [RunSFXFileCoordinator createWithObjectShell:pObjShell fileName:pFileName filterName:pFilterName itemSet:pSet medium:NULL commit:NO];
	if ( pURL1 || pURL2 )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		ULONG nCount = Application::ReleaseSolarMutex();
		[pRunSFXFileCoordinator performSelectorOnMainThread:@selector(objectShellPreDoSaveAs_Impl:) withObject:pRunSFXFileCoordinator waitUntilDone:YES modes:pModes];
		Application::AcquireSolarMutex( nCount );
	}
	else
	{
		[pRunSFXFileCoordinator objectShellPreDoSaveAs_Impl:pRunSFXFileCoordinator];
	}
	bRet = [pRunSFXFileCoordinator result];

	[pPool release];

	return bRet;
}

sal_Bool NSFileCoordinator_objectShellSave_Impl( SfxObjectShell *pObjShell, const SfxItemSet *pSet )
{
	sal_Bool bRet = sal_False;

	if ( !pObjShell )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	pObjShell->GetMedium()->CheckForMovedFile( pObjShell );
	NSURL *pURL = NSURLFromSfxMedium( pObjShell->GetMedium() );
	RunSFXFileCoordinator *pRunSFXFileCoordinator = [RunSFXFileCoordinator createWithObjectShell:pObjShell fileName:NULL filterName:NULL itemSet:pSet medium:NULL commit:NO];
	if ( pURL )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		ULONG nCount = Application::ReleaseSolarMutex();
		[pRunSFXFileCoordinator performSelectorOnMainThread:@selector(objectShellSave_Impl:) withObject:pRunSFXFileCoordinator waitUntilDone:YES modes:pModes];
		Application::AcquireSolarMutex( nCount );
	}
	else
	{
		[pRunSFXFileCoordinator objectShellSave_Impl:pRunSFXFileCoordinator];
	}
	bRet = [pRunSFXFileCoordinator result];

	[pPool release];

	return bRet;
}
