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
#include "quartz/utils.h"

#include "a11yactionwrapper.h"

// Wrapper for XAccessibleAction

@implementation AquaA11yActionWrapper : NSObject

+(NSString *)nativeActionNameFor:(NSString *)actionName {
    // TODO: Optimize ?
    //       Use NSAccessibilityActionDescription
    if ( [ actionName isEqualToString: @"press" ] ) {
        return NSAccessibilityPressAction;
    } else if ( [ actionName isEqualToString: @"togglePopup" ] ) {
        return NSAccessibilityShowMenuAction;
    } else if ( [ actionName isEqualToString: @"select" ] ) {
        return NSAccessibilityPickAction;
    } else if ( [ actionName isEqualToString: @"incrementLine" ] ) {
        return NSAccessibilityIncrementAction;
    } else if ( [ actionName isEqualToString: @"decrementLine" ] ) {
        return NSAccessibilityDecrementAction;
    } else if ( [ actionName isEqualToString: @"incrementBlock" ] ) {
        return NSAccessibilityIncrementAction; // TODO ?
    } else if ( [ actionName isEqualToString: @"decrementBlock" ] ) {
        return NSAccessibilityDecrementAction; // TODO ?
    } else if ( [ actionName isEqualToString: @"Browse" ] ) {
        return NSAccessibilityPressAction; // TODO ?
    } else {
        return [ NSString string ];
    }
}

+(NSArray *)actionNamesForElement:(AquaA11yWrapper *)wrapper {
#ifdef USE_JAVA
    NSMutableArray * actionNames = [ NSMutableArray arrayWithCapacity: 10 ];
#else	// USE_JAVA
    NSMutableArray * actionNames = [ [ NSMutableArray alloc ] init ];
#endif	// USE_JAVA
    if ( [ wrapper accessibleAction ] != nil ) {
        for ( int cnt = 0; cnt < [ wrapper accessibleAction ] -> getAccessibleActionCount(); cnt++ ) {
#ifdef USE_JAVA
            [ actionNames addObject: [ AquaA11yActionWrapper nativeActionNameFor: [ CreateNSString ( [ wrapper accessibleAction ] -> getAccessibleActionDescription ( cnt ) ) autorelease ] ] ];
#else	// USE_JAVA
            [ actionNames addObject: [ AquaA11yActionWrapper nativeActionNameFor: CreateNSString ( [ wrapper accessibleAction ] -> getAccessibleActionDescription ( cnt ) ) ] ];
#endif	// USE_JAVA
        }
    }
    return actionNames;
}

+(void)doAction:(NSString *)action ofElement:(AquaA11yWrapper *)wrapper {
    if ( [ wrapper accessibleAction ] != nil ) {
        for ( int cnt = 0; cnt < [ wrapper accessibleAction ] -> getAccessibleActionCount(); cnt++ ) {
#ifdef USE_JAVA
            if ( [ action isEqualToString: [ AquaA11yActionWrapper nativeActionNameFor: [ CreateNSString ( [ wrapper accessibleAction ] -> getAccessibleActionDescription ( cnt ) ) autorelease ] ] ] ) {
#else	// USE_JAVA
            if ( [ action isEqualToString: [ AquaA11yActionWrapper nativeActionNameFor: CreateNSString ( [ wrapper accessibleAction ] -> getAccessibleActionDescription ( cnt ) ) ] ] ) {
#endif	// USE_JAVA
                [ wrapper accessibleAction ] -> doAccessibleAction ( cnt );
                break;
            }
        }
    }
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
