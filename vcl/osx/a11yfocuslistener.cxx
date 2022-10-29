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
    NSAutoreleasePool *pPool = [ [ NSAutoreleasePool alloc ] init ];

    if ( m_focusedObject )
        osl_performSelectorOnMainThread( m_focusedObject, @selector(release), m_focusedObject, NO );

    [ pPool release ]; 
}

#endif	// USE_JAVA

id AquaA11yFocusListener::getFocusedUIElement()
{
#ifdef USE_JAVA
    // This method appears to only be called by the native NSAccessibility
    // calls and should already be running on the main thread so we can safely
    // call +[AquaA11yFactory wrapperForAccessibleContext:] without having to
    // release the application mutex
    if ( nil == m_focusedObject && CFRunLoopGetCurrent() == CFRunLoopGetMain() ) {
#else	// USE_JAVA
    if ( nil == m_focusedObject ) {
#endif	// USE_JAVA
        Reference< XAccessible > xAccessible( AquaA11yFocusTracker::get().getFocusedObject() );
        try {
            if( xAccessible.is() ) {
                Reference< XAccessibleContext > xContext(xAccessible->getAccessibleContext());
                if( xContext.is() )
#ifdef USE_JAVA
                {
#endif	// USE_JAVA
                    m_focusedObject = [ AquaA11yFactory wrapperForAccessibleContext: xContext ];
#ifdef USE_JAVA
                    if ( m_focusedObject && ImplIsValidAquaA11yWrapper( m_focusedObject ) && ! [ m_focusedObject isDisposed ] )
                        [ m_focusedObject retain ];
                    else
                        m_focusedObject = nil;
                }
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
#ifdef USE_JAVA
    NSAutoreleasePool *pPool = [ [ NSAutoreleasePool alloc ] init ];
#endif	// USE_JAVA

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
                // The focused wrapper needs to be created immediately as many
                // are transient objects and if creation of the wrapper's
                // underlying C++ objects are not done immmediately, posted
                // notifications will almost always be ignored by VoiceOver
#ifdef USE_ONLY_MAIN_THREAD_TO_CREATE_AQUAA11YWRAPPERS
                AquaA11yWrapperForAccessibleContext *pAquaA11yWrapperForAccessibleContext = [ AquaA11yWrapperForAccessibleContext createWithAccessibleContext: xContext ];
                sal_uLong nCount = Application::ReleaseSolarMutex();
                osl_performSelectorOnMainThread( pAquaA11yWrapperForAccessibleContext, @selector(wrapperForAccessibleContext:), pAquaA11yWrapperForAccessibleContext, YES );
                Application::AcquireSolarMutex( nCount );
                m_focusedObject = [pAquaA11yWrapperForAccessibleContext wrapper];
#else	// USE_ONLY_MAIN_THREAD_TO_CREATE_AQUAA11YWRAPPERS
                // There are no known NSWindow, NSView, etc. calls when
                // creating an AquaA11yWrapper instance so we do not need to
                // create it on the main thread
                m_focusedObject = [ AquaA11yFactory wrapperForAccessibleContext: xContext ];
#endif	// USE_ONLY_MAIN_THREAD_TO_CREATE_AQUAA11YWRAPPERS
                if ( m_focusedObject && ImplIsValidAquaA11yWrapper( m_focusedObject ) && ! [ m_focusedObject isDisposed ] ) {
                    [ m_focusedObject retain ];
                    [ AquaA11yPostNotification addElementToPendingNotificationQueue: m_focusedObject name: NSAccessibilityFocusedUIElementChangedNotification ];
                }
                else {
                    m_focusedObject = nil;
                }
#else	// USE_JAVA
                m_focusedObject = [ AquaA11yFactory wrapperForAccessibleContext: xContext ];
                NSAccessibilityPostNotification(m_focusedObject, NSAccessibilityFocusedUIElementChangedNotification);
#endif	// USE_JAVA
            }
        }
    } catch(const RuntimeException &) {
        // intentionally do nothing ..
    }

#ifdef USE_JAVA
    [ pPool release ];
#endif	// USE_JAVA

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
