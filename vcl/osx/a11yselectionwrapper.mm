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
            // Fix issue #11 by limiting the maximum number of child views that
            // can be attached to the window
            if ( n < 0 )
                n = 0;
            else if ( n > SAL_MAX_UINT8 )
                n = SAL_MAX_UINT8;

            NSMutableArray * children = [ NSMutableArray arrayWithCapacity: n ];
#endif	// USE_JAVA
            for ( sal_Int32 i=0 ; i < n ; ++i ) {
#ifdef USE_JAVA
                // Eliminate exception thrown when opening the Tools > Options
                // dialog by skipping NULL wrappers
                AquaA11yWrapper * child_wrapper = [ AquaA11yFactory wrapperForAccessible: xAccessibleSelection -> getSelectedAccessibleChild( i ) ];
                if ( child_wrapper && ImplIsValidAquaA11yWrapper( child_wrapper ) && ! [ child_wrapper isDisposed ] )
                    [ children addObject: child_wrapper ];
#else	// USE_JAVA
                [ children addObject: [ AquaA11yFactory wrapperForAccessible: xAccessibleSelection -> getSelectedAccessibleChild( i ) ] ];
#endif	// USE_JAVA
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
#ifdef USE_JAVA
    if( xAccessibleSelection.is() && value )
    {
#endif	// USE_JAVA
    try {
        xAccessibleSelection -> clearAccessibleSelection();

#ifdef USE_JAVA
        for ( AquaA11yWrapper *element : value ) {
            if ( element && ImplIsValidAquaA11yWrapper( element ) && ! [ element isDisposed ] && [ element accessibleContext ] )
                xAccessibleSelection -> selectAccessibleChild( [ element accessibleContext ] -> getAccessibleIndexInParent() );
#else	// USE_JAVA
        unsigned c = [ value count ];
        for ( unsigned i = 0 ; i < c ; ++i ) {
            xAccessibleSelection -> selectAccessibleChild( [ [ value objectAtIndex: i ] accessibleContext ] -> getAccessibleIndexInParent() );
#endif	// USE_JAVA
        }
    } catch ( Exception& e) {
    }
#ifdef USE_JAVA
    }
#endif	// USE_JAVA
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
