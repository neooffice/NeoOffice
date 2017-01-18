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
 *  Patrick Luby, January 2017
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2017 Planamesa Inc.
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
#import <objc/objc-runtime.h>
#include <postmac.h>

#import "unoexe_cocoa.h"

// Redefine Cocoa YES and NO defines types for convenience
#ifdef YES
#undef YES
#define YES (MacOSBOOL)1
#endif
#ifdef NO
#undef NO
#define NO (MacOSBOOL)0
#endif

static MacOSBOOL bNativeGUIDisabled = NO;

@interface UnoExeApplication : NSApplication
{
}
- (void)run;
@end

@implementation UnoExeApplication

- (void)run
{
	[self terminate:self];
}

@end

@interface DisableNativeGUI : NSObject
{
}
+ (id)create;
- (void)disableNativeGUI:(id)pObject;
- (id)init;
@end

@implementation DisableNativeGUI

+ (id)create
{
	DisableNativeGUI *pRet = [[DisableNativeGUI alloc] init];
	[pRet autorelease];
	return pRet;
}

- (void)disableNativeGUI:(id)pObject
{
	(void)pObject;

	if ( !bNativeGUIDisabled )
	{
		bNativeGUIDisabled  = YES;

		SEL aSelector = @selector(run);
		Method aOldMethod = class_getInstanceMethod( [NSApplication class], aSelector );
		Method aNewMethod = class_getInstanceMethod( [UnoExeApplication class], aSelector );
		if ( aOldMethod && aNewMethod )
		{
			IMP aNewIMP = method_getImplementation( aNewMethod );
			if ( aNewIMP )
				method_setImplementation( aOldMethod, aNewIMP );
		}
	}
}

- (id)init
{
	[super init];

	return self;
}

@end

void UnoExe_disableNativeGUI()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	DisableNativeGUI *pDisableNativeGUI = [DisableNativeGUI create];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pDisableNativeGUI performSelectorOnMainThread:@selector(disableNativeGUI:) withObject:pDisableNativeGUI waitUntilDone:YES modes:pModes];

	[pPool release];
}
