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

#include <stdio.h>
#include <unistd.h>

#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include "java_dndcontext.hxx"

using namespace com::sun::star;
using namespace cppu;

// ========================================================================

JavaDragSourceContext::JavaDragSourceContext() :
	WeakImplHelper1< XDragSourceContext >()
{
}

// ------------------------------------------------------------------------

JavaDragSourceContext::~JavaDragSourceContext()
{
}

// ------------------------------------------------------------------------

sal_Int32 SAL_CALL JavaDragSourceContext::getCurrentCursor() throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
#ifdef DEBUG
	fprintf( stderr, "JavaDragSourceContext::getCurrentCursor not implemented\n" );
#endif
	return 0;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDragSourceContext::setCursor( sal_Int32 /* cursorId */ ) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
#ifdef DEBUG
	fprintf( stderr, "JavaDragSourceContext::setCursor not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDragSourceContext::setImage( sal_Int32 /* imageId */ ) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
#ifdef DEBUG
	fprintf( stderr, "JavaDragSourceContext::setImage not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDragSourceContext::transferablesFlavorsChanged() throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
#ifdef DEBUG
	fprintf( stderr, "JavaDragSourceContext::transferablesFlavorsChanged not implemented\n" );
#endif
}

// ========================================================================

JavaDropTargetDropContext::JavaDropTargetDropContext( sal_Int8 nAction ) :
	WeakImplHelper1< XDropTargetDropContext >(),
	mnAction( nAction ),
	mbRejected( false ),
	mbSuccess( false )
{
}

// ------------------------------------------------------------------------

JavaDropTargetDropContext::~JavaDropTargetDropContext()
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDropTargetDropContext::acceptDrop( sal_Int8 dragOperation ) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
	mnAction &= datatransfer::dnd::DNDConstants::ACTION_DEFAULT;

	if ( dragOperation & datatransfer::dnd::DNDConstants::ACTION_MOVE )
		mnAction |= datatransfer::dnd::DNDConstants::ACTION_MOVE;
	else if ( dragOperation & datatransfer::dnd::DNDConstants::ACTION_COPY )
		mnAction |= datatransfer::dnd::DNDConstants::ACTION_COPY;
	else if ( dragOperation & datatransfer::dnd::DNDConstants::ACTION_LINK )
		mnAction |= datatransfer::dnd::DNDConstants::ACTION_LINK;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDropTargetDropContext::rejectDrop() throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
	mbRejected = true;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDropTargetDropContext::dropComplete( sal_Bool success ) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
	// Multiple listeners may call this method so don't reset when false
	if ( !mbRejected && success )
		mbSuccess = success;
}

// ------------------------------------------------------------------------

bool JavaDropTargetDropContext::getDropComplete()
{
	return mbSuccess;
}

// ------------------------------------------------------------------------

bool JavaDropTargetDropContext::isRejected()
{
	return mbRejected;
}

// ========================================================================

JavaDropTargetDragContext::JavaDropTargetDragContext( sal_Int8 nAction ) :
	WeakImplHelper1< XDropTargetDragContext >(),
	mnAction( nAction ),
	mbRejected( false )
{
}

// ------------------------------------------------------------------------

JavaDropTargetDragContext::~JavaDropTargetDragContext()
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDropTargetDragContext::acceptDrag( sal_Int8 dragOperation ) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
	mnAction &= datatransfer::dnd::DNDConstants::ACTION_DEFAULT;

	if ( dragOperation & datatransfer::dnd::DNDConstants::ACTION_MOVE )
		mnAction |= datatransfer::dnd::DNDConstants::ACTION_MOVE;
	else if ( dragOperation & datatransfer::dnd::DNDConstants::ACTION_COPY )
		mnAction |= datatransfer::dnd::DNDConstants::ACTION_COPY;
	else if ( dragOperation & datatransfer::dnd::DNDConstants::ACTION_LINK )
		mnAction |= datatransfer::dnd::DNDConstants::ACTION_LINK;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDropTargetDragContext::rejectDrag() throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
	mbRejected = true;
}

// ------------------------------------------------------------------------

bool JavaDropTargetDragContext::isRejected()
{
	return mbRejected;
}
