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
