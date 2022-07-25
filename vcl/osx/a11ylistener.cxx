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

#include "osx/salinst.h"
#include "osx/a11ylistener.hxx"
#include "osx/a11yfactory.h"
#include "osx/a11yfocustracker.hxx"
#include "osx/a11ywrapper.h"

#include "a11ytextwrapper.h"

#ifdef USE_JAVA

#include <osl/objcutils.h>

#include "../java/source/app/salinst_cocoa.h"

#endif	// USE_JAVA

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

NSString * getTableNotification( const AccessibleEventObject& aEvent )
{
    AccessibleTableModelChange aChange;
    NSString * notification = nil;

    if( (aEvent.NewValue >>= aChange) &&
        ( AccessibleTableModelChangeType::INSERT == aChange.Type || AccessibleTableModelChangeType::DELETE == aChange.Type ) &&
        aChange.FirstRow != aChange.LastRow )
    {
        notification = NSAccessibilityRowCountChangedNotification;
    }

    return notification;
}

AquaA11yEventListener::AquaA11yEventListener(id wrapperObject, sal_Int16 role) : m_wrapperObject(wrapperObject), m_role(role)
{
    [ m_wrapperObject retain ];
}

AquaA11yEventListener::~AquaA11yEventListener()
{
#ifdef USE_JAVA
    NSAutoreleasePool *pPool = [ [ NSAutoreleasePool alloc ] init ];

    osl_performSelectorOnMainThread( m_wrapperObject, @selector(release), m_wrapperObject, NO );

    [ pPool release ]; 
#else	// USE_JAVA
    [ m_wrapperObject release ];
#endif	// USE_JAVA
}

void SAL_CALL
AquaA11yEventListener::disposing( const EventObject& ) throw( RuntimeException, std::exception )
{
#ifdef USE_JAVA
    NSAutoreleasePool *pPool = [ [ NSAutoreleasePool alloc ] init ];

    if ( m_wrapperObject && [ m_wrapperObject isKindOfClass:[ AquaA11yWrapper class ] ] )
        osl_performSelectorOnMainThread( (AquaA11yWrapper *)m_wrapperObject, @selector(removeFromWrapperRepository:), m_wrapperObject, NO );

    [ pPool release ]; 
#else	// USE_JAVA
    [ AquaA11yFactory removeFromWrapperRepositoryFor: [ (AquaA11yWrapper *) m_wrapperObject accessibleContext ] ];
#endif	// USE_JAVA
}

void SAL_CALL
AquaA11yEventListener::notifyEvent( const AccessibleEventObject& aEvent ) throw( RuntimeException, std::exception )
{
#ifdef USE_JAVA
    NSAutoreleasePool *pPool = [ [ NSAutoreleasePool alloc ] init ];
#endif	// USE_JAVA

    NSString * notification = nil;
    id element = m_wrapperObject;
#ifdef USE_JAVA
    ::com::sun::star::awt::Rectangle bounds;
#else	// USE_JAVA
    Rectangle bounds;
#endif	// USE_JAVA

    // TODO: NSAccessibilityValueChanged, NSAccessibilitySelectedRowsChangedNotification
    switch( aEvent.EventId )
    {
        case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED:
            if( m_role != AccessibleRole::LIST ) {
                Reference< XAccessible > xAccessible;
                if( aEvent.NewValue >>= xAccessible )
                    AquaA11yFocusTracker::get().setFocusedObject( xAccessible );
            }
            break;

        case AccessibleEventId::NAME_CHANGED:
            notification = NSAccessibilityTitleChangedNotification;
            break;

        case AccessibleEventId::CHILD:
            // only needed for tooltips (says Apple)
            if ( m_role == AccessibleRole::TOOL_TIP ) {
                if(aEvent.NewValue.hasValue()) {
                    notification = NSAccessibilityCreatedNotification;
                } else if(aEvent.OldValue.hasValue()) {
                    notification = NSAccessibilityUIElementDestroyedNotification;
                }
            }
            break;

        case AccessibleEventId::INVALIDATE_ALL_CHILDREN:
            // TODO: depricate or remember all children
            break;

        case AccessibleEventId::BOUNDRECT_CHANGED:
#ifdef USE_JAVA
        {
            // The accessibleComponent selector only executes C++ code so no
            // need to perform the selector on the main thread
#endif	// USE_JAVA
            bounds = [ element accessibleComponent ] -> getBounds();
            if ( m_oldBounds.X != 0 && ( bounds.X != m_oldBounds.X || bounds.Y != m_oldBounds.Y ) ) {
#ifdef USE_JAVA
                AquaA11yPostNotification *pAquaA11yPostNotification = [ AquaA11yPostNotification createWithElement: element name: NSAccessibilityMovedNotification ];
                osl_performSelectorOnMainThread( pAquaA11yPostNotification, @selector(postPendingNotifications:), pAquaA11yPostNotification, NO );
#else	// USE_JAVA
                NSAccessibilityPostNotification(element, NSAccessibilityMovedNotification); // post directly since both cases can happen simultaneously
#endif	// USE_JAVA
            }
            if ( m_oldBounds.X != 0 && ( bounds.Width != m_oldBounds.Width || bounds.Height != m_oldBounds.Height ) ) {
#ifdef USE_JAVA
                AquaA11yPostNotification *pAquaA11yPostNotification = [ AquaA11yPostNotification createWithElement: element name: NSAccessibilityResizedNotification ];
                osl_performSelectorOnMainThread( pAquaA11yPostNotification, @selector(postPendingNotifications:), pAquaA11yPostNotification, NO );
#else	// USE_JAVA
                NSAccessibilityPostNotification(element, NSAccessibilityResizedNotification); // post directly since both cases can happen simultaneously
#endif	// USE_JAVA
            }
            m_oldBounds = bounds;
#ifdef USE_JAVA
        }
#endif	// USE_JAVA
            break;

        case AccessibleEventId::SELECTION_CHANGED:
            notification = NSAccessibilitySelectedChildrenChangedNotification;
            break;

        case AccessibleEventId::TEXT_SELECTION_CHANGED:
            notification = NSAccessibilitySelectedTextChangedNotification;
            break;

        case AccessibleEventId::TABLE_MODEL_CHANGED:
            notification = getTableNotification(aEvent);
            break;

        case AccessibleEventId::CARET_CHANGED:
            notification = NSAccessibilitySelectedTextChangedNotification;
            break;

        case AccessibleEventId::TEXT_CHANGED:
            notification = NSAccessibilityValueChangedNotification;
            break;

        default:
            break;
    }

    if( nil != notification )
#ifdef USE_JAVA
    {
        AquaA11yPostNotification *pAquaA11yPostNotification = [ AquaA11yPostNotification createWithElement: element name: notification ];
        osl_performSelectorOnMainThread( pAquaA11yPostNotification, @selector(postPendingNotifications:), pAquaA11yPostNotification, NO );
    }

    [ pPool release ]; 
#else	// USE_JAVA
        NSAccessibilityPostNotification(element, notification);
#endif	// USE_JAVA
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
