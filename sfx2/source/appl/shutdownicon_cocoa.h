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
 *  Patrick Luby, July 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 by Patrick Luby (patrick.luby@planamesa.com)
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

#ifndef __SHUTDOWNICON_COCOA_H__
#define __SHUTDOWNICON_COCOA_H__

#ifdef __cplusplus
#include <premac.h>
#endif
#include <Carbon/Carbon.h>
#ifdef __cplusplus
#include <postmac.h>
#endif

#define WRITER_COMMAND_ID			'SDI1'
#define CALC_COMMAND_ID				'SDI2'
#define IMPRESS_COMMAND_ID			'SDI3'
#define DRAW_COMMAND_ID				'SDI4'
#define MATH_COMMAND_ID				'SDI5'
#define FROMTEMPLATE_COMMAND_ID		'SDI6'
#define FILEOPEN_COMMAND_ID			'SDI7'

#ifdef __cplusplus
BEGIN_C
#endif
struct AddQuickstartMenuItemsParams
{
	int						mnCount;
	MenuCommand*			mpIDs;
	CFStringRef*			mpStrings;
};

void AddQuickstartMenuItems( struct AddQuickstartMenuItemsParams *pParams );
void ProcessShutdownIconCommand( MenuCommand nCommand );
#ifdef __cplusplus
END_C
#endif

#endif
