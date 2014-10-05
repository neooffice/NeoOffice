/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.
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
 * Modified October 2014 by Patrick Luby. NeoOffice is distributed under
 * GPL only under Section 4 of the Apache License v2.0.
 * 
 *************************************************************/



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


OOXMLFastDocumentHandler::OOXMLFastDocumentHandler(
    uno::Reference< uno::XComponentContext > const & context,
    Stream* pStream,
    OOXMLDocument* pDocument )
    : m_xContext(context)
    , mpStream( pStream )
#ifdef DEBUG_ELEMENT
    , mpTmpStream()
#endif
    , mpDocument( pDocument )
    , mpContextHandler()
{
#ifdef DEBUG_PROTOCOL
    if ( pStream )
    {
        mpTmpStream.reset( new StreamProtocol( pStream, debug_logger ) );
        mpStream = mpTmpStream.get();
    }
#endif
}

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
#endif  // USE_JAVA
        mpContextHandler.reset(
            new OOXMLFastContextHandler(m_xContext) );
        mpContextHandler->setStream(mpStream);
        mpContextHandler->setDocument(mpDocument);
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

    if ( mpStream == 0 && mpDocument == 0 )
    {
        // document handler has been created as unknown child - see <OOXMLFastDocumentHandler::createUnknownChildContext(..)>
        // --> do not provide a child context
        return NULL;
    }

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
        ( new OOXMLFastDocumentHandler( m_xContext, 0, 0 ) );
}

void SAL_CALL OOXMLFastDocumentHandler::characters(const ::rtl::OUString & /*aChars*/) 
    throw (uno::RuntimeException, xml::sax::SAXException)
{
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
}

void OOXMLFastDocumentHandler::setIsSubstream( bool bSubstream )
{
    if ( mpStream != 0 && mpDocument != 0 )
    {
        getContextHandler( )->getParserState( )->setInSectionGroup( bSubstream );
    }
}

}}
