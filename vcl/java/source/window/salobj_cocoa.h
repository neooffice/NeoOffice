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
 *  Patrick Luby, December 2007
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2007 Planamesa Inc.
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

#ifndef __SALOBJ_COCOA_H__
#define __SALOBJ_COCOA_H__

#ifdef __cplusplus
#include <premac.h>
#endif
#include <Carbon/Carbon.h>
#ifdef __cplusplus
#include <postmac.h>
typedef void* id;
#endif

#ifdef __cplusplus
BEGIN_C
#endif
id VCLChildView_create();
void VCLChildView_release( id pVCLChildView );
void VCLChildView_setBackgroundColor( id pVCLChildView, int nColor );
void VCLChildView_setBounds( id pVCLChildView, long nX, long nY, long nWidth, long nHeight );
void VCLChildView_setClip( id pVCLChildView, long nX, long nY, long nWidth, long nHeight );
void VCLChildView_show( id pVCLChildView, id pParentNSWindow, BOOL bShow );
#ifdef __cplusplus
END_C
#endif

#endif
