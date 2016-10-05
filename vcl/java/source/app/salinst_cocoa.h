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
 *  Patrick Luby, September 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 Planamesa Inc.
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

#ifndef __SALINST_COCOA_H__
#define __SALINST_COCOA_H__

#include <sal/types.h>

#include "java/salinst.h"

SAL_DLLPRIVATE void NSApplication_dispatchPendingEvents( sal_Bool bInNativeDragPrint, sal_Bool bWait );
SAL_DLLPRIVATE id NSApplication_getModalWindow();
SAL_DLLPRIVATE sal_Bool VCLInstance_retainIfInDragPrintLock( id aObject );
SAL_DLLPRIVATE sal_Bool VCLInstance_setDragPrintLock( sal_Bool bLock );
SAL_DLLPRIVATE sal_Bool VCLInstance_updateNativeMenus();

// Note: these must not be static as the symbol will be loaded by various
// modules
#ifdef __cplusplus
BEGIN_C
#endif	// __cplusplus
SAL_DLLPUBLIC_EXPORT sal_Bool Application_beginModalSheet( id *pNSWindowForSheet );
SAL_DLLPUBLIC_EXPORT void Application_endModalSheet();
SAL_DLLPUBLIC_EXPORT void Application_postWakeUpEvent();
SAL_DLLPUBLIC_EXPORT id Application_acquireSecurityScopedURLFromOUString( const OUString *pNonSecurityScopedURL, unsigned char bMustShowDialogIfNoBookmark, const OUString *pDialogTitle );
SAL_DLLPUBLIC_EXPORT id Application_acquireSecurityScopedURLFromNSURL( const id pNonSecurityScopedURL, unsigned char bMustShowDialogIfNoBookmark, const id pDialogTitle );
SAL_DLLPUBLIC_EXPORT void Application_cacheSecurityScopedURLFromOUString( const OUString *pNonSecurityScopedURL );
SAL_DLLPUBLIC_EXPORT void Application_cacheSecurityScopedURL( id pNonSecurityScopedURL );
SAL_DLLPUBLIC_EXPORT void Application_releaseSecurityScopedURL( id pSecurityScopedURLs );
#ifdef __cplusplus
END_C
#endif	// __cplusplus

#endif
