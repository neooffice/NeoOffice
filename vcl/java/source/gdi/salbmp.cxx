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

#include <salbmp.h>
#include <saldata.hxx>
#include <salgdi.h>
#include <vcl/bmpacc.hxx>

using namespace vcl;

// ==================================================================
 
void ReleaseBitmapBufferBytePointerCallback( void *pInfo, const void *pPointer, size_t nSize )
{
	BYTE *pBits = (BYTE *)pPointer;
	if ( pBits )
		delete[] pBits;

}

// ==================================================================

ULONG JavaSalBitmap::Get32BitNativeFormat()
{
	return BMP_FORMAT_32BIT_TC_BGRA;
}

// ------------------------------------------------------------------

ULONG JavaSalBitmap::GetNativeDirectionFormat()
{
	return BMP_FORMAT_TOP_DOWN;
}

// ------------------------------------------------------------------

JavaSalBitmap::JavaSalBitmap() :
	maSize( 0, 0 ),
	mnBitCount( 0 ),
	mpBits( NULL ),
	mpBuffer( NULL ),
	mpGraphics( NULL )
{
	GetSalData()->maBitmapList.push_back( this );
}

// ------------------------------------------------------------------

JavaSalBitmap::~JavaSalBitmap()
{
	GetSalData()->maBitmapList.remove( this );
	Destroy();
}

// ------------------------------------------------------------------

void JavaSalBitmap::NotifyGraphicsChanged( bool bDisposed )
{
	// Force copying of the buffer if it has not already been done
	if ( mpGraphics )
	{
		mpGraphics->removeGraphicsChangeListener( this );

		if ( !bDisposed && !mpBits )
		{
			// Use generic color space since the OOo code creates bitmaps using
			// its own implied or explicit color space so we need to be
			// consistent with that when creating bitmaps from a native graphics
			CGColorSpaceRef aColorSpace = CGColorSpaceCreateWithName( kCGColorSpaceGenericRGB );
			if ( aColorSpace )
			{
				long nScanlineSize = AlignedWidth4Bytes( mnBitCount * maSize.Width() );
				long nCapacity = nScanlineSize * maSize.Height();
				// Force copying of the buffer
				try
				{
					mpBits = new BYTE[ nCapacity ];
				}
				catch( const std::bad_alloc& ) {}
				if ( mpBits )
				{
					memset( mpBits, 0, nCapacity );

					CGContextRef aContext = CGBitmapContextCreate( mpBits, maSize.Width(), maSize.Height(), 8, nScanlineSize, aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little );
					if ( aContext )
					{
						CGRect aUnflippedSrcRect = UnflipFlippedRect( CGRectMake( maPoint.X(), maPoint.Y(), maSize.Width(), maSize.Height() ), mpGraphics->maNativeBounds );
						CGRect aDestRect = CGRectMake( 0, 0, maSize.Width(), maSize.Height() );
						mpGraphics->copyToContext( NULL, NULL, false, false, aContext, aDestRect, aUnflippedSrcRect, aDestRect );

						CGContextRelease( aContext );
					}
				}

				CGColorSpaceRelease( aColorSpace );
			}
		}
	}

	mpGraphics = NULL;
	maPoint = Point( 0, 0 );
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

bool JavaSalBitmap::Create( const Point& rPoint, const Size& rSize, JavaSalGraphics *pGraphics, const BitmapPalette& rPal )
{
	Destroy();

	if ( !pGraphics )
		return false;

	long nX = rPoint.X();
	long nY = rPoint.Y();
	long nWidth = rSize.Width();
	long nHeight = rSize.Height();

	maPoint = Point( nX, nY );
	maSize = Size( nWidth, nHeight );

	if ( maSize.Width() <= 0 || maSize.Height() <= 0 )
		return false;

	mpGraphics = pGraphics;
	mnBitCount = mpGraphics->GetBitCount();

	// Save the palette
	USHORT nColors = ( ( mnBitCount <= 8 ) ? ( 1 << mnBitCount ) : 0 );
	if ( nColors )
	{
		maPalette = rPal;
		maPalette.SetEntryCount( nColors );
	}

	NotifyGraphicsChanged( false );

	return true;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Create( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal )
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
	USHORT nColors = ( ( mnBitCount <= 8 ) ? ( 1 << mnBitCount ) : 0 );
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
		BitmapBuffer *pSrcBuffer = rJavaSalBmp.AcquireBuffer( TRUE );
		if ( pSrcBuffer )
		{
			BitmapBuffer *pDestBuffer = AcquireBuffer( FALSE );
			if ( pDestBuffer && pDestBuffer->mpBits && pDestBuffer->mnScanlineSize == pSrcBuffer->mnScanlineSize && pDestBuffer->mnHeight == pSrcBuffer->mnHeight )
			{
				memcpy( pDestBuffer->mpBits, pSrcBuffer->mpBits, pDestBuffer->mnScanlineSize * pDestBuffer->mnHeight );
				pDestBuffer->maColorMask = pSrcBuffer->maColorMask;
				pDestBuffer->maPalette = pSrcBuffer->maPalette;
				ReleaseBuffer( pDestBuffer, FALSE );
			}
			else
			{
				Destroy();
				bRet = false;
			}
			rJavaSalBmp.ReleaseBuffer( pSrcBuffer, TRUE );
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

bool JavaSalBitmap::Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics )
{
	return false;
}

// ------------------------------------------------------------------

bool JavaSalBitmap::Create( const SalBitmap& rSalBmp, USHORT nNewBitCount )
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

	if ( mpGraphics )
	{
		mpGraphics->removeGraphicsChangeListener( this );
		mpGraphics = NULL;
	}
}

// ------------------------------------------------------------------

USHORT JavaSalBitmap::GetBitCount() const
{
	return mnBitCount;
}

// ------------------------------------------------------------------

BitmapBuffer* JavaSalBitmap::AcquireBuffer( bool bReadOnly )
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

	NotifyGraphicsChanged( false );

	if ( !mpBits )
	{
		try
		{
			mpBits = new BYTE[ pBuffer->mnScanlineSize * pBuffer->mnHeight ];
		}
		catch( const std::bad_alloc& ) {}
		if ( mpBits )
		{
			memset( mpBits, 0, pBuffer->mnScanlineSize * pBuffer->mnHeight );
		}
		else
		{
			delete pBuffer;
			return NULL;
		}
	}

	pBuffer->mpBits = mpBits;

	return pBuffer;
}

// ------------------------------------------------------------------

void JavaSalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly )
{
	if ( pBuffer )
	{
		if ( !bReadOnly )
		{
			// Save the palette
			USHORT nColors = ( ( mnBitCount <= 8 ) ? ( 1 << mnBitCount ) : 0 );
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

bool JavaSalBitmap::GetSystemData( BitmapSystemData& rData )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalBitmap::GetSystemData not implemented\n" );
#endif
	return false;
}
