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
 *  Patrick Luby, May 2014
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2014 Planamesa Inc.
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

#include <sfx2/objsh.hxx>
#include <sfx2/sfxsids.hrc>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#import <objc/objc-runtime.h>
#include <postmac.h>

#import "objserv_cocoa.h"

sal_Bool SfxObjectShell_canSave( SfxObjectShell *pObjShell, USHORT nID )
{
	sal_Bool bRet = sal_True;

	if ( pObjShell && ( nID == SID_SAVEDOC || nID == SID_SAVEASDOC ) )
	{
		char *env = getenv( "SAL_ENABLE_MAS" );
		if ( !env || strcmp( env, "1" ) )
		{
			bRet = sal_False;
		}
	}

	return bRet;
}
