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
 * service implementation:	 org.neooffice.ImageCapture
 * exported interfaces:		 org.neooffice.XImageCapture
 *
 * simple glue mapping image capture functions onto UNO types
 * to export them within the OOo scripting environment
 *
 *************************************************************************
 *************************************************************************/

#include <map>
#include <dlfcn.h>

#include <rtl/ustring.hxx>
#include <sfx2/sfx.hrc>
#include <tools/resmgr.hxx>
#include <tools/simplerm.hxx>
#include <vcl/svapp.hxx>
#include <vos/module.hxx>
 
#include <cppuhelper/queryinterface.hxx> // helper for queryInterface() impl
#include <cppuhelper/factory.hxx> // helper for component factory
#include <cppuhelper/implementationentry.hxx>

// generated c++ interfaces

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <org/neooffice/XImageCapture.hpp>
#include <cppuhelper/implbase2.hxx>

#include "premac.h"
#import <Cocoa/Cocoa.h>
#include "postmac.h"

#define SERVICENAME "org.neooffice.ImageCapture"
#define IMPLNAME	"org.neooffice.XImageCapture"

#ifndef DLLPOSTFIX
#error DLLPOSTFIX must be defined in makefile.mk
#endif
 
#define DOSTRING( x )			#x
#define STRING( x )				DOSTRING( x )

typedef void ShowOnlyMenusForWindow_Type( void*, sal_Bool );
 
static ::vos::OModule aModule;
static ShowOnlyMenusForWindow_Type *pShowOnlyMenusForWindow = NULL;

@interface ImageCaptureImpl : NSObject
{
	bool gotImage;
}
+ (id)create;
- (id)init;
- (void)doImageCapture:(id)pObj;
- (bool)capturedImage;
- (void)setCapturedImage:(bool)bCaptured;
@end

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::org::neooffice;
using namespace ::vos;

//========================================================================

class MacOSXImageCaptureImpl
	: public ::cppu::WeakImplHelper2<XServiceInfo, XImageCapture>
{
	// to obtain other services if needed
	Reference< XComponentContext > m_xServiceManager;

public:
	MacOSXImageCaptureImpl( const Reference< XComponentContext > & xServiceManager )
		: m_xServiceManager( xServiceManager ) {}
	virtual ~MacOSXImageCaptureImpl() {}

    // XServiceInfo	implementation
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

	// XImageCaptureImpl
    virtual ::sal_Bool SAL_CALL hasImageCapture(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL captureImageToClipboard(  ) throw (::com::sun::star::uno::RuntimeException);
};

//*************************************************************************
OUString SAL_CALL MacOSXImageCaptureImpl::getImplementationName(  ) 
	throw(RuntimeException)
{
	return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}	

//*************************************************************************
sal_Bool SAL_CALL MacOSXImageCaptureImpl::supportsService( const OUString& ServiceName ) 
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
Sequence<OUString> SAL_CALL MacOSXImageCaptureImpl::getSupportedServiceNames(  ) 
	throw(RuntimeException)
{
	return getSupportedServiceNames_Static();
}	

//*************************************************************************
Sequence<OUString> SAL_CALL MacOSXImageCaptureImpl::getSupportedServiceNames_Static(  ) 
{
	OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
	return Sequence< OUString >( &aName, 1 );
}	




/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
Reference< XInterface > SAL_CALL MacOSXImageCaptureImpl_create(
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
		xRet = static_cast<XTypeProvider *>(new MacOSXImageCaptureImpl(xContext));

	return xRet;
}


//#########################################################################
//#### EXPORTED ###########################################################
//#########################################################################

		/* shared lib exports implemented without helpers in service_impl1.cxx */
		namespace neo_macosximagecapture_impl
		{
		
	static Sequence< OUString > getSupportedServiceNames_MacOSXImageCaptureImpl()
		{
			static Sequence < OUString > *pNames = 0;
			if( ! pNames )
			{
				if( !pNames )
				{
					static Sequence< OUString > seqNames(1);
					seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICENAME));
					pNames = &seqNames;
				}
			}
			return *pNames;
		}
		 
		static OUString getImplementationName_MacOSXImageCaptureImpl()
		{
			static OUString *pImplName = 0;
			if( ! pImplName )
			{
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
		        MacOSXImageCaptureImpl_create, getImplementationName_MacOSXImageCaptureImpl,
		        getSupportedServiceNames_MacOSXImageCaptureImpl, ::cppu::createSingleComponentFactory,
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
	return ::cppu::component_writeInfoHelper(pServiceManager, pRegistryKey, ::neo_macosximagecapture_impl::s_component_entries);
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
	return ::cppu::component_getFactoryHelper(pImplName, pServiceManager, pRegistryKey, ::neo_macosximagecapture_impl::s_component_entries);
}

#pragma mark -

/**
 * Check if the service implementation has the image capture framework available.
 * As IC is available on 10.3+, we will always have this for Neo.
 */
::sal_Bool SAL_CALL MacOSXImageCaptureImpl::hasImageCapture(  ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	return(true);
}

/**
 * Use the image capture framework to perform a capture from a supported device
 * and place it onto the system clipboard.
 */
::sal_Bool SAL_CALL MacOSXImageCaptureImpl::captureImageToClipboard(  ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
	
	ImageCaptureImpl *imp=[ImageCaptureImpl create];

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	unsigned long nCount = Application::ReleaseSolarMutex();
	[imp performSelectorOnMainThread:@selector(doImageCapture:) withObject:imp waitUntilDone:YES modes:pModes];
	Application::AcquireSolarMutex( nCount );
	
	bool toReturn=[imp capturedImage];
	
	[pool release];
	
	return(toReturn);
}

#pragma mark -

@implementation ImageCaptureImpl

+ (id)create
{
	ImageCaptureImpl *pRet = [[ImageCaptureImpl alloc] init];
	[pRet autorelease];
	return pRet;
}

- (id)init
{
	self = [super init];

	gotImage=false;

	return(self);
}

- (void)doImageCapture:(id)pObj
{
	// Do nothing if we are recursing
	if ( gotImage )
		return;

	// Always offload to the Image Capture application as Apple's App Sandbox
	// will prevent our application from accessing a device's files
	NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
	if ( pWorkspace )
		[pWorkspace launchAppWithBundleIdentifier:@"com.apple.Image_Capture" options:NSWorkspaceLaunchDefault additionalEventParamDescriptor:nil launchIdentifier:nil];
}

- (bool)capturedImage
{
	return(gotImage);
}

- (void)setCapturedImage:(bool)bCaptured
{
	gotImage = bCaptured;
}

@end
