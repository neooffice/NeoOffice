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

#include "oox/core/fasttokenhandler.hxx"

#include <com/sun/star/uno/XComponentContext.hpp>
#include "oox/helper/helper.hxx"
#include "oox/token/tokenmap.hxx"
#if SUPD != 310
#include <cppuhelper/supportsservice.hxx>
#endif	// SUPD != 310

#include <services.hxx>

namespace oox {
namespace core {

using namespace ::com::sun::star::uno;

OUString SAL_CALL FastTokenHandler_getImplementationName()
{
#if SUPD == 310
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.oox.FastTokenHandlerService"));
#else	// SUPD == 310
    return OUString( "com.sun.star.comp.oox.core.FastTokenHandler" );
#endif	// SUPD == 310
}

Sequence< OUString > SAL_CALL FastTokenHandler_getSupportedServiceNames()
{
#if SUPD == 310
    css::uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.xml.sax.FastTokenHandler"));
    return s;
#else	// SUPD == 310
    Sequence< OUString > aServiceNames( 1 );
    aServiceNames[ 0 ] = "com.sun.star.xml.sax.FastTokenHandler";
    return aServiceNames;
#endif	// SUPD == 310
}

Reference< XInterface > SAL_CALL FastTokenHandler_createInstance( const Reference< XComponentContext >& /*rxContext*/ ) throw (Exception)
{
    return static_cast< ::cppu::OWeakObject* >( new FastTokenHandler );
}

FastTokenHandler::FastTokenHandler() :
    mrTokenMap( StaticTokenMap::get() )
{
}

FastTokenHandler::~FastTokenHandler()
{
}

// XServiceInfo
#if SUPD == 310
OUString SAL_CALL FastTokenHandler::getImplementationName() throw (RuntimeException)
#else	// SUPD == 310
OUString SAL_CALL FastTokenHandler::getImplementationName() throw (RuntimeException, std::exception)
#endif	// SUPD == 310
{
    return FastTokenHandler_getImplementationName();
}

#if SUPD == 310
sal_Bool SAL_CALL FastTokenHandler::supportsService( const OUString& rServiceName ) throw (RuntimeException)
#else	// SUPD == 310
sal_Bool SAL_CALL FastTokenHandler::supportsService( const OUString& rServiceName ) throw (RuntimeException, std::exception)
#endif	// SUPD == 310
{
#if SUPD == 310
    Sequence< OUString > aServiceNames = FastTokenHandler_getSupportedServiceNames();
    for( sal_Int32 nIndex = 0, nLength = aServiceNames.getLength(); nIndex < nLength; ++nIndex )
        if( aServiceNames[ nIndex ] == rServiceName )
            return sal_True;
    return sal_False;
#else	// SUPD == 310
    return cppu::supportsService(this, rServiceName);
#endif	// SUPD == 310
}

#if SUPD == 310
Sequence< OUString > SAL_CALL FastTokenHandler::getSupportedServiceNames() throw (RuntimeException)
#else	// SUPD == 310
Sequence< OUString > SAL_CALL FastTokenHandler::getSupportedServiceNames() throw (RuntimeException, std::exception)
#endif	// SUPD == 310
{
    return FastTokenHandler_getSupportedServiceNames();
}

// XFastTokenHandler
#if SUPD == 310
sal_Int32 FastTokenHandler::getToken( const OUString& rIdentifier ) throw( RuntimeException )
#else	// SUPD == 310
sal_Int32 FastTokenHandler::getToken( const OUString& rIdentifier ) throw( RuntimeException, std::exception )
#endif	// SUPD == 310
{
    return mrTokenMap.getTokenFromUnicode( rIdentifier );
}

#if SUPD == 310
OUString FastTokenHandler::getIdentifier( sal_Int32 nToken ) throw( RuntimeException )
#else	// SUPD == 310
OUString FastTokenHandler::getIdentifier( sal_Int32 nToken ) throw( RuntimeException, std::exception )
#endif	// SUPD == 310
{
    return mrTokenMap.getUnicodeTokenName( nToken );
}

#if SUPD == 310
Sequence< sal_Int8 > FastTokenHandler::getUTF8Identifier( sal_Int32 nToken ) throw( RuntimeException )
#else	// SUPD == 310
Sequence< sal_Int8 > FastTokenHandler::getUTF8Identifier( sal_Int32 nToken ) throw( RuntimeException, std::exception )
#endif	// SUPD == 310
{
    return mrTokenMap.getUtf8TokenName( nToken );
}

#if SUPD == 310
sal_Int32 FastTokenHandler::getTokenFromUTF8( const Sequence< sal_Int8 >& rIdentifier ) throw( RuntimeException )
#else	// SUPD == 310
sal_Int32 FastTokenHandler::getTokenFromUTF8( const Sequence< sal_Int8 >& rIdentifier ) throw( RuntimeException, std::exception )
#endif	// SUPD == 310
{
    return mrTokenMap.getTokenFromUtf8( rIdentifier );
}

sal_Int32 FastTokenHandler::getTokenDirect( const char *pToken, sal_Int32 nLength ) const
{
    return mrTokenMap.getTokenFromUTF8( pToken, nLength );
}



} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
