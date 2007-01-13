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

#define _SV_COM_SUN_STAR_VCL_VCLFRAME_CXX

#ifndef _SV_COM_SUN_STAR_VCL_VCLFRAME_HXX
#include <com/sun/star/vcl/VCLFrame.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENT_HXX
#include <com/sun/star/vcl/VCLEvent.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
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

#include "VCLFrame_cocoa.h"

using namespace vcl;
using namespace vos;

// ============================================================================

static jobject JNICALL Java_com_sun_star_vcl_VCLFrame_getTextLocation0( JNIEnv *pEnv, jobject object, jlong _par0 )
{
	jobject out = NULL;

	IMutex& rSolarMutex = Application::GetSolarMutex();
	rSolarMutex.acquire();

	static jmethodID mID = NULL;

	jclass pointClass = pEnv->FindClass( "java/awt/Rectangle" );
	if ( pointClass )
	{
		if ( !mID )
		{
			char *cSignature = "(IIII)V";
			mID = pEnv->GetMethodID( pointClass, "<init>", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			SalExtTextInputPosEvent aInputPosEvent;
			memset( &aInputPosEvent, 0, sizeof( SalExtTextInputPosEvent ) );
			com_sun_star_vcl_VCLEvent aEvent( SALEVENT_EXTTEXTINPUTPOS, (JavaSalFrame *)_par0, &aInputPosEvent );
			aEvent.dispatch();

			jvalue args[4];
			if ( aInputPosEvent.mbVertical )
			{
				args[0].i = jint( aInputPosEvent.mnX + aInputPosEvent.mnWidth + 25 );
				args[1].i = jint( aInputPosEvent.mnY - aInputPosEvent.mnHeight + 15 );
			}
			else
			{
				args[0].i = jint( aInputPosEvent.mnX + aInputPosEvent.mnWidth );
				args[1].i = jint( aInputPosEvent.mnY + 20 );
			}
			args[2].i = jint( aInputPosEvent.mnWidth );
			args[3].i = jint( aInputPosEvent.mnHeight );
			out = pEnv->NewObjectA( pointClass, mID, args );
		}
	}

	rSolarMutex.release();

	return out;
}

// ----------------------------------------------------------------------------

static void JNICALL Java_com_sun_star_vcl_VCLFrame_updateLocation( JNIEnv *pEnv, jobject object, jobject _par0 )
{
	if ( _par0 )
	{
		jclass tempClass = pEnv->FindClass( "apple/awt/ComponentModel" );
		if ( tempClass && pEnv->IsInstanceOf( _par0, tempClass ) )
		{
			static jmethodID mIDGetModelPtr = NULL;
			static bool bReturnsInt = false;
			if ( !mIDGetModelPtr )
			{
				char *cSignature = "()J";
				mIDGetModelPtr = pEnv->GetMethodID( tempClass, "getModelPtr", cSignature );
				if ( !mIDGetModelPtr )
				{
					// Java 1.4.1 has a different signature so check
					// for it if we cannot find the first signature
					if ( pEnv->ExceptionCheck() )
						pEnv->ExceptionClear();
					cSignature = "()I";
					mIDGetModelPtr = pEnv->GetMethodID( tempClass, "getModelPtr", cSignature );
					if ( mIDGetModelPtr )
						bReturnsInt = true;
				}
			}
			OSL_ENSURE( mIDGetModelPtr, "Unknown method id!" );
			if ( mIDGetModelPtr )
			{
				if ( bReturnsInt )
					CWindow_updateLocation( (void *) pEnv->CallIntMethod( _par0, mIDGetModelPtr ) );
				else
					CWindow_updateLocation( (void *) pEnv->CallLongMethod( _par0, mIDGetModelPtr ) );
			}
		}
	}
}

// ============================================================================

jclass com_sun_star_vcl_VCLFrame::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_vcl_VCLFrame::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLFrame" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );

		if ( tempClass )
		{
			// Register the native methods for our class
			JNINativeMethod pMethods[2];
			pMethods[0].name = "getTextLocation0";
			pMethods[0].signature = "(J)Ljava/awt/Rectangle;";
			pMethods[0].fnPtr = (void *)Java_com_sun_star_vcl_VCLFrame_getTextLocation0;
			pMethods[1].name = "updateLocation";
			pMethods[1].signature = "(Ljava/awt/peer/ComponentPeer;)V";
			pMethods[1].fnPtr = (void *)Java_com_sun_star_vcl_VCLFrame_updateLocation;
			t.pEnv->RegisterNatives( tempClass, pMethods, 2 );
		}

		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLFrame::com_sun_star_vcl_VCLFrame( ULONG nSalFrameStyle, const JavaSalFrame *pFrame, const JavaSalFrame *pParent ) : java_lang_Object( (jobject)NULL )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( !t.pEnv )
		return;
	if ( !mID )
	{
		char *cSignature = "(JLcom/sun/star/vcl/VCLEventQueue;JLcom/sun/star/vcl/VCLFrame;)V";
		mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
	}
	OSL_ENSURE( mID, "Unknown method id!" );

	jvalue args[4];
	args[0].j = jlong( nSalFrameStyle );
	args[1].l = GetSalData()->mpEventQueue->getJavaObject();
	args[2].j = jlong( pFrame );
	if ( pParent )
		args[3].l = pParent->mpVCLFrame->getJavaObject();
	else
		args[3].l = NULL;
	jobject tempObj;
	tempObj = t.pEnv->NewObjectA( getMyClass(), mID, args );
	saveRef( tempObj );
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFrame::addChild( JavaSalFrame *_par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Lcom/sun/star/vcl/VCLFrame;)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "addChild", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].l = _par0->mpVCLFrame->getJavaObject();
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFrame::dispose()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetMethodID( getMyClass(), "dispose", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );

		if ( mID )
			t.pEnv->CallNonvirtualVoidMethod( object, getMyClass(), mID );
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFrame::enableFlushing( sal_Bool _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Z)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "enableFlushing", cSignature );
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

// ----------------------------------------------------------------------------

const Rectangle com_sun_star_vcl_VCLFrame::getBounds()
{
	static jmethodID mID = NULL;
	static jfieldID fIDX = NULL;
	static jfieldID fIDY = NULL;
	static jfieldID fIDWidth = NULL;
	static jfieldID fIDHeight = NULL;
	Rectangle out( Point( 0, 0 ), Size( 0, 0 ) );
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/awt/Rectangle;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getBounds", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
			{
				jclass tempObjClass = t.pEnv->GetObjectClass( tempObj );
				if ( !fIDX )
				{
					char *cSignature = "I";
					fIDX = t.pEnv->GetFieldID( tempObjClass, "x", cSignature );
				}
				OSL_ENSURE( fIDX, "Unknown field id!" );
				if ( !fIDY )
				{
					char *cSignature = "I";
					fIDY = t.pEnv->GetFieldID( tempObjClass, "y", cSignature );
				}
				OSL_ENSURE( fIDY, "Unknown field id!" );
				if ( !fIDWidth )
				{
					char *cSignature = "I";
					fIDWidth = t.pEnv->GetFieldID( tempObjClass, "width", cSignature );
				}
				OSL_ENSURE( fIDWidth, "Unknown field id!" );
				if ( !fIDHeight )
				{
					char *cSignature = "I";
					fIDHeight = t.pEnv->GetFieldID( tempObjClass, "height", cSignature );
				}
				OSL_ENSURE( fIDHeight, "Unknown field id!" );
				if ( fIDX && fIDY && fIDWidth && fIDHeight )
				{
					Point aPoint( (long)t.pEnv->GetIntField( tempObj, fIDX ), (long)t.pEnv->GetIntField( tempObj, fIDY ) );
					Size aSize( (long)t.pEnv->GetIntField( tempObj, fIDWidth ), (long)t.pEnv->GetIntField( tempObj, fIDHeight ) );
					out = Rectangle( aPoint, aSize );
				}
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLGraphics *com_sun_star_vcl_VCLFrame::getGraphics()
{
	static jmethodID mID = NULL;
	com_sun_star_vcl_VCLGraphics *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Lcom/sun/star/vcl/VCLGraphics;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getGraphics", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
				out = new com_sun_star_vcl_VCLGraphics( tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

const Rectangle com_sun_star_vcl_VCLFrame::getInsets()
{
	static jmethodID mID = NULL;
	static jfieldID fIDLeft = NULL;
	static jfieldID fIDTop = NULL;
	static jfieldID fIDRight = NULL;
	static jfieldID fIDBottom = NULL;
	Rectangle out( 0, 0, 0, 0 );
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/awt/Insets;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getInsets", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
			{
				jclass tempObjClass = t.pEnv->GetObjectClass( tempObj );
				if ( !fIDLeft )
				{
					char *cSignature = "I";
					fIDLeft = t.pEnv->GetFieldID( tempObjClass, "left", cSignature );
				}
				OSL_ENSURE( fIDLeft, "Unknown field id!" );
				if ( !fIDTop )
				{
					char *cSignature = "I";
					fIDTop  = t.pEnv->GetFieldID( tempObjClass, "top", cSignature );
				}
				OSL_ENSURE( fIDTop, "Unknown field id!" );
				if ( !fIDRight )
				{
					char *cSignature = "I";
					fIDRight = t.pEnv->GetFieldID( tempObjClass, "right", cSignature );
				}
				OSL_ENSURE( fIDRight, "Unknown field id!" );
				if ( !fIDBottom )
				{
					char *cSignature = "I";
					fIDBottom = t.pEnv->GetFieldID( tempObjClass, "bottom", cSignature );
				}
				OSL_ENSURE( fIDBottom, "Unknown field id!" );
				if ( fIDLeft && fIDTop && fIDRight && fIDBottom )
				{
					out = Rectangle( (long)t.pEnv->GetIntField( tempObj, fIDLeft ), (long)t.pEnv->GetIntField( tempObj, fIDTop ), (long)t.pEnv->GetIntField( tempObj, fIDRight ), (long)t.pEnv->GetIntField( tempObj, fIDBottom ) );
				}
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

::rtl::OUString com_sun_star_vcl_VCLFrame::getKeyName( USHORT _par0 )
{
	static jmethodID mID = NULL;
	::rtl::OUString out;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(I)Ljava/lang/String;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getKeyName", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].i = jint( _par0 );
			jstring tempObj = (jstring)t.pEnv->CallNonvirtualObjectMethodA( object, getMyClass(), mID, args );
			if ( tempObj )
				out = JavaString2String( t.pEnv, tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

void *com_sun_star_vcl_VCLFrame::getNativeWindow()
{
	void *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		java_lang_Object *peer = getPeer();
		if ( peer )
		{
			jobject tempObj = peer->getJavaObject();
			if ( tempObj )
			{
				jclass tempClass = t.pEnv->FindClass( "apple/awt/ComponentModel" );
				if ( tempClass && t.pEnv->IsInstanceOf( tempObj, tempClass ) )
				{
					static jmethodID mIDGetModelPtr = NULL;
					static bool bReturnsInt = false;
					if ( !mIDGetModelPtr )
					{
						char *cSignature = "()J";
						mIDGetModelPtr = t.pEnv->GetMethodID( tempClass, "getModelPtr", cSignature );
						if ( !mIDGetModelPtr )
						{
							// Java 1.4.1 has a different signature so check
							// for it if we cannot find the first signature
							if ( t.pEnv->ExceptionCheck() )
								t.pEnv->ExceptionClear();
							cSignature = "()I";
							mIDGetModelPtr = t.pEnv->GetMethodID( tempClass, "getModelPtr", cSignature );
							if ( mIDGetModelPtr )
								bReturnsInt = true;
						}
					}
					OSL_ENSURE( mIDGetModelPtr, "Unknown method id!" );
					if ( mIDGetModelPtr )
					{
						if ( bReturnsInt )
							out = (void *)CWindow_getNSWindow( (void *) t.pEnv->CallIntMethod( tempObj, mIDGetModelPtr ) );
						else
							out = (void *)CWindow_getNSWindow( (void *) t.pEnv->CallLongMethod( tempObj, mIDGetModelPtr ) );
					}
				}
			}
			delete peer;
		}
	}

	return out;
}

// ----------------------------------------------------------------------------

void *com_sun_star_vcl_VCLFrame::getNativeWindowRef()
{
	void *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		java_lang_Object *peer = getPeer();
		if ( peer )
		{
			jobject tempObj = peer->getJavaObject();
			if ( tempObj )
			{
				jclass tempClass = t.pEnv->FindClass( "apple/awt/ComponentModel" );
				if ( tempClass && t.pEnv->IsInstanceOf( tempObj, tempClass ) )
				{
					static jmethodID mIDGetModelPtr = NULL;
					static bool bReturnsInt = false;
					if ( !mIDGetModelPtr )
					{
						char *cSignature = "()J";
						mIDGetModelPtr = t.pEnv->GetMethodID( tempClass, "getModelPtr", cSignature );
						if ( !mIDGetModelPtr )
						{
							// Java 1.4.1 has a different signature so check
							// for it if we cannot find the first signature
							if ( t.pEnv->ExceptionCheck() )
								t.pEnv->ExceptionClear();
							cSignature = "()I";
							mIDGetModelPtr = t.pEnv->GetMethodID( tempClass, "getModelPtr", cSignature );
							if ( mIDGetModelPtr )
								bReturnsInt = true;
						}
					}
					OSL_ENSURE( mIDGetModelPtr, "Unknown method id!" );
					if ( mIDGetModelPtr )
					{
						if ( bReturnsInt )
							out = (void *)CWindow_getWindowRef( (void *)t.pEnv->CallIntMethod( tempObj, mIDGetModelPtr ) );
						else
							out = (void *)CWindow_getWindowRef( (void *)t.pEnv->CallLongMethod( tempObj, mIDGetModelPtr ) );
					}
				}
			}
			delete peer;
		}
	}

	return out;
}

// ----------------------------------------------------------------------------

java_lang_Object *com_sun_star_vcl_VCLFrame::getPeer()
{
	static jmethodID mID = NULL;
	java_lang_Object *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/awt/peer/ComponentPeer;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getPeer", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
				out = new java_lang_Object( tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

ULONG com_sun_star_vcl_VCLFrame::getState()
{
	static jmethodID mID = NULL;
	ULONG out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()J";
			mID = t.pEnv->GetMethodID( getMyClass(), "getState", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (ULONG)t.pEnv->CallNonvirtualLongMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFrame::removeChild( JavaSalFrame *_par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Lcom/sun/star/vcl/VCLFrame;)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "removeChild", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].l = _par0->mpVCLFrame->getJavaObject();
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLFrame::requestFocus()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "requestFocus", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFrame::setBounds( long _par0, long _par1, long _par2, long _par3 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(IIII)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setBounds", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[4];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFrame::setFullScreenMode( sal_Bool _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Z)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setFullScreenMode", cSignature );
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

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFrame::setMinClientSize( long _par0, long _par1 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(II)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setMinClientSize", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[2];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFrame::setPointer( USHORT _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(I)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setPointer", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].i = jint( _par0 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFrame::setState( ULONG _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(J)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setState", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].j = jlong( _par0 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFrame::setTitle( ::rtl::OUString _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Ljava/lang/String;)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setTitle", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].l = StringToJavaString( t.pEnv, _par0 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFrame::setVisible( sal_Bool _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Z)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setVisible", cSignature );
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

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFrame::sync()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetMethodID( getMyClass(), "sync", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			t.pEnv->CallNonvirtualVoidMethod( object, getMyClass(), mID );
	}
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLFrame::toFront()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "toFront", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethod( object, getMyClass(), mID );
	}
	return out;
}
