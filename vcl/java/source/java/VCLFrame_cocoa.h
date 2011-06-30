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
 *  Copyright 2005 Planamesa Inc.
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

#ifndef __VCLFRAME_COCOA_H__
#define __VCLFRAME_COCOA_H__

// Uncomment the following line to enable rounded bottom corners in Java frames
// #define USE_ROUNDED_BOTTOM_CORNERS_IN_JAVA_FRAMES

// Uncomment out the following line to enable full screen mode
// #define USE_NATIVE_FULL_SCREEN_MODE

#ifdef __cplusplus
typedef void* id;

BEGIN_C
#endif
id CWindow_getNSWindow( id pCWindow );
void CWindow_getNSWindowBounds( id pCWindow, float *pX, float *pY, float *pWidth, float *pHeight, BOOL *pInLiveResize, BOOL bFullScreen );
id CWindow_getNSWindowContentView( id pCWindow, BOOL bTopLevelWindow );
int CWindow_makeFloatingWindow( id pCWindow );
void CWindow_makeModalWindow( id pCWindow );
void CWindow_makeUnshadowedWindow( id pCWindow );
void CWindow_updateLocation( id pCWindow );
#ifdef __cplusplus
END_C
#endif

#endif
