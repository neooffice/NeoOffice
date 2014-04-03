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

#include <sfx2/objsh.hxx>

#include <premac.h>
#import <Foundation/Foundation.h>
#import <objc/objc-runtime.h>
#include <postmac.h>

#import <osl/file.hxx>
#import <sfx2/docfile.hxx>

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

@interface NSObject (NSFileCoordinator)
- (id)initWithFilePresenter:(id)pFilePresenter;
- (void)coordinateWritingItemAtURL:(NSURL *)pURL options:(NSFileCoordinatorWritingOptions)nOptions error:(NSError **)ppOutError byAccessor:(void (^)(NSURL *pNewURL))writer;
@end

static NSObject *CreateNSFileCoordinator()
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

static NSURL *NSURLFromOUString( ::rtl::OUString aURL )
{
	NSURL *pRet = nil;

	if ( aURL.indexOf( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "file://" ) ) ) == 0 )
	{
		::rtl::OUString aPath;
		if ( ::osl::FileBase::getSystemPathFromFileURL( aURL, aPath ) == osl::FileBase::E_None && aPath.getLength() )
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

sal_Bool NSFileCoordinator_objectShellDoSave_Impl( SfxObjectShell *pObjShell, const SfxItemSet* pSet )
{
	__block sal_Bool bRet = sal_False;

	if ( !pObjShell )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	__block BOOL bBlockExecuted = NO;

	NSURL *pURL = NSURLFromSfxMedium( pObjShell->GetMedium() );
	NSObject *pFileCoordinator = CreateNSFileCoordinator();
	if ( pURL && pFileCoordinator && [pFileCoordinator respondsToSelector:@selector(coordinateWritingItemAtURL:options:error:byAccessor:)] )
	{
		NSError *pError = nil;
		[pFileCoordinator coordinateWritingItemAtURL:pURL options:NSFileCoordinatorWritingForReplacing error:&pError byAccessor:^(NSURL *pNewURL) {
			bBlockExecuted = YES;
			bRet = pObjShell->DoSave_Impl( pSet );
		}];
	}

	if ( !bBlockExecuted )
		bRet = pObjShell->DoSave_Impl( pSet );

	[pPool release];

	return bRet;
}

sal_Bool NSFileCoordinator_objectShellPreDoSaveAs_Impl( SfxObjectShell *pObjShell, const String &rFileName, const String &aFilterName, SfxItemSet *pSet )
{
	__block sal_Bool bRet = sal_False;

	if ( !pObjShell )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	__block BOOL bBlockExecuted = NO;

	NSURL *pURL = NSURLFromOUString( rFileName );
	NSObject *pFileCoordinator = CreateNSFileCoordinator();
	if ( pURL && pFileCoordinator && [pFileCoordinator respondsToSelector:@selector(coordinateWritingItemAtURL:options:error:byAccessor:)] )
	{
		NSError *pError = nil;
		[pFileCoordinator coordinateWritingItemAtURL:pURL options:NSFileCoordinatorWritingForReplacing error:&pError byAccessor:^(NSURL *pNewURL) {
			bBlockExecuted = YES;
			bRet = pObjShell->PreDoSaveAs_Impl( rFileName, aFilterName, pSet );
		}];
	}

	if ( !bBlockExecuted )
		bRet = pObjShell->PreDoSaveAs_Impl( rFileName, aFilterName, pSet );

	[pPool release];

	return bRet;
}

sal_Bool NSFileCoordinator_objectShellSave_Impl( SfxObjectShell *pObjShell, const SfxItemSet *pSet )
{
	__block sal_Bool bRet = sal_False;

	if ( !pObjShell )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	__block BOOL bBlockExecuted = NO;

	NSURL *pURL = NSURLFromSfxMedium( pObjShell->GetMedium() );
	NSObject *pFileCoordinator = CreateNSFileCoordinator();
	if ( pURL && pFileCoordinator && [pFileCoordinator respondsToSelector:@selector(coordinateWritingItemAtURL:options:error:byAccessor:)] )
	{
		NSError *pError = nil;
		[pFileCoordinator coordinateWritingItemAtURL:pURL options:NSFileCoordinatorWritingForReplacing error:&pError byAccessor:^(NSURL *pNewURL) {
			bBlockExecuted = YES;
			bRet = pObjShell->Save_Impl( pSet );
		}];
	}

	if ( !bBlockExecuted )
		bRet = pObjShell->Save_Impl( pSet );

	[pPool release];

	return bRet;
}
