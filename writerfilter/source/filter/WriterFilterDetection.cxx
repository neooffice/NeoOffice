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

#include <cppuhelper/implementationentry.hxx>
#if SUPD != 310
#include <cppuhelper/supportsservice.hxx>
#endif	// SUPD != 310
#include <WriterFilterDetection.hxx>
#include <comphelper/storagehelper.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <osl/diagnose.h>
#include <sot/storage.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::com::sun::star;

WriterFilterDetection::WriterFilterDetection(
    const uno::Reference< uno::XComponentContext >& rxContext) :
    m_xContext( rxContext )
{
}


WriterFilterDetection::~WriterFilterDetection()
{
}


OUString WriterFilterDetection_getImplementationName () throw (uno::RuntimeException)
{
   return OUString ( "com.sun.star.comp.Writer.WriterFilterDetector"  );
}



OUString WriterFilterDetection::detect( uno::Sequence< beans::PropertyValue >& rDescriptor )
#if SUPD == 310
   throw( uno::RuntimeException )
#else	//SUPD == 310
   throw( uno::RuntimeException, std::exception )
#endif	//SUPD == 310
{
    OUString sTypeName;
    bool bWord = false;
    sal_Int32 nPropertyCount = rDescriptor.getLength();
    const beans::PropertyValue* pValues = rDescriptor.getConstArray();
    OUString sURL;
    uno::Reference < io::XStream > xStream;
    uno::Reference < io::XInputStream > xInputStream;
    for( sal_Int32 nProperty = 0; nProperty < nPropertyCount; ++nProperty )
    {
        if ( pValues[nProperty].Name == "TypeName" )
            rDescriptor[nProperty].Value >>= sTypeName;
        else if ( pValues[nProperty].Name == "URL" )
            pValues[nProperty].Value >>= sURL;
        else if ( pValues[nProperty].Name == "Stream" )
            pValues[nProperty].Value >>= xStream;
        else if ( pValues[nProperty].Name == "InputStream" )
            pValues[nProperty].Value >>= xInputStream;
    }
    try
    {
        uno::Reference< embed::XStorage > xDocStorage;
        if ( sURL == "private:stream" )
            xDocStorage = comphelper::OStorageHelper::GetStorageFromInputStream( xInputStream );
        else
            xDocStorage = comphelper::OStorageHelper::GetStorageFromURL( sURL, embed::ElementModes::READ );
        if( xDocStorage.is() )
        {
            uno::Sequence< OUString > aNames = xDocStorage->getElementNames();
            const OUString* pNames = aNames.getConstArray();
            for(sal_Int32 nName = 0; nName < aNames.getLength(); ++nName)
            {
                if ( pNames[nName] == "word" )
                {
                    bWord = true;
                    if( sTypeName.isEmpty() )
                        sTypeName = "writer_MS_Word_2007";
                    break;
                }
            }
        }
    }
    catch(const uno::Exception&)
    {
        OSL_FAIL("exception while opening storage");
    }
    if( !bWord )
#if SUPD == 310
        sTypeName = OUString();
#else	// SUPD == 310
        sTypeName.clear();
#endif	// SUPD == 310
   return sTypeName;
}


#if SUPD == 310

#define SERVICE_NAME1 "com.sun.star.document.ExtendedTypeDetection"

bool WriterFilterDetection_supportsService( const OUString& ServiceName ) throw (uno::RuntimeException)
{
   return (ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME1 ) ) );
}

#endif	// SUPD == 310

uno::Sequence< OUString > WriterFilterDetection_getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
   uno::Sequence < OUString > aRet(1);
   OUString* pArray = aRet.getArray();
   pArray[0] = "com.sun.star.document.ExtendedTypeDetection";
   return aRet;
}


uno::Reference< uno::XInterface > WriterFilterDetection_createInstance( const uno::Reference< uno::XComponentContext >& xContext)
                throw( uno::Exception )
{
   return (cppu::OWeakObject*) new WriterFilterDetection( xContext );
}


#if SUPD == 310
OUString WriterFilterDetection::getImplementationName(  ) throw (uno::RuntimeException)
#else	//SUPD == 310
OUString WriterFilterDetection::getImplementationName(  ) throw (uno::RuntimeException, std::exception)
#endif	//SUPD == 310
{
   return WriterFilterDetection_getImplementationName();
}


#if SUPD == 310
sal_Bool WriterFilterDetection::supportsService( const OUString& rServiceName ) throw (uno::RuntimeException)
#else	//SUPD == 310
sal_Bool WriterFilterDetection::supportsService( const OUString& rServiceName ) throw (uno::RuntimeException, std::exception)
#endif	//SUPD == 310
{
#if SUPD == 310
    return WriterFilterDetection_supportsService( rServiceName );
#else	//SUPD == 310
    return cppu::supportsService( this, rServiceName );
#endif	//SUPD == 310
}


#if SUPD == 310
uno::Sequence< OUString > WriterFilterDetection::getSupportedServiceNames(  ) throw (uno::RuntimeException)
#else	//SUPD == 310
uno::Sequence< OUString > WriterFilterDetection::getSupportedServiceNames(  ) throw (uno::RuntimeException, std::exception)
#endif	//SUPD == 310
{
    return WriterFilterDetection_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
