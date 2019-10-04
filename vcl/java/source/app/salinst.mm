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
 *  Copyright 2003 Planamesa Inc.
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

#include <unistd.h>
#include <sys/syslimits.h>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>
#undef check

#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/resmgr.hxx>
#include <tools/simplerm.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/inputtypes.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/settings.hxx>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "jobset.h"
#include "print.h"
#include "salimestatus.hxx"
#include "salptype.hxx"
#include "saltimer.hxx"
#include "svids.hrc"
#include "java/salbmp.h"
#include "java/saldata.hxx"
#include "java/salframe.h"
#include "java/salgdi.h"
#include "java/saljava.h"
#include "java/salmenu.h"
#include "java/salobj.h"
#include "java/salprn.h"
#include "java/salsys.h"
#include "java/saltimer.h"
#include "java/salvd.h"

#include "salinst_cocoa.h"
#include "../java/VCLEventQueue_cocoa.h"

class JavaSalI18NImeStatus : public SalI18NImeStatus
{
public:
							JavaSalI18NImeStatus() {}
	virtual					~JavaSalI18NImeStatus() {}

	virtual bool			canToggle() { return false; }
	virtual void			toggle() {}
};

static bool bInUnitTest = false;
static bool bAllowReleaseYieldMutex = false;
static sal_Bool bInNativeDragPrint = sal_False;
static SalYieldMutex aEventQueueMutex;
static sal_uLong nCurrentTimeout = 0;
static NSMutableArray *pObjectsRetainedDuringDragPrintLock = nil;

using namespace com::sun::star;
using namespace osl;
using namespace vcl;

// ============================================================================

static JavaSalFrame *FindMouseEventFrame( JavaSalFrame *pFrame, const Point &rScreenPoint )
{
	if ( !pFrame || !pFrame->mbVisible )
		return nullptr;

	// Iterate through children
	for ( ::std::list< JavaSalFrame* >::const_iterator it = pFrame->maChildren.begin(); it != pFrame->maChildren.end(); ++it )
	{
		JavaSalFrame *pRet = pFrame;
		pRet = FindMouseEventFrame( *it, rScreenPoint );
		if ( pRet && pRet != pFrame )
			return pRet;
	}

	if ( pFrame->IsFloatingFrame() && ! ( pFrame->mnStyle & SalFrameStyleFlags::TOOLTIP ) )
	{
		tools::Rectangle aBounds( Point( pFrame->maGeometry.nX - pFrame->maGeometry.nLeftDecoration, pFrame->maGeometry.nY - pFrame->maGeometry.nTopDecoration ), Size( pFrame->maGeometry.nWidth + pFrame->maGeometry.nLeftDecoration + pFrame->maGeometry.nRightDecoration, pFrame->maGeometry.nHeight + pFrame->maGeometry.nTopDecoration + pFrame->maGeometry.nBottomDecoration ) );
		if ( aBounds.IsInside( rScreenPoint ) )
			return pFrame;
	}

	return nullptr;
}

// ----------------------------------------------------------------------------

static void InvalidateControls( Window *pWindow )
{
	// Fix bug reported in the following NeoOffice forum support by
	// invalidating the entire window:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=62916#62916
	if ( pWindow && pWindow->IsReallyVisible() )
		pWindow->Invalidate();
}

// -----------------------------------------------------------------------

static sal_uLong ReleaseEventQueueMutex()
{
	if ( aEventQueueMutex.GetThreadId() == Thread::getCurrentIdentifier() )
	{
		sal_uLong nCount = aEventQueueMutex.ReleaseAcquireCount();
		sal_uLong n = nCount;
		while ( n )
		{
			aEventQueueMutex.release();
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

static void AcquireEventQueueMutex( sal_uLong nCount )
{
	while ( nCount )
	{
		aEventQueueMutex.acquire();
		nCount--;
	}
}

// ----------------------------------------------------------------------------

static JavaSalFrame *FindValidFrame( JavaSalFrame *pFrame )
{
	if ( !pFrame )
		return nullptr;

	SalData *pSalData = GetSalData();
	for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
	{
		if ( pFrame == *it )
		{
			if ( pFrame->GetWindow() )
				return pFrame;
			else
				break;
		}
	}

	return nullptr;
}

// ============================================================================

sal_Bool VCLInstance_isInDragPrintLock()
{
	return ( CFRunLoopGetCurrent() == CFRunLoopGetMain() && bInNativeDragPrint );
}

// ----------------------------------------------------------------------------

sal_Bool VCLInstance_retainIfInDragPrintLock( id aObject )
{
	sal_Bool bRet = sal_False;

	if ( !aObject || CFRunLoopGetCurrent() != CFRunLoopGetMain() || !bInNativeDragPrint )
		return bRet;

	if ( !pObjectsRetainedDuringDragPrintLock )
	{
		pObjectsRetainedDuringDragPrintLock = [NSMutableArray arrayWithCapacity:10];
		if ( pObjectsRetainedDuringDragPrintLock )
			[pObjectsRetainedDuringDragPrintLock retain];
	}

	if ( pObjectsRetainedDuringDragPrintLock )
	{
		[pObjectsRetainedDuringDragPrintLock addObject:aObject];
		bRet = YES;
	}

	return bRet;
}

// ----------------------------------------------------------------------------

sal_Bool VCLInstance_setDragPrintLock( sal_Bool bLock )
{
	sal_Bool bRet = sal_False;

	if ( CFRunLoopGetCurrent() != CFRunLoopGetMain() )
		return bRet;

	if ( bLock )
	{
		if ( !Application::IsShutDown() && !bInNativeDragPrint )
		{
			comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
			rSolarMutex.acquire();
			if ( !Application::IsShutDown() && !bInNativeDragPrint )
			{	
				bInNativeDragPrint = sal_True;
				bRet = sal_True;
			}
			else
			{
				rSolarMutex.release();
				return bRet;
			}
		}
	}
	else if ( bInNativeDragPrint )
	{
		bInNativeDragPrint = sal_False;
		if ( pObjectsRetainedDuringDragPrintLock )
			[pObjectsRetainedDuringDragPrintLock removeAllObjects];
		comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.release();
		bRet = sal_True;
	}

	return bRet;
}

// ----------------------------------------------------------------------------

sal_Bool VCLInstance_updateNativeMenus()
{
	sal_Bool bRet = sal_False;

	// If no application mutex exists yet, queue event as we are likely to
	// crash. Check if ImplSVData exists first since Application::IsShutDown()
	// uses it.
	if ( !ImplGetSVData() || !ImplGetSVData()->mpDefInst || Application::IsShutDown() )
		return bRet;

	// Check if there is a native modal window as we will deadlock when a
	// native modal window is showing.
	if ( bInNativeDragPrint || NSApplication_getModalWindow() )
		return bRet;

	// Fix bug 2783 by cancelling menu actions if the input method if the
	// there is any marked text in the key window
	if ( NSWindow_hasMarkedText( nil ) )
		return bRet;

	// Make sure that any events fetched from the queue while the application
	// mutex was unlocked are already dispatched before we try to lock the
	// mutex. Fix bug 3467 by speeding up acquiring of the event queue mutex
	// by releasing the application mutex if already acquired by this thread.
	bAllowReleaseYieldMutex = true;
	sal_uLong nCount = Application::ReleaseSolarMutex();
	bAllowReleaseYieldMutex = false;
	aEventQueueMutex.acquire();
	Application::AcquireSolarMutex( nCount );

	comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
	rSolarMutex.acquire();

	ImplSVData *pSVData = ImplGetSVData();
	SalData *pSalData = GetSalData();
	if ( Application::IsShutDown() || pSalData->mbInNativeModalSheet || !pSVData || !pSalData )
	{
		rSolarMutex.release();
		aEventQueueMutex.release();
		return bRet;
	}

	size_t nFrames = pSalData->maFrameList.size();

	pSalData->maNativeEventCondition.reset();

	// Dispatch pending VCL events until the queue is clear
	while ( !Application::IsShutDown() && !pSalData->maNativeEventCondition.check() )
		pSalData->mpFirstInstance->DoYield( false, true, 0 );

	// Fix bug 3451 by not updating menus when there is a modal dialog
	bRet = ( !Application::IsShutDown() && !pSalData->mbInNativeModalSheet && !pSVData->maWinData.mpLastExecuteDlg );
	if ( bRet )
	{
		if ( pSalData->mpFocusFrame && pSalData->mpFocusFrame->mbVisible )
		{
			// Fix update problem in bug 1577 when the menubar is selected and
			// the focus frame is a child of another frame. Fix bug 3461 by
			// updating parent frame's menu.
			JavaSalFrame *pFrame = pSalData->mpFocusFrame;
			while ( pFrame && pFrame->mbVisible )
			{
				UpdateMenusForFrame( pFrame, nullptr, true );
				pFrame = pFrame->mpParent;
			}
		}
		else
		{
			// Fix inability to access menu when there is no backing window by
			// only cancelling dislay of menus when there are visible windows
			for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
			{
				// Fix bug 3571 by only cancelling when at least one visible
				// non-backing, non-floating, and non-floating frame
				if ( (*it)->mbVisible && !(*it)->mbShowOnlyMenus && !(*it)->IsFloatingFrame() && (*it)->GetState() != WindowStateState::Minimized )
				{
					bRet = sal_False;
					break;
				}
			}
		}

		if ( bRet && pSVData && pSVData->maWinData.mpFirstFloat )
		{
			// Close all popups but return false to cancel menu tracking
			// since any shutdown event dispatched after this will cause a
			// crash when a second level popup menu is open
			if ( !(pSVData->maWinData.mpFirstFloat->GetPopupModeFlags() & FloatWinPopupFlags::NoAppFocusClose) )
			{
				pSVData->maWinData.mpFirstFloat->EndPopupMode( FloatWinPopupEndFlags::CloseAll );
				bRet = sal_False;
			}
		}

		// Probably overkill, but if the number of frames changes, cancel menu
		// tracking
		if ( bRet && pSalData->maFrameList.size() != nFrames )
			bRet = sal_False;

		// We need to let any timers run that were added by any menu
		// changes. Otherwise, some menus will be drawn in the state
		// that they were in before we updated the menus.
		CFRunLoopRunInMode( CFSTR( "AWTRunLoopMode" ), 0, false );
	}

	rSolarMutex.release();
	aEventQueueMutex.release();

	return bRet;
}

// -----------------------------------------------------------------------

// Note: this must not be static as the symbol will be loaded by the salhelper 
// module
extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool Application_acquireSolarMutex()
{
	sal_Bool bRet = sal_False;

	// Check if ImplSVData exists first since Application::IsShutDown() uses it
	if ( ImplGetSVData() && ImplGetSVData()->mpDefInst && !Application::IsShutDown() )
	{
		Application::GetSolarMutex().acquire();
		bRet = sal_True;
	}

	return bRet;
}

// -----------------------------------------------------------------------

// Note: this must not be static as the symbol will be loaded by the salhelper
// module
extern "C" SAL_DLLPUBLIC_EXPORT void Application_releaseSolarMutex()
{
	// Check if ImplSVData exists first since Application::IsShutDown() uses it
	if ( ImplGetSVData() && ImplGetSVData()->mpDefInst && !Application::IsShutDown() )
		Application::GetSolarMutex().release();
}

// -----------------------------------------------------------------------

// Note: this must not be static as the symbol will be loaded by the salhelper 
// module
extern "C" SAL_DLLPUBLIC_EXPORT void Application_acquireAllSolarMutex( sal_uLong nCount )
{
	// Check if ImplSVData exists first since Application::IsShutDown() uses it
	if ( ImplGetSVData() && ImplGetSVData()->mpDefInst && !Application::IsShutDown() )
		Application::AcquireSolarMutex( nCount );
}

// -----------------------------------------------------------------------

// Note: this must not be static as the symbol will be loaded by the salhelper
// module
extern "C" SAL_DLLPUBLIC_EXPORT sal_uLong Application_releaseAllSolarMutex()
{
	sal_uLong nRet = 0;

	// Check if ImplSVData exists first since Application::IsShutDown() uses it
	if ( ImplGetSVData() && ImplGetSVData()->mpDefInst && !Application::IsShutDown() )
		nRet = Application::ReleaseSolarMutex();

	return nRet;
}

// =======================================================================

void SalAbort( const OUString& rErrorText, bool /* bDumpCore */ )
{
	if ( rErrorText.getLength() )
		fprintf( stderr, "%s", OUStringToOString( rErrorText, osl_getThreadTextEncoding() ).getStr() );
	else
		fprintf( stderr, "Application Error" );
	fprintf( stderr, "\n" );
	abort();
}

// -----------------------------------------------------------------------

const OUString& SalGetDesktopEnvironment()
{
	static OUString aDesktopEnvironment( "MacOSX" );
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
	SetSalData( nullptr );
	delete pSalData;
}

// -----------------------------------------------------------------------

void InitSalMain()
{
}

// -----------------------------------------------------------------------

SalInstance* CreateSalInstance( bool bInImplSVMain )
{
	SalData *pSalData = GetSalData();

	// If we are running unit tests, initialize the native classes here since
	// NSApplication_run() will never be called
	bInUnitTest = !bInImplSVMain;
	if ( bInUnitTest )
		VCLEventQueue_installVCLEventQueueClasses();

	JavaSalInstance *pInst = new JavaSalInstance();
	pSalData->mpFirstInstance = pInst;

	// Set required Mac OS X NWF settings
	ImplGetSVData()->maNWFData.mbNoFocusRects = true;
	ImplGetSVData()->maNWFData.mbProgressNeedsErase = true;
	ImplGetSVData()->maNWFData.mbCheckBoxNeedsErase = true;

	return pInst;
}

// -----------------------------------------------------------------------

void DestroySalInstance( SalInstance* pInst )
{
	SalData *pSalData = GetSalData();

	if ( pSalData->mpFirstInstance == pInst )
		pSalData->mpFirstInstance = nullptr;

	if ( pInst )
		delete pInst;
}

// =======================================================================

JavaSalInstance::JavaSalInstance() :
	mpSalYieldMutex( new SalYieldMutex() )
{
	mpSalYieldMutex->acquire();
    ::comphelper::SolarMutex::setSolarMutex( mpSalYieldMutex );
}

// -----------------------------------------------------------------------

JavaSalInstance::~JavaSalInstance()
{
    ::comphelper::SolarMutex::setSolarMutex( nullptr );
	if ( mpSalYieldMutex )
	{
		mpSalYieldMutex->release();
		delete mpSalYieldMutex;
	}
}

// -----------------------------------------------------------------------

comphelper::SolarMutex* JavaSalInstance::GetYieldMutex()
{
	return mpSalYieldMutex;
}

// -----------------------------------------------------------------------

sal_uLong JavaSalInstance::ReleaseYieldMutex()
{
	return mpSalYieldMutex->ReleaseAcquireCount();
}

// -----------------------------------------------------------------------

void JavaSalInstance::AcquireYieldMutex( sal_uLong nCount )
{
	SalYieldMutex* pYieldMutex = mpSalYieldMutex;
	while ( nCount )
	{
		pYieldMutex->acquire();
		nCount--;
	}
}

// -----------------------------------------------------------------------

bool JavaSalInstance::CheckYieldMutex()
{
	return ( mpSalYieldMutex->GetThreadId() == Thread::getCurrentIdentifier() );
}

// -----------------------------------------------------------------------

SalYieldResult JavaSalInstance::DoYield( bool bWait, bool bHandleAllCurrentEvents, sal_uLong /* nReleased */ )
{
	sal_uLong nCount = 0;
	SalData *pSalData = GetSalData();
	bool bMainEventLoop = ( !bInUnitTest && CFRunLoopGetCurrent() == CFRunLoopGetMain() );

	// Fix bug 2575 by manually dispatching native events.
	if ( bMainEventLoop )
	{
		// Fix bug 2731 by not doing this when we are in the begin menubar
		// tracking handler
		if ( pSalData->maNativeEventCondition.check() )
			NSApplication_dispatchPendingEvents( bInNativeDragPrint, bWait );
	}
	else
	{
		// Fix bug 3455 by always acquiring the event queue mutex during the
		// entire event dispatching process
		nCount = ReleaseYieldMutex();
		aEventQueueMutex.acquire();
		AcquireYieldMutex( nCount );
	}

	JavaSalEvent *pEvent = nullptr;

	// Dispatch next pending non-AWT event. Also dispatch events if
	// Desktop::doShutdown() displays a modal dialog.
	ImplSVData *pSVData = ImplGetSVData();
	if ( ( !Application::IsShutDown() || pSVData->maWinData.mpLastExecuteDlg ) && ( pEvent = JavaSalEventQueue::getNextCachedEvent( 0, sal_False ) ) != nullptr )
	{
		pEvent->dispatch();
		pEvent->release();
		if ( !bMainEventLoop )
			aEventQueueMutex.release();

		JavaSalFrame::FlushAllFrames();

		return SalYieldResult::EVENT;
	}

	// Dispatch the next pending document event
	if ( !Application::IsShutDown() && pSalData->maPendingDocumentEventsList.size() && pSVData && pSVData->maAppData.mnDispatchLevel == 1 && !pSVData->maWinData.mpLastExecuteDlg && !pSalData->mbInNativeModalSheet )
	{
		pEvent = pSalData->maPendingDocumentEventsList.front();
		pSalData->maPendingDocumentEventsList.pop_front();
		pEvent->dispatch();
		pEvent->release();
		if ( !bMainEventLoop )
			aEventQueueMutex.release();

		JavaSalFrame::FlushAllFrames();

		return SalYieldResult::EVENT;
	}

	if ( !bMainEventLoop && pSalData->maNativeEventCondition.check() )
	{
		sal_uLong nEventQueueMutexCount = ReleaseEventQueueMutex();
		nCount = ReleaseYieldMutex();
		Thread::yield();
		AcquireEventQueueMutex( nEventQueueMutexCount );
		AcquireYieldMutex( nCount );
	}

	// Check timer
	if ( pSVData && pSVData->mpSalTimer && pSalData->mnTimerInterval )
	{
		timeval aCurrentTime;
		gettimeofday( &aCurrentTime, nullptr );
		// Fix cursor drawing failure in Writer by firing timer if timer is
		// within the blink rate away from expiring
		aCurrentTime += Application::GetSettings().GetStyleSettings().GetCursorBlinkTime();
		if ( aCurrentTime >= pSalData->maTimeout )
		{
			gettimeofday( &pSalData->maTimeout, nullptr );
			pSalData->maTimeout += pSalData->mnTimerInterval;
			pSVData->mpSalTimer->CallCallback( true );

			// Reduce noticeable pause when opening a new document by delaying
			// update of submenus until next available timer timeout.
			// Fix bug 3669 by not invoking menu updates while dragging.
			if ( pSalData->maNativeEventCondition.check() && pSalData->mpFocusFrame && pSalData->mpFocusFrame->mbVisible && !pSalData->mpLastDragFrame )
			{
				for ( int i = 0; pSalData->mpFocusFrame->maUpdateMenuList.size() && i < 8; i++ )
					UpdateMenusForFrame( pSalData->mpFocusFrame, pSalData->mpFocusFrame->maUpdateMenuList.front(), false );
			}
		}
	}

	JavaSalFrame::FlushAllFrames();

	// Determine timeout
	SalYieldResult nRet = SalYieldResult::TIMEOUT;
	sal_uLong nTimeout = 0;
	if ( !bMainEventLoop && bWait && pSalData->maNativeEventCondition.check() && !Application::IsShutDown() )
	{
		if ( pSalData->mnTimerInterval )
		{
			timeval aTimeout;
			gettimeofday( &aTimeout, nullptr );
			if ( pSalData->maTimeout > aTimeout )
			{
				aTimeout = pSalData->maTimeout - aTimeout;
				nTimeout = aTimeout.tv_sec * 1000 + aTimeout.tv_usec / 1000;
			}
		}

		// Wait a little bit to prevent excessive CPU usage. Fix bug 2588
		// by only doing so when the timeout is already set to a non-zero
		// value.
		if ( nTimeout < 10 )
			nTimeout = 10;
	}

	if ( nTimeout )
	{
		nCount = ReleaseYieldMutex();
		if ( !nCount )
			nTimeout = 0;
	}
	else
	{
		nCount = 0;
	}

	// Cache timeout for other threads
	nCurrentTimeout = nTimeout;

	// Dispatch any pending AWT events. Fix bug 2126 by always acting as if
	// the bHandleAllCurrentEvents parameter is true. Also dispatch events if
	// Desktop::doShutdown() displays a modal dialog.
	size_t nFrames = pSalData->maFrameList.size();
	bool bContinue = true;
	while ( bContinue && ( !Application::IsShutDown() || pSVData->maWinData.mpLastExecuteDlg ) && ( pEvent = JavaSalEventQueue::getNextCachedEvent( nTimeout, sal_True ) ) != nullptr )
	{
		nRet = SalYieldResult::EVENT;
		nTimeout = 0;
		nCurrentTimeout = 0;

		AcquireYieldMutex( nCount );
		nCount = 0;

		SalEvent nID = pEvent->getID();
		pEvent->dispatch();
		switch ( nID )
		{
			case SalEvent::Close:
				// Fix bug 1971 by breaking after closing a window
				bContinue = false;
				break;
			case SalEvent::KeyInput:
			case SalEvent::MouseButtonDown:
				// Fix bugs 437 and 2264 by ensuring that if the next
				// event is a matching event, it will be dispatched before
				// the painting timer runs.
				Thread::yield();
				break;
			case SalEvent::KeyUp:
				// Fix bug 3390 by letting any timers run when releasing
				// a Command-key event
				if ( pEvent->getModifiers() & KEY_MOD1 )
					bContinue = false;
				break;
			case SalEvent::MouseMove:
				// Make highlighting by dragging more responsive
                if ( pEvent->getModifiers() & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) )
					bContinue = false;
				break;
			case SalEvent::WheelMouse:
				// Make scroll wheel and swipes more responsive
				bContinue = false;
				break;
			default:
				// Fix bug that causes slideshows to display a blank black
				// first slide to break if bHandleAllCurrentEvents is false
				// when the event type does not requiring continuing
				if ( !bHandleAllCurrentEvents && pSalData->mpPresentationFrame && pSalData->mpPresentationFrame == pSalData->mpFocusFrame )
					bContinue = false;
				break;
		}
		pEvent->release();

		// Fix bug 2941 without triggering bugs 2962 and 2963 by
		// breaking if any frames have been created or destroyed
		if ( bContinue && pSalData->maFrameList.size() != nFrames )
			bContinue = false;
	}

	nCurrentTimeout = 0;

	AcquireYieldMutex( nCount );

	if ( !bMainEventLoop )
		aEventQueueMutex.release();

	JavaSalFrame::FlushAllFrames();

	// Update all objects
	for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
		(*it)->FlushAllObjects();

	// Allow main event loop to proceed
	if ( !pEvent && !pSalData->maNativeEventCondition.check() )
		pSalData->maNativeEventCondition.set();

	return nRet;
}

// -----------------------------------------------------------------------

bool JavaSalInstance::AnyInput( VclInputFlags nType )
{
	bool bRet = false;

	if ( nType & VclInputFlags::TIMER )
	{
		// Check timer
		SalData *pSalData = GetSalData();
		ImplSVData *pSVData = ImplGetSVData();
		if ( pSVData && pSVData->mpSalTimer && pSalData->mnTimerInterval )
		{
			timeval aCurrentTime;
			gettimeofday( &aCurrentTime, nullptr );
			if ( aCurrentTime >= pSalData->maTimeout )
				bRet = true;
		}
	}

	if ( !bRet )
		bRet = JavaSalEventQueue::anyCachedEvent( nType );

	return bRet;
}

// -----------------------------------------------------------------------

SalFrame* JavaSalInstance::CreateChildFrame( SystemParentData* /* pParent */, SalFrameStyleFlags /* nStyle */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalInstance::CreateChildFrame not implemented\n" );
#endif
	return nullptr;
}

// -----------------------------------------------------------------------

SalFrame* JavaSalInstance::CreateFrame( SalFrame* pParent, SalFrameStyleFlags nSalFrameStyle )
{
	JavaSalFrame *pFrame = new JavaSalFrame( nSalFrameStyle, static_cast< JavaSalFrame* >( pParent ) );
	if ( !pFrame->mpWindow )
	{
		delete pFrame;
		return nullptr;
	}

	// Get work area of the parent window or, if no parent, the main screen
	tools::Rectangle aWorkArea( Point( 0, 0 ), Size( 0, 0 ) );
	pFrame->GetWorkArea( aWorkArea );

	// Set default window size based on style
	long nX = aWorkArea.Left();
	long nY = aWorkArea.Top();
	long nWidth = aWorkArea.GetWidth();
	long nHeight = aWorkArea.GetHeight();
	if ( nSalFrameStyle & SalFrameStyleFlags::FLOAT )
	{
		nWidth = 10;
		nHeight = 10;
	}
	else
	{
		if ( nSalFrameStyle & SalFrameStyleFlags::SIZEABLE && nSalFrameStyle & SalFrameStyleFlags::MOVEABLE )
		{
			nWidth = static_cast< long >( nWidth * 0.8 );
			nHeight = static_cast< long >( nHeight * 0.8 );

			// Don't let width be too much wider than height as it can look too
			// wide on many LCD screens
			long nMaxWidth = static_cast< long >( nHeight * 4 / 3 );
			if ( nWidth > nMaxWidth )
				nWidth = nMaxWidth;
		}
		if ( !pFrame->mpParent )
		{
			SalData *pSalData = GetSalData();

			// Find the next document window if any exist
			JavaSalFrame* pNextFrame = nullptr;
			for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
			{
				if ( (*it) && (*it) != pFrame &&
					! (*it)->mpParent &&
					(*it)->mnStyle != SalFrameStyleFlags::DEFAULT &&
					(*it)->mnStyle & SalFrameStyleFlags::SIZEABLE &&
					(*it)->GetGeometry().nWidth &&
					(*it)->GetGeometry().nHeight )
						pNextFrame = *it;
			}

			if ( pNextFrame )
			{
				// Set screen to same screen as next frame
				pFrame->mbCenter = sal_False;
				const SalFrameGeometry& rGeom( pNextFrame->GetGeometry() );
				nX = rGeom.nX - rGeom.nLeftDecoration;
				nY = rGeom.nY - rGeom.nTopDecoration;
				nWidth = rGeom.nWidth;
				nHeight = rGeom.nHeight;
			}
		}
	}

	long nFlags = SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT;
	if ( !pFrame->mbCenter )
		nFlags |= SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y;
	pFrame->SetPosSize( nX, nY, nWidth, nHeight, nFlags );

	// Reset mbCenter flag to default value
	pFrame->mbCenter = sal_True;

	return pFrame;
}

// -----------------------------------------------------------------------

void JavaSalInstance::DestroyFrame( SalFrame* pFrame )
{
	if ( pFrame )
		delete pFrame;
}

// -----------------------------------------------------------------------

SalObject* JavaSalInstance::CreateObject( SalFrame* pParent, SystemWindowData* /* pWindowData */, bool bShow )
{
	JavaSalObject *pObject = new JavaSalObject( pParent );
	pObject->Show( bShow );
	return pObject;
}

// -----------------------------------------------------------------------

void JavaSalInstance::DestroyObject( SalObject* pObject )
{
	if ( pObject )
		delete pObject;
}

// -----------------------------------------------------------------------

void JavaSalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
	// Create a dummy queue for our dummy default printer
	SalPrinterQueueInfo *pInfo = new SalPrinterQueueInfo();
	pInfo->maPrinterName = GetDefaultPrinter();
	pInfo->mpSysData = nullptr;
	pList->Add( pInfo );
}

// -----------------------------------------------------------------------

void JavaSalInstance::GetPrinterQueueState( SalPrinterQueueInfo* /* pInfo */ )
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
	// Populate data
	if ( pSetupData )
	{
		pSetupData->SetSystem( JOBSETUP_SYSTEM_MAC );
		pSetupData->SetPrinterName( pQueueInfo->maPrinterName );
		pSetupData->SetDriver( pQueueInfo->maDriver );

		// Changing the driver data won't free it so set its length to zero
		pSetupData->SetDriverDataLen( 0 );
	}

	// Create a dummy printer configuration for our dummy printer
	return new JavaSalInfoPrinter( pSetupData );
}

// -----------------------------------------------------------------------

void JavaSalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
	if ( pPrinter )
		delete pPrinter;
}

// -----------------------------------------------------------------------

OUString JavaSalInstance::GetDefaultPrinter()
{
	// Create a dummy default printer
	SalData *pSalData = GetSalData();
	if ( !pSalData->maDefaultPrinter.getLength() )
	{
		ResMgr *pResMgr = ImplGetResMgr();
		if ( pResMgr )
			pSalData->maDefaultPrinter = OUString( ResId( SV_PRINT_DEFPRT_TXT, *pResMgr ) );
	}

	if ( !pSalData->maDefaultPrinter.getLength() )
		pSalData->maDefaultPrinter = "Printer";

	return pSalData->maDefaultPrinter;
}

// -----------------------------------------------------------------------

SalPrinter* JavaSalInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
	return new JavaSalPrinter( static_cast< JavaSalInfoPrinter* >( pInfoPrinter ) );
}

// -----------------------------------------------------------------------

void JavaSalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
	if ( pPrinter )
		delete pPrinter;
}

// -----------------------------------------------------------------------

SalVirtualDevice* JavaSalInstance::CreateVirtualDevice( SalGraphics* /* pGraphics */,
                                                    long& rDX, long& rDY,
                                                    DeviceFormat /* eFormat - ignore as Mac OS X bit count is always 32 */,
                                                    const SystemGraphicsData* /* pData */ )
{
	JavaSalVirtualDevice *pDevice = nullptr;

	pDevice = new JavaSalVirtualDevice();
	if ( pDevice->SetSize( rDX, rDY ) )
	{
		rDX = pDevice->GetWidth();
		rDY = pDevice->GetHeight();
	}
	else
	{
		rDX = 0;
		rDY = 0;
		delete pDevice;
		pDevice = nullptr;
	}

   	return pDevice;
}

// -----------------------------------------------------------------------

SalTimer* JavaSalInstance::CreateSalTimer()
{
    return new JavaSalTimer();
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
	return nullptr;
}

// -----------------------------------------------------------------------

OpenGLContext* JavaSalInstance::CreateOpenGLContext()
{
	return nullptr;
}

// -----------------------------------------------------------------------

SalSystem* JavaSalInstance::CreateSalSystem()
{
	return new JavaSalSystem();
}

// -----------------------------------------------------------------------

OUString JavaSalInstance::GetConnectionIdentifier()
{
	return OUString();
}

// -----------------------------------------------------------------------

void JavaSalInstance::AddToRecentDocumentList( const OUString& /* rFileUrl */, const OUString& /* rMimeType */, const OUString& /* rDocumentService */ )
{
	// Do not do anything as each document's NSDocument instance created in
	// sfx2/source/view/topfrm_cocoa.mm updates the OS X recent items menu
}

// =========================================================================

SalYieldMutex::SalYieldMutex() :
	mnCount( 0 ),
	mnThreadId( 0 ),
	mnReacquireThreadId( 0 )
{
	maMainThreadCondition.set();
	maReacquireThreadCondition.set();
}

// -------------------------------------------------------------------------

void SalYieldMutex::acquire()
{
	if ( mnThreadId != Thread::getCurrentIdentifier() )
	{
		// If we are in a signal handler and we don't have the mutex, don't
		// block waiting for the mutex as most likely the thread with the mutex
		// is blocked in an [NSObject performSelectorOnMainThread] message and
		// we will deadlock. Also, fix bug 1496 by not allowing native timers
		// to run before trying to acquire the mutex when in the main thread.
		SalData *pSalData = GetSalData();
		if ( !pSalData || ( pSalData->mbInSignalHandler && !tryToAcquire() ) )
		{
			return;
		}
		else if ( CFRunLoopGetCurrent() == CFRunLoopGetMain() )
		{
			// Wait for other thread to release mutex
			// We need to let any pending timers run so that we don't deadlock
			sal_uInt16 nIterationsSinceLastWakeUpEvent = 0;
			while ( !Application::IsShutDown() )
			{
				// Fix hanging in bug 3503 by posting a dummy event to wakeup
				// the VCL event thread if the VCL event dispatch thread is in
				// a potentially long wait. Fix hanging when selecting a menu
				// while Writer is doing a background spellcheck run by posting
				// a wake up event so that
				// JavaSalInstance::AnyEvent( VclInputFlags::OTHER ) will eventually
				// succeed.
				if ( ++nIterationsSinceLastWakeUpEvent % 10000 == 0 || nCurrentTimeout > 100 )
				{
					nIterationsSinceLastWakeUpEvent = 0;

					TimeValue aDelay;
					aDelay.Seconds = 0;
					aDelay.Nanosec = 10000;
					maMainThreadCondition.reset();
					Application_postWakeUpEvent();
					if ( !maMainThreadCondition.check() )
						maMainThreadCondition.wait( &aDelay );
					maMainThreadCondition.set();
				}

				CFRunLoopRunInMode( CFSTR( "AWTRunLoopMode" ), 0, false );

				// Fix hanging that occurs when the native Open dialog and the 
				// the OOo "Enter password" dialogs are both displayed and the
				// OOo dialog is closed first by dispatching any pending
				// NSModalPanelRunLoopMode events
				if ( NSApplication_getModalWindow() )
					NSApplication_dispatchPendingEvents( bInNativeDragPrint, sal_False );

				// Fix crashing bug when quitting by checking if another thread
				// has started shutting down the application to avoid using this
				// instance after it has been deleted
				if ( Application::IsShutDown() || tryToAcquire() )
					break;
			}

			return;
		}
	}

	WaitForReacquireThread();

	maMutex.acquire();
	mnThreadId = Thread::getCurrentIdentifier();
	mnCount++;
	if ( mnReacquireThreadId == Thread::getCurrentIdentifier() )
	{
		mnReacquireThreadId = 0;
		maReacquireThreadCondition.set();
	}
}

// -------------------------------------------------------------------------

void SalYieldMutex::release()
{
	if ( mnThreadId == Thread::getCurrentIdentifier() )
	{
		if ( mnCount == 1 )
			mnThreadId = 0;

		if ( mnCount )
			mnCount--;

		// Notify main thread that it can grab the mutex
		if ( !mnCount && !maMainThreadCondition.check() )
		{
			maMainThreadCondition.set();
			Thread::yield();
		}
	}

	maMutex.release();
}

// -------------------------------------------------------------------------

bool SalYieldMutex::tryToAcquire()
{
	WaitForReacquireThread();

	if ( maMutex.tryToAcquire() )
	{
		mnThreadId = Thread::getCurrentIdentifier();
		mnCount++;
		if ( mnReacquireThreadId == Thread::getCurrentIdentifier() )
		{
			mnReacquireThreadId = 0;
			maReacquireThreadCondition.set();
		}

		return true;
	}
	else
		return false;
}

// -------------------------------------------------------------------------

sal_uLong SalYieldMutex::ReleaseAcquireCount()
{
	sal_uLong nRet = 0;

	// Never release the mutex in the main thread as it can cause crashing
	// when dragging when the OOo code's VCL event dispatching thread runs
	// while we are in the middle of a native drag event
	if ( ( ( !bAllowReleaseYieldMutex && !bInUnitTest ) || bInNativeDragPrint ) && CFRunLoopGetCurrent() == CFRunLoopGetMain() )
		return nRet;

	if ( mnThreadId == Thread::getCurrentIdentifier() )
	{
		if ( mnCount )
		{
			// If this thread is not the main thread, make other threads aware
			// that this thread should have priority for reacquiring the mutex
			if ( CFRunLoopGetCurrent() != CFRunLoopGetMain() )
			{
				mnReacquireThreadId = Thread::getCurrentIdentifier();
				maReacquireThreadCondition.reset();
			}

			nRet = mnCount;
			while ( mnCount && mnThreadId == Thread::getCurrentIdentifier() )
				release();
		}
	}

	return nRet;
}

// -------------------------------------------------------------------------

void SalYieldMutex::WaitForReacquireThread()
{
	if ( mnReacquireThreadId && mnReacquireThreadId != Thread::getCurrentIdentifier() && mnThreadId != Thread::getCurrentIdentifier() && CFRunLoopGetCurrent() != CFRunLoopGetMain() )
	{
		// Fix hang that occurs when the native frame is being created on a
    	// thread other than the OOo event dispatch thread while opening a Base
		// document by letting the thread that invoked ReleaseAcquireCount()
		// have a chance to reacquire the lock first. There may be cases where
		// the reacquiring thread actually expects the current thread to do
		// some work so stop waiting after a short period of time.
		if ( !Application::IsShutDown() && !maReacquireThreadCondition.check() )
		{
			TimeValue aDelay;
			aDelay.Seconds = 0;
			aDelay.Nanosec = 10000;
			maReacquireThreadCondition.wait( &aDelay );
		}
	}
}

// =========================================================================

JavaSalEvent::JavaSalEvent( SalEvent nID, JavaSalFrame *pFrame, void *pData, const OString& rPath, sal_uLong nCommittedCharacters, sal_uLong nCursorPosition ) :
	mnID( nID  ),
	mpFrame( pFrame ),
	mbNative( false ),
	mbShutdownCancelled( sal_False ),
	mnCommittedCharacters( nCommittedCharacters ),
	mnCursorPosition( nCursorPosition ),
	mpData( pData ),
	mnRefCount( 1 )
{
	switch ( mnID )
	{
		case SalEvent::Close:
		case SalEvent::EndExtTextInput:
		case SalEvent::ExtTextInput:
		case SalEvent::ExtTextInputPos:
		case SalEvent::GetFocus:
		case SalEvent::LoseFocus:
		case SalEvent::KeyInput:
		case SalEvent::KeyModChange:
		case SalEvent::KeyUp:
		case SalEvent::MouseButtonDown:
		case SalEvent::MouseButtonUp:
		case SalEvent::MouseLeave:
		case SalEvent::MouseMove:
		case SalEvent::Move:
		case SalEvent::MoveResize:
		case SalEvent::Paint:
		case SalEvent::Resize:
		case SalEvent::WheelMouse:
			mbNative = true;
			break;
		default:
			break;
	}

	maPath = OUString( rPath.getStr(), rPath.getLength(), RTL_TEXTENCODING_UTF8 );
}

// -------------------------------------------------------------------------

JavaSalEvent::~JavaSalEvent()
{
	if ( mpData )
	{
		switch ( mnID )
		{
			case SalEvent::EndExtTextInput:
			case SalEvent::ExtTextInput:
			{
				SalExtTextInputEvent *pInputEvent = static_cast< SalExtTextInputEvent* >( mpData );
				if ( pInputEvent->mpTextAttr )
					rtl_freeMemory( const_cast< ExtTextInputAttr* >( pInputEvent->mpTextAttr ) );
				delete pInputEvent;
				break;
			}
			case SalEvent::ExtTextInputPos:
			{
				SalExtTextInputPosEvent *pInputPosEvent = static_cast< SalExtTextInputPosEvent* >( mpData );
				delete pInputPosEvent;
				break;
			}
			case SalEvent::KeyInput:
			case SalEvent::KeyUp:
			{
				SalKeyEvent *pKeyEvent = static_cast< SalKeyEvent* >( mpData );
				delete pKeyEvent;
				break;
			}
			case SalEvent::KeyModChange:
			{
				SalKeyModEvent *pKeyModEvent = static_cast< SalKeyModEvent* >( mpData );
				delete pKeyModEvent;
				break;
			}
			case SalEvent::MouseButtonDown:
			case SalEvent::MouseButtonUp:
			case SalEvent::MouseLeave:
			case SalEvent::MouseMove:
			{
				SalMouseEvent *pMouseEvent = static_cast< SalMouseEvent* >( mpData );
				delete pMouseEvent;
				break;
			}
			case SalEvent::Move:
			case SalEvent::MoveResize:
			case SalEvent::Resize:
			{
				tools::Rectangle *pPosSize = static_cast< tools::Rectangle* >( mpData );
				delete pPosSize;
				break;
			}
			case SalEvent::Paint:
			{
				SalPaintEvent *pPaintEvent = static_cast< SalPaintEvent* >( mpData );
				delete pPaintEvent;
				break;
			}
			case SalEvent::WheelMouse:
			{
				SalWheelMouseEvent *pWheelMouseEvent = static_cast< SalWheelMouseEvent* >( mpData );
				delete pWheelMouseEvent;
				break;
			}
			case SalEvent::CommandMediaData:
			{
				CommandMediaData *pCommandMediaData = static_cast< CommandMediaData* >( mpData );
				delete pCommandMediaData;
				break;
			}
			default:
				break;
		}
	}

	while ( maOriginalKeyEvents.size() )
	{
		JavaSalEvent *pEvent = maOriginalKeyEvents.front();
		maOriginalKeyEvents.pop_front();
		pEvent->release();
	}
}

// -------------------------------------------------------------------------

void JavaSalEvent::addRepeatCount( sal_uInt16 nCount )
{
	if ( mpData && mnID == SalEvent::KeyInput )
	{
		SalKeyEvent *pKeyEvent = static_cast< SalKeyEvent* >( mpData );
		pKeyEvent->mnRepeat += nCount;
	}
}

// -------------------------------------------------------------------------

void JavaSalEvent::addOriginalKeyEvent( JavaSalEvent *pEvent )
{
	if ( pEvent && ( mnID == SalEvent::KeyInput || mnID == SalEvent::KeyUp ) )
	{
		pEvent->reference();
		maOriginalKeyEvents.push_back( pEvent );
	}
}

// -------------------------------------------------------------------------

void JavaSalEvent::addUpdateRect( const tools::Rectangle& rRect )
{
	if ( mpData && mnID == SalEvent::Paint )
	{
		tools::Rectangle aUpdateRect( getUpdateRect() );
		aUpdateRect.Justify();
		
		tools::Rectangle aRect( rRect );
		aRect.Justify();

		if ( aRect.GetWidth() > 0 && aRect.GetHeight() > 0 )
		{
			if ( aUpdateRect.GetWidth() > 0 && aUpdateRect.GetHeight() > 0 )
				aRect = aRect.GetUnion( aUpdateRect );

			SalPaintEvent *pPaintEvent = static_cast< SalPaintEvent* >( mpData );
			pPaintEvent->mnBoundX = aRect.Left();
			pPaintEvent->mnBoundY = aRect.Top();
			pPaintEvent->mnBoundWidth = aRect.GetWidth();
			pPaintEvent->mnBoundHeight = aRect.GetHeight();
		}
	}
}

// -------------------------------------------------------------------------

bool JavaSalEvent::addWheelRotationAndScrollLines( long nRotation, sal_uLong nScrollLines, sal_Bool bHorizontal )
{
	bool bRet = false;

	if ( mpData && mnID == SalEvent::WheelMouse && nScrollLines != SAL_WHEELMOUSE_EVENT_PAGESCROLL )
	{
		SalWheelMouseEvent *pWheelMouseEvent = static_cast< SalWheelMouseEvent* >( mpData );
		if ( pWheelMouseEvent->mbHorz == bHorizontal )
		{
			// Suppress excessive magnification but not adding rotation for
			// magnify events
			if ( ! ( pWheelMouseEvent->mnCode & KEY_MOD1 ) )
				pWheelMouseEvent->mnDelta += nRotation;
			pWheelMouseEvent->mnNotchDelta = ( pWheelMouseEvent->mnDelta < 0 ? -1 : 1 );
			pWheelMouseEvent->mnScrollLines = labs( pWheelMouseEvent->mnDelta );
			bRet = true;
		}
	}

	return bRet;
}

// -------------------------------------------------------------------------

void JavaSalEvent::cancelShutdown()
{
	if ( mnID == SalEvent::Shutdown )
		mbShutdownCancelled = sal_True;
}

// -------------------------------------------------------------------------

void JavaSalEvent::dispatch()
{
	SalEvent nID = mnID;
	SalData *pSalData = GetSalData();

	// Handle events that do not need a JavaSalFrame pointer
	switch ( nID )
	{
		case SalEvent::Shutdown:
		{
			bool bCancelShutdown = true;

			// Ignore SalEvent::Shutdown events when recursing into this
			// method or when in presentation mode
			if ( !isShutdownCancelled() )
			{
				// Do not shutdown if any popups are visible since any shutdown
				// event dispatched will cause a crash when a second level
				// popup menu is open. Fix crashing bug while saving reported
				// in the following NeoOffice support forum post by ignoring
				// shutdown requests when JavaSalInstance::Yield() has been
				// recursively called:
				// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=63716#63716
				ImplSVData *pSVData = ImplGetSVData();
				if ( pSVData && !pSVData->maWinData.mpFirstFloat && !pSVData->maWinData.mpLastExecuteDlg && !pSalData->mbInNativeModalSheet && pSalData->maFrameList.size() && pSVData->maAppData.mnDispatchLevel < 3 )
				{
					JavaSalFrame *pFrame = pSalData->maFrameList.front();
					if ( pFrame && !pFrame->CallCallback( nID, nullptr ) )
						bCancelShutdown = false;
				}
			}

			if ( bCancelShutdown )
			{
				cancelShutdown();

				// Invoke the native shutdown cancelled handler
				JavaSalEventQueue::setShutdownDisabled( sal_False );
			}

			return;
		}
		case SalEvent::OpenDocument:
		case SalEvent::PrintDocument:
		{
			// Fix bug 168 && 607 by reposting SalEvent::*DOCUMENT events when
			// recursing into this method while opening a document
			ImplSVData *pSVData = ImplGetSVData();
			if ( pSVData && pSVData->maAppData.mnDispatchLevel == 1 && !pSVData->maWinData.mpLastExecuteDlg && !pSalData->mbInNativeModalSheet )
			{
				std::vector< OUString > aAppEvtData;
				aAppEvtData.push_back( getPath() );
				ApplicationEvent aAppEvt( nID == SalEvent::OpenDocument ? ApplicationEvent::Type::Open : ApplicationEvent::Type::Print, aAppEvtData );
				pSVData->mpApp->AppEvent( aAppEvt );
			}
			else
			{
				reference();
				pSalData->maPendingDocumentEventsList.push_back( this );
			}
			return;
		}
		case SalEvent::ScreenParamsChanged:
		{
			bool bChangedResolution = false;
			for ( std::list< JavaSalVirtualDevice * >::const_iterator it = pSalData->maVirDevList.begin(); it != pSalData->maVirDevList.end(); ++it )
			{
				if ( (*it)->ScreenParamsChanged() )
					bChangedResolution = true;
			}
			for ( std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
			{
				if ( (*it)->ScreenParamsChanged() )
					bChangedResolution = true;
 			}

			// Force complete redraw of all windows if any windows or virtual
			// devices changed resolution
			if ( bChangedResolution )
			{
				JavaSalEvent aEvent( SalEvent::SystemColorsChanged, nullptr, nullptr );
				aEvent.dispatch();
			}
			return;
		}
		case SalEvent::SystemColorsChanged:
		{
			ImplSVData *pSVData = ImplGetSVData();
			if ( pSVData )
			{
				// Reset the radio button and checkbox images
				pSVData->maCtrlData.maRadioImgList.clear();
				pSVData->maCtrlData.maCheckImgList.clear();

				// Dispatch a settings changed event as it appears to more
				// thoroughly propagate system color changes when changing
				// to or from macOS Dark Mode
				JavaSalFrame *pFrame = pSalData->maFrameList.front();
				if ( pFrame )
					pFrame->CallCallback( SalEvent::SettingsChanged, nullptr );

				// Invalidate all top level windows just to be safe
				Window *pWindow = Application::GetFirstTopLevelWindow();
				while ( pWindow )
				{
					pWindow->Invalidate();
					pWindow = Application::GetNextTopLevelWindow( pWindow );
				}
			}
			return;
		}
		case SalEvent::CommandMediaData:
		{
			Window *pWindow = nullptr;
			if ( pSalData->mpPresentationFrame && pSalData->mpPresentationFrame->mbVisible )
				pWindow = pSalData->mpPresentationFrame->GetWindow();
			if ( !pWindow && pSalData->mpFocusFrame && pSalData->mpFocusFrame->mbVisible )
				pWindow = pSalData->mpFocusFrame->GetWindow();
			if ( !pWindow )
				pWindow = Application::GetFirstTopLevelWindow();

			CommandMediaData *pCommandMediaData = static_cast< CommandMediaData* >( mpData );
			if ( pWindow && pCommandMediaData )
			{
				MediaCommand nCommand = pCommandMediaData->GetMediaId();
				if ( nCommand == MediaCommand::Play && pSalData->mpPresentationFrame && pSalData->mpPresentationFrame->mbVisible )
					nCommand = MediaCommand::PlayPause;
				CommandMediaData aModifiedCommandMediaData( nCommand );
				CommandEvent aCommandEvent( Point(), CommandEventId::Media, false, &aModifiedCommandMediaData );
				NotifyEvent aNotifyEvent( MouseNotifyEvent::COMMAND, pWindow, &aCommandEvent );
				if ( !ImplCallPreNotify( aNotifyEvent ) )
					pWindow->Command( aCommandEvent );
			}

			return;
		}
		case SalEvent::About:
		{
			JavaSalFrame *pFrame = pSalData->mpFocusFrame;
			if ( pFrame && pFrame->mbVisible )
			{
				// Pass all events received by a utility window to its parent
				// window
				while ( pFrame->mpParent && pFrame->mpParent->mbVisible && pFrame->IsUtilityWindow() )
					pFrame = pFrame->mpParent;
				pFrame->CallCallback( SalEvent::ShowDialog, reinterpret_cast< void* >( ShowDialogId::About ) );
			}
			else
			{
				ImplSVData *pSVData = ImplGetSVData();
				if ( pSVData )
				{
					ApplicationEvent aAppEvent( ApplicationEvent::Type::ShowDialog, "ABOUT" );
					pSVData->mpApp->AppEvent( aAppEvent );
				}
			}

			return;
		}
		case SalEvent::Preferences:
		{
			JavaSalFrame *pFrame = pSalData->mpFocusFrame;
			if ( pFrame && pFrame->mbVisible )
			{
				// Pass all events received by a utility window to its parent
				// window
				while ( pFrame->mpParent && pFrame->mpParent->mbVisible && pFrame->IsUtilityWindow() )
					pFrame = pFrame->mpParent;
				pFrame->CallCallback( SalEvent::ShowDialog, reinterpret_cast< void* >( ShowDialogId::Preferences ) );
			}
			else
			{
				ImplSVData *pSVData = ImplGetSVData();
				if ( pSVData )
				{
					ApplicationEvent aAppEvent( ApplicationEvent::Type::ShowDialog, "PREFERENCES" );
					pSVData->mpApp->AppEvent( aAppEvent );
				}
			}

			return;
		}
		case SalEvent::WakeUp:
		{
			return;
		}
		default:
			break;
	}
	
	// Handle events that require a JavaSalFrame pointer
	JavaSalFrame *pFrame = FindValidFrame( getFrame() );

	if ( pSalData->mbInNativeModalSheet && pSalData->mpNativeModalSheetFrame && pFrame != pSalData->mpNativeModalSheetFrame )
	{
		// We need to prevent dispatching of events other than system events
		// like bounds change or paint events. Fix bug 3429 by only forcing
		// a focus change if there is not a native modal window showing.
		if ( NSApplication_isActive() )
			pSalData->mpNativeModalSheetFrame->ToTop( SalFrameToTop::RestoreWhenMin | SalFrameToTop::GrabFocus );

		// Fix document lockup bug reported in the following NeoOffice forum
		// topic by dispatching bounds change, paint events, and user events
		// even through we are ignoring other events:
		// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8527
		switch ( nID )
		{
			case SalEvent::Move:
			case SalEvent::MoveResize:
			case SalEvent::Paint:
			case SalEvent::Resize:
			case SalEvent::UserEvent:
				break;
			default:
				return;
		}
	}

	switch ( nID )
	{
		case SalEvent::Close:
		{
			if ( pFrame && pFrame->mbVisible )
				pFrame->CallCallback( nID, nullptr );
			break;
		}
		case SalEvent::Deminimized:
		{
			// Fix bug 3649 by not hiding any windows when minimizing
			break;
		}
		case SalEvent::Minimized:
		{
			// Fix bug 3649 by not hiding any windows when minimizing. Fix bug
			// reported in the following NeoOffice forum post where selecting
			// the focussed document window in the Window menu fails to
			// unminimize the document window by dispatching a lose focus event
			// when the window is minimized:
			// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=63242#63242
			if ( pFrame && pFrame == pSalData->mpFocusFrame )
			{
				// Update menus before giving up focus otherwise the window
				// list in the windows menu will not include other minimized
				// windows
				UpdateMenusForFrame( pFrame, nullptr, true );
				JavaSalEvent aEvent( SalEvent::LoseFocus, pFrame, nullptr );
				aEvent.dispatch();
			}
			break;
		}
		case SalEvent::FullScreenEntered:
		case SalEvent::FullScreenExited:
		{
			sal_Bool bFullScreen = ( nID == SalEvent::FullScreenEntered ? sal_True : sal_False );
			if ( pFrame && !pFrame->mbInShowFullScreen && bFullScreen != pFrame->mbFullScreen )
			{
				Window *pWindow = Application::GetFirstTopLevelWindow();
				while ( pWindow && pWindow->ImplGetFrame() != pFrame )
					pWindow = Application::GetNextTopLevelWindow( pWindow );

				if ( pWindow )
				{
					uno::Reference< frame::XFramesSupplier > xFramesSupplier( ::comphelper::getProcessServiceFactory()->createInstance( "com.sun.star.frame.Desktop" ), uno::UNO_QUERY );
					if ( xFramesSupplier.is() )
					{
						uno::Reference< container::XIndexAccess > xList( xFramesSupplier->getFrames(), uno::UNO_QUERY );
						if ( xList.is() )
						{
							sal_Int32 nCount = xList->getCount();
							for ( sal_Int32 i = 0; i < nCount; i++ )
							{
								uno::Reference< frame::XFrame > xFrame;
								xList->getByIndex( i ) >>= xFrame;
								if ( xFrame.is() )
								{
									uno::Reference< awt::XWindow > xWindow = xFrame->getComponentWindow();
									if ( xWindow.is() )
									{
										Window *pCurrentWindow = VCLUnoHelper::GetWindow( xWindow );
										while ( pCurrentWindow && pCurrentWindow != pWindow && pCurrentWindow->GetParent() )
											pCurrentWindow = pCurrentWindow->GetParent();
										if ( pCurrentWindow == pWindow )
										
										{
											uno::Reference< frame::XDispatchProvider > xDispatchProvider( xFrame, uno::UNO_QUERY );
											if ( xDispatchProvider.is() )
											{
												uno::Reference< frame::XDispatchHelper > xDispatchHelper( ::comphelper::getProcessServiceFactory()->createInstance( "com.sun.star.frame.DispatchHelper" ), uno::UNO_QUERY );
												if ( xDispatchHelper.is() )
												{
													pFrame->mbInWindowDidExitFullScreen = !bFullScreen;
													pFrame->mbInWindowWillEnterFullScreen = bFullScreen;

													try
													{
														xDispatchHelper->executeDispatch( xDispatchProvider, ".uno:FullScreen", "_self", 0, uno::Sequence< beans::PropertyValue >() );
													}
													catch ( ... )
													{
													}

													pFrame->mbInWindowDidExitFullScreen = sal_False;
													pFrame->mbInWindowWillEnterFullScreen = sal_False;
												}
											}

											break;
										}
									}
								}
							}
						}
					}
				}

				// Fix incorrect content bounds when presentations are in
				// full screen mode by explicitly setting the window's full
				// screen flag since some document types never call
				// JavaSalFrame::ShowFullScreen()
				if ( pFrame->mbFullScreen != bFullScreen )
				{
					pFrame->mbFullScreen = bFullScreen;

					JavaSalEvent *pMoveResizeEvent = new JavaSalEvent( SalEvent::MoveResize, pFrame, nullptr );
					JavaSalEventQueue::postCachedEvent( pMoveResizeEvent );
					pMoveResizeEvent->release();
				}
			}
			break;
		}
		case SalEvent::EndExtTextInput:
		{
			if ( pFrame && pFrame->mbVisible )
			{
				SalExtTextInputEvent *pInputEvent = static_cast< SalExtTextInputEvent* >( mpData );

				// Fix bug 1158 by resetting the focus to whichever window is
				// receiving key events
				if ( pFrame != pSalData->mpFocusFrame )
				{
					JavaSalEvent aEvent( SalEvent::GetFocus, pFrame, nullptr );
					aEvent.dispatch();
				}
				pFrame->CallCallback( nID, pInputEvent );
			}
			break;
		}
		case SalEvent::ExtTextInput:
		{
			if ( pFrame && pFrame->mbVisible )
			{
				SalExtTextInputEvent *pInputEvent = static_cast< SalExtTextInputEvent* >( mpData );
				sal_uLong nCommitted = getCommittedCharacterCount();
				if ( !pInputEvent )
				{
					sal_uLong nCursorPos = getCursorPosition();
					pInputEvent = new SalExtTextInputEvent();
					pInputEvent->maText = getText();
					pInputEvent->mpTextAttr = getTextAttributes();
					pInputEvent->mnCursorPos = nCursorPos > nCommitted ? nCursorPos : nCommitted;
					pInputEvent->mnCursorFlags = 0;

					mpData = pInputEvent;
				}
				// Fix bug 1158 by resetting the focus to whichever window is
				// receiving key events
				if ( pFrame != pSalData->mpFocusFrame )
				{
					JavaSalEvent aEvent( SalEvent::GetFocus, pFrame, nullptr );
					aEvent.dispatch();
				}
				pFrame->CallCallback( nID, pInputEvent );
				pFrame = FindValidFrame( pFrame );
				// If there is no text, the character is committed
				if ( pFrame && static_cast< sal_uLong >( pInputEvent->maText.getLength() ) == nCommitted )
					pFrame->CallCallback( SalEvent::EndExtTextInput, nullptr );
			}
			break;
		}
		case SalEvent::ExtTextInputPos:
		{
			SalExtTextInputPosEvent *pInputPosEvent = static_cast< SalExtTextInputPosEvent* >( mpData );
			if ( pInputPosEvent && pFrame && pFrame->mbVisible )
				pFrame->CallCallback( SalEvent::ExtTextInputPos, pInputPosEvent );
			break;
		}
		case SalEvent::GetFocus:
		{
			// Ignore focus events for floating windows
			if ( pFrame != pSalData->mpFocusFrame )
			{
				if ( pSalData->mpFocusFrame && pSalData->mpFocusFrame->mbVisible )
					pSalData->mpFocusFrame->CallCallback( SalEvent::LoseFocus, nullptr );
				pSalData->mpFocusFrame = nullptr;
			}

			if ( pFrame && pFrame->mbVisible && !pFrame->IsFloatingFrame() )
			{
				pSalData->mpFocusFrame = pFrame;
				pFrame->CallCallback( nID, nullptr );
				pFrame = FindValidFrame( pFrame );
				JavaSalMenu::SetMenuBarToFocusFrame();

				if ( pFrame )
				{
					Window *pWindow = Application::GetFirstTopLevelWindow();
					while ( pWindow && pWindow->ImplGetFrame() != pFrame )
						pWindow = Application::GetNextTopLevelWindow( pWindow );
					InvalidateControls( pWindow );
				}
			}
			else
			{
				JavaSalMenu::SetMenuBarToFocusFrame();
			}
			break;
		}
		case SalEvent::LoseFocus:
		{
			if ( pFrame && pFrame == pSalData->mpFocusFrame )
			{
				pSalData->mpFocusFrame = nullptr;
				pFrame->CallCallback( nID, nullptr );
				pFrame = FindValidFrame( pFrame );
				JavaSalMenu::SetMenuBarToFocusFrame();
			}

			if ( pFrame )
			{
				// Fix bug 3098 by hiding tooltip windows but leaving the
				// visible flag set to true
				for ( ::std::list< JavaSalFrame* >::const_iterator cit = pFrame->maChildren.begin(); cit != pFrame->maChildren.end(); ++cit )
				{
					if ( (*cit)->mbVisible && (*cit)->mnStyle & SalFrameStyleFlags::TOOLTIP )
						(*cit)->SetVisible( sal_False, sal_False );
				}

				if ( pFrame->mbVisible && !pFrame->IsFloatingFrame() )
				{
					Window *pWindow = Application::GetFirstTopLevelWindow();
					while ( pWindow && pWindow->ImplGetFrame() != pFrame )
						pWindow = Application::GetNextTopLevelWindow( pWindow );
					InvalidateControls( pWindow );
				}
			}
			break;
		}
		case SalEvent::KeyInput:
		case SalEvent::KeyUp:
		{
			if ( pFrame && pFrame->mbVisible )
			{
				SalKeyEvent *pKeyEvent = static_cast< SalKeyEvent* >( mpData );
				if ( !pKeyEvent )
				{
					pKeyEvent = new SalKeyEvent();
					pKeyEvent->mnTime = getWhen();
					pKeyEvent->mnCode = getKeyCode() | getModifiers();
					pKeyEvent->mnCharCode = getKeyChar();
					pKeyEvent->mnRepeat = getRepeatCount();

					mpData = pKeyEvent;
				}

				// Fix bug reported in the following NeoOffice forum post by
				// ignoring character codes that are in the Unicode private use
				// use area and there is a non-alphanumeric key code:
				// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=63098#63098
				if ( pKeyEvent->mnCharCode >= 0xe000 && pKeyEvent->mnCharCode < 0xf900 && ( getKeyCode() & ~( KEYGROUP_NUM | KEYGROUP_ALPHA ) ) )
					pKeyEvent->mnCharCode = 0;

				// Fix bug 1158 by resetting the focus to whichever window is
				// receiving key events
				if ( pFrame != pSalData->mpFocusFrame )
				{
					JavaSalEvent aEvent( SalEvent::GetFocus, pFrame, nullptr );
					aEvent.dispatch();
				}
				// Pass all potential menu shortcuts received by a utility
				// window to its parent window. Fix bug 3432 by not sending
				// Command-Shift-F10 to parent.
				if ( pKeyEvent->mnCode & KEY_MOD1 && pKeyEvent->mnCode != ( KEY_MOD1 | KEY_SHIFT | KEY_F10 ) )
				{
					while ( pFrame->mpParent && pFrame->mpParent->mbVisible && pFrame->IsUtilityWindow() )
						pFrame = pFrame->mpParent;
				}
				// Fix bug reported in the following NeoOffice forum post that
				// by only skipping a key binding event when there are original
				// events attached to the key event event:
				// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=62803#62803
				if ( ( !pFrame->mbAllowKeyBindings && maOriginalKeyEvents.size() ) || !pFrame->CallCallback( nID, pKeyEvent ) )
				{
					// If the key event fails and this is a command event,
					// dispatch the original events
					JavaSalEvent *pEvent;
					while ( ( pEvent = getNextOriginalKeyEvent() ) != nullptr )
					{
						pEvent->dispatch();
						pEvent->release();
					}
				}
			}
			break;
		}
		case SalEvent::KeyModChange:
		{
			if ( pFrame && pFrame->mbVisible )
			{
				SalKeyModEvent *pKeyModEvent = static_cast< SalKeyModEvent* >( mpData );
				if ( !pKeyModEvent )
				{
					pKeyModEvent = new SalKeyModEvent();
					pKeyModEvent->mnTime = getWhen();
					pKeyModEvent->mnCode = getModifiers();
					pKeyModEvent->mnModKeyCode = ModKeyFlags::NONE;

					mpData = pKeyModEvent;
				}
				pFrame->CallCallback( nID, pKeyModEvent );
			}
			break;
		}
		case SalEvent::MouseButtonDown:
		case SalEvent::MouseButtonUp:
		case SalEvent::MouseLeave:
		case SalEvent::MouseMove:
		{
			if ( pFrame && pFrame->mbVisible )
			{
				SalMouseEvent *pMouseEvent = static_cast< SalMouseEvent* >( mpData );
				if ( !pMouseEvent )
				{
					sal_uInt16 nModifiers = getModifiers();
					pMouseEvent = new SalMouseEvent();
					pMouseEvent->mnTime = getWhen();
					pMouseEvent->mnX = getX();
					pMouseEvent->mnY = getY();
					pMouseEvent->mnCode = nModifiers;
					if ( nID == SalEvent::MouseLeave || nID == SalEvent::MouseMove )
						pMouseEvent->mnButton = 0;
					else
						pMouseEvent->mnButton = nModifiers & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT );

					mpData = pMouseEvent;
				}

				sal_uInt16 nButtons = pMouseEvent->mnCode & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT );
				if ( nID == SalEvent::MouseMove )
				{
					if ( nButtons && !pSalData->mpLastDragFrame )
						pSalData->mpLastDragFrame = pFrame;
					if ( !nButtons && !pSalData->mpLastMouseMoveFrame )
						pSalData->mpLastMouseMoveFrame = pFrame;
				}

				// Find the real mouse frame
				JavaSalFrame *pOriginalFrame = pFrame;
				Point aScreenPoint( pMouseEvent->mnX + pFrame->maGeometry.nX, pMouseEvent->mnY + pFrame->maGeometry.nY );
				if ( pSalData->mpCaptureFrame && pSalData->mpCaptureFrame->mbVisible )
				{
					if ( pSalData->mpCaptureFrame != pFrame )
					{
						pMouseEvent->mnX = aScreenPoint.X() - pSalData->mpCaptureFrame->maGeometry.nX;
						pMouseEvent->mnY = aScreenPoint.Y() - pSalData->mpCaptureFrame->maGeometry.nY;
						pFrame = pSalData->mpCaptureFrame;
					}
				}
				else if ( nID != SalEvent::MouseLeave )
				{
					JavaSalFrame *pMouseFrame = FindMouseEventFrame( pFrame, aScreenPoint );
					if ( pMouseFrame && pMouseFrame != pFrame && pMouseFrame->mbVisible )
					{
						pMouseEvent->mnX = aScreenPoint.X() - pMouseFrame->maGeometry.nX;
						pMouseEvent->mnY = aScreenPoint.Y() - pMouseFrame->maGeometry.nY;
						pFrame = pMouseFrame;
					}
				}

				// If we are releasing after dragging, send the event to the
				// last dragged frame
				if ( pSalData->mpLastDragFrame && ( nID == SalEvent::MouseButtonUp || nID == SalEvent::MouseMove ) )
				{
 					if ( pSalData->mpLastDragFrame != pFrame && pSalData->mpLastDragFrame->mbVisible )
					{
						// If dragging and there are floating windows visible,
						// don't let the mouse fall through to a non-floating
						// window
						if ( nID == SalEvent::MouseButtonUp || ( pFrame == pOriginalFrame && !pFrame->IsFloatingFrame() ) )
						{
							pMouseEvent->mnX = aScreenPoint.X() - pSalData->mpLastDragFrame->maGeometry.nX;
							pMouseEvent->mnY = aScreenPoint.Y() - pSalData->mpLastDragFrame->maGeometry.nY;
							pFrame = pSalData->mpLastDragFrame;
						}
					}

					if ( nButtons )
						pSalData->mpLastDragFrame = pFrame;
					else
						pSalData->mpLastDragFrame = nullptr;
				}

				// Create a synthetic mouse leave event when a mouse move event
				// resolves to a different window than the last mouse move event
				// or mouse position is outside of the window's content area
				if ( pSalData->mpLastMouseMoveFrame && nID == SalEvent::MouseMove && !nButtons )
				{
					if ( pSalData->mpLastMouseMoveFrame != pFrame )
					{
						nID = SalEvent::MouseLeave;
						pMouseEvent->mnX = aScreenPoint.X() - pSalData->mpLastMouseMoveFrame->maGeometry.nX;
						pMouseEvent->mnY = aScreenPoint.Y() - pSalData->mpLastMouseMoveFrame->maGeometry.nY;

						JavaSalFrame *pLastMouseMoveFrame = pSalData->mpLastMouseMoveFrame;
						pSalData->mpLastMouseMoveFrame = pFrame;
						pFrame = pLastMouseMoveFrame;
					}
					else
					{
						tools::Rectangle aBounds( Point( pFrame->maGeometry.nX, pFrame->maGeometry.nY ), Size( pFrame->maGeometry.nWidth, pFrame->maGeometry.nHeight ) );
						if ( !aBounds.IsInside( aScreenPoint ) )
							nID = SalEvent::MouseLeave;

					}
				}
				else
				{
					pSalData->mpLastMouseMoveFrame = nullptr;
				}

				// Check if we are not clicking on a floating window
				FloatingWindow *pPopupWindow = nullptr;
    			if ( nID == SalEvent::MouseButtonDown )
				{
					ImplSVData *pSVData = ImplGetSVData();
					if ( !pFrame->IsFloatingFrame() && pSVData && pSVData->maWinData.mpFirstFloat )
					{
						if ( !(pSVData->maWinData.mpFirstFloat->GetPopupModeFlags() & FloatWinPopupFlags::NoAppFocusClose) )
							pPopupWindow = pSVData->maWinData.mpFirstFloat;
					}
				}

				// Adjust position for RTL layout
				if ( Application::GetSettings().GetLayoutRTL() )
					pMouseEvent->mnX = pFrame->maGeometry.nWidth - pFrame->maGeometry.nLeftDecoration - pFrame->maGeometry.nRightDecoration - pMouseEvent->mnX - 1;

				// Fix bugs 1583, 2166, and 2320 by setting the last pointer
				// state before dispatching the event
				pSalData->maLastPointerState.mnState = pMouseEvent->mnCode;
				pSalData->maLastPointerState.maPos = Point( aScreenPoint.X(), aScreenPoint.Y() );

				pFrame->CallCallback( nID, pMouseEvent );

    			if ( pPopupWindow )
				{
					ImplSVData *pSVData = ImplGetSVData();
					if ( pSVData && pSVData->maWinData.mpFirstFloat == pPopupWindow )
						pPopupWindow->EndPopupMode( FloatWinPopupEndFlags::CloseAll );
				}
			}
			break;
		}
		case SalEvent::Move:
		case SalEvent::MoveResize:
		case SalEvent::Resize:
		{
			if ( pFrame )
			{
				tools::Rectangle *pPosSize = static_cast< tools::Rectangle* >( mpData );

				// Update size
				sal_Bool bInLiveResize = sal_False;
				sal_Bool bInFullScreenMode = sal_False;
				if ( !pPosSize )
				{
					pPosSize = new tools::Rectangle( pFrame->GetBounds( &bInLiveResize, &bInFullScreenMode ) );
					mpData = pPosSize;
				}

				// If in live resize, ignore event and just repaint
				bool bSkipEvent = false;
				if ( bInLiveResize )
				{
					timeval aCurrentTime;
					gettimeofday( &aCurrentTime, nullptr );
					if ( pSalData->mpLastResizeFrame == pFrame && pSalData->maLastResizeTime >= aCurrentTime )
						bSkipEvent = true;
				}

				// If too little time has passed since the last "in live resize"
				// event, skip it and repost this event
				if ( bSkipEvent )
				{
					JavaSalEvent *pEvent = new JavaSalEvent( nID, pFrame, nullptr );
					JavaSalEventQueue::postCachedEvent( pEvent );
					pEvent->release();
				}
				else
				{
					// Update resize timer
					pSalData->mpLastResizeFrame = ( bInLiveResize ? pFrame : nullptr );
					if ( pSalData->mpLastResizeFrame )
					{
						gettimeofday( &pSalData->maLastResizeTime, nullptr );
						pSalData->maLastResizeTime += 100;
					}

					// Fix bug 3252 by always comparing the bounds against the
					// work area. Fix hidden buttons in tall resizable dialogs
					// (such as the Tools > Mail Merge Wizard dialog) when using
					// a small monitor by forcing a resize event when only the
					// height of the window has been reduced to fit the work
					// area.
					bool bForceResize = false;
					if ( pFrame->mbInShow )
					{
						tools::Rectangle aRect( *pPosSize );
						pFrame->GetWorkArea( aRect );
						if ( ( aRect.Left() == pPosSize->Left() && aRect.GetWidth() == pPosSize->GetWidth() ) || ( aRect.Top() == pPosSize->Top() && aRect.GetHeight() == pPosSize->GetHeight() ) )
							bForceResize = true;
					}
					// Reimplement fix for missized content area when opening
					// a form in a full screen mode database document by
					// forcing a resize
					else if ( bInFullScreenMode && !pFrame->mbInSetPosSize && !pFrame->mbInSetWindowState )
					{
						bForceResize = true;
					}

					bool bPosChanged = false;
					int nX = pPosSize->Left() + pFrame->maGeometry.nLeftDecoration;
					if ( pFrame->maGeometry.nX != nX )
					{
						bPosChanged = true;
						if ( pFrame == pSalData->mpLastDragFrame )
							pSalData->maLastPointerState.maPos.X() += nX - pFrame->maGeometry.nX;
						pFrame->maGeometry.nX = nX;
					}
					int nY = pPosSize->Top() + pFrame->maGeometry.nTopDecoration;
					if ( pFrame->maGeometry.nY != nY )
					{
						bPosChanged = true;
						if ( pFrame == pSalData->mpLastDragFrame )
							pSalData->maLastPointerState.maPos.Y() += nY - pFrame->maGeometry.nY;
						pFrame->maGeometry.nY = nY;
					}

					bool bSizeChanged = false;
					unsigned int nWidth = pPosSize->GetWidth() - pFrame->maGeometry.nLeftDecoration - pFrame->maGeometry.nRightDecoration;
					if ( pFrame->maGeometry.nWidth != nWidth )
					{
						bSizeChanged = true;
						pFrame->maGeometry.nWidth = nWidth;
					}
					unsigned int nHeight = pPosSize->GetHeight() - pFrame->maGeometry.nTopDecoration - pFrame->maGeometry.nBottomDecoration;
					if ( pFrame->maGeometry.nHeight != nHeight )
					{
						bSizeChanged = true;
						pFrame->maGeometry.nHeight = nHeight;
					}

					// Fix bug 3045 by setting the event ID to the actual
					// changes that have occurred. This also fixes the
					// autodocking of native utility windows problem described
					// in bug 3035
					if ( bForceResize || bPosChanged || bSizeChanged )
					{
						if ( bPosChanged && bSizeChanged )
							nID = SalEvent::MoveResize;
						else if ( bPosChanged )
							nID = SalEvent::Move;
						else
							nID = SalEvent::Resize;

						if ( bForceResize || bSizeChanged )
							pFrame->UpdateLayer();

						// Fix bug 2769 by creating synthetic mouse dragged
						// events when dragging a window's title bar
						if ( bPosChanged && pFrame == pSalData->mpLastDragFrame )
						{
							NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

							SalMouseEvent *pMouseEvent = new SalMouseEvent();
							pMouseEvent->mnTime = static_cast< sal_uLong >( JavaSalEventQueue::getLastNativeEventTime() * 1000 );
							pMouseEvent->mnX = pSalData->maLastPointerState.maPos.X() - pFrame->maGeometry.nX;
							pMouseEvent->mnY = pSalData->maLastPointerState.maPos.Y() - pFrame->maGeometry.nY;
							pMouseEvent->mnCode = pSalData->maLastPointerState.mnState;
							pMouseEvent->mnButton = 0;

							JavaSalEvent *pMouseDraggedEvent = new JavaSalEvent( SalEvent::MouseMove, pFrame, pMouseEvent );
							JavaSalEventQueue::postCachedEvent( pMouseDraggedEvent );
							pMouseDraggedEvent->release();

							[pPool release];
						}

						pFrame->CallCallback( nID, nullptr );
					}
				}
			}
			break;
		}
		case SalEvent::Paint:
		{
			if ( pFrame && pFrame->mbVisible )
			{
				SalPaintEvent *pPaintEvent = static_cast< SalPaintEvent* >( mpData );
				if ( !pPaintEvent )
				{
					// Get paint region
					tools::Rectangle aUpdateRect( getUpdateRect() );
					pPaintEvent = new SalPaintEvent( aUpdateRect.Left(), aUpdateRect.Top(), aUpdateRect.GetWidth(), aUpdateRect.GetHeight() );

					mpData = pPaintEvent;
				}
				// Adjust position for RTL layout
				if ( Application::GetSettings().GetLayoutRTL() )
					pPaintEvent->mnBoundX = pFrame->maGeometry.nWidth - pFrame->maGeometry.nLeftDecoration - pFrame->maGeometry.nRightDecoration - pPaintEvent->mnBoundWidth - pPaintEvent->mnBoundX;

				pFrame->CallCallback( nID, pPaintEvent );
			}
			break;
		}
		case SalEvent::UserEvent:
		{
			if ( pFrame )
				pFrame->CallCallback( nID, mpData );
			break;
		}
		case SalEvent::WheelMouse:
		{
			if ( pFrame && pFrame->mbVisible )
			{
				SalWheelMouseEvent *pWheelMouseEvent = static_cast< SalWheelMouseEvent* >( mpData );
				if ( !pWheelMouseEvent )
				{
					long nWheelRotation = getWheelRotation();
					sal_Bool bHorz = isHorizontal();
					pWheelMouseEvent = new SalWheelMouseEvent();
					pWheelMouseEvent->mnTime = getWhen();
					pWheelMouseEvent->mnX = getX();
					pWheelMouseEvent->mnY = getY();
					pWheelMouseEvent->mnNotchDelta = ( nWheelRotation < 0 ? -1 : 1 );
					pWheelMouseEvent->mnDelta = ( nWheelRotation ? nWheelRotation : pWheelMouseEvent->mnNotchDelta );
					pWheelMouseEvent->mnScrollLines = getScrollAmount();
					pWheelMouseEvent->mnCode = getModifiers();
					pWheelMouseEvent->mbHorz = bHorz;

					mpData = pWheelMouseEvent;
				}
				// Fix misplaced tooltip windows reported in the following
				// NeoOffice forum post by posting a synthetic mouse move event
				// if the pointer state does not match the state in the mouse
				// wheel event:
				// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=64100#64100
				Point aScreenPoint( pWheelMouseEvent->mnX + pFrame->maGeometry.nX, pWheelMouseEvent->mnY + pFrame->maGeometry.nY );
				if ( pWheelMouseEvent->mnCode != pSalData->maLastPointerState.mnState || pSalData->maLastPointerState.maPos.X() != aScreenPoint.X() || pSalData->maLastPointerState.maPos.Y() != aScreenPoint.Y() )
				{
					SalMouseEvent *pMouseEvent = new SalMouseEvent();
					pMouseEvent->mnTime = pWheelMouseEvent->mnTime;
					pMouseEvent->mnX = pWheelMouseEvent->mnX;
					pMouseEvent->mnY = pWheelMouseEvent->mnY;
					pMouseEvent->mnCode = pWheelMouseEvent->mnCode;
					pMouseEvent->mnButton = 0;

					JavaSalEvent aEvent( SalEvent::MouseMove, pFrame, pMouseEvent );
					aEvent.dispatch();

					// Check if frame is still valid
					pFrame = FindValidFrame( pFrame );
					if ( !pFrame || !pFrame->mbVisible )
						break;
				}
				// Adjust position for RTL layout
				if ( Application::GetSettings().GetLayoutRTL() )
					pWheelMouseEvent->mnX = pFrame->maGeometry.nWidth - pFrame->maGeometry.nLeftDecoration - pFrame->maGeometry.nRightDecoration - pWheelMouseEvent->mnX - 1;
				pFrame->CallCallback( nID, pWheelMouseEvent );
			}
			break;
		}
		case SalEvent::MenuActivate:
		case SalEvent::MenuCommand:
		case SalEvent::MenuDeactivate:
		{
			// Fix crash when pressing the Command-W keys in an unmodified
			// document that has a modal sheet
			if ( pFrame && pFrame->mbVisible && ( !pSalData->mbInNativeModalSheet || pFrame != pSalData->mpNativeModalSheetFrame ) )
			{
				SalMenuEvent *pMenuEvent = static_cast< SalMenuEvent* >( mpData );
				if ( !pMenuEvent )
				{
					pMenuEvent = new SalMenuEvent( getMenuID(), getMenuCookie() );

					mpData = pMenuEvent;
				}

				// Pass all menu selections received by a utility window to
				// its parent window
				if ( nID == SalEvent::MenuCommand )
				{
					while ( pFrame->mpParent && pFrame->mpParent->mbVisible && pFrame->IsUtilityWindow() )
						pFrame = pFrame->mpParent;
				}

				// Check that the OOo code has not deleted the menu after we
				// posted this event
				if ( Menu::IsValidMenu( static_cast< Menu* >( pMenuEvent->mpMenu ) ) )
					pFrame->CallCallback( nID, pMenuEvent );
			}
			break;
		}
		default:
		{
			if ( pFrame )
				pFrame->CallCallback( nID, mpData );
			break;
		}
	}
}

// -------------------------------------------------------------------------

sal_uLong JavaSalEvent::getCommittedCharacterCount()
{
	sal_uLong nRet = 0;

	if ( mnID == SalEvent::ExtTextInput )
		nRet = mnCommittedCharacters;

	return nRet;
}

// -------------------------------------------------------------------------

sal_uLong JavaSalEvent::getCursorPosition()
{
	sal_uLong nRet = 0;

	if ( mnID == SalEvent::ExtTextInput )
		nRet = mnCursorPosition;

	return nRet;
}

// -------------------------------------------------------------------------

JavaSalFrame *JavaSalEvent::getFrame()
{
	return mpFrame;
}

// -------------------------------------------------------------------------

sal_uInt16 JavaSalEvent::getKeyChar()
{
	sal_uInt16 nRet = 0;

	if ( mpData && ( mnID == SalEvent::KeyInput || mnID == SalEvent::KeyUp ) )
	{
		SalKeyEvent *pKeyEvent = static_cast< SalKeyEvent* >( mpData );
		nRet = pKeyEvent->mnCharCode;
	}

	return nRet;
}

// -------------------------------------------------------------------------

sal_uInt16 JavaSalEvent::getKeyCode()
{
	sal_uInt16 nRet = 0;

	if ( mpData && ( mnID == SalEvent::KeyInput || mnID == SalEvent::KeyUp ) )
	{
		SalKeyEvent *pKeyEvent = static_cast< SalKeyEvent* >( mpData );
		nRet = pKeyEvent->mnCode & ~( KEY_MOD1 | KEY_MOD2 | KEY_MOD3 | KEY_SHIFT | MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT );
	}

	return nRet;
}

// -------------------------------------------------------------------------

SalEvent JavaSalEvent::getID()
{
	return mnID;
}

// -------------------------------------------------------------------------

sal_uInt16 JavaSalEvent::getModifiers()
{
	sal_uInt16 nRet = 0;

	if ( mpData )
	{
		switch ( mnID )
		{
			case SalEvent::KeyInput:
			case SalEvent::KeyUp:
			{
				SalKeyEvent *pKeyEvent = static_cast< SalKeyEvent* >( mpData );
				nRet = pKeyEvent->mnCode;
				break;
			}
			case SalEvent::KeyModChange:
			{
				SalKeyModEvent *pKeyModEvent = static_cast< SalKeyModEvent* >( mpData );
				nRet = pKeyModEvent->mnCode;
				break;
			}
			case SalEvent::MouseButtonDown:
			case SalEvent::MouseButtonUp:
			case SalEvent::MouseLeave:
			case SalEvent::MouseMove:
			{
				SalMouseEvent *pMouseEvent = static_cast< SalMouseEvent* >( mpData );
				nRet = pMouseEvent->mnCode;
				break;
			}
			case SalEvent::WheelMouse:
			{
				SalWheelMouseEvent *pWheelMouseEvent = static_cast< SalWheelMouseEvent* >( mpData );
				nRet = pWheelMouseEvent->mnCode;
				break;
			}
			default:
				break;
		}
	}

	return nRet;
}

// -------------------------------------------------------------------------

JavaSalEvent *JavaSalEvent::getNextOriginalKeyEvent()
{
	JavaSalEvent *pRet = nullptr;

	if ( maOriginalKeyEvents.size() )
	{
		pRet = maOriginalKeyEvents.front();
		maOriginalKeyEvents.pop_front();
	}

	return pRet;
}

// -------------------------------------------------------------------------

OUString JavaSalEvent::getPath()
{
	return maPath;
}

// -------------------------------------------------------------------------

sal_uInt16 JavaSalEvent::getRepeatCount()
{
	sal_uInt16 nRet = 0;

	if ( mpData && mnID == SalEvent::KeyInput )
	{
		SalKeyEvent *pKeyEvent = static_cast< SalKeyEvent* >( mpData );
		nRet = pKeyEvent->mnRepeat;
	}

	return nRet;
}

// -------------------------------------------------------------------------

OUString JavaSalEvent::getText()
{
	OUString aRet;

	if ( mpData && mnID == SalEvent::ExtTextInput )
	{
		SalExtTextInputEvent *pInputEvent = static_cast< SalExtTextInputEvent* >( mpData );
		aRet = pInputEvent->maText;
	}

	return aRet;
}

// -------------------------------------------------------------------------

const ExtTextInputAttr *JavaSalEvent::getTextAttributes()
{
	const ExtTextInputAttr *pRet = 0;

	if ( mpData && mnID == SalEvent::ExtTextInput )
	{
		SalExtTextInputEvent *pInputEvent = static_cast< SalExtTextInputEvent* >( mpData );
		pRet = pInputEvent->mpTextAttr;
	}

	return pRet;
}

// -------------------------------------------------------------------------

const tools::Rectangle JavaSalEvent::getUpdateRect()
{
	tools::Rectangle aRet( Point( 0, 0 ), Size( 0, 0 ) );

	if ( mpData && mnID == SalEvent::Paint )
	{
		SalPaintEvent *pPaintEvent = static_cast< SalPaintEvent* >( mpData );
		aRet = tools::Rectangle( Point( pPaintEvent->mnBoundX, pPaintEvent->mnBoundY ), Size( pPaintEvent->mnBoundWidth, pPaintEvent->mnBoundHeight ) );
	}

	return aRet;
}

// -------------------------------------------------------------------------

sal_uLong JavaSalEvent::getWhen()
{
	sal_uLong nRet = 0;

	if ( mpData )
	{
		switch ( mnID )
		{
			case SalEvent::KeyInput:
			case SalEvent::KeyUp:
			{
				SalKeyEvent *pKeyEvent = static_cast< SalKeyEvent* >( mpData );
				nRet = pKeyEvent->mnTime;
				break;
			}
			case SalEvent::KeyModChange:
			{
				SalKeyModEvent *pKeyModEvent = static_cast< SalKeyModEvent* >( mpData );
				nRet = pKeyModEvent->mnCode;
				break;
			}
			case SalEvent::MouseButtonDown:
			case SalEvent::MouseButtonUp:
			case SalEvent::MouseLeave:
			case SalEvent::MouseMove:
			{
				SalMouseEvent *pMouseEvent = static_cast< SalMouseEvent* >( mpData );
				nRet = pMouseEvent->mnTime;
				break;
			}
			case SalEvent::WheelMouse:
			{
				SalWheelMouseEvent *pWheelMouseEvent = static_cast< SalWheelMouseEvent* >( mpData );
				nRet = pWheelMouseEvent->mnTime;
				break;
			}
			default:
				break;
		}
	}

	return nRet;
}

// -------------------------------------------------------------------------

long JavaSalEvent::getX()
{
	long nRet = 0;

	if ( mpData )
	{
		switch ( mnID )
		{
			case SalEvent::MouseButtonDown:
			case SalEvent::MouseButtonUp:
			case SalEvent::MouseLeave:
			case SalEvent::MouseMove:
			{
				SalMouseEvent *pMouseEvent = static_cast< SalMouseEvent* >( mpData );
				nRet = pMouseEvent->mnX;
				break;
			}
			case SalEvent::WheelMouse:
			{
				SalWheelMouseEvent *pWheelMouseEvent = static_cast< SalWheelMouseEvent* >( mpData );
				nRet = pWheelMouseEvent->mnX;
				break;
			}
			default:
				break;
		}
	}

	return nRet;
}

// -------------------------------------------------------------------------

long JavaSalEvent::getY()
{
	long nRet = 0;

	if ( mpData )
	{
		switch ( mnID )
		{
			case SalEvent::MouseButtonDown:
			case SalEvent::MouseButtonUp:
			case SalEvent::MouseLeave:
			case SalEvent::MouseMove:
			{
				SalMouseEvent *pMouseEvent = static_cast< SalMouseEvent* >( mpData );
				nRet = pMouseEvent->mnY;
				break;
			}
			case SalEvent::WheelMouse:
			{
				SalWheelMouseEvent *pWheelMouseEvent = static_cast< SalWheelMouseEvent* >( mpData );
				nRet = pWheelMouseEvent->mnY;
				break;
			}
			default:
				break;
		}
	}

	return nRet;
}

// -------------------------------------------------------------------------

short JavaSalEvent::getMenuID()
{
	short nRet = 0;

	if ( mpData && ( mnID == SalEvent::MenuActivate || mnID == SalEvent::MenuCommand || mnID == SalEvent::MenuDeactivate ) )
	{
		SalMenuEvent *pMenuEvent = static_cast< SalMenuEvent* >( mpData );
		nRet = pMenuEvent->mnId;
	}

	return nRet;
}

// -------------------------------------------------------------------------

void *JavaSalEvent::getMenuCookie()
{
	void *pRet = nullptr;

	if ( mpData && ( mnID == SalEvent::MenuActivate || mnID == SalEvent::MenuCommand || mnID == SalEvent::MenuDeactivate ) )
	{
		SalMenuEvent *pMenuEvent = static_cast< SalMenuEvent* >( mpData );
		pRet = pMenuEvent->mpMenu;
	}

	return pRet;
}

// -------------------------------------------------------------------------

long JavaSalEvent::getScrollAmount()
{
	long nRet = 0;

	if ( mpData && mnID == SalEvent::WheelMouse )
	{
		SalWheelMouseEvent *pWheelMouseEvent = static_cast< SalWheelMouseEvent* >( mpData );
		nRet = pWheelMouseEvent->mnScrollLines;
	}

	return nRet;
}

// -------------------------------------------------------------------------

long JavaSalEvent::getWheelRotation()
{
	long nRet = 0;

	if ( mpData && mnID == SalEvent::WheelMouse )
	{
		SalWheelMouseEvent *pWheelMouseEvent = static_cast< SalWheelMouseEvent* >( mpData );
		nRet = pWheelMouseEvent->mnDelta;
	}

	return nRet;
}

// -------------------------------------------------------------------------

sal_Bool JavaSalEvent::isHorizontal()
{
	sal_Bool bRet = sal_False;

	if ( mpData && mnID == SalEvent::WheelMouse )
	{
		SalWheelMouseEvent *pWheelMouseEvent = static_cast< SalWheelMouseEvent* >( mpData );
		bRet = static_cast< sal_Bool >( pWheelMouseEvent->mbHorz );
	}

	return bRet;
}

// -------------------------------------------------------------------------

sal_Bool JavaSalEvent::isShutdownCancelled()
{
	return mbShutdownCancelled;
}

// ----------------------------------------------------------------------------

void JavaSalEvent::reference() const
{
	// Fix crashing bug reported in the following NeoOffice forum topic by
	// incrementing and decrementing the reference count in a thread safe
	// manner:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8555
	osl_incrementInterlockedCount( &mnRefCount );
}

// ----------------------------------------------------------------------------

void JavaSalEvent::release() const
{
	// Fix crashing bug reported in the following NeoOffice forum topic by
	// incrementing and decrementing the reference count in a thread safe
	// manner:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8555
	if ( osl_decrementInterlockedCount( &mnRefCount ) > 0 )
		return;

	// const_cast because some compilers violate ANSI C++ spec
	delete const_cast< JavaSalEvent* >( this );
}

// =========================================================================

JavaSalEventQueueItem::JavaSalEventQueueItem( JavaSalEvent *pEvent, const ::std::list< JavaSalEventQueueItem* > *pEventQueue ) :
	mpEvent( pEvent ),
	mpEventQueue( pEventQueue ),
	mbRemove( false ),
	mnType( VclInputFlags::NONE )
{
	if ( mpEvent )
	{
		mpEvent->reference();

		// Set event type
		switch ( mpEvent->getID() )
		{
			case SalEvent::ExtTextInput:
			case SalEvent::KeyInput:
			case SalEvent::KeyModChange:
			case SalEvent::KeyUp:
				mnType = VclInputFlags::KEYBOARD;
				break;
			case SalEvent::MouseMove:
			case SalEvent::MouseLeave:
			case SalEvent::MouseButtonUp:
			case SalEvent::MouseButtonDown:
			case SalEvent::WheelMouse:
				mnType = VclInputFlags::MOUSE;
				break;
			case SalEvent::Paint:
				mnType = VclInputFlags::PAINT;
				break;
			default:
				mnType = VclInputFlags::OTHER;
				break;
		}
	}
	else
	{
		mbRemove = true;
	}
}

// -------------------------------------------------------------------------

JavaSalEventQueueItem::~JavaSalEventQueueItem()
{
	if ( mpEvent )
		mpEvent->release();
}

// =========================================================================

Mutex JavaSalEventQueue::maMutex;

// -------------------------------------------------------------------------

Condition JavaSalEventQueue::maCondition;

// -------------------------------------------------------------------------

::std::list< JavaSalEventQueueItem* > JavaSalEventQueue::maNativeEventQueue;

// -------------------------------------------------------------------------

::std::list< JavaSalEventQueueItem* > JavaSalEventQueue::maNonNativeEventQueue;

// -------------------------------------------------------------------------

double JavaSalEventQueue::mfLastNativeEventTime = 0;

// -------------------------------------------------------------------------

JavaSalEventQueueItem* JavaSalEventQueue::mpKeyInputItem = nullptr;

// -------------------------------------------------------------------------

JavaSalEventQueueItem* JavaSalEventQueue::mpMoveResizeItem = nullptr;

// -------------------------------------------------------------------------

JavaSalEventQueueItem* JavaSalEventQueue::mpPaintItem = nullptr;

// -------------------------------------------------------------------------

sal_Bool JavaSalEventQueue::mbShutdownDisabled = sal_True;

// -------------------------------------------------------------------------

void JavaSalEventQueue::purgeRemovedEventsFromFront( ::std::list< JavaSalEventQueueItem* > *pEventQueue )
{
	MutexGuard aGuard( maMutex );

	if ( pEventQueue )
	{
		while ( pEventQueue->size() && pEventQueue->front()->isRemove() )
		{
			JavaSalEventQueueItem *pItem = pEventQueue->front();
			pEventQueue->pop_front();
			if ( mpKeyInputItem == pItem )
				mpKeyInputItem = nullptr;
			if ( mpMoveResizeItem == pItem )
				mpMoveResizeItem = nullptr;
			if ( mpPaintItem == pItem )
				mpPaintItem = nullptr;
			delete pItem;
		}
	}
}

// -------------------------------------------------------------------------

sal_Bool JavaSalEventQueue::anyCachedEvent( VclInputFlags nType )
{
	sal_Bool bRet = sal_False;

	MutexGuard aGuard( maMutex );

	::std::list< JavaSalEventQueueItem* >::const_iterator it = maNativeEventQueue.begin();
	for ( ; it != maNativeEventQueue.end(); ++it )
	{
		if ( !(*it)->isRemove() && (*it)->getType() & nType )
		{
			bRet = sal_True;
			break;
		}
	}

	if ( !bRet )
	{
		it = maNonNativeEventQueue.begin();
		for ( ; it != maNonNativeEventQueue.end(); ++it )
		{
			if ( !(*it)->isRemove() && (*it)->getType() & nType )
			{
				bRet = sal_True;
				break;
			}
		}
	}

	return bRet;
}

// -------------------------------------------------------------------------

void JavaSalEventQueue::dispatchNextEvent()
{
	if ( CFRunLoopGetCurrent() == CFRunLoopGetMain() )
		CFRunLoopRunInMode( CFSTR( "AWTRunLoopMode" ), 0, false );
}

// -------------------------------------------------------------------------

double JavaSalEventQueue::getLastNativeEventTime()
{
	return mfLastNativeEventTime;
}

// -------------------------------------------------------------------------

JavaSalEvent *JavaSalEventQueue::getNextCachedEvent( sal_uLong nTimeout, sal_Bool bNativeEvents )
{
	JavaSalEvent *pRet = nullptr;

	ClearableGuard< Mutex > aGuard( maMutex );

	// Wait if there are no events in either queue and a timeout is requested
	if ( nTimeout && !maNativeEventQueue.size() && !maNonNativeEventQueue.size() )
	{
		aGuard.clear();

		// Attempt to fix hanging bug reported in the following NeoOffice forum
		// topic by resetting the condition after the mutex has been released
		// and setting it after waiting:
		// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8611
		maCondition.reset();
		TimeValue aWait;
		aWait.Seconds = nTimeout / 1000;
		aWait.Nanosec = ( nTimeout % 1000 ) * 1000000;
		if ( !maCondition.check() )
			maCondition.wait( &aWait );
		maCondition.set();

		return JavaSalEventQueue::getNextCachedEvent( 0, bNativeEvents );
	}

	::std::list< JavaSalEventQueueItem* > *pEventQueue;
	if ( bNativeEvents )
		pEventQueue = &maNativeEventQueue;
	else
		pEventQueue = &maNonNativeEventQueue;

	if ( pEventQueue->size() )
	{
		JavaSalEventQueueItem *pItem = pEventQueue->front();
		pRet = pItem->getEvent();
		if ( pRet )
			pRet->reference();
		pItem->remove();
		purgeRemovedEventsFromFront( pEventQueue );
	}

	return pRet;
}

// -------------------------------------------------------------------------

sal_Bool JavaSalEventQueue::isShutdownDisabled()
{
	MutexGuard aGuard( maMutex );

	return mbShutdownDisabled;
}

// -------------------------------------------------------------------------

void JavaSalEventQueue::postCachedEvent( JavaSalEvent *pEvent )
{
	if ( !pEvent )
		return;

	MutexGuard aGuard( maMutex );

	::std::list< JavaSalEventQueueItem* > *pEventQueue;
	if ( pEvent->isNative() )
		pEventQueue = &maNativeEventQueue;
	else
		pEventQueue = &maNonNativeEventQueue;

	JavaSalEventQueueItem *pQueueItem = new JavaSalEventQueueItem( pEvent, pEventQueue );
	if ( pQueueItem )
	{
		// Coalesce events
		JavaSalEvent *pNewEvent = pQueueItem->getEvent();
		if ( pNewEvent )
		{
			JavaSalFrame *pFrame = pNewEvent->getFrame();
			SalEvent nID = pNewEvent->getID();
			switch ( nID )
			{
				case SalEvent::Close:
				case SalEvent::WakeUp:
				{
					for ( ::std::list< JavaSalEventQueueItem* >::const_iterator it = pEventQueue->begin(); it != pEventQueue->end(); ++it )
					{
						JavaSalEvent *pOldEvent = (*it)->getEvent();
						if ( pOldEvent && pOldEvent->getID() == nID && pOldEvent->getFrame() == pFrame && (*it)->getEventQueue() == pEventQueue && !(*it)->isRemove() )
						{
							pQueueItem->remove();
							break;
						}
					}

					break;
				}
				case SalEvent::ExtTextInput:
				{
					// Reduce flicker when backspacing through uncommitted text
					if ( pEventQueue->size() )
					{
						JavaSalEvent *pOldEvent = pEventQueue->back()->getEvent();
						if ( pOldEvent && pOldEvent->getID() == nID && pOldEvent->getFrame() == pFrame && !pEventQueue->back()->isRemove() && !pOldEvent->getText().getLength() )
							pEventQueue->back()->remove();
					}

					break;
				}
				case SalEvent::KeyInput:
				{
					if ( mpKeyInputItem )
					{
						JavaSalEvent *pOldEvent = mpKeyInputItem->getEvent();
						if ( pOldEvent && pOldEvent->getFrame() == pFrame && mpKeyInputItem->getEventQueue() == pEventQueue && !mpKeyInputItem->isRemove() && pOldEvent->getKeyChar() && pNewEvent->getKeyChar() && pOldEvent->getKeyCode() == pNewEvent->getKeyCode() && pOldEvent->getModifiers() == pNewEvent->getModifiers() )
						{
							mpKeyInputItem->remove();
							pNewEvent->addRepeatCount( 1 );
						}
					}

					mpKeyInputItem = pQueueItem;
					break;
				}
				case SalEvent::KeyUp:
				{
					mpKeyInputItem = nullptr;
					break;
				}
				case SalEvent::MouseMove:
				{
					if ( pEventQueue->size() )
					{
						JavaSalEvent *pOldEvent = pEventQueue->back()->getEvent();
						if ( pOldEvent && pOldEvent->getID() == nID && pOldEvent->getFrame() == pFrame && !pEventQueue->back()->isRemove() )
							pEventQueue->back()->remove();
					}

					break;
				}
				case SalEvent::WheelMouse:
				{
					// Fix lag when swiping while automatic spell checking is
					// enabled by coalescing all queued mouse wheel events
					for ( ::std::list< JavaSalEventQueueItem* >::const_iterator it = pEventQueue->begin(); it != pEventQueue->end(); ++it )
					{
						JavaSalEvent *pOldEvent = (*it)->getEvent();
						if ( pOldEvent && pOldEvent->getID() == nID && pOldEvent->getFrame() == pFrame && !(*it)->isRemove() )
						{
							if ( pNewEvent->addWheelRotationAndScrollLines( pOldEvent->getWheelRotation(), pOldEvent->getScrollAmount(), pOldEvent->isHorizontal() ) )
								(*it)->remove();
						}
					}

					break;
				}
				case SalEvent::MoveResize:
				{
					if ( mpMoveResizeItem )
					{
						JavaSalEvent *pOldEvent = mpMoveResizeItem->getEvent();
						if ( pOldEvent && pOldEvent->getFrame() == pFrame && mpMoveResizeItem->getEventQueue() == pEventQueue && !mpMoveResizeItem->isRemove() )
							mpMoveResizeItem->remove();
					}

					mpMoveResizeItem = pQueueItem;
					break;
				}
				case SalEvent::Paint:
				{
					if ( mpPaintItem )
					{
						JavaSalEvent *pOldEvent = mpPaintItem->getEvent();
						if ( pOldEvent && pOldEvent->getFrame() == pFrame && mpPaintItem->getEventQueue() == pEventQueue && !mpPaintItem->isRemove() )
						{
							mpPaintItem->remove();
							pNewEvent->addUpdateRect( pOldEvent->getUpdateRect() );
						}
					}

					mpPaintItem = pQueueItem;
					break;
				}
				default:
					break;
			}
		}

		pEventQueue->push_back( pQueueItem );
		purgeRemovedEventsFromFront( pEventQueue );
	}

	maCondition.set();
}

// -------------------------------------------------------------------------

void JavaSalEventQueue::removeCachedEvents( const JavaSalFrame *pFrame )
{
	MutexGuard aGuard( maMutex );

	::std::list< JavaSalEventQueueItem* >::const_iterator it = maNativeEventQueue.begin();
	for ( ; it != maNativeEventQueue.end(); ++it )
	{
		JavaSalEvent *pEvent = (*it)->getEvent();
		if ( pEvent && pEvent->getFrame() == pFrame )
			(*it)->remove();
	}

	purgeRemovedEventsFromFront( &maNativeEventQueue );

	it = maNonNativeEventQueue.begin();
	for ( ; it != maNonNativeEventQueue.end(); ++it )
	{
		JavaSalEvent *pEvent = (*it)->getEvent();
		if ( pEvent && pEvent->getFrame() == pFrame )
			(*it)->remove();
	}

	purgeRemovedEventsFromFront( &maNonNativeEventQueue );
}

// -------------------------------------------------------------------------

void JavaSalEventQueue::setLastNativeEventTime( double nEventTime )
{
	mfLastNativeEventTime = nEventTime;
}

// -------------------------------------------------------------------------

void JavaSalEventQueue::setShutdownDisabled( sal_Bool bShutdownDisabled )
{
	VCLEventQueue_cancelTermination();

	MutexGuard aGuard( maMutex );

	mbShutdownDisabled = bShutdownDisabled;
	if ( mbShutdownDisabled )
	{
		::std::list< JavaSalEventQueueItem* >::const_iterator it = maNativeEventQueue.begin();
		for ( ; it != maNativeEventQueue.end(); ++it )
		{
			JavaSalEvent *pEvent = (*it)->getEvent();
			if ( pEvent && pEvent->getID() == SalEvent::Shutdown )
				pEvent->cancelShutdown();
		}

		purgeRemovedEventsFromFront( &maNativeEventQueue );

		it = maNonNativeEventQueue.begin();
		for ( ; it != maNonNativeEventQueue.end(); ++it )
		{
			JavaSalEvent *pEvent = (*it)->getEvent();
			if ( pEvent && pEvent->getID() == SalEvent::Shutdown )
				pEvent->cancelShutdown();
		}

		purgeRemovedEventsFromFront( &maNonNativeEventQueue );
	}
}
