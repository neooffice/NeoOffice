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

#define _SV_COM_SUN_STAR_VCL_VCLBITMAP_CXX

#ifndef _SV_COM_SUN_STAR_VCL_VCLBITMAP_HXX
#include <com/sun/star/vcl/VCLBitmap.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif

using namespace vcl;

// ============================================================================

jclass com_sun_star_vcl_VCLBitmap::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_vcl_VCLBitmap::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLBitmap" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLBitmap::com_sun_star_vcl_VCLBitmap( long nDX, long nDY, USHORT nBitCount ) : java_lang_Object( (jobject)NULL )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( !t.pEnv )
		return;
	if ( !mID )
	{
		char *cSignature = "(III)V";
		mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
	}
	OSL_ENSURE( mID, "Unknown method id!" );
	jvalue args[3];
	args[0].i = jint( nDX );
	args[1].i = jint( nDY );
	args[2].i = jint( nBitCount );
	jobject tempObj;
	tempObj = t.pEnv->NewObjectA( getMyClass(), mID, args );
	saveRef( tempObj );
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLBitmap::copyBits( const com_sun_star_vcl_VCLGraphics *_par0, long _par1, long _par2, long _par3, long _par4, long _par5, long _par6 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Lcom/sun/star/vcl/VCLGraphics;IIIIII)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "copyBits", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[7];
			args[0].l = _par0->getJavaObject();
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].i = jint( _par4 );
			args[5].i = jint( _par5 );
			args[6].i = jint( _par6 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

USHORT com_sun_star_vcl_VCLBitmap::getBitCount()
{
	static jmethodID mID = NULL;
	USHORT out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getBitCount", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (USHORT)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

java_lang_Object *com_sun_star_vcl_VCLBitmap::getData()
{
	static jmethodID mID = NULL;
	java_lang_Object *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/lang/Object;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getData", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj;
			tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
				out = new java_lang_Object( tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLBitmap::getPalette( BitmapPalette& _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()[I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getPalette", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jintArray tempObj = (jintArray)t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
			{
				jsize nColors = t.pEnv->GetArrayLength( tempObj );
				if ( nColors )
				{
					_par0.SetEntryCount( nColors );
					jboolean bCopy( sal_False );
					jint *pColorBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( tempObj, &bCopy );
					for ( jsize i = 0 ; i < nColors ; i++ )
					{
						jint nCurrentColor = pColorBits[ i ];
						_par0[ i ] = BitmapColor( SALCOLOR_RED( nCurrentColor ), SALCOLOR_GREEN( nCurrentColor ), SALCOLOR_BLUE( nCurrentColor ) );
					}
					t.pEnv->ReleasePrimitiveArrayCritical( tempObj, pColorBits, JNI_ABORT );
				}
			}
			else
			{
				_par0.SetEntryCount( 0 );
			}
		}
	}
}


// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLBitmap::setPalette( const BitmapPalette& _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "([I)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setPalette", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jsize nColors = _par0.GetEntryCount();
			if ( nColors )
			{
				jintArray pColors = t.pEnv->NewIntArray( nColors );
				jboolean bCopy( sal_False );
				jint *pColorBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( pColors, &bCopy );
				for ( jsize i = 0 ; i < nColors ; i++ )
				{
					const BitmapColor &rCol = _par0[ i ];
					pColorBits[ i ] = MAKE_SALCOLOR( rCol.GetRed(), rCol.GetGreen(), rCol.GetBlue() );
				}
				t.pEnv->ReleasePrimitiveArrayCritical( pColors, pColorBits, 0 );
				jvalue args[1];
				args[0].l = pColors;
				t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
			}
			else
			{
				jvalue args[1];
				args[0].l = NULL;
				t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
			}
		}
	}
}

