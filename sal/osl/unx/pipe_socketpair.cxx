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
 *  Patrick Luby, January 2013
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2013 Planamesa Inc.
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

#include <map>
#include <errno.h>
#include <fcntl.h>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include "pipe_socketpair.h"

static ::std::map< oslPipe, int* > aSocketPairMap;
static ::osl::Mutex aSocketPairMutex;

using namespace osl;
using namespace rtl;

oslPipe osl_psz_createSocketPairPipe( const sal_Char *pszPipeName, oslPipeOptions nOptions )
{
	if ( !pszPipeName )
		return NULL;

	oslPipe pPipe = __osl_createPipeImpl();
	if ( !pPipe )
		return NULL;

	strncpy( pPipe->m_Name, pszPipeName, sizeof( pPipe->m_Name ) );

	Guard< Mutex > aGuard( aSocketPairMutex );

	::std::map< oslPipe, int* >::const_iterator it = aSocketPairMap.begin();
	for ( ; it != aSocketPairMap.end(); ++it )
	{
		if ( !strncmp( pPipe->m_Name, it->first->m_Name, sizeof( pPipe->m_Name ) ) )
			break;
	}

	if ( nOptions & osl_Pipe_CREATE )
	{
		if ( it == aSocketPairMap.end() )
		{
			int *pSocketFDs = new int[2];
			if ( pSocketFDs )
			{
				if ( socketpair( AF_UNIX, SOCK_STREAM, 0, pSocketFDs ) == 0 )
				{
					// Set close-on-exec flag
					int nFlags;
					if ( ( nFlags = fcntl( pSocketFDs[0], F_GETFD, 0 ) ) != -1 )
					{
						nFlags |= FD_CLOEXEC;
						if ( fcntl( pSocketFDs[0], F_SETFD, nFlags ) == -1 )
							OSL_TRACE( "osl_psz_createSocketPairPipe failed changing socket flags. Errno: %d; %s\n", errno, strerror( errno ) );
					}
					if ( ( nFlags = fcntl( pSocketFDs[1], F_GETFD, 0 ) ) != -1 )
					{
						nFlags |= FD_CLOEXEC;
						if ( fcntl( pSocketFDs[1], F_SETFD, nFlags ) == -1 )
							OSL_TRACE( "osl_psz_createSocketPairPipe failed changing socket flags. Errno: %d; %s\n", errno, strerror( errno ) );
					}

					pPipe->m_Socket = pSocketFDs[0];
					aSocketPairMap[ pPipe ] = pSocketFDs;
					return pPipe;
				}
				else
				{
					delete[] pSocketFDs;
				}
			}
		}
	}
	else
	{
		if ( it != aSocketPairMap.end() )
		{
			int nSocketFD = dup( it->second[1] );
			if ( nSocketFD >= 0 )
			{
				// Set close-on-exec flag
				int nFlags;
				if ( ( nFlags = fcntl( nSocketFD, F_GETFD, 0 ) ) != -1 )
				{
					nFlags |= FD_CLOEXEC;
					if ( fcntl( nSocketFD, F_SETFD, nFlags ) == -1 )
						OSL_TRACE( "osl_psz_createSocketPairPipe failed changing socket flags. Errno: %d; %s\n", errno, strerror( errno ) );
				}

				pPipe->m_Socket = nSocketFD;
				return pPipe;
			}
		}
	}

	__osl_destroyPipeImpl( pPipe );
	return NULL;
}

void osl_psz_closeSocketPairPipe( oslPipe pImpl )
{
	if ( !pImpl || !pImpl->m_bClosed )
		return;

	Guard< Mutex > aGuard( aSocketPairMutex );

	::std::map< oslPipe, int* >::iterator it = aSocketPairMap.begin();
	for ( ; it != aSocketPairMap.end(); ++it )
	{
		if ( !strncmp( pImpl->m_Name, it->first->m_Name, sizeof( pImpl->m_Name ) ) )
			break;
	}

	if ( it != aSocketPairMap.end() )
	{
		// The first descriptor should have already been closed
		shutdown( it->second[1], SHUT_RDWR );
		close( it->second[1] );

		aSocketPairMap.erase( it );
	}
}
