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
 *  Patrick Luby, September 2005
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

#ifndef __VCLGRAPHICS_COCOA_H__
#define __VCLGRAPHICS_COCOA_H__

#ifdef __cplusplus
#include <premac.h>
#endif
#include <ApplicationServices/ApplicationServices.h>
#ifdef __cplusplus
#include <postmac.h>
#endif

#ifdef __cplusplus
BEGIN_C
#endif
CFStringRef GetDoDrawFontName();
void CGImageRef_drawInRect( CGImageRef aImage, float fX, float fY, float fWidth, float fHeight, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread );
void NSEPSImageRep_drawInRect( void *pPtr, unsigned nSize, float fX, float fY, float fWidth, float fHeight, BOOL bDrawInMainThread );
#ifdef __cplusplus
END_C
#endif

#endif
