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

#include <com/sun/star/vcl/VCLEvent.hxx>
#include <com/sun/star/vcl/VCLGraphics.hxx>
#include <saldata.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

#include "VCLFrame_cocoa.h"

using namespace vcl;
using namespace vos;

// ============================================================================

static jobject JNICALL Java_com_sun_star_vcl_VCLFrame_getBounds0( JNIEnv *pEnv, jobject object, jobject _par0, jlong _par1, jboolean _par2 )
{
	jobject out = NULL;

	if ( _par0 )
	{
		static jmethodID mID = NULL;

		jclass rectangleClass = pEnv->FindClass( "java/awt/Rectangle" );
		if ( rectangleClass )
		{
			if ( !mID )
			{
				char *cSignature = "(IIII)V";
				mID = pEnv->GetMethodID( rectangleClass, "<init>", cSignature );
			}
			OSL_ENSURE( mID, "Unknown method id!" );
			if ( mID )
			{
				jclass tempClass = pEnv->FindClass( "apple/awt/ComponentModel" );
				if ( tempClass && pEnv->IsInstanceOf( _par0, tempClass ) )
				{
					static jmethodID mIDGetModelPtr = NULL;
					if ( !mIDGetModelPtr )
					{
						char *cSignature = "()J";
						mIDGetModelPtr = pEnv->GetMethodID( tempClass, "getModelPtr", cSignature );
					}
					OSL_ENSURE( mIDGetModelPtr, "Unknown method id!" );
					if ( mIDGetModelPtr )
					{
						float fX = 0;
						float fY = 0;
						float fWidth = 0;
						float fHeight = 0;
						CWindow_getNSWindowBounds( (void *) pEnv->CallLongMethod( _par0, mIDGetModelPtr ), &fX, &fY, &fWidth, &fHeight, (BOOL *)_par1, _par2 );

						jvalue args[4];
						args[0].i = jint( fX > 0 ? fX + 0.5 : fX );
						args[1].i = jint( fY > 0 ? fY + 0.5 : fY );
						args[2].i = jint( fWidth > 0 ? fWidth + 0.5 : fWidth );
						args[3].i = jint( fHeight > 0 ? fHeight + 0.5 : fHeight );
						out = pEnv->NewObjectA( rectangleClass, mID, args );
					}
				}
			}
		}
	}

	return out;
}

// ----------------------------------------------------------------------------

static jobject JNICALL Java_com_sun_star_vcl_VCLFrame_getTextLocation0( JNIEnv *pEnv, jobject object, jlong _par0 )
{
	jobject out = NULL;

	JavaSalFrame *pFrame = (JavaSalFrame *)_par0;
	if ( !pFrame )
		return out;

	if ( !Application::IsShutDown() )
	{
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
				IMutex& rSolarMutex = Application::GetSolarMutex();
				rSolarMutex.acquire();
				if ( !Application::IsShutDown() )
				{
					SalExtTextInputPosEvent aInputPosEvent;
					memset( &aInputPosEvent, 0, sizeof( SalExtTextInputPosEvent ) );

					// Fix bug 2426 by checking the frame pointer before any use
					SalData *pSalData = GetSalData();
					for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
					{
						if ( *it == pFrame )
						{
							com_sun_star_vcl_VCLEvent aEvent( SALEVENT_EXTTEXTINPUTPOS, pFrame, &aInputPosEvent );
							aEvent.dispatch();
							break;
						}
					}

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

				rSolarMutex.release();
			}
		}
	}

	return out;
}

// ----------------------------------------------------------------------------

static jint JNICALL Java_com_sun_star_vcl_VCLFrame_makeFloatingWindow( JNIEnv *pEnv, jobject object, jobject _par0 )
{
	jint nRet = 0;

	if ( _par0 )
	{
		jclass tempClass = pEnv->FindClass( "apple/awt/ComponentModel" );
		jclass tempClass2 = pEnv->FindClass( "apple/awt/ContainerModel" );
		if ( tempClass && tempClass2 && pEnv->IsInstanceOf( _par0, tempClass ) && pEnv->IsInstanceOf( _par0, tempClass2 ) )
		{
			static jmethodID mIDGetModelPtr = NULL;
			static jfieldID fIDInsets = NULL;
			if ( !mIDGetModelPtr )
			{
				char *cSignature = "()J";
				mIDGetModelPtr = pEnv->GetMethodID( tempClass, "getModelPtr", cSignature );
			}
			OSL_ENSURE( mIDGetModelPtr, "Unknown method id!" );
			if ( !fIDInsets )
			{
				char *cSignature = "Ljava/awt/Insets;";
				fIDInsets = pEnv->GetFieldID( tempClass2, "fInsets", cSignature );
			}
			OSL_ENSURE( fIDInsets, "Unknown field id!" );
			if ( mIDGetModelPtr && fIDInsets )
			{
				jobject tempObj = pEnv->GetObjectField( _par0, fIDInsets );
				if ( tempObj )
				{
					jclass tempObjClass = pEnv->GetObjectClass( tempObj );
					if ( tempObjClass )
					{
						static jfieldID fIDTop = NULL;
						if ( !fIDTop )
						{
							char *cSignature = "I";
							fIDTop = pEnv->GetFieldID( tempObjClass, "top", cSignature );
						}
						OSL_ENSURE( fIDInsets, "Unknown field id!" );
						if ( fIDTop )
						{
							nRet = (jint)CWindow_makeFloatingWindow( (void *) pEnv->CallLongMethod( _par0, mIDGetModelPtr ) );
							if ( nRet > 0 )
								pEnv->SetIntField( tempObj, fIDTop, nRet );
						}
					}
				}
			}
		}
	}

	return nRet;
}

// ----------------------------------------------------------------------------

static void JNICALL Java_com_sun_star_vcl_VCLFrame_makeUnshadowedWindow( JNIEnv *pEnv, jobject object, jobject _par0 )
{
	if ( _par0 )
	{
		jclass tempClass = pEnv->FindClass( "apple/awt/ComponentModel" );
		jclass tempClass2 = pEnv->FindClass( "apple/awt/ContainerModel" );
		if ( tempClass && tempClass2 && pEnv->IsInstanceOf( _par0, tempClass ) && pEnv->IsInstanceOf( _par0, tempClass2 ) )
		{
			static jmethodID mIDGetModelPtr = NULL;
			if ( !mIDGetModelPtr )
			{
				char *cSignature = "()J";
				mIDGetModelPtr = pEnv->GetMethodID( tempClass, "getModelPtr", cSignature );
			}
			OSL_ENSURE( mIDGetModelPtr, "Unknown method id!" );
			if ( mIDGetModelPtr )
				CWindow_makeUnshadowedWindow( (void *) pEnv->CallLongMethod( _par0, mIDGetModelPtr ) );
		}
	}
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
			if ( !mIDGetModelPtr )
			{
				char *cSignature = "()J";
				mIDGetModelPtr = pEnv->GetMethodID( tempClass, "getModelPtr", cSignature );
			}
			OSL_ENSURE( mIDGetModelPtr, "Unknown method id!" );
			if ( mIDGetModelPtr )
				CWindow_updateLocation( (void *) pEnv->CallLongMethod( _par0, mIDGetModelPtr ) );
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
			JNINativeMethod pMethods[5];
			pMethods[0].name = "getBounds0";
			pMethods[0].signature = "(Ljava/awt/peer/ComponentPeer;JZ)Ljava/awt/Rectangle;";
			pMethods[0].fnPtr = (void *)Java_com_sun_star_vcl_VCLFrame_getBounds0;
			pMethods[1].name = "getTextLocation0";
			pMethods[1].signature = "(J)Ljava/awt/Rectangle;";
			pMethods[1].fnPtr = (void *)Java_com_sun_star_vcl_VCLFrame_getTextLocation0;
			pMethods[2].name = "makeFloatingWindow";
			pMethods[2].signature = "(Ljava/awt/peer/ComponentPeer;)I";
			pMethods[2].fnPtr = (void *)Java_com_sun_star_vcl_VCLFrame_makeFloatingWindow;
			pMethods[3].name = "makeUnshadowedWindow";
			pMethods[3].signature = "(Ljava/awt/peer/ComponentPeer;)V";
			pMethods[3].fnPtr = (void *)Java_com_sun_star_vcl_VCLFrame_makeUnshadowedWindow;
			pMethods[4].name = "updateLocation";
			pMethods[4].signature = "(Ljava/awt/peer/ComponentPeer;)V";
			pMethods[4].fnPtr = (void *)Java_com_sun_star_vcl_VCLFrame_updateLocation;
			t.pEnv->RegisterNatives( tempClass, pMethods, 5 );
		}

		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

#ifndef USE_NATIVE_WINDOW

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFrame::flushAllFrames()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "flushAllFrames", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			t.pEnv->CallStaticVoidMethod( getMyClass(), mID );
	}
}

#endif	// !USE_NATIVE_WINDOW

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLFrame::com_sun_star_vcl_VCLFrame( ULONG nSalFrameStyle, const JavaSalFrame *pFrame, const JavaSalFrame *pParent, sal_Bool bShowOnlyMenus, sal_Bool bUtilityWindow ) : java_lang_Object( (jobject)NULL )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( !t.pEnv )
		return;
	if ( !mID )
	{
		char *cSignature = "(JLcom/sun/star/vcl/VCLEventQueue;JLcom/sun/star/vcl/VCLFrame;ZZZ)V";
		mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
	}
	OSL_ENSURE( mID, "Unknown method id!" );

	jvalue args[7];
	args[0].j = jlong( nSalFrameStyle );
	args[1].l = GetSalData()->mpEventQueue->getJavaObject();
	args[2].j = jlong( pFrame );
	if ( pParent )
		args[3].l = pParent->mpVCLFrame->getJavaObject();
	else
		args[3].l = NULL;
	args[4].z = jboolean( bShowOnlyMenus );
	args[5].z = jboolean( bUtilityWindow );
#ifdef USE_ROUNDED_BOTTOM_CORNERS_IN_JAVA_FRAMES
	args[6].z = jboolean( !IsRunningLeopard() && !IsRunningSnowLeopard() );
#else	// USE_ROUNDED_BOTTOM_CORNERS_IN_JAVA_FRAMES
	args[6].z = jboolean( sal_False );
#endif	// USE_ROUNDED_BOTTOM_CORNERS_IN_JAVA_FRAMES
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

const Rectangle com_sun_star_vcl_VCLFrame::getBounds( sal_Bool *_par0, sal_Bool _par1 )
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
			char *cSignature = "(JZ)Ljava/awt/Rectangle;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getBounds", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[2];
			args[0].j = jlong( _par0 );
			args[1].z = jboolean( _par1 );
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethodA( object, getMyClass(), mID, args );
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

#ifndef USE_NATIVE_WINDOW

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

#endif	// !USE_NATIVE_WINDOW

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
	return (void *)CWindow_getNSWindow( getPeer() );
}

// ----------------------------------------------------------------------------

void *com_sun_star_vcl_VCLFrame::getNativeWindowContentView( sal_Bool _par0 )
{
	return (void *)CWindow_getNSWindowContentView( getPeer(), _par0 );
}

// ----------------------------------------------------------------------------

void *com_sun_star_vcl_VCLFrame::getPeer()
{
	static jmethodID mID = NULL;
	void *out = NULL;
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
			{
				jclass tempClass = t.pEnv->FindClass( "apple/awt/ComponentModel" );
				if ( tempClass && t.pEnv->IsInstanceOf( tempObj, tempClass ) )
				{
					static jmethodID mIDGetModelPtr = NULL;
					if ( !mIDGetModelPtr )
					{
						char *cSignature = "()J";
						mIDGetModelPtr = t.pEnv->GetMethodID( tempClass, "getModelPtr", cSignature );
					}
					OSL_ENSURE( mIDGetModelPtr, "Unknown method id!" );
					if ( mIDGetModelPtr )
						out = (void *)t.pEnv->CallLongMethod( tempObj, mIDGetModelPtr );
				}
			}
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

void com_sun_star_vcl_VCLFrame::makeModal()
{
	CWindow_makeModalWindow( getPeer() );
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

void com_sun_star_vcl_VCLFrame::setAllowKeyBindings( sal_Bool _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Z)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setAllowKeyBindings", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].z = jint( _par0 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
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

void com_sun_star_vcl_VCLFrame::setVisible( sal_Bool _par0, sal_Bool _par1 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(ZZ)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setVisible", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[2];
			args[0].z = jboolean( _par0 );
			args[1].z = jboolean( _par1 );
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
