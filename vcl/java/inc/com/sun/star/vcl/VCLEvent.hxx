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

#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENT_HXX
#define	_SV_COM_SUN_STAR_VCL_VCLEVENT_HXX

#include <java/lang/Object.hxx>
#include <sal/types.h>
#include <tools/gen.hxx>

#ifndef USE_NATIVE_EVENTS

class JavaSalFrame;

namespace vcl {

class SAL_DLLPRIVATE com_sun_star_vcl_VCLEvent: public java_lang_Object
{
protected:
	static jclass		theClass;

public:
	static jclass		getMyClass();

						com_sun_star_vcl_VCLEvent( jobject myObj ) : java_lang_Object( myObj ) {};
						com_sun_star_vcl_VCLEvent( USHORT nID, const JavaSalFrame *pFrame, void *pData );
						com_sun_star_vcl_VCLEvent( USHORT nID, const JavaSalFrame *pFrame, void *pData, const ::rtl::OString &rPath );
	virtual				~com_sun_star_vcl_VCLEvent() {};

	void				cancelShutdown();
	ULONG				getCommittedCharacterCount();
	ULONG				getCursorPosition();
	void*				getData();
	JavaSalFrame*		getFrame();
	USHORT				getKeyChar();
	USHORT				getKeyCode();
	USHORT				getID();
	USHORT				getModifiers();
	com_sun_star_vcl_VCLEvent*	getNextOriginalKeyEvent();
	::rtl::OUString		getPath();
	USHORT				getRepeatCount();
	::rtl::OUString		getText();
	USHORT*				getTextAttributes();
	const Rectangle		getUpdateRect();
	ULONG				getWhen();
	long				getX();
	long				getY();
	short				getMenuID();
	void*				getMenuCookie();
	long				getScrollAmount();
	long				getWheelRotation();
	sal_Bool			isHorizontal();
	sal_Bool			isShutdownCancelled();
};

} // namespace vcl

#endif	// !USE_NATIVE_EVENTS

#endif // _SV_COM_SUN_STAR_VCL_VCLEVENT_HXX
