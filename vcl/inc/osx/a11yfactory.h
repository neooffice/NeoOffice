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
 * 
 *   Modified July 2022 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDED_VCL_INC_OSX_A11YFACTORY_H
#define INCLUDED_VCL_INC_OSX_A11YFACTORY_H

#include "osxvcltypes.h"
#include "a11ywrapper.h"
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

@interface AquaA11yFactory : NSObject
{
}
+(void)insertIntoWrapperRepository: (NSView *) viewElement forAccessibleContext: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) rxAccessibleContext;
+(AquaA11yWrapper *)wrapperForAccessible: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessible >) rxAccessible;
+(AquaA11yWrapper *)wrapperForAccessibleContext: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) rxAccessibleContext;
+(AquaA11yWrapper *)wrapperForAccessibleContext: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) rxAccessibleContext createIfNotExists:(BOOL) bCreate;
+(AquaA11yWrapper *)wrapperForAccessibleContext: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) rxAccessibleContext createIfNotExists:(BOOL) bCreate asRadioGroup:(BOOL) asRadioGroup;
+(void)removeFromWrapperRepositoryFor: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) rxAccessibleContext;
#ifdef USE_JAVA
+(void)removeFromWrapperRepositoryForWrapper: (AquaA11yWrapper *) theWrapper;
#endif	// USE_JAVA
+(void)registerView: (NSView *) theView;
+(void)revokeView: (NSView *) theViewt;
@end

#ifdef USE_JAVA

@interface AquaA11yPostNotification : NSObject
{
    id                      mpElement;
    NSAccessibilityNotificationName mpName;
}
+ (id)createWithElement:(id)pElement name:(NSAccessibilityNotificationName)pName;
- (id)initWithElement:(id)pElement name:(NSAccessibilityNotificationName)pName;
- (void)dealloc;
- (void)postNotification:(id)pObject;
@end

#endif	// USE_JAVA

#endif // INCLUDED_VCL_INC_OSX_A11YFACTORY_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
