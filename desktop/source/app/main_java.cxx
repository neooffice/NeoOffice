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
 *  Patrick Luby, November 2008
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2008 Planamesa Inc.
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

#ifdef USE_JAVA

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "app.hxx"

#include <errno.h>
#include <stdio.h>

#include "sal/main.h"
#include <tools/fsys.hxx>
#include <vos/module.hxx>

#include <premac.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>

#include "main_java_cocoa.hxx"

#define TMPDIR "/tmp"

typedef int SofficeMain_Type();

using namespace rtl;
using namespace vos;

// -=-= main() -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// All references to main() need to be redefined to private_main()
#define main private_main
SAL_IMPLEMENT_MAIN_WITH_ARGS( argc, argv ) {
	// Dynamically load soffice_main symbol to improve startup speed
	OModule aSofficeMainModule;
	if ( aSofficeMainModule.load( OUString( RTL_CONSTASCII_USTRINGPARAM( "libsofficeapp.dylib" ) ) ) )
	{
		SofficeMain_Type *pSofficeMain = (SofficeMain_Type *)aSofficeMainModule.getSymbol( OUString( RTL_CONSTASCII_USTRINGPARAM( "soffice_main" ) ) );
		if ( pSofficeMain )
			return pSofficeMain();
	}

    return 0;
}
#undef main

extern "C" int java_main( int argc, char **argv )
{
    char *pCmdPath = argv[ 0 ];

    // Don't allow running as root as we really cannot trust that we won't
    // do any accidental damage
    if ( getuid() == 0 )
    {
        fprintf( stderr, "%s: running as root user is not allowed\n", argv[ 0 ] );
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

    // Fix bug 3631 by setting the temporary directory to something other
    // than /tmp if we can since Mac OS X will clear out the /tmp directory
    // periodically
    OString aTmpDir( GetNSTemporaryDirectory() );
    if ( !aTmpDir.getLength() )
        aTmpDir = OString( TMPDIR );
  	OString aTmpEnv( "TMPDIR=" );
    aTmpEnv += aTmpDir;
    putenv( (char *)aTmpEnv.getStr() );
  	aTmpEnv = OString( "TMP=" );
    aTmpEnv += aTmpDir;
    putenv( (char *)aTmpEnv.getStr() );
  	aTmpEnv = OString( "TEMP=" );
    aTmpEnv += aTmpDir;
    putenv( (char *)aTmpEnv.getStr() );

    // Get absolute path of command's directory
    OString aCmdPath( pCmdPath );
    if ( aCmdPath.getLength() )
    {
        DirEntry aCmdDirEntry( aCmdPath );
        aCmdDirEntry.ToAbs();
        aCmdPath = OUStringToOString( OUString( aCmdDirEntry.GetPath().GetFull().GetBuffer() ), RTL_TEXTENCODING_UTF8 );
    }

    // Unset the CLASSPATH environment variable
    unsetenv( "CLASSPATH" );

    // Assign command's directory to PATH environment variable
    OString aPath( getenv( "PATH" ) );
    OString aStandardPath( aCmdPath );
    aStandardPath += OString( ":" );
    aStandardPath += aCmdPath;
    aStandardPath += OString( "/../basis-link/program:" );
    aStandardPath += aCmdPath;
    aStandardPath += OString( "/../basis-link/ure-link/bin:/bin:/sbin:/usr/bin:/usr/sbin:" );
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
    aStandardLibPath += OString( ":" );
    aStandardLibPath += aCmdPath;
    aStandardLibPath += OString( "/../basis-link/program:" );
    aStandardLibPath += aCmdPath;
    aStandardLibPath += OString( "/../basis-link/ure-link/lib:/usr/lib:/usr/local/lib:" );
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

    // Use default launch options if there are no application arguments
    if ( argc < 2 || ( argc == 2 && !strncmp( "-psn", argv[ 1 ], 4 ) ) )
    {
        CFPropertyListRef aPref = CFPreferencesCopyAppValue( CFSTR( "DefaultLaunchOptions" ), kCFPreferencesCurrentApplication );
        if ( aPref )
        {
            if ( CFGetTypeID( aPref ) == CFStringGetTypeID() )
            {
                char **pNewArgv = (char **)rtl_allocateMemory( sizeof( char** ) * ( argc + 2 ) );
                memcpy( pNewArgv, argv, sizeof( char** ) * argc );

                CFIndex nLen = CFStringGetLength( (CFStringRef)aPref );
                CFRange aRange = CFRangeMake( 0, nLen );
                sal_Unicode pBuffer[ nLen + 1 ];
                CFStringGetCharacters( (CFStringRef)aPref, aRange, pBuffer );
                pBuffer[ nLen ] = 0;
                OUString aOption( pBuffer );
                pNewArgv[ argc ] = (char *)OUStringToOString( aOption, RTL_TEXTENCODING_UTF8 ).getStr();
                if ( pNewArgv[ argc ] )
                {
                    pNewArgv[ argc ] = strdup( pNewArgv[ argc ] );
                    argc++;
                }

                pNewArgv[ argc ] = NULL;
                argv = pNewArgv;
                bRestart = true;
            }
            else if ( CFGetTypeID( aPref ) == CFArrayGetTypeID() )
            {
                CFIndex nArrayLen = CFArrayGetCount( (CFArrayRef)aPref );
                char **pNewArgv = (char **)rtl_allocateMemory( sizeof( char** ) * ( argc + nArrayLen + 1 ) );
                memcpy( pNewArgv, argv, sizeof( char** ) * argc );

                for ( int i = 0; i < nArrayLen; i++ )
                {
                    CFStringRef aElement = (CFStringRef)CFArrayGetValueAtIndex( (CFArrayRef)aPref, i );
                    if ( aElement )
                    {
                        CFIndex nLen = CFStringGetLength( aElement );
                        CFRange aRange = CFRangeMake( 0, nLen );
                        sal_Unicode pBuffer[ nLen + 1 ];
                        CFStringGetCharacters( aElement, aRange, pBuffer );
                        pBuffer[ nLen ] = 0;
                        OUString aOption( pBuffer );
                        pNewArgv[ argc ] = (char *)OUStringToOString( aOption, RTL_TEXTENCODING_UTF8 ).getStr();
                        if ( pNewArgv[ argc ] )
                        {
                            pNewArgv[ argc ] = strdup( pNewArgv[ argc ] );
                            argc++;
                        }
                    }
                }

                pNewArgv[ argc ] = NULL;
                argv = pNewArgv;
                bRestart = true;
            }

            CFRelease( aPref );
        }
    }

    // Restart if necessary since most library path changes don't have any
    // effect after the application has already started on most platforms
    if ( bRestart )
    {
		OString aPageinPath( aCmdPath );
		aPageinPath += OString( "/../basis-link/program/pagein" );
		char *pPageinPath = (char *)aPageinPath.getStr();
		if ( !access( pPageinPath, R_OK | X_OK ) )
		{
			int nCurrentArg = 0;
			char *pPageinArgs[ argc + 3 ];
			pPageinArgs[ nCurrentArg++ ] = pPageinPath;
			OString aPageinSearchArg( "-L" );
			aPageinSearchArg += aCmdPath;
			aPageinSearchArg += OString( "/../basis-link/program" );
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
        fprintf( stderr, "%s: execv() function failed with error %i\n", argv[ 0 ], errno );
        _exit( 1 );
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
	// Fix bug 2394 by turning off shared memory. Note that Mono is picky and
	// so the value must be set to "yes" to actually disable shared memory.
	aTmpPath = OString( "MONO_DISABLE_SHM=yes" );
	putenv( (char *)aTmpPath.getStr() );

	return private_main( argc, argv );
}

#endif	// USE_JAVA
