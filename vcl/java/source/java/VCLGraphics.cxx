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

#include <list>
#include <map>

#ifndef _SV_SALBMP_H
#include <salbmp.h>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
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

#include "VCLGraphics_cocoa.h"

static ::std::list< CGImageRef > aCGImageList;
static ::osl::Mutex aBitmapBufferMutex;
static ::std::map< BitmapBuffer*, USHORT > aBitmapBufferMap;
static ::std::list< jlong > aEPSDataList;
static ::std::map< ATSFontRef, CGFontRef > aATSFontMap;
static ::osl::Mutex aATSFontMutex;

using namespace osl;
using namespace rtl;
using namespace vcl;

// ============================================================================

static void ReleaseBytePointerCallback( void *pInfo, const void *pPointer, size_t nSize )
{
	BYTE *pBits = (BYTE *)pPointer;
	if ( pBits )
		delete[] pBits;
}

// ----------------------------------------------------------------------------
 
static void ReleaseBitmapBufferCallback( void *pInfo, const void *pPointer, size_t nSize )
{
	BitmapBuffer *pBuffer = (BitmapBuffer *)pInfo;
	if ( pBuffer )
	{
		MutexGuard aGuard( aBitmapBufferMutex );
		::std::map< BitmapBuffer*, USHORT >::iterator it = aBitmapBufferMap.find( pBuffer );
		if ( it != aBitmapBufferMap.end() )
		{
			it->second--;
			if ( !it->second )
			{
				aBitmapBufferMap.erase( it );
				if ( pBuffer->mpBits )
					delete[] pBuffer->mpBits;
				delete pBuffer;
			}
		}
	}
}

// ----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_com_sun_star_vcl_VCLGraphics_drawBitmap0( JNIEnv *pEnv, jobject object, jintArray _par0, jint _par1, jint _par2, jint _par3, jint _par4, jint _par5, jint _par6, jfloat _par7, jfloat _par8, jfloat _par9, jfloat _par10, jfloat _par11, jfloat _par12, jfloat _par13, jfloat _par14, jboolean _par15, jfloat _par16, jfloat _par17, jfloat _par18, jfloat _par19, jfloat _par20 )
{
	if ( !_par0 )
		return;

	float fScaleX = _par9 / _par5;
	float fScaleY = _par10 / _par6;

	// Adjust for negative source origin
	if ( _par3 < 0 )
	{
		_par7 -= fScaleX * _par3;
		_par9 += fScaleX * _par3;
		_par5 += _par3;
		_par3 = 0;
	}
	if ( _par4 < 0 )
	{
		_par8 -= fScaleY * _par4;
		_par10 += fScaleY * _par4;
		_par6 += _par4;
		_par4 = 0;
	}
	
	// Adjust for size outside of the source image
	jint nExtraWidth = _par3 + _par5 - _par1;
	if ( nExtraWidth > 0 )
	{
		_par9 -= fScaleX * nExtraWidth;
		_par5 -= nExtraWidth;
	}
	jint nExtraHeight = _par4 + _par6 - _par2;
	if ( nExtraHeight > 0 )
	{
		_par10 -= fScaleY * nExtraHeight;
		_par6 -= nExtraHeight;
	}

	jboolean bCopy( sal_False );
	jint *pJavaBits = (jint *)pEnv->GetPrimitiveArrayCritical( _par0, &bCopy );
	if ( !pJavaBits )
		return;

	size_t nRowSize = _par5 * sizeof( jint );
	size_t nSize = nRowSize * _par6;
	jint *pCGBits = new jint[ _par5 * _par6 ];
	if ( pCGBits )
	{
		// Copy the subimage
		jint *pBitsIn = pJavaBits + ( _par4 * _par1 ) + _par3;
		jint *pBitsOut = pCGBits;
		for ( jint i = 0; i < _par6; i++ )
		{
			memcpy( pBitsOut, pBitsIn, nRowSize );
			pBitsIn += _par1;
			pBitsOut += _par5;
		}

		pEnv->ReleasePrimitiveArrayCritical( _par0, pJavaBits, JNI_ABORT );
	}
	else
	{
		pEnv->ReleasePrimitiveArrayCritical( _par0, pJavaBits, JNI_ABORT );
		return;
	}

	CGDataProviderRef aProvider = CGDataProviderCreateWithData( NULL, pCGBits, nSize, ReleaseBytePointerCallback );
	if ( !aProvider )
	{
		delete[] pCGBits;
		return;
	}

	CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
	if ( !aColorSpace )
	{
		CGDataProviderRelease( aProvider );
		return;
	}

#ifdef POWERPC
	CGImageRef aImage = CGImageCreate( _par5, _par6, 8, sizeof( jint ) * 8, nRowSize, aColorSpace, kCGImageAlphaPremultipliedFirst, aProvider, NULL, false, kCGRenderingIntentDefault );
#else	// POWERPC
	CGImageRef aImage = CGImageCreate( _par5, _par6, 8, sizeof( jint ) * 8, nRowSize, aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little, aProvider, NULL, false, kCGRenderingIntentDefault );
#endif	// POWERPC
	CGColorSpaceRelease( aColorSpace );
	CGDataProviderRelease( aProvider );

	if ( aImage )
	{
		aCGImageList.push_back( aImage );
		CGImageRef_drawInRect( aImage, _par7, _par8, _par9, _par10, _par11, _par12, _par13, _par14, _par15, _par16, _par17, _par18, _par19, _par20 );
	}
}

// ----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_com_sun_star_vcl_VCLGraphics_drawBitmapBuffer0( JNIEnv *pEnv, jobject object, jlong _par0, jint _par1, jint _par2, jint _par3, jint _par4, jfloat _par5, jfloat _par6, jfloat _par7, jfloat _par8, jfloat _par9, jfloat _par10, jfloat _par11, jfloat _par12, jboolean _par13, jfloat _par14, jfloat _par15, jfloat _par16, jfloat _par17, jfloat _par18 )
{
	BitmapBuffer *pBuffer = (BitmapBuffer *)_par0;
	if ( !pBuffer )
		return;

	if ( !pBuffer->mpBits )
	{
		delete pBuffer;
		return;
	}

	float fScaleX = _par7 / _par3;
	float fScaleY = _par8 / _par4;

	// Adjust for negative source origin
	if ( _par1 < 0 )
	{
		_par5 -= fScaleX * _par1;
		_par7 += fScaleX * _par1;
		_par3 += _par1;
		_par1 = 0;
	}
	if ( _par2 < 0 )
	{
		_par6 -= fScaleY * _par2;
		_par8 += fScaleY * _par2;
		_par4 += _par2;
		_par2 = 0;
	}
	
	// Adjust for size outside of the source image
	long nWidth = pBuffer->mnWidth;
	long nHeight = pBuffer->mnHeight;
	jint nExtraWidth = _par1 + _par3 - nWidth;
	if ( nExtraWidth > 0 )
	{
		_par7 -= fScaleX * nExtraWidth;
		_par3 -= nExtraWidth;
	}
	jint nExtraHeight = _par2 + _par4 - nHeight;
	if ( nExtraHeight > 0 )
	{
		_par8 -= fScaleY * nExtraHeight;
		_par4 -= nExtraHeight;
	}

	BYTE *pCGBits = pBuffer->mpBits + ( _par2 * pBuffer->mnScanlineSize ) + ( _par1 * sizeof( jint ) );
	CGDataProviderRef aProvider = CGDataProviderCreateWithData( pBuffer, pCGBits, pBuffer->mnScanlineSize * ( _par4 - _par2 ), ReleaseBitmapBufferCallback );
	if ( !aProvider )
	{
		delete[] pBuffer->mpBits;
		delete pBuffer;
		return;
	}

	ClearableMutexGuard aGuard( aBitmapBufferMutex );
	::std::map< BitmapBuffer*, USHORT >::iterator it = aBitmapBufferMap.find( pBuffer );
	if ( it != aBitmapBufferMap.end() )
		it->second++;
	else
		aBitmapBufferMap[ pBuffer ] = 1;
	aGuard.clear();

	CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
	if ( !aColorSpace )
	{
		CGDataProviderRelease( aProvider );
		return;
	}

#ifdef POWERPC
	CGImageRef aImage = CGImageCreate( _par3, _par4, 8, sizeof( jint ) * 8, pBuffer->mnScanlineSize, aColorSpace, kCGImageAlphaPremultipliedFirst, aProvider, NULL, false, kCGRenderingIntentDefault );
#else	// POWERPC
	CGImageRef aImage = CGImageCreate( _par3, _par4, 8, sizeof( jint ) * 8, pBuffer->mnScanlineSize, aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little, aProvider, NULL, false, kCGRenderingIntentDefault );
#endif	// POWERPC
	CGColorSpaceRelease( aColorSpace );
	CGDataProviderRelease( aProvider );

	if ( aImage )
	{
		aCGImageList.push_back( aImage );
		CGImageRef_drawInRect( aImage, _par5, _par6, _par7, _par8, _par9, _par10, _par11, _par12, _par13, _par14, _par15, _par16, _par17, _par18 );
	}
}

// ----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_com_sun_star_vcl_VCLGraphics_notifyGraphicsChanged( JNIEnv *pEnv, jobject object, jlong _par0 )
{
	JavaSalBitmap *pBitmap = (JavaSalBitmap *)_par0;
	if ( pBitmap )
	{
		SalData *pSalData = GetSalData();
		for ( ::std::list< JavaSalBitmap* >::const_iterator it = pSalData->maBitmapList.begin(); it != pSalData->maBitmapList.end(); ++it )
        {
            if ( *it == pBitmap )
			{
				pBitmap->NotifyGraphicsChanged();
				return;
			}
        }

	}
}

// ----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_com_sun_star_vcl_VCLGraphics_releaseNativeBitmaps( JNIEnv *pEnv, jobject object )
{
	while ( aCGImageList.size() )
	{
		CGImageRelease( aCGImageList.front() );
		aCGImageList.pop_front();
	}

	while ( aEPSDataList.size() )
	{
		rtl_freeMemory( (void *)aEPSDataList.front() );
		aEPSDataList.pop_front();
	}
}

// ----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_com_sun_star_vcl_VCLGraphics_drawEPS0( JNIEnv *pEnv, jobject object, jlong _par0, jlong _par1, jfloat _par2, jfloat _par3, jfloat _par4, jfloat _par5, jfloat _par6, jfloat _par7, jfloat _par8, jfloat _par9, jboolean _par10, jfloat _par11, jfloat _par12, jfloat _par13, jfloat _par14, jfloat _par15 )
{
	if ( _par0 )
	{
		bool bFound = false;
		for ( ::std::list< jlong >::const_iterator it = aEPSDataList.begin(); it != aEPSDataList.end(); ++it )
		{
			if ( *it == _par0 )
			{
				bFound = true;
				break;
			}
		}

		if ( !bFound )
			aEPSDataList.push_back( _par0 );

		NSEPSImageRep_drawInRect( (void *)_par0, _par1, _par2, _par3, _par4, _par5, _par6, _par7, _par8, _par9, _par10, _par11, _par12, _par13, _par14, _par15 );
	}
}

// ----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_com_sun_star_vcl_VCLGraphics_drawLine0( JNIEnv *pEnv, jobject object, jfloat _par0, jfloat _par1, jfloat _par2, jfloat _par3, jint _par4, jfloat _par5, jfloat _par6, jfloat _par7, jfloat _par8, jboolean _par9, jfloat _par10, jfloat _par11, jfloat _par12, jfloat _par13, jfloat _par14 )
{
	CGContext_drawLine( _par0, _par1, _par2, _par3, _par4, _par5, _par6, _par7, _par8, _par9, _par10, _par11, _par12, _par13, _par14 );
}

// ----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_com_sun_star_vcl_VCLGraphics_drawPolygon0( JNIEnv *pEnv, jobject object, jint _par0, jintArray _par1, jintArray _par2, jint _par3, jboolean _par4, jfloat _par5, jfloat _par6, jfloat _par7, jfloat _par8, jboolean _par9, jfloat _par10, jfloat _par11, jfloat _par12, jfloat _par13, jfloat _par14 )
{
	if ( _par0 > 0 && _par1 && pEnv->GetArrayLength( _par1 ) >= _par0 && _par2 && pEnv->GetArrayLength( _par2 ) >= _par0  )
	{
		float aXPoints[ _par0 ];
		float aYPoints[ _par0 ];

		jboolean bCopy;
		bCopy = sal_False;
		jint *pXBits = (jint *)pEnv->GetPrimitiveArrayCritical( _par1, &bCopy );
		bCopy = sal_False;
		jint *pYBits = (jint *)pEnv->GetPrimitiveArrayCritical( _par2, &bCopy );
		for ( jint i = 0; i < _par0; i++ )
		{
			aXPoints[ i ] = (float)pXBits[ i ];
			aYPoints[ i ] = (float)pYBits[ i ];
		}
		pEnv->ReleasePrimitiveArrayCritical( _par2, pYBits, 0 );
		pEnv->ReleasePrimitiveArrayCritical( _par1, pXBits, 0 );
		
		CGContext_drawPolygon( _par0, aXPoints, aYPoints, _par3, _par4, _par5, _par6, _par7, _par8, _par9, _par10, _par11, _par12, _par13, _par14 );
	}
}

// ----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_com_sun_star_vcl_VCLGraphics_drawPolyline0( JNIEnv *pEnv, jobject object, jint _par0, jintArray _par1, jintArray _par2, jint _par3, jfloat _par4, jfloat _par5, jfloat _par6, jfloat _par7, jboolean _par8, jfloat _par9, jfloat _par10, jfloat _par11, jfloat _par12, jfloat _par13 )
{
	if ( _par0 > 0 && _par1 && pEnv->GetArrayLength( _par1 ) >= _par0 && _par2 && pEnv->GetArrayLength( _par2 ) >= _par0  )
	{
		float aXPoints[ _par0 ];
		float aYPoints[ _par0 ];

		jboolean bCopy;
		bCopy = sal_False;
		jint *pXBits = (jint *)pEnv->GetPrimitiveArrayCritical( _par1, &bCopy );
		bCopy = sal_False;
		jint *pYBits = (jint *)pEnv->GetPrimitiveArrayCritical( _par2, &bCopy );
		for ( jint i = 0; i < _par0; i++ )
		{
			aXPoints[ i ] = (float)pXBits[ i ];
			aYPoints[ i ] = (float)pYBits[ i ];
		}
		pEnv->ReleasePrimitiveArrayCritical( _par2, pYBits, 0 );
		pEnv->ReleasePrimitiveArrayCritical( _par1, pXBits, 0 );
		
		CGContext_drawPolyline( _par0, aXPoints, aYPoints, _par3, _par4, _par5, _par6, _par7, _par8, _par9, _par10, _par11, _par12, _par13 );
	}
}

// ----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_com_sun_star_vcl_VCLGraphics_drawRect0( JNIEnv *pEnv, jobject object, jfloat _par0, jfloat _par1, jfloat _par2, jfloat _par3, jint _par4, jboolean _par5, jfloat _par6, jfloat _par7, jfloat _par8, jfloat _par9, jboolean _par10, jfloat _par11, jfloat _par12, jfloat _par13, jfloat _par14, jfloat _par15 )
{
	CGContext_drawRect( _par0, _par1, _par2, _par3, _par4, _par5, _par6, _par7, _par8, _par9, _par10, _par11, _par12, _par13, _par14, _par15 );
}

// ----------------------------------------------------------------------------

CGFontRef CreateCachedCGFont( ATSFontRef aATSFont )
{
	CGFontRef aFont = NULL;

	MutexGuard aGuard( aATSFontMutex );

	::std::map< ATSFontRef, CGFontRef >::iterator it = aATSFontMap.find( aATSFont );
	if ( it != aATSFontMap.end() )
	{
		aFont = it->second;
	}
	else
	{
		aFont = CGFontCreateWithPlatformFont( (void *)&aATSFont );
		if ( aFont )
			aATSFontMap[ aATSFont ] = aFont;
	}

	if ( aFont )
		CGFontRetain( aFont );

	return aFont;
}

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

		if ( tempClass )
		{
			// Register the native methods for our class
			JNINativeMethod pMethods[9]; 
			pMethods[0].name = "drawBitmap0";
			pMethods[0].signature = "([IIIIIIIFFFFFFFFZFFFFF)V";
			pMethods[0].fnPtr = (void *)Java_com_sun_star_vcl_VCLGraphics_drawBitmap0;
			pMethods[1].name = "drawBitmapBuffer0";
			pMethods[1].signature = "(JIIIIFFFFFFFFZFFFFF)V";
			pMethods[1].fnPtr = (void *)Java_com_sun_star_vcl_VCLGraphics_drawBitmapBuffer0;
			pMethods[2].name = "drawEPS0";
			pMethods[2].signature = "(JJFFFFFFFFZFFFFF)V";
			pMethods[2].fnPtr = (void *)Java_com_sun_star_vcl_VCLGraphics_drawEPS0;
			pMethods[3].name = "drawLine0";
			pMethods[3].signature = "(FFFFIFFFFZFFFFF)V";
			pMethods[3].fnPtr = (void *)Java_com_sun_star_vcl_VCLGraphics_drawLine0;
			pMethods[4].name = "drawPolygon0";
			pMethods[4].signature = "(I[I[IIZFFFFZFFFFF)V";
			pMethods[4].fnPtr = (void *)Java_com_sun_star_vcl_VCLGraphics_drawPolygon0;
			pMethods[5].name = "drawPolyline0";
			pMethods[5].signature = "(I[I[IIFFFFZFFFFF)V";
			pMethods[5].fnPtr = (void *)Java_com_sun_star_vcl_VCLGraphics_drawPolyline0;
			pMethods[6].name = "drawRect0";
			pMethods[6].signature = "(FFFFIZFFFFZFFFFF)V";
			pMethods[6].fnPtr = (void *)Java_com_sun_star_vcl_VCLGraphics_drawRect0;
			pMethods[7].name = "notifyGraphicsChanged";
			pMethods[7].signature = "(J)V";
			pMethods[7].fnPtr = (void *)Java_com_sun_star_vcl_VCLGraphics_notifyGraphicsChanged;
			pMethods[8].name = "releaseNativeBitmaps";
			pMethods[8].signature = "()V";
			pMethods[8].fnPtr = (void *)Java_com_sun_star_vcl_VCLGraphics_releaseNativeBitmaps;
			t.pEnv->RegisterNatives( tempClass, pMethods, 9 );
		}

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

void com_sun_star_vcl_VCLGraphics::addGraphicsChangeListener( JavaSalBitmap *_par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(J)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "addGraphicsChangeListener", cSignature );
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

void com_sun_star_vcl_VCLGraphics::copyBits( const com_sun_star_vcl_VCLGraphics *_par0, long _par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8, sal_Bool _par9 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Lcom/sun/star/vcl/VCLGraphics;IIIIIIIIZ)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "copyBits", cSignature );
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
			args[9].z = jboolean( _par9 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::copyBits( BYTE *_par0, long _par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8, long _par9 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Ljava/nio/ByteBuffer;IIIIIIIII)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "copyBits", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject byteBuffer = t.pEnv->NewDirectByteBuffer( _par0, _par1 );
			if ( byteBuffer )
			{
				jvalue args[10];
				args[0].l = byteBuffer;
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
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawBitmap( const com_sun_star_vcl_VCLBitmap *_par0, long _par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8, sal_Bool _par9 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Lcom/sun/star/vcl/VCLBitmap;IIIIIIIIZ)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawBitmap", cSignature );
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
			args[9].z = jboolean( _par9 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawBitmapBuffer( BitmapBuffer *_par0, long _par1, long _par2, long _par3, long _par4, long _par5, long _par6, long _par7, long _par8 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(JIIIIIIII)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawBitmapBuffer", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[9];
			args[0].j = jlong( _par0 );
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

void com_sun_star_vcl_VCLGraphics::drawGlyphs( long _par0, long _par1, int _par2, long *_par3, long *_par4, com_sun_star_vcl_VCLFont *_par5, SalColor _par6, int _par7, int _par8, long _par9, long _par10, float _par11 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(II[I[ILcom/sun/star/vcl/VCLFont;IIIIIF)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawGlyphs", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jboolean bCopy;
			jsize elements( _par2 );
			jintArray glypharray = t.pEnv->NewIntArray( elements );
			bCopy = sal_False;
			jint *pGlyphBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( glypharray, &bCopy );
			memcpy( pGlyphBits, (jint *)_par3, elements * sizeof( jint ) );
			t.pEnv->ReleasePrimitiveArrayCritical( glypharray, pGlyphBits, 0 );
			jintArray advancearray = t.pEnv->NewIntArray( elements );
			bCopy = sal_False;
			jint *pAdvanceBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( advancearray, &bCopy );
			memcpy( pAdvanceBits, (jint *)_par4, elements * sizeof( jint ) );
			t.pEnv->ReleasePrimitiveArrayCritical( advancearray, pAdvanceBits, 0 );

			jvalue args[11];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].l = glypharray;
			args[3].l = advancearray;
			args[4].l = _par5->getJavaObject();
			args[5].i = jint( _par6 );
			args[6].i = jint( _par7 );
			args[7].i = jint( _par8 );
			args[8].i = jint( _par9 );
			args[9].i = jint( _par10 );
			args[10].f = jfloat( _par11 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawEPS( void *_par0, long _par1, long _par2, long _par3, long _par4, long _par5 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(JJIIII)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawEPS", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[6];
			args[0].j = jlong( _par0 );
			args[1].j = jlong( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].i = jint( _par4 );
			args[5].i = jint( _par5 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
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

void com_sun_star_vcl_VCLGraphics::drawPolygon( ULONG _par0, const SalPoint *_par1, SalColor _par2, sal_Bool _par3 )
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
			jboolean bCopy;
			jsize elements( _par0 );
			jintArray xarray = t.pEnv->NewIntArray( elements );
			jintArray yarray = t.pEnv->NewIntArray( elements );
			bCopy = sal_False;
			jint *pXBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( xarray, &bCopy );
			bCopy = sal_False;
			jint *pYBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( yarray, &bCopy );
			for ( jsize i = 0; i < elements; i++ )
			{
				pXBits[ i ] = _par1[ i ].mnX;
				pYBits[ i ] = _par1[ i ].mnY;
			}
			t.pEnv->ReleasePrimitiveArrayCritical( yarray, pYBits, 0 );
			t.pEnv->ReleasePrimitiveArrayCritical( xarray, pXBits, 0 );

			jvalue args[5];
			args[0].i = jint( _par0 );
			args[1].l = xarray;
			args[2].l = yarray;
			args[3].i = jint( _par2 );
			args[4].z = jboolean( _par3 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawPolyline( ULONG _par0, const SalPoint *_par1, SalColor _par2 )
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
			jboolean bCopy;
			jsize elements( _par0 );
			jintArray xarray = t.pEnv->NewIntArray( elements );
			jintArray yarray = t.pEnv->NewIntArray( elements );
			bCopy = sal_False;
			jint *pXBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( xarray, &bCopy );
			bCopy = sal_False;
			jint *pYBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( yarray, &bCopy );
			for ( jsize i = 0; i < elements; i++ )
			{
				pXBits[ i ] = _par1[ i ].mnX;
				pYBits[ i ] = _par1[ i ].mnY;
			}
			t.pEnv->ReleasePrimitiveArrayCritical( yarray, pYBits, 0 );
			t.pEnv->ReleasePrimitiveArrayCritical( xarray, pXBits, 0 );

			jvalue args[4];
			args[0].i = jint( _par0 );
			args[1].l = xarray;
			args[2].l = yarray;
			args[3].i = jint( _par2 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawPolyPolygon( ULONG _par0, const ULONG *_par1, PCONSTSALPOINT *_par2, SalColor _par3, sal_Bool _par4 )
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
			jboolean bCopy;
			jsize elements( _par0 );
			jintArray ptsarray = t.pEnv->NewIntArray( elements );
			bCopy = sal_False;
			jint *pPtsBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( ptsarray, &bCopy );
			memcpy( pPtsBits, (jint *)_par1, elements * sizeof( jint ) );
			t.pEnv->ReleasePrimitiveArrayCritical( ptsarray, pPtsBits, 0 );
			
			jintArray tempArray = t.pEnv->NewIntArray( 0 );
			jobjectArray xptsarray = t.pEnv->NewObjectArray( elements, tempClass, tempArray );
			jobjectArray yptsarray = t.pEnv->NewObjectArray( elements, tempClass, tempArray );
			for ( jsize i = 0; i < elements; i++ )
			{
				jsize points( _par1[ i ] );
				const SalPoint *pPts = _par2[ i ];
				jintArray xarray = t.pEnv->NewIntArray( points );
				jintArray yarray = t.pEnv->NewIntArray( points );
				bCopy = sal_False;
				jint *pXBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( xarray, &bCopy );
				bCopy = sal_False;
				jint *pYBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( yarray, &bCopy );
				for ( jsize j = 0; j < points; j++ )
				{
					pXBits[ j ] = pPts[ j ].mnX;
					pYBits[ j ] = pPts[ j ].mnY;
				}
				t.pEnv->ReleasePrimitiveArrayCritical( yarray, pYBits, 0 );
				t.pEnv->ReleasePrimitiveArrayCritical( xarray, pXBits, 0 );
				t.pEnv->SetObjectArrayElement( yptsarray, i, yarray );
				t.pEnv->SetObjectArrayElement( xptsarray, i, xarray );
			}

			jvalue args[6];
			args[0].i = jint( _par0 );
			args[1].l = ptsarray;
			args[2].l = xptsarray;
			args[3].l = yptsarray;
			args[4].i = jint( _par3 );
			args[5].z = jboolean( _par4 );
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

void com_sun_star_vcl_VCLGraphics::drawPushButton( long _par0, long _par1, long _par2, long _par3, ::rtl::OUString _par4, sal_Bool _par5, sal_Bool _par6, sal_Bool _par7, sal_Bool _par8 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(IIIILjava/lang/String;ZZZZ)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawPushButton", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[9];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].l = StringToJavaString( t.pEnv, _par4 );
			args[5].z = jboolean( _par5 );
			args[6].z = jboolean( _par6 );
			args[7].z = jboolean( _par7 );
			args[8].z = jboolean( _par8 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

const Rectangle com_sun_star_vcl_VCLGraphics::getPreferredPushButtonBounds( long _par0, long _par1, long _par2, long _par3, ::rtl::OUString _par4 )
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
			char *cSignature = "(IIIILjava/lang/String;)Ljava/awt/Rectangle;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getPreferredPushButtonBounds", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[5];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].l = StringToJavaString( t.pEnv, _par4 );
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

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawRadioButton( long _par0, long _par1, long _par2, long _par3, ::rtl::OUString _par4, sal_Bool _par5, sal_Bool _par6, sal_Bool _par7, long _par8 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(IIIILjava/lang/String;ZZZI)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawRadioButton", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[9];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].l = StringToJavaString( t.pEnv, _par4 );
			args[5].z = jboolean( _par5 );
			args[6].z = jboolean( _par6 );
			args[7].z = jboolean( _par7 );
			args[8].i = jint( _par8 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

const Rectangle com_sun_star_vcl_VCLGraphics::getPreferredRadioButtonBounds( long _par0, long _par1, long _par2, long _par3, ::rtl::OUString _par4 )
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
			char *cSignature = "(IIIILjava/lang/String;)Ljava/awt/Rectangle;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getPreferredRadioButtonBounds", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[5];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].l = StringToJavaString( t.pEnv, _par4 );
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

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::drawCheckBox( long _par0, long _par1, long _par2, long _par3, ::rtl::OUString _par4, sal_Bool _par5, sal_Bool _par6, sal_Bool _par7, long _par8 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(IIIILjava/lang/String;ZZZI)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "drawCheckBox", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[9];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].l = StringToJavaString( t.pEnv, _par4 );
			args[5].z = jboolean( _par5 );
			args[6].z = jboolean( _par6 );
			args[7].z = jboolean( _par7 );
			args[8].i = jint( _par8 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

const Rectangle com_sun_star_vcl_VCLGraphics::getPreferredCheckBoxBounds( long _par0, long _par1, long _par2, long _par3, ::rtl::OUString _par4 )
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
			char *cSignature = "(IIIILjava/lang/String;)Ljava/awt/Rectangle;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getPreferredCheckBoxBounds", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[5];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			args[2].i = jint( _par2 );
			args[3].i = jint( _par3 );
			args[4].l = StringToJavaString( t.pEnv, _par4 );
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

const Rectangle com_sun_star_vcl_VCLGraphics::getGlyphBounds( int _par0, com_sun_star_vcl_VCLFont *_par1, int _par2 )
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
			char *cSignature = "(ILcom/sun/star/vcl/VCLFont;I)Ljava/awt/Rectangle;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getGlyphBounds", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[3];
			args[0].i = jint( _par0 );
			args[1].l = _par1->getJavaObject();
			args[2].i = jint( _par2 );
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

void com_sun_star_vcl_VCLGraphics::invert( ULONG _par0, const SalPoint *_par1, SalInvert _par2 )
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
			jboolean bCopy;
			jsize elements( _par0 );
			jintArray xarray = t.pEnv->NewIntArray( elements );
			jintArray yarray = t.pEnv->NewIntArray( elements );
			bCopy = sal_False;
			jint *pXBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( xarray, &bCopy );
			bCopy = sal_False;
			jint *pYBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( yarray, &bCopy );
			for ( jsize i = 0; i < elements; i++ )
			{
				pXBits[ i ] = _par1[ i ].mnX;
				pYBits[ i ] = _par1[ i ].mnY;
			}
			t.pEnv->ReleasePrimitiveArrayCritical( yarray, pYBits, 0 );
			t.pEnv->ReleasePrimitiveArrayCritical( xarray, pXBits, 0 );

			jvalue args[4];
			args[0].i = jint( _par0 );
			args[1].l = xarray;
			args[2].l = yarray;
			args[3].i = jint( _par2 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLGraphics::removeGraphicsChangeListener( JavaSalBitmap *_par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(J)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "removeGraphicsChangeListener", cSignature );
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
