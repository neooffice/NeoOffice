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

#define _JAVA_DTRANS_JAVA_LANG_THROWABLE_CXX

#ifndef _JAVA_DTRANS_JAVA_LANG_THROWABLE_HXX
#include <java/lang/Throwable.hxx>
#endif

using namespace java::dtrans;

// ============================================================================

jclass java_lang_Throwable::theClass = NULL;

// ----------------------------------------------------------------------------

jclass java_lang_Throwable::getMyClass()
{
	if ( !theClass )
	{
		DTransThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "java/lang/Throwable" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

::rtl::OUString java_lang_Throwable::getMessage() const
{
	static jmethodID mID = NULL;
	::rtl::OUString aStr;
	DTransThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/lang/String;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getMessage", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID );
			if (out)
				aStr = JavaString2String( t.pEnv, out );
		}
	}
	return aStr;
}

// ----------------------------------------------------------------------------

::rtl::OUString java_lang_Throwable::getLocalizedMessage() const
{
	static jmethodID mID = NULL;
	::rtl::OUString aStr;
	DTransThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/lang/String;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getLocalizedMessage", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID );
			if (out)
				aStr = JavaString2String( t.pEnv, out );
		}
	}
	return aStr;
}

// ----------------------------------------------------------------------------

::rtl::OUString java_lang_Throwable::toString() const
{
	static jmethodID mID = NULL;
	::rtl::OUString aStr;
	DTransThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/lang/String;";
			mID = t.pEnv->GetMethodID( getMyClass(), "toString", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jstring out = (jstring)t.pEnv->CallObjectMethod( object, mID );
			if (out)
				aStr = JavaString2String( t.pEnv, out );
		}
	}
	return aStr;
}
