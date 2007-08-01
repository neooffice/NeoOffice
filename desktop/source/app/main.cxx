/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU General Public License Version 2.1.
 *
 *
 *	GNU General Public License Version 2.1
 *	=============================================
 *	Copyright 2005 by Sun Microsystems, Inc.
 *	901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public
 *	License version 2.1, as published by the Free Software Foundation.
 *
 *	This library is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public
 *	License along with this library; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *	MA  02111-1307  USA
 *
 *	Modified March 2006 by Patrick Luby. NeoOffice is distributed under
 *	GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#if defined(UNX)
#if defined(MACOSX) || defined(FREEBSD)
#include <sys/types.h>
#include <sys/time.h>
#endif /* MACOSX || FREEBSD */

#include <sys/resource.h>

#ifdef USE_JAVA

#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _DESKTOPX11PRODUCTCHECK_HXX
#include "X11productcheck.hxx"
#endif

#include "main_cocoa.h"

#define TMPDIR "/tmp"

#endif	// USE_JAVA

#define UNLIMIT_DESCRIPTORS() \
{ \
	rlimit aLimit; \
	if ( !getrlimit( RLIMIT_NOFILE, &aLimit ) ) \
	{ \
		aLimit.rlim_cur = aLimit.rlim_max;   \
		setrlimit( RLIMIT_NOFILE, &aLimit ); \
	} \
}

#else  /* ! UNX */
#define UNLIMIT_DESCRIPTORS()
#endif /* ! UNX */

#ifndef _SAL_MAIN_H_
#include "sal/main.h"
#endif

#include "app.hxx"

#include <rtl/logfile.hxx>


BOOL SVMain();

// -=-= main() -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifdef USE_JAVA
SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
#else	// USE_JAVA
SAL_IMPLEMENT_MAIN_WITH_ARGS(EMPTYARG, EMPTYARG)
#endif	// USE_JAVA
{
#ifdef USE_JAVA
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
	{
		ByteString aPageinPath( aCmdPath );
		aPageinPath += ByteString( "/pagein" );
		char *pPageinPath = (char *)aPageinPath.GetBuffer();
		if ( !access( pPageinPath, R_OK | X_OK ) )
		{
			int nCurrentArg = 0;
			char *pPageinArgs[ argc + 3 ];
			pPageinArgs[ nCurrentArg++ ] = pPageinPath;
			ByteString aPageinSearchArg( "-L" );
			aPageinSearchArg += aCmdPath;
			pPageinArgs[ nCurrentArg++ ] = (char *)aPageinSearchArg.GetBuffer();
			for ( int i = 1; i < argc; i++ )
			{
				if ( !strcmp( "-calc", argv[ i ] ) )
					pPageinArgs[ nCurrentArg++ ] = "@pagein-calc";
				else if ( !strcmp( "-draw", argv[ i ] ) )
					pPageinArgs[ nCurrentArg++ ] = "@pagein-draw";
				else if ( !strcmp( "-impress", argv[ i ] ) )
					pPageinArgs[ nCurrentArg++ ] = "@pagein-impress";
				else if ( !strcmp( "-writer", argv[ i ] ) )
					pPageinArgs[ nCurrentArg++ ] = "@pagein-writer";
			}
			if ( nCurrentArg == 1 )
				pPageinArgs[ nCurrentArg++ ] = "@pagein-writer";
			pPageinArgs[ nCurrentArg++ ] = "@pagein-common";
			pPageinArgs[ nCurrentArg++ ] = NULL;

			// Execute the pagein command in child process
			pid_t pid = fork();
			if ( !pid )
			{
				close( 0 );
				execvp( pPageinPath, pPageinArgs );
				_exit( 1 );
			}
			else if ( pid > 0 )
			{
				// Invoke waitpid to prevent zombie processes
				int status;
				while ( waitpid( pid, &status, 0 ) > 0 && EINTR == errno )
					;
			}
		}

		// Reexecute the parent process
		execv( pCmdPath, argv );
	}

	// File locking is enabled by default
	putenv( "SAL_ENABLE_FILE_LOCKING=1" );

	// Set Mozilla environment variables
	ByteString aTmpPath( "OPENOFFICE_MOZILLA_FIVE_HOME=" );
	aTmpPath += aCmdPath;
	putenv( (char *)aTmpPath.GetBuffer() );

	// Set Mono environment variables
	aTmpPath = ByteString( "MONO_ROOT=" );
	aTmpPath += aCmdPath;
	putenv( (char *)aTmpPath.GetBuffer() );
	aTmpPath = ByteString( "MONO_CFG_DIR=" );
	aTmpPath += aCmdPath;
	putenv( (char *)aTmpPath.GetBuffer() );
	aTmpPath = ByteString( "MONO_CONFIG=" );
	aTmpPath += aCmdPath;
	aTmpPath += ByteString( "/mono/2.0/machine.config" );
	putenv( (char *)aTmpPath.GetBuffer() );
	// Unset MONO_DISABLE_SHM variable as OdfConverter will abort with it set
	aTmpPath = ByteString( "MONO_DISABLE_SHM=" );
	putenv( (char *)aTmpPath.GetBuffer() );
#endif	// USE_JAVA

	RTL_LOGFILE_PRODUCT_TRACE( "PERFORMANCE - enter Main()" );
	UNLIMIT_DESCRIPTORS();

	desktop::Desktop aDesktop;

#ifdef USE_JAVA
	// If this is an X11 product, we need to explicitly start the NSApplication
	// event dispatching before SVMain() creates and runs the OOo code in a
	// secondary thread
	if ( ::desktop::IsX11Product() )
	{
		CFRunLoopTimerRef aTimer = CFRunLoopTimerCreate( NULL, CFAbsoluteTimeGetCurrent(), 0, 0, 0, NSApplication_run, NULL );
		if ( aTimer )
			CFRunLoopAddTimer( CFRunLoopGetCurrent(), aTimer, kCFRunLoopDefaultMode );
	}
#endif	// USE_JAVA

    SVMain();

#ifdef USE_JAVA
    // Force exit since some JVMs won't shutdown when only exit() is invoked
    _exit( 0 );
#else	// USE_JAVA
    return 0;
#endif	// USE_JAVA
}
