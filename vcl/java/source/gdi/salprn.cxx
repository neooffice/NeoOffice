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
#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
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
	if ( maPrinterData.mpVirDev )
		return maPrinterData.mpVirDev->GetGraphics();
	else
		return NULL;
}

// -----------------------------------------------------------------------

void SalInfoPrinter::ReleaseGraphics( SalGraphics* pGraphics )
{
	if ( maPrinterData.mpVirDev )
		maPrinterData.mpVirDev->ReleaseGraphics( pGraphics );
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::Setup( SalFrame* pFrame, ImplJobSetup* pSetupData )
{
	// Display a native page setup dialog
	com_sun_star_vcl_VCLPrintJob::setup();

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
#ifdef DEBUG
	fprintf( stderr, "SalInfoPrinter::SetPrinterData not implemented\n" );
#endif
	return TRUE;
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::SetData( ULONG nFlags, ImplJobSetup* pSetupData )
{
#ifdef DEBUG
	fprintf( stderr, "SalInfoPrinter::SetData not implemented\n" );
#endif
	return FALSE;
}

// -----------------------------------------------------------------------

ULONG SalInfoPrinter::GetPaperBinCount( const ImplJobSetup* pSetupData )
{
#ifdef DEBUG
	fprintf( stderr, "SalInfoPrinter::GetPaperBinCount not implemented\n" );
#endif
	return 0;
}

// -----------------------------------------------------------------------

XubString SalInfoPrinter::GetPaperBinName( const ImplJobSetup* pSetupData, ULONG nPaperBin )
{
#ifdef DEBUG
	fprintf( stderr, "SalInfoPrinter::GetPaperBinName not implemented\n" );
#endif
	return XubString();
}

// -----------------------------------------------------------------------

ULONG SalInfoPrinter::GetCapabilities( const ImplJobSetup* pSetupData, USHORT nType )
{
	return 0;
}

// -----------------------------------------------------------------------

void SalInfoPrinter::GetPageInfo( const ImplJobSetup*,
								  long& rOutWidth, long& rOutHeight,
								  long& rPageOffX, long& rPageOffY,
								  long& rPageWidth, long& rPageHeight )
{
	Size aSize( com_sun_star_vcl_VCLPrintJob::getPageSize() );
	rOutWidth = aSize.Width();
	rOutHeight = aSize.Height();
	Rectangle aRect( com_sun_star_vcl_VCLPrintJob::getImageableBounds() );
	rPageOffX = aRect.nLeft;
	rPageOffY = aRect.nTop;
	rPageWidth = aRect.nRight - aRect.nLeft;
	rPageHeight = aRect.nBottom - aRect.nTop;
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
	return maPrinterData.mpVCLPrintJob->startJob();
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
#ifdef DEBUG
	fprintf( stderr, "SalPrinter::GetErrorCode not implemented\n" );
#endif
	return 0;
}

// -----------------------------------------------------------------------

ULONG SalPrinter::GetCopies()
{
	return maPrinterData.mpVCLPrintJob->getCopies();
}

// -----------------------------------------------------------------------

BOOL SalPrinter::IsCollate()
{
	return maPrinterData.mpVCLPrintJob->isCollate();
}

// =======================================================================

SalPrinterData::SalPrinterData()
{
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
	mpVirDev = NULL;
}

// -----------------------------------------------------------------------

SalInfoPrinterData::~SalInfoPrinterData()
{
}
