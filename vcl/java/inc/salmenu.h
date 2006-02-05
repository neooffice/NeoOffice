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
#ifndef _SV_SALMENU_HXX
#include <salmenu.hxx>
#endif

class JavaSalFrame;
class Menu;

namespace vcl
{
class com_sun_star_vcl_VCLMenuBar;
class com_sun_star_vcl_VCLMenu;
class com_sun_star_vcl_VCLMenuItemData;
}

// =======================================================================

class JavaSalMenu : public SalMenu
{
public:
	// used for menubars only
	::vcl::com_sun_star_vcl_VCLMenuBar *	mpVCLMenuBar;
	
	// used for menus
	::vcl::com_sun_star_vcl_VCLMenu *	mpVCLMenu;
	
	// Generic data
	JavaSalFrame*			mpParentFrame;		// pointer to the parent frame
	BOOL					mbIsMenuBarMenu;	// true for menu bars
	JavaSalMenu* 			mpParentMenu;		// Parent menu if this is a submenu
	Menu*					mpParentVCLMenu;

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
	XubString				mText;			// the item text
	XubString				mAccelText;		// the accelerator string
	Bitmap					maBitmap;		// item image
	int						mnId;			// item id

	::vcl::com_sun_star_vcl_VCLMenuItemData *mpVCLMenuItemData;
	
	JavaSalMenu*			mpSalMenu;		// SalMenu into which this item is inserted
	JavaSalMenu*			mpSalSubmenu;	// Submenu SalMenu if this item has a submenu
	Menu*					mpVCLMenu;		// VCL menu into which this item is inserted

							JavaSalMenuItem();
	virtual					~JavaSalMenuItem();
};

void ResetMenuEnabledStateForFrame( JavaSalFrame *pFrame, JavaSalMenu *pMenu );
void UpdateMenusForFrame( JavaSalFrame *pFrame, JavaSalMenu *pMenu );

#endif // _SV_SALMENU_H
