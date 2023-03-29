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

#import "cmdmailsuppl.hxx"
#import "cmdmailsuppl_cocoa.h"

using namespace com::sun::star::uno;

@interface CmdMailSupplOpenURLs : NSObject
{
	NSString*			mpAppID;
	BOOL				mbResult;
	NSArray*			mpURLs;
}
+ (id)createWithURLs:(NSArray *)pURLs appID:(NSString *)pAppID;
- (void)dealloc;
- (id)initWithURLs:(NSArray *)pURLs appID:(NSString *)pAppID;
- (void)openURLs:(id)pSender;
- (BOOL)result;
@end

@implementation CmdMailSupplOpenURLs

+ (id)createWithURLs:(NSArray *)pURLs appID:(NSString *)pAppID
{
	CmdMailSupplOpenURLs *pRet = [[CmdMailSupplOpenURLs alloc] initWithURLs:pURLs appID:pAppID];
	[pRet autorelease];
	return pRet;
}

- (void)dealloc
{
	if ( mpAppID )
		[mpAppID release];

	if ( mpURLs )
		[mpURLs release];

	[super dealloc];
}

- (id)initWithURLs:(NSArray *)pURLs appID:(NSString *)pAppID
{
	[super init];

	mpAppID = pAppID;
	if ( mpAppID )
		[mpAppID retain];
	mbResult = NO;
	mpURLs = pURLs;
	if ( mpURLs )
		[mpURLs retain];

	return self;
}

- (void)openURLs:(id)pSender
{
	(void)pSender;

	mbResult = NO;

	NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
	if ( pWorkspace && mpURLs && [mpURLs count] )
	{
		NSMutableArray *pAppURLs = [NSMutableArray arrayWithCapacity:3];
		if ( pAppURLs )
		{
			NSURL *pAppURL = [pWorkspace URLForApplicationWithBundleIdentifier:@"com.apple.mail"];
			if ( pAppURL )
				[pAppURLs addObject:pAppURL];
			pAppURL = [pWorkspace URLForApplicationToOpenURL:[NSURL URLWithString:@"mailto://"]];
			if ( pAppURL )
				[pAppURLs addObject:pAppURL];
			if ( mpAppID && [mpAppID length] )
			{
				pAppURL = [pWorkspace URLForApplicationWithBundleIdentifier:mpAppID];
				if ( pAppURL )
					[pAppURLs addObject:pAppURL];
			}

			NSWorkspaceOpenConfiguration *pConfiguration = [NSWorkspaceOpenConfiguration configuration];
			if ( [pAppURLs count] && pConfiguration )
			{
				pAppURL = [pAppURLs lastObject];
				[pAppURLs removeLastObject];
				[pWorkspace openURLs:mpURLs withApplicationAtURL:pAppURL configuration:pConfiguration completionHandler:^(NSRunningApplication *pApp, NSError *pError) {
					(void)pError;
					if ( !pApp && [pAppURLs count] )
					{
						NSURL *pAppURL = [pAppURLs lastObject];
						[pAppURLs removeLastObject];
						[pWorkspace openURLs:mpURLs withApplicationAtURL:pAppURL configuration:pConfiguration completionHandler:^(NSRunningApplication *pApp, NSError *pError) {
							(void)pError;
							if ( !pApp && [pAppURLs count] )
							{
								NSURL *pAppURL = [pAppURLs lastObject];
								[pAppURLs removeLastObject];
								[pWorkspace openURLs:mpURLs withApplicationAtURL:pAppURL configuration:pConfiguration completionHandler:nil];
							}
						}];
					}
				}];
				mbResult = YES;
			}
		}
	}
}

- (BOOL)result
{
	return mbResult;
}

@end

sal_Bool CmdMailSuppl_sendSimpleMailMessage( Sequence< OUString > &rStringList, OUString aMailerPath )
{
	sal_Bool bRet = sal_False;

	sal_Int32 nLen = rStringList.getLength();
	if ( !nLen )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSMutableArray *pURLs = [NSMutableArray arrayWithCapacity:nLen];
	if ( pURLs )
	{
		sal_Int32 i = 0;
		for ( ; i < nLen; i++ )
		{
			OUString aSystemPath;
			if ( ::osl::FileBase::E_None == ::osl::FileBase::getSystemPathFromFileURL( rStringList[ i ], aSystemPath ) )
			{
				NSString *pAttachPath = [NSString stringWithCharacters:aSystemPath.getStr() length:aSystemPath.getLength()];
				if ( pAttachPath )
				{
					NSURL *pAttachURL = [NSURL fileURLWithPath:pAttachPath];
					if ( pAttachURL )
						[pURLs addObject:pAttachURL];
				}
			}
		}

		if ( [pURLs count] )
		{
			NSString *pAppID = nil;
			if ( aMailerPath.getLength() )
			{
				NSString *pMailerPath = [NSString stringWithCharacters:aMailerPath.getStr() length:aMailerPath.getLength()];
				if ( pMailerPath )
				{
					NSBundle *pBundle = [NSBundle bundleWithPath:pMailerPath];
					if ( pBundle )
						pAppID = [pBundle bundleIdentifier];

					// Unquote single quoted paths. In previous versions, the
					// mailer path was stored with single quotes.
					if ( !pAppID && [pMailerPath length] > 2 && [pMailerPath characterAtIndex:0] == '\'' && [pMailerPath characterAtIndex:[pMailerPath length] - 1] == '\'')
					{
						pMailerPath = [pMailerPath substringWithRange:NSMakeRange( 1, [pMailerPath length] - 2)];
						if ( pMailerPath )
						{
							pBundle = [NSBundle bundleWithPath:pMailerPath];
							if ( pBundle )
								pAppID = [pBundle bundleIdentifier];
						}
					}
				}
			}

			CmdMailSupplOpenURLs *pCmdMailSupplOpenURLs = [CmdMailSupplOpenURLs createWithURLs:pURLs appID:pAppID];
			osl_performSelectorOnMainThread( pCmdMailSupplOpenURLs, @selector(openURLs:), pCmdMailSupplOpenURLs, YES );
			bRet = [pCmdMailSupplOpenURLs result];
		}
	}

	[pPool release];

	return bRet;
}
