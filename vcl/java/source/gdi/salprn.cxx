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

#include <stdio.h>

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

	maPrinterData.mpGraphics->maGraphicsData.mpVCLGraphics = com_sun_star_vcl_VCLPrintJob::getGraphics();
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
	// Set incoming values
	com_sun_star_vcl_VCLPrintJob::setOrientation( pSetupData->meOrientation );

	// Display a native page setup dialog
	if ( !com_sun_star_vcl_VCLPrintJob::setup() )
		return FALSE;

    // Update the job setup with the new default values
    pSetupData->meOrientation = com_sun_star_vcl_VCLPrintJob::getOrientation();
    Size aSize( com_sun_star_vcl_VCLPrintJob::getPageSize() );
    pSetupData->mnPaperWidth = aSize.Width();
   	pSetupData->mnPaperHeight = aSize.Height();

	return TRUE;
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::SetPrinterData( ImplJobSetup* pSetupData )
{
	// Always return TRUE as there is no driver data that needs updating
	return TRUE;
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::SetData( ULONG nFlags, ImplJobSetup* pSetupData )
{
	// Set values
	if ( nFlags == SAL_JOBSET_ORIENTATION )
		com_sun_star_vcl_VCLPrintJob::setOrientation( pSetupData->meOrientation );

	// Update values
	pSetupData->meOrientation = com_sun_star_vcl_VCLPrintJob::getOrientation();
	Size aSize( com_sun_star_vcl_VCLPrintJob::getPageSize() );
    pSetupData->mnPaperWidth = aSize.Width();
   	pSetupData->mnPaperHeight = aSize.Height();

	return TRUE;
}

// -----------------------------------------------------------------------

ULONG SalInfoPrinter::GetPaperBinCount( const ImplJobSetup* pSetupData )
{
	// Return a dummy value
	return 0;
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
	Size aSize( com_sun_star_vcl_VCLPrintJob::getPageSize() );
	Rectangle aRect( com_sun_star_vcl_VCLPrintJob::getImageableBounds() );
    if ( pSetupData->meOrientation == ORIENTATION_LANDSCAPE )
	{
		rOutWidth = aSize.Height();
		rOutHeight = aSize.Width();
		rPageOffX = aRect.nTop;
		rPageOffY = aRect.nLeft;
		rPageWidth = aRect.nBottom - aRect.nTop;
		rPageHeight = aRect.nRight - aRect.nLeft;
	}
	else
	{
		rOutWidth = aSize.Width();
		rOutHeight = aSize.Height();
		rPageOffX = aRect.nLeft;
		rPageOffY = aRect.nTop;
		rPageWidth = aRect.nRight - aRect.nLeft;
		rPageHeight = aRect.nBottom - aRect.nTop;
	}
}

// =======================================================================

SalPrinter::SalPrinter()
{
	maPrinterData.mpGraphics->maGraphicsData.mpPrinter = this;
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
	maPrinterData.mbStarted = maPrinterData.mpVCLPrintJob->startJob();
	return maPrinterData.mbStarted;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::EndJob()
{
	maPrinterData.mpVCLPrintJob->endJob();
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

	maPrinterData.mpGraphics->maGraphicsData.mpVCLGraphics = maPrinterData.mpVCLPrintJob->startPage();
	if ( !maPrinterData.mpGraphics->maGraphicsData.mpVCLGraphics )
		return NULL;
	maPrinterData.mbGraphics = TRUE;

	return maPrinterData.mpGraphics;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::EndPage()
{
	maPrinterData.mpVCLPrintJob->endPage();
	if ( maPrinterData.mpGraphics && maPrinterData.mpGraphics->maGraphicsData.mpVCLGraphics )
		delete maPrinterData.mpGraphics->maGraphicsData.mpVCLGraphics;
	maPrinterData.mpGraphics->maGraphicsData.mpVCLGraphics = NULL;
	maPrinterData.mbGraphics = FALSE;
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

// =======================================================================

SalPrinterData::SalPrinterData()
{
	mbStarted = FALSE;
	mpGraphics = new SalGraphics();
	mbGraphics = FALSE;
	mpVCLPrintJob = new com_sun_star_vcl_VCLPrintJob();
}

// -----------------------------------------------------------------------

SalPrinterData::~SalPrinterData()
{
	if ( mpGraphics )
		delete mpGraphics;

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
}

// -----------------------------------------------------------------------

SalInfoPrinterData::~SalInfoPrinterData()
{
	if ( mpGraphics )
		delete mpGraphics;
}
