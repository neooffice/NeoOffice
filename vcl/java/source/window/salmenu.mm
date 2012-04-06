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

#include <map>

#include <salinst.h>
#include <salmenu.h>
#include <salframe.h>
#include <vcl/window.hxx>
#include <com/sun/star/vcl/VCLEvent.hxx>
#ifndef USE_NATIVE_WINDOW
#include <com/sun/star/vcl/VCLMenuBar.hxx>
#include <com/sun/star/vcl/VCLMenuItemData.hxx>
#include <com/sun/star/vcl/VCLMenu.hxx>
#endif	// !USE_NATIVE_WINDOW
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>

#include <premac.h>
#import <CoreFoundation/CoreFoundation.h>
#ifdef USE_NATIVE_WINDOW
#import <Cocoa/Cocoa.h>
#endif	// USE_NATIVE_WINDOW
#include <postmac.h>

static ::std::map< JavaSalMenu*, JavaSalMenu* > aMenuMap;

using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::uno;
using namespace vcl;

//=============================================================================

JavaSalMenu::JavaSalMenu()
{
#ifdef USE_NATIVE_WINDOW
	mpMenuBar = NULL;
	mpMenu = NULL;
#else	// USE_NATIVE_WINDOW
	mpVCLMenuBar = NULL;
	mpVCLMenu = NULL;
#endif	// USE_NATIVE_WINDOW
	mpParentFrame = NULL;
	mbIsMenuBarMenu = FALSE;
	mpParentVCLMenu = NULL;
	aMenuMap[ this ] = this;
}

//-----------------------------------------------------------------------------

JavaSalMenu::~JavaSalMenu()
{
	aMenuMap.erase( this );

#ifdef USE_NATIVE_WINDOW
	fprintf( stderr, "JavaSalMenu::~JavaSalMenu not implemented\n" );
#else	// USE_NATIVE_WINDOW
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
#endif	// USE_NATIVE_WINDOW
}

//-----------------------------------------------------------------------------

BOOL JavaSalMenu::VisibleMenuBar()
{
	return TRUE;
}

//-----------------------------------------------------------------------------

void JavaSalMenu::SetFrame( const SalFrame *pFrame )
{
#ifdef USE_NATIVE_WINDOW
	fprintf( stderr, "JavaSalMenu::SetFrame not implemented\n" );
#else	// USE_NATIVE_WINDOW
	if( mbIsMenuBarMenu && mpVCLMenuBar )
	{
		JavaSalFrame *pJavaFrame = (JavaSalFrame *)pFrame;
		mpVCLMenuBar->setFrame( pJavaFrame->mpVCLFrame );
		mpParentFrame=pJavaFrame;
	}
#endif	// USE_NATIVE_WINDOW
}

//-----------------------------------------------------------------------------

void JavaSalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
#ifdef USE_NATIVE_WINDOW
	fprintf( stderr, "JavaSalMenu::InsertItem not implemented\n" );
#else	// USE_NATIVE_WINDOW
	JavaSalMenuItem *pJavaSalMenuItem = (JavaSalMenuItem *)pSalMenuItem;
	if( mbIsMenuBarMenu && mpVCLMenuBar )
	{
		mpVCLMenuBar->addMenuItem( pJavaSalMenuItem->mpVCLMenuItemData, nPos );
	}
	else if( mpVCLMenu )
	{
		mpVCLMenu->insertItem( pJavaSalMenuItem->mpVCLMenuItemData, nPos );
	}
#endif	// USE_NATIVE_WINDOW
}

//-----------------------------------------------------------------------------

void JavaSalMenu::RemoveItem( unsigned nPos )
{
#ifdef USE_NATIVE_WINDOW
	fprintf( stderr, "JavaSalMenu::RemoveItem not implemented\n" );
#else	// USE_NATIVE_WINDOW
	if( mbIsMenuBarMenu && mpVCLMenuBar )
	{
		mpVCLMenuBar->removeMenu( nPos );
	}
	else if( mpVCLMenu )
	{
		mpVCLMenu->removeItem( nPos );
	}
#endif	// USE_NATIVE_WINDOW
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
#ifdef USE_NATIVE_WINDOW
	fprintf( stderr, "JavaSalMenu::SetSubItem not implemented\n" );
#else	// USE_NATIVE_WINDOW
	JavaSalMenu* pJavaSubMenu = (JavaSalMenu *)pSubMenu;
	if( mbIsMenuBarMenu && mpVCLMenuBar && pJavaSubMenu && pJavaSubMenu->mpVCLMenu )
	{
		mpVCLMenuBar->changeMenu( pJavaSubMenu->mpVCLMenu->getMenuItemDataObject(), nPos );
	}
	else if( mpVCLMenu && pSubMenu )
	{
		mpVCLMenu->attachSubmenu( pJavaSubMenu->mpVCLMenu->getMenuItemDataObject(), nPos );
	}
#endif	// USE_NATIVE_WINDOW
	pJavaSalMenuItem->mpSalSubmenu=(JavaSalMenu *)pSubMenu;
}

//-----------------------------------------------------------------------------

void JavaSalMenu::CheckItem( unsigned nPos, BOOL bCheck )
{
#ifdef USE_NATIVE_WINDOW
	fprintf( stderr, "JavaSalMenu::CheckItem not implemented\n" );
#else	// USE_NATIVE_WINDOW
	if( mbIsMenuBarMenu )
	{
		// doesn't make sense to check top level menus!
	}
	else if( mpVCLMenu )
	{
		mpVCLMenu->checkItem(nPos, bCheck);
	}
#endif	// USE_NATIVE_WINDOW
}

//-----------------------------------------------------------------------------

void JavaSalMenu::EnableItem( unsigned nPos, BOOL bEnable )
{
#ifdef USE_NATIVE_WINDOW
	fprintf( stderr, "JavaSalMenu::EnableItem not implemented\n" );
#else	// USE_NATIVE_WINDOW
	if( mbIsMenuBarMenu && mpVCLMenuBar )
	{
		mpVCLMenuBar->enableMenu( nPos, bEnable );
	}
	else if( mpVCLMenu )
	{
		mpVCLMenu->enableItem( nPos, bEnable );
	}
#endif	// USE_NATIVE_WINDOW
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
#ifdef USE_NATIVE_WINDOW
	fprintf( stderr, "JavaSalMenu::SetItemText not implemented\n" );
#else	// USE_NATIVE_WINDOW
	// assume pSalMenuItem is a pointer to the menu item object already at nPos
	JavaSalMenuItem *pJavaSalMenuItem = (JavaSalMenuItem *)pSalMenuItem;
	if( pJavaSalMenuItem && pJavaSalMenuItem->mpVCLMenuItemData )
	{
		// remove accelerator character
		XubString theText(rText);
		theText.EraseAllChars('~');
		OUString aText( theText );
		pJavaSalMenuItem->mpVCLMenuItemData->setTitle( aText );
	}
#endif	// USE_NATIVE_WINDOW
}

//-----------------------------------------------------------------------------

void JavaSalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName )
{
#ifdef USE_NATIVE_WINDOW
	fprintf( stderr, "JavaSalMenu::SetAccelerator not implemented\n" );
#else	// USE_NATIVE_WINDOW
	// assume pSalMenuItem is a pointer to the item to be associated with the
	// new shortcut
	JavaSalMenuItem *pJavaSalMenuItem = (JavaSalMenuItem *)pSalMenuItem;
	if( pJavaSalMenuItem && pJavaSalMenuItem->mpVCLMenuItemData ) {
		// Only pass through keycodes that are using the command key as Java
		// will always add a command key to any shortcut. Also, ignore any
		// shortcuts that use modifiers other than the shift key as Java only
		// allows adding of the shift key. Also, exclude standard shortcuts
		// in the application menu. Also, fix bug 2886 by not allowing any
		// shortcuts with a space as Java will disable a tab and the shortcut
		// be unusable.
		if ( rKeyCode.IsMod1() && !rKeyCode.IsMod2() && !rKeyCode.IsMod3() && ! ( rKeyCode.GetCode() == KEY_H && !rKeyCode.IsShift() ) && rKeyCode.GetCode() != KEY_Q && rKeyCode.GetCode() != KEY_COMMA && rKeyCode.GetCode() != KEY_SPACE )
			pJavaSalMenuItem->mpVCLMenuItemData->setKeyboardShortcut( rKeyCode.GetCode(), rKeyCode.IsShift() );
	}
#endif	// USE_NATIVE_WINDOW
}

//-----------------------------------------------------------------------------

void JavaSalMenu::GetSystemMenuData( SystemMenuData* pData )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalMenu::GetSystemMenuData not implemented\n" );
#endif
}

// =======================================================================

JavaSalMenuItem::JavaSalMenuItem() :
#ifdef USE_NATIVE_WINDOW
	mpMenuItem( NULL ),
#else	// USE_NATIVE_WINDOW
	mpVCLMenuItemData( NULL ),
#endif	// USE_NATIVE_WINDOW
	mpSalSubmenu( NULL )
{
}

//-----------------------------------------------------------------------------

JavaSalMenuItem::~JavaSalMenuItem()
{
#ifdef USE_NATIVE_WINDOW
	fprintf( stderr, "JavaSalMenuItem::~JavaSalMenuItem not implemented\n" );
#else	// USE_NATIVE_WINDOW
	if( mpVCLMenuItemData )
	{
		mpVCLMenuItemData->dispose();
		delete mpVCLMenuItemData;
	}
#endif	// USE_NATIVE_WINDOW
}

//-----------------------------------------------------------------------------

SalMenu* JavaSalInstance::CreateMenu( BOOL bMenuBar, Menu *pVCLMenu )
{
#ifndef NO_NATIVE_MENUS
	JavaSalMenu *pSalMenu = new JavaSalMenu();
	pSalMenu->mbIsMenuBarMenu = bMenuBar;
#ifdef USE_NATIVE_WINDOW
	pSalMenu->mpMenuBar=NULL;
	pSalMenu->mpMenu=NULL;
#else	// USE_NATIVE_WINDOW
	pSalMenu->mpVCLMenuBar=NULL;
	pSalMenu->mpVCLMenu=NULL;
#endif	// USE_NATIVE_WINDOW
	pSalMenu->mpParentVCLMenu=pVCLMenu;

#ifdef USE_NATIVE_WINDOW
	fprintf( stderr, "JavaSalInstance::CreateMenu not implemented\n" );
#else	// USE_NATIVE_WINDOW
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
#endif	// USE_NATIVE_WINDOW

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
	OUString aTitle( title );
#ifdef USE_NATIVE_WINDOW
	fprintf( stderr, "JavaSalInstance::CreateMenuItem not implemented\n" );
#else	// USE_NATIVE_WINDOW
	pSalMenuItem->mpVCLMenuItemData=new ::vcl::com_sun_star_vcl_VCLMenuItemData( aTitle, ( pItemData->eType == MENUITEM_SEPARATOR ), pItemData->nId, pItemData->pMenu );
#endif	// USE_NATIVE_WINDOW
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
void UpdateMenusForFrame( JavaSalFrame *pFrame, JavaSalMenu *pMenu, bool bUpdateSubmenus )
{
#ifndef NO_NATIVE_MENUS
	// Don't allow updating of menus while we are resetting the show only menus
	// state or when the frame is not visible
	if ( !pFrame || !pFrame->mbVisible || pFrame->mbInShowOnlyMenus )
		return;

	if(!pMenu) {
		// locate the menubar for the frame
		pMenu = pFrame->mpMenuBar;
		if(!pMenu)
			return;
	}

	pFrame->maUpdateMenuList.remove( pMenu );

	// Check that menu has not been deleted
	::std::map< JavaSalMenu*, JavaSalMenu* >::const_iterator it = aMenuMap.find( pMenu );
	if ( it == aMenuMap.end() )
		return;

	Menu *pVCLMenu = pMenu->mpParentVCLMenu;
	if ( !pVCLMenu )
		return;

	// Force the clipboard service to update itself before we update the
	// menus as if the native clipboard was cleared when we last checked, we
	// won't be notified when another application puts content.
	if ( pMenu->mbIsMenuBarMenu && CFRunLoopGetCurrent() == CFRunLoopGetMain() )
	{
		// Reset the frame's menu update list
		pFrame->maUpdateMenuList.clear();

		Window *pWindow = pVCLMenu->GetWindow();
		if ( pWindow )
		{
			Reference< XClipboard > aClipboard = pWindow->GetClipboard();
			if ( aClipboard.is() )
				aClipboard->getContents();
		}
	}

	// Post the SALEVENT_MENUACTIVATE event
	SalMenuEvent *pActivateEvent = new SalMenuEvent( 0, pVCLMenu );
	com_sun_star_vcl_VCLEvent aActivateEvent( SALEVENT_MENUACTIVATE, pFrame, pActivateEvent );
	aActivateEvent.dispatch();

	USHORT nCount = pVCLMenu->GetItemCount();
	for( USHORT i = 0; i < nCount; i++ )
	{
		// If this menu item has a submenu, fix that submenu up
		JavaSalMenuItem *pSalMenuItem = (JavaSalMenuItem *)pVCLMenu->GetItemSalItem( i );
		if ( pSalMenuItem && pSalMenuItem->mpSalSubmenu )
		{
			if ( bUpdateSubmenus )
				UpdateMenusForFrame( pFrame, pSalMenuItem->mpSalSubmenu, true );
			else
				pFrame->maUpdateMenuList.push_back( pSalMenuItem->mpSalSubmenu );
		}
	}

	// Post the SALEVENT_MENUDEACTIVATE event
	SalMenuEvent *pDeactivateEvent = new SalMenuEvent( 0, pVCLMenu );
	com_sun_star_vcl_VCLEvent aDeactivateEvent( SALEVENT_MENUDEACTIVATE, pFrame, pDeactivateEvent );
	aDeactivateEvent.dispatch();
#endif	// !NO_NATIVE_MENUS
}
