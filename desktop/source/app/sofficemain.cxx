/*************************************************************************
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of NeoOffice.
 *
 * NeoOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * NeoOffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 3 along with NeoOffice.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 * Modified August 2008 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "app.hxx"

#include <rtl/logfile.hxx>
#include <tools/extendapplicationenvironment.hxx>

#ifdef USE_JAVA

#include <errno.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif

#define TMPDIR "/tmp"

using namespace rtl;
using namespace vos;

#endif	// USE_JAVA

BOOL SVMain();

// -=-= main() -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifdef USE_JAVA
extern "C" int soffice_main( int argc, char **argv )
{
    char *pCmdPath = argv[ 0 ];

    // Don't allow running as root as we really cannot trust that we won't
    // do any accidental damage
    if ( getuid() == 0 )
    {
        fprintf( stderr, "%s: running as root user is not allowed\n", argv[ 0 ]  );
        _exit( 1 );
    }

  	// Fix bug 3182 by detecting incorrectly formatted HOME values
  	OString aHomePath( getenv( "HOME" ) );
  	if ( aHomePath.getLength() )
  	{
  		// Make path absolute
  		if ( aHomePath.getStr()[0] != '/' )
  			aHomePath = OString( "/" ) + aHomePath;
  		// Trim any trailing '/' characters
  		sal_Int32 i = aHomePath.getLength() - 1;
  		while ( i && aHomePath.getStr()[ i ] == '/' )
  			i--;
  		aHomePath = aHomePath.copy( 0, i + 1 );
  
  		OString aTmpPath( "HOME=" );
  		aTmpPath += aHomePath;
  		putenv( (char *)aTmpPath.getStr() );
  	}

    // Make sure TMPDIR exists as a softlink to /private/tmp as it can be
    // easily removed. In most cases, this call should fail, but we do it
    // just to be sure.
    symlink( "private/tmp", TMPDIR );

    // If TMPDIR is not set, set it to /tmp
    if ( !getenv( "TMPDIR" ) )
        putenv( "TMPDIR=" TMPDIR );
    if ( !getenv( "TMP" ) )
        putenv( "TMP=" TMPDIR );
    if ( !getenv( "TEMP" ) )
        putenv( "TEMP=" TMPDIR );

    // Get absolute path of command's directory
    OString aCmdPath( pCmdPath );
    if ( aCmdPath.getLength() )
    {
        DirEntry aCmdDirEntry( aCmdPath );
        aCmdDirEntry.ToAbs();
        aCmdPath = OUStringToOString( OUString( aCmdDirEntry.GetPath().GetFull().GetBuffer() ), RTL_TEXTENCODING_UTF8 );
    }

    // Assign command's directory to PATH environment variable
    OString aPath( getenv( "PATH" ) );
    OString aStandardPath( aCmdPath );
    aStandardPath += OString( ":/bin:/sbin:/usr/bin:/usr/sbin:" );
    if ( aPath.compareTo( aStandardPath, aStandardPath.getLength() ) )
    {
        OString aTmpPath( "PATH=" );
        aTmpPath += aStandardPath;
        if ( aPath.getLength() )
        {
            aTmpPath += OString( ":" );
            aTmpPath += aPath;
        }
        putenv( (char *)aTmpPath.getStr() );
    }

    // Fix bug 1198 and eliminate "libzip.jnilib not found" crashes by
    // unsetting DYLD_FRAMEWORK_PATH
    bool bRestart = false;
    OString aFrameworkPath( getenv( "DYLD_FRAMEWORK_PATH" ) );
    // Always unset DYLD_FRAMEWORK_PATH
    unsetenv( "DYLD_FRAMEWORK_PATH" );
    if ( aFrameworkPath.getLength() )
    {
        OString aFallbackFrameworkPath( getenv( "DYLD_FALLBACK_FRAMEWORK_PATH" ) );
        if ( aFallbackFrameworkPath.getLength() )
        {
            aFrameworkPath += OString( ":" );
            aFrameworkPath += aFallbackFrameworkPath;
        }
        if ( aFrameworkPath.getLength() )
        {
            OString aTmpPath( "DYLD_FALLBACK_FRAMEWORK_PATH=" );
            aTmpPath += aFrameworkPath;
            putenv( (char *)aTmpPath.getStr() );
        }
        bRestart = true;
    }

    OString aStandardLibPath( aCmdPath );
    aStandardLibPath += OString( ":/usr/lib:/usr/local/lib:" );
    if ( aHomePath.getLength() )
    {
        aStandardLibPath += aHomePath;
        aStandardLibPath += OString( "/lib:" );
    }
    OString aLibPath( getenv( "LD_LIBRARY_PATH" ) );
    OString aDyLibPath( getenv( "DYLD_LIBRARY_PATH" ) );
    OString aDyFallbackLibPath( getenv( "DYLD_FALLBACK_LIBRARY_PATH" ) );
    // Always unset LD_LIBRARY_PATH and DYLD_LIBRARY_PATH
    unsetenv( "LD_LIBRARY_PATH" );
    unsetenv( "DYLD_LIBRARY_PATH" );
    if ( aDyFallbackLibPath.compareTo( aStandardLibPath, aStandardLibPath.getLength() ) )
    {
        OString aTmpPath( "DYLD_FALLBACK_LIBRARY_PATH=" );
        aTmpPath += aStandardLibPath;
        if ( aLibPath.getLength() )
        {
            aTmpPath += OString( ":" );
            aTmpPath += aLibPath;
        }
        if ( aDyLibPath.getLength() )
        {
            aTmpPath += OString( ":" );
            aTmpPath += aDyLibPath;
        }
        putenv( (char *)aTmpPath.getStr() );
        bRestart = true;
    }

    // Restart if necessary since most library path changes don't have any
    // effect after the application has already started on most platforms.
    // We have to set SAL_NO_FORCE_SYSALLOC to some value in order to turn
    // on OOo's custom memory manager which is, apparently, required for
    // XML export to work but it cannot be used before we invoke execv().
    if ( bRestart )
    {
        OString aPageinPath( aCmdPath );
        aPageinPath += OString( "/pagein" );
        char *pPageinPath = (char *)aPageinPath.getStr();
        if ( !access( pPageinPath, R_OK | X_OK ) )
        {
            int nCurrentArg = 0;
            char *pPageinArgs[ argc + 3 ];
            pPageinArgs[ nCurrentArg++ ] = pPageinPath;
            OString aPageinSearchArg( "-L" );
            aPageinSearchArg += aCmdPath;
            pPageinArgs[ nCurrentArg++ ] = (char *)aPageinSearchArg.getStr();
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
                    usleep( 10 );
            }
        }

        // Reexecute the parent process
        execv( pCmdPath, argv );
    }

    // File locking is enabled by default
    putenv( "SAL_ENABLE_FILE_LOCKING=1" );

    // Set Mono environment variables
    OString aTmpPath( "MONO_ROOT=" );
    aTmpPath += aCmdPath;
    putenv( (char *)aTmpPath.getStr() );
    aTmpPath = OString( "MONO_CFG_DIR=" );
    aTmpPath += aCmdPath;
    putenv( (char *)aTmpPath.getStr() );
    aTmpPath = OString( "MONO_CONFIG=" );
    aTmpPath += aCmdPath;
    aTmpPath += OString( "/mono/2.0/machine.config" );
    putenv( (char *)aTmpPath.getStr() );
    // Unset MONO_DISABLE_SHM variable as OdfConverter will abort with it set
    aTmpPath = OString( "MONO_DISABLE_SHM=" );
    putenv( (char *)aTmpPath.getStr() );

    // We need to fork and exec javaldx to properly create preferences the
    // first time or else some preferences won't be imported
    OUString aUserInstallPath;
    ::utl::Bootstrap::PathStatus aLocateResult = ::utl::Bootstrap::locateUserInstallation( aUserInstallPath );
    if ( aLocateResult != ::utl::Bootstrap::PATH_EXISTS )
    {
        OString aJavaldxPath( aCmdPath );
        aJavaldxPath += OString( "/javaldx" );
        char *pJavaldxPath = (char *)aJavaldxPath.getStr();
        if ( !access( pJavaldxPath, R_OK | X_OK ) )
        {
            char *pJavaldxArgs[ 2 ];
            pJavaldxArgs[ 0 ] = pJavaldxPath;
            pJavaldxArgs[ 1 ] = NULL;

            // Execute the javaldx command in child process
            pid_t pid = fork();
            if ( !pid )
            {
                close( 0 );
                close( 1 );
                execvp( pJavaldxPath, pJavaldxArgs );
                _exit( 1 );
            }
            else if ( pid > 0 )
            {
                // Invoke waitpid to prevent zombie processes
                int status;
                while ( waitpid( pid, &status, 0 ) > 0 && EINTR == errno )
                    usleep( 10 );
            }
        }
    }
#else	// USE_JAVA
extern "C" int soffice_main()
{
#endif	// USE_JAVA

    tools::extendApplicationEnvironment();

	RTL_LOGFILE_PRODUCT_TRACE( "PERFORMANCE - enter Main()" );

	desktop::Desktop aDesktop;
    // This string is used during initialization of the Gtk+ VCL module
    aDesktop.SetAppName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("soffice")) );
    SVMain();

#ifdef USE_JAVA
    // Force exit since some JVMs won't shutdown when only exit() is invoked
    _exit( 0 );
#else	// USE_JAVA
    return 0;
#endif	// USE_JAVA
}
