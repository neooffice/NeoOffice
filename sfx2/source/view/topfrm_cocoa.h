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
 *  Patrick Luby, May 2011
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2011 Planamesa Inc.
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
 
#ifndef __TOPFRM_COCOA_H__
#define __TOPFRM_COCOA_H__

#if defined __cplusplus && !defined __OBJC__
#include <premac.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>
class NSView;
#elif defined __OBJC__
class SfxViewFrame;
#endif

// Comment out the following line to disable native versions support
#define USE_NATIVE_VERSIONS

#ifdef __cplusplus
extern "C" {
#endif
SAL_DLLPUBLIC_EXPORT ::rtl::OUString NSDocument_revertToSavedLocalizedString( Window *pWindow );
SAL_DLLPUBLIC_EXPORT ::rtl::OUString NSDocument_saveAVersionLocalizedString( Window *pWindow );
sal_Bool NSDocument_isValidMoveToPath( ::rtl::OUString aPath );
sal_Bool NSDocument_versionsEnabled();
sal_Bool NSDocument_versionsSupported();
void SFXDocument_createDocument( SfxViewFrame *pFrame, NSView *pView, CFURLRef aURL, sal_Bool bReadOnly );
void SFXDocument_documentHasBeenDeleted( SfxViewFrame *pFrame );
void SFXDocument_documentHasBeenModified( SfxViewFrame *pFrame );
void SFXDocument_documentHasMoved( SfxViewFrame *pFrame, ::rtl::OUString aNewURL );
sal_Bool SFXDocument_documentIsReliquished( SfxViewFrame *pFrame );
void SFXDocument_duplicate( SfxViewFrame *pFrame, sal_Bool bWaitForRevertCall, sal_Bool bSetModified );
sal_Bool SFXDocument_hasDocument( SfxViewFrame *pFrame );
void SFXDocument_openPendingDuplicateURLs();
void SFXDocument_releaseDocument( SfxViewFrame *pFrame );
void SFXDocument_reload( SfxViewFrame *pFrame, sal_Bool bSilent );
void SFXDocument_revertDocumentToSaved( SfxViewFrame *pFrame );
void SFXDocument_saveVersionOfDocument( SfxViewFrame *pFrame );
sal_Bool SFXDocument_setDocumentModified( SfxViewFrame *pFrame, sal_Bool bModified );
#ifdef __cplusplus
}
#endif

#endif
