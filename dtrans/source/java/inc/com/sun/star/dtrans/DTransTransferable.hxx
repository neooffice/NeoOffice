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

namespace java {

namespace dtrans {

class com_sun_star_dtrans_DTransTransferable : public java_lang_Object, public ::cppu::WeakImplHelper1 < ::com::sun::star::datatransfer::XTransferable >
{
protected:
	static jclass		theClass;

private:
#ifdef MACOSX
	void*				mpNativeObj;
#endif	// MACOSX

						com_sun_star_dtrans_DTransTransferable() : java_lang_Object( NULL ) {};

public:
	static jclass		getMyClass();

#ifdef MACOSX
						com_sun_star_dtrans_DTransTransferable( void *myNativeObj ) : java_lang_Object( NULL ) { mpNativeObj = myNativeObj; }
#else	// MACOSX
						com_sun_star_dtrans_DTransTransferable( jobject myObj ) : java_lang_Object( myObj ) {}
#endif	// MACOSX

	virtual ::com::sun::star::uno::Any SAL_CALL	getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw ( ::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL	getTransferDataFlavors() throw ( ::com::sun::star::uno::RuntimeException );
	virtual sal_Bool SAL_CALL	isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw ( ::com::sun::star::uno::RuntimeException );
};

} // namespace dtrans

} // namespace java

#endif // _JAVA_DTRANS_COM_SUN_STAR_DTRANS_DTRANSTRANSFERABLE_HXX
