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
}

// -----------------------------------------------------------------------

SalGraphics::~SalGraphics()
{
}

// -----------------------------------------------------------------------

void SalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
	long nDPI = com_sun_star_vcl_VCLGraphics::getScreenResolution();
	rDPIX = nDPI;
	rDPIY = nDPI;
}

// -----------------------------------------------------------------------

void SalGraphics::GetScreenFontResolution( long& rDPIX, long& rDPIY )
{
	long nDPI = com_sun_star_vcl_VCLGraphics::getScreenResolution();
	rDPIX = nDPI;
	rDPIY = nDPI;
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

BOOL SalGraphics::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
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
	maGraphicsData.mnLineColor = 0xffffffff;
}

// -----------------------------------------------------------------------

void SalGraphics::SetLineColor( SalColor nSalColor )
{
	maGraphicsData.mnLineColor = nSalColor;
}

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor()
{
	maGraphicsData.mnFillColor = 0xffffffff;
}

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor( SalColor nSalColor )
{
	maGraphicsData.mnFillColor = nSalColor;
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

void SalGraphics::DrawPixel( long nX, long nY )
{
	if ( maGraphicsData.mnFillColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->setPixel( nX, nY, maGraphicsData.mnFillColor );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long nX, long nY, SalColor nSalColor )
{
	maGraphicsData.mpVCLGraphics->setPixel( nX, nY, nSalColor );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawLine( long nX1, long nY1, long nX2, long nY2 )
{
	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawLine( nX1, nY1, nX2, nY2, maGraphicsData.mnLineColor );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawRect( long nX, long nY, long nWidth, long nHeight )
{
	if ( maGraphicsData.mnFillColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, maGraphicsData.mnFillColor, TRUE );
	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, maGraphicsData.mnLineColor, FALSE );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyLine( ULONG nPoints, const SalPoint* pPtAry )
{
	long pXPoints[ nPoints + 1 ];
	long pYPoints[ nPoints + 1 ];
	for ( ULONG i = 0; i < nPoints; i++ )
	{
		pXPoints[ i ] = pPtAry->mnX;
		pYPoints[ i ] = pPtAry->mnY;
		pPtAry++;
	}

	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawPolyline( nPoints, pXPoints, pYPoints, maGraphicsData.mnLineColor );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
	long pXPoints[ nPoints + 1 ];
	long pYPoints[ nPoints + 1 ];
	for ( ULONG i = 0; i < nPoints; i++ )
	{
		pXPoints[ i ] = pPtAry->mnX;
		pYPoints[ i ] = pPtAry->mnY;
		pPtAry++;
	}
	// Close the points
	pXPoints[ nPoints ] = pXPoints[ 0 ];
	pYPoints[ nPoints ] = pYPoints[ 0 ];
	
	if ( maGraphicsData.mnFillColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawPolygon( nPoints, pXPoints, pYPoints, maGraphicsData.mnFillColor, TRUE );
	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawPolygon( nPoints, pXPoints, pYPoints, maGraphicsData.mnLineColor, FALSE );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyPolygon( ULONG nPoly, const ULONG* pPoints,
								   PCONSTSALPOINT* pPtAry )
{
	for ( ULONG i = 0; i < nPoly; i++ )
		DrawPolygon( pPoints[i], pPtAry[i] );
}


// -----------------------------------------------------------------------

BOOL SalGraphics::DrawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::DrawEPS not implemented\n" );
#endif
	return FALSE;
}

// =======================================================================

SalGraphicsData::SalGraphicsData()
{
	mnFillColor = MAKE_SALCOLOR( 0xff, 0xff, 0xff );
	mnLineColor = MAKE_SALCOLOR( 0, 0, 0 );
	mnTextColor = MAKE_SALCOLOR( 0, 0, 0 );
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
}
