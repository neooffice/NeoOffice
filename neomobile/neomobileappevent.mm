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

#import <crt_externs.h>
#import "neomobile.hxx"
#import "neomobileappevent.hxx"
#import "neomobilei18n.hxx"

using namespace ::rtl;

@implementation NeoMobileRunPasswordProtectionAlertOnMainThread

- (MacOSBOOL)cancelled
{
	return mcancelled;
}

- (id)init
{
	[super init];
	
	mcancelled=YES;
	return self;
}

- (void)runModal:(id)arg;
{
	NSAlert *alert = [NSAlert alertWithMessageText:NeoMobileGetLocalizedString(NEOMOBILEUPLOADPASSWORDPROTECTED) defaultButton:NeoMobileGetLocalizedString(NEOMOBILEUPLOAD) alternateButton:NeoMobileGetLocalizedString(NEOMOBILECANCEL) otherButton:nil informativeTextWithFormat:NeoMobileGetLocalizedString(NEOMOBILEUPLOADCONTINUE)];
	if(alert && [alert runModal] == NSAlertDefaultReturn)
		mcancelled = NO;
}

@end

@implementation NeoMobileDoFileManagerOnMainThread

- (id)init
{
	[super init];
	
	mpath=nil;
	return self;
}

- (void)dealloc
{
	if(mpath)
	{
		[mpath release];
		mpath=nil;
	}
	
	[super dealloc];
}

- (void)makeBasePath:(id)arg
{
#pragma unused(arg)

	NSString *basePath = nil;

	NSFileManager *fileManager = [NSFileManager defaultManager];
	if (fileManager)
	{
		// Use NSCachesDirectory to stay within FileVault encryption
		NSArray *cachePaths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
		if (cachePaths)
		{
			NSNumber *perms = [NSNumber numberWithUnsignedLong:(S_IRUSR | S_IWUSR | S_IXUSR)];
			NSDictionary *dict = (perms ? [NSDictionary dictionaryWithObject:perms forKey:NSFilePosixPermissions] : nil);

 			unsigned int dirCount = [cachePaths count];
 			unsigned int i = 0;
			for (; i < dirCount && !basePath; i++)
			{
				MacOSBOOL isDir = NO;
				NSString *cachePath = (NSString *)[cachePaths objectAtIndex:i];
				if (([fileManager fileExistsAtPath:cachePath isDirectory:&isDir] && isDir) || [fileManager createDirectoryAtPath:cachePath withIntermediateDirectories:NO attributes:dict error:nil])
				{
					// Append program name to cache path
					char **progName = _NSGetProgname();
					if (progName && *progName)
					{
						cachePath = [cachePath stringByAppendingPathComponent:[NSString stringWithUTF8String:(const char *)*progName]];
						isDir = NO;
						if (([fileManager fileExistsAtPath:cachePath isDirectory:&isDir] && isDir) || [fileManager createDirectoryAtPath:cachePath withIntermediateDirectories:NO attributes:dict error:nil])
						{
							basePath = cachePath;
							break;
						}
					}
				}
			}
		}
	}

	if (!basePath)
	{
		basePath = NSTemporaryDirectory();
		if (!basePath)
			basePath = @"/tmp";
	}

	NSString *filePath = [basePath stringByAppendingPathComponent:@"_nm_export"];
	if (fileManager)
	{
		while ([fileManager fileExistsAtPath:filePath])
			filePath = [basePath stringByAppendingPathComponent:[NSString stringWithFormat:@"_nm_export_%d", rand()]];
	}
	[filePath retain];
	mpath=filePath;
}

- (NSString *)filePath
{
	return(mpath);
}

- (void)createDir:(NSString *)path
{
	NSNumber *perms = [NSNumber numberWithUnsignedLong:(S_IRUSR | S_IWUSR | S_IXUSR)];
	NSDictionary *dict = (perms ? [NSDictionary dictionaryWithObject:perms forKey:NSFilePosixPermissions] : nil);
	[[NSFileManager defaultManager] createDirectoryAtPath:path withIntermediateDirectories:NO attributes:dict error:nil];
}

- (void)removeItem:(NSString *)path
{
	[[NSFileManager defaultManager] removeItemAtPath:path error:nil];
}

@end

NeoMobileExportFileAppEvent::NeoMobileExportFileAppEvent( OUString aSaveUUID, NSFileManager *pFileManager, NSMutableData *pPostBody, NSArray *pMimeTypes ) :
	mnErrorCode( 0 ),
	mpFileManager( pFileManager ),
	mbFinished( false ),
	mpPostBody( pPostBody ),
	maSaveUUID( aSaveUUID ),
	mpMimeTypes( pMimeTypes ),
	mbCanceled( false ),
	mbUnsupportedComponentType( false )
{
}
