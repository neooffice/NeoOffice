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

#include "oox/drawingml/chart/chartconverter.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include "oox/drawingml/chart/chartspaceconverter.hxx"
#include "oox/drawingml/chart/chartspacemodel.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/core/xmlfilterbase.hxx"

#if SUPD == 310
#include <com/sun/star/chart2/data/XDataProvider2.hpp>
#endif	// SUPD == 310

using ::oox::drawingml::chart::DataSequenceModel;
using ::com::sun::star::uno::Any;
namespace oox {
namespace drawingml {
namespace chart {



using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star::chart2::data;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;

using ::oox::core::XmlFilterBase;



static const sal_Unicode API_TOKEN_ARRAY_OPEN      = '{';
static const sal_Unicode API_TOKEN_ARRAY_CLOSE     = '}';
static const sal_Unicode API_TOKEN_ARRAY_ROWSEP    = '|';
static const sal_Unicode API_TOKEN_ARRAY_COLSEP    = ';';

// Code similar to oox/source/xls/formulabase.cxx
static OUString lclGenerateApiString( const OUString& rString )
{
    OUString aRetString = rString;
    sal_Int32 nQuotePos = aRetString.getLength();
    while( (nQuotePos = aRetString.lastIndexOf( '"', nQuotePos )) >= 0 )
        aRetString = aRetString.replaceAt( nQuotePos, 1, "\"\"" );
    return OUStringBuffer().append( '"' ).append( aRetString ).append( '"' ).makeStringAndClear();
}

static OUString lclGenerateApiArray( const Matrix< Any >& rMatrix )
{
    OSL_ENSURE( !rMatrix.empty(), "ChartConverter::lclGenerateApiArray - missing matrix values" );
    OUStringBuffer aBuffer;
    aBuffer.append( API_TOKEN_ARRAY_OPEN );
    for( size_t nRow = 0, nHeight = rMatrix.height(); nRow < nHeight; ++nRow )
    {
        if( nRow > 0 )
            aBuffer.append( API_TOKEN_ARRAY_ROWSEP );
        for( Matrix< Any >::const_iterator aBeg = rMatrix.row_begin( nRow ), aIt = aBeg, aEnd = rMatrix.row_end( nRow ); aIt != aEnd; ++aIt )
        {
            double fValue = 0.0;
            OUString aString;
            if( aIt != aBeg )
                aBuffer.append( API_TOKEN_ARRAY_COLSEP );
            if( *aIt >>= fValue )
                aBuffer.append( fValue );
            else if( *aIt >>= aString )
                aBuffer.append( lclGenerateApiString( aString ) );
            else
                aBuffer.appendAscii( "\"\"" );
        }
    }
    aBuffer.append( API_TOKEN_ARRAY_CLOSE );
    return aBuffer.makeStringAndClear();
}



ChartConverter::ChartConverter()
{
}

ChartConverter::~ChartConverter()
{
}

void ChartConverter::convertFromModel( XmlFilterBase& rFilter,
#if SUPD == 310
        ChartSpaceModel& rChartModel, const css::uno::Reference< XChartDocument >& rxChartDoc,
        const css::uno::Reference< XShapes >& rxExternalPage, const awt::Point& rChartPos, const awt::Size& rChartSize )
#else	// SUPD == 310
        ChartSpaceModel& rChartModel, const Reference< XChartDocument >& rxChartDoc,
        const Reference< XShapes >& rxExternalPage, const awt::Point& rChartPos, const awt::Size& rChartSize )
#endif	// SUPD == 310
{
    OSL_ENSURE( rxChartDoc.is(), "ChartConverter::convertFromModel - missing chart document" );
    if( rxChartDoc.is() )
    {
        ConverterRoot aConvBase( rFilter, *this, rChartModel, rxChartDoc, rChartSize );
        ChartSpaceConverter aSpaceConv( aConvBase, rChartModel );
        aSpaceConv.convertFromModel( rxExternalPage, rChartPos );
    }
}

#if SUPD == 310
void ChartConverter::createDataProvider( const css::uno::Reference< XChartDocument >& rxChartDoc )
#else	// SUPD == 310
void ChartConverter::createDataProvider( const Reference< XChartDocument >& rxChartDoc )
#endif	// SUPD == 310
{
    try
    {
        if( !rxChartDoc->hasInternalDataProvider() )
            rxChartDoc->createInternalDataProvider( sal_False );
    }
    catch( Exception& )
    {
    }
}

#if SUPD == 310
css::uno::Reference< XDataSequence > ChartConverter::createDataSequence(
    const css::uno::Reference< XDataProvider >& rxDataProvider, const DataSequenceModel& rDataSeq,
#else	// SUPD == 310
Reference< XDataSequence > ChartConverter::createDataSequence(
    const Reference< XDataProvider >& rxDataProvider, const DataSequenceModel& rDataSeq,
#endif	// SUPD == 310
    const OUString& rRole )
{
#if SUPD == 310
    css::uno::Reference< XDataSequence > xDataSeq;
#else	// SUPD == 310
    Reference< XDataSequence > xDataSeq;
#endif	// SUPD == 310
    if( rxDataProvider.is() )
    {
        OUString aRangeRep;
        if( !rDataSeq.maData.empty() )
        {
            // create a single-row array from constant source data
            Matrix< Any > aMatrix( rDataSeq.maData.size(), 1 );
            Matrix< Any >::iterator aMIt = aMatrix.begin();
            // TODO: how to handle missing values in the map?
            for( DataSequenceModel::AnyMap::const_iterator aDIt = rDataSeq.maData.begin(), aDEnd = rDataSeq.maData.end(); aDIt != aDEnd; ++aDIt, ++aMIt )
                *aMIt = aDIt->second;
            aRangeRep = lclGenerateApiArray( aMatrix );
        }

        if( !aRangeRep.isEmpty() ) try
        {
            // create the data sequence
#if SUPD == 310
            const css::uno::Reference< XDataProvider2 > rxDataProvider2( rxDataProvider, UNO_QUERY_THROW );
            xDataSeq = rxDataProvider2->createDataSequenceByValueArray(rRole, aRangeRep);
#else	// SUPD == 310
            xDataSeq = rxDataProvider->createDataSequenceByValueArray(rRole, aRangeRep);
#endif	// SUPD == 310
            return xDataSeq;
        }
        catch( Exception& )
        {
            OSL_FAIL( "ChartConverter::createDataSequence - cannot create data sequence" );
        }
    }

    return 0;
}



} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
