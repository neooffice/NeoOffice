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

#define _SV_SALGDI2_CXX

#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFRAME_HXX
#include <com/sun/star/vcl/VCLFrame.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLIMAGE_HXX
#include <com/sun/star/vcl/VCLImage.hxx>
#endif

using namespace vcl;

// =======================================================================

void SalGraphics::CopyBits( const SalTwoRect* pPosAry, SalGraphics* pSrcGraphics )
{
	if ( !pSrcGraphics )
		pSrcGraphics = this;

	maGraphicsData.mpVCLGraphics->copyBits( pSrcGraphics->maGraphicsData.mpVCLGraphics, pPosAry->mnSrcX, pPosAry->mnSrcY, pPosAry->mnSrcWidth, pPosAry->mnSrcHeight, pPosAry->mnDestX, pPosAry->mnDestY );
}

// -----------------------------------------------------------------------

void SalGraphics::CopyArea( long nDestX, long nDestY,
							long nSrcX, long nSrcY,
							long nSrcWidth, long nSrcHeight,
							USHORT nFlags )
{
	maGraphicsData.mpVCLGraphics->copyBits( maGraphicsData.mpVCLGraphics, nSrcX, nSrcY, nSrcWidth, nSrcHeight, nDestX, nDestY);
}

// -----------------------------------------------------------------------

void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
							  const SalBitmap& rSalBitmap )
{
	if ( pPosAry->mnSrcWidth != pPosAry->mnDestWidth || pPosAry->mnSrcHeight != pPosAry->mnDestHeight )
	{
		SalBitmap aBitmap;
		SalTwoRect aPosAry( *pPosAry );
		aPosAry.mnDestX = 0;
		aPosAry.mnDestY = 0;
		aBitmap.Create( rSalBitmap, aPosAry );
		Size rSize = aBitmap.GetSize();
		maGraphicsData.mpVCLGraphics->drawBitmap( aBitmap.mpVCLBitmap, 0, 0, rSize.Width(), rSize.Height(), pPosAry->mnDestX, pPosAry->mnDestY );
	}
	else
	{
		maGraphicsData.mpVCLGraphics->drawBitmap( rSalBitmap.mpVCLBitmap, pPosAry->mnSrcX, pPosAry->mnSrcY, pPosAry->mnSrcWidth, pPosAry->mnSrcHeight, pPosAry->mnDestX, pPosAry->mnDestY );
	}
}

// -----------------------------------------------------------------------

void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
							  const SalBitmap& rSalBitmap,
							  SalColor nTransparentColor )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::DrawBitmap #2 not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
							  const SalBitmap& rSalBitmap,
							  const SalBitmap& rTransparentBitmap )
{
	if ( pPosAry->mnSrcWidth != pPosAry->mnDestWidth || pPosAry->mnSrcHeight != pPosAry->mnDestHeight )
	{
		SalBitmap aBitmap;
		SalBitmap aTransBitmap;
		SalTwoRect aPosAry( *pPosAry );
		aPosAry.mnDestX = 0;
		aPosAry.mnDestY = 0;
		aBitmap.Create( rSalBitmap, aPosAry );
		aTransBitmap.Create( rTransparentBitmap, aPosAry );
		Size rSize = aBitmap.GetSize();
		maGraphicsData.mpVCLGraphics->drawBitmap( aBitmap.mpVCLBitmap, aTransBitmap.mpVCLBitmap, 0, 0, rSize.Width(), rSize.Height(), pPosAry->mnDestX, pPosAry->mnDestY );
	}
	else
	{
		maGraphicsData.mpVCLGraphics->drawBitmap( rSalBitmap.mpVCLBitmap, rTransparentBitmap.mpVCLBitmap, pPosAry->mnSrcX, pPosAry->mnSrcY, pPosAry->mnSrcWidth, pPosAry->mnSrcHeight, pPosAry->mnDestX, pPosAry->mnDestY );
	}

}

// -----------------------------------------------------------------------

void SalGraphics::DrawMask( const SalTwoRect* pPosAry,
							const SalBitmap& rSalBitmap,
							SalColor nMaskColor )
{
	if ( pPosAry->mnSrcWidth != pPosAry->mnDestWidth || pPosAry->mnSrcHeight != pPosAry->mnDestHeight )
	{
		SalBitmap aBitmap;
		SalTwoRect aPosAry( *pPosAry );
		aPosAry.mnDestX = 0;
		aPosAry.mnDestY = 0;
		aBitmap.Create( rSalBitmap, aPosAry );
		Size rSize = aBitmap.GetSize();
		maGraphicsData.mpVCLGraphics->drawMask( aBitmap.mpVCLBitmap, nMaskColor, 0, 0, rSize.Width(), rSize.Height(), pPosAry->mnDestX, pPosAry->mnDestY );
	}
	else
	{
		maGraphicsData.mpVCLGraphics->drawMask( rSalBitmap.mpVCLBitmap, nMaskColor, pPosAry->mnSrcX, pPosAry->mnSrcY, pPosAry->mnSrcWidth, pPosAry->mnSrcHeight, pPosAry->mnDestX, pPosAry->mnDestY );
	}
}

// -----------------------------------------------------------------------

SalBitmap* SalGraphics::GetBitmap( long nX, long nY, long nDX, long nDY )
{
	// Don't do anything if this is a printer
	if ( maGraphicsData.mpPrinter )
		return NULL;

	SalBitmap *pBitmap = new SalBitmap();

	if ( !pBitmap->Create( Size( nDX, nDY ), GetBitCount(), BitmapPalette() ) )
	{
		delete pBitmap;
		pBitmap = NULL;
	}

	if ( pBitmap )
		pBitmap->mpVCLBitmap->copyBits( maGraphicsData.mpVCLGraphics, nX, nY, nDX, nDY, 0, 0 );

	return pBitmap;
}

// -----------------------------------------------------------------------

SalColor SalGraphics::GetPixel( long nX, long nY )
{
	return maGraphicsData.mpVCLGraphics->getPixel( nX, nY );
}

// -----------------------------------------------------------------------

void SalGraphics::Invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags )
{
	maGraphicsData.mpVCLGraphics->invert( nX, nY, nWidth, nHeight, nFlags );
}

// -----------------------------------------------------------------------

void SalGraphics::Invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nFlags )
{
	long pXPoints[ nPoints + 1 ];
	long pYPoints[ nPoints + 1 ];
	for ( ULONG i = 0; i < nPoints; i++ )
	{
		pXPoints[ i ] = pPtAry->mnX;
		pYPoints[ i ] = pPtAry->mnY;
		pPtAry++;
	}

	maGraphicsData.mpVCLGraphics->invert( nPoints, pXPoints, pYPoints, nFlags );
}
