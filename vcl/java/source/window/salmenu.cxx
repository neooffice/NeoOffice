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

#include <map>

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
#ifndef _SV_COM_SUN_STAR_VCL_VCLEVENT_HXX
#include <com/sun/star/vcl/VCLEvent.hxx>
#endif

#undef check

#define NATIVE_MENU_FRAMEWORK_CODE 'NWF '

struct InsertItemTimerParams
{
	SalMenu*			mpSalMenu;
	SalMenuItem*		mpSalMenuItem;
	unsigned			mnPos;
};

struct RemoveItemTimerParams
{
	SalMenu*			mpSalMenu;
	unsigned			mnPos;
};

struct SetSubMenuTimerParams
{
	SalMenu*			mpSalMenu;
	SalMenuItem*		mpSalMenuItem;
	SalMenu*			mpSubmenu;
	unsigned			mnPos;
};

struct CheckItemTimerParams
{
	SalMenu*			mpSalMenu;
	unsigned			mnPos;
	BOOL				mbCheck;
};

struct EnableItemTimerParams
{
	SalMenu*			mpSalMenu;
	unsigned			mnPos;
	BOOL				mbEnable;
};

struct SetItemTextTimerParams
{
	SalMenu*			mpSalMenu;
	unsigned			mnPos;
	SalMenuItem*		mpSalMenuItem;
	XubString			maText;
};

struct SetAcceleratorTimerParams
{
	SalMenu*			mpSalMenu;
	unsigned			mnPos;
	SalMenuItem*		mpSalMenuItem;
	KeyCode				maKeyCode;
	XubString			maKeyName;
};

struct UpdateMenusForFrameTimerParams
{
	SalFrame*			mpFrame;
	SalMenu*			mpMenu;
};

using namespace osl;
using namespace vcl;

static const bool bTrue = 1;
static const bool bFalse = 0;

static ::std::map< SalMenu*, SalMenu* > aMenuMap;
static ::std::map< SalMenuItem*, SalMenuItem* > aMenuItemMap;
static EventHandlerUPP pEventHandlerUPP = NULL;
static EventLoopTimerUPP pInsertItemTimerUPP = NULL;
static EventLoopTimerUPP pRemoveItemTimerUPP = NULL;
static EventLoopTimerUPP pSetSubMenuTimerUPP = NULL;
static EventLoopTimerUPP pCheckItemTimerUPP = NULL;
static EventLoopTimerUPP pEnableItemTimerUPP = NULL;
static EventLoopTimerUPP pSetItemTextTimerUPP = NULL;
static EventLoopTimerUPP pSetAcceleratorTimerUPP = NULL;
static EventLoopTimerUPP pUpdateMenusForFrameTimerUPP = NULL;
static EventLoopTimerUPP pSetActiveMenuBarForFrameTimerUPP = NULL;

//=============================================================================

static OSStatus CarbonMenuEventHandler( EventHandlerCallRef aNextHandler, EventRef aEvent, void *pData )
{
	if ( !Application::IsShutDown() )
	{
		SalData *pSalData = GetSalData();

		if ( pSalData && pSalData->mpEventQueue )
		{
			EventClass nClass = GetEventClass( aEvent );
			EventKind nKind = GetEventKind( aEvent );

    		if ( nClass == kEventClassCommand && nKind == kEventProcessCommand )
			{
                HICommandExtended aCommand;
                if ( GetEventParameter( aEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( HICommandExtended ), NULL, &aCommand ) == noErr && aCommand.commandID )
				{
					// Unlock the Java lock
					ReleaseJavaLock();

					// Make sure condition is not already waiting
					if ( !pSalData->maNativeEventCondition.check() )
					{
						pSalData->maNativeEventCondition.wait();
						pSalData->maNativeEventCondition.set();
					}

					// Wakeup the event queue by sending it a dummy event
					com_sun_star_vcl_VCLEvent aEvent( SALEVENT_USEREVENT, NULL, NULL );
					pSalData->mpEventQueue->postCachedEvent( &aEvent );

					// Wait for all pending AWT events to be dispatched
					pSalData->mbNativeEventSucceeded = false;
					pSalData->maNativeEventCondition.reset();
					pSalData->maNativeEventCondition.wait();
					pSalData->maNativeEventCondition.set();

					// Fix bug 679 by checking if the condition was
					// released to avoid a deadlock
					if ( pSalData->mbNativeEventSucceeded )
					{
						Application::GetSolarMutex().acquire();

						SalMenuItem *pMenuItem = (SalMenuItem *)aCommand.commandID;
						::std::map< SalMenuItem*, SalMenuItem* >::const_iterator it = aMenuItemMap.find( pMenuItem );
						if ( it != aMenuItemMap.end() )
						{
							SalMenu *pMenu = pMenuItem->maData.GetSalMenu();
							if ( pMenu )
							{
								USHORT nID = (USHORT)aCommand.source.menu.menuItemIndex;
								if ( pMenu->mpParentVCLMenu->GetItemSalItem( nID ) != pMenuItem )
								{
									// Handle cases where the menu items may have
									// changed since the event was generated
									nID = 0;
									USHORT nItems = pMenu->mpParentVCLMenu->GetItemCount();
									for ( USHORT i = 1; i <= nItems; i++ )
									{
										SalMenuItem *pItem = pMenu->mpParentVCLMenu->GetItemSalItem( i );
										if ( pItem == pMenuItem )
										{
											nID = i;
											break;
										}
									}
								}

								if ( nID )
								{
									SalMenuEvent *pMenuEvent = new SalMenuEvent();
									pMenuEvent->mnId = pMenu->mpParentVCLMenu->GetItemId( nID );
									pMenuEvent->mpMenu = pMenu->mpParentVCLMenu;;
									SalFrame *pFrame = NULL;
									while ( pMenu && ( pFrame = pMenu->maData.GetFrame() ) == NULL )
										pMenu = pMenu->maData.GetParentMenu();

									com_sun_star_vcl_VCLEvent aEvent( SALEVENT_MENUCOMMAND, pFrame, pMenuEvent );
									pSalData->mpEventQueue->postCachedEvent( &aEvent );     
								}
							}
						}

						// Relock the Java lock
						AcquireJavaLock();

						Application::GetSolarMutex().release();
					}
					else
					{
						// Relock the Java lock
						AcquireJavaLock();
					}

					return noErr;
				}
			}
		}
	}

	return CallNextEventHandler( aNextHandler, aEvent );
}

//-----------------------------------------------------------------------------

static void InsertItemCallback( EventLoopTimerRef aTimer, void *pData )
{
	// Unlock the Java lock
	ReleaseJavaLock();

	Application::GetSolarMutex().acquire();

	// Make sure that the params are still valid
	InsertItemTimerParams *pParams = (InsertItemTimerParams *)pData;
	::std::map< SalMenu*, SalMenu* >::const_iterator it = aMenuMap.find( pParams->mpSalMenu );
	if ( it != aMenuItemMap.end() )
	{
		::std::map< SalMenuItem*, SalMenuItem* >::const_iterator mit = aMenuItemMap.find( pParams->mpSalMenuItem );
		if ( mit != aMenuItemMap.end() )
			pParams->mpSalMenu->InsertItem( pParams->mpSalMenuItem, pParams->mnPos );
	}

	// Relock the Java lock
	AcquireJavaLock();

	Application::GetSolarMutex().release();

	delete pParams;
}
 
//-----------------------------------------------------------------------------

static void RemoveItemCallback( EventLoopTimerRef aTimer, void *pData )
{
	// Unlock the Java lock
	ReleaseJavaLock();

	Application::GetSolarMutex().acquire();

	// Make sure that the params are still valid
	RemoveItemTimerParams *pParams = (RemoveItemTimerParams *)pData;
	::std::map< SalMenu*, SalMenu* >::const_iterator it = aMenuMap.find( pParams->mpSalMenu );
	if ( it != aMenuItemMap.end() )
		pParams->mpSalMenu->RemoveItem( pParams->mnPos );

	// Relock the Java lock
	AcquireJavaLock();

	Application::GetSolarMutex().release();

	delete pParams;
}
 
//-----------------------------------------------------------------------------

static void SetSubMenuCallback( EventLoopTimerRef aTimer, void *pData )
{
	// Unlock the Java lock
	ReleaseJavaLock();

	Application::GetSolarMutex().acquire();

	// Make sure that the params are still valid
	SetSubMenuTimerParams *pParams = (SetSubMenuTimerParams *)pData;
	if ( pParams->mpSubmenu )
	{
		::std::map< SalMenu*, SalMenu* >::const_iterator smit = aMenuMap.find( pParams->mpSubmenu );
		if ( smit == aMenuItemMap.end() )
			pParams->mpSubmenu = NULL;
	}
	::std::map< SalMenu*, SalMenu* >::const_iterator it = aMenuMap.find( pParams->mpSalMenu );
	if ( it != aMenuItemMap.end() )
	{
		::std::map< SalMenuItem*, SalMenuItem* >::const_iterator mit = aMenuItemMap.find( pParams->mpSalMenuItem );
		if ( mit != aMenuItemMap.end() )
			pParams->mpSalMenu->SetSubMenu( pParams->mpSalMenuItem, pParams->mpSubmenu, pParams->mnPos );
	}

	// Relock the Java lock
	AcquireJavaLock();

	Application::GetSolarMutex().release();

	delete pParams;
}
 
//-----------------------------------------------------------------------------

static void CheckItemCallback( EventLoopTimerRef aTimer, void *pData )
{
	// Unlock the Java lock
	ReleaseJavaLock();

	Application::GetSolarMutex().acquire();

	// Make sure that the params are still valid
	CheckItemTimerParams *pParams = (CheckItemTimerParams *)pData;
	::std::map< SalMenu*, SalMenu* >::const_iterator it = aMenuMap.find( pParams->mpSalMenu );
	if ( it != aMenuItemMap.end() )
		pParams->mpSalMenu->CheckItem( pParams->mnPos, pParams->mbCheck );

	// Relock the Java lock
	AcquireJavaLock();

	Application::GetSolarMutex().release();

	delete pParams;
}
 
//-----------------------------------------------------------------------------

static void EnableItemCallback( EventLoopTimerRef aTimer, void *pData )
{
	// Unlock the Java lock
	ReleaseJavaLock();

	Application::GetSolarMutex().acquire();

	// Make sure that the params are still valid
	EnableItemTimerParams *pParams = (EnableItemTimerParams *)pData;
	::std::map< SalMenu*, SalMenu* >::const_iterator it = aMenuMap.find( pParams->mpSalMenu );
	if ( it != aMenuItemMap.end() )
		pParams->mpSalMenu->EnableItem( pParams->mnPos, pParams->mbEnable );

	// Relock the Java lock
	AcquireJavaLock();

	Application::GetSolarMutex().release();

	delete pParams;
}
 
//-----------------------------------------------------------------------------

static void SetItemTextCallback( EventLoopTimerRef aTimer, void *pData )
{
	// Unlock the Java lock
	ReleaseJavaLock();

	Application::GetSolarMutex().acquire();

	// Make sure that the params are still valid
	SetItemTextTimerParams *pParams = (SetItemTextTimerParams *)pData;
	::std::map< SalMenu*, SalMenu* >::const_iterator it = aMenuMap.find( pParams->mpSalMenu );
	if ( it != aMenuItemMap.end() )
	{
		::std::map< SalMenuItem*, SalMenuItem* >::const_iterator mit = aMenuItemMap.find( pParams->mpSalMenuItem );
		if ( mit != aMenuItemMap.end() )
			pParams->mpSalMenu->SetItemText( pParams->mnPos, pParams->mpSalMenuItem, pParams->maText );
	}

	// Relock the Java lock
	AcquireJavaLock();

	Application::GetSolarMutex().release();

	delete pParams;
}
 
//-----------------------------------------------------------------------------

static void SetAcceleratorCallback( EventLoopTimerRef aTimer, void *pData )
{
	// Unlock the Java lock
	ReleaseJavaLock();

	Application::GetSolarMutex().acquire();

	// Make sure that the params are still valid
	SetAcceleratorTimerParams *pParams = (SetAcceleratorTimerParams *)pData;
	::std::map< SalMenu*, SalMenu* >::const_iterator it = aMenuMap.find( pParams->mpSalMenu );
	if ( it != aMenuItemMap.end() )
	{
		::std::map< SalMenuItem*, SalMenuItem* >::const_iterator mit = aMenuItemMap.find( pParams->mpSalMenuItem );
		if ( mit != aMenuItemMap.end() )
			pParams->mpSalMenu->SetAccelerator( pParams->mnPos, pParams->mpSalMenuItem, pParams->maKeyCode, pParams->maKeyName );
	}

	// Relock the Java lock
	AcquireJavaLock();

	Application::GetSolarMutex().release();

	delete pParams;
}
 
//-----------------------------------------------------------------------------

static void UpdateMenusForFrameCallback( EventLoopTimerRef aTimer, void *pData )
{
	// Unlock the Java lock
	ReleaseJavaLock();

	Application::GetSolarMutex().acquire();

	// Make sure that the params are still valid
	SalData *pSalData = GetSalData();
	UpdateMenusForFrameTimerParams *pParams = (UpdateMenusForFrameTimerParams *)pData;
	for ( ::std::list< SalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
	{
		if ( pParams->mpFrame == *it )
		{
			UpdateMenusForFrame( pParams->mpFrame, pParams->mpMenu );
			break;
		}
	}

	// Relock the Java lock
	AcquireJavaLock();

	Application::GetSolarMutex().release();

	delete pParams;
}
 
//-----------------------------------------------------------------------------

static void SetActiveMenuBarForFrameCallback( EventLoopTimerRef aTimer, void *pData )
{
	// Unlock the Java lock
	ReleaseJavaLock();

	Application::GetSolarMutex().acquire();

	// Make sure that the params are still valid
	SalData *pSalData = GetSalData();
	SalFrame *pFrame = (SalFrame *)pData;
	for ( ::std::list< SalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
	{
		if ( pFrame == *it )
		{
			SetActiveMenuBarForFrame( pFrame );
			break;
		}
	}

	// Relock the Java lock
	AcquireJavaLock();

	Application::GetSolarMutex().release();
}
 
//=============================================================================

SalMenu::SalMenu()
{
	mpParentVCLMenu = NULL;
}

//-----------------------------------------------------------------------------

SalMenu::~SalMenu()
{
}

//-----------------------------------------------------------------------------

BOOL SalMenu::VisibleMenuBar()
{
	return TRUE;
}

//-----------------------------------------------------------------------------

void SalMenu::SetFrame( SalFrame *pFrame )
{
	if ( maData.mbIsMenuBarMenu )
	{
		if ( pFrame )
			pFrame->maFrameData.mpMenuBar = this;
		maData.mpFrame = pFrame;
	}
}

//-----------------------------------------------------------------------------

void SalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
	if ( !pSalMenuItem )
		return;

	if ( GetCurrentEventLoop() != GetMainEventLoop() )
	{
		if ( !pInsertItemTimerUPP )
			pInsertItemTimerUPP = NewEventLoopTimerUPP( InsertItemCallback );
		if ( pInsertItemTimerUPP )
		{
			InsertItemTimerParams *pParams = new InsertItemTimerParams();
			pParams->mpSalMenu = this;
			pParams->mpSalMenuItem = pSalMenuItem;
			pParams->mnPos = nPos;
			InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pInsertItemTimerUPP, (void *)pParams, NULL );
		}
		return;
	}

	if ( InsertMenuItemTextWithCFString( maData.maMenu, pSalMenuItem->maData.maTitle, nPos - 1, pSalMenuItem->maData.mbSeparator ? kMenuItemAttrSeparator : 0, (MenuCommand)pSalMenuItem ) == noErr )
	{
		pSalMenuItem->maData.mpSalMenu = this;
		EnableItem( nPos, TRUE );
	}
}

//-----------------------------------------------------------------------------

void SalMenu::RemoveItem( unsigned nPos )
{
	if ( GetCurrentEventLoop() != GetMainEventLoop() )
	{
		if ( !pRemoveItemTimerUPP )
			pRemoveItemTimerUPP = NewEventLoopTimerUPP( RemoveItemCallback );
		if ( pRemoveItemTimerUPP )
		{
			RemoveItemTimerParams *pParams = new RemoveItemTimerParams();
			pParams->mpSalMenu = this;
			pParams->mnPos = nPos;
			InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pRemoveItemTimerUPP, (void *)pParams, NULL );
		}
		return;
	}

	SalMenuItem *pSalMenuItem = mpParentVCLMenu->GetItemSalItem( nPos );
	if ( pSalMenuItem )
		pSalMenuItem->maData.mpSalMenu = NULL;
	DeleteMenuItem( maData.maMenu, nPos + 1 );
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
	if ( !pSalMenuItem )
		return;

	if ( GetCurrentEventLoop() != GetMainEventLoop() )
	{
		if ( !pSetSubMenuTimerUPP )
			pSetSubMenuTimerUPP = NewEventLoopTimerUPP( SetSubMenuCallback );
		if ( pSetSubMenuTimerUPP )
		{
			SetSubMenuTimerParams *pParams = new SetSubMenuTimerParams();
			pParams->mpSalMenu = this;
			pParams->mpSalMenuItem = pSalMenuItem;
			pParams->mpSubmenu = pSubMenu;
			pParams->mnPos = nPos;
			InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pSetSubMenuTimerUPP, (void *)pParams, NULL );
		}
		return;
	}

	if ( pSubMenu )
	{
		if ( SetMenuItemHierarchicalMenu( maData.maMenu, nPos + 1, pSubMenu->maData.maMenu ) == noErr )
		{
			SetMenuTitleWithCFString( pSubMenu->maData.maMenu, pSalMenuItem->maData.maTitle );
			pSubMenu->maData.mpParentMenu = this;
			pSalMenuItem->maData.mpSalSubmenu = pSubMenu;
		}
	}
	else
	{
		if ( SetMenuItemHierarchicalMenu( maData.maMenu, nPos + 1, NULL ) == noErr )
		{
			SetMenuItemTextWithCFString( maData.maMenu, nPos + 1, pSalMenuItem->maData.maTitle );
			pSalMenuItem->maData.mpSalSubmenu->maData.mpParentMenu = NULL;
			pSalMenuItem->maData.mpSalSubmenu = NULL;
		}
	}
}

//-----------------------------------------------------------------------------

void SalMenu::CheckItem( unsigned nPos, BOOL bCheck )
{
	if ( maData.maMenu && !maData.mbIsMenuBarMenu )
	{
		if ( GetCurrentEventLoop() != GetMainEventLoop() )
		{
			if ( !pCheckItemTimerUPP )
				pCheckItemTimerUPP = NewEventLoopTimerUPP( CheckItemCallback );
			if ( pCheckItemTimerUPP )
			{
				CheckItemTimerParams *pParams = new CheckItemTimerParams();
				pParams->mpSalMenu = this;
				pParams->mnPos = nPos;
				pParams->mbCheck = bCheck;
				InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pCheckItemTimerUPP, (void *)pParams, NULL );
			}
			return;
		}

		CheckMenuItem( maData.maMenu, nPos + 1, bCheck );
	}
}

//-----------------------------------------------------------------------------

void SalMenu::EnableItem( unsigned nPos, BOOL bEnable )
{
	if ( GetCurrentEventLoop() != GetMainEventLoop() )
	{
		if ( !pEnableItemTimerUPP )
			pEnableItemTimerUPP = NewEventLoopTimerUPP( EnableItemCallback );
		if ( pEnableItemTimerUPP )
		{
			EnableItemTimerParams *pParams = new EnableItemTimerParams();
			pParams->mpSalMenu = this;
			pParams->mnPos = nPos;
			pParams->mbEnable = bEnable;
			InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pEnableItemTimerUPP, (void *)pParams, NULL );
		}
		return;
	}

	SetMenuItemProperty( maData.maMenu, nPos + 1, NATIVE_MENU_FRAMEWORK_CODE, NATIVE_MENU_FRAMEWORK_CODE, sizeof( bool ), bEnable ? &bTrue : &bFalse );

	// Only disable menus when we are tracking the menubar to ensure that the
	// root menu's disabled shortcuts don't override the OOo shortcuts
	if ( bEnable || !GetSalData()->mbInNativeMenuTracking )
		EnableMenuItem( maData.maMenu, nPos + 1 );
	else
		DisableMenuItem( maData.maMenu, nPos + 1 );
}

//-----------------------------------------------------------------------------

void SalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage )
{
	// For now we'll ignore putting icons in native menus.  Most Mac apps don't
	// have them, so they're kind of extraneous on the platform anyhow.
}

//-----------------------------------------------------------------------------

void SalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText )
{
	if ( !pSalMenuItem )
		return;

	if ( GetCurrentEventLoop() != GetMainEventLoop() )
	{
		if ( !pSetItemTextTimerUPP )
			pSetItemTextTimerUPP = NewEventLoopTimerUPP( SetItemTextCallback );
		if ( pSetItemTextTimerUPP )
		{
			SetItemTextTimerParams *pParams = new SetItemTextTimerParams();
			pParams->mpSalMenu = this;
			pParams->mnPos = nPos;
			pParams->mpSalMenuItem = pSalMenuItem;
			pParams->maText = XubString( rText );
			InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pSetItemTextTimerUPP, (void *)pParams, NULL );
		}
		return;
	}

	if ( pSalMenuItem->maData.maTitle )
		CFRelease( pSalMenuItem->maData.maTitle );
	XubString aTitle( rText );
	aTitle.EraseAllChars( '~' );
	pSalMenuItem->maData.maTitle = CFStringCreateWithCharacters( NULL, aTitle.GetBuffer(), aTitle.Len() );

	if ( pSalMenuItem->maData.mpSalSubmenu )
		SetMenuTitleWithCFString( pSalMenuItem->maData.mpSalSubmenu->maData.maMenu, pSalMenuItem->maData.maTitle );
	else
		SetMenuItemTextWithCFString( maData.maMenu, nPos + 1, pSalMenuItem->maData.maTitle );
}

//-----------------------------------------------------------------------------

void SalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName )
{
	if ( GetCurrentEventLoop() != GetMainEventLoop() )
	{
		if ( !pSetAcceleratorTimerUPP )
			pSetAcceleratorTimerUPP = NewEventLoopTimerUPP( SetAcceleratorCallback );
		if ( pSetAcceleratorTimerUPP )
		{
			SetAcceleratorTimerParams *pParams = new SetAcceleratorTimerParams();
			pParams->mpSalMenu = this;
			pParams->mnPos = nPos;
			pParams->mpSalMenuItem = pSalMenuItem;
			pParams->maKeyCode = rKeyCode;
			pParams->maKeyName = XubString( rKeyName );
			InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pSetAcceleratorTimerUPP, (void *)pParams, NULL );
		}
		return;
	}

	USHORT nCode = rKeyCode.GetCode();
	UInt8 nKey = 0;
	MacOSBoolean bVirtual = false;

	if ( nCode >= KEY_0 && nCode <= KEY_9 )
	{
		nKey = (UInt8)( nCode - KEYGROUP_NUM ) + 0x30;
	}
	else if ( nCode >= KEY_A && nCode <= KEY_Z )
	{
		// Don't allow overriding of the standard shortcuts in the root menu
		if ( ( nCode == KEY_D && rKeyCode.IsMod1() && rKeyCode.IsMod2() && !rKeyCode.IsShift() && !rKeyCode.IsControlMod() ) || ( nCode == KEY_H && rKeyCode.IsMod1() && !rKeyCode.IsShift() && !rKeyCode.IsControlMod() ) || ( nCode == KEY_Q && rKeyCode.IsMod1() && !rKeyCode.IsShift() && !rKeyCode.IsControlMod() ) )
			return;
		nKey = (UInt8)( nCode - KEYGROUP_ALPHA ) + 0x41;
	}
	else
	{
		switch ( nCode )
		{
			case KEY_F1:
				nKey = 0x7a;
				bVirtual = true;
				break;
			case KEY_F2:
				nKey = 0x78;
				bVirtual = true;
				break;
			case KEY_F3:
				return;
			case KEY_F4:
				nKey = 0x76;
				bVirtual = true;
				break;
			case KEY_F5:
				nKey = 0x60;
				bVirtual = true;
				break;
			case KEY_F6:
				nKey = 0x61;
				bVirtual = true;
				break;
			case KEY_F7:
				nKey = 0x62;
				bVirtual = true;
				break;
			case KEY_F8:
				nKey = 0x64;
				bVirtual = true;
				break;
			case KEY_F9:
				nKey = 0x65;
				bVirtual = true;
				break;
			case KEY_F10:
				nKey = 0x6d;
				bVirtual = true;
				break;
			case KEY_F11:
				nKey = 0x67;
				bVirtual = true;
				break;
			case KEY_F12:
				nKey = 0x6f;
				bVirtual = true;
				break;
			case KEY_F13:
				nKey = 0x79;
				bVirtual = true;
				break;
			case KEY_F14:
				nKey = 0x79;
				bVirtual = true;
				break;
			case KEY_F15:
				nKey = 0x79;
				bVirtual = true;
				break;
			case KEY_F16:
			case KEY_F17:
			case KEY_F18:
			case KEY_F19:
			case KEY_F20:
			case KEY_F21:
			case KEY_F22:
			case KEY_F23:
			case KEY_F24:
			case KEY_F25:
			case KEY_F26:
				return;
			case KEY_DOWN:
				nKey = 0x7d;
				bVirtual = true;
				break;
			case KEY_UP:
				nKey = 0x7e;
				bVirtual = true;
				break;
			case KEY_LEFT:
				nKey = 0x7b;
				bVirtual = true;
				break;
			case KEY_RIGHT:
				nKey = 0x7c;
				bVirtual = true;
				break;
			case KEY_HOME:
				nKey = 0x73;
				bVirtual = true;
				break;
			case KEY_END:
				nKey = 0x77;
				bVirtual = true;
				break;
			case KEY_PAGEUP:
				nKey = 0x74;
				bVirtual = true;
				break;
			case KEY_PAGEDOWN:
				nKey = 0x79;
				bVirtual = true;
				break;
			case KEY_TAB:
				return;
			case KEY_ESCAPE:
				nKey = 0x35;
				bVirtual = true;
				break;
			case KEY_BACKSPACE:
			case KEY_SPACE:
			case KEY_INSERT:
				return;
			case KEY_DELETE:
				nKey = 0x33;
				bVirtual = true;
				break;
			case KEY_ADD:
				nKey = 0x2b;
				break;
			case KEY_SUBTRACT:
				nKey = 0x2d;
				break;
			case KEY_MULTIPLY:
				nKey = 0x2a;
				break;
			case KEY_DIVIDE:
				nKey = 0x2f;
				break;
			case KEY_POINT:
				nKey = 0x2e;
				break;
			case KEY_COMMA:
				nKey = 0x2c;
				break;
			case KEY_LESS:
				nKey = 0x3c;
				break;
			case KEY_GREATER:
				nKey = 0x3e;
				break;
			case KEY_EQUAL:
				nKey = 0x3d;
				break;
			case KEY_OPEN:
			case KEY_CUT:
			case KEY_COPY:
			case KEY_PASTE:
			case KEY_UNDO:
			case KEY_REPEAT:
			case KEY_FIND:
			case KEY_PROPERTIES:
			case KEY_FRONT:
			case KEY_CONTEXTMENU:
			case KEY_MENU:
				return;
			case KEY_HELP:
				nKey = 0x72;
				bVirtual = true;
				break;
			case KEY_HANGUL_HANJA:
				return;
			default:
				return;
		}
	}

	if ( SetMenuItemCommandKey( maData.maMenu, nPos + 1, bVirtual, nKey ) == noErr )
	{
		UInt8 nModifiers = kMenuNoModifiers;
		if ( !rKeyCode.IsMod1() )
			nModifiers |= kMenuNoCommandModifier;
		if ( rKeyCode.IsShift() )
			nModifiers |= kMenuShiftModifier;
		if ( rKeyCode.IsMod2() )
			nModifiers |= kMenuOptionModifier;
		if ( rKeyCode.IsControlMod() )
			nModifiers |= kMenuControlModifier;

		SetMenuItemModifiers( maData.maMenu, nPos + 1, nModifiers );
	}
}

//-----------------------------------------------------------------------------

void SalMenu::GetSystemMenuData( SystemMenuData* pData )
{
}

// =======================================================================

SalMenuItem::SalMenuItem()
{
}

//-----------------------------------------------------------------------------

SalMenuItem::~SalMenuItem()
{
}

//-----------------------------------------------------------------------------

SalMenu* SalInstance::CreateMenu( BOOL bMenuBar, Menu* pVCLMenu )
{
	SalMenu *pSalMenu = new SalMenu();

	pSalMenu->maData.mbIsMenuBarMenu = bMenuBar;
	pSalMenu->mpParentVCLMenu = pVCLMenu;

	if ( CreateNewMenu( 0, 0, &pSalMenu->maData.maMenu ) != noErr )
	{
		delete pSalMenu;
		return NULL;
	}

	aMenuMap[ pSalMenu ] = pSalMenu;

	if ( !pEventHandlerUPP )
		pEventHandlerUPP = NewEventHandlerUPP( CarbonMenuEventHandler );

	if ( pEventHandlerUPP )
	{
		// Set up native event handler
		EventTypeSpec aType;
		aType.eventClass = kEventClassCommand;
		aType.eventKind = kEventProcessCommand;
		InstallMenuEventHandler( pSalMenu->maData.maMenu, pEventHandlerUPP, 1, &aType, NULL, NULL );
	}

	return pSalMenu;
}

//-----------------------------------------------------------------------------

void SalInstance::DestroyMenu( SalMenu* pMenu )
{
	if ( pMenu )
	{
		aMenuMap.erase( pMenu );
		delete pMenu;
	}
}

//-----------------------------------------------------------------------------

SalMenuItem* SalInstance::CreateMenuItem( const SalItemParams* pItemData )
{
	if ( !pItemData )
		return NULL;

	SalMenuItem *pSalMenuItem = new SalMenuItem();

	XubString aTitle( pItemData->aText );
	aTitle.EraseAllChars( '~' );
	pSalMenuItem->maData.maTitle = CFStringCreateWithCharacters( NULL, aTitle.GetBuffer(), aTitle.Len() );
	if ( pItemData->eType == MENUITEM_SEPARATOR )
		pSalMenuItem->maData.mbSeparator = true;

	aMenuItemMap[ pSalMenuItem ] = pSalMenuItem;

	return pSalMenuItem;
}

//-----------------------------------------------------------------------------

void SalInstance::DestroyMenuItem( SalMenuItem* pItem )
{
	if ( pItem )
	{
		aMenuItemMap.erase( pItem );
		delete pItem;
	}
}

//=============================================================================

SalMenuData::SalMenuData()
{
	maMenu = NULL;
	mpFrame = NULL;
	mbIsMenuBarMenu = false;
	mpParentMenu = NULL;
}

//-----------------------------------------------------------------------------

SalMenuData::~SalMenuData()
{
	if ( maMenu )
		ReleaseMenu( maMenu );
}

//=============================================================================

SalMenuItemData::SalMenuItemData()
{
	maTitle = NULL;
	mbSeparator = false;
	mpSalMenu = NULL;
	mpSalSubmenu = NULL;
}

//-----------------------------------------------------------------------------

SalMenuItemData::~SalMenuItemData()
{
	if ( maTitle )
		CFRelease( maTitle );
}

// ============================================================================

/**
 * Given a frame, post SALEVENT_MENUACTIVATE and SALEVENT_MENUDEACTIVATE events
 * to all of the VCL menu objects in the frame's menubar.
 */
void UpdateMenusForFrame( SalFrame *pFrame, SalMenu *pMenu )
{
	if ( !pFrame )
		return;

	if ( !pMenu )
	{
		pMenu = pFrame->maFrameData.mpMenuBar;
		if ( !pMenu )
			return;
	}

	if ( GetCurrentEventLoop() != GetMainEventLoop() )
	{
		if ( !pUpdateMenusForFrameTimerUPP )
			pUpdateMenusForFrameTimerUPP = NewEventLoopTimerUPP( UpdateMenusForFrameCallback );
		if ( pUpdateMenusForFrameTimerUPP )
		{
			UpdateMenusForFrameTimerParams *pParams = new UpdateMenusForFrameTimerParams();
			pParams->mpFrame = pFrame;
			pParams->mpMenu = pMenu;
			InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pUpdateMenusForFrameTimerUPP, (void *)pParams, NULL );
		}
		return;
	}

	Menu *pVCLMenu = pMenu->mpParentVCLMenu;

	// Post the SALEVENT_MENUACTIVATE event
	SalMenuEvent *pActivateEvent = new SalMenuEvent();
	pActivateEvent->mnId = 0;
	pActivateEvent->mpMenu = pVCLMenu;
	com_sun_star_vcl_VCLEvent aActivateEvent( SALEVENT_MENUACTIVATE, pFrame, pActivateEvent );
	aActivateEvent.dispatch();

	USHORT nCount = pVCLMenu->GetItemCount();
	for( USHORT i = 0; i < nCount; i++ )
	{
		// If this menu item has a submenu, fix that submenu up
		SalMenuItem *pSalMenuItem = pVCLMenu->GetItemSalItem( i );
		if ( pSalMenuItem && pSalMenuItem->maData.mpSalSubmenu )
			UpdateMenusForFrame( pFrame, pSalMenuItem->maData.mpSalSubmenu );
	}

	// Post the SALEVENT_MENUDEACTIVATE event
	SalMenuEvent *pDeactivateEvent = new SalMenuEvent();
	pDeactivateEvent->mnId = 0;
	pDeactivateEvent->mpMenu = pVCLMenu;
	com_sun_star_vcl_VCLEvent aDeactivateEvent( SALEVENT_MENUDEACTIVATE, pFrame, pDeactivateEvent );
	aDeactivateEvent.dispatch();

	UInt16 nItems = CountMenuItems( pMenu->maData.maMenu );
	if ( GetSalData()->mbInNativeMenuTracking )
	{
		// Set actual enable state since the menus are visible
		bool bEnable;
		for ( UInt16 i = 1; i <= nItems; i++ )
		{
			if ( GetMenuItemProperty( pMenu->maData.maMenu, i, NATIVE_MENU_FRAMEWORK_CODE, NATIVE_MENU_FRAMEWORK_CODE, sizeof( bool ), NULL, &bEnable ) == noErr )
			{
				if ( bEnable )
					EnableMenuItem( pMenu->maData.maMenu, i );
				else
					DisableMenuItem( pMenu->maData.maMenu, i );
			}
		}
	}
	else
	{
		// Enable all of the menus to ensure that the root menu's disabled
		// shortcuts don't override the OOo shortcuts
		for ( UInt16 i = 1; i <= nItems; i++ )
			EnableMenuItem( pMenu->maData.maMenu, i );
	}
}

/**
 * Given a frame, add all of the VCL menu objects in the system menubar.
 */
void SetActiveMenuBarForFrame( SalFrame *pFrame )
{
	if ( GetCurrentEventLoop() != GetMainEventLoop() )
	{
		if ( !pSetActiveMenuBarForFrameTimerUPP )
			pSetActiveMenuBarForFrameTimerUPP = NewEventLoopTimerUPP( SetActiveMenuBarForFrameCallback );
		if ( pSetActiveMenuBarForFrameTimerUPP )
			InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pSetActiveMenuBarForFrameTimerUPP, (void *)pFrame, NULL );
		return;
	}

	static MenuBarHandle hMenuBar = GetMenuBar();

	// Restore menubar to original empty menubar
	SetMenuBar( hMenuBar );

	if ( pFrame && pFrame->maFrameData.mpMenuBar && pFrame->maFrameData.mbVisible )
	{
		// Insert menus into menubar
		UInt16 nItems = CountMenuItems( pFrame->maFrameData.mpMenuBar->maData.maMenu );
		for ( UInt16 i = 1; i <= nItems; i++ )
		{
			MenuRef aMenu;
			if ( GetMenuItemHierarchicalMenu( pFrame->maFrameData.mpMenuBar->maData.maMenu, i, &aMenu ) == noErr )
				InsertMenu( aMenu, 0 );
		}
	}

	// Update the menubar
	DrawMenuBar();
}
