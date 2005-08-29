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
	if ( aCmdPath.Len() )
	{
		ByteString aTmpPath( "PATH=" );
		aTmpPath += aCmdPath;
		if ( aPath.Len() )
		{
			aTmpPath += ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_UTF8 );
			aTmpPath += aPath;
		}
		putenv( aTmpPath.GetBuffer() );
	}

	// Assign command's directory to DYLD_LIBRARY_PATH environment variable
	ByteString aLibPath( getenv( "DYLD_LIBRARY_PATH" ) );
	if ( aCmdPath.Len() )
	{
		ByteString aTmpPath( "DYLD_LIBRARY_PATH=" );
		aTmpPath += aCmdPath;
		if ( aLibPath.Len() )
		{
			aTmpPath += ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_UTF8 );
			aTmpPath += aLibPath;
		}
		putenv( aTmpPath.GetBuffer() );
		// Restart if necessary since most library path changes don't have
		// any affect after the application has already started on most
		// platforms
		if ( aLibPath.GetToken( 0, ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_UTF8 ).GetChar( 0 ) ).CompareTo( aCmdPath, aCmdPath.Len() ) != COMPARE_EQUAL )
			execv( pCmdPath, argv );
	}

	SVMain();

	// Force exit since some JVMs won't shutdown when only exit() is invoked
	_exit( 0 );
}

END_C
