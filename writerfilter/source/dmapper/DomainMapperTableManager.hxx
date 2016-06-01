/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 * This file incorporates work covered by the following license notice:
 * 
 *   Portions of this file are part of the LibreOffice project.
 *
 *   This Source Code Form is subject to the terms of the Mozilla Public
 *   License, v. 2.0. If a copy of the MPL was not distributed with this
 *   file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *************************************************************/


#ifndef INCLUDED_DOMAIN_MAPPER_TABLE_MANAGER_HXX
#define INCLUDED_DOMAIN_MAPPER_TABLE_MANAGER_HXX

#include "TablePropertiesHandler.hxx"

#include <resourcemodel/TableManager.hxx>
#include <PropertyMap.hxx>
#include <StyleSheetTable.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <vector>
#include <stack>

namespace writerfilter {
namespace dmapper {

class DomainMapperTableManager : public DomainMapperTableManager_Base_t
{
    typedef boost::shared_ptr< std::vector<sal_Int32> > IntVectorPtr;

    ::std::stack< sal_uInt32 > m_nCellCounterForCurrentRow;
    sal_uInt32 m_nGridSpanOfCurrentCell;
#ifndef NO_LIBO_4_0_TABLE_FIXES
    sal_uInt32      m_nGridBefore; ///< number of grid columns in the parent table's table grid which must be skipped before the contents of this table row are added to the parent table
    sal_uInt32      m_nGridAfter; ///< number of grid columns in the parent table's table grid which shall be left after the last cell in the table row
#endif	// !NO_LIBO_4_0_TABLE_FIXES
    ::std::stack< sal_uInt32 > m_nCurrentCellBorderIndex; //borders are provided for all cells and need counting
    ::std::stack< sal_Int32 > m_nCurrentHeaderRepeatCount; //counter of repeated headers - if == -1 then the repeating stops
    ::std::stack< sal_Int32 > m_nTableWidthOfCurrentTable; //might be set directly or has to be calculated from the column positions
    bool            m_bOOXML;

    ::std::stack< IntVectorPtr >  m_aTableGrid;
    ::std::stack< IntVectorPtr >  m_aGridSpans;
#ifndef NO_LIBO_4_0_TABLE_FIXES
    /// If this is true, then we pushed a width before the next level started, and that should be carried over when starting the next level.
    bool            m_bPushCurrentWidth;
    /// Individual table cell width values, used only in case the number of cells doesn't match the table grid.
    ::std::vector< IntVectorPtr >  m_aCellWidths;
#endif	// !NO_LIBO_4_0_TABLE_FIXES
    
    TablePropertiesHandler   *m_pTablePropsHandler;
    PropertyMapPtr            m_pStyleProps;

    void pushStackOfMembers();
    void popStackOfMembers();

    IntVectorPtr getCurrentGrid();
    IntVectorPtr getCurrentSpans( );
#ifndef NO_LIBO_4_0_TABLE_FIXES
    IntVectorPtr getCurrentCellWidths( );
#endif	// !NO_LIBO_4_0_TABLE_FIXES

public:

    DomainMapperTableManager(bool bOOXML);
    virtual ~DomainMapperTableManager();

    // use this method to avoid adding the properties for the table
    // but in the provided properties map.
    void SetStyleProperties( PropertyMapPtr pProperties );

    virtual bool sprm(Sprm & rSprm);

    virtual void startLevel();
    virtual void endLevel();

    virtual void endOfCellAction();
    virtual void endOfRowAction();

    inline virtual void cellProps(TablePropertyMapPtr pProps)
    {
        if ( m_pStyleProps.get( ) )
            m_pStyleProps->insert( pProps, true );
        else
           DomainMapperTableManager_Base_t::cellProps( pProps );
    };

    inline virtual void cellPropsByCell(unsigned int i, TablePropertyMapPtr pProps)
    {
        if ( m_pStyleProps.get( ) )
            m_pStyleProps->insert( pProps, true );
        else
           DomainMapperTableManager_Base_t::cellPropsByCell( i, pProps );
    };

    inline virtual void insertRowProps(TablePropertyMapPtr pProps)
    {
        if ( m_pStyleProps.get( ) )
            m_pStyleProps->insert( pProps, true );
        else
           DomainMapperTableManager_Base_t::insertRowProps( pProps );
    };

    inline virtual void insertTableProps(TablePropertyMapPtr pProps)
    {
        if ( m_pStyleProps.get( ) )
            m_pStyleProps->insert( pProps, true );
        else
           DomainMapperTableManager_Base_t::insertTableProps( pProps );
    };
    
};

}}

#endif // INCLUDED_DOMAIN_MAPPER_TABLE_MANAGER_HXX
