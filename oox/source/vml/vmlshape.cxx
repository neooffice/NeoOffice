/*************************************************************************
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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
 * Modified March 2013 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

#include <rtl/ustring.hxx>
#include "oox/vml/shape.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFrame.hpp>

#if DEBUG
#include <iostream>
using namespace std;
#endif

using rtl::OUString;
using namespace com::sun::star;

namespace oox { namespace vml {

Shape::Shape( const rtl::OUString& rServiceName )
: msServiceName( rServiceName )
, mnCoordWidth( 0 )
, mnCoordHeight( 0 )
, mnStroked( 0 )
, mnZOrder( 0 )
, mbContainsText( false )
{
    // By default all the shapes are filled
    moFilled = ::boost::optional< sal_Bool > ( sal_True );

    sal_Int32 nWhite = OUString::createFromAscii( "FFFFFF" ).toInt32( sal_Int16( 16 ) );
    moFillColor = ::boost::optional< sal_Int32 > ( nWhite );
}
Shape::~Shape()
{
}

void Shape::applyAttributes( const vml::Shape& rSource )
{
	if ( rSource.msId.getLength() )
		msId = rSource.msId;
	if ( rSource.msSpId.getLength() )
		msSpId = rSource.msSpId;
	if ( rSource.msType.getLength() )
		msType = rSource.msType;
	if ( rSource.msShapeType.getLength() )
		msShapeType = rSource.msShapeType;
	if ( rSource.mnCoordWidth )
		mnCoordWidth = rSource.mnCoordWidth;
	if ( rSource.mnCoordHeight )
		mnCoordHeight = rSource.mnCoordHeight;
	if ( rSource.mnStroked )
		mnStroked = rSource.mnStroked;
    if ( rSource.moFilled )
        moFilled = rSource.moFilled;
    if ( rSource.moFillType )
		moFillType = rSource.moFillType;
    if ( rSource.moFillImageUrl )
        moFillImageUrl = rSource.moFillImageUrl;
	if ( rSource.moFillColor )
		moFillColor = rSource.moFillColor;
	if ( rSource.maPath.Name.getLength() )
		maPath = rSource.maPath;
	if ( rSource.msPosition.getLength() )
		msPosition = rSource.msPosition;
    if ( rSource.moStrokeColor )
        moStrokeColor = rSource.moStrokeColor;
    if ( rSource.moStrokeWeight )
        moStrokeWeight = rSource.moStrokeWeight;
	maPosition = rSource.maPosition;
	maSize = rSource.maSize;
    meFillImageMode = rSource.meFillImageMode;
    mbContainsText = rSource.mbContainsText;
}

::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > Shape::createAndInsert(
	const ::oox::core::XmlFilterBase& rFilterBase, const ::oox::vml::Shape& rShape,
		const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
			const awt::Rectangle* pShapeRect )
{
	uno::Reference< drawing::XShape > xShape;
            
#ifdef USE_JAVA
	// Fix crashing bug reported in the following NeoOffice forum topic by
	// disabling VML shapes. The bug is triggered by a group shape in an OOXML
	// document. The group shape will fail to be added to the document and that
	// will leave child shapes with dangling no parent which leads to a crash:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8547
	return xShape;
#else	// USE_JAVA
	if ( rShape.msServiceName )
	{
		try
		{
			uno::Reference< lang::XMultiServiceFactory > xServiceFact( rFilterBase.getModel(), uno::UNO_QUERY_THROW );
			xShape.set( xServiceFact->createInstance( rShape.msServiceName ), uno::UNO_QUERY_THROW );
    		rxShapes->add( xShape );

			awt::Point aPosition;
			awt::Size aSize;
			if ( pShapeRect )
			{
				aPosition.X = pShapeRect->X;
				aPosition.Y = pShapeRect->Y;
				aSize.Width = pShapeRect->Width;
				aSize.Height = pShapeRect->Height;
			}
			else
			{
				aPosition = maPosition;
				aSize = maSize;
			}
			xShape->setPosition( aPosition );
			xShape->setSize( aSize );
			uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
			try
			{
				if ( maPath.Name.getLength() )
					xPropSet->setPropertyValue( maPath.Name, maPath.Value );
                if ( !moFilled )
                    moFillType = ::boost::optional< sal_Int32 >( drawing::FillStyle_NONE );

				::rtl::OUString sFillStyle( rtl::OUString::createFromAscii( "FillStyle" ) );
				xPropSet->setPropertyValue( sFillStyle, uno::Any( *moFillType ) );

                if ( moFillImageUrl )
                {
                    // load the bitmap
                    uno::Reference< io::XInputStream > xInputStream( rFilterBase.openInputStream( *moFillImageUrl ), uno::UNO_QUERY_THROW );

		        	// load the fill bitmap into an XGraphic with the GraphicProvider
                    static const rtl::OUString sGraphicProvider = rtl::OUString::createFromAscii( "com.sun.star.graphic.GraphicProvider" );
                    uno::Reference< graphic::XGraphicProvider > xGraphicProvider( 
                            rFilterBase.getGlobalFactory()->createInstance( sGraphicProvider ), uno::UNO_QUERY_THROW );

                    static const rtl::OUString sInputStream = rtl::OUString::createFromAscii( "InputStream" );
                    beans::PropertyValues aMediaProperties(1);
		        	aMediaProperties[0].Name = sInputStream;
        			aMediaProperties[0].Value <<= xInputStream;

                    uno::Reference< awt::XBitmap > xBitmap( xGraphicProvider->queryGraphic( aMediaProperties ), 
                            uno::UNO_QUERY_THROW );
                    xPropSet->setPropertyValue( rtl::OUString::createFromAscii( "FillBitmap" ), uno::Any( xBitmap ) );

                    // Set the bitmap fill size/repeat properties
                    xPropSet->setPropertyValue( rtl::OUString::createFromAscii( "FillBitmapMode" ), uno::Any( meFillImageMode ) );
                }

                ::rtl::OUString sFillColor( rtl::OUString::createFromAscii( "FillColor" ) );
                if ( moFillColor )
                    xPropSet->setPropertyValue( sFillColor, uno::Any( *moFillColor ) );
                
                ::rtl::OUString sLineColor( rtl::OUString::createFromAscii( "LineColor" ) );
                if ( moStrokeColor )
                    xPropSet->setPropertyValue( sLineColor, uno::Any( *moStrokeColor ) );
                
                ::rtl::OUString sLineWidth( rtl::OUString::createFromAscii( "LineWidth" ) );
                if ( moStrokeWeight )
                    xPropSet->setPropertyValue( sLineWidth, uno::Any( *moStrokeWeight ) );
			}
			catch ( uno::Exception& e )
			{
#if DEBUG
                clog << "Exception when setting shape properties: ";
                clog << rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr( );
                clog << endl;
#endif
			}
			::rtl::OUString rServiceName( rtl::OUString::createFromAscii( "com.sun.star.drawing.CustomShape" ) );
			if ( rShape.msShapeType.getLength() && ( msServiceName == rServiceName ) )
			{
				uno::Reference< drawing::XEnhancedCustomShapeDefaulter > xDefaulter( xShape, uno::UNO_QUERY );
				if( xDefaulter.is() )
					xDefaulter->createCustomShapeDefaults( rShape.msShapeType );
			}
			mxShape = xShape;
		}
		catch( uno::Exception& e )
		{
#if DEBUG
            clog << "Exception thrown when creating shape: ";
            clog << rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr( ) << endl;
#endif
		}
	}
	return xShape;
#endif	// USE_JAVA
}

::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > Shape::createAndInsertFrame( 
	const ::oox::core::XmlFilterBase& rFilterBase, const ::oox::vml::Shape& rShape,
		const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
			const awt::Rectangle* pShapeRect )
{
	uno::Reference< drawing::XShape > xShape;
    static const OUString sFrameService( OUString::createFromAscii( "com.sun.star.text.TextFrame" ) );

    try
    {
        uno::Reference< lang::XMultiServiceFactory > xServiceFact( rFilterBase.getModel(), uno::UNO_QUERY_THROW );
        uno::Reference< text::XTextFrame > xFrame ( xServiceFact->createInstance( sFrameService ), uno::UNO_QUERY_THROW );
        xShape.set( xFrame, uno::UNO_QUERY_THROW );
  
        // Set all the properties to the frame
	    awt::Point aPosition;
    	awt::Size aSize;
	    if ( pShapeRect )
		{
			aPosition.X = pShapeRect->X;
			aPosition.Y = pShapeRect->Y;
			aSize.Width = pShapeRect->Width;
			aSize.Height = pShapeRect->Height;
		}
		else
		{
			aPosition = maPosition;
			aSize = maSize;
		}
		
        uno::Reference< beans::XPropertySet > xProps( xFrame, uno::UNO_QUERY_THROW );

        // The size
        xProps->setPropertyValue( OUString::createFromAscii( "SizeType" ), uno::Any( text::SizeType::FIX ) );
        xProps->setPropertyValue( OUString::createFromAscii( "FrameIsAutomaticHeight" ), uno::Any( sal_False ) );
        xProps->setPropertyValue( OUString::createFromAscii( "Height" ), uno::Any( aSize.Height ) );
        xProps->setPropertyValue( OUString::createFromAscii( "Width" ), uno::Any( aSize.Width ) );

        // The position
        xProps->setPropertyValue( OUString::createFromAscii( "HoriOrientPosition" ), uno::Any( aPosition.X ) );
        xProps->setPropertyValue( OUString::createFromAscii( "HoriOrientRelation" ), 
                uno::Any( text::RelOrientation::FRAME ) );
        xProps->setPropertyValue( OUString::createFromAscii( "HoriOrient" ), 
                uno::Any( text::HoriOrientation::NONE ) );

        xProps->setPropertyValue( OUString::createFromAscii( "VertOrientPosition" ), uno::Any( aPosition.Y ) );
        xProps->setPropertyValue( OUString::createFromAscii( "VertOrientRelation" ), 
                uno::Any( text::RelOrientation::FRAME ) );
        xProps->setPropertyValue( OUString::createFromAscii( "VertOrient" ), 
                uno::Any( text::VertOrientation::NONE ) );


        // Anchor the frame into the document
        uno::Reference< text::XTextDocument > xDoc( rFilterBase.getModel( ), uno::UNO_QUERY_THROW );
        uno::Reference< text::XTextContent > xCtnt( xShape, uno::UNO_QUERY_THROW );
        xCtnt->attach( xDoc->getText( )->getStart( ) );
			
        mxShape = xShape;
    }
    catch ( uno::Exception& e )
    {
#if DEBUG
        clog << "Exception during TextFrame creation: ";
        clog << rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr( );
        clog << endl;
#endif
    }

    return xShape;
}

void Shape::addChilds( const ::oox::core::XmlFilterBase& rFilterBase, const ::oox::vml::Drawing& rDrawing,
		const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
			const awt::Rectangle& rClientRect )
{
    sal_Int32 nGlobalLeft  = SAL_MAX_INT32;
    sal_Int32 nGlobalRight = SAL_MIN_INT32;
    sal_Int32 nGlobalTop   = SAL_MAX_INT32;
    sal_Int32 nGlobalBottom= SAL_MIN_INT32;
	std::vector< ShapePtr >::iterator aIter( maChilds.begin() );
	while( aIter != maChilds.end() )
	{
		sal_Int32 l = (*aIter)->maPosition.X;
		sal_Int32 t = (*aIter)->maPosition.Y;
		sal_Int32 r = l + (*aIter)->maSize.Width;
		sal_Int32 b = t + (*aIter)->maSize.Height;
		if ( nGlobalLeft > l )
			nGlobalLeft = l;
		if ( nGlobalRight < r )
			nGlobalRight = r;
		if ( nGlobalTop > t )
			nGlobalTop = t;
		if ( nGlobalBottom < b )
			nGlobalBottom = b;
		aIter++;
	}
	aIter = maChilds.begin();
	while( aIter != maChilds.end() )
	{
		awt::Rectangle aShapeRect;
		awt::Rectangle* pShapeRect = 0;
        if ( ( nGlobalLeft != SAL_MAX_INT32 ) && ( nGlobalRight != SAL_MIN_INT32 ) && ( nGlobalTop != SAL_MAX_INT32 ) && ( nGlobalBottom != SAL_MIN_INT32 ) )
		{
			sal_Int32 nGlobalWidth = nGlobalRight - nGlobalLeft;
			sal_Int32 nGlobalHeight = nGlobalBottom - nGlobalTop;
			if ( nGlobalWidth && nGlobalHeight )
			{
				double fWidth = (*aIter)->maSize.Width;
				double fHeight= (*aIter)->maSize.Height;
				double fXScale = (double)rClientRect.Width / (double)nGlobalWidth;
				double fYScale = (double)rClientRect.Height / (double)nGlobalHeight;
				aShapeRect.X = static_cast< sal_Int32 >( ( ( (*aIter)->maPosition.X - nGlobalLeft ) * fXScale ) + rClientRect.X );
				aShapeRect.Y = static_cast< sal_Int32 >( ( ( (*aIter)->maPosition.Y - nGlobalTop  ) * fYScale ) + rClientRect.Y );
				fWidth *= fXScale;
				fHeight *= fYScale;
				aShapeRect.Width = static_cast< sal_Int32 >( fWidth );
				aShapeRect.Height = static_cast< sal_Int32 >( fHeight );
				pShapeRect = &aShapeRect;
			}
		}
        (*aIter++)->addShape( rFilterBase, rDrawing, rxShapes, pShapeRect );
	}
}

void Shape::addShape( const ::oox::core::XmlFilterBase& rFilterBase, const ::oox::vml::Drawing& rDrawing,
		const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
			const awt::Rectangle* pShapeRect )
{
	oox::vml::Shape aShape( msServiceName );
	if ( msType.getLength() )
	{
		std::vector< ShapePtr >& rShapeTypes = const_cast< ::oox::vml::Drawing& >( rDrawing ).getShapeTypes();
		std::vector< ShapePtr >::const_iterator aShapeTypeIter( rShapeTypes.begin() );
		while( aShapeTypeIter != rShapeTypes.end() )
		{
			if ( (*aShapeTypeIter)->msType == aShape.msType )
			{
				aShape.applyAttributes( *(*aShapeTypeIter).get() );
				break;
			}
			aShapeTypeIter++;
		}
	}
	aShape.applyAttributes( *this );

	// creating XShape
    static const OUString sRectService( OUString::createFromAscii( "com.sun.star.drawing.RectangleShape" ) );
    bool bIsRectangle = msServiceName.equals( sRectService );

    uno::Reference< drawing::XShape > xShape;
    if ( mbContainsText && bIsRectangle )
        xShape.set( createAndInsertFrame( rFilterBase, aShape, rxShapes, pShapeRect ) );
    else
    	xShape.set( createAndInsert( rFilterBase, aShape, rxShapes, pShapeRect ) );

	// creating GroupShape if possible
	uno::Reference< drawing::XShapes > xShapes( xShape, uno::UNO_QUERY );
	if ( xShapes.is() )
	{
		awt::Rectangle aChildRect;
		if ( pShapeRect )
			aChildRect = *pShapeRect;
		else
		{
			aChildRect.X = maPosition.X;
			aChildRect.Y = maPosition.Y;
			aChildRect.Width = maSize.Width;
			aChildRect.Height = maSize.Height;
		}
		addChilds( rFilterBase, rDrawing, xShapes, aChildRect );
	}
}

void Shape::updateShape( )
{
    // TODO Apply the shape attributes to the already added shape
}

} }
