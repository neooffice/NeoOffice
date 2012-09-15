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

#include <dlfcn.h>
#include <rtl/ustring.hxx>
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
// Need to include for ICAImportImagePB struct but we don't link to it
#import <Carbon/Carbon.h>
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

typedef OSErr Gestalt_Type( OSType selector, long *response );
typedef ICAError ICAImportImage_Type( ICAImportImagePB *pPB, ICACompletion nCompletion );
typedef void ShowOnlyMenusForWindow_Type( void*, sal_Bool );
 
static ::vos::OModule aModule;
static ShowOnlyMenusForWindow_Type *pShowOnlyMenusForWindow = NULL;

@interface ImageCaptureImpl : NSObject < IKDeviceBrowserViewDelegate, IKCameraDeviceViewDelegate, IKScannerDeviceViewDelegate >
{
	bool gotImage;
	NSPanel *mpPanel;
	NSSplitView *mpSplitView;
	IKDeviceBrowserView *mpDeviceBrowserView;
	NSView *mpEmptyView;
	IKCameraDeviceView *mpCameraDeviceView;
	IKScannerDeviceView *mpScannerDeviceView;
}
- (id)init;
- (void)doImageCapture: (id)pObj;
- (bool)capturedImage;
- (void)cameraDeviceView:(IKCameraDeviceView *)pCameraDeviceView didDownloadFile:(ICCameraFile *)pFile location:(NSURL *)pURL fileData:(NSData *)pFileData error:(NSError *)pError;
- (void)cameraDeviceView:(IKCameraDeviceView *)pCameraDeviceView didEncounterError:(NSError *)pError;
- (void)cameraDeviceViewSelectionDidChange:(IKCameraDeviceView *)pCameraDeviceView;
- (void)deviceBrowserView:(IKDeviceBrowserView *)pDeviceBrowserView didEncounterError:(NSError *)pError;
- (void)deviceBrowserView:(IKDeviceBrowserView *)pDeviceBrowserView selectionDidChange:(ICDevice *)pDevice;
- (void)scannerDeviceView:(IKScannerDeviceView *)pScannerDeviceView didEncounterError:(NSError *)pError;
- (void)scannerDeviceView:(IKScannerDeviceView *)pScannerDeviceView didScanToURL:(NSURL *)pURL fileData:(NSData *)pFileData error:(NSError *)pError;
- (void)windowWillClose:(NSNotification *)pNotification;
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

static void ResetDeviceViewProperties( NSView *pView )
{
	if ( pView )
	{
		if ( [pView isKindOfClass:[IKDeviceBrowserView class]] )
			((IKDeviceBrowserView *)pView).delegate = nil;
		else if ( [pView isKindOfClass:[IKCameraDeviceView class]] )
			((IKCameraDeviceView *)pView).delegate = nil;
		else if ( [pView isKindOfClass:[IKScannerDeviceView class]] )
			((IKScannerDeviceView *)pView).delegate = nil;
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
	
	ImageCaptureImpl *imp=[[ImageCaptureImpl alloc] init];

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	unsigned long nCount = Application::ReleaseSolarMutex();
	[imp performSelectorOnMainThread:@selector(doImageCapture:) withObject:imp waitUntilDone:YES modes:pModes];
	Application::AcquireSolarMutex( nCount );
	
	bool toReturn=[imp capturedImage];
	
	[imp release];
	
	[pool release];
	
	return(toReturn);
}

#pragma mark -

@implementation ImageCaptureImpl

- (id)init
{
	self = [super init];

	gotImage=false;
	mpPanel=nil;
	mpSplitView=nil;
	mpDeviceBrowserView=nil;
	mpEmptyView=nil;
	mpCameraDeviceView=nil;
	mpScannerDeviceView=nil;

	return(self);
}

- (void)doImageCapture: (id)pObj
{
	// Do nothing if we are recursing
	if ( gotImage || mpPanel || mpSplitView || mpDeviceBrowserView || mpEmptyView || mpCameraDeviceView || mpScannerDeviceView )
		return;

	void *pLib = dlopen( NULL, RTLD_LAZY | RTLD_LOCAL );
	if ( pLib )
	{
		bool bUseICAImageImport = false;
		Gestalt_Type *pGestalt = (Gestalt_Type *)dlsym( pLib, "Gestalt" );
		if ( pGestalt )
		{
			// Use ICAImportImage() if we are running on Mac OS X 10.7 or
			// earlier as IKDeviceBrowserView does not work on 10.7
			SInt32 res = 0;
			pGestalt( gestaltSystemVersionMajor, &res );
			if ( res == 10 )
			{
				res = 0;
				pGestalt( gestaltSystemVersionMinor, &res );
				if ( res <= 7 )
				{
					bUseICAImageImport = true;

					CFArrayRef theTypes = (CFArrayRef)[NSArray arrayWithObjects: @"tif", @"tiff", @"jpg", @"jpeg", @"gif", @"png", @"pdf", @"bmp", NULL];
					NSPasteboard *thePasteboard=[NSPasteboard generalPasteboard];
					if (theTypes && thePasteboard)
					{
						ICAImportImage_Type *pICAImportImage = (ICAImportImage_Type *)dlsym( pLib, "ICAImportImage" );
						if ( pICAImportImage )
						{
							ICAImportImagePB thePB;
							memset(&thePB, '\0', sizeof(thePB));

							// Fix bug 3641 by passing a pointer to NULL
							CFArrayRef importedImages = NULL;
							thePB.importedImages = &importedImages;
							thePB.supportedFileTypes = theTypes;
							ICAError error = pICAImportImage(&thePB, NULL);
							if(thePB.importedImages && *thePB.importedImages)
							{
								if((error==noErr) && CFArrayGetCount(*thePB.importedImages))
								{
									CFDataRef theImage=(CFDataRef)CFArrayGetValueAtIndex(*thePB.importedImages, 0);
									if(theImage)
									{
										// convert image into TIFF so we can put
										// it on the pasteboard
										NSImage *theNSImage=[[NSImage alloc] initWithData:(NSData *)theImage];
										if(theNSImage)
										{
											NSData *theNSTIFFData=[theNSImage TIFFRepresentation];
											if(theNSTIFFData)
											{
												// no need to acquire global
												// mutex now that libdtransjava
												// does all pasteboard actions
												// on the main thread
												[thePasteboard declareTypes:[NSArray arrayWithObject:NSTIFFPboardType] owner:self];
												[thePasteboard setData:theNSTIFFData forType:NSTIFFPboardType];
												// mark that we've successfully
												// imported the image and placed
												// it onto the clipboard
												gotImage=true;
											}

											[theNSImage release];
										}
									}
								}

								CFRelease(*thePB.importedImages);
							}
						}
					}
				}
			}
		}

		dlclose( pLib );

		if ( bUseICAImageImport )
			return;
	}

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		mpPanel = [[NSPanel alloc] initWithContentRect:NSMakeRect( 0, 0, 800, 500 ) styleMask:NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask backing:NSBackingStoreBuffered defer:YES];
		if ( mpPanel )
		{
			[mpPanel autorelease];
			[mpPanel setReleasedWhenClosed:NO];

			NSView *pContentView = [mpPanel contentView];
			if ( pContentView )
			{
				mpSplitView = [[NSSplitView alloc] initWithFrame:[pContentView bounds]];
				if ( mpSplitView )
				{
					[mpSplitView autorelease];
					[mpSplitView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
					[mpSplitView setDividerStyle:NSSplitViewDividerStyleThin];
					[mpSplitView setVertical:YES];
					[pContentView addSubview:mpSplitView];

					NSRect aSplitViewBounds = [mpSplitView bounds];
					mpDeviceBrowserView = [[IKDeviceBrowserView alloc] initWithFrame:aSplitViewBounds];
					if ( mpDeviceBrowserView )
					{
						[mpDeviceBrowserView autorelease];
						[mpSplitView addSubview:mpDeviceBrowserView];

						mpEmptyView = [[NSView alloc] initWithFrame:aSplitViewBounds];
						if ( mpEmptyView )
						{
							[mpEmptyView autorelease];
							[mpSplitView addSubview:mpEmptyView];
							[mpSplitView setPosition:aSplitViewBounds.size.width / 4 ofDividerAtIndex:0];

							mpCameraDeviceView = [[IKCameraDeviceView alloc] initWithFrame:[mpEmptyView bounds]];
							if ( mpCameraDeviceView )
							{
								[mpCameraDeviceView autorelease];
								[mpCameraDeviceView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

								mpScannerDeviceView = [[IKScannerDeviceView alloc] initWithFrame:[mpEmptyView bounds]];
								if ( mpScannerDeviceView )
								{
									[mpScannerDeviceView autorelease];
									[mpScannerDeviceView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

									[mpPanel setDelegate:self];
									mpDeviceBrowserView.delegate = self;
									@try
									{
										[pApp runModalForWindow:mpPanel];
									}
									@catch ( NSException *pExc )
									{
										// Close the window after catching an
										// exception as the device browser view
										// will likely crash
										[mpPanel close];
										if ( pExc )
											CFShow( pExc );
									}
								}
							}
						}
					}
				}
			}
		}
	}

	mpPanel = nil;
	mpSplitView = nil;
	mpDeviceBrowserView = nil;
	mpEmptyView = nil;
	mpCameraDeviceView = nil;
	mpScannerDeviceView = nil;
}

- (bool)capturedImage
{
	return(gotImage);
}

- (void)cameraDeviceView:(IKCameraDeviceView *)pCameraDeviceView didDownloadFile:(ICCameraFile *)pFile location:(NSURL *)pURL fileData:(NSData *)pFileData error:(NSError *)pError
{
	if ( !pCameraDeviceView )
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
			gotImage = true;

			// Close modal panel after pasting data to pasteboard
			if ( [pWindow isVisible] )
				[pWindow close];
		}
	}
}

- (void)cameraDeviceView:(IKCameraDeviceView *)pCameraDeviceView didEncounterError:(NSError *)pError
{
	ShowAlertWithError( pError );
}

- (void)cameraDeviceViewSelectionDidChange:(IKCameraDeviceView *)pCameraDeviceView
{
}

- (void)deviceBrowserView:(IKDeviceBrowserView *)pDeviceBrowserView didEncounterError:(NSError *)pError
{
	ShowAlertWithError( pError );
}

- (void)deviceBrowserView:(IKDeviceBrowserView *)pDeviceBrowserView selectionDidChange:(ICDevice *)pDevice
{
	// Do nothing if we aren't in running the modal panel
	if ( !pDeviceBrowserView || gotImage || !mpPanel || !mpSplitView || !mpDeviceBrowserView || !mpEmptyView || !mpCameraDeviceView || !mpScannerDeviceView )
		return;

	NSWindow *pWindow = [pDeviceBrowserView window];
	if ( !pWindow || pWindow != mpPanel )
		return;

	// Remove all device subviews from the empty view
	if ( [mpCameraDeviceView superview] )
	{
		ResetDeviceViewProperties( mpCameraDeviceView );
		[mpCameraDeviceView removeFromSuperview];
	}
	if ( [mpScannerDeviceView superview] )
	{
		ResetDeviceViewProperties( mpScannerDeviceView );
		[mpScannerDeviceView removeFromSuperview];
	}

	// Add a subview for the device in the empty view
	if ( pDevice && [pDevice isKindOfClass:[ICCameraDevice class]] )
	{
		[mpCameraDeviceView setFrame:[mpEmptyView bounds]];
		mpCameraDeviceView.cameraDevice = (ICCameraDevice *)pDevice;
		mpCameraDeviceView.delegate = self;
		[mpEmptyView addSubview:mpCameraDeviceView];
	}
	else if ( pDevice && [pDevice isKindOfClass:[ICScannerDevice class]] )
	{
		[mpScannerDeviceView setFrame:[mpEmptyView bounds]];
		mpScannerDeviceView.scannerDevice = (ICScannerDevice *)pDevice;
		mpScannerDeviceView.delegate = self;
		[mpEmptyView addSubview:mpScannerDeviceView];
	}
}

- (void)scannerDeviceView:(IKScannerDeviceView *)pScannerDeviceView didEncounterError:(NSError *)pError
{
	ShowAlertWithError( pError );
}

- (void)scannerDeviceView:(IKScannerDeviceView *)pScannerDeviceView didScanToURL:(NSURL *)pURL fileData:(NSData *)pFileData error:(NSError *)pError
{
	if ( !pScannerDeviceView )
		return;

	NSWindow *pWindow = [pScannerDeviceView window];
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
			gotImage = true;

			// Close modal panel after pasting data to pasteboard
			if ( [pWindow isVisible] )
				[pWindow close];
		}
	}
}

- (void)windowWillClose:(NSNotification *)pNotification
{
	// Set delegate to nil otherwise crashing will occur when pressing
	// the red window close button
	ResetDeviceViewProperties( mpDeviceBrowserView );
	ResetDeviceViewProperties( mpCameraDeviceView );
	ResetDeviceViewProperties( mpScannerDeviceView );

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
		[pApp stopModal];
}

@end
