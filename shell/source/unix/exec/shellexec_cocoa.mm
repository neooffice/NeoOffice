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

#include <osl/file.hxx>
#include <osl/objcutils.h>

#import "shellexec.hxx"
#import "shellexec_cocoa.h"

using namespace com::sun::star::uno;

@interface ShellExecOpenURL : NSObject
{
	BOOL				mbSelectInFinder;
	BOOL				mbResult;
	NSURL*				mpURL;
}
+ (id)createWithURL:(NSURL *)pURL selectInFinder:(BOOL)bSelectInFinder;
- (void)dealloc;
- (id)initWithURL:(NSURL *)pURL selectInFinder:(BOOL)bSelectInFinder;
- (void)openURL:(id)pSender;
- (BOOL)result;
@end

@implementation ShellExecOpenURL

+ (id)createWithURL:(NSURL *)pURL selectInFinder:(BOOL)bSelectInFinder
{
	ShellExecOpenURL *pRet = [[ShellExecOpenURL alloc] initWithURL:pURL selectInFinder:bSelectInFinder];
	[pRet autorelease];
	return pRet;
}

- (void)dealloc
{
	if ( mpURL )
		[mpURL release];

	[super dealloc];
}

- (id)initWithURL:(NSURL *)pURL selectInFinder:(BOOL)bSelectInFinder
{
	[super init];

	mbSelectInFinder = bSelectInFinder;
	mbResult = NO;
	mpURL = pURL;
	if ( mpURL )
		[mpURL retain];

	return self;
}

- (void)openURL:(id)pSender
{
	(void)pSender;

	mbResult = NO;

	NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
	if ( pWorkspace && mpURL )
	{
		if ( mbSelectInFinder )
		{
			[pWorkspace activateFileViewerSelectingURLs:[NSArray arrayWithObject:mpURL]];
			mbResult = YES;
		}
		else
		{
			mbResult = [pWorkspace openURL:mpURL];
			if ( !mbResult )
			{
				NSURL *pAppURL = nil;
				if ( [@"mailto" isEqualToString:[mpURL scheme]] )
					pAppURL = [pWorkspace URLForApplicationWithBundleIdentifier:@"com.apple.mail"];
				else
					pAppURL = [pWorkspace URLForApplicationWithBundleIdentifier:@"com.apple.Safari"];
				NSWorkspaceOpenConfiguration *pConfiguration = [NSWorkspaceOpenConfiguration configuration];
				if ( pAppURL && pConfiguration )
				{
					[pWorkspace openURLs:[NSArray arrayWithObject:mpURL] withApplicationAtURL:pAppURL configuration:pConfiguration completionHandler:nil];
					mbResult = YES;
				}
			}
		}
	}
}

- (BOOL)result
{
	return mbResult;
}

@end

sal_Bool ShellExec_openURL( OUString &rURL, sal_Bool bSelectInFinder )
{
	sal_Bool bRet = sal_False;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSString *pURLString = [NSString stringWithCharacters:rURL.getStr() length:rURL.getLength()];
	if ( pURLString )
	{
		NSURL *pURL = [NSURL URLWithString:pURLString];
		if ( pURL )
		{
			// If the file is an alias file, open the directory in the Finder
			// like LibO does for softlinks and other insecure extensions
			NSNumber *pAlias = nil;
			if ( !bSelectInFinder && [pURL getResourceValue:&pAlias forKey:NSURLIsAliasFileKey error:nil] && pAlias && [pAlias boolValue] )
				bSelectInFinder = sal_True;

			ShellExecOpenURL *pShellExecOpenURL = [ShellExecOpenURL createWithURL:pURL selectInFinder:bSelectInFinder];
			osl_performSelectorOnMainThread( pShellExecOpenURL, @selector(openURL:), pShellExecOpenURL, YES );
			bRet = [pShellExecOpenURL result];
		}
	}

	[pPool release];

	return bRet;
}
