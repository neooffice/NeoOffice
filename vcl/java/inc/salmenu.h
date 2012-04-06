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

#include <salframe.h>
#include <vcl/sv.h>
#include <vcl/image.h>
#include <vcl/salmenu.hxx>

class JavaSalFrame;
class Menu;

#ifndef USE_NATIVE_WINDOW
namespace vcl
{
class com_sun_star_vcl_VCLMenuBar;
class com_sun_star_vcl_VCLMenu;
class com_sun_star_vcl_VCLMenuItemData;
}
#endif	// !USE_NATIVE_WINDOW

// =======================================================================

class JavaSalMenu : public SalMenu
{
public:
	// used for menubars only
#ifdef USE_NATIVE_WINDOW
	id						mpMenuBar;
#else	// USE_NATIVE_WINDOW
	::vcl::com_sun_star_vcl_VCLMenuBar *	mpVCLMenuBar;
#endif	// USE_NATIVE_WINDOW
	
	// used for menus
#ifdef USE_NATIVE_WINDOW
	id						mpMenu;
#else	// USE_NATIVE_WINDOW
	::vcl::com_sun_star_vcl_VCLMenu *	mpVCLMenu;
#endif	// USE_NATIVE_WINDOW
	
	// Generic data
	JavaSalFrame*			mpParentFrame;		// pointer to the parent frame
	BOOL					mbIsMenuBarMenu;	// true for menu bars
	Menu*					mpParentVCLMenu;
	XubString				maText;

							JavaSalMenu();
	virtual					~JavaSalMenu();

	virtual BOOL			VisibleMenuBar();
	virtual void			InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos );
	virtual void			RemoveItem( unsigned nPos );
	virtual void			SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos );
	virtual void			SetFrame( const SalFrame* pFrame );
	virtual void			CheckItem( unsigned nPos, BOOL bCheck );
	virtual void			EnableItem( unsigned nPos, BOOL bEnable );
	virtual void			SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText );
	virtual void			SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage );
	virtual void			SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName );
	virtual void			GetSystemMenuData( SystemMenuData* pData );
};

class JavaSalMenuItem : public SalMenuItem
{
public:
#ifdef USE_NATIVE_WINDOW
	id						mpMenuItem;
#else	// USE_NATIVE_WINDOW
	::vcl::com_sun_star_vcl_VCLMenuItemData *mpVCLMenuItemData;
#endif	// USE_NATIVE_WINDOW
	
	JavaSalMenu*			mpSalSubmenu;	// Submenu SalMenu if this item has a submenu

							JavaSalMenuItem();
	virtual					~JavaSalMenuItem();
};

SAL_DLLPRIVATE void UpdateMenusForFrame( JavaSalFrame *pFrame, JavaSalMenu *pMenu, bool bUpdateSubmenus );

#endif // _SV_SALMENU_H
