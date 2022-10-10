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
 *  Patrick Luby, March 2017
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
