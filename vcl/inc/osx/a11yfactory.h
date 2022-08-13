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
#ifdef USE_JAVA
+(void)insertIntoWrapperRepository: (NSAccessibilityElement *) viewElement forAccessibleContext: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) rxAccessibleContext;
#else	// USE_JAVA
+(void)insertIntoWrapperRepository: (NSView *) viewElement forAccessibleContext: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) rxAccessibleContext;
#endif	// USE_JAVA
+(AquaA11yWrapper *)wrapperForAccessible: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessible >) rxAccessible;
+(AquaA11yWrapper *)wrapperForAccessibleContext: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) rxAccessibleContext;
+(AquaA11yWrapper *)wrapperForAccessibleContext: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) rxAccessibleContext createIfNotExists:(BOOL) bCreate;
+(AquaA11yWrapper *)wrapperForAccessibleContext: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) rxAccessibleContext createIfNotExists:(BOOL) bCreate asRadioGroup:(BOOL) asRadioGroup;
+(void)removeFromWrapperRepositoryFor: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) rxAccessibleContext;
#ifdef USE_JAVA
+(void)removeFromWrapperRepositoryForWrapper: (AquaA11yWrapper *) theWrapper;
+(void)registerView: (AquaA11yWrapper *) theView;
+(void)revokeView: (AquaA11yWrapper *) theViewt;
#else	// USE_JAVA
+(void)registerView: (NSView *) theView;
+(void)revokeView: (NSView *) theViewt;
#endif	// USE_JAVA
@end

#ifdef USE_JAVA

@interface AquaA11yWrapperForAccessibleContext : NSObject
{
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > mxAccessibleContext;
    AquaA11yWrapper*        mpWrapper;
}
+ (id)createWithAccessibleContext:(::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >)xAccessibleContext;
- (id)initWithAccessibleContext:(::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >)xAccessibleContext;
- (void)dealloc;
- (void)wrapperForAccessibleContext:(id)pObject;
- (AquaA11yWrapper *)wrapper;
@end

@interface AquaA11yRemoveFromWrapperRepository : NSObject
{
    AquaA11yWrapper*        mpElement;
}
+ (id)createWithElement:(AquaA11yWrapper *)pElement;
- (id)initWithElement:(AquaA11yWrapper *)pElement;
- (void)dealloc;
- (void)removeFromWrapperRepository:(id)pObject;
@end

@interface AquaA11yPostNotification : NSObject
{
    id                      mpElement;
    NSAccessibilityNotificationName mpName;
}
+ (id)createWithElement:(id)pElement name:(NSAccessibilityNotificationName)pName;
- (id)initWithElement:(id)pElement name:(NSAccessibilityNotificationName)pName;
- (void)dealloc;
- (void)postNotification;
- (void)postPendingNotifications:(id)pObject;
@end

#endif	// USE_JAVA

#endif // INCLUDED_VCL_INC_OSX_A11YFACTORY_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
