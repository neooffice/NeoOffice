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

#include "quicktimecommon.h"
#include "quicktimecommon.hxx"
#include "quicktimeframegrabber.hxx"

#include <osl/objcutils.h>
#include <tools/gen.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>
#include <vcl/salbtype.hxx>

using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::media;
using namespace ::com::sun::star::uno;

namespace avmedia
{
namespace quicktime
{

// ============================================================================

FrameGrabber::FrameGrabber( const Reference< XMultiServiceFactory >& rxMgr ) :
	mxMgr( rxMgr ),
	mpMoviePlayer( NULL )
{
}

// ----------------------------------------------------------------------------

FrameGrabber::~FrameGrabber()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		[(AvmediaMoviePlayer *)mpMoviePlayer release];
		mpMoviePlayer = NULL;
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

Reference< XGraphic > SAL_CALL FrameGrabber::grabFrame( double fMediaTime ) throw( RuntimeException )
{
	Reference< XGraphic > xRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithDouble:fMediaTime]]];
		osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(frameImageAtTime:), pArgs, YES );
		NSBitmapImageRep *pImageRep = (NSBitmapImageRep *)[pArgs result];
		if ( pImageRep )
		{
			// Fix the color shifting reported in the following NeoOffice forum
			// post by copying the image to the 32 bit format expected by the
			// code in vcl/java/source/gdi/salbmp.cxx:
			// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=63924#63924
			CGImageRef aImage = [pImageRep CGImage];
			if ( aImage )
			{
				Size aBitmapSize( CGImageGetWidth( aImage ), CGImageGetHeight( aImage ) );
				
				if ( aBitmapSize.Width() > 0 && aBitmapSize.Height() > 0 )
				{
					Bitmap aBitmap( aBitmapSize, 32 );
					if ( !aBitmap.IsEmpty() )
					{
						BitmapWriteAccess *pAccess = aBitmap.AcquireWriteAccess();
						if ( pAccess )
						{
							BOOL bDrawn = NO;
							long nWidth = pAccess->Width();
							long nHeight = pAccess->Height();
							sal_uLong nScanlineSize = pAccess->GetScanlineSize();
							Scanline pBits = pAccess->GetBuffer();
							if ( nWidth > 0 && nHeight > 0 && nScanlineSize > 0 && pBits )
							{
								CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
								if ( aColorSpace )
								{
									CGContextRef aContext = CGBitmapContextCreate( pBits, nWidth, nHeight, 8, nScanlineSize, aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little );
									if ( aContext )
									{
										bDrawn = YES;
									    CGContextDrawImage( aContext, CGRectMake( 0, 0, nWidth, nHeight ), aImage );
									    CGContextRelease( aContext );
									}

									CGColorSpaceRelease( aColorSpace );
								}
							}

							aBitmap.ReleaseAccess( pAccess );

							if ( bDrawn )
							{
								Graphic aGraphic( aBitmap );
								xRet = aGraphic.GetXGraphic();
							}
						}
					}
				}
			}
		}
	}

	[pPool release];

	return xRet;
}

// ----------------------------------------------------------------------------

OUString SAL_CALL FrameGrabber::getImplementationName() throw( RuntimeException )
{
	return OUString( AVMEDIA_QUICKTIME_FRAMEGRABBER_IMPLEMENTATIONNAME );
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL FrameGrabber::supportsService( const OUString& ServiceName ) throw( RuntimeException )
{
	return ServiceName == AVMEDIA_QUICKTIME_FRAMEGRABBER_SERVICENAME;
}

// ----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL FrameGrabber::getSupportedServiceNames() throw( RuntimeException )
{
	Sequence< OUString > aRet(1);
	aRet[0] = OUString( AVMEDIA_QUICKTIME_FRAMEGRABBER_SERVICENAME );

	return aRet;
}

// ----------------------------------------------------------------------------

bool FrameGrabber::create( void *pMoviePlayer )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		[(AvmediaMoviePlayer *)mpMoviePlayer release];
		mpMoviePlayer = NULL;
	}

	if ( pMoviePlayer )
	{
		mpMoviePlayer = pMoviePlayer;
		[(AvmediaMoviePlayer *)mpMoviePlayer retain];

		bRet = true;
	}

	[pPool release];

	return bRet;
}

} // namespace quicktime
} // namespace avmedia
