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

#include "drawingml/textparagraph.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "drawingml/textcharacterproperties.hxx"

#include <rtl/ustring.hxx>
#include "oox/helper/propertyset.hxx"
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>

#if SUPD == 310
#include <sal/log.hxx>
#endif	// SUPD == 310

using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

namespace oox { namespace drawingml {

TextParagraph::TextParagraph()
{
}

TextParagraph::~TextParagraph()
{
}

void TextParagraph::insertAt(
        const ::oox::core::XmlFilterBase& rFilterBase,
#if SUPD == 310
        const css::uno::Reference < XText > &xText,
        const css::uno::Reference < XTextCursor > &xAt,
#else	// SUPD == 310
        const Reference < XText > &xText,
        const Reference < XTextCursor > &xAt,
#endif	// SUPD == 310
        const TextCharacterProperties& rTextStyleProperties,
        const TextListStyle& rTextListStyle, bool bFirst, float nDefaultCharHeight) const
{
    try {
        sal_Int32 nParagraphSize = 0;
#if SUPD == 310
        css::uno::Reference< XTextRange > xStart( xAt, UNO_QUERY );
#endif	// SUPD == 310

        sal_Int16 nLevel = maProperties.getLevel();

        SAL_INFO("oox", "TextParagraph::insertAt() - level " << nLevel);

        const TextParagraphPropertiesVector& rListStyle = rTextListStyle.getListStyle();
        if ( nLevel >= static_cast< sal_Int16 >( rListStyle.size() ) )
            nLevel = 0;
        TextParagraphPropertiesPtr pTextParagraphStyle;
        if ( rListStyle.size() )
            pTextParagraphStyle = rListStyle[ nLevel ];

        TextCharacterProperties aTextCharacterStyle;
        if ( pTextParagraphStyle.get() )
            aTextCharacterStyle.assignUsed( pTextParagraphStyle->getTextCharacterProperties() );
        aTextCharacterStyle.assignUsed( rTextStyleProperties );
        aTextCharacterStyle.assignUsed( maProperties.getTextCharacterProperties() );

        if( !bFirst )
        {
#if SUPD == 310
            xText->insertControlCharacter( xStart, ControlCharacter::APPEND_PARAGRAPH, sal_False );
#else	// SUPD == 310
            xText->insertControlCharacter( xAt, ControlCharacter::APPEND_PARAGRAPH, sal_False );
#endif	// SUPD == 310
            xAt->gotoEnd( sal_True );
        }

        sal_Int32 nCharHeight = 0;
        if ( maRuns.begin() == maRuns.end() )
        {
            PropertySet aPropSet( xAt );

            TextCharacterProperties aTextCharacterProps( aTextCharacterStyle );
            aTextCharacterProps.assignUsed( maEndProperties );
            if ( aTextCharacterProps.moHeight.has() )
                nCharHeight = aTextCharacterProps.moHeight.get();
            aTextCharacterProps.pushToPropSet( aPropSet, rFilterBase );
        }
        else
        {
            for( TextRunVector::const_iterator aIt = maRuns.begin(), aEnd = maRuns.end(); aIt != aEnd; ++aIt )
            {
                sal_Int32 nLen = (*aIt)->getText().getLength();
                // n#759180: Force use, maEndProperties for the last segment
                // This is currently applied to only empty runs
                if( !nLen && ( ( aIt + 1 ) == aEnd ) )
                    (*aIt)->getTextCharacterProperties().assignUsed( maEndProperties );
                nCharHeight = std::max< sal_Int32 >( nCharHeight, (*aIt)->insertAt( rFilterBase, xText, xAt, aTextCharacterStyle, nDefaultCharHeight ) );
                nParagraphSize += nLen;
            }
        }
        xAt->gotoEnd( sal_True );

        PropertyMap aioBulletList;
#if SUPD == 310
        css::uno::Reference< XPropertySet > xProps( xAt, UNO_QUERY);
#else	// SUPD == 310
        Reference< XPropertySet > xProps( xAt, UNO_QUERY);
#endif	// SUPD == 310
        if ( pTextParagraphStyle.get() )
        {
            TextParagraphProperties aParaProp;
            aParaProp.apply( *pTextParagraphStyle );
            aParaProp.apply( maProperties );

            // bullets have same color as following texts by default
            if( !aioBulletList.hasProperty( PROP_BulletColor ) && maRuns.size() > 0
                && (*maRuns.begin())->getTextCharacterProperties().maCharColor.isUsed() )
                aioBulletList.setProperty( PROP_BulletColor, (*maRuns.begin())->getTextCharacterProperties().maCharColor.getColor( rFilterBase.getGraphicHelper() ));
            if( !aioBulletList.hasProperty( PROP_BulletColor ) && aTextCharacterStyle.maCharColor.isUsed() )
                aioBulletList.setProperty( PROP_BulletColor, aTextCharacterStyle.maCharColor.getColor( rFilterBase.getGraphicHelper() ));

            float fCharacterSize = nCharHeight > 0 ? GetFontHeight ( nCharHeight ) : pTextParagraphStyle->getCharHeightPoints( 12 );
            aParaProp.pushToPropSet( &rFilterBase, xProps, aioBulletList, &pTextParagraphStyle->getBulletList(), true, fCharacterSize, true );
        }

        // empty paragraphs do not have bullets in ppt
        if ( !nParagraphSize )
        {
            const OUString sNumberingLevel( "NumberingLevel" );
            xProps->setPropertyValue( sNumberingLevel, Any( static_cast< sal_Int16 >( -1 ) ) );
        }

// FIXME this is causing a lot of dispruption (ie does not work). I wonder what to do -- Hub
//          Reference< XTextRange > xEnd( xAt, UNO_QUERY );
//      Reference< XPropertySet > xProps2( xEnd, UNO_QUERY );
//          mpEndProperties->pushToPropSet( xProps2 );
    }
    catch( Exception & )
    {
        SAL_INFO("oox", "exception in TextParagraph::insertAt");
    }
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
