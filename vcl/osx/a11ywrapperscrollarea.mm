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

#include "a11ywrapperscrollarea.h"
#include "a11ywrapperscrollbar.h"
#include "a11yrolehelper.h"

#ifdef USE_JAVA
#include "../java/source/app/salinst_cocoa.h"
#endif	// USE_JAVA

// Wrapper for AXScrollArea role

@implementation AquaA11yWrapperScrollArea : AquaA11yWrapper

-(id)scrollBarWithOrientation:(NSString *)orientation {
    AquaA11yWrapper * theScrollBar = nil;
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    NSArray * elementChildren = [ self accessibilityAttributeValue: NSAccessibilityChildrenAttribute ];
    if ( [ elementChildren count ] > 0 ) {
        NSEnumerator * enumerator = [ elementChildren objectEnumerator ];
        id child;
        while ( ( child = [ enumerator nextObject ] ) ) {
            AquaA11yWrapper * element = ( AquaA11yWrapper * ) child;
            if ( [ element isKindOfClass: [ AquaA11yWrapperScrollBar class ] ] ) { 
                AquaA11yWrapperScrollBar * scrollBar = (AquaA11yWrapperScrollBar *) element;
                if ( [ [ scrollBar orientationAttribute ] isEqualToString: orientation ] ) {
                    theScrollBar = scrollBar;
                    break;
                }
            }
        }
    }
    [ pool release ];
    return theScrollBar;
}

-(id)verticalScrollBarAttribute {
    return [ self scrollBarWithOrientation: NSAccessibilityVerticalOrientationValue ];
}

-(id)horizontalScrollBarAttribute {
    return [ self scrollBarWithOrientation: NSAccessibilityHorizontalOrientationValue ];
}

-(NSArray *)accessibilityAttributeNames {
#ifdef USE_JAVA
    NSMutableArray * attributeNames = nil;
    if ( !ImplApplicationIsRunning() )
        return [ NSArray array ];
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    // Default Attributes
    attributeNames = [ NSMutableArray arrayWithArray: [ super accessibilityAttributeNames ] ];
#else	// USE_JAVA
    // Default Attributes
    NSMutableArray * attributeNames = [ NSMutableArray arrayWithArray: [ super accessibilityAttributeNames ] ];
#endif	// USE_JAVA
    // Special Attributes and removing unwanted attributes depending on role
    [ attributeNames removeObject: NSAccessibilityEnabledAttribute ];
    [ attributeNames addObjectsFromArray: [ NSArray arrayWithObjects:
            NSAccessibilityContentsAttribute, 
            NSAccessibilityVerticalScrollBarAttribute, 
            NSAccessibilityHorizontalScrollBarAttribute, 
            nil ]
    ];
#ifdef USE_JAVA
    RELEASE_DRAGPRINTLOCK
    if ( !attributeNames )
        return [ NSArray array ];
    else
#endif	// USE_JAVA
    return attributeNames;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
