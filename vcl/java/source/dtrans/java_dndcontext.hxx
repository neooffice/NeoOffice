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

#ifndef _DTRANS_JAVA_DNDCONTEXT_HXX
#define _DTRANS_JAVA_DNDCONTEXT_HXX

#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>
#include <cppuhelper/implbase1.hxx>

using namespace com::sun::star;

class JavaDragSourceContext : public ::cppu::WeakImplHelper1< ::com::sun::star::datatransfer::dnd::XDragSourceContext >
{
public:
							JavaDragSourceContext();
	virtual					~JavaDragSourceContext();

	// XDragSourceContext
	virtual sal_Int32		SAL_CALL getCurrentCursor() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual void			SAL_CALL setCursor( sal_Int32 cursorId ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual void			SAL_CALL setImage( sal_Int32 imageId ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual void			SAL_CALL transferablesFlavorsChanged() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
};

class JavaDropTargetDropContext : public ::cppu::WeakImplHelper1< ::com::sun::star::datatransfer::dnd::XDropTargetDropContext >
{
	sal_Int8				mnAction;
	bool					mbRejected;
	bool					mbSuccess;

public:
							JavaDropTargetDropContext( sal_Int8 nAction );
	virtual					~JavaDropTargetDropContext();

	// XDropTargetDropContext
	virtual void			SAL_CALL acceptDrop( sal_Int8 dragOperation ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual void			SAL_CALL rejectDrop() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual void			SAL_CALL dropComplete( sal_Bool success ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

	bool					getDropComplete();
	bool					isRejected();
};

class JavaDropTargetDragContext : public ::cppu::WeakImplHelper1< ::com::sun::star::datatransfer::dnd::XDropTargetDragContext >
{
	sal_Int8				mnAction;
	bool					mbRejected;

public:
							JavaDropTargetDragContext( sal_Int8 nAction );
	virtual					~JavaDropTargetDragContext();

	// XDropTargetDragContext
	virtual void			SAL_CALL acceptDrag( sal_Int8 dragOperation ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual void			SAL_CALL rejectDrag() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

	bool					isRejected();
};

#endif
