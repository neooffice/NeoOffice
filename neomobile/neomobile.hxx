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

#ifndef _NEOMOBILE_HXX
#define _NEOMOBILE_HXX

#import <rtl/ustring.hxx>

#include "premac.h"
#import <Cocoa/Cocoa.h>
#include "postmac.h"

extern const NSString *kNeoMobileAboutURL;
extern const NSString *kNeoMobileLoginURI;
extern const NSString *kNeoMobileLastURLPref;
extern const NSString *kNeoMobileXPosPref;
extern const NSString *kNeoMobileYPosPref;
extern const NSString *kNeoMobileWidthPref;
extern const NSString *kNeoMobileHeightPref;
extern const NSString *kNeoMobileVisiblePref;
extern const NSString *kNeoMobileServerTypePref;

@interface NeoMobileCreateWebViewImpl : NSObject
{
	const NSString*				mpURI;
	const NSString*				mpUserAgent;
}
+ (id)createWithURI:(const NSString *)pURI userAgent:(const NSString *)pUserAgent;
- (id)initWithURI:(const NSString *)pURI userAgent:(const NSString *)pUserAgent;
- (void)showWebView:(id)obj;
- (void)showWebViewOnlyIfVisible:(id)obj;
@end

SAL_DLLPRIVATE NSArray *NeoMobileGetPerformSelectorOnMainThreadModes();
SAL_DLLPRIVATE NSString *NeoMobileGetUserAgent();
SAL_DLLPRIVATE ::rtl::OUString NeoMobileNSStringToOUString( NSString *pString );
SAL_DLLPRIVATE ::sal_Bool NeoMobileZipDirectory( const rtl::OUString& dirPath, const rtl::OUString& zipFilePath );

#endif	// _NEOMOBILE_HXX
