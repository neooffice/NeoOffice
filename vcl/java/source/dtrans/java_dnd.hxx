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

#ifndef _DTRANS_JAVA_DND_HXX_
#define _DTRANS_JAVA_DND_HXX_

#include <list>

#include <cppuhelper/compbase3.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <osl/thread.h>
#include <vcl/window.hxx>

#ifdef __OBJC__
@class NSView;
#else
typedef void* id;
struct NSView;
#endif

class JavaDragSource : public ::cppu::WeakComponentImplHelper3< ::com::sun::star::datatransfer::dnd::XDragSource, ::com::sun::star::lang::XInitialization, ::com::sun::star::lang::XServiceInfo >
{
public:
	sal_Int8				mnActions;
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	maContents;
	id						mpDraggingSource;
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSourceListener >	maListener;
    ::osl::Mutex			maMutex;
	id						mpPasteboardHelper;
	Window*					mpWindow;

							DECL_STATIC_LINK( JavaDragSource, dragDropEnd, void* );

							JavaDragSource();
	virtual					~JavaDragSource();

	virtual ::rtl::OUString	SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
	virtual sal_Bool		SAL_CALL supportsService( const ::rtl::OUString& serviceName ) throw( com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >	SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& arguments ) throw( ::com::sun::star::uno::RuntimeException );
	virtual sal_Bool		SAL_CALL isDragImageSupported() throw( com::sun::star::uno::RuntimeException );
	virtual sal_Int32		SAL_CALL getDefaultCursor( sal_Int8 dragAction ) throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL startDrag( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& trigger, sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& transferable, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSourceListener >& listener ) throw( com::sun::star::uno::RuntimeException );

	NSView*					getNSView();
	void					handleDrag( sal_Int32 nX, sal_Int32 nY );
};

class JavaDropTarget : public ::cppu::WeakComponentImplHelper3< ::com::sun::star::datatransfer::dnd::XDropTarget, ::com::sun::star::lang::XInitialization, ::com::sun::star::lang::XServiceInfo >
{
public:
    sal_Bool				mbActive;
	sal_Int8				mnDefaultActions;
	::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener > >	maListeners;
    ::osl::Mutex			maMutex; 
	id						mpPasteboardHelper;
    bool					mbRejected;
	Window*					mpWindow;

							JavaDropTarget();
	virtual					~JavaDropTarget();

	// Overrides WeakComponentImplHelper::disposing() which is called by
	// WeakComponentImplHelper::dispose()
    virtual void			SAL_CALL disposing();

	virtual void			SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& arguments ) throw( ::com::sun::star::uno::RuntimeException );

	virtual void			SAL_CALL addDropTargetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& xListener ) throw( com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL removeDropTargetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& xListener ) throw( com::sun::star::uno::RuntimeException );
	virtual sal_Bool		SAL_CALL isActive() throw( com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL setActive( sal_Bool active ) throw( com::sun::star::uno::RuntimeException );
	virtual sal_Int8		SAL_CALL getDefaultActions() throw( com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL setDefaultActions( sal_Int8 actions ) throw( com::sun::star::uno::RuntimeException );
	virtual ::rtl::OUString	SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
	virtual sal_Bool		SAL_CALL supportsService( const ::rtl::OUString& serviceName ) throw( com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >	SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException );

	NSView*					getNSView();
	sal_Int8				handleDragEnter( sal_Int32 nX, sal_Int32 nY, id aInfo );
	void					handleDragExit( sal_Int32 nX, sal_Int32 nY, id aInfo );
	sal_Int8				handleDragOver( sal_Int32 nX, sal_Int32 nY, id aInfo );
	bool					handleDrop( sal_Int32 nX, sal_Int32 nY, id aInfo );
	bool					isRejected() { return mbRejected; }
};

#endif
