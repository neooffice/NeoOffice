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
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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

#define _SV_SALINST_CXX

#include <sys/syslimits.h>

#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SALJAVA_H
#include <saljava.h>
#endif
#ifndef _SV_SALPTYPE_HXX
#include <salptype.hxx>
#endif
#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_SALPRN_HXX
#include <salprn.hxx>
#endif
#ifndef _VCL_APPTYPES_HXX
#include <apptypes.hxx>
#endif
#ifndef _SV_PRINT_H
#include <print.h>
#endif
#ifndef _SV_JOBSET_H
#include <jobset.h>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif
#ifndef _TOOLS_SIMPLERESMGR_HXX_
#include <tools/simplerm.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENT_HXX
#include <com/sun/star/vcl/VCLEvent.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFRAME_HXX
#include <com/sun/star/vcl/VCLFrame.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLIMAGE_HXX
#include <com/sun/star/vcl/VCLImage.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLPAGEFORMAT_HXX
#include <com/sun/star/vcl/VCLPageFormat.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif
#include "salinst.hrc"

#ifndef _OSL_PROCESS_H_
#include <rtl/process.h>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>
#undef check

typedef jobject Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance_Type( JNIEnv *, jobject );
typedef void Java_com_apple_mrj_macos_carbon_CarbonLock_init_Type( JNIEnv *, jobject );
typedef jint Java_com_apple_mrj_internal_awt_graphics_CGJavaPixelsPen_UpdateContext_Type( JNIEnv *, jobject, jint, jint, jint, jint, jint );

using namespace osl;
using namespace rtl;
using namespace vcl;
using namespace vos;
using namespace utl;
using namespace com::sun::star::uno;

static Mutex aMutex;
static OModule aJDirectModule;
static OModule aRealAWTModule;
static Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance_Type *pCarbonLockGetInstance = NULL;
static Java_com_apple_mrj_macos_carbon_CarbonLock_init_Type *pCarbonLockInit = NULL;
static Java_com_apple_mrj_internal_awt_graphics_CGJavaPixelsPen_UpdateContext_Type *pUpdateContext = NULL;

static jobject JNICALL Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance( JNIEnv *pEnv, jobject object );
static OSStatus CarbonEventHandler( EventHandlerCallRef aNextHandler, EventRef aEvent, void *pData );
static pascal OSErr DoAEQuit( const AppleEvent *message, AppleEvent *reply, long refcon );
static pascal OSErr DoAEOpenPrintDocuments( const AppleEvent *message, AppleEvent *reply, long refcon );
static pascal OSErr DoAEOpen( const AppleEvent *message, AppleEvent *reply, long refcon );
static pascal OSErr DoAEReopen( const AppleEvent *message, AppleEvent *reply, long refcon );

class SVMainThread : public ::vos::OThread
{
	Application*			mpApp;
	CFRunLoopRef			maRunLoop;

public:
							SVMainThread( Application* pApp, CFRunLoopRef aRunLoop ) : ::vos::OThread(), mpApp( pApp ), maRunLoop( aRunLoop ) {}

	virtual void			run();
};

// ============================================================================

static void RunAppMain( Application *pApp )
{
	SalData *pSalData = GetSalData();

	// Cache event queue
	pSalData->mpEventQueue = new com_sun_star_vcl_VCLEventQueue( NULL );

	// Cache font data
	pSalData->maFontMapping = com_sun_star_vcl_VCLFont::getAllFonts();
	SalGraphics aGraphics;
	ImplDevFontList aDevFontList;
	aGraphics.GetDevFontList( &aDevFontList );
	for ( ImplDevFontListData *pFontData = aDevFontList.First(); pFontData; pFontData = aDevFontList.Next() )
	{
		void *pNativeFont = ((com_sun_star_vcl_VCLFont *)pFontData->mpFirst->mpSysData)->getNativeFont();
		pSalData->maNativeFontMapping[ pNativeFont ] = pFontData->mpFirst;
	}

	EventHandlerUPP pEventHandlerUPP = NewEventHandlerUPP( CarbonEventHandler );
	if ( pEventHandlerUPP )
	{
		// Set up native event handler
		EventTypeSpec aTypes[6];
		aTypes[0].eventClass = kEventClassAppleEvent;
		aTypes[0].eventKind = kEventAppleEvent;
		aTypes[1].eventClass = kEventClassMouse;
		aTypes[1].eventKind = kEventMouseWheelMoved;
		aTypes[2].eventClass = kEventClassMenu;
		aTypes[2].eventKind = kEventMenuBeginTracking;
		aTypes[3].eventClass = kEventClassMenu;
		aTypes[3].eventKind = kEventMenuEndTracking;
		aTypes[4].eventClass = kEventClassApplication;
		aTypes[4].eventKind = kEventAppShown;
		aTypes[5].eventClass = kEventClassCommand;
		aTypes[5].eventKind = kEventProcessCommand;
		InstallApplicationEventHandler( pEventHandlerUPP, 6, aTypes, NULL, NULL );
	}

	// Install AppleEvent handlers for processing open and print events
	// to fix bug 209 
	AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP( DoAEQuit ), 0, FALSE );
	AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments, NewAEEventHandlerUPP( DoAEOpenPrintDocuments ), 0, FALSE );
	AEInstallEventHandler( kCoreEventClass, kAEPrintDocuments, NewAEEventHandlerUPP( DoAEOpenPrintDocuments ), 0, FALSE );
	AEInstallEventHandler( kCoreEventClass, kAEOpenApplication, NewAEEventHandlerUPP( DoAEOpen ), 0, FALSE );
	AEInstallEventHandler( kCoreEventClass, kAEReopenApplication, NewAEEventHandlerUPP( DoAEReopen ), 0, FALSE );

	// Fix bug 223 by registering a display manager notification callback
	ProcessSerialNumber nProc;
	if ( GetCurrentProcess( &nProc ) == noErr )
	{
		DMExtendedNotificationUPP pExtendedNotificationUPP = NewDMExtendedNotificationUPP( CarbonDMExtendedNotificationCallback );
		if ( pExtendedNotificationUPP )
			DMRegisterExtendedNotifyProc( pExtendedNotificationUPP, NULL, NULL, &nProc );
	}

	pApp->Main();
}

// ----------------------------------------------------------------------------

static void ImplFontListChangedCallback( ATSFontNotificationInfoRef, void* )
{
	MutexGuard aGuard( aMutex );

	// Get the array of fonts
	::std::list< ATSFontRef > aNewNativeFontList;
	BOOL bContinue = TRUE;
	while ( bContinue )
	{
		ATSFontIterator aIterator;
		ATSFontIteratorCreate( kATSFontContextLocal, NULL, NULL, kATSOptionFlagsUnRestrictedScope, &aIterator );
		for ( ; ; )
		{
			ATSFontRef aFont;
			OSStatus nErr = ATSFontIteratorNext( aIterator, &aFont );
			if ( nErr == kATSIterationCompleted )
			{
				bContinue = FALSE;
				break;
			}
			else if ( nErr == kATSIterationScopeModified )
			{
				aNewNativeFontList.clear();
				break;
			}
			else
			{
				aNewNativeFontList.push_back( aFont );
			}
		}
		ATSFontIteratorRelease( &aIterator );
	}

	// If any of the Java fonts have been disabled, use the default font
	BOOL bUseDefaultFont = FALSE;
	for ( ::std::list< void* >::const_iterator it = com_sun_star_vcl_VCLFont::validNativeFonts.begin(); it != com_sun_star_vcl_VCLFont::validNativeFonts.end(); ++it )
	{
		for ( ::std::list< ATSFontRef >::const_iterator nit = aNewNativeFontList.begin(); *nit != (ATSFontRef)( *it ) && nit != aNewNativeFontList.end(); ++nit )
			;

		if ( nit == aNewNativeFontList.end() )
		{
			bUseDefaultFont = TRUE;
			break;
		}
	}

	com_sun_star_vcl_VCLFont::useDefaultFont = bUseDefaultFont;
}

// ----------------------------------------------------------------------------

static jint JNICALL Java_com_apple_mrj_macos_carbon_CarbonLock_acquire0( JNIEnv *pEnv, jobject object )
{
	jobject lockObject = Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance( pEnv, object );
	if ( lockObject )
	{
		jint nRet = pEnv->MonitorEnter( lockObject );
		if ( pEnv->ExceptionOccurred() )
			pEnv->ExceptionClear();
		return nRet == JNI_OK ? 0 : 1;
	}
	else
	{
		return 1;
	}
}

// ----------------------------------------------------------------------------

static jobject JNICALL Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance( JNIEnv *pEnv, jobject object )
{
	if ( pCarbonLockGetInstance )
		return pCarbonLockGetInstance( pEnv, object );
	else
		return NULL;
}

// ----------------------------------------------------------------------------

static void JNICALL Java_com_apple_mrj_macos_carbon_CarbonLock_init( JNIEnv *pEnv, jobject object )
{
	if ( pCarbonLockInit )
		pCarbonLockInit( pEnv, object );
}

// ----------------------------------------------------------------------------

static jint JNICALL Java_com_apple_mrj_macos_carbon_CarbonLock_release0( JNIEnv *pEnv, jobject object )
{
	jobject lockObject = Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance( pEnv, object );
	if ( lockObject )
	{
		jint nRet = pEnv->MonitorExit( lockObject );
		if ( pEnv->ExceptionOccurred() )
			pEnv->ExceptionClear();
		return nRet == JNI_OK ? 0 : 1;
	}
	else
	{
		return 1;
	}
}

// ----------------------------------------------------------------------------

static jint JNICALL Java_com_apple_mrj_internal_awt_graphics_CGJavaPixelsPen_UpdateContext( JNIEnv *pEnv, jobject object, jint pContextRef, jint nX, jint nY, jint nWidth, jint nHeight )
{
	jint nRet = pUpdateContext( pEnv, object, pContextRef, nX, nY, nWidth, nHeight );

	CGContextRetain( (CGContextRef)nRet );

	return nRet;
}

// ----------------------------------------------------------------------------

static jint JNICALL Java_com_apple_mrj_internal_awt_graphics_CGSGraphics_CGContextRelease( JNIEnv *pEnv, jobject object, jint pContextRef )
{
	CGContextRef aContext = (CGContextRef)pContextRef;

	CFIndex nCount = CFGetRetainCount( aContext );
	jint nRet = ( nCount > 2 ? pContextRef : 0 );

	if ( nCount > 2 )
		nCount = 2;
	while ( nCount-- );
		CGContextRelease( aContext );

	return nRet;
}

// ----------------------------------------------------------------------------

static OSStatus CarbonEventHandler( EventHandlerCallRef aNextHandler, EventRef aEvent, void *pData )
{
	EventClass nClass = GetEventClass( aEvent );
	EventKind nKind = GetEventKind( aEvent );

	if ( nClass == kEventClassAppleEvent )
	{
		// Fix bug 209 by ignoring all Apple events that have not already been
		// handled by the JVM's handler
		OSType nType;
		if ( nKind == kEventAppleEvent && GetEventParameter( aEvent, kEventParamAEEventID, typeType, NULL, sizeof( OSType ), NULL, &nType ) == noErr && !Application::IsShutDown() )
		{
			switch ( nType )
			{
				case kAEQuitApplication:
				case kAEOpenDocuments:
				case kAEPrintDocuments:
				case kAEOpenApplication:
				case kAEReopenApplication:
				case kAEAbout:
				case 'mPRF':
					// Note that we can't actually get the Apple event from the
					// Carbon event. We must dispatch it to registered Apple
					// event handlers
					EventRecord eventRec;
					if ( ConvertEventRefToEventRecord( aEvent, &eventRec ) )
						AEProcessAppleEvent( &eventRec );
					break;
			}
		}

		return noErr;
	}
	else if ( !Application::IsShutDown() )
	{
		SalData *pSalData = GetSalData();

		if ( pSalData && pSalData->mpEventQueue )
		{
			if ( nClass == kEventClassMouse && nKind == kEventMouseWheelMoved )
			{
				EventMouseWheelAxis nAxis;
				if ( GetEventParameter( aEvent, kEventParamMouseWheelAxis, typeMouseWheelAxis, NULL, sizeof( EventMouseWheelAxis ), NULL, &nAxis ) == noErr && nAxis == kEventMouseWheelAxisY )
				{
					MacOSPoint aPoint;
					SInt32 nDelta;
					WindowRef aWindow;
					UInt32 nKeyModifiers;
					if ( GetEventParameter( aEvent, kEventParamWindowMouseLocation, typeQDPoint, NULL, sizeof( MacOSPoint ), NULL, &aPoint ) == noErr && GetEventParameter( aEvent, kEventParamMouseWheelDelta, typeSInt32, NULL, sizeof( SInt32 ), NULL, &nDelta ) == noErr && GetEventParameter( aEvent, kEventParamWindowRef, typeWindowRef, NULL, sizeof( WindowRef ), NULL, &aWindow ) == noErr && GetEventParameter( aEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof( UInt32 ), NULL, &nKeyModifiers ) == noErr )
					{
						// Unlock the Java lock
						ReleaseJavaLock();

						// Wakeup the event queue by sending it a dummy event
						com_sun_star_vcl_VCLEvent aEvent( SALEVENT_USEREVENT, NULL, NULL );
						pSalData->mpEventQueue->postCachedEvent( &aEvent );

						// Block the VCL event loop while checking mapping
						Application::GetSolarMutex().acquire();

						// Relock the Java lock
						AcquireJavaLock();

						for ( ::std::list< SalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
						{
							if ( (*it)->GetSystemData()->aWindow == (long)aWindow )
							{
								USHORT nModifiers = 0;
								if ( nKeyModifiers & controlKey )
									nModifiers |= KEY_MOD1;
								if ( nKeyModifiers & optionKey )
									nModifiers |= KEY_MOD2;
								if ( nKeyModifiers & shiftKey )
									nModifiers |= KEY_SHIFT;
								if ( nKeyModifiers & cmdKey )
									nModifiers |= KEY_CONTROLMOD;
								pSalData->mpEventQueue->postMouseWheelEvent( *it, 0, aPoint.h, aPoint.v, 1, nDelta * -1, nModifiers );
								break;
							}
						}

						// Unblock the VCL event loop
						Application::GetSolarMutex().release();
					}
				}

				return noErr;
			}
			else if ( nClass == kEventClassMenu && ( nKind == kEventMenuBeginTracking || nKind == kEventMenuEndTracking ) )
			{
				MenuRef trackingRef;
				if ( GetEventParameter( aEvent, kEventParamDirectObject, typeMenuRef, NULL, sizeof( MenuRef ), NULL, &trackingRef ) == noErr )
				{
					// According to Carbon documentation, the direct object
					// parameter should be NULL when tracking is beginning
					// in the menubar.  In reality, however, the direct
					// object is in fact a menu reference to the root
					// menu. To determine if we're a menubar tracking
					// event, we need to to compare against both NULL and
					// the root menu.
					MenuRef rootMenu = AcquireRootMenu(); // increments ref count
					bool isMenubar = false;

					if ( ( trackingRef == NULL ) || ( trackingRef == rootMenu ) )
						isMenubar = true;

					if ( rootMenu != NULL )
						ReleaseMenu( rootMenu );

					// Check if the front window is a native modal window as
					// we will deadlock when a native modal window is showing
					WindowRef aWindow = FrontWindow();
					WindowModality nModality;
					if ( isMenubar && aWindow && GetWindowModality( aWindow, &nModality, NULL ) == noErr && ( nModality == kWindowModalitySystemModal || nModality == kWindowModalityAppModal ) )
						isMenubar = false;
					
					if ( isMenubar )
					{
						// Unlock the Java lock
						ReleaseJavaLock();

						// Make sure condition is not already waiting
						if ( !pSalData->maNativeEventCondition.check() )
						{
							pSalData->maNativeEventCondition.wait();
							pSalData->maNativeEventCondition.set();
						}

						// Wakeup the event queue by sending it a dummy event
						com_sun_star_vcl_VCLEvent aEvent( SALEVENT_USEREVENT, NULL, NULL );
						pSalData->mpEventQueue->postCachedEvent( &aEvent );

						// Wait for all pending AWT events to be dispatched
						pSalData->mbNativeEventSucceeded = false;
						pSalData->maNativeEventCondition.reset();
						pSalData->maNativeEventCondition.wait();
						pSalData->maNativeEventCondition.set();

						// Fix bug 679 by checking if the condition was
						// released to avoid a deadlock
						if ( pSalData->mbNativeEventSucceeded )
						{
							Application::GetSolarMutex().acquire();

							if ( nKind == kEventMenuBeginTracking )
								pSalData->mbInNativeMenuTracking = true;
							else
								pSalData->mbInNativeMenuTracking = true;

							// Execute menu updates while the VCL event queue is
							// blocked
							UpdateMenusForFrame( pSalData->mpFocusFrame, NULL );

							// Relock the Java lock
							AcquireJavaLock();

							Application::GetSolarMutex().release();

							return noErr;
						}
						else
						{
							// Relock the Java lock
							AcquireJavaLock();

							return userCanceledErr;
						}
					}
				}
			}
			else if ( nClass == kEventClassApplication && nKind == kEventAppShown )
			{
				// Unlock the Java lock
				ReleaseJavaLock();

				Application::GetSolarMutex().acquire();

				for ( ::std::list< SalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
				{
					SalPaintEvent *pPaintEvent = new SalPaintEvent();
					pPaintEvent->mnBoundX = 0;
					pPaintEvent->mnBoundY = 0;
					pPaintEvent->mnBoundWidth = (*it)->maGeometry.nWidth;
					pPaintEvent->mnBoundHeight = (*it)->maGeometry.nHeight;
					com_sun_star_vcl_VCLEvent aVCLPaintEvent( SALEVENT_PAINT, *it, (void *)pPaintEvent );
					pSalData->mpEventQueue->postCachedEvent( &aVCLPaintEvent );
				}

				// Relock the Java lock
				AcquireJavaLock();

				Application::GetSolarMutex().release();
			}
			else if ( nClass == kEventClassCommand && nKind == kEventProcessCommand )
			{
				HICommandExtended aCommand;
				if ( GetEventParameter( aEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( HICommandExtended ), NULL, &aCommand ) == noErr )
				{
					USHORT nID = 0;
					switch ( aCommand.commandID )
					{
						case kHICommandAbout:
							nID = SALEVENT_ABOUT;
							break;
						case kHICommandPreferences:
							nID = SALEVENT_PREFS;
							break;
					}

					if ( nID )
					{
						com_sun_star_vcl_VCLEvent aEvent( nID, NULL, NULL );
						pSalData->mpEventQueue->postCachedEvent( &aEvent );
						return noErr;
					}
				}
			}
		}
	}

	// Always execute the next registered handler
	return CallNextEventHandler( aNextHandler, aEvent );
}

// ----------------------------------------------------------------------------

void CarbonDMExtendedNotificationCallback( void *pUserData, short nMessage, void *pNotifyData )
{
	if ( !Application::IsShutDown() && ( nMessage == kDMNotifyEvent || nMessage == kDMNotifyDisplayDidWake ) )
	{
		SalData *pSalData = GetSalData();
		if ( pSalData && pSalData->mpEventQueue )
		{
			// Unlock the Java lock
			ReleaseJavaLock();

			Application::GetSolarMutex().acquire();

			Rect aRect;
			for ( ::std::list< SalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
			{
				WindowRef aWindow = (WindowRef)( (*it)->GetSystemData()->aWindow );
				if ( aWindow && GetWindowBounds( aWindow, kWindowStructureRgn, &aRect ) == noErr )
					(*it)->maFrameData.mpVCLFrame->setBounds( (long)aRect.left, (long)aRect.top, (long)( aRect.right - aRect.left + 1 ), (long)( aRect.bottom - aRect.top + 1 ) );
			}

			// Relock the Java lock
			AcquireJavaLock();

			Application::GetSolarMutex().release();
		}
	}
}

// ----------------------------------------------------------------------------

static OSErr DoAEQuit( const AppleEvent *message, AppleEvent *reply, long refcon )
{
	if ( !Application::IsShutDown() )
	{
		SalData *pSalData = GetSalData();
		if ( pSalData && pSalData->mpEventQueue )
		{
			com_sun_star_vcl_VCLEvent aEvent( SALEVENT_SHUTDOWN, NULL, NULL );
			pSalData->mpEventQueue->postCachedEvent( &aEvent );
		}
	}

	return noErr;
}

// ----------------------------------------------------------------------------

static OSErr DoAEOpenPrintDocuments( const AppleEvent *message, AppleEvent *reply, long refcon )
{
	OSErr err = noErr;
	AEDesc theDesc;
	OSType eventID;
	DescType ignoreType;
	MacOSSize ignoreSize;

	if ( !Application::IsShutDown() )
	{
		SalData *pSalData = GetSalData();
		if ( pSalData && pSalData->mpEventQueue )
		{
			AEGetAttributePtr( message, keyEventIDAttr, typeType, &ignoreType, &eventID, sizeof( OSType ), &ignoreSize );

			err = AEGetParamDesc( message, keyDirectObject, typeAEList, &theDesc );
			if ( err == noErr )
			{
				long numFiles;
				err = AECountItems( &theDesc, &numFiles );
				if ( err == noErr )
				{
					for ( long i = 1; i <= numFiles; i++ )
					{
						FSSpec fileSpec;
						AEKeyword ignoreKeyword;

						err = AEGetNthPtr( &theDesc, i, typeFSS, &ignoreKeyword, &ignoreType, (void *)&fileSpec, sizeof(FSSpec), &ignoreSize );
						if ( err == noErr )
						{
							// Convert to a full path for our VCL event
							FSRef fileRef;
							err = FSpMakeFSRef( &fileSpec, &fileRef );
							if ( err == noErr )
							{
								char posixPath[PATH_MAX];
								memset( posixPath, '\0', PATH_MAX );
								err = FSRefMakePath( &fileRef, (UInt8 *)posixPath, sizeof(posixPath) );
								if ( err == noErr )
								{
									com_sun_star_vcl_VCLEvent aEvent( ( ( eventID == kAEOpenDocuments ) ? SALEVENT_OPENDOCUMENT : SALEVENT_PRINTDOCUMENT ), NULL, NULL, posixPath );
									pSalData->mpEventQueue->postCachedEvent( &aEvent );
								}
							}
						}

						// Don't continue processing if we have an error
						if ( err != noErr )
							break;
					}
				}

				AEDisposeDesc( &theDesc );
			}
		}
	}

	// Insert a reply containing any error code
	AEPutParamPtr( reply, 'errn', typeShortInteger, (void *)&err, sizeof(short) );

	return ( err );
}

// ----------------------------------------------------------------------------

static OSErr DoAEOpen( const AppleEvent *message, AppleEvent *reply, long refcon )
{
	// Fix bug 221 by explicitly reenabling all keyboards
	KeyScript( smKeyEnableKybds );

	return noErr;
}

// ----------------------------------------------------------------------------

static OSErr DoAEReopen( const AppleEvent *message, AppleEvent *reply, long refcon )
{
	// Fix bug 221 by explicitly reenabling all keyboards
	KeyScript( smKeyEnableKybds );

	if ( !Application::IsShutDown() )
	{
		SalData *pSalData = GetSalData();
		if ( pSalData && pSalData->mpEventQueue )
		{
			com_sun_star_vcl_VCLEvent aEvent( SALEVENT_ACTIVATE_APPLICATION, NULL, NULL );
			pSalData->mpEventQueue->postCachedEvent( &aEvent );
		}
	}

	return noErr;
}

// ----------------------------------------------------------------------------

static void SourceContextCallBack( void *pInfo )
{
}

// ----------------------------------------------------------------------------

void ExecuteApplicationMain( Application *pApp )
{
	// If there is a "user/fonts" directory, explicitly activate the
	// fonts since Panther does not automatically add fonts in the user's
	// Library/Fonts directory until they reboot or relogin
	OUString aUserStr;
	OUString aUserPath;
	if ( Bootstrap::locateUserInstallation( aUserStr ) == Bootstrap::PATH_EXISTS && osl_getSystemPathFromFileURL( aUserStr.pData, &aUserPath.pData ) == osl_File_E_None )
	{
		ByteString aFontDir( aUserPath.getStr(), RTL_TEXTENCODING_UTF8 );
		if ( aFontDir.Len() )
		{
			aFontDir += ByteString( "/user/fonts", RTL_TEXTENCODING_UTF8 );
			FSRef aFontPath;
			FSSpec aFontSpec;
			if ( FSPathMakeRef( (const UInt8 *)aFontDir.GetBuffer(), &aFontPath, 0 ) == noErr && FSGetCatalogInfo( &aFontPath, kFSCatInfoNone, NULL, NULL, &aFontSpec, NULL) == noErr )
				ATSFontActivateFromFileSpecification( &aFontSpec, kATSFontContextLocal, kATSFontFormatUnspecified, NULL, kATSOptionFlagsDefault, NULL );
		}
	}

	// If there is a "share/fonts/truetype" directory, explicitly activate the
	// fonts since Panther does not automatically add fonts in the user's
	// Library/Fonts directory until they reboot or relogin
	OUString aExecStr;
	OUString aExecPath;
	if ( osl_getExecutableFile( &aExecStr.pData ) == osl_Process_E_None && osl_getSystemPathFromFileURL( aExecStr.pData, &aExecPath.pData ) == osl_File_E_None )
	{
		ByteString aFontDir( aExecPath.getStr(), RTL_TEXTENCODING_UTF8 );
		if ( aFontDir.Len() )
		{
			DirEntry aFontDirEntry( aFontDir );
			aFontDirEntry.ToAbs();
			aFontDir = ByteString( aFontDirEntry.GetPath().GetFull(), RTL_TEXTENCODING_UTF8 );
			aFontDir += ByteString( "/../share/fonts/truetype", RTL_TEXTENCODING_UTF8 );
			FSRef aFontPath;
			FSSpec aFontSpec;
			if ( FSPathMakeRef( (const UInt8 *)aFontDir.GetBuffer(), &aFontPath, 0 ) == noErr && FSGetCatalogInfo( &aFontPath, kFSCatInfoNone, NULL, NULL, &aFontSpec, NULL) == noErr )
				ATSFontActivateFromFileSpecification( &aFontSpec, kATSFontContextLocal, kATSFontFormatUnspecified, NULL, kATSOptionFlagsDefault, NULL );
		}
	}

	ATSFontIterator aIterator;

	// Get the array of fonts
	BOOL bContinue = TRUE;
	while ( bContinue )
	{
		MutexGuard aGuard( aMutex );

		::std::list< CFStringRef > aFontNameList;
		ATSFontIteratorCreate( kATSFontContextLocal, NULL, NULL, kATSOptionFlagsUnRestrictedScope, &aIterator );
		for ( ; ; )
		{
			ATSFontRef aFont;
			OSStatus nErr = ATSFontIteratorNext( aIterator, &aFont );
			if ( nErr == kATSIterationCompleted )
			{
				// Register notification callback
				ATSFontNotificationSubscribe( (ATSNotificationCallback)ImplFontListChangedCallback, kATSFontNotifyOptionReceiveWhileSuspended, NULL, NULL );
				bContinue = FALSE;
				break;
			}
			else if ( nErr == kATSIterationScopeModified )
			{
				com_sun_star_vcl_VCLFont::validNativeFonts.clear();
				break;
			}
			else
			{
				// Eliminate duplicate font names
				CFStringRef aFontNameRef;
				if ( ATSFontGetName( aFont, kATSOptionFlagsDefault, &aFontNameRef ) == noErr )
				{
					for ( ::std::list< CFStringRef >::const_iterator sit = aFontNameList.begin(); sit != aFontNameList.end(); ++sit )
					{
						if ( CFStringCompare( *sit, aFontNameRef, kCFCompareCaseInsensitive ) == kCFCompareEqualTo )
							break;
					}

					if ( sit == aFontNameList.end() )
					{
						aFontNameList.push_back( aFontNameRef );
						com_sun_star_vcl_VCLFont::validNativeFonts.push_back( (void *)aFont );
					}
					else
					{
						CFRelease( aFontNameRef );
					}
				}
			}
		}
		ATSFontIteratorRelease( &aIterator );

		for ( ::std::list< CFStringRef >::const_iterator sit = aFontNameList.begin(); sit != aFontNameList.end(); ++sit )
			CFRelease( *sit );
		aFontNameList.clear();
	}

	VCLThreadAttach t;
	if ( t.pEnv )
	{
		SalData *pSalData = GetSalData();

		// Test the JVM version and if it is 1.4 or higher use Cocoa, otherwise
		// use Carbon
		if ( t.pEnv->GetVersion() >= JNI_VERSION_1_4 )
		{
			// Create the thread to run the Main() method in
			SVMainThread aSVMainThread( pApp, CFRunLoopGetCurrent() );
			aSVMainThread.create();

			ULONG nCount = Application::ReleaseSolarMutex();

			// Start the CFRunLoop
			CFRunLoopSourceContext aSourceContext;
			aSourceContext.version = 0;
			aSourceContext.info = NULL;
			aSourceContext.retain = NULL;
			aSourceContext.release = NULL;
			aSourceContext.copyDescription = NULL;
			aSourceContext.equal = NULL;
			aSourceContext.hash = NULL;
			aSourceContext.schedule = NULL;
			aSourceContext.cancel = NULL;
			aSourceContext.perform = &SourceContextCallBack;
			CFRunLoopSourceRef aSourceRef = CFRunLoopSourceCreate( NULL, 0, &aSourceContext );
			CFRunLoopAddSource( CFRunLoopGetCurrent(), aSourceRef, kCFRunLoopCommonModes );
			CFRunLoopRun();

			aSVMainThread.join();

			Application::AcquireSolarMutex( nCount );

			return;
		}
		else
		{
			// Panther expects applications to run their event loop in main
			// thread and Java 1.3.1 runs its event loop in a separate thread.
			// So, we need to disable the lock that Java 1.3.1 uses.
			jclass systemClass = t.pEnv->FindClass( "java/lang/System" );
			if ( systemClass )
			{
				// Find libJDirect.jnilib
				jmethodID mID = NULL;
				OUString aJavaHomePath;
				if ( !mID )
				{
					char *cSignature = "(Ljava/lang/String;)Ljava/lang/String;";
					mID = t.pEnv->GetStaticMethodID( systemClass, "getProperty", cSignature );
				}
				OSL_ENSURE( mID, "Unknown method id!" );
				if ( mID )
				{
					jvalue args[1];
					args[0].l = StringToJavaString( t.pEnv, OUString::createFromAscii( "java.home" ) );
					jstring out;
					out = (jstring)t.pEnv->CallStaticObjectMethodA( systemClass, mID, args );
					if ( out )
						aJavaHomePath = JavaString2String( t.pEnv, out );
				}

				// Load libJDirect.jnilib and librealawt.jnilib and cache
				// symbols
				if ( aJavaHomePath.getLength() )
				{
					OUString aJDirectPath( aJavaHomePath );
					aJDirectPath += OUString::createFromAscii( "/../Libraries/libJDirect.jnilib" );
					if ( aJDirectModule.load( aJDirectPath ) )
					{
						pCarbonLockGetInstance = (Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance_Type *)aJDirectModule.getSymbol( OUString::createFromAscii( "Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance" ) );
						pCarbonLockInit = (Java_com_apple_mrj_macos_carbon_CarbonLock_init_Type *)aJDirectModule.getSymbol( OUString::createFromAscii( "Java_com_apple_mrj_macos_carbon_CarbonLock_init" ) );
					}

					OUString aRealAWTPath( aJavaHomePath );
					aRealAWTPath += OUString::createFromAscii( "/../Libraries/librealawt.jnilib" );
					if ( aRealAWTModule.load( aRealAWTPath ) )
						pUpdateContext = (Java_com_apple_mrj_internal_awt_graphics_CGJavaPixelsPen_UpdateContext_Type *)aRealAWTModule.getSymbol( OUString::createFromAscii( "Java_com_apple_mrj_internal_awt_graphics_CGJavaPixelsPen_UpdateContext" ) );
				}

			}

			jclass carbonLockClass = t.pEnv->FindClass( "com/apple/mrj/macos/carbon/CarbonLock" );
			jclass cgJavaPixelsPenClass = t.pEnv->FindClass( "com/apple/mrj/internal/awt/graphics/CGJavaPixelsPen" );
			jclass cgsGraphicsClass = t.pEnv->FindClass( "com/apple/mrj/internal/awt/graphics/CGSGraphics" );
			if ( carbonLockClass && pCarbonLockGetInstance && pCarbonLockInit && cgJavaPixelsPenClass && pUpdateContext && cgsGraphicsClass )
			{
				// Reregister the native methods
				JNINativeMethod pMethods[4];
				pMethods[0].name = "acquire0";
				pMethods[0].signature = "()I";
				pMethods[0].fnPtr = (void *)Java_com_apple_mrj_macos_carbon_CarbonLock_acquire0;
				pMethods[1].name = "getInstance";
				pMethods[1].signature = "()Ljava/lang/Object;";
				pMethods[1].fnPtr = (void *)Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance;
				pMethods[2].name = "init";
				pMethods[2].signature = "()V";
				pMethods[2].fnPtr = (void *)Java_com_apple_mrj_macos_carbon_CarbonLock_init;
				pMethods[3].name = "release0";
				pMethods[3].signature = "()I";
				pMethods[3].fnPtr = (void *)Java_com_apple_mrj_macos_carbon_CarbonLock_release0;
				t.pEnv->RegisterNatives( carbonLockClass, pMethods, 4 );

				pMethods[0].name = "UpdateContext";
				pMethods[0].signature = "(IIIII)I";
				pMethods[0].fnPtr = (void *)Java_com_apple_mrj_internal_awt_graphics_CGJavaPixelsPen_UpdateContext;
				t.pEnv->RegisterNatives( cgJavaPixelsPenClass, pMethods, 1 );

				pMethods[0].name = "CGContextRelease";
				pMethods[0].signature = "(I)I";
				pMethods[0].fnPtr = (void *)Java_com_apple_mrj_internal_awt_graphics_CGSGraphics_CGContextRelease;
				t.pEnv->RegisterNatives( cgsGraphicsClass, pMethods, 1 );

				// Peek for a Carbon event. This is enough to solve the
				// keyboard layout switching problem on Panther.
				ReceiveNextEvent( 0, NULL, 0, false, NULL );

				// We need to be fill in the static sFonts and sNumFonts fields
				// in the NativeFontWrapper class as the JVM's implementation
				// will include disabled fonts will can crash the application
				jclass nativeFontWrapperClass = t.pEnv->FindClass( "sun/awt/font/NativeFontWrapper" );
				if ( nativeFontWrapperClass )
				{
					static jfieldID fIDFonts = NULL;
					static jfieldID fIDNumFonts = NULL;
					static jmethodID mIDPutNativeQuartzFontIntoCache = NULL;
					if ( !fIDFonts )
					{
						char *cSignature = "[I";
						fIDFonts = t.pEnv->GetStaticFieldID( nativeFontWrapperClass, "sFonts", cSignature );
					}
					OSL_ENSURE( fIDFonts, "Unknown field id!" );
					if ( !fIDNumFonts )
					{
						char *cSignature = "I";
						fIDNumFonts = t.pEnv->GetStaticFieldID( nativeFontWrapperClass, "sNumFonts", cSignature );
					}
					OSL_ENSURE( fIDNumFonts, "Unknown field id!" );
					if ( !mIDPutNativeQuartzFontIntoCache )
					{
						char *cSignature = "(Ljava/lang/String;[I)V";
						mIDPutNativeQuartzFontIntoCache = t.pEnv->GetStaticMethodID( nativeFontWrapperClass, "putNativeQuartzFontIntoCache", cSignature );
					}
					OSL_ENSURE( mIDPutNativeQuartzFontIntoCache, "Unknown method id!" );
					if ( fIDFonts && fIDNumFonts && mIDPutNativeQuartzFontIntoCache )
					{
						ClearableMutexGuard aGuard( aMutex );

						// Create the font array and add the fonts in reverse
						// order since we originally stored them that way
						jsize nFonts  = com_sun_star_vcl_VCLFont::validNativeFonts.size();
						jintArray pFonts = t.pEnv->NewIntArray( nFonts );
						jintArray pFontTypes = t.pEnv->NewIntArray( 4 );
						jboolean bCopy( sal_False );
						jint *pData = t.pEnv->GetIntArrayElements( pFonts, &bCopy );
						jsize i = 0;
						for ( ::std::list< void* >::const_iterator it = com_sun_star_vcl_VCLFont::validNativeFonts.begin(); it != com_sun_star_vcl_VCLFont::validNativeFonts.end(); ++it )
						{
							ATSFontRef aFontRef = (ATSFontRef)( *it );

							jint nCGFont = (jint)CGFontCreateWithPlatformFont( (void *)&aFontRef );
							pData[i++] = nCGFont;

							CFStringRef aFontNameRef;
							if ( ATSFontGetName( aFontRef, kATSOptionFlagsDefault, &aFontNameRef ) == noErr )
							{
								sal_Int32 nBufSize = CFStringGetLength( aFontNameRef );
								sal_Unicode aBuf[ nBufSize ];
								CFRange aRange;

								aRange.location = 0;
								aRange.length = nBufSize;
								CFStringGetCharacters( aFontNameRef, aRange, aBuf );
								CFRelease( aFontNameRef );

								jstring fontName = t.pEnv->NewString( aBuf, nBufSize );
								if ( fontName )
								{
									jboolean bCopy( sal_False );
									jint *pDataTypes = t.pEnv->GetIntArrayElements( pFontTypes, &bCopy );
									pDataTypes[ 0 ] = nCGFont;
									pDataTypes[ 1 ] = nCGFont;
									pDataTypes[ 2 ] = nCGFont;
									pDataTypes[ 3 ] = nCGFont;
									t.pEnv->ReleaseIntArrayElements( pFontTypes, pDataTypes, 0 );

									jvalue args[2];
									args[0].l = fontName;
									args[1].l = pFontTypes;
									t.pEnv->CallStaticVoidMethodA( nativeFontWrapperClass, mIDPutNativeQuartzFontIntoCache, args );
								}
							}
						}
						t.pEnv->ReleaseIntArrayElements( pFonts, pData, 0 );

						aGuard.clear();

						// Save the font data
						t.pEnv->SetStaticObjectField( nativeFontWrapperClass, fIDFonts, pFonts );
						t.pEnv->SetStaticIntField( nativeFontWrapperClass, fIDNumFonts, nFonts );
					}
				}
			}
		}
	}

	// Now that Java is properly initialized, run the application's Main()
	RunAppMain( pApp );
}

// -----------------------------------------------------------------------

void AcquireJavaLock()
{
	// Lock the Carbon lock
	VCLThreadAttach t;
	if ( t.pEnv && t.pEnv->GetVersion() < JNI_VERSION_1_4 )
		Java_com_apple_mrj_macos_carbon_CarbonLock_acquire0( t.pEnv, NULL );
}

// -----------------------------------------------------------------------

void ReleaseJavaLock()
{
	// Unlock the Carbon lock
	VCLThreadAttach t;
	if ( t.pEnv && t.pEnv->GetVersion() < JNI_VERSION_1_4 )
		Java_com_apple_mrj_macos_carbon_CarbonLock_release0( t.pEnv, NULL );
}

// ============================================================================

void SVMainThread::run()
{
	Application::GetSolarMutex().acquire();
	RunAppMain( mpApp );
	CFRunLoopStop( maRunLoop );
	Application::GetSolarMutex().release();
}

// =======================================================================

void SalAbort( const XubString& rErrorText )
{
	if ( !rErrorText.Len() )
		fprintf( stderr, "Application Error" );
	else
		fprintf( stderr, ByteString( rErrorText, gsl_getSystemTextEncoding() ).GetBuffer() );
	fprintf( stderr, "\n" );
	abort();
}

// -----------------------------------------------------------------------

void InitSalData()
{
	SalData *pSalData = new SalData();
	SetSalData( pSalData );
}

// -----------------------------------------------------------------------

void DeInitSalData()
{
	SalData *pSalData = GetSalData();
	SetSalData( NULL );
	delete pSalData;
}

// -----------------------------------------------------------------------

void InitSalMain()
{
}

// -----------------------------------------------------------------------

void DeInitSalMain()
{
}

// -----------------------------------------------------------------------

void SetFilterCallback( void* pCallback, void* pInst )
{
	SalData* pSalData = GetSalData();
	pSalData->mpFirstInstance->maInstData.mpFilterCallback = pCallback;
	pSalData->mpFirstInstance->maInstData.mpFilterInst = pInst;
}

// -----------------------------------------------------------------------

SalInstance* CreateSalInstance()
{
	SalData *pSalData = GetSalData();

	SalInstance *pInst = new SalInstance();
	pSalData->mpFirstInstance = pInst;

	return pInst;
}

// -----------------------------------------------------------------------

void DestroySalInstance( SalInstance* pInst )
{
	SalData *pSalData = GetSalData();

	if ( pSalData->mpFirstInstance == pInst )
		pSalData->mpFirstInstance = NULL;

	if ( pInst )
		delete pInst;
}

// =======================================================================

SalInstance::SalInstance()
{
	maInstData.mpSalYieldMutex = new SalYieldMutex();
	maInstData.mpSalYieldMutex->acquire();
}

// -----------------------------------------------------------------------

SalInstance::~SalInstance()
{
	maInstData.mpSalYieldMutex->release();
	if ( maInstData.mpSalYieldMutex )
		delete maInstData.mpSalYieldMutex;
}

// -----------------------------------------------------------------------

IMutex* SalInstance::GetYieldMutex()
{
	return maInstData.mpSalYieldMutex;
}

// -----------------------------------------------------------------------

ULONG SalInstance::ReleaseYieldMutex()
{
	SalYieldMutex* pYieldMutex = maInstData.mpSalYieldMutex;
	if ( pYieldMutex->GetThreadId() == OThread::getCurrentIdentifier() )
	{
		ULONG nCount = pYieldMutex->GetAcquireCount();
		ULONG n = nCount;
		while ( n )
		{
			pYieldMutex->release();
			n--;
		}
		return nCount;
	}
	else
	{
		return 0;
	}
}

// -----------------------------------------------------------------------

void SalInstance::AcquireYieldMutex( ULONG nCount )
{
	SalYieldMutex* pYieldMutex = maInstData.mpSalYieldMutex;
	while ( nCount )
	{
		pYieldMutex->acquire();
		nCount--;
	}
}

// -----------------------------------------------------------------------

void SalInstance::Yield( BOOL bWait )
{
	static USHORT nRecursionLevel = 0;
	static com_sun_star_vcl_VCLEvent *pPendingEvent = NULL;
	SalData *pSalData = GetSalData();
	com_sun_star_vcl_VCLEvent *pEvent;

	nRecursionLevel++;

	// Dispatch pending non-AWT events
	if ( ( pEvent = pSalData->mpEventQueue->getNextCachedEvent( 0, FALSE ) ) != NULL )
	{
		bool bReturn = true;
		USHORT nID = pEvent->getID();
		if ( nID == SALEVENT_SHUTDOWN )
		{
			// Ignore SALEVENT_SHUTDOWN events when recursing into this
			// method or when in presentation mode
			ImplSVData *pSVData = ImplGetSVData();
			if ( nRecursionLevel == 1 && !pSVData->maWinData.mpFirstFloat && !pSVData->maWinData.mpLastExecuteDlg && !pSalData->mpPresentationFrame )
				pEvent->dispatch();
		}
		else if ( nID == SALEVENT_OPENDOCUMENT || nID == SALEVENT_PRINTDOCUMENT )
		{
			// Fix bug 168 && 607 by reposting SALEVENT_*DOCUMENT events when
			// recursing into this method while opening a document
			if ( nRecursionLevel == 1 && !ImplGetSVData()->maWinData.mpLastExecuteDlg && !pSalData->mpPresentationFrame )
			{
				pEvent->dispatch();
			}
			else
			{
				com_sun_star_vcl_VCLEvent aEvent( pEvent->getJavaObject() );
				pSalData->mpEventQueue->postCachedEvent( &aEvent );
				bReturn = false;
			}
		}
		else
		{
			pEvent->dispatch();
		}
		delete pEvent;
		pEvent = NULL;

		if ( bReturn )
		{
			nRecursionLevel--;
			return;
		}
	}

	ULONG nCount = ReleaseYieldMutex();
	if ( !bWait )
		OThread::yield();
	AcquireYieldMutex( nCount );

	// Check timer
	if ( pSalData->mnTimerInterval )
	{
		timeval aCurrentTime;
		gettimeofday( &aCurrentTime, NULL );
		if ( pSalData->mpTimerProc && aCurrentTime >= pSalData->maTimeout )
		{
			if ( pSalData->mpPresentationFrame )
				pSalData->mpPresentationFrame->maFrameData.mpVCLFrame->setAutoFlush( TRUE );

			gettimeofday( &pSalData->maTimeout, NULL );
			pSalData->maTimeout += pSalData->mnTimerInterval;
			pSalData->mpTimerProc();

			if ( pSalData->mpPresentationFrame )
				pSalData->mpPresentationFrame->maFrameData.mpVCLFrame->setAutoFlush( FALSE );

			// Flush all of the window buffers to the native windows and
			// synchronize native menus
			for ( ::std::list< SalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
				(*it)->Flush();
		}
	}

	// Determine timeout
	ULONG nTimeout = 0;
	if ( bWait && pSalData->mnTimerInterval && pSalData->maNativeEventCondition.check() && !Application::IsShutDown() )
	{
		timeval aTimeout;

		gettimeofday( &aTimeout, NULL );
		if ( pSalData->maTimeout > aTimeout )
		{
			aTimeout = pSalData->maTimeout - aTimeout;
			nTimeout = aTimeout.tv_sec * 1000 + aTimeout.tv_usec / 1000;
		}

		// Prevent excessively short timeouts
		if ( nTimeout < 10 )
			nTimeout = 10;
	}

	// Dispatch pending AWT events
	if ( pPendingEvent )
	{
		pEvent = pPendingEvent;
		pPendingEvent = NULL;
	}
	nCount = ReleaseYieldMutex();
	if ( !pEvent )
		pEvent = pSalData->mpEventQueue->getNextCachedEvent( nTimeout, TRUE );
	AcquireYieldMutex( nCount );
	if ( pEvent )
	{
		USHORT nID = pEvent->getID();
		pEvent->dispatch();
		delete pEvent;

		// We cannot avoid bug 437 if we allow the timer to run between
		// consecutive mouse button down and up events
		if ( nID == SALEVENT_MOUSEBUTTONDOWN )
		{
			pEvent = pSalData->mpEventQueue->getNextCachedEvent( 0, TRUE );
			if ( pEvent )
			{
				if ( pEvent->getID() == SALEVENT_MOUSEBUTTONUP )
				{
					pEvent->dispatch();
					delete pEvent;
				}
				else
				{
					// Dispatch it the next time through
					pPendingEvent = pEvent;
				}
			}
		}
	}

	// Allow Carbon event loop to proceed
	if ( !pEvent && !pSalData->maNativeEventCondition.check() )
	{
		pSalData->mbNativeEventSucceeded = true;
		pSalData->maNativeEventCondition.set();
		nCount = ReleaseYieldMutex();
		OThread::yield();
		AcquireYieldMutex( nCount );
	}

	nRecursionLevel--;
}

// -----------------------------------------------------------------------

BOOL SalInstance::AnyInput( USHORT nType )
{
	BOOL bRet = FALSE;

	if ( nType & INPUT_TIMER )
	{
		// Check timer
		SalData *pSalData = GetSalData();
		if ( pSalData->mnTimerInterval )
		{
			timeval aCurrentTime;
			gettimeofday( &aCurrentTime, NULL );
			if ( pSalData->mpTimerProc && aCurrentTime >= pSalData->maTimeout )
				bRet = TRUE;
		}
	}

	if ( !bRet )
		bRet = (BOOL)GetSalData()->mpEventQueue->anyCachedEvent( nType );

	return bRet;
}

// -----------------------------------------------------------------------

SalFrame* SalInstance::CreateChildFrame( SystemParentData* pSystemParentData, ULONG nSalFrameStyle )
{
#ifdef DEBUG
	fprintf( stderr, "SalInstance::CreateChildFrame not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

SalFrame* SalInstance::CreateFrame( SalFrame* pParent, ULONG nSalFrameStyle )
{
	SalFrame *pFrame = new SalFrame();

	pFrame->maFrameData.mpVCLFrame = new com_sun_star_vcl_VCLFrame( nSalFrameStyle, pFrame, pParent );
	pFrame->maFrameData.maSysData.aWindow = 0;
	pFrame->maFrameData.maSysData.pSalFrame = pFrame;
	pFrame->maFrameData.mnStyle = nSalFrameStyle;

	pFrame->SetParent( pParent );

	// Insert this window into the window list
	SalData *pSalData = GetSalData();
	pSalData->maFrameList.push_front( pFrame );

	// Cache the insets
	Rectangle aRect = pFrame->maFrameData.mpVCLFrame->getInsets();
	pFrame->maGeometry.nLeftDecoration = aRect.nLeft;
	pFrame->maGeometry.nTopDecoration = aRect.nTop;
	pFrame->maGeometry.nRightDecoration = aRect.nRight;
	pFrame->maGeometry.nBottomDecoration = aRect.nBottom;

	// Set default window size based on style
	Rectangle aWorkArea;
	if ( pFrame->maFrameData.mpParent )
		pFrame->maFrameData.mpParent->GetWorkArea( aWorkArea );
	else
		pFrame->GetWorkArea( aWorkArea );
	long nX = aWorkArea.nLeft;
	long nY = aWorkArea.nTop;
	long nWidth = aWorkArea.GetWidth();
	long nHeight = aWorkArea.GetHeight();
	if ( nSalFrameStyle & SAL_FRAME_STYLE_FLOAT )
	{
		nWidth = 10;
		nHeight = 10;
	}
	else
	{
		if ( nSalFrameStyle & SAL_FRAME_STYLE_SIZEABLE && nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE )
		{
			nWidth = (long)( nWidth * 0.8 );
			nHeight = (long)( nHeight * 0.8 );
		}
		if ( !pFrame->maFrameData.mpParent )
		{
			// Find the next document window if any exist
			SalFrame* pNextFrame = NULL;
			for ( ::std::list< SalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
			{
				if ( (*it) && (*it) != pFrame &&
					! (*it)->maFrameData.mpParent &&
					(*it)->maFrameData.mnStyle != SAL_FRAME_STYLE_DEFAULT &&
					(*it)->maFrameData.mnStyle & SAL_FRAME_STYLE_SIZEABLE &&
					(*it)->GetGeometry().nWidth &&
					(*it)->GetGeometry().nHeight )
						pNextFrame = *it;
			}

			if ( pNextFrame )
			{
				// Set screen to same screen as next frame
				pNextFrame->GetWorkArea( aWorkArea );
				pFrame->maFrameData.mbCenter = FALSE;
				const SalFrameGeometry& rGeom( pNextFrame->GetGeometry() );
				nX = rGeom.nX - rGeom.nLeftDecoration;
				nY = rGeom.nY - rGeom.nTopDecoration;
				nWidth = rGeom.nWidth + rGeom.nLeftDecoration + rGeom.nRightDecoration;
				nHeight = rGeom.nHeight + rGeom.nTopDecoration + rGeom.nBottomDecoration;
			}
		}
	}

	long nFlags = SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT;
	if ( !pFrame->maFrameData.mbCenter )
		nFlags |= SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y;
	pFrame->SetPosSize( nX, nY, nWidth, nHeight, nFlags );

	// Reset mbCenter flag to default value
	pFrame->maFrameData.mbCenter = TRUE;

	return pFrame;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyFrame( SalFrame* pFrame )
{
	// Remove this window from the window list
	if ( pFrame )
	{
		pFrame->SetParent( NULL );

		if ( pFrame->maFrameData.mbVisible )
			pFrame->Show( FALSE );

		SalData *pSalData = GetSalData();
		pSalData->maFrameList.remove( pFrame );

		delete pFrame;
	}
}

// -----------------------------------------------------------------------

SalObject* SalInstance::CreateObject( SalFrame* pParent )
{
#ifdef DEBUG
	fprintf( stderr, "SalInstance::CreateObject not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyObject( SalObject* pObject )
{
#ifdef DEBUG
	fprintf( stderr, "SalInstance::DestroyObject not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void SalInstance::SetEventCallback( void* pInstance, bool(*pCallback)(void*,void*,int) )
{
}

// -----------------------------------------------------------------------

void SalInstance::SetErrorEventCallback( void* pInstance, bool(*pCallback)(void*,void*,int) )
{
}

// -----------------------------------------------------------------------

void* SalInstance::GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes )
{
	rReturnedBytes = 1;
	rReturnedType = AsciiCString;
	return (void *)"";
}

// -----------------------------------------------------------------------

void SalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
	// Create a dummy queue for our dummy default printer
	SalPrinterQueueInfo *pInfo = new SalPrinterQueueInfo();
	pInfo->maPrinterName = GetDefaultPrinter();
	pInfo->mpSysData = NULL;
	pList->Add( pInfo );
}

// -----------------------------------------------------------------------

void SalInstance::GetPrinterQueueState( SalPrinterQueueInfo* pInfo )
{
}

// -----------------------------------------------------------------------

void SalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
	if ( pInfo )
		delete pInfo;
}

// -----------------------------------------------------------------------

SalInfoPrinter* SalInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                                ImplJobSetup* pSetupData )
{
	SalData *pSalData = GetSalData();

	// Create a dummy printer configuration for our dummy printer
	SalInfoPrinter *pPrinter = new SalInfoPrinter();

	// Populate data
	pSetupData->mnSystem = JOBSETUP_SYSTEM_JAVA;
	pSetupData->maPrinterName = pQueueInfo->maPrinterName;
	pSetupData->maDriver = pQueueInfo->maDriver;

	// Check driver data
	if ( pSetupData->mpDriverData )
	{
		if ( pSetupData->mnSystem != JOBSETUP_SYSTEM_JAVA || pSetupData->mnDriverDataLen != sizeof( SalDriverData ) )
		{
			rtl_freeMemory( pSetupData->mpDriverData );
			pSetupData->mpDriverData = NULL;
			pSetupData->mnDriverDataLen = 0;
		}
		else
		{
			SalDriverData *pDriverData = (SalDriverData *)pSetupData->mpDriverData;
			if ( !pDriverData->mpVCLPageFormat )
			{
				delete (SalDriverData *)pSetupData->mpDriverData;
				pSetupData->mpDriverData = NULL;
				pSetupData->mnDriverDataLen = 0;
			}
		}
	}

	// Set driver data
	if ( !pSetupData->mpDriverData )
	{
		SalDriverData *pDriverData = new SalDriverData();
		pDriverData->mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat();
		pSetupData->mpDriverData = (BYTE *)pDriverData;
		pSetupData->mnDriverDataLen = sizeof( SalDriverData );
	}

	// Create a new page format instance that points to the same Java object
	pPrinter->maPrinterData.mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat( ((SalDriverData *)pSetupData->mpDriverData)->mpVCLPageFormat->getJavaObject() );

	// Update values
	pPrinter->SetData( 0, pSetupData );

	return pPrinter;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
	if ( pPrinter )
		delete pPrinter;
}

// -----------------------------------------------------------------------

XubString SalInstance::GetDefaultPrinter()
{
	// Create a dummy default printer
	SalData *pSalData = GetSalData();
	if ( !pSalData->maDefaultPrinter.Len() )
	{
		SimpleResMgr *pResMgr = SimpleResMgr::Create( CREATEVERSIONRESMGR_NAME( salapp ) );
		if ( pResMgr )
		{
			pSalData->maDefaultPrinter = XubString( pResMgr->ReadString( DEFAULT_PRINTER ) );
			delete pResMgr;
		}
	}
	return pSalData->maDefaultPrinter;
}

// -----------------------------------------------------------------------

SalPrinter* SalInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
	SalPrinter *pPrinter = new SalPrinter();
	return pPrinter;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
	if ( pPrinter )
		delete pPrinter;
}

// -----------------------------------------------------------------------

SalVirtualDevice* SalInstance::CreateVirtualDevice( SalGraphics* pGraphics,
                                                    long nDX, long nDY,
                                                    USHORT nBitCount )
{
	SalVirtualDevice *pDevice = NULL;

	if ( nDX > 0 && nDY > 0 )
	{
		pDevice = new SalVirtualDevice();

		if ( pGraphics )
			nBitCount = pGraphics->GetBitCount();
		pDevice->maVirDevData.mnBitCount = nBitCount;

		if ( !pDevice->SetSize( nDX, nDY ) )
		{
			delete pDevice;
			pDevice = NULL;
		}
	}

   	return pDevice;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyVirtualDevice( SalVirtualDevice* pDevice )
{
	if ( pDevice )
		delete pDevice;
}

// -----------------------------------------------------------------------

SalSession* SalInstance::CreateSalSession()
{
	return NULL;
}

// =======================================================================

SalInstanceData::SalInstanceData()
{
	mpSalYieldMutex = NULL;
	mpFilterCallback = NULL;
	mpFilterInst = NULL;
}

// -----------------------------------------------------------------------

SalInstanceData::~SalInstanceData()
{
}

// =========================================================================

SalYieldMutex::SalYieldMutex()
{
	mnCount	 = 0;
	mnThreadId  = 0;
}

// -------------------------------------------------------------------------

void SalYieldMutex::acquire()
{
	OMutex::acquire();
	mnThreadId = OThread::getCurrentIdentifier();
	mnCount++;
}

void SalYieldMutex::release()
{
	if ( mnThreadId == OThread::getCurrentIdentifier() )
	{
		if ( mnCount == 1 )
			mnThreadId = 0;
		mnCount--;
	}
	OMutex::release();
}

sal_Bool SalYieldMutex::tryToAcquire()
{
	if ( OMutex::tryToAcquire() )
	{
		mnThreadId = OThread::getCurrentIdentifier();
		mnCount++;
		return sal_True;
	}
	else
		return sal_False;
}
