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

#ifndef _JAVA_FILEPICKER_HXX_
#define _JAVA_FILEPICKER_HXX_

#include <list>

#include <cppuhelper/compbase4.hxx>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <rtl/ustring.hxx>
#include <tools/resmgr.hxx>

#include <premac.h>
#include <objc/objc.h>
#include <postmac.h>

namespace java {

class JavaFilePicker : public ::cppu::WeakComponentImplHelper4< ::com::sun::star::ui::dialogs::XFilePicker3, ::com::sun::star::ui::dialogs::XFilePickerControlAccess, ::com::sun::star::lang::XInitialization, ::com::sun::star::lang::XServiceInfo >
{
	id					mpDialog;
	bool				mbInExecute;
	::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener > >	maListeners;
	::osl::Mutex		maMutex;
	ResMgr*				mpResMgr;
	sal_Int16			mnType;

public:
						JavaFilePicker();
	virtual				~JavaFilePicker();

    // XFilePickerNotifier
    virtual void SAL_CALL addFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual void SAL_CALL removeFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    // XExecutableDialog
    virtual void SAL_CALL setTitle( const OUString& aTitle ) throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL execute(  ) throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    // XFilePicker
    virtual void SAL_CALL setMultiSelectionMode( sal_Bool bMode ) throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual void SAL_CALL setDefaultName( const OUString& aName ) throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual void SAL_CALL setDisplayDirectory( const OUString& aDirectory ) throw( com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual OUString SAL_CALL getDisplayDirectory(  ) throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getFiles(  ) throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    // XFilterManager
    virtual void SAL_CALL appendFilter( const OUString& aTitle, const OUString& aFilter ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual void SAL_CALL setCurrentFilter( const OUString& aTitle ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual OUString SAL_CALL getCurrentFilter(  ) throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    // XFilterGroupManager
    virtual void SAL_CALL appendFilterGroup( const OUString& sGroupTitle, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aFilters ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    // XFilePickerControlAccess
    virtual void SAL_CALL setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL enableControl( sal_Int16 nControlId, sal_Bool bEnable ) throw(::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;
    virtual void SAL_CALL setLabel( sal_Int16 nControlId, const OUString& aLabel ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual OUString SAL_CALL getLabel( sal_Int16 nControlId ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    // XCancellable
    virtual void SAL_CALL cancel( ) throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

    // XEventListener
    using cppu::WeakComponentImplHelperBase::disposing;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    // FilePicker event functions
   void SAL_CALL fileSelectionChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent );
   void SAL_CALL directoryChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent );
   OUString SAL_CALL helpRequested( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent ) const;
   void SAL_CALL controlStateChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent );
   void SAL_CALL dialogSizeChanged( );

private:
	void				implInit() throw( ::com::sun::star::uno::Exception );
};

}

#endif	// _JAVA_FILEPICKER_HXX_
