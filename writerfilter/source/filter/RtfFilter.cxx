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

#include <cppuhelper/implementationentry.hxx>
#include <RtfFilter.hxx>
#if SUPD == 310
#include <comphelper/mediadescriptor.hxx>
#else	// SUPD == 310
#include <unotools/mediadescriptor.hxx>
#include <cppuhelper/supportsservice.hxx>
#endif	// SUPD == 310
#include <dmapper/DomainMapper.hxx>
#include <rtftok/RTFDocument.hxx>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <osl/diagnose.h>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>

#if SUPD == 310
#include <sal/log.hxx>
#endif	// SUPD == 310

using namespace ::com::sun::star;

RtfFilter::RtfFilter(const uno::Reference< uno::XComponentContext >& rxContext)
    : m_xContext(rxContext)
{
}

RtfFilter::~RtfFilter()
{
}

#if SUPD == 310
sal_Bool RtfFilter::filter(const uno::Sequence< beans::PropertyValue >& aDescriptor) throw(uno::RuntimeException)
#else	// SUPD == 310
sal_Bool RtfFilter::filter(const uno::Sequence< beans::PropertyValue >& aDescriptor) throw(uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    sal_uInt32 nStartTime = osl_getGlobalTimer();
    if (m_xSrcDoc.is())
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF(m_xContext->getServiceManager(), uno::UNO_QUERY_THROW);
        uno::Reference< uno::XInterface > xIfc(xMSF->createInstance("com.sun.star.comp.Writer.RtfExport"), uno::UNO_QUERY_THROW);
        if (!xIfc.is())
            return sal_False;
        uno::Reference< document::XExporter > xExporter(xIfc, uno::UNO_QUERY_THROW);
        uno::Reference< document::XFilter > xFilter(xIfc, uno::UNO_QUERY_THROW);
        if (!xExporter.is() || !xFilter.is())
            return sal_False;
        xExporter->setSourceDocument(m_xSrcDoc);
        return xFilter->filter(aDescriptor);
    }

    bool bResult(false);
    uno::Reference<task::XStatusIndicator> xStatusIndicator;

    try
    {
#if SUPD == 310
        comphelper::MediaDescriptor aMediaDesc(aDescriptor);
#else	// SUPD == 310
        utl::MediaDescriptor aMediaDesc(aDescriptor);
#endif	// SUPD == 310
        bool bRepairStorage = aMediaDesc.getUnpackedValueOrDefault("RepairPackage", false);
        bool bIsNewDoc = !aMediaDesc.getUnpackedValueOrDefault("InsertMode", false);
        uno::Reference<text::XTextRange> xInsertTextRange = aMediaDesc.getUnpackedValueOrDefault("TextInsertModeRange", uno::Reference<text::XTextRange>());
#ifdef DEBUG_WRITERFILTER
#if SUPD == 310
        OUString sURL = aMediaDesc.getUnpackedValueOrDefault(comphelper::MediaDescriptor::PROP_URL(), OUString());
#else	// SUPD == 310
        OUString sURL = aMediaDesc.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_URL(), OUString());
#endif	// SUPD == 310
        std::string sURLc = OUStringToOString(sURL, RTL_TEXTENCODING_ASCII_US).getStr();

        writerfilter::TagLogger::Pointer_t dmapper_logger
        (writerfilter::TagLogger::getInstance("DOMAINMAPPER"));
        if (getenv("SW_DEBUG_WRITERFILTER"))
            dmapper_logger->setFileName(sURLc);
        dmapper_logger->startDocument();
#endif
        uno::Reference< io::XInputStream > xInputStream;

        aMediaDesc.addInputStream();
#if SUPD == 310
        aMediaDesc[ comphelper::MediaDescriptor::PROP_INPUTSTREAM() ] >>= xInputStream;
#else	// SUPD == 310
        aMediaDesc[ utl::MediaDescriptor::PROP_INPUTSTREAM() ] >>= xInputStream;
#endif	// SUPD == 310

#if SUPD != 310
        // If this is set, write to this file, instead of the real document during paste.
        char* pEnv = getenv("SW_DEBUG_RTF_PASTE_TO");
        OUString aOutStr;
        if (!bIsNewDoc && pEnv && utl::LocalFileHelper::ConvertPhysicalNameToURL(OUString::fromUtf8(pEnv), aOutStr))
        {
            SvStream* pOut = utl::UcbStreamHelper::CreateStream(aOutStr, STREAM_WRITE);
            SvStream* pIn = utl::UcbStreamHelper::CreateStream(xInputStream);
            pOut->WriteStream(*pIn);
            delete pOut;
            return true;
        }

        // If this is set, read from this file, instead of the real clipboard during paste.
        pEnv = getenv("SW_DEBUG_RTF_PASTE_FROM");
        if (!bIsNewDoc && pEnv)
        {
            OUString aInStr;
            utl::LocalFileHelper::ConvertPhysicalNameToURL(OUString::fromUtf8(pEnv), aInStr);
            SvStream* pStream = utl::UcbStreamHelper::CreateStream(aInStr, STREAM_READ);
            uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
            xInputStream.set(xStream, uno::UNO_QUERY);
        }
#endif	// SUPD != 310

#if SUPD == 310
        uno::Reference<frame::XFrame> xFrame = aMediaDesc.getUnpackedValueOrDefault(comphelper::MediaDescriptor::PROP_FRAME(),
#else	// SUPD == 310
        uno::Reference<frame::XFrame> xFrame = aMediaDesc.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_FRAME(),
#endif	// SUPD == 310
                                               uno::Reference<frame::XFrame>());

#if SUPD == 310
        xStatusIndicator = aMediaDesc.getUnpackedValueOrDefault(comphelper::MediaDescriptor::PROP_STATUSINDICATOR(),
#else	// SUPD == 310
        xStatusIndicator = aMediaDesc.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_STATUSINDICATOR(),
#endif	// SUPD == 310
                           uno::Reference<task::XStatusIndicator>());

        writerfilter::Stream::Pointer_t pStream(
            new writerfilter::dmapper::DomainMapper(m_xContext, xInputStream, m_xDstDoc, bRepairStorage, writerfilter::dmapper::DOCUMENT_RTF, xInsertTextRange, bIsNewDoc));
        writerfilter::rtftok::RTFDocument::Pointer_t const pDocument(
            writerfilter::rtftok::RTFDocumentFactory::createDocument(m_xContext, xInputStream, m_xDstDoc, xFrame, xStatusIndicator));
        pDocument->resolve(*pStream);
        bResult = true;
#ifdef DEBUG_WRITERFILTER
        dmapper_logger->endDocument();
#endif
        sal_uInt32 nEndTime = osl_getGlobalTimer();
        SAL_INFO("writerfilter.profile", OSL_THIS_FUNC << " finished in " << nEndTime - nStartTime << " ms");
    }
    catch (const io::WrongFormatException& e)
    {
        // cannot throw WrongFormatException directly :(
        throw lang::WrappedTargetRuntimeException("",
                static_cast<OWeakObject*>(this), uno::makeAny(e));
    }
    catch (const uno::Exception& e)
    {
        SAL_INFO("writerfilter", "Exception caught: " << e.Message);
    }

    if (xStatusIndicator.is())
        xStatusIndicator->end();
    return bResult;
}

#if SUPD == 310
void RtfFilter::cancel() throw(uno::RuntimeException)
#else	// SUPD == 310
void RtfFilter::cancel() throw(uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
}

#if SUPD == 310
void RtfFilter::setSourceDocument(const uno::Reference< lang::XComponent >& xDoc) throw(lang::IllegalArgumentException, uno::RuntimeException)
#else	// SUPD == 310
void RtfFilter::setSourceDocument(const uno::Reference< lang::XComponent >& xDoc) throw(lang::IllegalArgumentException, uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    m_xSrcDoc = xDoc;
}

#if SUPD == 310
void RtfFilter::setTargetDocument(const uno::Reference< lang::XComponent >& xDoc) throw(lang::IllegalArgumentException, uno::RuntimeException)
#else	// SUPD == 310
void RtfFilter::setTargetDocument(const uno::Reference< lang::XComponent >& xDoc) throw(lang::IllegalArgumentException, uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    m_xDstDoc = xDoc;
}

#if SUPD == 310
void RtfFilter::initialize(const uno::Sequence< uno::Any >& /*aArguments*/) throw(uno::Exception, uno::RuntimeException)
#else	// SUPD == 310
void RtfFilter::initialize(const uno::Sequence< uno::Any >& /*aArguments*/) throw(uno::Exception, uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    // The DOCX exporter here extracts 'type' of the filter, ie 'Word' or
    // 'Word Template' but we don't need it for RTF.
}

#if SUPD == 310
OUString RtfFilter::getImplementationName() throw(uno::RuntimeException)
#else	// SUPD == 310
OUString RtfFilter::getImplementationName() throw(uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    return RtfFilter_getImplementationName();
}

#if SUPD == 310
#define SERVICE_NAME1 "com.sun.star.document.ImportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ExportFilter"
sal_Bool RtfFilter::supportsService(const OUString& rServiceName) throw(uno::RuntimeException)
#else	// SUPD == 310
sal_Bool RtfFilter::supportsService(const OUString& rServiceName) throw(uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
#if SUPD == 310
    return (rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME1 ) ) ||
            rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME2 ) ));
#else	// SUPD == 310
    return cppu::supportsService(this, rServiceName);
#endif	// SUPD == 310
}

#if SUPD == 310
uno::Sequence< OUString > RtfFilter::getSupportedServiceNames() throw(uno::RuntimeException)
#else	// SUPD == 310
uno::Sequence< OUString > RtfFilter::getSupportedServiceNames() throw(uno::RuntimeException, std::exception)
#endif	// SUPD == 310
{
    return RtfFilter_getSupportedServiceNames();
}

/* Helpers, used by shared lib exports. */
OUString RtfFilter_getImplementationName() throw(uno::RuntimeException)
{
    return OUString("com.sun.star.comp.Writer.RtfFilter");
}

uno::Sequence< OUString > RtfFilter_getSupportedServiceNames() throw(uno::RuntimeException)
{
    uno::Sequence < OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.document.ImportFilter";
    pArray[1] = "com.sun.star.document.ExportFilter";
    return aRet;
}

uno::Reference< uno::XInterface > RtfFilter_createInstance(const uno::Reference< uno::XComponentContext >& xContext) throw(uno::Exception)
{
    return (cppu::OWeakObject*) new RtfFilter(xContext);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
