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

#include <salgdi.h>
#include <salbmp.h>
#include <vcl/salwtype.hxx>
#include <vcl/sysdata.hxx>
#include <com/sun/star/vcl/VCLBitmap.hxx>
#include <com/sun/star/vcl/VCLEvent.hxx>
#include <com/sun/star/vcl/VCLFrame.hxx>
#include <com/sun/star/vcl/VCLGraphics.hxx>
#include <vcl/bmpacc.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

using namespace osl;
using namespace vcl;

// =======================================================================

JavaSalGraphicsDrawImageOp::JavaSalGraphicsDrawImageOp( const CGPathRef aNativeClipPath, bool bInvert, bool bXOR, CGDataProviderRef aProvider, int nDataBitCount, size_t nDataScanlineSize, size_t nDataWidth, size_t nDataHeight, const CGRect aSrcRect, const CGRect aRect, bool bFlip ) :
	JavaSalGraphicsOp( aNativeClipPath, bInvert, bXOR ),
	maImage( NULL ),
	maRect( aRect ),
	mbFlip( bFlip )
{
	if ( aProvider && nDataScanlineSize && nDataWidth && nDataHeight && !CGRectIsEmpty( aSrcRect ) && CGRectIntersectsRect( aSrcRect, CGRectMake( 0, 0, nDataWidth, nDataHeight ) ) )
	{
		CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
		if ( aColorSpace )
		{
			CGImageRef aImage = CGImageCreate( nDataWidth, nDataHeight, 8, nDataBitCount, nDataScanlineSize, aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little, aProvider, NULL, false, kCGRenderingIntentDefault );
			if ( aImage )
			{
				maImage = CGImageCreateWithImageInRect( aImage, aSrcRect );
				CGImageRelease( aImage );
			}

			CGColorSpaceRelease( aColorSpace );
		}
	}
}

// -----------------------------------------------------------------------

JavaSalGraphicsDrawImageOp::~JavaSalGraphicsDrawImageOp()
{
	if ( maImage )
		CGImageRelease( maImage );
}

// -----------------------------------------------------------------------

void JavaSalGraphicsDrawImageOp::drawOp( JavaSalGraphics *pGraphics, CGContextRef aContext, CGRect aBounds )
{
	if ( !aContext || !maImage )
		return;

	CGRect aDrawBounds = maRect;
	if ( !CGRectIsEmpty( aBounds ) )
		aDrawBounds = CGRectIntersection( aDrawBounds, aBounds );
	if ( maNativeClipPath )
		aDrawBounds = CGRectIntersection( aDrawBounds, CGPathGetBoundingBox( maNativeClipPath ) );
	if ( CGRectIsEmpty( aDrawBounds ) )
		return;

	aContext = saveClipXORGState( pGraphics, aContext, aDrawBounds );
	if ( !aContext )
		return;

	CGContextClipToRect( aContext, maRect );
	if ( !mbFlip && JavaSalBitmap::GetNativeDirectionFormat() == BMP_FORMAT_BOTTOM_UP )
	{
		CGContextDrawImage( aContext, CGRectMake( maRect.origin.x, maRect.origin.y, maRect.size.width, maRect.size.height ), maImage );
	}
	else
	{
		CGContextTranslateCTM( aContext, maRect.origin.x, maRect.origin.y + maRect.size.height );
		CGContextScaleCTM( aContext, 1.0f, -1.0f );
		CGContextDrawImage( aContext, CGRectMake( 0, 0, maRect.size.width, maRect.size.height ), maImage );
	}

	restoreClipXORGState();
}

// =======================================================================

void JavaSalGraphics::copyBits( const SalTwoRect* pPosAry, SalGraphics* pSrcGraphics )
{
	JavaSalGraphics *pJavaSrcGraphics = (JavaSalGraphics *)pSrcGraphics;
	if ( !pJavaSrcGraphics )
		pJavaSrcGraphics = this;

	// Don't do anything if the source is a printer
	if ( pJavaSrcGraphics->mpPrinter )
		return;

	// If either the source or destination uses native drawing, create and draw
	// a bitmap as drawing the source's layer will cause fully transparent
	// to be drawn as partially transparent
	if ( mpPrinter || useNativeDrawing() || pJavaSrcGraphics->useNativeDrawing() || pPosAry->mnSrcWidth != pPosAry->mnDestWidth || pPosAry->mnSrcHeight != pPosAry->mnDestHeight )
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
	else if ( mpVCLGraphics )
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

	if ( useNativeDrawing() )
		copyFromGraphics( this, CGPointMake( nSrcX, nSrcY ), CGRectMake( nDestX, nDestY, nSrcWidth, nSrcHeight ), false );
	else if ( mpVCLGraphics )
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
	if ( mpPrinter || ( useNativeDrawing() && pJavaSalBitmap->GetBitCount() != GetBitCount() ) || getBitmapDirectionFormat() != JavaSalBitmap::GetNativeDirectionFormat() || aPosAry.mnSrcWidth != aPosAry.mnDestWidth || aPosAry.mnSrcHeight != aPosAry.mnDestHeight )
	{
		BitmapBuffer *pSrcBuffer = pJavaSalBitmap->AcquireBuffer( TRUE );
		if ( pSrcBuffer )
		{
			if ( mpPrinter || useNativeDrawing() )
			{
				SalTwoRect aCopyPosAry;
				memcpy( &aCopyPosAry, &aPosAry, sizeof( SalTwoRect ) );
				aCopyPosAry.mnDestX = 0;
				aCopyPosAry.mnDestY = 0;
				aCopyPosAry.mnDestWidth = aCopyPosAry.mnSrcWidth;
				aCopyPosAry.mnDestHeight = aCopyPosAry.mnSrcHeight;
				BitmapBuffer *pCopyBuffer = StretchAndConvert( *pSrcBuffer, aCopyPosAry, JavaSalBitmap::Get32BitNativeFormat() | getBitmapDirectionFormat() );
				if ( pCopyBuffer )
				{
					if ( useNativeDrawing() )
					{
						// Assign ownership of bits to a CGDataProvider instance
						CGDataProviderRef aProvider = CGDataProviderCreateWithData( NULL, pCopyBuffer->mpBits, pCopyBuffer->mnScanlineSize * pCopyBuffer->mnHeight, ReleaseBitmapBufferBytePointerCallback );
						if ( aProvider )
						{
							pCopyBuffer->mpBits = NULL;
							addUndrawnNativeOp( new JavaSalGraphicsDrawImageOp( maNativeClipPath, mbInvert, mbXOR, aProvider, pCopyBuffer->mnBitCount, pCopyBuffer->mnScanlineSize, pCopyBuffer->mnWidth, pCopyBuffer->mnHeight, CGRectMake( 0, 0, pCopyBuffer->mnWidth, pCopyBuffer->mnHeight ), CGRectMake( aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight ) ) );
							CGDataProviderRelease( aProvider );
						}
						else
						{
							delete[] pCopyBuffer->mpBits;
						}
		
						delete pCopyBuffer;
					}
					else if ( mpVCLGraphics )
					{
						// Don't delete the bitmap buffer and let the Java
						// native method print the bitmap buffer directly
						mpVCLGraphics->drawBitmapBuffer( pCopyBuffer, 0, 0, pCopyBuffer->mnWidth, pCopyBuffer->mnHeight, aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight, maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
					}
				}
			}
			else if ( mpVCLGraphics )
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
							sal_uInt32 *pBits = (sal_uInt32 *)t.pEnv->GetPrimitiveArrayCritical( (jintArray)pData->getJavaObject(), &bCopy );
							if ( pBits )
							{
								BitmapBuffer *pDestBuffer = StretchAndConvert( *pSrcBuffer, aPosAry, JavaSalBitmap::Get32BitNativeFormat() | getBitmapDirectionFormat(), NULL, NULL, (BYTE *)pBits );
								if ( pDestBuffer )
								{
									t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)pData->getJavaObject(), pBits, 0 );
									pBits = NULL;

									mpVCLGraphics->drawBitmap( &aVCLBitmap, 0, 0, pDestBuffer->mnWidth, pDestBuffer->mnHeight, aPosAry.mnDestX, aPosAry.mnDestY, pDestBuffer->mnWidth, pDestBuffer->mnHeight, NULL );

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
			}

			pJavaSalBitmap->ReleaseBuffer( pSrcBuffer, TRUE );
		}
	}
	else if ( useNativeDrawing() )
	{
		// If the bitmap is backed by a layer, draw that
		JavaSalGraphics *pGraphics = pJavaSalBitmap->GetGraphics();
		if ( pGraphics )
		{
			Point aPoint( pJavaSalBitmap->GetPoint() );
			copyFromGraphics( pGraphics, CGPointMake( aPoint.X() + aPosAry.mnSrcX, aPoint.Y() + aPosAry.mnSrcY ), CGRectMake( aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight ), true );
		}
		else
		{
			BitmapBuffer *pSrcBuffer = pJavaSalBitmap->AcquireBuffer( TRUE );
			if ( pSrcBuffer )
			{
				if ( pSrcBuffer->mpBits )
				{
					SalTwoRect aCopyPosAry;
					memcpy( &aCopyPosAry, &aPosAry, sizeof( SalTwoRect ) );
					aCopyPosAry.mnDestX = 0;
					aCopyPosAry.mnDestY = 0;
					aCopyPosAry.mnDestWidth = aCopyPosAry.mnSrcWidth;
					aCopyPosAry.mnDestHeight = aCopyPosAry.mnSrcHeight;
					BitmapBuffer *pCopyBuffer = StretchAndConvert( *pSrcBuffer, aCopyPosAry, JavaSalBitmap::Get32BitNativeFormat() | getBitmapDirectionFormat() );
					if ( pCopyBuffer )
					{
						// Assign ownership of bits to a CGDataProvider
						// instance
						CGDataProviderRef aProvider = CGDataProviderCreateWithData( NULL, pCopyBuffer->mpBits, pCopyBuffer->mnScanlineSize * pCopyBuffer->mnHeight, ReleaseBitmapBufferBytePointerCallback );
						if ( aProvider )
						{
							pCopyBuffer->mpBits = NULL;
							addUndrawnNativeOp( new JavaSalGraphicsDrawImageOp( maNativeClipPath, mbInvert, mbXOR, aProvider, pCopyBuffer->mnBitCount, pCopyBuffer->mnScanlineSize, pCopyBuffer->mnWidth, pCopyBuffer->mnHeight, CGRectMake( 0, 0, pCopyBuffer->mnWidth, pCopyBuffer->mnHeight ), CGRectMake( aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight ) ) );
							CGDataProviderRelease( aProvider );
						}
						else
						{
							delete[] pCopyBuffer->mpBits;
						}
	
						delete pCopyBuffer;
					}
				}

				pJavaSalBitmap->ReleaseBuffer( pSrcBuffer, TRUE );
			}
		}
	}
	else if ( mpVCLGraphics )
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
				mpVCLGraphics->drawBitmap( pVCLBitmap, 0, 0, aPosAry.mnSrcWidth, aPosAry.mnSrcHeight, aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
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
		if ( useNativeDrawing() )
		{
			BitmapBuffer *pDestBuffer = StretchAndConvert( *pSrcBuffer, aPosAry, JavaSalBitmap::Get32BitNativeFormat() | getBitmapDirectionFormat() );
			if ( pDestBuffer )
			{
				if ( pDestBuffer->mpBits )
				{
					// Mark all transparent color pixels as transparent
					nTransparentColor |= 0xff000000;
					long nBits = pDestBuffer->mnWidth * pDestBuffer->mnHeight;
					sal_uInt32 *pBits = (sal_uInt32 *)pDestBuffer->mpBits;
					for ( long i = 0; i < nBits; i++ )
					{
						if ( pBits[ i ] == nTransparentColor )
							pBits[ i ] = 0x00000000;
					}

					// Assign ownership of bits to a CGDataProvider instance
					CGDataProviderRef aProvider = CGDataProviderCreateWithData( NULL, pDestBuffer->mpBits, pDestBuffer->mnScanlineSize * pDestBuffer->mnHeight, ReleaseBitmapBufferBytePointerCallback );
					if ( aProvider )
					{
						pDestBuffer->mpBits = NULL;
						addUndrawnNativeOp( new JavaSalGraphicsDrawImageOp( maNativeClipPath, mbInvert, mbXOR, aProvider, pDestBuffer->mnBitCount, pDestBuffer->mnScanlineSize, pDestBuffer->mnWidth, pDestBuffer->mnHeight, CGRectMake( 0, 0, pDestBuffer->mnWidth, pDestBuffer->mnHeight ), CGRectMake( aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight ) ) );
						CGDataProviderRelease( aProvider );
					}
					else
					{
						delete[] pDestBuffer->mpBits;
					}
				}

				delete pDestBuffer;
			}
		}
		else if ( mpVCLGraphics )
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
						sal_uInt32 *pBits = (sal_uInt32 *)t.pEnv->GetPrimitiveArrayCritical( (jintArray)pData->getJavaObject(), &bCopy );
						if ( pBits )
						{
							BitmapBuffer *pDestBuffer = StretchAndConvert( *pSrcBuffer, aPosAry, JavaSalBitmap::Get32BitNativeFormat() | getBitmapDirectionFormat(), NULL, NULL, (BYTE *)pBits );
							if ( pDestBuffer )
							{
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

								mpVCLGraphics->drawBitmap( &aVCLBitmap, 0, 0, pDestBuffer->mnWidth, pDestBuffer->mnHeight, aPosAry.mnDestX, aPosAry.mnDestY, pDestBuffer->mnWidth, pDestBuffer->mnHeight, NULL );

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
		if ( useNativeDrawing() )
		{
			BitmapBuffer *pDestBuffer = StretchAndConvert( *pSrcBuffer, aPosAry, JavaSalBitmap::Get32BitNativeFormat() | getBitmapDirectionFormat() );
			if ( pDestBuffer )
			{
				if ( pDestBuffer->mpBits )
				{
					BitmapBuffer *pTransSrcBuffer = pTransJavaSalBitmap->AcquireBuffer( TRUE );
					if ( pTransSrcBuffer )
					{
						// Fix bug 2475 by handling the case where the
						// transparent bitmap is smaller than the main bitmap
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
						BitmapBuffer *pTransDestBuffer = StretchAndConvert( *pTransSrcBuffer, aTransPosAry, BMP_FORMAT_1BIT_MSB_PAL | getBitmapDirectionFormat(), &pTransSrcBuffer->maPalette );
						if ( pTransDestBuffer )
						{
							if ( pTransDestBuffer->mpBits )
							{
								// Mark all non-black pixels in the transparent
								// bitmap as transparent in the mask bitmap
								sal_uInt32 *pBits = (sal_uInt32 *)pDestBuffer->mpBits;
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

								// Assign ownership of bits to a CGDataProvider
								// instance
								CGDataProviderRef aProvider = CGDataProviderCreateWithData( NULL, pDestBuffer->mpBits, pDestBuffer->mnScanlineSize * pDestBuffer->mnHeight, ReleaseBitmapBufferBytePointerCallback );
								if ( aProvider )
								{
									pDestBuffer->mpBits = NULL;
									addUndrawnNativeOp( new JavaSalGraphicsDrawImageOp( maNativeClipPath, mbInvert, mbXOR, aProvider, pDestBuffer->mnBitCount, pDestBuffer->mnScanlineSize, pDestBuffer->mnWidth, pDestBuffer->mnHeight, CGRectMake( 0, 0, pDestBuffer->mnWidth, pDestBuffer->mnHeight ), CGRectMake( aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight ) ) );
									CGDataProviderRelease( aProvider );
								}
							}

							delete pTransDestBuffer;
						}

						pTransJavaSalBitmap->ReleaseBuffer( pTransSrcBuffer, TRUE );
					}

					if ( pDestBuffer->mpBits )
						delete[] pDestBuffer->mpBits;
				}

				delete pDestBuffer;
			}
		}
		else if ( mpVCLGraphics )
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
						sal_uInt32 *pBits = (sal_uInt32 *)t.pEnv->GetPrimitiveArrayCritical( (jintArray)pData->getJavaObject(), &bCopy );
						if ( pBits )
						{
							BitmapBuffer *pDestBuffer = StretchAndConvert( *pSrcBuffer, aPosAry, JavaSalBitmap::Get32BitNativeFormat() | getBitmapDirectionFormat(), NULL, NULL, (BYTE *)pBits );
							if ( pDestBuffer )
							{
								BitmapBuffer *pTransSrcBuffer = pTransJavaSalBitmap->AcquireBuffer( TRUE );
								if ( pTransSrcBuffer )
								{
									// Fix bug 2475 by handling the case where
									// the transparent bitmap is smaller than
									// the main bitmap
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
									BitmapBuffer *pTransDestBuffer = StretchAndConvert( *pTransSrcBuffer, aTransPosAry, BMP_FORMAT_1BIT_MSB_PAL | getBitmapDirectionFormat(), &pTransSrcBuffer->maPalette );
									if ( pTransDestBuffer )
									{
										if ( pTransDestBuffer->mpBits )
										{
											// Mark all non-black pixels in the
											// transparent bitmap as transparent
											// in the mask bitmap
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

											mpVCLGraphics->drawBitmap( &aVCLBitmap, 0, 0, pDestBuffer->mnWidth, pDestBuffer->mnHeight, aPosAry.mnDestX, aPosAry.mnDestY, pDestBuffer->mnWidth, pDestBuffer->mnHeight, NULL );
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
		if ( useNativeDrawing() )
		{
			BitmapBuffer *pDestBuffer = StretchAndConvert( *pSrcBuffer, aPosAry, JavaSalBitmap::Get32BitNativeFormat() | getBitmapDirectionFormat() );
			if ( pDestBuffer )
			{
				if ( pDestBuffer->mpBits )
				{
					// Mark all non-black pixels as transparent
					nMaskColor |= 0xff000000;
					long nBits = pDestBuffer->mnWidth * pDestBuffer->mnHeight;
					sal_uInt32 *pBits = (sal_uInt32 *)pDestBuffer->mpBits;
					for ( long i = 0; i < nBits; i++ )
					{
						if ( pBits[ i ] == 0xff000000 )
							pBits[ i ] = nMaskColor;
						else
							pBits[ i ] = 0x00000000;
					}

					// Assign ownership of bits to a CGDataProvider instance
					CGDataProviderRef aProvider = CGDataProviderCreateWithData( NULL, pDestBuffer->mpBits, pDestBuffer->mnScanlineSize * pDestBuffer->mnHeight, ReleaseBitmapBufferBytePointerCallback );
					if ( aProvider )
					{
						pDestBuffer->mpBits = NULL;
						addUndrawnNativeOp( new JavaSalGraphicsDrawImageOp( maNativeClipPath, mbInvert, mbXOR, aProvider, pDestBuffer->mnBitCount, pDestBuffer->mnScanlineSize, pDestBuffer->mnWidth, pDestBuffer->mnHeight, CGRectMake( 0, 0, pDestBuffer->mnWidth, pDestBuffer->mnHeight ), CGRectMake( aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight ) ) );
						CGDataProviderRelease( aProvider );
					}
					else
					{
						delete[] pDestBuffer->mpBits;
					}
				}

				delete pDestBuffer;
			}
		}
		else if ( mpVCLGraphics )
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
						sal_uInt32 *pBits = (sal_uInt32 *)t.pEnv->GetPrimitiveArrayCritical( (jintArray)pData->getJavaObject(), &bCopy );
						if ( pBits )
						{
							BitmapBuffer *pDestBuffer = StretchAndConvert( *pSrcBuffer, aPosAry, JavaSalBitmap::Get32BitNativeFormat() | getBitmapDirectionFormat(), NULL, NULL, (BYTE *)pBits );
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

								mpVCLGraphics->drawBitmap( &aVCLBitmap, 0, 0, aPosAry.mnSrcWidth, aPosAry.mnSrcHeight, aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight, NULL );

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

#ifndef USE_NATIVE_WINDOW
	// Fix bug 3189 and wipe down presentation transition previews without
	// causing bug 3191 by flushing in certain cases
	if ( !useNativeDrawing() )
		com_sun_star_vcl_VCLFrame::flushAllFrames();
#endif	// !USE_NATIVE_WINDOW

	JavaSalBitmap *pBitmap = new JavaSalBitmap();

	if ( !pBitmap->Create( Point( nX, nY ), Size( nDX, nDY ), this, BitmapPalette() ) )
	{
		delete pBitmap;
		pBitmap = NULL;
	}

	return pBitmap;
}

// -----------------------------------------------------------------------

SalColor JavaSalGraphics::getPixel( long nX, long nY )
{
	SalColor nRet = 0xff000000;

	// Don't do anything if this is a printer
	if ( mpPrinter )
		return nRet;

	if ( useNativeDrawing() )
	{
		if ( !maPixelContext )
		{
			CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
			if ( aColorSpace )
			{
				maPixelContext = CGBitmapContextCreate( &mnPixelContextData, 1, 1, 8, sizeof( mnPixelContextData ), aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little );
				CGColorSpaceRelease( aColorSpace );
			}
		}

		// Draw to a 1 x 1 pixel native bitmap
		if ( maPixelContext )
		{
			mnPixelContextData = 0;
			copyToContext( NULL, false, false, maPixelContext, CGRectMake( 0, 0, 1, 1 ), CGPointMake( nX, nY ), CGRectMake( 0, 0, 1, 1 ) );
			nRet = mnPixelContextData & 0x00ffffff;
		}
	}
	else if ( mpVCLGraphics )
	{
		nRet = mpVCLGraphics->getPixel( nX, nY ) & 0x00ffffff;
	}

	return nRet;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags )
{
	// Don't do anything if this is a printer
	if ( mpPrinter )
		return;

	if ( useNativeDrawing() )
	{
		CGMutablePathRef aPath = CGPathCreateMutable();
		if ( aPath )
		{
			CGRect aRect = CGRectStandardize( CGRectMake( nX, nY, nWidth, nHeight ) );
			CGPathAddRect( aPath, NULL, aRect );
			float fNativeLineWidth = getNativeLineWidth();
			if ( aRect.size.width < fNativeLineWidth )
				aRect.size.width = 0;
			if ( aRect.size.height < fNativeLineWidth )
				aRect.size.height = 0;
			if ( CGRectIsEmpty( aRect ) )
			{
				CGPathRelease( aPath );
				aPath = CGPathCreateMutable();
				if ( aPath )
				{
					CGPathMoveToPoint( aPath, NULL, aRect.origin.x, aRect.origin.y );
					CGPathAddLineToPoint( aPath, NULL, aRect.origin.x + aRect.size.width, aRect.origin.y + aRect.size.height );
				}
			}

			if ( aPath )
			{
				if ( nFlags & SAL_INVERT_50 )
				{
					// Fix bug 3443 by filling with gray instead of the
					// checkerboard pattern
					addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, false, true, false, 0xff000000, 0x00000000, aPath ) );
				}
				else if ( SAL_INVERT_TRACKFRAME )
				{
					addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, false, true, false, 0x00000000, 0xff000000, aPath, 0, ::basegfx::B2DLINEJOIN_NONE, true ) );
				}
				else
				{
					addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, true, false, false, 0xffffffff, 0x00000000, aPath ) );
				}
			}

			CGPathRelease( aPath );
		}
	}
	else if ( mpVCLGraphics )
	{
		mpVCLGraphics->invert( nX, nY, nWidth, nHeight, nFlags );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nFlags )
{
	// Don't do anything if this is a printer
	if ( mpPrinter )
		return;

	if ( useNativeDrawing() )
	{
		if ( nPoints && pPtAry )
		{
			::basegfx::B2DPolygon aPoly;
			for ( ULONG i = 0 ; i < nPoints; i++ )
				aPoly.append( ::basegfx::B2DPoint( pPtAry[ i ].mnX, pPtAry[ i ].mnY ) );
			aPoly.removeDoublePoints();
			aPoly.setClosed( true );

			CGMutablePathRef aPath = CGPathCreateMutable();
			if ( aPath )
			{
				AddPolygonToPaths( NULL, aPath, aPoly, aPoly.isClosed() );
				CGRect aRect = CGPathGetBoundingBox( aPath );
				float fNativeLineWidth = getNativeLineWidth();
				if ( aRect.size.width < fNativeLineWidth )
					aRect.size.width = 0;
				if ( aRect.size.height < fNativeLineWidth )
					aRect.size.height = 0;
				if ( CGRectIsEmpty( aRect ) )
				{
					CGPathRelease( aPath );
					aPath = CGPathCreateMutable();
					if ( aPath )
					{
						CGPathMoveToPoint( aPath, NULL, aRect.origin.x, aRect.origin.y );
						CGPathAddLineToPoint( aPath, NULL, aRect.origin.x + aRect.size.width, aRect.origin.y + aRect.size.height );
					}
				}

				if ( aPath )
				{
					if ( nFlags & SAL_INVERT_50 )
					{
						// Fix bug 3443 by filling with gray instead of the
						// checkerboard pattern
						addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, false, true, false, 0xff000000, 0x00000000, aPath ) );
					}
					else if ( SAL_INVERT_TRACKFRAME )
					{
						addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, false, true, false, 0x00000000, 0xff000000, aPath, 0, ::basegfx::B2DLINEJOIN_NONE, true ) );
					}
					else
					{
						addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, true, false, false, 0xffffffff, 0x00000000, aPath ) );
					}
				}

				CGPathRelease( aPath );
			}
		}
	}
	else if ( mpVCLGraphics )
	{
		mpVCLGraphics->invert( nPoints, pPtAry, nFlags );
	}
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawAlphaBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSourceBitmap, const SalBitmap& rAlphaBitmap )
{
	// Don't do anything if the source is not a printer
	if ( !mpPrinter )
		return false;

	JavaSalBitmap *pJavaSalBitmap = (JavaSalBitmap *)&rSourceBitmap;
	JavaSalBitmap *pTransJavaSalBitmap = (JavaSalBitmap *)&rAlphaBitmap;

	SalTwoRect aPosAry;
	memcpy( &aPosAry, &rPosAry, sizeof( SalTwoRect ) );

	// Adjust the source and destination to eliminate unnecessary copying
	float fScaleX = (float)aPosAry.mnDestWidth / aPosAry.mnSrcWidth;
	float fScaleY = (float)aPosAry.mnDestHeight / aPosAry.mnSrcHeight;
	if ( aPosAry.mnSrcX < 0 )
	{
		aPosAry.mnSrcWidth += aPosAry.mnSrcX;
		if ( aPosAry.mnSrcWidth < 1 )
			return true;
		aPosAry.mnDestWidth = (long)( ( fScaleX * aPosAry.mnSrcWidth ) + 0.5 );
		aPosAry.mnDestX -= (long)( ( fScaleX * aPosAry.mnSrcX ) + 0.5 );
		aPosAry.mnSrcX = 0;
	}
	if ( aPosAry.mnSrcY < 0 )
	{
		aPosAry.mnSrcHeight += aPosAry.mnSrcY;
		if ( aPosAry.mnSrcHeight < 1 )
			return true;
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
			return true;
		aPosAry.mnDestWidth = (long)( ( fScaleX * aPosAry.mnSrcWidth ) + 0.5 );
	}
	if ( nExcessHeight > 0 )
	{
		aPosAry.mnSrcHeight -= nExcessHeight;
		if ( aPosAry.mnSrcHeight < 1 )
			return true;
		aPosAry.mnDestHeight = (long)( ( fScaleY * aPosAry.mnSrcHeight ) + 0.5 );
	}

	if ( aPosAry.mnDestX < 0 )
	{
		aPosAry.mnDestWidth += aPosAry.mnDestX;
		if ( aPosAry.mnDestWidth < 1 )
			return true;
		aPosAry.mnSrcWidth = (long)( ( aPosAry.mnDestWidth / fScaleX ) + 0.5 );
		aPosAry.mnSrcX -= (long)( ( aPosAry.mnDestX / fScaleX ) + 0.5 );
		aPosAry.mnDestX = 0;
	}
	if ( aPosAry.mnDestY < 0 )
	{
		aPosAry.mnDestHeight += aPosAry.mnDestY;
		if ( aPosAry.mnDestHeight < 1 )
			return true;
		aPosAry.mnSrcHeight = (long)( ( aPosAry.mnDestHeight / fScaleY ) + 0.5 );
		aPosAry.mnSrcY -= (long)( ( aPosAry.mnDestY / fScaleY ) + 0.5 );
		aPosAry.mnDestY = 0;
	}

	if ( aPosAry.mnSrcWidth < 1 || aPosAry.mnSrcHeight < 1 || aPosAry.mnDestWidth < 1 || aPosAry.mnDestHeight < 1 )
		return true;

	// Always make a copy so that we can mask out the appropriate bits
	BitmapBuffer *pTransSrcBuffer = pTransJavaSalBitmap->AcquireBuffer( TRUE );
	if ( pTransSrcBuffer )
	{
		SalTwoRect aCopyPosAry;
		memcpy( &aCopyPosAry, &aPosAry, sizeof( SalTwoRect ) );
		aCopyPosAry.mnDestX = 0;
		aCopyPosAry.mnDestY = 0;
		aCopyPosAry.mnDestWidth = aCopyPosAry.mnSrcWidth;
		aCopyPosAry.mnDestHeight = aCopyPosAry.mnSrcHeight;

		// Fix bug 2475 by handling the case where the transparent bitmap is
		// smaller than the main bitmap
		SalTwoRect aTransPosAry;
		memcpy( &aTransPosAry, &aCopyPosAry, sizeof( SalTwoRect ) );
		Size aTransSize( pTransJavaSalBitmap->GetSize() );
		long nTransExcessWidth = aCopyPosAry.mnSrcX + aCopyPosAry.mnSrcWidth - aTransSize.Width();
		if ( nTransExcessWidth > 0 )
		{
			aTransPosAry.mnSrcWidth -= nTransExcessWidth;
			aTransPosAry.mnDestWidth = aTransPosAry.mnSrcWidth;
		}
		long nTransExcessHeight = aCopyPosAry.mnSrcY + aCopyPosAry.mnSrcHeight - aTransSize.Height();
		if ( nTransExcessHeight > 0 )
		{
			aTransPosAry.mnSrcHeight -= nTransExcessHeight;
			aTransPosAry.mnDestHeight = aTransPosAry.mnSrcHeight;
		}
		BitmapBuffer *pTransDestBuffer = StretchAndConvert( *pTransSrcBuffer, aTransPosAry, BMP_FORMAT_8BIT_PAL | getBitmapDirectionFormat(), &pTransSrcBuffer->maPalette );
		if ( pTransDestBuffer )
		{
			if ( pTransDestBuffer->mpBits )
			{
				BitmapBuffer *pSrcBuffer = pJavaSalBitmap->AcquireBuffer( TRUE );
				if ( pSrcBuffer )
				{
					BitmapBuffer *pCopyBuffer = StretchAndConvert( *pSrcBuffer, aCopyPosAry, JavaSalBitmap::Get32BitNativeFormat() | getBitmapDirectionFormat() );
					if ( pCopyBuffer )
					{
						sal_uInt32 *pBits = (sal_uInt32 *)pCopyBuffer->mpBits;
						if ( pCopyBuffer->mpBits )
						{
							Scanline pTransBits = (Scanline)pTransDestBuffer->mpBits;
							FncGetPixel pFncGetPixel = BitmapReadAccess::GetPixelFor_8BIT_PAL;

							pTransBits = (Scanline)pTransDestBuffer->mpBits;
							for ( int i = 0; i < pCopyBuffer->mnHeight; i++ )
							{
								bool bTransPixels = ( i < pTransDestBuffer->mnHeight );
								for ( int j = 0; j < pCopyBuffer->mnWidth; j++ )
								{
									if ( bTransPixels && j < pTransDestBuffer->mnWidth )
									{
										BYTE nAlpha = ~pFncGetPixel( pTransBits, j, pTransDestBuffer->maColorMask );
										if ( !nAlpha )
										{
											pBits[ j ] = 0x00000000;
										}
										else if ( nAlpha != 0xff )
										{
											// Fix bugs 2549 and 2576 by
											// premultiplying the colors
											float fTransPercent = (float)nAlpha / 0xff;
											pBits[ j ] =  ( ( (SalColor)( (BYTE)( pBits[ j ] >> 24 ) * fTransPercent ) << 24 ) & 0xff000000 ) | ( ( (SalColor)( (BYTE)( pBits[ j ] >> 16 ) * fTransPercent ) << 16 ) & 0x00ff0000 )  | ( ( (SalColor)( (BYTE)( pBits[ j ] >> 8 ) * fTransPercent ) << 8 ) & 0x0000ff00 )  | ( (SalColor)( (BYTE)( pBits[ j ] ) * fTransPercent ) & 0x000000ff );
										}
									}
									else
									{
										pBits[ j ] = 0x00000000;
									}
								}

								pBits += pCopyBuffer->mnWidth;
								pTransBits += pTransDestBuffer->mnScanlineSize;
							}
						}

						if ( useNativeDrawing() )
						{
							// Assign ownership of bits to a CGDataProvider
							// instance
							CGDataProviderRef aProvider = CGDataProviderCreateWithData( NULL, pCopyBuffer->mpBits, pCopyBuffer->mnScanlineSize * pCopyBuffer->mnHeight, ReleaseBitmapBufferBytePointerCallback );
							if ( aProvider )
							{
								pCopyBuffer->mpBits = NULL;
								addUndrawnNativeOp( new JavaSalGraphicsDrawImageOp( maNativeClipPath, mbInvert, mbXOR, aProvider, pCopyBuffer->mnBitCount, pCopyBuffer->mnScanlineSize, pCopyBuffer->mnWidth, pCopyBuffer->mnHeight, CGRectMake( 0, 0, pCopyBuffer->mnWidth, pCopyBuffer->mnHeight ), CGRectMake( aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight ) ) );
								CGDataProviderRelease( aProvider );
							}
							else
							{
								delete[] pCopyBuffer->mpBits;
							}
		
							delete pCopyBuffer;
						}
						else if ( mpVCLGraphics )
						{
							// Don't delete the bitmap buffer and let the Java
							// native method print the bitmap buffer directly
							mpVCLGraphics->drawBitmapBuffer( pCopyBuffer, 0, 0, pCopyBuffer->mnWidth, pCopyBuffer->mnHeight, aPosAry.mnDestX, aPosAry.mnDestY, aPosAry.mnDestWidth, aPosAry.mnDestHeight, maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
						}
					}

					pJavaSalBitmap->ReleaseBuffer( pSrcBuffer, TRUE );
				}

				delete[] pTransDestBuffer->mpBits;
			}

			delete pTransDestBuffer;
		}

		pTransJavaSalBitmap->ReleaseBuffer( pTransSrcBuffer, TRUE );
	}

	return true;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::supportsOperation( OutDevSupportType eType ) const
{
	bool bRet = false;

	switch( eType )
	{
		case OutDevSupport_B2DClip:
		case OutDevSupport_B2DDraw:
		case OutDevSupport_TransparentRect:
			bRet = true;
			break;
		default:
			break;
	}

	return bRet;
}

// -----------------------------------------------------------------------

SystemGraphicsData JavaSalGraphics::GetGraphicsData() const
{
	SystemGraphicsData aRes;
	aRes.nSize = sizeof( SystemGraphicsData );
	aRes.rCGContext = NULL;
	return aRes; 
}
