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
 *  Patrick Luby, June 2004
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2004 by Patrick Luby (patrick.luby@planamesa.com)
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

#ifndef _SV_COM_SUN_STAR_VCL_VCLTEXTLAYOUT_HXX
#define	_SV_COM_SUN_STAR_VCL_VCLTEXTLAYOUT_HXX

#ifndef _SV_SALGTYPE_HXX
#include <salgtype.hxx>
#endif
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif
#ifndef _SV_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif

namespace vcl {

class com_sun_star_vcl_VCLFont;
class com_sun_star_vcl_VCLGraphics;

class com_sun_star_vcl_VCLTextLayout : public java_lang_Object
{
protected:
	static jclass		theClass;

public:
	static jclass		getMyClass();

						com_sun_star_vcl_VCLTextLayout( jobject myObj ) : java_lang_Object( myObj ) {};
						com_sun_star_vcl_VCLTextLayout( ::vcl::com_sun_star_vcl_VCLGraphics *pGraphics, ::vcl::com_sun_star_vcl_VCLFont *pFont );
	virtual				~com_sun_star_vcl_VCLTextLayout() {};

	void				drawText( long _par0, long _par1, SalColor _par2 );
	long				fillDXArray( long *_par0 );
	bool				getBounds( Rectangle& _par0 );
	void				getCaretPositions( int _par0, long *_par1 );
	int					getTextBreak( long _par0, long _par1, int _par2 );
	void				justify( long _par0 );
	void				layoutText( ImplLayoutArgs& _par0 );
	void				setDXArray( const long *_par0, int _par1 );
};

} // namespace vcl

#endif // _SV_COM_SUN_STAR_VCL_VCLTEXTLAYOUT_HXX
