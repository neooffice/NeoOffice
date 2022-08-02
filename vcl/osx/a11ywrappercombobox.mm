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

#include "a11ywrappercombobox.h"
#include "a11yrolehelper.h"

#include <com/sun/star/accessibility/AccessibleStateType.hpp>

#ifdef USE_JAVA
#include "../java/source/app/salinst_cocoa.h"
#endif	// USE_JAVA

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

// Wrapper for AXCombobox role

@implementation AquaA11yWrapperComboBox : AquaA11yWrapper

#pragma mark -
#pragma mark Specialized Init Method

-(id)initWithAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext {
    self = [ super initWithAccessibleContext: rxAccessibleContext ];
    if ( self != nil )
    {
        textArea = nil;
    }
    return self;
}

#pragma mark -
#pragma mark Private Helper Method

-(AquaA11yWrapper *)textArea {
    // FIXME: May cause problems when stored. Then get dynamically each time (bad performance!)
    if ( textArea == nil ) {
        NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
        NSArray * elementChildren = [ super childrenAttribute ];
        if ( [ elementChildren count ] > 0 ) {
            NSEnumerator * enumerator = [ elementChildren objectEnumerator ];
            id child;
            while ( ( child = [ enumerator nextObject ] ) ) {
                AquaA11yWrapper * element = ( AquaA11yWrapper * ) child;
                if ( [ [ AquaA11yRoleHelper getNativeRoleFrom: [ element accessibleContext ] ] isEqualToString: NSAccessibilityTextAreaRole ] ) {
                    textArea = element;
                    break;
                }
            }
        }
        [ pool release ];
    }
    return textArea;
}

#pragma mark -
#pragma mark Wrapped Attributes From Contained Text Area

-(id)valueAttribute {
    if ( [ self textArea ] != nil ) {
        return [ [ self textArea ] valueAttribute ];
    }
    return @"";
}

-(id)numberOfCharactersAttribute {
    if ( [ self textArea ] != nil ) {
        return [ [ self textArea ] numberOfCharactersAttribute ];
    }
    return [ NSNumber numberWithInt: 0 ];
}

-(id)selectedTextAttribute {
    if ( [ self textArea ] != nil ) {
        return [ [ self textArea ] selectedTextAttribute ];
    }
    return @"";
}

-(id)selectedTextRangeAttribute {
    if ( [ self textArea ] != nil ) {
        return [ [ self textArea ] selectedTextRangeAttribute ];
    }
    return [ NSValue valueWithRange: NSMakeRange ( 0, 0 ) ];
}

-(id)visibleCharacterRangeAttribute {
    if ( [ self textArea ] != nil ) {
        return [ [ self textArea ] visibleCharacterRangeAttribute ];
    }
    return [ NSValue valueWithRange: NSMakeRange ( 0, 0 ) ];
}

#pragma mark -
#pragma mark Accessibility Protocol

-(BOOL)accessibilityIsAttributeSettable:(NSString *)attribute {
#ifdef USE_JAVA
    BOOL isSettable = NO;
    if ( !ImplApplicationIsRunning() )
        return isSettable;
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    if ( [ self isDisposed ] ) {
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return [ NSArray array ];
    }
#endif	// USE_JAVA
    if ( [ self textArea ] != nil && (
         [ attribute isEqualToString: NSAccessibilitySelectedTextAttribute ]
      || [ attribute isEqualToString: NSAccessibilitySelectedTextRangeAttribute ]
      || [ attribute isEqualToString: NSAccessibilityVisibleCharacterRangeAttribute ] ) ) {
#ifdef USE_JAVA
        isSettable = [ [ self textArea ] accessibilityIsAttributeSettable: attribute ];
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return isSettable;
#else	// USE_JAVA
        return [ [ self textArea ] accessibilityIsAttributeSettable: attribute ];
#endif	// USE_JAVA
    }
#ifdef USE_JAVA
    isSettable = [ super accessibilityIsAttributeSettable: attribute ];
    RELEASE_DRAGPRINTLOCK
    return isSettable;
#else	// USE_JAVA
    return [ super accessibilityIsAttributeSettable: attribute ];
#endif	// USE_JAVA
}

-(void)accessibilitySetValue:(id)value forAttribute:(NSString *)attribute {
#ifdef USE_JAVA
    if ( !ImplApplicationIsRunning() )
        return;
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    if ( [ self isDisposed ] ) {
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return;
    }
#endif	// USE_JAVA
    if ( [ self textArea ] != nil && (
         [ attribute isEqualToString: NSAccessibilitySelectedTextAttribute ]
      || [ attribute isEqualToString: NSAccessibilitySelectedTextRangeAttribute ]
      || [ attribute isEqualToString: NSAccessibilityVisibleCharacterRangeAttribute ] ) ) {
#ifdef USE_JAVA
        [ [ self textArea ] accessibilitySetValue: value forAttribute: attribute ];
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return;
#else	// USE_JAVA
        return [ [ self textArea ] accessibilitySetValue: value forAttribute: attribute ];
#endif	// USE_JAVA
    }
#ifdef USE_JAVA
    [ super accessibilitySetValue: value forAttribute: attribute ];
    RELEASE_DRAGPRINTLOCK
    return;
#else	// USE_JAVA
    return [ super accessibilitySetValue: value forAttribute: attribute ];
#endif	// USE_JAVA
}

-(NSArray *)accessibilityAttributeNames {
#ifdef USE_JAVA
    NSMutableArray * attributeNames = nil;
    if ( !ImplApplicationIsRunning() )
        return [ NSArray array ];
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    if ( [ self isDisposed ] ) {
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return [ NSArray array ];
    }
    // Default Attributes
    attributeNames = [ NSMutableArray arrayWithArray: [ super accessibilityAttributeNames ] ];
#else	// USE_JAVA
    // Default Attributes
    NSMutableArray * attributeNames = [ NSMutableArray arrayWithArray: [ super accessibilityAttributeNames ] ];
#endif	// USE_JAVA
    // Special Attributes and removing unwanted attributes depending on role
    [ attributeNames removeObjectsInArray: [ NSArray arrayWithObjects:
            NSAccessibilityTitleAttribute, 
            NSAccessibilityChildrenAttribute, 
            nil ]
    ];
    [ attributeNames addObjectsFromArray: [ NSArray arrayWithObjects:
            NSAccessibilityExpandedAttribute, 
            NSAccessibilityValueAttribute, 
            NSAccessibilityNumberOfCharactersAttribute, 
            NSAccessibilitySelectedTextAttribute, 
            NSAccessibilitySelectedTextRangeAttribute, 
            NSAccessibilityVisibleCharacterRangeAttribute, 
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
