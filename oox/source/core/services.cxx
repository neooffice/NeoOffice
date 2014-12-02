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

#include <services.hxx>

#if SUPD == 310
#include <string.h>
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif	// SUPD == 310

using namespace ::com::sun::star::uno;

#if SUPD == 310

#ifdef __cplusplus
extern "C"
{
#endif

OOX_DLLPUBLIC void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
	*ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

void SAL_CALL writeInfo( css::registry::XRegistryKey * pRegistryKey, const OUString& rImplementationName, const css::uno::Sequence< OUString >& rServices )
{
	css::uno::Reference< css::registry::XRegistryKey > xNewKey(
		pRegistryKey->createKey(
            OUString( sal_Unicode( '/' ) ) + rImplementationName + OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) ) );

	for( sal_Int32 i = 0; i < rServices.getLength(); i++ )
		xNewKey->createKey( rServices.getConstArray()[i]);
}

#define WRITEINFO(className)\
	writeInfo( pKey, className##_getImplementationName(), className##_getSupportedServiceNames() )

OOX_DLLPUBLIC sal_Bool SAL_CALL component_writeInfo( void * , void * pRegistryKey )
{
	if( pRegistryKey )
	{
		try
		{
			css::registry::XRegistryKey *pKey = reinterpret_cast< css::registry::XRegistryKey * >( pRegistryKey );

            WRITEINFO( ::oox::core::FastTokenHandler );
            WRITEINFO( ::oox::core::FilterDetect );
            WRITEINFO( ::oox::docprop::DocumentPropertiesImport );
            WRITEINFO( ::oox::ppt::PowerPointImport );
            WRITEINFO( ::oox::ppt::QuickDiagrammingImport );
            WRITEINFO( ::oox::ppt::QuickDiagrammingLayout );
            WRITEINFO( ::oox::shape::ShapeContextHandler );
            WRITEINFO( ::oox::xls::BiffDetector );
            WRITEINFO( ::oox::xls::ExcelFilter );
            WRITEINFO( ::oox::xls::ExcelBiffFilter );
		}
		catch (css::registry::InvalidRegistryException &)
		{
			OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
		}
	}
    return sal_True;
}

#define SINGLEFACTORY(classname)\
		if( classname##_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )\
		{\
			xFactory = ::cppu::createSingleFactory( xMSF,\
				classname##_getImplementationName(),\
				classname##_createInstance,\
				classname##_getSupportedServiceNames() );\
		}

#define SINGLEFACTORY2(classname)\
		if( classname##_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )\
		{\
			xCompFactory = ::cppu::createSingleComponentFactory(\
				classname##_createInstance,\
				classname##_getImplementationName(),\
				classname##_getSupportedServiceNames() );\
		}

OOX_DLLPUBLIC void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * )
{
	void * pRet = 0;
	if( pServiceManager )
	{
		css::uno::Reference< css::lang::XMultiServiceFactory > xMSF( reinterpret_cast< css::lang::XMultiServiceFactory * >( pServiceManager ) );

		css::uno::Reference< css::lang::XSingleServiceFactory > xFactory;
		css::uno::Reference< css::lang::XSingleComponentFactory > xCompFactory;

		const sal_Int32 nImplNameLen = strlen( pImplName );

		// impress oasis import
        SINGLEFACTORY2( ::oox::core::FastTokenHandler )
        else SINGLEFACTORY2( ::oox::core::FilterDetect )
        else SINGLEFACTORY2( ::oox::docprop::DocumentPropertiesImport )
        else SINGLEFACTORY2( ::oox::ppt::PowerPointImport )
        else SINGLEFACTORY2( ::oox::ppt::QuickDiagrammingImport )
        else SINGLEFACTORY2( ::oox::ppt::QuickDiagrammingLayout )
        else SINGLEFACTORY2( ::oox::shape::ShapeContextHandler )
        else SINGLEFACTORY2( ::oox::xls::BiffDetector )
        else SINGLEFACTORY2( ::oox::xls::ExcelFilter )
        else SINGLEFACTORY2( ::oox::xls::ExcelBiffFilter )

		if( xFactory.is())
		{
			xFactory->acquire();
			pRet = xFactory.get();
		}
        else if ( xCompFactory.is() )
        {
            xCompFactory->acquire();
            pRet = xCompFactory.get();
        }
	}
	return pRet;
}

#ifdef __cplusplus
}
#endif

#else	// SUPD == 310

namespace {

#define IMPLEMENTATION_ENTRY( className ) \
    { &className##_createInstance, &className##_getImplementationName, &className##_getSupportedServiceNames, ::cppu::createSingleComponentFactory, 0, 0 }

//TODO: QuickDiagrammingImport and QuickDiagrammingLayout are not listed in
// oox/util/oox.component (and not directly referenced from anywhere in the code
// either); it is unclear whether they are dead code or whether
// a81327ff2faaf21c22f1a902bea170942d5207e6 "Import SmartArt graphics to
// Impress" would actually want to make use of them:
static ::cppu::ImplementationEntry const spServices[] =
{
    IMPLEMENTATION_ENTRY( ::oox::core::FastTokenHandler ),
    IMPLEMENTATION_ENTRY( ::oox::core::FilterDetect ),
    IMPLEMENTATION_ENTRY( ::oox::docprop::DocumentPropertiesImport ),
    IMPLEMENTATION_ENTRY( ::oox::ppt::PowerPointImport ),
    IMPLEMENTATION_ENTRY( ::oox::ppt::QuickDiagrammingImport ),
    IMPLEMENTATION_ENTRY( ::oox::ppt::QuickDiagrammingLayout ),
    IMPLEMENTATION_ENTRY( ::oox::shape::ShapeContextHandler ),
    { 0, 0, 0, 0, 0, 0 }
};

#undef IMPLEMENTATION_ENTRY

} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL oox_component_getFactory( const char* pImplName, void* pServiceManager, void* pRegistryKey )
{
    return ::cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey, spServices );
}

#endif	// SUPD == 310

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
