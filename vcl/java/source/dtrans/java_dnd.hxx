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
	vcl::Window*			mpWindow;

							DECL_STATIC_LINK( JavaDragSource, dragDropEnd, void* );

							JavaDragSource();
	virtual					~JavaDragSource();

	// XDragSource
	virtual sal_Bool		SAL_CALL isDragImageSupported() throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual sal_Int32		SAL_CALL getDefaultCursor( sal_Int8 dragAction ) throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual void			SAL_CALL startDrag( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& trigger, sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& transferable, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSourceListener >& listener ) throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

	// XInitialization
	virtual void			SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& arguments ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

	// XServiceInfo
	virtual OUString		SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual sal_Bool		SAL_CALL supportsService( const OUString& serviceName ) throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual ::com::sun::star::uno::Sequence< OUString >	SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

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
	vcl::Window*			mpWindow;

							JavaDropTarget();
	virtual					~JavaDropTarget();

	// Overrides WeakComponentImplHelper::disposing() which is called by
	// WeakComponentImplHelper::dispose()
    virtual void			SAL_CALL disposing() SAL_OVERRIDE;

	virtual void			SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& arguments ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

	virtual void			SAL_CALL addDropTargetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& xListener ) throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual void			SAL_CALL removeDropTargetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& xListener ) throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual sal_Bool		SAL_CALL isActive() throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual void			SAL_CALL setActive( sal_Bool active ) throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual sal_Int8		SAL_CALL getDefaultActions() throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual void			SAL_CALL setDefaultActions( sal_Int8 actions ) throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual OUString	SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual sal_Bool		SAL_CALL supportsService( const OUString& serviceName ) throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual ::com::sun::star::uno::Sequence< OUString >	SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

	NSView*					getNSView();
	sal_Int8				handleDragEnter( sal_Int32 nX, sal_Int32 nY, id aInfo );
	void					handleDragExit( sal_Int32 nX, sal_Int32 nY, id aInfo );
	sal_Int8				handleDragOver( sal_Int32 nX, sal_Int32 nY, id aInfo );
	bool					handleDrop( sal_Int32 nX, sal_Int32 nY, id aInfo );
	bool					isRejected() { return mbRejected; }
};

#endif
