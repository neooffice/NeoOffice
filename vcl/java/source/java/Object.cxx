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

#define _SV_JAVA_LANG_OBJECT_CXX

#ifndef _SV_SALINST_H
#include <salinst.h>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VCL_UNOHELP_HXX
#include <vcl/unohelp.hxx>
#endif
#ifndef _SV_JAVA_LANG_CLASS_HXX
#include <java/lang/Class.hxx>
#endif
#ifndef _SV_JAVA_LANG_THROWABLE_HXX
#include <java/lang/Throwable.hxx>
#endif
#ifndef _COM_SUN_STAR_JAVA_XJAVAVM_HPP_
#include <com/sun/star/java/XJavaVM.hpp>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _RTL_PROCESS_H_
#include <rtl/process.h>
#endif

using namespace vcl;
using namespace vos;
using namespace com::sun::star::java;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

// ============================================================================

static JavaVM *pJVM = NULL;
static Reference< XJavaVM > xVM;

// ----------------------------------------------------------------------------

extern "C" void SAL_DLLPUBLIC_EXPORT DetachCurrentThreadFromJVM()
{
	if ( xVM.is() && pJVM )
		pJVM->DetachCurrentThread();
}

// ----------------------------------------------------------------------------

VCLThreadAttach::VCLThreadAttach()
{
	pEnv = NULL;
	if ( !Application::IsShutDown() )
	{
		StartJava();
		AttachThread();
		if ( pEnv && pEnv->PushLocalFrame( 16 ) < 0 )
		{
			ThrowException();
			DetachThread();
			pEnv = NULL;
		}
		ThrowException();
	}
}

// ----------------------------------------------------------------------------

VCLThreadAttach::~VCLThreadAttach()
{
	ThrowException();
	if ( pEnv )
		pEnv->PopLocalFrame( NULL );
	DetachThread();
}

// ----------------------------------------------------------------------------

void VCLThreadAttach::AttachThread()
{
	if ( !xVM.is() || !pJVM || pJVM->AttachCurrentThread( (void**)&pEnv, NULL ) != JNI_OK )
		pEnv = NULL;
}

// ----------------------------------------------------------------------------

void VCLThreadAttach::DetachThread()
{
}

// ----------------------------------------------------------------------------

sal_Bool VCLThreadAttach::StartJava()
{
	if ( !xVM.is() )
	{
		OGuard aGuard( OMutex::getGlobalMutex() );
		if ( !xVM.is() )
		{
			pJVM = NULL;

			Reference<XMultiServiceFactory> xFactory = unohelper::GetMultiServiceFactory();
			OSL_ENSURE( xFactory.is(), "No XMultiServiceFactory available!" );
			if ( !xFactory.is() )
				return sal_False;

		    JNIEnv *pTmpEnv = NULL;

			xVM = Reference< XJavaVM >( xFactory->createInstance( rtl::OUString::createFromAscii( "com.sun.star.java.JavaVirtualMachine") ), UNO_QUERY );

			OSL_ENSURE( xVM.is(), "VCLThreadAttach::StartJava: invalid java reference!" );
			if ( !xVM.is() || !xFactory.is() )
				return sal_False;

			Sequence<sal_Int8> processID(16);
			rtl_getGlobalProcessId( (sal_uInt8*) processID.getArray() );

			Any uaJVM = xVM->getJavaVM( processID );

			if ( uaJVM.hasValue() )
			{
				sal_Int32 nValue = 0;
				uaJVM >>= nValue;
				pJVM = (JavaVM *)nValue;
			}
			else
			{
				xVM.clear();
				return sal_False;
			}

		  	pJVM->AttachCurrentThread( (void **)&pTmpEnv, NULL );

			if ( pTmpEnv )
				InitJavaAWT();

			return sal_True;
		}
	}

	return sal_True;
}

// ----------------------------------------------------------------------------

void VCLThreadAttach::ThrowException()
{
	if ( pEnv && pEnv->ExceptionCheck() )
	{
		pEnv->ExceptionDescribe();
		pEnv->ExceptionClear();
	}
}

// ============================================================================

jclass java_lang_Object::theClass = NULL;

// ----------------------------------------------------------------------------

jclass java_lang_Object::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach aThreadAttach;
		if ( !aThreadAttach.pEnv ) return (jclass)NULL;
		jclass tempClass = aThreadAttach.pEnv->FindClass( "java/lang/Object" );
		theClass = (jclass)aThreadAttach.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

java_lang_Object::java_lang_Object( jobject myObj ) : object( NULL )
{
	saveRef( myObj );
}

// ----------------------------------------------------------------------------

java_lang_Object::~java_lang_Object()
{
	saveRef( NULL );
}

// ----------------------------------------------------------------------------

void java_lang_Object::saveRef( jobject myObj )
{
//	OSL_ENSURE( myObj, "object in c++ -> Java Wrapper" );
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( object )
			t.pEnv->DeleteGlobalRef( object );
		if ( myObj )
			object = t.pEnv->NewGlobalRef( myObj );
		else
			object = NULL;
	}
}

// ----------------------------------------------------------------------------

java_lang_Class *java_lang_Object::getClass()
{
	static jmethodID mID = NULL;
	jobject out;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/lang/Class;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getClass", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			out = t.pEnv->CallObjectMethod( object, mID );
			return new java_lang_Class( out );
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------------

::rtl::OUString java_lang_Object::toString()
{
	
	static jmethodID mID = NULL;
	::rtl::OUString aStr;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/lang/String;";
			mID = t.pEnv->GetMethodID( getMyClass(), "toString", cSignature );
		}
		OSL_ENSURE( mID,"Unknown method id!" );
		if ( mID )
		{
			jstring out;
			out = (jstring)t.pEnv->CallObjectMethod( object, mID );
			if (out)
				aStr = JavaString2String( t.pEnv, out );
		}
	}
	return aStr;
}
