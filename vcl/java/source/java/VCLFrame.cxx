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
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif

using namespace vcl;

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
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLFrame::com_sun_star_vcl_VCLFrame( ULONG nSalFrameStyle, const SalFrame *pFrame ) : java_lang_Object( (jobject)NULL )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( !t.pEnv )
		return;
	if ( !mID )
	{
		char *cSignature = "(JLcom/sun/star/vcl/VCLEventQueue;J)V";
		mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
	}
	OSL_ENSURE( mID, "Unknown method id!" );
	jvalue args[3];
	args[0].j = jlong( nSalFrameStyle );
	args[1].l = GetSalData()->mpEventQueue->getJavaObject();
	args[2].j = jlong( pFrame );
	jobject tempObj;
	tempObj = t.pEnv->NewObjectA( getMyClass(), mID, args );
	saveRef( tempObj );
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
		{
			t.pEnv->CallVoidMethod( object, mID );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFrame::flush()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetMethodID( getMyClass(), "flush", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			t.pEnv->CallVoidMethod( object, mID );
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
			jobject tempObj = t.pEnv->CallObjectMethod( object, mID );
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
			jobject tempObj = t.pEnv->CallObjectMethod( object, mID );
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
			jobject tempObj = t.pEnv->CallObjectMethod( object, mID );
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
			jstring tempObj = (jstring)t.pEnv->CallObjectMethodA( object, mID, args );
			if ( tempObj )
				out = JavaString2String( t.pEnv, tempObj );
		}
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
			t.pEnv->CallVoidMethodA( object, mID, args );
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
			t.pEnv->CallVoidMethodA( object, mID, args );
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
			t.pEnv->CallVoidMethodA( object, mID, args );
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
			t.pEnv->CallVoidMethodA( object, mID, args );
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
			t.pEnv->CallVoidMethodA( object, mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFrame::toFront()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetMethodID( getMyClass(), "toFront", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			t.pEnv->CallVoidMethod( object, mID );
	}
}
