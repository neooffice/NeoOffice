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

#ifndef _COCOA_FILEDIALOG_H_
#define _COCOA_FILEDIALOG_H_

#include <tools/solar.h>

#ifdef __cplusplus
#include <premac.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>
#endif

enum CocoaControlType {
	COCOA_CONTROL_TYPE_BUTTON,
	COCOA_CONTROL_TYPE_CHECKBOX,
	COCOA_CONTROL_TYPE_POPUP,
	MAX_COCOA_CONTROL_TYPE
};

// These are defined in reverse order of their appearance
enum CocoaControlID {
	COCOA_CONTROL_ID_AUTOEXTENSION,
	COCOA_CONTROL_ID_FILTEROPTIONS,
	COCOA_CONTROL_ID_IMAGE_TEMPLATE,
	COCOA_CONTROL_ID_LINK,
	COCOA_CONTROL_ID_PASSWORD,
	COCOA_CONTROL_ID_PLAY,
	COCOA_CONTROL_ID_PREVIEW,
	COCOA_CONTROL_ID_READONLY,
	COCOA_CONTROL_ID_SELECTION,
	COCOA_CONTROL_ID_TEMPLATE,
	COCOA_CONTROL_ID_VERSION,
	COCOA_CONTROL_ID_FILETYPE,
	MAX_COCOA_CONTROL_ID
};

#ifdef __cplusplus
extern "C" {
#endif
void JavaFilePicker_controlStateChanged( int nID, void *pPicker );
void NSFileDialog_addFilter( id pDialog, CFStringRef aItem, CFStringRef aFilter );
void NSFileDialog_addItem( id pDialog, int nID, CFStringRef aItem );
void NSFileDialog_cancel( id pDialog );
int NSFileDialog_controlType( int nID );
id NSFileDialog_create( void *pPicker, sal_Bool bUseFileOpenDialog, sal_Bool bChooseFiles, sal_Bool bShowAutoExtension, sal_Bool bShowFilterOptions, sal_Bool bShowImageTemplate, sal_Bool bShowLink, sal_Bool bShowPassword, sal_Bool bShowReadOnly, sal_Bool bShowSelection, sal_Bool bShowTemplate, sal_Bool bShowVersion );
void NSFileDialog_deleteItem( id pDialog, int nID, CFStringRef aItem );
CFStringRef NSFileDialog_directory( id pDialog );
CFStringRef *NSFileDialog_URLs( id pDialog );
sal_Bool NSFileDialog_isChecked( id pDialog, int nID );
CFStringRef *NSFileDialog_items( id pDialog, int nID );
CFStringRef NSFileDialog_label( id pDialog, int nID );
void NSFileDialog_release( id pDialog );
void NSFileManager_releaseURLs( CFStringRef *pURLs );
void NSFileManager_releaseItems( CFStringRef *pItems );
CFStringRef NSFileDialog_selectedFilter( id pDialog );
CFStringRef NSFileDialog_selectedItem( id pDialog, int nID );
int NSFileDialog_selectedItemIndex( id pDialog, int nID );
void NSFileDialog_setChecked( id pDialog, int nID, sal_Bool bChecked );
void NSFileDialog_setDefaultName( id pDialog, CFStringRef aName );
void NSFileDialog_setDirectory( id pDialog, CFStringRef aDirectory );
void NSFileDialog_setEnabled( id pDialog, int nID, sal_Bool bEnabled );
void NSFileDialog_setLabel( id pDialog, int nID, CFStringRef aLabel );
void NSFileDialog_setMultiSelectionMode( id pDialog, sal_Bool bMultiSelectionMode );
void NSFileDialog_setSelectedFilter( id pDialog, CFStringRef aItem );
void NSFileDialog_setSelectedItem( id pDialog, int nID, int nItem );
void NSFileDialog_setTitle( id pDialog, CFStringRef aTitle );
short NSFileDialog_showFileDialog( id pDialog );
#ifdef __cplusplus
}
#endif

#endif	// _COCOA_FILEDIALOG_H_
