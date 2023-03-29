/* -*- Mode: C++; eval:(c-set-style "bsd"); tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * service implementation:	 org.neooffice.MediaBrowser
 * exported interfaces:		 org.neooffice.XMediaBrowser
 *
 *************************************************************************
 *************************************************************************/

#include <stdio.h>
#include <dlfcn.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
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
#ifndef _CPPUHELPER_IMPLBASE_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _ORG_NEOOFFICE_XREMOTECONTROL_HPP_
#include <org/neooffice/XRemoteControl.hpp>
#endif
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/task/XJob.hpp>
#include <comphelper/processfactory.hxx>

#include "premac.h"
#import <Cocoa/Cocoa.h>
// Need to include for SystemUIMode and SystemUIOptions but we don't link to it
#import <Carbon/Carbon.h>
#include "postmac.h"

#define SERVICENAME "org.neooffice.RemoteControl"
#define IMPLNAME	"org.neooffice.XRemoteControl"

typedef void GetSystemUIMode_Type( SystemUIMode *nMode, SystemUIOptions *nOptions );
typedef void ShowOnlyMenusForWindow_Type( void*, sal_Bool );
 
static NSString *kRemoteControlFrameworkName=@"RemoteControl.framework";

static ::vos::OModule aModule;
static ShowOnlyMenusForWindow_Type *pShowOnlyMenusForWindow = NULL;
static NSString *kMainControllerClass = @"MainController";

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
using namespace ::vos;

typedef enum _RemoteControlEventIdentifier {
	// normal events
	kRemoteButtonPlus				=1<<1,
	kRemoteButtonMinus				=1<<2,
	kRemoteButtonMenu				=1<<3,
	kRemoteButtonPlay				=1<<4,
	kRemoteButtonRight				=1<<5,
	kRemoteButtonLeft				=1<<6,
	
	// hold events
	kRemoteButtonPlus_Hold			=1<<7,
	kRemoteButtonMinus_Hold			=1<<8,	
	kRemoteButtonMenu_Hold			=1<<9,	
	kRemoteButtonPlay_Hold			=1<<10,	
	kRemoteButtonRight_Hold			=1<<11,
	kRemoteButtonLeft_Hold			=1<<12,
	
	// special events (not supported by all devices)	
	kRemoteControl_Switched			=1<<13,
} RemoteControlEventIdentifier;

@class RemoteControlDelegateImpl;

//========================================================================
class MacOSXRemoteControlImpl
	: public ::cppu::WeakImplHelper2<XServiceInfo, XRemoteControl>
{
public:
	// to obtain other services if needed
	Reference< XComponentContext > m_xServiceManager;

private:	
	static RemoteControlDelegateImpl *imp;
	
public:
	MacOSXRemoteControlImpl( const Reference< XComponentContext > & xServiceManager );
	virtual ~MacOSXRemoteControlImpl();

    // XServiceInfo	implementation
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );
    
    // XRemoteControl implementation
	virtual ::sal_Bool 
		SAL_CALL hasRemoteControl( ) 
		throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool 
		SAL_CALL initRemoteControl( ) 
		throw (::com::sun::star::uno::RuntimeException);

	// XJob implementation
	virtual ::com::sun::star::uno::Any SAL_CALL execute(
		const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Arguments )
		throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
};

RemoteControlDelegateImpl * MacOSXRemoteControlImpl::imp=NULL;

//*************************************************************************
OUString SAL_CALL MacOSXRemoteControlImpl::getImplementationName(  ) 
	throw(RuntimeException)
{
	return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}	

//*************************************************************************
sal_Bool SAL_CALL MacOSXRemoteControlImpl::supportsService( const OUString& ServiceName ) 
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
Sequence<OUString> SAL_CALL MacOSXRemoteControlImpl::getSupportedServiceNames(  ) 
	throw(RuntimeException)
{
	return getSupportedServiceNames_Static();
}	

//*************************************************************************
Sequence<OUString> SAL_CALL MacOSXRemoteControlImpl::getSupportedServiceNames_Static(  ) 
{
	OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
	return Sequence< OUString >( &aName, 1 );
}	




/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
Reference< XInterface > SAL_CALL MacOSXRemoteControlImpl_create(
	const Reference< XComponentContext > & xContext )
{
	Reference< XTypeProvider > xRet;

	// Locate libvcl and invoke the ShowOnlyMenusForWindow function
	if ( !pShowOnlyMenusForWindow )
	{
		::rtl::OUString aLibName( RTL_CONSTASCII_USTRINGPARAM( "libvcl.dylib" ) );
		if ( aModule.load( aLibName ) )
			pShowOnlyMenusForWindow = (ShowOnlyMenusForWindow_Type *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "ShowOnlyMenusForWindow" ) );
	}

	if ( pShowOnlyMenusForWindow )
		xRet = static_cast<XTypeProvider *>(new MacOSXRemoteControlImpl(xContext));

	return xRet;
}


//#########################################################################
//#### EXPORTED ###########################################################
//#########################################################################

		/* shared lib exports implemented without helpers in service_impl1.cxx */
		namespace neo_MacOSXRemoteControl_impl
		{
		
	static Sequence< OUString > getSupportedServiceNames_MacOSXRemoteControlImpl()
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
		 
		static OUString getImplementationName_MacOSXRemoteControlImpl()
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
		        MacOSXRemoteControlImpl_create, getImplementationName_MacOSXRemoteControlImpl,
		        getSupportedServiceNames_MacOSXRemoteControlImpl, ::cppu::createSingleComponentFactory,
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
extern "C" sal_Bool SAL_CALL component_writeInfo(void* pServiceManager, void* pRegistryKey)
{
	return ::cppu::component_writeInfoHelper(pServiceManager, pRegistryKey, ::neo_MacOSXRemoteControl_impl::s_component_entries);
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
	return ::cppu::component_getFactoryHelper(pImplName, pServiceManager, pRegistryKey, ::neo_MacOSXRemoteControl_impl::s_component_entries);
}

#pragma mark -

MacOSXRemoteControlImpl::MacOSXRemoteControlImpl( const Reference< XComponentContext > & xServiceManager )
	: m_xServiceManager( xServiceManager )
{
}

MacOSXRemoteControlImpl::~MacOSXRemoteControlImpl()
{
}

::com::sun::star::uno::Any SAL_CALL MacOSXRemoteControlImpl::execute(
	const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Arguments )
	throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
	sal_Int32 res=0;
	
	initRemoteControl();
	
	return(::com::sun::star::uno::Any(res));
}

@interface NSObject (RemoteControlAppDelegate)
- (void)applicationWillBecomeActive:(NSNotification *)pNotification;
- (void)applicationWillResignActive:(NSNotification *)pNotification;
@end

@interface RemoteControlDelegateImpl : NSObject <NSApplicationDelegate>
{
id realAppDelegate;
id rcController;
id rcControl;
}
+ (SystemUIMode)systemUIMode;
- (id)init;
- (void)dealloc;
- (void)bindRemoteControls:(id)obj;
- (void)sendRemoteButtonEvent: (RemoteControlEventIdentifier)buttonIdentifier pressedDown: (BOOL) pressedDown remoteControl: (id)remoteControl;
- (void)startPresentation:(id)obj;
- (void)previousSlide:(id)obj;
- (void)nextSlide:(id)obj;
- (void)applicationWillBecomeActive:(NSNotification *)pNotification;
- (void)applicationWillResignActive:(NSNotification *)pNotification;
- (void)forwardInvocation:(NSInvocation *)pInvocation;
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector;
- (BOOL)respondsToSelector:(SEL)aSelector;
@end

/**
 * Check if the we have the iMediaBrowser framework available
 */
::sal_Bool 
		SAL_CALL MacOSXRemoteControlImpl::hasRemoteControl( ) 
		throw (::com::sun::star::uno::RuntimeException)
{
	::sal_Bool bRet = sal_False;

#ifdef DEBUG
	MacOSBoolean hasKey=false;
	MacOSBoolean useRemote=CFPreferencesGetAppBooleanValue(CFSTR("remoteEnabled"), kCFPreferencesCurrentApplication, &hasKey);
	if(hasKey && !useRemote)
		return(bRet);
#endif	// DEBUG
	
	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	// load our framework out of our bundle's directory
	
	NSBundle *mainBundle=[NSBundle mainBundle];
	if(mainBundle)
	{
		NSString *frameworksPath=[mainBundle privateFrameworksPath];
		if(frameworksPath)
		{
			NSString *frameworkLibPath=[frameworksPath stringByAppendingPathComponent:kRemoteControlFrameworkName];
			if(frameworkLibPath)
			{
				NSBundle *frameworkLib=[NSBundle bundleWithPath:frameworkLibPath];
				if(frameworkLib)
				{
					// check to see if we can locate our class after we've loaded the framework

					Class rcClass=[frameworkLib classNamed:kMainControllerClass];
					if(rcClass)
						bRet = sal_True;
				}
			}
		}
	}

	[pool release];

	return(bRet);
}

/**
 * Allocate and bind the remote control instances to our object.
 */
::sal_Bool 
		SAL_CALL MacOSXRemoteControlImpl::initRemoteControl( ) 
		throw (::com::sun::star::uno::RuntimeException)
{
	if(imp)
		return(true);
	
	if(!hasRemoteControl())
		return(false);
		
	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
	
	imp=[[RemoteControlDelegateImpl alloc] init];
	
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[imp performSelectorOnMainThread:@selector(bindRemoteControls:) withObject:imp waitUntilDone: 1 modes: pModes];
	
	[pool release];
	
	return(imp!=NULL);
}

@implementation RemoteControlDelegateImpl

+ (SystemUIMode)systemUIMode
{
	SystemUIMode nRet=kUIModeNormal;

	void *pLib = dlopen( NULL, RTLD_LAZY | RTLD_LOCAL );
	if ( pLib )
	{
		GetSystemUIMode_Type *pGetSystemUIMode = (GetSystemUIMode_Type *)dlsym( pLib, "GetSystemUIMode" );
		if ( pGetSystemUIMode )
			pGetSystemUIMode(&nRet, NULL);
	}

	return nRet;
}

- (id)init
{
	realAppDelegate=nil;
	rcController=nil;
	rcControl=nil;
	self = [super init];
	return(self);
}

- (void)dealloc
{
	if(realAppDelegate)
		[realAppDelegate autorelease];
	if(rcController)
		[rcController autorelease];
	if(rcControl)
		[rcControl autorelease];
	
	[super dealloc];
}

- (void)bindRemoteControls:(id)obj
{
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( !pApp )
		return;

	// load our framework out of our bundle's directory
	
	Class rcClass=nil;
	NSBundle *mainBundle=[NSBundle mainBundle];
	if(mainBundle)
	{
		NSString *frameworksPath=[mainBundle privateFrameworksPath];
		if(frameworksPath)
		{
			NSString *frameworkLibPath=[frameworksPath stringByAppendingPathComponent:kRemoteControlFrameworkName];
			if(frameworkLibPath)
			{
				NSBundle *frameworkLib=[NSBundle bundleWithPath:frameworkLibPath];
				if(frameworkLib)
					rcClass=[frameworkLib classNamed:kMainControllerClass];
			}
		}
	}

	
	if(rcClass)
	{
		rcController=[rcClass performSelector:@selector(alloc)];
		if(rcController)
		{
		    [rcController performSelector:@selector(init)];
		    [rcController performSelector:@selector(awakeFromNib)];
		    rcControl=[rcController performSelector:@selector(remoteControl)];
		    if(rcControl)
			{
		    	[rcControl performSelector:@selector(setDelegate:) withObject:self];

				realAppDelegate = [pApp delegate];
				if(realAppDelegate)
					[realAppDelegate retain];
				[pApp setDelegate:self];

				// Manually start listening if the application is already active
				if([pApp isActive])
					[rcController performSelector:@selector(applicationWillBecomeActive:) withObject:nil];
			}
		}
	}
	else
	{
		fprintf(stderr, "Controller class not found\n");
	}
}

- (void)sendRemoteButtonEvent: (RemoteControlEventIdentifier)buttonIdentifier pressedDown: (BOOL) pressedDown remoteControl: (id)remoteControl
{	
	switch(buttonIdentifier) 
	{
		case kRemoteButtonPlay:
			if(!pressedDown)
			{
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				unsigned long nCount = Application::ReleaseSolarMutex();
				[self performSelectorOnMainThread:@selector(startPresentation:) withObject:self waitUntilDone: 1 modes: pModes];
				Application::AcquireSolarMutex( nCount );
			}
			break;			

		case kRemoteButtonRight:	
			if(!pressedDown)
			{
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				[self performSelectorOnMainThread:@selector(nextSlide:) withObject:self waitUntilDone: 1 modes: pModes];
			}
			break;			

		case kRemoteButtonLeft:
			if(!pressedDown)
			{
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				[self performSelectorOnMainThread:@selector(previousSlide:) withObject:self waitUntilDone: 1 modes: pModes];
			}
			break;			

		default:
			break;
	}
}

- (void)startPresentation: (id)ignore
{
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( !pApp )
		return;

	NSWindow *pKeyWindow = [pApp keyWindow];
	if ( !pKeyWindow )
		return;

	SystemUIMode outMode=[RemoteControlDelegateImpl systemUIMode];
	if(outMode==kUIModeNormal)
	{
		if(!Application::IsShutDown())
		{
			IMutex& rSolarMutex = Application::GetSolarMutex();
			rSolarMutex.acquire();
			if(!Application::IsShutDown())
			{
				try
				{
					Reference< XComponentContext > component( comphelper_getProcessComponentContext() );
					Reference< XMultiComponentFactory > rServiceManager = component->getServiceManager();
					Reference< XInterface > rDesktop = rServiceManager->createInstanceWithContext(OUString::createFromAscii("com.sun.star.frame.Desktop"), component);
					
					Reference< XDispatchHelper > rDispatchHelper = Reference< XDispatchHelper >(rServiceManager->createInstanceWithContext(OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.DispatchHelper" )), component), UNO_QUERY ); 
					
					Reference< XDesktop > Desktop(rDesktop,UNO_QUERY);
					Reference< XFrame > rFrame=Desktop->getCurrentFrame(); 
				
					Reference< XDispatchProvider > rDispatchProvider(rFrame,UNO_QUERY); 
					
					Sequence< PropertyValue > args(0);
					
					rDispatchHelper->executeDispatch(rDispatchProvider, OUString::createFromAscii(".uno:Presentation"), OUString::createFromAscii(""), 0, args);
				}
				catch (...)
				{
				}
			}

			rSolarMutex.release();
		}
	}
	else if(outMode==kUIModeAllHidden)
	{
		NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

		unichar escape=27;
		NSString *characters=[NSString stringWithCharacters: &escape length: 1];
		[pApp postEvent:
			[NSEvent keyEventWithType:NSKeyDown
				location: NSZeroPoint
				modifierFlags: 0
				timestamp: 0
				windowNumber: [pKeyWindow windowNumber]
				context: nil
				characters: characters
				charactersIgnoringModifiers: characters
				isARepeat: NO
				keyCode: 53]
			atStart: NO];
		
		[pool release];
	}
}

- (void)previousSlide:(id)ignore
{
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( !pApp )
		return;

	NSWindow *pKeyWindow = [pApp keyWindow];
	if ( !pKeyWindow )
		return;

	SystemUIMode outMode=[RemoteControlDelegateImpl systemUIMode];
	if(outMode==kUIModeAllHidden)
	{
		NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
		
		unichar leftArrow=NSLeftArrowFunctionKey;
		NSString *characters=[NSString stringWithCharacters: &leftArrow length: 1];
		[pApp postEvent:
			[NSEvent keyEventWithType:NSKeyDown
				location: NSZeroPoint
				modifierFlags: 0
				timestamp: 0
				windowNumber: [pKeyWindow windowNumber]
				context: nil
				characters: characters
				charactersIgnoringModifiers: characters
				isARepeat: NO
				keyCode: 123]
			atStart: NO];
		
		[pool release];
	}
}

- (void)nextSlide:(id)ignore
{
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( !pApp )
		return;

	NSWindow *pKeyWindow = [pApp keyWindow];
	if ( !pKeyWindow )
		return;

	SystemUIMode outMode=[RemoteControlDelegateImpl systemUIMode];
	if(outMode==kUIModeAllHidden)
	{
		NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
	
		unichar rightArrow=NSRightArrowFunctionKey;
		NSString *characters=[NSString stringWithCharacters: &rightArrow length: 1];
		[pApp postEvent:
			[NSEvent keyEventWithType:NSKeyDown
				location: NSZeroPoint
				modifierFlags: 0
				timestamp: 0
				windowNumber: [pKeyWindow windowNumber]
				context: nil
				characters: characters
				charactersIgnoringModifiers: characters
				isARepeat: NO
				keyCode: 124]
			atStart: NO];
		
		[pool release];
	}
}

- (void)applicationWillBecomeActive:(NSNotification *)pNotification
{
	if(rcController)
		[rcController performSelector:@selector(applicationWillBecomeActive:) withObject:pNotification];

	if(realAppDelegate && [realAppDelegate respondsToSelector:@selector(applicationWillBecomeActive:)])
		[realAppDelegate applicationWillBecomeActive:pNotification];
}

- (void)applicationWillResignActive:(NSNotification *)pNotification
{
	if(rcController)
		[rcController performSelector:@selector(applicationWillResignActive:) withObject:pNotification];

	if(realAppDelegate && [realAppDelegate respondsToSelector:@selector(applicationWillResignActive:)])
		[realAppDelegate applicationWillResignActive:pNotification];
}

- (void)forwardInvocation:(NSInvocation *)pInvocation
{
	BOOL bHandled = NO;

	SEL aSelector = [pInvocation selector];

	if(realAppDelegate && [realAppDelegate respondsToSelector:aSelector])
	{
		[pInvocation invokeWithTarget:realAppDelegate];
		bHandled = YES;
	}

	if ( !bHandled )
		[self doesNotRecognizeSelector:aSelector];
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	NSMethodSignature *pRet = [super methodSignatureForSelector:aSelector];

	if(!pRet && realAppDelegate && [realAppDelegate respondsToSelector:aSelector])
		pRet = [realAppDelegate methodSignatureForSelector:aSelector];

	return pRet;
}

- (BOOL)respondsToSelector:(SEL)aSelector
{
	BOOL bRet = [super respondsToSelector:aSelector];

	if (!bRet && realAppDelegate)
		bRet = [realAppDelegate respondsToSelector:aSelector];

	return bRet;
}

@end
