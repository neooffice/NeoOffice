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

	virtual SalGraphics*	AcquireGraphics() SAL_OVERRIDE;
	virtual void			ReleaseGraphics( SalGraphics* pGraphics ) SAL_OVERRIDE;
	virtual bool			Setup( SalFrame* pFrame, ImplJobSetup* pSetupData ) SAL_OVERRIDE;
	virtual bool			SetPrinterData( ImplJobSetup* pSetupData ) SAL_OVERRIDE;
	virtual bool			SetData( sal_uLong nFlags, ImplJobSetup* pSetupData ) SAL_OVERRIDE;
	virtual void			GetPageInfo( const ImplJobSetup* pSetupData, long& rOutWidth, long& rOutHeight, long& rPageOffX, long& rPageOffY, long& rPageWidth, long& rPageHeight ) SAL_OVERRIDE;
	virtual sal_uLong		GetCapabilities( const ImplJobSetup* pSetupData, sal_uInt16 nType ) SAL_OVERRIDE;
	virtual sal_uLong		GetPaperBinCount( const ImplJobSetup* pSetupData ) SAL_OVERRIDE;
	virtual OUString		GetPaperBinName( const ImplJobSetup* pSetupData, sal_uLong nPaperBin ) SAL_OVERRIDE;
	virtual void			InitPaperFormats( const ImplJobSetup* pSetupData ) SAL_OVERRIDE;
	virtual int				GetLandscapeAngle( const ImplJobSetup* pSetupData ) SAL_OVERRIDE;

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
	Paper					mePaperFormat;
	long					mnPaperWidth;
	long					mnPaperHeight;
	id						mpInfo;
	sal_Bool				mbPaperRotated;
	id						mpPrintJob;
	::std::list< id >		maSecurityScopeURLList;

public:
							JavaSalPrinter( JavaSalInfoPrinter *pInfoPrinter );
	virtual					~JavaSalPrinter();

	virtual bool			StartJob( const OUString* pFileName, const OUString& rJobName, const OUString& rAppName, sal_uLong nCopies, bool bCollate, bool bDirect, ImplJobSetup* pSetupData ) SAL_OVERRIDE;
	virtual bool			StartJob( const OUString* pFileName, const OUString& rJobName, const OUString& rAppName, ImplJobSetup* pSetupData, vcl::PrinterController& rController ) SAL_OVERRIDE;
	virtual bool			EndJob() SAL_OVERRIDE;
	virtual bool			AbortJob() SAL_OVERRIDE;
	virtual SalGraphics*	StartPage( ImplJobSetup* pSetupData, bool bNewJobData ) SAL_OVERRIDE;
	virtual bool			EndPage() SAL_OVERRIDE;
	virtual sal_uLong		GetErrorCode() SAL_OVERRIDE;

	void					UpdatePageInfo( const ImplJobSetup* pSetupData );
};

#endif // _SV_SALPRN_H
