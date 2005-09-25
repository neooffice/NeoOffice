/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU General Public License Version 2.1
 *
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 ************************************************************************/

#define _SV_SALGDI_CXX

#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif

using namespace vcl;

// =======================================================================

SalGraphics::SalGraphics()
{
	GetSalData()->maGraphicsList.push_back( this );
}

// -----------------------------------------------------------------------

SalGraphics::~SalGraphics()
{
	GetSalData()->maGraphicsList.remove( this );
}

// -----------------------------------------------------------------------

void SalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
	Size aSize( maGraphicsData.mpVCLGraphics->getResolution() );
	rDPIX = aSize.Width();
	rDPIY = aSize.Height();
}

// -----------------------------------------------------------------------

void SalGraphics::GetScreenFontResolution( long& rDPIX, long& rDPIY )
{
	Size aSize( maGraphicsData.mpVCLGraphics->getScreenFontResolution() );
	rDPIX = aSize.Width();
	rDPIY = aSize.Height();
}

// -----------------------------------------------------------------------

USHORT SalGraphics::GetBitCount()
{
	return maGraphicsData.mpVCLGraphics->getBitCount();
}

// -----------------------------------------------------------------------

void SalGraphics::ResetClipRegion()
{
	maGraphicsData.mpVCLGraphics->resetClipRegion();
}

// -----------------------------------------------------------------------

void SalGraphics::BeginSetClipRegion( ULONG nRectCount )
{
	maGraphicsData.mpVCLGraphics->beginSetClipRegion();
}

// -----------------------------------------------------------------------

BOOL SalGraphics::UnionClipRegion( long nX, long nY, long nWidth, long nHeight,
                                   const OutputDevice *pOutDev )
{
	maGraphicsData.mpVCLGraphics->unionClipRegion( nX, nY, nWidth, nHeight );
	return TRUE;
}

// -----------------------------------------------------------------------

void SalGraphics::EndSetClipRegion()
{
	maGraphicsData.mpVCLGraphics->endSetClipRegion();
}

// -----------------------------------------------------------------------

void SalGraphics::SetLineColor()
{
	maGraphicsData.mnLineColor = 0x00000000;
}

// -----------------------------------------------------------------------

void SalGraphics::SetLineColor( SalColor nSalColor )
{
	maGraphicsData.mnLineColor = nSalColor | 0xff000000;
}

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor()
{
	maGraphicsData.mnFillColor = 0x00000000;
}

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor( SalColor nSalColor )
{
	maGraphicsData.mnFillColor = nSalColor | 0xff000000;
}

// -----------------------------------------------------------------------

void SalGraphics::SetXORMode( BOOL bSet )
{
	maGraphicsData.mpVCLGraphics->setXORMode( bSet );
}

// -----------------------------------------------------------------------

void SalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
	if ( nROPColor == SAL_ROP_0 )
		SetLineColor( MAKE_SALCOLOR( 0, 0, 0 ) );
	else
		SetLineColor( MAKE_SALCOLOR( 0xff, 0xff, 0xff ) );
}

// -----------------------------------------------------------------------

void SalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
	if ( nROPColor == SAL_ROP_0 )
		SetFillColor( MAKE_SALCOLOR( 0, 0, 0 ) );
	else
		SetFillColor( MAKE_SALCOLOR( 0xff, 0xff, 0xff ) );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long nX, long nY, const OutputDevice *pOutDev )
{
	if ( maGraphicsData.mnLineColor )
		maGraphicsData.mpVCLGraphics->setPixel( nX, nY, maGraphicsData.mnLineColor );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long nX, long nY, SalColor nSalColor,
                             const OutputDevice *pOutDev )
{
	maGraphicsData.mpVCLGraphics->setPixel( nX, nY, nSalColor | 0xff000000 );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawLine( long nX1, long nY1, long nX2, long nY2,
                             const OutputDevice *pOutDev )
{
	if ( maGraphicsData.mnLineColor )
		maGraphicsData.mpVCLGraphics->drawLine( nX1, nY1, nX2, nY2, maGraphicsData.mnLineColor );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawRect( long nX, long nY, long nWidth, long nHeight,
                            const OutputDevice *pOutDev )
{
	if ( maGraphicsData.mnFillColor )
		maGraphicsData.mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, maGraphicsData.mnFillColor, TRUE );
	if ( maGraphicsData.mnLineColor )
		maGraphicsData.mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, maGraphicsData.mnLineColor, FALSE );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyLine( ULONG nPoints, const SalPoint* pPtAry,
                                const OutputDevice *pOutDev )
{
	if ( maGraphicsData.mnLineColor )
		maGraphicsData.mpVCLGraphics->drawPolyline( nPoints, pPtAry, maGraphicsData.mnLineColor );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolygon( ULONG nPoints, const SalPoint* pPtAry,
                               const OutputDevice *pOutDev )
{
	if ( maGraphicsData.mnFillColor )
		maGraphicsData.mpVCLGraphics->drawPolygon( nPoints, pPtAry, maGraphicsData.mnFillColor, TRUE );
	if ( maGraphicsData.mnLineColor )
		maGraphicsData.mpVCLGraphics->drawPolygon( nPoints, pPtAry, maGraphicsData.mnLineColor, FALSE );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyPolygon( ULONG nPoly, const ULONG* pPoints,
								   PCONSTSALPOINT* pPtAry,
                                   const OutputDevice *pOutDev )
{
	if ( maGraphicsData.mnFillColor )
		maGraphicsData.mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pPtAry, maGraphicsData.mnFillColor, TRUE );
	if ( maGraphicsData.mnLineColor )
		maGraphicsData.mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pPtAry, maGraphicsData.mnLineColor, FALSE);
}

// -----------------------------------------------------------------------

sal_Bool SalGraphics::DrawPolyLineBezier( ULONG nPoints,
                                          const SalPoint* pPtAry,
                                          const BYTE* pFlgAry,
                                          const OutputDevice* )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::DrawPolyLineBezier not implemented\n" );
#endif
	return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool SalGraphics::DrawPolygonBezier( ULONG nPoints,
                                         const SalPoint* pPtAry,
                                         const BYTE* pFlgAry,
                                         const OutputDevice* )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::DrawPolygonBezier not implemented\n" );
#endif
	return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool SalGraphics::DrawPolyPolygonBezier( ULONG nPoly, const ULONG* nPoints,
                                         const SalPoint* const* pPtAry,
                                         const BYTE* const* pFlgAry,
                                         const OutputDevice* )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::DrawPolyPolygonBezier not implemented\n" );
#endif
	return sal_False;
}

// -----------------------------------------------------------------------

BOOL SalGraphics::DrawEPS( long nX, long nY, long nWidth, long nHeight,
                           void* pPtr, ULONG nSize,
                           const OutputDevice *pOutDev )
{
	if ( maGraphicsData.mpPrinter )
	{
		maGraphicsData.mpVCLGraphics->drawEPS( pPtr, nSize, nX, nY, nWidth, nHeight );
		return TRUE;
	}

	return FALSE;
}

// -----------------------------------------------------------------------

long SalGraphics::GetGraphicsWidth()
{
	if ( maGraphicsData.mpFrame )
		return maGraphicsData.mpFrame->maGeometry.nWidth;
	else
		return 0;
}

// =======================================================================

SalGraphicsData::SalGraphicsData()
{
	mnFillColor = MAKE_SALCOLOR( 0xff, 0xff, 0xff ) | 0xff000000;
	mnLineColor = MAKE_SALCOLOR( 0, 0, 0 ) | 0xff000000;
	mnTextColor = MAKE_SALCOLOR( 0, 0, 0 ) | 0xff000000;
	mpFrame = NULL;
	mpPrinter = NULL;
	mpVirDev = NULL;
	mpVCLGraphics = NULL;
	mpVCLFont = NULL;
}

// -----------------------------------------------------------------------

SalGraphicsData::~SalGraphicsData()
{
	if ( mpVCLFont )
		delete mpVCLFont;

	for ( ::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator it = maFallbackFonts.begin(); it != maFallbackFonts.end(); ++it )
		delete it->second;
}
