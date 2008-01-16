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
 * service implementation:	 org.neooffice.GrammarChecker
 * exported interfaces:		 org.neooffice.XGrammarChecker
 *
 * simple glue mapping 10.5 grammar check structures onto UNO types
 * to export them within the OOo scripting environment
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
#ifndef _ORG_NEOOFFICE_GRAMMARREPLACEMENT_HPP_
#include <org/neooffice/GrammarReplacement.hpp>
#endif
#ifndef _ORG_NEOOFFICE_XGRAMMARCHECKER_HPP_
#include <org/neooffice/XGrammarChecker.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#include <com/sun/star/lang/Locale.hpp>

#include "premac.h"
#import <AppKit/NSSpellChecker.h>
#import <AppKit/NSAlert.h>
#import <Foundation/Foundation.h>
#import <Carbon/Carbon.h>
#include "postmac.h"

#include <string>
#include <strstream>

#if BUILD_OS_MINOR < 5
// building on 10.4.  Define required constants for the grammar check system
// manually.

#define NSGrammarRange	@"NSGrammarRange"
#define NSGrammarCorrections @"NSGrammarCorrections"
#define NSGrammarUserDescription @"NSGrammarUserDescription"

// add the signature for the grammar checking method via a category
@interface NSSpellChecker (LeopardSpellCheckMethods)
- (NSRange)checkGrammarOfString: (NSString *)string startingAt:(unsigned long)start language:(NSString*)language wrap:(signed char)wrap inSpellDocumentWithTag:(unsigned long)documentTag details:(NSArray **)outDetails;
@end

#endif

#define SERVICENAME "org.neooffice.GrammarChecker"
#define IMPLNAME	"org.neooffice.XGrammarChecker"

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::org::neooffice;

//========================================================================
class MacOSXGrammarCheckerImpl
	: public ::cppu::WeakImplHelper2<XServiceInfo, XGrammarChecker>
{
	// to obtain other services if needed
	Reference< XComponentContext > m_xServiceManager;
	
	sal_Int32 m_nRefCount;
	sal_Int32 m_nCount;
	Locale m_aLocale;
	
public:
	MacOSXGrammarCheckerImpl( const Reference< XComponentContext > & xServiceManager )
		: m_xServiceManager( xServiceManager ), m_nRefCount( 0 )
		{ printf( "< MacOSXGrammarCheckerImpl ctor called >\n" ); }
	~MacOSXGrammarCheckerImpl()
		{ printf( "< MacOSXGrammarCheckerImpl dtor called >\n" ); }

    // XServiceInfo	implementation
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

	// XGrammarChecker implementation
	virtual com::sun::star::uno::Sequence<org::neooffice::GrammarReplacement> 
		SAL_CALL checkString(const rtl::OUString&)
   		throw (com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool 
		SAL_CALL setLocale( const ::com::sun::star::lang::Locale& aLocale ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool 
		SAL_CALL hasGrammarChecker( ) 
		throw (::com::sun::star::uno::RuntimeException);
};

//*************************************************************************
OUString SAL_CALL MacOSXGrammarCheckerImpl::getImplementationName(  ) 
	throw(RuntimeException)
{
	return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}	

//*************************************************************************
sal_Bool SAL_CALL MacOSXGrammarCheckerImpl::supportsService( const OUString& ServiceName ) 
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
Sequence<OUString> SAL_CALL MacOSXGrammarCheckerImpl::getSupportedServiceNames(  ) 
	throw(RuntimeException)
{
	return getSupportedServiceNames_Static();
}	

//*************************************************************************
Sequence<OUString> SAL_CALL MacOSXGrammarCheckerImpl::getSupportedServiceNames_Static(  ) 
{
	OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
	return Sequence< OUString >( &aName, 1 );
}	




/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
Reference< XInterface > SAL_CALL MacOSXGrammarCheckerImpl_create(
	const Reference< XComponentContext > & xContext )
{
	return static_cast<XTypeProvider *>(new MacOSXGrammarCheckerImpl(xContext));
}


//#########################################################################
//#### EXPORTED ###########################################################
//#########################################################################

		/* shared lib exports implemented without helpers in service_impl1.cxx */
		namespace neo_macosxgrammarchecker_impl
		{
		
	static Sequence< OUString > getSupportedServiceNames_MacOSXGrammarCheckerImpl()
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
		 
		static OUString getImplementationName_MacOSXGrammarCheckerImpl()
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
		        MacOSXGrammarCheckerImpl_create, getImplementationName_MacOSXGrammarCheckerImpl,
		        getSupportedServiceNames_MacOSXGrammarCheckerImpl, ::cppu::createSingleComponentFactory,
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
	return ::cppu::component_writeInfoHelper(pServiceManager, pRegistryKey, ::neo_macosxgrammarchecker_impl::s_component_entries);
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
	return ::cppu::component_getFactoryHelper(pImplName, pServiceManager, pRegistryKey, ::neo_macosxgrammarchecker_impl::s_component_entries);
}

#pragma mark -

com::sun::star::uno::Sequence<org::neooffice::GrammarReplacement> 
   SAL_CALL MacOSXGrammarCheckerImpl::checkString(const rtl::OUString& toCheck)
   throw (com::sun::star::uno::RuntimeException)
{
	NSAutoreleasePool *localPool=[[NSAutoreleasePool alloc] init];
	
	NSString *stringToCheck=[NSString stringWithCharacters: toCheck.getStr() length: toCheck.getLength()];

	// Do not ever invoke [NSApplication sharedApplication] because ifNSApp is
	// not already initialized, we are either running in a separate subprocess
	// of the application (which means this was not installed as a shared
	// extension) or we are running in an X11 application and we are not
	// xhosting to the localhost (which will cause the application to abort)
	NSSpellChecker *spelling=[NSSpellChecker sharedSpellChecker];
	
	Sequence< org::neooffice::GrammarReplacement > toReturn;
	
	if(!spelling)
		return(toReturn);
	
	NSArray *detailArray = nil;
	
	unsigned long tempDocTag=[NSSpellChecker uniqueSpellDocumentTag];
	// +++ map m_aLocale into apprporiate locale code for grammar checking of string
	[spelling checkGrammarOfString: stringToCheck startingAt: 0 language: @"en" wrap: 0L inSpellDocumentWithTag: tempDocTag details: &detailArray];
	[spelling closeSpellDocumentWithTag: tempDocTag];
	
	if(detailArray && [detailArray count])
	{
		toReturn=Sequence< org::neooffice::GrammarReplacement >([detailArray count]);
		
		for(int i=0; i<[detailArray count]; i++)
		{
			NSDictionary *dic=(NSDictionary *)[detailArray objectAtIndex: i];
			
			{
				NSString *userDescription=(NSString *)[dic valueForKey: NSGrammarUserDescription];
				
				sal_Unicode pBuffer[ [userDescription length] ];
				[userDescription getCharacters: pBuffer];
				
				toReturn[i].aDescription=OUString(pBuffer, [userDescription length]);
			}
			
			NSValue *rangeToCorrect=(NSValue *)[dic valueForKey: NSGrammarRange];
			toReturn[i].lStartIndex=[rangeToCorrect rangeValue].location;
			toReturn[i].lLength=[rangeToCorrect rangeValue].length;
			
			NSArray *corrections=(NSArray *)[dic valueForKey: NSGrammarCorrections];
			
			toReturn[i].aSuggestedReplacements=Sequence< OUString >([corrections count]);
			for(int j=0; j<[corrections count]; j++)
			{
				NSString *corrString=(NSString *)[corrections objectAtIndex: j];
				
				sal_Unicode pBuffer[ [corrString length] ];
				[corrString getCharacters: pBuffer];
				
				toReturn[i].aSuggestedReplacements[j]=OUString(pBuffer, [corrString length]);
			}
		}
	}

	[localPool release];
	
	return(toReturn);
}

/**
 * Set the locale used for performing the grammar checking.  Returns true if the locale has
 * a supported Mac OS X grammar checker, false if not.
 */
::sal_Bool 
		SAL_CALL MacOSXGrammarCheckerImpl::setLocale( const ::com::sun::star::lang::Locale& aLocale ) 
		throw (::com::sun::star::uno::RuntimeException)
{
	m_aLocale=aLocale;
	// +++ check if locale is supported;  presently interface only returns if spellchecking is supported,
	// not grammar check
	return(true);
}

/**
 * Check if the underlying operating system is 10.5 or higher
 */
::sal_Bool 
		SAL_CALL MacOSXGrammarCheckerImpl::hasGrammarChecker( ) 
		throw (::com::sun::star::uno::RuntimeException)
{
	// we currently need to be running on 10.5 in order to have a grammar
	// checker.  Check using our gestalt
	
	long res=0;
	if(Gestalt(gestaltSystemVersion, &res)==noErr)
	{
		bool isLeopardOrHigher = ( ( ( ( res >> 8 ) & 0x00FF ) == 0x10 ) && ( ( ( res >> 4 ) & 0x000F ) >= 0x5 ) );
		if(!isLeopardOrHigher)
			return(false);
	}
	
	return(true);
}
