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
 *  Copyright 2004 Planamesa Inc.
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

#include <vcl/sv.h>

#include "image.h"
#include "salmenu.hxx"
#include "java/salframe.h"

class JavaSalFrame;
class Menu;

// =======================================================================

class JavaSalMenu : public SalMenu
{
public:
	id						mpMenu;

	static void				SetMenuBarToFocusFrame();

	// Generic data
	JavaSalFrame*			mpParentFrame;		// pointer to the parent frame
	sal_Bool				mbIsMenuBarMenu;	// true for menu bars
	Menu*					mpParentVCLMenu;
	XubString				maText;

							JavaSalMenu();
	virtual					~JavaSalMenu();

	virtual sal_Bool		VisibleMenuBar();
	virtual void			InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos );
	virtual void			RemoveItem( unsigned nPos );
	virtual void			SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos );
	virtual void			SetFrame( const SalFrame* pFrame );
	virtual void			CheckItem( unsigned nPos, sal_Bool bCheck );
	virtual void			EnableItem( unsigned nPos, sal_Bool bEnable );
	virtual void			SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText );
	virtual void			SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage );
	virtual void			SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName );
	virtual void			GetSystemMenuData( SystemMenuData* pData );
};

class JavaSalMenuItem : public SalMenuItem
{
public:
	id						mpMenuItem;
	JavaSalMenu*			mpSalSubmenu;	// Submenu SalMenu if this item has a submenu

							JavaSalMenuItem();
	virtual					~JavaSalMenuItem();
};

SAL_DLLPRIVATE void UpdateMenusForFrame( JavaSalFrame *pFrame, JavaSalMenu *pMenu, bool bUpdateSubmenus );

#endif // _SV_SALMENU_H
