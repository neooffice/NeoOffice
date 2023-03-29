/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <premac.h>
#import <objc/objc-runtime.h>
#include <postmac.h>

#include <osl/objcutils.h>

#include "unoexe_cocoa.h"

static BOOL bNativeGUIDisabled = NO;

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
	osl_performSelectorOnMainThread( pDisableNativeGUI, @selector(disableNativeGUI:), pDisableNativeGUI, YES );

	[pPool release];
}
