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

#include <osl/objcutils.h>

#include "scanunx_cocoa.h"

@interface ScnLaunchImageCaptureApplication : NSObject
{
}
+ (id)create;
- (id)init;
- (void)launchImageCaptureApplication:(id)pObject;
@end

@implementation ScnLaunchImageCaptureApplication

+ (id)create
{
	ScnLaunchImageCaptureApplication *pRet = [[ScnLaunchImageCaptureApplication alloc] init];
	[pRet autorelease];
	return pRet;
}

- (id)init
{
	[super init];

	return self;
}

- (void)launchImageCaptureApplication:(id)pObject
{
	(void)pObject;

	// Always offload to the Image Capture application as Apple's App Sandbox
	// will prevent our application from accessing a device's files
	NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
	if ( pWorkspace )
	{
		NSURL *pAppURL = [pWorkspace URLForApplicationWithBundleIdentifier:@"com.apple.Image_Capture"];
		NSWorkspaceOpenConfiguration *pConfiguration = [NSWorkspaceOpenConfiguration configuration];
		if ( pAppURL && pConfiguration )
			[pWorkspace openApplicationAtURL:pAppURL configuration:pConfiguration completionHandler:nil];
	}
}

@end

void NSWorkspace_launchImageCaptureApplication()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	ScnLaunchImageCaptureApplication *pScnLaunchImageCaptureApplication = [ScnLaunchImageCaptureApplication create];
	osl_performSelectorOnMainThread( pScnLaunchImageCaptureApplication, @selector(launchImageCaptureApplication:), pScnLaunchImageCaptureApplication, NO );

	[pPool release];
}
