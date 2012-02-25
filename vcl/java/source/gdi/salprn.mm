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
 * Copyright 2003 Planamesa Inc.
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

#include <salprn.h>
#include <saldata.hxx>
#include <salgdi.h>
#include <vcl/salptype.hxx>
#include <com/sun/star/vcl/VCLGraphics.hxx>
#include <com/sun/star/vcl/VCLPageFormat.hxx>
#include <com/sun/star/vcl/VCLPrintJob.hxx>

static rtl::OUString aPageScalingFactorKey( RTL_CONSTASCII_USTRINGPARAM( "PAGE_SCALING_FACTOR" ) );

using namespace rtl;
using namespace vcl;

// =======================================================================

JavaSalInfoPrinter::JavaSalInfoPrinter( ImplJobSetup* pSetupData ) :
	mpGraphics( new JavaSalGraphics() ),
	mbGraphics( FALSE ),
	mpVCLPageFormat( new com_sun_star_vcl_VCLPageFormat() )
{
	SetData( 0, pSetupData );
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
	BOOL bRet = mpVCLPageFormat->setup();
	if ( bRet )
	{
		// Update values
		SetData( 0, pSetupData );

		// Fix bug 2777 by caching the scaling factor
		pSetupData->maValueMap[ aPageScalingFactorKey ] = OUString::valueOf( mpVCLPageFormat->getScaleFactor() );
	}

	return bRet;
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
			Size aResolution( mpVCLPageFormat->getResolution() );
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

JavaSalPrinter::JavaSalPrinter( const com_sun_star_vcl_VCLPageFormat *pVCLPageFormat ) :
	mbStarted( FALSE ),
	mpGraphics( NULL ),
	mbGraphics( FALSE ),
	mePaperFormat( PAPER_USER ),
	mnPaperWidth( 0 ),
	mnPaperHeight( 0 ),
	mpVCLPageFormat( NULL )
#ifndef USE_NATIVE_PRINTING
	, mpVCLPrintJob( new com_sun_star_vcl_VCLPrintJob() )
#endif	// !USE_NATIVE_PRINTING
{
	if ( pVCLPageFormat )
		mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat( pVCLPageFormat->getJavaObject() );
	else
		mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat();
}

// -----------------------------------------------------------------------

JavaSalPrinter::~JavaSalPrinter()
{
	if ( mbStarted )
		GetSalData()->mpEventQueue->setShutdownDisabled( sal_False );
	if ( mpGraphics )
		delete mpGraphics;
	if ( mpVCLPageFormat )
		delete mpVCLPageFormat;
#ifdef USE_NATIVE_PRINTING
	fprintf( stderr, "JavaSalPrinter::~JavaSalPrinter not implemented\n" );
#else	// USE_NATIVE_PRINTING
	if ( mpVCLPrintJob )
	{
		mpVCLPrintJob->dispose();
		delete mpVCLPrintJob;
	}
#endif	// USE_NATIVE_PRINTING
}

// -----------------------------------------------------------------------

BOOL JavaSalPrinter::StartJob( const XubString* pFileName,
							   const XubString& rJobName,
							   const XubString& rAppName,
							   ULONG nCopies, BOOL bCollate,
							   ImplJobSetup* pSetupData, BOOL bFirstPass )
{
	// Set paper type
	if ( !bFirstPass )
	{
		mpVCLPageFormat->setOrientation( pSetupData->meOrientation );
		mePaperFormat = pSetupData->mePaperFormat;
		mnPaperWidth = pSetupData->mnPaperWidth;
		mnPaperHeight = pSetupData->mnPaperHeight;
		mpVCLPageFormat->setPaperType( pSetupData->mePaperFormat, pSetupData->mnPaperWidth * 72 / 2540, pSetupData->mnPaperHeight * 72 / 2540 );
	}

	float fScaleFactor = 1.0f;
	::std::hash_map< OUString, OUString, OUStringHash >::const_iterator it = pSetupData->maValueMap.find( aPageScalingFactorKey );
	if ( it != pSetupData->maValueMap.end() )
	{
		fScaleFactor = it->second.toFloat();
		if ( fScaleFactor <= 0.0f )
			fScaleFactor = 1.0f;
	}

	// Fix bug by detecting when an OOo printer job is being reused for serial
	// print jobs
	maJobName = XubString( rJobName );
#ifdef USE_NATIVE_PRINTING
	fprintf( stderr, "JavaSalPrinter::StartJob not implemented\n" );
#else	// USE_NATIVE_PRINTING
	mbStarted = mpVCLPrintJob->startJob( mpVCLPageFormat, OUString( rJobName ), fScaleFactor, !bFirstPass ? sal_True : mbStarted );
#endif	// USE_NATIVE_PRINTING

	if ( mbStarted )
		GetSalData()->mpEventQueue->setShutdownDisabled( sal_True );

	return mbStarted;
}

// -----------------------------------------------------------------------

BOOL JavaSalPrinter::EndJob()
{
#ifdef USE_NATIVE_PRINTING
	fprintf( stderr, "JavaSalPrinter::EndPage not implemented\n" );
#else	// USE_NATIVE_PRINTING
	mpVCLPrintJob->endJob();
#endif	// USE_NATIVE_PRINTING
	GetSalData()->mpEventQueue->setShutdownDisabled( sal_False );
	mbStarted = FALSE;
	return TRUE;
}

// -----------------------------------------------------------------------

BOOL JavaSalPrinter::AbortJob()
{
#ifdef USE_NATIVE_PRINTING
	fprintf( stderr, "JavaSalPrinter::StartPage not implemented\n" );
#else	// USE_NATIVE_PRINTING
	mpVCLPrintJob->abortJob();
#endif	// USE_NATIVE_PRINTING
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
		if ( pSetupData->mePaperFormat == PAPER_USER && ( !pSetupData->mnPaperWidth || !pSetupData->mnPaperHeight ) )
		{
			// Fix bug 3660 by ignoring custom paper size with no width or
			// height as that indicates that the OpenOffice.org code is
			// automatically inserting a page
		}
		else if ( pSetupData->mePaperFormat != mePaperFormat )
		{
			bEndJob = true;
		}
		else if ( pSetupData->mePaperFormat == PAPER_USER && ( pSetupData->mnPaperWidth != mnPaperWidth || pSetupData->mnPaperHeight != mnPaperHeight ) )
		{
			bEndJob = true;
		}

		if ( bEndJob )
		{
			EndJob();
#ifdef USE_NATIVE_PRINTING
			fprintf( stderr, "JavaSalPrinter::StartPage not implemented\n" );
			return NULL;
#else	// USE_NATIVE_PRINTING
			delete mpVCLPrintJob;
			mpVCLPrintJob = new com_sun_star_vcl_VCLPrintJob();
			if ( !StartJob( NULL, maJobName, XubString(), 1, TRUE, pSetupData, FALSE ) )
				return NULL;
#endif	// USE_NATIVE_PRINTING
		}
	}

#ifdef USE_NATIVE_PRINTING
	fprintf( stderr, "JavaSalPrinter::StartPage not implemented\n" );
	return NULL;
#else	// USE_NATIVE_PRINTING
	com_sun_star_vcl_VCLGraphics *pVCLGraphics = mpVCLPrintJob->startPage( pSetupData->meOrientation );
	if ( !pVCLGraphics )
		return NULL;
#endif	// USE_NATIVE_PRINTING

	mpGraphics = new JavaSalGraphics();
#ifdef USE_NATIVE_PRINTING
	fprintf( stderr, "JavaSalPrinter::StartPage not implemented\n" );
#else	// USE_NATIVE_PRINTING
	mpGraphics->mpVCLGraphics = pVCLGraphics;
#endif	// USE_NATIVE_PRINTING
	mpGraphics->mpPrinter = this;
	mbGraphics = TRUE;

	return mpGraphics;
}

// -----------------------------------------------------------------------

BOOL JavaSalPrinter::EndPage()
{
	if ( mpGraphics )
	{
		if ( mpGraphics->mpVCLGraphics )
			delete mpGraphics->mpVCLGraphics;
		delete mpGraphics;
	}
	mpGraphics = NULL;
	mbGraphics = FALSE;
#ifdef USE_NATIVE_PRINTING
	fprintf( stderr, "JavaSalPrinter::EndPage not implemented\n" );
#else	// USE_NATIVE_PRINTING
	mpVCLPrintJob->endPage();
#endif	// USE_NATIVE_PRINTING
	return TRUE;
}

// -----------------------------------------------------------------------

ULONG JavaSalPrinter::GetErrorCode()
{
#ifdef USE_NATIVE_PRINTING
	fprintf( stderr, "JavaSalPrinter::GetErrorCode not implemented\n" );
	return SAL_PRINTER_ERROR_ABORT;
#else	// USE_NATIVE_PRINTING
	if ( !mbStarted || mpVCLPrintJob->isFinished() )
		return SAL_PRINTER_ERROR_ABORT;
	else
		return 0;
#endif	// USE_NATIVE_PRINTING
}

// -----------------------------------------------------------------------

XubString JavaSalPrinter::GetPageRange()
{
#ifdef USE_NATIVE_PRINTING
	fprintf( stderr, "JavaSalPrinter::GetPageRange not implemented\n" );
	return XubString();
#else	// USE_NATIVE_PRINTING
	return mpVCLPrintJob->getPageRange( mpVCLPageFormat );
#endif	// USE_NATIVE_PRINTING
}
