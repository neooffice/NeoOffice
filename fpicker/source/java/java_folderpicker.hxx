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
 *  Patrick Luby, July 2006
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2006 Planamesa Inc.
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

#ifndef _JAVA_FOLDERPICKER_HXX_
#define _JAVA_FOLDERPICKER_HXX_

#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <rtl/ustring.hxx>

#include <premac.h>
#include <objc/objc.h>
#include <postmac.h>

namespace java {

class JavaFolderPicker : public ::cppu::WeakImplHelper3< ::com::sun::star::ui::dialogs::XFolderPicker2, ::com::sun::star::lang::XServiceInfo, ::com::sun::star::lang::XEventListener >
{
	id					mpDialog;
	::osl::Mutex		maMutex;

public:
						JavaFolderPicker( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceMgr );
	virtual				~JavaFolderPicker();

    // XExecutableDialog
    virtual void SAL_CALL setTitle( const OUString& aTitle ) throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL execute(  ) throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    // XFolderPicker
    virtual void SAL_CALL setDisplayDirectory( const OUString& rDirectory ) throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual OUString SAL_CALL getDisplayDirectory(  ) throw( com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual OUString SAL_CALL getDirectory( ) throw( com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual void SAL_CALL setDescription( const OUString& rDescription ) throw( com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    // XCancellable
    virtual void SAL_CALL cancel( ) throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
};

}

#endif	// _JAVA_FOLDERPICKER_HXX_
