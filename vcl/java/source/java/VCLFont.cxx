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

#define _SV_COM_SUN_STAR_VCL_VCLFONT_CXX

#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif

using namespace rtl;
using namespace vcl;

// ============================================================================

jclass com_sun_star_vcl_VCLFont::theClass = NULL;

// ----------------------------------------------------------------------------

java_lang_Object *com_sun_star_vcl_VCLFont::getAllFonts()
{
	static jmethodID mID = NULL;
	java_lang_Object *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()[Lcom/sun/star/vcl/VCLFont;";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getAllFonts", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobjectArray tempObj = (jobjectArray)t.pEnv->CallStaticObjectMethod( getMyClass(), mID );
			if ( tempObj )
				out = new java_lang_Object( tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

jclass com_sun_star_vcl_VCLFont::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLFont" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLFont *com_sun_star_vcl_VCLFont::deriveFont( long _par0, short _par1, sal_Bool _par2, sal_Bool _par3, double _par4 )
{
	static jmethodID mID = NULL;
	com_sun_star_vcl_VCLFont *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(ISZZD)Lcom/sun/star/vcl/VCLFont;";
			mID = t.pEnv->GetMethodID( getMyClass(), "deriveFont", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[5];
			args[0].i = jint( _par0 );
			args[1].s = jshort( _par1 );
			args[2].z = jboolean( _par2 );
			args[3].z = jboolean( _par3 );
			args[4].d = jdouble( _par4 );
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethodA( object, getMyClass(), mID, args );
			if ( tempObj )
				out = new com_sun_star_vcl_VCLFont( tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

OUString com_sun_star_vcl_VCLFont::findFontNameForStyle( sal_Bool _par0, sal_Bool _par1 )
{
	static jmethodID mID = NULL;
	OUString out;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(ZZ)Ljava/lang/String;";
			mID = t.pEnv->GetMethodID( getMyClass(), "findFontNameForStyle", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[2];
			args[0].z = jboolean( _par0 );
			args[1].z = jboolean( _par1 );
			jstring tempObj = (jstring)t.pEnv->CallNonvirtualObjectMethodA( object, getMyClass(), mID, args );
			if ( tempObj )
				out = JavaString2String( t.pEnv, tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

long com_sun_star_vcl_VCLFont::getAscent()
{
	static jmethodID mID = NULL;
	long out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getAscent", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

long com_sun_star_vcl_VCLFont::getDescent()
{
	static jmethodID mID = NULL;
	long out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getDescent", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

long com_sun_star_vcl_VCLFont::getKerning( USHORT _par0, USHORT _par1 )
{
	static jmethodID mID = NULL;
	long out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getKerning", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[2];
			args[0].c = jchar( _par0 );
			args[1].c = jchar( _par1 );
			out = (long)t.pEnv->CallNonvirtualIntMethodA( object, getMyClass(), mID, args );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

long com_sun_star_vcl_VCLFont::getLeading()
{
	static jmethodID mID = NULL;
	long out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getLeading", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

OUString com_sun_star_vcl_VCLFont::getName()
{
	static jmethodID mID = NULL;
	OUString out;
	VCLThreadAttach t;
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
			jstring tempObj;
			tempObj = (jstring)t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
				out = JavaString2String( t.pEnv, tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

void *com_sun_star_vcl_VCLFont::getNativeFont()
{
	static jmethodID mID = NULL;
	void *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getNativeFont", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (void *)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

short com_sun_star_vcl_VCLFont::getOrientation()
{
	static jmethodID mID = NULL;
	short out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()S";
			mID = t.pEnv->GetMethodID( getMyClass(), "getOrientation", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (short)t.pEnv->CallNonvirtualShortMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

double com_sun_star_vcl_VCLFont::getScaleX()
{
	static jmethodID mID = NULL;
	double out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()D";
			mID = t.pEnv->GetMethodID( getMyClass(), "getScaleX", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (double)t.pEnv->CallNonvirtualDoubleMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

long com_sun_star_vcl_VCLFont::getSize()
{
	static jmethodID mID = NULL;
	long out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getSize", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLFont::isAntialiased()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "isAntialiased", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLFont::isVertical()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "isVertical", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFont::setNativeFont( void *_par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(I)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setNativeFont", cSignature );
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
