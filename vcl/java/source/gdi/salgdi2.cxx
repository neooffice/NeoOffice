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
#ifndef _SV_SALWTYPE_HXX
#include <salwtype.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENT_HXX
#include <com/sun/star/vcl/VCLEvent.hxx>
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

	mpVCLGraphics->copyBits( pJavaSrcGraphics->mpVCLGraphics, pPosAry->mnSrcX, pPosAry->mnSrcY, pPosAry->mnSrcWidth, pPosAry->mnSrcHeight, pPosAry->mnDestX, pPosAry->mnDestY, pPosAry->mnDestWidth, pPosAry->mnDestHeight, sal_True );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth, long nSrcHeight, USHORT nFlags )
{
	mpVCLGraphics->copyBits( mpVCLGraphics, nSrcX, nSrcY, nSrcWidth, nSrcHeight, nDestX, nDestY, nSrcWidth, nSrcHeight, sal_False );
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
	// Don't do anything if this is a printer
	if ( mpPrinter )
		return;

	fprintf( stderr, "Here\n" );
	JavaSalBitmap *pJavaSalBitmap = (JavaSalBitmap *)&rSalBitmap;

	SalTwoRect aPosAry;
	memcpy( &aPosAry, pPosAry, sizeof( SalTwoRect ) );

	// Scale the bitmap if necessary and always make a copy so that we can
	// mask out the appropriate bits
	JavaSalBitmap aJavaSalBitmap;
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
				BitmapBuffer *pMaskBuffer = aJavaSalBitmap.AcquireBuffer( FALSE );
				if ( pMaskBuffer )
				{
					if ( pMaskBuffer->mpBits )
					{
						// Mark all transparent color pixels as transparent
						nTransparentColor |= 0xff000000;
						long nBits = pMaskBuffer->mnWidth * pMaskBuffer->mnHeight;
						jint *pBits = (jint *)pMaskBuffer->mpBits;
						for ( long i = 0; i < nBits; i++ )
						{
							if ( pBits[ i ] == nTransparentColor )
								pBits[ i ] = 0x00000000;
						}
					}

					aJavaSalBitmap.ReleaseBuffer( pMaskBuffer, FALSE );
					drawBitmap( &aPosAry, aJavaSalBitmap );
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

	// Scale the bitmap if necessary and always make a copy so that we can
	// mask out the appropriate bits
	JavaSalBitmap aJavaSalBitmap;
	JavaSalBitmap aTransJavaSalBitmap;
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
						if ( pTransDestBuffer->mpBits )
						{
							aPosAry.mnSrcX = 0;
							aPosAry.mnSrcY = 0;
							aPosAry.mnSrcWidth = pTransDestBuffer->mnWidth;
							aPosAry.mnSrcHeight = pTransDestBuffer->mnHeight;
							BitmapBuffer *pMaskBuffer = aJavaSalBitmap.AcquireBuffer( FALSE );
							if ( pMaskBuffer )
							{
								if ( pMaskBuffer->mpBits )
								{
									// Mark all non-black pixels in the
									// transparent bitmap as transparent in the
									// mask bitmap
									long nBits = pMaskBuffer->mnWidth * pMaskBuffer->mnHeight;
									jint *pBits = (jint *)pMaskBuffer->mpBits;
									jint *pTransBits = (jint *)pTransDestBuffer->mpBits;
									for ( long i = 0; i < nBits; i++ )
									{
										if ( pTransBits[ i ] != 0xff000000 )
											pBits[ i ] = 0x00000000;
									}
								}

								aJavaSalBitmap.ReleaseBuffer( pMaskBuffer, FALSE );
								drawBitmap( &aPosAry, aJavaSalBitmap );
							}

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

// -----------------------------------------------------------------------

void JavaSalGraphics::drawMask( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap, SalColor nMaskColor )
{
	// Don't do anything if this is a printer
	if ( mpPrinter )
		return;

	JavaSalBitmap *pJavaSalBitmap = (JavaSalBitmap *)&rSalBitmap;

	SalTwoRect aPosAry;
	memcpy( &aPosAry, pPosAry, sizeof( SalTwoRect ) );

	// Scale the bitmap if necessary and always make a copy so that we can
	// mask out the appropriate bits
	JavaSalBitmap aJavaSalBitmap;
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
				BitmapBuffer *pMaskBuffer = aJavaSalBitmap.AcquireBuffer( FALSE );
				if ( pMaskBuffer )
				{
					if ( pMaskBuffer->mpBits )
					{
						// Mark all non-black pixels as transparent
						nMaskColor |= 0xff000000;
						long nBits = pMaskBuffer->mnWidth * pMaskBuffer->mnHeight;
						jint *pBits = (jint *)pMaskBuffer->mpBits;
						for ( long i = 0; i < nBits; i++ )
						{
							if ( pBits[ i ] == 0xff000000 )
								pBits[ i ] = nMaskColor;
							else
								pBits[ i ] = 0x00000000;
						}
					}

					aJavaSalBitmap.ReleaseBuffer( pMaskBuffer, FALSE );
					drawBitmap( &aPosAry, aJavaSalBitmap );
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
	return mpVCLGraphics->getPixel( nX, nY ) & 0x00ffffff;
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
