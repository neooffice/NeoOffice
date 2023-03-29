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

#import <stdio.h>
#import <Foundation/Foundation.h>

static printUsageAndExit( char *command )
{
	fprintf( stderr, "Usage: %s <old plist file> <new plist file>\n", command );
}

int main( int args, char **argv )
{
	if ( args != 3 )
		printUsageAndExit( argv[ 0 ] );

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NSDictionary *oldPlist = [NSDictionary dictionaryWithContentsOfFile:[NSString stringWithCString:argv[ 1 ] encoding:NSUTF8StringEncoding]];
	NSDictionary *newPlist = [NSDictionary dictionaryWithContentsOfFile:[NSString stringWithCString:argv[ 2 ] encoding:NSUTF8StringEncoding]];
	if ( !oldPlist || !newPlist )
	{
		[pool release];
		printUsageAndExit( argv[ 0 ] );
	}

	NSMutableDictionary *mergedPlist = [NSMutableDictionary dictionaryWithCapacity:[oldPlist count] + [newPlist count]];
	[mergedPlist addEntriesFromDictionary:oldPlist];

	// Merge "files" dictionary from new plist
	NSString *filesKey = @"files";
	NSDictionary *oldFiles = (NSDictionary *)[oldPlist objectForKey:filesKey];
	NSDictionary *newFiles = (NSDictionary *)[newPlist objectForKey:filesKey];
	if ( newFiles )
	{
		if ( oldFiles )
		{
			NSMutableDictionary *mergedFiles = [NSMutableDictionary dictionaryWithCapacity:[oldFiles count] + [newFiles count]];
			[mergedFiles addEntriesFromDictionary:oldFiles];
			[mergedFiles addEntriesFromDictionary:newFiles];
			[mergedPlist setObject:mergedFiles forKey:filesKey];
		}
		else
		{
			[mergedPlist setObject:newFiles forKey:filesKey];
		}
	}

	[mergedPlist writeToFile:@"/dev/stdout" atomically:NO];

	[pool release];

	exit( 0 );
}
