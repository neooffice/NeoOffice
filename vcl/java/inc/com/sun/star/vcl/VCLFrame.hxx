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

#ifndef _SV_COM_SUN_STAR_VCL_VCLFRAME_HXX
#define	_SV_COM_SUN_STAR_VCL_VCLFRAME_HXX

#ifndef _SV_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif
#ifndef _SV_SV_H    
#include <sv.h>     
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

class SalFrame;

namespace vcl {

class com_sun_star_vcl_VCLGraphics;

class com_sun_star_vcl_VCLFrame : public java_lang_Object
{
protected:
	static jclass		theClass;

public:
	static jclass		getMyClass();

						com_sun_star_vcl_VCLFrame( jobject myObj ) : java_lang_Object( myObj ) {}
						com_sun_star_vcl_VCLFrame( ULONG nSalFrameStyle, const SalFrame *pFrame );
	virtual				~com_sun_star_vcl_VCLFrame() {}

	void				dispose();
	void				flush();
	const Rectangle		getBounds();
	com_sun_star_vcl_VCLGraphics*	getGraphics();
	const Rectangle		getInsets();
	::rtl::OUString		getKeyName( USHORT _par0 );
	void				setBounds( long _par0, long _par1, long _par2, long _par3 );
	void				setFullScreenMode( sal_Bool _par0 );
	void				setPointer( USHORT _par0 );
	void				setTitle( ::rtl::OUString _par0 );
	void				setVisible( sal_Bool _par0 );
	void				toFront();
};

} // namespace vcl

#endif // _SV_COM_SUN_STAR_VCL_VCLFRAME_HXX
