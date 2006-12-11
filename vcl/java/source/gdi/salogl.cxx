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
#ifndef _SV_SALBMP_H
#include <salbmp.h>
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
#ifndef _SV_COM_SUN_STAR_VCL_VCLFRAME_HXX
#include <com/sun/star/vcl/VCLFrame.hxx>
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
	mpBitmap( NULL ),
	mpBuffer( NULL ),
	mpGraphics( NULL )
{
}

// ------------------------------------------------------------------------

JavaSalOpenGL::~JavaSalOpenGL()
{
	if ( mpBitmap )
	{
		if ( mpBuffer )
			mpBitmap->ReleaseBuffer( mpBuffer, FALSE );
		delete mpBitmap;
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

	if ( !mpBitmap )
	{
		long nWidth = 0;
		long nHeight = 0;
		if ( mpGraphics->mpFrame )
		{
			Rectangle aRect( mpGraphics->mpFrame->mpVCLFrame->getBounds() );
			nWidth = aRect.GetWidth();
			nHeight = aRect.GetHeight();
		}
		else if ( mpGraphics->mpVirDev )
		{
			nWidth = mpGraphics->mpVirDev->mpVCLImage->getWidth();
			nHeight = mpGraphics->mpVirDev->mpVCLImage->getHeight();
		}

		if ( nWidth > 0 && nHeight > 0 )
			mpBitmap = (JavaSalBitmap *)mpGraphics->getBitmap( 0, 0, nWidth, nHeight );
	}

	if ( mpBitmap )
	{
		if ( !mpBuffer )
			mpBuffer = mpBitmap->AcquireBuffer( FALSE );

		if ( mpBuffer && mpBuffer->mpBits )
			NSOpenGLContext_setOffScreen( mpNativeContext, mpBuffer->mpBits, mpBuffer->mnWidth, mpBuffer->mnHeight, mpBuffer->mnScanlineSize );
	}

	NSOpenGLContext_makeCurrentContext( mpNativeContext );
}       
            
// ------------------------------------------------------------------------

void JavaSalOpenGL::StopScene()
{
	NSOpenGLContext_flushBuffer( mpNativeContext );
	NSOpenGLContext_clearDrawable( mpNativeContext );

	if ( mpBitmap )
	{
		if ( mpBuffer )
		{
			SalTwoRect aPosAry;
			aPosAry.mnSrcX = 0;
			aPosAry.mnSrcY = 0;
			aPosAry.mnSrcWidth = mpBuffer->mnWidth;
			aPosAry.mnSrcHeight = mpBuffer->mnHeight;
			aPosAry.mnDestX = 0;
			aPosAry.mnDestY = 0;
			aPosAry.mnDestWidth = mpBuffer->mnWidth;
			aPosAry.mnDestHeight = mpBuffer->mnHeight;

			mpBitmap->ReleaseBuffer( mpBuffer, FALSE );
			mpBuffer = NULL;

			mpGraphics->drawBitmap( &aPosAry, *mpBitmap );
		}

		delete mpBitmap;
		mpBitmap = NULL;
	}
}
