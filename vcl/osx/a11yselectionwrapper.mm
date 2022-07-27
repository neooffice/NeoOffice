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
#include "osx/a11yfactory.h"

#include "a11yselectionwrapper.h"

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

@implementation AquaA11ySelectionWrapper : NSObject

+(id)selectedChildrenAttributeForElement:(AquaA11yWrapper *)wrapper
{
    Reference< XAccessibleSelection > xAccessibleSelection = [ wrapper accessibleSelection ];
    if( xAccessibleSelection.is() )
    {
#ifndef USE_JAVA
        NSMutableArray * children = [ [ NSMutableArray alloc ] init ];
#endif	// !USE_JAVA
        try {
            sal_Int32 n = xAccessibleSelection -> getSelectedAccessibleChildCount();
#ifdef USE_JAVA
            NSMutableArray * children = [ NSMutableArray arrayWithCapacity: n ];
#endif	// USE_JAVA
            for ( sal_Int32 i=0 ; i < n ; ++i ) {
                [ children addObject: [ AquaA11yFactory wrapperForAccessible: xAccessibleSelection -> getSelectedAccessibleChild( i ) ] ];
            }

            return children;

        } catch ( Exception& e)
        {
        }
    }

    return nil;
}


+(void)addAttributeNamesTo:(NSMutableArray *)attributeNames
{
    [ attributeNames addObject: NSAccessibilitySelectedChildrenAttribute ];
}

+(BOOL)isAttributeSettable:(NSString *)attribute forElement:(AquaA11yWrapper *)wrapper
{
    (void)wrapper;
    if ( [ attribute isEqualToString: NSAccessibilitySelectedChildrenAttribute ] )
    {
        return YES;
    }
    else
    {
        return NO;
    }
}

+(void)setSelectedChildrenAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value
{
    Reference< XAccessibleSelection > xAccessibleSelection = [ wrapper accessibleSelection ];
    try {
        xAccessibleSelection -> clearAccessibleSelection();

        unsigned c = [ value count ];
        for ( unsigned i = 0 ; i < c ; ++i ) {
            xAccessibleSelection -> selectAccessibleChild( [ [ value objectAtIndex: i ] accessibleContext ] -> getAccessibleIndexInParent() );
        }
    } catch ( Exception& e) {
    }
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
