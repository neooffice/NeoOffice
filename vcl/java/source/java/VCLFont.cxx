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

com_sun_star_vcl_VCLFontList::~com_sun_star_vcl_VCLFontList()
{
	if ( mpFonts )
	{
		for ( int i = 0; i < mnCount; i++ )
			delete mpFonts[ i ];
		rtl_freeMemory( mpFonts );
	}
}

// ============================================================================

jclass com_sun_star_vcl_VCLFont::theClass = NULL;

// ----------------------------------------------------------------------------

jboolean com_sun_star_vcl_VCLFont::useDefaultFont = FALSE;

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
				out->mnCount = t.pEnv->GetArrayLength( tempArray );
				out->mpFonts = (com_sun_star_vcl_VCLFont **)rtl_allocateMemory( out->mnCount * sizeof( com_sun_star_vcl_VCLFont* ) );
				for ( jsize i = 0; i < out->mnCount; i++ )
				{
					jobject tempObj = t.pEnv->GetObjectArrayElement( tempArray, i );
					out->mpFonts[ i ] = new com_sun_star_vcl_VCLFont( tempObj );
				}
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLFont *com_sun_star_vcl_VCLFont::deriveFont( long _par0, sal_Bool _par1, sal_Bool _par2, sal_Bool _par3 )
{
	static jmethodID mID = NULL;
	com_sun_star_vcl_VCLFont *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(IZZZ)Lcom/sun/star/vcl/VCLFont;";
			mID = t.pEnv->GetMethodID( getMyClass(), "deriveFont", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[4];
			args[0].i = jint( _par0 );
			args[1].z = jboolean( _par1 );
			args[2].z = jboolean( _par2 );
			args[3].z = jboolean( _par3 );
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethodA( object, getMyClass(), mID, args );
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
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getAscent", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			if ( com_sun_star_vcl_VCLFont::useDefaultFont )
			{
				com_sun_star_vcl_VCLFont *pDefaultFont = getDefaultFont();
				out = (long)t.pEnv->CallNonvirtualIntMethod( pDefaultFont->getJavaObject(), getMyClass(), mID );
				delete pDefaultFont;
			}
			else
			{
				out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLFont *com_sun_star_vcl_VCLFont::getDefaultFont()
{
	static jmethodID mID = NULL;
	com_sun_star_vcl_VCLFont *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Lcom/sun/star/vcl/VCLFont;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getDefaultFont", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj;
			tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
				out = new com_sun_star_vcl_VCLFont( tempObj );
		}
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
		{
			if ( com_sun_star_vcl_VCLFont::useDefaultFont )
			{
				com_sun_star_vcl_VCLFont *pDefaultFont = getDefaultFont();
				out = (long)t.pEnv->CallNonvirtualIntMethod( pDefaultFont->getJavaObject(), getMyClass(), mID );
				delete pDefaultFont;
			}
			else
			{
				out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

FontFamily com_sun_star_vcl_VCLFont::getFamilyType()
{
	static jmethodID mID = NULL;
	FontFamily out = FAMILY_DONTKNOW;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getFamilyType", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (FontFamily)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
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
			if ( com_sun_star_vcl_VCLFont::useDefaultFont )
			{
				com_sun_star_vcl_VCLFont *pDefaultFont = getDefaultFont();
				out = (long)t.pEnv->CallNonvirtualIntMethodA( pDefaultFont->getJavaObject(), getMyClass(), mID, args );
				delete pDefaultFont;
			}
			else
			{
				out = (long)t.pEnv->CallNonvirtualIntMethodA( object, getMyClass(), mID, args );
			}
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
		{
			if ( com_sun_star_vcl_VCLFont::useDefaultFont )
			{
				com_sun_star_vcl_VCLFont *pDefaultFont = getDefaultFont();
				out = (long)t.pEnv->CallNonvirtualIntMethod( pDefaultFont->getJavaObject(), getMyClass(), mID );
				delete pDefaultFont;
			}
			else
			{
				out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
			}
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
			tempObj = (jstring)t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
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
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethod( object, getMyClass(), mID );
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
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethod( object, getMyClass(), mID );
	}
	return out;
}
