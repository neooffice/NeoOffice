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
 *	 - GNU General Public License Version 2.1
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

#ifndef _SV_SALFRAME_H
#define _SV_SALFRAME_H

#include <list>

#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_SYSDATA_HXX 
#include <sysdata.hxx>
#endif
#ifndef _SV_SALGEOM_HXX
#include <salgeom.hxx>
#endif

namespace vcl
{
class com_sun_star_vcl_VCLEvent;
class com_sun_star_vcl_VCLEventQueue;
class com_sun_star_vcl_VCLFrame;
class com_sun_star_vcl_VCLMenuBar;
class java_lang_Object;
}

class SalBitmap;
class SalMenu;

// ----------------
// - SalFrameData -
// ----------------

class SalFrameData
{
public:
	::vcl::com_sun_star_vcl_VCLFrame*	mpVCLFrame;
	::vcl::java_lang_Object*	mpPanel;
	SalGraphics*	mpGraphics;
	ULONG			mnStyle;
	SalFrame*		mpParent;
	BOOL			mbGraphics;
	BOOL			mbVisible;
	::std::list< SalFrame* > maChildren;
	void*			mpInst;
	SALFRAMEPROC	mpProc;
	SystemEnvData	maSysData;
	BOOL			mbCenter;
	SalFrameGeometry	maOriginalGeometry;
	BOOL			mbFullScreen;
	BOOL			mbPresentation;
	SalMenu*		mpMenuBar;
	BOOL			mbUseMainScreenOnly;
	BOOL			mbInShow;

					SalFrameData();
					~SalFrameData();
};

#endif // _SV_SALFRAME_H
