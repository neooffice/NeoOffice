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

#ifndef _SV_JAVA_TOOLS_HXX
#define _SV_JAVA_TOOLS_HXX

#include <jni.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <sal/types.h>

namespace vcl {

	SAL_DLLPRIVATE jstring		StringToJavaString( JNIEnv *pEnv,const ::rtl::OUString& _Temp );
	SAL_DLLPRIVATE ::rtl::OUString	JavaString2String( JNIEnv *pEnv,jstring _Str );

	SAL_DLLPRIVATE bool			IsRunningLeopard( );
	SAL_DLLPRIVATE bool			IsRunningSnowLeopard( );
	SAL_DLLPRIVATE bool			IsRunningLion( );
	SAL_DLLPRIVATE bool			IsRunningMountainLion( );

	SAL_DLLPRIVATE bool			IsFullKeyboardAccessEnabled( );

} // namespace vcl

#endif // _SV_JAVA_TOOLS_HXX
