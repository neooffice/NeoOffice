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

#define _JAVA_DTRANS_JAVA_LANG_CLASS_CXX

#ifndef _JAVA_DTRANS_JAVA_LANG_CLASS_HXX
#include <java/lang/Class.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

using namespace java::dtrans;

// ============================================================================

jclass java_lang_Class::theClass = NULL;

// ----------------------------------------------------------------------------

jclass java_lang_Class::getMyClass()
{
	if ( !theClass )
	{
		DTransThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "java/lang/Class" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

java_lang_Class *java_lang_Class::forName( const ::rtl::OUString& _par0 )
{
	jobject out = NULL;
	DTransThreadAttach t;
	if ( t.pEnv )
	{
		::rtl::OString sClassName = ::rtl::OUStringToOString( _par0, RTL_TEXTENCODING_ASCII_US );
		sClassName = sClassName.replace( '.', '/' );
		out = t.pEnv->FindClass( sClassName );
		if ( t.pEnv->ExceptionCheck() )
		{
			t.pEnv->ExceptionClear();
			out = NULL;
		}
	}
	return ( out ? new java_lang_Class( out ) : NULL );
}

// ----------------------------------------------------------------------------

sal_Bool java_lang_Class::isAssignableFrom( java_lang_Class *_par0 )
{
	static jmethodID mID = NULL;
	jboolean out;
	DTransThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Ljava/lang/Class;)Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "isAssignableFrom", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].l = _par0->getJavaObject();
			out = t.pEnv->CallBooleanMethodA( object, mID, args );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

java_lang_Object *java_lang_Class::newInstance()
{
	static jmethodID mID = NULL;
	jobject out = NULL;
	DTransThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/lang/Object;";
			mID = t.pEnv->GetMethodID( getMyClass(), "newInstance", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			out = t.pEnv->CallObjectMethod( object, mID );
		}
	}
	return out == NULL ? NULL : new java_lang_Object( out );
}

// ----------------------------------------------------------------------------

jobject java_lang_Class::newInstanceObject()
{
	static jmethodID mID = NULL;
	jobject out = NULL;
	DTransThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/lang/Object;";
			mID = t.pEnv->GetMethodID( getMyClass(), "newInstance", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			out = t.pEnv->CallObjectMethod( object, mID );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

::rtl::OUString java_lang_Class::getName()
{
	static jmethodID mID = NULL;
	jstring out = (jstring)NULL;
	::rtl::OUString aStr;
	DTransThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/lang/String;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getName", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			out = (jstring)t.pEnv->CallObjectMethod( object, mID );
			if ( out )
				aStr = JavaString2String( t.pEnv, (jstring)out );
		}
	}
	return aStr;
}
