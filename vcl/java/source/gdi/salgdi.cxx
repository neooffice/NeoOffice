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
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

using namespace rtl;
using namespace vos;

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
	if ( maGraphicsData.mnLineColor != 0xffffffff )
		maGraphicsData.mpVCLGraphics->setPixel( nX, nY, maGraphicsData.mnLineColor );
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

void SalGraphics::DrawPolygon( ULONG nPoints, const SalPoint* pPtAry )
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
								   PCONSTSALPOINT* pPtAry )
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

BOOL SalGraphics::DrawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize )
{
	if ( !maGraphicsData.mpPrinter )
		return FALSE;

#ifdef MACOSX
	// Test the JVM version and if it is below 1.4, use Carbon printing APIs
	java_lang_Class* pClass = java_lang_Class::forName( OUString::createFromAscii( "java/lang/CharSequence" ) );
	if ( !pClass )
	{
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
	}
	else
	{
		delete pClass;
	}
#else	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::DrawEPS not implemented\n" );
#endif
	return FALSE;
#endif	// MACOSX
}

// -----------------------------------------------------------------------

void SalGraphics::SetAntialias( BOOL bAntialias )
{
	maGraphicsData.mpVCLGraphics->setAntialias( bAntialias );
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

