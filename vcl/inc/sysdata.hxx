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
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
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
 *  =================================================
 *  Modified June 2003 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

#ifndef _SV_SYSDATA_HXX
#define _SV_SYSDATA_HXX

#ifdef USE_JAVA
#include <com/sun/star/vcl/VCLFrame.hxx>
#endif

// -----------------
// - SystemEnvData -
// -----------------

struct SystemEnvData
{
	unsigned long		nSize;			// size in bytes of this structure
#ifndef SYSDATA_ONLY_BASETYPE
#if defined( WIN ) || defined( WNT )
	HWND				hWnd;			// the window hwnd
#elif defined( OS2 )
	HWND				hWnd;			// the client hwnd
#elif defined( USE_JAVA )
	::vcl::com_sun_star_vcl_VCLFrame*	pVCLFrame;	// the Java window
#elif defined( UNX )
	void*				pDisplay;		// the relevant display connection
	long				aWindow;		// the window of the object
	void*				pSalFrame;		// contains a salframe, if object has one
	void*				pWidget;		// the corresponding widget
	void*				pVisual;		// the visual in use
	int					nDepth; 		// depth of said visual
	long				aColormap;		// the colormap being used
	void*				pAppContext;	// the application context in use
	long				aShellWindow;	// the window of the frame's shell
	void*				pShellWidget;	// the frame's shell widget
#endif
#endif
};

#define SystemChildData SystemEnvData

// --------------------
// - SystemParentData -
// --------------------

struct SystemParentData
{
	unsigned long	nSize;			// size in bytes of this structure
#ifndef SYSDATA_ONLY_BASETYPE
#if defined( WIN ) || defined( WNT )
	HWND			hWnd;			// the window hwnd
#elif defined( OS2 )
	HWND			hWnd;			// the client hwnd
#elif defined( USE_JAVA )
	::vcl::com_sun_star_vcl_VCLFrame*	pVCLFrame;	// the Java window
#elif defined( UNX )
	long		    aWindow;		// the window of the object
#endif
#endif
};

#endif // _SV_SYSDATA_HXX
