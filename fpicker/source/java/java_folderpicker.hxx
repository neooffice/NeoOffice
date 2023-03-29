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
