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


#define _SV_SALMENU_CXX

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALMENU_HXX
#include <salmenu.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLMENUBAR_HXX
#include <com/sun/star/vcl/VCLMenuBar.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLMENUITEMDATA_HXX
#include <com/sun/star/vcl/VCLMenuItemData.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLMENU_HXX
#include <com/sun/star/vcl/VCLMenu.hxx>
#endif

/*
 * SalMenu stub implementations
 */

SalMenu::SalMenu()
{
    memset( &maData, 0, sizeof(maData) );
    mpParentVCLMenu = NULL;
}

SalMenu::~SalMenu()
{
    if( maData.mbIsMenuBarMenu && maData.mpVCLMenuBar )
    {
        maData.mpVCLMenuBar->dispose();
        delete maData.mpVCLMenuBar;
    }
    else if( maData.mpVCLMenu )
    {
	maData.mpVCLMenu->dispose();
        delete maData.mpVCLMenu;
    }
}

BOOL SalMenu::VisibleMenuBar()
{
    return FALSE; // TRUE suppresses the display of VCL menu bars 
}

void SalMenu::SetFrame( SalFrame *pFrame )
{
    if( maData.mbIsMenuBarMenu && maData.mpVCLMenuBar )
    {
        maData.mpVCLMenuBar->setFrame( pFrame->maFrameData.mpVCLFrame );
        pFrame->maFrameData.mpMenuBar=this;
    }
}

void SalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
    if( maData.mbIsMenuBarMenu && maData.mpVCLMenuBar )
    {
        maData.mpVCLMenuBar->addMenuItem( pSalMenuItem->maData.mpVCLMenuItemData, nPos );
    }
    else if( maData.mpVCLMenu )
    {
        maData.mpVCLMenu->insertItem( pSalMenuItem->maData.mpVCLMenuItemData, nPos );
    }
}

void SalMenu::RemoveItem( unsigned nPos )
{
    if( maData.mbIsMenuBarMenu && maData.mpVCLMenuBar )
    {
        maData.mpVCLMenuBar->removeMenu( nPos );
    }
    else if( maData.mpVCLMenu )
    {
        maData.mpVCLMenu->removeItem( nPos );
    }
}

/**
 * Attach a new submenu to a menu item
 *
 * @param pSalMenuItem		? (? pointer to the item already at nPos ?)
 * @param pSubMenu		new menu to provide the contents of the submenu
 * @param nPos			position of the submenu in the menu item list
 */
void SalMenu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos )
{
    if( maData.mbIsMenuBarMenu && maData.mpVCLMenuBar && pSubMenu && pSubMenu->maData.mpVCLMenu )
    {
        maData.mpVCLMenuBar->changeMenu( pSubMenu->maData.mpVCLMenu->getMenuItemDataObject(), nPos );
    }
    else if( maData.mpVCLMenu && pSubMenu )
    {
        maData.mpVCLMenu->attachSubmenu( pSubMenu->maData.mpVCLMenu->getMenuItemDataObject(), nPos );
    }
    pSalMenuItem->maData.mpSalSubmenu=pSubMenu;
}

void SalMenu::CheckItem( unsigned nPos, BOOL bCheck )
{
    if( maData.mbIsMenuBarMenu )
    {
        // doesn't make sense to check top level menus!
    }
    else if( maData.mpVCLMenu )
    {
        maData.mpVCLMenu->checkItem(nPos, bCheck);
    }
}

void SalMenu::EnableItem( unsigned nPos, BOOL bEnable )
{
    if( maData.mbIsMenuBarMenu && maData.mpVCLMenuBar )
    {
        maData.mpVCLMenuBar->enableMenu( nPos, bEnable );
    }
    else if( maData.mpVCLMenu )
    {
        maData.mpVCLMenu->enableItem(nPos, bEnable);
    }
}

void SalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage )
{
    // for now we'll ignore putting icons in AWT menus.  Most Mac apps don't have them, so they're
    // kind of extraneous on the platform anyhow.
}

void SalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText )
{
    // assume pSalMenuItem is a pointer to the menu item object already at nPos
    if( pSalMenuItem && pSalMenuItem->maData.mpVCLMenuItemData )
    {
        // remove accelerator character
        XubString theText(rText);
        theText.EraseAllChars('~');
        pSalMenuItem->maData.mpVCLMenuItemData->setTitle( theText );
    }
}

void SalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName )
{
    // assume pSalMenuItem is a pointer to the item to be associated with the new shortcut
    if( pSalMenuItem && pSalMenuItem->maData.mpVCLMenuItemData ) {
	// only pass through keycodes that are using Mod1, the equivalent of
	// the "control" key.  Java AWT only allows us control and shift to
	// be used as menu accelerator modifiers.  Bugs in AWT 1.3
	// implementaion cause function keys to be misinterpreted as letter keys
	// so we can only allow in Mod1 Alphanumeric keys with/without shift
	// as valid modifiers.
	if(rKeyCode.IsMod1() && 
	    !rKeyCode.IsMod2() && 
	    (((rKeyCode.GetCode()>=KEY_0) && (rKeyCode.GetCode()<=KEY_9)) ||
	     ((rKeyCode.GetCode()>=KEY_A) && (rKeyCode.GetCode()<=KEY_Z)))
	   ) {
	    pSalMenuItem->maData.mpVCLMenuItemData->setKeyboardShortcut(rKeyCode.GetCode(), rKeyCode.IsShift());
	}
    }
}

void SalMenu::GetSystemMenuData( SystemMenuData* pData )
{
}

// =======================================================================

/*
 * SalMenuItem
 */


SalMenuItem::SalMenuItem()
{
    maData.mpVCLMenuItemData = NULL;
    maData.mpSalSubmenu = NULL;
}

SalMenuItem::~SalMenuItem()
{
    if( maData.mpVCLMenuItemData )
    {
	maData.mpVCLMenuItemData->dispose();
        delete maData.mpVCLMenuItemData;
    }
}

// -------------------------------------------------------------------

// -----------------------------------------------------------------------

SalMenu* SalInstance::CreateMenu( BOOL bMenuBar, Menu* pVCLMenu )
{
#ifndef NO_NATIVE_MENUS
    SalMenu *	pSalMenu = new SalMenu();
    pSalMenu->maData.mbIsMenuBarMenu = bMenuBar;
    pSalMenu->maData.mpVCLMenuBar=NULL;
    pSalMenu->maData.mpVCLMenu=NULL;
    pSalMenu->mpParentVCLMenu=pVCLMenu;
    
    if( bMenuBar )
    {
        // create a menubar java object
        
        pSalMenu->maData.mpVCLMenuBar=new ::vcl::com_sun_star_vcl_VCLMenuBar();
    }
    else
    {
        // create a regular menu instance
        
        pSalMenu->maData.mpVCLMenu=new ::vcl::com_sun_star_vcl_VCLMenu();
    }
    
    return( pSalMenu );
#else	// !NO_NATIVE_MENUS
	return NULL;
#endif	// !NO_NATIVE_MENUS
}

// -----------------------------------------------------------------------

void SalInstance::DestroyMenu( SalMenu* pMenu )
{
#ifndef NO_NATIVE_MENUS
    delete pMenu;
#endif	// !NO_NATIVE_MENUS
}

// -----------------------------------------------------------------------

SalMenuItem* SalInstance::CreateMenuItem( const SalItemParams* pItemData )
{
#ifndef NO_NATIVE_MENUS
    if(!pItemData)
        return NULL;
    
    SalMenuItem *	pSalMenuItem = new SalMenuItem();
    XubString title(pItemData->aText);
    title.EraseAllChars('~');
    pSalMenuItem->maData.mpVCLMenuItemData=new ::vcl::com_sun_star_vcl_VCLMenuItemData( title, ( pItemData->eType == MENUITEM_SEPARATOR ), pItemData->nId, pItemData->pMenu );
    return( pSalMenuItem );
#else	// !NO_NATIVE_MENUS
	return NULL;
#endif	// !NO_NATIVE_MENUS
}

// -----------------------------------------------------------------------

void SalInstance::DestroyMenuItem( SalMenuItem* pItem )
{
#ifndef NO_NATIVE_MENUS
    delete pItem;
#endif	// !NO_NATIVE_MENUS
}

// ----------------------------------------------------------------------------

/**
 * Given a frame and a submenu, dispatch ACTIVATE events to all of the VCL menu objects.  The activate
 * events are the ones which have attached actions to update the menu contents and dimmed/active state
 * of menu items.
 *
 * Java AWT doesn't give us the opportunity to generate activate events before menus are displayed, so
 * we need to call this function manually for the front frame periodically
 */
void UpdateMenusForFrame( SalFrame *pFrame, SalMenu *pMenu )
{
#ifndef NO_NATIVE_MENUS
    if(!pMenu) {
        // locate the menubar for the frame
        
        pMenu = pFrame->maFrameData.mpMenuBar;
        if(!pMenu)
            return;
    }
    
    {
		Menu *		pVCLMenu = pMenu->mpParentVCLMenu;
		USHORT		i;
		SalMenuEvent   pEvent;
		
                OSL_ENSURE(pVCLMenu, "Unknown VCL menu for SalMenu!");
                
		pEvent.mpMenu = pVCLMenu;

		pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame, SALEVENT_MENUACTIVATE, &pEvent );
		pFrame->maFrameData.mpProc( pFrame->maFrameData.mpInst, pFrame, SALEVENT_MENUDEACTIVATE, &pEvent);

		for( i = 0; i < pVCLMenu->GetItemCount(); i++ )
		{
			SalMenuItem *  pSalMenuItem = pVCLMenu->GetItemSalItem( i );
			
			if ( pSalMenuItem )
			{
				// If this menu item has a submenu, fix that submenu up
				if ( pSalMenuItem->maData.mpSalSubmenu )
					UpdateMenusForFrame( pFrame, pSalMenuItem->maData.mpSalSubmenu );
			}
		}
	}
#endif	// !NO_NATIVE_MENUS
}
