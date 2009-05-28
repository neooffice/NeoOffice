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
 *		 - GNU General Public License Version 2.1
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2008 by Planamesa Inc.
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
 * service implementation:	 org.neooffice.NeoOfficeMobile
 * exported interfaces:		 org.neooffice.XNeoOfficeMobile
 *
 *************************************************************************
 *************************************************************************/

#include <stdio.h>

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
#include <org/neooffice/XNeoOfficeMobile.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#include <com/sun/star/lang/Locale.hpp>

#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/task/XJob.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include "premac.h"
#import <Carbon/Carbon.h>
#include "postmac.h"

#include "neomobilewebview.h"
#include <unistd.h>

#define SERVICENAME "org.neooffice.NeoOfficeMobile"
#define IMPLNAME	"org.neooffice.XNeoOfficeMobile"

static const NSString *pAboutURI = @"/mobile/";
static const NSString *pOpenURI = @"/";

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::org::neooffice;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::document;

//========================================================================

OUString NSStringToOUString( NSString *pString )
{
	if ( !pString )
		return OUString();

	unsigned int nLen = [pString length];
	if ( !nLen )
		return OUString();

	sal_Unicode aBuf[ nLen + 1 ];
	[pString getCharacters:aBuf];
	aBuf[ nLen ] = 0;

	return OUString( aBuf );
}

//========================================================================

class MacOSXNeoOfficeMobileImpl
	: public ::cppu::WeakImplHelper3< XServiceInfo, XJob, XNeoOfficeMobile >
{
	// to obtain other services if needed
	Reference< XComponentContext > m_xServiceManager;
	
	sal_Int32 m_nRefCount;
	sal_Int32 m_nCount;
	OUString m_aLocale;
	
public:
	MacOSXNeoOfficeMobileImpl( const Reference< XComponentContext > & xServiceManager )
		: m_xServiceManager( xServiceManager ), m_nRefCount( 0 ) {}
	virtual ~MacOSXNeoOfficeMobileImpl() {}

    // XServiceInfo	implementation
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

    // XJob implementation
    virtual Any SAL_CALL execute( const Sequence< NamedValue >& rNamedValues ) throw (IllegalArgumentException, Exception);

	// XNeoOfficeMobile implementation
	virtual ::sal_Bool 
		SAL_CALL aboutNeoOfficeMobile( ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool 
		SAL_CALL hasNeoOfficeMobile( ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool 
		SAL_CALL openNeoOfficeMobile( ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool 
		SAL_CALL openNeoOfficeMobileOnlyIfVisible( ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool 
		SAL_CALL setPropertyValue( const rtl::OUString& key, const rtl::OUString& value ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::rtl::OUString
		SAL_CALL getPropertyValue( const rtl::OUString& key ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::rtl::OUString
		SAL_CALL getOpenDocumentExtension( ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::rtl::OUString
		SAL_CALL getMimeType( ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool
		SAL_CALL saveAsPDF( const rtl::OUString& url ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool
		SAL_CALL saveAsHTML( const rtl::OUString& url ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool
		SAL_CALL saveAsOpenDocument( const rtl::OUString& url ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool
		SAL_CALL zipDirectory( const rtl::OUString& dirPath, const rtl::OUString& zipFilePath ) 
		throw (::com::sun::star::uno::RuntimeException);
};

//*************************************************************************
OUString SAL_CALL MacOSXNeoOfficeMobileImpl::getImplementationName(  ) 
	throw(RuntimeException)
{
	return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}	

//*************************************************************************
sal_Bool SAL_CALL MacOSXNeoOfficeMobileImpl::supportsService( const OUString& ServiceName ) 
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
Sequence< OUString > SAL_CALL MacOSXNeoOfficeMobileImpl::getSupportedServiceNames(  ) 
	throw(RuntimeException)
{
	return getSupportedServiceNames_Static();
}	

//*************************************************************************
Any SAL_CALL MacOSXNeoOfficeMobileImpl::execute( const Sequence< NamedValue >& rNamedVAlues )
{
	openNeoOfficeMobileOnlyIfVisible();

	return Any();
}

//*************************************************************************
Sequence< OUString > SAL_CALL MacOSXNeoOfficeMobileImpl::getSupportedServiceNames_Static(  ) 
{
	OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
	return Sequence< OUString >( &aName, 1 );
}	




/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
Reference< XInterface > SAL_CALL MacOSXNeoOfficeMobileImpl_create(
	const Reference< XComponentContext > & xContext )
{
	Reference< XTypeProvider > xRet = static_cast< XTypeProvider* >(new MacOSXNeoOfficeMobileImpl(xContext));
	return xRet;
}


//#########################################################################
//#### EXPORTED ###########################################################
//#########################################################################

		/* shared lib exports implemented without helpers in service_impl1.cxx */
		namespace neo_macosxneomobile_impl
		{
		
	static Sequence< OUString > getSupportedServiceNames_MacOSXNeoOfficeMobileImpl()
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
		 
		static OUString getImplementationName_MacOSXNeoOfficeMobileImpl()
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
		        MacOSXNeoOfficeMobileImpl_create, getImplementationName_MacOSXNeoOfficeMobileImpl,
		        getSupportedServiceNames_MacOSXNeoOfficeMobileImpl, ::cppu::createSingleComponentFactory,
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
	return ::cppu::component_writeInfoHelper(pServiceManager, pRegistryKey, ::neo_macosxneomobile_impl::s_component_entries);
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
	return ::cppu::component_getFactoryHelper(pImplName, pServiceManager, pRegistryKey, ::neo_macosxneomobile_impl::s_component_entries);
}

#pragma mark -

static NeoMobileWebView *pSharedWebView = nil;

@interface CreateWebViewImpl : NSObject
{
	const NSString*				mpURI;
}
+ (id)createWithURI:(const NSString *)pURI;
- (id)initWithURI:(const NSString *)pURI;
- (void)showWebView:(id)obj;
- (void)showWebViewOnlyIfVisible:(id)obj;
@end

@implementation CreateWebViewImpl

+ (id)createWithURI:(const NSString *)pURI
{
	CreateWebViewImpl *pRet = [[CreateWebViewImpl alloc] initWithURI:pURI];
	[pRet autorelease];
	return pRet;
}

- (id)initWithURI:(const NSString *)pURI
{
	self = [super init];

	mpURI = pURI;

	return(self);
}

- (void)showWebView:(id)obj
{
	if ( !pSharedWebView )
	{
		pSharedWebView = [[NeoMobileWebView alloc] initWithFrame:NSMakeRect( 0, 0, 500, 500 ) frameName:nil groupName:nil];
		
		// check for retained user position.  If not available, make relative to the
		// primary frame.
		
		NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
		
		NSPoint windowPos={0, 0};
		
		NSString *xPosStr=[defaults stringForKey:@"nmXPos"];
		NSString *yPosStr=[defaults stringForKey:@"nmYPos"];
		if(xPosStr && yPosStr)
		{
			windowPos.x=[xPosStr intValue];
			windowPos.y=[yPosStr intValue];
		}
		else
		{
			NSWindow *keyWindow=[NSApp mainWindow];
			if(keyWindow)
			{
				windowPos=[keyWindow frame].origin;
				windowPos.x+=75;
				windowPos.y+=75;
			}
		}
		
		if([pSharedWebView window])
			[[pSharedWebView window] setFrameOrigin:windowPos];
	}
	
	if ( pSharedWebView )
	{
		NSWindow *pWindow = [pSharedWebView window];
		if ( pWindow )
		{
			// Make sure window is visible
			if ( ![pWindow isVisible] )
			{
				[pWindow orderFront:self];

				NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
				[defaults setBool:YES forKey:@"nmVisible"];
				[defaults synchronize];
			}

			// Load URI
			[pSharedWebView loadURI:mpURI];
		}
	}
}

- (void)showWebViewOnlyIfVisible:(id)obj
{
	NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
	if ( [defaults boolForKey:@"nmVisible"] )
		[self showWebView:obj];
}
@end

/**
 * Show NeoMobile's "about" webpage
 */
::sal_Bool 
		SAL_CALL MacOSXNeoOfficeMobileImpl::aboutNeoOfficeMobile( ) 
		throw (::com::sun::star::uno::RuntimeException)
{
	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	CreateWebViewImpl *imp=[CreateWebViewImpl createWithURI:pAboutURI];

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	unsigned long nCount = Application::ReleaseSolarMutex();
	[imp performSelectorOnMainThread:@selector(showWebView:) withObject:imp waitUntilDone:YES modes:pModes];
	Application::AcquireSolarMutex( nCount );
		
	[pool release];
	
	return(sal_True);
}

/**
 * Check if the we have full WebView support available
 */
::sal_Bool 
		SAL_CALL MacOSXNeoOfficeMobileImpl::hasNeoOfficeMobile( ) 
		throw (::com::sun::star::uno::RuntimeException)
{
	// we currently need to be running on 10.4 in order to have full WebView
	// support.  Check using our gestalt.
	
	long res=0;
	if(Gestalt(gestaltSystemVersion, &res)==noErr)
	{
		bool isTigerOrHigher = ( ( ( ( res >> 8 ) & 0x00FF ) == 0x10 ) && ( ( ( res >> 4 ) & 0x000F ) >= 0x4 ) );
		if(!isTigerOrHigher)
			return(sal_False);
	}
	
	return(sal_True);
}

/**
 * Show main NeoMobile webpage
 */
::sal_Bool 
		SAL_CALL MacOSXNeoOfficeMobileImpl::openNeoOfficeMobile( ) 
		throw (::com::sun::star::uno::RuntimeException)
{
	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
	
	CreateWebViewImpl *imp=[CreateWebViewImpl createWithURI:pOpenURI];

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	unsigned long nCount = Application::ReleaseSolarMutex();
	[imp performSelectorOnMainThread:@selector(showWebView:) withObject:imp waitUntilDone:YES modes:pModes];
	Application::AcquireSolarMutex( nCount );
		
	[pool release];
	
	return(sal_True);
}

/**
 * Show main NeoMobile webpage only if the nmVisible CFPreference is set
 */
::sal_Bool 
		SAL_CALL MacOSXNeoOfficeMobileImpl::openNeoOfficeMobileOnlyIfVisible( ) 
		throw (::com::sun::star::uno::RuntimeException)
{
	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
	
	CreateWebViewImpl *imp=[CreateWebViewImpl createWithURI:pOpenURI];

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	unsigned long nCount = Application::ReleaseSolarMutex();
	[imp performSelectorOnMainThread:@selector(showWebViewOnlyIfVisible:) withObject:imp waitUntilDone:YES modes:pModes];
	Application::AcquireSolarMutex( nCount );
		
	[pool release];
	
	return(sal_True);
}

::sal_Bool 
	SAL_CALL MacOSXNeoOfficeMobileImpl::setPropertyValue( const rtl::OUString& key, const rtl::OUString& value ) 
	throw (::com::sun::star::uno::RuntimeException)
{
		try
		{
			Reference< XDesktop > rDesktop(::comphelper::getProcessServiceFactory()->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"))), UNO_QUERY_THROW);
			
			Reference< XDispatchHelper > rDispatchHelper(::comphelper::getProcessServiceFactory()->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.DispatchHelper"))), UNO_QUERY_THROW); 
			
			Reference< XFrame > rFrame=rDesktop->getCurrentFrame(); 
			if (rFrame.is())
			{
				Reference< XDocumentInfoSupplier > xDocInfoSupplier( rFrame->getController()->getModel(), UNO_QUERY_THROW );
				
				Reference< XDocumentInfo > docInfo = xDocInfoSupplier->getDocumentInfo();
				if (docInfo.is())
				{
					try
					{
						Reference< XPropertyContainer > propContainer( docInfo, UNO_QUERY_THROW );
						propContainer->addProperty(key, 0, Any(value));
						return(sal_True);
					}
					catch (PropertyExistException e)
					{
						Reference< XPropertySet > bagOfMe( docInfo, UNO_QUERY );
						if (bagOfMe.is())
							bagOfMe->setPropertyValue(key, Any(value));
					}
					
					return(sal_True);
				}
			}
		}
		catch (...)
		{
		}
		
		return(sal_False);
}

::rtl::OUString
	SAL_CALL MacOSXNeoOfficeMobileImpl::getPropertyValue( const rtl::OUString& key ) 
	throw (::com::sun::star::uno::RuntimeException)
{
		try
		{
			Reference< XDesktop > rDesktop(::comphelper::getProcessServiceFactory()->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"))), UNO_QUERY_THROW);

			Reference< XDispatchHelper > rDispatchHelper(::comphelper::getProcessServiceFactory()->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.DispatchHelper"))), UNO_QUERY_THROW ); 
			
			Reference< XFrame > rFrame=rDesktop->getCurrentFrame(); 
			if (rFrame.is())
			{
				Reference< XDocumentInfoSupplier > xDocInfoSupplier( rFrame->getController()->getModel(), UNO_QUERY_THROW );
				
				Reference< XDocumentInfo > docInfo = xDocInfoSupplier->getDocumentInfo();
				Reference< XPropertySet > bagOfMe( docInfo, UNO_QUERY_THROW );
				
				rtl::OUString res;
				bagOfMe->getPropertyValue(key) >>= res;
				return(res);
			}
		}
		catch (...)
		{
		}
		
	return(OUString::createFromAscii(""));
}

::rtl::OUString
	SAL_CALL MacOSXNeoOfficeMobileImpl::getOpenDocumentExtension( void ) 
	throw (::com::sun::star::uno::RuntimeException)
{
		try
		{
			Reference< XComponentContext > component( comphelper_getProcessComponentContext() );
			Reference< XMultiComponentFactory > rServiceManager = component->getServiceManager();
			Reference< XInterface > rDesktop = rServiceManager->createInstanceWithContext(OUString::createFromAscii("com.sun.star.frame.Desktop"), component);
			
			Reference< XDispatchHelper > rDispatchHelper = Reference< XDispatchHelper >(rServiceManager->createInstanceWithContext(OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.DispatchHelper" )), component), UNO_QUERY ); 
			
			Reference< XDesktop > Desktop(rDesktop,UNO_QUERY);
			Reference< XFrame > rFrame=Desktop->getCurrentFrame();
			Reference< XModel > rModel=rFrame->getController()->getModel();
						
			Reference< XServiceInfo > serviceInfo(rModel, UNO_QUERY);
			if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.text.TextDocument")))
				return(OUString::createFromAscii(".odt"));
			else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.sheet.SpreadsheetDocument")))
				return(OUString::createFromAscii(".ods"));
			else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.drawing.DrawingDocument")))
				return(OUString::createFromAscii(".odg"));
			else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.presentation.PresentationDocument")))
				return(OUString::createFromAscii(".odp"));
			else
				return(OUString::createFromAscii(""));
		}
		catch (...)
		{
		}
		
	return(OUString::createFromAscii(""));
}

::rtl::OUString
	SAL_CALL MacOSXNeoOfficeMobileImpl::getMimeType( void ) 
	throw (::com::sun::star::uno::RuntimeException)
{
		try
		{
			Reference< XComponentContext > component( comphelper_getProcessComponentContext() );
			Reference< XMultiComponentFactory > rServiceManager = component->getServiceManager();
			Reference< XInterface > rDesktop = rServiceManager->createInstanceWithContext(OUString::createFromAscii("com.sun.star.frame.Desktop"), component);
			
			Reference< XDispatchHelper > rDispatchHelper = Reference< XDispatchHelper >(rServiceManager->createInstanceWithContext(OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.DispatchHelper" )), component), UNO_QUERY ); 
			
			Reference< XDesktop > Desktop(rDesktop,UNO_QUERY);
			Reference< XFrame > rFrame=Desktop->getCurrentFrame();
			Reference< XModel > rModel=rFrame->getController()->getModel();
						
			Reference< XServiceInfo > serviceInfo(rModel, UNO_QUERY);
						if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.text.TextDocument")))
				return(OUString::createFromAscii("application/vnd.oasis.opendocument.text"));
			else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.sheet.SpreadsheetDocument")))
				return(OUString::createFromAscii("application/vnd.oasis.opendocument.spreadsheet"));
			else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.drawing.DrawingDocument")))
				return(OUString::createFromAscii("application/vnd.oasis.opendocument.graphics"));
			else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.presentation.PresentationDocument")))
				return(OUString::createFromAscii("application/vnd.oasis.opendocument.presentation"));
			else
				return(OUString::createFromAscii(""));
		}
		catch (...)
		{
		}
		
	return(OUString::createFromAscii(""));
}


::sal_Bool
	SAL_CALL MacOSXNeoOfficeMobileImpl::saveAsPDF( const rtl::OUString& url ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	try
	{
		Reference< XComponentContext > component( comphelper_getProcessComponentContext() );
		Reference< XMultiComponentFactory > rServiceManager = component->getServiceManager();
		Reference< XInterface > rDesktop = rServiceManager->createInstanceWithContext(OUString::createFromAscii("com.sun.star.frame.Desktop"), component);
		
		Reference< XDispatchHelper > rDispatchHelper = Reference< XDispatchHelper >(rServiceManager->createInstanceWithContext(OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.DispatchHelper" )), component), UNO_QUERY ); 
		
		Reference< XDesktop > Desktop(rDesktop,UNO_QUERY);
		Reference< XFrame > rFrame=Desktop->getCurrentFrame();
		Reference< XModel > rModel=rFrame->getController()->getModel();
		
		Sequence< PropertyValue > lProperties(2);
		
		lProperties[0].Name=OUString::createFromAscii("FilterName");
		
		Reference< XServiceInfo > serviceInfo(rModel, UNO_QUERY);
		if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.text.TextDocument")))
			lProperties[0].Value <<= OUString::createFromAscii("writer_pdf_Export");
		else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.sheet.SpreadsheetDocument")))
			lProperties[0].Value <<= OUString::createFromAscii("calc_pdf_Export");
		else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.drawing.DrawingDocument")))
			lProperties[0].Value <<= OUString::createFromAscii("draw_pdf_Export");
		else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.presentation.PresentationDocument")))
			lProperties[0].Value <<= OUString::createFromAscii("impress_pdf_Export");
		else
			return(sal_False);
		
		lProperties[1].Name=OUString::createFromAscii("Overwrite");
		lProperties[1].Value <<= sal_True;
		
		Reference< XStorable > xStore(rModel, UNO_QUERY);
		xStore->storeToURL(url, lProperties);
		
		return(sal_True);
	}
	catch (...)
	{
	}

	return(sal_False);
}

::sal_Bool
	SAL_CALL MacOSXNeoOfficeMobileImpl::saveAsHTML( const rtl::OUString& url ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	try
	{
		Reference< XComponentContext > component( comphelper_getProcessComponentContext() );
		Reference< XMultiComponentFactory > rServiceManager = component->getServiceManager();
		Reference< XInterface > rDesktop = rServiceManager->createInstanceWithContext(OUString::createFromAscii("com.sun.star.frame.Desktop"), component);
		
		Reference< XDispatchHelper > rDispatchHelper = Reference< XDispatchHelper >(rServiceManager->createInstanceWithContext(OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.DispatchHelper" )), component), UNO_QUERY ); 
		
		Reference< XDesktop > Desktop(rDesktop,UNO_QUERY);
		Reference< XFrame > rFrame=Desktop->getCurrentFrame();
		Reference< XModel > rModel=rFrame->getController()->getModel();
		
		Sequence< PropertyValue > lProperties(2);
		
		lProperties[0].Name=OUString::createFromAscii("FilterName");
		
		Reference< XServiceInfo > serviceInfo(rModel, UNO_QUERY);
		if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.text.TextDocument")))
			lProperties[0].Value <<= OUString::createFromAscii("HTML (StarWriter)");
		else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.sheet.SpreadsheetDocument")))
			lProperties[0].Value <<= OUString::createFromAscii("HTML (StarCalc)");
		else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.drawing.DrawingDocument")))
			lProperties[0].Value <<= OUString::createFromAscii("draw_html_Export");
		else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.presentation.PresentationDocument")))
			lProperties[0].Value <<= OUString::createFromAscii("impress_html_Export");
		else
			return(sal_False);
		
		lProperties[1].Name=OUString::createFromAscii("Overwrite");
		lProperties[1].Value <<= sal_True;
		
		Reference< XStorable > xStore(rModel, UNO_QUERY);
		xStore->storeToURL(url, lProperties);
		
		return(sal_True);
	}
	catch (...)
	{
	}

	return(sal_False);
}

::sal_Bool
	SAL_CALL MacOSXNeoOfficeMobileImpl::saveAsOpenDocument( const rtl::OUString& url ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	try
	{
		Reference< XComponentContext > component( comphelper_getProcessComponentContext() );
		Reference< XMultiComponentFactory > rServiceManager = component->getServiceManager();
		Reference< XInterface > rDesktop = rServiceManager->createInstanceWithContext(OUString::createFromAscii("com.sun.star.frame.Desktop"), component);
		
		Reference< XDispatchHelper > rDispatchHelper = Reference< XDispatchHelper >(rServiceManager->createInstanceWithContext(OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.DispatchHelper" )), component), UNO_QUERY ); 
		
		Reference< XDesktop > Desktop(rDesktop,UNO_QUERY);
		Reference< XFrame > rFrame=Desktop->getCurrentFrame();
		Reference< XModel > rModel=rFrame->getController()->getModel();
		
		Sequence< PropertyValue > lProperties(2);
		
		lProperties[0].Name=OUString::createFromAscii("FilterName");
		
		Reference< XServiceInfo > serviceInfo(rModel, UNO_QUERY);
		if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.text.TextDocument")))
			lProperties[0].Value <<= OUString::createFromAscii("writer8");
		else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.sheet.SpreadsheetDocument")))
			lProperties[0].Value <<= OUString::createFromAscii("calc8");
		else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.drawing.DrawingDocument")))
			lProperties[0].Value <<= OUString::createFromAscii("draw8");
		else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.presentation.PresentationDocument")))
			lProperties[0].Value <<= OUString::createFromAscii("impress8");
		else
			return(sal_False);
		
		lProperties[1].Name=OUString::createFromAscii("Overwrite");
		lProperties[1].Value <<= sal_True;
		
		Reference< XStorable > xStore(rModel, UNO_QUERY);
		xStore->storeToURL(url, lProperties);
		
		return(sal_True);
	}
	catch (...)
	{
	}

	return(sal_False);
}

/**
 * Zip the contents of a directory into new selfcontained zip file.
 *
 * @param dirPath	absolute path to the directory whose contents should be
 *					compressed.  Trailing path separator is optional.
 * @param zipFilePath	absolute path to the output ZIP file.  This should
 *						include the ".zip" suffix.  If not present, the
 *						suffix will be added.
 * @return sal_True if the zip operation succeeded, sal_False on error.
 */
::sal_Bool
	SAL_CALL MacOSXNeoOfficeMobileImpl::zipDirectory( const rtl::OUString& dirPath, const rtl::OUString& zipFilePath ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	try
	{
		OString asciiDirPath = OUStringToOString(dirPath,RTL_TEXTENCODING_UTF8);
		if (!asciiDirPath.getLength())
			return(sal_False);
		
		OString asciiZipFilePath = OUStringToOString(zipFilePath,RTL_TEXTENCODING_UTF8);
		if (!asciiZipFilePath.getLength())
			return(sal_False);
			
		char oldWD[2048];
		
		getcwd(oldWD, sizeof(oldWD));
		chdir(asciiDirPath.getStr());
		OString zipCmd("/usr/bin/zip \"");
		zipCmd+=asciiZipFilePath;
		zipCmd+=OString("\" *");
		short outVal=system(zipCmd.getStr());
		chdir(oldWD);
		return((outVal==0));
	}
	catch (...)
	{
	}
	
	return(sal_False);
}
