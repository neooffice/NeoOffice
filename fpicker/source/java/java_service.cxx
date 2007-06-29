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
 *  Patrick Luby, July 2006
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

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif
#ifndef _JAVA_SERVICE_HXX_
#include "java_service.hxx"
#endif
#ifndef _JAVA_FILEPICKER_HXX_
#include "java_filepicker.hxx"
#endif
#ifndef _JAVA_FOLDERPICKER_HXX_
#include "java_folderpicker.hxx"
#endif

using namespace cppu;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::uno;
using namespace rtl;
using namespace java;

extern "C"
{

void SAL_CALL component_getImplementationEnvironment(
	const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
	*ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo( void *pServiceManager, void *pRegistryKey )
{
	sal_Bool bRetVal = sal_False;

	if ( pRegistryKey )
	{
		try
		{
			Reference< XRegistryKey > pXNewKey( static_cast< XRegistryKey* >( pRegistryKey ) );
			pXNewKey->createKey( OUString::createFromAscii( FILE_PICKER_REGKEY_NAME ) );
			pXNewKey->createKey( OUString::createFromAscii( FOLDER_PICKER_REGKEY_NAME ) );
			bRetVal = sal_True;
		}
		catch( InvalidRegistryException& )
		{
			OSL_ENSURE(sal_False, "InvalidRegistryException caught");
			bRetVal = sal_False;
		}
	}

	return bRetVal;
}

void *SAL_CALL component_getFactory( const sal_Char *pImplName, uno_Interface *pSrvManager, uno_Interface *pRegistryKey )
{
	void *pRet = 0;

	Sequence< OUString > aSNS( 1 );

	if ( pSrvManager )
	{
		OUString aImplName( OUString::createFromAscii( pImplName ) );
		Reference< XMultiServiceFactory > xMgr( reinterpret_cast< XMultiServiceFactory* >( pSrvManager ) );
		Reference< XSingleServiceFactory > xFactory;
		if ( OUString::createFromAscii( FILE_PICKER_IMPL_NAME ).equals( aImplName ) )
		{
			aSNS[0] = OUString::createFromAscii( FILE_PICKER_SERVICE_NAME );
			xFactory = createSingleFactory( xMgr, aImplName, JavaFilePicker_createInstance, aSNS );
		}
		else if ( OUString::createFromAscii( FOLDER_PICKER_IMPL_NAME ).equals( aImplName ) )
		{
			aSNS[0] = OUString::createFromAscii( FOLDER_PICKER_SERVICE_NAME );
			xFactory = createSingleFactory( xMgr, aImplName, JavaFolderPicker_createInstance, aSNS );
		}

		if ( xFactory.is() )
		{
			xFactory->acquire();
			pRet = xFactory.get();
		}
	}

	return pRet;
}

}
