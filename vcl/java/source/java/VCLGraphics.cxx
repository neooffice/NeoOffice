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

#define _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_CXX

#ifndef _SV_JAVA_LANG_CLASS_HXX
#include <java/lang/Class.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLBITMAP_HXX
#include <com/sun/star/vcl/VCLBitmap.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLIMAGE_HXX
#include <com/sun/star/vcl/VCLImage.hxx>
#endif

#ifdef MACOSX

#include <premac.h>
#include <QuickTime/QuickTime.h>
#include <postmac.h>

using namespace rtl;

#endif	// MACOSX

using namespace vcl;

// ============================================================================

jclass com_sun_star_vcl_VCLGraphics::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_vcl_VCLGraphics::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLGraphics" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::beep()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
#ifdef MACOSX
		// Test the JVM version and if it is below 1.4, use Carbon APIs
		if ( t.pEnv->GetVersion() < JNI_VERSION_1_4 )
		{
			// Toolkit.beep() doesn't work so call it natively
			SysBeep( 30 );
			return;
		}
#endif	// MACOSX

		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "beep", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			t.pEnv->CallStaticVoidMethod( getMyClass(), mID );
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::beginSetClipRegion()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetMethodID( getMyClass(), "beginSetClipRegion", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			t.pEnv->CallNonvirtualVoidMethod( object, getMyClass(), mID );
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::copyBits( const com_sun_star_vcl_VCLGraphics *_par0, long _par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Lcom/sun/star/vcl/VCLGraphics;IIIIIIII)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "copyBits", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[9];
			args[0].l = _par0->getJavaObject();
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].i = jint( _par4 );
			args[5].i = jint( _par5 );
			args[6].i = jint( _par6 );
			args[7].i = jint( _par7 );
			args[8].i = jint( _par8 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawBitmap( const com_sun_star_vcl_VCLBitmap *_par0, long _par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Lcom/sun/star/vcl/VCLBitmap;IIIIIIII)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawBitmap", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[9];
			args[0].l = _par0->getJavaObject();
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].i = jint( _par4 );
			args[5].i = jint( _par5 );
			args[6].i = jint( _par6 );
			args[7].i = jint( _par7 );
			args[8].i = jint( _par8 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawBitmap( const com_sun_star_vcl_VCLBitmap *_par0, const com_sun_star_vcl_VCLBitmap *_par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8, long _par9 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Lcom/sun/star/vcl/VCLBitmap;Lcom/sun/star/vcl/VCLBitmap;IIIIIIII)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawBitmap", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[10];
			args[0].l = _par0->getJavaObject();
			args[1].l = _par1->getJavaObject();
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].i = jint( _par4 );
			args[5].i = jint( _par5 );
			args[6].i = jint( _par6 );
			args[7].i = jint( _par7 );
			args[8].i = jint( _par8 );
			args[9].i = jint( _par9 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawGlyphs( long _par0, long _par1, int _par2, long *_par3, long *_par4, com_sun_star_vcl_VCLFont *_par5, SalColor _par6, int _par7, long _par8, long _par9, int _par10, sal_Bool _par11 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(II[I[ILcom/sun/star/vcl/VCLFont;IIIIIZ)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawGlyphs", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jsize elements( _par2 );
			jintArray glypharray = t.pEnv->NewIntArray( elements );
			jintArray advancearray = t.pEnv->NewIntArray( elements );
			t.pEnv->SetIntArrayRegion( glypharray, 0, elements, (jint *)_par3 );
			t.pEnv->SetIntArrayRegion( advancearray, 0, elements, (jint *)_par4 );

			com_sun_star_vcl_VCLFont *pFont = NULL;
			if ( com_sun_star_vcl_VCLFont::useDefaultFont )
				pFont = _par5->getDefaultFont();
			jvalue args[11];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].l = glypharray;
			args[3].l = advancearray;
			args[4].l = pFont ? pFont->getJavaObject() : _par5->getJavaObject();
			args[5].i = jint( _par6 );
			args[6].i = jint( _par7 );
			args[7].i = jint( _par8 );
			args[8].i = jint( _par9 );
			args[9].i = jint( _par10 );
			args[10].z = jboolean( _par11 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
			if ( pFont )
				delete pFont;
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawLine( long _par0, long _par1, long _par2, long _par3, SalColor _par4 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(IIIII)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawLine", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[5];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].i = jint( _par4 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawMask( const com_sun_star_vcl_VCLBitmap *_par0, SalColor _par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8, long _par9 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Lcom/sun/star/vcl/VCLBitmap;IIIIIIIII)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawMask", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[10];
			args[0].l = _par0->getJavaObject();
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].i = jint( _par4 );
			args[5].i = jint( _par5 );
			args[6].i = jint( _par6 );
			args[7].i = jint( _par7 );
			args[8].i = jint( _par8 );
			args[9].i = jint( _par9 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawPolygon( ULONG _par0, const long *_par1, const long *_par2, SalColor _par3, sal_Bool _par4 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(I[I[IIZ)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawPolygon", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jsize elements( _par0 );
			jintArray xarray = t.pEnv->NewIntArray( elements );
			t.pEnv->SetIntArrayRegion( xarray, 0, elements, (jint *)_par1 );
			jintArray yarray = t.pEnv->NewIntArray( elements );
			t.pEnv->SetIntArrayRegion( yarray, 0, elements, (jint *)_par2 );
			jvalue args[5];
			args[0].i = jint( _par0 );
			args[1].l = xarray;
			args[2].l = yarray;
			args[3].i = jint( _par3 );
			args[4].z = jboolean( _par4 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawPolyline( ULONG _par0, const long *_par1, const long *_par2, SalColor _par3 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(I[I[II)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawPolyline", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jsize elements( _par0 );
			jintArray xarray = t.pEnv->NewIntArray( elements );
			t.pEnv->SetIntArrayRegion( xarray, 0, elements, (jint *)_par1 );
			jintArray yarray = t.pEnv->NewIntArray( elements );
			t.pEnv->SetIntArrayRegion( yarray, 0, elements, (jint *)_par2 );
			jvalue args[4];
			args[0].i = jint( _par0 );
			args[1].l = xarray;
			args[2].l = yarray;
			args[3].i = jint( _par3 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawPolyPolygon( ULONG _par0, const ULONG *_par1, long **_par2, long **_par3, SalColor _par4, sal_Bool _par5 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(I[I[[I[[IIZ)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawPolyPolygon", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		jclass tempClass = t.pEnv->FindClass( "[I" );
		if ( mID && tempClass )
		{
			jsize elements( _par0 );
			jintArray ptsarray = t.pEnv->NewIntArray( elements );
			t.pEnv->SetIntArrayRegion( ptsarray, 0, elements, (jint *)_par1 );
			jintArray tempArray = t.pEnv->NewIntArray( 0 );
			jobjectArray xptsarray = t.pEnv->NewObjectArray( elements, tempClass, tempArray );
			jobjectArray yptsarray = t.pEnv->NewObjectArray( elements, tempClass, tempArray );
			for ( jsize i = 0; i < elements; i++ )
			{
				jsize points = _par1[ i ];
				jintArray xarray = t.pEnv->NewIntArray( points );
				t.pEnv->SetIntArrayRegion( xarray, 0, points, (jint *)_par2[ i ] );
				t.pEnv->SetObjectArrayElement( xptsarray, i, xarray );
				jintArray yarray = t.pEnv->NewIntArray( points );
				t.pEnv->SetIntArrayRegion( yarray, 0, points, (jint *)_par3[ i ] );
				t.pEnv->SetObjectArrayElement( yptsarray, i, yarray );
			}
			jvalue args[6];
			args[0].i = jint( _par0 );
			args[1].l = ptsarray;
			args[2].l = xptsarray;
			args[3].l = yptsarray;
			args[4].i = jint( _par4 );
			args[5].z = jboolean( _par5 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawRect( long _par0, long _par1, long _par2, long _par3, SalColor _par4, sal_Bool _par5 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(IIIIIZ)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawRect", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[6];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].i = jint( _par4 );
			args[5].z = jboolean( _par5 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::endSetClipRegion()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetMethodID( getMyClass(), "endSetClipRegion", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			t.pEnv->CallNonvirtualVoidMethod( object, getMyClass(), mID );
	}
}

// ----------------------------------------------------------------------------

USHORT com_sun_star_vcl_VCLGraphics::getBitCount()
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

const Rectangle com_sun_star_vcl_VCLGraphics::getGlyphBounds( int _par0, com_sun_star_vcl_VCLFont *_par1, sal_Bool _par2 )
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
			char *cSignature = "(ILcom/sun/star/vcl/VCLFont;Z)Ljava/awt/Rectangle;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getGlyphBounds", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			com_sun_star_vcl_VCLFont *pFont = NULL;
			if ( com_sun_star_vcl_VCLFont::useDefaultFont )
				pFont = _par1->getDefaultFont();
			jvalue args[3];
			args[0].i = jint( _par0 );
			args[1].l = pFont ? pFont->getJavaObject() : _par1->getJavaObject();
			args[2].z = jboolean( _par2 );
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethodA( object, getMyClass(), mID, args );
			if ( pFont )
				delete pFont;
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

com_sun_star_vcl_VCLImage *com_sun_star_vcl_VCLGraphics::getImage()
{
	static jmethodID mID = NULL;
	com_sun_star_vcl_VCLImage *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Lcom/sun/star/vcl/VCLImage;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getImage", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
				out = new com_sun_star_vcl_VCLImage( tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

SalColor com_sun_star_vcl_VCLGraphics::getPixel( long _par0, long _par1 )
{
	static jmethodID mID = NULL;
	SalColor out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(II)I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getPixel", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[2];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			out = (SalColor)t.pEnv->CallNonvirtualIntMethodA( object, getMyClass(), mID, args );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

const Size com_sun_star_vcl_VCLGraphics::getResolution()
{
	static jmethodID mID = NULL;
	static jfieldID fIDWidth = NULL;
	static jfieldID fIDHeight = NULL;	 
	Size out;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/awt/Dimension;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getResolution", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
			{
				jclass tempObjClass = t.pEnv->GetObjectClass( tempObj );
				OSL_ENSURE( tempObjClass, "Java : FindClass not found!" );
				if ( !fIDWidth )
				{
					char *cSignature = "I";
					fIDWidth = t.pEnv->GetFieldID( tempObjClass, "width", cSignature );
				}
				out.setWidth( (long)t.pEnv->GetIntField( tempObj, fIDWidth ) );
				if ( !fIDHeight )
				{
					char *cSignature = "I";
					fIDHeight = t.pEnv->GetFieldID( tempObjClass, "height", cSignature );
				}
				out.setHeight( (long)t.pEnv->GetIntField( tempObj, fIDHeight ) );
			}
		}
	}
    return out;
}

// ----------------------------------------------------------------------------

const Size com_sun_star_vcl_VCLGraphics::getScreenFontResolution()
{
	static jmethodID mID = NULL;
	static jfieldID fIDWidth = NULL;
	static jfieldID fIDHeight = NULL;	 
	Size out;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/awt/Dimension;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getScreenFontResolution", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
			{
				jclass tempObjClass = t.pEnv->GetObjectClass( tempObj );
				OSL_ENSURE( tempObjClass, "Java : FindClass not found!" );
				if ( !fIDWidth )
				{
					char *cSignature = "I";
					fIDWidth = t.pEnv->GetFieldID( tempObjClass, "width", cSignature );
				}
				out.setWidth( (long)t.pEnv->GetIntField( tempObj, fIDWidth ) );
				if ( !fIDHeight )
				{
					char *cSignature = "I";
					fIDHeight = t.pEnv->GetFieldID( tempObjClass, "height", cSignature );
				}
				out.setHeight( (long)t.pEnv->GetIntField( tempObj, fIDHeight ) );
			}
		}
	}
    return out;
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::invert( long _par0, long _par1, long _par2, long _par3, SalInvert _par4 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(IIIII)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "invert", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[5];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].i = jint( _par4 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::invert( ULONG _par0, const long *_par1, const long *_par2, SalInvert _par3 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(I[I[II)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "invert", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jsize elements( _par0 );
			jintArray xarray = t.pEnv->NewIntArray( elements );
			t.pEnv->SetIntArrayRegion( xarray, 0, elements, (jint *)_par1 );
			jintArray yarray = t.pEnv->NewIntArray( elements );
			t.pEnv->SetIntArrayRegion( yarray, 0, elements, (jint *)_par2 );
			jvalue args[4];
			args[0].i = jint( _par0 );
			args[1].l = xarray;
			args[2].l = yarray;
			args[3].i = jint( _par3 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::resetClipRegion()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetMethodID( getMyClass(), "resetClipRegion", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			t.pEnv->CallNonvirtualVoidMethod( object, getMyClass(), mID );
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::resetGraphics()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetMethodID( getMyClass(), "resetGraphics", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			t.pEnv->CallNonvirtualVoidMethod( object, getMyClass(), mID );
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::setLineAntialiasing( sal_Bool _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Z)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setLineAntialiasing", cSignature );
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

void com_sun_star_vcl_VCLGraphics::setPixel( long _par0, long _par1, SalColor _par2 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(III)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setPixel", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[3];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::setXORMode( sal_Bool _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Z)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setXORMode", cSignature );
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

void com_sun_star_vcl_VCLGraphics::unionClipRegion( long _par0, long _par1, long _par2, long _par3 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(IIII)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "unionClipRegion", cSignature );
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
