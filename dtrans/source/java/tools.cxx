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

#define _DTRANS_JAVA_TOOLS_CXX

#ifndef _DTRANS_JAVA_TOOLS_HXX
#include <java/tools.hxx>
#endif
#ifndef _JAVA_DTRANS_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif

using namespace java::dtrans;

// ============================================================================

jstring java::dtrans::StringToJavaString( JNIEnv *pEnv, const ::rtl::OUString& _rTemp )
{
	jstring pStr = NULL;
	if ( pEnv )
	{
		pStr = pEnv->NewString( _rTemp.getStr(), _rTemp.getLength() );
		pEnv->ExceptionClear();
		OSL_ENSURE( pStr, "Could not create a jstring object!" );
	}
	return pStr;
}

// ----------------------------------------------------------------------------

::rtl::OUString java::dtrans::JavaString2String( JNIEnv *pEnv, jstring _Str )
{
	::rtl::OUString aStr;
	if ( _Str )
	{
		jboolean bCopy( sal_True );
		const jchar *pChar = pEnv->GetStringChars( _Str, &bCopy );
		jsize len = pEnv->GetStringLength( _Str );
		aStr = ::rtl::OUString( pChar, len );
		pEnv->ReleaseStringChars( _Str, pChar );
	}
	return aStr;
}

// ----------------------------------------------------------------------------

jboolean java::dtrans::AcquireJavaLock()
{
	jboolean out = 1;

#ifdef MACOSX
	// Lock the Carbon lock
	static jclass carbonLockClass = NULL;
	static jmethodID mIDAcquire0 = NULL;
	DTransThreadAttach t; 
	if ( t.pEnv && t.pEnv->GetVersion() < JNI_VERSION_1_4 )
	{
		carbonLockClass = t.pEnv->FindClass( "com/apple/mrj/macos/carbon/CarbonLock" );
		if ( carbonLockClass )
		{
			if ( !mIDAcquire0 )
			{
				char *cSignature = "()I";
				mIDAcquire0 = t.pEnv->GetStaticMethodID( carbonLockClass, "acquire0", cSignature );
			}
			OSL_ENSURE( mIDAcquire0, "Unknown method id!" );
			if ( mIDAcquire0 )
				out = ( t.pEnv->CallStaticIntMethod( carbonLockClass, mIDAcquire0 ) == 0 );
		}
	}
#endif  // MACOSX

	return out;
}

// ----------------------------------------------------------------------------

jboolean java::dtrans::ReleaseJavaLock()
{
	jboolean out = false;

#ifdef MACOSX
	// Unlock the Carbon lock
	static jclass carbonLockClass = NULL;
	static jmethodID mIDRelease0 = NULL;
	DTransThreadAttach t; 
	if ( t.pEnv && t.pEnv->GetVersion() < JNI_VERSION_1_4 )
	{
		carbonLockClass = t.pEnv->FindClass( "com/apple/mrj/macos/carbon/CarbonLock" );
		if ( carbonLockClass )
		{
			if ( !mIDRelease0 )
			{
				char *cSignature = "()I";
				mIDRelease0 = t.pEnv->GetStaticMethodID( carbonLockClass, "release0", cSignature );
			}
			OSL_ENSURE( mIDRelease0, "Unknown method id!" );
			if ( mIDRelease0 )
				out = ( t.pEnv->CallStaticIntMethod( carbonLockClass, mIDRelease0 ) == 0 );
		}
	}
#endif  // MACOSX

	return out;
}
