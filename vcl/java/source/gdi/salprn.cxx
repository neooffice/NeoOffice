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

#ifndef _SV_SALPRN_HXX
#include <salprn.hxx>
#endif
#ifndef _SV_SALPTYPE_HXX
#include <salptype.hxx>
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

using namespace rtl;
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

	BOOL bOK = maPrinterData.mpVCLPageFormat->setup();
	if ( !bOK )
		maPrinterData.mpVCLPageFormat->setOrientation( nOrientation );

	// Update values
	SetData( 0, pSetupData );

	return TRUE;
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::SetPrinterData( ImplJobSetup* pSetupData )
{
	// Clear driver data
	if ( pSetupData->mpDriverData )
	{
		rtl_freeMemory( pSetupData->mpDriverData );
		pSetupData->mpDriverData = NULL;
		pSetupData->mnDriverDataLen = 0;
	}

	// Set but don't update values
	SetData( SAL_JOBSET_ALL, pSetupData );

	return TRUE;
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::SetData( ULONG nFlags, ImplJobSetup* pSetupData )
{
	// Set or update values
	if ( ! ( nFlags & SAL_JOBSET_ORIENTATION ) )
		pSetupData->meOrientation = maPrinterData.mpVCLPageFormat->getOrientation();
	else
		maPrinterData.mpVCLPageFormat->setOrientation( pSetupData->meOrientation );

	if ( ! ( nFlags & SAL_JOBSET_PAPERBIN ) )
		pSetupData->mnPaperBin = 0;


	if ( ! ( nFlags & SAL_JOBSET_PAPERSIZE ) )
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
						   ImplJobSetup* pSetupData )
{
	maPrinterData.mbStarted = maPrinterData.mpVCLPrintJob->startJob( maPrinterData.mpVCLPageFormat, OUString( rJobName ) );
	return maPrinterData.mbStarted;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::EndJob()
{
	maPrinterData.mpVCLPrintJob->endJob();
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

	com_sun_star_vcl_VCLGraphics *pVCLGraphics = maPrinterData.mpVCLPrintJob->startPage( pSetupData->meOrientation );
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
		delete mpVCLPageFormat;
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
		delete mpVCLPageFormat;
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
