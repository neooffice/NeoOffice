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

#include <osl/objcutils.h>
#include <vcl/window.hxx>
#include <framework/menuconfiguration.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>

#include "java/saldata.hxx"
#include "java/salframe.h"
#include "java/salinst.h"
#include "java/salmenu.h"

#include "../app/salinst_cocoa.h"
#include "../java/VCLApplicationDelegate_cocoa.h"

// Comment out the following line to disable using the native windows menu
#define USE_NATIVE_WINDOWS_MENU

#define MAIN_MENU_CHANGE_WAIT_INTERVAL ( (NSTimeInterval)0.25f )

static ::std::map< JavaSalMenu*, JavaSalMenu* > aMenuMap;

using namespace com::sun::star;
using namespace vcl;

@interface VCLMenuWrapperArgs : NSObject
{
	NSArray*				mpArgs;
	NSObject*				mpResult;
}
+ (id)argsWithArgs:(NSArray *)pArgs;
- (NSArray *)args;
- (void)dealloc;
- (id)initWithArgs:(NSArray *)pArgs;
- (NSObject *)result;
- (void)setResult:(NSObject *)pResult;
@end

@implementation VCLMenuWrapperArgs

+ (id)argsWithArgs:(NSArray *)pArgs
{
	VCLMenuWrapperArgs *pRet = [[VCLMenuWrapperArgs alloc] initWithArgs:(NSArray *)pArgs];
	[pRet autorelease];
	return pRet;
}

- (NSArray *)args
{
	return mpArgs;
}

- (void)dealloc
{
	if ( mpArgs )
		[mpArgs release];

	if ( mpResult )
		[mpResult release];

	[super dealloc];
}

- (id)initWithArgs:(NSArray *)pArgs
{
	[super init];

	mpResult = nil;
	mpArgs = pArgs;
	if ( mpArgs )
		[mpArgs retain];

	return self;
}

- (NSObject *)result
{
	return mpResult;
}

- (void)setResult:(NSObject *)pResult
{
	if ( mpResult )
		[mpResult release];

	mpResult = pResult;

	if ( mpResult )
		[mpResult retain];
}

@end

@interface VCLMenu : NSMenu
{
}
- (id)copyWithZone:(NSZone *)pZone;
- (id)initWithTitle:(NSString *)pTitle;
@end

@interface VCLMenuWrapper : NSObject
{
	JavaSalFrame*			mpFrame;
	VCLMenu*				mpMenu;
	BOOL					mbMenuBar;
	NSMutableArray*			mpMenuItems;
}
- (id)init:(BOOL)bMenuBar;
- (void)checkMenuItem:(VCLMenuWrapperArgs *)pArgs;
- (void)destroy:(id)pObject;
- (void)enableMenuItem:(VCLMenuWrapperArgs *)pArgs;
- (void)insertMenuItem:(VCLMenuWrapperArgs *)pArgs;
- (VCLMenu *)menu;
- (void)removeMenuAsMainMenu:(id)pObject;
- (void)removeMenuItem:(VCLMenuWrapperArgs *)pArgs;
- (void)setFrame:(VCLMenuWrapperArgs *)pArgs;
- (void)setMenuAsMainMenu:(id)pObject;
- (void)setMenuItemKeyEquivalent:(VCLMenuWrapperArgs *)pArgs;
- (void)setMenuItemSubmenu:(VCLMenuWrapperArgs *)pArgs;
- (void)setMenuItemTitle:(VCLMenuWrapperArgs *)pArgs;
@end

static NSTimeInterval nLastMenuItemSelectedTime = 0;
static JavaSalFrame *pMenuBarFrame = NULL;
static VCLMenuWrapper *pMenuBarMenu = nil;
static VCLMenuWrapper *pPendingSetMenuAsMainMenu = nil;
static BOOL bRemovePendingSetMenuAsMainMenu = NO;

@implementation VCLMenu

- (id)copyWithZone:(NSZone *)pZone
{
	VCLMenu *pRet = [[VCLMenu alloc] initWithTitle:[self title]];
	if ( pRet )
	{
		for ( NSMenuItem *pItem in [self itemArray] )
		{
			if ( pItem )
			{
				pItem = [pItem copyWithZone:pZone];
				if ( pItem )
				{
					[pItem autorelease];
					[pRet addItem:pItem];
				}
			}
		}
	}

	return pRet;
}

- (id)initWithTitle:(NSString *)pTitle
{
	[super initWithTitle:pTitle];

	[self setDelegate:[VCLApplicationDelegate sharedDelegate]];

	return self;
}

@end

@implementation VCLMainMenuDidEndTracking

+ (void)mainMenuDidEndTracking:(BOOL)bNoDelay
{
	VCLMainMenuDidEndTracking *pVCLMainMenuDidEndTracking = [[VCLMainMenuDidEndTracking alloc] init];
	[pVCLMainMenuDidEndTracking autorelease];

	// Queue processing to occur after a very slight delay
	if ( bNoDelay )
		[pVCLMainMenuDidEndTracking handlePendingMainMenuChanges:[NSNumber numberWithBool:bNoDelay]];
	else
		[pVCLMainMenuDidEndTracking performSelector:@selector(handlePendingMainMenuChanges:) withObject:nil afterDelay:MAIN_MENU_CHANGE_WAIT_INTERVAL inModes:osl_getStandardRunLoopModes()];
}

- (void)handlePendingMainMenuChanges:(id)pObject
{
	VCLApplicationDelegate *pAppDelegate = [VCLApplicationDelegate sharedDelegate];
	if ( ( pAppDelegate && ( [pAppDelegate isInPerformKeyEquivalent] || [pAppDelegate isInTracking] ) ) || nLastMenuItemSelectedTime > [NSDate timeIntervalSinceReferenceDate] || NSApplication_getModalWindow() )
	{
		// Requeue this operation to occur later if the no delay flag is not set
		if ( !pObject || ![pObject isKindOfClass:[NSNumber class]] || ![(NSNumber *)pObject boolValue] )
		{
			if ( pPendingSetMenuAsMainMenu )
				[VCLMainMenuDidEndTracking mainMenuDidEndTracking:NO];
			return;
		}
	}

	if ( pPendingSetMenuAsMainMenu )
	{
		if ( bRemovePendingSetMenuAsMainMenu )
			[pPendingSetMenuAsMainMenu removeMenuAsMainMenu:nil];
		else
			[pPendingSetMenuAsMainMenu setMenuAsMainMenu:nil];
	}
}

@end

@interface VCLMenuItem : NSMenuItem
{
	sal_uInt16				mnID;
	Menu*					mpMenu;
	BOOL					mbReallyEnabled;
	MenuItemType			meType;
}
- (sal_uInt16)ID;
- (id)copyWithZone:(NSZone *)pZone;
- (id)initWithTitle:(NSString *)pTitle type:(MenuItemType)eType id:(sal_uInt16)nID menu:(Menu *)pMenu;
- (BOOL)isReallyEnabled;
- (void)selected;
- (void)setMenuType:(NSNumber *)pMenuType;
- (void)setReallyEnabled:(BOOL)bEnabled;
- (BOOL)validateMenuItem:(NSMenuItem *)pMenuItem;
@end

@implementation VCLMenuWrapper

- (id)init:(BOOL)bMenuBar
{
	[super init];

	mpMenu = nil;
	mbMenuBar = bMenuBar;
	mpMenuItems = [NSMutableArray arrayWithCapacity:10];
	if ( mpMenuItems )
		[mpMenuItems retain];

	return self;
}

- (void)checkMenuItem:(VCLMenuWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

    NSNumber *pPos = (NSNumber *)[pArgArray objectAtIndex:0];
    if ( !pPos )
        return;

    NSNumber *pCheck = (NSNumber *)[pArgArray objectAtIndex:1];
    if ( !pCheck )
        return;

    unsigned int nPos = [pPos unsignedIntValue];
	if ( mpMenuItems && nPos < [mpMenuItems count] )
	{
		NSMenuItem *pMenuItem = [mpMenuItems objectAtIndex:nPos];
		if ( pMenuItem )
			[pMenuItem setState:( [pCheck boolValue] ? NSControlStateValueOn : NSControlStateValueOff )];
	}
}

- (void)destroy:(id)pObject
{
	// Attempt to fix Mac App Store crash when opening a menu by delaying
	// release of the native menu and menu items
	VCLApplicationDelegate *pAppDelegate = [VCLApplicationDelegate sharedDelegate];
	if ( pObject || ( pAppDelegate && ( [pAppDelegate isInPerformKeyEquivalent] || [pAppDelegate isInTracking] ) ) )
	{
		[self performSelector:@selector(destroy:) withObject:nil afterDelay:MAIN_MENU_CHANGE_WAIT_INTERVAL];
		return;
	}

	[self removeMenuAsMainMenu:self];

	if ( mpMenu )
	{
		// Fix Mac App Store crash by removing delegate
		[mpMenu setDelegate:nil];

		NSMenu *pSupermenu = [mpMenu supermenu];
		if ( pSupermenu )
		{
			NSUInteger nCount = [pSupermenu numberOfItems];
			if ( nCount > 0 )
			{
				// Remove remaining menus while still showing
				NSUInteger i = nCount - 1;
				for ( ; i > 0; i-- )
				{
					NSMenuItem *pMenuItem = [pSupermenu itemAtIndex:i];
					if ( pMenuItem && [pMenuItem submenu] == mpMenu )
						[pSupermenu removeItem:pMenuItem];
				}
			}
		}

		[mpMenu release];
		mpMenu = nil;
	}

	if ( mpMenuItems )
	{
		for ( NSMenuItem *pMenuItem in mpMenuItems )
		{
			if ( pMenuItem )
			{
				NSMenu *pMenu = [pMenuItem menu];
				if ( pMenu )
					[pMenu removeItem:pMenuItem];
			}
		}

		[mpMenuItems release];
		mpMenuItems = nil;
	}
}

- (void)enableMenuItem:(VCLMenuWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

    NSNumber *pPos = (NSNumber *)[pArgArray objectAtIndex:0];
    if ( !pPos )
        return;

    NSNumber *pEnable = (NSNumber *)[pArgArray objectAtIndex:1];
    if ( !pEnable )
        return;

    unsigned int nPos = [pPos unsignedIntValue];
	if ( mpMenuItems && nPos < [mpMenuItems count] )
	{
		NSMenuItem *pMenuItem = [mpMenuItems objectAtIndex:nPos];
		if ( pMenuItem )
		{
			// Fix bug described at the end of the following NeoOffice forum
			// post by ignoring any menu item disabling calls not made by our
			// own code:
			// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=64112#64112
			if ( [pMenuItem isKindOfClass:[VCLMenuItem class]] )
				[(VCLMenuItem *)pMenuItem setReallyEnabled:[pEnable boolValue]];
			else
				[pMenuItem setEnabled:[pEnable boolValue]];
		}
	}
}

- (void)insertMenuItem:(VCLMenuWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

	NSMenuItem *pMenuItem = (NSMenuItem *)[pArgArray objectAtIndex:0];
	if ( !pMenuItem )
		return;

    NSNumber *pPos = (NSNumber *)[pArgArray objectAtIndex:1];
    if ( !pPos )
        return;

    unsigned int nPos = [pPos unsignedIntValue];
	if ( mpMenuItems )
	{
		if ( nPos < [mpMenuItems count] )
			[mpMenuItems insertObject:pMenuItem atIndex:nPos];
		else
			[mpMenuItems addObject:pMenuItem];

		if ( mpMenu )
		{
			if ( nPos < (unsigned int)[mpMenu numberOfItems] )
				[mpMenu insertItem:pMenuItem atIndex:nPos];
			else
				[mpMenu addItem:pMenuItem];
		}
	}
}

- (VCLMenu *)menu
{
	if ( !mpMenu && !mbMenuBar && mpMenuItems )
	{
		mpMenu = [[VCLMenu alloc] initWithTitle:@""];
		if ( mpMenu )
		{
			for ( NSMenuItem *pMenuItem in mpMenuItems )
			{
				if ( pMenuItem )
					[mpMenu addItem:pMenuItem];
			}
		}
	}

	return mpMenu;
}

- (void)removeMenuAsMainMenu:(id)pObject
{
	(void)pObject;

	if ( self != pMenuBarMenu )
		return;

	// Fix highlighted menu item bug reported in the following forum post by
	// not changing the main menu when a modal or sheet window is being displayed.
	// This should be safe since menubar tracking is disabled in the
	// VCLApplicationDelegate class when this case is true:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=62810#62810
	VCLApplicationDelegate *pAppDelegate = [VCLApplicationDelegate sharedDelegate];
	if ( ( pAppDelegate && ( [pAppDelegate isInPerformKeyEquivalent] || [pAppDelegate isInTracking] ) ) || nLastMenuItemSelectedTime > [NSDate timeIntervalSinceReferenceDate] || NSApplication_getModalWindow() )
	{
		if ( !pPendingSetMenuAsMainMenu )
		{
			pPendingSetMenuAsMainMenu = self;
			[pPendingSetMenuAsMainMenu retain];
			bRemovePendingSetMenuAsMainMenu = YES;
		}
		else if ( !bRemovePendingSetMenuAsMainMenu && pPendingSetMenuAsMainMenu == self )
		{
			bRemovePendingSetMenuAsMainMenu = YES;
		}

		[VCLMainMenuDidEndTracking mainMenuDidEndTracking:NO];

		return;
	}

	pMenuBarFrame = NULL;
	pMenuBarMenu = nil;

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSMenu *pMainMenu = [pApp mainMenu];
		if ( pMainMenu )
		{
			// Just to be safe, cancel tracking before making any changes
			[pMainMenu cancelTracking];

			NSUInteger nCount = [pMainMenu numberOfItems];
			if ( nCount > 0 )
			{
				// Remove remaining menus while still showing
				NSUInteger i = nCount - 1;
				for ( ; i > 0; i-- )
					[pMainMenu removeItemAtIndex:i];
			}
		}
	}

	// Fix empty menu bug that occurs when opening a document using the
	// File :: Recent Documents menu by clearing the pending main menu. Fix
	// empty menu bug when using clicking on the Window :: Close menu by
	// only applying the fix for the File :: Recent Documents menu if the
	// pending main menu is set to the menu that was just removed.
	if ( pPendingSetMenuAsMainMenu == self )
	{
		[pPendingSetMenuAsMainMenu release];
		pPendingSetMenuAsMainMenu = nil;
		bRemovePendingSetMenuAsMainMenu = NO;
	}
}

- (void)removeMenuItem:(VCLMenuWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

    NSNumber *pPos = (NSNumber *)[pArgArray objectAtIndex:0];
    if ( !pPos )
        return;

    unsigned int nPos = [pPos unsignedIntValue];
	if ( mpMenuItems && nPos < [mpMenuItems count] )
	{
		NSMenuItem *pMenuItem = [mpMenuItems objectAtIndex:nPos];
		if ( pMenuItem )
		{
			[mpMenuItems removeObject:pMenuItem];

			if ( mpMenu )
				[mpMenu removeItem:pMenuItem];
		}
	}
}

- (void)setFrame:(VCLMenuWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

    NSNumber *pNumber = (NSNumber *)[pArgArray objectAtIndex:0];
    if ( !pNumber )
        return;

	mpFrame = (JavaSalFrame *)[pNumber unsignedLongValue];
}

- (void)setMenuAsMainMenu:(id)pObject
{
	(void)pObject;

	if ( !mbMenuBar || self == pMenuBarMenu )
		return;

	// Fix highlighted menu item bug reported in the following forum post by
	// not changing the main menu when a modal or sheet window is being displayed.
	// This should be safe since menubar tracking is disabled in the
	// VCLApplicationDelegate class when this case is true:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=62810#62810
	VCLApplicationDelegate *pAppDelegate = [VCLApplicationDelegate sharedDelegate];
	if ( ( pAppDelegate && ( [pAppDelegate isInPerformKeyEquivalent] || [pAppDelegate isInTracking] ) ) || nLastMenuItemSelectedTime > [NSDate timeIntervalSinceReferenceDate] || NSApplication_getModalWindow() )
	{
		if ( pPendingSetMenuAsMainMenu != self )
		{
			if ( pPendingSetMenuAsMainMenu )
				[pPendingSetMenuAsMainMenu release];
			pPendingSetMenuAsMainMenu = self;
			[pPendingSetMenuAsMainMenu retain];
		}

		bRemovePendingSetMenuAsMainMenu = NO;

		[VCLMainMenuDidEndTracking mainMenuDidEndTracking:NO];

		return;
	}

	pMenuBarFrame = mpFrame;
	pMenuBarMenu = self;

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSMenu *pMainMenu = [pApp mainMenu];
		if ( pMainMenu )
		{
			// Just to be safe, cancel tracking before making any changes
			[pMainMenu cancelTracking];

			NSUInteger nCount = [pMainMenu numberOfItems];
			if ( nCount > 0 )
			{
				// Remove remaining menus while still showing
				NSUInteger i = nCount - 1;
				for ( ; i > 0; i-- )
					[pMainMenu removeItemAtIndex:i];

				// Add our menu items to menubar. Note that we must add
				// the menu items after the menu has been set as the
				// main menu for [VCLApplicationDelegate addMenuItem:]
				// to get called.
				for ( NSMenuItem *pMenuItem in mpMenuItems )
				{
					if ( pMenuItem )
					{
						NSMenu *pMenu = [pMenuItem menu];
						if ( pMenu )
							[pMenu removeItem:pMenuItem];

						NSMenu *pSubmenu = [pMenuItem submenu];
						if ( pSubmenu )
						{
							if ( pMenuItem.representedObject && [pMenuItem.representedObject isKindOfClass:[NSNumber class]] && [pMenuItem isKindOfClass:[VCLMenuItem class]] )
							{
								// Set help menu
								if ( (JavaSalMenuItemType)[(NSNumber *)pMenuItem.representedObject intValue] == JavaSalMenuItemType::HELP )
								{
									pApp.helpMenu = pSubmenu;
								}
#ifdef USE_NATIVE_WINDOWS_MENU
								// Set windows menu
								else if ( (JavaSalMenuItemType)[(NSNumber *)pMenuItem.representedObject intValue] == JavaSalMenuItemType::WINDOWS && pApp.windowsMenu && [pSubmenu isKindOfClass:[VCLMenu class]] )
								{
									pMenuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
									if ( pMenuItem )
									{
										[pMenuItem autorelease];

										// Remove windows menu from parent item
										NSMenu *pSupermenu = [pApp.windowsMenu supermenu];
										if ( pSupermenu )
										{
											for ( NSMenuItem *pSupermenuItem in [pSupermenu itemArray] )
											{
												if ( pSupermenuItem && [pSupermenuItem submenu] == pApp.windowsMenu )
												{
													[pSupermenuItem setSubmenu:nil];
													break;
												}
											}
										}

										// Cache the copied submenu to add later
										pMenuItem.representedObject = pSubmenu;
										[pMenuItem setSubmenu:pApp.windowsMenu];
										[pApp.windowsMenu setTitle:[pSubmenu title]];
									}
								}
#endif	// USE_NATIVE_WINDOWS_MENU
							}

							[pMainMenu addItem:pMenuItem];
						}
					}
				}
			}
		}
	}

	// Fix empty menu bug that occurs when opening a document using the
	// File :: Recent Documents menu by clearing the pending main menu
	if ( pPendingSetMenuAsMainMenu )
	{
		[pPendingSetMenuAsMainMenu release];
		pPendingSetMenuAsMainMenu = nil;
		bRemovePendingSetMenuAsMainMenu = NO;
	}
}

- (void)setMenuItemKeyEquivalent:(VCLMenuWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 3 )
		return;

    NSNumber *pPos = (NSNumber *)[pArgArray objectAtIndex:0];
    if ( !pPos )
        return;

	NSString *pKeyText = (NSString *)[pArgArray objectAtIndex:1];
	if ( !pKeyText || [pKeyText length] > 1 )
		return;

    NSNumber *pTag = (NSNumber *)[pArgArray objectAtIndex:2];
    if ( !pTag )
        return;

    unsigned int nPos = [pPos unsignedIntValue];
	if ( mpMenuItems && nPos < [mpMenuItems count] )
	{
		NSMenuItem *pMenuItem = [mpMenuItems objectAtIndex:nPos];
		if ( pMenuItem )
		{
			// This selector should not have been called if there is no key
			// code or command key modifier
			NSInteger nTag = [pTag unsignedShortValue] & ( KEY_CODE_MASK | KEY_MOD1 | KEY_MOD2 | KEY_MOD3 | KEY_SHIFT );
			if ( nTag & KEY_CODE_MASK && nTag & KEY_MOD1 )
			{
				NSUInteger nMask = NSEventModifierFlagCommand;
    			if ( nTag & KEY_MOD2 )
					nMask |= NSEventModifierFlagOption;
    			if ( nTag & KEY_MOD3 )
					nMask |= NSEventModifierFlagControl;
    			if ( nTag & KEY_SHIFT )
					nMask |= NSEventModifierFlagShift;

				[pMenuItem setKeyEquivalent:pKeyText];
				[pMenuItem setKeyEquivalentModifierMask:nMask];
				[pMenuItem setTag:nTag];
			}
			else
			{
				[pMenuItem setKeyEquivalent:@""];
				[pMenuItem setKeyEquivalentModifierMask:0];
				[pMenuItem setTag:0];
			}
		}
	}
}

- (void)setMenuItemSubmenu:(VCLMenuWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

	VCLMenuWrapper *pMenu = (VCLMenuWrapper *)[pArgArray objectAtIndex:0];
	if ( !pMenu )
		return;

    NSNumber *pPos = (NSNumber *)[pArgArray objectAtIndex:1];
    if ( !pPos )
        return;

    unsigned int nPos = [pPos unsignedIntValue];
	if ( mpMenuItems && nPos < [mpMenuItems count] )
	{
		NSMenuItem *pMenuItem = [mpMenuItems objectAtIndex:nPos];
		if ( pMenuItem )
		{
			NSMenu *pSubmenu = [pMenu menu];
			if ( pSubmenu )
			{
				[pMenuItem setSubmenu:pSubmenu];

				// Copy title to submenu
				[pSubmenu setTitle:[pMenuItem title]];
			}
		}
	}
}

- (void)setMenuItemTitle:(VCLMenuWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

	NSString *pTitle = (NSString *)[pArgArray objectAtIndex:0];
	if ( !pTitle )
		return;

    NSMenuItem *pMenuItem = (NSMenuItem *)[pArgArray objectAtIndex:1];
    if ( !pMenuItem )
        return;

	[pMenuItem setTitle:pTitle];

	// Copy title to submenu
	NSMenu *pSubmenu = [pMenuItem submenu];
	if ( pSubmenu )
		[pSubmenu setTitle:pTitle];
}

@end

@implementation VCLMenuItem

- (sal_uInt16)ID
{
	return mnID;
}

- (id)copyWithZone:(NSZone *)pZone
{
	VCLMenuItem *pRet = [[VCLMenuItem alloc] initWithTitle:[self title] type:meType id:mnID menu:mpMenu];
	if ( pRet )
	{
		NSMenu *pSubmenu = [self submenu];
		if ( pSubmenu )
		{
			pSubmenu = [pSubmenu copyWithZone:pZone];
			if ( pSubmenu )
			{
				[pSubmenu autorelease];
				[pRet setSubmenu:pSubmenu];
			}
		}

		[pRet setKeyEquivalent:[self keyEquivalent]];
		[pRet setKeyEquivalentModifierMask:[self keyEquivalentModifierMask]];
		[pRet setState:[self state]];
		[pRet setTag:[self tag]];
		[pRet setReallyEnabled:mbReallyEnabled];
	}

	return pRet;
}

- (id)initWithTitle:(NSString *)pTitle type:(MenuItemType)eType id:(sal_uInt16)nID menu:(Menu *)pMenu
{
	(void)eType;

	[super initWithTitle:( pTitle ? pTitle : @"" ) action:nil keyEquivalent:@""];

	mnID = nID;
	mpMenu = pMenu;
	mbReallyEnabled = [self isEnabled];
	meType = eType;

	[self setTarget:self];
	[self setAction:@selector(selected)];

	return self;
}

- (BOOL)isReallyEnabled
{
	return mbReallyEnabled;
}

- (void)selected
{
	BOOL bOldInPerformKeyEquivalent = NO;
	VCLApplicationDelegate *pAppDelegate = [VCLApplicationDelegate sharedDelegate];
	if ( pAppDelegate )
	{
		bOldInPerformKeyEquivalent = [pAppDelegate isInPerformKeyEquivalent];
		[pAppDelegate setInPerformKeyEquivalent:YES];
	}

	// If no application mutex exists yet, ignore event as we are likely to
	// crash
	if ( ImplApplicationIsRunning() )
	{
		// Prevent flooding of the OOo event queue when holding down a native
		// menu shortcut by by locking the application mutex
			ACQUIRE_SOLARMUTEX
			JavaSalEvent *pActivateEvent = new JavaSalEvent( SALEVENT_MENUACTIVATE, pMenuBarFrame, new SalMenuEvent( mnID, mpMenu ) );
			JavaSalEventQueue::postCachedEvent( pActivateEvent );
			pActivateEvent->release();

			JavaSalEvent *pCommandEvent = new JavaSalEvent( SALEVENT_MENUCOMMAND, pMenuBarFrame, new SalMenuEvent( mnID, mpMenu ) );
			JavaSalEventQueue::postCachedEvent( pCommandEvent );
			pCommandEvent->release();

			JavaSalEvent *pDeactivateEvent = new JavaSalEvent( SALEVENT_MENUDEACTIVATE, pMenuBarFrame, new SalMenuEvent( mnID, mpMenu ) );
			JavaSalEventQueue::postCachedEvent( pDeactivateEvent );
			pDeactivateEvent->release();
			RELEASE_SOLARMUTEX
	}

	nLastMenuItemSelectedTime = [NSDate timeIntervalSinceReferenceDate] + MAIN_MENU_CHANGE_WAIT_INTERVAL;

	if ( pAppDelegate )
		[pAppDelegate setInPerformKeyEquivalent:bOldInPerformKeyEquivalent];
}

- (void)setMenuType:(NSNumber *)pMenuType
{
	self.representedObject = pMenuType;
}

- (void)setReallyEnabled:(BOOL)bEnabled
{
	mbReallyEnabled = bEnabled;
	[self setEnabled:mbReallyEnabled];
}

- (BOOL)validateMenuItem:(NSMenuItem *)pMenuItem
{
	BOOL bRet = YES;

	VCLApplicationDelegate *pAppDelegate = [VCLApplicationDelegate sharedDelegate];
	if ( pAppDelegate )
		bRet = [pAppDelegate validateMenuItem:pMenuItem];

	if ( bRet && pMenuItem )
	{
		// Fix bug described at the end of the following NeoOffice forum post
		// by ignoring any menu item disabling calls not made by our own code:
		// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=64112#64112
		if ( [pMenuItem isKindOfClass:[VCLMenuItem class]] )
			bRet = [(VCLMenuItem *)pMenuItem isReallyEnabled];
		else
			bRet = [pMenuItem isEnabled];
	}

	return bRet;
}

@end

@interface VCLCreateMenu : NSObject
{
	VCLMenuWrapper*			mpMenu;
	BOOL					mbMenuBar;
}
+ (id)create:(BOOL)bMenuBar;
- (id)init:(BOOL)bMenuBar;
- (void)dealloc;
- (void)createMenu:(id)pObject;
- (VCLMenuWrapper *)menu;
@end

@implementation VCLCreateMenu

+ (id)create:(BOOL)bMenuBar
{
	VCLCreateMenu *pRet = [[VCLCreateMenu alloc] init:bMenuBar];
	[pRet autorelease];
	return pRet;
}

- (id)init:(BOOL)bMenuBar
{
	[super init];

	mpMenu = nil;
	mbMenuBar = bMenuBar;

	return self;
}

- (void)dealloc
{
	if ( mpMenu )
		[mpMenu release];

	[super dealloc];
}

- (void)createMenu:(id)pObject
{
	(void)pObject;

	if ( mpMenu )
		return;

	mpMenu = [[VCLMenuWrapper alloc] init:mbMenuBar];
}

- (VCLMenuWrapper *)menu
{
	return mpMenu;
}

@end

@interface VCLCreateMenuItem : NSObject
{
	sal_uInt16				mnID;
	Menu*					mpMenu;
	NSMenuItem*				mpMenuItem;
	NSString*				mpTitle;
	MenuItemType			meType;
}
+ (id)createWithTitle:(NSString *)pTitle type:(MenuItemType)eType id:(sal_uInt16)nID menu:(Menu *)pMenu;
- (id)initWithTitle:(NSString *)pTitle type:(MenuItemType)eType id:(sal_uInt16)nID menu:(Menu *)pMenu;
- (void)dealloc;
- (void)createMenuItem:(id)pObject;
- (NSMenuItem *)menuItem;
@end

@implementation VCLCreateMenuItem

+ (id)createWithTitle:(NSString *)pTitle type:(MenuItemType)eType id:(sal_uInt16)nID menu:(Menu *)pMenu
{
	VCLCreateMenuItem *pRet = [[VCLCreateMenuItem alloc] initWithTitle:pTitle type:eType id:nID menu:pMenu];
	[pRet autorelease];
	return pRet;
}

- (id)initWithTitle:(NSString *)pTitle type:(MenuItemType)eType id:(sal_uInt16)nID menu:(Menu *)pMenu
{
	[super init];

	mnID = nID;
	mpMenu = pMenu;
	mpMenuItem = nil;
	mpTitle = pTitle;
	if ( mpTitle )
		[mpTitle retain];
	meType = eType;

	return self;
}

- (void)dealloc
{
	if ( mpMenuItem )
		[mpMenuItem release];

	if ( mpTitle )
		[mpTitle release];

	[super dealloc];
}

- (void)createMenuItem:(id)pObject
{
	(void)pObject;

	if ( mpMenuItem )
		return;

	if ( meType == MenuItemType::SEPARATOR )
	{
		mpMenuItem = [NSMenuItem separatorItem];
		if ( mpMenuItem )
			[mpMenuItem retain];
	}
	else
	{
		mpMenuItem = [[VCLMenuItem alloc] initWithTitle:mpTitle type:meType id:mnID menu:mpMenu];
	}
}

- (NSMenuItem *)menuItem
{
	return mpMenuItem;
}

@end

@interface VCLDestroyMenuItem : NSObject
{
	NSMenuItem*				mpMenuItem;
}
+ (id)createWithMenuItem:(NSMenuItem *)pMenuItem;
- (id)initWithMenuItem:(NSMenuItem *)pMenuItem;
- (void)destroy:(id)pObject;
@end

@implementation VCLDestroyMenuItem

+ (id)createWithMenuItem:(NSMenuItem *)pMenuItem
{
	VCLDestroyMenuItem *pRet = [[VCLDestroyMenuItem alloc] initWithMenuItem:pMenuItem];
	[pRet autorelease];
	return pRet;
}

- (id)initWithMenuItem:(NSMenuItem *)pMenuItem
{
	[super init];

	// Do not retain as it will block deallocation on the main thread
	mpMenuItem = pMenuItem;

	return self;
}

- (void)destroy:(id)pObject
{
	// Attempt to fix Mac App Store crash when opening a menu by delaying
	// release of the native menu item
	VCLApplicationDelegate *pAppDelegate = [VCLApplicationDelegate sharedDelegate];
	if ( pObject || ( pAppDelegate && ( [pAppDelegate isInPerformKeyEquivalent] || [pAppDelegate isInTracking] ) ) )
	{
		[self performSelector:@selector(destroy:) withObject:nil afterDelay:MAIN_MENU_CHANGE_WAIT_INTERVAL];
		return;
	}

	if ( mpMenuItem )
	{
		NSMenu *pMenu = [mpMenuItem menu];
		if ( pMenu )
			[pMenu removeItem:mpMenuItem];
		[mpMenuItem setSubmenu:nil];
		[mpMenuItem release];
		mpMenuItem = nil;
	}
}

@end

//=============================================================================

JavaSalMenu::JavaSalMenu() :
	mpMenu( nil ),
	mpParentFrame( NULL ),
	mbIsMenuBarMenu( false ),
	mpParentVCLMenu( NULL )
{
	aMenuMap[ this ] = this;
}

//-----------------------------------------------------------------------------

JavaSalMenu::~JavaSalMenu()
{
	aMenuMap.erase( this );

	// Fix disabled menu bug when editing embedded OLE object reported in the
	// following forum topic by only setting the frame's menubar to NULL if
	// this menu is the menubar:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8462
	if ( mpParentFrame && mpParentFrame->mpMenuBar == this )
		mpParentFrame->SetMenu( NULL );

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMenu )
	{
		osl_performSelectorOnMainThread( mpMenu, @selector(destroy:), mpMenu, YES );
		[mpMenu release];
	}

	[pPool release];
}

//-----------------------------------------------------------------------------

void JavaSalMenu::SetMenuBarToFocusFrame()
{
	// Find first non-floating, non-utility window in hierarchy. Fix bug
	// reported in the following NeoOffice forum post by only doing this if the
	// frame is a floating or utility window:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=62883#62883
	JavaSalFrame *pFrame = GetSalData()->mpFocusFrame;
	while ( pFrame && ( pFrame->IsFloatingFrame() || pFrame->IsUtilityWindow() ) )
		pFrame = pFrame->mpParent;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	// Fix bug reported in the following NeoOffice forum post that causes the
	// empty menu to be set when editing an embedded OLE object by only setting
	// the empty menu if the frame has no menu and has a visible parent:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=63032#63032
	// Allow empty menubar for help window by allowing non-floating, non-utility
	// frames that have no parent to also have an empty menubar.
	if ( pFrame && pFrame->mbVisible )
	{
		if ( pFrame->mpMenuBar && pFrame->mpMenuBar->mbIsMenuBarMenu && pFrame->mpMenuBar->mpMenu )
		{
			osl_performSelectorOnMainThread( pFrame->mpMenuBar->mpMenu, @selector(setMenuAsMainMenu:), pFrame->mpMenuBar->mpMenu, NO );
		}
		else if ( ( pFrame->mpParent && pFrame->mpParent->mbVisible ) || ( !pFrame->mpParent && !pFrame->IsFloatingFrame() && !pFrame->IsUtilityWindow() ) )
		{
			static JavaSalMenu *pEmptyMenuBar = NULL;
			if ( !pEmptyMenuBar )
			{
				JavaSalInstance *pInst = GetSalData()->mpFirstInstance;
				if ( pInst )
					pEmptyMenuBar = (JavaSalMenu *)pInst->CreateMenu( sal_True, NULL );
			}

			if ( pEmptyMenuBar && pEmptyMenuBar->mbIsMenuBarMenu && pEmptyMenuBar->mpMenu )
				osl_performSelectorOnMainThread( pEmptyMenuBar->mpMenu, @selector(setMenuAsMainMenu:), pEmptyMenuBar->mpMenu, NO );
		}
	}

	[pPool release];
}

//-----------------------------------------------------------------------------

bool JavaSalMenu::VisibleMenuBar()
{
	return true;
}

//-----------------------------------------------------------------------------

void JavaSalMenu::SetFrame( const SalFrame *pFrame )
{
	if ( mbIsMenuBarMenu && mpMenu )
	{
		mpParentFrame = (JavaSalFrame *)pFrame;

		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLMenuWrapperArgs *pSetFrameArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithUnsignedLong:(unsigned long)mpParentFrame]]];
		osl_performSelectorOnMainThread( mpMenu, @selector(setFrame:), pSetFrameArgs, NO );

		if ( mpParentFrame && mpParentFrame == GetSalData()->mpFocusFrame )
			SetMenuBarToFocusFrame();

		[pPool release];
	}
}

//-----------------------------------------------------------------------------

void JavaSalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
	JavaSalMenuItem *pJavaSalMenuItem = (JavaSalMenuItem *)pSalMenuItem;
	if ( mpMenu && pJavaSalMenuItem && pJavaSalMenuItem->mpMenuItem )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLMenuWrapperArgs *pInsertMenuItemArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObjects:pJavaSalMenuItem->mpMenuItem, [NSNumber numberWithUnsignedInt:nPos], nil]];
		osl_performSelectorOnMainThread( mpMenu, @selector(insertMenuItem:), pInsertMenuItemArgs, NO );

		[pPool release];
	}
}

//-----------------------------------------------------------------------------

void JavaSalMenu::RemoveItem( unsigned nPos )
{
	if ( mpMenu )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLMenuWrapperArgs *pRemoveMenuItemArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithUnsignedInt:nPos]]];
		osl_performSelectorOnMainThread( mpMenu, @selector(removeMenuItem:), pRemoveMenuItemArgs, NO );

		[pPool release];
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
	if ( mpMenu && pJavaSubMenu && pJavaSubMenu && pJavaSubMenu->mpMenu )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLMenuWrapperArgs *pSetMenuItemSubmenuArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObjects:pJavaSubMenu->mpMenu, [NSNumber numberWithUnsignedInt:nPos], nil]];
		osl_performSelectorOnMainThread( mpMenu, @selector(setMenuItemSubmenu:), pSetMenuItemSubmenuArgs, NO );

		[pPool release];
	}
	pJavaSalMenuItem->mpSalSubmenu = pJavaSubMenu;
}

//-----------------------------------------------------------------------------

void JavaSalMenu::CheckItem( unsigned nPos, bool bCheck )
{
	if ( mpMenu )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLMenuWrapperArgs *pCheckMenuItemArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithUnsignedInt:nPos], [NSNumber numberWithBool:bCheck], nil]];
		osl_performSelectorOnMainThread( mpMenu, @selector(checkMenuItem:), pCheckMenuItemArgs, NO );

		[pPool release];
	}
}

//-----------------------------------------------------------------------------

void JavaSalMenu::EnableItem( unsigned nPos, bool bEnable )
{
	if ( mpMenu )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLMenuWrapperArgs *pEnableMenuItemArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithUnsignedInt:nPos], [NSNumber numberWithBool:bEnable], nil]];
		osl_performSelectorOnMainThread( mpMenu, @selector(enableMenuItem:), pEnableMenuItemArgs, NO );

		[pPool release];
	}
}

//-----------------------------------------------------------------------------

void JavaSalMenu::SetItemImage( unsigned /* nPos */, SalMenuItem* /* pSalMenuItem */, const Image& /* rImage */ )
{
	// for now we'll ignore putting icons in AWT menus.  Most Mac apps don't
	// have them, so they're kind of extraneous on the platform anyhow.
}

//-----------------------------------------------------------------------------

void JavaSalMenu::SetItemText( unsigned /* nPos */, SalMenuItem* pSalMenuItem, const OUString& rText )
{
	// assume pSalMenuItem is a pointer to the menu item object already at nPos
	JavaSalMenuItem *pJavaSalMenuItem = (JavaSalMenuItem *)pSalMenuItem;

	// remove accelerator character
	OUString aText( rText );
	aText = aText.replaceAll("~", "");

	if ( mpMenu && pJavaSalMenuItem && pJavaSalMenuItem->mpMenuItem )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSString *pTitle = [NSString stringWithCharacters:aText.getStr() length:aText.getLength()];
		VCLMenuWrapperArgs *pSetMenuItemTitleArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObjects:( pTitle ? pTitle : @"" ), pJavaSalMenuItem->mpMenuItem, nil]];
		osl_performSelectorOnMainThread( mpMenu, @selector(setMenuItemTitle:), pSetMenuItemTitleArgs, NO );

		[pPool release];
	}
}

//-----------------------------------------------------------------------------

void JavaSalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const vcl::KeyCode& rKeyCode, const OUString& /* rKeyName */ )
{
	// Only pass through keycodes that are using the command key as of the
	// Alt and Control modifiers are likely to conflict with standard Mac key
	// input manager and document editing actions. Also, exclude standard
	// shortcuts in the application menu.
	OUString aKeyEquivalent;
	if ( mpMenu && rKeyCode.IsMod1() && ! ( rKeyCode.GetCode() == KEY_H && !rKeyCode.IsMod3() && !rKeyCode.IsShift() ) && ! ( rKeyCode.GetCode() == KEY_Q && !rKeyCode.IsMod2() && !rKeyCode.IsMod3() && !rKeyCode.IsShift() ) && ! ( rKeyCode.GetCode() == KEY_COMMA && !rKeyCode.IsMod2() && !rKeyCode.IsMod3() && !rKeyCode.IsShift() ) )
	{
		aKeyEquivalent = JavaSalFrame::ConvertVCLKeyCode( rKeyCode.GetCode(), true );
		if ( !rKeyCode.IsShift() )
			aKeyEquivalent = aKeyEquivalent.toAsciiLowerCase();
	}

	// assume pSalMenuItem is a pointer to the item to be associated with
	// the new shortcut
	JavaSalMenuItem *pJavaSalMenuItem = (JavaSalMenuItem *)pSalMenuItem;
	if ( pJavaSalMenuItem && pJavaSalMenuItem->mpMenuItem )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSString *pKeyEquivalent = [NSString stringWithCharacters:aKeyEquivalent.getStr() length:aKeyEquivalent.getLength()];
		VCLMenuWrapperArgs *pSetMenuItemKeyEquivalentArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithUnsignedInt:nPos], ( pKeyEquivalent ? pKeyEquivalent : @"" ), [NSNumber numberWithUnsignedShort:rKeyCode.GetFullCode()], nil]];
		osl_performSelectorOnMainThread( mpMenu, @selector(setMenuItemKeyEquivalent:), pSetMenuItemKeyEquivalentArgs, NO );

		[pPool release];
	}
}

//-----------------------------------------------------------------------------

void JavaSalMenu::GetSystemMenuData( SystemMenuData* /* pData */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalMenu::GetSystemMenuData not implemented\n" );
#endif
}

// =======================================================================

JavaSalMenuItem::JavaSalMenuItem() :
	mpMenuItem( nil ),
	meMenuType( JavaSalMenuItemType::NONE ),
	mpSalSubmenu( NULL )
{
}

//-----------------------------------------------------------------------------

JavaSalMenuItem::~JavaSalMenuItem()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMenuItem )
	{
		VCLDestroyMenuItem *pVCLDestroyMenuItem = [VCLDestroyMenuItem createWithMenuItem:mpMenuItem];
		osl_performSelectorOnMainThread( pVCLDestroyMenuItem, @selector(destroy:), pVCLDestroyMenuItem, YES );
	}

	[pPool release];
}

//-----------------------------------------------------------------------------

void JavaSalMenuItem::SetCommand( const OUString& rCommand )
{
	JavaSalMenuItemType eOldMenuType = meMenuType;

	if ( rCommand == ".uno:HelpMenu" )
		meMenuType = JavaSalMenuItemType::HELP;
	else if ( rCommand == ".uno:WindowList" )
		meMenuType = JavaSalMenuItemType::WINDOWS;
	else
		meMenuType = JavaSalMenuItemType::NONE;

	if ( mpMenuItem && eOldMenuType != meMenuType )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		osl_performSelectorOnMainThread( mpMenuItem, @selector(setMenuType:), [NSNumber numberWithInt:meMenuType], NO );

		[pPool release];
	}
}

// =======================================================================

SalMenu* JavaSalInstance::CreateMenu( bool bMenuBar, Menu *pVCLMenuWrapper )
{
#ifndef NO_NATIVE_MENUS
	JavaSalMenu *pSalMenu = new JavaSalMenu();
	pSalMenu->mbIsMenuBarMenu = bMenuBar;
	pSalMenu->mpParentVCLMenu = pVCLMenuWrapper;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	VCLCreateMenu *pVCLCreateMenu = [VCLCreateMenu create:bMenuBar];
	osl_performSelectorOnMainThread( pVCLCreateMenu, @selector(createMenu:), pVCLCreateMenu, YES );
	VCLMenuWrapper *pMenu = [pVCLCreateMenu menu];
	if ( pMenu )
	{
		[pMenu retain];
		pSalMenu->mpMenu = pMenu;
	}
	else
	{
		// If no native menu is created, revert to non-native menus or else
		// the OOo code will eventually crash
		delete pSalMenu;
		return NULL;
	}

	[pPool release];

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
	OUString aTitle(pItemData->aText.replaceAll("~", ""));

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSString *pTitle = [NSString stringWithCharacters:aTitle.getStr() length:aTitle.getLength()];
	VCLCreateMenuItem *pVCLCreateMenuItem = [VCLCreateMenuItem createWithTitle:pTitle type:pItemData->eType id:pItemData->nId menu:pItemData->pMenu];
	osl_performSelectorOnMainThread( pVCLCreateMenuItem, @selector(createMenuItem:), pVCLCreateMenuItem, YES );
	NSMenuItem *pMenuItem = [pVCLCreateMenuItem menuItem];
	if ( pMenuItem )
	{
		[pMenuItem retain];
		pSalMenuItem->mpMenuItem = pMenuItem;
	}

	[pPool release];

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

	Menu *pVCLMenuWrapper = pMenu->mpParentVCLMenu;
	if ( !pVCLMenuWrapper )
		return;

	// Force the clipboard service to update itself before we update the
	// menus as if the native clipboard was cleared when we last checked, we
	// won't be notified when another application puts content.
	if ( pMenu->mbIsMenuBarMenu && CFRunLoopGetCurrent() == CFRunLoopGetMain() )
	{
		// Reset the frame's menu update list
		pFrame->maUpdateMenuList.clear();

		Window *pWindow = pVCLMenuWrapper->GetWindow();
		if ( pWindow )
		{
			uno::Reference< datatransfer::clipboard::XClipboard > aClipboard = pWindow->GetClipboard();
			if ( aClipboard.is() )
				aClipboard->getContents();
		}
	}

	// Post the SALEVENT_MENUACTIVATE event
	JavaSalEvent *pActivateEvent = new JavaSalEvent( SALEVENT_MENUACTIVATE, pFrame, new SalMenuEvent( 0, pVCLMenuWrapper ) );
	pActivateEvent->dispatch();
	pActivateEvent->release();

	sal_uInt16 nCount = pVCLMenuWrapper->GetItemCount();
	for( sal_uInt16 i = 0; i < nCount; i++ )
	{
		// If this menu item has a submenu, fix that submenu up
		JavaSalMenuItem *pSalMenuItem = (JavaSalMenuItem *)pVCLMenuWrapper->GetItemSalItem( i );
		if ( pSalMenuItem && pSalMenuItem->mpSalSubmenu )
		{
			if ( bUpdateSubmenus )
				UpdateMenusForFrame( pFrame, pSalMenuItem->mpSalSubmenu, true );
			else
				pFrame->maUpdateMenuList.push_back( pSalMenuItem->mpSalSubmenu );
		}
	}

	// Post the SALEVENT_MENUDEACTIVATE event
	JavaSalEvent *pDeactivateEvent = new JavaSalEvent( SALEVENT_MENUDEACTIVATE, pFrame, new SalMenuEvent( 0, pVCLMenuWrapper ) );
	pDeactivateEvent->dispatch();
	pDeactivateEvent->release();
#endif	// !NO_NATIVE_MENUS
}

//-----------------------------------------------------------------------------

void VCLMenu_updateNativeWindowsMenu()
{
#ifdef USE_NATIVE_WINDOWS_MENU
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp && pApp.windowsMenu )
	{
		NSMenu *pMainMenu = [pApp mainMenu];
		if ( pMainMenu && pMainMenu == [pApp.windowsMenu supermenu] )
		{
			// Remove previously copied menu items
			for ( NSMenuItem *pWindowsItem in [pApp.windowsMenu itemArray] )
			{
				if ( pWindowsItem && pWindowsItem.representedObject && [pWindowsItem.representedObject isKindOfClass:[VCLMenu class]] )
					[pApp.windowsMenu removeItem:pWindowsItem];
			}

			VCLMenu *pSubmenu = nil;
			for ( NSMenuItem *pMainMenuItem in [pMainMenu itemArray] )
			{
				if ( pMainMenuItem && pMainMenuItem.representedObject && [pMainMenuItem.representedObject isKindOfClass:[VCLMenu class]] )
				{
					pSubmenu = (VCLMenu *)pMainMenuItem.representedObject;
					break;
				}
			}

			// Copy submenu items into windows menu
			if ( pSubmenu )
			{
				NSInteger nInsertIndex = 0;

				// Insert after "Bring All to Front" menu item
				for ( NSMenuItem *pWindowsMenuItem in [pApp.windowsMenu itemArray] )
				{
					if ( pWindowsMenuItem && [pWindowsMenuItem action] == @selector(arrangeInFront:) )
						break;

					nInsertIndex++;
				}

				for ( NSMenuItem *pSubmenuItem in [pSubmenu itemArray] )
				{
					if ( pSubmenuItem )
					{
						// Don't copy windows list items
						if ( [pSubmenuItem isKindOfClass:[VCLMenuItem class]] )
						{
							sal_uInt16 nID = [(VCLMenuItem *)pSubmenuItem ID];
							if ( nID >= START_ITEMID_WINDOWLIST && nID < END_ITEMID_WINDOWLIST )
								continue;
						}

						pSubmenuItem = [pSubmenuItem copy];
						if ( pSubmenuItem )
						{
							[pSubmenuItem autorelease];

							pSubmenuItem.representedObject = pSubmenu;
							[pApp.windowsMenu insertItem:pSubmenuItem atIndex:nInsertIndex++];
						}
					}
				}

				if ( nInsertIndex && nInsertIndex < [pApp.windowsMenu numberOfItems] )
				{
					NSMenuItem *pSeparatorItem = [NSMenuItem separatorItem];
					if ( pSeparatorItem )
					{
						pSeparatorItem.representedObject = pSubmenu;
						[pApp.windowsMenu insertItem:pSeparatorItem atIndex:nInsertIndex++];
					}
				}
			}
		}
	}
#endif	// USE_NATIVE_WINDOWS_MENU
}
