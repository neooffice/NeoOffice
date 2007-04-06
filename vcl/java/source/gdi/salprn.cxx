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

#ifndef _SV_SALPRN_H
#include <salprn.h>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
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

JavaSalInfoPrinter::JavaSalInfoPrinter()
{
	mpGraphics = new JavaSalGraphics();
	mbGraphics = FALSE;
	mpVCLPageFormat = NULL;
}

// -----------------------------------------------------------------------

JavaSalInfoPrinter::~JavaSalInfoPrinter()
{
	if ( mpGraphics )
		delete mpGraphics;
	if ( mpVCLPageFormat )
		delete mpVCLPageFormat;
}

// -----------------------------------------------------------------------

SalGraphics* JavaSalInfoPrinter::GetGraphics()
{
	if ( mbGraphics )
		return NULL;

	mpGraphics->mpVCLGraphics = mpVCLPageFormat->getGraphics();
	mbGraphics = TRUE;

	return mpGraphics;
}

// -----------------------------------------------------------------------

void JavaSalInfoPrinter::ReleaseGraphics( SalGraphics* pGraphics )
{
	if ( pGraphics != mpGraphics )
		return;

	if ( mpGraphics && mpGraphics->mpVCLGraphics )
		delete mpGraphics->mpVCLGraphics;
	mpGraphics->mpVCLGraphics = NULL;
	mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL JavaSalInfoPrinter::Setup( SalFrame* pFrame, ImplJobSetup* pSetupData )
{
	// Display a native page setup dialog
	Orientation nOrientation = mpVCLPageFormat->getOrientation();
	mpVCLPageFormat->setOrientation( pSetupData->meOrientation );

	BOOL bOK = mpVCLPageFormat->setup();
	if ( !bOK )
		mpVCLPageFormat->setOrientation( nOrientation );

	// Update values
	SetData( 0, pSetupData );

	return TRUE;
}

// -----------------------------------------------------------------------

BOOL JavaSalInfoPrinter::SetPrinterData( ImplJobSetup* pSetupData )
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

BOOL JavaSalInfoPrinter::SetData( ULONG nFlags, ImplJobSetup* pSetupData )
{
	// Set or update values
	if ( ! ( nFlags & SAL_JOBSET_ORIENTATION ) )
		pSetupData->meOrientation = mpVCLPageFormat->getOrientation();
	else
		mpVCLPageFormat->setOrientation( pSetupData->meOrientation );

	if ( ! ( nFlags & SAL_JOBSET_PAPERBIN ) )
		pSetupData->mnPaperBin = 0;

	if ( ! ( nFlags & SAL_JOBSET_PAPERSIZE ) )
	{
		pSetupData->mePaperFormat = mpVCLPageFormat->getPaperType();
		if ( pSetupData->mePaperFormat == PAPER_USER )
		{
			Size aSize( mpVCLPageFormat->getPageSize() );
			Size aResolution( mpVCLPageFormat->getTextResolution() );
			pSetupData->mnPaperWidth = aSize.Width() * 2540 / aResolution.Width();
			pSetupData->mnPaperHeight = aSize.Height() * 2540 / aResolution.Height();
		}
		else
		{
			pSetupData->mnPaperWidth = 0;
			pSetupData->mnPaperHeight = 0;
		}
	}
	else
	{
		mpVCLPageFormat->setOrientation( pSetupData->meOrientation );
		mpVCLPageFormat->setPaperType( pSetupData->mePaperFormat, pSetupData->mnPaperWidth * 72 / 2540, pSetupData->mnPaperHeight * 72 / 2540 );
	}

	return TRUE;
}

// -----------------------------------------------------------------------

ULONG JavaSalInfoPrinter::GetPaperBinCount( const ImplJobSetup* pSetupData )
{
	// Return a dummy value
	return 1;
}

// -----------------------------------------------------------------------

XubString JavaSalInfoPrinter::GetPaperBinName( const ImplJobSetup* pSetupData, ULONG nPaperBin )
{
	// Return a dummy value
	return XubString();
}

// -----------------------------------------------------------------------

ULONG JavaSalInfoPrinter::GetCapabilities( const ImplJobSetup* pSetupData, USHORT nType )
{
	ULONG nRet = 0;

	switch ( nType )
	{
		case PRINTER_CAPABILITIES_SETORIENTATION:
		case PRINTER_CAPABILITIES_SETPAPER:
		case PRINTER_CAPABILITIES_SETPAPERSIZE:
			nRet = 1;
			break;
		default:
			break;
	}

	return nRet;
}

// -----------------------------------------------------------------------

void JavaSalInfoPrinter::GetPageInfo( const ImplJobSetup* pSetupData,
								  long& rOutWidth, long& rOutHeight,
								  long& rPageOffX, long& rPageOffY,
								  long& rPageWidth, long& rPageHeight )
{
	Size aSize( mpVCLPageFormat->getPageSize() );
	Rectangle aRect( mpVCLPageFormat->getImageableBounds() );

	// Fix bug 2278 by detecting if the OOo code wants rotated bounds
	if ( pSetupData->meOrientation != ORIENTATION_PORTRAIT )
	{
		rPageWidth = aSize.Height();
		rPageHeight = aSize.Width();
		rPageOffX = aRect.nTop;
		rPageOffY = aRect.nLeft;
		rOutWidth = aRect.nBottom - aRect.nTop + 1;
		rOutHeight = aRect.nRight - aRect.nLeft + 1;
	}
	else
	{
		rPageWidth = aSize.Width();
		rPageHeight = aSize.Height();
		rPageOffX = aRect.nLeft;
		rPageOffY = aRect.nTop;
		rOutWidth = aRect.nRight - aRect.nLeft + 1;
		rOutHeight = aRect.nBottom - aRect.nTop + 1;
	}
}

// -----------------------------------------------------------------------

void JavaSalInfoPrinter::InitPaperFormats( const ImplJobSetup* pSetupData )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalInfoPrinter::InitPaperFormats not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

int JavaSalInfoPrinter::GetLandscapeAngle( const ImplJobSetup* pSetupData )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalInfoPrinter::GetLandscapeAngle not implemented\n" );
#endif
	return 900;
}

// -----------------------------------------------------------------------

DuplexMode JavaSalInfoPrinter::GetDuplexMode( const ImplJobSetup* pJobSetup )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalInfoPrinter::GetDuplexMode not implemented\n" );
#endif
	return DUPLEX_UNKNOWN;
}

// =======================================================================

JavaSalPrinter::JavaSalPrinter()
{
	mbStarted = FALSE;
	mpGraphics = NULL;
	mbGraphics = FALSE;
	mpVCLPrintJob = new com_sun_star_vcl_VCLPrintJob();
	mpVCLPageFormat = NULL;
	mePaperFormat = PAPER_USER;
	mnPaperWidth = 0;
	mnPaperHeight = 0;
}

// -----------------------------------------------------------------------

JavaSalPrinter::~JavaSalPrinter()
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

// -----------------------------------------------------------------------

BOOL JavaSalPrinter::StartJob( const XubString* pFileName,
							   const XubString& rJobName,
							   const XubString& rAppName,
							   ULONG nCopies, BOOL bCollate,
							   ImplJobSetup* pSetupData )
{
	sal_Bool bFirstPass = ( rJobName.Len() ? sal_False : sal_True );

	// Set paper type
	if ( !bFirstPass )
	{
		mpVCLPageFormat->setOrientation( pSetupData->meOrientation );
		mePaperFormat = pSetupData->mePaperFormat;
		mnPaperWidth = pSetupData->mnPaperWidth;
		mnPaperHeight = pSetupData->mnPaperHeight;
		mpVCLPageFormat->setPaperType( pSetupData->mePaperFormat, pSetupData->mnPaperWidth * 72 / 2540, pSetupData->mnPaperHeight * 72 / 2540 );
	}

	// Fix bug by detecting when an OOo printer job is being reused for serial
	// print jobs
	maJobName = XubString( rJobName );
	mbStarted = mpVCLPrintJob->startJob( mpVCLPageFormat, OUString( rJobName ), !bFirstPass ? sal_True : mbStarted );

	return mbStarted;
}

// -----------------------------------------------------------------------

BOOL JavaSalPrinter::EndJob()
{
	mpVCLPrintJob->endJob();
	mbStarted = FALSE;
	return TRUE;
}

// -----------------------------------------------------------------------

BOOL JavaSalPrinter::AbortJob()
{
	mpVCLPrintJob->abortJob();
	return TRUE;
}

// -----------------------------------------------------------------------

SalGraphics* JavaSalPrinter::StartPage( ImplJobSetup* pSetupData, BOOL bNewJobData )
{
	if ( mbGraphics )
		return NULL;

	// Fix bug 2060 by creating a new print job with the same printer if a
	// change in paper size is requested. Change in orientation does not
	// require a new print job.
	if ( bNewJobData )
	{
		bool bEndJob = false;
		if ( pSetupData->mePaperFormat != mePaperFormat )
			bEndJob = true;
		else if ( pSetupData->mePaperFormat == PAPER_USER && ( pSetupData->mnPaperWidth != mnPaperWidth || pSetupData->mnPaperHeight != mnPaperHeight ) )
			bEndJob = true;

		if ( bEndJob )
		{
			EndJob();
			delete mpVCLPrintJob;
			mpVCLPrintJob = new com_sun_star_vcl_VCLPrintJob();
			if ( !StartJob( NULL, maJobName, XubString(), 1, TRUE, pSetupData ) )
				return NULL;
		}
	}

	com_sun_star_vcl_VCLGraphics *pVCLGraphics = mpVCLPrintJob->startPage( pSetupData->meOrientation );
	if ( !pVCLGraphics )
		return NULL;

	mpGraphics = new JavaSalGraphics();
	mpGraphics->mpVCLGraphics = pVCLGraphics;
	mpGraphics->mpPrinter = this;
	mbGraphics = TRUE;

	return mpGraphics;
}

// -----------------------------------------------------------------------

BOOL JavaSalPrinter::EndPage()
{
	if ( mpGraphics && mpGraphics->mpVCLGraphics )
		delete mpGraphics->mpVCLGraphics;
	if ( mpGraphics )
		delete mpGraphics;
	mpGraphics = NULL;
	mbGraphics = FALSE;
	mpVCLPrintJob->endPage();
	return TRUE;
}

// -----------------------------------------------------------------------

ULONG JavaSalPrinter::GetErrorCode()
{
	if ( !mbStarted || mpVCLPrintJob->isFinished() )
		return SAL_PRINTER_ERROR_ABORT;
	else
		return 0;
}

// -----------------------------------------------------------------------

XubString JavaSalPrinter::GetPageRange()
{
	return mpVCLPrintJob->getPageRange( mpVCLPageFormat );
}
