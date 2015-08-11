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

#include "oox/vml/drawingfragmenthandler.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/core/contexthandler.hxx"
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

#if DEBUG
#include <iostream>
using namespace std;
#endif


using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;

namespace oox { namespace vml {

static sal_Int32 getColor( const rtl::OUString& rColor )
{
    sal_Int32 nId = 0;
    rtl::OUString sValue ( rColor.getToken( 0, ' ', nId ) );
    
    static const rtl::OUString aColorNames[] =
    {
        rtl::OUString::createFromAscii( "Black" ),
        rtl::OUString::createFromAscii( "Silver" ),
        rtl::OUString::createFromAscii( "Gray" ),
        rtl::OUString::createFromAscii( "White" ),
        rtl::OUString::createFromAscii( "Maroon" ),
        rtl::OUString::createFromAscii( "Red" ),
        rtl::OUString::createFromAscii( "Purple" ),
        rtl::OUString::createFromAscii( "Fuchsia" ),
        rtl::OUString::createFromAscii( "Green" ),
        rtl::OUString::createFromAscii( "Lime" ),
        rtl::OUString::createFromAscii( "Olive" ),
        rtl::OUString::createFromAscii( "Yellow" ),
        rtl::OUString::createFromAscii( "Navy" ),
        rtl::OUString::createFromAscii( "Blue" ),
        rtl::OUString::createFromAscii( "Teal" ),
        rtl::OUString::createFromAscii( "Aqua" )
    };

    static const rtl::OUString aColorValues[] =
    {
        rtl::OUString::createFromAscii( "000000" ),
        rtl::OUString::createFromAscii( "C0C0C0" ),
        rtl::OUString::createFromAscii( "808080" ),
        rtl::OUString::createFromAscii( "FFFFFF" ),
        rtl::OUString::createFromAscii( "800000" ),
        rtl::OUString::createFromAscii( "FF0000" ),
        rtl::OUString::createFromAscii( "800080" ),
        rtl::OUString::createFromAscii( "FF00FF" ),
        rtl::OUString::createFromAscii( "008000" ),
        rtl::OUString::createFromAscii( "00FF00" ),
        rtl::OUString::createFromAscii( "808000" ),
        rtl::OUString::createFromAscii( "FFFF00" ),
        rtl::OUString::createFromAscii( "000080" ),
        rtl::OUString::createFromAscii( "0000FF" ),
        rtl::OUString::createFromAscii( "008080" ),
        rtl::OUString::createFromAscii( "00FFFF" )
    };

    rtl::OUString sHexValue;
    if ( sValue.compareToAscii( "#", 1 ) == 0 )
    {
        // Removes the # of the hexa value if needed
        sHexValue = sValue.copy( 1 );

        // Check if the color is 6 or 3 chars long
        if ( sHexValue.getLength( ) == 3 )
        {
            // All the characters have to be doubled
            OUString sRed = sHexValue.copy( 0, 1 );
            OUString sGreen = sHexValue.copy( 1, 1 );
            OUString sBlue = sHexValue.copy( 2, 1 );

            sHexValue = sRed + sRed + sGreen + sGreen + sGreen + sBlue  + sBlue;
        }
    }
    else
    {
        // Convert the color name into an hexa value
        int i = 0;
        while ( sHexValue.getLength( ) == 0 && i < 16 )
        {
            if ( sValue.equalsIgnoreAsciiCase( aColorNames[i] ) )
            {
                sHexValue = aColorValues[i];
            }
            i++;
        }
    }

    return sHexValue.toInt32( sal_Int16( 16 ) );
}

static sal_Int32 getMeasure( const rtl::OUString& rVal )
{
	double fVal = rVal.toDouble();
	const sal_Int32 nLen = rVal.getLength();
	if ( nLen >= 2 )
	{
		switch( static_cast< sal_Int8 >( rVal[ nLen - 1 ] ) )
		{
			case 'n' : fVal *= 2540; break;
			case 'm' : rVal[ nLen - 2 ] == 'm' ? fVal *= 100.0 : fVal *= 1000.0; break;
			case 't' : fVal *= 2540.0 / 72.0; break;
			case 'c' : fVal *= ( 2540.0 / 72.0 ) * 12.0; break;
		}
	}
	return static_cast< sal_Int32 >( fVal );
}

static OUString getRelId( const Reference< XFastAttributeList >& xAttribs )
{
    static const sal_Int32 aTokens[] = 
    {
        NMSP_RELATIONSHIPS | XML_id,
        NMSP_OFFICE | XML_relid,
        NMSP_RELATIONSHIPS | XML_href
    };

    // Look for the relation id
    OUString sId;

    int i = 0;
    while ( ( i < 3 ) && ( sId.getLength( ) == 0 ) )
    {
        OUString sValue( xAttribs->getOptionalValue( aTokens[i] ) );
        if ( sValue.getLength( ) > 0 )
            sId = sValue;
        i++;
    }

    return sId;
}

// AG_CoreAttributes
static void ApplyCoreAttributes( const Reference< XFastAttributeList >& xAttribs, Shape& rShape )
{
	// AG_Id
	rShape.msId = xAttribs->getOptionalValue( XML_id );
	rShape.msSpId = xAttribs->getOptionalValue( NMSP_OFFICE|XML_spid );

	// AG_Style
	if ( xAttribs->hasAttribute( XML_style ) )
	{
		rtl::OUString sStyle( xAttribs->getOptionalValue( XML_style ) );
        sal_Int32 nIndex = 0;
        do
        {
			OUString aStyleToken( sStyle.getToken( 0, ';', nIndex ) );
			if ( aStyleToken.getLength() )
			{
				sal_Int32 nIndex2 = 0;
				OUString aName( aStyleToken.getToken( 0, ':', nIndex2 ) );
				OUString aVal ( aStyleToken.getToken( 0, ':', nIndex2 ) );
				if ( aName.getLength() && aVal.getLength() )
				{
					static const ::rtl::OUString sPosition( RTL_CONSTASCII_USTRINGPARAM( "position" ) );
					static const ::rtl::OUString sLeft( RTL_CONSTASCII_USTRINGPARAM( "left" ) );
					static const ::rtl::OUString sTop( RTL_CONSTASCII_USTRINGPARAM( "top" ) );
					static const ::rtl::OUString sWidth( RTL_CONSTASCII_USTRINGPARAM( "width" ) );
					static const ::rtl::OUString sHeight( RTL_CONSTASCII_USTRINGPARAM( "height" ) );
					static const ::rtl::OUString sMarginLeft( RTL_CONSTASCII_USTRINGPARAM( "margin-left" ) );
					static const ::rtl::OUString sMarginTop( RTL_CONSTASCII_USTRINGPARAM( "margin-top" ) );
					static const ::rtl::OUString sZIndex( RTL_CONSTASCII_USTRINGPARAM( "z-index" ) );
					if ( aName == sPosition )
						rShape.msPosition = aVal;
					else if ( aName == sLeft )
						rShape.maPosition.X = getMeasure( aVal );
					else if ( aName == sTop )
						rShape.maPosition.Y = getMeasure( aVal );
					else if ( aName == sWidth )
						rShape.maSize.Width = getMeasure( aVal );
					else if ( aName == sHeight )
						rShape.maSize.Height = getMeasure( aVal );
					else if ( aName == sMarginLeft )
						rShape.maPosition.X = getMeasure( aVal );
					else if ( aName == sMarginTop )
						rShape.maPosition.Y = getMeasure( aVal );
                    else if ( aName == sZIndex )
                        rShape.mnZOrder = aVal.toInt32( );
				}
			}
        }
        while ( nIndex >= 0 );
	}
	
	// href
	// target
	// class
	// title
	// alt

	// coordsize
	rtl::OUString aCoordSize( xAttribs->getOptionalValue( XML_coordsize ) );
	if ( aCoordSize.getLength() )
	{
		sal_Int32 nIndex = 0;
		rtl::OUString aCoordWidth ( aCoordSize.getToken( 0, ',', nIndex ) );
		rtl::OUString aCoordHeight( aCoordSize.getToken( 0, ',', nIndex ) );
		if ( aCoordWidth.getLength() )
			rShape.mnCoordWidth = aCoordWidth.toInt32();
		if ( aCoordHeight.getLength() )
			rShape.mnCoordHeight = aCoordHeight.toInt32();
	}

	// coordorigin
	// wrapcoords
	// print
}

// AG_ShapeAttributes
static void ApplyShapeAttributes( const Reference< XFastAttributeList >& xAttribs, Shape& rShape )
{
	AttributeList aAttributeList( xAttribs );
	rShape.mnStroked = xAttribs->getOptionalValueToken( XML_stroked, 0 );
	if ( xAttribs->hasAttribute( XML_filled ) )
		rShape.moFilled = ::boost::optional< sal_Bool >( aAttributeList.getBool( XML_filled, sal_False ) );

	if ( xAttribs->hasAttribute( XML_fillcolor ) )
    {
		rShape.moFillColor = ::boost::optional< sal_Int32 >( getColor( xAttribs->getOptionalValue( XML_fillcolor ) ) );
        rShape.moFillType = ::boost::optional< sal_Int32 >( drawing::FillStyle_SOLID );
    }

    if ( xAttribs->hasAttribute( XML_strokecolor ) )
        rShape.moStrokeColor = ::boost::optional< sal_Int32 >( getColor( xAttribs->getOptionalValue( XML_strokecolor ) ) );
    else
        rShape.moStrokeColor = ::boost::optional< sal_Int32 >( 0 );
    
    if ( xAttribs->hasAttribute( XML_strokeweight ) )
        rShape.moStrokeWeight = ::boost::optional< sal_Int32 >( getMeasure( xAttribs->getOptionalValue( XML_strokeweight ) ) );
    else
        rShape.moStrokeColor = ::boost::optional< sal_Int32 >( getMeasure( OUString::createFromAscii( "1pt" ) ) );
}

//--------------------------------------------------------------------------------------------------------------
// EG_ShapeElements
class BasicShapeContext : public ContextHandler
{
public:
    BasicShapeContext( ContextHandler& rParent,
		sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, Shape& rShape );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 Element,
		const Reference< XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, RuntimeException);
protected:
	Shape& mrShape;
};

BasicShapeContext::BasicShapeContext( ContextHandler& rParent,
	sal_Int32 /* aElement */, const Reference< XFastAttributeList >& xAttribs, Shape& rShape )
: ContextHandler( rParent )
, mrShape( rShape )
{
	mrShape.msType = xAttribs->getOptionalValue( XML_type );
	mrShape.msShapeType = xAttribs->getOptionalValue( NMSP_OFFICE|XML_spt );
}
Reference< XFastContextHandler > BasicShapeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
	throw (SAXException, RuntimeException)
{
	Reference< XFastContextHandler > xRet;
	switch( aElementToken )
	{
		case NMSP_VML|XML_imagedata:
			{
#ifdef NO_LIBO_4_0_VML_IMAGEDATA_FIX
                // A picture in a custom shape isn't shown
                if ( mrShape.msServiceName.equalsAscii( "com.sun.star.drawing.CustomShape" ) )
                    mrShape.msServiceName = OUString::createFromAscii( "com.sun.star.drawing.RectangleShape" );
#else	// NO_LIBO_4_0_VML_IMAGEDATA_FIX
                // Fix crashing bug reported in the following NeoOffice forum
                // topic by not changing the service name:
                // http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8547
#endif	// NO_LIBO_4_0_VML_IMAGEDATA_FIX

                OUString aRelId( getRelId( xAttribs ) );
                OUString sUrl( getFragmentPathFromRelId( aRelId ) );
                mrShape.moFillImageUrl = ::boost::optional< OUString >( sUrl );
                mrShape.moFillType = ::boost::optional< sal_Int32 > ( drawing::FillStyle_BITMAP );
				mrShape.msImageTitle = xAttribs->getOptionalValue( NMSP_OFFICE|XML_title );
			}
			break;
        case NMSP_VML|XML_textbox:
            {
                mrShape.mbContainsText = true;
            }
            break;
		default:
			break;
	}
	if( !xRet.is() )
		xRet.set( this );
	return xRet;
}

//--------------------------------------------------------------------------------------------------------------
// CT_Shapetype
class ShapeTypeContext : public BasicShapeContext
{
public:
    ShapeTypeContext( ContextHandler& rParent,
		sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, Shape& rShape );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 Element,
		const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);
};

ShapeTypeContext::ShapeTypeContext( ContextHandler& rParent,
	sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, Shape& rShape )
: BasicShapeContext( rParent, aElement, xAttribs, rShape )
{
}

Reference< XFastContextHandler > ShapeTypeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
	throw (SAXException, RuntimeException)
{
	Reference< XFastContextHandler > xRet;
//	switch( aElementToken )
//	{
//		default:
			xRet = BasicShapeContext::createFastChildContext( aElementToken, xAttribs );
//		break;
//	}
	if( !xRet.is() )
		xRet.set( this );
	return xRet;
}
//--------------------------------------------------------------------------------------------------------------
// CT_PolyLine
class PolyLineContext : public BasicShapeContext
{
public:
    PolyLineContext( ContextHandler& rParent,
		sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, Shape& rShape );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 Element,
		const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);
};

PolyLineContext::PolyLineContext( ContextHandler& rParent,
	sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, Shape& rShape )
: BasicShapeContext( rParent, aElement, xAttribs, rShape )
{
	ApplyShapeAttributes( xAttribs, rShape );
	ApplyCoreAttributes( xAttribs, rShape );

	rtl::OUString aPoints( xAttribs->getOptionalValue( XML_points ) );
	if ( aPoints.getLength() )
	{
		std::vector< awt::Point > vPoints;
		sal_Int32 nIndex = 0;
		do
		{
			OUString aX( aPoints.getToken( 0, ',', nIndex ) );
			OUString aY( aPoints.getToken( 0, ',', nIndex ) );
			awt::Point aPt( getMeasure( aX ), getMeasure( aY ) );
			vPoints.push_back( aPt );
		}
		while ( nIndex >= 0 );

		drawing::PointSequenceSequence aPointSeq( 1 );
		aPointSeq[ 0 ] = drawing::PointSequence( &vPoints.front(), vPoints.size() );
		static const ::rtl::OUString sPolyPolygon( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygon" ) );
		rShape.maPath.Name = sPolyPolygon;
		rShape.maPath.Value <<= aPointSeq;

/* not sure if the following is needed

		// calculating the bounding box
		sal_Int32 nGlobalLeft  = SAL_MAX_INT32;
		sal_Int32 nGlobalRight = SAL_MIN_INT32;
		sal_Int32 nGlobalTop   = SAL_MAX_INT32;
		sal_Int32 nGlobalBottom= SAL_MIN_INT32;
		std::vector< awt::Point >::const_iterator aIter( vPoints.begin() );
		while( aIter != vPoints.end() )
		{
			sal_Int32 x = (*aIter).X;
			sal_Int32 y = (*aIter).Y;
			if ( nGlobalLeft > x )
				nGlobalLeft = x;
			if ( nGlobalRight < x )
				nGlobalRight = x;
			if ( nGlobalTop > y )
				nGlobalTop = y;
			if ( nGlobalBottom < y )
				nGlobalBottom = y;
			aIter++;
		}
		rShape.maPosition.X = nGlobalLeft;
		rShape.maPosition.Y = nGlobalTop;
		rShape.maSize.Width = nGlobalRight - nGlobalLeft;
		rShape.maSize.Height = nGlobalBottom - nGlobalTop;
*/
	}
}

Reference< XFastContextHandler > PolyLineContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
	throw (SAXException, RuntimeException)
{
	Reference< XFastContextHandler > xRet;
//	switch( aElementToken )
//	{
//		default:
			xRet = BasicShapeContext::createFastChildContext( aElementToken, xAttribs );
//		break;
//	}
	if( !xRet.is() )
		xRet.set( this );
	return xRet;
}

//--------------------------------------------------------------------------------------------------------------
// CT_Shape
class ShapeContext : public BasicShapeContext
{
public:
    ShapeContext( ContextHandler& rParent,
		sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, Shape& rShape );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 Element,
		const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException);
    void ApplyFillAttributes( const Reference< XFastAttributeList >& xAttribs );
};

ShapeContext::ShapeContext( ContextHandler& rParent,
	sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs, Shape& rShape )
: BasicShapeContext( rParent, aElement, xAttribs, rShape )
{
	ApplyShapeAttributes( xAttribs, rShape );
	ApplyCoreAttributes( xAttribs, rShape );
//	rShape.msPath = xAttribs->getOptionalValue( XML_path );
}

Reference< XFastContextHandler > ShapeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
	throw (SAXException, RuntimeException)
{
	Reference< XFastContextHandler > xRet;
	switch( aElementToken )
	{
        case NMSP_VML|XML_fill:
            {
                ApplyFillAttributes( xAttribs );
            }
            break;
		default:
			xRet = BasicShapeContext::createFastChildContext( aElementToken, xAttribs );
		break;
	}
	if( !xRet.is() )
		xRet.set( this );
	return xRet;
}

void ShapeContext::ApplyFillAttributes( const Reference< XFastAttributeList >& xAttribs )
{
    // Fill type
    if ( xAttribs->hasAttribute( XML_type ) )
    {
       OUString sType = xAttribs->getValue( XML_type );

       if ( sType.equalsIgnoreAsciiCaseAscii( "frame" ) ||
         sType.equalsIgnoreAsciiCaseAscii( "tile" ) )
       {
           mrShape.moFillType = ::boost::optional< sal_Int32 >( drawing::FillStyle_BITMAP );
           mrShape.moFilled = ::boost::optional< sal_Bool >( sal_True );

           // TODO Its a picture
           if ( sType.equalsIgnoreAsciiCaseAscii( "frame" ) )
               mrShape.meFillImageMode = com::sun::star::drawing::BitmapMode_STRETCH;
           else
               mrShape.meFillImageMode = com::sun::star::drawing::BitmapMode_REPEAT;
           
           if ( xAttribs->hasAttribute( NMSP_RELATIONSHIPS|XML_id ) )
           {
               OUString aRelId( getRelId( xAttribs ) );
               OUString sUrl = getFragmentPathFromRelId( aRelId );
               mrShape.moFillImageUrl = ::boost::optional< OUString >( sUrl );
           }
       }
       else if ( sType.equalsIgnoreAsciiCaseAscii( "gradient" ) || 
               sType.equalsIgnoreAsciiCaseAscii( "grandientradial" ) )
       {
           // TODO Its a gradient
           mrShape.moFillType = ::boost::optional< sal_Int32 >( drawing::FillStyle_GRADIENT );
       }
       else if ( sType.equalsIgnoreAsciiCaseAscii( "solid" ) )
       {
           // TODO Its a solid
           mrShape.moFillType = ::boost::optional< sal_Int32 >( drawing::FillStyle_SOLID );
       }
       else if ( sType.equalsIgnoreAsciiCaseAscii( "pattern" ) )
       {
           // TODO Its a Hatch
           mrShape.moFillType = ::boost::optional< sal_Int32 >( drawing::FillStyle_HATCH );
       }
    }
}


//--------------------------------------------------------------------------------------------------------------

class GroupShapeContext : public BasicShapeContext
{
public:
    GroupShapeContext( ContextHandler& rParent, sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs,
		Shape& rShape, std::vector< ShapePtr >& rShapeTypes );
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 Element,
		const Reference< XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, RuntimeException);
private:
	Shape& mrShape;
	std::vector< ShapePtr >& mrShapeTypes;
};

GroupShapeContext::GroupShapeContext( ContextHandler& rParent,
	sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs,
		Shape& rShape, std::vector< ShapePtr >& rShapeTypes )
: BasicShapeContext( rParent, aElement, xAttribs, rShape )
, mrShape( rShape )
, mrShapeTypes( rShapeTypes )
{
	AttributeList aAttributeList( xAttribs );
	if ( xAttribs->hasAttribute( XML_filled ) )
		rShape.moFilled = ::boost::optional< sal_Int32>( aAttributeList.getBool( XML_filled, sal_False ) );
	if ( xAttribs->hasAttribute( XML_fillcolor ) )
		rShape.moFillColor = ::boost::optional< sal_Int32 >( getColor( xAttribs->getOptionalValue( XML_fillcolor ) ) );
	ApplyCoreAttributes( xAttribs, rShape );
}
Reference< XFastContextHandler > GroupShapeContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
	throw (SAXException, RuntimeException)
{
	return DrawingFragmentHandler::StaticCreateContext( *this, aElementToken, xAttribs, mrShape.getChilds(), mrShapeTypes );
}

//--------------------------------------------------------------------------------------------------------------

DrawingFragmentHandler::DrawingFragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath,
												std::vector< ShapePtr >& rShapes, std::vector< ShapePtr >& rShapeTypes )
	throw()
: FragmentHandler( rFilter, rFragmentPath )
, mrShapes( rShapes )
, mrShapeTypes( rShapeTypes )
, maFragmentPath( rFragmentPath )
{
}
DrawingFragmentHandler::~DrawingFragmentHandler()
	throw()
{
}

::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > DrawingFragmentHandler::StaticCreateContext( oox::core::ContextHandler& rParent,
    sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs,
		std::vector< ShapePtr >& rShapes, std::vector< ShapePtr >& rShapeTypes )
		throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
#if DEBUG
    clog << "DrawingFragmentHandler::StaticCreateContext" << endl;
#endif

	static const ::rtl::OUString sCustomShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.CustomShape" ) );

	Reference< XFastContextHandler > xRet;
	switch( aElementToken )
	{
		case NMSP_VML|XML_group :
			{
				static const ::rtl::OUString sGroupShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GroupShape" ) );
				ShapePtr pShapePtr( new Shape( sGroupShape ) );
				xRet = new GroupShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get(), rShapeTypes );
				rShapes.push_back( pShapePtr );
			}
		break;
		case NMSP_VML|XML_shapetype :
			{
				ShapePtr pShapePtr( new Shape( OUString() ) );
                xRet = new ShapeTypeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
				rShapeTypes.push_back( pShapePtr );
			}
		break;
		case NMSP_VML|XML_shape:
			{
				ShapePtr pShapePtr( new Shape( sCustomShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
				rShapes.push_back( pShapePtr );
			}
		break;
		case NMSP_VML|XML_oval:
			{
				static const ::rtl::OUString sEllipseShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.EllipseShape" ) );
				ShapePtr pShapePtr( new Shape( sEllipseShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
				rShapes.push_back( pShapePtr );
			}
		break;
		case NMSP_VML|XML_polyline:
			{
				static const ::rtl::OUString sPolyLineShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.PolyLineShape" ) );
				ShapePtr pShapePtr( new Shape( sPolyLineShape ) );
                xRet = new PolyLineContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
				rShapes.push_back( pShapePtr );
			}
		break;
		case NMSP_VML|XML_rect:
			{
				static const ::rtl::OUString sRectangleShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.RectangleShape" ) );
				ShapePtr pShapePtr( new Shape( sRectangleShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
				rShapes.push_back( pShapePtr );
			}
		break;
		case NMSP_VML|XML_roundrect:
			{
				static const ::rtl::OUString sRectangleShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.RectangleShape" ) );
				ShapePtr pShapePtr( new Shape( sRectangleShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
				rShapes.push_back( pShapePtr );
			}
		break;

		// TODO:
		case NMSP_VML|XML_arc:
			{
				ShapePtr pShapePtr( new Shape( sCustomShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
				rShapes.push_back( pShapePtr );
			}
		break;
		case NMSP_VML|XML_curve:
			{
				ShapePtr pShapePtr( new Shape( sCustomShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
				rShapes.push_back( pShapePtr );
			}
		break;
		case NMSP_VML|XML_line:
			{
				ShapePtr pShapePtr( new Shape( sCustomShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
				rShapes.push_back( pShapePtr );
			}
		break;
		case NMSP_OFFICE|XML_diagram:
			{
				ShapePtr pShapePtr( new Shape( sCustomShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
				rShapes.push_back( pShapePtr );
			}
		break;
		case NMSP_VML|XML_image:
			{
				ShapePtr pShapePtr( new Shape( sCustomShape ) );
                xRet = new ShapeContext( rParent, aElementToken, xAttribs, *pShapePtr.get() );
				rShapes.push_back( pShapePtr );
			}
		break;
	}
	return xRet;
}


// CT_GROUP
Reference< XFastContextHandler > DrawingFragmentHandler::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs )
	throw (SAXException, RuntimeException)
{
#if DEBUG
    clog << "DrawingFragmentHandler::createFastChildContext" << endl;
#endif
	return aElementToken == XML_xml
		? getFastContextHandler()
		: StaticCreateContext( *this, aElementToken, xAttribs, mrShapes, mrShapeTypes );
}

void SAL_CALL DrawingFragmentHandler::endDocument()
	throw (SAXException, RuntimeException)
{
}

//--------------------------------------------------------------------------------------------------------------



} }

