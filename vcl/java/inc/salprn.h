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
#include <vcl/salprn.hxx>
#include <vcl/sv.h>

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
	virtual BOOL			Setup( SalFrame* pFrame, ImplJobSetup* pSetupData );
	virtual BOOL			SetPrinterData( ImplJobSetup* pSetupData );
	virtual BOOL			SetData( ULONG nFlags, ImplJobSetup* pSetupData );
	virtual void			GetPageInfo( const ImplJobSetup* pSetupData, long& rOutWidth, long& rOutHeight, long& rPageOffX, long& rPageOffY, long& rPageWidth, long& rPageHeight );
	virtual ULONG			GetCapabilities( const ImplJobSetup* pSetupData, USHORT nType );
	virtual ULONG			GetPaperBinCount( const ImplJobSetup* pSetupData );
	virtual String			GetPaperBinName( const ImplJobSetup* pSetupData, ULONG nPaperBin );
	virtual void			InitPaperFormats( const ImplJobSetup* pSetupData );
	virtual int				GetLandscapeAngle( const ImplJobSetup* pSetupData );
	virtual DuplexMode		GetDuplexMode( const ImplJobSetup* pSetupData );
	virtual const id		GetPrintInfo() { return mpInfo; }
	virtual sal_Bool		IsPaperRotated() { return mbPaperRotated; }
};

// ------------------
// - JavaSalPrinter -
// ------------------

class JavaSalPrinter : public SalPrinter
{
	JavaSalGraphics*		mpGraphics;
	JavaSalInfoPrinter*		mpInfoPrinter;
	BOOL					mbGraphics;
	XubString				maJobName;
	Paper					mePaperFormat;
	long					mnPaperWidth;
	long					mnPaperHeight;
	BOOL					mbStarted;
	id						mpInfo;
	sal_Bool				mbPaperRotated;
	id						mpPrintOperation;
	oslThread				maPrintThread;
	id						mpPrintView;
	id						mpRetainedPrintObjects;
	::std::list< id >		maSecurityScopeURLList;

public:
							JavaSalPrinter( JavaSalInfoPrinter *pInfoPrinter );
	virtual					~JavaSalPrinter();

	virtual BOOL			StartJob( const XubString* pFileName, const XubString& rJobName, const XubString& rAppName, ULONG nCopies, BOOL bCollate, ImplJobSetup* pSetupData, BOOL bFirstPass );
	virtual BOOL			EndJob();
	virtual BOOL			AbortJob();
	virtual SalGraphics*	StartPage( ImplJobSetup* pSetupData, BOOL bNewJobData );
	virtual BOOL			EndPage();
	virtual ULONG			GetErrorCode();
	virtual XubString		GetJobDisposition();
	virtual XubString		GetJobSavingPath();
	virtual XubString		GetPageRange();
	virtual void			SetJobDisposition( const XubString *pJobDisposition );
	virtual void			SetJobSavingPath( const XubString *pJobSavingPath, sal_Int32 nIteration );

	void					RetainUntilEndOfPrintJob( id aObject );
	void					RunPrintOperation();
};

#endif // _SV_SALPRN_H
