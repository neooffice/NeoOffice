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

#ifndef __SALINST_COCOA_H__
#define __SALINST_COCOA_H__

#include <sal/types.h>

#include <com/sun/star/lang/DisposedException.hpp>

#include "java/salinst.h"

SAL_DLLPRIVATE void NSApplication_dispatchPendingEvents( sal_Bool bInNativeDragPrint, sal_Bool bWait );
SAL_DLLPRIVATE id NSApplication_getModalWindow();
SAL_DLLPRIVATE sal_Bool VCLInstance_isInDragPrintLock();
SAL_DLLPRIVATE sal_Bool VCLInstance_isInOrAcquiringDragPrintLock();
SAL_DLLPRIVATE sal_Bool VCLInstance_retainIfInDragPrintLock( id aObject );
SAL_DLLPRIVATE sal_Bool VCLInstance_setDragPrintLock( sal_Bool bLock );
SAL_DLLPRIVATE sal_Bool VCLInstance_updateNativeMenus();

// Note: these must not be static as the symbol will be loaded by various
extern "C"
{
// modules
SAL_DLLPUBLIC_EXPORT sal_Bool Application_beginModalSheet( id *pNSWindowForSheet );
SAL_DLLPUBLIC_EXPORT void Application_endModalSheet();
SAL_DLLPUBLIC_EXPORT void Application_postWakeUpEvent();
SAL_DLLPUBLIC_EXPORT id Application_acquireSecurityScopedURLFromOUString( const OUString *pNonSecurityScopedURL, unsigned char bMustShowDialogIfNoBookmark, const OUString *pDialogTitle );
SAL_DLLPUBLIC_EXPORT id Application_acquireSecurityScopedURLFromNSURL( const id pNonSecurityScopedURL, unsigned char bMustShowDialogIfNoBookmark, const id pDialogTitle );
SAL_DLLPUBLIC_EXPORT void Application_cacheSecurityScopedURLFromOUString( const OUString *pNonSecurityScopedURL );
SAL_DLLPUBLIC_EXPORT void Application_cacheSecurityScopedURL( id pNonSecurityScopedURL );
SAL_DLLPUBLIC_EXPORT void Application_releaseSecurityScopedURL( id pSecurityScopedURLs );
}

#define ACQUIRE_DRAGPRINTLOCK \
	if ( VCLInstance_setDragPrintLock( sal_True ) ) { \
		try {

#define RELEASE_DRAGPRINTLOCKIFNEEDED \
			VCLInstance_setDragPrintLock( sal_False );

#define RELEASE_DRAGPRINTLOCK \
		} catch ( const ::com::sun::star::lang::DisposedException& ) { \
		} catch ( ... ) { \
			NSLog( @"Exception caught while in drag print lock: %s", __PRETTY_FUNCTION__ ); \
		} \
		RELEASE_DRAGPRINTLOCKIFNEEDED \
	}

#endif
