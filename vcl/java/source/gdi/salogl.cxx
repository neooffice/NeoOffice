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

#ifndef _SV_SALOGL_H
#include <salogl.h>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALVD_H
#include <salvd.h>
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

void *JavaSalOpenGL::mpNativeContext = NULL;

// ------------------------------------------------------------------------

ULONG JavaSalOpenGL::mnOGLState = OGL_STATE_UNLOADED;

// ------------------------------------------------------------------------

JavaSalOpenGL::JavaSalOpenGL() :
	mpBits( NULL ),
	mpData( NULL ),
	mpImage( NULL ),
	mpGraphics( NULL )
{
}

// ------------------------------------------------------------------------

JavaSalOpenGL::~JavaSalOpenGL()
{
	if ( mpImage )
	{
		mpImage->dispose();
		delete mpImage;
		mpImage = NULL;
	}
}

// ------------------------------------------------------------------------

bool JavaSalOpenGL::IsValid()
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

void *JavaSalOpenGL::GetOGLFnc( const char* pFncName )
{
	void *pRet = NULL;

	if ( mnOGLState == OGL_STATE_VALID )
		pRet = aModule.getSymbol( OUString::createFromAscii( pFncName ) );

	return pRet;
}

// ------------------------------------------------------------------------

void JavaSalOpenGL::OGLEntry( SalGraphics* pGraphics )
{
}

// ------------------------------------------------------------------------

void JavaSalOpenGL::OGLExit( SalGraphics* pGraphics )
{
}

// ------------------------------------------------------------------------

void JavaSalOpenGL::StartScene( SalGraphics* pGraphics )
{
	NSOpenGLContext_clearDrawable( mpNativeContext );

	mpGraphics = (JavaSalGraphics *)pGraphics;

	if ( !mpImage )
		mpImage = mpGraphics->mpVCLGraphics->createImage();

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

void JavaSalOpenGL::StopScene()
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
			mpGraphics->mpVCLGraphics->copyBits( pGraphics, 0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight );
			delete pGraphics;
		}
	}
}
