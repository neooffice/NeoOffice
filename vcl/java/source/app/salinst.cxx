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

#ifndef _SV_SALINST_H
#include <salinst.h>
#endif
#ifndef _SV_SALBMP_H
#include <salbmp.h>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALIMESTATUS_HXX
#include <salimestatus.hxx>
#endif
#ifndef _SALJAVA_H
#include <saljava.h>
#endif
#ifndef _SV_SALMENU_H
#include <salmenu.h>
#endif
#ifndef _SV_SALOGL_H
#include <salogl.h>
#endif
#ifndef _SV_SALPTYPE_HXX
#include <salptype.hxx>
#endif
#ifndef _SV_SALSOUND_H
#include <salsound.h>
#endif
#ifndef _SV_SALSYS_H
#include <salsys.h>
#endif
#ifndef _SV_SALTIMER_H
#include <saltimer.h>
#endif
#ifndef _SV_SALVD_H
#include <salvd.h>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_SALPRN_H
#include <salprn.h>
#endif
#ifndef _SV_SALTIMER_HXX
#include <saltimer.hxx>
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
#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif
#ifndef _TOOLS_SIMPLERESMGR_HXX_
#include <tools/simplerm.hxx>
#endif

#include "salinst.hrc"
#include "salinst_cocoa.h"

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>
#undef check

class JavaSalI18NImeStatus : public SalI18NImeStatus
{
public:
							JavaSalI18NImeStatus() {}
	virtual					~JavaSalI18NImeStatus() {}

	virtual bool			canToggle() { return false; }
	virtual void			toggle() {}
};

static bool bFirstPass = true;

using namespace rtl;
using namespace vcl;
using namespace vos;

// ============================================================================

static OSStatus CarbonEventHandler( EventHandlerCallRef aNextHandler, EventRef aEvent, void *pData )
{
	EventClass nClass = GetEventClass( aEvent );
	EventKind nKind = GetEventKind( aEvent );

	if ( !Application::IsShutDown() )
	{
		SalData *pSalData = GetSalData();

		if ( nClass == kEventClassMenu && ( nKind == kEventMenuBeginTracking || nKind == kEventMenuEndTracking ) )
		{
			// Check if this a menubar event as we don't want to dispatch
			// native popup menus in modal dialogs and make sure that this is
			// not a duplicate menu opening event
			UInt32 nContext;
			if ( GetEventParameter( aEvent, kEventParamMenuContext, typeUInt32, NULL, sizeof( UInt32 ), NULL, &nContext ) == noErr && nContext & kMenuContextMenuBarTracking )
			{
				// Check if there is a native modal window as we will deadlock
				// when a native modal window is showing
				if ( NSApplication_getModalWindow() || !pSalData->maNativeEventCondition.check() )
					return userCanceledErr;

				// Wakeup the event queue by sending it a dummy event
				// and wait for all pending AWT events to be dispatched
				pSalData->mbInNativeMenuTracking = ( nKind == kEventMenuBeginTracking );
				pSalData->mbNativeEventSucceeded = false;
				pSalData->maNativeEventCondition.reset();
				com_sun_star_vcl_VCLEvent aEvent( SALEVENT_USEREVENT, NULL, NULL );
				pSalData->mpEventQueue->postCachedEvent( &aEvent );

				// We need to let any pending timers run while we are
				// waiting for the VCL event queue to clear so that
				// we don't deadlock
				TimeValue aDelay;
				aDelay.Seconds = 0;
				aDelay.Nanosec = 10;
				while ( !Application::IsShutDown() && !pSalData->maNativeEventCondition.check() )
				{
					ReceiveNextEvent( 0, NULL, 0, false, NULL );
					OThread::wait( aDelay );
				}

				// We need to let any timers run that were added by any menu
				// changes. Otherwise, some menus will be drawn in the state
				// that they were in before we updated the menus.
				if ( !Application::IsShutDown() && pSalData->mbNativeEventSucceeded )
					ReceiveNextEvent( 0, NULL, 0, false, NULL );
				else
					return userCanceledErr;
			}
		}
	}

	// Always execute the next registered handler
	return CallNextEventHandler( aNextHandler, aEvent );
}

// ----------------------------------------------------------------------------

void ExecuteApplicationMain( Application *pApp )
{
	// Now that Java is properly initialized, run the application's Main()
	GetSalData()->mpEventQueue = new com_sun_star_vcl_VCLEventQueue( NULL );

	EventHandlerUPP pEventHandlerUPP = NewEventHandlerUPP( CarbonEventHandler );
	if ( pEventHandlerUPP )
	{
		// Set up native event handler
		EventTypeSpec aTypes[2];
		aTypes[0].eventClass = kEventClassMenu;
		aTypes[0].eventKind = kEventMenuBeginTracking;
		aTypes[1].eventClass = kEventClassMenu;
		aTypes[1].eventKind = kEventMenuEndTracking;
		InstallApplicationEventHandler( pEventHandlerUPP, 2, aTypes, NULL, NULL );
	}

	pApp->Main();
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

const ::rtl::OUString& SalGetDesktopEnvironment()
{
	static ::rtl::OUString aDesktopEnvironment( RTL_CONSTASCII_USTRINGPARAM( "Mac OS X" ) );
	return aDesktopEnvironment;
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

SalInstance* CreateSalInstance()
{
	SalData *pSalData = GetSalData();

	JavaSalInstance *pInst = new JavaSalInstance();
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

JavaSalInstance::JavaSalInstance()
{
	mpSalYieldMutex = new SalYieldMutex();
	mpSalYieldMutex->acquire();
}

// -----------------------------------------------------------------------

JavaSalInstance::~JavaSalInstance()
{
	if ( mpSalYieldMutex )
	{
		mpSalYieldMutex->release();
		delete mpSalYieldMutex;
	}
}

// -----------------------------------------------------------------------

IMutex* JavaSalInstance::GetYieldMutex()
{
	return mpSalYieldMutex;
}

// -----------------------------------------------------------------------

ULONG JavaSalInstance::ReleaseYieldMutex()
{
	SalYieldMutex* pYieldMutex = mpSalYieldMutex;
	if ( pYieldMutex->GetThreadId() == OThread::getCurrentIdentifier() )
	{
		// Fix bug 1079 by not allowing releasing of the mutex when we are in
		// the native event dispatch thread
		if ( GetCurrentEventLoop() == GetMainEventLoop() )
			return 0;

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

void JavaSalInstance::AcquireYieldMutex( ULONG nCount )
{
	SalYieldMutex* pYieldMutex = mpSalYieldMutex;
	while ( nCount )
	{
		pYieldMutex->acquire();
		nCount--;
	}
}

// -----------------------------------------------------------------------

void JavaSalInstance::Yield( BOOL bWait )
{
	SalData *pSalData = GetSalData();

	// When we are in the native event dispatch thread, allow any pending
	// native timers to run but don't dispatch any events as we might be in
	// a signal handler and we will block since we don't have the SalYieldMutex
	// lock
	if ( GetCurrentEventLoop() == GetMainEventLoop() || mpSalYieldMutex->GetThreadId() != OThread::getCurrentIdentifier() )
	{
		ReceiveNextEvent( 0, NULL, 0, false, NULL );
		com_sun_star_vcl_VCLEvent aEvent( SALEVENT_USEREVENT, NULL, NULL );
		pSalData->mpEventQueue->postCachedEvent( &aEvent );
		return;
	}

	com_sun_star_vcl_VCLEvent *pEvent;

	// Dispatch next pending non-AWT event
	if ( ( pEvent = pSalData->mpEventQueue->getNextCachedEvent( 0, FALSE ) ) != NULL )
	{
		USHORT nID = pEvent->getID();
		pEvent->dispatch();
		delete pEvent;

		// We need to break out of dispatching non-AWT events if this is
		// an open or print document event as these events may be reposted
		// which could cause an infinite loop
		if ( nID != SALEVENT_OPENDOCUMENT && nID != SALEVENT_PRINTDOCUMENT )
			return;
	}

	ULONG nCount = ReleaseYieldMutex();
	OThread::yield();
	AcquireYieldMutex( nCount );

	// Check timer
	ImplSVData* pSVData = ImplGetSVData();
	if ( pSVData->mpSalTimer && pSalData->mnTimerInterval )
	{
		timeval aCurrentTime;
		gettimeofday( &aCurrentTime, NULL );
		if ( aCurrentTime >= pSalData->maTimeout )
		{
			::std::list< JavaSalFrame* >::const_iterator it;
			for ( it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
			{
				if ( (*it)->mbVisible )
					(*it)->mpVCLFrame->enableFlushing( sal_False );
			}

			gettimeofday( &pSalData->maTimeout, NULL );
			pSalData->maTimeout += pSalData->mnTimerInterval;
			pSVData->mpSalTimer->CallCallback();

			for ( it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
			{
				if ( (*it)->mbVisible )
					(*it)->mpVCLFrame->enableFlushing( sal_True );
			}
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

	// Dispatch any newly posted events
	if ( nTimeout )
		nCount = ReleaseYieldMutex();
	else
		nCount = 0;

	// Dispatch any pending AWT events
	while ( !Application::IsShutDown() && ( pEvent = pSalData->mpEventQueue->getNextCachedEvent( nTimeout, TRUE ) ) != NULL )
	{
		nTimeout = 0;

		if ( nCount )
		{
			AcquireYieldMutex( nCount );
			nCount = 0;
		}

		USHORT nID = pEvent->getID();
		pEvent->dispatch();
		delete pEvent;

		// Fix bug 1147 by allowing non-AWT events to be dispatched after a
		// mouse released event
		if ( nID == SALEVENT_MOUSEBUTTONUP )
		{
			if ( nCount )
				AcquireYieldMutex( nCount );
			return;
		}
	}

	if ( nCount )
		AcquireYieldMutex( nCount );

	// Allow Carbon event loop to proceed
	if ( !pEvent && !pSalData->maNativeEventCondition.check() )
	{
		pSalData->mbNativeEventSucceeded = ( !Application::IsShutDown() && !pSalData->mbInNativeModalSheet );
		if ( pSalData->mbNativeEventSucceeded )
		{
			if ( pSalData->mpFocusFrame && pSalData->mpFocusFrame->mbVisible )
			{
				ResetMenuEnabledStateForFrame( pSalData->mpFocusFrame, NULL );
				if ( pSalData->mbInNativeMenuTracking )
					UpdateMenusForFrame( pSalData->mpFocusFrame, NULL );
			}
		}
		else
		{
			for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
			{
				if ( (*it)->mbVisible )
				{
					ResetMenuEnabledStateForFrame( *it, NULL );
					if ( pSalData->mbInNativeMenuTracking )
						UpdateMenusForFrame( *it, NULL );
				}
			}
		}

		pSalData->maNativeEventCondition.set();
		nCount = ReleaseYieldMutex();
		OThread::yield();
		AcquireYieldMutex( nCount );
	}
}

// -----------------------------------------------------------------------

bool JavaSalInstance::AnyInput( USHORT nType )
{
	bool bRet = false;

	if ( nType & INPUT_TIMER )
	{
		// Check timer
		SalData *pSalData = GetSalData();
		ImplSVData* pSVData = ImplGetSVData();
		if ( pSVData->mpSalTimer && pSalData->mnTimerInterval )
		{
			timeval aCurrentTime;
			gettimeofday( &aCurrentTime, NULL );
			if ( aCurrentTime >= pSalData->maTimeout )
				bRet = true;
		}
	}

	if ( !bRet )
		bRet = (bool)GetSalData()->mpEventQueue->anyCachedEvent( nType );

	return bRet;
}

// -----------------------------------------------------------------------

SalFrame* JavaSalInstance::CreateChildFrame( SystemParentData* pSystemParentData, ULONG nSalFrameStyle )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalInstance::CreateChildFrame not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

SalFrame* JavaSalInstance::CreateFrame( SalFrame* pParent, ULONG nSalFrameStyle )
{
	JavaSalFrame *pFrame = new JavaSalFrame();

	pFrame->mnStyle = nSalFrameStyle;
	com_sun_star_vcl_VCLFrame *pVCLFrame = new com_sun_star_vcl_VCLFrame( pFrame->mnStyle, pFrame, (JavaSalFrame *)pParent );
	if ( !pVCLFrame || !pVCLFrame->getJavaObject() )
	{
		if ( pVCLFrame )
			delete pVCLFrame;
		delete pFrame;
		return NULL;
	}
	pFrame->mpVCLFrame = pVCLFrame;
	pFrame->maSysData.aWindow = 0;
	pFrame->maSysData.pSalFrame = pFrame;

	// Set initial parent
	pFrame->SetParent( pParent );

	// Insert this window into the window list
	SalData *pSalData = GetSalData();
	pSalData->maFrameList.push_front( pFrame );

	// Cache the insets
	Rectangle aRect = pFrame->mpVCLFrame->getInsets();
	pFrame->maGeometry.nLeftDecoration = aRect.nLeft;
	pFrame->maGeometry.nTopDecoration = aRect.nTop;
	pFrame->maGeometry.nRightDecoration = aRect.nRight;
	pFrame->maGeometry.nBottomDecoration = aRect.nBottom;

	// Set default window size based on style
	Rectangle aWorkArea;
	if ( pFrame->mpParent )
	{
		aWorkArea = Rectangle( Point( pFrame->mpParent->maGeometry.nX, pFrame->mpParent->maGeometry.nY ), Size( pFrame->mpParent->maGeometry.nWidth, pFrame->mpParent->maGeometry.nHeight ) );
		pFrame->GetWorkArea( aWorkArea );
	}
	else
	{
		pFrame->GetWorkArea( aWorkArea );
	}

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
		if ( !pFrame->mpParent )
		{
			// Find the next document window if any exist
			JavaSalFrame* pNextFrame = NULL;
			for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
			{
				if ( (*it) && (*it) != pFrame &&
					! (*it)->mpParent &&
					(*it)->mnStyle != SAL_FRAME_STYLE_DEFAULT &&
					(*it)->mnStyle & SAL_FRAME_STYLE_SIZEABLE &&
					(*it)->GetGeometry().nWidth &&
					(*it)->GetGeometry().nHeight )
						pNextFrame = *it;
			}

			if ( pNextFrame )
			{
				// Set screen to same screen as next frame
				pFrame->mbCenter = FALSE;
				const SalFrameGeometry& rGeom( pNextFrame->GetGeometry() );
				nX = rGeom.nX - rGeom.nLeftDecoration;
				nY = rGeom.nY - rGeom.nTopDecoration;
				nWidth = rGeom.nWidth + rGeom.nLeftDecoration + rGeom.nRightDecoration;
				nHeight = rGeom.nHeight + rGeom.nTopDecoration + rGeom.nBottomDecoration;
			}
		}
	}

	long nFlags = SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT;
	if ( !pFrame->mbCenter )
		nFlags |= SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y;
	pFrame->SetPosSize( nX, nY, nWidth, nHeight, nFlags );

	// Reset mbCenter flag to default value
	pFrame->mbCenter = TRUE;

	return pFrame;
}

// -----------------------------------------------------------------------

void JavaSalInstance::DestroyFrame( SalFrame* pFrame )
{
	// Remove this window from the window list
	if ( pFrame )
	{
		JavaSalFrame *pJavaFrame = (JavaSalFrame *)pFrame;

		pJavaFrame->SetParent( NULL );

		if ( pJavaFrame->mbVisible )
			pJavaFrame->Show( FALSE );

		SalData *pSalData = GetSalData();
		pSalData->maFrameList.remove( pJavaFrame );

		delete pJavaFrame;
	}
}

// -----------------------------------------------------------------------

SalObject* JavaSalInstance::CreateObject( SalFrame* pParent, SystemWindowData* pWindowData )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalInstance::CreateObject not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

void JavaSalInstance::DestroyObject( SalObject* pObject )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalInstance::DestroyObject not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void* JavaSalInstance::GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes )
{
	rReturnedBytes = 1;
	rReturnedType = AsciiCString;
	return (void *)"";
}

// -----------------------------------------------------------------------

void JavaSalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
	// Create a dummy queue for our dummy default printer
	SalPrinterQueueInfo *pInfo = new SalPrinterQueueInfo();
	pInfo->maPrinterName = GetDefaultPrinter();
	pInfo->mpSysData = NULL;
	pList->Add( pInfo );
}

// -----------------------------------------------------------------------

void JavaSalInstance::GetPrinterQueueState( SalPrinterQueueInfo* pInfo )
{
}

// -----------------------------------------------------------------------

void JavaSalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
	if ( pInfo )
		delete pInfo;
}

// -----------------------------------------------------------------------

SalInfoPrinter* JavaSalInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                                ImplJobSetup* pSetupData )
{
	SalData *pSalData = GetSalData();

	// Create a dummy printer configuration for our dummy printer
	JavaSalInfoPrinter *pPrinter = new JavaSalInfoPrinter();

	// Populate data
	pSetupData->mnSystem = JOBSETUP_SYSTEM_JAVA;
	pSetupData->maPrinterName = pQueueInfo->maPrinterName;
	pSetupData->maDriver = pQueueInfo->maDriver;

	// Clear driver data
	if ( pSetupData->mpDriverData )
	{
		rtl_freeMemory( pSetupData->mpDriverData );
		pSetupData->mpDriverData = NULL;
		pSetupData->mnDriverDataLen = 0;
	}

	// Create a new page format instance
	pPrinter->mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat();

	// Update values
	pPrinter->SetData( 0, pSetupData );

	return pPrinter;
}

// -----------------------------------------------------------------------

void JavaSalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
	if ( pPrinter )
		delete pPrinter;
}

// -----------------------------------------------------------------------

XubString JavaSalInstance::GetDefaultPrinter()
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

SalPrinter* JavaSalInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
	JavaSalPrinter *pPrinter = new JavaSalPrinter();

	JavaSalInfoPrinter *pJavaInfoPrinter = (JavaSalInfoPrinter *)pInfoPrinter;
	if ( pJavaInfoPrinter && pJavaInfoPrinter->mpVCLPageFormat )
		pPrinter->mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat( pJavaInfoPrinter->mpVCLPageFormat->getJavaObject() );
	else
		pPrinter->mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat();

	return pPrinter;
}

// -----------------------------------------------------------------------

void JavaSalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
	if ( pPrinter )
		delete pPrinter;
}

// -----------------------------------------------------------------------

SalVirtualDevice* JavaSalInstance::CreateVirtualDevice( SalGraphics* pGraphics,
                                                    long nDX, long nDY,
                                                    USHORT nBitCount,
                                                    const SystemGraphicsData *pData )
{
	JavaSalVirtualDevice *pDevice = NULL;

	if ( nDX > 0 && nDY > 0 )
	{
		pDevice = new JavaSalVirtualDevice();

		if ( pGraphics )
			nBitCount = pGraphics->GetBitCount();
		pDevice->mnBitCount = nBitCount;

		if ( !pDevice->SetSize( nDX, nDY ) )
		{
			delete pDevice;
			pDevice = NULL;
		}
	}

   	return pDevice;
}

// -----------------------------------------------------------------------

void JavaSalInstance::DestroyVirtualDevice( SalVirtualDevice* pDevice )
{
	if ( pDevice )
		delete pDevice;
}

// -----------------------------------------------------------------------

SalSound* JavaSalInstance::CreateSalSound()
{
    return new JavaSalSound();
}

// -----------------------------------------------------------------------

SalTimer* JavaSalInstance::CreateSalTimer()
{
    return new JavaSalTimer();
}

// -----------------------------------------------------------------------

SalOpenGL* JavaSalInstance::CreateSalOpenGL( SalGraphics* pGraphics )
{
    return new JavaSalOpenGL();
}

// -----------------------------------------------------------------------

SalI18NImeStatus* JavaSalInstance::CreateI18NImeStatus()
{
    return new JavaSalI18NImeStatus();
}

// -----------------------------------------------------------------------

SalBitmap* JavaSalInstance::CreateSalBitmap()
{
    return new JavaSalBitmap();
}

// -----------------------------------------------------------------------

SalSession* JavaSalInstance::CreateSalSession()
{
	return NULL;
}

// -----------------------------------------------------------------------

SalSystem* JavaSalInstance::CreateSalSystem()
{
	return new JavaSalSystem();
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
