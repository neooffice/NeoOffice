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

#define _SV_SALPRN_CXX

#include <stdio.h>

#ifndef _SV_SALPRN_HXX
#include <salprn.hxx>
#endif

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
#ifdef DEBUG
	fprintf( stderr, "SalInfoPrinter::GetGraphics not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

void SalInfoPrinter::ReleaseGraphics( SalGraphics* )
{
#ifdef DEBUG
	fprintf( stderr, "SalInfoPrinter::ReleaseGraphics not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::Setup( SalFrame* pFrame, ImplJobSetup* pSetupData )
{
#ifdef DEBUG
	fprintf( stderr, "SalInfoPrinter::Setup not implemented\n" );
#endif
	return FALSE;
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::SetPrinterData( ImplJobSetup* pSetupData )
{
#ifdef DEBUG
	fprintf( stderr, "SalInfoPrinter::SetPrinterData not implemented\n" );
#endif
	return FALSE;
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
#ifdef DEBUG
	fprintf( stderr, "SalInfoPrinter::GetCapabilities not implemented\n" );
#endif
	return 0;
}

// -----------------------------------------------------------------------

void SalInfoPrinter::GetPageInfo( const ImplJobSetup*,
								  long& rOutWidth, long& rOutHeight,
								  long& rPageOffX, long& rPageOffY,
								  long& rPageWidth, long& rPageHeight )
{
#ifdef DEBUG
	fprintf( stderr, "SalInfoPrinter::GetPageInfo not implemented\n" );
#endif
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
#ifdef DEBUG
	fprintf( stderr, "SalPrinter::StartJob not implemented\n" );
#endif
	return FALSE;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::EndJob()
{
#ifdef DEBUG
	fprintf( stderr, "SalPrinter::EndJob not implemented\n" );
#endif
	return FALSE;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::AbortJob()
{
#ifdef DEBUG
	fprintf( stderr, "SalPrinter::AbortJob not implemented\n" );
#endif
	return FALSE;
}

// -----------------------------------------------------------------------

SalGraphics* SalPrinter::StartPage( ImplJobSetup* pSetupData, BOOL bNewJobData )
{
#ifdef DEBUG
	fprintf( stderr, "SalPrinter::StartPage not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::EndPage()
{
#ifdef DEBUG
	fprintf( stderr, "SalPrinter::EndPage not implemented\n" );
#endif
	return FALSE;
}

// -----------------------------------------------------------------------

ULONG SalPrinter::GetErrorCode()
{
#ifdef DEBUG
	fprintf( stderr, "SalPrinter::GetErrorCode not implemented\n" );
#endif
	return 0;
}

// =======================================================================

SalPrinterData::SalPrinterData()
{
}

// -----------------------------------------------------------------------

SalPrinterData::~SalPrinterData()
{
}

// =======================================================================

SalInfoPrinterData::SalInfoPrinterData()
{
}

// -----------------------------------------------------------------------

SalInfoPrinterData::~SalInfoPrinterData()
{
}
