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

#include <salvd.h>
#include <salgdi.h>
#include <com/sun/star/vcl/VCLGraphics.hxx>
#include <com/sun/star/vcl/VCLImage.hxx>

using namespace vcl;

// =======================================================================

JavaSalVirtualDevice::JavaSalVirtualDevice() :
#ifdef USE_NATIVE_VIRTUAL_DEVICE
	mnWidth( 0 ),
	mnHeight( 0 ),
	mpBits( NULL ),
	maBitmapContext( NULL ),
	maBitmapLayer( NULL ),
#else	// !USE_NATIVE_VIRTUAL_DEVICE
	mpVCLImage( NULL ),
#endif	// !USE_NATIVE_VIRTUAL_DEVICE
	mnBitCount( 32 ),
	mpGraphics( new JavaSalGraphics() ),
	mbGraphics( FALSE )
{
	// By default no mirroring for VirtualDevices
	mpGraphics->SetLayout( 0 );
	mpGraphics->mpVirDev = this;
#ifdef USE_NATIVE_VIRTUAL_DEVICE
	mpGraphics->mnDPIX = MIN_SCREEN_RESOLUTION;
	mpGraphics->mnDPIY = MIN_SCREEN_RESOLUTION;
#endif	// USE_NATIVE_VIRTUAL_DEVICE
}

// -----------------------------------------------------------------------

JavaSalVirtualDevice::~JavaSalVirtualDevice()
{
	if ( mpGraphics )
		delete mpGraphics;

#ifdef USE_NATIVE_VIRTUAL_DEVICE
	if ( maBitmapLayer )
		CGLayerRelease( maBitmapLayer );

	if ( maBitmapContext )
		CGContextRelease( maBitmapContext );

	if ( mpBits )
		delete[] mpBits;
#else	// USE_NATIVE_VIRTUAL_DEVICE
	if ( mpVCLImage )
	{
		mpVCLImage->dispose();
		delete mpVCLImage;
	}
#endif	// USE_NATIVE_VIRTUAL_DEVICE
}

// -----------------------------------------------------------------------

SalGraphics* JavaSalVirtualDevice::GetGraphics()
{
	if ( mbGraphics )
		return NULL;

#ifndef USE_NATIVE_VIRTUAL_DEVICE
	if ( !mpGraphics->mpVCLGraphics )
		mpGraphics->mpVCLGraphics = mpVCLImage->getGraphics();
#endif	// !USE_NATIVE_VIRTUAL_DEVICE
	mbGraphics = TRUE;

	return mpGraphics;
}

// -----------------------------------------------------------------------

void JavaSalVirtualDevice::ReleaseGraphics( SalGraphics* pGraphics )
{
	if ( pGraphics != mpGraphics )
		return;

	mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL JavaSalVirtualDevice::SetSize( long nDX, long nDY )
{
	BOOL bRet = FALSE;

#ifdef USE_NATIVE_VIRTUAL_DEVICE
	mnWidth = 0;
	mnHeight = 0;

	mpGraphics->setLayer( NULL );

	if ( maBitmapLayer )
	{
		CGLayerRelease( maBitmapLayer );
		maBitmapLayer = NULL;
	}

	if ( nDX < 1 )
		nDX = 1;
	if ( nDY < 1 )
		nDY = 1;

	if ( !mpBits )
	{
		if ( maBitmapContext )
		{
			CGContextRelease( maBitmapContext );
			maBitmapContext = NULL;
		}

		long nScanlineSize = AlignedWidth4Bytes( mnBitCount );
		try
		{
			mpBits = new BYTE[ nScanlineSize ];
		}
		catch( const std::bad_alloc& ) {}

		// Make a native layer backed by a 1 x 1 pixel native bitmap
		if ( mpBits )
		{
			CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
			if ( aColorSpace )
			{
				memset( mpBits, 0, nScanlineSize );
				maBitmapContext = CGBitmapContextCreate( mpBits, 1, 1, 8, nScanlineSize, aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little );
			}
		}
	}

	if ( maBitmapContext )
	{
		maBitmapLayer = CGLayerCreateWithContext( maBitmapContext, CGSizeMake( nDX, nDY ), NULL );
		if ( maBitmapLayer )
		{
			mpGraphics->setLayer( maBitmapLayer );
			mnWidth = nDX;
			mnHeight = nDY;
			bRet = TRUE;
		}
	}
#else	// USE_NATIVE_VIRTUAL_DEVICE
	if ( mpGraphics->mpVCLGraphics )
	{
		delete mpGraphics->mpVCLGraphics;
		mpGraphics->mpVCLGraphics = NULL;
	}

	if ( mpVCLImage )
	{
		mpVCLImage->dispose();
		delete mpVCLImage;
		mpVCLImage = NULL;
	}

	if ( nDX > 0 && nDY > 0 )
	{
		com_sun_star_vcl_VCLImage *pVCLImage = new com_sun_star_vcl_VCLImage( nDX, nDY, mnBitCount );
		if ( pVCLImage )
		{
			if ( pVCLImage->getJavaObject() )
			{
				mpVCLImage = pVCLImage;
				bRet = TRUE;
			}
			else
			{
				delete pVCLImage;
			}
		}
	}

	if ( !mpVCLImage )
	{
		// Try to create something so that we don't crash
		com_sun_star_vcl_VCLImage *pVCLImage = new com_sun_star_vcl_VCLImage( 1, 1, mnBitCount );
		if ( pVCLImage )
		{
			if ( pVCLImage->getJavaObject() )
				mpVCLImage = pVCLImage;
			else
				delete pVCLImage;
		}
	}

	if ( mbGraphics && mpVCLImage )
		mpGraphics->mpVCLGraphics = mpVCLImage->getGraphics();
#endif	// USE_NATIVE_VIRTUAL_DEVICE

	return bRet;
}

// -----------------------------------------------------------------------

void JavaSalVirtualDevice::GetSize( long& rWidth, long& rHeight )
{
#ifdef USE_NATIVE_VIRTUAL_DEVICE
	rWidth = mnWidth;
	rHeight = mnHeight;
#else	// USE_NATIVE_VIRTUAL_DEVICE
	if ( mpVCLImage )
	{
		rWidth = mpVCLImage->getWidth();
		rHeight = mpVCLImage->getHeight();
	}
	else
	{
		rWidth = 0;
		rHeight = 0;
	}
#endif	// USE_NATIVE_VIRTUAL_DEVICE
}
