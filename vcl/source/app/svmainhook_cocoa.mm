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
 *  Patrick Luby, April 2012
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2012 Planamesa Inc.
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

#include <premac.h>
#import <objc/objc-runtime.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>
#undef check

#include "svmainhook_cocoa.h"
#include "../../java/source/java/VCLEventQueue_cocoa.h"

@interface NSBundle (VCLBundle)
- (MacOSBOOL)loadNibNamed:(NSString *)pNibName owner:(id)pOwner topLevelObjects:(NSArray **)pTopLevelObjects;
@end
@interface VCLPostWillTerminateNotification : NSObject
+ (id)create;
- (void)postWillTerminateNotification:(id)pObj;
@end

@implementation VCLPostWillTerminateNotification

+ (id)create
{
	VCLPostWillTerminateNotification *pRet = [[VCLPostWillTerminateNotification alloc] init];
	[pRet autorelease];
	return pRet;
}

- (void)postWillTerminateNotification:(id)pObj
{
	NSApplication *pApp = [NSApplication sharedApplication];
	NSNotificationCenter *pNotificationCenter = [NSNotificationCenter defaultCenter];
	if ( pApp && pNotificationCenter )
		[pNotificationCenter postNotificationName:NSApplicationWillTerminateNotification object:pApp];
}

@end

void NSApplication_postWillTerminateNotification()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	VCLPostWillTerminateNotification *pVCLPostWillTerminateNotification = [VCLPostWillTerminateNotification create];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLPostWillTerminateNotification performSelectorOnMainThread:@selector(postWillTerminateNotification:) withObject:pVCLPostWillTerminateNotification waitUntilDone:YES modes:pModes];

	[pPool release];
}


void NSApplication_run()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSBundle *pBundle = [NSBundle mainBundle];
		if ( pBundle && [pBundle respondsToSelector:@selector(loadNibNamed:owner:topLevelObjects:)] )
 			[pBundle loadNibNamed:@"MainMenu" owner:pApp topLevelObjects:nil];
		else if ( class_getClassMethod( [NSBundle class], @selector(loadNibNamed:owner:) ) )
			[NSBundle loadNibNamed:@"MainMenu" owner:pApp];
		VCLEventQueue_installVCLEventQueueClasses();
		[pApp run];
	}

	[pPool release];
}
