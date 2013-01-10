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
#undef MAC_OS_X_VERSION_MIN_REQUIRED
#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_6
#import <Quartz/Quartz.h>
#include "postmac.h"

// Redefine Cocoa YES and NO defines types for convenience
#ifdef YES
#undef YES
#define YES (MacOSBOOL)1
#endif
#ifdef NO
#undef NO
#define NO (MacOSBOOL)0
#endif

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

@interface NSView (ImageCaptureImpl)
- (void)setTranslatesAutoresizingMaskIntoConstraints:(MacOSBOOL)bFlag;
@end

@interface ImageCaptureImplNSPanel : NSPanel
- (void)windowWillClose:(NSNotification *)pNotification;
@end

@interface ImageCaptureImplIKCameraDeviceView : IKCameraDeviceView < IKCameraDeviceViewDelegate >
- (void)cameraDeviceView:(ImageCaptureImplIKCameraDeviceView *)pCameraDeviceView didDownloadFile:(ICCameraFile *)pFile location:(NSURL *)pURL fileData:(NSData *)pFileData error:(NSError *)pError;
- (void)cameraDeviceView:(ImageCaptureImplIKCameraDeviceView *)pCameraDeviceView didEncounterError:(NSError *)pError;
- (void)cameraDeviceViewSelectionDidChange:(ImageCaptureImplIKCameraDeviceView *)pCameraDeviceView;
@end

@interface ImageCaptureImplIKDeviceBrowserView : IKDeviceBrowserView < IKDeviceBrowserViewDelegate >
- (void)deviceBrowserView:(ImageCaptureImplIKDeviceBrowserView *)pDeviceBrowserView didEncounterError:(NSError *)pError;
- (void)deviceBrowserView:(ImageCaptureImplIKDeviceBrowserView *)pDeviceBrowserView selectionDidChange:(ICDevice *)pDevice;
- (void)selectionDidChangeToSelectedDevice;
@end

@interface ImageCaptureImplIKScannerDeviceView : IKScannerDeviceView < IKScannerDeviceViewDelegate >
- (void)scannerDeviceView:(ImageCaptureImplIKScannerDeviceView *)pScannerDeviceView didEncounterError:(NSError *)pError;
- (void)scannerDeviceView:(ImageCaptureImplIKScannerDeviceView *)pScannerDeviceView didScanToURL:(NSURL *)pURL fileData:(NSData *)pFileData error:(NSError *)pError;
@end

@interface ImageCaptureImpl : NSObject
{
	bool gotImage;
}
+ (id)create;
+ (NSString *)defaultTitle;
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

static void ShowAlertWithError( NSError *pError )
{
	if ( pError )
	{
		NSAlert *pAlert = [NSAlert alertWithError:pError];
		if ( pAlert )
			[pAlert runModal];
	}
}

//========================================================================

class MacOSXImageCaptureImpl
	: public ::cppu::WeakImplHelper2<XServiceInfo, XImageCapture>
{
	// to obtain other services if needed
	Reference< XComponentContext > m_xServiceManager;
	
	sal_Int32 m_nRefCount;
	sal_Int32 m_nCount;
	
public:
	MacOSXImageCaptureImpl( const Reference< XComponentContext > & xServiceManager )
		: m_xServiceManager( xServiceManager ), m_nRefCount( 0 ) {}
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

static ImageCaptureImpl *mpCurrentImageCaptureImpl = nil;
static NSModalSession maModalSession = nil;
static NSString *mpDefaultTitle = nil;
static NSPanel *mpPanel = nil;
static NSSplitView *mpSplitView = nil;
static ImageCaptureImplIKDeviceBrowserView *mpDeviceBrowserView = nil;
static NSView *mpEmptyView = nil;
static NSProgressIndicator *mpWaitingView = nil;
static ::std::map< ICCameraDevice*, ImageCaptureImplIKCameraDeviceView* > aCameraDeviceViewMap;
static ::std::map< ICScannerDevice*, ImageCaptureImplIKScannerDeviceView* > aScannerDeviceViewMap;

@implementation ImageCaptureImplNSPanel

- (void)windowWillClose:(NSNotification *)pNotification
{
	if ( maModalSession )
	{
		NSApplication *pApp = [NSApplication sharedApplication];
		if ( pApp )
			[pApp stopModal];
	}

	// Clear all of the device views from the empty view so that the panel is
	// in a "no selected device" state when reopened
	if ( mpDeviceBrowserView )
		[mpDeviceBrowserView deviceBrowserView:mpDeviceBrowserView selectionDidChange:nil];

	// Remove the split view to stop CGSGetSurfaceBounds errors on Mac OS X 10.6
	if ( mpSplitView )
		[mpSplitView removeFromSuperview];
}

@end

@implementation ImageCaptureImplIKCameraDeviceView

- (void)cameraDeviceView:(ImageCaptureImplIKCameraDeviceView *)pCameraDeviceView didDownloadFile:(ICCameraFile *)pFile location:(NSURL *)pURL fileData:(NSData *)pFileData error:(NSError *)pError
{
	if ( !pCameraDeviceView || !mpCurrentImageCaptureImpl || [mpCurrentImageCaptureImpl capturedImage] || !maModalSession || !mpPanel || ![mpPanel isVisible] )
		return;

	NSWindow *pWindow = [pCameraDeviceView window];
	if ( !pWindow || pWindow != mpPanel )
		return;

	NSPasteboard *pPasteboard = [NSPasteboard generalPasteboard];
	if ( pPasteboard )
	{
		// Convert image into TIFF so we can put it on the pasteboard
		NSData *pTIFFData = nil;

		// Try file data first
		if ( pFileData )
		{
			NSImage *pImage = [[NSImage alloc] initWithData:pFileData];
			if ( pImage )
			{
				[pImage autorelease];
				pTIFFData = [pImage TIFFRepresentation];
			}
		}

		// Try URL second
		if ( !pTIFFData && pURL )
		{
			NSImage *pImage = [[NSImage alloc] initByReferencingURL:pURL];
			if ( pImage )
			{
				[pImage autorelease];
				pTIFFData = [pImage TIFFRepresentation];
			}
		}

		if ( pTIFFData )
		{
			[pPasteboard declareTypes:[NSArray arrayWithObject:NSTIFFPboardType] owner:nil];
			[pPasteboard setData:pTIFFData forType:NSTIFFPboardType];
			[mpCurrentImageCaptureImpl setCapturedImage:true];

			// Close modal panel after pasting data to pasteboard
			if ( [mpPanel isVisible] )
				[mpPanel close];
		}
	}
}

- (void)cameraDeviceView:(ImageCaptureImplIKCameraDeviceView *)pCameraDeviceView didEncounterError:(NSError *)pError
{
	if ( !pCameraDeviceView || !mpCurrentImageCaptureImpl || [mpCurrentImageCaptureImpl capturedImage] || !maModalSession || !mpPanel || ![mpPanel isVisible] )
		return;

	NSWindow *pWindow = [pCameraDeviceView window];
	if ( !pWindow || pWindow != mpPanel )
		return;

	ShowAlertWithError( pError );
}

- (void)cameraDeviceViewSelectionDidChange:(ImageCaptureImplIKCameraDeviceView *)pCameraDeviceView
{
}

@end

@implementation ImageCaptureImplIKDeviceBrowserView

- (void)deviceBrowserView:(ImageCaptureImplIKDeviceBrowserView *)pDeviceBrowserView didEncounterError:(NSError *)pError
{
	// Do nothing if we aren't in running the modal panel
	if ( !pDeviceBrowserView || !mpCurrentImageCaptureImpl || [mpCurrentImageCaptureImpl capturedImage] || !maModalSession || !mpPanel || ![mpPanel isVisible] )
		return;

	NSWindow *pWindow = [pDeviceBrowserView window];
	if ( !pWindow || pWindow != mpPanel )
		return;

	ShowAlertWithError( pError );
}

- (void)deviceBrowserView:(ImageCaptureImplIKDeviceBrowserView *)pDeviceBrowserView selectionDidChange:(ICDevice *)pDevice
{
	[NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(selectionDidChangeToSelectedDevice) object:nil];

	// Do nothing if we aren't in running the modal panel
	if ( !pDeviceBrowserView || !mpCurrentImageCaptureImpl || [mpCurrentImageCaptureImpl capturedImage] || !maModalSession || !mpPanel || ![mpPanel isVisible] )
		return;

	NSWindow *pWindow = [pDeviceBrowserView window];
	if ( !pWindow || pWindow != mpPanel )
		return;

	// Remove all device subviews from the empty view
	NSArray *pSubviews = [mpEmptyView subviews];
	if ( pSubviews )
	{
		NSArray *pSubviewsCopy = [NSArray arrayWithArray:pSubviews];
		if ( pSubviewsCopy )
		{
			NSUInteger nCount = [pSubviewsCopy count];
			NSUInteger i = 0;
			for ( ; i < nCount; i++ )
			{
				NSView *pSubview = [pSubviewsCopy objectAtIndex:i];
				if ( pSubview )
				{
					if ( [pSubview isKindOfClass:[IKCameraDeviceView class]] )
					{
						IKCameraDeviceView *pCameraDeviceView = (IKCameraDeviceView *)pSubview;
						if ( pCameraDeviceView.cameraDevice )
						{
							[pCameraDeviceView.cameraDevice cancelDownload];
							[pCameraDeviceView.cameraDevice requestCloseSession];
							pCameraDeviceView.cameraDevice = nil;
						}
					}
					else if ( [pSubview isKindOfClass:[IKScannerDeviceView class]] )
					{
						IKScannerDeviceView *pScannerDeviceView = (IKScannerDeviceView *)pSubview;
						if ( pScannerDeviceView.scannerDevice )
						{
							[pScannerDeviceView.scannerDevice cancelScan];
							[pScannerDeviceView.scannerDevice requestCloseSession];
							pScannerDeviceView.scannerDevice = nil;
						}
					}
					else if ( [pSubview isKindOfClass:[NSProgressIndicator class]] )
					{
						[mpWaitingView stopAnimation:self];
					}

					[pSubview removeFromSuperview];
				}
			}
		}
	}

	// Add a subview for the device in the empty view
	[mpPanel setTitle:[ImageCaptureImpl defaultTitle]];
	if ( pDevice )
	{
		if ( pDevice.hasOpenSession )
		{
			if ( mpWaitingView )
			{
				// Center in content view
				NSRect aEmptyBounds = [mpEmptyView bounds];
				NSRect aWaitingFrame = [mpWaitingView frame];
				[mpWaitingView setFrameOrigin:NSMakePoint( ( aEmptyBounds.size.width - aWaitingFrame.size.width ) / 2, ( aEmptyBounds.size.height - aWaitingFrame.size.height ) / 2 )];

				[mpEmptyView addSubview:mpWaitingView];
				[mpWaitingView startAnimation:self];
			}

			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[self performSelector:@selector(selectionDidChangeToSelectedDevice) withObject:nil afterDelay:(NSTimeInterval)1.0f inModes:pModes];
		}
		else if ( [pDevice isKindOfClass:[ICCameraDevice class]] )
		{
			ImageCaptureImplIKCameraDeviceView *pCameraDeviceView = nil;
			::std::map< ICCameraDevice*, ImageCaptureImplIKCameraDeviceView* >::const_iterator it = aCameraDeviceViewMap.find( (ICCameraDevice *)pDevice );
			if ( it != aCameraDeviceViewMap.end() )
				pCameraDeviceView = it->second;

			if ( !pCameraDeviceView )
			{
				pCameraDeviceView = [[ImageCaptureImplIKCameraDeviceView alloc] initWithFrame:[mpEmptyView bounds]];
				if ( pCameraDeviceView )
				{
					pCameraDeviceView.delegate = pCameraDeviceView;

					[pCameraDeviceView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
					// Setting translates autoresizing mask is needed for view
					// to resize on Mac OS X 10.8
					if ( [pCameraDeviceView respondsToSelector:@selector(setTranslatesAutoresizingMaskIntoConstraints:)] )
						[pCameraDeviceView setTranslatesAutoresizingMaskIntoConstraints:YES];

					pCameraDeviceView.displaysDownloadsDirectoryControl = NO;
					pCameraDeviceView.displaysPostProcessApplicationControl = NO;
					pCameraDeviceView.mode = IKCameraDeviceViewDisplayModeTable;
					// Stop opening of Preview application after import
					pCameraDeviceView.postProcessApplication = nil;
					pCameraDeviceView.transferMode = IKScannerDeviceViewTransferModeMemoryBased;

					aCameraDeviceViewMap[ (ICCameraDevice *)pDevice ] = pCameraDeviceView;
				}
			}

			if ( pCameraDeviceView )
			{
				[mpEmptyView addSubview:pCameraDeviceView];
				// Set device after display to eliminate Mac OS X 10.6 log messages
				pCameraDeviceView.cameraDevice = (ICCameraDevice *)pDevice;
				[mpPanel setTitle:[pDevice name]];
			}
		}
		else if ( [pDevice isKindOfClass:[ICScannerDevice class]] )
		{
			ImageCaptureImplIKScannerDeviceView *pScannerDeviceView = nil;
			::std::map< ICScannerDevice*, ImageCaptureImplIKScannerDeviceView* >::const_iterator it = aScannerDeviceViewMap.find( (ICScannerDevice *)pDevice );
			if ( it != aScannerDeviceViewMap.end() )
				pScannerDeviceView = it->second;

			if ( !pScannerDeviceView )
			{
				pScannerDeviceView = [[ImageCaptureImplIKScannerDeviceView alloc] initWithFrame:[mpEmptyView bounds]];
				if ( pScannerDeviceView )
				{
					pScannerDeviceView.delegate = pScannerDeviceView;

					[pScannerDeviceView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
					// Setting translates autoresizing mask is needed for view
					// to resize on Mac OS X 10.8
					if ( [pScannerDeviceView respondsToSelector:@selector(setTranslatesAutoresizingMaskIntoConstraints:)] )
						[pScannerDeviceView setTranslatesAutoresizingMaskIntoConstraints:YES];

					pScannerDeviceView.hasDisplayModeAdvanced = YES;
					pScannerDeviceView.hasDisplayModeSimple = YES;
					pScannerDeviceView.displaysDownloadsDirectoryControl = NO;
					pScannerDeviceView.displaysPostProcessApplicationControl = NO;
					pScannerDeviceView.mode = IKScannerDeviceViewDisplayModeSimple;
					// Stop opening of Preview application after import
					pScannerDeviceView.postProcessApplication = nil;
					pScannerDeviceView.transferMode = IKScannerDeviceViewTransferModeMemoryBased;

					aScannerDeviceViewMap[ (ICScannerDevice *)pDevice ] = pScannerDeviceView;
				}
			}

			if ( pScannerDeviceView )
			{
				[mpEmptyView addSubview:pScannerDeviceView];
				// Set device after display to eliminate Mac OS X 10.6 log messages
				pScannerDeviceView.scannerDevice = (ICScannerDevice *)pDevice;
				[mpPanel setTitle:[pDevice name]];
			}
		}
	}
}

- (void)selectionDidChangeToSelectedDevice
{
	[self deviceBrowserView:self selectionDidChange:self.selectedDevice];
}

@end

@implementation ImageCaptureImplIKScannerDeviceView

- (void)scannerDeviceView:(ImageCaptureImplIKScannerDeviceView *)pScannerDeviceView didEncounterError:(NSError *)pError
{
	if ( pScannerDeviceView != self || !mpCurrentImageCaptureImpl || [mpCurrentImageCaptureImpl capturedImage] || !maModalSession || !mpPanel || ![mpPanel isVisible] )
		return;

	NSWindow *pWindow = [self window];
	if ( !pWindow || pWindow != mpPanel )
		return;

	ShowAlertWithError( pError );
}

- (void)scannerDeviceView:(ImageCaptureImplIKScannerDeviceView *)pScannerDeviceView didScanToURL:(NSURL *)pURL fileData:(NSData *)pFileData error:(NSError *)pError
{
	if ( pScannerDeviceView != self || !mpCurrentImageCaptureImpl || [mpCurrentImageCaptureImpl capturedImage] || !maModalSession || !mpPanel || ![mpPanel isVisible] )
		return;

	NSWindow *pWindow = [self window];
	if ( !pWindow || pWindow != mpPanel )
		return;

	NSPasteboard *pPasteboard = [NSPasteboard generalPasteboard];
	if ( pPasteboard )
	{
		// Convert image into TIFF so we can put it on the pasteboard
		NSData *pTIFFData = nil;

		// Try file data first
		if ( pFileData )
		{
			NSImage *pImage = [[NSImage alloc] initWithData:pFileData];
			if ( pImage )
			{
				[pImage autorelease];
				pTIFFData = [pImage TIFFRepresentation];
			}
		}

		// Try URL second
		if ( !pTIFFData && pURL )
		{
			NSImage *pImage = [[NSImage alloc] initByReferencingURL:pURL];
			if ( pImage )
			{
				[pImage autorelease];
				pTIFFData = [pImage TIFFRepresentation];
			}
		}

		if ( pTIFFData )
		{
			[pPasteboard declareTypes:[NSArray arrayWithObject:NSTIFFPboardType] owner:nil];
			[pPasteboard setData:pTIFFData forType:NSTIFFPboardType];
			[mpCurrentImageCaptureImpl setCapturedImage:true];

			// Close modal panel after pasting data to pasteboard
			if ( [mpPanel isVisible] )
				[mpPanel close];
		}
	}
}

@end

@implementation ImageCaptureImpl

+ (id)create
{
	ImageCaptureImpl *pRet = [[ImageCaptureImpl alloc] init];
	[pRet autorelease];
	return pRet;
}

+ (NSString *)defaultTitle
{
	if ( !mpDefaultTitle )
	{
		// Get STR_NONE localized string
		SimpleResMgr *pResMgr = SimpleResMgr::Create( CREATEVERSIONRESMGR_NAME( sfx ) );
		if ( pResMgr )
		{
			XubString aNone( pResMgr->ReadString( STR_NONE ) );
			aNone.EraseAllChars( '~' );
			if ( aNone.Len() );
				mpDefaultTitle = [NSString stringWithCharacters:aNone.GetBuffer() length:aNone.Len()];
			delete pResMgr;
		}

		if ( !mpDefaultTitle )
			mpDefaultTitle = @"- None -";

		if ( mpDefaultTitle )
			[mpDefaultTitle retain];
	}

	return mpDefaultTitle;
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
	if ( gotImage || maModalSession || ( mpPanel && [mpPanel isVisible] ) )
		return;

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		if ( !mpPanel )
			mpPanel = [[ImageCaptureImplNSPanel alloc] initWithContentRect:NSMakeRect( 0, 0, 800, 550 ) styleMask:NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask backing:NSBackingStoreBuffered defer:YES];
		if ( mpPanel )
		{
			[mpPanel setDelegate:mpPanel];
			[mpPanel setReleasedWhenClosed:NO];
			[mpPanel setTitle:[ImageCaptureImpl defaultTitle]];

			// Set background to a slightly dark gray since the text is white
			// in the scanner device view on Mac OS X 10.8
			[mpPanel setBackgroundColor:[NSColor grayColor]];

			NSView *pContentView = [mpPanel contentView];
			if ( pContentView )
			{
				if ( !mpSplitView )
				{
					mpSplitView = [[NSSplitView alloc] initWithFrame:[pContentView bounds]];
					if ( mpSplitView )
					{
						[mpSplitView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
						[mpSplitView setDividerStyle:NSSplitViewDividerStyleThin];
						[mpSplitView setVertical:YES];
					}
				}

				if ( mpSplitView )
				{
					if ( [mpSplitView superview] != pContentView )
					{
						[mpSplitView removeFromSuperview];
						[pContentView addSubview:mpSplitView];
					}

					NSRect aSplitViewBounds = [mpSplitView bounds];
					if ( !mpDeviceBrowserView )
					{
						mpDeviceBrowserView = [[ImageCaptureImplIKDeviceBrowserView alloc] initWithFrame:aSplitViewBounds];
						if ( mpDeviceBrowserView )
						{
							// On Mac OS X 10.7, the awakeFromNib selector needs
							// to be invoked or else no devices will be listed
							if ( [mpDeviceBrowserView respondsToSelector:@selector(awakeFromNib)] )
								[mpDeviceBrowserView awakeFromNib];

							mpDeviceBrowserView.delegate = mpDeviceBrowserView;

							mpDeviceBrowserView.mode = IKDeviceBrowserViewDisplayModeIcon;
							mpDeviceBrowserView.displaysLocalCameras = YES;
							mpDeviceBrowserView.displaysNetworkCameras = YES;
							mpDeviceBrowserView.displaysLocalScanners = YES;
							mpDeviceBrowserView.displaysNetworkScanners = YES;
						}
					}

					if ( mpDeviceBrowserView )
					{
						if ( [mpDeviceBrowserView superview] != mpSplitView )
						{
							[mpDeviceBrowserView removeFromSuperview];
							[mpSplitView addSubview:mpDeviceBrowserView];
						}

						if ( !mpEmptyView )
						{
							mpEmptyView = [[NSView alloc] initWithFrame:aSplitViewBounds];
							if ( mpEmptyView )
							{
								if ( [mpEmptyView superview] != mpSplitView )
								{
									[mpEmptyView removeFromSuperview];
									[mpSplitView addSubview:mpEmptyView];
								}

								[mpSplitView setPosition:125.0f ofDividerAtIndex:0];
							}
						}

						if ( mpEmptyView )
						{
							mpWaitingView = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect( 0, 0, 1, 1 )];
							if ( mpWaitingView )
							{
								[mpWaitingView setIndeterminate:YES];
								[mpWaitingView setStyle:NSProgressIndicatorSpinningStyle];
								[mpWaitingView setDisplayedWhenStopped:NO];
								[mpWaitingView sizeToFit];
								[mpWaitingView setAutoresizingMask:NSViewMinXMargin | NSViewMaxXMargin | NSViewMinYMargin | NSViewMaxYMargin];
							}

							mpCurrentImageCaptureImpl = self;

							// Run modal session
							maModalSession = [pApp beginModalSessionForWindow:mpPanel];

							// Forcefully set the device view to the selected
							// device after the start of the modal session
							// makes the panel visible since the empty view
							// will be in a "no selected device" state
							[mpDeviceBrowserView selectionDidChangeToSelectedDevice];
							@try
							{
								while ( [pApp runModalSession:maModalSession] == NSRunContinuesResponse && [mpPanel isVisible] )
									[pApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantFuture] inMode:NSModalPanelRunLoopMode dequeue:NO];
							}
							@catch ( NSException *pExc )
							{
								if ( pExc )
								{
									NSString *pReason = [pExc reason];
									NSAlert *pAlert = [NSAlert alertWithMessageText:[pExc name] defaultButton:nil alternateButton:nil otherButton:nil informativeTextWithFormat:@"%@", pReason ? pReason : @""];
									if ( pAlert )
										[pAlert runModal];
								}
							}

							[pApp endModalSession:maModalSession];
							maModalSession = nil;

							[mpPanel close];
							mpCurrentImageCaptureImpl = nil;
						}
					}
				}
			}
		}
	}
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
