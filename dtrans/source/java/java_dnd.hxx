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

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCE_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGET_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#define JAVA_DRAGSOURCE_SERVICE_NAME "com.sun.star.datatransfer.dnd.JavaDragSource"
#define JAVA_DRAGSOURCE_IMPL_NAME "com.sun.star.datatransfer.dnd.JavaDragSource"
#define JAVA_DRAGSOURCE_REGKEY_NAME "/com.sun.star.datatransfer.dnd.JavaDragSource/UNO/SERVICES/com.sun.star.datatransfer.dnd.JavaDragSource"

#define JAVA_DROPTARGET_SERVICE_NAME "com.sun.star.datatransfer.dnd.JavaDropTarget"
#define JAVA_DROPTARGET_IMPL_NAME "com.sun.star.datatransfer.dnd.JavaDropTarget"
#define JAVA_DROPTARGET_REGKEY_NAME "/com.sun.star.datatransfer.dnd.JavaDropTarget/UNO/SERVICES/com.sun.star.datatransfer.dnd.JavaDropTarget"

using namespace ::com::sun::star::uno;

namespace java {

class JavaDragSource : public ::cppu::WeakComponentImplHelper3< ::com::sun::star::datatransfer::dnd::XDragSource, ::com::sun::star::lang::XInitialization, ::com::sun::star::lang::XServiceInfo >
{
public:
	sal_Int8				mnActions;
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	maContents;
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSourceListener >	maListener;
    ::osl::Mutex			maMutex;
	const SystemEnvData*	mpEnvData;
	Window*					mpWindow;

							DECL_STATIC_LINK( JavaDragSource, dragDropEnd, void* );

							JavaDragSource();
	virtual					~JavaDragSource();

	virtual ::rtl::OUString	SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
	virtual sal_Bool		SAL_CALL supportsService( const ::rtl::OUString& serviceName ) throw( com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >	SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL initialize( const Sequence< Any >& arguments ) throw( ::com::sun::star::uno::RuntimeException );
	virtual sal_Bool		SAL_CALL isDragImageSupported() throw( com::sun::star::uno::RuntimeException );
	virtual sal_Int32		SAL_CALL getDefaultCursor( sal_Int8 dragAction ) throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL startDrag( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& trigger, sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image, const Reference< ::com::sun::star::datatransfer::XTransferable >& transferable, const Reference< ::com::sun::star::datatransfer::dnd::XDragSourceListener >& listener ) throw( com::sun::star::uno::RuntimeException );

	WindowRef				getNativeWindow();
	void					handleActionChange();
	void					handleDrag( sal_Int32 nX, sal_Int32 nY );
};

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL JavaDragSource_getSupportedServiceNames();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL JavaDragSource_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xMultiServiceFactory );

class JavaDropTarget : public ::cppu::WeakComponentImplHelper3< ::com::sun::star::datatransfer::dnd::XDropTarget, ::com::sun::star::lang::XInitialization, ::com::sun::star::lang::XServiceInfo >
{
public:
    sal_Bool				mbActive;
	sal_Int8				mnDefaultActions;
	::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener > >	maListeners;
    ::osl::Mutex			maMutex; 
    bool					mbRejected;
	const SystemEnvData*	mpEnvData;
	Window*					mpWindow;

							JavaDropTarget();
	virtual					~JavaDropTarget();

	// Overrides WeakComponentImplHelper::disposing() which is called by
	// WeakComponentImplHelper::dispose()
    virtual void			SAL_CALL disposing();

	virtual void			SAL_CALL initialize( const Sequence< Any >& arguments ) throw( ::com::sun::star::uno::RuntimeException );

	virtual void			SAL_CALL addDropTargetListener( const Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& xListener ) throw( com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL removeDropTargetListener( const Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& xListener ) throw( com::sun::star::uno::RuntimeException );
	virtual sal_Bool		SAL_CALL isActive() throw( com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL setActive( sal_Bool active ) throw( com::sun::star::uno::RuntimeException );
	virtual sal_Int8		SAL_CALL getDefaultActions() throw( com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL setDefaultActions( sal_Int8 actions ) throw( com::sun::star::uno::RuntimeException );
	virtual ::rtl::OUString	SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
	virtual sal_Bool		SAL_CALL supportsService( const ::rtl::OUString& serviceName ) throw( com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >	SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException );

	WindowRef				getNativeWindow();
	void					handleDragEnter( sal_Int32 nX, sal_Int32 nY, DragRef aNativeTransferable );
	void					handleDragExit( sal_Int32 nX, sal_Int32 nY, DragRef aNativeTransferable );
	void					handleDragOver( sal_Int32 nX, sal_Int32 nY, DragRef aNativeTransferable );
	bool					handleDrop( sal_Int32 nX, sal_Int32 nY, DragRef aNativeTransferable );
	bool					isRejected() { return mbRejected; }
};

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL JavaDropTarget_getSupportedServiceNames();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL JavaDropTarget_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xMultiServiceFactory );
}

#endif
