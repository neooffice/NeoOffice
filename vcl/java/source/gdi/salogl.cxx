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

// ========================================================================

SalOpenGL::SalOpenGL( SalGraphics* pGraphics )
{
}

// ------------------------------------------------------------------------

SalOpenGL::~SalOpenGL()
{
}

// ------------------------------------------------------------------------

BOOL SalOpenGL::Create()
{
#ifdef DEBUG
	fprintf( stderr, "SalOpenGL::Create not implemented\n" );
#endif
	return FALSE;
}

// ------------------------------------------------------------------------

void SalOpenGL::Release()
{
#ifdef DEBUG
	fprintf( stderr, "SalOpenGL::Release not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void* SalOpenGL::GetOGLFnc( const char* pFncName )
{
#ifdef DEBUG
	fprintf( stderr, "SalOpenGL::GetOGLFnc not implemented\n" );
#endif
	return NULL;
}

// ------------------------------------------------------------------------

void SalOpenGL::OGLEntry( SalGraphics* pGraphics )
{
#ifdef DEBUG
	fprintf( stderr, "SalOpenGL::OGLEntry not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SalOpenGL::OGLExit( SalGraphics* pGraphics )
{
#ifdef DEBUG
	fprintf( stderr, "SalOpenGL::OGLExit not implemented\n" );
#endif
}
