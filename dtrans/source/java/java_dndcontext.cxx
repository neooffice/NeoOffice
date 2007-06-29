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

#ifndef _DTRANS_JAVA_DNDCONTEXT_HXX
#include "java_dndcontext.hxx"
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_DNDCONSTANTS_HPP_
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#endif

using namespace com::sun::star::datatransfer::dnd;
using namespace cppu;
using namespace java;

// ========================================================================

DragSourceContext::DragSourceContext() :
	WeakImplHelper1< XDragSourceContext >()
{
}

// ------------------------------------------------------------------------

DragSourceContext::~DragSourceContext()
{
}

// ------------------------------------------------------------------------

sal_Int32 SAL_CALL DragSourceContext::getCurrentCursor() throw( ::com::sun::star::uno::RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "DragSourceContext::getCurrentCursor not implemented\n" );
#endif
	return 0;
}

// ------------------------------------------------------------------------

void SAL_CALL DragSourceContext::setCursor( sal_Int32 cursorId ) throw( ::com::sun::star::uno::RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "DragSourceContext::setCursor not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL DragSourceContext::setImage( sal_Int32 imageId ) throw( ::com::sun::star::uno::RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "DragSourceContext::setImage not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL DragSourceContext::transferablesFlavorsChanged() throw( ::com::sun::star::uno::RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "DragSourceContext::transferablesFlavorsChanged not implemented\n" );
#endif
}

// ========================================================================

DropTargetDropContext::DropTargetDropContext( sal_Int8 nAction ) :
	WeakImplHelper1< XDropTargetDropContext >(),
	mnAction( nAction ),
	mbRejected( false ),
	mbSuccess( false )
{
}

// ------------------------------------------------------------------------

DropTargetDropContext::~DropTargetDropContext()
{
}

// ------------------------------------------------------------------------

void SAL_CALL DropTargetDropContext::acceptDrop( sal_Int8 dragOperation ) throw( ::com::sun::star::uno::RuntimeException )
{
	mnAction &= DNDConstants::ACTION_DEFAULT;

	if ( dragOperation & DNDConstants::ACTION_MOVE )
		mnAction |= DNDConstants::ACTION_MOVE;
	else if ( dragOperation & DNDConstants::ACTION_COPY )
		mnAction |= DNDConstants::ACTION_COPY;
	else if ( dragOperation & DNDConstants::ACTION_LINK )
		mnAction |= DNDConstants::ACTION_LINK;
}

// ------------------------------------------------------------------------

void SAL_CALL DropTargetDropContext::rejectDrop() throw( ::com::sun::star::uno::RuntimeException )
{
	mbRejected = true;
}

// ------------------------------------------------------------------------

void SAL_CALL DropTargetDropContext::dropComplete( sal_Bool success ) throw( ::com::sun::star::uno::RuntimeException )
{
	// Multiple listeners may call this method so don't reset when false
	if ( !mbRejected && success )
		mbSuccess = success;
}

// ------------------------------------------------------------------------

bool DropTargetDropContext::getDropComplete()
{
	return mbSuccess;
}

// ------------------------------------------------------------------------

bool DropTargetDropContext::isRejected()
{
	return mbRejected;
}

// ========================================================================

DropTargetDragContext::DropTargetDragContext( sal_Int8 nAction ) :
	WeakImplHelper1< XDropTargetDragContext >(),
	mnAction( nAction ),
	mbRejected( false )
{
}

// ------------------------------------------------------------------------

DropTargetDragContext::~DropTargetDragContext()
{
}

// ------------------------------------------------------------------------

void SAL_CALL DropTargetDragContext::acceptDrag( sal_Int8 dragOperation ) throw( ::com::sun::star::uno::RuntimeException )
{
	mnAction &= DNDConstants::ACTION_DEFAULT;

	if ( dragOperation & DNDConstants::ACTION_MOVE )
		mnAction |= DNDConstants::ACTION_MOVE;
	else if ( dragOperation & DNDConstants::ACTION_COPY )
		mnAction |= DNDConstants::ACTION_COPY;
	else if ( dragOperation & DNDConstants::ACTION_LINK )
		mnAction |= DNDConstants::ACTION_LINK;
}

// ------------------------------------------------------------------------

void SAL_CALL DropTargetDragContext::rejectDrag() throw( ::com::sun::star::uno::RuntimeException )
{
	mbRejected = true;
}

// ------------------------------------------------------------------------

bool DropTargetDragContext::isRejected()
{
	return mbRejected;
}
