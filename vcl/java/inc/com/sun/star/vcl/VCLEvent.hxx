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

#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENT_HXX
#define	_SV_COM_SUN_STAR_VCL_VCLEVENT_HXX

#ifndef _SV_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif
#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

// Custom event types
#define SALEVENT_OPENDOCUMENT	((USHORT)100)
#define SALEVENT_PRINTDOCUMENT	((USHORT)101)

class SalFrame;

namespace vcl {

class com_sun_star_vcl_VCLFrame;

class com_sun_star_vcl_VCLEvent: public java_lang_Object
{
	void				dispatchEvent( USHORT nID, SalFrame *pFrame, void *pData );

protected:
	static jclass		theClass;

public:
	static jclass		getMyClass();

						com_sun_star_vcl_VCLEvent( jobject myObj ) : java_lang_Object( myObj ) {};
						com_sun_star_vcl_VCLEvent( USHORT nID, const SalFrame *pFrame, void *pData );
	virtual				~com_sun_star_vcl_VCLEvent() {};

	void				dispatch();
	const Rectangle		getBounds();
	void*				getData();
	SalFrame*			getFrame();
	USHORT				getKeyChar();
	USHORT				getKeyCode();
	USHORT				getID();
	USHORT				getModifiers();
	::rtl::OUString		getPath();
	const Rectangle		getUpdateRect();
	ULONG				getWhen();
	long				getX();
	long				getY();
};

} // namespace vcl

#endif // _SV_COM_SUN_STAR_VCL_VCLEVENT_HXX
