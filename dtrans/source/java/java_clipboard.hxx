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

#ifndef _JAVA_CLIPBOARD_HXX_
#define _JAVA_CLIPBOARD_HXX_

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _JAVA_DTRANS_COM_SUN_STAR_DTRANS_DTRANSCLIPBOARD_HXX
#include <com/sun/star/dtrans/DTransClipboard.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDEX_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDOWNER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDLISTENER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDNOTIFIER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef __SGI_STL_HASHMAP
#include <hash_map>
#endif
#ifndef __SGI_STL_LIST
#include <list>
#endif

#define JAVA_CLIPBOARD_SERVICE_NAME "com.sun.star.datatransfer.clipboard.SystemClipboard"
#define JAVA_CLIPBOARD_IMPL_NAME "com.sun.star.datatransfer.clipboard.JavaClipboard"
#define JAVA_CLIPBOARD_REGKEY_NAME "/com.sun.star.datatransfer.clipboard.JavaClipboard/UNO/SERVICES/com.sun.star.datatransfer.clipboard.SystemClipboard"

namespace java {

class JavaClipboard : public ::cppu::WeakComponentImplHelper4< ::com::sun::star::datatransfer::clipboard::XClipboardEx, ::com::sun::star::datatransfer::clipboard::XClipboardNotifier, ::com::sun::star::lang::XServiceInfo, ::com::sun::star::lang::XInitialization >
{
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	maContents;
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >	maOwner;
	::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener > >	maListeners;
	::osl::Mutex			maMutex;
	::java::dtrans::com_sun_star_dtrans_DTransClipboard *mpSystemClipboard;

public:
							JavaClipboard( BOOL bSystemClipboard );
	virtual					~JavaClipboard();
	
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	SAL_CALL getContents() throw( ::com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL setContents( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTransferable, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner ) throw( ::com::sun::star::uno::RuntimeException );
	virtual ::rtl::OUString	SAL_CALL getName() throw( ::com::sun::star::uno::RuntimeException );
	virtual sal_Int8		SAL_CALL getRenderingCapabilities() throw( ::com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL addClipboardListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener ) throw( ::com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL removeClipboardListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener ) throw( ::com::sun::star::uno::RuntimeException );
	virtual ::rtl::OUString	SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
	virtual sal_Bool		SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );

	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >	SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );
	virtual void			SAL_CALL initialize( const com::sun::star::uno::Sequence<com::sun::star::uno::Any>& xAny ) throw( ::com::sun::star::uno::RuntimeException );
};

class JavaClipboardFactory : public ::cppu::WeakComponentImplHelper1< ::com::sun::star::lang::XSingleServiceFactory >
{
	::std::hash_map< ::rtl::OUString, ::com::sun::star::uno::Reference< XInterface >, ::rtl::OUStringHash >	maInstances;
	::osl::Mutex maMutex;

public:
							JavaClipboardFactory();
	virtual					~JavaClipboardFactory();

	virtual ::com::sun::star::uno::Reference< XInterface >	SAL_CALL createInstance() throw();
	virtual ::com::sun::star::uno::Reference< XInterface >	SAL_CALL createInstanceWithArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rArgs ) throw();
};

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL JavaClipboard_getSupportedServiceNames();
}

#endif
