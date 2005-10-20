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
#ifndef _SV_SALMENU_HXX
#include <salmenu.hxx>
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

		if ( nClass == kEventClassMenu && ( nKind == kEventMenuOpening || nKind == kEventMenuEndTracking ) )
		{
			// Check if this is a menubar event as we don't want to dispatch
			// native popup menus in modal dialogs
			UInt32 nContext;
			if ( GetEventParameter( aEvent, kEventParamMenuContext, typeUInt32, NULL, sizeof( UInt32 ), NULL, &nContext ) == noErr && nContext & kMenuContextMenuBarTracking )
			{
				// Check if there is a native modal window as we will deadlock
				// when a native modal window is showing
				if ( NSApplication_getModalWindow() )
					return userCanceledErr;

				// Wakeup the event queue by sending it a dummy event
				// and wait for all pending AWT events to be dispatched
				pSalData->mbNativeEventSucceeded = false;
				pSalData->maNativeEventCondition.reset();
				com_sun_star_vcl_VCLEvent aEvent( SALEVENT_USEREVENT, NULL, NULL );
				pSalData->mpEventQueue->postCachedEvent( &aEvent );

				// We need to let any pending timers run while we are
				// waiting for the VCL event queue to clear so that
				// we don't deadlock
				IMutex& rSolarMutex = Application::GetSolarMutex();
				while ( !Application::IsShutDown() && !pSalData->maNativeEventCondition.check() )
				{
					ReceiveNextEvent( 0, NULL, 0, false, NULL );
					OThread::yield();
				}

				// Fix bug 679 by checking if the condition was
				// released to avoid a deadlock
				if ( !Application::IsShutDown() && pSalData->mbNativeEventSucceeded )
				{
					// We need to let any pending timers run so that we
					// don't deadlock
					bool bAcquired = false;
					while ( !Application::IsShutDown() )
					{
						if ( rSolarMutex.tryToAcquire() )
						{
							if ( !Application::IsShutDown() )
								bAcquired = true;
							else
								rSolarMutex.release();
							break;
						}

						ReceiveNextEvent( 0, NULL, 0, false, NULL );
						OThread::yield();
					}

					if ( bAcquired )
					{
						bool bFirstPass = !pSalData->mbInNativeMenuTracking;
						pSalData->mbInNativeMenuTracking = ( nKind == kEventMenuOpening );

						// Execute menu updates while the VCL event
						// queue is blocked
						for ( ::std::list< SalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
						{
							if ( (*it)->maFrameData.mbVisible )
							{
								ResetMenuEnabledStateForFrame( *it, NULL );
								if ( bFirstPass )
									UpdateMenusForFrame( *it, NULL );
							}
						}

						// We need to let any timers run that were added
						// by any menu changes. Otherwise, some menus
						// will be drawn in the state that they were in
						// before we updated the menus.
						ReceiveNextEvent( 0, NULL, 0, false, NULL );
					}

					rSolarMutex.release();
				}
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
		aTypes[0].eventKind = kEventMenuOpening;
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
	// When we are in the native event dispatch thread, allow any pending
	// native timers to run but don't dispatch any events as we might be in
	// a signal handler and we will block since we don't have the SalYieldMutex
	// lock
	if ( GetCurrentEventLoop() == GetMainEventLoop() )
	{
		ReceiveNextEvent( 0, NULL, 0, false, NULL );
		return;
	}

	SalData *pSalData = GetSalData();
	com_sun_star_vcl_VCLEvent *pEvent;

	// Dispatch next pending non-AWT event
	if ( ( pEvent = pSalData->mpEventQueue->getNextCachedEvent( 0, FALSE ) ) != NULL )
	{
		pEvent->dispatch();
		delete pEvent;
	}

	ULONG nCount = ReleaseYieldMutex();
	OThread::yield();
	AcquireYieldMutex( nCount );

	// Check timer
	if ( pSalData->mnTimerInterval )
	{
		timeval aCurrentTime;
		gettimeofday( &aCurrentTime, NULL );
		if ( pSalData->mpTimerProc && aCurrentTime >= pSalData->maTimeout )
		{
			gettimeofday( &pSalData->maTimeout, NULL );
			pSalData->maTimeout += pSalData->mnTimerInterval;
			pSalData->mpTimerProc();
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

		if ( pSalData->mbInNativeModalSheet )
		{
			if ( pEvent->getFrame() != pSalData->mpNativeModalSheetFrame )
				pSalData->mpNativeModalSheetFrame->ToTop( SAL_FRAME_TOTOP_RESTOREWHENMIN | SAL_FRAME_TOTOP_GRABFOCUS );
			if ( pEvent->getID() == SALEVENT_PAINT )
				pEvent->dispatch();
		}
		else
		{
			pEvent->dispatch();
		}
		delete pEvent;
	}

	if ( nCount )
		AcquireYieldMutex( nCount );

	// Allow Carbon event loop to proceed
	if ( !pSalData->maNativeEventCondition.check() )
	{
		pSalData->mbNativeEventSucceeded = !pSalData->mbInNativeModalSheet;
		pSalData->maNativeEventCondition.set();
		nCount = ReleaseYieldMutex();
		OThread::yield();
		AcquireYieldMutex( nCount );
	}
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

	pFrame->maFrameData.mnStyle = nSalFrameStyle;
	com_sun_star_vcl_VCLFrame *pVCLFrame = new com_sun_star_vcl_VCLFrame( pFrame->maFrameData.mnStyle, pFrame, pParent );
	if ( !pVCLFrame || !pVCLFrame->getJavaObject() )
	{
		if ( pVCLFrame )
			delete pVCLFrame;
		delete pFrame;
		return NULL;
	}
	pFrame->maFrameData.mpVCLFrame = pVCLFrame;
	pFrame->maFrameData.maSysData.aWindow = 0;
	pFrame->maFrameData.maSysData.pSalFrame = pFrame;

	// Set initial parent
	pFrame->maFrameData.mpParent = pParent;
	if ( pFrame->maFrameData.mpParent )
	{
		pFrame->maFrameData.mpParent->maFrameData.mpVCLFrame->addChild( pFrame );
		pFrame->maFrameData.mpParent->maFrameData.maChildren.push_back( pFrame );
	}

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

	// Clear driver data
	if ( pSetupData->mpDriverData )
	{
		rtl_freeMemory( pSetupData->mpDriverData );
		pSetupData->mpDriverData = NULL;
		pSetupData->mnDriverDataLen = 0;
	}

	// Create a new page format instance
	pPrinter->maPrinterData.mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat();

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

	if ( pInfoPrinter && pInfoPrinter->maPrinterData.mpVCLPageFormat )
		pPrinter->maPrinterData.mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat( pInfoPrinter->maPrinterData.mpVCLPageFormat->getJavaObject() );
	else
		pPrinter->maPrinterData.mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat();

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
