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
			const char *pHomeDirStr = [pHomeDir UTF8String];
			if ( pHomeDirStr && strlen( pHomeDirStr ) )
			{
				strncpy( path, pHomeDirStr, buflen );

				// Remove trailing directory separator
				size_t len;
				while ( ( len = strlen( path ) ) && path[ len - 1 ] == '/' )
					path[ len - 1 ] = '\0';

				bRet = sal_True;
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
