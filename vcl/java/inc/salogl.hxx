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

#ifndef _SV_SALOGL_HXX
#define _SV_SALOGL_HXX

#ifndef _SV_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif
#ifndef _SV_SV_H
#include <sv.h>
#endif

// -----------------
// - State defines -
// -----------------

#define OGL_STATE_UNLOADED		(0x00000000)
#define OGL_STATE_INVALID		(0x00000001)
#define	OGL_STATE_VALID			(0x00000002)

// -------------
// - SalOpenGL -
// -------------

class SalGraphics;
class String;

class SalOpenGL
{
private:
	static void*		mpNativeContext;
	static ULONG        mnOGLState;
	BYTE*				mpBits;
	::vcl::java_lang_Object*	mpData;

public:					
						SalOpenGL( SalGraphics* pGraphics );
						~SalOpenGL();
						
	BOOL	    		Create();
	static void			Release();
	static ULONG		GetState() { return SalOpenGL::mnOGLState; }
	static BOOL			IsValid() { return ( OGL_STATE_VALID == SalOpenGL::mnOGLState ); } 
						
	static void*		GetOGLFnc( const String& rFncName );
    static void*        GetOGLFnc( const char* pFncName );
						
	void    			OGLEntry( SalGraphics* pGraphics );
	void    			OGLExit( SalGraphics* pGraphics );
};

#endif // _SV_SALOGL_HXX
