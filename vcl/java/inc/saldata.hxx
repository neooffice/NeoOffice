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

#ifndef _SV_SALDATA_HXX
#define _SV_SALDATA_HXX

#include <list>
#include <map>

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENTQUEUE_HXX
#include <com/sun/star/vcl/VCLEventQueue.hxx>
#endif
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif

class JavaImplFontData;
class JavaSalFrame;
class JavaSalGraphics;
class JavaSalInstance;

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
	ULONG					mnTimerInterval;
	XubString				maDefaultPrinter;
	::vcl::com_sun_star_vcl_VCLEventQueue*	mpEventQueue;
	::std::map< String, JavaImplFontData* >	maFontNameMapping;
	::std::map< ::rtl::OUString, JavaImplFontData* >	maJavaFontNameMapping;
	::std::map< int, JavaImplFontData* >	maNativeFontMapping;
	JavaSalFrame*			mpPresentationFrame;
	::osl::Condition		maNativeEventCondition;
	bool					mbInNativeModalSheet;
	JavaSalFrame*			mpNativeModalSheetFrame;
	::std::list< JavaSalGraphics* >	maGraphicsList;
	SalFrame::SalPointerState	maLastPointerState;
	JavaSalFrame*			mpLastDragFrame;

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
