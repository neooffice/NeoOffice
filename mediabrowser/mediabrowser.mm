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
 * service implementation:	 org.neooffice.MediaBrowser
 * exported interfaces:		 org.neooffice.XMediaBrowser
 *
 *************************************************************************
 *************************************************************************/

#include <stdio.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
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
#ifndef _ORG_NEOOFFICE_XGRAMMARCHECKER_HPP_
#include <org/neooffice/XMediaBrowser.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include "premac.h"
#include <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>
#include <mach-o/dyld.h>
#import <Foundation/NSObjCRuntime.h>
#include "postmac.h"

#define kMediaBrowserFrameworkName	"@executable_path/../Frameworks/iMediaBrowser.framework/Versions/A/iMediaBrowser"

#include <string>
#include <strstream>

#define SERVICENAME "org.neooffice.MediaBrowser"
#define IMPLNAME	"org.neooffice.XMediaBrowser"

#ifndef DLLPOSTFIX
#error DLLPOSTFIX must be defined in makefile.mk
#endif
 
#define DOSTRING( x )			#x
#define STRING( x )				DOSTRING( x )
 
typedef void ShowOnlyMenusForWindow_Type( void*, sal_Bool );
 
static ::vos::OModule aModule;
static ShowOnlyMenusForWindow_Type *pShowOnlyMenusForWindow = NULL;

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::org::neooffice;

//========================================================================
class MacOSXMediaBrowserImpl
	: public ::cppu::WeakImplHelper2<XServiceInfo, XMediaBrowser>
{
	// to obtain other services if needed
	Reference< XComponentContext > m_xServiceManager;
	
	sal_Int32 m_nRefCount;
	sal_Int32 m_nCount;
	OUString m_aLocale;
	
public:
	MacOSXMediaBrowserImpl( const Reference< XComponentContext > & xServiceManager )
		: m_xServiceManager( xServiceManager ), m_nRefCount( 0 ) {}
	virtual ~MacOSXMediaBrowserImpl() {}

    // XServiceInfo	implementation
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

	// XMediaBrowser implementation
	virtual ::sal_Bool 
		SAL_CALL hasMediaBrowser( ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool 
		SAL_CALL showMediaBrowser( ) 
		throw (::com::sun::star::uno::RuntimeException);
};

//*************************************************************************
OUString SAL_CALL MacOSXMediaBrowserImpl::getImplementationName(  ) 
	throw(RuntimeException)
{
	return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}	

//*************************************************************************
sal_Bool SAL_CALL MacOSXMediaBrowserImpl::supportsService( const OUString& ServiceName ) 
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
Sequence<OUString> SAL_CALL MacOSXMediaBrowserImpl::getSupportedServiceNames(  ) 
	throw(RuntimeException)
{
	return getSupportedServiceNames_Static();
}	

//*************************************************************************
Sequence<OUString> SAL_CALL MacOSXMediaBrowserImpl::getSupportedServiceNames_Static(  ) 
{
	OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
	return Sequence< OUString >( &aName, 1 );
}	




/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
Reference< XInterface > SAL_CALL MacOSXMediaBrowserImpl_create(
	const Reference< XComponentContext > & xContext )
{
	Reference< XTypeProvider > xRet;

	// Locate libvcl and invoke the ShowOnlyMenusForWindow function
	if ( !pShowOnlyMenusForWindow )
	{
		::rtl::OUString aLibName = ::rtl::OUString::createFromAscii( "libvcl" );
		aLibName += ::rtl::OUString::createFromAscii( STRING( DLLPOSTFIX ) );
		aLibName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".dylib" ) );
		if ( aModule.load( aLibName ) )
			pShowOnlyMenusForWindow = (ShowOnlyMenusForWindow_Type *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "ShowOnlyMenusForWindow" ) );
	}

	if ( pShowOnlyMenusForWindow )
		xRet = static_cast<XTypeProvider *>(new MacOSXMediaBrowserImpl(xContext));

	return xRet;
}


//#########################################################################
//#### EXPORTED ###########################################################
//#########################################################################

		/* shared lib exports implemented without helpers in service_impl1.cxx */
		namespace neo_macosxmediabrowser_impl
		{
		
	static Sequence< OUString > getSupportedServiceNames_MacOSXMediaBrowserImpl()
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
		 
		static OUString getImplementationName_MacOSXMediaBrowserImpl()
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
		        MacOSXMediaBrowserImpl_create, getImplementationName_MacOSXMediaBrowserImpl,
		        getSupportedServiceNames_MacOSXMediaBrowserImpl, ::cppu::createSingleComponentFactory,
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
	return ::cppu::component_writeInfoHelper(pServiceManager, pRegistryKey, ::neo_macosxmediabrowser_impl::s_component_entries);
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
	return ::cppu::component_getFactoryHelper(pImplName, pServiceManager, pRegistryKey, ::neo_macosxmediabrowser_impl::s_component_entries);
}

#pragma mark -

@interface CreateBrowserImpl : NSObject
{
}
- (id)init;
- (void)makeBrowser:(id)obj;
@end

@implementation CreateBrowserImpl

- (id)init
{
	self = [super init];
	return(self);
}

- (void)makeBrowser:(id)obj
{
	Class mbClass=NSClassFromString(@"iMediaBrowser");
	if(mbClass)
	{
		id mbInstance=[mbClass performSelector:@selector(sharedBrowser)];
		[mbInstance performSelector:@selector(showWindow:) withObject:nil];
	}
}
@end

/**
 * Check if the we have the iMediaBrowser framework available
 */
::sal_Bool 
		SAL_CALL MacOSXMediaBrowserImpl::hasMediaBrowser( ) 
		throw (::com::sun::star::uno::RuntimeException)
{
	// we currently need to be running on 10.3 in order to have the interfaces
	// for the media brwoser framework.  Check using our gestalt
	
	long res=0;
	if(Gestalt(gestaltSystemVersion, &res)==noErr)
	{
		bool isTigerOrHigher = ( ( ( ( res >> 8 ) & 0x00FF ) == 0x10 ) && ( ( ( res >> 4 ) & 0x000F ) >= 0x4 ) );
		if(!isTigerOrHigher)
			return(false);
	}
	
	// load our framework out of our bundle's directory
	
	const struct mach_header * frameworkLib=NSAddImage(kMediaBrowserFrameworkName, NSADDIMAGE_OPTION_RETURN_ON_ERROR | NSADDIMAGE_OPTION_WITH_SEARCHING);
	if(!frameworkLib)
		return(false);
	
	// check to see if we can locate our class after we've loaded the framework
	
	Class mbClass=NSClassFromString(@"iMediaBrowser");
	if(!mbClass)
		return(false);

	return(true);
}

/**
 * Construct a new media browser instance.
 */
::sal_Bool 
		SAL_CALL MacOSXMediaBrowserImpl::showMediaBrowser( ) 
		throw (::com::sun::star::uno::RuntimeException)
{
	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
	
	CreateBrowserImpl *imp=[[CreateBrowserImpl alloc] init];

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	unsigned long nCount = Application::ReleaseSolarMutex();
	[imp performSelectorOnMainThread:@selector(makeBrowser:) withObject:imp waitUntilDone: 1 modes: pModes];
	Application::AcquireSolarMutex( nCount );
		
	[imp release];
	
	[pool release];
	
	return(true);
}
