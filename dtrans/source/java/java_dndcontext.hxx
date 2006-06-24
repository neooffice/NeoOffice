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

#ifndef _DTRANS_JAVA_DNDCONTEXT_HXX
#define _DTRANS_JAVA_DNDCONTEXT_HXX

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCECONTEXT_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETDROPCONTEXT_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETDRAGCONTEXT_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

using namespace com::sun::star::uno;

namespace java {

class DragSourceContext : public ::cppu::WeakImplHelper1< ::com::sun::star::datatransfer::dnd::XDragSourceContext >
{
public:
							DragSourceContext();
	virtual					~DragSourceContext();

	virtual sal_Int32		SAL_CALL getCurrentCursor() throw( ::com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL setCursor( sal_Int32 cursorId ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL setImage( sal_Int32 imageId ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL transferablesFlavorsChanged() throw( ::com::sun::star::uno::RuntimeException );
};

class DropTargetDropContext : public ::cppu::WeakImplHelper1< ::com::sun::star::datatransfer::dnd::XDropTargetDropContext >
{
	sal_Int8				mnAction;
	bool					mbRejected;
	bool					mbSuccess;

public:
							DropTargetDropContext( sal_Int8 nAction );
	virtual					~DropTargetDropContext();

	virtual void			SAL_CALL acceptDrop( sal_Int8 dragOperation ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL rejectDrop() throw( ::com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL dropComplete( sal_Bool success ) throw( ::com::sun::star::uno::RuntimeException );

	bool					getDropComplete();
	bool					isRejected();
};

class DropTargetDragContext : public ::cppu::WeakImplHelper1< ::com::sun::star::datatransfer::dnd::XDropTargetDragContext >
{
	sal_Int8				mnAction;
	bool					mbRejected;

public:
							DropTargetDragContext( sal_Int8 nAction );
	virtual					~DropTargetDragContext();

	virtual void			SAL_CALL acceptDrag( sal_Int8 dragOperation ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL rejectDrag() throw( ::com::sun::star::uno::RuntimeException );

	bool					isRejected();
};

}

#endif
