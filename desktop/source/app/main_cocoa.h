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
 *  Patrick Luby, August 2007
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

#ifndef __MAIN_COCOA_H__
#define __MAIN_COCOA_H__

#ifdef __cplusplus
#include <premac.h>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>
#else
#import <Carbon/Carbon.h>
#import <CoreFoundation/CoreFoundation.h>
#endif

#ifdef __cplusplus
BEGIN_C
#endif
void Application_openOrPrintFile( const char *pFileName, BOOL bPrint );
void Application_queryExit();
void NSApplication_run( CFRunLoopTimerRef aTimer, void *pInfo );
#ifdef __cplusplus
END_C
#endif

#endif
