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

#ifndef INCLUDED_OSL_OBJCUTILS_H
#define INCLUDED_OSL_OBJCUTILS_H

#ifdef __OBJC__

#include <premac.h>
#import <AppKit/AppKit.h>
#include <postmac.h>

#include <sal/saldllapi.h>

#define JAVA_AWT_RUNLOOPMODE CFSTR( "AWTRunLoopMode" )

SAL_DLLPUBLIC void osl_performSelectorOnMainThread( NSObject *pObj, SEL aSel, NSObject *pArg, sal_Bool bWait );
SAL_DLLPUBLIC NSArray<NSRunLoopMode> *osl_getStandardRunLoopModes();

#endif	// __OBJC__

#endif
