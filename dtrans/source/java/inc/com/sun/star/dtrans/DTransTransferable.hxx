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

#ifndef _JAVA_DTRANS_COM_SUN_STAR_DTRANS_DTRANSTRANSFERABLE_HXX
#define	_JAVA_DTRANS_COM_SUN_STAR_DTRANS_DTRANSTRANSFERABLE_HXX

#ifndef _JAVA_DTRANS_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DATAFLAVOR_HPP_
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#define JAVA_DTRANS_TRANSFERABLE_TYPE_CLIPBOARD		0x0
#define JAVA_DTRANS_TRANSFERABLE_TYPE_DRAG			0x1

namespace java {

namespace dtrans {

class com_sun_star_dtrans_DTransTransferable;

class com_sun_star_dtrans_DTransTransferable : public java_lang_Object, public ::cppu::WeakImplHelper1 < ::com::sun::star::datatransfer::XTransferable >
{
protected:
	static jclass		theClass;

private:
	void*				mpNativeTransferable;
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	mxTransferable;
	int					mnTransferableType;

public:
	static jclass		getMyClass();

#ifdef MACOSX
						com_sun_star_dtrans_DTransTransferable( void *myNativeTransferable, int nTransferableType ) : java_lang_Object( NULL ), mpNativeTransferable( myNativeTransferable ), mnTransferableType( nTransferableType ) {}
#else	// MACOSX
						com_sun_star_dtrans_DTransTransferable( jobject myObj, int nTransferableType ) : java_lang_Object( myObj ), mpNativeTransferable( NULL ), mnTransferableType( nTransferableType ) {}
#endif	// MACOSX
	virtual				~com_sun_star_dtrans_DTransTransferable();

	void*				getNativeTransferable() { return mpNativeTransferable; }
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	getTransferable() { return mxTransferable; }
	virtual ::com::sun::star::uno::Any SAL_CALL	getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw ( ::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL	getTransferDataFlavors() throw ( ::com::sun::star::uno::RuntimeException );
	int					getType() { return mnTransferableType; }
	sal_Bool			hasOwnership();
	virtual sal_Bool SAL_CALL	isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw ( ::com::sun::star::uno::RuntimeException );
	sal_Bool			setContents( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > &xTransferable );
};

} // namespace dtrans

} // namespace java

#endif // _JAVA_DTRANS_COM_SUN_STAR_DTRANS_DTRANSTRANSFERABLE_HXX
