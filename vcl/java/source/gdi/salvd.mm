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

#include <osl/objcutils.h>

#include "java/saldata.hxx"
#include "java/salgdi.h"
#include "java/salvd.h"

#include "../java/VCLEventQueue_cocoa.h"

using namespace vcl;

@interface VCLVirtualDeviceGetGraphicsLayer : NSObject
{
	CGLayerRef				maLayer;
	long					mnDX;
	long					mnDY;
}
+ (id)createWithWidth:(long)nDX height:(long)nDY;
- (id)initWithWidth:(long)nDX height:(long)nDY;
- (void)dealloc;
- (void)getGraphicsLayer:(id)pObject;
- (CGLayerRef)layer;
@end

@implementation VCLVirtualDeviceGetGraphicsLayer

+ (id)createWithWidth:(long)nDX height:(long)nDY
{
	VCLVirtualDeviceGetGraphicsLayer *pRet = [[VCLVirtualDeviceGetGraphicsLayer alloc] initWithWidth:nDX height:nDY];
	[pRet autorelease];
	return pRet;
}

- (id)initWithWidth:(long)nDX height:(long)nDY
{
	[super init];

	maLayer = NULL;
	mnDX = nDX;
	if ( mnDX < 1 )
		mnDX = 1;
	mnDY = nDY;
	if ( mnDY < 1 )
		mnDY = 1;

	return self;
}

- (void)dealloc
{
	if ( maLayer )
		CGLayerRelease( maLayer );

	[super dealloc];
}

- (void)getGraphicsLayer:(id)pObject
{
	(void)pObject;

	if ( maLayer )
	{
		CGLayerRelease( maLayer );
		maLayer = NULL;
	}

	CGContextRef aContext = NSWindow_cachedCGContext();
	if ( aContext )
		maLayer = CGLayerCreateWithContext( aContext, CGSizeMake( mnDX, mnDY ), NULL );
}

- (CGLayerRef)layer
{
	return maLayer;
}

@end

// =======================================================================

JavaSalVirtualDevice::JavaSalVirtualDevice() :
	mnWidth( 0 ),
	mnHeight( 0 ),
	maVirDevLayer( NULL ),
	mpGraphics( new JavaSalGraphics() ),
	mbGraphics( sal_False )
{
	// By default no mirroring for VirtualDevices
	mpGraphics->SetLayout( 0 );
	mpGraphics->mpVirDev = this;
	mpGraphics->mnDPIX = MIN_SCREEN_RESOLUTION;
	mpGraphics->mnDPIY = MIN_SCREEN_RESOLUTION;

	// Insert this virtual device into the virtual device list
	GetSalData()->maVirDevList.push_front( this );
}

// -----------------------------------------------------------------------

JavaSalVirtualDevice::~JavaSalVirtualDevice()
{
	// Remove this virtual device from the virtual device list
	GetSalData()->maVirDevList.remove( this );

	if ( maVirDevLayer )
		CGLayerRelease( maVirDevLayer );

	// Delete graphics last as it may be needed by a JavaSalBitmap
	if ( mpGraphics )
		delete mpGraphics;
}

// -----------------------------------------------------------------------

bool JavaSalVirtualDevice::ScreenParamsChanged()
{
	bool bRet = false;

	if ( maVirDevLayer && NSWindow_cachedCGContextScaleFactorHasChanged( maVirDevLayer ) )
	{
		bRet = true;

		CGLayerRelease( maVirDevLayer );
		maVirDevLayer = NULL;

		SetSize( mnWidth, mnHeight );
	}

	return bRet;
}

// -----------------------------------------------------------------------

SalGraphics* JavaSalVirtualDevice::AcquireGraphics()
{
	if ( mbGraphics )
		return NULL;

	mbGraphics = sal_True;

	return mpGraphics;
}

// -----------------------------------------------------------------------

void JavaSalVirtualDevice::ReleaseGraphics( SalGraphics* pGraphics )
{
	if ( pGraphics != mpGraphics )
		return;

	mbGraphics = sal_False;
}

// -----------------------------------------------------------------------

bool JavaSalVirtualDevice::SetSize( long nDX, long nDY )
{
	if ( nDX < 1 )
		nDX = 1;
	if ( nDY < 1 )
		nDY = 1;

	bool bRet = false;

	mnWidth = 0;
	mnHeight = 0;

	if ( maVirDevLayer )
	{
		CGLayerRelease( maVirDevLayer );
		maVirDevLayer = NULL;
	}

	mpGraphics->maNativeBounds = CGRectNull;
	mpGraphics->setLayer( NULL );

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	VCLVirtualDeviceGetGraphicsLayer *pVCLVirtualDeviceGetGraphicsLayer = [VCLVirtualDeviceGetGraphicsLayer createWithWidth:nDX height:nDY];
	osl_performSelectorOnMainThread( pVCLVirtualDeviceGetGraphicsLayer, @selector(getGraphicsLayer:), pVCLVirtualDeviceGetGraphicsLayer, YES );
	maVirDevLayer = [pVCLVirtualDeviceGetGraphicsLayer layer];
	if ( maVirDevLayer )
	{
		CGLayerRetain( maVirDevLayer );
	}
	else
	{
		// Make a native layer backed by a 1 x 1 pixel native bitmap
		CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
		if ( aColorSpace )
		{
			sal_uInt32 nBit = 0;
			CGContextRef aBitmapContext = CGBitmapContextCreate( &nBit, 1, 1, 8, sizeof( nBit ), aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little );
			if ( aBitmapContext )
			{
				maVirDevLayer = CGLayerCreateWithContext( aBitmapContext, CGSizeMake( nDX, nDY ), NULL );
				CGContextRelease( aBitmapContext );
			}

			CGColorSpaceRelease( aColorSpace );
		}
	}

	if ( maVirDevLayer )
	{
		CGSize aLayerSize = CGLayerGetSize( maVirDevLayer );
		mpGraphics->maNativeBounds = CGRectMake( 0, 0, aLayerSize.width, aLayerSize.height );
		mpGraphics->setLayer( maVirDevLayer );
		mnWidth = nDX;
		mnHeight = nDY;
		bRet = true;
	}

	[pPool release];

	return bRet;
}

// -----------------------------------------------------------------------

long JavaSalVirtualDevice::GetWidth() const
{
	return mnWidth;
}

// -----------------------------------------------------------------------

long JavaSalVirtualDevice::GetHeight() const
{
	return mnHeight;
}
