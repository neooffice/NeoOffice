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

#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#define	_SV_COM_SUN_STAR_VCL_VCLFONT_HXX

#ifndef _SV_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif
#ifndef _VCL_VCLENUM_HXX
#include <vclenum.hxx>
#endif

namespace vcl {

class com_sun_star_vcl_VCLFontList;
class com_sun_star_vcl_VCLGraphics;

class com_sun_star_vcl_VCLFont : public java_lang_Object
{
protected:
	static jclass		theClass;

public:
	static jboolean		useDefaultFont;

	static jclass		getMyClass();
	static com_sun_star_vcl_VCLFontList*	getAllFonts();

						com_sun_star_vcl_VCLFont( jobject myObj ) : java_lang_Object( myObj ) {};

	com_sun_star_vcl_VCLFont*	deriveFont( long _par0, sal_Bool _par1, sal_Bool _par2, short _par3, sal_Bool _par4 );
	long				getAscent();
	long				getDescent();
	com_sun_star_vcl_VCLFont*	getDefaultFont();
	FontFamily			getFamilyType();
	long				getKerning( USHORT _par0, USHORT _par1 );
	long				getLeading();
	::rtl::OUString		getName();
	void*				getNativeFont();
	short				getOrientation();
	java_lang_Object*	getPeer();
	long				getSize();
	sal_Bool			isAntialiased();
	sal_Bool			isBold();
	sal_Bool			isItalic();
};

class com_sun_star_vcl_VCLFontList
{
public:
	jsize				mnCount;
	com_sun_star_vcl_VCLFont**	mpFonts;

						com_sun_star_vcl_VCLFontList() : mnCount(0), mpFonts(NULL) {};
	virtual				~com_sun_star_vcl_VCLFontList();
};

} // namespace vcl

#endif // _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
