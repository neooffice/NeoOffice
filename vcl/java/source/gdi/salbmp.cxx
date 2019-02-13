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

#include <vcl/bitmapaccess.hxx>

#include "java/salbmp.h"
#include "java/saldata.hxx"
#include "java/salgdi.h"
#include "java/salinst.h"

using namespace vcl;

// ==================================================================
 
void ReleaseBitmapBufferBytePointerCallback( void* /* pInfo */, const void *pPointer, size_t /* nSize */ )
{
	sal_uInt8 *pBits = static_cast< sal_uInt8* >( const_cast< void* >( pPointer ) );
	if ( pBits )
		delete[] pBits;

}

// ==================================================================

ScanlineFormat JavaSalBitmap::Get32BitNativeFormat()
{
	return ScanlineFormat::N32BitTcBgra;
}

// ------------------------------------------------------------------

ScanlineFormat JavaSalBitmap::GetNativeDirectionFormat()
{
	return ScanlineFormat::TopDown;
}

// ------------------------------------------------------------------

JavaSalBitmap::JavaSalBitmap() :
	maPoint( 0, 0 ),
	maSize( 0, 0 ),
	mnBitCount( 0 ),
	mpBits( nullptr ),
	mpBuffer( nullptr ),
	mpGraphics( nullptr ),
	mpVirDev( nullptr )
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
 * so after calling this method, the mpBits member will be set to nullptr.
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
		pBuffer->mpBits = nullptr;
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
		long nPadding = static_cast< long >( fLineWidth + 0.5 );
		maPoint = Point( nPadding, nPadding );
	}

	JavaSalInstance *pInst = GetSalData()->mpFirstInstance;
	if ( pInst )
	{
		long nWidth = maSize.Width() + ( maPoint.X() * 2 );
		long nHeight = maSize.Height() + ( maPoint.Y() * 2 );
		mpVirDev = static_cast< JavaSalVirtualDevice* >( pInst->CreateVirtualDevice( pSrcGraphics, nWidth, nHeight, DeviceFormat::DEFAULT, nullptr ) );
		if ( mpVirDev )
		{
			mpGraphics = static_cast< JavaSalGraphics* >( mpVirDev->AcquireGraphics() );
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

	JavaSalBitmap& rJavaSalBmp = static_cast< JavaSalBitmap& >( const_cast< SalBitmap& >( rSalBmp ) );
	bool bRet = Create( rJavaSalBmp.GetSize(), rJavaSalBmp.GetBitCount(), rJavaSalBmp.maPalette );

	if ( bRet )
	{
		BitmapBuffer *pSrcBuffer = rJavaSalBmp.AcquireBuffer( BitmapAccessMode::Write );
		if ( pSrcBuffer )
		{
			BitmapBuffer *pDestBuffer = AcquireBuffer( BitmapAccessMode::Read );
			if ( pDestBuffer && pDestBuffer->mpBits && pDestBuffer->mnScanlineSize == pSrcBuffer->mnScanlineSize && pDestBuffer->mnHeight == pSrcBuffer->mnHeight )
			{
				memcpy( pDestBuffer->mpBits, pSrcBuffer->mpBits, pDestBuffer->mnScanlineSize * pDestBuffer->mnHeight );
				pDestBuffer->maColorMask = pSrcBuffer->maColorMask;
				pDestBuffer->maPalette = pSrcBuffer->maPalette;
				ReleaseBuffer( pDestBuffer, BitmapAccessMode::Read );
			}
			else
			{
				Destroy();
				bRet = false;
			}
			rJavaSalBmp.ReleaseBuffer( pSrcBuffer, BitmapAccessMode::Read );
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

bool JavaSalBitmap::Create( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas >& /* rBitmapCanvas */, Size& /* rSize */, bool /* bMask */ )
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
		mpBits = nullptr;
	}

	if ( mpBuffer )
	{
		delete mpBuffer;
		mpBuffer = nullptr;
	}

	maPalette.SetEntryCount( 0 );

	if ( mpVirDev )
	{
		if ( mpGraphics )
			mpVirDev->ReleaseGraphics( mpGraphics );
		delete mpVirDev;
	}

	mpGraphics = nullptr;
	mpVirDev = nullptr;
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
		pBuffer->mnFormat |= ScanlineFormat::N1BitMsbPal;
	}
	else if ( mnBitCount <= 4 )
	{
		pBuffer->mnFormat |= ScanlineFormat::N4BitMsnPal;
	}
	else if ( mnBitCount <= 8 )
	{
		pBuffer->mnFormat |= ScanlineFormat::N8BitPal;
	}
	else if ( mnBitCount <= 16 )
	{
		pBuffer->mnFormat |= ScanlineFormat::N16BitTcMsbMask;
		ColorMaskElement aRedMask( 0x7c00 );
		aRedMask.CalcMaskShift();
		ColorMaskElement aGreenMask( 0x03e0 );
		aGreenMask.CalcMaskShift();
		ColorMaskElement aBlueMask( 0x001f );
		aBlueMask.CalcMaskShift();
		pBuffer->maColorMask = ColorMask( aRedMask, aGreenMask, aBlueMask );
	}
	else if ( mnBitCount <= 24 )
	{
		pBuffer->mnFormat |= ScanlineFormat::N24BitTcRgb;
	}
	else
	{
		pBuffer->mnFormat |= JavaSalBitmap::Get32BitNativeFormat();
		ColorMaskElement aRedMask( 0x00ff0000 );
		aRedMask.CalcMaskShift();
		ColorMaskElement aGreenMask( 0x0000ff00 );
		aGreenMask.CalcMaskShift();
		ColorMaskElement aBlueMask( 0x000000ff );
		aBlueMask.CalcMaskShift();
		pBuffer->maColorMask = ColorMask( aRedMask, aGreenMask, aBlueMask );
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
						mpGraphics->copyToContext( nullptr, nullptr, false, false, aContext, aDestRect, aSrcRect, aDestRect );

						CGContextRelease( aContext );
					}

					CGColorSpaceRelease( aColorSpace );
				}
			}
		}
		else
		{
			delete pBuffer;
			return nullptr;
		}
	}

	if ( nMode == BitmapAccessMode::Write )
	{
		// Release the virtual device if the bits will change
		if ( mpVirDev )
		{
			if ( mpGraphics )
				mpVirDev->ReleaseGraphics( mpGraphics );
			delete mpVirDev;
		}

		maPoint = Point( 0, 0 );
		mpGraphics = nullptr;
		mpVirDev = nullptr;
	}

	pBuffer->mpBits = mpBits;

	return pBuffer;
}

// ------------------------------------------------------------------

void JavaSalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode )
{
	if ( pBuffer )
	{
		if ( nMode == BitmapAccessMode::Write )
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

bool JavaSalBitmap::ScalingSupported() const
{
	return false;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Scale( const double& /* rScaleX */, const double& /* rScaleY */, BmpScaleFlag /* nScaleFlag */ )
{
	return false;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Replace( const Color& /* rSearchColor */, const Color& /* rReplaceColor */, sal_uLong /* nTol */ )
{
	return false;
}
