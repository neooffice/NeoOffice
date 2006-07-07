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

#ifndef _SV_JAVA_TOOLS_HXX
#define _SV_JAVA_TOOLS_HXX

#ifndef JNI_H
#include <jni.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

namespace vcl {

	jstring					StringToJavaString( JNIEnv *pEnv,const ::rtl::OUString& _Temp );
	::rtl::OUString			JavaString2String( JNIEnv *pEnv,jstring _Str );

	    template< class T, class JT > ::com::sun::star::uno::Sequence< T > copyArrayAndDelete( JNIEnv *pEnv, jobjectArray _Array, const T& _rD1, const JT& _rD2 )
	{
		::com::sun::star::uno::Sequence< T > xOut;
		if ( _Array )
		{
			jsize nLen = pEnv->GetArrayLength( _Array );
			xOut.realloc( nLen );
			for ( jsize i = 0; i < nLen; ++i )
			{
				JT xInfo( pEnv, pEnv->GetObjectArrayElement( _Array, i ) );
				xOut.getArray()[i] = xInfo;
			}
			pEnv->DeleteLocalRef( _Array );
		}
		return xOut;
	}

#ifdef GENESIS_OF_THE_NEW_WEAPONS
	bool					IsRunningPanther( );
#endif

} // namespace vcl

#endif // _SV_JAVA_TOOLS_HXX
