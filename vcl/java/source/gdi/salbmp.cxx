/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <vcl/bmpacc.hxx>

#include "java/salbmp.h"
#include "java/saldata.hxx"
#include "java/salgdi.h"
#include "java/salinst.h"

using namespace vcl;

// ==================================================================
 
void ReleaseBitmapBufferBytePointerCallback( void* /* pInfo */, const void *pPointer, size_t /* nSize */ )
{
	sal_uInt8 *pBits = (sal_uInt8 *)pPointer;
	if ( pBits )
		delete[] pBits;

}

// ==================================================================

sal_uLong JavaSalBitmap::Get32BitNativeFormat()
{
	return BMP_FORMAT_32BIT_TC_BGRA;
}

// ------------------------------------------------------------------

sal_uLong JavaSalBitmap::GetNativeDirectionFormat()
{
	return BMP_FORMAT_TOP_DOWN;
}

// ------------------------------------------------------------------

JavaSalBitmap::JavaSalBitmap() :
	maPoint( 0, 0 ),
	maSize( 0, 0 ),
	mnBitCount( 0 ),
	mpBits( NULL ),
	mpBuffer( NULL ),
	mpGraphics( NULL ),
	mpVirDev( NULL )
{
}

// ------------------------------------------------------------------

JavaSalBitmap::~JavaSalBitmap()
{
	Destroy();
}

// ------------------------------------------------------------------

/**
 * Warning: this method takes ownership of the BitmapBuffer's mpBits member
 * so after calling this method, the mpBits member will be set to NULL.
 */
bool JavaSalBitmap::Create( BitmapBuffer *pBuffer )
{
	Destroy();

	bool bRet = false;

	if ( !pBuffer || !pBuffer->mpBits )
		return bRet;

	Size aSize( pBuffer->mnWidth, pBuffer->mnHeight );
	bRet = Create( aSize, pBuffer->mnBitCount, pBuffer->maPalette );
	if ( bRet )
	{
		mpBits = pBuffer->mpBits;
		pBuffer->mpBits = NULL;
	}

	return bRet;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Create( const Point& rPoint, const Size& rSize, JavaSalGraphics *pSrcGraphics, const BitmapPalette& rPal )
{
	Destroy();

	if ( !pSrcGraphics )
		return false;

	maSize = Size( rSize.Width(), rSize.Height() );

	if ( maSize.Width() <= 0 || maSize.Height() <= 0 )
		return false;

	mnBitCount = pSrcGraphics->GetBitCount();

	// Save the palette
	sal_uInt16 nColors = ( ( mnBitCount <= 8 ) ? ( 1 << mnBitCount ) : 0 );
	if ( nColors )
	{
		maPalette = rPal;
		maPalette.SetEntryCount( nColors );
	}

	float fLineWidth = pSrcGraphics->getNativeLineWidth();
	if ( fLineWidth > 0 )
	{
		long nPadding = (long)( fLineWidth + 0.5 );
		maPoint = Point( nPadding, nPadding );
	}

	JavaSalInstance *pInst = GetSalData()->mpFirstInstance;
	if ( pInst )
	{
		long nWidth = maSize.Width() + ( maPoint.X() * 2 );
		long nHeight = maSize.Height() + ( maPoint.Y() * 2 );
		mpVirDev = (JavaSalVirtualDevice *)pInst->CreateVirtualDevice( pSrcGraphics, nWidth, nHeight, mnBitCount, NULL );
		if ( mpVirDev )
		{
			mpGraphics = (JavaSalGraphics *)mpVirDev->AcquireGraphics();
			if ( mpGraphics )
			{
				CGRect aUnflippedSrcRect = UnflipFlippedRect( CGRectMake( rPoint.X(), rPoint.Y(), maSize.Width(), maSize.Height() ), pSrcGraphics->maNativeBounds );
				mpGraphics->copyFromGraphics( pSrcGraphics, aUnflippedSrcRect, CGRectMake( maPoint.X(), maPoint.Y(), maSize.Width(), maSize.Height() ), false );
			}
		}
	}

	return true;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Create( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal )
{
	Destroy();

	maSize = Size( rSize );
	if ( maSize.Width() <= 0 || maSize.Height() <= 0 )
		return false;

	if ( nBitCount <= 1 )
		mnBitCount = 1;
	else if ( nBitCount <= 4 )
		mnBitCount = 4;
	else if ( nBitCount <= 8 )
		mnBitCount = 8;
	else if ( nBitCount <= 16 )
		mnBitCount = 16;
	else if ( nBitCount <= 24 )
		mnBitCount = 24;
	else
		mnBitCount = 32;

	// Save the palette
	sal_uInt16 nColors = ( ( mnBitCount <= 8 ) ? ( 1 << mnBitCount ) : 0 );
	if ( nColors )
	{
		maPalette = rPal;
		maPalette.SetEntryCount( nColors );
	}

	return true;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Create( const SalBitmap& rSalBmp )
{
	Destroy();

	JavaSalBitmap& rJavaSalBmp = (JavaSalBitmap&)rSalBmp;
	bool bRet = Create( rJavaSalBmp.GetSize(), rJavaSalBmp.GetBitCount(), rJavaSalBmp.maPalette );

	if ( bRet )
	{
		BitmapBuffer *pSrcBuffer = rJavaSalBmp.AcquireBuffer( BITMAP_WRITE_ACCESS );
		if ( pSrcBuffer )
		{
			BitmapBuffer *pDestBuffer = AcquireBuffer( BITMAP_READ_ACCESS );
			if ( pDestBuffer && pDestBuffer->mpBits && pDestBuffer->mnScanlineSize == pSrcBuffer->mnScanlineSize && pDestBuffer->mnHeight == pSrcBuffer->mnHeight )
			{
				memcpy( pDestBuffer->mpBits, pSrcBuffer->mpBits, pDestBuffer->mnScanlineSize * pDestBuffer->mnHeight );
				pDestBuffer->maColorMask = pSrcBuffer->maColorMask;
				pDestBuffer->maPalette = pSrcBuffer->maPalette;
				ReleaseBuffer( pDestBuffer, BITMAP_READ_ACCESS );
			}
			else
			{
				Destroy();
				bRet = false;
			}
			rJavaSalBmp.ReleaseBuffer( pSrcBuffer, BITMAP_READ_ACCESS );
		}
		else
		{
			Destroy();
			bRet = false;
		}
	}

	return bRet;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Create( const SalBitmap& /* rSalBmp */, SalGraphics* /* pGraphics */ )
{
	return false;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Create( const SalBitmap& /* rSalBmp */, sal_uInt16 /* nNewBitCount */ )
{
	return false;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Create( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > /* xBitmapCanvas */, Size& /* rSize */, bool /** bMask */ )
{
	return false;
}

// ------------------------------------------------------------------

void JavaSalBitmap::Destroy()
{
	maPoint = Point( 0, 0 );
	maSize = Size( 0, 0 );
	mnBitCount = 0;

	if ( mpBits )
	{
		delete[] mpBits;
		mpBits = NULL;
	}

	if ( mpBuffer )
	{
		delete mpBuffer;
		mpBuffer = NULL;
	}

	maPalette.SetEntryCount( 0 );

	if ( mpVirDev )
	{
		if ( mpGraphics )
			mpVirDev->ReleaseGraphics( mpGraphics );
		delete mpVirDev;
	}

	mpGraphics = NULL;
	mpVirDev = NULL;
}

// ------------------------------------------------------------------

sal_uInt16 JavaSalBitmap::GetBitCount() const
{
	return mnBitCount;
}

// ------------------------------------------------------------------

BitmapBuffer* JavaSalBitmap::AcquireBuffer( BitmapAccessMode nMode )
{
	BitmapBuffer *pBuffer = new BitmapBuffer();

	pBuffer->mnBitCount = mnBitCount;
	pBuffer->mnFormat = JavaSalBitmap::GetNativeDirectionFormat();
	if ( mnBitCount <= 1 )
	{
		pBuffer->mnFormat |= BMP_FORMAT_1BIT_MSB_PAL;
	}
	else if ( mnBitCount <= 4 )
	{
		pBuffer->mnFormat |= BMP_FORMAT_4BIT_MSN_PAL;
	}
	else if ( mnBitCount <= 8 )
	{
		pBuffer->mnFormat |= BMP_FORMAT_8BIT_PAL;
	}
	else if ( mnBitCount <= 16 )
	{
		pBuffer->mnFormat |= BMP_FORMAT_16BIT_TC_MSB_MASK;
		pBuffer->maColorMask = ColorMask( 0x7c00, 0x03e0, 0x001f );
	}
	else if ( mnBitCount <= 24 )
	{
		pBuffer->mnFormat |= BMP_FORMAT_24BIT_TC_RGB;
	}
	else
	{
		pBuffer->mnFormat |= JavaSalBitmap::Get32BitNativeFormat();
	}

	pBuffer->mnWidth = maSize.Width();
	pBuffer->mnHeight = maSize.Height();
	pBuffer->mnScanlineSize = AlignedWidth4Bytes( mnBitCount * maSize.Width() );
	pBuffer->maPalette = maPalette;

	if ( !mpBits )
	{
		try
		{
			// If the total bytes is more than 32 bits, loading the
			// vcl/qa/cppunit/graphicfilter/data/bmp/pass/CVE-2007-3741-1.bmp
			// image will cause the unit tests to crash when memset() is called
			// on the allocated buffer
			if ( pBuffer->mnScanlineSize <= std::numeric_limits< sal_uInt32 >::max() / pBuffer->mnHeight )
				mpBits = new sal_uInt8[ pBuffer->mnScanlineSize * pBuffer->mnHeight ];
		}
		catch( const std::bad_alloc& ) {}
		if ( mpBits )
		{
			memset( mpBits, 0, pBuffer->mnScanlineSize * pBuffer->mnHeight );

			if ( mpGraphics )
			{
				CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
				if ( aColorSpace )
				{
					CGContextRef aContext = CGBitmapContextCreate( mpBits, pBuffer->mnWidth, pBuffer->mnHeight, 8, pBuffer->mnScanlineSize, aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little );
					if ( aContext )
					{
						CGRect aSrcRect = CGRectMake( maPoint.X(), maPoint.Y(), pBuffer->mnWidth, pBuffer->mnHeight );
						CGRect aDestRect = CGRectMake( 0, 0, pBuffer->mnWidth, pBuffer->mnHeight );
						mpGraphics->copyToContext( NULL, NULL, false, false, aContext, aDestRect, aSrcRect, aDestRect );

						CGContextRelease( aContext );
					}

					CGColorSpaceRelease( aColorSpace );
				}
			}
		}
		else
		{
			delete pBuffer;
			return NULL;
		}
	}

	if ( nMode == BITMAP_WRITE_ACCESS )
	{
		// Release the virtual device if the bits will change
		if ( mpVirDev )
		{
			if ( mpGraphics )
				mpVirDev->ReleaseGraphics( mpGraphics );
			delete mpVirDev;
		}

		maPoint = Point( 0, 0 );
		mpGraphics = NULL;
		mpVirDev = NULL;
	}

	pBuffer->mpBits = mpBits;

	return pBuffer;
}

// ------------------------------------------------------------------

void JavaSalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode )
{
	if ( pBuffer )
	{
		if ( nMode == BITMAP_WRITE_ACCESS )
		{
			// Save the palette
			sal_uInt16 nColors = ( ( mnBitCount <= 8 ) ? ( 1 << mnBitCount ) : 0 );
			if ( nColors )
			{
				maPalette = pBuffer->maPalette;
				maPalette.SetEntryCount( nColors );
			}
		}

		delete pBuffer;
	}
}

// ------------------------------------------------------------------

bool JavaSalBitmap::GetSystemData( BitmapSystemData& /* rData */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalBitmap::GetSystemData not implemented\n" );
#endif
	return false;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Crop( const Rectangle& /* rRectPixel */ )
{
	return false;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Erase( const Color& /* rFillColor */ )
{
	return false;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Scale( const double& /* rScaleX */, const double& /* rScaleY */, sal_uInt32 /* nScaleFlag */ )
{
	return false;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Replace( const Color& /* rSearchColor */, const Color& /* rReplaceColor */, sal_uLong /* nTol */ )
{
	return false;
}
