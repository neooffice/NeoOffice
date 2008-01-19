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

#import <premac.h>
#import <QTKit/QTKit.h>
#import <postmac.h>
#import "quicktimeplayerwindow.hxx"

// Redefine Cocoa YES and NO defines types for convenience
#ifdef YES
#undef YES
#define YES (MacOSBOOL)1
#endif
#ifdef NO
#undef NO
#define NO (MacOSBOOL)0
#endif

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::media;
using namespace ::com::sun::star::uno;

namespace avmedia
{
namespace quicktime
{

// ============================================================================

Window::Window( const Reference< XMultiServiceFactory >& rxMgr, Player& rPlayer ) :
	mxMgr( rxMgr ),
	mpMoviePlayerView( NULL ),
	maPlayer( rPlayer )
{
}

// ----------------------------------------------------------------------------

Window::~Window()
{
}

// ----------------------------------------------------------------------------

void Window::update() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::update not implemented\n" );
#endif
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

void Window::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::setPosSize not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

Rectangle Window::getPosSize() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::getPosSize not implemented\n" );
#endif
	return Rectangle();
}

// ----------------------------------------------------------------------------

void Window::setVisible( sal_Bool Visible ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::setVisible not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void Window::setEnable( sal_Bool Enable ) throw( RuntimeException )
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
#ifdef DEBUG
	fprintf( stderr, "Window::dispose not implemented\n" );
#endif
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

::rtl::OUString Window::getImplementationName() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::getImplementationName not implemented\n" );
#endif
	return ::rtl::OUString();
}

// ----------------------------------------------------------------------------

sal_Bool Window::supportsService( const ::rtl::OUString& ServiceName ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::supportsService not implemented\n" );
#endif
	return false;
}

// ----------------------------------------------------------------------------

Sequence< ::rtl::OUString > Window::getSupportedServiceNames() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Window::getSupportedServiceNames not implemented\n" );
#endif
	return Sequence< ::rtl::OUString >();
}

} // namespace quicktime
} // namespace avmedia
