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
	if( !rErrorText.Len() )
		fprintf( stderr, "Application Error" );
	else
		fprintf( stderr, ByteString( rErrorText, gsl_getSystemTextEncoding() ).GetBuffer() );
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
			pEvent->dispatch();
			com_sun_star_vcl_VCLGraphics::flushAll();
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
			pSalData->mpTimerProc();
			com_sun_star_vcl_VCLGraphics::flushAll();
			if ( pSalData->mnTimerInterval )
			{
				gettimeofday( &aCurrentTime, NULL );
				pSalData->maTimeout = aCurrentTime + pSalData->mnTimerInterval;
			}
		}
	}

	// Dispatch pending AWT events
	if ( bWait && !ImplGetSVData()->maAppData.mbAppQuit )
	{
		ULONG nTimeout = 0;

		// Determine timeout
		if ( pSalData->mnTimerInterval )
		{
			timeval aTimeout;

			gettimeofday( &aTimeout, NULL );
			if ( pSalData->maTimeout > aTimeout )
			{
				aTimeout = pSalData->maTimeout - aTimeout;
				nTimeout = aTimeout.tv_sec * 1000 + aTimeout.tv_usec / 1000;
			}
			if ( nTimeout < 10 )
				nTimeout = 10;
		}

		while ( ( pEvent = pSalData->mpEventQueue->getNextCachedEvent( nTimeout, TRUE ) ) != NULL )
		{
			// Reset timeout
			nTimeout = 0;

			pEvent->dispatch();
			com_sun_star_vcl_VCLGraphics::flushAll();
			delete pEvent;
		}

		// Check timer
		if ( pSalData->mnTimerInterval )
		{
			timeval aCurrentTime;
			gettimeofday( &aCurrentTime, NULL );
			if ( pSalData->mpTimerProc && aCurrentTime >= pSalData->maTimeout )
			{
				pSalData->mpTimerProc();
				com_sun_star_vcl_VCLGraphics::flushAll();
				if ( pSalData->mnTimerInterval )
				{
					gettimeofday( &aCurrentTime, NULL );
					pSalData->maTimeout = aCurrentTime + pSalData->mnTimerInterval;
				}
			}
		}
	}

	nRecursionLevel--;
}

// -----------------------------------------------------------------------

BOOL SalInstance::AnyInput( USHORT nType )
{
	return (BOOL)GetSalData()->mpEventQueue->anyCachedEvent( nType );
}

// -----------------------------------------------------------------------

SalFrame* SalInstance::CreateChildFrame( SystemParentData* pSystemParentData, ULONG nSalFrameStyle )
{
	// Java does not provide access to any windows from other processes
	return CreateFrame( NULL, nSalFrameStyle );
}

// -----------------------------------------------------------------------

SalFrame* SalInstance::CreateFrame( SalFrame* pParent, ULONG nSalFrameStyle )
{
	SalFrame *pFrame = new SalFrame();

	pFrame->maFrameData.mpVCLFrame = new com_sun_star_vcl_VCLFrame( nSalFrameStyle, pFrame, pParent );
	pFrame->maFrameData.maSysData.pVCLFrame = pFrame->maFrameData.mpVCLFrame;
	pFrame->maFrameData.mnStyle = nSalFrameStyle;

	pFrame->maFrameData.mpParent = pParent;
	if ( pParent )
		pFrame->maFrameData.mpParent->maFrameData.maChildren.push_back( pFrame );

	// Insert this window into the window list
	SalData *pSalData = GetSalData();
	pFrame->maFrameData.mpNextFrame = pSalData->mpFirstFrame;
	pSalData->mpFirstFrame = pFrame;

	// Cache the insets
	Rectangle aRect = pFrame->maFrameData.mpVCLFrame->getInsets();
	pFrame->maGeometry.nLeftDecoration = aRect.nLeft;
	pFrame->maGeometry.nTopDecoration = aRect.nTop;
	pFrame->maGeometry.nRightDecoration = aRect.nRight;
	pFrame->maGeometry.nBottomDecoration = aRect.nBottom;

	// Set default window size based on style
	Rectangle aWorkArea;
	pFrame->GetWorkArea( aWorkArea );
	long nX = 0;
	long nY = 0;
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
			SalFrame* pNextFrame = pFrame->maFrameData.mpNextFrame;
			while ( pNextFrame &&
				( pNextFrame->maFrameData.mpParent ||
					pNextFrame->maFrameData.mnStyle == SAL_FRAME_STYLE_DEFAULT ||
					! ( pNextFrame->maFrameData.mnStyle & SAL_FRAME_STYLE_SIZEABLE ) ||
					! pNextFrame->GetGeometry().nWidth ||
					! pNextFrame->GetGeometry().nHeight )
				)
					pNextFrame = pNextFrame->maFrameData.mpNextFrame;
			if ( pNextFrame )
			{
				const SalFrameGeometry& rGeom( pNextFrame->GetGeometry() );
				pFrame->maFrameData.mbCenter = FALSE;
				nX = rGeom.nX - rGeom.nLeftDecoration + pFrame->maGeometry.nTopDecoration;
				nY = rGeom.nY;
				nWidth = rGeom.nWidth + rGeom.nLeftDecoration + rGeom.nRightDecoration;
				nHeight = rGeom.nHeight + rGeom.nTopDecoration + rGeom.nBottomDecoration;
				// If the window spills off the screen, place it at the 
				// top left of the screen
				if ( ( nX + nWidth ) > aWorkArea.GetWidth() || ( nY + nHeight ) > aWorkArea.GetHeight() )
				{
					nX = 0;
					nY = 0;
				}
				
			}
		}
	}
	// Center the window by default
	if ( pFrame->maFrameData.mbCenter )
	{
		nX = ( aWorkArea.GetWidth() - nWidth ) / 2;
		nY = ( aWorkArea.GetHeight() - nHeight ) / 2;
	}

	pFrame->maFrameData.mpVCLFrame->setBounds( nX, nY, nWidth, nHeight );

 	// Update the cached position
 	Rectangle *pBounds = new Rectangle( pFrame->maFrameData.mpVCLFrame->getBounds() );
	com_sun_star_vcl_VCLEvent aEvent( SALEVENT_MOVERESIZE, pFrame, (void *)pBounds );
	aEvent.dispatch();

	return pFrame;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyFrame( SalFrame* pFrame )
{
	SalData *pSalData = GetSalData();

	// Remove this window from the window list
	if ( pFrame == pSalData->mpFirstFrame )
	{
		pSalData->mpFirstFrame = pFrame->maFrameData.mpNextFrame;
	}
	else
	{
		SalFrame* pNextFrame = pSalData->mpFirstFrame;
		while ( pFrame != pNextFrame->maFrameData.mpNextFrame )
			pNextFrame = pNextFrame->maFrameData.mpNextFrame;
		pNextFrame->maFrameData.mpNextFrame = pFrame->maFrameData.mpNextFrame;
	}

	if ( pFrame->maFrameData.mpParent )
		pFrame->maFrameData.mpParent->maFrameData.maChildren.remove( pFrame );

	delete pFrame;
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
	delete pInfo;
}

// -----------------------------------------------------------------------

SalInfoPrinter* SalInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                                ImplJobSetup* pSetupData )
{
	// Create a dummy printer configuration for our dummy printer
	SalInfoPrinter *pPrinter = new SalInfoPrinter();

	// Set values
	if ( !pSetupData->mpDriverData )
	{
		pPrinter->maPrinterData.mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat();
		pPrinter->maPrinterData.mpVCLPageFormat->setOrientation( pSetupData->meOrientation );
		BYTE *pDriverData = (BYTE *)rtl_allocateMemory( sizeof( com_sun_star_vcl_VCLPageFormat* ) );
		memcpy( pDriverData, &pPrinter->maPrinterData.mpVCLPageFormat, sizeof( com_sun_star_vcl_VCLPageFormat* ) );
		pSetupData->mpDriverData = pDriverData;
		pSetupData->mnDriverDataLen = sizeof( com_sun_star_vcl_VCLPageFormat* );
	}
	else
	{
		// Create a new page format instance that points to the same Java
		// object
		pPrinter->maPrinterData.mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat( ((com_sun_star_vcl_VCLPageFormat *)pSetupData->mpDriverData)->getJavaObject() );
	}

	// Populate the job setup
	pSetupData->mnSystem = JOBSETUP_SYSTEM_JAVA;
	pSetupData->maPrinterName = pQueueInfo->maPrinterName;
	pSetupData->maDriver = pQueueInfo->maDriver;
	pSetupData->meOrientation = pPrinter->maPrinterData.mpVCLPageFormat->getOrientation();
	pSetupData->mnPaperBin = 0;
	pSetupData->mePaperFormat = pPrinter->maPrinterData.mpVCLPageFormat->getPaperType();
	Size aSize( pPrinter->maPrinterData.mpVCLPageFormat->getPageSize() );
	pSetupData->mnPaperWidth = aSize.Width();
	pSetupData->mnPaperHeight = aSize.Height();

    return pPrinter;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
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
	pPrinter->maPrinterData.mpPrinter = pInfoPrinter;
	return pPrinter;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
	delete pPrinter;
}

// -----------------------------------------------------------------------

SalVirtualDevice* SalInstance::CreateVirtualDevice( SalGraphics* pGraphics,
                                                    long nDX, long nDY,
                                                    USHORT nBitCount )
{
	SalVirtualDevice *pDevice = new SalVirtualDevice();

	if ( !nBitCount && pGraphics )
		nBitCount = pGraphics->GetBitCount();
	pDevice->maVirDevData.mnBitCount = nBitCount;

	if ( !pDevice->SetSize( nDX, nDY ) )
	{
		delete pDevice;
		pDevice = NULL;
	}

   	return pDevice;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyVirtualDevice( SalVirtualDevice* pDevice )
{
	delete pDevice;
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
