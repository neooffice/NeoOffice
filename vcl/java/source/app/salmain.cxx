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

#include <stdio.h>
#include <unistd.h>

#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif

#ifdef MACOSX
#include <crt_externs.h>
#endif

// ============================================================================

BEGIN_C

int main( int argc, char *argv[] )
{

	char *pCmdPath = argv[ 0 ];

#ifdef MACOSX
	// We need to use _NSGetEnviron() here to get the path of this executable
	// because argv[0] does not have any directory when the executable is
	// found in the user's PATH environment variable
	char **ppEnviron = NULL;
	if(_NSGetEnviron())
		ppEnviron = *_NSGetEnviron();

	// Get full executable path. We can't use __progname as that only holds
	// the name of the executable and not the path. The full executable path
	// is listed after the first NULL in *environ.
	if ( ppEnviron ) {
		char **ppTmp;
		ppTmp = ppEnviron;
		while ( *ppTmp++ )
			;
		pCmdPath = *ppTmp;
	}
#endif	// MACOSX

	// Get absolute path of command's directory
	ByteString aCmdPath( pCmdPath );
	if ( aCmdPath.Len() )
	{
		DirEntry aCmdDirEntry( aCmdPath );
		aCmdDirEntry.ToAbs();
		aCmdPath = ByteString( aCmdDirEntry.GetPath().GetFull(), gsl_getSystemTextEncoding() );
	}

	// Assign command's directory to PATH environment variable
	ByteString aPath( getenv( "PATH" ) );
	if ( aCmdPath.Len() )
	{
		ByteString aTmpPath( "PATH=" );
		aTmpPath += aCmdPath;
		if ( aPath.Len() )
		{
			aTmpPath += ByteString( DirEntry::GetSearchDelimiter(), gsl_getSystemTextEncoding() );
			aTmpPath += aPath;
		}
		putenv( aTmpPath.GetBuffer() );
	}

	// Assign command's directory to STAR_RESOURCEPATH environment variable
	ByteString aResPath( getenv( "STAR_RESOURCEPATH" ) );
	if ( aCmdPath.Len() )
	{
		ByteString aTmpPath( "STAR_RESOURCEPATH=" );
		aTmpPath += aCmdPath;
		if ( aResPath.Len() )
		{
			aTmpPath += ByteString( DirEntry::GetSearchDelimiter(), gsl_getSystemTextEncoding() );
			aTmpPath += aResPath;
		}
		putenv( aTmpPath.GetBuffer() );
	}

	// Assign command's directory to DYLD_LIBRARY_PATH environment variable
#ifdef MACOSX
	ByteString aLibPath( getenv( "DYLD_LIBRARY_PATH" ) );
#else
	ByteString aLibPath( getenv( "LD_LIBRARY_PATH" ) );
#endif	// MACOSX
	if ( aCmdPath.Len() )
	{
#ifdef MACOSX
		ByteString aTmpPath( "DYLD_LIBRARY_PATH=" );
#else
		ByteString aTmpPath( "LD_LIBRARY_PATH=" );
#endif	// MACOSX
		aTmpPath += aCmdPath;
		if ( aLibPath.Len() )
		{
			aTmpPath += ByteString( DirEntry::GetSearchDelimiter(), gsl_getSystemTextEncoding() );
			aTmpPath += aLibPath;
		}
		putenv( aTmpPath.GetBuffer() );
		// Restart if necessary since most library path changes don't have
		// any affect after the application has already started on most
		// platforms
		if ( aLibPath.GetToken( 0, DirEntry::GetSearchDelimiter().GetBuffer()[0] ).CompareTo( aCmdPath, aCmdPath.Len() ) != COMPARE_EQUAL )
			execv( pCmdPath, argv );
	}

	SVMain();

	exit( 0 );
}

END_C
