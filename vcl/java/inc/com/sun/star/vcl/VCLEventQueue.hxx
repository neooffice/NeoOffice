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

#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENTQUEUE_HXX
#define	_SV_COM_SUN_STAR_VCL_VCLEVENTQUEUE_HXX

#include <salframe.h>
#include <java/lang/Object.hxx>
#include <sal/types.h>
#include <vcl/menu.hxx>

#ifndef USE_NATIVE_EVENTS

namespace vcl {

class com_sun_star_vcl_VCLEvent;
class com_sun_star_vcl_VCLFrame;

class SAL_DLLPRIVATE com_sun_star_vcl_VCLEventQueue : public java_lang_Object
{
protected:
	static jclass		theClass;

public:
	static jclass		getMyClass();
	static sal_Bool		postCommandEvent( jobject _par0, short _par1, sal_Bool _par2, sal_Bool _par3, sal_Bool _par4, sal_Bool _par5, jchar _par6, sal_Bool _par7, sal_Bool _par8, sal_Bool _par9, sal_Bool _par10 );
	static void			postMouseWheelEvent( jobject _par0, long _par1, long _par2, long _par3, long _par4, sal_Bool _par5, sal_Bool _par6, sal_Bool _par7, sal_Bool _par8 );
#ifdef USE_NATIVE_WINDOW
	static void			postMenuItemSelectedEvent( JavaSalFrame *pFrame, USHORT nID, Menu *pMenu );
#endif	// USE_NATIVE_WINDOW
	static void			postWindowMoveSessionEvent( jobject _par0, long _par1, long _par2, sal_Bool _par3 );

						com_sun_star_vcl_VCLEventQueue( jobject myObj );
	virtual				~com_sun_star_vcl_VCLEventQueue() {};

	sal_Bool			anyCachedEvent( USHORT _par0 );
	void				dispatchNextEvent();
	com_sun_star_vcl_VCLEvent*	getNextCachedEvent( ULONG _par0, sal_Bool _par1 );
	sal_Bool			isShutdownDisabled();
	void				postCachedEvent( const com_sun_star_vcl_VCLEvent *_par0 );
	void				removeCachedEvents( const JavaSalFrame *_par0 );
	void				setShutdownDisabled( sal_Bool _par0 );
};

} // namespace vcl

#endif	// !USE_NATIVE_EVENTS

#endif // _SV_COM_SUN_STAR_VCL_VCLEVENTQUEUE_HXX
