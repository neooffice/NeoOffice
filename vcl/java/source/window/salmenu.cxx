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
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENT_HXX
#include <com/sun/star/vcl/VCLEvent.hxx>
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
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#endif

using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::uno;
using namespace vcl;

//=============================================================================

SalMenu::SalMenu()
{
	memset( &maData, 0, sizeof(maData) );
	mpParentVCLMenu = NULL;
}

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

BOOL SalMenu::VisibleMenuBar()
{
	return TRUE;
}

//-----------------------------------------------------------------------------

void SalMenu::SetFrame( SalFrame *pFrame )
{
	if( maData.mbIsMenuBarMenu && maData.mpVCLMenuBar )
	{
		maData.mpVCLMenuBar->setFrame( pFrame->maFrameData.mpVCLFrame );
		pFrame->maFrameData.mpMenuBar=this;
	}
}

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

/**
 * Attach a new submenu to a menu item
 *
 * @param pSalMenuItem	pointer to the item already at nPos
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

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

void SalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage )
{
	// for now we'll ignore putting icons in AWT menus.  Most Mac apps don't
	// have them, so they're kind of extraneous on the platform anyhow.
}

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

void SalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName )
{
	// assume pSalMenuItem is a pointer to the item to be associated with the
	// new shortcut
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
		)
		{
			pSalMenuItem->maData.mpVCLMenuItemData->setKeyboardShortcut(rKeyCode.GetCode(), rKeyCode.IsShift());
		}
	}
}

//-----------------------------------------------------------------------------

void SalMenu::GetSystemMenuData( SystemMenuData* pData )
{
}

//-----------------------------------------------------------------------------

void SalMenu::SetDisplayed( BOOL bDisplay )
{
	if( maData.mbIsMenuBarMenu && maData.mpVCLMenuBar )
	{
		if( bDisplay )
			maData.mpVCLMenuBar->show();
		else
			maData.mpVCLMenuBar->hide();
	}
}

// =======================================================================

SalMenuItem::SalMenuItem()
{
	maData.mpVCLMenuItemData = NULL;
	maData.mpSalSubmenu = NULL;
}

//-----------------------------------------------------------------------------

SalMenuItem::~SalMenuItem()
{
	if( maData.mpVCLMenuItemData )
	{
		maData.mpVCLMenuItemData->dispose();
		delete maData.mpVCLMenuItemData;
	}
}

//-----------------------------------------------------------------------------

SalMenu* SalInstance::CreateMenu( BOOL bMenuBar, Menu* pVCLMenu )
{
#ifndef NO_NATIVE_MENUS
	SalMenu *pSalMenu = new SalMenu();
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

//-----------------------------------------------------------------------------

void SalInstance::DestroyMenu( SalMenu* pMenu )
{
#ifndef NO_NATIVE_MENUS
	delete pMenu;
#endif	// !NO_NATIVE_MENUS
}

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

void SalInstance::DestroyMenuItem( SalMenuItem* pItem )
{
#ifndef NO_NATIVE_MENUS
	delete pItem;
#endif	// !NO_NATIVE_MENUS
}

// ============================================================================

/**
 * Given a frame and a submenu, post SALEVENT_MENUACTIVATE and
 * SALEVENT_MENUDEACTIVATE events to all of the VCL menu objects. This function
 * is normally called before the native menus are shown.
 */
void UpdateMenusForFrame( SalFrame *pFrame, SalMenu *pMenu )
{
#ifndef NO_NATIVE_MENUS
	SalData *pSalData = GetSalData();

	// Check is frame is valid
	bool bFrameFound = false;
	for ( ::std::list< SalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
	{
		if ( *it == pFrame )
		{
			if ( pFrame->maFrameData.mbVisible )
				bFrameFound = true;
			break;
		}
	}
	if ( !bFrameFound )
		return;

	bool bWasMenuBarInvocation = false;
	if(!pMenu) {
		// locate the menubar for the frame
		pMenu = pFrame->maFrameData.mpMenuBar;
		if(!pMenu)
			return;
		bWasMenuBarInvocation = true;
	}

	Menu *pVCLMenu = pMenu->mpParentVCLMenu;
	OSL_ENSURE(pVCLMenu, "Unknown VCL menu for SalMenu!");

	// Force the clipboard service to update itself before we update the
	// menus as if the native clipboard was cleared when we last checked, we
	// won't be notified when another application puts content.
	Window *pWindow = pVCLMenu->GetWindow();
	if ( pWindow )
	{
		Reference< XClipboard > aClipboard = pWindow->GetClipboard();
		if ( aClipboard.is() )
			aClipboard->getContents();
	}

	// Post the SALEVENT_MENUACTIVATE event
	SalMenuEvent *pActivateEvent = new SalMenuEvent();
	pActivateEvent->mnId = 0;
	pActivateEvent->mpMenu = pVCLMenu;
	com_sun_star_vcl_VCLEvent aActivateEvent( SALEVENT_MENUACTIVATE, pFrame, pActivateEvent );
	aActivateEvent.dispatch();

	USHORT nCount = pVCLMenu->GetItemCount();
	for( USHORT i = 0; i < nCount; i++ )
	{
		SalMenuItem *pSalMenuItem = pVCLMenu->GetItemSalItem( i );

		if ( pSalMenuItem )
		{
			// If this menu item has a submenu, fix that submenu up
			if ( pSalMenuItem->maData.mpSalSubmenu )
				UpdateMenusForFrame( pFrame, pSalMenuItem->maData.mpSalSubmenu );
		}
	}

	// Post the SALEVENT_MENUDEACTIVATE event
	SalMenuEvent *pDeactivateEvent = new SalMenuEvent();
	pDeactivateEvent->mnId = 0;
	pDeactivateEvent->mpMenu = pVCLMenu;
	com_sun_star_vcl_VCLEvent aDeactivateEvent( SALEVENT_MENUDEACTIVATE, pFrame, pDeactivateEvent );
	aDeactivateEvent.dispatch();
	
	// For our menubars, following insertion of all of the items dispatch
	// a refresh for all of the checkbox menu items.  We need to refresh
	// their state before the menus are displayed in order to work around
	// bugs in the Apple 1.3.1 VM that prevent the state from being
	// set properly if the setState() call is issued prior to the
	// checkbox menu item having a peer.  Bug 182.
	
	if( bWasMenuBarInvocation )
	{
		if( pFrame->maFrameData.mpMenuBar->maData.mpVCLMenuBar )
			pFrame->maFrameData.mpMenuBar->maData.mpVCLMenuBar->syncCheckboxMenuItemState();
	}
#endif	// !NO_NATIVE_MENUS
}
