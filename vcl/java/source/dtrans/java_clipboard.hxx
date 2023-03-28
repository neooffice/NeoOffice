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

#ifndef _JAVA_CLIPBOARD_HXX_
#define _JAVA_CLIPBOARD_HXX_

#include <unordered_map>
#include <list>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/compbase3.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XSystemClipboard.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include "DTransClipboard.hxx"

class JavaClipboard : public ::cppu::WeakComponentImplHelper3< ::com::sun::star::datatransfer::clipboard::XSystemClipboard, ::com::sun::star::datatransfer::clipboard::XFlushableClipboard, ::com::sun::star::lang::XServiceInfo >
{
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	maContents;
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >	maOwner;
	::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener > >	maListeners;
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	maPrivateContents;
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >	maPrivateOwner;
	::osl::Mutex			maMutex;
	bool					mbSystemClipboard;
	sal_Bool				mbPrivateClipboard;

public:
							JavaClipboard( bool bSystemClipboard );
	virtual					~JavaClipboard();

	// XFlushableClipboard
    virtual void			SAL_CALL flushClipboard( ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

	// XClipboard
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	SAL_CALL getContents() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual void			SAL_CALL setContents( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTransferable, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual OUString		SAL_CALL getName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

	// XClipboardEx
	virtual sal_Int8		SAL_CALL getRenderingCapabilities() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

	// XClipboardNotifier
	virtual void			SAL_CALL addClipboardListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual void			SAL_CALL removeClipboardListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

	// XServiceInfo
	virtual OUString		SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
	virtual sal_Bool		SAL_CALL supportsService( const OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
	virtual ::com::sun::star::uno::Sequence< OUString >	SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

	void					setPrivateClipboard( sal_Bool bPrivateClipboard );
};

#endif
