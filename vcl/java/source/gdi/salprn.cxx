/*************************************************************************
 *
 * $RCSfile$
 *
 * $Revision$
 *
 * last change: $Author$ $Date$
 *
 * The Contents of this file are made available subject to the terms of
 * either of the following licenses
 *
 *        - GNU General Public License Version 2.1
 *
 * Patrick Luby, June 2003
 *
 * GNU General Public License Version 2.1
 * =============================================
 * Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2.1, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307  USA
 *
 ************************************************************************/

#define _SV_SALPRN_CXX

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALPRN_HXX
#include <salprn.hxx>
#endif
#ifndef _SV_SALPTYPE_HXX
#include <salptype.hxx>
#endif
#ifndef _SV_JOBSET_H
#include <jobset.h>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLPAGEFORMAT_HXX
#include <com/sun/star/vcl/VCLPageFormat.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLPRINTJOB_HXX
#include <com/sun/star/vcl/VCLPrintJob.hxx>
#endif

using namespace vcl;

// =======================================================================

SalInfoPrinter::SalInfoPrinter()
{
}

// -----------------------------------------------------------------------

SalInfoPrinter::~SalInfoPrinter()
{
}

// -----------------------------------------------------------------------

SalGraphics* SalInfoPrinter::GetGraphics()
{
	if ( maPrinterData.mbGraphics )
		return NULL;

	maPrinterData.mpGraphics->maGraphicsData.mpVCLGraphics = maPrinterData.mpVCLPageFormat->getGraphics();
	maPrinterData.mbGraphics = TRUE;

	return maPrinterData.mpGraphics;
}

// -----------------------------------------------------------------------

void SalInfoPrinter::ReleaseGraphics( SalGraphics* pGraphics )
{
	if ( pGraphics != maPrinterData.mpGraphics )
		return;

	if ( maPrinterData.mpGraphics && maPrinterData.mpGraphics->maGraphicsData.mpVCLGraphics )
		delete maPrinterData.mpGraphics->maGraphicsData.mpVCLGraphics;
	maPrinterData.mpGraphics->maGraphicsData.mpVCLGraphics = NULL;
	maPrinterData.mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::Setup( SalFrame* pFrame, ImplJobSetup* pSetupData )
{
	// Display a native page setup dialog
	Orientation nOrientation = maPrinterData.mpVCLPageFormat->getOrientation();
	maPrinterData.mpVCLPageFormat->setOrientation( pSetupData->meOrientation );

	// Unlock the VCL event loop so that the native event loop can do work
	// while the native modal dialog is open
	ULONG nCount = Application::ReleaseSolarMutex();

	BOOL bOK = maPrinterData.mpVCLPageFormat->setup();

	// Relock the VCL event loop
	Application::AcquireSolarMutex( nCount );

	if ( !bOK )
	{
		maPrinterData.mpVCLPageFormat->setOrientation( nOrientation );
		return FALSE;
	}

	// Update values
	SetData( 0, pSetupData );

	return TRUE;
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::SetPrinterData( ImplJobSetup* pSetupData )
{
	SalData *pSalData = GetSalData();

	// Check driver data
	if ( pSetupData->mpDriverData )
	{
		BOOL bDelete = FALSE;

		if ( pSetupData->mnSystem != JOBSETUP_SYSTEM_JAVA || pSetupData->mnDriverDataLen != sizeof( SalDriverData ) )
			bDelete = TRUE;

		if ( !bDelete )
		{
			bDelete = TRUE;
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
		pDriverData->mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat( maPrinterData.mpVCLPageFormat->getJavaObject() );
		pSalData->maVCLPageFormats.push_back( pDriverData->mpVCLPageFormat );
		pSetupData->mpDriverData = (BYTE *)pDriverData;
		pSetupData->mnDriverDataLen = sizeof( SalDriverData );
	}
	else
	{
		if ( maPrinterData.mpVCLPageFormat )
		{
			pSalData->maVCLPageFormats.remove( maPrinterData.mpVCLPageFormat );
			delete maPrinterData.mpVCLPageFormat;
		}
		// Create a new page format instance that points to the same Java
		// object
		SalDriverData *pDriverData = (SalDriverData *)pSetupData->mpDriverData;
		maPrinterData.mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat( pDriverData->mpVCLPageFormat->getJavaObject() );
		pSalData->maVCLPageFormats.push_back( maPrinterData.mpVCLPageFormat );
	}

	// Set but don't update values
	SetData( SAL_JOBSET_ALL, pSetupData );

	return TRUE;
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::SetData( ULONG nFlags, ImplJobSetup* pSetupData )
{
	// Set or update values
	if ( nFlags & SAL_JOBSET_ORIENTATION == 0 )
		pSetupData->meOrientation = maPrinterData.mpVCLPageFormat->getOrientation();
	else
		maPrinterData.mpVCLPageFormat->setOrientation( pSetupData->meOrientation );

	if ( nFlags & SAL_JOBSET_PAPERBIN == 0 )
		pSetupData->mnPaperBin = 0;


	if ( nFlags & SAL_JOBSET_PAPERSIZE == 0 )
	{
		pSetupData->mePaperFormat = maPrinterData.mpVCLPageFormat->getPaperType();
		Size aSize( maPrinterData.mpVCLPageFormat->getPageSize() );
		pSetupData->mnPaperWidth = aSize.Width();
		pSetupData->mnPaperHeight = aSize.Height();
	}

	return TRUE;
}

// -----------------------------------------------------------------------

ULONG SalInfoPrinter::GetPaperBinCount( const ImplJobSetup* pSetupData )
{
	// Return a dummy value
	return 1;
}

// -----------------------------------------------------------------------

XubString SalInfoPrinter::GetPaperBinName( const ImplJobSetup* pSetupData, ULONG nPaperBin )
{
	// Return a dummy value
	return XubString();
}

// -----------------------------------------------------------------------

ULONG SalInfoPrinter::GetCapabilities( const ImplJobSetup* pSetupData, USHORT nType )
{
	if ( nType == PRINTER_CAPABILITIES_SETORIENTATION )
		return 1;
	else
		return 0;
}

// -----------------------------------------------------------------------

void SalInfoPrinter::GetPageInfo( const ImplJobSetup* pSetupData,
								  long& rOutWidth, long& rOutHeight,
								  long& rPageOffX, long& rPageOffY,
								  long& rPageWidth, long& rPageHeight )
{
	Size aSize( maPrinterData.mpVCLPageFormat->getPageSize() );
	Rectangle aRect( maPrinterData.mpVCLPageFormat->getImageableBounds() );
	rPageWidth = aSize.Width();
	rPageHeight = aSize.Height();
	rPageOffX = aRect.nLeft;
	rPageOffY = aRect.nTop;
	rOutWidth = aRect.nRight - aRect.nLeft + 1;
	rOutHeight = aRect.nBottom - aRect.nTop + 1;
}

// =======================================================================

SalPrinter::SalPrinter()
{
}

// -----------------------------------------------------------------------

SalPrinter::~SalPrinter()
{
}

// -----------------------------------------------------------------------

BOOL SalPrinter::StartJob( const XubString* pFileName,
						   const XubString& rJobName,
						   const XubString&,
						   ULONG nCopies, BOOL bCollate,
						   ImplJobSetup* pSetupData,
						   BOOL bShowDialog )
{
	SalData *pSalData = GetSalData();

	if ( maPrinterData.mpVCLPageFormat )
	{
		pSalData->maVCLPageFormats.remove( maPrinterData.mpVCLPageFormat );
		delete maPrinterData.mpVCLPageFormat;
	}

	// Create a new page format instance that points to the same Java object
	SalDriverData *pDriverData = (SalDriverData *)pSetupData->mpDriverData;
	maPrinterData.mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat( pDriverData->mpVCLPageFormat->getJavaObject() );
	pSalData->maVCLPageFormats.push_back( maPrinterData.mpVCLPageFormat );

	// Unlock the VCL event loop so that the native event loop can do work
	// while the native modal dialog is open
	ULONG nCount = Application::ReleaseSolarMutex();

	maPrinterData.mbStarted = maPrinterData.mpVCLPrintJob->startJob( maPrinterData.mpVCLPageFormat, bShowDialog );

	// Relock the VCL event loop
	Application::AcquireSolarMutex( nCount );

	return maPrinterData.mbStarted;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::EndJob()
{
	maPrinterData.mpVCLPrintJob->endJob();
	if ( maPrinterData.mpVCLPageFormat )
		maPrinterData.mpVCLPageFormat->resetPageResolution();
	maPrinterData.mbStarted = FALSE;
	return TRUE;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::AbortJob()
{
	maPrinterData.mpVCLPrintJob->abortJob();
	return TRUE;
}

// -----------------------------------------------------------------------

SalGraphics* SalPrinter::StartPage( ImplJobSetup* pSetupData, BOOL bNewJobData )
{
	if ( maPrinterData.mbGraphics )
		return NULL;

	com_sun_star_vcl_VCLGraphics *pVCLGraphics = maPrinterData.mpVCLPrintJob->startPage();
	if ( !pVCLGraphics )
		return NULL;
	maPrinterData.mpGraphics = new SalGraphics();
	maPrinterData.mpGraphics->maGraphicsData.mpVCLGraphics = pVCLGraphics;
	maPrinterData.mpGraphics->maGraphicsData.mpPrinter = this;
	maPrinterData.mbGraphics = TRUE;

	return maPrinterData.mpGraphics;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::EndPage()
{
	if ( maPrinterData.mpGraphics && maPrinterData.mpGraphics->maGraphicsData.mpVCLGraphics )
		delete maPrinterData.mpGraphics->maGraphicsData.mpVCLGraphics;
	if ( maPrinterData.mpGraphics )
		delete maPrinterData.mpGraphics;
	maPrinterData.mpGraphics = NULL;
	maPrinterData.mbGraphics = FALSE;
	maPrinterData.mpVCLPrintJob->endPage();
	return TRUE;
}

// -----------------------------------------------------------------------

ULONG SalPrinter::GetErrorCode()
{
	if ( !maPrinterData.mbStarted || maPrinterData.mpVCLPrintJob->isFinished() )
		return SAL_PRINTER_ERROR_ABORT;
	else
		return 0;
}

// -----------------------------------------------------------------------

XubString SalPrinter::GetPageRange()
{
	return maPrinterData.mpVCLPrintJob->getPageRange();
}

// -----------------------------------------------------------------------

void SalPrinter::SetResolution( long nDPIX, long nDPIY )
{
	if ( maPrinterData.mpVCLPageFormat )
		maPrinterData.mpVCLPageFormat->setPageResolution( nDPIX, nDPIY );
}

// =======================================================================

SalPrinterData::SalPrinterData()
{
	mbStarted = FALSE;
	mpGraphics = NULL;
	mbGraphics = FALSE;
	mpVCLPrintJob = new com_sun_star_vcl_VCLPrintJob();
	mpVCLPageFormat = NULL;
}

// -----------------------------------------------------------------------

SalPrinterData::~SalPrinterData()
{
	if ( mpGraphics )
		delete mpGraphics;
	if ( mpVCLPageFormat )
	{
		GetSalData()->maVCLPageFormats.remove( mpVCLPageFormat );
		delete mpVCLPageFormat;
	}
	if ( mpVCLPrintJob )
	{
		mpVCLPrintJob->dispose();
		delete mpVCLPrintJob;
	}
}

// =======================================================================

SalInfoPrinterData::SalInfoPrinterData()
{
	mpGraphics = new SalGraphics();
	mbGraphics = FALSE;
	mpVCLPageFormat = NULL;
}

// -----------------------------------------------------------------------

SalInfoPrinterData::~SalInfoPrinterData()
{
	if ( mpGraphics )
		delete mpGraphics;
	if ( mpVCLPageFormat )
	{
		GetSalData()->maVCLPageFormats.remove( mpVCLPageFormat );
		delete mpVCLPageFormat;
	}
}

// -----------------------------------------------------------------------

int SalInfoPrinter::GetLandscapeAngle( const ImplJobSetup* pSetupData )
{
#ifdef DEBUG
	fprintf( stderr, "SalInfoPrinter::GetLandscapeAngle not implemented\n" );
#endif
	return 900;
}

// -----------------------------------------------------------------------

void SalInfoPrinter::InitPaperFormats( const ImplJobSetup* pSetupData )
{
#ifdef DEBUG
	fprintf( stderr, "SalInfoPrinter::InitPaperFormats not implemented\n" );
#endif
}
