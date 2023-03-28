/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
	Menu*					mpVCLMenu;
	JavaSalMenu*			mpParentSalMenu;

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
	virtual bool			ShowNativePopupMenu( FloatingWindow *pWin, const Rectangle& rRect, sal_uLong nFlags ) SAL_OVERRIDE;
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
	JavaSalMenu*			mpParentSalMenu;

							JavaSalMenuItem();
	virtual					~JavaSalMenuItem();

	void					SetCommand( const OUString& rCommand );
};

SAL_DLLPRIVATE void UpdateMenusForFrame( JavaSalFrame *pFrame, JavaSalMenu *pMenu, bool bUpdateSubmenus );
SAL_DLLPRIVATE void VCLMenu_updateNativeWindowsMenu();

#ifdef __OBJC__
SAL_DLLPRIVATE BOOL VCLMenu_isPopUpMenu( NSMenu *pMenu );
SAL_DLLPRIVATE BOOL VCLMenu_isShowingPopUpMenu();
#endif	// __OBJC__

#endif // _SV_SALMENU_H
