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
 *  Edward Peterlin, July 2004
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2004 by Edward Peterlin (OPENSTEP@neooffice.org)
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

#ifndef _SV_SALMENU_H
#define _SV_SALMENU_H

#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_IMAGE_H
#include <image.h>
#endif

class SalMenu;
class SalMenuItem;
class SalFrame;
class Menu;

namespace vcl
{
class com_sun_star_vcl_VCLMenuBar;
class com_sun_star_vcl_VCLMenu;
class com_sun_star_vcl_VCLMenuItemData;
}

// =======================================================================

class SalMenuData
{
public:
	// used for menubars only
	::vcl::com_sun_star_vcl_VCLMenuBar *	mpVCLMenuBar;
	
	// used for menus
	::vcl::com_sun_star_vcl_VCLMenu *	mpVCLMenu;
	
	// Generic data
	SalFrame *			mpParentFrame;		// pointer to the parent frame
	BOOL				mbIsMenuBarMenu;	// true for menu bars
};

class SalMenuItemData
{
public:
	::vcl::com_sun_star_vcl_VCLMenuItemData *mpVCLMenuItemData;
	
	SalMenu *			mpSalSubmenu;	// Submenu SalMenu if this item has a submenu
};

void UpdateMenusForFrame( SalFrame *pFrame, SalMenu *pMenu );

#endif // _SV_SALMENU_H
