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

#ifndef _SV_COM_SUN_STAR_VCL_VCLSCREEN_HXX
#define	_SV_COM_SUN_STAR_VCL_VCLSCREEN_HXX

#ifndef _SV_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif
#ifndef _SV_SALGTYPE_HXX
#include <salgtype.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

namespace vcl {

class com_sun_star_vcl_VCLScreen : public java_lang_Object
{
protected:
	static jclass		theClass;

public:
	static jclass		getMyClass();
	static SalColor		getControlColor();
	static unsigned int	getDefaultScreenNumber();
	static const Rectangle	getScreenBounds( long _par0, long _par1, long _par2, long _par3, sal_Bool _par4 );
	static const Rectangle	getScreenBounds( unsigned int _par0, sal_Bool _par1 );
	static unsigned int	getScreenCount();
	static SalColor		getTextHighlightColor();
	static SalColor		getTextHighlightTextColor();
	static SalColor		getTextTextColor();
	static const Rectangle	getVirtualScreenBounds();

						com_sun_star_vcl_VCLScreen( jobject myObj ) : java_lang_Object( myObj ) {};
	virtual				~com_sun_star_vcl_VCLScreen() {};
};

} // namespace vcl

#endif // _SV_COM_SUN_STAR_VCL_VCLSCREEN_HXX
