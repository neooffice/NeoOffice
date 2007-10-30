/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU General Public License Version 2.1.
 *
 *
 *    GNU General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *    Modified October 2007 by Patrick Luby. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#ifndef SC_XCLEXPCHARTS_HXX
#include "XclExpCharts.hxx"
#endif

// BEGIN *** old contents from chrtdefs.hxx *** ===============================

//  Min. Abstand zum Rand des Ole-Objekts
//  (0 ist fuer Chart nochmal spezial)
#define SC_CHART_MINDIST                200

// for chart and axis titles
#define MM100_TO_PIXEL(MM100)           INT32(((MM100)*96.0+1270.0)/2540.0)

// rotation, for various records
#define EXC_CHART_ROT_CCLOCKW           0x0002      // rotate counterclockwise
#define EXC_CHART_ROT_CLOCKW            0x0003      // rotate clockwise

// (0x1062) AXCEXT
#define EXC_CHART_AXCEXT_FLAGS          0x00EF      // default format

#define EXC_CHART_TEXT_GETROTFLAGS(nF)  (((nF) & 0x0003) << 8)  // rotation flags (export)
#define EXC_CHART_TICK_GETROTFLAGS(nF)  (((nF) & 0x0003) << 2)  // rotation flags (export)

// END *** old contents from chrtdefs.hxx *** =================================



SV_IMPL_REF_LIST( ScRangeList, ScRangeList* );

#include "xcl97esc.hxx"

#include "chartarr.hxx"
#ifndef _SCH_DLL_HXX //autogen wg. SchMemChart
#include <sch/schdll.hxx>
#endif
#include <sch/memchrt.hxx>

#include "compiler.hxx"

#ifndef SC_XEFORMULA_HXX
#include "xeformula.hxx"
#endif

#ifndef _SVDOOLE2_HXX //autogen wg. SdrOle2Obj
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen wg. SdrObject
#include <svx/svdobj.hxx>
#endif
#ifndef _SV_OUTDEV_HXX //autogen wg. OutputDevice
#include <vcl/outdev.hxx>
#endif

#ifndef _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTS
#endif
#include <svtools/svstdarr.hxx>

#ifndef _COM_SUN_STAR_EMBED_XCOMPONENTSUPPLIER_HPP_
#include <com/sun/star/embed/XComponentSupplier.hpp>
#endif

// lang
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
// text
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
// awt
#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif
// drawing
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
// chart
#ifndef _COM_SUN_STAR_CHART_CHARTAXISASSIGN_HPP_
#include <com/sun/star/chart/ChartAxisAssign.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATAROWSOURCE_HPP_
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XAXISXSUPPLIER_HPP_
#include <com/sun/star/chart/XAxisXSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XAXISYSUPPLIER_HPP_
#include <com/sun/star/chart/XAxisYSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XTWOAXISYSUPPLIER_HPP_
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XAXISZSUPPLIER_HPP_
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTSOLIDTYPE_HPP_
#include <com/sun/star/chart/ChartSolidType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTLEGENDPOSITION_HPP_
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTAXISMARKS_HPP_
#include <com/sun/star/chart/ChartAxisMarks.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTSYMBOLTYPE_HPP_
#include <com/sun/star/chart/ChartSymbolType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATACAPTION_HPP_
#include <com/sun/star/chart/ChartDataCaption.hpp>
#endif
#ifndef __com_sun_star_chart_XStatisticDisplay_HPP__
#include <com/sun/star/chart/XStatisticDisplay.hpp>
#endif


using namespace ::rtl;
using namespace ::com::sun::star;


//#ifndef _VOS_NO_NAMESPACE
//using namespace vos;
//#endif


// --- little helpers ------------------------------------------------

UINT16 lcl_GetXclLineStyle( drawing::LineStyle eStyle, const drawing::LineDash& rDash, sal_Int16 nTrans )
{
    UINT16 nStyle = EXC_CHLINEFORMAT_SOLID;
	switch( eStyle )
	{
		case drawing::LineStyle_NONE:
            nStyle = EXC_CHLINEFORMAT_NONE;
		break;
		case drawing::LineStyle_SOLID:
		{
			if( nTrans < 13 )
                nStyle = EXC_CHLINEFORMAT_SOLID;
			else if( nTrans < 38 )
                nStyle = EXC_CHLINEFORMAT_DARKTRANS;
			else if( nTrans < 63 )
                nStyle = EXC_CHLINEFORMAT_MEDTRANS;
			else if( nTrans < 100 )
                nStyle = EXC_CHLINEFORMAT_LIGHTTRANS;
			else
                nStyle = EXC_CHLINEFORMAT_NONE;
		}
		break;
		case drawing::LineStyle_DASH:
		{
			if( (rDash.Dots == 1) && (rDash.Dashes == 1) )
                nStyle = EXC_CHLINEFORMAT_DASHDOT;
			else if( (rDash.Dots == 2) && (rDash.Dashes == 1) ||
					(rDash.Dots == 1) && (rDash.Dashes == 2) )
                nStyle = EXC_CHLINEFORMAT_DASHDOTDOT;
			else if( ((rDash.Dots == 0) && (rDash.DashLen < 250)) ||
					((rDash.Dashes == 0) && (rDash.DotLen < 250)) ||
					((rDash.DotLen < 250) && (rDash.DashLen < 250)) )
                nStyle = EXC_CHLINEFORMAT_DOT;
			else
                nStyle = EXC_CHLINEFORMAT_DASH;
		}
		break;
	}
	return nStyle;
}


INT16 lcl_GetXclLineWidth( sal_Int32 nWidth )
{
	if ( nWidth <= 0 )
        return EXC_CHLINEFORMAT_HAIR;
	else if ( nWidth <= 35 )
        return EXC_CHLINEFORMAT_SINGLE;
	else if ( nWidth <= 70 )
        return EXC_CHLINEFORMAT_DOUBLE;
	else
        return EXC_CHLINEFORMAT_TRIPLE;
}


UINT8 lcl_GetLegendPosition( chart::ChartLegendPosition ePos )
{
	switch( ePos )
	{
        case chart::ChartLegendPosition_LEFT:   return EXC_CHLEGEND_LEFT;
        case chart::ChartLegendPosition_TOP:    return EXC_CHLEGEND_TOP;
        case chart::ChartLegendPosition_RIGHT:  return EXC_CHLEGEND_RIGHT;
        case chart::ChartLegendPosition_BOTTOM: return EXC_CHLEGEND_BOTTOM;
	}
    return EXC_CHLEGEND_NOTDOCKED;
}


UINT8 lcl_GetMarkPos( sal_Int32 nPos )
{
    UINT8 nRet = 0;
	if( nPos & chart::ChartAxisMarks::INNER )
        nRet |= EXC_CHTICK_INSIDE;
	if( nPos & chart::ChartAxisMarks::OUTER )
        nRet |= EXC_CHTICK_OUTSIDE;
	return nRet;
}


XclExpStream& operator<<( XclExpStream& rStrm, EscherPropertyContainer& rPropSet )
{
    SvMemoryStream aMemStrm;
    rPropSet.Commit( aMemStrm );
    aMemStrm.Seek( STREAM_SEEK_TO_BEGIN );
    rStrm.CopyFromStream( aMemStrm );
    return rStrm;
}


// --- class XclChartDataFormat --------------------------------------

XclChartDataFormat::XclChartDataFormat() :
        nLineStyle( EXC_CHLINEFORMAT_NONE ),
        nLineWidth( EXC_CHLINEFORMAT_HAIR ),
		aLineColor( COL_BLACK ),
        nLineIndex( EXC_COLOR_CHWINDOWTEXT ),
        nAreaStyle( EXC_CHAREAFORMAT_NONE ),
		aAreaColor( COL_BLACK ),
        nAreaIndex( EXC_COLOR_CHWINDOWTEXT ),
		nSegment( 0 ),
		bHasSymbol( TRUE ),
        nFontx( EXC_FONT_APP ),
		aTextColor( COL_BLACK ),
        nTextIndex( EXC_COLOR_CHWINDOWTEXT ),
		nTextFlags( 0x0000 ),
		nLabelFlags( 0x0000 ),
        nDataFormatBase( EXC_CH3DDATAFORMAT_RECT ),
        nDataFormatTop( EXC_CH3DDATAFORMAT_STRAIGHT )
{
}


void XclChartDataFormat::SetLineFormat( UINT16 nStyle, INT16 nWidth, const Color& rColor, UINT16 nIndex )
{
	nLineStyle = nStyle;
	nLineWidth = nWidth;
	aLineColor.SetColor( rColor.GetColor() );
	nLineIndex = nIndex;
}


void XclChartDataFormat::SetTextFormat( UINT16 nFx, const Color& rColor, UINT16 nIndex, UINT16 nTFl, UINT16 nLFl )
{
	nFontx = nFx;
	aTextColor.SetColor( rColor.GetColor() );
	nTextIndex = nIndex;
	nTextFlags = nTFl;
	nLabelFlags = nLFl;
}


void XclChartDataFormat::SetSolidType( UINT8 nBase, UINT8 nTop )
{
	nDataFormatBase = nBase;
	nDataFormatTop = nTop;
}


UINT16 XclChartDataFormat::GetSymbol( UINT16 nSeries ) const
{
    static UINT16 nSymbols[] = { 2, 1, 3, 4, 5, 8, 9, 6, 7 };
    return bHasSymbol ? nSymbols[ nSeries % EXC_CHMARKERFORMAT_SYMBOLCOUNT ] : EXC_CHMARKERFORMAT_NOSYMBOL;
}


// --- struct XclChartText -------------------------------------------

XclChartText::XclChartText( UINT16 nSer, UINT16 nPt, const XclChartDataFormat& rFormats ) :
		nSeries( nSer ),
		nPoint( nPt ),
		nFontx( rFormats.nFontx ),
		nIndex( rFormats.nTextIndex ),
		nRGB( rFormats.aTextColor.GetColor() ),
		nFlags( rFormats.nTextFlags )
{
}


// --- class XclChartTextList ----------------------------------------

XclChartTextList::~XclChartTextList()
{
	for( XclChartText* pText = _First(); pText; pText = _Next() )
		delete pText;
}


// --- class XclObjChart ---------------------------------------------

XclObjChart::XclObjChart( const XclExpRoot& rRoot, const uno::Reference< drawing::XShape >& rShape )
			:
            XclObj( rRoot, EXC_OBJ_CMO_CHART ),
            XclExpRoot( rRoot ),
			xChartShape( rShape ),
			rChartObj( *(EscherEx::GetSdrObject( rShape )) ),
            rRootData( rRoot.GetOldRoot() ),
            rPal( rRoot.GetPalette() ),
            rFontList( rRoot.GetFontBuffer() ),
			pChartArray( NULL ),
			pStrm( NULL ),
            eChartType( EXC_CHART_UNKNOWN ),
			nMaxSegOffset( 0 ),
			nLevel( 0 ),
			nChartDataColumns( 0 ),
            nChartCategoriesDataType( EXC_CHSERIES_NUMERIC ),       // numeric
            nCurrAxesSet( EXC_CHAXESSET_PRIMARY ),
            nCurrAxis( EXC_CHAXIS_X ),
			bColHeaders( FALSE ),
			bRowHeaders( FALSE ),
			bXAxisDescr( FALSE ),
			bYAxisDescr( FALSE ),
			bChartStacked( FALSE ),
			bChartPercent( FALSE ),
			bChartLine( FALSE ),
			bChartSpline( FALSE ),
			bChart3D( FALSE ),
			bChartDeep( FALSE ),
			bStockUpDown( FALSE ),
			bStockVolume( FALSE ),
			bChartTypeRound( FALSE ),
			bChartTypeBars( FALSE ),
            bHasSecAxes( FALSE ),
			bWriteMode( FALSE )
{
	XclEscherEx* pEx = pMsodrawing->GetEscherEx();
	pEx->OpenContainer( ESCHER_SpContainer );
	pEx->AddShape( ESCHER_ShpInst_HostControl, SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT );
	EscherPropertyContainer aPropOpt;
	aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x01040104 );
	aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x00080008 );
	aPropOpt.AddOpt( ESCHER_Prop_fillColor, 0x0800004E );
	aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0x0800004D );
	aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00110010 );
	aPropOpt.AddOpt( ESCHER_Prop_lineColor, 0x0800004D );
	aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x00080008 );
	aPropOpt.AddOpt( ESCHER_Prop_fshadowObscured, 0x00020000 );
	aPropOpt.AddOpt( ESCHER_Prop_fPrint, 0x00080000 );
	aPropOpt.Commit( pEx->GetStream() );
    XclExpEscherAnchor( rRoot, rChartObj ).WriteData( *pEx );
	pEx->AddAtom( 0, ESCHER_ClientData );						// OBJ record
	pEx->CloseContainer();	// ESCHER_SpContainer
	pMsodrawing->UpdateStopPos();

	aSerieslistList.Insert( NULL, LIST_APPEND );	// one dummy on index 0

	bInitOk = InitInterface();
	DBG_ASSERT( bInitOk, "XclObjChart: init failed" );
	if ( bInitOk )
	{
		GetChartType();
		// must do it here so that EXTERNSHEET XTIs get updated before written
		BuildSeriesList();

		InitValues();
		InitChartFormats();

		delete pChartArray, pChartArray = NULL;
	}
}


XclObjChart::~XclObjChart()
{
	ULONG j, nCnt;
	SvUShorts* ps;
	nCnt = aSerieslistList.Count();
	for ( j=0, ps = (SvUShorts*) aSerieslistList.First(); j<nCnt;
			j++, ps = (SvUShorts*) aSerieslistList.Next() )
	{
		delete ps;
	}
}


//___________________________________________________________________
// get properties

BOOL XclObjChart::GetPropValue( const uno::Reference< beans::XPropertySet >& xProp, const OUString& rString )
{
	BOOL bRet = FALSE;
	if ( xProp.is() )
	{
		try
		{
			uno::Reference< beans::XPropertySetInfo > xInfo = xProp->getPropertySetInfo();
			if( xInfo.is() && xInfo->hasPropertyByName( rString ) )
			{
				aAny = xProp->getPropertyValue( rString );
				if( aAny.hasValue() )
					bRet = TRUE;
			}
		}
		catch(...)
		{
			bRet = FALSE;
		}
	}
	return bRet;
}


// get boolean, simple mode: FALSE = not found or error or property is set to FALSE
BOOL XclObjChart::GetPropBool( const uno::Reference< beans::XPropertySet >& xProp, const OUString& rString )
{
	sal_Bool bVal;
	return GetPropValue( xProp, rString ) && (aAny >>= bVal) && bVal;
}


// get boolean, precise mode: return value for success, property value in rValue
BOOL XclObjChart::GetPropBool( sal_Bool& rValue, const uno::Reference< beans::XPropertySet >& xProp,
								const OUString& rString )
{
	sal_Bool bGetVal;
	if( GetPropValue( xProp, rString ) && (aAny >>= bGetVal) )
	{
		rValue = bGetVal;
		return TRUE;
	}
	return FALSE;
}


BOOL XclObjChart::GetPropInt32( sal_Int32& rValue, const uno::Reference< beans::XPropertySet >& xProp,
								const OUString& rString )
{
	sal_Int32 nGetVal;
	if( GetPropValue( xProp, rString ) && (aAny >>= nGetVal) )
	{
		rValue = nGetVal;
		return TRUE;
	}
	return FALSE;
}


BOOL XclObjChart::GetPropInt16( sal_Int16& rValue, const uno::Reference< beans::XPropertySet >& xProp,
								const OUString& rString )
{
	sal_Int16 nGetVal;
	if( GetPropValue( xProp, rString ) && (aAny >>= nGetVal) )
	{
		rValue = nGetVal;
		return TRUE;
	}
	return FALSE;
}


BOOL XclObjChart::GetPropDouble( double& rValue, const uno::Reference< beans::XPropertySet >& xProp,
								const OUString& rString )
{
	double fGetVal;
	if( GetPropValue( xProp, rString ) && (aAny >>= fGetVal) )
	{
		rValue = fGetVal;
		return TRUE;
	}
	return FALSE;
}


void XclObjChart::GetChartType()
{
	sal_Int32 nVal;

	OUString aType( xDiagram->getDiagramType() );
    if ( aType == SERVICE_CHART_LINE )
        eChartType = EXC_CHART_LINE;
    else if ( aType == SERVICE_CHART_AREA )
        eChartType = EXC_CHART_AREA;
    else if ( aType == SERVICE_CHART_BAR )
	{
        if ( GetPropBool( xDiaProp, EXC_CHPROP_VERTICAL ) )
            eChartType = EXC_CHART_BAR;
		else
            eChartType = EXC_CHART_COLUMN;
	}
    else if ( aType == SERVICE_CHART_PIE )
        eChartType = EXC_CHART_PIE;
    else if ( aType == SERVICE_CHART_DONUT )
        eChartType = EXC_CHART_DONUT;
    else if ( aType == SERVICE_CHART_XY )
        eChartType = EXC_CHART_SCATTER;
    else if ( aType == SERVICE_CHART_NET )
        eChartType = EXC_CHART_RADARLINE;
    else if ( aType == SERVICE_CHART_STOCK )
	{
        eChartType = EXC_CHART_STOCK;
        bStockUpDown = GetPropBool( xDiaProp, EXC_CHPROP_UPDOWN );
        bStockVolume = GetPropBool( xDiaProp, EXC_CHPROP_VOLUME );
	}

    bChartPercent = GetPropBool( xDiaProp, EXC_CHPROP_PERCENT );
    bChartStacked = GetPropBool( xDiaProp, EXC_CHPROP_STACKED );

    if( (eChartType == EXC_CHART_LINE) || (eChartType == EXC_CHART_SCATTER) )
	{
        bChartLine = GetPropBool( xDiaProp, EXC_CHPROP_LINES );
        if( GetPropInt32( nVal, xDiaProp, EXC_CHPROP_SPLINETYPE ) )
			bChartSpline = (nVal != 0);
	}

    bChart3D = GetPropBool( xDiaProp, EXC_CHPROP_DIM3D );
	if( bChart3D )
        bChartDeep = GetPropBool( xDiaProp, EXC_CHPROP_DEEP );

    bChartTypeBars = (eChartType == EXC_CHART_BAR) || (eChartType == EXC_CHART_COLUMN);
    bChartTypeRound = (eChartType == EXC_CHART_PIE) || (eChartType == EXC_CHART_DONUT);
}


ColorData XclObjChart::GetColor( const uno::Reference< beans::XPropertySet >& xProp,
								const OUString& rString )
{
	sal_Int32 nRGB;
    if( GetPropValue( xProp, rString ) && (aAny >>= nRGB) )
		return COLORDATA_RGB( nRGB );		// ignore transparency
	return ColorData( 0 );
}


void XclObjChart::GetTextColor( Color& rColor, UINT16& rIndex,
								const uno::Reference< beans::XPropertySet >& xProp )
{
	sal_Int32 nRGB;
    if( !GetPropValue( xProp, EXC_CHPROP_CHARCOLOR ) || !(aAny >>= nRGB) )
        nRGB = COL_AUTO;
    rColor.SetColor( nRGB );
	if( bWriteMode )						// write mode: find color index
	{
        rIndex = rPal.GetColorIndex( rColor, EXC_COLOR_CHWINDOWTEXT );
        rColor.SetColor( rPal.GetColorData( rIndex ) );
	}
	else									// init mode: insert new color
        rPal.InsertColor( rColor, EXC_COLOR_CHARTTEXT, EXC_COLOR_CHWINDOWTEXT );
}


bool XclObjChart::CreateXclFont( XclExpFontData& rFontData, const uno::Reference< beans::XPropertySet >& xProp )
{
    if( !xProp.is() ) return false;

	float			fFloatVal;
	sal_Int16		nShortVal;
	awt::FontSlant	eItalic;
	OUString		sFontName;

    if( GetPropValue( xProp, EXC_CHPROP_CHARHEIGHT ) && (aAny >>= fFloatVal) )
        rFontData.SetApiHeight( fFloatVal );
	else
        return false;         // property not found -> default font
    if( GetPropValue( xProp, EXC_CHPROP_CHARFONTNAME ) && (aAny >>= sFontName) )
        rFontData.maName = XclTools::GetXclFontName( sFontName );
    if( GetPropValue( xProp, EXC_CHPROP_CHARPOSTURE ) && (aAny >>= eItalic) )
        rFontData.SetApiPosture( eItalic );
    if( GetPropValue( xProp, EXC_CHPROP_CHARWEIGHT ) && (aAny >>= fFloatVal) )
        rFontData.SetApiWeight( fFloatVal );
    if( GetPropValue( xProp, EXC_CHPROP_CHARUNDERL ) && (aAny >>= nShortVal) )
        rFontData.SetApiUnderline( nShortVal );
    if( GetPropValue( xProp, EXC_CHPROP_CHARFONTFAMILY ) && (aAny >>= nShortVal) )
        rFontData.SetApiFamily( nShortVal );
    if( GetPropValue( xProp, EXC_CHPROP_CHARFONTCHARSET ) && (aAny >>= nShortVal) )
        rFontData.SetApiFontEncoding( nShortVal );

    rFontData.mbStrikeout = !!GetPropBool( xProp, EXC_CHPROP_CHARCROSSEDOUT );
    rFontData.mbOutline   = !!GetPropBool( xProp, EXC_CHPROP_CHARCONTOURED );
    rFontData.mbShadow    = !!GetPropBool( xProp, EXC_CHPROP_CHARSHADOWED );

    return true;
}


UINT16 XclObjChart::GetFontx( const uno::Reference< beans::XPropertySet >& xProp )
{
    XclExpFontData aFontData;
    UINT16 nFontx = EXC_FONT_APP;

    if( CreateXclFont( aFontData, xProp ) )
	{
		if( bWriteMode )						// write mode: find font in list
            nFontx = rFontList.GetXclIndex( aFontData, EXC_FONT_APP );
		else									// init mode: add the font
            nFontx = rFontList.Insert( aFontData );
	}
	return nFontx;
}


void XclObjChart::GetTextFormat( UINT16& rFontx, Color& rColor, UINT16& rColIndex,
								const uno::Reference< beans::XPropertySet >& xProp )
{
	rFontx = GetFontx( xProp );
	GetTextColor( rColor, rColIndex, xProp );
}


void XclObjChart::GetLineformat( UINT16& rStyle, INT16& rWidth, Color& rColor, UINT16& rColIndex,
								const uno::Reference< beans::XPropertySet >& xProp )
{
	drawing::LineStyle	eStyle;
	drawing::LineDash	aDash;
	sal_Int32			nApiWidth;
	sal_Int16			nApiTrans;

    rColor.SetColor( GetColor( xProp, EXC_CHPROP_LINECOLOR ) );
    rStyle = EXC_CHLINEFORMAT_SOLID;
    rWidth = EXC_CHLINEFORMAT_HAIR;

    if( GetPropValue( xProp, EXC_CHPROP_LINESTYLE ) && (aAny >>= eStyle) &&
        GetPropValue( xProp, EXC_CHPROP_LINEDASH ) && (aAny >>= aDash) &&
        GetPropInt16( nApiTrans, xProp, EXC_CHPROP_LINETRANSPARENCE ) )
		rStyle = lcl_GetXclLineStyle( eStyle, aDash, nApiTrans );
    if( GetPropInt32( nApiWidth, xProp, EXC_CHPROP_LINEWIDTH ) )
		rWidth = lcl_GetXclLineWidth( nApiWidth );

	if( bWriteMode )
	{
		rColIndex = rPal.GetColorIndex( rColor );
        rColor.SetColor( rPal.GetColorData( rColIndex ) );
	}
    else if( rStyle != EXC_CHLINEFORMAT_NONE )
        rPal.InsertColor( rColor, EXC_COLOR_CHARTLINE );
}


void XclObjChart::GetAreaformat( UINT16& rStyle, Color& rColor, UINT16& rColIndex,
								const uno::Reference< beans::XPropertySet >& xProp )
{
    rColor.SetColor( GetColor( xProp, EXC_CHPROP_FILLCOLOR ) );
    rStyle = EXC_CHAREAFORMAT_SOLID;

	drawing::FillStyle	eStyle;
    if( GetPropValue( xProp, EXC_CHPROP_FILLSTYLE ) && (aAny >>= eStyle) )
        rStyle = (eStyle == drawing::FillStyle_NONE) ? EXC_CHAREAFORMAT_NONE : EXC_CHAREAFORMAT_SOLID;

	if( bWriteMode )
	{
		rColIndex = rPal.GetColorIndex( rColor );
        rColor.SetColor( rPal.GetColorData( rColIndex ) );
	}
    else if( rStyle != EXC_CHAREAFORMAT_NONE )
        rPal.InsertColor( rColor, EXC_COLOR_CHARTAREA );
}


void XclObjChart::RegisterEscherColor( EscherPropertyContainer& rPropSet, sal_uInt16 nPropId, XclExpColorType eColorType )
{
    sal_uInt32 nColor;
    if( rPropSet.GetOpt( nPropId, nColor ) )
    {
        // swap red and blue
        Color aColor( RGB_COLORDATA( COLORDATA_BLUE( nColor ), COLORDATA_GREEN( nColor ), COLORDATA_RED( nColor ) ) );
        if( bWriteMode )
            rPropSet.AddOpt( nPropId, 0x08000000 | rPal.GetColorIndex( aColor ) );
        else
            rPal.InsertColor( aColor, eColorType );
    }
}

EscherPropertyContainer* XclObjChart::CreateGelframe( const uno::Reference< beans::XPropertySet >& xProp )
{
    EscherPropertyContainer* pGelframe = NULL;

    drawing::FillStyle eStyle;
    if( GetPropValue( xProp, EXC_CHPROP_FILLSTYLE ) && (aAny >>= eStyle) )
    {
        switch( eStyle )
        {
            case drawing::FillStyle_GRADIENT:
                pGelframe = new EscherPropertyContainer;
            break;
            case drawing::FillStyle_HATCH:
            case drawing::FillStyle_BITMAP:
                if( bWriteMode )
                    pGelframe = new EscherPropertyContainer;
            break;
        }
    }

    if( pGelframe )
    {
        pGelframe->CreateFillProperties( xProp, sal_True );
        RegisterEscherColor( *pGelframe, ESCHER_Prop_fillColor, EXC_COLOR_CHARTAREA );
        RegisterEscherColor( *pGelframe, ESCHER_Prop_fillBackColor, EXC_COLOR_CHARTAREA );
    }

    return pGelframe;
}


BOOL XclObjChart::GetDataCaption( UINT16& rTextFlags, UINT16& rLabelFlags,
								const uno::Reference< beans::XPropertySet >& xProp )
{
	rTextFlags = rLabelFlags = 0x0000;

	sal_Int32	nCaption;
    if( !GetPropInt32( nCaption, xProp, EXC_CHPROP_DATACAPTION ) )
		return FALSE;

    bool bValue     = ::get_flag( nCaption, chart::ChartDataCaption::VALUE );
    bool bPercent   = ::get_flag( nCaption, chart::ChartDataCaption::PERCENT );
    bool bText      = ::get_flag( nCaption, chart::ChartDataCaption::TEXT );

	if( bPercent && !bChartTypeRound )
	{
		bValue = TRUE;
		bPercent = FALSE;
	}

	if( bValue )
	{
        rTextFlags |= EXC_CHTEXT_SHOWVALUE;
        rLabelFlags |= EXC_CHATTLABEL_SHOWVALUE;
	}
	else if( bPercent )
	{
        rTextFlags |= EXC_CHTEXT_SHOWPERCENT;
        rLabelFlags |= EXC_CHATTLABEL_SHOWPERCENT;
	}

	if( bText )
	{
        rTextFlags |= EXC_CHTEXT_SHOWCATEG;
        rLabelFlags |= EXC_CHATTLABEL_SHOWCATEG;
		if( bPercent )
		{
            rTextFlags |= EXC_CHTEXT_SHOWCATEGPERC;
            rLabelFlags |= EXC_CHATTLABEL_SHOWCATEGPERC;
		}
	}
	if( (bText || bValue || bPercent) && (nCaption & chart::ChartDataCaption::SYMBOL) )
        rTextFlags |= EXC_CHTEXT_SHOWSYMBOL;
	return TRUE;
}


void XclObjChart::GetFormats( XclChartDataFormat& rFormats,
							const uno::Reference< beans::XPropertySet >& xProp,
							BOOL bGetLineArea, BOOL bGetText )
{
	sal_Int32	nVal;

	// lines & areas
	if( bGetLineArea )
	{
        UINT16      nStyle, nIndex;
        INT16       nWidth;
        Color       aColor;

		GetLineformat( nStyle, nWidth, aColor, nIndex, xProp );
        if( (eChartType == EXC_CHART_SCATTER) && !bChartLine )
            nStyle = EXC_CHLINEFORMAT_NONE;
		rFormats.SetLineFormat( nStyle, nWidth, aColor, nIndex );

        GetAreaformat( rFormats.nAreaStyle, rFormats.aAreaColor, rFormats.nAreaIndex, xProp );
        rFormats.mpGelframe.reset( CreateGelframe( xProp ) );
	}

	// pie chart segments
	if( bChartTypeRound )
        if( GetPropInt32( nVal, xProp, EXC_CHPROP_SEGMENTOFFSET ) )
			rFormats.SetSegmentOffset( UINT16(nVal) );

	// line symbols
    if( GetPropInt32( nVal, xProp, EXC_CHPROP_SYMBOLTYPE ) )
		rFormats.SetSymbol( nVal != chart::ChartSymbolType::NONE );

	// data captions
	if( bGetText )
	{
		UINT16 nTextFlags, nLabelFlags;
		if( GetDataCaption( nTextFlags, nLabelFlags, xProp ) )
		{
			UINT16	nFontx, nIndex;
			Color	aColor;

			GetTextFormat( nFontx, aColor, nIndex, xProp );
			rFormats.SetTextFormat( nFontx, aColor, nIndex, nTextFlags, nLabelFlags );
		}
	}

	// 3D bar style
    if ( GetPropInt32( nVal, xProp, EXC_CHPROP_SOLIDTYPE ) )
		switch( nVal )
		{
			case chart::ChartSolidType::RECTANGULAR_SOLID:
                rFormats.SetSolidType( EXC_CH3DDATAFORMAT_RECT, EXC_CH3DDATAFORMAT_STRAIGHT );
			break;
			case chart::ChartSolidType::CYLINDER:
                rFormats.SetSolidType( EXC_CH3DDATAFORMAT_CIRC, EXC_CH3DDATAFORMAT_STRAIGHT );
			break;
			case chart::ChartSolidType::PYRAMID:
                rFormats.SetSolidType( EXC_CH3DDATAFORMAT_RECT, EXC_CH3DDATAFORMAT_SHARP );
			break;
			case chart::ChartSolidType::CONE:
                rFormats.SetSolidType( EXC_CH3DDATAFORMAT_CIRC, EXC_CH3DDATAFORMAT_SHARP );
			break;
			default:
                rFormats.SetSolidType( EXC_CH3DDATAFORMAT_RECT, EXC_CH3DDATAFORMAT_STRAIGHT );
		}
}


//___________________________________________________________________
// init, saving

void XclObjChart::InitChartFormats()
{
	GoThroughChart( FALSE );
}


void XclObjChart::Save( XclExpStream& rStrm )
{
	// content of OBJ record
	XclObj::Save( rStrm );

    // write the chart records
	ExcBofC8().Save( rStrm );
	pStrm = &rStrm;
	GoThroughChart( TRUE );
	pStrm = NULL;
	ExcEof().Save( rStrm );
}


void XclObjChart::GoThroughChart( BOOL bSetWriteMode )
{
	bWriteMode = bSetWriteMode;
	DBG_ASSERT( !bWriteMode || pStrm, "XclObjChart::GoThroughChart() - no output stream!" );

    nCurrAxesSet = EXC_CHAXESSET_PRIMARY;
    nCurrAxis = EXC_CHAXIS_X;

	if ( bInitOk )
	{
		WriteSheetHeaderSetup();
		WriteChart();

		WriteBeginLevel();
		WriteGroupFrame( xChartDoc->getArea(), FALSE );
		WriteTheSeries();
		WriteShtprops();
		WriteTheText();
		WriteAllAxes();
		WriteTheMainTitle();
		WriteTextList();
		WriteEndLevel();

		DBG_ASSERT( !nLevel, "XclObjChart::Save - missing END level!" );
		while( nLevel )
			WriteEndLevel();

		WriteTheSiindex();
	}
}


BOOL XclObjChart::InitInterface()
{
    uno::Reference < embed::XEmbeddedObject > xObj =  ((SdrOle2Obj&)rChartObj).GetObjRef();
    if ( !xObj.is() ) return FALSE;

    if ( !svt::EmbeddedObjectRef::TryRunningState(xObj) )
        return FALSE;

    xChartDoc = uno::Reference< chart::XChartDocument >( xObj->getComponent(), uno::UNO_QUERY );
	xDocProp = uno::Reference< beans::XPropertySet >( xChartDoc, uno::UNO_QUERY );
	if ( !xChartDoc.is() || !xDocProp.is() ) return FALSE;

	xDiagram = xChartDoc->getDiagram();
	xDiaProp = uno::Reference< beans::XPropertySet >( xDiagram, uno::UNO_QUERY );
	xDiagr3D = uno::Reference< chart::X3DDisplay >( xDiagram, uno::UNO_QUERY );
	if ( !xDiagram.is() || !xDiaProp.is() || !xDiagr3D.is() ) return FALSE;

    SchMemChart* pChartData = SchDLL::GetChartData( xObj );
	if ( !pChartData ) return FALSE;

    // #108357# set Calc number formatter (Chart NF may be different directly after XML import)
    pChartData->SetNumberFormatter( &rRootData.pER->GetFormatter() );
    SchDLL::Update( xObj, pChartData );
	((SdrOle2Obj&)rChartObj).GetNewReplacement();
    // get pChartArray again, Chart moves it away in SchDLL::Update
    pChartData = SchDLL::GetChartData( xObj );
    if ( !pChartData ) return FALSE;

    pChartArray = new ScChartArray( GetDocPtr(), *pChartData );

	return TRUE;
}


void XclObjChart::InitValues()
{
	// scaling factors for nearly all positions and sizes
	const awt::Size& rSize = xChartShape->getSize();
	fX4000 = 4000.0 / double( rSize.Width );
	fY4000 = 4000.0 / double( rSize.Height );

	// position and size of diagram shape (without axis titles), existence of axis description
	uno::Reference< drawing::XShape > xDiaShape( xDiagram, uno::UNO_QUERY );
	GetPosSize( aDiaPosSize, xDiaShape );

	uno::Reference< chart::XAxisXSupplier > xDiaXSup( xDiagram, uno::UNO_QUERY );
    if ( xDiaXSup.is() && GetPropBool( xDiaProp, EXC_CHPROP_HASXAXIS ) )
	{
        if( GetPropBool( xDiaProp, EXC_CHPROP_HASXAXISTIT ) )
		{
			uno::Reference< drawing::XShape > xTitleShape = xDiaXSup->getXAxisTitle();
			if( xTitleShape.is() )
			{
				XclPosSize aPosSize;
				GetPosSize( aPosSize, xTitleShape );
                if( eChartType == EXC_CHART_BAR )
				{
					aDiaPosSize.nPosX += aPosSize.nWidth;
					aDiaPosSize.nWidth -= aPosSize.nWidth;
				}
				else
					aDiaPosSize.nHeight -= aPosSize.nHeight;
			}
		}
        bXAxisDescr = GetPropBool( xDiaProp, EXC_CHPROP_HASXAXISDESCR );
	}

	uno::Reference< chart::XAxisYSupplier > xDiaYSup( xDiagram, uno::UNO_QUERY );
    if ( xDiaYSup.is() && GetPropBool( xDiaProp, EXC_CHPROP_HASYAXIS ) )
	{
        if( GetPropBool( xDiaProp, EXC_CHPROP_HASYAXISTIT ) )
		{
			uno::Reference< drawing::XShape > xTitleShape = xDiaYSup->getYAxisTitle();
			if( xTitleShape.is() )
			{
				XclPosSize aPosSize;
				GetPosSize( aPosSize, xTitleShape );
                if( eChartType == EXC_CHART_BAR )
					aDiaPosSize.nHeight -= aPosSize.nHeight;
				else
				{
					aDiaPosSize.nPosX += aPosSize.nWidth;
					aDiaPosSize.nWidth -= aPosSize.nWidth;
				}
			}
		}
        bYAxisDescr = GetPropBool( xDiaProp, EXC_CHPROP_HASYAXISDESCR );
	}
}


//___________________________________________________________________
// common records

void XclObjChart::WriteBeginLevel()
{
	if( !bWriteMode ) return;
	nLevel++;
	WriteEmptyRecord( 0x1033 );	// BEGIN
}


void XclObjChart::WriteEndLevel()
{
	if( !bWriteMode ) return;
	DBG_ASSERT( nLevel, "XclObjChart::WriteEndLevel: no level" );
	if ( !nLevel ) return;

	nLevel--;
	WriteEmptyRecord( 0x1034 );	// END
}


void XclObjChart::WriteSheetHeaderSetup()
{
	if( !bWriteMode ) return;
	const BYTE pData[] = {
		0x14, 0x00, 0x00, 0x00,								// HEADER (none)
		0x15, 0x00, 0x00, 0x00,								// FOOTER (none)
		0x83, 0x00, 0x02, 0x00, 0x00, 0x00,					// HCENTER
		0x84, 0x00, 0x02, 0x00, 0x00, 0x00,					// VCENTER
		0xa1, 0x00, 0x22, 0x00,								// SETUP
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
		0x01, 0x00, 0x04, 0x00, 0x00, 0x00,	0x80, 0x07,
		0x8A, 0x1D, 0x3C, 0xFC, 0xFD, 0x7E, 0xDF, 0x3F,
		0x8A, 0x1D, 0x3C, 0xFC, 0xFD, 0x7E, 0xDF, 0x3F,
		0x00, 0x00
	};
    pStrm->SetSvStreamPos( STREAM_SEEK_TO_END );
    pStrm->Write( pData, sizeof(pData) );
}


void XclObjChart::WriteChart()
{
	if( !bWriteMode ) return;

	const MapMode aSrc( MAP_100TH_MM );
	const MapMode aDst( MAP_POINT );
	Size aSize( OutputDevice::LogicToLogic( rChartObj.GetCurrentBoundRect().GetSize(), aSrc, aDst ) );

	StartRecord( 0x1002, 16 );	// CHART
		// xPos, yPos, xSize, ySize   all in 16.16 format
	*pStrm	<< UINT32(0) << UINT32(0)
			<< UINT32(UINT32(aSize.Width()) << 16)
			<< UINT32(UINT32(aSize.Height()) << 16);
	EndRecord();
}


void XclObjChart::WriteShtprops()
{
	if( !bWriteMode ) return;
	StartRecord( 0x1044, 4 );					// SHTPROPS
    *pStrm  << EXC_CHPROPS_CHANGED              // flags
            << EXC_CHPROPS_EMPTY_SKIP           // empty cells: not plotted
            << sal_uInt8( 0 );
	EndRecord();
}


//___________________________________________________________________
// FRAME group

void XclObjChart::WriteGroupFrame( const uno::Reference< drawing::XShape >& xShape, BOOL bAutoSize )
{
	uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
	WriteGroupFrame( xProp, bAutoSize );
}


void XclObjChart::WriteGroupFrame( const uno::Reference< beans::XPropertySet >& xProp, BOOL bAutoSize )
{
	WriteFrame( bAutoSize );
	WriteBeginLevel();
	WriteLineformat( xProp );
	WriteAreaformat( xProp );
	WriteEndLevel();
}


void XclObjChart::WriteFrame( BOOL bAutoSize )
{
	if( !bWriteMode ) return;
    StartRecord( 0x1032, 4 );           // FRAME
    *pStrm  << EXC_CHFRAME_STANDARD     // regular rectangle, no border
            << UINT16( EXC_CHFRAME_AUTOSIZE | (bAutoSize ? EXC_CHFRAME_AUTOPOS : 0) );
	EndRecord();
}


void XclObjChart::WriteLineformat( const uno::Reference< beans::XPropertySet >& xProp, UINT16 nGrbit )
{
	UINT16	nStyle, nIndex;
	INT16	nWidth;
	Color	aColor;
	GetLineformat( nStyle, nWidth, aColor, nIndex, xProp );
	WriteLineformat( aColor.GetColor(), nIndex, nStyle, nWidth, nGrbit );
}


void XclObjChart::WriteLineformat( const XclChartDataFormat& rF, UINT16 nGrbit )
{
	WriteLineformat( rF.aLineColor.GetColor(), rF.nLineIndex, rF.nLineStyle, rF.nLineWidth, nGrbit );
}


void XclObjChart::WriteLineformat( ColorData nRGB, UINT16 nIndex, UINT16 nStyle, INT16 nWidth, UINT16 nGrbit )
{
	if( !bWriteMode ) return;
	StartRecord( 0x1007, 12 );	// LINEFORMAT
	WriteRGB( nRGB );			// line RGB
	*pStrm	<< nStyle			// line style
			<< nWidth			// line weight
			<< nGrbit			// flags
			<< nIndex;			// color index
	EndRecord();
}


void XclObjChart::WriteAreaformat( const uno::Reference< beans::XPropertySet >& xProp, UINT16 nGrbit )
{
	UINT16	nStyle, nIndex;
	Color	aColor;
	GetAreaformat( nStyle, aColor, nIndex, xProp );
    WriteAreaformat( aColor.GetColor(), COL_BLACK, nIndex, EXC_COLOR_CHWINDOWTEXT, nStyle, nGrbit );
	WriteGelframe( xProp );
}


void XclObjChart::WriteAreaformat( const XclChartDataFormat& rF, UINT16 nGrbit )
{
    WriteAreaformat( rF.aAreaColor.GetColor(), COL_BLACK, rF.nAreaIndex, EXC_COLOR_CHWINDOWTEXT, rF.nAreaStyle, nGrbit );
	WriteGelframe( rF );
}


void XclObjChart::WriteAreaformat( ColorData nForeRGB, ColorData nBackRGB,
			UINT16 nForeIndex, UINT16 nBackIndex, UINT16 nStyle, UINT16 nGrbit )
{
	if( !bWriteMode ) return;
	StartRecord( 0x100A, 16 );	// AREAFORMAT
	WriteRGB( nForeRGB );		// foreground RGB
	WriteRGB( nBackRGB );		// background RGB
	*pStrm	<< nStyle			// fill style
			<< nGrbit			// flags
			<< nForeIndex		// foreground color index
			<< nBackIndex;		// background color index
	EndRecord();
}


void XclObjChart::WriteGelframe( const uno::Reference< beans::XPropertySet >& xProp )
{
    ::std::auto_ptr< EscherPropertyContainer > pGelframe( CreateGelframe( xProp ) );
    if( pGelframe.get() )
        WriteGelframe( *pGelframe );
}


void XclObjChart::WriteGelframe( const XclChartDataFormat& rF )
{
    if( rF.mpGelframe.get() )
        WriteGelframe( *rF.mpGelframe );
}


void XclObjChart::WriteGelframe( EscherPropertyContainer& rGelframe )
{
    if( bWriteMode )
    {
        StartRecord( 0x1066, 0 );       // GELFRAME
        *pStrm << rGelframe;
        EndRecord();
    }
}


//___________________________________________________________________
// POS record, positions & sizes

void XclObjChart::WritePos( const uno::Reference< drawing::XShape >& xShape, TextType eType )
{
	if( !bWriteMode ) return;
	XclPosSize aPos;
	GetPosSize( aPos, xShape );
	WritePos( aPos, eType );
}


void XclObjChart::WritePos( const XclPosSize& rPos, TextType eType )
{
// #90643# diagram/legend/title positioning disabled
#if 0
	if( !bWriteMode ) return;
	StartRecord( 0x104F, 20 );	// POS
	if( eType == ttLegend )
		*pStrm << UINT16(5) << UINT16(2);
	else
		*pStrm << UINT16(2) << UINT16(2);
	WritePosData( rPos, eType );
	EndRecord();
#endif
}

void XclObjChart::WritePosData( const uno::Reference< drawing::XShape >& xShape, TextType eType )
{
	if( !bWriteMode ) return;
	XclPosSize aPos;
	GetPosSize( aPos, xShape );
	WritePosData( aPos, eType );
}


void XclObjChart::WritePosData( const XclPosSize& rPos, TextType eType )
{
	if( !bWriteMode ) return;

	// sometimes Excel expects default-relative position:
	// -> position is relative to default position
	// -> default position is dependent on text type
	// -> axis title position is dependent on axis direction
	// -> axis direction is dependent on chart type
	INT32 nPosX = rPos.nPosX;
	INT32 nPosY = rPos.nPosY;
	if( eType == ttTitle )
	{
		nPosX = GetDefRelPos( rPos.nPosX, rPos.nWidth, 0, 4000, daCenter, 0 );
		nPosY = GetDefRelPos( rPos.nPosY, rPos.nHeight, 0, 4000, daBelowTop, 80 );
	}
    else if( ((eType == ttXAxisTitle) && (eChartType != EXC_CHART_BAR)) || ((eType == ttYAxisTitle) && (eChartType == EXC_CHART_BAR)) )
	{
		nPosY = GetDefRelPos( rPos.nPosX, rPos.nWidth, aDiaPosSize.nPosX, aDiaPosSize.nWidth, daCenter, 0, 1000 );
		nPosX = GetDefRelPos( rPos.nPosY, rPos.nHeight, aDiaPosSize.nPosY, aDiaPosSize.nHeight, daBelowBottom, 0, -1000 );
	}
    else if( ((eType == ttYAxisTitle) && (eChartType != EXC_CHART_BAR)) || ((eType == ttXAxisTitle) && (eChartType == EXC_CHART_BAR)) )
	{
		nPosX = GetDefRelPos( rPos.nPosX, rPos.nWidth, aDiaPosSize.nPosX, aDiaPosSize.nWidth, daAboveTop, 0, 1000 );
		nPosY = GetDefRelPos( rPos.nPosY, rPos.nHeight, aDiaPosSize.nPosY, aDiaPosSize.nHeight, daCenter, 0, -1000 );
	}
	else if( eType == ttZAxisTitle )
		nPosX = nPosY = 0;

	// correct size
	INT32	nWidth	= rPos.nWidth;
	INT32	nHeight	= rPos.nHeight;
	switch( eType )
	{
		case ttTitle:
		case ttXAxisTitle:
		case ttYAxisTitle:
		case ttZAxisTitle:
			nWidth = MM100_TO_PIXEL( double(nWidth) / fX4000 );
			nHeight = MM100_TO_PIXEL( double(nHeight) / fY4000 );
		break;
	}
	if( eType == ttTitle )
		nHeight += (nHeight >> 1);

	*pStrm << nPosX << nPosY << nWidth << nHeight;
}


// INT32 GetDefRelPos() - get position relative to default position
// >nPos<, >nSize<:			position, size of object
// >nParPos<, >nParSize<:	position, size of parent shape
// >eDefaultAlign<:			enum DefaultAlign, alignment of default pos to parent shape
// >nDist<:					distance between margins of parent shape and default pos
// >nScaling<:				alternative scaling factor, default is 4000
INT32 XclObjChart::GetDefRelPos( INT32 nPos, INT32 nSize, INT32 nParPos, INT32 nParSize,
								DefaultAlign eDefAlign, INT32 nDist, INT32 nScaling )
{
	switch( eDefAlign )
	{
		case daAboveTop:	nPos -= (nParPos - nSize - nDist);				break;
		case daBelowTop:	nPos -= (nParPos + nDist);						break;
		case daCenter:		nPos -= (nParPos + ((nParSize - nSize) >> 1));	break;
		case daAboveBottom:	nPos -= (nParPos + nParSize - nSize - nDist);	break;
		case daBelowBottom:	nPos -= (nParPos + nParSize + nDist);			break;
	}
	return INT32(double(nPos) * nScaling / nParSize);
}


void XclObjChart::GetPosSize( XclPosSize& rP, const uno::Reference< drawing::XShape >& xShape )
{
	rP.Set( 0, 0, 0, 0, 0 );
	if ( !xShape.is() ) return;

	const awt::Point&	rPos	= xShape->getPosition();
	const awt::Size&	rSize	= xShape->getSize();
	rP.Set( INT32(rPos.X), INT32(rPos.Y), INT32(rSize.Width), INT32(rSize.Height) );

	uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
	if( xProp.is() )
	{
		UINT16 nRealAngle;
		if( GetRotation( nRealAngle, rP.nAngle, xProp ) && nRealAngle )
		{
			double	fSin			= sin( F_PI180 * nRealAngle );
			double	fCos			= cos( F_PI180 * nRealAngle );
			INT32	nSinWidth		= INT32(fSin * rP.nWidth);
			INT32	nCosWidth		= INT32(fCos * rP.nWidth);
			INT32	nSinHeight		= INT32(fSin * rP.nHeight);
			INT32	nCosHeight		= INT32(fCos * rP.nHeight);
			INT32	nBoundWidth		= Abs( nCosWidth ) + Abs( nSinHeight );
			INT32	nBoundHeight	= Abs( nSinWidth ) + Abs( nCosHeight );

			if( nRealAngle <= 90 )
				rP.nPosY -= nSinWidth;
			else if( nRealAngle < 180 )
			{
				rP.nPosX += nCosWidth;
				rP.nPosY -= nBoundHeight;
			}
			else if( nRealAngle < 270 )
			{
				rP.nPosX -= nBoundWidth;
				rP.nPosY += nCosHeight;
			}
			else
				rP.nPosX += nSinHeight;

			rP.nWidth = nBoundWidth;
			rP.nHeight = nBoundHeight;
		}
	}

	rP.nPosX	= INT32(double(rP.nPosX) * fX4000);
	rP.nPosY	= INT32(double(rP.nPosY) * fY4000);
	rP.nWidth	= INT32(double(rP.nWidth) * fX4000);
	rP.nHeight	= INT32(double(rP.nHeight) * fY4000);
}


BOOL XclObjChart::GetRotation( UINT16& nRealAngle, UINT16& nExcAngle,
								const uno::Reference< beans::XPropertySet >& xProp )
{
	nRealAngle = nExcAngle = 0;

	sal_Int32 nApiAngle;
    if( GetPropInt32( nApiAngle, xProp, EXC_CHPROP_TEXTROTATION ) )
	{
		nRealAngle = (UINT16)(nApiAngle / 100);
        nExcAngle = XclTools::GetXclRotation( nApiAngle );
		return TRUE;
	}
	return FALSE;
}


UINT16 XclObjChart::GetRotationFlags( UINT16 nExcAngle )
{
	if( (nExcAngle > 45) && (nExcAngle <= 90) )
		return EXC_CHART_ROT_CCLOCKW;
	if( (nExcAngle > 135) && (nExcAngle <=180) )
		return EXC_CHART_ROT_CLOCKW;
	return 0x0000;
}


//___________________________________________________________________
// TEXT group

void XclObjChart::WriteTheText()
{
	if( !bWriteMode ) return;

	WriteDefaulttext( 2 );		// default text characteristics for all text in the chart
	uno::Reference< drawing::XShape > xTextShape;
	WriteGroupText( xTextShape, ttNone );
	switch ( eChartType )
	{
        case EXC_CHART_PIE :
        case EXC_CHART_DONUT :
        case EXC_CHART_STOCK :
		{
			WriteDefaulttext( 3 );			// undocumented
			uno::Reference< drawing::XShape > xTextShape;
			WriteGroupText( xTextShape, ttNone );
		}
		break;
	}
}


void XclObjChart::WriteDefaulttext( UINT16 nId )
{
	if( bWriteMode )
		WriteUINT16Record( 0x1024, nId );	// DEFAULTTEXT
}


void XclObjChart::WriteGroupText( const uno::Reference< drawing::XShape >& xTextShape, TextType eType )
{
	// starting the text group...
	XclPosSize aPosSize;
	GetPosSize( aPosSize, xTextShape );
	WriteText( xTextShape, aPosSize, eType );	// find color or write TEXT

	WriteBeginLevel();

	// write POS for specific text types
	switch( eType )
	{
		case ttTitle:
		case ttXAxisTitle:
		case ttYAxisTitle:
		case ttZAxisTitle:	WritePos( aPosSize, eType );	break;
	}

	uno::Reference< beans::XPropertySet > xProp( xTextShape, uno::UNO_QUERY );
	WriteFontx( xProp );		// insert font to font list or write the index
	WriteAI();

	// link data for specific text types
    UINT16 nLinkObj = EXC_CHOBJLINK_NONE;
	switch( eType )
	{
        case ttTitle:       nLinkObj = EXC_CHOBJLINK_TITLE;   break;
        case ttXAxisTitle:  nLinkObj = EXC_CHOBJLINK_XAXIS;   break;
        case ttYAxisTitle:  nLinkObj = EXC_CHOBJLINK_YAXIS;   break;
        case ttZAxisTitle:  nLinkObj = EXC_CHOBJLINK_ZAXIS;   break;
	}

    if ( (nLinkObj != EXC_CHOBJLINK_NONE) && xTextShape.is() )
	{
		WriteSeriestext( xProp );
		WriteGroupFrame( xTextShape );		// find frame colors or write them
		WriteObjectlink( nLinkObj, 0, 0 );
	}

	WriteEndLevel();
}


// data point caption
void XclObjChart::WriteGroupText( UINT16 nSeries, UINT16 nPoint, UINT16 nFontx,
								ColorData nRGB, UINT16 nIndex, UINT16 nFlags )
{
	XclPosSize aPosSize;
	aPosSize.nPosX = aPosSize.nPosY = aPosSize.nWidth = aPosSize.nHeight = 0;
	aPosSize.nAngle = 0;

    WriteText( aPosSize, nRGB, nIndex, ttDataCaption, nFlags | EXC_CHTEXT_STORED );
	WriteBeginLevel();
	WritePos( aPosSize, ttDataCaption );
	WriteFontx( nFontx );
	WriteAI();
    WriteObjectlink( EXC_CHOBJLINK_DATA, nSeries, nPoint );
	WriteEndLevel();
}


void XclObjChart::WriteText( const uno::Reference< drawing::XShape >& xTextShape,
							const XclPosSize& rPosSize, TextType eType )
{
    UINT16  nFlags = xTextShape.is() ? 0x0000 : EXC_CHTEXT_STORED;
    UINT16  nIndex = EXC_COLOR_CHWINDOWTEXT;
	Color	aColor( COL_BLACK );

	// text color
	uno::Reference< beans::XPropertySet > xProp( xTextShape, uno::UNO_QUERY );
	if( xProp.is() )
		GetTextColor( aColor, nIndex, xProp );		// insert into palette or get index

    if( !xProp.is() || (nIndex == EXC_COLOR_CHWINDOWTEXT) )
        nFlags |= EXC_CHTEXT_AUTOCOLOR;

	WriteText( rPosSize, aColor.GetColor(), nIndex, eType, nFlags );
}


void XclObjChart::WriteText( const XclPosSize& rPosSize, const ColorData nRGB,
							UINT16 nIndex, TextType eType, UINT16 nFlags )
{
	if( !bWriteMode ) return;

	// flags
    nFlags |= EXC_CHTEXT_AUTOMODE;
    nFlags |= EXC_CHART_TEXT_GETROTFLAGS( GetRotationFlags( rPosSize.nAngle ) );

	// label placement
    UINT16 nPlacement = EXC_CHTEXT_POS_DEFAULT;
// #90643# diagram/legend/title positioning disabled
	switch( eType )
	{
#if 0
		case ttTitle:
		case ttXAxisTitle:
        case ttYAxisTitle:  nPlacement = EXC_CHTEXT_POS_MOVED;      break;
		case ttLegend:
        case ttZAxisTitle:  nPlacement = EXC_CHTEXT_POS_DEFAULT;    break;
#endif
		case ttDataCaption:
            if( (eChartType == EXC_CHART_LINE) || (eChartType == EXC_CHART_AREA) )
                nPlacement = EXC_CHTEXT_POS_ABOVE;
		break;
	}

	StartRecord( 0x1025, 32 );						// TEXT
    *pStrm  << EXC_CHTEXT_ALIGN_CENTER              // horizontal alignment
            << EXC_CHTEXT_ALIGN_CENTER              // vertical alignment
            << EXC_CHTEXT_TRANSPARENT;              // character background
	WriteRGB( nRGB );								// RGB value of the text color
	WritePosData( rPosSize, ttCommon );				// position & size
	*pStrm	<< nFlags								// option flags
			<< nIndex								// index to color value of text
			<< nPlacement							// data label placement
			<< rPosSize.nAngle;						// text rotation
	EndRecord();
}


void XclObjChart::WriteFontx( const uno::Reference< beans::XPropertySet >& xProp )
{
	WriteFontx( GetFontx( xProp ) );
}


void XclObjChart::WriteFontx( UINT16 nFontx )
{
	if( bWriteMode )
		WriteUINT16Record( 0x1026, nFontx );	// FONTX
}


void XclObjChart::WriteAI()
{
	if( !bWriteMode ) return;
	StartRecord( 0x1051, 8 );	// AI linked data
	*pStrm	<< UINT8(0)			// title or text
			<< UINT8(1)			// text or value entered directly into the formula bar (2do: really?)
			<< UINT16(0)		// flags, number format is linked to data source
			<< UINT16(0)		// index to number format record
			<< UINT16(0);		// size of formula
	EndRecord();
}


void XclObjChart::WriteSeriestext( const uno::Reference< beans::XPropertySet >& xProp )
{
	if( !bWriteMode ) return;

	OUString sString;
    if( GetPropValue( xProp, EXC_CHPROP_STRING ) && (aAny >>= sString) )
        WriteSeriestext( XclExpString( sString, EXC_STR_FORCEUNICODE | EXC_STR_8BITLENGTH | EXC_STR_SMARTFLAGS ) );
}


void XclObjChart::WriteSeriestext( const XclExpString& rString )
{
	if( !bWriteMode ) return;

    StartRecord( 0x100D, 2 + rString.GetSize() );   // SERIESTEXT
    *pStrm  << UINT16(0)                            // text identifier: 0:= series name or text
            << rString;
	EndRecord();
}


void XclObjChart::WriteObjectlink( UINT16 nLinkObj, UINT16 nLinkVar1, UINT16 nLinkVar2 )
{
	if( !bWriteMode ) return;
	StartRecord( 0x1027, 6 );	// OBJECTLINK
    *pStrm  << nLinkObj         // object text link ( EXC_CHOBJLINK_*** )
			<< nLinkVar1		// link index 1, series number
			<< nLinkVar2;		// link index 2, data point number
	EndRecord();
}


void XclObjChart::WriteTextList()
{
	for( const XclChartText* pText = aTextList.First(); pText; pText = aTextList.Next() )
		WriteGroupText( pText->nSeries, pText->nPoint, pText->nFontx, pText->nRGB, pText->nIndex, pText->nFlags );
}


//___________________________________________________________________
// SERIES group

void XclObjChart::AppendToSeriesNUpnList( const ScAddress* pA )
{
	if ( pA )
        aSeriesNUpnList.push_back( GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_CHART, *pA ) );
}


void XclObjChart::BuildSeriesList()
{
	chart::ChartDataRowSource eSource;
	//! how about extracting an enum value from any?
    if( GetPropValue( xDiaProp, EXC_CHPROP_DATAROWSOURCE ) &&
			(aAny.getValueTypeClass() == uno::TypeClass_ENUM) )
		eSource = (chart::ChartDataRowSource) *(sal_Int32*)aAny.getValue();
	else
		eSource = chart::ChartDataRowSource_ROWS;
	const ScChartPositionMap* pPosMap = pChartArray->GetPositionMap();
	bColHeaders = pChartArray->HasColHeaders();
	bRowHeaders = pChartArray->HasRowHeaders();
	if ( eSource == chart::ChartDataRowSource_ROWS )
	{
		nChartDataColumns = pPosMap->GetColCount();
		SCSIZE nRows = pPosMap->GetRowCount();
		SCSIZE nRow = 0;
		switch ( eChartType )
		{
            case EXC_CHART_SCATTER :
				if ( nRow < nRows )
					aSeriesXRangeList.Append( pPosMap->GetRowRanges( nRow++ ) );
			break;
			default:
				if ( bColHeaders )
				{
                    nChartCategoriesDataType = EXC_CHSERIES_TEXT;    // text
					ScRangeListRef xRL = new ScRangeList;
					SCSIZE nCols = pPosMap->GetColCount();
					for ( SCSIZE nCol = 0; nCol < nCols; nCol++ )
					{
						const ScAddress* pAdr = pPosMap->GetColHeaderPosition( nCol );
						if ( pAdr )
							xRL->Join( ScRange( *pAdr ) );
					}
					aSeriesXRangeList.Append( xRL );
				}
		}
		for ( ; nRow < nRows; nRow++ )
		{
			aSeriesYRangeList.Append( pPosMap->GetRowRanges( nRow ) );
			// N-list must have same count as Y-list if any
			if ( bRowHeaders )
				AppendToSeriesNUpnList( pPosMap->GetRowHeaderPosition( nRow ) );
		}
	}
	else
	{
		nChartDataColumns = pPosMap->GetRowCount();
		SCSIZE nCols = pPosMap->GetColCount();
		SCSIZE nCol = 0;
		switch ( eChartType )
		{
            case EXC_CHART_SCATTER :
				if ( nCol < nCols )
					aSeriesXRangeList.Append( pPosMap->GetColRanges( nCol++ ) );
			break;
			default:
				if ( bRowHeaders )
				{
                    nChartCategoriesDataType = EXC_CHSERIES_TEXT;    // text
					ScRangeListRef xRL = new ScRangeList;
					SCSIZE nRows = pPosMap->GetRowCount();
					for ( SCSIZE nRow = 0; nRow < nRows; nRow++ )
					{
						const ScAddress* pAdr = pPosMap->GetRowHeaderPosition( nRow );
						if ( pAdr )
							xRL->Join( ScRange( *pAdr ) );
					}
					aSeriesXRangeList.Append( xRL );
				}
		}
		for ( ; nCol < nCols; nCol++ )
		{
			aSeriesYRangeList.Append( pPosMap->GetColRanges( nCol ) );
			// N-list must have same count as Y-list if any
			if ( bColHeaders )
				AppendToSeriesNUpnList( pPosMap->GetColHeaderPosition( nCol ) );
		}
	}
	// Y-values
	ScRangeListMemberList* pRLML = &aSeriesYRangeList;
    XclTokenArrayVec* pUL = &aSeriesYUpnList;
	ULONG nCnt = pRLML->Count();
	do
	{
		ULONG j;
		ScRangeListRef xRL;
		for ( j=0, xRL = pRLML->First(); j<nCnt; j++, xRL = pRLML->Next() )
            pUL->push_back( GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_CHART, *xRL ) );

		if ( pRLML != &aSeriesXRangeList )
		{
			// X-categories
			pRLML = &aSeriesXRangeList;
			pUL = &aSeriesXUpnList;
			nCnt = pRLML->Count();
		}
		else
			nCnt = 0;
	} while ( nCnt );
}


void XclObjChart::WriteTheSeries()
{
	ULONG nCount = aSeriesYRangeList.Count();
	DBG_ASSERT( nCount, "XclObjChart::WriteTheSeries: no Y-RangeList" );
	if ( !nCount )
		return ;
	DBG_ASSERT( aSeriesXRangeList.Count() <= 1, "XclObjChart::WriteTheSeries: more than one X-RangeList" );
	ScRangeListRef xYRL, xXRL;
	xXRL = aSeriesXRangeList.First();
	UINT16 nXCellCount = xXRL.Is() ? UINT16(xXRL->GetCellCount()) : 0;
#ifndef PRODUCT
	switch ( eChartType )
	{
        case EXC_CHART_SCATTER :
			DBG_ASSERT( nXCellCount, "XclObjChart::WriteTheSeries: no X-Range" );
		break;
	}
#endif
    size_t nYUPN, nXUPN, nNUPN;
	UINT16 nIndex, nNumber;
	SvUShorts* pSerieslist = NULL;
    nXUPN = 0;

    BOOL bReverse = ((eChartType == EXC_CHART_AREA) && !bChart3D && !bChartStacked && !bChartPercent) || (eChartType == EXC_CHART_DONUT);
	if ( bReverse )
	{	// reverse order to make it more look alike
		xYRL = aSeriesYRangeList.Last();
        nYUPN = aSeriesYUpnList.size() - 1;
        nNUPN = aSeriesNUpnList.size() - 1;
		nIndex = nNumber = (UINT16)(nCount - 1);
	}
	else
	{
		xYRL = aSeriesYRangeList.First();
        nYUPN = 0;
        nNUPN = 0;
		nIndex = nNumber = 0;
	}
	for ( ULONG nCnt = 0; nCnt < nCount; nCnt++ )
	{
		UINT16 nYValCount = UINT16(xYRL->GetCellCount());
		UINT16 nXCatCount;
		switch ( eChartType )
		{
            case EXC_CHART_SCATTER :
				nXCatCount = nXCellCount;
			break;
			default:
				nXCatCount = nYValCount;
		}

        UINT16 nOldAxesSet = nCurrAxesSet;
        if ( eChartType == EXC_CHART_STOCK && bStockVolume )
		{
			switch ( nIndex )
			{
				case 0:		nNumber = bStockUpDown ? 4 : 3;	break;
                case 1:     nNumber = 0; nCurrAxesSet++;    break;
				case 2:		nNumber = 1;					break;
				case 3:		nNumber = 2;					break;
				case 4:		nNumber = bStockUpDown ? 3 : 4;	break;
				default:	nNumber = nIndex;				break;
			}
		}
        if ( nCurrAxesSet < nOldAxesSet )
            pSerieslist = (SvUShorts*) aSerieslistList.GetObject( nCurrAxesSet );
        else if ( nCurrAxesSet > nOldAxesSet )
		{
			pSerieslist = new SvUShorts( 4, 4 );
			aSerieslistList.Insert( pSerieslist, LIST_APPEND );
		}
		if ( pSerieslist )
			pSerieslist->Insert( nIndex, pSerieslist->Count() );

        XclTokenArrayRef xYUPN;
        if( nYUPN < aSeriesYUpnList.size() )
            xYUPN = aSeriesYUpnList[ nYUPN ];
        XclTokenArrayRef xXUPN;
        if( nXUPN < aSeriesXUpnList.size() )
            xXUPN = aSeriesXUpnList[ nXUPN ];
        XclTokenArrayRef xNUPN;
        if( nNUPN < aSeriesNUpnList.size() )
            xNUPN = aSeriesNUpnList[ nNUPN ];

        WriteGroupSeries( (UINT16) nCnt, nIndex, nNumber, nXCatCount, nYValCount, xYUPN, xXUPN, xNUPN );

		if ( bReverse )
		{
			xYRL = aSeriesYRangeList.Prev();
            --nYUPN;
            --nNUPN;
			nIndex--;
			nNumber--;
		}
		else
		{
			xYRL = aSeriesYRangeList.Next();
            ++nYUPN;
            ++nNUPN;
			nIndex++;
			nNumber++;
		}
	}
}


void XclObjChart::WriteGroupSeries( UINT16 nSeriesCnt, UINT16 nSeriesIndex, UINT16 nSeriesNumber,
									UINT16 nXCatCount, UINT16 nYValCount,
                                    XclTokenArrayRef xYUPN,
                                    XclTokenArrayRef xXUPN,
                                    XclTokenArrayRef xNUPN )
{
	WriteSeries( nXCatCount, nYValCount );
	WriteBeginLevel();

	if( bWriteMode )
	{
		UINT16	nFmt = 0;	// linked to data source
        UINT8   nRef = xNUPN.is() ? EXC_CHSRCLINK_WORKSHEET : EXC_CHSRCLINK_DIRECTLY;
        WriteAI( xNUPN, EXC_CHSRCLINK_TITLE, nRef, nFmt );

        nRef = xYUPN.is() ? EXC_CHSRCLINK_WORKSHEET : EXC_CHSRCLINK_DIRECTLY;
        WriteAI( xYUPN, EXC_CHSRCLINK_VALUES, nRef, nFmt );

        nRef = xXUPN.is() ? EXC_CHSRCLINK_WORKSHEET : EXC_CHSRCLINK_DEFAULT;
        WriteAI( xXUPN, EXC_CHSRCLINK_CATEGORY, nRef, nFmt );

        WriteAI( XclTokenArrayRef(), EXC_CHSRCLINK_BUBBLES, EXC_CHSRCLINK_DIRECTLY, 0 );
	}

	// properties of entire series
	BOOL				bGetLineArea = bChartTypeBars || bChartTypeRound;
    XclChartDataFormat  aSerFormats;
    UINT16              nSerNum = (eChartType == EXC_CHART_SCATTER) ? nSeriesIndex + 1 : nSeriesIndex;
	if( !bChartTypeRound )
	{
#ifdef USE_JAVA
		// Fix bug 2663 by catching any exceptions that are thrown since the
		// application will abort with it since there are no exception handling
		// block higher up in the stack
		try
		{
#endif	// USE_JAVA
		uno::Reference< beans::XPropertySet >	xSerProp = xDiagram->getDataRowProperties( nSerNum );
		GetFormats( aSerFormats, xSerProp, TRUE, TRUE );
        WriteGroupDataformat( nSeriesCnt, nSeriesNumber, EXC_CHDATAFORMAT_ALLPOINTS, aSerFormats );

        // series on secondary Y-axis? (for stock charts done in WriteTheSeries())
        if( eChartType != EXC_CHART_STOCK )
        {
            sal_Int32 nYAssign;
            if( GetPropInt32( nYAssign, xSerProp, EXC_CHPROP_AXIS ) )
                nCurrAxesSet = (nYAssign == chart::ChartAxisAssign::SECONDARY_Y) ? EXC_CHAXESSET_SECONDARY : EXC_CHAXESSET_PRIMARY;
            else
                nCurrAxesSet = EXC_CHAXESSET_PRIMARY;
        }
        bHasSecAxes |= (nCurrAxesSet == EXC_CHAXESSET_SECONDARY);
#ifdef USE_JAVA
		}
		catch ( ... )
		{
		}
#endif	// USE_JAVA
	}

    uno::Sequence< uno::Sequence< sal_Int32 > > aAttributed;
    if( GetPropValue( xDiaProp, EXC_CHPROP_ATTRIBUTEDDP ) && (aAny >>= aAttributed) )
    {
        if( aAttributed.getLength() > nSerNum )
        {
            // properties of single data points
            uno::Sequence< sal_Int32 >& rAttrPoints = aAttributed[ nSerNum ];
            sal_Int32 nPointCount = rAttrPoints.getLength();
            for( sal_Int32 nPoint = 0; nPoint < nPointCount; ++nPoint )
            {
                sal_Int32 nCol = rAttrPoints[ nPoint ];
                XclChartDataFormat aPointFormats;
                uno::Reference< beans::XPropertySet >   xPointProp = (eChartType == EXC_CHART_DONUT) ?
                    xDiagram->getDataPointProperties( nSerNum, nCol ) :
                    xDiagram->getDataPointProperties( nCol, nSerNum );
                GetFormats( aPointFormats, xPointProp, bGetLineArea, TRUE );
                WriteGroupDataformat( nSeriesCnt, nSeriesNumber, (UINT16)nCol, aPointFormats );
            }
        }
	}
	WriteSertocrt();
	WriteEndLevel();
}


void XclObjChart::WriteSeries( UINT16 nXCatCount, UINT16 nYValCount )
{
	if( !bWriteMode ) return;

	UINT16 sdtY, sdtBSize, cValBSize;

//2do: data types
    sdtY = EXC_CHSERIES_NUMERIC;    // assume numeric data type

//2do: data type and count of bubble size values ?!?
    sdtBSize = EXC_CHSERIES_NUMERIC;
	cValBSize = 0;

	StartRecord( 0x1003, 12 );				// SERIES
	*pStrm	<< nChartCategoriesDataType		// type of data in categories
			<< sdtY							// type of data in values
			<< nXCatCount					// count of categories
			<< nYValCount					// count of values
			<< sdtBSize						// type of data in bubble size series
			<< cValBSize;					// count of bubble series values
	EndRecord();
}


//___________________________________________________________________
// DATAFORMAT group

void XclObjChart::WriteGroupDataformat( UINT16 nSeriesIndex, UINT16 nSeriesNumber,
										UINT16 nSeriesPoint, const XclChartDataFormat& rFormats )
{
    if( !bWriteMode ) return;

	WriteDataformat( nSeriesIndex, nSeriesNumber, nSeriesPoint );
	WriteBeginLevel();
	Write3DDataformat( rFormats );
	WriteLineformat( rFormats );
	WriteAreaformat( rFormats );
	if( bChartTypeRound )
		WritePieformat( rFormats.nSegment );
    if( ((nSeriesPoint == EXC_CHDATAFORMAT_ALLPOINTS) || (nSeriesNumber == EXC_CHDATAFORMAT_DEFAULT)) && bChartSpline )
        WriteSerfmt( EXC_CHSERIESFORMAT_SMOOTHED );
    if( (eChartType == EXC_CHART_LINE) || (eChartType == EXC_CHART_SCATTER) || (eChartType == EXC_CHART_RADARLINE) || (eChartType == EXC_CHART_STOCK) )
	{
		UINT16 nSymbol;
        if( eChartType == EXC_CHART_STOCK )
			nSymbol = (!bStockUpDown && ((!bStockVolume && (nSeriesIndex == 2)) || (bStockVolume && (nSeriesIndex == 3)))) ?
                EXC_CHMARKERFORMAT_DOWJ : EXC_CHMARKERFORMAT_NOSYMBOL;
		else
			nSymbol = rFormats.GetSymbol( nSeriesNumber );
        if( (eChartType == EXC_CHART_SCATTER) && !bChartLine )
			WriteMarkerformat( nSymbol, rFormats.nLineIndex, rFormats.nAreaIndex );
		else
            WriteMarkerformat( nSymbol, EXC_COLOR_CHWINDOWTEXT, rFormats.nLineIndex );
	}
    if( (nSeriesPoint == EXC_CHDATAFORMAT_ALLPOINTS) || rFormats.nLabelFlags )
		WriteAttachedLabel( rFormats.nLabelFlags );
	WriteEndLevel();

	aTextList.Append( new XclChartText( nSeriesIndex, nSeriesPoint, rFormats ) );
}


void XclObjChart::WriteDataformat( UINT16 nSeriesIndex, UINT16 nSeriesNumber,
			UINT16 nSeriesPoint )
{
	if( !bWriteMode ) return;
	StartRecord( 0x1006, 8 );	// DATAFORMAT
	*pStrm	<< nSeriesPoint		// point number, 0xFFFF:= entire series
			<< nSeriesIndex		// series index
			<< nSeriesNumber	// series number
			<< UINT16(0);		// format flags
	EndRecord();
}


void XclObjChart::Write3DDataformat( const XclChartDataFormat& rFormats )
{
	if( !bWriteMode ) return;
	StartRecord( 0x105F, 2 );	// 3DDATAFORMAT
	*pStrm	<< rFormats.nDataFormatBase
			<< rFormats.nDataFormatTop;
	EndRecord();
}


void XclObjChart::WritePieformat( UINT16 nPercent )
{
	if( !bWriteMode ) return;
	nMaxSegOffset = Max( nMaxSegOffset, nPercent );
	WriteUINT16Record( 0x100B, nPercent );	// PIEFORMAT
}


void XclObjChart::WriteMarkerformat( UINT16 nType, UINT16 nBordInd, UINT16 nFillInd )
{
	if( !bWriteMode ) return;
    StartRecord( 0x1009, 20 );                  // MARKERFORMAT
    WriteRGB( rPal.GetColorData( nBordInd ) );  // foreground color RGB
    WriteRGB( rPal.GetColorData( nFillInd ) );  // background color RGB
    *pStrm  << nType                            // type of marker
            << UINT16(0)                        // format flags
            << nBordInd                         // index to color of marker border
            << nFillInd                         // index to color of marker fill
            << UINT32(120);                     // size of line markers in twips
	EndRecord();
}


void XclObjChart::WriteSerfmt( UINT16 nFlags )
{
	if( bWriteMode )
		WriteUINT16Record( 0x105D, nFlags );	// SERFMT
}


void XclObjChart::WriteAttachedLabel( UINT16 nFlags )
{
	if( bWriteMode )
		WriteUINT16Record( 0x100C, nFlags );	// ATTACHEDLABEL
}


void XclObjChart::WriteSertocrt()
{
	if( bWriteMode )
        WriteUINT16Record( 0x1045, nCurrAxesSet );   // SERTOCRT
}


void XclObjChart::WriteAI( XclTokenArrayRef xUPN, UINT8 nType, UINT8 nRef, UINT16 nFmt )
{
	if( !bWriteMode ) return;

    UINT16 nFormLen = xUPN.is() ? xUPN->GetSize() : 0;
	StartRecord( 0x1051, 8 + nFormLen );	// AI linked data
	*pStrm	<< nType						// link type: title, values, categories
			<< nRef							// reference type
			<< UINT16(0)					// flags: number format is linked to data source
			<< nFmt							// index to number format record
			<< nFormLen;					// size of formula
    if ( xUPN.is() )
        xUPN->WriteArray( *pStrm );
	EndRecord();
}


//___________________________________________________________________
// AXIS group

void XclObjChart::WriteAllAxes()
{
//    if( !bHasSecAxes )
//    {
//        bHasSecAxes |= GetPropBool( xDiaProp, EXC_CHPROP_HASSECXAXIS );
//        bHasSecAxes |= GetPropBool( xDiaProp, EXC_CHPROP_HASSECYAXIS );
//    }

	WriteAxesused( bHasSecAxes ? 2 : 1 );
    WriteTheAxes( EXC_CHAXESSET_PRIMARY );
	if( bHasSecAxes )
        WriteTheAxes( EXC_CHAXESSET_SECONDARY );
}


void XclObjChart::WriteAxesused( UINT16 nNumber )
{
	if( bWriteMode )
		WriteUINT16Record( 0x1046, nNumber );	// AXESUSED
}


void XclObjChart::WriteTheAxes( UINT16 nNewAxesSet )
{
    nCurrAxesSet = nNewAxesSet;

	WriteAxisparent();
	WriteBeginLevel();
    WritePos( aDiaPosSize, ttCommon );

	if( !bChartTypeRound )
	{
        WriteGroupAxis( EXC_CHAXIS_X );
        WriteGroupAxis( EXC_CHAXIS_Y );
        if( nCurrAxesSet == EXC_CHAXESSET_PRIMARY )
        {
            if( bChart3D )
                WriteGroupAxis( EXC_CHAXIS_Z );
            else if( eChartType != EXC_CHART_RADARLINE )
            {
                WritePlotarea();
                WriteGroupFrame( xDiagr3D->getWall() );
            }
        }
	}

    WriteGroupChartformat();
	WriteEndLevel();
}


void XclObjChart::WriteAxisparent()
{
	if( !bWriteMode ) return;

	// shrink diagram area for exploded pie charts
	if( nMaxSegOffset )
	{
		double fScale = 100.0 / (double(nMaxSegOffset) + 100.0);
		INT32 nNew = INT32(double(aDiaPosSize.nWidth) * fScale);
		aDiaPosSize.nPosX += ((aDiaPosSize.nWidth - nNew) >> 1);
		aDiaPosSize.nWidth = nNew;
		nNew = INT32(double(aDiaPosSize.nHeight) * fScale);
		aDiaPosSize.nPosY += ((aDiaPosSize.nHeight - nNew) >> 1);
		aDiaPosSize.nHeight = nNew;
	}

	StartRecord( 0x1041, 18 );			// AXISPARENT
    *pStrm  << nCurrAxesSet;            // index of axes set
	WritePosData( aDiaPosSize, ttCommon );
	EndRecord();
}


void XclObjChart::WritePlotarea()
{
	if( bWriteMode )
		WriteEmptyRecord( 0x1035 );	// PLOTAREA
}


void XclObjChart::WriteGroupAxis( UINT16 nNewAxis )
{
    nCurrAxis = nNewAxis;

	uno::Reference< drawing::XShape >	xTitleShape;
	TextType							eTextType = ttNone;

	WriteAxis();
	WriteBeginLevel();

    switch( nCurrAxesSet )
    {
        case EXC_CHAXESSET_PRIMARY:
        {
            switch( nCurrAxis )
            {
                case EXC_CHAXIS_X:
                {
                    WriteXZRange();
                    if( eChartType == EXC_CHART_RADARLINE )
                        WriteFirstYAxis( xTitleShape, eTextType, FALSE );
                    else
                        WriteFirstXAxis( xTitleShape, eTextType, FALSE );
                }
                break;
                case EXC_CHAXIS_Y:
                    WriteFirstYAxis( xTitleShape, eTextType, TRUE );
                break;
                case EXC_CHAXIS_Z:
                    WriteXZRange();
                    WriteFirstZAxis( xTitleShape, eTextType );
                break;
            }
        }
        break;
        case EXC_CHAXESSET_SECONDARY:
        {
            switch( nCurrAxis )
            {
                case EXC_CHAXIS_X:
                    WriteXZRange();
                    WriteFirstXAxis( xTitleShape, eTextType, TRUE );
                    eTextType = ttNone;     // don't write title for 2nd axis
                break;
                case EXC_CHAXIS_Y:
                    WriteSecondYAxis();
                break;
            }
        }
        break;
    }

	WriteEndLevel();

    if( (eChartType != EXC_CHART_RADARLINE) && (eTextType != ttNone) )
		WriteGroupText( xTitleShape, eTextType );
}


void XclObjChart::WriteFirstXAxis( uno::Reference< drawing::XShape >& xTitle, TextType& rType, BOOL bMaxCross )
{
	uno::Reference< chart::XAxisXSupplier > xDiaXSup( xDiagram, uno::UNO_QUERY );
	DBG_ASSERT( xDiaXSup.is(), "XclObjChart::WriteFirstXAxis: no XAxisXSupplierRef" );
	rType = ttNone;

    if ( xDiaXSup.is() && GetPropBool( xDiaProp, EXC_CHPROP_HASXAXIS ) )
	{
		uno::Reference< beans::XPropertySet > xXAxis = xDiaXSup->getXAxis();

        if( eChartType == EXC_CHART_SCATTER )
		{
            WriteValuerange( xXAxis, bMaxCross );
			WriteIfmt( xXAxis );
		}
		WriteTick( xXAxis, bXAxisDescr );
		WriteFontx( xXAxis );
        WriteTheAxisline( xXAxis, EXC_CHAXISLINE_AXISLINE, bXAxisDescr );
        if( GetPropBool( xDiaProp, EXC_CHPROP_HASXAXISGRID ) )
            WriteTheAxisline( xDiaXSup->getXMainGrid(), EXC_CHAXISLINE_MAJORGRID );
        if( GetPropBool( xDiaProp, EXC_CHPROP_HASXAXISHELPGRID ) )
            WriteTheAxisline( xDiaXSup->getXHelpGrid(), EXC_CHAXISLINE_MINORGRID );
		if( bChart3D )
            WriteTheAxisframe( xDiagr3D->getWall(), EXC_CHAXISLINE_WALLS );
        if( GetPropBool( xDiaProp, EXC_CHPROP_HASXAXISTIT ) )
		{
			xTitle = xDiaXSup->getXAxisTitle();
			rType = ttXAxisTitle;
		}
	}
}

#if 0
void XclObjChart::WriteSecondXAxis()
{
	uno::Reference< chart::XTwoAxisXSupplier > xDiaXSup( xDiagram, uno::UNO_QUERY );
	DBG_ASSERT( xDiaXSup.is(), "XclObjChart::WriteGroupAxis: no XTwoAxisXSupplierRef" );

    if ( xDiaXSup.is() && GetPropBool( xDiaProp, EXC_CHPROP_HASSECXAXIS ) )
	{
		uno::Reference< beans::XPropertySet > xXAxis = xDiaXSup->getSecondaryXAxis();

        if( eChartType == EXC_CHART_SCATTER )
		{
            WriteValuerange( xXAxis, FALSE );
			WriteIfmt( xXAxis );
		}
        BOOL bSecXAxisDescr = GetPropBool( xDiaProp, EXC_CHPROP_HASSECXAXISDESCR );
		WriteTick( xXAxis, bSecXAxisDescr );
		WriteFontx( xXAxis );
        WriteTheAxisline( xXAxis, EXC_CHAXISLINE_AXISLINE, bSecXAxisDescr );
	}
}
#endif

void XclObjChart::WriteFirstYAxis( uno::Reference< drawing::XShape >& xTitle, TextType& rType, BOOL bWriteExtras )
{
	uno::Reference< chart::XAxisYSupplier > xDiaYSup( xDiagram, uno::UNO_QUERY );
	DBG_ASSERT( xDiaYSup.is(), "XclObjChart::WriteGroupAxis: no XAxisYSupplierRef" );
	rType = ttNone;

    if ( xDiaYSup.is() && GetPropBool( xDiaProp, EXC_CHPROP_HASYAXIS ) )
	{
		uno::Reference< beans::XPropertySet > xYAxis = xDiaYSup->getYAxis();

		if( bWriteExtras )
		{
            WriteValuerange( xYAxis, FALSE );
			WriteIfmt( xYAxis );
		}

		WriteTick( xYAxis, bYAxisDescr );
		WriteFontx( xYAxis );

		if( bWriteExtras )
		{
            WriteTheAxisline( xYAxis, EXC_CHAXISLINE_AXISLINE, bYAxisDescr );
            if( GetPropBool( xDiaProp, EXC_CHPROP_HASYAXISGRID ) )
                WriteTheAxisline( xDiaYSup->getYMainGrid(), EXC_CHAXISLINE_MAJORGRID );
            if( GetPropBool( xDiaProp, EXC_CHPROP_HASYAXISHELPGRID ) )
                WriteTheAxisline( xDiaYSup->getYHelpGrid(), EXC_CHAXISLINE_MINORGRID );
			if( bChart3D )
                WriteTheAxisframe( xDiagr3D->getFloor(), EXC_CHAXISLINE_WALLS );
            if( GetPropBool( xDiaProp, EXC_CHPROP_HASYAXISTIT ) )
			{
				xTitle = xDiaYSup->getYAxisTitle();
				rType = ttYAxisTitle;
			}
		}
	}
}


void XclObjChart::WriteSecondYAxis()
{
	uno::Reference< chart::XTwoAxisYSupplier > xDiaYSup( xDiagram, uno::UNO_QUERY );
	DBG_ASSERT( xDiaYSup.is(), "XclObjChart::WriteGroupAxis: no XTwoAxisYSupplierRef" );

    if ( xDiaYSup.is() && GetPropBool( xDiaProp, EXC_CHPROP_HASSECYAXIS ) )
	{
		uno::Reference< beans::XPropertySet > xYAxis = xDiaYSup->getSecondaryYAxis();

        WriteValuerange( xYAxis, FALSE );
		WriteIfmt( xYAxis );
        BOOL bSecYAxisDescr = GetPropBool( xDiaProp, EXC_CHPROP_HASSECYAXISDESCR );
		WriteTick( xYAxis, bSecYAxisDescr );
		WriteFontx( xYAxis );
        WriteTheAxisline( xYAxis, EXC_CHAXISLINE_AXISLINE, bSecYAxisDescr );
	}
}


void XclObjChart::WriteFirstZAxis( uno::Reference< drawing::XShape >& xTitle, TextType& rType )
{
	uno::Reference< chart::XAxisZSupplier > xDiaZSup( xDiagram, uno::UNO_QUERY );
	DBG_ASSERT( xDiaZSup.is(), "XclObjChart::WriteFirstZAxis: no XAxisZSupplierRef" );
	rType = ttNone;

    if ( xDiaZSup.is() && GetPropBool( xDiaProp, EXC_CHPROP_HASZAXIS ) )
	{
		uno::Reference< beans::XPropertySet > xZAxis = xDiaZSup->getZAxis();

        BOOL bZAxisDescr = GetPropBool( xDiaProp, EXC_CHPROP_HASZAXISDESCR );
		WriteTick( xZAxis, bZAxisDescr );
		WriteFontx( xZAxis );
        WriteTheAxisline( xZAxis, EXC_CHAXISLINE_AXISLINE, bZAxisDescr );
        if( GetPropBool( xDiaProp, EXC_CHPROP_HASZAXISGRID ) )
            WriteTheAxisline( xDiaZSup->getZMainGrid(), EXC_CHAXISLINE_MAJORGRID );
        if( GetPropBool( xDiaProp, EXC_CHPROP_HASZAXISHELPGRID ) )
            WriteTheAxisline( xDiaZSup->getZHelpGrid(), EXC_CHAXISLINE_MINORGRID );
        if( GetPropBool( xDiaProp, EXC_CHPROP_HASZAXISTIT ) )
		{
			xTitle = xDiaZSup->getZAxisTitle();
			rType = ttZAxisTitle;
		}
	}
}


void XclObjChart::WriteAxis()
{
	if( !bWriteMode ) return;
	StartRecord( 0x101D, 18 );			// AXIS
    *pStrm  << nCurrAxis;               // X, Y, Z axis
    pStrm->WriteZeroBytes( 16 );        // reserved
	EndRecord();
}


void XclObjChart::WriteXZRange()
{
    if( eChartType != EXC_CHART_SCATTER )
	{
		WriteCatserrange();
		if( !bChart3D )
			WriteAxcext();
	}
}


void XclObjChart::WriteCatserrange()
{
	if( !bWriteMode ) return;

	UINT16 nGrbit = 0x0000;
    if( bChartTypeBars || (eChartType == EXC_CHART_STOCK) )
        nGrbit |= EXC_CHLABELRANGE_BETWEEN;
    if( nCurrAxesSet == EXC_CHAXESSET_SECONDARY )
        nGrbit |= EXC_CHLABELRANGE_MAXCROSS;

	StartRecord( 0x1020, 8 );	// CATSERRANGE
	*pStrm	<< UINT16(1)		// value axis / category crossing point (2D charts only)
			<< UINT16(1)		// frequency of labels
			<< UINT16(1)		// frequency of tick marks
			<< nGrbit;			// format flags
	EndRecord();
}


void XclObjChart::WriteAxcext()
{
	if( !bWriteMode ) return;
	StartRecord( 0x1062, 18 );					// AXCEXT
	*pStrm	<< UINT16(0)						// minimum category on axis
			<< UINT16(0)						// maximum category on axis
			<< UINT16(1)						// value of major unit
			<< UINT16(0)						// units of major unit
			<< UINT16(1)						// value of minor unit
			<< UINT16(0)						// units of minor unit
			<< UINT16(0)						// base unit of axis
			<< UINT16(0)						// crossing point of value axis (date)
			<< UINT16(EXC_CHART_AXCEXT_FLAGS);	// option flags
	EndRecord();
}


BOOL XclObjChart::WriteTick( const uno::Reference< beans::XPropertySet >& xProp, BOOL bHasDescr )
{
	sal_Int32	nVal32;
    UINT8       nMarkPos        = 0;
    UINT8       nHelpMarkPos    = 0;
    UINT8       nLabel          = EXC_CHTICK_NOLABEL;
	Color		aColor( COL_BLACK );
	UINT16		nIndex;
	UINT16		nFlags			= 0x0000;
	UINT16		nAngle;
	UINT16		nDummy;

	GetTextColor( aColor, nIndex, xProp );		// insert into palette or get index
	if( !bWriteMode ) return FALSE;

    if( nIndex == EXC_COLOR_CHWINDOWTEXT )
        nFlags |= EXC_CHTICK_AUTOCOLOR;

    if( GetPropInt32( nVal32, xProp, EXC_CHPROP_MARKS ) )
		nMarkPos = lcl_GetMarkPos( nVal32 );
    if( GetPropInt32( nVal32, xProp, EXC_CHPROP_HELPMARKS ) )
		nHelpMarkPos = lcl_GetMarkPos( nVal32 );
	if( bHasDescr )
	{
        if( eChartType == EXC_CHART_RADARLINE )
            nLabel = EXC_CHTICK_NEXT;
        else if( nCurrAxesSet == EXC_CHAXESSET_PRIMARY )
            nLabel = EXC_CHTICK_LOW;
		else
            nLabel = EXC_CHTICK_HIGH;
	}
	GetRotation( nDummy, nAngle, xProp );
    nFlags |= EXC_CHART_TICK_GETROTFLAGS( GetRotationFlags( nAngle ) );

	StartRecord( 0x101E, 30 );					// TICK
	*pStrm	<< nMarkPos							// major tick mark
			<< nHelpMarkPos						// minor tick mark
			<< nLabel							// tick label position
            << EXC_CHTICK_TRANSPARENT;          // background mode: transparent
	WriteRGB( aColor.GetColor() );				// tick label text color
	*pStrm	<< UINT32(0) << UINT32(0)			// reserved
			<< UINT32(0) << UINT32(0)			// reserved
			<< nFlags							// display flags
			<< nIndex							// index to text color
			<< nAngle;							// text rotation
	EndRecord();

    return (nMarkPos != 0) || (nHelpMarkPos != 0);
}


void XclObjChart::WriteValuerange( const uno::Reference< beans::XPropertySet >& xProp, BOOL bMaxCross )
{
	if( !bWriteMode ) return;

    UINT16  nGrbit = EXC_CHVALUERANGE_BIT8;        // reserved, but Xcl writes it
    double  fMin = 0.0;
    double  fMax = 0.0;
    double  fMain = 0.0;
    double  fHelp = 0.0;
    double  fOrigin = 0.0;

    if( GetPropBool( xProp, EXC_CHPROP_AUTOMIN ) || !GetPropDouble( fMin, xProp, EXC_CHPROP_MIN ) )
        nGrbit |= EXC_CHVALUERANGE_AUTOMIN;
    if( GetPropBool( xProp, EXC_CHPROP_AUTOMAX ) || !GetPropDouble( fMax, xProp, EXC_CHPROP_MAX ) )
        nGrbit |= EXC_CHVALUERANGE_AUTOMAX;
    if( GetPropBool( xProp, EXC_CHPROP_AUTOSTMAIN ) || !GetPropDouble( fMain, xProp, EXC_CHPROP_STEPMAIN ) )
        nGrbit |= EXC_CHVALUERANGE_AUTOMAJOR;
    if( GetPropBool( xProp, EXC_CHPROP_AUTOSTHELP ) || !GetPropDouble( fHelp, xProp, EXC_CHPROP_STEPHELP ) )
        nGrbit |= EXC_CHVALUERANGE_AUTOMINOR;
    if( GetPropBool( xProp, EXC_CHPROP_AUTOORIG ) || !GetPropDouble( fOrigin, xProp, EXC_CHPROP_ORIGIN ) )
        nGrbit |= EXC_CHVALUERANGE_AUTOCROSS;
    if( GetPropBool( xProp, EXC_CHPROP_LOG ) )
        nGrbit |= EXC_CHVALUERANGE_LOGSCALE;
    if( bMaxCross )
        nGrbit |= EXC_CHVALUERANGE_MAXCROSS;

	StartRecord( 0x101F, 42 );	// VALUERANGE
	*pStrm	<< fMin				// minimum value on axis
			<< fMax				// maximum value on axis
			<< fMain			// value of major increment
			<< fHelp			// value of minor increment
			<< fOrigin			// value where the category axis crosses
			<< nGrbit;			// format flags
	EndRecord();
}


void XclObjChart::WriteIfmt( const uno::Reference< beans::XPropertySet >& xProp )
{
	sal_Int32 nIndex;
    if( GetPropInt32( nIndex, xProp, EXC_CHPROP_NUMFMT ) )
	{
        UINT16 nExcIndex = rRootData.pER->GetNumFmtBuffer().Insert( nIndex );

		if( bWriteMode )
			WriteUINT16Record( 0x104E, nExcIndex );	// IFMT
	}
}


void XclObjChart::WriteTheAxisline( const uno::Reference< beans::XPropertySet >& xProp, UINT16 nId, BOOL bTicks )
{
	if( !xProp.is() ) return;
	WriteAxislineformat( nId );
    WriteLineformat( xProp, bTicks ? EXC_CHLINEFORMAT_SHOWAXIS : 0x0000 );
}


void XclObjChart::WriteTheAxisframe( const uno::Reference< beans::XPropertySet >& xProp, UINT16 nId )
{
	if( !xProp.is() ) return;
	WriteAxislineformat( nId );
	WriteLineformat( xProp );
	WriteAreaformat( xProp );
}


void XclObjChart::WriteAxislineformat( UINT16 nId )
{
	if( bWriteMode )
		WriteUINT16Record( 0x1021, nId );	// AXISLINEFORMAT
}


//___________________________________________________________________
// chart type formats

void XclObjChart::WriteGroupChartformat()
{
    WriteChartformat( nCurrAxesSet );
	WriteBeginLevel();

    if( bWriteMode )
    {
        switch( eChartType )
        {
            case EXC_CHART_LINE:        WriteLine();        break;
            case EXC_CHART_AREA:        WriteArea();        break;
            case EXC_CHART_BAR:
            case EXC_CHART_COLUMN:      WriteBar();         break;
            case EXC_CHART_PIE:
            case EXC_CHART_DONUT:       WritePie();         break;
            case EXC_CHART_SCATTER:     WriteScatter();     break;
            case EXC_CHART_RADARLINE:   WriteRadar();       break;
            case EXC_CHART_STOCK:
                if ( bStockVolume )
                    WriteBar();
                else
                    WriteLine();
            break;
            default:            WriteBar();
        }

        if( bChart3D )
            Write3D();
    }

    if( nCurrAxesSet == EXC_CHAXESSET_PRIMARY )
	{
		WriteGroupLegend();

        if( (eChartType == EXC_CHART_STOCK) && !bStockVolume )
		{
			WriteTheDropbars();
            WriteTheChartline( EXC_CHCHARTLINE_HILO );
		}
	}
    else if( eChartType == EXC_CHART_STOCK )
	{
		WriteLine();
        WriteSerieslist( nCurrAxesSet );
		WriteTheDropbars();
        WriteTheChartline( EXC_CHCHARTLINE_HILO );
	}

    // global formats
    GetFormats( aDiaFormats, xDiaProp, TRUE, FALSE );
    WriteGroupDataformat( 0, EXC_CHDATAFORMAT_DEFAULT, 0, aDiaFormats );

	WriteEndLevel();
}


void XclObjChart::WriteChartformat( UINT16 nZOrder )
{
	if( !bWriteMode ) return;

    UINT16 nGrbit = bChartTypeRound ? EXC_CHTYPEGROUP_VARIED : 0x0000;
	StartRecord( 0x1014, 20 );	// CHARTFORMAT
	*pStrm 	<< UINT32(0)		// reserved
			<< UINT32(0)		// reserved
			<< UINT32(0)		// reserved
			<< UINT32(0)		// reserved
			<< nGrbit			// format flags
			<< nZOrder;			// drawing order, 0:= bottom of the z-order
	EndRecord();
}


void XclObjChart::Write3D()
{
	UINT16	nRot	= bChartTypeRound ? 0 : 30;
	UINT16	nElev	= 20;
	UINT16	nDist	= 30;
	UINT16	nHeight	= 100;
	UINT16	nDepth	= 100;
	UINT16	nGap	= 150;
    UINT16  nGrbit  = EXC_CHCHART3D_PERSP | EXC_CHCHART3D_AUTOSCALE | EXC_CHCHART3D_BIT4;
	if( bChartTypeBars && !bChartStacked && !bChartPercent && !bChartDeep )
        nGrbit |= EXC_CHCHART3D_CLUSTER;

	StartRecord( 0x103A, 14 );	// 3D
	*pStrm	<< nRot				// rotation angle (0 to 360 degrees)
			<< nElev			// elevation angle (-90 to +90 degrees)
			<< nDist			// distance from eye to chart (0 to 100)
			<< nHeight			// height of plot volume relative to width and depth
			<< nDepth			// depth of points relative to width
			<< nGap				// space between series
			<< nGrbit;			// option flags
	EndRecord();
}


void XclObjChart::WriteTheChartline( UINT16 nId )
{
    if( nId == EXC_CHCHARTLINE_HILO )
	{
		uno::Reference< chart::XStatisticDisplay > xStat( xDiagram, uno::UNO_QUERY );
		if( !xStat.is() ) return;
		uno::Reference< beans::XPropertySet > xProp( xStat->getMinMaxLine() );
		if( !xProp.is() ) return;

		WriteChartline( nId );
		WriteLineformat( xProp );
	}
}


void XclObjChart::WriteChartline( UINT16 nId )
{
	if( bWriteMode )
		WriteUINT16Record( 0x101C, nId );	// CHARTLINE
}


void XclObjChart::WriteTheDropbars()
{
	if( bStockUpDown )
	{
        WriteGroupDropbar( EXC_CHDROPBAR_UP );
        WriteGroupDropbar( EXC_CHDROPBAR_DOWN );
	}
}


void XclObjChart::WriteGroupDropbar( UINT16 nBar )
{
	uno::Reference< chart::XStatisticDisplay > xStat( xDiagram, uno::UNO_QUERY );
	if( !xStat.is() ) return;

	uno::Reference< beans::XPropertySet > xProp;
	switch( nBar )
	{
        case EXC_CHDROPBAR_UP:      xProp = xStat->getUpBar();      break;
        case EXC_CHDROPBAR_DOWN:    xProp = xStat->getDownBar();    break;
	}
	if( !xProp.is() ) return;

	WriteDropbar();
	WriteBeginLevel();
	WriteLineformat( xProp );
	WriteAreaformat( xProp );
	WriteEndLevel();
}


void XclObjChart::WriteDropbar()
{
	if( bWriteMode )
		WriteUINT16Record( 0x103D, 100 );	// DROPBAR
}


void XclObjChart::WriteSerieslist( UINT16 nAxesSet )
{
	if( !bWriteMode ) return;

	SvUShorts* pList = (SvUShorts*) aSerieslistList.GetObject( nAxesSet );
	if( pList )
	{
		UINT16 nCnt = pList->Count();
		StartRecord( 0x1016, 2 + nCnt * 2 );	// SERIESLIST
		*pStrm	<< nCnt;
		for ( UINT16 j = 0; j < nCnt; j++ )
			*pStrm << UINT16( (*pList)[j] );
		EndRecord();
	}
}


//___________________________________________________________________
// chart types

void XclObjChart::WriteLineArea( UINT16 nRecId )
{
	if( !bWriteMode ) return;

	UINT16 nGrbit = 0x0000;
	if( bChartStacked )
        nGrbit |= EXC_CHLINE_STACKED;
	if( bChartPercent )
        nGrbit |= EXC_CHLINE_PERCENT;

	WriteUINT16Record( nRecId, nGrbit );	// LINE or AREA
}


void XclObjChart::WriteBar()
{
	if( !bWriteMode ) return;

    UINT16 nGrbit = (eChartType == EXC_CHART_BAR) ? EXC_CHBAR_HORIZONTAL : 0x0000;
	if( bChartStacked )
        nGrbit |= EXC_CHBAR_STACKED;
	if( bChartPercent )
        nGrbit |= EXC_CHBAR_PERCENT;

	BOOL	bIsOnTop	= (bChartStacked || bChartPercent);
	INT16	nOverlap	= bIsOnTop ? -100 : 0;
	INT16	nGap		= 50;

	uno::Reference< chart::XAxisYSupplier > xDiaYSup( xDiagram, uno::UNO_QUERY );
	if ( xDiaYSup.is() )
	{
		uno::Reference< beans::XPropertySet >	xYAxis( xDiaYSup->getYAxis() );
		sal_Int32								nVal;

        if ( !bIsOnTop && GetPropInt32( nVal, xYAxis, EXC_CHPROP_OVERLAP ) )
			nOverlap = -INT16(nVal);		//! negative
        if ( GetPropInt32( nVal, xYAxis, EXC_CHPROP_GAPWIDTH ) )
			nGap = INT16(nVal);
	}

	StartRecord( 0x1017, 6 );	// BAR
	*pStrm	<< nOverlap			// space between bars (percent of bar width), default=0
			<< nGap				// space between categories (percent of bar width), default=50
			<< nGrbit;			// format flags
	EndRecord();
}


void XclObjChart::WritePie()
{
	if( !bWriteMode ) return;
	// we have a donut hole radius of the size of one data ring
    UINT16 nHole = (eChartType == EXC_CHART_DONUT) ? (UINT16)(100 / (aSeriesYUpnList.size() + 1)) : 0;
	StartRecord( 0x1019, 6 );	// PIE
	*pStrm	<< UINT16(0)		// angle of the first pie slice expressed in degrees
			<< nHole			// 0:= true pie chart; !0:= size of center hole in a donut chart (as a percentage)
			<< UINT16(0);		// option flags
	EndRecord();
}


void XclObjChart::WriteScatter()
{
	if( !bWriteMode ) return;
	StartRecord( 0x101B, 6 );	// SCATTER
	*pStrm	<< UINT16(100)		// percent of largest bubble compared to chart in general
			<< UINT16(1)		// 1:= bubble size is area; 2:= bubble size is width
			<< UINT16(0);		// option flags
	EndRecord();
}


void XclObjChart::WriteRadar()
{
	if( !bWriteMode ) return;
    UINT16 nGrbit = bYAxisDescr ? EXC_CHRADAR_AXISLABELS : 0x0000;
	WriteUINT16Record( 0x103E, nGrbit );	// RADAR
}


//___________________________________________________________________
// main titel, legend

void XclObjChart::WriteTheMainTitle()
{
    if ( GetPropBool( xDocProp, EXC_CHPROP_HASMAINTIT ) )
		WriteGroupText( xChartDoc->getTitle(), ttTitle );
}


void XclObjChart::WriteGroupLegend()
{
    if( GetPropBool( xDocProp, EXC_CHPROP_HASLEGEND ) )
	{
		uno::Reference< drawing::XShape > xLegendShape = xChartDoc->getLegend();

		WriteLegend( xLegendShape );
		WriteBeginLevel();
		WritePos( xLegendShape, ttLegend );		// needed for user defined position
		WriteGroupText( xLegendShape, ttLegend );
		WriteGroupFrame( xLegendShape );
		WriteEndLevel();
	}
}


void XclObjChart::WriteLegend( const uno::Reference< drawing::XShape >& xShape )
{
	if( !bWriteMode ) return;

	uno::Reference< beans::XPropertySet >	xProp( xShape, uno::UNO_QUERY );
    UINT8                                   nPos    = EXC_CHLEGEND_NOTDOCKED;
	UINT16									nFlags	= 0x0000;

//--------------------------
//Chart doesn't return "chart::ChartLegendPosition_NONE"
//-> results in wrong position in Excel, if moved manually in Calc
//	if( xProp.is() )
//      if( GetPropValue( xProp, EXC_CHPROP_ALIGNMENT ) && (aAny >>= ePos) )
//			nPos = lcl_GetLegendPosition( ePos );
//--------------------------

// #90643# diagram/legend/title positioning disabled
//    chart::ChartLegendPosition ePos;
//    if( !xProp.is() || !GetPropValue( xProp, EXC_CHPROP_ALIGNMENT ) || !(aAny >>= ePos) )
//        ePos = chart::ChartLegendPosition_NONE;
    chart::ChartLegendPosition ePos = chart::ChartLegendPosition_RIGHT;
    if( xProp.is() && GetPropValue( xProp, EXC_CHPROP_ALIGNMENT ) )
        aAny >>= ePos;
    if( ePos == chart::ChartLegendPosition_NONE )
        ePos = chart::ChartLegendPosition_RIGHT;
    nPos = lcl_GetLegendPosition( ePos );

    nFlags |= EXC_CHLEGEND_AUTOSERIES;
    if( (ePos == chart::ChartLegendPosition_LEFT) || (ePos == chart::ChartLegendPosition_RIGHT) || (ePos == chart::ChartLegendPosition_NONE) )
        nFlags |= EXC_CHLEGEND_STACKED;
    if( nPos != EXC_CHLEGEND_NOTDOCKED )
        nFlags |= EXC_CHLEGEND_DOCKED | EXC_CHLEGEND_AUTOPOSX | EXC_CHLEGEND_AUTOPOSY;

	StartRecord( 0x1015, 20 );						// LEGEND
	WritePosData( xShape, ttCommon );
	*pStrm	<< nPos									// default position
            << EXC_CHLEGEND_MEDIUM                  // spacing, always medium
			<< nFlags;								// flags
	EndRecord();
}


//___________________________________________________________________
// SIINDEX

void XclObjChart::WriteTheSiindex()
{
	if( !bWriteMode ) return;
//2do: what the .... are they for?
#if 0
	WriteSiindex( 2 );
	WriteSiindex( 1 );
	WriteSiindex( 3 );
#endif
}


void XclObjChart::WriteSiindex( UINT16 nIndex )
{
	if( bWriteMode )
		WriteUINT16Record( 0x1065, nIndex );	// SIINDEX
}

