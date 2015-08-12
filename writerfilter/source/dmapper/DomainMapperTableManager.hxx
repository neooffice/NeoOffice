/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * This file is part of NeoOffice.
 *
 * NeoOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * NeoOffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 3 along with NeoOffice.  If not, see
 * <http://www.gnu.org/licenses/gpl-3.0.txt>
 * for a copy of the GPLv3 License.
 *
 * Modified February 2013 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/
#ifndef INCLUDED_DOMAIN_MAPPER_TABLE_MANAGER_HXX
#define INCLUDED_DOMAIN_MAPPER_TABLE_MANAGER_HXX

#include "TablePropertiesHandler.hxx"

#include <resourcemodel/TableManager.hxx>
#include <PropertyMap.hxx>
#include <StyleSheetTable.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <vector>

namespace writerfilter {
namespace dmapper {

class DomainMapperTableManager : public DomainMapperTableManager_Base_t
{
    typedef boost::shared_ptr< std::vector<sal_Int32> > IntVectorPtr;

    sal_uInt32      m_nRow;
#ifdef NO_LIBO_4_0_TABLE_FIXES
    sal_uInt32      m_nCell;
#else	// NO_LIBO_4_0_TABLE_FIXES
    ::std::vector< sal_uInt32 > m_nCell;
#endif	// NO_LIBO_4_0_TABLE_FIXES
    sal_uInt32      m_nGridSpan;
#ifndef NO_LIBO_4_0_TABLE_FIXES
    sal_uInt32      m_nGridBefore; ///< number of grid columns in the parent table's table grid which must be skipped before the contents of this table row are added to the parent table
    sal_uInt32      m_nGridAfter; ///< number of grid columns in the parent table's table grid which shall be left after the last cell in the table row
#endif	// !NO_LIBO_4_0_TABLE_FIXES
    sal_uInt32      m_nCellBorderIndex; //borders are provided for all cells and need counting
    sal_Int32       m_nHeaderRepeat; //counter of repeated headers - if == -1 then the repeating stops
    sal_Int32       m_nTableWidth; //might be set directly or has to be calculated from the column positions
    bool            m_bOOXML;
    ::rtl::OUString m_sTableStyleName;    
    PropertyMapPtr  m_pTableStyleTextProperies;

    ::std::vector< IntVectorPtr >  m_aTableGrid;
    ::std::vector< IntVectorPtr >  m_aGridSpans;
#ifndef NO_LIBO_4_0_TABLE_FIXES
    /// If this is true, then we pushed a width before the next level started, and that should be carried over when starting the next level.
    bool            m_bPushCurrentWidth;
    /// Individual table cell width values, used only in case the number of cells doesn't match the table grid.
    ::std::vector< IntVectorPtr >  m_aCellWidths;
#endif	// !NO_LIBO_4_0_TABLE_FIXES
    
    TablePropertiesHandler   *m_pTablePropsHandler;
    PropertyMapPtr            m_pStyleProps;

    virtual void clearData();

public:

    DomainMapperTableManager(bool bOOXML);
    virtual ~DomainMapperTableManager();

    // use this method to avoid adding the properties for the table
    // but in the provided properties map.
    inline void SetStyleProperties( PropertyMapPtr pProperties ) { m_pStyleProps = pProperties; };

    virtual bool sprm(Sprm & rSprm);

    virtual void startLevel( );
    virtual void endLevel( );

    virtual void endOfCellAction();
    virtual void endOfRowAction();

    IntVectorPtr getCurrentGrid( );
    IntVectorPtr getCurrentSpans( );
#ifndef NO_LIBO_4_0_TABLE_FIXES
    IntVectorPtr getCurrentCellWidths( );
#endif	// !NO_LIBO_4_0_TABLE_FIXES

    const ::rtl::OUString& getTableStyleName() const { return m_sTableStyleName; }
    /// copy the text properties of the table style and its parent into pContext
    void    CopyTextProperties(PropertyMapPtr pContext, StyleSheetTablePtr pStyleSheetTable);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
