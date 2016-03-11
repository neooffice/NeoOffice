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
#define YES (BOOL)1
#endif
#ifdef NO
#undef NO
#define NO (BOOL)0
#endif

static NSURL *macxp_resolveAliasImpl( NSURL *url )
{
	NSURL *pRet = nil;

	if ( url )
	{
		NSData *pData = [NSURL bookmarkDataWithContentsOfURL:url error:nil];
		if ( pData )
		{
			BOOL bStale = NO;
			NSURL *pURL = [NSURL URLByResolvingBookmarkData:pData options:NSURLBookmarkResolutionWithoutUI | NSURLBookmarkResolutionWithoutMounting relativeToURL:nil bookmarkDataIsStale:&bStale error:nil];
			if ( !bStale && pURL )
			{
				pURL = [pURL URLByStandardizingPath];
				if ( pURL )
				{
					// Recurse to check if the URL is also an alias
					NSURL *pRecursedURL = macxp_resolveAliasImpl( pURL );
					if ( pRecursedURL )
						pRet = pRecursedURL;
					else
						pRet = pURL;
				}
			}
		}
	}

	return pRet;
}

int macxp_resolveAlias(char *path, unsigned int buflen, sal_Bool noResolveLastElement)
{
	int nRet = 0;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSString *pString = [NSString stringWithUTF8String:path];
	if ( pString )
	{
		NSURL *pURL = [NSURL fileURLWithPath:pString];
		if ( pURL )
		{
			pURL = [pURL URLByStandardizingPath];
			if ( pURL )
			{
				// Find lowest part of path that exists and see if it is an
				// alias
				NSURL *pTmpURL = pURL;
				while ( pTmpURL && ![pTmpURL checkResourceIsReachableAndReturnError:nil] )
				{
					NSURL *pOldTmpURL = pTmpURL;
					pTmpURL = [pTmpURL URLByDeletingLastPathComponent];
					if ( pTmpURL )
					{
						pTmpURL = [pTmpURL URLByStandardizingPath];
						if ( pTmpURL && [pTmpURL isEqual:pOldTmpURL] )
							pTmpURL = nil;
					}
				}

				if ( pTmpURL )
				{
					// We can skip checking if the URL is an alias if the
					// original URL exists and the noResolveLastElement
					// flag is true
					if ( noResolveLastElement && pTmpURL == pURL )
					{
						pURL = nil;
					}
					else
					{
						NSNumber *pAlias = nil;
						if ( [pTmpURL getResourceValue:&pAlias forKey:NSURLIsAliasFileKey error:nil] && pAlias && ![pAlias boolValue] )
							pURL = nil;
					}
            	}

				if ( pURL )
				{
					// Iterate through path and resolve any aliases
					NSArray *pPathComponents = [pURL pathComponents];
					if ( pPathComponents && [pPathComponents count] )
					{
						pURL = [NSURL fileURLWithPath:[pPathComponents objectAtIndex:0]];
						if ( pURL )
						{
							NSUInteger nCount = [pPathComponents count];
							NSUInteger i = 1;
							for ( ; i < nCount ; i++ )
							{
								pURL = [pURL URLByAppendingPathComponent:[pPathComponents objectAtIndex:i]];
								if ( !pURL || ( noResolveLastElement && i == nCount - 1 ) )
									break;

								NSNumber *pAlias = nil;
								if ( [pURL checkResourceIsReachableAndReturnError:nil] && [pURL getResourceValue:&pAlias forKey:NSURLIsAliasFileKey error:nil] && pAlias && [pAlias boolValue] )
								{
									NSURL *pResolvedURL = macxp_resolveAliasImpl( pURL );
									if ( pResolvedURL )
										pURL = pResolvedURL;
								}
							}

							if ( pURL )
							{
								NSString *pURLPath = [pURL path];
								if ( pURLPath )
								{
									const char *pURLPathString = [pURLPath UTF8String];
									if ( pURLPathString && strlen( pURLPathString ) < buflen )
									{
										strcpy( path, pURLPathString );
									}
									else
									{
										errno = ENAMETOOLONG;
										nRet = -1;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	[pPool release];

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
			{
				pURL = [pURL URLByStandardizingPath];
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

sal_Bool macxp_isUbiquitousPath(sal_Unicode *path, sal_Int32 len)
{
	sal_Bool bRet = sal_False;

	if ( path && len > 0 )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSFileManager *pFileManager = [NSFileManager defaultManager];
		if ( pFileManager )
		{
			NSString *pPath = [NSString stringWithCharacters:path length:len];
			if ( pPath && [pPath length] )
			{
				NSURL *pURL = [NSURL fileURLWithPath:pPath];
				if ( pURL && [pURL isFileURL] )
				{
					pURL = [pURL URLByStandardizingPath];
					if ( pURL )
					{
						pURL = [pURL URLByResolvingSymlinksInPath];
						if ( pURL )
						{
							// Don't call [NSFileManager isUbiquitousItemAtURL:]
							// as it will cause momentary hanging on OS X 10.9
							NSString *pURLPath = [pURL path];
							if ( pURLPath && [pURLPath length] )
							{
								NSArray *pLibraryFolders = NSSearchPathForDirectoriesInDomains( NSLibraryDirectory, NSUserDomainMask, NO );
								NSString *pRealHomeFolder = nil;
								struct passwd *pPasswd = getpwuid( getuid() );
								if ( pPasswd )
									pRealHomeFolder = [NSString stringWithUTF8String:pPasswd->pw_dir];
								if ( pLibraryFolders && pRealHomeFolder && [pRealHomeFolder length] )
								{
									NSUInteger nCount = [pLibraryFolders count];
									NSUInteger i = 0;
									for ( ; i < nCount; i++ )
									{
										NSString *pFolder = [pLibraryFolders objectAtIndex:i];
										if ( pFolder && [pFolder length] )
										{
											pFolder = [[pFolder stringByAppendingPathComponent:@"Mobile Documents"] stringByReplacingOccurrencesOfString:@"~" withString:pRealHomeFolder];
											if ( pFolder && [pFolder length] )
											{
												NSRange aRange = [pURLPath rangeOfString:pFolder];
												if ( !aRange.location && aRange.length )
													bRet = sal_True;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		[pPool release];
	}

	return bRet;
}
