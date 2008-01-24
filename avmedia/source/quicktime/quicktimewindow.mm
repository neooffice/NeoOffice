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
#import "quicktimewindow.hxx"

#define AVMEDIA_QUICKTIME_WINDOW_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Window_QuickTime"
#define AVMEDIA_QUICKTIME_WINDOW_SERVICENAME "com.sun.star.media.Window_QuickTime"

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::media;
using namespace ::com::sun::star::uno;

namespace avmedia
{
namespace quicktime
{

// ============================================================================

Window::Window( const Reference< XMultiServiceFactory >& rxMgr ) :
	mxMgr( rxMgr ),
	mpMoviePlayer( NULL ),
	mpParentView( NULL ),
	mbVisible( sal_False )
{
}

// ----------------------------------------------------------------------------

Window::~Window()
{
	dispose();
}

// ----------------------------------------------------------------------------

void Window::update() throw( RuntimeException )
{
	setVisible( mbVisible );
}

// ----------------------------------------------------------------------------

sal_Bool Window::setZoomLevel( ZoomLevel ZoomLevel ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::setZoomLevel not implemented\n" );
#endif
	return sal_False;
}

// ----------------------------------------------------------------------------

ZoomLevel Window::getZoomLevel() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::getZoomLevel not implemented\n" );
#endif
	return ZoomLevel_NOT_AVAILABLE;
}

// ----------------------------------------------------------------------------

void Window::setPointerType( sal_Int32 nPointerType ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::setPointerType not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::setPosSize( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nFlags ) throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	maRect = Rectangle( nX, nY, nWidth, nHeight );

	if ( mpMoviePlayer && mpParentView )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObjects:(AvmediaMoviePlayer *)mpParentView, [NSValue valueWithRect:NSMakeRect( maRect.X, maRect.Y, maRect.Width, maRect.Height )], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setBounds:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

Rectangle Window::getPosSize() throw( RuntimeException )
{
	Rectangle aRet( 0, 0, 0, 0 );

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(bounds:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSValue *pRet = (NSValue *)[pArgs result];
		if ( pRet )
		{
			NSRect aRect = [pRet rectValue];
			if ( aRect.size.width > 0 && aRect.size.height > 0 )
				aRet = Rectangle( (long)aRect.origin.x, (long)aRect.origin.y, (long)aRect.size.width, (long)aRect.size.height );
		}
	}

	[pPool release];

	return aRet;
}

// ----------------------------------------------------------------------------

void Window::setVisible( sal_Bool bVisible ) throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	mbVisible = bVisible;

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs;
		if ( bVisible && mpParentView )
			pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObjects:(AvmediaMoviePlayer *)mpParentView, [NSValue valueWithRect:NSMakeRect( maRect.X, maRect.Y, maRect.Width, maRect.Height )], nil]];
		else
			pArgs = [AvmediaArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setSuperview:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

void Window::setEnable( sal_Bool bEnable ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::setEnable not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::setFocus() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::setFocus not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::addWindowListener( const Reference< XWindowListener >& xListener ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::addWindowListener not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::removeWindowListener( const Reference< XWindowListener >& xListener ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::removeWindowListener not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::addFocusListener( const Reference< XFocusListener >& xListener ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::addFocusListener not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::removeFocusListener( const Reference< XFocusListener >& xListener ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::removeFocusListener not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::addKeyListener( const Reference< XKeyListener >& xListener ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::addKeyListener not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::removeKeyListener( const Reference< XKeyListener >& xListener ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::removeKeyListener not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::addMouseListener( const Reference< XMouseListener >& xListener ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::addMouseListener not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::removeMouseListener( const Reference< XMouseListener >& xListener ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::removeMouseListener not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::addMouseMotionListener( const Reference< XMouseMotionListener >& xListener ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::addMouseMotionListener not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::removeMouseMotionListener( const Reference< XMouseMotionListener >& xListener ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::removeMouseMotionListener not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::addPaintListener( const Reference< XPaintListener >& xListener ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::addPaintListener not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::removePaintListener( const Reference< XPaintListener >& xListener ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::removePaintListener not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::dispose() throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	if ( mpMoviePlayer )
	{
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(release:) withObject:(id)mpMoviePlayer waitUntilDone:YES modes:pModes];
		mpMoviePlayer = NULL;
	}

	if ( mpParentView )
	{
		[(NSView *)mpParentView performSelectorOnMainThread:@selector(release) withObject:nil waitUntilDone:YES modes:pModes];
		mpParentView = NULL;
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

void Window::addEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::addEventListener not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::removeEventListener( const Reference< XEventListener >& aListener ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::removeEventListener not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

::rtl::OUString SAL_CALL Window::getImplementationName() throw( RuntimeException )
{
	return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_QUICKTIME_WINDOW_IMPLEMENTATIONNAME ) );
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Window::supportsService( const ::rtl::OUString& ServiceName ) throw( RuntimeException )
{
	return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( AVMEDIA_QUICKTIME_WINDOW_SERVICENAME ) );
}

// ----------------------------------------------------------------------------

Sequence< ::rtl::OUString > SAL_CALL Window::getSupportedServiceNames() throw( RuntimeException )
{
	Sequence< ::rtl::OUString > aRet(1);
	aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( AVMEDIA_QUICKTIME_WINDOW_SERVICENAME ) );

	return aRet;
}

// ----------------------------------------------------------------------------

bool Window::create( void *pMoviePlayer, const Sequence< Any >& rArguments )
{
	bool bRet = false;

	dispose();

	if ( pMoviePlayer && rArguments.getLength() > 1 )
	{
		sal_Int32 nPtr;
		rArguments.getConstArray()[0] >>= nPtr;
		if ( nPtr )
		{
			mpMoviePlayer = pMoviePlayer;
			[(AvmediaMoviePlayer *)mpMoviePlayer retain];

			mpParentView = (void *)nPtr;
			[(NSView *)mpParentView retain];

			Rectangle aRect;
			rArguments.getConstArray()[1] >>= maRect;

			setVisible( sal_True );
			bRet = true;
		}
	}

	return bRet;
}

} // namespace quicktime
} // namespace avmedia
