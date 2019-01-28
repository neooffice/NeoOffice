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

#ifndef _SV_SALDATA_HXX
#define _SV_SALDATA_HXX

#include <list>
#include <map>
#include <unordered_map>

#include <osl/conditn.hxx>

class JavaPhysicalFontFace;
class JavaSalBitmap;
class JavaSalEvent;
class JavaSalFrame;
class JavaSalGraphics;
class JavaSalInstance;

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
	::std::unordered_map< OUString, JavaPhysicalFontFace*, OUStringHash >	maJavaFontNameMapping;
	::std::unordered_map< OUString, sal_IntPtr, OUStringHash >	maJavaNativeFontMapping;
	::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* >	maNativeFontMapping;
	::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* >	maPlainFamilyNativeFontMapping;
	::std::unordered_map< sal_IntPtr, ::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* > >	maItalicNativeFontMapping;
	::std::unordered_map< sal_IntPtr, ::std::unordered_map< sal_IntPtr, JavaPhysicalFontFace* > >	maUnitalicNativeFontMapping;
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
