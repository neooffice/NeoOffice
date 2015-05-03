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

#include "drawingml/textfield.hxx"

#include <list>

#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextField.hpp>

#include "oox/helper/helper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "drawingml/textparagraphproperties.hxx"
#include "drawingml/textcharacterproperties.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;

namespace oox { namespace drawingml {

TextField::TextField()
{
}

namespace {

/** intsanciate the textfields. Because of semantics difference between
 * OpenXML and OpenOffice, some OpenXML field might cause two fields to be created.
 * @param aFields the created fields. The list is empty if no field has been created.
 * @param xModel the model
 * @param sType the OpenXML field type.
 */
#if SUPD == 310
void lclCreateTextFields( std::list< css::uno::Reference< XTextField > > & aFields,
                                                            const css::uno::Reference< XModel > & xModel, const OUString & sType )
#else	// SUPD == 310
void lclCreateTextFields( std::list< Reference< XTextField > > & aFields,
                                                            const Reference< XModel > & xModel, const OUString & sType )
#endif	// SUPD == 310
{
#if SUPD == 310
    css::uno::Reference< XInterface > xIface;
    css::uno::Reference< XMultiServiceFactory > xFactory( xModel, UNO_QUERY_THROW );
#else	// SUPD == 310
    Reference< XInterface > xIface;
    Reference< XMultiServiceFactory > xFactory( xModel, UNO_QUERY_THROW );
#endif	// SUPD == 310

    if( sType.startsWith("datetime"))
    {
        OString s = OUStringToOString( sType, RTL_TEXTENCODING_UTF8);
        OString p( s.pData->buffer + 8 );
        try
        {
            bool bIsDate = true;
            int idx = p.toInt32();
//              OSL_TRACE( "OOX: p = %s, %d", p.pData->buffer, idx );
            xIface = xFactory->createInstance( "com.sun.star.text.TextField.DateTime" );
#if SUPD == 310
            aFields.push_back( css::uno::Reference< XTextField > ( xIface, UNO_QUERY ) );
            css::uno::Reference< XPropertySet > xProps( xIface, UNO_QUERY_THROW );
#else	// SUPD == 310
            aFields.push_back( Reference< XTextField > ( xIface, UNO_QUERY ) );
            Reference< XPropertySet > xProps( xIface, UNO_QUERY_THROW );
#endif	// SUPD == 310

            // here we should format the field properly. waiting after #i81091.
            switch( idx )
            {
            case 1: // Date dd/mm/yyyy
                // this is the default format...
                break;
            case 2: // Date Day, Month dd, yyyy
                break;
            case 3: // Date dd Month yyyy
                break;
            case 4: // Date Month dd, yyyy
                break;
            case 5: // Date dd-Mon-yy
                break;
            case 6: // Date Month yy
                break;
            case 7: // Date Mon-yy
                break;
            case 8: // DateTime dd/mm/yyyy H:MM PM
                lclCreateTextFields( aFields, xModel, "datetime12" );
                break;
            case 9: // DateTime dd/mm/yy H:MM:SS PM
                lclCreateTextFields( aFields, xModel, "datetime13" );
                break;
            case 10: // Time H:MM
                bIsDate = false;
                break;
            case 11: // Time H:MM:SS
                bIsDate = false;
                // this is the default format
                break;
            case 12: // Time H:MM PM
                bIsDate = false;
                break;
            case 13: // Time H:MM:SS PM
                bIsDate = false;
                break;
            }
            xProps->setPropertyValue( "IsDate", makeAny( bIsDate ) );
            xProps->setPropertyValue( "IsFixed", makeAny( false ) );
        }
        catch(Exception & e)
        {
            OSL_TRACE( "Exception %s",  OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
        }
    }
    else if ( sType.equalsAscii( "slidenum" ) )
    {
        xIface = xFactory->createInstance( "com.sun.star.text.TextField.PageNumber" );
#if SUPD == 310
        aFields.push_back( css::uno::Reference< XTextField > ( xIface, UNO_QUERY ) );
#else	// SUPD == 310
        aFields.push_back( Reference< XTextField > ( xIface, UNO_QUERY ) );
#endif	// SUPD == 310
    }
}

} // namespace

sal_Int32 TextField::insertAt(
        const ::oox::core::XmlFilterBase& rFilterBase,
#if SUPD == 310
        const css::uno::Reference < XText > & xText,
        const css::uno::Reference < XTextCursor > &xAt,
#else	// SUPD == 310
        const Reference < XText > & xText,
        const Reference < XTextCursor > &xAt,
#endif	// SUPD == 310
        const TextCharacterProperties& rTextCharacterStyle,
        float /*nDefaultCharHeight*/) const
{
    sal_Int32 nCharHeight = 0;
    try
    {
        PropertyMap aioBulletList;
#if SUPD == 310
        css::uno::Reference< XTextRange > xStart( xAt, UNO_QUERY );
        css::uno::Reference< XPropertySet > xProps( xStart, UNO_QUERY);
#else	// SUPD == 310
        Reference< XPropertySet > xProps( xAt, UNO_QUERY);
#endif	// SUPD == 310
        PropertySet aPropSet( xProps );

        maTextParagraphProperties.pushToPropSet( &rFilterBase, xProps, aioBulletList, NULL, true, 18 );

        TextCharacterProperties aTextCharacterProps( rTextCharacterStyle );
        aTextCharacterProps.assignUsed( maTextParagraphProperties.getTextCharacterProperties() );
        aTextCharacterProps.assignUsed( getTextCharacterProperties() );
        if ( aTextCharacterProps.moHeight.has() )
            nCharHeight = aTextCharacterProps.moHeight.get();
        aTextCharacterProps.pushToPropSet( aPropSet, rFilterBase );

#if SUPD == 310
        std::list< css::uno::Reference< XTextField > > fields;
#else	// SUPD == 310
        std::list< Reference< XTextField > > fields;
#endif	// SUPD == 310
        lclCreateTextFields( fields, rFilterBase.getModel(), msType );
        if( !fields.empty() )
        {
            bool bFirst = true;
#if SUPD == 310
            for( std::list< css::uno::Reference< XTextField > >::iterator iter = fields.begin();
#else	// SUPD == 310
            for( std::list< Reference< XTextField > >::iterator iter = fields.begin();
#endif	// SUPD == 310
                     iter != fields.end(); ++iter )
            {
                if( iter->is() )
                {
#if SUPD == 310
                    css::uno::Reference< XTextContent > xContent( *iter, UNO_QUERY);
#else	// SUPD == 310
                    Reference< XTextContent > xContent( *iter, UNO_QUERY);
#endif	// SUPD == 310
                    if( bFirst)
                    {
                        bFirst = false;
                    }
                    else
                    {
#if SUPD == 310
                        xText->insertString( xStart, " ", sal_False );
#else	// SUPD == 310
                        xText->insertString( xAt, " ", sal_False );
#endif	// SUPD == 310
                    }
#if SUPD == 310
                    xText->insertTextContent( xStart, xContent, sal_False );
#else	// SUPD == 310
                    xText->insertTextContent( xAt, xContent, sal_False );
#endif	// SUPD == 310
                }
            }
        }
        else
        {
#if SUPD == 310
            xText->insertString( xStart, getText(), sal_False );
#else	// SUPD == 310
            xText->insertString( xAt, getText(), sal_False );
#endif	// SUPD == 310
        }
    }
    catch( const Exception&  )
    {
        OSL_TRACE("OOX:  TextField::insertAt() exception");
    }

    return nCharHeight;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
