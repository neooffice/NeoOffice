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

#define _JAVA_DTRANS_JAVA_LANG_OBJECT_CXX

#ifndef _JAVA_DTRANS_JAVA_LANG_CLASS_HXX
#include <java/lang/Class.hxx>
#endif
#ifndef _JAVA_DTRANS_JAVA_LANG_THROWABLE_HXX
#include <java/lang/Throwable.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_JAVA_XJAVATHREADREGISTER_11_HPP_
#include <com/sun/star/java/XJavaThreadRegister_11.hpp>
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
#ifndef _TOOLS_TEMPFILE_HXX
#include <tools/tempfile.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

using namespace java::dtrans;
using namespace osl;
using namespace rtl;
using namespace vos;
using namespace com::sun::star::java;
using namespace com::sun::star::registry;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

// ============================================================================

static JavaVM *pJVM;
static Reference< XJavaThreadRegister_11 > xRG11Ref;

// ----------------------------------------------------------------------------

DTransThreadAttach::DTransThreadAttach()
{
	pEnv = NULL;
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

// ----------------------------------------------------------------------------

DTransThreadAttach::~DTransThreadAttach()
{
	ThrowException();
	if ( pEnv )
		pEnv->PopLocalFrame( NULL );
	DetachThread();
}

// ----------------------------------------------------------------------------

void DTransThreadAttach::AttachThread()
{
	if ( xRG11Ref.is() )
	{
		pJVM->AttachCurrentThread( (void**)&pEnv, NULL );
		xRG11Ref->registerThread();
	}
}

// ----------------------------------------------------------------------------

void DTransThreadAttach::DetachThread()
{
	if ( xRG11Ref.is() )
	{
		xRG11Ref->revokeThread();
		if ( !xRG11Ref->isThreadAttached() )
			pJVM->DetachCurrentThread();
	}
}

// ----------------------------------------------------------------------------

sal_Bool DTransThreadAttach::StartJava()
{
	static sal_Bool bStarted = sal_False;
	if ( !bStarted )
	{
		OGuard aGuard( OMutex::getGlobalMutex() );
		if ( !bStarted )
		{

			Reference<XMultiServiceFactory> xFactory = ::comphelper::getProcessServiceFactory();

			if ( !xFactory.is() )
			{
				TempFile aTempFile;
				OUString aTempFileName;
				FileBase::getSystemPathFromFileURL( aTempFile.GetName(), aTempFileName );
				xFactory = ::cppu::createRegistryServiceFactory( aTempFileName, rtl::OUString(), sal_False );

				Reference < XImplementationRegistration > xReg( xFactory->createInstance( OUString::createFromAscii( "com.sun.star.registry.ImplementationRegistration" ) ), UNO_QUERY );

#ifdef WNT
				OUString aComponentPathString = OUString::createFromAscii( "jen" );
				aComponentPathString += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".dll" ));
#else
				OUString aComponentPathString = OUString( RTL_CONSTASCII_USTRINGPARAM( "libjen" ));
#ifdef MACOSX
				aComponentPathString += OUString( RTL_CONSTASCII_USTRINGPARAM( ".dylib" ));
#else
				aComponentPathString += OUString( RTL_CONSTASCII_USTRINGPARAM( ".so" ));
#endif
#endif
				if ( aComponentPathString.getLength() )
				{
					try
					{
						xReg->registerImplementation( OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ), aComponentPathString, NULL );
					}
					catch( Exception & )
					{
					}
				}
				::comphelper::setProcessServiceFactory( xFactory );
			}

		    JNIEnv *pEnv = NULL;

			Reference< XJavaVM > xVM( xFactory->createInstance( rtl::OUString::createFromAscii( "com.sun.star.java.JavaVirtualMachine") ), UNO_QUERY );

			OSL_ENSURE( xFactory.is(), "DTransThreadAttach::StartJava: invalid factory!" );
			if ( !xVM.is() || !xFactory.is() )
				return sal_False;

			Sequence<sal_Int8> processID(16);
			rtl_getGlobalProcessId( (sal_uInt8*) processID.getArray() );

			Any uaJVM = xVM->getJavaVM( processID );

			if ( uaJVM.hasValue() )
			{
				sal_Int32 nValue;
				uaJVM >>= nValue;
				pJVM = (JavaVM *)nValue;
			}
			else
			{
				return sal_False;
			}

			xRG11Ref = Reference< XJavaThreadRegister_11 >( xVM, UNO_QUERY );
			if ( xRG11Ref.is() )
				xRG11Ref->registerThread();

		  	pJVM->AttachCurrentThread( (void **)&pEnv, NULL );

			if ( pEnv )
			{
				if ( xRG11Ref.is() )
					xRG11Ref->revokeThread();
				
				if ( !xRG11Ref.is() || !xRG11Ref->isThreadAttached() )
					pJVM->DetachCurrentThread();
			}

			bStarted = sal_True;
		}
	}
	return bStarted;
}

// ----------------------------------------------------------------------------

void DTransThreadAttach::ThrowException()
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
		DTransThreadAttach aThreadAttach;
		if ( !aThreadAttach.pEnv ) return (jclass)NULL;
		jclass tempClass = aThreadAttach.pEnv->FindClass( "java/lang/Object" );
		theClass = (jclass)aThreadAttach.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

java_lang_Object::java_lang_Object( jobject myObj ) : object( NULL )
{
	DTransThreadAttach t;
	if ( t.pEnv && myObj )
		saveRef( myObj );
}

// ----------------------------------------------------------------------------

java_lang_Object::~java_lang_Object()
{
	if ( object )
	{
		DTransThreadAttach t;
		if ( t.pEnv )
			t.pEnv->DeleteGlobalRef( object );
	}
}

// ----------------------------------------------------------------------------

void java_lang_Object::saveRef( jobject myObj )
{
	OSL_ENSURE( myObj, "object in c++ -> Java Wrapper" );
	DTransThreadAttach t;
	if ( t.pEnv )
	{
		if ( object )
			t.pEnv->DeleteGlobalRef( object );
		if ( myObj )
			object = t.pEnv->NewGlobalRef( myObj );
	}
}

// ----------------------------------------------------------------------------

java_lang_Class *java_lang_Object::getClass()
{
	static jmethodID mID = NULL;
	jobject out;
	DTransThreadAttach t;
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
	DTransThreadAttach t;
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
