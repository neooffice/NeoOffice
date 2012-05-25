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
 *  Patrick Luby, May 2012
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
