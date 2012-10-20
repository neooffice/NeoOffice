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
 *  Patrick Luby, August 2012
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

#include <premac.h>
#import <Foundation/Foundation.h>
#include <postmac.h>

#include "system.h"

// Redefine Cocoa YES and NO defines types for convenience
#ifdef YES
#undef YES
#define YES (MacOSBOOL)1
#endif
#ifdef NO
#undef NO
#define NO (MacOSBOOL)0
#endif

static NSString *macxp_resolveAliasImpl(const char *path)
{
	NSString *pRet = NULL;

	if ( path && strlen( path ) )
	{
		NSURL *pURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:path]];
		if ( pURL )
			pURL = [pURL URLByStandardizingPath];
		if ( pURL )
			pURL = [pURL URLByResolvingSymlinksInPath];
		if ( pURL )
		{
			NSNumber *bAlias = nil;
			if ( [pURL getResourceValue:&bAlias forKey:NSURLIsAliasFileKey error:nil] )
			{
				if ( bAlias && [bAlias boolValue] )
				{
					NSData *pData = [NSURL bookmarkDataWithContentsOfURL:pURL error:nil];
					if ( pData )
					{
						MacOSBOOL bStale = NO;
						pURL = [NSURL URLByResolvingBookmarkData:pData options:NSURLBookmarkResolutionWithoutUI | NSURLBookmarkResolutionWithoutMounting relativeToURL:nil bookmarkDataIsStale:&bStale error:nil];
						if ( pURL )
						{
 							if ( bStale )
								pURL = nil;
							if ( pURL )
								pURL = [pURL URLByStandardizingPath];
							if ( pURL )
								pURL = [pURL URLByResolvingSymlinksInPath];

							// Recurse to check if the URL is also an alias
							if ( pURL )
							{
								NSString *pPath = [pURL path];
								if ( pPath )
								{
									NSString *pRecursedPath = macxp_resolveAliasImpl( [pPath UTF8String] );
									if ( pRecursedPath )
										pURL = [NSURL fileURLWithPath:pRecursedPath];
										
								}
							}
						}
					}
					else
					{
						pURL = nil;
					}
				}

				if ( pURL )
					pRet = [pURL path];
			}
		}
	}

	return pRet;
}

int macxp_resolveAlias(char *path, int buflen, sal_Bool noResolveLastElement)
{
	int nRet = 0;

	if ( noResolveLastElement )
	{
		char *basePath = strrchr( path, '/' );
		if ( !basePath )
			return nRet;
		basePath = strdup( basePath );
		if ( !basePath )
			return nRet;
		path[ strlen( path ) - strlen( basePath ) ] = '\0';

		sal_Bool bModified = sal_False;
		if ( !macxp_resolveAlias( path, buflen, sal_False ) )
		{
			int nLen = strlen( path ) + strlen( basePath );
			if ( nLen < buflen )
			{
				strcat( path, basePath );
				bModified = sal_True;
			}
			else
			{
				errno = ENAMETOOLONG;
				nRet = -1;
			}
		}

		if ( !bModified )
			strcat( path, basePath );
		free( basePath );

		return nRet;
	}

	// If the path exists and is not an alias, return without changing
	// anything
	struct stat aFileStat;
	if ( !stat( path, &aFileStat ) )
	{
		if ( aFileStat.st_mode & S_IFREG )
		{
			NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

			NSString *pPath = macxp_resolveAliasImpl( path );
			if ( pPath )
			{
				const char *tmpPath = [pPath UTF8String];
				int nLen = strlen( tmpPath );
				if ( nLen < buflen )
					strcpy( path, tmpPath );
			}

			[pPool release];
		}

		return nRet;
	}

	// Iterate through the directories from the top down and resolve any
	// aliases that might be encountered
	const char *unprocessedPath = path;
	if ( *unprocessedPath == '/' )
		unprocessedPath++;

	sal_Bool bContinue = sal_True;
	while ( bContinue && unprocessedPath && *unprocessedPath )
	{
		unprocessedPath = strchr( unprocessedPath, '/' );
		if ( !unprocessedPath )
			unprocessedPath = "";

		char *basePath = strdup( unprocessedPath );
		if ( !basePath )
			return nRet;
		path[ strlen( path ) - strlen( basePath ) ] = '\0';

		sal_Bool bModified = sal_False;
		if ( !stat( path, &aFileStat ) )
		{
			if ( aFileStat.st_mode & S_IFREG )
			{
				NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

				NSString *pPath = macxp_resolveAliasImpl( path );
				if ( pPath )
				{
					const char *tmpPath = [pPath UTF8String];
					int nLen = strlen( tmpPath ) + strlen( basePath );
					if ( nLen < buflen )
					{
						strcpy( path, tmpPath );
						strcat( path, basePath );
						bModified = sal_True;
					}
					else
					{
						errno = ENAMETOOLONG;
						nRet = -1;
						bContinue = sal_False;
					}
				}

				[pPool release];
			}
		}
		else
		{
			 bContinue = sal_False;
		}

		if ( !bModified )
			strcat( path, basePath );
		unprocessedPath = path + strlen( path ) - strlen( basePath );
		if ( *unprocessedPath == '/' )
			unprocessedPath++;
		free( basePath );
	}

	return nRet;
}

sal_Bool macxp_getNSHomeDirectory(char *path, int buflen)
{
	sal_Bool bRet = sal_False;

	if ( path && buflen > 0 )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		// NSHomeDirectory() returns the application's container directory
		// in sandboxed applications
		NSString *pHomeDir = NSHomeDirectory();
		if ( pHomeDir )
		{
			NSURL *pURL = [NSURL fileURLWithPath:pHomeDir];
			if ( pURL )
				pURL = [pURL URLByStandardizingPath];
			if ( pURL )
				pURL = [pURL URLByResolvingSymlinksInPath];
			if ( pURL )
			{
				NSString *pHomeDir = [pURL path];
				if ( pHomeDir )
				{
					const char *pHomeDirStr = [pHomeDir UTF8String];
					if ( pHomeDirStr )
					{
						int nLen = strlen( pHomeDirStr );
						if ( nLen < buflen )
						{
							strcpy( path, pHomeDirStr );
							bRet = sal_True;
						}
					}
				}
			}
		}

		[pPool release];
	}

	return bRet;
}

void macxp_setFileType(const sal_Char* path)
{
#ifdef PRODUCT_FILETYPE
	if ( path && strlen( path ) )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSString *pPath = [NSString stringWithUTF8String:path];
		NSFileManager *pFileManager = [NSFileManager defaultManager];
		if ( pPath && pFileManager )
		{
			NSDictionary *pAttributes = [pFileManager attributesOfItemAtPath:pPath error:nil];
			if ( !pAttributes || ![pAttributes fileHFSTypeCode] )
			{
				pAttributes = [NSDictionary dictionaryWithObject:[NSNumber numberWithUnsignedLong:(unsigned long)PRODUCT_FILETYPE] forKey:NSFileHFSTypeCode];
				if ( pAttributes )
					[pFileManager setAttributes:pAttributes ofItemAtPath:pPath error:nil];
			}
		}

		[pPool release];
	}
#endif	// PRODUCT_FILETYPE
}
