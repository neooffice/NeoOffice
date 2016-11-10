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
 *		 - GNU General Public License Version 2.1
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2012 by Planamesa Inc.
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
 *************************************************************************/

#ifndef _UPDATE_COCOA_HXX
#define _UPDATE_COCOA_HXX

#ifdef __OBJC__

#include <rtl/ustring.hxx>
#include <tools/solar.h>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

extern NSString *kUpdateLastURLPref;
extern NSString *kUpdateXPosPref;
extern NSString *kUpdateYPosPref;
extern NSString *kUpdateWidthPref;
extern NSString *kUpdateHeightPref;
extern NSString *kUpdateVisiblePref;
extern NSString *kUpdateServerTypePref;

SAL_DLLPRIVATE OUString UpdateNSStringToOUString( NSString *pString );

#else	// __OBJC__

typedef void* id;

#endif	// __OBJC__

SAL_DLLPRIVATE sal_Bool UpdateQuitNativeDownloadWebView();
SAL_DLLPRIVATE sal_Bool UpdateShowNativeDownloadWebView( OUString aURL, OUString aUserAgent, OUString aTitle );

#endif	// _UPDATE_COCOA_HXX
