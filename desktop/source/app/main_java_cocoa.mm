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
 *  Patrick Luby, August 2010
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2010 Planamesa Inc.
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

#include <Foundation/Foundation.h>
#include <crt_externs.h>
#include "main_java_cocoa.hxx"

using namespace rtl;

OString GetNSTemporaryDirectory()
{
	OString aRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSString *pTempDir = nil;

	// Use NSCachesDirectory to stay within FileVault encryption
	NSArray *pCachePaths = NSSearchPathForDirectoriesInDomains( NSCachesDirectory, NSUserDomainMask, YES );
	if ( pCachePaths )
	{
 		unsigned int nCount = [pCachePaths count];
 		unsigned int i = 0;
		for ( ; i < nCount && !pTempDir; i++ )
		{
			BOOL bDir = NO;
			NSString *pCachePath = (NSString *)[pCachePaths objectAtIndex:i];
			if ( [[NSFileManager defaultManager] fileExistsAtPath:pCachePath isDirectory:&bDir] && bDir )
			{
				// Append program name to cache path
				char **pProgName = _NSGetProgname();
				if ( pProgName && *pProgName )
				{
					pCachePath = [pCachePath stringByAppendingPathComponent:[NSString stringWithUTF8String:(const char *)*pProgName]];
					bDir = NO;
					if ( [[NSFileManager defaultManager] fileExistsAtPath:pCachePath isDirectory:&bDir] && bDir )
					{
						pTempDir = pCachePath;
						break;
					}
					else if ( [[NSFileManager defaultManager] createDirectoryAtPath:pCachePath attributes:nil] )
					{
						pTempDir = pCachePath;
						break;
					}
				}
			}
		}
	}

	if ( !pTempDir )
	{
		pTempDir = NSTemporaryDirectory();
		if ( !pTempDir )
			pTempDir = @"/tmp";
	}

	const char *pUTFTempDir = [pTempDir UTF8String];
	if ( pUTFTempDir )
		aRet = OString( pUTFTempDir );

	[pPool release];

	return aRet;
}
