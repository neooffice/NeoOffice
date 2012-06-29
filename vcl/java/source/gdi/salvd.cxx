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

using namespace vcl;

// =======================================================================

JavaSalVirtualDevice::JavaSalVirtualDevice() :
	mnWidth( 0 ),
	mnHeight( 0 ),
	mnBit( 0 ),
	maBitmapContext( NULL ),
	maBitmapLayer( NULL ),
	mnBitCount( 32 ),
	mpGraphics( new JavaSalGraphics() ),
	mbGraphics( FALSE )
{
	// By default no mirroring for VirtualDevices
	mpGraphics->SetLayout( 0 );
	mpGraphics->mpVirDev = this;
	mpGraphics->mnDPIX = MIN_SCREEN_RESOLUTION;
	mpGraphics->mnDPIY = MIN_SCREEN_RESOLUTION;
}

// -----------------------------------------------------------------------

JavaSalVirtualDevice::~JavaSalVirtualDevice()
{
	if ( maBitmapLayer )
		CGLayerRelease( maBitmapLayer );

	if ( maBitmapContext )
		CGContextRelease( maBitmapContext );

	// Delete graphics last as it may be needed by a JavaSalBitmap
	if ( mpGraphics )
		delete mpGraphics;
}

// -----------------------------------------------------------------------

SalGraphics* JavaSalVirtualDevice::GetGraphics()
{
	if ( mbGraphics )
		return NULL;

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

	mnWidth = 0;
	mnHeight = 0;

	mpGraphics->maNativeBounds = CGRectNull;
	mpGraphics->setLayer( NULL, 1.0f );

	if ( maBitmapLayer )
	{
		CGLayerRelease( maBitmapLayer );
		maBitmapLayer = NULL;
	}

	if ( nDX < 1 )
		nDX = 1;
	if ( nDY < 1 )
		nDY = 1;

	// Make a native layer backed by a 1 x 1 pixel native bitmap
	if ( !maBitmapContext )
	{
		CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
		if ( aColorSpace )
		{
			maBitmapContext = CGBitmapContextCreate( &mnBit, 1, 1, 8, sizeof( mnBit ), aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little );
			CGColorSpaceRelease( aColorSpace );
		}
	}

	if ( maBitmapContext )
	{
		float fBackingScaleFactor = JavaSalFrame::GetBackingScaleFactor();
		float fDX = (float)nDX * fBackingScaleFactor;
		float fDY = (float)nDY * fBackingScaleFactor;
		maBitmapLayer = CGLayerCreateWithContext( maBitmapContext, CGSizeMake( fDX, fDY ), NULL );
		if ( maBitmapLayer )
		{
			mpGraphics->maNativeBounds = CGRectMake( 0, 0, fDX, fDY );
			mpGraphics->setLayer( maBitmapLayer, fBackingScaleFactor );
			mnWidth = nDX;
			mnHeight = nDY;
			bRet = TRUE;
		}
	}

	return bRet;
}

// -----------------------------------------------------------------------

void JavaSalVirtualDevice::GetSize( long& rWidth, long& rHeight )
{
	rWidth = mnWidth;
	rHeight = mnHeight;
}
