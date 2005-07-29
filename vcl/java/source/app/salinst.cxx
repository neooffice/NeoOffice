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
#ifndef _OSL_PROCESS_H_
#include <rtl/process.h>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif
#ifndef _TOOLS_SIMPLERESMGR_HXX_
#include <tools/simplerm.hxx>
#endif
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif

#include "salinst.hrc"

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>
#undef check

using namespace rtl;
using namespace vcl;
using namespace vos;
using namespace utl;
using namespace com::sun::star::uno;

// ============================================================================

static void ImplFontListChangedCallback( ATSFontNotificationInfoRef, void* )
{
	Application::GetSolarMutex().acquire();

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

	Application::GetSolarMutex().acquire();
}

// ----------------------------------------------------------------------------

static OSStatus CarbonEventHandler( EventHandlerCallRef aNextHandler, EventRef aEvent, void *pData )
{
	EventClass nClass = GetEventClass( aEvent );
	EventKind nKind = GetEventKind( aEvent );

	if ( !Application::IsShutDown() )
	{
		SalData *pSalData = GetSalData();

		if ( pSalData && pSalData->mpEventQueue )
		{
			if ( nClass == kEventClassMenu && ( nKind == kEventMenuBeginTracking || nKind == kEventMenuEndTracking ) )
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
						// Wakeup the event queue by sending it a dummy event
						// and wait for all pending AWT events to be dispatched
						pSalData->mbNativeEventSucceeded = false;
						pSalData->maNativeEventCondition.reset();
						com_sun_star_vcl_VCLEvent aEvent( SALEVENT_USEREVENT, NULL, NULL );
						pSalData->mpEventQueue->postCachedEvent( &aEvent );
						pSalData->maNativeEventCondition.wait();

						// Fix bug 679 by checking if the condition was
						// released to avoid a deadlock
						if ( pSalData->mbNativeEventSucceeded )
						{
							Application::GetSolarMutex().acquire();

							pSalData->mbInNativeMenuTracking = ( nKind == kEventMenuBeginTracking );

							// Execute menu updates while the VCL event queue is
							// blocked
							UpdateMenusForFrame( pSalData->mpFocusFrame, NULL );

							// We need to let any timers run that were added
							// by any menu changes. Otherwise, some menus will
							// be drawn in the state the menus were in before
							// we updated the menus.
							ReceiveNextEvent( 0, NULL, 0, false, NULL );

							Application::GetSolarMutex().release();

							return noErr;
						}
						else
						{
							return userCanceledErr;
						}
					}
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
				ATSFontActivateFromFileSpecification( &aFontSpec, kATSFontContextGlobal, kATSFontFormatUnspecified, NULL, kATSOptionFlagsDefault, NULL );
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
				ATSFontActivateFromFileSpecification( &aFontSpec, kATSFontContextGlobal, kATSFontFormatUnspecified, NULL, kATSOptionFlagsDefault, NULL );
		}
	}

	ATSFontIterator aIterator;

	// Get the array of fonts
	BOOL bContinue = TRUE;
	while ( bContinue )
	{
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

	// Now that Java is properly initialized, run the application's Main()
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
	pFrame->maFrameData.maSysData.aWindow = pFrame->maFrameData.mpVCLFrame->getNativeWindow();
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
