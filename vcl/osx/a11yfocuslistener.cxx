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

#include <salhelper/refobj.hxx>

#include "osx/a11yfocustracker.hxx"
#include "osx/a11yfactory.h"

#include "a11yfocuslistener.hxx"

#ifdef USE_JAVA

#include <osl/objcutils.h>

#include "../java/source/app/salinst_cocoa.h"

#endif	// USE_JAVA

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

rtl::Reference< AquaA11yFocusListener > AquaA11yFocusListener::theListener;

rtl::Reference< AquaA11yFocusListener > AquaA11yFocusListener::get()
{
    if ( ! theListener.is() )
        theListener = new AquaA11yFocusListener();

    return theListener;
}

AquaA11yFocusListener::AquaA11yFocusListener() : m_focusedObject(nil)
{
}

#ifdef USE_JAVA

AquaA11yFocusListener::~AquaA11yFocusListener()
{
    if ( m_focusedObject )
        osl_performSelectorOnMainThread( m_focusedObject, @selector(release), m_focusedObject, NO );
}

#endif	// USE_JAVA

id AquaA11yFocusListener::getFocusedUIElement()
{
    if ( nil == m_focusedObject ) {
        Reference< XAccessible > xAccessible( AquaA11yFocusTracker::get().getFocusedObject() );
        try {
            if( xAccessible.is() ) {
                Reference< XAccessibleContext > xContext(xAccessible->getAccessibleContext());
                if( xContext.is() )
#ifdef USE_JAVA
                {
                    sal_uLong nCount = Application::ReleaseSolarMutex();
                    AquaA11yFactoryWrapperForAccessibleContext *pAquaA11yFactoryWrapperForAccessibleContext = [ AquaA11yFactoryWrapperForAccessibleContext createWithContext: xContext ];
                    osl_performSelectorOnMainThread( pAquaA11yFactoryWrapperForAccessibleContext, @selector(wrapperForAccessibleContext:), pAquaA11yFactoryWrapperForAccessibleContext, YES );
                    m_focusedObject = [pAquaA11yFactoryWrapperForAccessibleContext wrapper ];
                    if ( m_focusedObject )
                        [ m_focusedObject retain ];
                    Application::AcquireSolarMutex( nCount );
                }
#else	// USE_JAVA
                    m_focusedObject = [ AquaA11yFactory wrapperForAccessibleContext: xContext ];
#endif	// USE_JAVA
            }
        } catch(const RuntimeException &)  {
            // intentionally do nothing ..
        }
    }

    return m_focusedObject;
}

void SAL_CALL
AquaA11yFocusListener::focusedObjectChanged(const Reference< XAccessible >& xAccessible)
{
    if ( nil != m_focusedObject ) {
#ifdef USE_JAVA
        osl_performSelectorOnMainThread( m_focusedObject, @selector(release), m_focusedObject, NO );
#else	// USE_JAVA
        [ m_focusedObject release ];
#endif	// USE_JAVA
        m_focusedObject = nil;
    }

    try {
        if( xAccessible.is() ) {
            Reference< XAccessibleContext > xContext(xAccessible->getAccessibleContext());
            if( xContext.is() )
            {
#ifdef USE_JAVA
                sal_uLong nCount = Application::ReleaseSolarMutex();
                AquaA11yFactoryWrapperForAccessibleContext *pAquaA11yFactoryWrapperForAccessibleContext = [ AquaA11yFactoryWrapperForAccessibleContext createWithContext: xContext ];
                AquaA11yPostNotification *pAquaA11yPostNotification = [ AquaA11yPostNotification createWithElement: m_focusedObject name: NSAccessibilityFocusedUIElementChangedNotification ];
                osl_performSelectorOnMainThread( pAquaA11yFactoryWrapperForAccessibleContext, @selector(wrapperForAccessibleContext:), pAquaA11yFactoryWrapperForAccessibleContext, YES );
                m_focusedObject = [pAquaA11yFactoryWrapperForAccessibleContext wrapper ];
                osl_performSelectorOnMainThread( pAquaA11yPostNotification, @selector(postNotification:), pAquaA11yPostNotification, NO );
                Application::AcquireSolarMutex( nCount );
#else	// USE_JAVA
                m_focusedObject = [ AquaA11yFactory wrapperForAccessibleContext: xContext ];
                NSAccessibilityPostNotification(m_focusedObject, NSAccessibilityFocusedUIElementChangedNotification);
#endif	// USE_JAVA
            }
        }
    } catch(const RuntimeException &) {
        // intentionally do nothing ..
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
