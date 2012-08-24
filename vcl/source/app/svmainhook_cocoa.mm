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
#import <Cocoa/Cocoa.h>
#import <objc/objc-class.h>
#include <postmac.h>
#undef check

#include "svmainhook_cocoa.h"
#include "../../java/source/java/VCLEventQueue_cocoa.h"

@interface NSApplication (VCLApplicationPoseAs)
- (void)poseAsSendEvent:(NSEvent *)pEvent;
@end

@interface VCLApplication : NSApplication
- (void)sendEvent:(NSEvent *)pEvent;
@end

@implementation VCLApplication

- (void)sendEvent:(NSEvent *)pEvent
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( [super respondsToSelector:@selector(poseAsSendEvent:)] )
		[super poseAsSendEvent:pEvent];

	[pPool release];
}

@end

void NSApplication_run()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	// VCLApplication selectors

	SEL aSelector = @selector(sendEvent:);
	SEL aPoseAsSelector = @selector(poseAsSendEvent:);
	Method aOldMethod = class_getInstanceMethod( [NSApplication class], aSelector );
	Method aNewMethod = class_getInstanceMethod( [VCLApplication class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSApplication class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		[NSBundle loadNibNamed:@"MainMenu" owner:pApp];
		VCLEventQueue_installVCLEventQueueClasses();
		[pApp run];
	}

	[pPool release];
}
