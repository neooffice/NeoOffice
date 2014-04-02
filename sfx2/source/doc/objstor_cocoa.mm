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

sal_Bool NSFileCoordinator_saveToImpl( SfxObjectShell *pObjShell, SfxMedium &rMedium, const SfxItemSet *pSet )
{
	__block sal_Bool bRet = sal_False;

	if ( !pObjShell )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	__block BOOL bBlockExecuted = NO;
	NSBundle *pBundle = [NSBundle bundleForClass:[NSBundle class]];
	if ( pBundle )
	{
		Class aClass = [pBundle classNamed:@"NSFileCoordinator"];
		if ( aClass && class_getInstanceMethod( aClass, @selector(initWithFilePresenter:) ) && class_getInstanceMethod( aClass, @selector(coordinateWritingItemAtURL:options:error:byAccessor:) ) )
		{
			NSObject *pFileCoordinator = [[aClass alloc] initWithFilePresenter:nil];
			if ( pFileCoordinator )
			{
				[pFileCoordinator autorelease];

				NSError *pError = nil;
				[pFileCoordinator coordinateWritingItemAtURL:[NSURL fileURLWithPath:@"/Users/pluby/Desktop/Test.odt"] options:NSFileCoordinatorWritingForReplacing error:&pError byAccessor:^(NSURL *pNewURL) {
					bBlockExecuted = YES;
					bRet = SfxObjectShell_saveToImpl( pObjShell, rMedium, pSet );
				}];
			}
		}
	}

	if ( !bBlockExecuted )
		bRet = SfxObjectShell_saveToImpl( pObjShell, rMedium, pSet );

	[pPool release];

	return bRet;
}

sal_Bool SfxObjectShell_saveToImpl( SfxObjectShell *pObjShell, SfxMedium &rMedium, const SfxItemSet *pSet )
{
	sal_Bool bRet = sal_False;

	if ( pObjShell )
		bRet = pObjShell->SaveTo_Impl( rMedium, pSet );

	return bRet;
}
