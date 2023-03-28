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

#ifndef __SALOBJ_COCOA_H__
#define __SALOBJ_COCOA_H__

#include <premac.h>
#import <Foundation/Foundation.h>
#include <postmac.h>

#include "java/salobj.h"

SAL_DLLPRIVATE id VCLChildView_create();
SAL_DLLPRIVATE void VCLChildView_release( id pVCLChildView );
SAL_DLLPRIVATE void VCLChildView_setBackgroundColor( id pVCLChildView, int nColor );
SAL_DLLPRIVATE void VCLChildView_setBounds( id pVCLChildView, NSRect aBounds );
SAL_DLLPRIVATE void VCLChildView_setClip( id pVCLChildView, NSRect aClipRect );
SAL_DLLPRIVATE void VCLChildView_show( id pVCLChildView, id pParentNSWindow, sal_Bool bShow );

#endif
