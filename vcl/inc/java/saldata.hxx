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
#include <hash_map>
#include <map>

#include <osl/conditn.hxx>

class JavaImplFontData;
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
	::std::list< JavaSalBitmap* >	maBitmapList;
	::std::list< JavaSalFrame* >	maFrameList;
	JavaSalFrame*			mpFocusFrame;
	timeval					maTimeout;
	sal_uLong				mnTimerInterval;
	XubString				maDefaultPrinter;
	::std::map< String, JavaImplFontData* >	maFontNameMapping;
	::std::hash_map< ::rtl::OUString, JavaImplFontData*, ::rtl::OUStringHash >	maJavaFontNameMapping;
	::std::hash_map< OUString, sal_IntPtr, OUStringHash >	maJavaNativeFontMapping;
	::std::hash_map< sal_IntPtr, JavaImplFontData* >	maNativeFontMapping;
	::std::hash_map< sal_IntPtr, JavaImplFontData* >	maPlainFamilyNativeFontMapping;
	::std::hash_map< sal_IntPtr, ::std::hash_map< sal_IntPtr, JavaImplFontData* > >	maItalicNativeFontMapping;
	::std::hash_map< sal_IntPtr, ::std::hash_map< sal_IntPtr, JavaImplFontData* > >	maUnitalicNativeFontMapping;
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
	::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >	mxClipboard;

							SalData();
							~SalData();
};

inline void SetSalData( SalData* pData )
{
	ImplGetSVData()->mpSalData = (void*)pData;
}

inline SalData* GetSalData()
{
	return (SalData*)ImplGetSVData()->mpSalData;
}

inline SalData* GetAppSalData()
{
	return (SalData*)ImplGetAppSVData()->mpSalData;
}

#endif // _SV_SALDATA_HXX
