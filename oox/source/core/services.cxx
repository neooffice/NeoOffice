/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.
 * 
 * $RCSfile$
 * $Revision$
 * 
 * This file is part of NeoOffice.
 * 
 * NeoOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * only, as published by the Free Software Foundation.
 * 
 * NeoOffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 * 
 * You should have received a copy of the GNU General Public License
 * version 3 along with NeoOffice.  If not, see
 * <http://www.gnu.org/licenses/gpl-3.0.txt>
 * for a copy of the GPLv3 License.
 * 
 * Modified October 2014 by Patrick Luby. NeoOffice is distributed under
 * GPL only under Section 4 of the Apache License v2.0.
 * 
 *************************************************************/



#include <cppuhelper/implementationentry.hxx>
#if SUPD == 310
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif	// SUPD == 310

using ::rtl::OUString;
#if SUPD == 310
using namespace com::sun::star;
#endif	// SUPD == 310
using namespace ::com::sun::star::uno;

// Declare static functions providing service information =====================

#define DECLARE_FUNCTIONS( className )                                                  \
extern OUString SAL_CALL className##_getImplementationName() throw();                   \
extern Sequence< OUString > SAL_CALL className##_getSupportedServiceNames() throw();    \
extern Reference< XInterface > SAL_CALL className##_createInstance(                     \
    const Reference< XComponentContext >& rxContext ) throw (Exception)

namespace oox {
    namespace core {    DECLARE_FUNCTIONS( FastTokenHandler );          }
    namespace core {    DECLARE_FUNCTIONS( FilterDetect );              }
    namespace docprop { DECLARE_FUNCTIONS( DocumentPropertiesImport );  }
    namespace ole {     DECLARE_FUNCTIONS( WordVbaProjectFilter );      }
    namespace ppt {     DECLARE_FUNCTIONS( PowerPointImport );          }
    namespace shape {   DECLARE_FUNCTIONS( ShapeContextHandler );       }
    namespace xls {     DECLARE_FUNCTIONS( BiffDetector );              }
    namespace xls {     DECLARE_FUNCTIONS( ExcelFilter );               }
    namespace xls {     DECLARE_FUNCTIONS( ExcelBiffFilter );           }
    namespace xls {     DECLARE_FUNCTIONS( ExcelVbaProjectFilter );     }
    namespace xls {     DECLARE_FUNCTIONS( OOXMLFormulaParser );        }
}

#undef DECLARE_FUNCTIONS

// ============================================================================

namespace {

#define IMPLEMENTATION_ENTRY( className ) \
    { &className##_createInstance, &className##_getImplementationName, &className##_getSupportedServiceNames, ::cppu::createSingleComponentFactory, 0, 0 }

static ::cppu::ImplementationEntry const spServices[] =
{
    IMPLEMENTATION_ENTRY( ::oox::core::FastTokenHandler ),
    IMPLEMENTATION_ENTRY( ::oox::core::FilterDetect ),
    IMPLEMENTATION_ENTRY( ::oox::docprop::DocumentPropertiesImport ),
    IMPLEMENTATION_ENTRY( ::oox::ole::WordVbaProjectFilter ),
    IMPLEMENTATION_ENTRY( ::oox::ppt::PowerPointImport ),
    IMPLEMENTATION_ENTRY( ::oox::shape::ShapeContextHandler ),
    IMPLEMENTATION_ENTRY( ::oox::xls::BiffDetector ),
    IMPLEMENTATION_ENTRY( ::oox::xls::ExcelFilter ),
    IMPLEMENTATION_ENTRY( ::oox::xls::ExcelBiffFilter ),
    IMPLEMENTATION_ENTRY( ::oox::xls::ExcelVbaProjectFilter ),
    IMPLEMENTATION_ENTRY( ::oox::xls::OOXMLFormulaParser ),
    { 0, 0, 0, 0, 0, 0 }
};

#undef IMPLEMENTATION_ENTRY

} // namespace

// ----------------------------------------------------------------------------

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
        const sal_Char** ppEnvironmentTypeName, uno_Environment** /*ppEnvironment*/ )
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

#if SUPD == 310

void SAL_CALL writeInfo( registry::XRegistryKey * pRegistryKey, const OUString& rImplementationName, const uno::Sequence< OUString >& rServices )
{
	uno::Reference< registry::XRegistryKey > xNewKey(
		pRegistryKey->createKey(
            OUString( sal_Unicode( '/' ) ) + rImplementationName + OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) ) );

	for( sal_Int32 i = 0; i < rServices.getLength(); i++ )
		xNewKey->createKey( rServices.getConstArray()[i]);
}

#define WRITEINFO(className)\
	writeInfo( pKey, className##_getImplementationName(), className##_getSupportedServiceNames() )

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo( void * , void * pRegistryKey )
{
	if( pRegistryKey )
	{
		try
		{
			registry::XRegistryKey *pKey = reinterpret_cast< registry::XRegistryKey * >( pRegistryKey );

            WRITEINFO( ::oox::core::FastTokenHandler );
            WRITEINFO( ::oox::core::FilterDetect );
            WRITEINFO( ::oox::docprop::DocumentPropertiesImport );
            WRITEINFO( ::oox::ole::WordVbaProjectFilter );
			WRITEINFO( ::oox::ppt::PowerPointImport );
            WRITEINFO( ::oox::shape::ShapeContextHandler );
            WRITEINFO( ::oox::xls::BiffDetector );
            WRITEINFO( ::oox::xls::ExcelFilter );
            WRITEINFO( ::oox::xls::ExcelBiffFilter );
            WRITEINFO( ::oox::xls::ExcelVbaProjectFilter );
            WRITEINFO( ::oox::xls::OOXMLFormulaParser );
		}
		catch (registry::InvalidRegistryException &)
		{
			OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
		}
	}
    return sal_True;
}

#endif	// SUPD == 310

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory( const char* pImplName, void* pServiceManager, void* pRegistryKey )
{
    return ::cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey, spServices );
}

// ============================================================================
