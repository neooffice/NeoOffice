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
#ifndef NO_LIBO_4_0_TABLE_FIXES
#include <boost/optional.hpp>
#endif	// !NO_LIBO_4_0_TABLE_FIXES
#include <DomainMapperTableManager.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <BorderHandler.hxx>
#include <CellColorHandler.hxx>
#include <CellMarginHandler.hxx>
#include <ConversionHelper.hxx>
#include <MeasureHandler.hxx>
#include <TDefTableHandler.hxx>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <ooxml/resourceids.hxx>
#include <doctok/sprmids.hxx>
#include <dmapperLoggers.hxx>

namespace writerfilter {
namespace dmapper {
    
using namespace ::com::sun::star;
using namespace ::std;
/*-- 23.04.2007 14:57:49---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapperTableManager::DomainMapperTableManager(bool bOOXML) :
    m_nRow(0),
#ifdef NO_LIBO_4_0_TABLE_FIXES
    m_nCell(0),
#else	// NO_LIBO_4_0_TABLE_FIXES
    m_nCell(),
#endif	// NO_LIBO_4_0_TABLE_FIXES
    m_nGridSpan(1),
#ifndef NO_LIBO_4_0_TABLE_FIXES
    m_nGridBefore(0),
    m_nGridAfter(0),
#endif	// !NO_LIBO_4_0_TABLE_FIXES
    m_nCellBorderIndex(0),
    m_nHeaderRepeat(0),
    m_nTableWidth(0),
    m_bOOXML( bOOXML ),
#ifndef NO_LIBO_4_0_TABLE_FIXES
    m_bPushCurrentWidth(false),
#endif	// !NO_LIBO_4_0_TABLE_FIXES
    m_pTablePropsHandler( new TablePropertiesHandler( bOOXML ) )
{
    m_pTablePropsHandler->SetTableManager( this );
    
#ifdef DEBUG_DOMAINMAPPER
#ifdef DEBUG_TABLE
    setTagLogger(dmapper_logger);
#endif
#endif
}
/*-- 23.04.2007 14:57:49---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapperTableManager::~DomainMapperTableManager()
{
    if ( m_pTablePropsHandler )
        delete m_pTablePropsHandler, m_pTablePropsHandler = NULL;
}
/*-- 23.04.2007 15:25:37---------------------------------------------------

  -----------------------------------------------------------------------*/
bool DomainMapperTableManager::sprm(Sprm & rSprm)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("tablemanager.sprm");
    string sSprm = rSprm.toString();
    dmapper_logger->chars(sSprm);
    dmapper_logger->endElement("tablemanager.sprm");
#endif
    bool bRet = DomainMapperTableManager_Base_t::sprm(rSprm);
    if( !bRet )
    {
        bRet = m_pTablePropsHandler->sprm( rSprm );
    }

    if ( !bRet )
    {
        bRet = true;
        sal_uInt32 nSprmId = rSprm.getId();
        Value::Pointer_t pValue = rSprm.getValue();
        sal_Int32 nIntValue = ((pValue.get() != NULL) ? pValue->getInt() : 0);
        switch ( nSprmId )
        {
            case 0xf661: //sprmTTRLeft left table indent
                /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
            case 0xf614: // sprmTTPreferredWidth - preferred table width
                /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TblPrBase_tblW:  //90722;
                /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
            case NS_ooxml::LN_CT_TblPrBase_tblInd: //90725
                /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
            {
                //contains unit and value 
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {   //contains attributes x2902 (LN_unit) and x17e2 (LN_trleft)
                    MeasureHandlerPtr pMeasureHandler( new MeasureHandler );
                    pProperties->resolve(*pMeasureHandler);
                    TablePropertyMapPtr pPropMap( new TablePropertyMap );
                    if( nSprmId == 0xf661 || nSprmId == sal_uInt32(NS_ooxml::LN_CT_TblPrBase_tblInd ))
                    {
                        pPropMap->setValue( TablePropertyMap::LEFT_MARGIN, pMeasureHandler->getMeasureValue() );
                    }
                    else
                    {
                        m_nTableWidth = pMeasureHandler->getMeasureValue();
                        if( m_nTableWidth )
                            pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, m_nTableWidth );
                    }
#ifdef DEBUG_DOMAINMAPPER
                    dmapper_logger->addTag(pPropMap->toTag());
#endif
                    insertTableProps(pPropMap);
                }
            }
            break;
            case 0x3404:// sprmTTableHeader
            case NS_ooxml::LN_CT_TrPrBase_tblHeader: //90704  
                /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
                // if nIntValue == 1 then the row is a repeated header line
                // to prevent later rows from increasing the repeating m_nHeaderRepeat is set to NULL when repeating stops
                if( nIntValue > 0 && m_nHeaderRepeat >= 0 ) 
                {
                    ++m_nHeaderRepeat;
                    TablePropertyMapPtr pPropMap( new TablePropertyMap );
                    pPropMap->Insert( PROP_HEADER_ROW_COUNT, false, uno::makeAny( m_nHeaderRepeat ));
                    insertTableProps(pPropMap);
                }
                else
                    m_nHeaderRepeat = -1;
            break;  
            case 0xd608: // TDefTable
                /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
            {    
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    TDefTableHandlerPtr pTDefTableHandler( new TDefTableHandler(m_bOOXML) );
                    pProperties->resolve( *pTDefTableHandler );
                    
                    TablePropertyMapPtr pRowPropMap( new TablePropertyMap );
                    pRowPropMap->insert( pTDefTableHandler->getRowProperties() );
                    insertRowProps( pRowPropMap );
                    if( !m_nTableWidth )
                    {
                        m_nTableWidth= pTDefTableHandler->getTableWidth();
                        if( m_nTableWidth )
                        {
                            TablePropertyMapPtr pPropMap( new TablePropertyMap );
                            pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, m_nTableWidth );
                            insertTableProps(pPropMap);
                        }
                    }
                    for( size_t nCell = 0; nCell < pTDefTableHandler->getCellCount(); ++nCell )
                    {
                        TablePropertyMapPtr pCellPropMap( new TablePropertyMap );
                        pTDefTableHandler->fillCellProperties( nCell, pCellPropMap );
                        cellPropsByCell( nCell, pCellPropMap );
                    }
                }    
            }
            break;
            case 0xD605: // sprmTTableBorders
                /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    BorderHandlerPtr pBorderHandler( new BorderHandler(m_bOOXML) );
                    pProperties->resolve(*pBorderHandler);
                    TablePropertyMapPtr pCellPropMap( new TablePropertyMap() );
                    pCellPropMap->insert( pBorderHandler->getProperties() );
                    cellPropsByCell( m_nCellBorderIndex, pCellPropMap );
                    ++m_nCellBorderIndex;
                }
            }
            break;
            case 0xd632 : //sprmTNewSpacing
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
            case 0xd634 : //sprmTNewSpacing
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
                //TODO: sprms contain default (TNew) and actual border spacing of cells - not resolvable yet
            break;
            case 0xd613: //sprmTGridLineProps
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
                // TODO: needs a handler 
                /*contains: 
                 GridLineProps">
                    rtf:LINEPROPSTOP
                    rtf:LINEPROPSLEFT
                    rtf:LINEPROPSBOTTOM
                    rtf:LINEPROPSRIGHT
                    rtf:LINEPROPSHORIZONTAL
                    rtf:LINEPROPSVERTICAL
                        rtf:LINECOLOR
                        rtf:LINEWIDTH
                        rtf:LINETYPE
                
                */
            break;
            case 0x740a : //sprmTTlp
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
                //TODO: Table look specifier
            break;
            case 0x6816 : //unknown
            case 0x3466 : //unknown
            case 0x3615 : //unknown
            case 0x646b : //unknown - expandable sprm - see ww8scan.cxx
            case 0x7479 : //unknown
            case 0xf617 : //unknown
            case 0xf618 : //unknown
                /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
                bRet = false;
            break;
            case NS_ooxml::LN_CT_TblPrBase_tblStyle: //table style name
                /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
            {    
                m_sTableStyleName = pValue->getString();
                TablePropertyMapPtr pPropMap( new TablePropertyMap );
                pPropMap->Insert( META_PROP_TABLE_STYLE_NAME, false, uno::makeAny( m_sTableStyleName ));
                insertTableProps(pPropMap);
            }
            break;
            case NS_ooxml::LN_CT_TblGridBase_gridCol:
                /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
            {   
                getCurrentGrid()->push_back( ConversionHelper::convertTwipToMM100( nIntValue ) );
            }    
            break;
            case NS_ooxml::LN_CT_TcPrBase_vMerge : //vertical merge
                /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
            {    
                // values can be: LN_Value_ST_Merge_restart, LN_Value_ST_Merge_continue, in reality the second one is a 0
                TablePropertyMapPtr pMergeProps( new TablePropertyMap );
                pMergeProps->Insert( PROP_VERTICAL_MERGE, false, uno::makeAny( bool( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_ST_Merge_restart )) );
                cellProps( pMergeProps);
            }
            break;
            case NS_ooxml::LN_CT_TcPrBase_gridSpan: //number of grid positions spanned by this cell
                /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
            {    
#if DEBUG_DOMAINMAPPER
                dmapper_logger->startElement("tablemanager.GridSpan");
                dmapper_logger->attribute("gridSpan", nIntValue);
                dmapper_logger->endElement("tablemanager.GridSpan");
#endif
                m_nGridSpan = nIntValue;
            }
            break;
            case NS_ooxml::LN_CT_TblPrBase_tblLook: 
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
                break; //todo: table look specifier
            case NS_ooxml::LN_CT_TcPrBase_tcW: 
                /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
#ifdef NO_LIBO_4_0_TABLE_FIXES
                break; //fixed column width is not supported
#else	// NO_LIBO_4_0_TABLE_FIXES
                {
                    // Contains unit and value, but unit is not interesting for
                    // us, later we'll just distribute these values in a
                    // 0..10000 scale.
                    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                    if( pProperties.get())
                    {
                        MeasureHandlerPtr pMeasureHandler(new MeasureHandler());
                        pProperties->resolve(*pMeasureHandler);
                        getCurrentCellWidths()->push_back(pMeasureHandler->getMeasureValue());
                        if (getTableDepthDifference() > 0)
                            m_bPushCurrentWidth = true;
                    }
                }
                break;
#endif	// NO_LIBO_4_0_TABLE_FIXES
            case NS_ooxml::LN_CT_TrPrBase_cnfStyle:
                /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
                {
                    TablePropertyMapPtr pProps( new TablePropertyMap );
                    pProps->Insert( PROP_CNF_STYLE, true, uno::makeAny( pValue->getString( ) ) );
                    insertRowProps( pProps );
                }
                break;
            case NS_ooxml::LN_CT_PPrBase_cnfStyle:
                /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
                // TODO cnfStyle on a paragraph
                break;
            case NS_ooxml::LN_CT_TcPrBase_cnfStyle:
                /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
                {
                    TablePropertyMapPtr pProps( new TablePropertyMap );
                    pProps->Insert( PROP_CNF_STYLE, true, uno::makeAny( pValue->getString( ) ) );
                    cellProps( pProps );
                }
                break;
            default:
                bRet = false;
                
#ifdef DEBUG_DOMAINMAPPER
                dmapper_logger->element("unhandled");
#endif
        }
    }
    return bRet;
}

boost::shared_ptr< vector<sal_Int32> > DomainMapperTableManager::getCurrentGrid( )
{
    return m_aTableGrid.back( );
}

boost::shared_ptr< vector< sal_Int32 > > DomainMapperTableManager::getCurrentSpans( )
{
    return m_aGridSpans.back( );
}

#ifndef NO_LIBO_4_0_TABLE_FIXES

boost::shared_ptr< vector< sal_Int32 > > DomainMapperTableManager::getCurrentCellWidths( )
{
    return m_aCellWidths.back( );
}

#endif	// !NO_LIBO_4_0_TABLE_FIXES

void DomainMapperTableManager::startLevel( )
{
    DomainMapperTableManager_Base_t::startLevel( );

#ifndef NO_LIBO_4_0_TABLE_FIXES
    // If requested, pop the value that was pushed too early.
    boost::optional<sal_Int32> oCurrentWidth;
    if (m_bPushCurrentWidth && !m_aCellWidths.empty() && !m_aCellWidths.back()->empty())
    {
        oCurrentWidth.reset(m_aCellWidths.back()->back());
        m_aCellWidths.back()->pop_back();
    }
#endif	// !NO_LIBO_4_0_TABLE_FIXES

    IntVectorPtr pNewGrid( new vector<sal_Int32> );
    IntVectorPtr pNewSpans( new vector<sal_Int32> );
#ifndef NO_LIBO_4_0_TABLE_FIXES
    IntVectorPtr pNewCellWidths( new vector<sal_Int32> );
#endif	// !NO_LIBO_4_0_TABLE_FIXES
    m_aTableGrid.push_back( pNewGrid );
    m_aGridSpans.push_back( pNewSpans );
#ifndef NO_LIBO_4_0_TABLE_FIXES
    m_aCellWidths.push_back( pNewCellWidths );
    m_nCell.push_back( 0 );
#endif	// !NO_LIBO_4_0_TABLE_FIXES
    m_nTableWidth = 0;

#ifndef NO_LIBO_4_0_TABLE_FIXES
    // And push it back to the right level.
    if (oCurrentWidth)
        m_aCellWidths.back()->push_back(*oCurrentWidth);
#endif	// !NO_LIBO_4_0_TABLE_FIXES
}

void DomainMapperTableManager::endLevel( )
{
    m_aTableGrid.pop_back( );
    m_aGridSpans.pop_back( );
#ifndef NO_LIBO_4_0_TABLE_FIXES
    m_aCellWidths.pop_back( );
    m_nCell.pop_back( );
#endif	// !NO_LIBO_4_0_TABLE_FIXES
    m_nTableWidth = 0;
    
    DomainMapperTableManager_Base_t::endLevel( );
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("dmappertablemanager.endLevel");
    PropertyMapPtr pProps = getTableProps();
    if (pProps.get() != NULL)
        dmapper_logger->addTag(getTableProps()->toTag());
        
    dmapper_logger->endElement("dmappertablemanager.endLevel");
#endif    
}

/*-- 02.05.2007 14:36:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapperTableManager::endOfCellAction()
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->element("endOFCellAction");
#endif
    
    getCurrentSpans()->push_back(m_nGridSpan);
    m_nGridSpan = 1;
#ifdef NO_LIBO_4_0_TABLE_FIXES
    ++m_nCell;
#else	// NO_LIBO_4_0_TABLE_FIXES
    ++m_nCell.back( );
#endif	// NO_LIBO_4_0_TABLE_FIXES
}
/*-- 02.05.2007 14:36:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapperTableManager::endOfRowAction()
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("endOfRowAction");
#endif
    
    IntVectorPtr pTableGrid = getCurrentGrid( );
#ifndef NO_LIBO_4_0_TABLE_FIXES
    IntVectorPtr pCellWidths = getCurrentCellWidths( );
#endif	// !NO_LIBO_4_0_TABLE_FIXES
    if(!m_nTableWidth && pTableGrid->size())
    {
        ::std::vector<sal_Int32>::const_iterator aCellIter = pTableGrid->begin();

#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->startElement("tableWidth");
#endif

        while( aCellIter != pTableGrid->end() )
        {
#ifdef DEBUG_DOMAINMAPPER
            dmapper_logger->startElement("col");
            dmapper_logger->attribute("width", *aCellIter);
            dmapper_logger->endElement("col");
#endif
            
             m_nTableWidth += *aCellIter++;
        }
        
        if( m_nTableWidth > 0)
        {
            TablePropertyMapPtr pPropMap( new TablePropertyMap );
//            pPropMap->Insert( PROP_WIDTH, false, uno::makeAny( m_nTableWidth ));
            pPropMap->setValue( TablePropertyMap::TABLE_WIDTH, m_nTableWidth );
            insertTableProps(pPropMap);
        }

#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->endElement("tableWidth");
#endif
    }

    IntVectorPtr pCurrentSpans = getCurrentSpans( );
#ifdef NO_LIBO_4_0_TABLE_FIXES
    if( pCurrentSpans->size() < m_nCell)
    {
        //fill missing elements with '1'
        pCurrentSpans->insert( pCurrentSpans->end( ), m_nCell - pCurrentSpans->size(), 1 );
    }    
#else	// NO_LIBO_4_0_TABLE_FIXES
    if( pCurrentSpans->size() < m_nCell.back( ) )
    {
        //fill missing elements with '1'
        pCurrentSpans->insert( pCurrentSpans->end( ), m_nCell.back( ) - pCurrentSpans->size(), 1 );
    }
#endif	// NO_LIBO_4_0_TABLE_FIXES
    
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("gridSpans");
    {
        ::std::vector<sal_Int32>::const_iterator aGridSpanIter = pCurrentSpans->begin();
        ::std::vector<sal_Int32>::const_iterator aGridSpanIterEnd = pCurrentSpans->end();
        
        while (aGridSpanIter != aGridSpanIterEnd)
        {
            dmapper_logger->startElement("gridSpan");
            dmapper_logger->attribute("span", *aGridSpanIter);
            dmapper_logger->endElement("gridSpan");
        
            aGridSpanIter++;
        }
    }
    dmapper_logger->endElement("gridSpans");
#endif
    
    //calculate number of used grids - it has to match the size of m_aTableGrid
    size_t nGrids = 0;
    ::std::vector<sal_Int32>::const_iterator aGridSpanIter = pCurrentSpans->begin();
    for( ; aGridSpanIter != pCurrentSpans->end(); ++aGridSpanIter)
        nGrids += *aGridSpanIter;

#ifdef NO_LIBO_4_0_TABLE_FIXES
    if( pTableGrid->size() == nGrids )
    {
        //determine table width 
        double nFullWidth = m_nTableWidth;
        //the positions have to be distibuted in a range of 10000 
        const double nFullWidthRelative = 10000.;
        uno::Sequence< text::TableColumnSeparator > aSeparators( m_nCell - 1 );
#else	// NO_LIBO_4_0_TABLE_FIXES
    //determine table width
    double nFullWidth = m_nTableWidth;
    //the positions have to be distibuted in a range of 10000
    const double nFullWidthRelative = 10000.;
    if( pTableGrid->size() == ( m_nGridBefore + nGrids + m_nGridAfter ) && m_nCell.back( ) > 0 )
    {
        uno::Sequence< text::TableColumnSeparator > aSeparators( m_nCell.back( ) - 1 );
#endif	// NO_LIBO_4_0_TABLE_FIXES
        text::TableColumnSeparator* pSeparators = aSeparators.getArray();
        sal_Int16 nLastRelPos = 0;
#ifdef NO_LIBO_4_0_TABLE_FIXES
        sal_uInt32 nBorderGridIndex = 0;
#else	// NO_LIBO_4_0_TABLE_FIXES
        sal_uInt32 nBorderGridIndex = m_nGridBefore;
#endif	// NO_LIBO_4_0_TABLE_FIXES

        ::std::vector< sal_Int32 >::const_iterator aSpansIter = pCurrentSpans->begin( );
#ifdef NO_LIBO_4_0_TABLE_FIXES
        for( sal_uInt32 nBorder = 0; nBorder < m_nCell - 1; ++nBorder )
#else	// NO_LIBO_4_0_TABLE_FIXES
        for( sal_uInt32 nBorder = 0; nBorder < m_nCell.back( ) - 1; ++nBorder )
#endif	// NO_LIBO_4_0_TABLE_FIXES
        {
            sal_Int32 nGridCount = *aSpansIter;
            double fGridWidth = 0.;
            do
            {
                fGridWidth += (*pTableGrid.get())[nBorderGridIndex++];
            }while( --nGridCount );
            
            sal_Int16 nRelPos = 
                sal::static_int_cast< sal_Int16 >(fGridWidth * nFullWidthRelative / nFullWidth );

            pSeparators[nBorder].Position =  nRelPos + nLastRelPos;
            pSeparators[nBorder].IsVisible = sal_True;
            nLastRelPos = nLastRelPos + nRelPos;
            aSpansIter++;
        }
        TablePropertyMapPtr pPropMap( new TablePropertyMap );
        pPropMap->Insert( PROP_TABLE_COLUMN_SEPARATORS, false, uno::makeAny( aSeparators ) );
        
#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->startElement("rowProperties");
        dmapper_logger->addTag(pPropMap->toTag());
        dmapper_logger->endElement("rowProperties");
#endif
        insertRowProps(pPropMap);
    }
#ifndef NO_LIBO_4_0_TABLE_FIXES
    else if (pCellWidths->size() > 0)
    {
        // If we're here, then the number of cells does not equal to the amount
        // defined by the grid, even after taking care of
        // gridSpan/gridBefore/gridAfter. Handle this by ignoring the grid and
        // providing the separators based on the provided cell widths.
        uno::Sequence< text::TableColumnSeparator > aSeparators(pCellWidths->size() - 1);
        text::TableColumnSeparator* pSeparators = aSeparators.getArray();
        sal_Int16 nSum = 0;
        sal_uInt32 nPos = 0;

        for (sal_uInt32 i = 0; i < pCellWidths->size() - 1; ++i)
        {
            nSum += (*pCellWidths.get())[i];
            pSeparators[nPos].Position = nSum * nFullWidthRelative / nFullWidth;
            pSeparators[nPos].IsVisible = sal_True;
            nPos++;
        }

        TablePropertyMapPtr pPropMap( new TablePropertyMap );
        pPropMap->Insert( PROP_TABLE_COLUMN_SEPARATORS, false, uno::makeAny( aSeparators ) );
#ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->startElement("rowProperties");
        pPropMap->dumpXml( dmapper_logger );
        dmapper_logger->endElement();
#endif
        insertRowProps(pPropMap);
    }
#endif	// !NO_LIBO_4_0_TABLE_FIXES

    ++m_nRow;
#ifdef NO_LIBO_4_0_TABLE_FIXES
    m_nCell = 0;
#else	// NO_LIBO_4_0_TABLE_FIXES
    m_nCell.back( ) = 0;
#endif	// NO_LIBO_4_0_TABLE_FIXES
    m_nCellBorderIndex = 0;
    pCurrentSpans->clear();
#ifndef NO_LIBO_4_0_TABLE_FIXES
    pCellWidths->clear();

    m_nGridBefore = m_nGridAfter = 0;
#endif	// !NO_LIBO_4_0_TABLE_FIXES
    
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("endOfRowAction");
#endif
}
/*-- 18.06.2007 10:34:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapperTableManager::clearData()
{
#ifdef NO_LIBO_4_0_TABLE_FIXES
    m_nRow = m_nCell = m_nCellBorderIndex = m_nHeaderRepeat = m_nTableWidth = 0;
#else	// NO_LIBO_4_0_TABLE_FIXES
    m_nRow = m_nCellBorderIndex = m_nHeaderRepeat = m_nTableWidth = 0;
#endif	// NO_LIBO_4_0_TABLE_FIXES
    m_sTableStyleName = ::rtl::OUString();
    m_pTableStyleTextProperies.reset();
}
/*-- 27.06.2007 14:19:50---------------------------------------------------

  -----------------------------------------------------------------------*/
void lcl_CopyTextProperties(PropertyMapPtr pToFill,
            const StyleSheetEntry* pStyleSheetEntry, StyleSheetTablePtr pStyleSheetTable)
{
    if( !pStyleSheetEntry )
        return;
    //fill base style properties first, recursively
    if( pStyleSheetEntry->sBaseStyleIdentifier.getLength())
    {
        const StyleSheetEntryPtr pParentStyleSheet = 
            pStyleSheetTable->FindStyleSheetByISTD(pStyleSheetEntry->sBaseStyleIdentifier);
        OSL_ENSURE( pParentStyleSheet, "table style not found" );
        lcl_CopyTextProperties( pToFill, pParentStyleSheet.get( ), pStyleSheetTable);
    }

    PropertyMap::const_iterator aPropIter = pStyleSheetEntry->pProperties->begin();
    while(aPropIter != pStyleSheetEntry->pProperties->end())
    {
        //copy all text properties form the table style to the current run attributes
        if( aPropIter->first.bIsTextProperty )
            pToFill->insert(*aPropIter);
        ++aPropIter;
    }    
}
void DomainMapperTableManager::CopyTextProperties(PropertyMapPtr pContext, StyleSheetTablePtr pStyleSheetTable)
{
    if( !m_pTableStyleTextProperies.get())
    {
        m_pTableStyleTextProperies.reset( new PropertyMap );
        const StyleSheetEntryPtr pStyleSheetEntry = pStyleSheetTable->FindStyleSheetByISTD(
                                                        m_sTableStyleName);
        OSL_ENSURE( pStyleSheetEntry, "table style not found" );
        lcl_CopyTextProperties(m_pTableStyleTextProperies, pStyleSheetEntry.get( ), pStyleSheetTable);
    }
    pContext->insert( m_pTableStyleTextProperies );
}


}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
