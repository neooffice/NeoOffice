/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to the terms of
 *  the GPL license.
 *  
 *  Copyright (c) 2007 Planamesa, Inc.
 *  All rights reserved.
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *		 - GNU General Public License Version 2.1
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2007 by Planamesa Inc. - http://www.planamesa.com
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
 *************************************************************************/

/*************************************************************************
 *************************************************************************
 *
 * service implementation:	 org.neooffice.OSXVersion
 * exported interfaces:		 org.neooffice.XOSXVersion
 *
 * simple glue component to allow for invocation of Gestalt from non-C++
 * languages with Uno bindings
 *
 *************************************************************************
 *************************************************************************/

#include <stdio.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx> // helper for queryInterface() impl
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx> // helper for component factory
#endif
#ifndef _CPPUHELPER_IMPLEMENATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

// generated c++ interfaces

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _ORG_NEOOFFICE_XOSXVERSION_HPP_
#include <org/neooffice/XOSXVersion.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#include "premac.h"
#import <Carbon/Carbon.h>
#include "postmac.h"

#include <string>
#include <strstream>

#define SERVICENAME "org.neooffice.OSXVersion"
#define IMPLNAME	"org.neooffice.XOSXVersion"

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::org::neooffice;

//========================================================================
class MacOSXOSXVersionImpl
	: public ::cppu::WeakImplHelper2<XServiceInfo, XOSXVersion>
{
	// to obtain other services if needed
	Reference< XComponentContext > m_xServiceManager;
	
	sal_Int32 m_nRefCount;
	sal_Int32 m_nCount;
	
public:
	MacOSXOSXVersionImpl( const Reference< XComponentContext > & xServiceManager )
		: m_xServiceManager( xServiceManager ), m_nRefCount( 0 )
		{ printf( "< MacOSXOSXVersionImpl ctor called >\n" ); }
	~MacOSXOSXVersionImpl()
		{ printf( "< MacOSXOSXVersionImpl dtor called >\n" ); }

    // XServiceInfo	implementation
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

	// XOSXVersion implementation
	virtual ::sal_uInt16 
		SAL_CALL sysvGestalt( ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Int16 
		SAL_CALL osMajor( ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Int16 
		SAL_CALL osMinor( ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Int16 
		SAL_CALL osRevision( ) 
		throw (::com::sun::star::uno::RuntimeException);
};

//*************************************************************************
OUString SAL_CALL MacOSXOSXVersionImpl::getImplementationName(  ) 
	throw(RuntimeException)
{
	return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}	

//*************************************************************************
sal_Bool SAL_CALL MacOSXOSXVersionImpl::supportsService( const OUString& ServiceName ) 
	throw(RuntimeException)
{
	Sequence< OUString > aSNL = getSupportedServiceNames();
	const OUString * pArray = aSNL.getArray();
	for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
		if( pArray[i] == ServiceName )
			return sal_True;
	return sal_False;
}	

//*************************************************************************
Sequence<OUString> SAL_CALL MacOSXOSXVersionImpl::getSupportedServiceNames(  ) 
	throw(RuntimeException)
{
	return getSupportedServiceNames_Static();
}	

//*************************************************************************
Sequence<OUString> SAL_CALL MacOSXOSXVersionImpl::getSupportedServiceNames_Static(  ) 
{
	OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
	return Sequence< OUString >( &aName, 1 );
}	




/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
Reference< XInterface > SAL_CALL MacOSXOSXVersionImpl_create(
	const Reference< XComponentContext > & xContext )
{
	return static_cast<XTypeProvider *>(new MacOSXOSXVersionImpl(xContext));
}


//#########################################################################
//#### EXPORTED ###########################################################
//#########################################################################

		/* shared lib exports implemented without helpers in service_impl1.cxx */
		namespace neo_macosXOSXVersion_impl
		{
		
	static Sequence< OUString > getSupportedServiceNames_MacOSXOSXVersionImpl()
		{
			static Sequence < OUString > *pNames = 0;
			if( ! pNames )
			{
		//		MutexGuard guard( Mutex::getGlobalMutex() );
				if( !pNames )
				{
					static Sequence< OUString > seqNames(1);
					seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICENAME));
					pNames = &seqNames;
				}
			}
			return *pNames;
		}
		 
		static OUString getImplementationName_MacOSXOSXVersionImpl()
		{
			static OUString *pImplName = 0;
			if( ! pImplName )
			{
		//		MutexGuard guard( Mutex::getGlobalMutex() );
				if( ! pImplName )
				{
					static OUString implName( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
				pImplName = &implName;
				}
			}
			return *pImplName;
	}
	
		static struct ::cppu::ImplementationEntry s_component_entries [] =
		{
		    //{
		    //    create_MyService1Impl, getImplementationName_MyService1Impl,
		    //    getSupportedServiceNames_MyService1Impl, ::cppu::createSingleComponentFactory,
		    //    0, 0
		    //},
		    {
		        MacOSXOSXVersionImpl_create, getImplementationName_MacOSXOSXVersionImpl,
		        getSupportedServiceNames_MacOSXOSXVersionImpl, ::cppu::createSingleComponentFactory,
		        0, 0
		    },
		    { 0, 0, 0, 0, 0, 0 }
		};
		}

/**
 * Gives the environment this component belongs to.
 */
extern "C" void SAL_CALL component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv)
{
	*ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

/**
 * This function creates an implementation section in the registry and another subkey
 *
 * for each supported service.
 * @param pServiceManager   the service manager
 * @param pRegistryKey      the registry key
 */
extern "C" sal_Bool SAL_CALL component_writeInfo(XMultiServiceFactory * pServiceManager, XRegistryKey * pRegistryKey)
{
	return ::cppu::component_writeInfoHelper(pServiceManager, pRegistryKey, ::neo_macosXOSXVersion_impl::s_component_entries);
}

/**
 * This function is called to get service factories for an implementation.
 *
 * @param pImplName       name of implementation
 * @param pServiceManager a service manager, need for component creation
 * @param pRegistryKey    the registry key for this component, need for persistent data
 * @return a component factory 
 */
extern "C" void * SAL_CALL component_getFactory(const sal_Char * pImplName, XMultiServiceFactory * pServiceManager, XRegistryKey * pRegistryKey)
{
	return ::cppu::component_getFactoryHelper(pImplName, pServiceManager, pRegistryKey, ::neo_macosXOSXVersion_impl::s_component_entries);
}

#pragma mark -

/**
 * Return the entire 16 bit result of the system component Gestalt.  This
 * is a decimal encoded major/minor/revision update of the OS version.
 */
::sal_uInt16 
	SAL_CALL MacOSXOSXVersionImpl::sysvGestalt( ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	long res=0;
	if(Gestalt(gestaltSystemVersion, &res)==noErr)
		return(res & 0x0000FFFF);
	
	return(0);
}

/**
 * Return the base 10 version of the underlying operating system.  This should
 * always return 10 in the forseeable future.
 */
::sal_Int16 
	SAL_CALL MacOSXOSXVersionImpl::osMajor( ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	long res=0;
	if(Gestalt(gestaltSystemVersion, &res)==noErr)
	{
		int osVersion=((res >> 12) & 0x000F)*10+((res >> 8) & 0x000F);
		return(osVersion);
	}
	
	return(0);
}

/**
 * Return the base 10 minor version of the operating system
 */
::sal_Int16 
	SAL_CALL MacOSXOSXVersionImpl::osMinor( ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	long res=0;
	if(Gestalt(gestaltSystemVersion, &res)==noErr)
		return((res >> 4) & 0x000F);
	
	return(0);
}

/**
 * Return the base 10 minor revision of the operating system
 */
::sal_Int16
	SAL_CALL MacOSXOSXVersionImpl::osRevision( ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	long res=0;
	if(Gestalt(gestaltSystemVersion, &res)==noErr)
		return(res & 0x000F);
	
	return(0);
}
