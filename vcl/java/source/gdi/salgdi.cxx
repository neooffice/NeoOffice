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
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLPRINTJOB_HXX
#include <com/sun/star/vcl/VCLPrintJob.hxx>
#endif

#ifdef MACOSX

#ifndef _SV_SALPRN_HXX
#include <salprn.hxx>
#endif
#ifndef _SV_JAVA_LANG_CLASS_HXX
#include <java/lang/Class.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#endif	// MACOSX

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

void SalGraphics::DrawPixel( long nX, long nY, const OutputDevice *pOutDev )
{
	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->setPixel( nX, nY, maGraphicsData.mnLineColor );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long nX, long nY, SalColor nSalColor,
                             const OutputDevice *pOutDev )
{
	maGraphicsData.mpVCLGraphics->setPixel( nX, nY, nSalColor );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawLine( long nX1, long nY1, long nX2, long nY2,
                             const OutputDevice *pOutDev )
{
	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawLine( nX1, nY1, nX2, nY2, maGraphicsData.mnLineColor );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawRect( long nX, long nY, long nWidth, long nHeight,
                            const OutputDevice *pOutDev )
{
	if ( maGraphicsData.mnFillColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, maGraphicsData.mnFillColor, TRUE );
	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, maGraphicsData.mnLineColor, FALSE );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyLine( ULONG nPoints, const SalPoint* pPtAry,
                                const OutputDevice *pOutDev )
{
	long pXPoints[ nPoints ];
	long pYPoints[ nPoints ];
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

void SalGraphics::DrawPolygon( ULONG nPoints, const SalPoint* pPtAry,
                               const OutputDevice *pOutDev )
{
	long pXPoints[ nPoints ];
	long pYPoints[ nPoints ];
	for ( ULONG i = 0; i < nPoints; i++ )
	{
		pXPoints[ i ] = pPtAry->mnX;
		pYPoints[ i ] = pPtAry->mnY;
		pPtAry++;
	}
	
	if ( maGraphicsData.mnFillColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawPolygon( nPoints, pXPoints, pYPoints, maGraphicsData.mnFillColor, TRUE );
	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawPolygon( nPoints, pXPoints, pYPoints, maGraphicsData.mnLineColor, FALSE );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyPolygon( ULONG nPoly, const ULONG* pPoints,
								   PCONSTSALPOINT* pPtAry,
                                   const OutputDevice *pOutDev )
{
	long *pXPtsAry[ nPoly ];
	long *pYPtsAry[ nPoly ];
	ULONG i;
	for ( i = 0; i < nPoly; i++ )
	{
		long *pXPts = new long[ pPoints[ i ] ];
		long *pYPts = new long[ pPoints[ i ] ];
		const SalPoint *pPts = pPtAry[ i ];
		for ( ULONG j = 0; j < pPoints[ i ]; j++ )
		{
			pXPts[ j ] = pPts->mnX;
			pYPts[ j ] = pPts->mnY;
			pPts++;
		}
		pXPtsAry[ i ] = pXPts;
		pYPtsAry[ i ] = pYPts;
	}
	
	if ( maGraphicsData.mnFillColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pXPtsAry, pYPtsAry, maGraphicsData.mnFillColor, TRUE );
	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pXPtsAry, pYPtsAry, maGraphicsData.mnLineColor, FALSE);

	for ( i = 0; i < nPoly; i++ )
	{
		delete pXPtsAry[ i ];
		delete pYPtsAry[ i ];
	}
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
	if ( !maGraphicsData.mpPrinter )
		return FALSE;

#ifdef MACOSX
	PMPrintSession pSession = (PMPrintSession)maGraphicsData.mpPrinter->maPrinterData.mpVCLPrintJob->getNativePrintJob();
	if ( pSession )
	{
		if ( PMSessionPostScriptBegin( pSession ) == kPMNoError )
		{
			PMSessionPostScriptData( pSession, (MacOSPtr)pPtr, nSize );
			PMSessionPostScriptEnd( pSession );
		}
		return TRUE;
	}
#else	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::DrawEPS not implemented\n" );
#endif
	return FALSE;
#endif	// MACOSX
}

// -----------------------------------------------------------------------

long SalGraphics::GetGraphicsWidth()
{
	if ( maGraphicsData.mpFrame )
		return maGraphicsData.mpFrame->maGeometry.nWidth;
	else
		return 0;
}

// -----------------------------------------------------------------------

void SalGraphics::SetLineAntialiasing( BOOL bAntialias )
{
	maGraphicsData.mpVCLGraphics->setLineAntialiasing( bAntialias );
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

	for ( ::std::map< int, com_sun_star_vcl_VCLFont* >::iterator it = maFallbackFonts.begin(); it != maFallbackFonts.end(); ++it )
		delete it->second;
}

