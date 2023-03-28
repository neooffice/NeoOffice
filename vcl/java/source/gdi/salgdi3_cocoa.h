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

#ifndef __SALOGL_COCOA_H__
#define __SALOGL_COCOA_H__

#include "java/salgdi.h"

SAL_DLLPRIVATE NSFont *NSFont_findPlainFont( NSFont *pNSFont );
SAL_DLLPRIVATE NSDictionary *NSFontManager_getAllFonts();
SAL_DLLPRIVATE sal_Bool NSFontManager_isFixedPitch( NSFont *pNSFont );
SAL_DLLPRIVATE sal_Bool NSFontManager_isItalic( NSFont *pNSFont );
SAL_DLLPRIVATE FontWidth NSFontManager_widthOfFont( NSFont *pNSFont );
SAL_DLLPRIVATE FontWeight NSFontManager_weightOfFont( NSFont *pNSFont );

#endif
