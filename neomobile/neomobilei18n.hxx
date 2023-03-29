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

#ifndef _NEOMOBILEAPPEVENT_HXX
#define _NEOMOBILEAPPEVENT_HXX

#import <tools/link.hxx>

#include <premac.h>
#import <Foundation/Foundation.h>
#include <postmac.h>

// Redefine Cocoa YES and NO defines types for convenience
#ifdef YES
#undef YES
#define YES (MacOSBOOL)1
#endif
#ifdef NO
#undef NO
#define NO (MacOSBOOL)0
#endif

@interface NeoMobileRunPasswordProtectionAlertOnMainThread : NSObject
{
	MacOSBOOL mcancelled;
}
- (id)init;
- (void)runModal:(id)arg;
- (MacOSBOOL)cancelled;
@end

@interface NeoMobileDoFileManagerOnMainThread : NSObject
{
	NSString *mpath;
}
- (id)init;
- (void)dealloc;
- (void)makeBasePath:(id)arg;
- (NSString *)filePath;
- (void)createDir:(NSString *)path;
- (void)removeItem:(NSString *)path;
@end

class SAL_DLLPRIVATE NeoMobileExportFileAppEvent
{
	int						mnErrorCode;
	NSFileManager*			mpFileManager;
	bool					mbFinished;
	NSMutableData*			mpPostBody;
	::rtl::OUString			maSaveUUID;
	NSArray*				mpMimeTypes;
	bool					mbCanceled;
	bool					mbUnsupportedComponentType;

public:
							NeoMobileExportFileAppEvent( ::rtl::OUString aSaveUUID, NSFileManager *pFileManager, NSMutableData *pPostBody, NSArray *pMimeTypes );
	virtual					~NeoMobileExportFileAppEvent() {};
							DECL_LINK( ExportFile, void* );
	void					Cancel() { mbCanceled=true; }
	void					Execute();
	int						GetErrorCode() { return mnErrorCode; }
	NSData*					GetPostBody() { return mpPostBody; }
	bool					IsCanceled() { return(mbCanceled); }
	bool					IsFinished() { return mbFinished; }
	bool					IsUnsupportedComponentType() { return(mbUnsupportedComponentType); }
};

#endif	// _NEOMOBILEAPPEVENT_HXX

#ifndef _NEOMOBILEI18N_HXX
#define _NEOMOBILEI18N_HXX

#import <com/sun/star/lang/Locale.hpp>
#import <sal/types.h>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include "postmac.h"

#define NEOMOBILEABOUT "about"
#define NEOMOBILEBACK "back"
#define NEOMOBILECANCEL "cancel"
#define NEOMOBILECREATEACCOUNT "create.new.account"
#define NEOMOBILEDOWNLOADCANCELED "download.canceled"
#define NEOMOBILEDOWNLOADFAILED "download.failed"
#define NEOMOBILEDOWNLOADINGFILE "downloading.file"
#define NEOMOBILEERROR "error"
#define NEOMOBILEEXPORTINGFILE "exporting.file"
#define NEOMOBILEFORGOTPASSWORD "forgot.password"
#define NEOMOBILELOADING "loading"
#define NEOMOBILELOGIN "login"
#define NEOMOBILELOGINTITLE "login.title"
#define NEOMOBILEMEGABYTE "megabyte"
#define NEOMOBILEPASSWORD "password"
#define NEOMOBILEPRODUCTNAME "product.name"
#define NEOMOBILESAVEPASSWORD "save.password"
#define NEOMOBILEUPLOAD "upload"
#define NEOMOBILEUPLOADCONTINUE "upload.continue"
#define NEOMOBILEUPLOADINGFILE "uploading.file"
#define NEOMOBILEUPLOADPASSWORDPROTECTED "upload.password.protected"
#define NEOMOBILEUSERNAME "username"

/**
 * Returns the application's locale.
 */
SAL_DLLPRIVATE ::com::sun::star::lang::Locale NeoMobileGetApplicationLocale();

/**
 * Lookup a string and retrieve a translated string.  If no translation
 * is available, default to english.
 */
SAL_DLLPRIVATE NSString *NeoMobileGetLocalizedString( const sal_Char *key );
SAL_DLLPRIVATE NSString *NeoMobileGetLocalizedDecimalSeparator();

#endif	// _NEOMOBILEI18N_HXX
