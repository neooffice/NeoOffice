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
 *  Patrick Luby, February 2006
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2006 by Patrick Luby (patrick.luby@planamesa.com)
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

#ifndef _SV_SALOGL_H
#define _SV_SALOGL_H

#ifndef _SV_SALOGL_HXX
#include <salogl.hxx>
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

class BitmapBuffer;
class JavaSalBitmap;
class JavaSalGraphics;

// -----------------
// - JavaSalOpenGL -
// -----------------

class JavaSalOpenGL : public SalOpenGL
{
private:
	static void*			mpNativeContext;
	static ULONG			mnOGLState;

	JavaSalBitmap*			mpBitmap;
	BitmapBuffer*			mpBuffer;
	JavaSalGraphics*		mpGraphics;

public:					
							JavaSalOpenGL();
	virtual					~JavaSalOpenGL();

	virtual bool			IsValid();
	virtual oglFunction		GetOGLFnc( const char* pFncName );
	virtual void			OGLEntry( SalGraphics* pGraphics );
	virtual void			OGLExit( SalGraphics* pGraphics );
	virtual void			StartScene( SalGraphics* pGraphics );
	virtual void			StopScene();
};

#endif // _SV_SALOGL_H
