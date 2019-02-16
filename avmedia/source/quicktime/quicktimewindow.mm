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
#include "quicktimecommon.hxx"
#include "quicktimewindow.hxx"

#include <vcl/svapp.hxx>

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::media;
using namespace ::com::sun::star::uno;
using namespace ::cppu;

namespace avmedia
{
namespace quicktime
{

// ============================================================================

::std::list< Window *> Window::maWindows;

// ----------------------------------------------------------------------------

Window* Window::findWindow( const void* pMoviePlayer )
{
	Window *pRet = nullptr;

	for ( ::std::list< Window* >::const_iterator it = Window::maWindows.begin(); it != Window::maWindows.end(); ++it )
	{
		if ( (*it)->mpMoviePlayer == pMoviePlayer )
		{
			pRet = *it;
			break;
		}
	}

	return pRet;
}

// ----------------------------------------------------------------------------

IMPL_STATIC_LINK( Window, fireFocusGainedEvent, void*, pEvtData, void )
{
	FocusEventData *pFocusEvtData = static_cast< FocusEventData* >( pEvtData );
	Window *pWindow = Window::findWindow( pFocusEvtData->mpMoviePlayer );
	if ( pWindow )
	{
		OInterfaceContainerHelper* pContainer = pWindow->maListeners.getContainer( cppu::UnoType< Reference< XFocusListener > >::get() );
		if ( pContainer )
		{
			OInterfaceIteratorHelper aIterator( *pContainer );
			while ( aIterator.hasMoreElements() )
				Reference< XFocusListener >( aIterator.next(), UNO_QUERY )->focusGained( pFocusEvtData->maFocusEvent );
		}
	}

	delete pFocusEvtData;
}

// ----------------------------------------------------------------------------

IMPL_STATIC_LINK( Window, fireMouseMovedEvent, void*, pEvtData, void )
{
	MouseEventData *pMouseEvtData = static_cast< MouseEventData* >( pEvtData );
	Window *pWindow = Window::findWindow( pMouseEvtData->mpMoviePlayer );
	if ( pWindow )
	{
		OInterfaceContainerHelper* pContainer = pWindow->maListeners.getContainer( cppu::UnoType< Reference< XMouseMotionListener > >::get() );
		if ( pContainer )
		{
			OInterfaceIteratorHelper aIterator( *pContainer );
			while ( aIterator.hasMoreElements() )
				Reference< XMouseMotionListener >( aIterator.next(), UNO_QUERY )->mouseMoved( pMouseEvtData->maMouseEvent );
		}
	}

	delete pMouseEvtData;
}

// ----------------------------------------------------------------------------

IMPL_STATIC_LINK( Window, fireMousePressedEvent, void*, pEvtData, void )
{
	MouseEventData *pMouseEvtData = static_cast< MouseEventData* >( pEvtData );
	Window *pWindow = Window::findWindow( pMouseEvtData->mpMoviePlayer );
	if ( pWindow )
	{
		OInterfaceContainerHelper* pContainer = pWindow->maListeners.getContainer( cppu::UnoType< Reference< XMouseListener > >::get() );
		if ( pContainer )
		{
			OInterfaceIteratorHelper aIterator( *pContainer );
			while ( aIterator.hasMoreElements() )
				Reference< XMouseListener >( aIterator.next(), UNO_QUERY )->mousePressed( pMouseEvtData->maMouseEvent );
		}
	}

	delete pMouseEvtData;
}

// ----------------------------------------------------------------------------

IMPL_STATIC_LINK( Window, fireMouseReleasedEvent, void*, pEvtData, void )
{
	MouseEventData *pMouseEvtData = static_cast< MouseEventData* >( pEvtData );
	Window *pWindow = Window::findWindow( pMouseEvtData->mpMoviePlayer );
	if ( pWindow )
	{
		OInterfaceContainerHelper* pContainer = pWindow->maListeners.getContainer( cppu::UnoType< Reference< XMouseListener > >::get() );
		if ( pContainer )
		{
			OInterfaceIteratorHelper aIterator( *pContainer );
			while ( aIterator.hasMoreElements() )
				Reference< XMouseListener >( aIterator.next(), UNO_QUERY )->mouseReleased( pMouseEvtData->maMouseEvent );
		}
	}

	delete pMouseEvtData;
}

// ----------------------------------------------------------------------------

Window::Window( const Reference< XMultiServiceFactory >& rxMgr ) :
	maListeners( maMutex ),
	mxMgr( rxMgr ),
	mpMoviePlayer( nullptr ),
	mpParentView( nullptr ),
	mbVisible( sal_False ),
	mnZoomLevel( ZoomLevel_NOT_AVAILABLE )
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

sal_Bool Window::setZoomLevel( ZoomLevel nZoomLevel ) throw( RuntimeException )
{
	sal_Bool bRet = sal_False;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithInt:static_cast< int >( nZoomLevel )]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(setZoomLevel:) withObject:pArgs waitUntilDone:YES modes:pModes];
		bRet = sal_True;
		if ( bRet )
			mnZoomLevel = nZoomLevel;
	}

	[pPool release];

	return bRet;
}

// ----------------------------------------------------------------------------

ZoomLevel Window::getZoomLevel() throw( RuntimeException )
{
	return mnZoomLevel;
}

// ----------------------------------------------------------------------------

void Window::setPointerType( sal_Int32 nPointerType ) throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithInt:nPointerType]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(setPointer:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

void Window::setPosSize( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 /* nFlags*/ ) throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	maRect = ::com::sun::star::awt::Rectangle( nX, nY, nWidth, nHeight );

	if ( mpMoviePlayer && mpParentView )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObjects:static_cast< NSView* >( mpParentView ), [NSValue valueWithRect:NSMakeRect( maRect.X, maRect.Y, maRect.Width, maRect.Height )], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(setBounds:) withObject:pArgs waitUntilDone:YES modes:pModes];
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
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(bounds:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSValue *pRet = static_cast< NSValue* >( [pArgs result] );
		if ( pRet )
		{
			NSRect aRect = [pRet rectValue];
			if ( aRect.size.width > 0 && aRect.size.height > 0 )
				aRet = ::com::sun::star::awt::Rectangle( static_cast< long >( aRect.origin.x ), static_cast< long >( aRect.origin.y ), static_cast< long >( aRect.size.width ), static_cast< long >( aRect.size.height ) );
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
			pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObjects:static_cast< AvmediaMoviePlayer* >( mpParentView ), [NSValue valueWithRect:NSMakeRect( maRect.X, maRect.Y, maRect.Width, maRect.Height )], nil]];
		else
			pArgs = [AvmediaArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(setSuperview:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

void Window::setEnable( sal_Bool /* bEnable */ ) throw( RuntimeException )
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
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(setFocus:) withObject:static_cast< id >( mpMoviePlayer ) waitUntilDone:YES modes:pModes];
	}
}

// ----------------------------------------------------------------------------

void Window::addWindowListener( const Reference< XWindowListener >& xListener ) throw( RuntimeException )
{
	maListeners.addInterface( cppu::UnoType< decltype( xListener ) >::get(), xListener );
}

// ----------------------------------------------------------------------------

void Window::removeWindowListener( const Reference< XWindowListener >& xListener ) throw( RuntimeException )
{
	maListeners.removeInterface( cppu::UnoType< decltype( xListener ) >::get(), xListener );
}

// ----------------------------------------------------------------------------

void Window::addFocusListener( const Reference< XFocusListener >& xListener ) throw( RuntimeException )
{
	maListeners.addInterface( cppu::UnoType< decltype( xListener ) >::get(), xListener );
}

// ----------------------------------------------------------------------------

void Window::removeFocusListener( const Reference< XFocusListener >& xListener ) throw( RuntimeException )
{
	maListeners.removeInterface( cppu::UnoType< decltype( xListener ) >::get(), xListener );
}

// ----------------------------------------------------------------------------

void Window::addKeyListener( const Reference< XKeyListener >& xListener ) throw( RuntimeException )
{
	maListeners.addInterface( cppu::UnoType< decltype( xListener ) >::get(), xListener );
}

// ----------------------------------------------------------------------------

void Window::removeKeyListener( const Reference< XKeyListener >& xListener ) throw( RuntimeException )
{
	maListeners.removeInterface( cppu::UnoType< decltype( xListener ) >::get(), xListener );
}

// ----------------------------------------------------------------------------

void Window::addMouseListener( const Reference< XMouseListener >& xListener ) throw( RuntimeException )
{
	maListeners.addInterface( cppu::UnoType< decltype( xListener ) >::get(), xListener );
}

// ----------------------------------------------------------------------------

void Window::removeMouseListener( const Reference< XMouseListener >& xListener ) throw( RuntimeException )
{
	maListeners.removeInterface( cppu::UnoType< decltype( xListener ) >::get(), xListener );
}

// ----------------------------------------------------------------------------

void Window::addMouseMotionListener( const Reference< XMouseMotionListener >& xListener ) throw( RuntimeException )
{
	maListeners.addInterface( cppu::UnoType< decltype( xListener ) >::get(), xListener );
}

// ----------------------------------------------------------------------------

void Window::removeMouseMotionListener( const Reference< XMouseMotionListener >& xListener ) throw( RuntimeException )
{
	maListeners.removeInterface( cppu::UnoType< decltype( xListener ) >::get(), xListener );
}

// ----------------------------------------------------------------------------

void Window::addPaintListener( const Reference< XPaintListener >& xListener ) throw( RuntimeException )
{
	maListeners.addInterface( cppu::UnoType< decltype( xListener ) >::get(), xListener );
}

// ----------------------------------------------------------------------------

void Window::removePaintListener( const Reference< XPaintListener >& xListener ) throw( RuntimeException )
{
	maListeners.removeInterface( cppu::UnoType< decltype( xListener ) >::get(), xListener );
}

// ----------------------------------------------------------------------------

void Window::dispose() throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) release];
		mpMoviePlayer = nullptr;
	}

	if ( mpParentView )
	{
		[static_cast< NSView* >( mpParentView ) release];
		mpParentView = nullptr;
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

void Window::addEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
	maListeners.addInterface( cppu::UnoType< decltype( xListener ) >::get(), xListener );
}

// ----------------------------------------------------------------------------

void Window::removeEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
	maListeners.removeInterface( cppu::UnoType< decltype( xListener ) >::get(), xListener );
}

// ----------------------------------------------------------------------------

OUString SAL_CALL Window::getImplementationName() throw( RuntimeException )
{
	return OUString( AVMEDIA_QUICKTIME_WINDOW_IMPLEMENTATIONNAME );
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Window::supportsService( const OUString& ServiceName ) throw( RuntimeException )
{
	return ServiceName == AVMEDIA_QUICKTIME_WINDOW_SERVICENAME;
}

// ----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL Window::getSupportedServiceNames() throw( RuntimeException )
{
	Sequence< OUString > aRet(1);
	aRet[0] = OUString( AVMEDIA_QUICKTIME_WINDOW_SERVICENAME );

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
		sal_IntPtr nPtr = 0;
		rArguments.getConstArray()[0] >>= nPtr;
		if ( nPtr )
		{
			mpMoviePlayer = pMoviePlayer;
			[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) retain];

			mpParentView = reinterpret_cast< void* >( nPtr );
			[static_cast< NSView* >( mpParentView ) retain];

			rArguments.getConstArray()[1] >>= maRect;

			setZoomLevel( ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT );
			setVisible( sal_True );
			bRet = true;
		}
	}

	[pPool release];

	return bRet;
}

} // namespace quicktime
} // namespace avmedia
