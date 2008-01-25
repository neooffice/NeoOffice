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

#import "quicktimecommon.h"
#import "quicktimeframegrabber.hxx"

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

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	if ( mpMoviePlayer )
	{
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(release:) withObject:(id)mpMoviePlayer waitUntilDone:YES modes:pModes];
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
			NSSize aSize = [pImageRep size];
			unsigned char *pBits = [pImageRep bitmapData];
			USHORT nBitCount = (USHORT)[pImageRep bitsPerPixel];
			long nHeight = (long)aSize.height;
			ULONG nScanlineSize = (ULONG)[pImageRep bytesPerRow];
			Size aBitmapSize( (long)aSize.width, nHeight );
			if ( pBits && ( nBitCount == 24 || nBitCount == 32 ) && nScanlineSize && aBitmapSize.Width() > 0 && aBitmapSize.Height() > 0 )
			{
				Bitmap aBitmap( aBitmapSize, nBitCount );
				if ( !aBitmap.IsEmpty() )
				{
					BitmapWriteAccess *pAccess = aBitmap.AcquireWriteAccess();
					if ( pAccess )
					{
						Scanline aScanline = (Scanline)pBits;
						ULONG nFormat = BMP_FORMAT_TOP_DOWN;
						if ( nBitCount == 24 )
							nFormat |= BMP_FORMAT_24BIT_TC_RGB;
						else
							nFormat |= BMP_FORMAT_32BIT_TC_ARGB;

						for ( long i = 0; i < nHeight; i++ )
						{
							pAccess->CopyScanline( i, aScanline, nFormat, nScanlineSize );
							aScanline += nScanlineSize;
						}

						aBitmap.ReleaseAccess( pAccess );

						Graphic aGraphic( aBitmap );
						xRet = aGraphic.GetXGraphic();
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

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	if ( mpMoviePlayer )
	{
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(release:) withObject:(id)mpMoviePlayer waitUntilDone:YES modes:pModes];
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
