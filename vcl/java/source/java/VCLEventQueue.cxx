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

#define _SV_COM_SUN_STAR_VCL_VCLEVENTQUEUE_CXX

#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENTQUEUE_HXX
#include <com/sun/star/vcl/VCLEventQueue.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENT_HXX
#include <com/sun/star/vcl/VCLEvent.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFRAME_HXX
#include <com/sun/star/vcl/VCLFrame.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#include "VCLEventQueue_cocoa.h"
#include "../app/salinst_cocoa.h"

using namespace vcl;
using namespace vos;

// ============================================================================

JNIEXPORT jboolean JNICALL Java_com_sun_star_vcl_VCLEventQueue_isNativeModalWindowShowing( JNIEnv *pEnv, jobject object )
{
	return ( NSApplication_getModalWindow() ? JNI_TRUE : JNI_FALSE );
}

// ============================================================================

jclass com_sun_star_vcl_VCLEventQueue::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_vcl_VCLEventQueue::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;

		// Determine if fixes for Java 1.4.x and early versions of Java 1.5
		// are needed
		BOOL bUseKeyEntryFix = FALSE;
		jclass systemClass = t.pEnv->FindClass( "java/lang/System" );
		if ( systemClass )
		{
			jmethodID mID = NULL;
			OUString aJavaHomePath;
			if ( !mID )
			{
				char *cSignature = "(Ljava/lang/String;)Ljava/lang/String;";
				mID = t.pEnv->GetStaticMethodID( systemClass, "getProperty", cSignature );
			}
			OSL_ENSURE( mID, "Unknown method id!" );
			if ( mID )
			{
				jvalue args[1];
				args[0].l = StringToJavaString( t.pEnv, OUString::createFromAscii( "java.specification.version" ) );
				jstring tempJavaSpecVersion = (jstring)t.pEnv->CallStaticObjectMethodA( systemClass, mID, args );
				if ( tempJavaSpecVersion )
				{
					OUString aJavaSpecVersion = JavaString2String( t.pEnv, tempJavaSpecVersion );
					if ( aJavaSpecVersion.getLength() )
					{
						if ( aJavaSpecVersion == OUString::createFromAscii( "1.4" ) )
						{
							bUseKeyEntryFix = TRUE;
						}
						else if ( aJavaSpecVersion == OUString::createFromAscii( "1.5" ) )
						{
							args[0].l = StringToJavaString( t.pEnv, OUString::createFromAscii( "java.version" ) );
							jstring tempJavaVersion = (jstring)t.pEnv->CallStaticObjectMethodA( systemClass, mID, args );
							if ( tempJavaVersion )
							{
								OUString aJavaVersion = JavaString2String( t.pEnv, tempJavaVersion );
								if ( aJavaVersion.compareTo( OUString::createFromAscii( "1.5.0_06" ) ) < 0 )
									bUseKeyEntryFix = TRUE;
							}
						}
					}
				}
			}
		}

		// Fix bugs 1390 and 1619 by inserting our own Cocoa class in place of
		// the NSView class. We need to do this because the JVM does not
		// properly handle key events where a single key press generates more
		// than one Unicode character.
        VCLEventQueue_installVCLEventQueueClasses(bUseKeyEntryFix);

		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLEventQueue" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );

		if ( tempClass )
		{
			// Register the native methods for our class
			JNINativeMethod aMethod; 
			aMethod.name = "isNativeModalWindowShowing";
			aMethod.signature = "()Z";
			aMethod.fnPtr = (void *)Java_com_sun_star_vcl_VCLEventQueue_isNativeModalWindowShowing;
			t.pEnv->RegisterNatives( tempClass, &aMethod, 1 );
		}

		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLEventQueue::com_sun_star_vcl_VCLEventQueue( jobject myObj ) : java_lang_Object( (jobject)NULL )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( !t.pEnv )
		return;
	if ( !mID )
	{
		char *cSignature = "(Z)V";
		mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
	}
	OSL_ENSURE( mID, "Unknown method id!" );

	jvalue args[1];
	args[0].z = jboolean( IsRunningPanther() );
	jobject tempObj = t.pEnv->NewObjectA( getMyClass(), mID, args );
	saveRef( tempObj );
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLEventQueue::anyCachedEvent( USHORT _par0 )
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(I)Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "anyCachedEvent", cSignature );	
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].i = jint( _par0 );
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethodA( object, getMyClass(), mID, args );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLEvent *com_sun_star_vcl_VCLEventQueue::getNextCachedEvent( ULONG _par0, sal_Bool _par1 )
{
	static jmethodID mID = NULL;
	com_sun_star_vcl_VCLEvent *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(JZ)Lcom/sun/star/vcl/VCLEvent;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getNextCachedEvent", cSignature );	
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[2];
			args[0].j = jlong( _par0 );
			args[1].z = jboolean( _par1 );
			jobject tempObj;
			tempObj = t.pEnv->CallNonvirtualObjectMethodA( object, getMyClass(), mID, args );
			if ( tempObj )
				out = new com_sun_star_vcl_VCLEvent( tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLEventQueue::postCachedEvent( const com_sun_star_vcl_VCLEvent *_par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Lcom/sun/star/vcl/VCLEvent;)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "postCachedEvent", cSignature );	
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].l = _par0->getJavaObject();
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLEventQueue::setShutdownDisabled( sal_Bool _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Z)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setShutdownDisabled", cSignature );	
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].z = jboolean( _par0 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}
