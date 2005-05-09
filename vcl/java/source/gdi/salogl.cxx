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

#define _SV_SALOGL_CXX

#include <stdio.h>

#ifndef _SV_SALOGL_HXX
#include <salogl.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLIMAGE_HXX
#include <com/sun/star/vcl/VCLImage.hxx>
#endif
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif

#ifdef MACOSX
#define DEF_OGLLIB "/System/Library/Frameworks/OpenGL.framework/OpenGL"
#endif	// MACOSX

#ifdef MACOSX

#include <premac.h>
#include <OpenGL/OpenGL.h>
#include <postmac.h>

typedef CGLError CGLChoosePixelFormat_Type( const CGLPixelFormatAttribute *, CGLPixelFormatObj *, long * );
typedef CGLError CGLClearDrawable_Type( CGLContextObj );
typedef CGLError CGLCreateContext_Type( CGLPixelFormatObj, CGLContextObj, CGLContextObj * );
typedef CGLError CGLDestroyContext_Type( CGLContextObj );
typedef CGLError CGLDestroyPixelFormat_Type( CGLPixelFormatObj );
typedef CGLError CGLSetCurrentContext_Type( CGLContextObj );
typedef CGLError CGLSetOffScreen_Type( CGLContextObj, long, long, long, void * );

static CGLChoosePixelFormat_Type *pChoosePixelFormat = NULL;
static CGLClearDrawable_Type *pClearDrawable = NULL;
static CGLCreateContext_Type *pCreateContext = NULL;
static CGLDestroyContext_Type *pDestroyContext = NULL;
static CGLDestroyPixelFormat_Type *pDestroyPixelFormat = NULL;
static CGLSetCurrentContext_Type *pSetCurrentContext = NULL;
static CGLSetOffScreen_Type *pSetOffScreen = NULL;

#endif	// MACOSX

static ::vos::OModule aModule;

using namespace rtl;
using namespace vcl;
using namespace vos;

// ========================================================================

BYTE *SalOpenGL::mpBits = NULL;

// ------------------------------------------------------------------------

java_lang_Object *SalOpenGL::mpData = NULL;

// ------------------------------------------------------------------------

SalGraphics *SalOpenGL::mpLastGraphics = NULL;

// ------------------------------------------------------------------------

void *SalOpenGL::mpNativeContext = NULL;

// ------------------------------------------------------------------------

ULONG SalOpenGL::mnOGLState = OGL_STATE_UNLOADED;

// ------------------------------------------------------------------------

SalOpenGL::SalOpenGL( SalGraphics* pGraphics ) : mpGraphics( NULL )
{
}

// ------------------------------------------------------------------------

SalOpenGL::~SalOpenGL()
{
	if ( mpGraphics && mpLastGraphics && mpGraphics->maGraphicsData.mpVCLGraphics == mpLastGraphics->maGraphicsData.mpVCLGraphics )
	{
#ifdef MACOSX
		if ( mpNativeContext && pClearDrawable )
			pClearDrawable( (CGLContextObj)mpNativeContext );
#endif	// MACOSX

		if ( mpData )
		{
			if ( mpBits )
			{
				VCLThreadAttach t;
				if ( t.pEnv )
					t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)mpData->getJavaObject(), (jint *)mpBits, JNI_ABORT );
			}
			delete mpData;
		}
		mpBits = NULL;
		mpData = NULL;
		mpLastGraphics = NULL;
	}
}

// ------------------------------------------------------------------------

BOOL SalOpenGL::Create()
{
	if ( mnOGLState == OGL_STATE_UNLOADED )
	{
#ifdef MACOSX
		mnOGLState = OGL_STATE_INVALID;
		if ( aModule.load( OUString::createFromAscii( DEF_OGLLIB ) ) )
		{
			pChoosePixelFormat = (CGLChoosePixelFormat_Type *)aModule.getSymbol( OUString::createFromAscii( "CGLChoosePixelFormat" ) );
			pClearDrawable = (CGLClearDrawable_Type *)aModule.getSymbol( OUString::createFromAscii( "CGLClearDrawable" ) );
			pCreateContext = (CGLCreateContext_Type *)aModule.getSymbol( OUString::createFromAscii( "CGLCreateContext" ) );
			pDestroyContext = (CGLDestroyContext_Type *)aModule.getSymbol( OUString::createFromAscii( "CGLDestroyContext" ) );
			pDestroyPixelFormat = (CGLDestroyPixelFormat_Type *)aModule.getSymbol( OUString::createFromAscii( "CGLDestroyPixelFormat" ) );
			pSetCurrentContext = (CGLSetCurrentContext_Type *)aModule.getSymbol( OUString::createFromAscii( "CGLSetCurrentContext" ) );
			pSetOffScreen = (CGLSetOffScreen_Type *)aModule.getSymbol( OUString::createFromAscii( "CGLSetOffScreen" ) );
			if ( pChoosePixelFormat && pClearDrawable && pCreateContext && pDestroyContext && pDestroyPixelFormat && pSetCurrentContext && pSetOffScreen )
			{

				CGLPixelFormatAttribute pAttributes[ 6 ];
				pAttributes[ 0 ] = kCGLPFAOffScreen;
				pAttributes[ 1 ] = kCGLPFAColorSize;
				pAttributes[ 2 ] = (CGLPixelFormatAttribute)32;
				pAttributes[ 3 ] = kCGLPFAAlphaSize;
				pAttributes[ 4 ] = (CGLPixelFormatAttribute)8;
				pAttributes[ 5 ] = (CGLPixelFormatAttribute)NULL;
				CGLPixelFormatObj aPixelFormat;
				long nPixelFormats;
				pChoosePixelFormat( pAttributes, &aPixelFormat, &nPixelFormats );
				if ( aPixelFormat )
				{
					pCreateContext( aPixelFormat, NULL, (CGLContextObj *)&mpNativeContext );
					pDestroyPixelFormat( aPixelFormat );

					if ( mpNativeContext )
					{
						pSetCurrentContext( (CGLContextObj)mpNativeContext );
						mnOGLState = OGL_STATE_VALID;
					}
				}
			}
		}
#else	// MACOSX
#ifdef DEBUG
		fprintf( stderr, "SalOpenGL::Create not implemented\n" );
#endif
#endif	// MACOSX
	}

	return ( mnOGLState == OGL_STATE_VALID ? TRUE : FALSE );
}

// ------------------------------------------------------------------------

void SalOpenGL::Release()
{
#ifdef MACOSX
	if ( mpNativeContext && pClearDrawable && pDestroyContext )
	{
		pClearDrawable( (CGLContextObj)mpNativeContext );
		pDestroyContext( (CGLContextObj)mpNativeContext );
	}
	mpNativeContext = NULL;

	pChoosePixelFormat = NULL;
	pClearDrawable = NULL;
	pCreateContext = NULL;
	pDestroyContext = NULL;
	pDestroyPixelFormat = NULL;
	pSetCurrentContext = NULL;
	pSetOffScreen = NULL;
#else	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "SalOpenGL::Release not implemented\n" );
#endif
#endif	// MACOSX

	if ( mpData )
	{
		if ( mpBits )
		{
			VCLThreadAttach t;
			if ( t.pEnv )
				t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)mpData->getJavaObject(), (jint *)mpBits, JNI_ABORT );
		}
		delete mpData;
	}
	mpBits = NULL;
	mpData = NULL;
	mpLastGraphics = NULL;

	aModule.unload();
	mnOGLState = OGL_STATE_UNLOADED;
}

// ------------------------------------------------------------------------

void *SalOpenGL::GetOGLFnc( const char* pFncName )
{
	void *pRet = NULL;

	// Fix bug 765 by not returning the glBlendFunc() function
	if ( mnOGLState == OGL_STATE_VALID && strcmp( pFncName, "glBlendFunc" ) )
		pRet = aModule.getSymbol( OUString::createFromAscii( pFncName ) );

	return pRet;
}

// ------------------------------------------------------------------------

void SalOpenGL::OGLEntry( SalGraphics* pGraphics )
{
	mpGraphics = pGraphics;

	if ( !mpGraphics || !mpLastGraphics || mpGraphics->maGraphicsData.mpVCLGraphics != mpLastGraphics->maGraphicsData.mpVCLGraphics )
	{
#ifdef MACOSX
		if ( mpNativeContext && pClearDrawable )
			pClearDrawable( (CGLContextObj)mpNativeContext );
#endif	// MACOSX

		if ( mpData )
		{
			if ( mpBits )
			{
				VCLThreadAttach t;
				if ( t.pEnv )
					t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)mpData->getJavaObject(), (jint *)mpBits, JNI_ABORT );
			}
			delete mpData;
		}
		mpBits = NULL;
		mpData = NULL;
		mpLastGraphics = mpGraphics;

		com_sun_star_vcl_VCLImage *pImage = pGraphics->maGraphicsData.mpVCLGraphics->getImage();
		if ( pImage )
		{
			mpData = pImage->getData();
			if ( mpData )
			{
				VCLThreadAttach t;
				if ( t.pEnv )
				{
					long nWidth = pImage->getWidth();
					long nHeight = pImage->getHeight();
					jboolean bCopy( sal_False );
					mpBits = (BYTE *)t.pEnv->GetPrimitiveArrayCritical( (jintArray)mpData->getJavaObject(), &bCopy );
#ifdef MACOSX
					if ( mpBits && mpNativeContext && pSetOffScreen )
						pSetOffScreen( (CGLContextObj)mpNativeContext, nWidth, nHeight, nWidth * 4, mpBits );
#else	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "SalOpenGL::OGLEntry not implemented\n" );
#endif
#endif	// MACOSX
				}
			}
			delete pImage;
		}
	}

#ifdef MACOSX
	// Fix bug 289 by resetting the current context in each thread
	if ( mpNativeContext )
		pSetCurrentContext( (CGLContextObj)mpNativeContext );
#endif	// MACOSX
}

// ------------------------------------------------------------------------

void SalOpenGL::OGLExit( SalGraphics* pGraphics )
{
	if ( mpData && mpBits )
	{
		VCLThreadAttach t;
		if ( t.pEnv )
			t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)mpData->getJavaObject(), (jint *)mpBits, JNI_COMMIT );
	}
}
