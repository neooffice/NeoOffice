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

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#define AVMEDIA_QUICKTIME_WINDOW_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Window_QuickTime"
#define AVMEDIA_QUICKTIME_WINDOW_SERVICENAME "com.sun.star.media.Window_QuickTime"

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::media;
using namespace ::com::sun::star::uno;
using namespace ::cppu;
using namespace ::vos;

namespace avmedia
{
namespace quicktime
{

// ============================================================================

::std::list< Window *> Window::maWindows;

// ----------------------------------------------------------------------------

Window* Window::findWindowAndLockSolarMutex( void* pMoviePlayer )
{
	Window *pRet = NULL;

	if ( !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			for ( ::std::list< Window* >::const_iterator it = Window::maWindows.begin(); it != Window::maWindows.end(); ++it )
			{
				if ( (*it)->mpMoviePlayer == pMoviePlayer )
				{
					pRet = *it;
					break;
				}
			}

			if ( !pRet )
				rSolarMutex.release();
		}
	}

	return pRet;
}

// ----------------------------------------------------------------------------

void Window::releaseSolarMutex()
{
	IMutex &rSolarMutex = Application::GetSolarMutex();
	rSolarMutex.release();
}

// ----------------------------------------------------------------------------

void Window::fireFocusGainedEvent( void* pMoviePlayer, const ::com::sun::star::awt::FocusEvent& rEvt )
{
	Window *pWindow = Window::findWindowAndLockSolarMutex( pMoviePlayer );
	if ( pWindow )
	{
		OInterfaceContainerHelper* pContainer = pWindow->maListeners.getContainer( getCppuType( (Reference< XFocusListener >*)0 ) );
		if ( pContainer )
		{
			OInterfaceIteratorHelper aIterator( *pContainer );
			while ( aIterator.hasMoreElements() )
				Reference< XFocusListener >( aIterator.next(), UNO_QUERY )->focusGained( rEvt );
		}

		Window::releaseSolarMutex();
	}
}

// ----------------------------------------------------------------------------

void Window::fireMouseMovedEvent( void *pMoviePlayer, const ::com::sun::star::awt::MouseEvent& rEvt )
{
	Window *pWindow = Window::findWindowAndLockSolarMutex( pMoviePlayer );
	if ( pWindow )
	{
		OInterfaceContainerHelper* pContainer = pWindow->maListeners.getContainer( getCppuType( (Reference< XMouseMotionListener >*)0 ) );
		if ( pContainer )
		{
			OInterfaceIteratorHelper aIterator( *pContainer );
			while ( aIterator.hasMoreElements() )
				Reference< XMouseMotionListener >( aIterator.next(), UNO_QUERY )->mouseMoved( rEvt );
		}

		Window::releaseSolarMutex();
	}
}

// ----------------------------------------------------------------------------

void Window::fireMousePressedEvent( void *pMoviePlayer, const ::com::sun::star::awt::MouseEvent& rEvt )
{
	Window *pWindow = Window::findWindowAndLockSolarMutex( pMoviePlayer );
	if ( pWindow )
	{
		OInterfaceContainerHelper* pContainer = pWindow->maListeners.getContainer( getCppuType( (Reference< XMouseListener >*)0 ) );
		if ( pContainer )
		{
			OInterfaceIteratorHelper aIterator( *pContainer );
			while ( aIterator.hasMoreElements() )
				Reference< XMouseListener >( aIterator.next(), UNO_QUERY )->mousePressed( rEvt );
		}

		Window::releaseSolarMutex();
	}
}

// ----------------------------------------------------------------------------

void Window::fireMouseReleasedEvent( void *pMoviePlayer, const ::com::sun::star::awt::MouseEvent& rEvt )
{
	Window *pWindow = Window::findWindowAndLockSolarMutex( pMoviePlayer );
	if ( pWindow )
	{
		OInterfaceContainerHelper* pContainer = pWindow->maListeners.getContainer( getCppuType( (Reference< XMouseListener >*)0 ) );
		if ( pContainer )
		{
			OInterfaceIteratorHelper aIterator( *pContainer );
			while ( aIterator.hasMoreElements() )
				Reference< XMouseListener >( aIterator.next(), UNO_QUERY )->mouseReleased( rEvt );
		}

		Window::releaseSolarMutex();
	}
}

// ----------------------------------------------------------------------------

Window::Window( const Reference< XMultiServiceFactory >& rxMgr ) :
	maListeners( maMutex ),
	mxMgr( rxMgr ),
	mpMoviePlayer( NULL ),
	mpParentView( NULL ),
	mbVisible( sal_False )
{
	Window::maWindows.push_back( this );
}

// ----------------------------------------------------------------------------

Window::~Window()
{
	dispose();

	Window::maWindows.remove( this );
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
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithInt:nPointerType]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setPointer:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

void Window::setPosSize( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nFlags ) throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	maRect = ::com::sun::star::awt::Rectangle( nX, nY, nWidth, nHeight );

	if ( mpMoviePlayer && mpParentView )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObjects:(NSView *)mpParentView, [NSValue valueWithRect:NSMakeRect( maRect.X, maRect.Y, maRect.Width, maRect.Height )], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setBounds:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

::com::sun::star::awt::Rectangle Window::getPosSize() throw( RuntimeException )
{
	::com::sun::star::awt::Rectangle aRet( 0, 0, 0, 0 );

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
				aRet = ::com::sun::star::awt::Rectangle( (long)aRect.origin.x, (long)aRect.origin.y, (long)aRect.size.width, (long)aRect.size.height );
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
	if ( mpMoviePlayer )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setFocus:) withObject:(id)mpMoviePlayer waitUntilDone:YES modes:pModes];
		mpMoviePlayer = NULL;
	}
}

// ----------------------------------------------------------------------------

void Window::addWindowListener( const Reference< XWindowListener >& xListener ) throw( RuntimeException )
{
	maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ----------------------------------------------------------------------------

void Window::removeWindowListener( const Reference< XWindowListener >& xListener ) throw( RuntimeException )
{
	maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ----------------------------------------------------------------------------

void Window::addFocusListener( const Reference< XFocusListener >& xListener ) throw( RuntimeException )
{
	maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ----------------------------------------------------------------------------

void Window::removeFocusListener( const Reference< XFocusListener >& xListener ) throw( RuntimeException )
{
	maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ----------------------------------------------------------------------------

void Window::addKeyListener( const Reference< XKeyListener >& xListener ) throw( RuntimeException )
{
	maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ----------------------------------------------------------------------------

void Window::removeKeyListener( const Reference< XKeyListener >& xListener ) throw( RuntimeException )
{
	maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ----------------------------------------------------------------------------

void Window::addMouseListener( const Reference< XMouseListener >& xListener ) throw( RuntimeException )
{
	maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ----------------------------------------------------------------------------

void Window::removeMouseListener( const Reference< XMouseListener >& xListener ) throw( RuntimeException )
{
	maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ----------------------------------------------------------------------------

void Window::addMouseMotionListener( const Reference< XMouseMotionListener >& xListener ) throw( RuntimeException )
{
	maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ----------------------------------------------------------------------------

void Window::removeMouseMotionListener( const Reference< XMouseMotionListener >& xListener ) throw( RuntimeException )
{
	maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ----------------------------------------------------------------------------

void Window::addPaintListener( const Reference< XPaintListener >& xListener ) throw( RuntimeException )
{
	maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ----------------------------------------------------------------------------

void Window::removePaintListener( const Reference< XPaintListener >& xListener ) throw( RuntimeException )
{
	maListeners.removeInterface( getCppuType( &xListener ), xListener );
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
	maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ----------------------------------------------------------------------------

void Window::removeEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
	maListeners.removeInterface( getCppuType( &xListener ), xListener );
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

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

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

			::com::sun::star::awt::Rectangle aRect;
			rArguments.getConstArray()[1] >>= maRect;

			setVisible( sal_True );
			bRet = true;
		}
	}

	[pPool release];

	return bRet;
}

} // namespace quicktime
} // namespace avmedia
