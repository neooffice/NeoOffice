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

using namespace vcl;

// ============================================================================

jclass com_sun_star_vcl_VCLFont::theClass = NULL;

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

com_sun_star_vcl_VCLFontList *com_sun_star_vcl_VCLFont::getAllFonts()
{
	static jmethodID mID = NULL;
	com_sun_star_vcl_VCLFontList *out = NULL;
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
			jobjectArray tempArray;
			tempArray = (jobjectArray)t.pEnv->CallStaticObjectMethod( getMyClass(), mID );
			if ( tempArray )
			{
				out = new com_sun_star_vcl_VCLFontList();
				out->nCount = t.pEnv->GetArrayLength( tempArray );
				out->pFonts = new com_sun_star_vcl_VCLFont[ out->nCount ];
				for ( jsize i = 0; i < out->nCount; i++ )
				{
					jobject tempObj = t.pEnv->GetObjectArrayElement( tempArray, i );
					out->pFonts[i].saveRef( tempObj );
				}
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLFont *com_sun_star_vcl_VCLFont::deriveFont( long _par0, sal_Bool _par1, sal_Bool _par2 )
{
	static jmethodID mID = NULL;
	com_sun_star_vcl_VCLFont *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(IZZ)Lcom/sun/star/vcl/VCLFont;";
			mID = t.pEnv->GetMethodID( getMyClass(), "deriveFont", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[3];
			args[0].i = jint( _par0 );
			args[1].z = jboolean( _par1 );
			args[2].z = jboolean( _par2 );
			jobject tempObj = t.pEnv->CallObjectMethodA( object, mID, args );
			if ( tempObj )
				out = new com_sun_star_vcl_VCLFont( tempObj );
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
			char *cSignature = "()J";
			mID = t.pEnv->GetMethodID( getMyClass(), "getAscent", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			out = (long)t.pEnv->CallLongMethod( object, mID );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFont::getCharWidth( sal_Unicode _par0, sal_Unicode _par1, long *_par2 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(CC)[I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getCharWidth", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[2];
			args[0].c = jchar( _par0 );
			args[1].c = jchar( _par1 );
			jintArray tempObj;
			tempObj = (jintArray)t.pEnv->CallObjectMethodA( object, mID, args );
			if ( tempObj )
			{
				jsize elements( _par1 - _par0 + 1 );
				t.pEnv->GetIntArrayRegion( tempObj, 0, elements, (jint *)_par2 );
			}
		}
	}
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
			char *cSignature = "()J";
			mID = t.pEnv->GetMethodID( getMyClass(), "getDescent", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			out = (long)t.pEnv->CallLongMethod( object, mID );
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
			char *cSignature = "()J";
			mID = t.pEnv->GetMethodID( getMyClass(), "getLeading", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			out = (long)t.pEnv->CallLongMethod( object, mID );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

::rtl::OUString com_sun_star_vcl_VCLFont::getName()
{
	static jmethodID mID = NULL;
	::rtl::OUString out;
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
			tempObj = (jstring)t.pEnv->CallObjectMethod( object, mID );
			if ( tempObj )
				out = JavaString2String( t.pEnv, tempObj );
		}
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
			char *cSignature = "()J";
			mID = t.pEnv->GetMethodID( getMyClass(), "getSize", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			out = (long)t.pEnv->CallLongMethod( object, mID );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLFont::isBold()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "isBold", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			out = (sal_Bool)t.pEnv->CallBooleanMethod( object, mID );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLFont::isItalic()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "isItalic", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (sal_Bool)t.pEnv->CallBooleanMethod( object, mID );
	}
	return out;
}
