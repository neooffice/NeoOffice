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
 *		 - GNU General Public License Version 2.1
 *
 *  Patrick Luby, January 2008
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2008 Planamesa Inc.
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

#include "quicktimecommon.h"
#include "quicktimeframegrabber.hxx"

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif

#define AVMEDIA_QUICKTIME_FRAMEGRABBER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.FrameGrabber_QuickTime"
#define AVMEDIA_QUICKTIME_FRAMEGRABBER_SERVICENAME "com.sun.star.media.FrameGrabber_QuickTime"

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
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(frameImageAtTime:) withObject:pArgs waitUntilDone:YES modes:pModes];
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

::rtl::OUString SAL_CALL FrameGrabber::getImplementationName() throw( RuntimeException )
{
	return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_QUICKTIME_FRAMEGRABBER_IMPLEMENTATIONNAME ) );
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL FrameGrabber::supportsService( const ::rtl::OUString& ServiceName ) throw( RuntimeException )
{
	return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( AVMEDIA_QUICKTIME_FRAMEGRABBER_SERVICENAME ) );
}

// ----------------------------------------------------------------------------

Sequence< ::rtl::OUString > SAL_CALL FrameGrabber::getSupportedServiceNames() throw( RuntimeException )
{
	Sequence< ::rtl::OUString > aRet(1);
	aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( AVMEDIA_QUICKTIME_FRAMEGRABBER_SERVICENAME ) );

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
