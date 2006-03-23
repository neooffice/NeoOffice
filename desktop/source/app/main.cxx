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
 *    GNU General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *    Modified March 2006 by Patrick Luby. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

#if defined(UNX)
#if defined(MACOSX) || defined(FREEBSD)
#include <sys/types.h>
#include <sys/time.h>
#endif /* MACOSX || FREEBSD */

#include <sys/resource.h>

#ifdef USE_JAVA

#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif

#include <stdio.h>
#include <unistd.h>

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

SAL_IMPLEMENT_MAIN()
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
    aStandardPath += ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_UTF8 );
    aStandardPath += ByteString( "/bin:/sbin:/usr/bin:/usr/sbin" );
    if ( aPath.CompareTo( aStandardPath, aStandardPath.Len() ) != COMPARE_EQUAL || ( aPath.Len() > aStandardPath.Len() && aPath.GetChar( aStandardPath.Len() ) != ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_UTF8 ).GetChar( 0 ) ) )
    {
        ByteString aTmpPath( "PATH=" );
        aTmpPath += aStandardPath;
        if ( aPath.Len() )
        {
            aTmpPath += ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_UTF8 );
            aTmpPath += aPath;
        }
        putenv( (char *)aTmpPath.GetBuffer() );
    }

    // Assign command's directory and DYLD_LIBRARY_PATH to
    // DYLD_FALLBACK_LIBRARY_PATH. Also, fix bug 1198 and eliminate
    // "libzip.jnilib not found" crashes by unsetting DYLD_FRAMEWORK_PATH and
    // DYLD_LIBRARY_PATH.
    bool bRestart = false;
    char *pFrameworkPath = getenv( "DYLD_FRAMEWORK_PATH" );
    if ( pFrameworkPath )
    {
        ByteString aFrameworkPath( pFrameworkPath );
        ByteString aFallbackFrameworkPath( getenv( "DYLD_FALLBACK_LIBRARY_PATH" ) );
        if ( aFallbackFrameworkPath.Len() )
        {
            aFrameworkPath += ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_UTF8 );
            aFrameworkPath += aFallbackFrameworkPath;
        }
        if ( aFrameworkPath.Len() )
        {
            ByteString aTmpPath( "DYLD_FALLBACK_FRAMEWORK_PATH=" );
            aTmpPath += ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_UTF8 );
            aTmpPath += aFrameworkPath;
            putenv( (char *)aTmpPath.GetBuffer() );
        }
        unsetenv( "DYLD_FRAMEWORK_PATH" );
        bRestart = true;
    }
    char *pLibPath = getenv( "DYLD_LIBRARY_PATH" );
    ByteString aFallbackLibPath( getenv( "DYLD_FALLBACK_LIBRARY_PATH" ) );
    if ( pLibPath || aFallbackLibPath.CompareTo( aCmdPath, aCmdPath.Len() ) != COMPARE_EQUAL || ( aFallbackLibPath.Len() > aCmdPath.Len() && aFallbackLibPath.GetChar( aCmdPath.Len() ) != ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_UTF8 ).GetChar( 0 ) ) )
    {
        ByteString aTmpPath( "DYLD_FALLBACK_LIBRARY_PATH=" );
        aTmpPath += aCmdPath;
        ByteString aLibPath( pLibPath );
        if ( aLibPath.Len() )
        {
            aTmpPath += ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_UTF8 );
            aTmpPath += aLibPath;
        }
        if ( aFallbackLibPath.Len() )
        {
            aTmpPath += ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_UTF8 );
            aTmpPath += aFallbackLibPath;
        }
        putenv( (char *)aTmpPath.GetBuffer() );
        unsetenv( "DYLD_LIBRARY_PATH" );
        bRestart = true;
    }

    // Restart if necessary since most library path changes don't have any
    // effect after the application has already started on most platforms
    if ( bRestart )
        execv( pCmdPath, argv );

    // File locking is enabled by default
    putenv( "SAL_ENABLE_FILE_LOCKING=1" );

    // Set Mozilla environment variables
    ByteString aTmpPath( "OPENOFFICE_MOZILLA_FIVE_HOME=" );
    aTmpPath += aCmdPath;
    putenv( (char *)aTmpPath.GetBuffer() );
#endif	// USE_JAVA

    RTL_LOGFILE_PRODUCT_TRACE( "PERFORMANCE - enter Main()" );
    UNLIMIT_DESCRIPTORS();

    desktop::Desktop aDesktop;
    SVMain();

#ifdef USE_JAVA
    // Force exit since some JVMs won't shutdown when only exit() is invoked
    _exit( 0 );
#else	// USE_JAVA
    return 0;
#endif	// USE_JAVA
}
