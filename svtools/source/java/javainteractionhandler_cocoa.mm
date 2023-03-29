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

#include "javainteractionhandler_cocoa.h"

@interface OpenJavaDownloadURL : NSObject
{
}
+ (id)create;
- (id)init;
- (void)openJavaDownloadURL:(id)pObject;
@end

@implementation OpenJavaDownloadURL

+ (id)create
{
	OpenJavaDownloadURL *pRet = [[OpenJavaDownloadURL alloc] init];
	[pRet autorelease];
	return pRet;
}

- (id)init
{
	[super init];

	return self;
}

- (void)openJavaDownloadURL:(id)pObject
{
	(void)pObject;

	NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
#ifdef PRODUCT_JAVA_DOWNLOAD_URL
	NSURL *pURL = [NSURL URLWithString:(NSString *)CFSTR( PRODUCT_JAVA_DOWNLOAD_URL )];
	if ( pWorkspace && pURL && ( [@"macappstores" isEqualToString:[pURL scheme]] || [@"http" isEqualToString:[pURL scheme]] || [@"https" isEqualToString:[pURL scheme]] ) )
		[pWorkspace openURL:pURL];
#endif	// PRODUCT_JAVA_DOWNLOAD_URL
}

@end

#ifdef PRODUCT_JAVA_DOWNLOAD_URL

void JavaInteractionHandler_downloadJava()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	OpenJavaDownloadURL *pOpenJavaDownloadURL = [OpenJavaDownloadURL create];
	osl_performSelectorOnMainThread( pOpenJavaDownloadURL, @selector(openJavaDownloadURL:), pOpenJavaDownloadURL, NO );

	[pPool release];
}

#endif	// PRODUCT_JAVA_DOWNLOAD_URL
