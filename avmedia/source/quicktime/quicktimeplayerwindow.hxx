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

#ifndef _WINDOW_HXX
#define _WINDOW_HXX

#include "quicktimeplayer.hxx"

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_MEDIA_XPLAYER_HPP_
#include <com/sun/star/media/XPlayer.hpp>
#endif

namespace avmedia
{
namespace quicktime
{

// ----------
// - Window -
// ----------

class Window : public ::cppu::WeakImplHelper2 < ::com::sun::star::media::XPlayerWindow, ::com::sun::star::lang::XServiceInfo >
{
	::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >	mxMgr;
	void*				mpMoviePlayerView;
	Player				maPlayer;

public:
						Window( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMgr, Player& rPlayer );
						~Window();

	// XPlayerWindow
	virtual void SAL_CALL	update() throw( ::com::sun::star::uno::RuntimeException );
	virtual sal_Bool SAL_CALL	setZoomLevel( ::com::sun::star::media::ZoomLevel ZoomLevel ) throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::media::ZoomLevel SAL_CALL getZoomLevel() throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	setPointerType( sal_Int32 nPointerType ) throw( ::com::sun::star::uno::RuntimeException );

	// XWindow
	virtual void SAL_CALL	setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::awt::Rectangle SAL_CALL getPosSize() throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	setVisible( sal_Bool Visible ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	setEnable( sal_Bool Enable ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	setFocus() throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );

	// XComponent
	virtual void SAL_CALL	dispose() throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw( ::com::sun::star::uno::RuntimeException );

	// XServiceInfo
	virtual ::rtl::OUString SAL_CALL	getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
	virtual sal_Bool SAL_CALL	supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL	getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );
};

} // namespace quicktime
} // namespace avmedia

#endif // _WINDOW_HXX
