/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
