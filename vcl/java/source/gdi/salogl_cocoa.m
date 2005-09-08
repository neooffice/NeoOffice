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
 *  Patrick Luby, July 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 by Patrick Luby (patrick.luby@planamesa.com)
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

#import <Cocoa/Cocoa.h>
#import "salogl_cocoa.h"

void NSOpenGLContext_clearDrawable( id pContext )
{
	if ( pContext )
		[(NSOpenGLContext *)pContext clearDrawable];
}

id NSOpenGLContext_create()
{
	NSOpenGLContext *pContext = nil;

	NSOpenGLPixelFormatAttribute pAttributes[ 8 ];
	pAttributes[ 0 ] = NSOpenGLPFAOffScreen;
	pAttributes[ 1 ] = NSOpenGLPFADepthSize;
	pAttributes[ 2 ] = 32;
	pAttributes[ 3 ] = NSOpenGLPFAAlphaSize;
	pAttributes[ 4 ] = 8;
	pAttributes[ 5 ] = NSOpenGLPFAColorSize;
	pAttributes[ 6 ] = 24;
	pAttributes[ 7 ] = 0;
	NSOpenGLPixelFormat *pFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pAttributes];
	if ( pFormat )
	{
		pContext = [[NSOpenGLContext alloc] initWithFormat:pFormat shareContext:nil];
		[pFormat release];
	}

	return pContext;
}

void NSOpenGLContext_flushBuffer( id pContext )
{
	if ( pContext )
		[(NSOpenGLContext *)pContext flushBuffer];
}

void NSOpenGLContext_makeCurrentContext( id pContext )
{
	if ( pContext )
		[(NSOpenGLContext *)pContext makeCurrentContext];
}

void NSOpenGLContext_release( id pContext )
{
	if ( pContext )
		[(NSOpenGLContext *)pContext release];
}

void NSOpenGLContext_setOffScreen( id pContext, void *pBits, long nWidth, long nHeight, long nScanlineSize )
{
	if ( pContext )
		[(NSOpenGLContext *)pContext setOffScreen:pBits width:nWidth height:nHeight rowbytes:nScanlineSize];
}
