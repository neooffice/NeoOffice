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
 *
 *  
 *  =================================================
 *  Modified September 2002 by Edward Peterlin. SISSL Removed. NeoOffice is distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_SALMENU_HXX
#define _SV_SALMENU_HXX

#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_SALMENU_H
#include <salmenu.h>
#endif
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif
#ifndef _SV_KEYCODE_HXX
#include <keycod.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <image.hxx>
#endif

struct SystemMenuData;

struct SalItemParams
{
    USHORT          nId;					// item Id
    MenuItemType    eType;					// MenuItem-Type
    MenuItemBits    nBits;					// MenuItem-Bits
    Menu*           pMenu;				    // Pointer to Menu
    XubString       aText;					// Menu-Text
    Image           aImage;					// Image
};


class SalMenuItem
{
    friend class SalInstance;

private:
    SalMenuItem();
    ~SalMenuItem();

public:
    SalMenuItemData maData;
};

class SalMenu
{
    friend class SalInstance;

private:
    SalMenu();
    ~SalMenu();

public:
    SalMenuData maData;
    Menu*       mpParentVCLMenu;

    static BOOL VisibleMenuBar();  // must return TRUE to actually DISPLAY native menu bars
                                   // otherwise only menu messages are processed (eg, OLE on Windows)

    void InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos );
    void RemoveItem( unsigned nPos );
    void SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos );
    void SetFrame( SalFrame* pFrame );
    void CheckItem( unsigned nPos, BOOL bCheck );
    void EnableItem( unsigned nPos, BOOL bEnable );
    void SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText );
    void SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage );
    void SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName );
    void GetSystemMenuData( SystemMenuData* pData );
};


#endif // _SV_SALMENU_HXX

