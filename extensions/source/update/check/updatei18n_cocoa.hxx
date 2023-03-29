/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _UPDATEI18N_COCOA_HXX
#define _UPDATEI18N_COCOA_HXX

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include "postmac.h"

#include <com/sun/star/lang/Locale.hpp>
#include <sal/types.h>
#include <svids.hrc>

#include "updatehdl.hrc"

#define UPDATEBACK "back"
#define UPDATEDOWNLOADCANCELED "download.canceled"
#define UPDATEDOWNLOADFAILED "download.failed"
#define UPDATEDOWNLOADINGFILE "downloading.file"
#define UPDATEERROR "error"
#define UPDATEINSTALLUPDATES "install.updates"
#define UPDATELOADING "loading"
#define UPDATEMEGABYTE "megabyte"
#define UPDATEOPENFILEFAILED "open.file.failed"
#define UPDATEOPENINGFILE "opening.file"
#define UPDATEREDOWNLOADFILE "redownload.file"

/**
 * Returns the application's locale.
 */
SAL_DLLPRIVATE ::com::sun::star::lang::Locale GetApplicationLocale();

/**
 * Lookup a string and retrieve a translated string.  If no translation
 * is available, default to english.
 */
SAL_DLLPRIVATE NSString *UpdateGetLocalizedString( const sal_Char *key );
SAL_DLLPRIVATE NSString *UpdateGetLocalizedDecimalSeparator();
SAL_DLLPRIVATE NSString *UpdateGetUPDResString( int nId );
SAL_DLLPRIVATE NSString *UpdateGetVCLResString( int nId );

#endif	// _UPDATEI18N_COCOA_HXX
