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

#define _JAVA_DTRANS_JAVA_LANG_STRING_CXX

#ifndef _JAVA_DTRANS_JAVA_LANG_STRING_HXX
#include <java/lang/String.hxx>
#endif

using namespace java::dtrans;

// ============================================================================

jclass java_lang_String::theClass = NULL;

// ----------------------------------------------------------------------------

jclass java_lang_String::getMyClass()
{
	if ( !theClass )
	{
		DTransThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "java/lang/String" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

java_lang_String::java_lang_String( const ::rtl::OUString& _par0 ) : java_lang_Object( (jobject)NULL )
{
	static jmethodID mID = NULL;
	DTransThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Ljava/lang/String;)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].l = StringToJavaString( t.pEnv, _par0 );
			jobject tempObj;
			tempObj = t.pEnv->NewObjectA( getMyClass(), mID, args );
			saveRef( tempObj );
		}
	}
}

// ----------------------------------------------------------------------------

java_lang_String::operator ::rtl::OUString()
{
	DTransThreadAttach t;
	if ( t.pEnv )
		return JavaString2String( t.pEnv, (jstring)object );
	else
		return ::rtl::OUString();
}
