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

#ifndef _WINDOW_HXX
#define _WINDOW_HXX

#include <list>

#include "quicktimeplayer.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/media/XPlayerWindow.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <tools/link.hxx>

namespace avmedia
{
namespace quicktime
{

struct FocusEventData
{
	const void*			mpMoviePlayer;
	const ::com::sun::star::awt::FocusEvent	maFocusEvent;

						FocusEventData( const void* pMoviePlayer, const ::com::sun::star::awt::FocusEvent &rFocusEvent ) : mpMoviePlayer( pMoviePlayer ), maFocusEvent( rFocusEvent ) {}
};

struct MouseEventData
{
	const void*			mpMoviePlayer;
	const ::com::sun::star::awt::MouseEvent	maMouseEvent;

						MouseEventData( const void* pMoviePlayer, const ::com::sun::star::awt::MouseEvent &rMouseEvent ) : mpMoviePlayer( pMoviePlayer ), maMouseEvent( rMouseEvent ) {}
};

// ----------
// - Window -
// ----------

class Window : public ::cppu::WeakImplHelper2 < ::com::sun::star::media::XPlayerWindow, ::com::sun::star::lang::XServiceInfo >
{
	static ::std::list< Window* >	maWindows;

	::cppu::OMultiTypeInterfaceContainerHelper	maListeners;
	::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >	mxMgr;
	void*				mpMoviePlayer;
	::osl::Mutex		maMutex;
	void*				mpParentView;
	::com::sun::star::awt::Rectangle	maRect;
	::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener > >	maWindowListeners;
	sal_Bool			mbVisible;
	::com::sun::star::media::ZoomLevel	mnZoomLevel;

	static Window*		findWindow( const void* pMoviePlayer );

public:
						Window( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMgr );
						~Window();

	DECL_STATIC_LINK( Window, fireFocusGainedEvent, void *pEvtData );
	DECL_STATIC_LINK( Window, fireMouseMovedEvent, void *pEvtData );
	DECL_STATIC_LINK( Window, fireMousePressedEvent, void *pEvtData );
	DECL_STATIC_LINK( Window, fireMouseReleasedEvent, void *pEvtData );

    // XPlayerWindow
    virtual void SAL_CALL update(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL setZoomLevel( ::com::sun::star::media::ZoomLevel ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual ::com::sun::star::media::ZoomLevel SAL_CALL getZoomLevel(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL setPointerType( sal_Int32 nPointerType ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    // XWindow
    virtual void SAL_CALL setPosSize( sal_Int32 , sal_Int32 , sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getPosSize(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL setVisible( sal_Bool ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL setEnable( sal_Bool ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL setFocus(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

	bool					create( void *pMoviePlayerView, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rArguments );
};

} // namespace quicktime
} // namespace avmedia

#endif // _WINDOW_HXX
