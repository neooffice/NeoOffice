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

#ifndef INCLUDED_VCL_A11Y_H
#define INCLUDED_VCL_A11Y_H

#if defined USE_JAVA && MACOSX

// Uncomment the following define to only create AquaA11yWrapper instances on
// the main thread when AquaA11yWrapper is a subclass of NSView.
// Note: if you change this, you will need to rebuild the following custom
// modules as they include this file:
//   accessibility
//   sw
//#define USE_ONLY_MAIN_THREAD_TO_CREATE_AQUAA11YWRAPPERS

#endif	// USE_JAVA && MACOSX

#endif	// INCLUDED_VCL_A11Y_H
