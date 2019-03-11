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

sal_Int32 SAL_CALL JavaDragSourceContext::getCurrentCursor() throw()
{
#ifdef DEBUG
	fprintf( stderr, "JavaDragSourceContext::getCurrentCursor not implemented\n" );
#endif
	return 0;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDragSourceContext::setCursor( sal_Int32 /* cursorId */ ) throw()
{
#ifdef DEBUG
	fprintf( stderr, "JavaDragSourceContext::setCursor not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDragSourceContext::setImage( sal_Int32 /* imageId */ ) throw()
{
#ifdef DEBUG
	fprintf( stderr, "JavaDragSourceContext::setImage not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDragSourceContext::transferablesFlavorsChanged() throw()
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

void SAL_CALL JavaDropTargetDragContext::acceptDrag( sal_Int8 dragOperation ) throw()
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

void SAL_CALL JavaDropTargetDragContext::rejectDrag() throw()
{
	mbRejected = true;
}

// ------------------------------------------------------------------------

bool JavaDropTargetDragContext::isRejected()
{
	return mbRejected;
}
