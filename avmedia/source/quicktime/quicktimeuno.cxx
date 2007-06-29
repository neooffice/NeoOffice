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
 *  Patrick Luby, May 2006
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

#include "mediamisc.hxx"
#include "quicktimemanager.hxx"

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::uno;

// ============================================================================

static Reference< XInterface > SAL_CALL create_MediaPlayer( const Reference< XMultiServiceFactory >& rxFact )
{
	return Reference< XInterface >( *new ::avmedia::quicktime::Manager( rxFact ) );
}

// ----------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
	*ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// ----------------------------------------------------------------------------

extern "C" sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, void* pRegistryKey )
{
	sal_Bool bRet = sal_False;

	if( pRegistryKey )
	{
		try
		{
			Reference< XRegistryKey > xNewKey1( static_cast< XRegistryKey* >( pRegistryKey )->createKey( ::rtl::OUString::createFromAscii( "/" AVMEDIA_MANAGER_SERVICE_NAME "/UNO/SERVICES/" AVMEDIA_MANAGER_SERVICE_NAME ) ) );
			bRet = sal_True;
		}
		catch( InvalidRegistryException& )
		{
			OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------------

extern "C" void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* pRegistryKey )
{
	Reference< XSingleServiceFactory > xFactory;
	void *pRet = 0;

	if( rtl_str_compare( pImplName, AVMEDIA_MANAGER_SERVICE_NAME ) == 0 )
	{
		const ::rtl::OUString aServiceName( ::rtl::OUString::createFromAscii( AVMEDIA_MANAGER_SERVICE_NAME ) );

		xFactory = Reference< XSingleServiceFactory >( ::cppu::createSingleFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ), ::rtl::OUString::createFromAscii( AVMEDIA_MANAGER_SERVICE_NAME ), create_MediaPlayer, Sequence< ::rtl::OUString >( &aServiceName, 1 ) ) );
	}

	if( xFactory.is() )
	{
		xFactory->acquire();
		pRet = xFactory.get();
	}

	return pRet;
}
