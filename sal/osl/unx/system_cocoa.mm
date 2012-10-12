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

typedef int chmod_Type(const char *path, mode_t mode);
typedef int link_Type(const char *path1, const char *path2);
typedef int mkdir_Type(const char *path, mode_t mode);
typedef int open_Type(const char *path, int oflag, ...);
typedef DIR *opendir_Type(const char *dirname);
typedef int rename_Type(const char *oldpath, const char *newpath);
typedef int rmdir_Type(const char *path);
typedef int symlink_Type(const char *path1, const char *path2);
typedef int unlink_Type(const char *path);

static chmod_Type *pChmod = NULL;
static link_Type *pLink = NULL;
static mkdir_Type *pMkdir = NULL;
static open_Type *pOpen = NULL;
static opendir_Type *pOpendir = NULL;
static rename_Type *pRename = NULL;
static rmdir_Type *pRmdir = NULL;
static symlink_Type *pSymlink = NULL;
static unlink_Type *pUnlink = NULL;

extern "C" int chmod(const char *path, mode_t mode)
{
	int nRet = -1;

	if ( !pChmod )
		pChmod = (chmod_Type *)dlsym( RTLD_NEXT, "chmod");

	if ( pChmod )
		nRet = pChmod( path, mode );
	else
		errno = EFAULT;

	return nRet;
}

extern "C" int link(const char *path1, const char *path2)
{
	int nRet = -1;

	if ( !pLink )
		pLink = (link_Type *)dlsym( RTLD_NEXT, "link");

	if ( pLink )
		nRet = pLink( path1, path2 );
	else
		errno = EFAULT;

	return nRet;
}

extern "C" int mkdir(const char *path, mode_t mode)
{
	int nRet = -1;

	if ( !pMkdir )
		pMkdir = (mkdir_Type *)dlsym( RTLD_NEXT, "mkdir");

	if ( pMkdir )
		nRet = pMkdir( path, mode );
	else
		errno = EFAULT;

	return nRet;
}

extern "C" int open(const char *path, int oflag, ...)
{
	int nRet = -1;

	if ( !pOpen )
		pOpen = (open_Type *)dlsym( RTLD_NEXT, "open" );

	if ( pOpen )
	{
		if ( oflag & O_CREAT )
		{
			va_list argp;
			va_start( argp, oflag );
			nRet = pOpen( path, oflag, (mode_t)va_arg( argp, int ) );
			va_end( argp );
		}
		else
		{
			nRet = pOpen( path, oflag );
		}
	}
	else
	{
		errno = EFAULT;
	}

	return nRet;
}

extern "C" DIR *opendir(const char *dirname)
{
	DIR *pRet = NULL;

	if ( !pOpendir )
		pOpendir = (opendir_Type *)dlsym( RTLD_NEXT, "opendir");

	if ( pOpendir )
		pRet = pOpendir( dirname );

	return pRet;
}

extern "C" int rename(const char *oldpath, const char *newpath)
{
	int nRet = -1;

	if ( !pRename )
		pRename = (rename_Type *)dlsym( RTLD_NEXT, "rename");

	if ( pRename )
		nRet = pRename( oldpath, newpath );
	else
		errno = EFAULT;

	return nRet;
}

extern "C" int rmdir(const char *path)
{
	int nRet = -1;

	if ( !pRmdir )
		pRmdir = (rmdir_Type *)dlsym( RTLD_NEXT, "rmdir");

	if ( pRmdir )
		nRet = pRmdir( path );
	else
		errno = EFAULT;

	return nRet;
}

extern "C" int symlink(const char *path1, const char *path2)
{
	int nRet = -1;

	if ( !pSymlink )
		pSymlink = (symlink_Type *)dlsym( RTLD_NEXT, "symlink");

	if ( pSymlink )
		nRet = pSymlink( path1, path2 );
	else
		errno = EFAULT;

	return nRet;
}

extern "C" int unlink(const char *path)
{
	int nRet = -1;

	if ( !pUnlink )
		pUnlink = (unlink_Type *)dlsym( RTLD_NEXT, "unlink");

	if ( pUnlink )
		nRet = pUnlink( path );
	else
		errno = EFAULT;

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
