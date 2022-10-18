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
#include <sal/types.h>

#include "system.hxx"
#include "sockimpl.hxx"

SAL_DLLPRIVATE sal_Bool osl_createPortFileForPipe( oslPipe pPipe );
SAL_DLLPRIVATE sal_uInt16 osl_getPortForPipeName( const char *pName );
SAL_DLLPRIVATE void osl_unlinkPortFileForPipeName( const char *pName );
