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
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
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

#include "salogl_cocoa.h"

#define DEF_LIB "/System/Library/Frameworks/OpenGL.framework/OpenGL"

using namespace rtl;
using namespace vcl;
using namespace vos;

static OModule aModule;

// ========================================================================

void *SalOpenGL::mpNativeContext = NULL;

// ------------------------------------------------------------------------

ULONG SalOpenGL::mnOGLState = OGL_STATE_UNLOADED;

// ------------------------------------------------------------------------

SalOpenGL::SalOpenGL( SalGraphics* pGraphics ) :
	mpBits( NULL ),
	mpData( NULL ),
	mpImage( NULL ),
	mpGraphics( NULL )
{
}

// ------------------------------------------------------------------------

SalOpenGL::~SalOpenGL()
{
	if ( mpImage )
	{
		mpImage->dispose();
		delete mpImage;
		mpImage = NULL;
	}
}

// ------------------------------------------------------------------------

BOOL SalOpenGL::Create()
{
#ifdef OGL_ENABLED
	if ( mnOGLState == OGL_STATE_UNLOADED )
	{
		mnOGLState = OGL_STATE_INVALID;
		if ( aModule.load( OUString::createFromAscii( DEF_LIB ) ) )
		{
			mpNativeContext = NSOpenGLContext_create();
			if ( mpNativeContext )
				mnOGLState = OGL_STATE_VALID;
		}
	}
#endif	// OGL_ENABLED

	return ( mnOGLState == OGL_STATE_VALID ? TRUE : FALSE );
}

// ------------------------------------------------------------------------

void SalOpenGL::Release()
{
	if ( mpNativeContext )
	{
		NSOpenGLContext_release( mpNativeContext );
		mpNativeContext = NULL;
	}

	aModule.unload();
	mnOGLState = OGL_STATE_UNLOADED;
}

// ------------------------------------------------------------------------

void *SalOpenGL::GetOGLFnc( const char* pFncName )
{
	void *pRet = NULL;

	if ( mnOGLState == OGL_STATE_VALID )
		pRet = aModule.getSymbol( OUString::createFromAscii( pFncName ) );

	return pRet;
}

// ------------------------------------------------------------------------

void SalOpenGL::OGLEntry( SalGraphics* pGraphics )
{
}

// ------------------------------------------------------------------------

void SalOpenGL::OGLExit( SalGraphics* pGraphics )
{
}

// ------------------------------------------------------------------------

void SalOpenGL::StartScene( SalGraphics* pGraphics )
{
	NSOpenGLContext_clearDrawable( mpNativeContext );

	mpGraphics = pGraphics;

	if ( !mpImage )
		mpImage = mpGraphics->maGraphicsData.mpVCLGraphics->createImage();

	if ( mpImage )
	{
		mpData = mpImage->getData();
		if ( mpData )
		{
			VCLThreadAttach t;
			if ( t.pEnv )
			{
				long nWidth = mpImage->getWidth();
				long nHeight = mpImage->getHeight();
				jboolean bCopy( sal_False );
				mpBits = (BYTE *)t.pEnv->GetPrimitiveArrayCritical( (jintArray)mpData->getJavaObject(), &bCopy );
				if ( mpBits )
					NSOpenGLContext_setOffScreen( mpNativeContext, mpBits, nWidth, nHeight, nWidth * 4 );
			}
		}
	}

	NSOpenGLContext_makeCurrentContext( mpNativeContext );
}       
            
// ------------------------------------------------------------------------

void SalOpenGL::StopScene()
{
	NSOpenGLContext_flushBuffer( mpNativeContext );
	NSOpenGLContext_clearDrawable( mpNativeContext );

	if ( mpData )
	{
		if ( mpBits )
		{
			VCLThreadAttach t;
			if ( t.pEnv )
				t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)mpData->getJavaObject(), (jint *)mpBits, 0 );
		}
		delete mpData;
	}
	mpBits = NULL;
	mpData = NULL;

	if ( mpImage )
	{
		com_sun_star_vcl_VCLGraphics *pGraphics = mpImage->getGraphics();
		if ( pGraphics )
		{
			long nWidth = mpImage->getWidth();
			long nHeight = mpImage->getHeight();
			mpGraphics->maGraphicsData.mpVCLGraphics->copyBits( pGraphics, 0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight );
			delete pGraphics;
		}
	}
}
