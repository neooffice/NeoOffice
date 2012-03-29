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

#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#define	_SV_COM_SUN_STAR_VCL_VCLFONT_HXX

#include <map>

#include <salframe.h>
#include <salprn.h>
#include <salvd.h>
#include <java/lang/Object.hxx>
#include <sal/types.h>
#include <vcl/vclenum.hxx>

#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING

namespace vcl {

class com_sun_star_vcl_VCLGraphics;

class SAL_DLLPRIVATE com_sun_star_vcl_VCLFont : public java_lang_Object
{
	::rtl::OUString		maName;
	sal_IntPtr			mnNativeFont;
	short				mnOrientation;
	::rtl::OUString		maPSName;
	double				mfScaleX;
	float				mfSize;
	sal_Bool			mbAntialiased;
	sal_Bool			mbVertical;
	sal_Bool			mbNativeFontOwner;

protected:
	static jclass		theClass;
	static ::std::map< ::vcl::com_sun_star_vcl_VCLFont*, ::vcl::com_sun_star_vcl_VCLFont* >	maInstancesMap;


public:
	static void			clearNativeFonts();
	static jclass		getMyClass();

						com_sun_star_vcl_VCLFont( ::rtl::OUString aName, float fSize, short nOrientation, sal_Bool bAntialiased, sal_Bool bVertical, double fScaleX );
						com_sun_star_vcl_VCLFont( com_sun_star_vcl_VCLFont *pVCLFont );
	virtual				~com_sun_star_vcl_VCLFont();

	::rtl::OUString		getName();
	sal_IntPtr			getNativeFont();
	short				getOrientation();
	::rtl::OUString		getPSName();
	double				getScaleX();
	float				getSize();
	sal_Bool			isAntialiased();
	sal_Bool			isVertical();
};

} // namespace vcl

#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING

#endif // _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
