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


#include "osx/a11yfactory.h"

#include "a11ytablewrapper.h"

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;

@implementation AquaA11yTableWrapper : AquaA11yWrapper

+(id)childrenAttributeForElement:(AquaA11yTableWrapper *)wrapper
{
    XAccessibleTable * accessibleTable = [ wrapper accessibleTable ];
    NSArray* pResult = nil;
    if( accessibleTable )
    {
#ifdef USE_JAVA
        NSMutableArray * cells = nil;
#else	// USE_JAVA
        NSMutableArray * cells = [ [ NSMutableArray alloc ] init ];
#endif	// USE_JAVA
        try
        {
            sal_Int32 nRows = accessibleTable->getAccessibleRowCount();
            sal_Int32 nCols = accessibleTable->getAccessibleColumnCount();
    
            if( nRows * nCols < MAXIMUM_ACCESSIBLE_TABLE_CELLS )
            {
                // make all children visible to the hierarchy
#ifdef USE_JAVA
                cells = [ NSMutableArray arrayWithCapacity: nRows * nCols ];
                if ( cells )
                {
#endif	// USE_JAVA
                for ( sal_Int32 rowCount = 0; rowCount < nRows; rowCount++ )
                {
                    for ( sal_Int32 columnCount = 0; columnCount < nCols; columnCount++ )
                    {
                        Reference < XAccessible > rAccessibleCell = accessibleTable -> getAccessibleCellAt ( rowCount, columnCount );
                        if ( rAccessibleCell.is() )
                        {
                            id cell_wrapper = [ AquaA11yFactory wrapperForAccessibleContext: rAccessibleCell -> getAccessibleContext() ];
                            [ cells addObject: cell_wrapper ];
#ifndef USE_JAVA
                            [ cell_wrapper release ];
#endif	// !USE_JAVA
                        }
                    }
#ifdef USE_JAVA
                }
#endif	// USE_JAVA
                }
            }
            else
            {
                XAccessibleComponent * accessibleComponent = [ wrapper accessibleComponent ];
                // find out which cells are actually visible by determining the top-left-cell and the bottom-right-cell
                Size tableSize = accessibleComponent -> getSize();
                Point point;
                point.X = 0;
                point.Y = 0;
                Reference < XAccessible > rAccessibleTopLeft = accessibleComponent -> getAccessibleAtPoint ( point );
                point.X = tableSize.Width - 1;
                point.Y = tableSize.Height - 1;
                Reference < XAccessible > rAccessibleBottomRight = accessibleComponent -> getAccessibleAtPoint ( point );
                if ( rAccessibleTopLeft.is() && rAccessibleBottomRight.is() )
                {
                    sal_Int32 idxTopLeft = rAccessibleTopLeft -> getAccessibleContext() -> getAccessibleIndexInParent();
                    sal_Int32 idxBottomRight = rAccessibleBottomRight -> getAccessibleContext() -> getAccessibleIndexInParent();
                    sal_Int32 rowTopLeft = accessibleTable -> getAccessibleRow ( idxTopLeft );
                    sal_Int32 columnTopLeft = accessibleTable -> getAccessibleColumn ( idxTopLeft );
                    sal_Int32 rowBottomRight = accessibleTable -> getAccessibleRow ( idxBottomRight );
                    sal_Int32 columnBottomRight = accessibleTable -> getAccessibleColumn ( idxBottomRight );
                    // create an array containing the visible cells
#ifdef USE_JAVA
                    cells = [ NSMutableArray arrayWithCapacity: ( rowBottomRight - rowTopLeft + 1 ) * ( columnBottomRight - columnTopLeft + 1 ) ];
                    if ( cells )
                    {
#endif	// USE_JAVA
                    for ( sal_Int32 rowCount = rowTopLeft; rowCount <= rowBottomRight; rowCount++ )
                    {
                        for ( sal_Int32 columnCount = columnTopLeft; columnCount <= columnBottomRight; columnCount++ )
                        {
                            Reference < XAccessible > rAccessibleCell = accessibleTable -> getAccessibleCellAt ( rowCount, columnCount );
                            if ( rAccessibleCell.is() )
                            {
                                id cell_wrapper = [ AquaA11yFactory wrapperForAccessibleContext: rAccessibleCell -> getAccessibleContext() ];
                                [ cells addObject: cell_wrapper ];
#ifndef USE_JAVA
                                [ cell_wrapper release ];
#endif	// !USE_JAVA
                            }
                        }
                    }
#ifdef USE_JAVA
                    }
#endif	// USE_JAVA
                }
            }
#ifdef USE_JAVA
            if ( cells )
                pResult = NSAccessibilityUnignoredChildren( cells ? cells : [ NSArray array ] );
#else	// USE_JAVA
            pResult = NSAccessibilityUnignoredChildren( cells );
#endif	// USE_JAVA
        }
        catch (const Exception &e) 
        {
        }
#ifndef USE_JAVA
        [cells autorelease];
#endif	// !USE_JAVA
    }
    
    return pResult;
}

+(void)addAttributeNamesTo: (NSMutableArray *)attributeNames object: (AquaA11yWrapper*)pObject
{
    XAccessibleTable * accessibleTable = [ pObject accessibleTable ];
    if( accessibleTable )
    {
        sal_Int32 nRows = accessibleTable->getAccessibleRowCount();
        sal_Int32 nCols = accessibleTable->getAccessibleColumnCount();    
        
        
        if( nRows*nCols < MAXIMUM_ACCESSIBLE_TABLE_CELLS )
        {
            [ attributeNames addObject: NSAccessibilityRowsAttribute ];
            [ attributeNames addObject: NSAccessibilityColumnsAttribute ];
        }
    }
}

-(id)rowsAttribute
{
    NSArray* pResult = nil;

    XAccessibleTable * accessibleTable = [ self accessibleTable ];
    if( accessibleTable )
    {
        sal_Int32 nRows = accessibleTable->getAccessibleRowCount();
        sal_Int32 nCols = accessibleTable->getAccessibleColumnCount();    
        if( nRows * nCols < MAXIMUM_ACCESSIBLE_TABLE_CELLS )
        {
#ifndef USE_JAVA
            NSMutableArray * cells = [ [ NSMutableArray alloc ] init ];
#endif	// !USE_JAVA
            try
            {
                // find out number of rows
                sal_Int32 nRows = accessibleTable->getAccessibleRowCount();
#ifdef USE_JAVA
                NSMutableArray * cells = [ NSMutableArray arrayWithCapacity: nRows ];
                if ( cells )
                {
#endif	// USE_JAVA
                for( sal_Int32 n = 0; n < nRows; n++ )
                {
                    Reference < XAccessible > rAccessibleCell = accessibleTable -> getAccessibleCellAt ( n, 0 );
                    if ( rAccessibleCell.is() )
                    {
                        id cell_wrapper = [ AquaA11yFactory wrapperForAccessibleContext: rAccessibleCell -> getAccessibleContext() ];
                        [ cells addObject: cell_wrapper ];
#ifndef USE_JAVA
                        [ cell_wrapper release ];
#endif	// !USE_JAVA
                    }
                }
                pResult = NSAccessibilityUnignoredChildren( cells );
#ifdef USE_JAVA
                }
#endif	// USE_JAVA
            }
            catch (const Exception &e) 
            {
                pResult = nil;
            }
#ifndef USE_JAVA
            [ cells autorelease ];
#endif	// !USE_JAVA
        }
    }
    
    return pResult;
}

-(id)columnsAttribute
{
    NSArray* pResult = nil;

    XAccessibleTable * accessibleTable = [ self accessibleTable ];
    
    if( accessibleTable )
    {
        sal_Int32 nRows = accessibleTable->getAccessibleRowCount();
        sal_Int32 nCols = accessibleTable->getAccessibleColumnCount();    
        if( nRows * nCols < MAXIMUM_ACCESSIBLE_TABLE_CELLS )
        {
#ifndef USE_JAVA
            NSMutableArray * cells = [ [ NSMutableArray alloc ] init ];
#endif	// !USE_JAVA
            try
            {
                // find out number of columns
#ifdef USE_JAVA
                NSMutableArray * cells = [ NSMutableArray arrayWithCapacity: nCols ];
#endif	// USE_JAVA
                for( sal_Int32 n = 0; n < nCols; n++ )
                {
                    Reference < XAccessible > rAccessibleCell = accessibleTable -> getAccessibleCellAt ( 0, n );
                    if ( rAccessibleCell.is() )
                    {
                        id cell_wrapper = [ AquaA11yFactory wrapperForAccessibleContext: rAccessibleCell -> getAccessibleContext() ];
                        [ cells addObject: cell_wrapper ];
#ifndef USE_JAVA
                        [ cell_wrapper release ];
#endif	// !USE_JAVA
                    }
                }
                pResult = NSAccessibilityUnignoredChildren( cells );
            }
            catch (const Exception &e) 
            {
                pResult = nil;
            }
#ifndef USE_JAVA
            [ cells autorelease ];
#endif	// !USE_JAVA
        }
    }
    
    return pResult;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
