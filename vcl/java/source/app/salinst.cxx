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

#include "salinst.hrc"

using namespace vcl;
using namespace vos;
using namespace com::sun::star::uno;

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
	if ( pSalData )
		delete pSalData;
	SetSalData( NULL );
}

// -----------------------------------------------------------------------

void InitSalMain()
{
	SalData *pSalData = GetSalData();
	pSalData->mpEventQueue = new com_sun_star_vcl_VCLEventQueue( NULL );
	pSalData->mpFontList = com_sun_star_vcl_VCLFont::getAllFonts();
}

// -----------------------------------------------------------------------

void DeInitSalMain()
{
	SalData *pSalData = GetSalData();
	if ( pSalData->mpEventQueue )
		delete pSalData->mpEventQueue;
	if ( pSalData->mpFontList )
		delete pSalData->mpFontList;
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
	SalData *pSalData = GetSalData();
	com_sun_star_vcl_VCLEvent *pEvent;

	nRecursionLevel++;

	// Dispatch pending non-AWT events
	if ( ( pEvent = pSalData->mpEventQueue->getNextCachedEvent( 0, FALSE ) ) != NULL )
	{
		// Ignore SALEVENT_SHUTDOWN events when recursing into this method or
		// when in presentation mode
		if ( ( nRecursionLevel == 1 && !pSalData->mpPresentationFrame ) || pEvent->getID() != SALEVENT_SHUTDOWN )
		{
			if ( pSalData->mpPresentationFrame )
				pSalData->mpPresentationFrame->maFrameData.mpVCLFrame->setAutoFlush( TRUE );
			pEvent->dispatch();
			if ( pSalData->mpPresentationFrame )
				pSalData->mpPresentationFrame->maFrameData.mpVCLFrame->setAutoFlush( FALSE );
		}
		delete pEvent;

		ULONG nCount = ReleaseYieldMutex();
		if ( bWait )
			OThread::yield();
		AcquireYieldMutex( nCount );
		nRecursionLevel--;
		return;
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
			{
				pSalData->mpPresentationFrame->maFrameData.mpVCLFrame->setAutoFlush( FALSE );
			}
			else
			{
				// Flush all of the window buffers to the native windows
				for ( ::std::list< SalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
					(*it)->Flush();
			}
		}
	}

	// Determine timeout
	ULONG nTimeout = 0;
	if ( bWait && pSalData->mnTimerInterval )
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
	BOOL bContinue = TRUE;
	while ( bContinue && ( pEvent = pSalData->mpEventQueue->getNextCachedEvent( nTimeout, TRUE ) ) != NULL )
	{
		// Reset timeout
		nTimeout = 0;

		// If this is a mouse or key pressed event, make another pass through
		// the loop in case the next event is a mouse released event. If the
		// timer is run between continguous mouse or key pressed and released
		// the application acts is if two mouse clicks have been made instead
		// of one.
		USHORT nID = pEvent->getID();
		if ( nID == SALEVENT_MOUSEMOVE )
			bContinue = FALSE;
		if ( pSalData->mpPresentationFrame )
			pSalData->mpPresentationFrame->maFrameData.mpVCLFrame->setAutoFlush( TRUE );
		pEvent->dispatch();
		if ( pSalData->mpPresentationFrame )
			pSalData->mpPresentationFrame->maFrameData.mpVCLFrame->setAutoFlush( FALSE );
		delete pEvent;
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

	pFrame->maFrameData.mpParent = pParent;
	if ( pParent )
		pFrame->maFrameData.mpParent->maFrameData.maChildren.push_back( pFrame );

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
		pFrame->maFrameData.mbCenter = FALSE;
		nWidth = 10;
		nHeight = 10;
	}
	else
	{
		if ( nSalFrameStyle & SAL_FRAME_STYLE_SIZEABLE && nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE )
		{
			nWidth -= 100;
			nHeight -= 100;
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
				const SalFrameGeometry& rGeom( pNextFrame->GetGeometry() );
				pFrame->maFrameData.mbCenter = FALSE;
				nX = rGeom.nX - rGeom.nLeftDecoration;
				nY = rGeom.nY - rGeom.nTopDecoration;
				nWidth = rGeom.nWidth + rGeom.nLeftDecoration + rGeom.nRightDecoration;
				nHeight = rGeom.nHeight + rGeom.nTopDecoration + rGeom.nBottomDecoration;
			}
		}
	}

	// Center the window by default
	if ( pFrame->maFrameData.mbCenter )
	{
		nX = aWorkArea.nLeft + ( ( aWorkArea.GetWidth() - nWidth ) / 2 );
		nY = aWorkArea.nTop + ( ( aWorkArea.GetHeight() - nHeight ) / 2 );
	}

    pFrame->SetPosSize( nX, nY, nWidth, nHeight, SAL_FRAMESTATE_MASK_X | SAL_FRAME_POSSIZE_Y | SAL_FRAMESTATE_MASK_WIDTH | SAL_FRAMESTATE_MASK_HEIGHT );

	return pFrame;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyFrame( SalFrame* pFrame )
{
	// Remove this window from the window list
	if ( pFrame )
	{
		GetSalData()->maFrameList.remove( pFrame);

		if ( pFrame->maFrameData.mpParent )
			pFrame->maFrameData.mpParent->maFrameData.maChildren.remove( pFrame );

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
		BOOL bDelete = FALSE;

		if ( pSetupData->mnSystem != JOBSETUP_SYSTEM_JAVA || pSetupData->mnDriverDataLen != sizeof( SalDriverData ) )
			bDelete = TRUE;

		if ( !bDelete )
		{
			BOOL bDelete = TRUE;
			for ( ::std::list< com_sun_star_vcl_VCLPageFormat* >::const_iterator it = pSalData->maVCLPageFormats.begin(); it != pSalData->maVCLPageFormats.end(); ++it )
			{
				if ( ((SalDriverData *)pSetupData->mpDriverData)->mpVCLPageFormat == *it && ((SalDriverData *)pSetupData->mpDriverData)->mpVCLPageFormat->getJavaObject() == (*it)->getJavaObject() )
				{
					bDelete = FALSE;
					break;
				}
			}
		}

		if ( bDelete )
		{
			rtl_freeMemory( pSetupData->mpDriverData );
			pSetupData->mpDriverData = NULL;
			pSetupData->mnDriverDataLen = 0;
		}
	}

	// Set driver data
	if ( !pSetupData->mpDriverData )
	{
		SalDriverData *pDriverData = (SalDriverData *)rtl_allocateMemory( sizeof( SalDriverData ) );
		pDriverData->mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat();
		pSalData->maVCLPageFormats.push_back( pDriverData->mpVCLPageFormat );
		pSetupData->mpDriverData = (BYTE *)pDriverData;
		pSetupData->mnDriverDataLen = sizeof( SalDriverData );
	}

	// Create a new page format instance that points to the same Java object
	pPrinter->maPrinterData.mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat( ((SalDriverData *)pSetupData->mpDriverData)->mpVCLPageFormat->getJavaObject() );
	pSalData->maVCLPageFormats.push_back( pPrinter->maPrinterData.mpVCLPageFormat );

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
