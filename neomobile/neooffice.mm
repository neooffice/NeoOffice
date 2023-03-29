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

/*************************************************************************
 *************************************************************************
 *
 * service implementation:	 org.neooffice.NeoOfficeMobile
 * exported interfaces:		 org.neooffice.XNeoOfficeMobile
 *
 *************************************************************************
 *************************************************************************/

#import "neomobile.hxx"
#import "neomobilewebview.h"

#import <com/sun/star/beans/PropertyValue.hpp>
#import <com/sun/star/beans/XPropertySet.hpp>
#import <com/sun/star/beans/XPropertyContainer.hpp>
#import <com/sun/star/document/XDocumentInfoSupplier.hpp>
#import <com/sun/star/frame/XFrame.hpp>
#import <com/sun/star/frame/XStorable.hpp>
#import <com/sun/star/lang/XMultiServiceFactory.hpp>
#import <com/sun/star/lang/XServiceInfo.hpp>
#import <com/sun/star/registry/XRegistryKey.hpp>
#import <com/sun/star/task/XJob.hpp>
#import <comphelper/processfactory.hxx>
#import <cppuhelper/bootstrap.hxx>
#import <cppuhelper/factory.hxx>
#import <cppuhelper/implbase3.hxx>
#import <cppuhelper/implementationentry.hxx>
#import <cppuhelper/queryinterface.hxx>
#import <org/neooffice/XNeoOfficeMobile.hpp>
#import <unotools/bootstrap.hxx>

#import <stdio.h>

#define SERVICENAME "org.neooffice.NeoOfficeMobile"
#define IMPLNAME	"org.neooffice.XNeoOfficeMobile"

static NSString *pUserAgent = nil;

using namespace rtl;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::task;
using namespace com::sun::star::document;
using namespace org::neooffice;
using namespace utl;

//========================================================================

NSArray *NeoMobileGetPerformSelectorOnMainThreadModes()
{
	return [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
}

NSString *NeoMobileGetUserAgent()
{
	if ( !pUserAgent )
	{
		OUString aProductKey = Bootstrap::getProductKey();
		if ( aProductKey.getLength() )
		{
#ifdef MACOSX
#ifdef POWERPC
			aProductKey += OUString( RTL_CONSTASCII_USTRINGPARAM( " (PPC" ) );
#else	// POWERPC
			aProductKey += OUString( RTL_CONSTASCII_USTRINGPARAM( " (Intel" ) );
#endif	// POWERPC
			aProductKey += OUString( RTL_CONSTASCII_USTRINGPARAM( " Mac OS X)" ) );
#endif	// MACOSX
			pUserAgent = [NSString stringWithCharacters:aProductKey.getStr() length:aProductKey.getLength()];
			if ( pUserAgent )
				[pUserAgent retain];
		}
	}

	return pUserAgent;
}

//========================================================================

class SAL_DLLPRIVATE MacOSXNeoOfficeMobileImpl
	: public ::cppu::WeakImplHelper3< XServiceInfo, XJob, XNeoOfficeMobile >
{
	// to obtain other services if needed
	Reference< XComponentContext > m_xServiceManager;
	
public:
	MacOSXNeoOfficeMobileImpl( const Reference< XComponentContext > & xServiceManager )
		: m_xServiceManager( xServiceManager ) {}
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
		SAL_CALL setPropertyValue( const Reference< XFrame >& frame, const rtl::OUString& key, const rtl::OUString& value ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::rtl::OUString
		SAL_CALL getPropertyValue( const Reference< XFrame >& frame, const rtl::OUString& key ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::rtl::OUString
		SAL_CALL getOpenDocumentExtension( const Reference< XFrame >& frame ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::rtl::OUString
		SAL_CALL getOfficeDocumentExtension( const Reference< XFrame >& frame ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::rtl::OUString
		SAL_CALL getMimeType( const Reference< XFrame >& frame ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::rtl::OUString
		SAL_CALL getOfficeMimeType( const Reference< XFrame >& frame ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool
		SAL_CALL isPasswordProtected( const Reference< XFrame >& frame )
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool
		SAL_CALL saveAsPDF( const Reference< XFrame >& frame, const rtl::OUString& url ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool
		SAL_CALL saveAsHTML( const Reference< XFrame >& frame, const rtl::OUString& url ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool
		SAL_CALL saveAsOpenDocument( const Reference< XFrame >& frame, const rtl::OUString& url ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool
		SAL_CALL saveAsOfficeDocument( const Reference< XFrame >& frame, const rtl::OUString& url ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool
		SAL_CALL zipDirectory( const rtl::OUString& dirPath, const rtl::OUString& zipFilePath ) throw (::com::sun::star::uno::RuntimeException);
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
	throw (IllegalArgumentException, Exception)
{
	if(hasNeoOfficeMobile())
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
SAL_DLLPRIVATE Reference< XInterface > SAL_CALL MacOSXNeoOfficeMobileImpl_create(
	const Reference< XComponentContext > & xContext )
{
	return static_cast< XTypeProvider* >(new MacOSXNeoOfficeMobileImpl(xContext));
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

/**
 * Show NeoMobile's "about" webpage
 */
::sal_Bool 
		SAL_CALL MacOSXNeoOfficeMobileImpl::aboutNeoOfficeMobile( ) 
		throw (::com::sun::star::uno::RuntimeException)
{
	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	// Have Mac OS X open the about URL
	NSURL *pURL = [NSURL URLWithString:kNeoMobileAboutURL];
	if ( pURL )
	{
		NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
		if ( pWorkspace )
			[pWorkspace openURL:pURL];
	}

	// Display about webpage in the default web browser
	[NSTask launchedTaskWithLaunchPath:@"/usr/bin/open" arguments:[NSArray arrayWithObjects:kNeoMobileAboutURL, nil]];

	[pool release];
	
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
	
	NeoMobileCreateWebViewImpl *imp=[NeoMobileCreateWebViewImpl createWithURI:@"" userAgent:NeoMobileGetUserAgent()];
	[imp performSelectorOnMainThread:@selector(showWebView:) withObject:imp waitUntilDone:YES modes:NeoMobileGetPerformSelectorOnMainThreadModes()];
	
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
	
	NeoMobileCreateWebViewImpl *imp=[NeoMobileCreateWebViewImpl createWithURI:@"" userAgent:NeoMobileGetUserAgent()];
	[imp performSelectorOnMainThread:@selector(showWebViewOnlyIfVisible:) withObject:imp waitUntilDone:YES modes:NeoMobileGetPerformSelectorOnMainThreadModes()];
	
	[pool release];
	
	return(sal_True);
}

::sal_Bool 
	SAL_CALL MacOSXNeoOfficeMobileImpl::setPropertyValue( const Reference< XFrame >& frame, const rtl::OUString& key, const rtl::OUString& value ) 
	throw (::com::sun::star::uno::RuntimeException)
{
		try
		{
			if (frame.is())
			{
				Reference< XDocumentInfoSupplier > xDocInfoSupplier( frame->getController()->getModel(), UNO_QUERY_THROW );
				
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
	SAL_CALL MacOSXNeoOfficeMobileImpl::getPropertyValue( const Reference< XFrame >& frame, const rtl::OUString& key ) 
	throw (::com::sun::star::uno::RuntimeException)
{
		try
		{
			if (frame.is())
			{
				Reference< XDocumentInfoSupplier > xDocInfoSupplier( frame->getController()->getModel(), UNO_QUERY_THROW );
				
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
	SAL_CALL MacOSXNeoOfficeMobileImpl::getOpenDocumentExtension( const Reference< XFrame >& frame ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	try
	{
		if (!frame.is())
			return(OUString::createFromAscii(""));
		Reference< XModel > rModel=frame->getController()->getModel();

		Reference< XServiceInfo > serviceInfo(rModel, UNO_QUERY_THROW);
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
	SAL_CALL MacOSXNeoOfficeMobileImpl::getOfficeDocumentExtension( const Reference< XFrame >& frame ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	try
	{
		if (!frame.is())
			return(OUString::createFromAscii(""));
		Reference< XModel > rModel=frame->getController()->getModel();

		Reference< XServiceInfo > serviceInfo(rModel, UNO_QUERY_THROW);
		if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.text.TextDocument")))
			return(OUString::createFromAscii(".doc"));
		else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.sheet.SpreadsheetDocument")))
			return(OUString::createFromAscii(".xls"));
		else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.presentation.PresentationDocument")))
			return(OUString::createFromAscii(".ppt"));
		else
			return(OUString::createFromAscii(""));
	}
	catch (...)
	{
	}
	
	return(OUString::createFromAscii(""));
}

::rtl::OUString
	SAL_CALL MacOSXNeoOfficeMobileImpl::getMimeType( const Reference< XFrame >& frame ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	try
	{
		if (!frame.is())
			return(OUString::createFromAscii(""));
		Reference< XModel > rModel=frame->getController()->getModel();
					
		Reference< XServiceInfo > serviceInfo(rModel, UNO_QUERY_THROW);
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

::rtl::OUString
	SAL_CALL MacOSXNeoOfficeMobileImpl::getOfficeMimeType( const Reference< XFrame >& frame ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	try
	{
		if (!frame.is())
			return(OUString::createFromAscii(""));
		Reference< XModel > rModel=frame->getController()->getModel();
					
		Reference< XServiceInfo > serviceInfo(rModel, UNO_QUERY_THROW);
		if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.text.TextDocument")))
			return(OUString::createFromAscii("application/msword"));
		else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.sheet.SpreadsheetDocument")))
			return(OUString::createFromAscii("application/msexcel"));
		else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.presentation.PresentationDocument")))
			return(OUString::createFromAscii("application/mspowerpoint"));
		else
			return(OUString::createFromAscii(""));
	}
	catch (...)
	{
	}
	
	return(OUString::createFromAscii(""));
}

::sal_Bool
	SAL_CALL MacOSXNeoOfficeMobileImpl::isPasswordProtected( const Reference< XFrame >& frame )
	throw (::com::sun::star::uno::RuntimeException)
{
	try
	{
		if (!frame.is())
			return(sal_False);
		Reference< XModel > rModel=frame->getController()->getModel();
		
		// Determine if this document is password protected
		const OUString aPasswordPropName( RTL_CONSTASCII_USTRINGPARAM( "Password" ) );
		Sequence< PropertyValue > aArgs = rModel->getArgs();
		sal_uInt32 nLen = aArgs.getLength();
		for ( sal_uInt32 i = 0; i < nLen; i++ )
		{
			if ( aArgs[ i ].Name == aPasswordPropName )
				return (sal_True);
		}
	}
	catch (...)
	{
	}

	return(sal_False);
}

::sal_Bool
	SAL_CALL MacOSXNeoOfficeMobileImpl::saveAsPDF( const Reference< XFrame >& frame, const rtl::OUString& url ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	try
	{
		if (!frame.is())
			return(sal_False);
		Reference< XModel > rModel=frame->getController()->getModel();
		
		Sequence< PropertyValue > lProperties(2);
		
		lProperties[0].Name=OUString::createFromAscii("FilterName");
		
		Reference< XServiceInfo > serviceInfo(rModel, UNO_QUERY_THROW);
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
		
		Reference< XStorable > xStore(rModel, UNO_QUERY_THROW);
		xStore->storeToURL(url, lProperties);
		
		return(sal_True);
	}
	catch (...)
	{
	}

	return(sal_False);
}

::sal_Bool
	SAL_CALL MacOSXNeoOfficeMobileImpl::saveAsHTML( const Reference< XFrame >& frame, const rtl::OUString& url ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	try
	{
		if (!frame.is())
			return(sal_False);
		Reference< XModel > rModel=frame->getController()->getModel();
		
		Sequence< PropertyValue > lProperties(2);
		
		lProperties[0].Name=OUString::createFromAscii("FilterName");
		
		Reference< XServiceInfo > serviceInfo(rModel, UNO_QUERY_THROW);
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
		
		Reference< XStorable > xStore(rModel, UNO_QUERY_THROW);
		xStore->storeToURL(url, lProperties);
		
		return(sal_True);
	}
	catch (...)
	{
	}

	return(sal_False);
}

::sal_Bool
	SAL_CALL MacOSXNeoOfficeMobileImpl::saveAsOpenDocument( const Reference< XFrame >& frame, const rtl::OUString& url ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	try
	{
		if (!frame.is())
			return(sal_False);
		Reference< XModel > rModel=frame->getController()->getModel();
		
		Sequence< PropertyValue > lProperties(2);
		
		lProperties[0].Name=OUString::createFromAscii("FilterName");
		
		Reference< XServiceInfo > serviceInfo(rModel, UNO_QUERY_THROW);
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
		
		Reference< XStorable > xStore(rModel, UNO_QUERY_THROW);
		xStore->storeToURL(url, lProperties);
		
		return(sal_True);
	}
	catch (...)
	{
	}

	return(sal_False);
}

::sal_Bool
	SAL_CALL MacOSXNeoOfficeMobileImpl::saveAsOfficeDocument( const Reference< XFrame >& frame, const rtl::OUString& url ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	try
	{
		if (!frame.is())
			return(sal_False);
		Reference< XModel > rModel=frame->getController()->getModel();
		
		Sequence< PropertyValue > lProperties(2);
		
		lProperties[0].Name=OUString::createFromAscii("FilterName");
		
		Reference< XServiceInfo > serviceInfo(rModel, UNO_QUERY_THROW);
		if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.text.TextDocument")))
			lProperties[0].Value <<= OUString::createFromAscii("MS Word 97");
		else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.sheet.SpreadsheetDocument")))
			lProperties[0].Value <<= OUString::createFromAscii("MS Excel 97");
		else if(serviceInfo->supportsService(OUString::createFromAscii("com.sun.star.presentation.PresentationDocument")))
			lProperties[0].Value <<= OUString::createFromAscii("MS PowerPoint 97");
		else
			return(sal_False);
		
		lProperties[1].Name=OUString::createFromAscii("Overwrite");
		lProperties[1].Value <<= sal_True;
		
		Reference< XStorable > xStore(rModel, UNO_QUERY_THROW);
		xStore->storeToURL(url, lProperties);
		
		return(sal_True);
	}
	catch (...)
	{
	}

	return(sal_False);
}

/**
 * Check if the we have full WebView support available
 */
::sal_Bool 
		SAL_CALL MacOSXNeoOfficeMobileImpl::hasNeoOfficeMobile( ) 
		throw (::com::sun::star::uno::RuntimeException)
{
	return(sal_True);
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
	return(NeoMobileZipDirectory(dirPath, zipFilePath));
}
