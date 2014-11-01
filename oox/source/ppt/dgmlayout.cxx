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

#include "oox/ppt/dgmlayout.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/drawingml/themefragmenthandler.hxx"
#include "oox/drawingml/diagram/diagram.hxx"
#include "oox/dump/pptxdumper.hxx"

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/container/XChild.hpp>

#include <services.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace oox::core;
using namespace ::oox::drawingml;

namespace oox { namespace ppt {

OUString SAL_CALL QuickDiagrammingLayout_getImplementationName()
{
    return OUString( "com.sun.star.comp.Impress.oox.QuickDiagrammingLayout" );
}

uno::Sequence< OUString > SAL_CALL QuickDiagrammingLayout_getSupportedServiceNames()
{
    const OUString aServiceName = "com.sun.star.comp.ooxpptx.dgm.layout";
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

#if SUPD == 310
uno::Reference< uno::XInterface > SAL_CALL QuickDiagrammingLayout_createInstance( const css::uno::Reference< XComponentContext >& rxContext ) throw( Exception )
#else	// SUPD == 310
uno::Reference< uno::XInterface > SAL_CALL QuickDiagrammingLayout_createInstance( const Reference< XComponentContext >& rxContext ) throw( Exception )
#endif	// SUPD == 310
{
    return (cppu::OWeakObject*)new QuickDiagrammingLayout( rxContext );
}

#if SUPD == 310
QuickDiagrammingLayout::QuickDiagrammingLayout( const css::uno::Reference< XComponentContext >& rxContext )
#else	// SUPD == 310
QuickDiagrammingLayout::QuickDiagrammingLayout( const Reference< XComponentContext >& rxContext )
#endif	// SUPD == 310
    : XmlFilterBase( rxContext ),
    mpThemePtr(new drawingml::Theme())
{}

bool QuickDiagrammingLayout::importDocument() throw()
{
#if SUPD == 310
    css::uno::Reference<drawing::XShape>  xParentShape(getParentShape(),
#else	// SUPD == 310
    Reference<drawing::XShape>  xParentShape(getParentShape(),
#endif	// SUPD == 310
                                             UNO_QUERY_THROW);
#if SUPD == 310
    css::uno::Reference<drawing::XShapes> xParentShapes(xParentShape,
#else	// SUPD == 310
    Reference<drawing::XShapes> xParentShapes(xParentShape,
#endif	// SUPD == 310
                                              UNO_QUERY_THROW);
#if SUPD == 310
    css::uno::Reference<beans::XPropertySet> xPropSet(xParentShape,
#else	// SUPD == 310
    Reference<beans::XPropertySet> xPropSet(xParentShape,
#endif	// SUPD == 310
                                            UNO_QUERY_THROW);

    // can we grab the theme from the master page?
#if SUPD == 310
    css::uno::Reference<container::XChild> xChild(xParentShape,
#else	// SUPD == 310
    Reference<container::XChild> xChild(xParentShape,
#endif	// SUPD == 310
                                        UNO_QUERY);
    if( xChild.is() )
    {
        // TODO: cater for diagram shapes inside groups
#if SUPD == 310
        css::uno::Reference<drawing::XMasterPageTarget> xMasterPageTarget(xChild->getParent(),
#else	// SUPD == 310
        Reference<drawing::XMasterPageTarget> xMasterPageTarget(xChild->getParent(),
#endif	// SUPD == 310
                                                                UNO_QUERY);
        if( xMasterPageTarget.is() )
        {
            uno::Reference<drawing::XDrawPage> xMasterPage(
                xMasterPageTarget->getMasterPage());

#if SUPD == 310
            css::uno::Reference<beans::XPropertySet> xPropSet2(xMasterPage,
#else	// SUPD == 310
            Reference<beans::XPropertySet> xPropSet2(xMasterPage,
#endif	// SUPD == 310
                                                     UNO_QUERY_THROW);
#if SUPD == 310
            css::uno::Reference<xml::dom::XDocument> xThemeFragment;
#else	// SUPD == 310
            Reference<xml::dom::XDocument> xThemeFragment;
#endif	// SUPD == 310
            xPropSet2->getPropertyValue("PPTTheme") >>= xThemeFragment;

            importFragment(
                new ThemeFragmentHandler(
                    *this, OUString(), *mpThemePtr ),
#if SUPD == 310
                css::uno::Reference<xml::sax::XFastSAXSerializable>(
#else	// SUPD == 310
                Reference<xml::sax::XFastSAXSerializable>(
#endif	// SUPD == 310
                    xThemeFragment,
                    UNO_QUERY_THROW));
        }
    }

#if SUPD == 310
    css::uno::Reference<xml::dom::XDocument> xDataModelDom;
    css::uno::Reference<xml::dom::XDocument> xLayoutDom;
    css::uno::Reference<xml::dom::XDocument> xQStyleDom;
    css::uno::Reference<xml::dom::XDocument> xColorStyleDom;
#else	// SUPD == 310
    Reference<xml::dom::XDocument> xDataModelDom;
    Reference<xml::dom::XDocument> xLayoutDom;
    Reference<xml::dom::XDocument> xQStyleDom;
    Reference<xml::dom::XDocument> xColorStyleDom;
#endif	// SUPD == 310

    xPropSet->getPropertyValue("DiagramData") >>= xDataModelDom;
    xPropSet->getPropertyValue("DiagramLayout") >>= xLayoutDom;
    xPropSet->getPropertyValue("DiagramQStyle") >>= xQStyleDom;
    xPropSet->getPropertyValue("DiagramColorStyle") >>= xColorStyleDom;

    oox::drawingml::ShapePtr pShape(
        new oox::drawingml::Shape( "com.sun.star.drawing.DiagramShape" ) );
    drawingml::loadDiagram(pShape,
                           *this,
                           xDataModelDom,
                           xLayoutDom,
                           xQStyleDom,
                           xColorStyleDom);

    // don't add pShape itself, but only its children
    pShape->setXShape(getParentShape());

    const awt::Size& rSize=xParentShape->getSize();
    const awt::Point& rPoint=xParentShape->getPosition();
    const long nScaleFactor=360;
    const awt::Rectangle aRect(nScaleFactor*rPoint.X,
                               nScaleFactor*rPoint.Y,
                               nScaleFactor*rSize.Width,
                               nScaleFactor*rSize.Height);
    basegfx::B2DHomMatrix aMatrix;
    pShape->addChildren( *this,
                         mpThemePtr.get(),
                         xParentShapes,
                         aMatrix,
                         &aRect );

    return true;
}

bool QuickDiagrammingLayout::exportDocument() throw()
{
    return false;
}

const ::oox::drawingml::Theme* QuickDiagrammingLayout::getCurrentTheme() const
{
    return mpThemePtr.get();
}

sal_Int32 QuickDiagrammingLayout::getSchemeClr( sal_Int32 nColorSchemeToken ) const
{
    sal_Int32 nColor = 0;
    if( mpThemePtr )
        mpThemePtr->getClrScheme().getColor( nColorSchemeToken,
                                             nColor );
    return nColor;
}

const oox::drawingml::table::TableStyleListPtr QuickDiagrammingLayout::getTableStyles()
{
    return oox::drawingml::table::TableStyleListPtr();
}

::oox::vml::Drawing* QuickDiagrammingLayout::getVmlDrawing()
{
    return 0;
}

::oox::drawingml::chart::ChartConverter* QuickDiagrammingLayout::getChartConverter()
{
    return 0;
}

OUString QuickDiagrammingLayout::implGetImplementationName() const
{
    return QuickDiagrammingLayout_getImplementationName();
}

::oox::ole::VbaProject* QuickDiagrammingLayout::implCreateVbaProject() const
{
    return 0;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
