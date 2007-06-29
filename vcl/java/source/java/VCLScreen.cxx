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

#define _SV_COM_SUN_STAR_VCL_VCLSCREEN_CXX

#ifndef _SV_COM_SUN_STAR_VCL_VCLSCREEN_HXX
#include <com/sun/star/vcl/VCLScreen.hxx>
#endif

using namespace vcl;

// ============================================================================

static jobject JNICALL Java_com_sun_star_vcl_VCLScreen_getScreenInsets( JNIEnv *pEnv, jobject object, jobject _par0 )
{
	jobject out = NULL;

	if ( _par0 )
	{
		jclass tempClass = pEnv->FindClass( "apple/awt/CGraphicsDevice" );
		if ( tempClass && pEnv->IsInstanceOf( _par0, tempClass ) )
		{
			static jmethodID mIDGetScreenInsets = NULL;
			if ( !mIDGetScreenInsets )
            {
				char *cSignature = "()Ljava/awt/Insets;";
				mIDGetScreenInsets = pEnv->GetMethodID( tempClass, "getScreenInsets", cSignature );
			}
			OSL_ENSURE( mIDGetScreenInsets, "Unknown method id!" );
			if ( mIDGetScreenInsets )
				out = pEnv->CallObjectMethod( _par0, mIDGetScreenInsets );
		}
	}

	return out;
}

// ============================================================================

jclass com_sun_star_vcl_VCLScreen::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_vcl_VCLScreen::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLScreen" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );

		if ( tempClass )
		{
			// Register the native methods for our class
			JNINativeMethod aMethod;
			aMethod.name = "getScreenInsets";
			aMethod.signature = "(Ljava/awt/GraphicsDevice;)Ljava/awt/Insets;";
			aMethod.fnPtr = (void *)Java_com_sun_star_vcl_VCLScreen_getScreenInsets;
			t.pEnv->RegisterNatives( tempClass, &aMethod, 1 );
		}

		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

SalColor com_sun_star_vcl_VCLScreen::getControlColor()
{
	static jmethodID mID = NULL;
	SalColor out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getControlColor", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (SalColor)t.pEnv->CallStaticIntMethod( getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

unsigned int com_sun_star_vcl_VCLScreen::getDefaultScreenNumber()
{
	static jmethodID mID = NULL;
	unsigned int out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getDefaultScreenNumber", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (unsigned int)t.pEnv->CallStaticIntMethod( getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

const Rectangle com_sun_star_vcl_VCLScreen::getScreenBounds( long _par0, long _par1, long _par2, long _par3, sal_Bool _par4, sal_Bool _par5 )
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
			char *cSignature = "(IIIIZZ)Ljava/awt/Rectangle;";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getScreenBounds", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[6];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].z = jboolean( _par4 );
			args[5].z = jboolean( _par5 );
			jobject tempObj = t.pEnv->CallStaticObjectMethodA( getMyClass(), mID, args );
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

const Rectangle com_sun_star_vcl_VCLScreen::getScreenBounds( unsigned int _par0, sal_Bool _par1 )
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
			char *cSignature = "(IZ)Ljava/awt/Rectangle;";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getScreenBounds", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[2];
			args[0].i = jint( _par0 );
			args[1].z = jboolean( _par1 );
			jobject tempObj = t.pEnv->CallStaticObjectMethodA( getMyClass(), mID, args );
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

unsigned int com_sun_star_vcl_VCLScreen::getScreenCount()
{
	static jmethodID mID = NULL;
	unsigned int out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getScreenCount", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (unsigned int)t.pEnv->CallStaticIntMethod( getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

SalColor com_sun_star_vcl_VCLScreen::getTextHighlightColor()
{
	static jmethodID mID = NULL;
	SalColor out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getTextHighlightColor", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (SalColor)t.pEnv->CallStaticIntMethod( getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

SalColor com_sun_star_vcl_VCLScreen::getTextHighlightTextColor()
{
	static jmethodID mID = NULL;
	SalColor out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getTextHighlightTextColor", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (SalColor)t.pEnv->CallStaticIntMethod( getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

SalColor com_sun_star_vcl_VCLScreen::getTextTextColor()
{
	static jmethodID mID = NULL;
	SalColor out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getTextTextColor", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (SalColor)t.pEnv->CallStaticIntMethod( getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

const Rectangle com_sun_star_vcl_VCLScreen::getVirtualScreenBounds()
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
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getVirtualScreenBounds", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallStaticObjectMethod( getMyClass(), mID );
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
