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
	bool					mbIsMenuBarMenu;	// true for menu bars
	Menu*					mpParentVCLMenu;

							JavaSalMenu();
	virtual					~JavaSalMenu();

	virtual bool			VisibleMenuBar() SAL_OVERRIDE;
	virtual void			InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos ) SAL_OVERRIDE;
	virtual void			RemoveItem( unsigned nPos ) SAL_OVERRIDE;
	virtual void			SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos ) SAL_OVERRIDE;
	virtual void			SetFrame( const SalFrame* pFrame ) SAL_OVERRIDE;
	virtual void			CheckItem( unsigned nPos, bool bCheck ) SAL_OVERRIDE;
	virtual void			EnableItem( unsigned nPos, bool bEnable ) SAL_OVERRIDE;
	virtual void			SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const OUString& rText ) SAL_OVERRIDE;
	virtual void			SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage ) SAL_OVERRIDE;
	virtual void			SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const vcl::KeyCode& rKeyCode, const OUString& rKeyName ) SAL_OVERRIDE;
	virtual void			GetSystemMenuData( SystemMenuData* pData ) SAL_OVERRIDE;
};

enum JavaSalMenuItemType
{
	NONE	= 0,
	HELP	= 1,
	WINDOWS	= 2
};

class JavaSalMenuItem : public SalMenuItem
{
public:
	id						mpMenuItem;
	JavaSalMenuItemType		meMenuType;
	JavaSalMenu*			mpSalSubmenu;	// Submenu SalMenu if this item has a submenu

							JavaSalMenuItem();
	virtual					~JavaSalMenuItem();

	void					SetCommand( const OUString& rCommand );
};

SAL_DLLPRIVATE void UpdateMenusForFrame( JavaSalFrame *pFrame, JavaSalMenu *pMenu, bool bUpdateSubmenus );
SAL_DLLPRIVATE void VCLMenu_updateNativeWindowsMenu();

#endif // _SV_SALMENU_H
