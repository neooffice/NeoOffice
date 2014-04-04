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
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#import <objc/objc-runtime.h>
#include <postmac.h>

#import "objstor_cocoa.h"

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

static NSURL *NSURLFromOUString( OUString aURL )
{
	NSURL *pRet = nil;

	if ( aURL.indexOf( OUString( RTL_CONSTASCII_USTRINGPARAM( "file://" ) ) ) == 0 )
	{
		OUString aPath;
		if ( ::osl::FileBase::getSystemPathFromFileURL( aURL, aPath ) == ::osl::FileBase::E_None && aPath.getLength() )
		{
			NSString *pString = [NSString stringWithCharacters:aPath.getStr() length:aPath.getLength()];
			if ( pString )
			{
				pRet = [NSURL fileURLWithPath:pString];
				if ( pRet )
					pRet = [pRet filePathURL];
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
- (void)coordinateWritingItemAtURL:(NSURL *)pURL options:(NSFileCoordinatorWritingOptions)nOptions error:(NSError **)ppOutError byAccessor:(void (^)(NSURL *pNewURL))writer;
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
	sal_Bool				mbResult;
}
+ (id)createWithObjectShell:(SfxObjectShell *)pObjShell fileName:(const String *)pFileName filterName:(const String *)pFilterName itemSet:(const SfxItemSet *)pSet;
- (NSDocument *)documentForURL:(NSURL *)pURL;
- (NSObject *)fileCoordinator;
- (id)initWithObjectShell:(SfxObjectShell *)pObjShell fileName:(const String *)pFileName filterName:(const String *)pFilterName itemSet:(const SfxItemSet *)pSet;
- (void)objectShellDoSave_Impl:(id)pObject;
- (void)objectShellPreDoSaveAs_Impl:(id)pObject;
- (void)objectShellSave_Impl:(id)pObject;
@end

@implementation RunSFXFileCoordinator

+ (id)createWithObjectShell:(SfxObjectShell *)pObjShell fileName:(const String *)pFileName filterName:(const String *)pFilterName itemSet:(const SfxItemSet *)pSet
{
	RunSFXFileCoordinator *pRet = [[RunSFXFileCoordinator alloc] initWithObjectShell:pObjShell fileName:pFileName filterName:pFilterName itemSet:pSet];
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
			pRet = [pDocController documentForURL:pURL];
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
			if ( pObject && [pObject respondsToSelector:@selector(initWithFilePresenter:)] )
			{
				pRet = [pObject initWithFilePresenter:nil];
				if ( pRet )
					[pRet autorelease];
			}
		}
	}

	return pRet;
}

- (id)initWithObjectShell:(SfxObjectShell *)pObjShell fileName:(const String *)pFileName filterName:(const String *)pFilterName itemSet:(const SfxItemSet *)pSet
{
	[super init];

	mpObjShell = pObjShell;
	mpFileName = pFileName;
	mpFilterName = pFilterName;
	mpSet = pSet;
	mbResult = sal_False;

	return self;
}

- (void)objectShellDoSave_Impl:(id)pObject
{
	mbResult = sal_False;

	if ( !mpObjShell )
		return;

	__block sal_Bool bBlockResult = sal_False;

	IMutex& rSolarMutex = Application::GetSolarMutex();
	rSolarMutex.acquire();
	if ( !Application::IsShutDown() )
	{
		__block BOOL bBlockExecuted = NO;

		mpObjShell->GetMedium()->CheckForMovedFile( mpObjShell );

		NSURL *pURL = NSURLFromSfxMedium( mpObjShell->GetMedium() );
		if ( pURL )
		{
			NSDocument *pDoc = [self documentForURL:pURL];
			if ( pDoc && [pDoc respondsToSelector:@selector(performSynchronousFileAccessUsingBlock:)] )
			{
				[pDoc performSynchronousFileAccessUsingBlock:^(void) {
					bBlockExecuted = YES;
					bBlockResult = mpObjShell->DoSave_Impl( mpSet );
				}];
			}

			if ( !bBlockExecuted )
			{
				NSObject *pFileCoordinator = [self fileCoordinator];
				if ( pFileCoordinator && [pFileCoordinator respondsToSelector:@selector(coordinateWritingItemAtURL:options:error:byAccessor:)] )
				{
					NSError *pError = nil;
					[pFileCoordinator coordinateWritingItemAtURL:pURL options:NSFileCoordinatorWritingForReplacing error:&pError byAccessor:^(NSURL *pNewURL) {
						bBlockExecuted = YES;
						bBlockResult = mpObjShell->DoSave_Impl( mpSet );
					}];
				}
			}
		}

		if ( !bBlockExecuted )
			bBlockResult = mpObjShell->DoSave_Impl( mpSet );
	}
	rSolarMutex.release();

	mbResult = bBlockResult;
}

- (void)objectShellPreDoSaveAs_Impl:(id)pObject
{
	mbResult = sal_False;

	if ( !mpObjShell || !mpFileName || !mpFilterName )
		return;

	__block sal_Bool bBlockResult = sal_False;

	IMutex& rSolarMutex = Application::GetSolarMutex();
	rSolarMutex.acquire();
	if ( !Application::IsShutDown() )
	{
		__block BOOL bBlockExecuted = NO;

		mpObjShell->GetMedium()->CheckForMovedFile( mpObjShell );

		NSURL *pURL = NSURLFromOUString( OUString( *mpFileName ) );
		if ( pURL )
		{
			NSDocument *pDoc = [self documentForURL:pURL];
			if ( pDoc && [pDoc respondsToSelector:@selector(performSynchronousFileAccessUsingBlock:)] )
			{
				[pDoc performSynchronousFileAccessUsingBlock:^(void) {
					bBlockExecuted = YES;
					bBlockResult = mpObjShell->PreDoSaveAs_Impl( *mpFileName, *mpFilterName, (SfxItemSet *)mpSet );
				}];
			}

			if ( !bBlockExecuted )
			{
				NSObject *pFileCoordinator = [self fileCoordinator];
				if ( pFileCoordinator && [pFileCoordinator respondsToSelector:@selector(coordinateWritingItemAtURL:options:error:byAccessor:)] )
				{
					NSError *pError = nil;
					[pFileCoordinator coordinateWritingItemAtURL:pURL options:NSFileCoordinatorWritingForReplacing error:&pError byAccessor:^(NSURL *pNewURL) {
						bBlockExecuted = YES;
						bBlockResult = mpObjShell->PreDoSaveAs_Impl( *mpFileName, *mpFilterName, (SfxItemSet *)mpSet );
					}];
				}
			}
		}

		if ( !bBlockExecuted )
			bBlockResult = mpObjShell->PreDoSaveAs_Impl( *mpFileName, *mpFilterName, (SfxItemSet *)mpSet );
	}
	rSolarMutex.release();

	mbResult = bBlockResult;
}

- (void)objectShellSave_Impl:(id)pObject
{
	mbResult = sal_False;

	if ( !mpObjShell )
		return;

	__block sal_Bool bBlockResult = sal_False;

	IMutex& rSolarMutex = Application::GetSolarMutex();
	rSolarMutex.acquire();
	if ( !Application::IsShutDown() )
	{
		__block BOOL bBlockExecuted = NO;

		mpObjShell->GetMedium()->CheckForMovedFile( mpObjShell );

		NSURL *pURL = NSURLFromSfxMedium( mpObjShell->GetMedium() );
		if ( pURL )
		{
			NSDocument *pDoc = [self documentForURL:pURL];
			if ( pDoc && [pDoc respondsToSelector:@selector(performSynchronousFileAccessUsingBlock:)] )
			{
				[pDoc performSynchronousFileAccessUsingBlock:^(void) {
					bBlockExecuted = YES;
					bBlockResult = mpObjShell->Save_Impl( mpSet );
				}];
			}

			if ( !bBlockExecuted )
			{
				NSObject *pFileCoordinator = [self fileCoordinator];
				if ( pFileCoordinator && [pFileCoordinator respondsToSelector:@selector(coordinateWritingItemAtURL:options:error:byAccessor:)] )
				{
					NSError *pError = nil;
					[pFileCoordinator coordinateWritingItemAtURL:pURL options:NSFileCoordinatorWritingForReplacing error:&pError byAccessor:^(NSURL *pNewURL) {
						bBlockExecuted = YES;
						bBlockResult = mpObjShell->Save_Impl( mpSet );
					}];
				}
			}
		}

		if ( !bBlockExecuted )
			bBlockResult = mpObjShell->Save_Impl( mpSet );
	}
	rSolarMutex.release();

	mbResult = bBlockResult;
}

- (sal_Bool)result
{
	return mbResult;
}

@end

sal_Bool NSFileCoordinator_objectShellDoSave_Impl( SfxObjectShell *pObjShell, const SfxItemSet* pSet )
{
	sal_Bool bRet = sal_False;

	if ( !pObjShell )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	RunSFXFileCoordinator *pRunSFXFileCoordinator = [RunSFXFileCoordinator createWithObjectShell:pObjShell fileName:NULL filterName:NULL itemSet:pSet];
	ULONG nCount = Application::ReleaseSolarMutex();
	[pRunSFXFileCoordinator performSelectorOnMainThread:@selector(objectShellDoSave_Impl:) withObject:pRunSFXFileCoordinator waitUntilDone:YES modes:pModes];
	Application::AcquireSolarMutex( nCount );
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

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	RunSFXFileCoordinator *pRunSFXFileCoordinator = [RunSFXFileCoordinator createWithObjectShell:pObjShell fileName:pFileName filterName:pFilterName itemSet:pSet];
	ULONG nCount = Application::ReleaseSolarMutex();
	[pRunSFXFileCoordinator performSelectorOnMainThread:@selector(objectShellPreDoSaveAs_Impl:) withObject:pRunSFXFileCoordinator waitUntilDone:YES modes:pModes];
	Application::AcquireSolarMutex( nCount );
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

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	RunSFXFileCoordinator *pRunSFXFileCoordinator = [RunSFXFileCoordinator createWithObjectShell:pObjShell fileName:NULL filterName:NULL itemSet:pSet];
	ULONG nCount = Application::ReleaseSolarMutex();
	[pRunSFXFileCoordinator performSelectorOnMainThread:@selector(objectShellSave_Impl:) withObject:pRunSFXFileCoordinator waitUntilDone:YES modes:pModes];
	Application::AcquireSolarMutex( nCount );
	bRet = [pRunSFXFileCoordinator result];

	[pPool release];

	return bRet;
}
