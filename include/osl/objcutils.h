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
 *  Patrick Luby, July 2022
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2022 Planamesa Inc.
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, *  MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_OSL_OBJCUTILS_H
#define INCLUDED_OSL_OBJCUTILS_H

#ifdef __OBJC__

#include <premac.h>
#import <AppKit/AppKit.h>
#include <postmac.h>

#include <sal/saldllapi.h>

#define JAVA_AWT_RUNLOOPMODE CFSTR( "AWTRunLoopMode" )

SAL_DLLPUBLIC void osl_performSelectorOnMainThread( NSObject *pObj, SEL aSel, NSObject *pArg, sal_Bool bWait );
SAL_DLLPUBLIC NSArray<NSRunLoopMode> *osl_getStandardRunLoopModes();

#endif	// __OBJC__

#endif
