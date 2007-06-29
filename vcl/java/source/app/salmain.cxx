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
 *		 - GNU General Public License Version 2.1
 *
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 Planamesa Inc.
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

#define _SV_SALMAIN_CXX

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifndef _SAL_MAIN_H_
#include "sal/main.h"
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif

#define TMPDIR "/tmp"

#define UNLIMIT_DESCRIPTORS() \
{ \
	rlimit aLimit; \
	if ( !getrlimit( RLIMIT_NOFILE, &aLimit ) ) \
	{ \
		aLimit.rlim_cur = aLimit.rlim_max;   \
		setrlimit( RLIMIT_NOFILE, &aLimit ); \
	} \
}

// ============================================================================
 
SAL_IMPLEMENT_MAIN()
{
	char *pCmdPath = argv[ 0 ];

	// Don't allow running as root as we really cannot trust that we won't
	// do any accidental damage
	if ( getuid() == 0 )
	{
		fprintf( stderr, "%s: running as root user is not allowed\n", argv[ 0 ]  );
		_exit( 1 );
	}

	// Make sure TMPDIR exists as a softlink to /private/tmp as it can be
	// easily removed. In most cases, this call should fail, but we do it
	// just to be sure.
	symlink( "private/tmp", TMPDIR );

	// If TMPDIR is not set, set it to /tmp
	if ( !getenv( "TMPDIR" ) )
		putenv( "TMPDIR=" TMPDIR );

	// Get absolute path of command's directory
	ByteString aCmdPath( pCmdPath );
	if ( aCmdPath.Len() )
	{
		DirEntry aCmdDirEntry( aCmdPath );
		aCmdDirEntry.ToAbs();
		aCmdPath = ByteString( aCmdDirEntry.GetPath().GetFull(), RTL_TEXTENCODING_UTF8 );
	}

	// Assign command's directory to PATH environment variable
	ByteString aPath( getenv( "PATH" ) );
	ByteString aStandardPath( aCmdPath );
	aStandardPath += ByteString( ":/bin:/sbin:/usr/bin:/usr/sbin:" );
	if ( aPath.CompareTo( aStandardPath, aStandardPath.Len() ) != COMPARE_EQUAL )
	{
		ByteString aTmpPath( "PATH=" );
		aTmpPath += aStandardPath;
		if ( aPath.Len() )
		{
			aTmpPath += ByteString( ":" );
			aTmpPath += aPath;
		}
		putenv( (char *)aTmpPath.GetBuffer() );
	}

	// Fix bug 1198 and eliminate "libzip.jnilib not found" crashes by
	// unsetting DYLD_FRAMEWORK_PATH
	bool bRestart = false;
	ByteString aFrameworkPath( getenv( "DYLD_FRAMEWORK_PATH" ) );
	// Always unset DYLD_FRAMEWORK_PATH
	unsetenv( "DYLD_FRAMEWORK_PATH" );
	if ( aFrameworkPath.Len() )
	{
		ByteString aFallbackFrameworkPath( getenv( "DYLD_FALLBACK_FRAMEWORK_PATH" ) );
		if ( aFallbackFrameworkPath.Len() )
		{
			aFrameworkPath += ByteString( ":" );
			aFrameworkPath += aFallbackFrameworkPath;
		}
		if ( aFrameworkPath.Len() )
		{
			ByteString aTmpPath( "DYLD_FALLBACK_FRAMEWORK_PATH=" );
			aTmpPath += aFrameworkPath;
			putenv( (char *)aTmpPath.GetBuffer() );
		}
		bRestart = true;
	}

	ByteString aStandardLibPath( aCmdPath );
	aStandardLibPath += ByteString( ":/usr/lib:/usr/local/lib:" );
	ByteString aHomePath( getenv( "HOME" ) );
	if ( aHomePath.Len() )
	{
		aStandardLibPath += aHomePath;
		aStandardLibPath += ByteString( "/lib:" );
	}
	ByteString aLibPath( getenv( "LD_LIBRARY_PATH" ) );
	ByteString aDyLibPath( getenv( "DYLD_LIBRARY_PATH" ) );
	ByteString aDyFallbackLibPath( getenv( "DYLD_FALLBACK_LIBRARY_PATH" ) );
	// Always unset LD_LIBRARY_PATH and DYLD_LIBRARY_PATH
	unsetenv( "LD_LIBRARY_PATH" );
	unsetenv( "DYLD_LIBRARY_PATH" );
	if ( aDyFallbackLibPath.CompareTo( aStandardLibPath, aStandardLibPath.Len() ) != COMPARE_EQUAL )
	{
		ByteString aTmpPath( "DYLD_FALLBACK_LIBRARY_PATH=" );
		aTmpPath += aStandardLibPath;
		if ( aLibPath.Len() )
		{
			aTmpPath += ByteString( ":" );
			aTmpPath += aLibPath;
		}
		if ( aDyLibPath.Len() )
		{
			aTmpPath += ByteString( ":" );
			aTmpPath += aDyLibPath;
		}
		putenv( (char *)aTmpPath.GetBuffer() );
		bRestart = true;
	}

	// Restart if necessary since most library path changes don't have any
	// effect after the application has already started on most platforms.
	// We have to set SAL_NO_FORCE_SYSALLOC to some value in order to turn
	// on OOo's custom memory manager which is, apparently, required for
	// XML export to work but it cannot be used before we invoke execv().
	if ( bRestart )
		execv( pCmdPath, argv );

	UNLIMIT_DESCRIPTORS();

	SVMain();

	// Force exit since some JVMs won't shutdown when only exit() is invoked
	_exit( 0 );
}
