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
 *  Copyright 2003 Planamesa Inc.
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
#ifndef _SV_COM_SUN_STAR_VCL_VCLBITMAP_HXX
#include <com/sun/star/vcl/VCLBitmap.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENT_HXX
#include <com/sun/star/vcl/VCLEvent.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <bmpacc.hxx>
#endif

using namespace vcl;

// =======================================================================

void JavaSalGraphics::copyBits( const SalTwoRect* pPosAry, SalGraphics* pSrcGraphics )
{
	JavaSalGraphics *pJavaSrcGraphics = (JavaSalGraphics *)pSrcGraphics;
	if ( !pJavaSrcGraphics )
		pJavaSrcGraphics = this;

	// Don't do anything if the source is a printer
	if ( pJavaSrcGraphics->mpPrinter )
		return;

	if ( mpPrinter || pPosAry->mnSrcWidth != pPosAry->mnDestWidth || pPosAry->mnSrcHeight != pPosAry->mnDestHeight )
	{
		SalTwoRect aPosAry;
		memcpy( &aPosAry, pPosAry, sizeof( SalTwoRect ) );

		JavaSalBitmap *pBitmap = (JavaSalBitmap *)pJavaSrcGraphics->getBitmap( pPosAry->mnSrcX, pPosAry->mnSrcY, pPosAry->mnSrcWidth, pPosAry->mnSrcHeight );
		if ( pBitmap )
		{
			aPosAry.mnSrcX = 0;
			aPosAry.mnSrcY = 0;
			drawBitmap( &aPosAry, *pBitmap );
			delete pBitmap;
		}
	}
	else
	{
		mpVCLGraphics->copyBits( pJavaSrcGraphics->mpVCLGraphics, pPosAry->mnSrcX, pPosAry->mnSrcY, pPosAry->mnSrcWidth, pPosAry->mnSrcHeight, pPosAry->mnDestX, pPosAry->mnDestY, pPosAry->mnDestWidth, pPosAry->mnDestHeight, sal_True );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth, long nSrcHeight, USHORT nFlags )
{
	// Don't do anything if this is a printer
	if ( mpPrinter )
		return;

	mpVCLGraphics->copyBits( mpVCLGraphics, nSrcX, nSrcY, nSrcWidth, nSrcHeight, nDestX, nDestY, nSrcWidth, nSrcHeight, sal_False );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap )
{
	JavaSalBitmap *pJavaSalBitmap = (JavaSalBitmap *)&rSalBitmap;

	SalTwoRect aPosAry;
	memcpy( &aPosAry, pPosAry, sizeof( SalTwoRect ) );

	// Adjust the source and destination to eliminate unnecessary copying
	float fScaleX = (float)aPosAry.mnDestWidth / aPosAry.mnSrcWidth;
	float fScaleY = (float)aPosAry.mnDestHeight / aPosAry.mnSrcHeight;
	if ( aPosAry.mnSrcX < 0 )
	{
		aPosAry.mnSrcWidth += aPosAry.mnSrcX;
		if ( aPosAry.mnSrcWidth < 1 )
			return;
		aPosAry.mnDestWidth = (long)( ( fScaleX * aPosAry.mnSrcWidth ) + 0.5 );
		aPosAry.mnDestX -= (long)( ( fScaleX * aPosAry.mnSrcX ) + 0.5 );
		aPosAry.mnSrcX = 0;
	}
	if ( aPosAry.mnSrcY < 0 )
	{
		aPosAry.mnSrcHeight += aPosAry.mnSrcY;
		if ( aPosAry.mnSrcHeight < 1 )
			return;
		aPosAry.mnDestHeight = (long)( ( fScaleY * aPosAry.mnSrcHeight ) + 0.5 );
		aPosAry.mnDestY -= (long)( ( fScaleY * aPosAry.mnSrcY ) + 0.5 );
		aPosAry.mnSrcY = 0;
	}

	Size aSize( pJavaSalBitmap->GetSize() );
	long nExcessWidth = aPosAry.mnSrcX + aPosAry.mnSrcWidth - aSize.Width();
	long nExcessHeight = aPosAry.mnSrcY + aPosAry.mnSrcHeight - aSize.Height();
	if ( nExcessWidth > 0 )
	{
		aPosAry.mnSrcWidth -= nExcessWidth;
		if ( aPosAry.mnSrcWidth < 1 )
			return;
		aPosAry.mnDestWidth = (long)( ( fScaleX * aPosAry.mnSrcWidth ) + 0.5 );
	}
	if ( nExcessHeight > 0 )
	{
		aPosAry.mnSrcHeight -= nExcessHeight;
		if ( aPosAry.mnSrcHeight < 1 )
			return;
		aPosAry.mnDestHeight = (long)( ( fScaleY * aPosAry.mnSrcHeight ) + 0.5 );
	}

	if ( aPosAry.mnDestX < 0 )
	{
		aPosAry.mnDestWidth += aPosAry.mnDestX;
		if ( aPosAry.mnDestWidth < 1 )
			return;
		aPosAry.mnSrcWidth = (long)( ( aPosAry.mnDestWidth / fScaleX ) + 0.5 );
		aPosAry.mnSrcX -= (long)( ( aPosAry.mnDestX / fScaleX ) + 0.5 );
		aPosAry.mnDestX = 0;
	}
	if ( aPosAry.mnDestY < 0 )
	{
		aPosAry.mnDestHeight += aPosAry.mnDestY;
		if ( aPosAry.mnDestHeight < 1 )
			return;
		aPosAry.mnSrcHeight = (long)( ( aPosAry.mnDestHeight / fScaleY ) + 0.5 );
		aPosAry.mnSrcY -= (long)( ( aPosAry.mnDestY / fScaleY ) + 0.5 );
		aPosAry.mnDestY = 0;
	}

	if ( aPosAry.mnSrcWidth < 1 || aPosAry.mnSrcHeight < 1 || aPosAry.mnDestWidth < 1 || aPosAry.mnDestHeight < 1 )
		return;

	// Scale the bitmap if necessary
	bool bDrawn = false;
	if ( mpPrinter || aPosAry.mnSrcWidth != aPosAry.mnDestWidth || aPosAry.mnSrcHeight != aPosAry.mnDestHeight )
	{
		BitmapBuffer *pSrcBuffer = pJavaSalBitmap->AcquireBuffer( TRUE );
		if ( pSrcBuffer )
		{
			if ( mpPrinter )
			{
				// Don't delete the bitmap buffer and let the Java native
				// method print the bitmap buffer directly
				BitmapBuffer *pDestBuffer = StretchAndConvert( *pSrcBuffer, aPosAry, JavaSalBitmap::Get32BitNativeFormat() | BMP_FORMAT_TOP_DOWN );
				if ( pDestBuffer )
				{
					aPosAry.mnSrcX = 0;
					aPosAry.mnSrcY = 0;
					aPosAry.mnSrcWidth = pDestBuffer->mnWidth;
					aPosAry.mnSrcHeight = pDestBuffer->mnHeight;
					mpVCLGraphics->drawBitmapBuffer( pDestBuffer, 0, 0, pDestBuffer->mnWidth, pDestBuffer->mnHeight, aPosAry.mnDestX, aPosAry.mnDestY, pDestBuffer->mnWidth, pDestBuffer->mnHeight );

					bDrawn = true;
				}
			}

			if ( !bDrawn )
			{
				com_sun_star_vcl_VCLBitmap aVCLBitmap( aPosAry.mnDestWidth, aPosAry.mnDestHeight, 32 );
				if ( aVCLBitmap.getJavaObject() )
				{
					java_lang_Object *pData = aVCLBitmap.getData();
					if ( pData )
					{
						VCLThreadAttach t;
						if ( t.pEnv )
						{
							jboolean bCopy( sal_False );
							jint *pBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( (jintArray)pData->getJavaObject(), &bCopy );
							if ( pBits )
							{
								BitmapBuffer *pDestBuffer = StretchAndConvert( *pSrcBuffer, aPosAry, JavaSalBitmap::Get32BitNativeFormat() | BMP_FORMAT_TOP_DOWN, NULL, NULL, (BYTE *)pBits );
								if ( pDestBuffer )
								{
									t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)pData->getJavaObject(), pBits, 0 );
									pBits = NULL;

									mpVCLGraphics->drawBitmap( &aVCLBitmap, 0, 0, pDestBuffer->mnWidth, pDestBuffer->mnHeight, aPosAry.mnDestX, aPosAry.mnDestY, pDestBuffer->mnWidth, pDestBuffer->mnHeight );

									delete pDestBuffer;
									bDrawn = true;
								}

								if ( pBits )
									t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)pData->getJavaObject(), pBits, JNI_ABORT );
							}
						}
		
						delete pData;
					}

					aVCLBitmap.dispose();
				}
			}

			pJavaSalBitmap->ReleaseBuffer( pSrcBuffer, TRUE );
		}
	}

	if ( !bDrawn )
	{
		// If the bitmap is backed by a VCLGraphics instance, draw that
		com_sun_star_vcl_VCLGraphics *pVCLGraphics = pJavaSalBitmap->GetVCLGraphics();
		if ( pVCLGraphics )
		{
			Point aPoint( pJavaSalBitmap->GetPoint() );
			mpVCLGraphics->copyBits( pVCLGraphics, aPoint.X() + aPosAry.mnSrcX, aPoint.Y() + aPosAry.mnSrcY, aPosAry.mnSrcWidth, aPosAry.mnSrcHeight, aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight, sal_True );
		}
		else
		{
			com_sun_star_vcl_VCLBitmap *pVCLBitmap = pJavaSalBitmap->CreateVCLBitmap( aPosAry.mnSrcX, aPosAry.mnSrcY, aPosAry.mnSrcWidth, aPosAry.mnSrcHeight );
			if ( pVCLBitmap )
			{
				mpVCLGraphics->drawBitmap( pVCLBitmap, 0, 0, aPosAry.mnSrcWidth, aPosAry.mnSrcHeight, aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight );
				pJavaSalBitmap->ReleaseVCLBitmap( pVCLBitmap );
			}
		}
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap, SalColor nTransparentColor )
{
	// Don't do anything if this is a printer
	if ( mpPrinter )
		return;

	JavaSalBitmap *pJavaSalBitmap = (JavaSalBitmap *)&rSalBitmap;

	SalTwoRect aPosAry;
	memcpy( &aPosAry, pPosAry, sizeof( SalTwoRect ) );

	// Adjust the source and destination to eliminate unnecessary copying
	float fScaleX = (float)aPosAry.mnDestWidth / aPosAry.mnSrcWidth;
	float fScaleY = (float)aPosAry.mnDestHeight / aPosAry.mnSrcHeight;
	if ( aPosAry.mnSrcX < 0 )
	{
		aPosAry.mnSrcWidth += aPosAry.mnSrcX;
		if ( aPosAry.mnSrcWidth < 1 )
			return;
		aPosAry.mnDestWidth = (long)( ( fScaleX * aPosAry.mnSrcWidth ) + 0.5 );
		aPosAry.mnDestX -= (long)( ( fScaleX * aPosAry.mnSrcX ) + 0.5 );
		aPosAry.mnSrcX = 0;
	}
	if ( aPosAry.mnSrcY < 0 )
	{
		aPosAry.mnSrcHeight += aPosAry.mnSrcY;
		if ( aPosAry.mnSrcHeight < 1 )
			return;
		aPosAry.mnDestHeight = (long)( ( fScaleY * aPosAry.mnSrcHeight ) + 0.5 );
		aPosAry.mnDestY -= (long)( ( fScaleY * aPosAry.mnSrcY ) + 0.5 );
		aPosAry.mnSrcY = 0;
	}

	Size aSize( pJavaSalBitmap->GetSize() );
	long nExcessWidth = aPosAry.mnSrcX + aPosAry.mnSrcWidth - aSize.Width();
	long nExcessHeight = aPosAry.mnSrcY + aPosAry.mnSrcHeight - aSize.Height();
	if ( nExcessWidth > 0 )
	{
		aPosAry.mnSrcWidth -= nExcessWidth;
		if ( aPosAry.mnSrcWidth < 1 )
			return;
		aPosAry.mnDestWidth = (long)( ( fScaleX * aPosAry.mnSrcWidth ) + 0.5 );
	}
	if ( nExcessHeight > 0 )
	{
		aPosAry.mnSrcHeight -= nExcessHeight;
		if ( aPosAry.mnSrcHeight < 1 )
			return;
		aPosAry.mnDestHeight = (long)( ( fScaleY * aPosAry.mnSrcHeight ) + 0.5 );
	}

	if ( aPosAry.mnDestX < 0 )
	{
		aPosAry.mnDestWidth += aPosAry.mnDestX;
		if ( aPosAry.mnDestWidth < 1 )
			return;
		aPosAry.mnSrcWidth = (long)( ( aPosAry.mnDestWidth / fScaleX ) + 0.5 );
		aPosAry.mnSrcX -= (long)( ( aPosAry.mnDestX / fScaleX ) + 0.5 );
		aPosAry.mnDestX = 0;
	}
	if ( aPosAry.mnDestY < 0 )
	{
		aPosAry.mnDestHeight += aPosAry.mnDestY;
		if ( aPosAry.mnDestHeight < 1 )
			return;
		aPosAry.mnSrcHeight = (long)( ( aPosAry.mnDestHeight / fScaleY ) + 0.5 );
		aPosAry.mnSrcY -= (long)( ( aPosAry.mnDestY / fScaleY ) + 0.5 );
		aPosAry.mnDestY = 0;
	}

	if ( aPosAry.mnSrcWidth < 1 || aPosAry.mnSrcHeight < 1 || aPosAry.mnDestWidth < 1 || aPosAry.mnDestHeight < 1 )
		return;

	// Scale the bitmap if necessary and always make a copy so that we can
	// mask out the appropriate bits
	BitmapBuffer *pSrcBuffer = pJavaSalBitmap->AcquireBuffer( TRUE );
	if ( pSrcBuffer )
	{
		com_sun_star_vcl_VCLBitmap aVCLBitmap( aPosAry.mnDestWidth, aPosAry.mnDestHeight, 32 );
		if ( aVCLBitmap.getJavaObject() )
		{
			java_lang_Object *pData = aVCLBitmap.getData();
			if ( pData )
			{
				VCLThreadAttach t;
				if ( t.pEnv )
				{
					jboolean bCopy( sal_False );
					jint *pBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( (jintArray)pData->getJavaObject(), &bCopy );
					if ( pBits )
					{
						BitmapBuffer *pDestBuffer = StretchAndConvert( *pSrcBuffer, aPosAry, JavaSalBitmap::Get32BitNativeFormat() | BMP_FORMAT_TOP_DOWN, NULL, NULL, (BYTE *)pBits );
						if ( pDestBuffer )
						{
							pJavaSalBitmap->ReleaseBuffer( pSrcBuffer, TRUE );

							// Mark all transparent color pixels as transparent
							nTransparentColor |= 0xff000000;
							long nBits = pDestBuffer->mnWidth * pDestBuffer->mnHeight;
							for ( long i = 0; i < nBits; i++ )
							{
								if ( pBits[ i ] == nTransparentColor )
									pBits[ i ] = 0x00000000;
							}

							t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)pData->getJavaObject(), pBits, 0 );
							pBits = NULL;

							mpVCLGraphics->drawBitmap( &aVCLBitmap, 0, 0, pDestBuffer->mnWidth, pDestBuffer->mnHeight, aPosAry.mnDestX, aPosAry.mnDestY, pDestBuffer->mnWidth, pDestBuffer->mnHeight );

							delete pDestBuffer;
						}

						if ( pBits )
							t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)pData->getJavaObject(), pBits, JNI_ABORT );
					}
				}

				delete pData;
			}

			aVCLBitmap.dispose();
		}

		pJavaSalBitmap->ReleaseBuffer( pSrcBuffer, TRUE );
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

	// Adjust the source and destination to eliminate unnecessary copying
	float fScaleX = (float)aPosAry.mnDestWidth / aPosAry.mnSrcWidth;
	float fScaleY = (float)aPosAry.mnDestHeight / aPosAry.mnSrcHeight;
	if ( aPosAry.mnSrcX < 0 )
	{
		aPosAry.mnSrcWidth += aPosAry.mnSrcX;
		if ( aPosAry.mnSrcWidth < 1 )
			return;
		aPosAry.mnDestWidth = (long)( ( fScaleX * aPosAry.mnSrcWidth ) + 0.5 );
		aPosAry.mnDestX -= (long)( ( fScaleX * aPosAry.mnSrcX ) + 0.5 );
		aPosAry.mnSrcX = 0;
	}
	if ( aPosAry.mnSrcY < 0 )
	{
		aPosAry.mnSrcHeight += aPosAry.mnSrcY;
		if ( aPosAry.mnSrcHeight < 1 )
			return;
		aPosAry.mnDestHeight = (long)( ( fScaleY * aPosAry.mnSrcHeight ) + 0.5 );
		aPosAry.mnDestY -= (long)( ( fScaleY * aPosAry.mnSrcY ) + 0.5 );
		aPosAry.mnSrcY = 0;
	}

	Size aSize( pJavaSalBitmap->GetSize() );
	long nExcessWidth = aPosAry.mnSrcX + aPosAry.mnSrcWidth - aSize.Width();
	long nExcessHeight = aPosAry.mnSrcY + aPosAry.mnSrcHeight - aSize.Height();
	if ( nExcessWidth > 0 )
	{
		aPosAry.mnSrcWidth -= nExcessWidth;
		if ( aPosAry.mnSrcWidth < 1 )
			return;
		aPosAry.mnDestWidth = (long)( ( fScaleX * aPosAry.mnSrcWidth ) + 0.5 );
	}
	if ( nExcessHeight > 0 )
	{
		aPosAry.mnSrcHeight -= nExcessHeight;
		if ( aPosAry.mnSrcHeight < 1 )
			return;
		aPosAry.mnDestHeight = (long)( ( fScaleY * aPosAry.mnSrcHeight ) + 0.5 );
	}

	if ( aPosAry.mnDestX < 0 )
	{
		aPosAry.mnDestWidth += aPosAry.mnDestX;
		if ( aPosAry.mnDestWidth < 1 )
			return;
		aPosAry.mnSrcWidth = (long)( ( aPosAry.mnDestWidth / fScaleX ) + 0.5 );
		aPosAry.mnSrcX -= (long)( ( aPosAry.mnDestX / fScaleX ) + 0.5 );
		aPosAry.mnDestX = 0;
	}
	if ( aPosAry.mnDestY < 0 )
	{
		aPosAry.mnDestHeight += aPosAry.mnDestY;
		if ( aPosAry.mnDestHeight < 1 )
			return;
		aPosAry.mnSrcHeight = (long)( ( aPosAry.mnDestHeight / fScaleY ) + 0.5 );
		aPosAry.mnSrcY -= (long)( ( aPosAry.mnDestY / fScaleY ) + 0.5 );
		aPosAry.mnDestY = 0;
	}

	if ( aPosAry.mnSrcWidth < 1 || aPosAry.mnSrcHeight < 1 || aPosAry.mnDestWidth < 1 || aPosAry.mnDestHeight < 1 )
		return;

	// Scale the bitmap if necessary and always make a copy so that we can
	// mask out the appropriate bits
	BitmapBuffer *pSrcBuffer = pJavaSalBitmap->AcquireBuffer( TRUE );
	if ( pSrcBuffer )
	{
		com_sun_star_vcl_VCLBitmap aVCLBitmap( aPosAry.mnDestWidth, aPosAry.mnDestHeight, 32 );
		if ( aVCLBitmap.getJavaObject() )
		{
			java_lang_Object *pData = aVCLBitmap.getData();
			if ( pData )
			{
				VCLThreadAttach t;
				if ( t.pEnv )
				{
					jboolean bCopy( sal_False );
					jint *pBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( (jintArray)pData->getJavaObject(), &bCopy );
					if ( pBits )
					{
						BitmapBuffer *pDestBuffer = StretchAndConvert( *pSrcBuffer, aPosAry, JavaSalBitmap::Get32BitNativeFormat() | BMP_FORMAT_TOP_DOWN, NULL, NULL, (BYTE *)pBits );
						if ( pDestBuffer )
						{
							BitmapBuffer *pTransSrcBuffer = pTransJavaSalBitmap->AcquireBuffer( TRUE );
							if ( pTransSrcBuffer )
							{
								// Fix bug 2475 by handling the case where the
								// transparent bitmap is smaller than the main
								// bitmap
								SalTwoRect aTransPosAry;
								memcpy( &aTransPosAry, &aPosAry, sizeof( SalTwoRect ) );
								Size aTransSize( pTransJavaSalBitmap->GetSize() );
								long nTransExcessWidth = aPosAry.mnSrcX + aPosAry.mnSrcWidth - aTransSize.Width();
								if ( nTransExcessWidth > 0 )
								{
									aTransPosAry.mnSrcWidth -= nTransExcessWidth;
									aTransPosAry.mnDestWidth = aTransPosAry.mnSrcWidth * aPosAry.mnSrcWidth / aPosAry.mnDestWidth;
								}
								long nTransExcessHeight = aPosAry.mnSrcY + aPosAry.mnSrcHeight - aTransSize.Height();
								if ( nTransExcessHeight > 0 )
								{
									aTransPosAry.mnSrcHeight -= nTransExcessHeight;
									aTransPosAry.mnDestHeight = aTransPosAry.mnSrcHeight * aPosAry.mnSrcHeight / aPosAry.mnDestHeight;
								}
								BitmapBuffer *pTransDestBuffer = StretchAndConvert( *pTransSrcBuffer, aTransPosAry, BMP_FORMAT_1BIT_MSB_PAL | BMP_FORMAT_TOP_DOWN, &pTransSrcBuffer->maPalette );
								if ( pTransDestBuffer )
								{
									if ( pTransDestBuffer->mpBits )
									{
										// Mark all non-black pixels in the
										// transparent bitmap as transparent in
										// the mask bitmap
										Scanline pTransBits = (Scanline)pTransDestBuffer->mpBits;
										FncGetPixel pFncGetPixel = BitmapReadAccess::GetPixelFor_1BIT_MSB_PAL;
										for ( int i = 0; i < pDestBuffer->mnHeight; i++ )
										{
											bool bTransPixels = ( i < pTransDestBuffer->mnHeight );
											for ( int j = 0; j < pDestBuffer->mnWidth; j++ )
											{
												if ( bTransPixels && j < pTransDestBuffer->mnWidth )
												{
													BitmapColor aColor( pTransDestBuffer->maPalette[ pFncGetPixel( pTransBits, j, pTransDestBuffer->maColorMask ) ] );
                                    				if ( ( MAKE_SALCOLOR( aColor.GetRed(), aColor.GetGreen(), aColor.GetBlue() ) | 0xff000000 ) != 0xff000000 )
														pBits[ j ] = 0x00000000;
												}
												else
												{
													pBits[ j ] = 0x00000000;
												}
											}
	
											pBits += pDestBuffer->mnWidth;
											pTransBits += pTransDestBuffer->mnScanlineSize;
										}

										delete[] pTransDestBuffer->mpBits;

										t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)pData->getJavaObject(), pBits, 0 );
										pBits = NULL;

										mpVCLGraphics->drawBitmap( &aVCLBitmap, 0, 0, pDestBuffer->mnWidth, pDestBuffer->mnHeight, aPosAry.mnDestX, aPosAry.mnDestY, pDestBuffer->mnWidth, pDestBuffer->mnHeight );
									}

									delete pTransDestBuffer;
								}

								pTransJavaSalBitmap->ReleaseBuffer( pTransSrcBuffer, TRUE );
							}

							delete pDestBuffer;
						}

						if ( pBits )
							t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)pData->getJavaObject(), pBits, JNI_ABORT );
					}
				}

				delete pData;
			}

			aVCLBitmap.dispose();
		}

		pJavaSalBitmap->ReleaseBuffer( pSrcBuffer, TRUE );
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

	// Adjust the source and destination to eliminate unnecessary copying
	float fScaleX = (float)aPosAry.mnDestWidth / aPosAry.mnSrcWidth;
	float fScaleY = (float)aPosAry.mnDestHeight / aPosAry.mnSrcHeight;
	if ( aPosAry.mnSrcX < 0 )
	{
		aPosAry.mnSrcWidth += aPosAry.mnSrcX;
		if ( aPosAry.mnSrcWidth < 1 )
			return;
		aPosAry.mnDestWidth = (long)( ( fScaleX * aPosAry.mnSrcWidth ) + 0.5 );
		aPosAry.mnDestX -= (long)( ( fScaleX * aPosAry.mnSrcX ) + 0.5 );
		aPosAry.mnSrcX = 0;
	}
	if ( aPosAry.mnSrcY < 0 )
	{
		aPosAry.mnSrcHeight += aPosAry.mnSrcY;
		if ( aPosAry.mnSrcHeight < 1 )
			return;
		aPosAry.mnDestHeight = (long)( ( fScaleY * aPosAry.mnSrcHeight ) + 0.5 );
		aPosAry.mnDestY -= (long)( ( fScaleY * aPosAry.mnSrcY ) + 0.5 );
		aPosAry.mnSrcY = 0;
	}

	Size aSize( pJavaSalBitmap->GetSize() );
	long nExcessWidth = aPosAry.mnSrcX + aPosAry.mnSrcWidth - aSize.Width();
	long nExcessHeight = aPosAry.mnSrcY + aPosAry.mnSrcHeight - aSize.Height();
	if ( nExcessWidth > 0 )
	{
		aPosAry.mnSrcWidth -= nExcessWidth;
		if ( aPosAry.mnSrcWidth < 1 )
			return;
		aPosAry.mnDestWidth = (long)( ( fScaleX * aPosAry.mnSrcWidth ) + 0.5 );
	}
	if ( nExcessHeight > 0 )
	{
		aPosAry.mnSrcHeight -= nExcessHeight;
		if ( aPosAry.mnSrcHeight < 1 )
			return;
		aPosAry.mnDestHeight = (long)( ( fScaleY * aPosAry.mnSrcHeight ) + 0.5 );
	}

	if ( aPosAry.mnDestX < 0 )
	{
		aPosAry.mnDestWidth += aPosAry.mnDestX;
		if ( aPosAry.mnDestWidth < 1 )
			return;
		aPosAry.mnSrcWidth = (long)( ( aPosAry.mnDestWidth / fScaleX ) + 0.5 );
		aPosAry.mnSrcX -= (long)( ( aPosAry.mnDestX / fScaleX ) + 0.5 );
		aPosAry.mnDestX = 0;
	}
	if ( aPosAry.mnDestY < 0 )
	{
		aPosAry.mnDestHeight += aPosAry.mnDestY;
		if ( aPosAry.mnDestHeight < 1 )
			return;
		aPosAry.mnSrcHeight = (long)( ( aPosAry.mnDestHeight / fScaleY ) + 0.5 );
		aPosAry.mnSrcY -= (long)( ( aPosAry.mnDestY / fScaleY ) + 0.5 );
		aPosAry.mnDestY = 0;
	}

	if ( aPosAry.mnSrcWidth < 1 || aPosAry.mnSrcHeight < 1 || aPosAry.mnDestWidth < 1 || aPosAry.mnDestHeight < 1 )
		return;

	// Scale the bitmap if necessary and always make a copy so that we can
	// mask out the appropriate bits
	BitmapBuffer *pSrcBuffer = pJavaSalBitmap->AcquireBuffer( TRUE );
	if ( pSrcBuffer )
	{
		com_sun_star_vcl_VCLBitmap aVCLBitmap( aPosAry.mnDestWidth, aPosAry.mnDestHeight, 32 );
		if ( aVCLBitmap.getJavaObject() )
		{
			java_lang_Object *pData = aVCLBitmap.getData();
			if ( pData )
			{
				VCLThreadAttach t;
				if ( t.pEnv )
				{
					jboolean bCopy( sal_False );
					jint *pBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( (jintArray)pData->getJavaObject(), &bCopy );
					if ( pBits )
					{
						BitmapBuffer *pDestBuffer = StretchAndConvert( *pSrcBuffer, aPosAry, JavaSalBitmap::Get32BitNativeFormat() | BMP_FORMAT_TOP_DOWN, NULL, NULL, (BYTE *)pBits );
						if ( pDestBuffer )
						{
							// Mark all non-black pixels as transparent
							nMaskColor |= 0xff000000;
							long nBits = pDestBuffer->mnWidth * pDestBuffer->mnHeight;
							for ( long i = 0; i < nBits; i++ )
							{
								if ( pBits[ i ] == 0xff000000 )
									pBits[ i ] = nMaskColor;
								else
									pBits[ i ] = 0x00000000;
							}

							t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)pData->getJavaObject(), pBits, 0 );
							pBits = NULL;

							mpVCLGraphics->drawBitmap( &aVCLBitmap, 0, 0, aPosAry.mnSrcWidth, aPosAry.mnSrcHeight, aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight );

							delete pDestBuffer;
						}

						if ( pBits )
							t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)pData->getJavaObject(), pBits, JNI_ABORT );
					}
				}

				delete pData;
			}
		}

		pJavaSalBitmap->ReleaseBuffer( pSrcBuffer, TRUE );
	}
}

// -----------------------------------------------------------------------

SalBitmap* JavaSalGraphics::getBitmap( long nX, long nY, long nDX, long nDY )
{
	// Don't do anything if this is a printer
	if ( mpPrinter || !nDX || !nDY )
		return NULL;

	// Normalize the bounds
	if ( nDX < 0 )
	{
		nX += nDX;
		nDX = -nDX;
    }
	if ( nDY < 0 )
	{
		nY += nDY;
		nDY = -nDY;
	}

	JavaSalBitmap *pBitmap = new JavaSalBitmap();

	if ( !pBitmap->Create( Point( nX, nY ), Size( nDX, nDY ), mpVCLGraphics, BitmapPalette() ) )
	{
		delete pBitmap;
		pBitmap = NULL;
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

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawAlphaBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSourceBitmap, const SalBitmap& rAlphaBitmap )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::drawAlphaBitmap not implemented\n" );
#endif
	return false;
}
