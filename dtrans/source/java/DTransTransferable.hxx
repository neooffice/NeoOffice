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

#ifndef _DTRANSTRANSFERABLE_HXX
#define _DTRANSTRANSFERABLE_HXX

#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DATAFLAVOR_HPP_
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#define TRANSFERABLE_TYPE_CLIPBOARD		0x0
#define TRANSFERABLE_TYPE_DRAG			0x1

namespace java {

class DTransTransferable : public ::cppu::WeakImplHelper1 < ::com::sun::star::datatransfer::XTransferable >
{
private:
	void*				mpNativeTransferable;
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	mxTransferable;
	int					mnTransferableType;
	sal_uInt16			mnItem;

public:
						DTransTransferable( void *pNativeTransferable, int nTransferableType, sal_uInt16 nItem = 0 );
	virtual				~DTransTransferable();

	void				flush();
	void*				getNativeTransferable() { return mpNativeTransferable; }
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	getTransferable() { return mxTransferable; }
	virtual ::com::sun::star::uno::Any getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw ( ::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > getTransferDataFlavors() throw ( ::com::sun::star::uno::RuntimeException );
	int					getType() { return mnTransferableType; }
	sal_Bool			hasOwnership();
	virtual sal_Bool	isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw ( ::com::sun::star::uno::RuntimeException );
	sal_Bool			setContents( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > &xTransferable );
};

} // namespace java

#endif // _DTRANSTRANSFERABLE_HXX
