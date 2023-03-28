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

#ifndef _SV_SALDATA_HXX
#define _SV_SALDATA_HXX

#include <list>
#include <map>

#include <boost/unordered_map.hpp>
#include <osl/conditn.hxx>

class JavaPhysicalFontFace;
class JavaSalBitmap;
class JavaSalEvent;
class JavaSalFrame;
class JavaSalGraphics;
class JavaSalInstance;
class JavaSalVirtualDevice;

#include "salframe.hxx"
#include "svdata.hxx"

// -----------
// - SalData -
// -----------

class SalData
{
public:
	JavaSalInstance*		mpFirstInstance;
	::std::list< JavaSalFrame* >	maFrameList;
	JavaSalFrame*			mpFocusFrame;
	timeval					maTimeout;
	sal_uLong				mnTimerInterval;
	OUString				maDefaultPrinter;
	::std::map< OUString, JavaPhysicalFontFace* >	maFontNameMapping;
	::boost::unordered_map< OUString, JavaPhysicalFontFace*, OUStringHash >	maJavaFontNameMapping;
	::boost::unordered_map< OUString, sal_IntPtr, OUStringHash >	maJavaNativeFontMapping;
	::boost::unordered_map< sal_IntPtr, JavaPhysicalFontFace* >	maNativeFontMapping;
	::boost::unordered_map< sal_IntPtr, JavaPhysicalFontFace* >	maPlainFamilyNativeFontMapping;
	::boost::unordered_map< sal_IntPtr, ::boost::unordered_map< sal_IntPtr, JavaPhysicalFontFace* > >	maItalicNativeFontMapping;
	::boost::unordered_map< sal_IntPtr, ::boost::unordered_map< sal_IntPtr, JavaPhysicalFontFace* > >	maUnitalicNativeFontMapping;
	JavaSalFrame*			mpPresentationFrame;
	::osl::Condition		maNativeEventCondition;
	bool					mbInNativeModalSheet;
	JavaSalFrame*			mpNativeModalSheetFrame;
	::std::list< JavaSalGraphics* >	maGraphicsList;
	SalFrame::SalPointerState	maLastPointerState;
	JavaSalFrame*			mpLastDragFrame;
	bool					mbInSignalHandler;
	::std::list< JavaSalEvent* >	maPendingDocumentEventsList;
	bool					mbDoubleScrollbarArrows;
	JavaSalFrame*			mpCaptureFrame;
	JavaSalFrame*			mpLastResizeFrame;
	timeval					maLastResizeTime;
	JavaSalFrame*			mpLastMouseMoveFrame;
	::vcl::Font				maSystemFont;
	::vcl::Font				maLabelFont;
	::vcl::Font				maMenuFont;
	::vcl::Font				maTitleBarFont;
	::std::list< JavaSalVirtualDevice* >	maVirDevList;

							SalData();
							~SalData();
};

inline void SetSalData( SalData* pData )
{
	ImplGetSVData()->mpSalData = pData;
}

inline SalData* GetSalData()
{
	return ImplGetSVData()->mpSalData;
}

#endif // _SV_SALDATA_HXX
