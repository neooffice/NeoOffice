/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 * This file incorporates work covered by the following license notice:
 * 
 *   Modified March 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 4
 *   of the Apache License, Version 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *************************************************************/



#include "oox/drawingml/table/tablecell.hxx"
#include "oox/drawingml/table/tableproperties.hxx"
#include "oox/drawingml/shapepropertymap.hxx"
#include "oox/drawingml/textbody.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/propertyset.hxx"
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/WritingMode.hpp>

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using ::com::sun::star::table::BorderLine;
using ::com::sun::star::drawing::LineStyle;

namespace oox { namespace drawingml { namespace table {

TableCell::TableCell()
#ifdef NO_REDHAT_BUG_842552_FIX
: mnRowSpan ( 1 )
#else	// NO_REDHAT_BUG_842552_FIX
: mpTextBody( new TextBody() )
, mnRowSpan ( 1 )
#endif	// NO_REDHAT_BUG_842552_FIX
, mnGridSpan( 1 )
, mbhMerge( sal_False )
, mbvMerge( sal_False )
, mnMarL( 91440 )
, mnMarR( 91440 )
, mnMarT( 45720 )
, mnMarB( 45720 )
, mnVertToken( XML_horz )
, mnAnchorToken( XML_t )
, mbAnchorCtr( sal_False )
, mnHorzOverflowToken( XML_clip )
{
}
TableCell::~TableCell()
{
}

void applyLineAttributes( const ::oox::core::XmlFilterBase& rFilterBase,
        Reference< XPropertySet >& rxPropSet, oox::drawingml::LineProperties& rLineProperties,
        sal_Int32 nPropId )
{
    BorderLine aBorderLine( 0, 0, 0, 0 );
    if( rLineProperties.maLineFill.moFillType.differsFrom( XML_noFill ) )
    {
        Color aColor = rLineProperties.maLineFill.getBestSolidColor();
        aBorderLine.Color = aColor.getColor( rFilterBase.getGraphicHelper() );
        aBorderLine.OuterLineWidth = static_cast< sal_Int16 >( GetCoordinate( rLineProperties.moLineWidth.get( 0 ) ) / 4 );
        aBorderLine.InnerLineWidth = static_cast< sal_Int16 >( GetCoordinate( rLineProperties.moLineWidth.get( 0 ) ) / 4 );
        aBorderLine.LineDistance = 0;
    }

    PropertySet aPropSet( rxPropSet );
    aPropSet.setProperty( nPropId, aBorderLine );
}

void applyBorder( TableStylePart& rTableStylePart, sal_Int32 nLineType, oox::drawingml::LineProperties& rLineProperties )
{
	std::map < sal_Int32, ::oox::drawingml::LinePropertiesPtr >& rPartLineBorders( rTableStylePart.getLineBorders() );
	std::map < sal_Int32, ::oox::drawingml::LinePropertiesPtr >::const_iterator aIter( rPartLineBorders.find( nLineType ) );
	if ( ( aIter != rPartLineBorders.end() ) && aIter->second.get() )
		rLineProperties.assignUsed( *aIter->second );
}

void applyTableStylePart( const ::oox::core::XmlFilterBase& rFilterBase, const Reference < ::com::sun::star::table::XCell >& rxCell, oox::drawingml::FillProperties& rFillProperties,
	 oox::drawingml::LineProperties& rLeftBorder,
	 oox::drawingml::LineProperties& rRightBorder,
	 oox::drawingml::LineProperties& rTopBorder,
	 oox::drawingml::LineProperties& rBottomBorder,
	 oox::drawingml::LineProperties& rTopLeftToBottomRightBorder,
	 oox::drawingml::LineProperties& rBottomLeftToTopRightBorder,
	TableStylePart& rTableStylePart )
{
	boost::shared_ptr< ::oox::drawingml::FillProperties >& rPartFillPropertiesPtr( rTableStylePart.getFillProperties() );
	if ( rPartFillPropertiesPtr.get() )
		rFillProperties.assignUsed( *rPartFillPropertiesPtr );

	applyBorder( rTableStylePart, XML_left, rLeftBorder );
	applyBorder( rTableStylePart, XML_right, rRightBorder );
	applyBorder( rTableStylePart, XML_top, rTopBorder );
	applyBorder( rTableStylePart, XML_bottom, rBottomBorder );
	applyBorder( rTableStylePart, XML_tl2br, rTopLeftToBottomRightBorder );
	applyBorder( rTableStylePart, XML_tr2bl, rBottomLeftToTopRightBorder );

    TextCharacterProperties aTextCharProps;
    aTextCharProps.maLatinFont = rTableStylePart.getLatinFont();
    aTextCharProps.maAsianFont = rTableStylePart.getAsianFont();
    aTextCharProps.maComplexFont = rTableStylePart.getComplexFont();
    aTextCharProps.maSymbolFont = rTableStylePart.getSymbolFont();
    aTextCharProps.maCharColor = rTableStylePart.getTextColor();

    PropertySet aPropSet( rxCell );
    aTextCharProps.pushToPropSet( aPropSet, rFilterBase );
}

void applyTableCellProperties( const Reference < ::com::sun::star::table::XCell >& rxCell, const TableCell& rTableCell )
{
	static const rtl::OUString	sTopBorder( RTL_CONSTASCII_USTRINGPARAM( "TextUpperDistance" ) );
	static const rtl::OUString	sBottomBorder( RTL_CONSTASCII_USTRINGPARAM( "TextLowerDistance" ) );
	static const rtl::OUString	sLeftBorder( RTL_CONSTASCII_USTRINGPARAM( "TextLeftDistance" ) );
	static const rtl::OUString	sRightBorder( RTL_CONSTASCII_USTRINGPARAM( "TextRightDistance" ) );
	static const rtl::OUString	sVerticalAdjust( RTL_CONSTASCII_USTRINGPARAM( "TextVerticalAdjust" ) );

	Reference< XPropertySet > xPropSet( rxCell, UNO_QUERY_THROW );
	xPropSet->setPropertyValue( sTopBorder, Any( static_cast< sal_Int32 >( rTableCell.getTopMargin() / 360 ) ) );
	xPropSet->setPropertyValue( sRightBorder, Any( static_cast< sal_Int32 >( rTableCell.getRightMargin() / 360 ) ) );
	xPropSet->setPropertyValue( sLeftBorder, Any( static_cast< sal_Int32 >( rTableCell.getLeftMargin() / 360 ) ) );
	xPropSet->setPropertyValue( sBottomBorder, Any( static_cast< sal_Int32 >( rTableCell.getBottomMargin() / 360 ) ) );

	drawing::TextVerticalAdjust eVA;
	switch( rTableCell.getAnchorToken() )
	{
		case XML_ctr:	eVA = drawing::TextVerticalAdjust_CENTER; break;
		case XML_b:		eVA = drawing::TextVerticalAdjust_BOTTOM; break;
		case XML_just:
		case XML_dist:
		default:
		case XML_t:		eVA = drawing::TextVerticalAdjust_TOP; break;
	}
	xPropSet->setPropertyValue( sVerticalAdjust, Any( eVA ) );
}

// save char color from tblstyle for combination later
void lcl_getCharPropFromTblStylePart(TextCharacterProperties& rDstCharProp, const TableStylePart& rSrcTblStyle)
{
    const Color& clr = const_cast<TableStylePart&>(rSrcTblStyle).getTextColor();
    if (clr.isUsed())
        rDstCharProp.maCharColor = clr;
    // TODO: there may be other similar properties from tblstyle which need combination later
}

void TableCell::pushToXCell( const ::oox::core::XmlFilterBase& rFilterBase, ::oox::drawingml::TextListStylePtr pMasterTextListStyle,
	const ::com::sun::star::uno::Reference < ::com::sun::star::table::XCell >& rxCell, const TableProperties& rTableProperties,
		const TableStyle& rTableStyle, sal_Int32 nColumn, sal_Int32 nMaxColumn, sal_Int32 nRow, sal_Int32 nMaxRow )
{
	TableStyle& rTable( const_cast< TableStyle& >( rTableStyle ) );
	TableProperties& rProperties( const_cast< TableProperties& >( rTableProperties ) );

	Reference< text::XText > xText( rxCell, UNO_QUERY_THROW );
	Reference< text::XTextCursor > xAt = xText->createTextCursor();

	applyTableCellProperties( rxCell, *this );

	Reference< XPropertySet > xPropSet( rxCell, UNO_QUERY_THROW );
	oox::drawingml::FillProperties aFillProperties;
	oox::drawingml::LineProperties aLinePropertiesLeft;
	oox::drawingml::LineProperties aLinePropertiesRight;
	oox::drawingml::LineProperties aLinePropertiesTop;
	oox::drawingml::LineProperties aLinePropertiesBottom;
	oox::drawingml::LineProperties aLinePropertiesTopLeftToBottomRight;
	oox::drawingml::LineProperties aLinePropertiesBottomLeftToTopRight;

	boost::shared_ptr< ::oox::drawingml::FillProperties >& rBackgroundFillPropertiesPtr( rTable.getBackgroundFillProperties() );
	if ( rBackgroundFillPropertiesPtr.get() )
		aFillProperties.assignUsed( *rBackgroundFillPropertiesPtr );

	applyTableStylePart( rFilterBase, rxCell, aFillProperties,
		aLinePropertiesLeft,
		aLinePropertiesRight,
		aLinePropertiesTop,
		aLinePropertiesBottom,
		aLinePropertiesTopLeftToBottomRight,
		aLinePropertiesBottomLeftToTopRight,
		rTable.getWholeTbl() );

	// get char color from tblstyle for combination later
	TextCharacterProperties aTextCharProps;
	lcl_getCharPropFromTblStylePart(aTextCharProps, rTable.getWholeTbl());

	if ( rProperties.isFirstRow() && ( nRow == 0 ) )
	{
		applyTableStylePart( rFilterBase, rxCell, aFillProperties,
			aLinePropertiesLeft,
			aLinePropertiesRight,
			aLinePropertiesTop,
			aLinePropertiesBottom,
			aLinePropertiesTopLeftToBottomRight,
			aLinePropertiesBottomLeftToTopRight,
			rTable.getFirstRow() );
       lcl_getCharPropFromTblStylePart(aTextCharProps, rTable.getFirstRow());
	}
	if ( rProperties.isLastRow() && ( nRow == nMaxRow ) )
	{
		applyTableStylePart( rFilterBase, rxCell, aFillProperties,
			aLinePropertiesLeft,
			aLinePropertiesRight,
			aLinePropertiesTop,
			aLinePropertiesBottom,
			aLinePropertiesTopLeftToBottomRight,
			aLinePropertiesBottomLeftToTopRight,
			rTable.getLastRow() );
		lcl_getCharPropFromTblStylePart(aTextCharProps, rTable.getLastRow());
	}
	if ( rProperties.isFirstCol() && ( nColumn == 0 ) )
	{
		applyTableStylePart( rFilterBase, rxCell, aFillProperties,
			aLinePropertiesLeft,
			aLinePropertiesRight,
			aLinePropertiesTop,
			aLinePropertiesBottom,
			aLinePropertiesTopLeftToBottomRight,
			aLinePropertiesBottomLeftToTopRight,
			rTable.getFirstCol() );
		lcl_getCharPropFromTblStylePart(aTextCharProps, rTable.getFirstCol());
	}
	if ( rProperties.isLastCol() && ( nColumn == nMaxColumn ) )
	{
		applyTableStylePart( rFilterBase, rxCell, aFillProperties,
			aLinePropertiesLeft,
			aLinePropertiesRight,
			aLinePropertiesTop,
			aLinePropertiesBottom,
			aLinePropertiesTopLeftToBottomRight,
			aLinePropertiesBottomLeftToTopRight,
			rTable.getLastCol() );
		lcl_getCharPropFromTblStylePart(aTextCharProps, rTable.getLastCol());
	}
	if ( rProperties.isBandRow() )
	{
		if ( ( !rProperties.isFirstRow() || ( nRow != 0 ) ) &&
			( !rProperties.isLastRow() || ( nRow != nMaxRow ) ) )
		{
			sal_Int32 nBand = nRow;
			if ( rProperties.isFirstRow() )
				nBand++;
			if ( nBand & 1 )
			{
				applyTableStylePart( rFilterBase, rxCell, aFillProperties,
					aLinePropertiesLeft,
					aLinePropertiesRight,
					aLinePropertiesTop,
					aLinePropertiesBottom,
					aLinePropertiesTopLeftToBottomRight,
					aLinePropertiesBottomLeftToTopRight,
					rTable.getBand2H() );
				lcl_getCharPropFromTblStylePart(aTextCharProps, rTable.getBand2H());
			}
			else
			{
				applyTableStylePart( rFilterBase, rxCell, aFillProperties,
					aLinePropertiesLeft,
					aLinePropertiesRight,
					aLinePropertiesTop,
					aLinePropertiesBottom,
					aLinePropertiesTopLeftToBottomRight,
					aLinePropertiesBottomLeftToTopRight,
					rTable.getBand1H() );
				lcl_getCharPropFromTblStylePart(aTextCharProps, rTable.getBand1H());
			}
		}
	}
	if ( ( nRow == 0 ) && ( nColumn == 0 ) )
	{
		applyTableStylePart( rFilterBase, rxCell, aFillProperties,
			aLinePropertiesLeft,
			aLinePropertiesRight,
			aLinePropertiesTop,
			aLinePropertiesBottom,
			aLinePropertiesTopLeftToBottomRight,
			aLinePropertiesBottomLeftToTopRight,
			rTable.getNwCell() );
		lcl_getCharPropFromTblStylePart(aTextCharProps, rTable.getNwCell());
	}
	if ( ( nRow == nMaxRow ) && ( nColumn == 0 ) )
	{
		applyTableStylePart( rFilterBase, rxCell, aFillProperties,
			aLinePropertiesLeft,
			aLinePropertiesRight,
			aLinePropertiesTop,
			aLinePropertiesBottom,
			aLinePropertiesTopLeftToBottomRight,
			aLinePropertiesBottomLeftToTopRight,
			rTable.getSwCell() );
		lcl_getCharPropFromTblStylePart(aTextCharProps, rTable.getSwCell());
	}
	if ( ( nRow == 0 ) && ( nColumn == nMaxColumn ) )
	{
		applyTableStylePart( rFilterBase, rxCell, aFillProperties,
			aLinePropertiesLeft,
			aLinePropertiesRight,
			aLinePropertiesTop,
			aLinePropertiesBottom,
			aLinePropertiesTopLeftToBottomRight,
			aLinePropertiesBottomLeftToTopRight,
			rTable.getNeCell() );
		lcl_getCharPropFromTblStylePart(aTextCharProps, rTable.getNeCell());
	}
	if ( ( nRow == nMaxColumn ) && ( nColumn == nMaxColumn ) )
	{
		applyTableStylePart( rFilterBase, rxCell, aFillProperties,
			aLinePropertiesLeft,
			aLinePropertiesRight,
			aLinePropertiesTop,
			aLinePropertiesBottom,
			aLinePropertiesTopLeftToBottomRight,
			aLinePropertiesBottomLeftToTopRight,
			rTable.getSeCell() );
		lcl_getCharPropFromTblStylePart(aTextCharProps, rTable.getSeCell());
	}
	if ( rProperties.isBandCol() )
	{
		if ( ( !rProperties.isFirstCol() || ( nColumn != 0 ) ) &&
			( !rProperties.isLastCol() || ( nColumn != nMaxColumn ) ) )
		{
			sal_Int32 nBand = nColumn;
			if ( rProperties.isFirstCol() )
				nBand++;
			if ( nBand & 1 )
			{
				applyTableStylePart( rFilterBase, rxCell, aFillProperties,
					aLinePropertiesLeft,
					aLinePropertiesRight,
					aLinePropertiesTop,
					aLinePropertiesBottom,
					aLinePropertiesTopLeftToBottomRight,
					aLinePropertiesBottomLeftToTopRight,
					rTable.getBand2V() );
				lcl_getCharPropFromTblStylePart(aTextCharProps, rTable.getBand2V());
			}
			else
			{
				applyTableStylePart( rFilterBase, rxCell, aFillProperties,
					aLinePropertiesLeft,
					aLinePropertiesRight,
					aLinePropertiesTop,
					aLinePropertiesBottom,
					aLinePropertiesTopLeftToBottomRight,
					aLinePropertiesBottomLeftToTopRight,
					rTable.getBand1V() );
				lcl_getCharPropFromTblStylePart(aTextCharProps, rTable.getBand1V());
			}
		}
	}

	getTextBody()->insertAt( rFilterBase, xText, xAt, aTextCharProps, pMasterTextListStyle );

	aLinePropertiesLeft.assignUsed( maLinePropertiesLeft );
    aLinePropertiesRight.assignUsed( maLinePropertiesRight );
    aLinePropertiesTop.assignUsed( maLinePropertiesTop );
    aLinePropertiesBottom.assignUsed( maLinePropertiesBottom );
    aLinePropertiesTopLeftToBottomRight.assignUsed( maLinePropertiesTopLeftToBottomRight );
    aLinePropertiesBottomLeftToTopRight.assignUsed( maLinePropertiesBottomLeftToTopRight );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesLeft, PROP_LeftBorder );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesRight, PROP_RightBorder );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesTop, PROP_TopBorder );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesBottom, PROP_BottomBorder );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesTopLeftToBottomRight, PROP_DiagonalTLBR );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesBottomLeftToTopRight, PROP_DiagonalBLTR );

	aFillProperties.assignUsed( maFillProperties );
    ShapePropertyMap aPropMap( rFilterBase.getModelObjectHelper() );
    // TODO: phClr?
    aFillProperties.pushToPropMap( aPropMap, rFilterBase.getGraphicHelper() );
    PropertySet( xPropSet ).setProperties( aPropMap );

    if ( getVertToken() == XML_eaVert )
    {
        xPropSet->setPropertyValue(::rtl::OUString::createFromAscii( "TextWritingMode" ) , Any(com::sun::star::text::WritingMode_TB_RL) );
    }
}

} } }
