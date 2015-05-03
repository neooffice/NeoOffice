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

#include "drawingml/chart/converterbase.hxx"

#include <com/sun/star/chart/XAxisXSupplier.hpp>
#include <com/sun/star/chart/XAxisYSupplier.hpp>
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XSecondAxisTitleSupplier.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <osl/diagnose.h>
#include "basegfx/numeric/ftools.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/theme.hxx"
#include <comphelper/processfactory.hxx>

#if SUPD == 310
// OpenOffice 3.1.1 chart2 module cannot handle the RelativeSize property
#define NO_OOO_4_1_1_CHARTS
#endif	// SUPD == 310

namespace oox {
namespace drawingml {
namespace chart {

namespace cssc = ::com::sun::star::chart;

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

using ::oox::core::XmlFilterBase;

namespace {

struct TitleKey : public ::std::pair< ObjectType, ::std::pair< sal_Int32, sal_Int32 > >
{
    inline explicit     TitleKey( ObjectType eObjType, sal_Int32 nMainIdx = -1, sal_Int32 nSubIdx = -1 )
                            { first = eObjType; second.first = nMainIdx; second.second = nSubIdx; }
};

/** A helper structure to store all data related to title objects. Needed for
    the conversion of manual title positions that needs the old Chart1 API.
 */
struct TitleLayoutInfo
{
#if SUPD == 310
    typedef css::uno::Reference< XShape > (*GetShapeFunc)( const css::uno::Reference< cssc::XChartDocument >& );

    css::uno::Reference< XTitle > mxTitle;        /// The API title object.
#else	// SUPD == 310
    typedef Reference< XShape > (*GetShapeFunc)( const Reference< cssc::XChartDocument >& );

    Reference< XTitle > mxTitle;        /// The API title object.
#endif	// SUPD == 310
    ModelRef< LayoutModel > mxLayout;   /// The layout model, if existing.
    GetShapeFunc        mpGetShape;     /// Helper function to receive the title shape.

    inline explicit     TitleLayoutInfo() : mpGetShape( 0 ) {}

    void                convertTitlePos(
                            ConverterRoot& rRoot,
#if SUPD == 310
                            const css::uno::Reference< cssc::XChartDocument >& rxChart1Doc );
#else	// SUPD == 310
                            const Reference< cssc::XChartDocument >& rxChart1Doc );
#endif	// SUPD == 310
};

#if SUPD == 310
void TitleLayoutInfo::convertTitlePos( ConverterRoot& rRoot, const css::uno::Reference< cssc::XChartDocument >& rxChart1Doc )
#else	// SUPD == 310
void TitleLayoutInfo::convertTitlePos( ConverterRoot& rRoot, const Reference< cssc::XChartDocument >& rxChart1Doc )
#endif	// SUPD == 310
{
    if( mxTitle.is() && mpGetShape ) try
    {
        // try to get the title shape
#if SUPD == 310
        css::uno::Reference< XShape > xTitleShape = mpGetShape( rxChart1Doc );
#else	// SUPD == 310
        Reference< XShape > xTitleShape = mpGetShape( rxChart1Doc );
#endif	// SUPD == 310
        // get title rotation angle, needed for correction of position of top-left edge
        double fAngle = 0.0;
        PropertySet aTitleProp( mxTitle );
        aTitleProp.getProperty( fAngle, PROP_TextRotation );
        // convert the position
        LayoutModel& rLayout = mxLayout.getOrCreate();
        LayoutConverter aLayoutConv( rRoot, rLayout );
        aLayoutConv.convertFromModel( xTitleShape, fAngle );
    }
    catch( Exception& )
    {
    }
}

/*  The following local functions implement getting the XShape interface of all
    supported title objects (chart and axes). This needs some effort due to the
    design of the old Chart1 API used to access these objects. */

/** A code fragment that returns a shape object from the passed shape supplier
    using the specified interface function. Checks a boolean property first. */
#if SUPD == 310
#define OOX_FRAGMENT_GETTITLESHAPE( shape_supplier, supplier_func, property_name ) \
    PropertySet aPropSet( shape_supplier ); \
    if( shape_supplier.is() && aPropSet.getBoolProperty( PROP_##property_name ) ) \
        return shape_supplier->supplier_func(); \
    return css::uno::Reference< XShape >(); \

/** Implements a function returning the drawing shape of an axis title, if
    existing, using the specified API interface and its function. */
#define OOX_DEFINEFUNC_GETAXISTITLESHAPE( func_name, interface_type, supplier_func, property_name ) \
css::uno::Reference< XShape > func_name( const css::uno::Reference< cssc::XChartDocument >& rxChart1Doc ) \
{ \
    css::uno::Reference< cssc::interface_type > xAxisSupp( rxChart1Doc->getDiagram(), UNO_QUERY ); \
    OOX_FRAGMENT_GETTITLESHAPE( xAxisSupp, supplier_func, property_name ) \
}
#else	// SUPD == 310
#define OOX_FRAGMENT_GETTITLESHAPE( shape_supplier, supplier_func, property_name ) \
    PropertySet aPropSet( shape_supplier ); \
    if( shape_supplier.is() && aPropSet.getBoolProperty( PROP_##property_name ) ) \
        return shape_supplier->supplier_func(); \
    return Reference< XShape >(); \

/** Implements a function returning the drawing shape of an axis title, if
    existing, using the specified API interface and its function. */
#define OOX_DEFINEFUNC_GETAXISTITLESHAPE( func_name, interface_type, supplier_func, property_name ) \
Reference< XShape > func_name( const Reference< cssc::XChartDocument >& rxChart1Doc ) \
{ \
    Reference< cssc::interface_type > xAxisSupp( rxChart1Doc->getDiagram(), UNO_QUERY ); \
    OOX_FRAGMENT_GETTITLESHAPE( xAxisSupp, supplier_func, property_name ) \
}
#endif	// SUPD == 310

/** Returns the drawing shape of the main title, if existing. */
#if SUPD == 310
css::uno::Reference< XShape > lclGetMainTitleShape( const css::uno::Reference< cssc::XChartDocument >& rxChart1Doc )
#else	// SUPD == 310
Reference< XShape > lclGetMainTitleShape( const Reference< cssc::XChartDocument >& rxChart1Doc )
#endif	// SUPD == 310
{
    OOX_FRAGMENT_GETTITLESHAPE( rxChart1Doc, getTitle, HasMainTitle )
}

OOX_DEFINEFUNC_GETAXISTITLESHAPE( lclGetXAxisTitleShape, XAxisXSupplier, getXAxisTitle, HasXAxisTitle )
OOX_DEFINEFUNC_GETAXISTITLESHAPE( lclGetYAxisTitleShape, XAxisYSupplier, getYAxisTitle, HasYAxisTitle )
OOX_DEFINEFUNC_GETAXISTITLESHAPE( lclGetZAxisTitleShape, XAxisZSupplier, getZAxisTitle, HasZAxisTitle )
OOX_DEFINEFUNC_GETAXISTITLESHAPE( lclGetSecXAxisTitleShape, XSecondAxisTitleSupplier, getSecondXAxisTitle, HasSecondaryXAxisTitle )
OOX_DEFINEFUNC_GETAXISTITLESHAPE( lclGetSecYAxisTitleShape, XSecondAxisTitleSupplier, getSecondYAxisTitle, HasSecondaryYAxisTitle )

#undef OOX_DEFINEFUNC_GETAXISTITLESHAPE
#undef OOX_IMPLEMENT_GETTITLESHAPE

} // namespace

struct ConverterData
{
    typedef ::std::map< TitleKey, TitleLayoutInfo > TitleMap;

    ObjectFormatter     maFormatter;
    TitleMap            maTitles;
    XmlFilterBase&      mrFilter;
    ChartConverter&     mrConverter;
#if SUPD == 310
    css::uno::Reference< XChartDocument > mxDoc;
#else	// SUPD == 310
    Reference< XChartDocument > mxDoc;
#endif	// SUPD == 310
    awt::Size                maSize;

    explicit            ConverterData(
                            XmlFilterBase& rFilter,
                            ChartConverter& rChartConverter,
                            const ChartSpaceModel& rChartModel,
#if SUPD == 310
                            const css::uno::Reference< XChartDocument >& rxChartDoc,
#else	// SUPD == 310
                            const Reference< XChartDocument >& rxChartDoc,
#endif	// SUPD == 310
                            const awt::Size& rChartSize );
                        ~ConverterData();
};

ConverterData::ConverterData(
        XmlFilterBase& rFilter,
        ChartConverter& rChartConverter,
        const ChartSpaceModel& rChartModel,
#if SUPD == 310
        const css::uno::Reference< XChartDocument >& rxChartDoc,
#else	// SUPD == 310
        const Reference< XChartDocument >& rxChartDoc,
#endif	// SUPD == 310
        const awt::Size& rChartSize ) :
    maFormatter( rFilter, rxChartDoc, rChartModel ),
    mrFilter( rFilter ),
    mrConverter( rChartConverter ),
    mxDoc( rxChartDoc ),
    maSize( rChartSize )
{
    OSL_ENSURE( mxDoc.is(), "ConverterData::ConverterData - missing chart document" );
    // lock the model to suppress internal updates during conversion
    try
    {
        mxDoc->lockControllers();
    }
    catch( Exception& )
    {
    }

    // prepare conversion of title positions
    maTitles[ TitleKey( OBJECTTYPE_CHARTTITLE ) ].mpGetShape = lclGetMainTitleShape;
    maTitles[ TitleKey( OBJECTTYPE_AXISTITLE, API_PRIM_AXESSET, API_X_AXIS ) ].mpGetShape = lclGetXAxisTitleShape;
    maTitles[ TitleKey( OBJECTTYPE_AXISTITLE, API_PRIM_AXESSET, API_Y_AXIS ) ].mpGetShape = lclGetYAxisTitleShape;
    maTitles[ TitleKey( OBJECTTYPE_AXISTITLE, API_PRIM_AXESSET, API_Z_AXIS ) ].mpGetShape = lclGetZAxisTitleShape;
    maTitles[ TitleKey( OBJECTTYPE_AXISTITLE, API_SECN_AXESSET, API_X_AXIS ) ].mpGetShape = lclGetSecXAxisTitleShape;
    maTitles[ TitleKey( OBJECTTYPE_AXISTITLE, API_SECN_AXESSET, API_Y_AXIS ) ].mpGetShape = lclGetSecYAxisTitleShape;
}

ConverterData::~ConverterData()
{
    // unlock the model
    try
    {
        mxDoc->unlockControllers();
    }
    catch( Exception& )
    {
    }
}

ConverterRoot::ConverterRoot(
        XmlFilterBase& rFilter,
        ChartConverter& rChartConverter,
        const ChartSpaceModel& rChartModel,
#if SUPD == 310
        const css::uno::Reference< XChartDocument >& rxChartDoc,
#else	// SUPD == 310
        const Reference< XChartDocument >& rxChartDoc,
#endif	// SUPD == 310
        const awt::Size& rChartSize ) :
    mxData( new ConverterData( rFilter, rChartConverter, rChartModel, rxChartDoc, rChartSize ) )
{
}

ConverterRoot::~ConverterRoot()
{
}

#if SUPD == 310
css::uno::Reference< XInterface > ConverterRoot::createInstance( const OUString& rServiceName ) const
#else	// SUPD == 310
Reference< XInterface > ConverterRoot::createInstance( const OUString& rServiceName ) const
#endif	// SUPD == 310
{
#if SUPD == 310
    css::uno::Reference< XInterface > xInt;
#else	// SUPD == 310
    Reference< XInterface > xInt;
#endif	// SUPD == 310
    try
    {
#if SUPD == 310
        css::uno::Reference<XMultiServiceFactory> xMSF = css::uno::Reference<XMultiServiceFactory>(getComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW);
#else	// SUPD == 310
        Reference<XMultiServiceFactory> xMSF = Reference<XMultiServiceFactory>(getComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW);
#endif	// SUPD == 310

        xInt = xMSF->createInstance( rServiceName );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xInt.is(), "ConverterRoot::createInstance - cannot create instance" );
    return xInt;
}

#if SUPD == 310
css::uno::Reference< XComponentContext > ConverterRoot::getComponentContext() const
#else	// SUPD == 310
Reference< XComponentContext > ConverterRoot::getComponentContext() const
#endif	// SUPD == 310
{
    return mxData->mrFilter.getComponentContext();
}

XmlFilterBase& ConverterRoot::getFilter() const
{
    return mxData->mrFilter;
}

ChartConverter* ConverterRoot::getChartConverter() const
{
    return &mxData->mrConverter;
}

#if SUPD == 310
css::uno::Reference< XChartDocument > ConverterRoot::getChartDocument() const
#else	// SUPD == 310
Reference< XChartDocument > ConverterRoot::getChartDocument() const
#endif	// SUPD == 310
{
    return mxData->mxDoc;
}

const awt::Size& ConverterRoot::getChartSize() const
{
    return mxData->maSize;
}

ObjectFormatter& ConverterRoot::getFormatter() const
{
    return mxData->maFormatter;
}

#if SUPD == 310
void ConverterRoot::registerTitleLayout( const css::uno::Reference< XTitle >& rxTitle,
#else	// SUPD == 310
void ConverterRoot::registerTitleLayout( const Reference< XTitle >& rxTitle,
#endif	// SUPD == 310
        const ModelRef< LayoutModel >& rxLayout, ObjectType eObjType, sal_Int32 nMainIdx, sal_Int32 nSubIdx )
{
    OSL_ENSURE( rxTitle.is(), "ConverterRoot::registerTitleLayout - missing title object" );
    TitleLayoutInfo& rTitleInfo = mxData->maTitles[ TitleKey( eObjType, nMainIdx, nSubIdx ) ];
    OSL_ENSURE( rTitleInfo.mpGetShape, "ConverterRoot::registerTitleLayout - invalid title key" );
    rTitleInfo.mxTitle = rxTitle;
    rTitleInfo.mxLayout = rxLayout;
}

void ConverterRoot::convertTitlePositions()
{
    try
    {
#if SUPD == 310
        css::uno::Reference< cssc::XChartDocument > xChart1Doc( mxData->mxDoc, UNO_QUERY_THROW );
#else	// SUPD == 310
        Reference< cssc::XChartDocument > xChart1Doc( mxData->mxDoc, UNO_QUERY_THROW );
#endif	// SUPD == 310
        for( ConverterData::TitleMap::iterator aIt = mxData->maTitles.begin(), aEnd = mxData->maTitles.end(); aIt != aEnd; ++aIt )
            aIt->second.convertTitlePos( *this, xChart1Doc );
    }
    catch( Exception& )
    {
    }
}

namespace {

/** Returns a position value in the chart area in 1/100 mm. */
sal_Int32 lclCalcPosition( sal_Int32 nChartSize, double fPos, sal_Int32 nPosMode )
{
    switch( nPosMode )
    {
        case XML_edge:      // absolute start position as factor of chart size
            return getLimitedValue< sal_Int32, double >( nChartSize * fPos + 0.5, 0, nChartSize );
        case XML_factor:    // position relative to object default position
            OSL_FAIL( "lclCalcPosition - relative positioning not supported" );
            return -1;
    };

    OSL_FAIL( "lclCalcPosition - unknown positioning mode" );
    return -1;
}

/** Returns a size value in the chart area in 1/100 mm. */
sal_Int32 lclCalcSize( sal_Int32 nPos, sal_Int32 nChartSize, double fSize, sal_Int32 nSizeMode )
{
    sal_Int32 nValue = getLimitedValue< sal_Int32, double >( nChartSize * fSize + 0.5, 0, nChartSize );
    switch( nSizeMode )
    {
        case XML_factor:    // passed value is width/height
            return nValue;
        case XML_edge:      // passed value is right/bottom position
            return nValue - nPos + 1;
    };

    OSL_FAIL( "lclCalcSize - unknown size mode" );
    return -1;
}

/** Returns a relative size value in the chart area. */
double lclCalcRelSize( double fPos, double fSize, sal_Int32 nSizeMode )
{
    switch( nSizeMode )
    {
        case XML_factor:    // passed value is width/height
        break;
        case XML_edge:      // passed value is right/bottom position
            fSize -= fPos;
        break;
        default:
            OSL_ENSURE( false, "lclCalcRelSize - unknown size mode" );
            fSize = 0.0;
    };
    return getLimitedValue< double, double >( fSize, 0.0, 1.0 - fPos );
}

} // namespace

LayoutConverter::LayoutConverter( const ConverterRoot& rParent, LayoutModel& rModel ) :
    ConverterBase< LayoutModel >( rParent, rModel )
{
}

LayoutConverter::~LayoutConverter()
{
}

bool LayoutConverter::calcAbsRectangle( awt::Rectangle& orRect ) const
{
    if( !mrModel.mbAutoLayout )
    {
        const awt::Size& rChartSize = getChartSize();
        orRect.X = lclCalcPosition( rChartSize.Width,  mrModel.mfX, mrModel.mnXMode );
        orRect.Y = lclCalcPosition( rChartSize.Height, mrModel.mfY, mrModel.mnYMode );
        if( (orRect.X >= 0) && (orRect.Y >= 0) )
        {
            orRect.Width  = lclCalcSize( orRect.X, rChartSize.Width,  mrModel.mfW, mrModel.mnWMode );
            orRect.Height = lclCalcSize( orRect.Y, rChartSize.Height, mrModel.mfH, mrModel.mnHMode );
            return (orRect.Width > 0) && (orRect.Height > 0);
        }
    }
    return false;
}

bool LayoutConverter::convertFromModel( PropertySet& rPropSet )
{
#ifndef NO_OOO_4_1_1_CHARTS
    if( !mrModel.mbAutoLayout &&
        (mrModel.mnXMode == XML_edge) && (mrModel.mfX >= 0.0) &&
        (mrModel.mnYMode == XML_edge) && (mrModel.mfY >= 0.0) )
    {
        RelativePosition aPos(
            getLimitedValue< double, double >( mrModel.mfX, 0.0, 1.0 ),
            getLimitedValue< double, double >( mrModel.mfY, 0.0, 1.0 ),
            Alignment_TOP_LEFT );
        rPropSet.setProperty( PROP_RelativePosition, aPos );

        RelativeSize aSize(
            lclCalcRelSize( aPos.Primary, mrModel.mfW, mrModel.mnWMode ),
            lclCalcRelSize( aPos.Secondary, mrModel.mfH, mrModel.mnHMode ) );
        if( (aSize.Primary > 0.0) && (aSize.Secondary > 0.0) )
        {
            rPropSet.setProperty( PROP_RelativeSize, aSize );
            return true;
        }
    }
#endif	// !NO_OOO_4_1_1_CHARTS
    return false;
}

#if SUPD == 310
bool LayoutConverter::convertFromModel( const css::uno::Reference< XShape >& rxShape, double fRotationAngle )
#else	// SUPD == 310
bool LayoutConverter::convertFromModel( const Reference< XShape >& rxShape, double fRotationAngle )
#endif	// SUPD == 310
{
    if( !mrModel.mbAutoLayout )
    {
        const awt::Size& rChartSize = getChartSize();
        awt::Point aShapePos(
            lclCalcPosition( rChartSize.Width,  mrModel.mfX, mrModel.mnXMode ),
            lclCalcPosition( rChartSize.Height, mrModel.mfY, mrModel.mnYMode ) );
        if( (aShapePos.X >= 0) && (aShapePos.Y >= 0) )
        {
            // the call to XShape.getSize() may recalc the chart view
            awt::Size aShapeSize = rxShape->getSize();
            // rotated shapes need special handling...
            double fSin = fabs( sin( fRotationAngle * F_PI180 ) );
            // add part of height to X direction, if title is rotated down
            if( fRotationAngle > 180.0 )
                aShapePos.X += static_cast< sal_Int32 >( fSin * aShapeSize.Height + 0.5 );
            // add part of width to Y direction, if title is rotated up
            else if( fRotationAngle > 0.0 )
                aShapePos.Y += static_cast< sal_Int32 >( fSin * aShapeSize.Width + 0.5 );
            // set the resulting position at the shape
            rxShape->setPosition( aShapePos );
            return true;
        }
    }
    return false;
}

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
