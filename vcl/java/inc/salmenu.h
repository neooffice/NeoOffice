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

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

class SalMenu;
class SalMenuItem;
class SalFrame;
class Menu;

// =======================================================================

class SalMenuData
{
	friend class		SalInstance;
	friend class		SalMenu;
	friend void			UpdateMenusForFrame( SalFrame*, SalMenu* );
	friend void			SetActiveMenuBarForFrame( SalFrame* );

	MenuRef				maMenu;
	SalFrame*			mpFrame;
	bool				mbIsMenuBarMenu;	// true for menu bars
	SalMenu*			mpParentMenu;		// Parent menu if this is a submenu

						SalMenuData();
						~SalMenuData();

public:
	SalFrame*			GetFrame() { return mpFrame; }
	SalMenu*			GetParentMenu() { return mpParentMenu; }
};

class SalMenuItemData
{
	friend class		SalInstance;
	friend class		SalMenu;
	friend class		SalMenuItem;
	friend void			UpdateMenusForFrame( SalFrame*, SalMenu* );

	CFStringRef			maTitle;
	bool				mbSeparator;
	SalMenu*			mpSalMenu;		// SalMenu into which this item is inserted
	SalMenu*			mpSalSubmenu;	// Submenu SalMenu if this item has a submenu

						SalMenuItemData();
						~SalMenuItemData();

public:
	SalMenu*			GetSalMenu() { return mpSalMenu; }
};

#endif // _SV_SALMENU_H

