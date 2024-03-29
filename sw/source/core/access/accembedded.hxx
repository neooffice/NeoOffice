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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCEMBEDDED_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCEMBEDDED_HXX

#include "accnotextframe.hxx"

#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>

class SwAccessibleEmbeddedObject : public   SwAccessibleNoTextFrame
            , public ::com::sun::star::accessibility::XAccessibleExtendedAttributes

{
protected:
    virtual ~SwAccessibleEmbeddedObject();

public:
#ifdef NO_LIBO_BUG_58624_FIX
    SwAccessibleEmbeddedObject( SwAccessibleMap* pInitMap,
#else	// NO_LIBO_BUG_58624_FIX
    SwAccessibleEmbeddedObject(std::shared_ptr<SwAccessibleMap> const& pInitMap,
#endif	// NO_LIBO_BUG_58624_FIX
                                const SwFlyFrm* pFlyFrm );

    // XInterface

    virtual com::sun::star::uno::Any SAL_CALL
        queryInterface (const com::sun::star::uno::Type & rType)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL
        acquire (void)
        throw () SAL_OVERRIDE;

    virtual void SAL_CALL
        release (void)
        throw () SAL_OVERRIDE;

    // XServiceInfo

    // Returns an identifier for the implementation of this object.
    virtual OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Return whether the specified service is supported by this class.
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Returns a list of all supported services.  In this case that is just
    // the AccessibleContext service.
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleExtendedAttributes
        virtual ::com::sun::star::uno::Any SAL_CALL getExtendedAttributes()
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE ;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
