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
 *  Copyright 2008 by Planamesa Inc.
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

#ifndef _NEOMOBILEI18N_HXX
#define _NEOMOBILEI18N_HXX

#include <com/sun/star/lang/Locale.hpp>
#include <sal/types.h>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include "postmac.h"

#define NEOMOBILECANCEL "cancel"
#define NEOMOBILEDOWNLOADCANCELED "download.canceled"
#define NEOMOBILEDOWNLOADFAILED "download.failed"
#define NEOMOBILEDOWNLOADINGFILE "downloading.file"
#define NEOMOBILEERROR "error"
#define NEOMOBILEEXPORTINGFILE "exporting.file"
#define NEOMOBILELOADING "loading"
#define NEOMOBILEPRODUCTNAME "product.name"
#define NEOMOBILEUPLOAD "upload"
#define NEOMOBILEUPLOADCONTINUE "upload.continue"
#define NEOMOBILEUPLOADINGFILE "uploading.file"
#define NEOMOBILEUPLOADPASSWORDPROTECTED "upload.password.protected"

/**
 * Returns the application's locale.
 */
::com::sun::star::lang::Locale GetApplicationLocale();

/**
 * Lookup a string and retrieve a translated string.  If no translation
 * is available, default to english.
 */
NSString *GetLocalizedString( const sal_Char *key );

#endif	// _NEOMOBILEI18N_HXX
