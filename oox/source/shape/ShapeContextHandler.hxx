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
#ifndef INCLUDED_OOX_SOURCE_SHAPE_SHAPECONTEXTHANDLER_HXX
#define INCLUDED_OOX_SOURCE_SHAPE_SHAPECONTEXTHANDLER_HXX

#include <boost/shared_ptr.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>
#if SUPD == 310
#include <com/sun/star/xml/sax/XFastShapeContextHandler2.hpp>
#else	// SUPD == 310
#include <com/sun/star/xml/sax/XFastShapeContextHandler.hpp>
#endif	// SUPD == 310
#include "oox/drawingml/graphicshapecontext.hxx"
#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/core/fragmenthandler2.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "ShapeFilterBase.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace oox { namespace shape {

class ShapeFragmentHandler : public core::FragmentHandler2
{
public:
    typedef boost::shared_ptr<ShapeFragmentHandler> Pointer_t;

    explicit ShapeFragmentHandler(core::XmlFilterBase& rFilter,
                                  const OUString& rFragmentPath )
    : FragmentHandler2(rFilter, rFragmentPath)
    {
    }
};

class ShapeContextHandler:
#if SUPD == 310
    public ::cppu::WeakImplHelper2< css::xml::sax::XFastShapeContextHandler2,
#else	// SUPD == 310
    public ::cppu::WeakImplHelper2< css::xml::sax::XFastShapeContextHandler,
#endif	// SUPD == 310
                                    css::lang::XServiceInfo >
{
public:
    explicit ShapeContextHandler
    (css::uno::Reference< css::uno::XComponentContext > const & context);

    virtual ~ShapeContextHandler();

    // ::com::sun::star::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName()
#if SUPD == 310
        throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    virtual sal_Bool SAL_CALL supportsService
#if SUPD == 310
    (const OUString & ServiceName) throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
    (const OUString & ServiceName) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    virtual css::uno::Sequence< OUString > SAL_CALL
#if SUPD == 310
    getSupportedServiceNames() throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
    getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    // ::com::sun::star::xml::sax::XFastContextHandler:
    virtual void SAL_CALL startFastElement
    (::sal_Int32 Element,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
#if SUPD == 310
        throw (css::uno::RuntimeException, css::xml::sax::SAXException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    virtual void SAL_CALL startUnknownElement
    (const OUString & Namespace,
     const OUString & Name,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
#if SUPD == 310
        throw (css::uno::RuntimeException, css::xml::sax::SAXException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    virtual void SAL_CALL endFastElement(::sal_Int32 Element)
#if SUPD == 310
        throw (css::uno::RuntimeException, css::xml::sax::SAXException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    virtual void SAL_CALL endUnknownElement
    (const OUString & Namespace,
     const OUString & Name)
#if SUPD == 310
        throw (css::uno::RuntimeException, css::xml::sax::SAXException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
    createFastChildContext
    (::sal_Int32 Element,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
#if SUPD == 310
        throw (css::uno::RuntimeException, css::xml::sax::SAXException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
    createUnknownChildContext
    (const OUString & Namespace,
     const OUString & Name,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
#if SUPD == 310
        throw (css::uno::RuntimeException, css::xml::sax::SAXException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    virtual void SAL_CALL characters(const OUString & aChars)
#if SUPD == 310
        throw (css::uno::RuntimeException, css::xml::sax::SAXException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    // ::com::sun::star::xml::sax::XFastShapeContextHandler:
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL getShape()
#if SUPD == 310
        throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getDrawPage()
#if SUPD == 310
        throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    virtual void SAL_CALL setDrawPage
    (const css::uno::Reference< css::drawing::XDrawPage > & the_value)
#if SUPD == 310
        throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

#if SUPD == 310
    virtual css::uno::Reference< css::drawing::XShapes > SAL_CALL getShapes()
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setShapes
    (const css::uno::Reference< css::drawing::XShapes > & the_value)
        throw (css::uno::RuntimeException);
#endif	// SUPD == 310

    virtual css::uno::Reference< css::frame::XModel > SAL_CALL getModel()
#if SUPD == 310
        throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    virtual void SAL_CALL setModel
    (const css::uno::Reference< css::frame::XModel > & the_value)
#if SUPD == 310
        throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL
#if SUPD == 310
    getInputStream() throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
    getInputStream() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    virtual void SAL_CALL setInputStream
    (const css::uno::Reference< css::io::XInputStream > & the_value)
#if SUPD == 310
        throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

    virtual OUString SAL_CALL getRelationFragmentPath()
#if SUPD == 310
        throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310
    virtual void SAL_CALL setRelationFragmentPath
    (const OUString & the_value)
#if SUPD == 310
        throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

#if SUPD == 310
    virtual ::sal_Int32 SAL_CALL getStartToken() throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL setStartToken( ::sal_Int32 _starttoken ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    virtual css::awt::Point SAL_CALL getPosition() throw (css::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL setPosition(const css::awt::Point& rPosition) throw (css::uno::RuntimeException) SAL_OVERRIDE;
#else	// SUPD == 310
    virtual ::sal_Int32 SAL_CALL getStartToken() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setStartToken( ::sal_Int32 _starttoken ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::awt::Point SAL_CALL getPosition() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPosition(const css::awt::Point& rPosition) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
#endif	// SUPD == 310

private:
    ShapeContextHandler(ShapeContextHandler &); // not defined
    void operator =(ShapeContextHandler &); // not defined

    ::sal_uInt32 mnStartToken;
    css::awt::Point maPosition;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    drawingml::ShapePtr mpShape;
    ::boost::shared_ptr< vml::Drawing > mpDrawing;

    typedef boost::shared_ptr<drawingml::GraphicShapeContext>
    GraphicShapeContextPtr;
    css::uno::Reference<XFastContextHandler> mxDrawingFragmentHandler;
    css::uno::Reference<XFastContextHandler> mxGraphicShapeContext;
    css::uno::Reference<XFastContextHandler> mxDiagramShapeContext;
    css::uno::Reference<XFastContextHandler> mxLockedCanvasContext;
    css::uno::Reference<XFastContextHandler> mxWpsContext;
    css::uno::Reference<css::drawing::XShape> mxSavedShape;
    css::uno::Reference<XFastContextHandler> mxWpgContext;
    css::uno::Reference<XFastContextHandler> mxChartShapeContext;

    core::XmlFilterRef mxFilterBase;
    drawingml::ThemePtr mpThemePtr;
    css::uno::Reference<css::drawing::XDrawPage> mxDrawPage;
    css::uno::Reference<css::io::XInputStream> mxInputStream;
    OUString msRelationFragmentPath;

    css::uno::Reference<XFastContextHandler> getGraphicShapeContext(::sal_Int32 Element);
    css::uno::Reference<XFastContextHandler> getChartShapeContext(::sal_Int32 Element);
    css::uno::Reference<XFastContextHandler> getDrawingShapeContext();
    css::uno::Reference<XFastContextHandler> getDiagramShapeContext();
    css::uno::Reference<XFastContextHandler> getLockedCanvasContext(sal_Int32 nElement);
    css::uno::Reference<XFastContextHandler> getWpsContext(sal_Int32 nStartElement, sal_Int32 nElement);
    css::uno::Reference<XFastContextHandler> getWpgContext(sal_Int32 nElement);
    css::uno::Reference<XFastContextHandler> getContextHandler(sal_Int32 nElement = 0);
};

}}

#endif // INCLUDED_OOX_SOURCE_SHAPE_SHAPECONTEXTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
