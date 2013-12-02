/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * $RCSfile$
 * $Revision$
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
 * Modified December 2013 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

#include <iostream>
#include <boost/shared_ptr.hpp>
#ifdef DEBUG_ELEMENT
#include "ooxmlLoggers.hxx"
#include <resourcemodel/Protocol.hxx>
#endif
#include "OOXMLFastDocumentHandler.hxx"
#include "OOXMLFastContextHandler.hxx"
#include "OOXMLFastTokens.hxx"
#include "OOXMLFactory.hxx"

namespace writerfilter {
namespace ooxml
{
using namespace ::com::sun::star;
using namespace ::std;


OOXMLFastDocumentHandler::OOXMLFastDocumentHandler
(uno::Reference< uno::XComponentContext > const & context)
: m_xContext(context)
#ifdef USE_JAVA
// Fix crashing bug reported in the following Debian bug when opening
// a .docx that has no <w:document> tag:
// http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=550359
, mpDocument(NULL)
#endif	// USE_JAVA
{}

// ::com::sun::star::xml::sax::XFastContextHandler:
void SAL_CALL OOXMLFastDocumentHandler::startFastElement
(::sal_Int32 
#ifdef DEBUG_CONTEXT_STACK
Element
#endif
, const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/) 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    clog << this << ":start element:" 
         << fastTokenToId(Element)
         << endl;
#endif
}

void SAL_CALL OOXMLFastDocumentHandler::startUnknownElement
(const ::rtl::OUString & 
#ifdef DEBUG_CONTEXT_STACK
Namespace
#endif
, const ::rtl::OUString & 
#ifdef DEBUG_CONTEXT_STACK
Name
#endif
, 
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/) 
throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    clog << this << ":start unknown element:" 
         << OUStringToOString(Namespace, RTL_TEXTENCODING_ASCII_US).getStr()
         << ":"
         << OUStringToOString(Name, RTL_TEXTENCODING_ASCII_US).getStr()
         << endl;
#endif
}

void SAL_CALL OOXMLFastDocumentHandler::endFastElement(::sal_Int32 
#ifdef DEBUG_CONTEXT_STACK
Element
#endif
) 
throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    clog << this << ":end element:" 
         << fastTokenToId(Element)
         << endl;
#endif
}

void SAL_CALL OOXMLFastDocumentHandler::endUnknownElement
(const ::rtl::OUString & 
#ifdef DEBUG_CONTEXT_STACK
Namespace
#endif
, const ::rtl::OUString & 
#ifdef DEBUG_CONTEXT_STACK
Name
#endif
) 
throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    clog << this << ":end unknown element:" 
         << OUStringToOString(Namespace, RTL_TEXTENCODING_ASCII_US).getStr()
         << ":"
         << OUStringToOString(Name, RTL_TEXTENCODING_ASCII_US).getStr()
         << endl;
#endif
}

OOXMLFastContextHandler::Pointer_t 
OOXMLFastDocumentHandler::getContextHandler() const
{
    if (mpContextHandler == OOXMLFastContextHandler::Pointer_t())
    {
#ifdef USE_JAVA
        // Fix crashing bug reported in the following Debian bug when opening
        // a .docx that has no <w:document> tag:
        // http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=550359
        if (!mpDocument)
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Failed to connect to mail server.")), uno::Reference< XInterface >());
#endif	// USE_JAVA
        mpContextHandler.reset
        (new OOXMLFastContextHandler(m_xContext));
        mpContextHandler->setStream(mpStream);
        mpContextHandler->setDocument(mpDocument);
        mpContextHandler->setXNoteId(mnXNoteId);
        mpContextHandler->setForwardEvents(true);
    }

    return mpContextHandler;
}
    
uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
 OOXMLFastDocumentHandler::createFastChildContext
(::sal_Int32 Element, 
 const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/) 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    clog << this << ":createFastChildContext:" 
         << fastTokenToId(Element)
         << endl;
#endif
    
    return OOXMLFactory::getInstance()->createFastChildContextFromStart(getContextHandler().get(), Element);
}
    
uno::Reference< xml::sax::XFastContextHandler > SAL_CALL 
OOXMLFastDocumentHandler::createUnknownChildContext
(const ::rtl::OUString & 
#ifdef DEBUG_CONTEXT_STACK
Namespace
#endif
, 
 const ::rtl::OUString & 
#ifdef DEBUG_CONTEXT_STACK
Name
#endif
, const uno::Reference< xml::sax::XFastAttributeList > & /*Attribs*/) 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
#ifdef DEBUG_CONTEXT_STACK
    clog << this << ":createUnknownChildContext:" 
         << OUStringToOString(Namespace, RTL_TEXTENCODING_ASCII_US).getStr()
         << ":"
         << OUStringToOString(Name, RTL_TEXTENCODING_ASCII_US).getStr()
         << endl;
#endif

    return uno::Reference< xml::sax::XFastContextHandler >
        (new OOXMLFastDocumentHandler(m_xContext));
}

void SAL_CALL OOXMLFastDocumentHandler::characters(const ::rtl::OUString & /*aChars*/) 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    // TODO: Insert your implementation for "characters" here.
}

// ::com::sun::star::xml::sax::XFastDocumentHandler:
void SAL_CALL OOXMLFastDocumentHandler::startDocument() 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
}

void SAL_CALL OOXMLFastDocumentHandler::endDocument() 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
}

void SAL_CALL OOXMLFastDocumentHandler::setDocumentLocator
(const uno::Reference< xml::sax::XLocator > & /*xLocator*/) 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    // TODO: Insert your implementation for "setDocumentLocator" here.
}

void OOXMLFastDocumentHandler::setStream(Stream * pStream)
{ 
#ifdef DEBUG_PROTOCOL
    mpTmpStream.reset(new StreamProtocol(pStream, debug_logger));
    mpStream = mpTmpStream.get();
#else
    mpStream = pStream;
#endif
}

void OOXMLFastDocumentHandler::setDocument(OOXMLDocument * pDocument)
{
    mpDocument = pDocument;
}

void OOXMLFastDocumentHandler::setXNoteId(const sal_Int32 nXNoteId)
{
    mnXNoteId = nXNoteId;
}

void OOXMLFastDocumentHandler::setIsSubstream( bool bSubstream )
{
    getContextHandler( )->getParserState( )->setInSectionGroup( bSubstream );
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
