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
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALTIMER_HXX
#include <saltimer.hxx>
#endif
#ifndef _SV_JAVA_LANG_CLASS_HXX
#include "java/lang/Class.hxx"
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENTQUEUE_HXX
#include <com/sun/star/vcl/VCLEventQueue.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLPAGEFORMAT_HXX
#include <com/sun/star/vcl/VCLPageFormat.hxx>
#endif
#ifndef _OSL_CONDITN_HXX_
#if defined MACOSX && defined check
#undef check
#endif
#include <osl/conditn.hxx>
#endif

class ImplFontData;

// -----------
// - SalData -
// -----------

class SalData
{
public:
	SalInstance*			mpFirstInstance;
	::std::list< SalFrame* >	maFrameList;
	SalFrame*				mpFocusFrame;
	timeval					maTimeout;
	SALTIMERPROC			mpTimerProc;
	ULONG					mnTimerInterval;
	XubString				maDefaultPrinter;
	::vcl::com_sun_star_vcl_VCLEventQueue*	mpEventQueue;
	::std::map< ::rtl::OUString, ::vcl::com_sun_star_vcl_VCLFont* >	maFontMapping;
	::std::map< void*, ImplFontData* >	maNativeFontMapping;
	SalFrame*				mpPresentationFrame;
	::std::list< SalFrame* >	maPresentationFrameList;
	::std::list< SalFrame* >	maAlwaysOnTopFrameList;
	::std::list< ::vcl::com_sun_star_vcl_VCLPageFormat* >	maVCLPageFormats;
	::osl::Condition		maNativeEventStartCondition; 
	::osl::Condition		maNativeEventEndCondition; 

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
