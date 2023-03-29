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
