/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
