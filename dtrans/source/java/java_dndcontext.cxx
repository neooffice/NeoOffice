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
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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

#include <java_dndcontext.hxx>

using namespace com::sun::star::datatransfer::dnd;
using namespace cppu;
using namespace java;

// ========================================================================

DragSourceContext::DragSourceContext() :
	WeakImplHelper1< XDragSourceContext >()
{
#ifdef DEBUG
	fprintf( stderr, "DragSourceContext::DragSourceContext not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

DragSourceContext::~DragSourceContext()
{
#ifdef DEBUG
	fprintf( stderr, "DragSourceContext::~DragSourceContext not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

sal_Int32 SAL_CALL DragSourceContext::getCurrentCursor() throw()
{
#ifdef DEBUG
	fprintf( stderr, "DragSourceContext::getCurrentCursor not implemented\n" );
#endif
	return 0;
}

// ------------------------------------------------------------------------

void SAL_CALL DragSourceContext::setCursor( sal_Int32 cursorId ) throw()
{
#ifdef DEBUG
	fprintf( stderr, "DragSourceContext::setCursor not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL DragSourceContext::setImage( sal_Int32 imageId ) throw()
{
#ifdef DEBUG
	fprintf( stderr, "DragSourceContext::setImage not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL DragSourceContext::transferablesFlavorsChanged() throw()
{
#ifdef DEBUG
	fprintf( stderr, "DragSourceContext::transferablesFlavorsChanged not implemented\n" );
#endif
}

// ========================================================================

DropTargetDropContext::DropTargetDropContext() :
	WeakImplHelper1< XDropTargetDropContext >(),
	mbSuccess( false )
{
#ifdef DEBUG
	fprintf( stderr, "DropTargetDropContext::DropTargetDropContext not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

DropTargetDropContext::~DropTargetDropContext()
{
#ifdef DEBUG
	fprintf( stderr, "DropTargetDropContext::~DropTargetDropContext not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL DropTargetDropContext::acceptDrop( sal_Int8 dragOperation ) throw()
{
#ifdef DEBUG
	fprintf( stderr, "DropTargetDropContext::acceptDrop not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL DropTargetDropContext::rejectDrop() throw()
{
#ifdef DEBUG
	fprintf( stderr, "DropTargetDropContext::rejectDrop not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL DropTargetDropContext::dropComplete( sal_Bool success ) throw()
{
#ifdef DEBUG
	fprintf( stderr, "DropTargetDropContext::dropComplete not implemented\n" );
#endif

	// Multiple listeners may call this method so don't reset when false
	if ( success )
		mbSuccess = success;
}

// ------------------------------------------------------------------------

bool DropTargetDropContext::getDropComplete()
{
	return mbSuccess;
}

// ========================================================================

DropTargetDragContext::DropTargetDragContext() :
	WeakImplHelper1< XDropTargetDragContext >()
{
#ifdef DEBUG
	fprintf( stderr, "DropTargetDragContext::DropTargetDragContext not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

DropTargetDragContext::~DropTargetDragContext()
{
}

// ------------------------------------------------------------------------

void SAL_CALL DropTargetDragContext::acceptDrag( sal_Int8 dragOperation ) throw()
{
#ifdef DEBUG
	fprintf( stderr, "DropTargetDragContext::acceptDrag not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL DropTargetDragContext::rejectDrag() throw()
{
#ifdef DEBUG
	fprintf( stderr, "DropTargetDragContext::rejectDrag not implemented\n" );
#endif
}
