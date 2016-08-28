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
 *  Copyright 2003 Planamesa Inc.
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

#ifndef _SV_SALPRN_H
#define _SV_SALPRN_H

#include <list>

#include <vcl/prntypes.hxx>
#include <vcl/sv.h>

#include "salprn.hxx"

#ifndef __OBJC__
typedef void* id;
#endif	// !__OBJC__

class JavaSalGraphics;
class JavaSalVirtualDevice;

// ----------------------
// - JavaSalInfoPrinter -
// ----------------------

class JavaSalInfoPrinter : public SalInfoPrinter
{
	id						mpInfo;
	sal_Bool				mbPaperRotated;
	JavaSalVirtualDevice*	mpVirDev;

public:
							JavaSalInfoPrinter( ImplJobSetup* pSetupData );
	virtual					~JavaSalInfoPrinter();

	virtual SalGraphics*	GetGraphics();
	virtual void			ReleaseGraphics( SalGraphics* pGraphics );
	virtual sal_Bool		Setup( SalFrame* pFrame, ImplJobSetup* pSetupData );
	virtual sal_Bool		SetPrinterData( ImplJobSetup* pSetupData );
	virtual sal_Bool		SetData( sal_uLong nFlags, ImplJobSetup* pSetupData );
	virtual void			GetPageInfo( const ImplJobSetup* pSetupData, long& rOutWidth, long& rOutHeight, long& rPageOffX, long& rPageOffY, long& rPageWidth, long& rPageHeight );
	virtual sal_uLong		GetCapabilities( const ImplJobSetup* pSetupData, sal_uInt16 nType );
	virtual sal_uLong		GetPaperBinCount( const ImplJobSetup* pSetupData );
	virtual String			GetPaperBinName( const ImplJobSetup* pSetupData, sal_uLong nPaperBin );
	virtual void			InitPaperFormats( const ImplJobSetup* pSetupData );
	virtual int				GetLandscapeAngle( const ImplJobSetup* pSetupData );
	virtual DuplexMode		GetDuplexMode( const ImplJobSetup* pSetupData );

	id						GetPrintInfo() { return mpInfo; }
	sal_Bool				IsPaperRotated() { return mbPaperRotated; }
};

// ------------------
// - JavaSalPrinter -
// ------------------

class JavaSalPrinter : public SalPrinter
{
	JavaSalGraphics*		mpGraphics;
	JavaSalInfoPrinter*		mpInfoPrinter;
	sal_Bool				mbGraphics;
	String					maJobName;
	Paper					mePaperFormat;
	long					mnPaperWidth;
	long					mnPaperHeight;
	sal_Bool				mbStarted;
	id						mpInfo;
	sal_Bool				mbPaperRotated;
	id						mpPrintOperation;
	oslThread				maPrintThread;
	id						mpPrintView;
	::std::list< id >		maSecurityScopeURLList;

public:
							JavaSalPrinter( JavaSalInfoPrinter *pInfoPrinter );
	virtual					~JavaSalPrinter();

	virtual sal_Bool		StartJob( const String* pFileName, const String& rJobName, const String& rAppName, sal_uLong nCopies, bool bCollate, bool bDirect, ImplJobSetup* pSetupData );
	virtual sal_Bool		StartJob( const String* pFileName, const String& rJobName, const String& rAppName, ImplJobSetup* pSetupData, vcl::PrinterController& rController );
	virtual sal_Bool		EndJob();
	virtual sal_Bool		AbortJob();
	virtual SalGraphics*	StartPage( ImplJobSetup* pSetupData, sal_Bool bNewJobData );
	virtual sal_Bool		EndPage();
	virtual sal_uLong		GetErrorCode();

	XubString				GetJobDisposition();
	XubString				GetJobSavingPath();
	XubString				GetPageRange();
	void					SetJobDisposition( const XubString *pJobDisposition );
	void					SetJobSavingPath( const XubString *pJobSavingPath, sal_Int32 nIteration );
	void					RunPrintOperation();
};

#endif // _SV_SALPRN_H
