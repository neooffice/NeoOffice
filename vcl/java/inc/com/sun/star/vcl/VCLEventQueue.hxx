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

#ifndef _SV_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif

class SalFrame;

namespace vcl {

class com_sun_star_vcl_VCLEvent;
class com_sun_star_vcl_VCLFrame;

class com_sun_star_vcl_VCLEventQueue : public java_lang_Object
{
protected:
	static jclass		theClass;

public:
	static jclass		getMyClass();

						com_sun_star_vcl_VCLEventQueue( jobject myObj );
	virtual				~com_sun_star_vcl_VCLEventQueue() {};

	sal_Bool			anyCachedEvent( USHORT _par0 );
	void				dispatchNextEvent();
	com_sun_star_vcl_VCLEvent*	getNextCachedEvent( ULONG _par0, sal_Bool _par1 );
	void				postCachedEvent( const com_sun_star_vcl_VCLEvent *_par0 );
	void				setShutdownDisabled( sal_Bool _par0 );
};

} // namespace vcl

#endif // _SV_COM_SUN_STAR_VCL_VCLEVENTQUEUE_HXX
