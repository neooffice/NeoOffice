/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU General Public License Version 2.1.
 *
 *
 *    GNU General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *    Modified February 2006 by Patrick Luby. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
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
#ifndef _VCL_DLLAPI_H
#include "dllapi.h"
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

struct VCL_DLLPUBLIC SalPrinterQueueInfo
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

class VCL_DLLPUBLIC SalInfoPrinter
{
public:
    std::vector< vcl::PaperInfo  >		m_aPaperFormats;	// all printer supported formats
    bool								m_bPapersInit;		// set to true after InitPaperFormats
    
    SalInfoPrinter() {}
    virtual ~SalInfoPrinter();

	// SalGraphics or NULL, but two Graphics for all SalFrames
	// must be returned
	virtual SalGraphics*			GetGraphics() = 0;
	virtual void					ReleaseGraphics( SalGraphics* pGraphics ) = 0;

	virtual BOOL					Setup( SalFrame* pFrame, ImplJobSetup* pSetupData ) = 0;
    // This function set the driver data and
    // set the new indepen data in pSetupData
	virtual BOOL					SetPrinterData( ImplJobSetup* pSetupData ) = 0;
    // This function merged the indepen driver data
    // and set the new indepen data in pSetupData
    // Only the data must changed, where the bit
    // in nFlags is set
	virtual BOOL					SetData( ULONG nFlags, ImplJobSetup* pSetupData ) = 0;

	virtual void					GetPageInfo( const ImplJobSetup* pSetupData,
                                                 long& rOutWidth, long& rOutHeight,
                                                 long& rPageOffX, long& rPageOffY,
                                                 long& rPageWidth, long& rPageHeight ) = 0;
	virtual ULONG					GetCapabilities( const ImplJobSetup* pSetupData, USHORT nType ) = 0;
	virtual ULONG					GetPaperBinCount( const ImplJobSetup* pSetupData ) = 0;
	virtual String					GetPaperBinName( const ImplJobSetup* pSetupData, ULONG nPaperBin ) = 0;
	// fills m_aPaperFormats and sets m_bPapersInit to true
    virtual void					InitPaperFormats( const ImplJobSetup* pSetupData ) = 0;
    // returns angle that a landscape page will be turned counterclockwise wrt to portrait
    virtual int					GetLandscapeAngle( const ImplJobSetup* pSetupData ) = 0;
    virtual DuplexMode          GetDuplexMode( const ImplJobSetup* pSetupData ) = 0;
};


// --------------
// - SalPrinter -
// --------------

class VCL_DLLPUBLIC SalPrinter
{
public: 					// public for Sal Implementation
    SalPrinter() {}
    virtual ~SalPrinter();

	virtual BOOL					StartJob( const XubString* pFileName,
                                              const XubString& rJobName,
                                              const XubString& rAppName,
                                              ULONG nCopies, BOOL bCollate,
                                              ImplJobSetup* pSetupData ) = 0;
	virtual BOOL					EndJob() = 0;
	virtual BOOL					AbortJob() = 0;
	virtual SalGraphics*			StartPage( ImplJobSetup* pSetupData, BOOL bNewJobData ) = 0;
	virtual BOOL					EndPage() = 0;
	virtual ULONG					GetErrorCode() = 0;
#ifdef USE_JAVA
	virtual XubString				GetPageRange() = 0;
#endif// USE_JAVA
};

#endif // _SV_SALPRN_HXX
