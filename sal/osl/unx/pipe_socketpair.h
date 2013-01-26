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

#include <sys/socket.h>

#include <osl/pipe.h>

#include "sockimpl.h"

#ifdef __cplusplus
extern "C"
{
#endif	/* _cplusplus */
SAL_DLLPRIVATE oslPipe __osl_createPipeImpl();
SAL_DLLPRIVATE void __osl_destroyPipeImpl( oslPipe pImpl );
SAL_DLLPRIVATE oslPipe osl_psz_createSocketPairPipe( const sal_Char *pszPipeName, oslPipeOptions nOptions );
SAL_DLLPRIVATE void osl_psz_closeSocketPairPipe( oslPipe pImpl );
#ifdef __cplusplus
}
#endif	/* _cplusplus */
