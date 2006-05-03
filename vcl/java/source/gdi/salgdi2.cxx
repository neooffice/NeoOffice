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

#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALBMP_H
#include <salbmp.h>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLIMAGE_HXX
#include <com/sun/star/vcl/VCLImage.hxx>
#endif

using namespace vcl;

// =======================================================================

void JavaSalGraphics::copyBits( const SalTwoRect* pPosAry, SalGraphics* pSrcGraphics )
{
	JavaSalGraphics *pJavaSrcGraphics = (JavaSalGraphics *)pSrcGraphics;
	if ( !pJavaSrcGraphics )
		pJavaSrcGraphics = this;

	mpVCLGraphics->copyBits( pJavaSrcGraphics->mpVCLGraphics, pPosAry->mnSrcX, pPosAry->mnSrcY, pPosAry->mnSrcWidth, pPosAry->mnSrcHeight, pPosAry->mnDestX, pPosAry->mnDestY, pPosAry->mnDestWidth, pPosAry->mnDestHeight );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth, long nSrcHeight, USHORT nFlags )
{
	mpVCLGraphics->copyBits( mpVCLGraphics, nSrcX, nSrcY, nSrcWidth, nSrcHeight, nDestX, nDestY, nSrcWidth, nSrcHeight );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap )
{
	JavaSalBitmap *pJavaSalBitmap = (JavaSalBitmap *)&rSalBitmap;

	SalTwoRect aPosAry;
	memcpy( &aPosAry, pPosAry, sizeof( SalTwoRect ) );

	// Scale the bitmap if necessary
	JavaSalBitmap aJavaSalBitmap;
	if ( mpPrinter || aPosAry.mnSrcWidth != aPosAry.mnDestWidth || aPosAry.mnSrcHeight != aPosAry.mnDestHeight )
	{
		BitmapBuffer *pSrcBuffer = pJavaSalBitmap->AcquireBuffer( TRUE );
		if ( pSrcBuffer )
		{
			BitmapBuffer *pDestBuffer = StretchAndConvert( *pSrcBuffer, *pPosAry, JavaSalBitmap::Get32BitNativeFormat() | BMP_FORMAT_TOP_DOWN );
			pJavaSalBitmap->ReleaseBuffer( pSrcBuffer, TRUE );
			if ( pDestBuffer )
			{
				// Don't delete the bitmap buffer and let the Java native
				// method print the bitmap buffer directly
				if ( mpPrinter )
				{
					aPosAry.mnSrcX = 0;
					aPosAry.mnSrcY = 0;
					aPosAry.mnSrcWidth = pDestBuffer->mnWidth;
					aPosAry.mnSrcHeight = pDestBuffer->mnHeight;
					mpVCLGraphics->drawBitmapBuffer( pDestBuffer, aPosAry.mnSrcX, aPosAry.mnSrcY, aPosAry.mnSrcWidth, aPosAry.mnSrcHeight, aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight );
					return;
				}

				if ( aJavaSalBitmap.Create( pDestBuffer ) )
				{
					aPosAry.mnSrcX = 0;
					aPosAry.mnSrcY = 0;
					aPosAry.mnSrcWidth = pDestBuffer->mnWidth;
					aPosAry.mnSrcHeight = pDestBuffer->mnHeight;
					pJavaSalBitmap = &aJavaSalBitmap;
				}
				else if ( pDestBuffer->mpBits )
				{
					delete[] pDestBuffer->mpBits;
				}

				delete pDestBuffer;
			}
		}
	}

	com_sun_star_vcl_VCLBitmap *pVCLBitmap = pJavaSalBitmap->GetVCLBitmap();
	if ( pVCLBitmap )
	{
		mpVCLGraphics->drawBitmap( pVCLBitmap, aPosAry.mnSrcX, aPosAry.mnSrcY, aPosAry.mnSrcWidth, aPosAry.mnSrcHeight, aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight );
		pJavaSalBitmap->ReleaseVCLBitmap( pVCLBitmap, false );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap, SalColor nTransparentColor )
{
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap, const SalBitmap& rTransparentBitmap )
{
	// Don't do anything if this is a printer
	if ( mpPrinter )
		return;

	JavaSalBitmap *pJavaSalBitmap = (JavaSalBitmap *)&rSalBitmap;
	JavaSalBitmap *pTransJavaSalBitmap = (JavaSalBitmap *)&rTransparentBitmap;

	SalTwoRect aPosAry;
	memcpy( &aPosAry, pPosAry, sizeof( SalTwoRect ) );

	// Scale the bitmap if necessary
	JavaSalBitmap aJavaSalBitmap;
	JavaSalBitmap aTransJavaSalBitmap;
	if ( aPosAry.mnSrcWidth != aPosAry.mnDestWidth || aPosAry.mnSrcHeight != aPosAry.mnDestHeight )
	{
		BitmapBuffer *pSrcBuffer = pJavaSalBitmap->AcquireBuffer( TRUE );
		if ( pSrcBuffer )
		{
			BitmapBuffer *pDestBuffer = StretchAndConvert( *pSrcBuffer, aPosAry, JavaSalBitmap::Get32BitNativeFormat() | BMP_FORMAT_TOP_DOWN );
			pJavaSalBitmap->ReleaseBuffer( pSrcBuffer, TRUE );
			if ( pDestBuffer )
			{
				if ( aJavaSalBitmap.Create( pDestBuffer ) )
				{
					BitmapBuffer *pTransSrcBuffer = pTransJavaSalBitmap->AcquireBuffer( TRUE );
					if ( pTransSrcBuffer )
					{
						BitmapBuffer *pTransDestBuffer = StretchAndConvert( *pTransSrcBuffer, aPosAry, JavaSalBitmap::Get32BitNativeFormat() | BMP_FORMAT_TOP_DOWN );
						pTransJavaSalBitmap->ReleaseBuffer( pTransSrcBuffer, TRUE );
						if ( pTransDestBuffer )
						{
							if ( aTransJavaSalBitmap.Create( pTransDestBuffer ) )
							{
								aPosAry.mnSrcX = 0;
								aPosAry.mnSrcY = 0;
								aPosAry.mnSrcWidth = pTransDestBuffer->mnWidth;
								aPosAry.mnSrcHeight = pTransDestBuffer->mnHeight;
								pJavaSalBitmap = &aJavaSalBitmap;
								pTransJavaSalBitmap = &aTransJavaSalBitmap;
							}
							else if ( pTransDestBuffer->mpBits )
							{
								delete[] pTransDestBuffer->mpBits;
							}

							delete pTransDestBuffer;
						}
					}
				}
				else if ( pDestBuffer->mpBits )
				{
					delete[] pDestBuffer->mpBits;
				}

				delete pDestBuffer;
			}
		}
	}

	com_sun_star_vcl_VCLBitmap *pVCLBitmap = pJavaSalBitmap->GetVCLBitmap();
	if ( pVCLBitmap )
	{
		com_sun_star_vcl_VCLBitmap *pTransVCLBitmap = pTransJavaSalBitmap->GetVCLBitmap();
		if ( pTransVCLBitmap )
		{
			mpVCLGraphics->drawBitmap( pVCLBitmap, pTransVCLBitmap, aPosAry.mnSrcX, aPosAry.mnSrcY, aPosAry.mnSrcWidth, aPosAry.mnSrcHeight, aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight );
			pTransJavaSalBitmap->ReleaseVCLBitmap( pTransVCLBitmap, false );
		}
		pJavaSalBitmap->ReleaseVCLBitmap( pVCLBitmap, false );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawMask( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap, SalColor nMaskColor )
{
	// Don't do anything if this is a printer
	if ( mpPrinter )
		return;

	JavaSalBitmap *pJavaSalBitmap = (JavaSalBitmap *)&rSalBitmap;

	SalTwoRect aPosAry;
	memcpy( &aPosAry, pPosAry, sizeof( SalTwoRect ) );

	// Scale the bitmap if necessary
	JavaSalBitmap aJavaSalBitmap;
	if ( aPosAry.mnSrcWidth != aPosAry.mnDestWidth || aPosAry.mnSrcHeight != aPosAry.mnDestHeight )
	{
		BitmapBuffer *pSrcBuffer = pJavaSalBitmap->AcquireBuffer( TRUE );
		if ( pSrcBuffer )
		{
			BitmapBuffer *pDestBuffer = StretchAndConvert( *pSrcBuffer, *pPosAry, JavaSalBitmap::Get32BitNativeFormat() | BMP_FORMAT_TOP_DOWN );
			pJavaSalBitmap->ReleaseBuffer( pSrcBuffer, TRUE );
			if ( pDestBuffer )
			{
				if ( aJavaSalBitmap.Create( pDestBuffer ) )
				{
					aPosAry.mnSrcX = 0;
					aPosAry.mnSrcY = 0;
					aPosAry.mnSrcWidth = pDestBuffer->mnWidth;
					aPosAry.mnSrcHeight = pDestBuffer->mnHeight;
					pJavaSalBitmap = &aJavaSalBitmap;
				}
				else if ( pDestBuffer->mpBits )
				{
					delete[] pDestBuffer->mpBits;
				}

				delete pDestBuffer;
			}
		}
	}

	com_sun_star_vcl_VCLBitmap *pVCLBitmap = pJavaSalBitmap->GetVCLBitmap();
	if ( pVCLBitmap )
	{
		mpVCLGraphics->drawMask( pVCLBitmap, nMaskColor | 0xff000000, aPosAry.mnSrcX, aPosAry.mnSrcY, aPosAry.mnSrcWidth, aPosAry.mnSrcHeight, aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight );
		pJavaSalBitmap->ReleaseVCLBitmap( pVCLBitmap, false );
	}
}

// -----------------------------------------------------------------------

SalBitmap* JavaSalGraphics::getBitmap( long nX, long nY, long nDX, long nDY )
{
	// Don't do anything if this is a printer
	if ( mpPrinter )
		return NULL;

	// Normalize rectangle
	nDX = abs( nDX );
	nDY = abs( nDY );

	JavaSalBitmap *pBitmap = new JavaSalBitmap();

	if ( !pBitmap->Create( Size( nDX, nDY ), GetBitCount(), BitmapPalette() ) )
	{
		delete pBitmap;
		pBitmap = NULL;
	}

	if ( pBitmap )
	{
		com_sun_star_vcl_VCLBitmap *pVCLBitmap = pBitmap->GetVCLBitmap();
		if ( pVCLBitmap )
		{
			pVCLBitmap->copyBits( mpVCLGraphics, nX, nY, nDX, nDY, 0, 0 );
			pBitmap->ReleaseVCLBitmap( pVCLBitmap, true );
		}
	}

	return pBitmap;
}

// -----------------------------------------------------------------------

SalColor JavaSalGraphics::getPixel( long nX, long nY )
{
	return mpVCLGraphics->getPixel( nX, nY );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags )
{
	mpVCLGraphics->invert( nX, nY, nWidth, nHeight, nFlags );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nFlags )
{
	mpVCLGraphics->invert( nPoints, pPtAry, nFlags );
}
