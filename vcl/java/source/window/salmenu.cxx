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
#ifndef _SV_SALINST_H
#include <salinst.h>
#endif
#ifndef _SV_SALMENU_H
#include <salmenu.h>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
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

JavaSalMenu::JavaSalMenu()
{
	mpVCLMenuBar = NULL;
	mpVCLMenu = NULL;
	mpParentFrame = NULL;
	mbIsMenuBarMenu = FALSE;
	mpParentVCLMenu = NULL;
}

//-----------------------------------------------------------------------------

JavaSalMenu::~JavaSalMenu()
{
	if( mbIsMenuBarMenu && mpVCLMenuBar )
	{
		mpVCLMenuBar->dispose();
		delete mpVCLMenuBar;
	}
	else if( mpVCLMenu )
	{
		mpVCLMenu->dispose();
		delete mpVCLMenu;
	}
}

//-----------------------------------------------------------------------------

BOOL JavaSalMenu::VisibleMenuBar()
{
	return TRUE;
}

//-----------------------------------------------------------------------------

void JavaSalMenu::SetFrame( const SalFrame *pFrame )
{
	if( mbIsMenuBarMenu && mpVCLMenuBar )
	{
		JavaSalFrame *pJavaFrame = (JavaSalFrame *)pFrame;
		mpVCLMenuBar->setFrame( pJavaFrame->mpVCLFrame );
		mpParentFrame=pJavaFrame;
	}
}

//-----------------------------------------------------------------------------

void JavaSalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
	JavaSalMenuItem *pJavaSalMenuItem = (JavaSalMenuItem *)pSalMenuItem;
	if( mbIsMenuBarMenu && mpVCLMenuBar )
	{
		mpVCLMenuBar->addMenuItem( pJavaSalMenuItem->mpVCLMenuItemData, nPos );
	}
	else if( mpVCLMenu )
	{
		mpVCLMenu->insertItem( pJavaSalMenuItem->mpVCLMenuItemData, nPos );
	}
}

//-----------------------------------------------------------------------------

void JavaSalMenu::RemoveItem( unsigned nPos )
{
	if( mbIsMenuBarMenu && mpVCLMenuBar )
	{
		mpVCLMenuBar->removeMenu( nPos );
	}
	else if( mpVCLMenu )
	{
		mpVCLMenu->removeItem( nPos );
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
void JavaSalMenu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos )
{
	JavaSalMenuItem *pJavaSalMenuItem = (JavaSalMenuItem *)pSalMenuItem;
	JavaSalMenu* pJavaSubMenu = (JavaSalMenu *)pSubMenu;
	if( mbIsMenuBarMenu && mpVCLMenuBar && pJavaSubMenu && pJavaSubMenu->mpVCLMenu )
	{
		mpVCLMenuBar->changeMenu( pJavaSubMenu->mpVCLMenu->getMenuItemDataObject(), nPos );
	}
	else if( mpVCLMenu && pSubMenu )
	{
		mpVCLMenu->attachSubmenu( pJavaSubMenu->mpVCLMenu->getMenuItemDataObject(), nPos );
	}
	pJavaSalMenuItem->mpSalSubmenu=(JavaSalMenu *)pSubMenu;
}

//-----------------------------------------------------------------------------

void JavaSalMenu::CheckItem( unsigned nPos, BOOL bCheck )
{
	if( mbIsMenuBarMenu )
	{
		// doesn't make sense to check top level menus!
	}
	else if( mpVCLMenu )
	{
		mpVCLMenu->checkItem(nPos, bCheck);
	}
}

//-----------------------------------------------------------------------------

void JavaSalMenu::EnableItem( unsigned nPos, BOOL bEnable )
{
	if( mbIsMenuBarMenu && mpVCLMenuBar )
	{
		mpVCLMenuBar->enableMenu( nPos, bEnable );
	}
	else if( mpVCLMenu )
	{
		mpVCLMenu->enableItem( nPos, bEnable );
	}
}

//-----------------------------------------------------------------------------

void JavaSalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage )
{
	// for now we'll ignore putting icons in AWT menus.  Most Mac apps don't
	// have them, so they're kind of extraneous on the platform anyhow.
}

//-----------------------------------------------------------------------------

void JavaSalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText )
{
	// assume pSalMenuItem is a pointer to the menu item object already at nPos
	JavaSalMenuItem *pJavaSalMenuItem = (JavaSalMenuItem *)pSalMenuItem;
	if( pJavaSalMenuItem && pJavaSalMenuItem->mpVCLMenuItemData )
	{
		// remove accelerator character
		XubString theText(rText);
		theText.EraseAllChars('~');
		pJavaSalMenuItem->mpVCLMenuItemData->setTitle( theText );
	}
}

//-----------------------------------------------------------------------------

void JavaSalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName )
{
	// assume pSalMenuItem is a pointer to the item to be associated with the
	// new shortcut
	JavaSalMenuItem *pJavaSalMenuItem = (JavaSalMenuItem *)pSalMenuItem;
	if( pJavaSalMenuItem && pJavaSalMenuItem->mpVCLMenuItemData ) {
		// Only pass through keycodes that are using the command key as Java
		// will always add a command key to any shortcut. Also, ignore any
		// shortcuts that use modifiers other than the shift key as Java only
		// allows adding of the shift key. Also, exclude standard shortcuts
		// in the application menu.
		if ( rKeyCode.IsMod1() && !rKeyCode.IsMod2() && !rKeyCode.IsControlMod() && ! ( rKeyCode.GetCode() == KEY_H && !rKeyCode.IsShift() ) && rKeyCode.GetCode() != KEY_Q && rKeyCode.GetCode() != KEY_COMMA )
			pJavaSalMenuItem->mpVCLMenuItemData->setKeyboardShortcut( rKeyCode.GetCode(), rKeyCode.IsShift() );
	}
}

//-----------------------------------------------------------------------------

void JavaSalMenu::GetSystemMenuData( SystemMenuData* pData )
{
}

// =======================================================================

JavaSalMenuItem::JavaSalMenuItem()
{
	mpVCLMenuItemData = NULL;
	mpSalSubmenu = NULL;
}

//-----------------------------------------------------------------------------

JavaSalMenuItem::~JavaSalMenuItem()
{
	if( mpVCLMenuItemData )
	{
		mpVCLMenuItemData->dispose();
		delete mpVCLMenuItemData;
	}
}

//-----------------------------------------------------------------------------

SalMenu* JavaSalInstance::CreateMenu( BOOL bMenuBar, Menu *pVCLMenu )
{
#ifndef NO_NATIVE_MENUS
	JavaSalMenu *pSalMenu = new JavaSalMenu();
	pSalMenu->mbIsMenuBarMenu = bMenuBar;
	pSalMenu->mpVCLMenuBar=NULL;
	pSalMenu->mpVCLMenu=NULL;
	pSalMenu->mpParentVCLMenu=pVCLMenu;

	if( bMenuBar )
	{
		// create a menubar java object
		pSalMenu->mpVCLMenuBar=new ::vcl::com_sun_star_vcl_VCLMenuBar();
	}
	else
	{
		// create a regular menu instance
		pSalMenu->mpVCLMenu=new ::vcl::com_sun_star_vcl_VCLMenu();
	}

	return( pSalMenu );
#else	// !NO_NATIVE_MENUS
	return NULL;
#endif	// !NO_NATIVE_MENUS
}

//-----------------------------------------------------------------------------

void JavaSalInstance::DestroyMenu( SalMenu* pMenu )
{
#ifndef NO_NATIVE_MENUS
	delete pMenu;
#endif	// !NO_NATIVE_MENUS
}

//-----------------------------------------------------------------------------

SalMenuItem* JavaSalInstance::CreateMenuItem( const SalItemParams* pItemData )
{
#ifndef NO_NATIVE_MENUS
	if(!pItemData)
		return NULL;

	JavaSalMenuItem *pSalMenuItem = new JavaSalMenuItem();
	XubString title(pItemData->aText);
	title.EraseAllChars('~');
	pSalMenuItem->mpVCLMenuItemData=new ::vcl::com_sun_star_vcl_VCLMenuItemData( title, ( pItemData->eType == MENUITEM_SEPARATOR ), pItemData->nId, pItemData->pMenu );
	return( pSalMenuItem );
#else	// !NO_NATIVE_MENUS
	return NULL;
#endif	// !NO_NATIVE_MENUS
}

//-----------------------------------------------------------------------------

void JavaSalInstance::DestroyMenuItem( SalMenuItem* pItem )
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
void UpdateMenusForFrame( JavaSalFrame *pFrame, JavaSalMenu *pMenu )
{
#ifndef NO_NATIVE_MENUS
	SalData *pSalData = GetSalData();

	// Check is frame is valid
	bool bFrameFound = false;
	for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
	{
		if ( *it == pFrame )
		{
			if ( pFrame->mbVisible )
				bFrameFound = true;
			break;
		}
	}
	if ( !bFrameFound )
		return;

	if(!pMenu) {
		// locate the menubar for the frame
		pMenu = pFrame->mpMenuBar;
		if(!pMenu)
			return;
	}

	Menu *pVCLMenu = pMenu->mpParentVCLMenu;
	OSL_ENSURE(pVCLMenu, "Unknown VCL menu for SalMenu!");

	// Force the clipboard service to update itself before we update the
	// menus as if the native clipboard was cleared when we last checked, we
	// won't be notified when another application puts content.
	if ( pFrame->mpMenuBar )
	{
		Window *pWindow = pVCLMenu->GetWindow();
		if ( pWindow )
		{
			Reference< XClipboard > aClipboard = pWindow->GetClipboard();
			if ( aClipboard.is() )
				aClipboard->getContents();
		}
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
		JavaSalMenuItem *pSalMenuItem = (JavaSalMenuItem *)pVCLMenu->GetItemSalItem( i );
		if ( pSalMenuItem )
		{
			// If this menu item has a submenu, fix that submenu up
			if ( pSalMenuItem->mpSalSubmenu )
				UpdateMenusForFrame( pFrame, pSalMenuItem->mpSalSubmenu );
		}
	}

	// Post the SALEVENT_MENUDEACTIVATE event
	SalMenuEvent *pDeactivateEvent = new SalMenuEvent();
	pDeactivateEvent->mnId = 0;
	pDeactivateEvent->mpMenu = pVCLMenu;
	com_sun_star_vcl_VCLEvent aDeactivateEvent( SALEVENT_MENUDEACTIVATE, pFrame, pDeactivateEvent );
	aDeactivateEvent.dispatch();
#endif	// !NO_NATIVE_MENUS
}
