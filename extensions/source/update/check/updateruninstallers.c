/*************************************************************************
 *i
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
 *  Patrick Luby, February 2012
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

#include <errno.h>
#include <stdio.h>
#ifdef MACOSX
#include <unistd.h>
#include <sys/wait.h>
#endif	// MACOSX

int main( int argc, char **argv )
{
	// Wait until parent process exits by waiting until stdin has closed
	char pBuf[ BUFSIZ ];
	while ( fread( pBuf, sizeof(pBuf), 1, stdin ) > 0 )
		;

#ifdef MACOSX
	// Launch each installer package using the "open" command
	char *pOpenArgs[ 3 ];
	pOpenArgs[ 0 ] = "/usr/bin/open";
	pOpenArgs[ 1 ] = NULL;
	pOpenArgs[ 2 ] = NULL;
	int i = 1;
	for ( ; i < argc ; i++ )
	{
		pOpenArgs[ 1 ] = argv[ i ];

        // Execute the open command in child process
		pid_t pid = fork();
		if ( !pid )
		{
			close( 0 );
			execvp( pOpenArgs[ 0 ], pOpenArgs );
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
#endif	// MACOSX

	return 0;
}
