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

#include "oox/core/fragmenthandler2.hxx"
#include "oox/core/xmlfilterbase.hxx"

namespace oox {
namespace core {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

using ::com::sun::star::uno::Sequence;

FragmentHandler2::FragmentHandler2( XmlFilterBase& rFilter, const OUString& rFragmentPath, bool bEnableTrimSpace ) :
    FragmentHandler( rFilter, rFragmentPath ),
    ContextHandler2Helper( bEnableTrimSpace )
{
}

FragmentHandler2::~FragmentHandler2()
{
}

// com.sun.star.xml.sax.XFastDocumentHandler interface --------------------

#if SUPD == 310
void SAL_CALL FragmentHandler2::startDocument() throw( SAXException, RuntimeException )
#else	// SUPD == 310
void SAL_CALL FragmentHandler2::startDocument() throw( SAXException, RuntimeException, std::exception )
#endif	// SUPD == 310
{
    initializeImport();
}

#if SUPD == 310
void SAL_CALL FragmentHandler2::endDocument() throw( SAXException, RuntimeException )
#else	// SUPD == 310
void SAL_CALL FragmentHandler2::endDocument() throw( SAXException, RuntimeException, std::exception )
#endif	// SUPD == 310
{
    finalizeImport();
}

bool FragmentHandler2::prepareMceContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case MCE_TOKEN( AlternateContent ):
            aMceState.push_back( MCE_STARTED );
            break;

        case MCE_TOKEN( Choice ):
            {
                OUString aRequires = rAttribs.getString( ( XML_Requires ), OUString("none") );
                if (!getFilter().hasNamespaceURL(aRequires))
                    // Check to see if we have this namespace defined first,
                    // because calling getNamespaceURL() would throw if the
                    // namespace doesn't exist.
                    return false;

                aRequires = getFilter().getNamespaceURL( aRequires );
                if( getFilter().getNamespaceId( aRequires ) > 0 && !aMceState.empty() && aMceState.back() == MCE_STARTED )
                    aMceState.back() = MCE_FOUND_CHOICE;
                else
                    return false;
            }
            break;

        case MCE_TOKEN( Fallback ):
            if( !aMceState.empty() && aMceState.back() == MCE_STARTED )
                break;
            return false;
        default:
            {
                OUString str = rAttribs.getString( MCE_TOKEN( Ignorable ), OUString() );
                if( !str.isEmpty() )
                {
                    // Sequence< ::com::sun::star::xml::FastAttribute > attrs = rAttribs.getFastAttributeList()->getFastAttributes();
                    // printf("MCE: %s\n", OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
                    // TODO: Check & Get the namespaces in "Ignorable"
                    // printf("NS: %d : %s\n", attrs.getLength(), OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
                }
            }
            return false;
    }
    return true;
}

// com.sun.star.xml.sax.XFastContextHandler interface -------------------------

#if SUPD == 310
css::uno::Reference< XFastContextHandler > SAL_CALL FragmentHandler2::createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< XFastAttributeList >& rxAttribs ) throw( SAXException, RuntimeException )
#else	// SUPD == 310
Reference< XFastContextHandler > SAL_CALL FragmentHandler2::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs ) throw( SAXException, RuntimeException, std::exception )
#endif	// SUPD == 310
{
    if( getNamespace( nElement ) == NMSP_mce ) // TODO for checking 'Ignorable'
    {
        if( prepareMceContext( nElement, AttributeList( rxAttribs ) ) )
            return getFastContextHandler();
        return NULL;
    }
    return implCreateChildContext( nElement, rxAttribs );
}

void SAL_CALL FragmentHandler2::startFastElement(
#if SUPD == 310
        sal_Int32 nElement, const css::uno::Reference< XFastAttributeList >& rxAttribs ) throw( SAXException, RuntimeException )
#else	// SUPD == 310
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs ) throw( SAXException, RuntimeException, std::exception )
#endif	// SUPD == 310
{
    implStartElement( nElement, rxAttribs );
}

#if SUPD == 310
void SAL_CALL FragmentHandler2::characters( const OUString& rChars ) throw( SAXException, RuntimeException )
#else	// SUPD == 310
void SAL_CALL FragmentHandler2::characters( const OUString& rChars ) throw( SAXException, RuntimeException, std::exception )
#endif	// SUPD == 310
{
    implCharacters( rChars );
}

#if SUPD == 310
void SAL_CALL FragmentHandler2::endFastElement( sal_Int32 nElement ) throw( SAXException, RuntimeException )
#else	// SUPD == 310
void SAL_CALL FragmentHandler2::endFastElement( sal_Int32 nElement ) throw( SAXException, RuntimeException, std::exception )
#endif	// SUPD == 310
{
    /* If MCE */
    switch( nElement )
    {
        case MCE_TOKEN( AlternateContent ):
            aMceState.pop_back();
            break;
    }

    implEndElement( nElement );
}

// oox.core.ContextHandler interface ------------------------------------------

ContextHandlerRef FragmentHandler2::createRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    return implCreateRecordContext( nRecId, rStrm );
}

void FragmentHandler2::startRecord( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    implStartRecord( nRecId, rStrm );
}

void FragmentHandler2::endRecord( sal_Int32 nRecId )
{
    implEndRecord( nRecId );
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextHandlerRef FragmentHandler2::onCreateContext( sal_Int32, const AttributeList& )
{
    return 0;
}

void FragmentHandler2::onStartElement( const AttributeList& )
{
}

void FragmentHandler2::onCharacters( const OUString& )
{
}

void FragmentHandler2::onEndElement()
{
}

ContextHandlerRef FragmentHandler2::onCreateRecordContext( sal_Int32, SequenceInputStream& )
{
    return 0;
}

void FragmentHandler2::onStartRecord( SequenceInputStream& )
{
}

void FragmentHandler2::onEndRecord()
{
}

// oox.core.FragmentHandler2 interface ----------------------------------------

void FragmentHandler2::initializeImport()
{
}

void FragmentHandler2::finalizeImport()
{
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
