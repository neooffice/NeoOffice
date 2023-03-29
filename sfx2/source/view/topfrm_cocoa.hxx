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
 
#ifndef __TOPFRM_COCOA_HXX__
#define __TOPFRM_COCOA_HXX__

#ifndef __OBJC__
#include <premac.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>
class NSView;
#elif defined __OBJC__
class SfxViewFrame;
#endif

// Comment out the following line to disable native versions support
#define USE_NATIVE_VERSIONS

extern "C" SAL_DLLPUBLIC_EXPORT OUString *NSDocument_revertToSavedLocalizedString( vcl::Window *pWindow );
extern "C" SAL_DLLPUBLIC_EXPORT OUString *NSDocument_saveAVersionLocalizedString( vcl::Window *pWindow );
sal_Bool NSDocument_isValidMoveToPath( OUString aPath );
sal_Bool NSDocument_versionsEnabled();
sal_Bool NSDocument_versionsSupported();
void SFXDocument_createDocument( SfxViewFrame *pFrame, NSView *pView, CFURLRef aURL, sal_Bool bReadOnly );
void SFXDocument_documentHasBeenDeleted( SfxViewFrame *pFrame );
void SFXDocument_documentHasBeenModified( SfxViewFrame *pFrame );
void SFXDocument_documentHasMoved( SfxViewFrame *pFrame, OUString aNewURL );
sal_Bool SFXDocument_documentIsReliquished( SfxViewFrame *pFrame );
void SFXDocument_duplicate( SfxViewFrame *pFrame, sal_Bool bWaitForRevertCall, sal_Bool bSetModified );
sal_Bool SFXDocument_hasDocument( SfxViewFrame *pFrame );
void SFXDocument_openPendingDuplicateURLs();
void SFXDocument_releaseDocument( SfxViewFrame *pFrame );
void SFXDocument_reload( SfxViewFrame *pFrame, sal_Bool bSilent );
void SFXDocument_revertDocumentToSaved( SfxViewFrame *pFrame );
void SFXDocument_saveVersionOfDocument( SfxViewFrame *pFrame );
sal_Bool SFXDocument_setDocumentModified( SfxViewFrame *pFrame, sal_Bool bModified );

#endif
