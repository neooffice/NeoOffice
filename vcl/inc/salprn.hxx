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
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
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
 *  =================================================
 *  Modified June 2004 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

#ifndef _SV_SALPRN_HXX
#define _SV_SALPRN_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_SALPRN_H
#include <salprn.h>
#endif
#ifndef _SV_PRNTYPES_HXX
#include <prntypes.hxx>
#endif

#include <vector>

class SalGraphics;
class SalFrame;
struct ImplJobSetup;

// -----------------------
// - SalPrinterQueueInfo -
// -----------------------

struct SalPrinterQueueInfo
{
	XubString				maPrinterName;
	XubString				maDriver;
	XubString				maLocation;
	XubString				maComment;
	ULONG					mnStatus;
	ULONG					mnJobs;
	void*					mpSysData;

							SalPrinterQueueInfo();
							~SalPrinterQueueInfo();
};


// ------------------
// - SalInfoPrinter -
// ------------------

class SalInfoPrinter
{
	friend class SalInstance;

public:
    std::vector< vcl::PaperInfo  >		m_aPaperFormats;	// all printer supported formats
    bool								m_bPapersInit;		// set to true after InitPaperFormats

							SalInfoPrinter();
							~SalInfoPrinter();

public: 					// public for Sal Implementation
	SalInfoPrinterData		maPrinterData;

#ifdef _INCL_SAL_SALIPRN_IMP
#include <saliprn.imp>
#endif

public:
	// SalGraphics or NULL, but two Graphics for all SalFrames
	// must be returned
	SalGraphics*			GetGraphics();
	void					ReleaseGraphics( SalGraphics* pGraphics );

	BOOL					Setup( SalFrame* pFrame, ImplJobSetup* pSetupData );
							// This function set the driver data and
							// set the new indepen data in pSetupData
	BOOL					SetPrinterData( ImplJobSetup* pSetupData );
							// This function merged the indepen driver data
							// and set the new indepen data in pSetupData
							// Only the data must changed, where the bit
							// in nFlags is set
	BOOL					SetData( ULONG nFlags, ImplJobSetup* pSetupData );

	void					GetPageInfo( const ImplJobSetup* pSetupData,
										 long& rOutWidth, long& rOutHeight,
										 long& rPageOffX, long& rPageOffY,
										 long& rPageWidth, long& rPageHeight );
	ULONG					GetCapabilities( const ImplJobSetup* pSetupData, USHORT nType );
	ULONG					GetPaperBinCount( const ImplJobSetup* pSetupData );
	XubString				GetPaperBinName( const ImplJobSetup* pSetupData, ULONG nPaperBin );
	// fills m_aPaperFormats and sets m_bPapersInit to true
    void					InitPaperFormats( const ImplJobSetup* pSetupData );
    // returns angle that a landscape page will be turned counterclockwise wrt to portrait
    int						GetLandscapeAngle( const ImplJobSetup* pSetupData );
};


// --------------
// - SalPrinter -
// --------------

class SalPrinter
{
	friend class SalInstance;

private:
							SalPrinter();
							~SalPrinter();

public: 					// public for Sal Implementation
	SalPrinterData			maPrinterData;

#ifdef _INCL_SAL_SALPRN_IMP
#include <salprn.imp>
#endif

public:
	BOOL					StartJob( const XubString* pFileName,
									  const XubString& rJobName,
									  const XubString& rAppName,
									  ULONG nCopies, BOOL bCollate,
									  ImplJobSetup* pSetupData );
	BOOL					EndJob();
	BOOL					AbortJob();
	SalGraphics*			StartPage( ImplJobSetup* pSetupData, BOOL bNewJobData );
	BOOL					EndPage();
	ULONG					GetErrorCode();
#ifdef USE_JAVA
	XubString				GetPageRange();
#endif	// USE_JAVA
};

#endif // _SV_SALPRN_HXX
