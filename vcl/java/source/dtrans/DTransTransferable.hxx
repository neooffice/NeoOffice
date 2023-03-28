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

#ifndef _DTRANSTRANSFERABLE_HXX
#define _DTRANSTRANSFERABLE_HXX

#include <cppuhelper/compbase1.hxx>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>

#include <premac.h>
#include <objc/NSObjCRuntime.h>
#include <postmac.h>

#define TRANSFERABLE_TYPE_CLIPBOARD		0x0
#define TRANSFERABLE_TYPE_DRAG			0x1

#ifdef __OBJC__
@class NSArray;
@class NSString;
#else
typedef void* id;
struct NSArray;
struct NSString;
#endif

class DTransTransferable : public ::cppu::WeakImplHelper1 < ::com::sun::star::datatransfer::XTransferable >
{
private:
	NSInteger			mnChangeCount;
	NSString*			mpPasteboardName;
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	mxTransferable;

public:
	static NSArray*		getSupportedPasteboardTypes();

						// Passing NULL uses the system clipboard
						DTransTransferable( NSString *pPasteboardName = NULL );
	virtual				~DTransTransferable();

	// XTransferable
	virtual ::com::sun::star::uno::Any getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw ( ::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > getTransferDataFlavors() throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
	virtual sal_Bool	isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

	void				flush();
	NSInteger			getChangeCount();
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	getTransferable() { return mxTransferable; }
	sal_Bool			hasOwnership();
	sal_Bool			setContents( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > &xTransferable, id *pPasteboardWriter = NULL );
	void				updateChangeCount();
};

#endif // _DTRANSTRANSFERABLE_HXX
