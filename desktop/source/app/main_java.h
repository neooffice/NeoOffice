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
 *  Patrick Luby, November 2008
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2008 Planamesa Inc.
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

#ifndef INCLUDED_DESKTOP_SOURCE_APP_MAIN_JAVA_H
#define INCLUDED_DESKTOP_SOURCE_APP_MAIN_JAVA_H

#include <sal/config.h>
#include <sal/types.h>

#define DOFUNCTION( x ) sal_Bool SAL_DLLPUBLIC_EXPORT _##x ()
#define FUNCTION( x ) DOFUNCTION( x )

#if defined __cplusplus
extern "C" {
#endif

int java_main( int argc, char **argv );

sal_Bool SAL_DLLPUBLIC_EXPORT Application_canSave();
sal_Bool SAL_DLLPUBLIC_EXPORT Application_canUseJava();

#if defined __cplusplus
}
#endif

#endif
