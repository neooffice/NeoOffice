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

#ifndef _UPDATE_COCOA_HXX
#define _UPDATE_COCOA_HXX

// Uncomment the following line to enable the native web view code
// #define USE_NATIVE_WEB_VIEW

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

SAL_DLLPRIVATE OUString UpdateGetOSVersion();
SAL_DLLPRIVATE sal_Bool UpdateQuitNativeDownloadWebView();
SAL_DLLPRIVATE sal_Bool UpdateShowNativeDownloadWebView( OUString aURL, OUString aUserAgent, OUString aTitle );

#endif	// _UPDATE_COCOA_HXX
