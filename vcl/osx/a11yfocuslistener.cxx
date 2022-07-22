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

@interface AquaA11yFocusListenerFocusedObjectChanged : NSObject
{
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessible > mxAccessible;
    AquaA11yFocusListener* mpFocusListener;
}
+ (id)createWithFocusListener:(AquaA11yFocusListener *)pFocusListener accessible:(const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >&) xAccessible;
- (void)focusedObjectChanged:(id)pObject;
- (void)focusListenerWillBeDeleted:(AquaA11yFocusListener *)pFocusListener;
- (id)initWithFocusListener:(AquaA11yFocusListener *)pFocusListener accessible:(const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >&) xAccessible;
@end

static NSMutableArray<AquaA11yFocusListenerFocusedObjectChanged*> *pPendingFocusedObjectChangedQueue = nil;
static ::osl::Mutex aPendingFocusedObjectChangedQueueMutex;

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
    ::osl::ClearableMutexGuard aGuard( aPendingFocusedObjectChangedQueueMutex );

    // Set the focus listener to NULl for any pending focused object change
    // instances that contain a pointer to this
    if ( pPendingFocusedObjectChangedQueue ) {
        for ( AquaA11yFocusListenerFocusedObjectChanged *pFocusedObjectChanged : pPendingFocusedObjectChangedQueue ) {
            if ( pFocusedObjectChanged )
                [ pFocusedObjectChanged focusListenerWillBeDeleted: this ];
        }
    }

    aGuard.clear();

    if ( m_focusedObject )
        osl_performSelectorOnMainThread( m_focusedObject, @selector(release), m_focusedObject, NO );
}

#endif	// USE_JAVA

id AquaA11yFocusListener::getFocusedUIElement()
{
#ifdef USE_JAVA
    // This method appears to only be called by the native NSAccessibility
    // calls and should already be running on the main thread so we can safely
    // call +[AquaA11yFactory wrapperForAccessibleContext:] without having to
    // release the application mutex
#endif	// USE_JAVA
    if ( nil == m_focusedObject ) {
        Reference< XAccessible > xAccessible( AquaA11yFocusTracker::get().getFocusedObject() );
        try {
            if( xAccessible.is() ) {
                Reference< XAccessibleContext > xContext(xAccessible->getAccessibleContext());
                if( xContext.is() )
                    m_focusedObject = [ AquaA11yFactory wrapperForAccessibleContext: xContext ];
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
    if ( CFRunLoopGetCurrent() != CFRunLoopGetMain() ) {
        AquaA11yFocusListenerFocusedObjectChanged *pAquaA11yFocusListenerFocusedObjectChanged = [ AquaA11yFocusListenerFocusedObjectChanged createWithFocusListener: this accessible: xAccessible ];
        osl_performSelectorOnMainThread( pAquaA11yFocusListenerFocusedObjectChanged, @selector(focusedObjectChanged:), pAquaA11yFocusListenerFocusedObjectChanged, NO );
        return;
    }
#endif	// USE_JAVA

    if ( nil != m_focusedObject ) {
        [ m_focusedObject release ];
        m_focusedObject = nil;
    }

    try {
        if( xAccessible.is() ) {
            Reference< XAccessibleContext > xContext(xAccessible->getAccessibleContext());
            if( xContext.is() )
            {
                m_focusedObject = [ AquaA11yFactory wrapperForAccessibleContext: xContext ];
#ifdef USE_JAVA
                AquaA11yPostNotification *pAquaA11yPostNotification = [ AquaA11yPostNotification createWithElement: m_focusedObject name: NSAccessibilityFocusedUIElementChangedNotification ];
                osl_performSelectorOnMainThread( pAquaA11yPostNotification, @selector(postNotification:), pAquaA11yPostNotification, NO );
#else	// USE_JAVA
                NSAccessibilityPostNotification(m_focusedObject, NSAccessibilityFocusedUIElementChangedNotification);
#endif	// USE_JAVA
            }
        }
    } catch(const RuntimeException &) {
        // intentionally do nothing ..
    }
}

#ifdef USE_JAVA

@implementation AquaA11yFocusListenerFocusedObjectChanged

+ (id)createWithFocusListener:(AquaA11yFocusListener *)pFocusListener accessible:(const Reference< XAccessible >&) xAccessible
{
    AquaA11yFocusListenerFocusedObjectChanged *pRet = [ [ AquaA11yFocusListenerFocusedObjectChanged alloc ] initWithFocusListener: pFocusListener accessible: xAccessible ];
    [ pRet autorelease ];
    return pRet;
}

- (void)focusedObjectChanged:(id)pObject
{
    // Prevent posting of focus change if we are already within an
    // NSAccessibility call by requeuing this selector
    if ( VCLInstance_isInOrAcquiringDragPrintLock() ) {
        [self performSelector:@selector(focusedObjectChanged:) withObject:pObject afterDelay:0.5f];
        return;
    }

    ACQUIRE_DRAGPRINTLOCK
    ::osl::ClearableMutexGuard aGuard( aPendingFocusedObjectChangedQueueMutex );

    if ( pPendingFocusedObjectChangedQueue ) {
        for ( AquaA11yFocusListenerFocusedObjectChanged *pFocusedObjectChanged : pPendingFocusedObjectChangedQueue ) {
            if ( pFocusedObjectChanged && pFocusedObjectChanged->mpFocusListener )
                pFocusedObjectChanged->mpFocusListener->focusedObjectChanged( mxAccessible );
        }
        [ pPendingFocusedObjectChangedQueue removeAllObjects ];
    }

    aGuard.clear();
    RELEASE_DRAGPRINTLOCK
}

- (void)focusListenerWillBeDeleted:(AquaA11yFocusListener *)pFocusListener
{
	if ( mpFocusListener == pFocusListener )
	    mpFocusListener = nullptr;
}

- (id)initWithFocusListener:(AquaA11yFocusListener *)pFocusListener accessible:(const Reference< XAccessible >&) xAccessible
{
    [ super init ];

    mxAccessible = xAccessible;
    mpFocusListener = pFocusListener;

    ::osl::ClearableMutexGuard aGuard( aPendingFocusedObjectChangedQueueMutex );

    if ( !pPendingFocusedObjectChangedQueue ) {
        pPendingFocusedObjectChangedQueue = [ NSMutableArray arrayWithCapacity: 10 ];
        if ( pPendingFocusedObjectChangedQueue )
            [ pPendingFocusedObjectChangedQueue retain ];
    }

    if ( pPendingFocusedObjectChangedQueue )
        [ pPendingFocusedObjectChangedQueue addObject: self ];

    aGuard.clear();

    return self;
}

@end

#endif	// USE_JAVA

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
