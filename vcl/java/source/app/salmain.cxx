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
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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

#include <unistd.h>

#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif

// ============================================================================
 
BEGIN_C

int main( int argc, char *argv[] )
{

	char *pCmdPath = argv[ 0 ];

	// Don't allow running as root as we really cannot trust that we won't
	// do any accidental damage
	if ( getuid() == 0 )
	{
		fprintf( stderr, "%s: running as root user is not allowed\n", argv[ 0 ]  );
		_exit( 1 );
	}

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
		putenv( aTmpPath.GetBuffer() );
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
			putenv( aTmpPath.GetBuffer() );
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
		putenv( aTmpPath.GetBuffer() );
		unsetenv( "DYLD_LIBRARY_PATH" );
		bRestart = true;
	}

	// Restart if necessary since most library path changes don't have any
	// effect after the application has already started on most platforms
	if ( bRestart )
		execv( pCmdPath, argv );

	SVMain();

	// Force exit since some JVMs won't shutdown when only exit() is invoked
	_exit( 0 );
}

END_C
