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

#include <saldata.hxx>
#include <salframe.h>
#include <salinst.h>
#include <salmenu.h>
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

#ifdef USE_NATIVE_WINDOW
#include "../java/VCLApplicationDelegate_cocoa.h"
#endif	// USE_NATIVE_WINDOW

static ::std::map< JavaSalMenu*, JavaSalMenu* > aMenuMap;

using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::uno;
using namespace vcl;

#ifdef USE_NATIVE_WINDOW

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
- (id)initWithTitle:(NSString *)pTitle;
@end

@implementation VCLMenu

- (id)initWithTitle:(NSString *)pTitle
{
	[super initWithTitle:pTitle];

	[self setDelegate:[VCLApplicationDelegate sharedDelegate]];

	return self;
}

@end

@interface VCLMenuWrapper : NSObject
{
	JavaSalFrame*			mpFrame;
	VCLMenu*				mpMenu;
	MacOSBOOL				mbMenuBar;
	NSMutableArray*			mpMenuItems;
}
- (id)init:(MacOSBOOL)bMenuBar;
- (void)checkMenuItem:(VCLMenuWrapperArgs *)pArgs;
- (void)dealloc;
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

static JavaSalFrame *pMenuBarFrame = NULL;
static VCLMenuWrapper *pMenuBarMenu = nil;

@implementation VCLMenuWrapper

- (id)init:(MacOSBOOL)bMenuBar
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
	if ( mpMenuItems && nPos >= 0 && nPos < [mpMenuItems count] )
	{
		NSMenuItem *pMenuItem = [mpMenuItems objectAtIndex:nPos];
		if ( pMenuItem )
			[pMenuItem setState:( [pCheck boolValue] ? NSOnState : NSOffState )];
	}
}

- (void)dealloc
{
	if ( self == pMenuBarMenu )
		[self removeMenuAsMainMenu:self];

	if ( mpMenu )
		[mpMenu release];

	if ( mpMenuItems )
		[mpMenuItems release];

	[super dealloc];
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
	if ( mpMenuItems && nPos >= 0 && nPos < [mpMenuItems count] )
	{
		NSMenuItem *pMenuItem = [mpMenuItems objectAtIndex:nPos];
		if ( pMenuItem )
			[pMenuItem setEnabled:[pEnable boolValue]];
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
		if ( nPos >= 0 && nPos < [mpMenuItems count] )
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

		if ( self == pMenuBarMenu )
		{
			[self removeMenuAsMainMenu:self];
			[self setMenuAsMainMenu:self];
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
			unsigned int nCount = [mpMenuItems count];
			unsigned int i = 0;
			for ( ; i < nCount; i++ )
			{
				NSMenuItem *pMenuItem = [mpMenuItems objectAtIndex:i];
				if ( pMenuItem )
					[mpMenu addItem:pMenuItem];
			}
		}
	}

	return mpMenu;
}

- (void)removeMenuAsMainMenu:(id)pObject
{
	if ( !mbMenuBar || self != pMenuBarMenu )
		return;

	pMenuBarFrame = NULL;
	pMenuBarMenu = nil;

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSMenu *pMainMenu = [pApp mainMenu];
		if ( pMainMenu )
		{
			NSUInteger nCount = [pMainMenu numberOfItems];
			if ( nCount > 0 )
			{
				NSMenuItem *pAppMenuItem = [pMainMenu itemAtIndex:0];
				if ( pAppMenuItem )
				{
					NSMenu *pNewMainMenu = [[VCLMenu alloc] initWithTitle:[pMainMenu title]];
					if ( pNewMainMenu )
					{
						[pAppMenuItem retain];
						[pMainMenu removeItemAtIndex:0];
						[pNewMainMenu addItem:pAppMenuItem];
						[pAppMenuItem release];

						// Remove remaining menus while still showing
						while ( [pMainMenu numberOfItems] > 0 )
							[pMainMenu removeItemAtIndex:0];

						[pApp setMainMenu:pNewMainMenu];
					}
				}
			}
		}
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
	if ( mpMenuItems && nPos >= 0 && nPos < [mpMenuItems count] )
	{
		NSMenuItem *pMenuItem = [mpMenuItems objectAtIndex:nPos];
		if ( pMenuItem )
		{
			[mpMenuItems removeObject:pMenuItem];

			if ( mpMenu )
				[mpMenu removeItem:pMenuItem];

			if ( self == pMenuBarMenu )
			{
				[self removeMenuAsMainMenu:self];
				[self setMenuAsMainMenu:self];
			}
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
	if ( !mbMenuBar || self == pMenuBarMenu )
		return;

	pMenuBarFrame = mpFrame;
	pMenuBarMenu = self;

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSMenu *pMainMenu = [pApp mainMenu];
		if ( pMainMenu )
		{
			NSUInteger nCount = [pMainMenu numberOfItems];
			if ( nCount > 0 )
			{
				NSMenuItem *pAppMenuItem = [pMainMenu itemAtIndex:0];
				if ( pAppMenuItem )
				{
					NSMenu *pNewMainMenu = [[VCLMenu alloc] initWithTitle:[pMainMenu title]];
					if ( pNewMainMenu )
					{
						[pAppMenuItem retain];
						[pMainMenu removeItemAtIndex:0];
						[pNewMainMenu addItem:pAppMenuItem];
						[pAppMenuItem release];

						// Remove remaining menus while still showing
						while ( [pMainMenu numberOfItems] > 0 )
							[pMainMenu removeItemAtIndex:0];

						[pApp setMainMenu:pNewMainMenu];

						// Add our menu items to menubar. Note that we must add
						// the menu items after the menu has been set as the
						// main menu for [VCLApplicationDelegate addMenuItem:]
						// to get called.
						if ( mpMenuItems )
						{
							nCount = [mpMenuItems count];
							NSUInteger i = 0;
							for ( ; i < nCount; i++ )
							{
								NSMenuItem *pMenuItem = [mpMenuItems objectAtIndex:i];
								if ( pMenuItem )
								{
									NSMenu *pMenu = [pMenuItem menu];
									if ( pMenu )
										[pMenu removeItem:pMenuItem];

									NSMenu *pSubmenu = [pMenuItem submenu];
									if ( pSubmenu )
										[pNewMainMenu addItem:pMenuItem];
								}
							}
						}
					}
				}
			}
		}
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
	if ( !pKeyText || [pKeyText length] != 1 )
		return;

    NSNumber *pShift = (NSNumber *)[pArgArray objectAtIndex:2];
    if ( !pShift )
        return;

    unsigned int nPos = [pPos unsignedIntValue];
	if ( mpMenuItems && nPos >= 0 && nPos < [mpMenuItems count] )
	{
		NSMenuItem *pMenuItem = [mpMenuItems objectAtIndex:nPos];
		if ( pMenuItem )
		{
			NSUInteger nMask = NSCommandKeyMask;
    		if ( [pShift boolValue] )
				nMask |= NSShiftKeyMask;
			[pMenuItem setKeyEquivalent:pKeyText];
			[pMenuItem setKeyEquivalentModifierMask:nMask];
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
	if ( mpMenuItems && nPos >= 0 && nPos < [mpMenuItems count] )
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

@interface VCLMenuItem : NSMenuItem
{
	USHORT					mnID;
	Menu*					mpMenu;
}
- (id)initWithTitle:(NSString *)pTitle type:(MenuItemType)eType id:(USHORT)nID menu:(Menu *)pMenu;
- (void)selected;
- (MacOSBOOL)validateMenuItem:(NSMenuItem *)pMenuItem;
@end

@implementation VCLMenuItem

- (id)initWithTitle:(NSString *)pTitle type:(MenuItemType)eType id:(USHORT)nID menu:(Menu *)pMenu
{
	[super initWithTitle:( pTitle ? pTitle : @"" ) action:nil keyEquivalent:@""];

	mnID = nID;
	mpMenu = pMenu;

	[self setTarget:self];
	[self setAction:@selector(selected)];

	return self;
}

- (void)selected
{
	JavaSalEventQueue::postMenuItemSelectedEvent( pMenuBarFrame, mnID, mpMenu );
}

- (MacOSBOOL)validateMenuItem:(NSMenuItem *)pMenuItem
{
	MacOSBOOL bRet = YES;

	VCLApplicationDelegate *pAppDelegate = [VCLApplicationDelegate sharedDelegate];
	if ( pAppDelegate )
		bRet = [pAppDelegate validateMenuItem:pMenuItem];

	if ( bRet && pMenuItem )
		bRet = [pMenuItem isEnabled];

	return bRet;
}

@end

@interface VCLCreateMenu : NSObject
{
	VCLMenuWrapper*			mpMenu;
	MacOSBOOL				mbMenuBar;
}
+ (id)create:(MacOSBOOL)bMenuBar;
- (id)init:(MacOSBOOL)bMenuBar;
- (void)dealloc;
- (void)createMenu:(id)pObject;
- (VCLMenuWrapper *)menu;
@end

@implementation VCLCreateMenu

+ (id)create:(MacOSBOOL)bMenuBar
{
	VCLCreateMenu *pRet = [[VCLCreateMenu alloc] init:bMenuBar];
	[pRet autorelease];
	return pRet;
}

- (id)init:(MacOSBOOL)bMenuBar
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
	USHORT					mnID;
	Menu*					mpMenu;
	NSMenuItem*				mpMenuItem;
	NSString*				mpTitle;
	MenuItemType			meType;
}
+ (id)createWithTitle:(NSString *)pTitle type:(MenuItemType)eType id:(USHORT)nID menu:(Menu *)pMenu;
- (id)initWithTitle:(NSString *)pTitle type:(MenuItemType)eType id:(USHORT)nID menu:(Menu *)pMenu;
- (void)dealloc;
- (void)createMenuItem:(id)pObject;
- (NSMenuItem *)menuItem;
@end

@implementation VCLCreateMenuItem

+ (id)createWithTitle:(NSString *)pTitle type:(MenuItemType)eType id:(USHORT)nID menu:(Menu *)pMenu
{
	VCLCreateMenuItem *pRet = [[VCLCreateMenuItem alloc] initWithTitle:pTitle type:eType id:nID menu:pMenu];
	[pRet autorelease];
	return pRet;
}

- (id)initWithTitle:(NSString *)pTitle type:(MenuItemType)eType id:(USHORT)nID menu:(Menu *)pMenu
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
	if ( mpMenuItem )
		return;

	if ( meType == MENUITEM_SEPARATOR )
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

#endif	// USE_NATIVE_WINDOW

//=============================================================================

JavaSalMenu::JavaSalMenu() :
#ifdef USE_NATIVE_WINDOW
	mpMenu( NULL ),
#else	// USE_NATIVE_WINDOW
	mpVCLMenuBar( NULL ),
	mpVCLMenu( NULL ),
#endif	// USE_NATIVE_WINDOW
	mpParentFrame( NULL ),
	mbIsMenuBarMenu( FALSE ),
	mpParentVCLMenu( NULL )
{
	aMenuMap[ this ] = this;
}

//-----------------------------------------------------------------------------

JavaSalMenu::~JavaSalMenu()
{
	aMenuMap.erase( this );

	if ( mpParentFrame )
		mpParentFrame->SetMenu( NULL );

#ifdef USE_NATIVE_WINDOW
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMenu )
		[mpMenu release];

	[pPool release];
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

#ifdef USE_NATIVE_WINDOW

//-----------------------------------------------------------------------------

void JavaSalMenu::SetMenuBarToFocusFrame()
{
	// Find first frame in hierarchy that has a menubar
	JavaSalFrame *pFrame = GetSalData()->mpFocusFrame;
	while ( pFrame && pFrame->mbVisible && ( !pFrame->mpMenuBar || !pFrame->mpMenuBar->mbIsMenuBarMenu || !pFrame->mpMenuBar->mpMenu || pFrame->IsFloatingFrame() || pFrame->IsUtilityWindow() ) )
		pFrame = pFrame->mpParent;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];

	if ( pFrame && pFrame->mbVisible && pFrame->mpMenuBar && pFrame->mpMenuBar->mbIsMenuBarMenu && pFrame->mpMenuBar->mpMenu && !pFrame->IsFloatingFrame() && !pFrame->IsUtilityWindow() )
	{
		[pFrame->mpMenuBar->mpMenu performSelectorOnMainThread:@selector(setMenuAsMainMenu:) withObject:pFrame->mpMenuBar->mpMenu waitUntilDone:NO modes:pModes];
	}
	else
	{
		static JavaSalMenu *pEmptyMenuBar = NULL;
		if ( !pEmptyMenuBar )
		{
			JavaSalInstance *pInst = GetSalData()->mpFirstInstance;
			if ( pInst )
				pEmptyMenuBar = (JavaSalMenu *)pInst->CreateMenu( TRUE, NULL );
		}

		if ( pEmptyMenuBar && pEmptyMenuBar->mbIsMenuBarMenu && pEmptyMenuBar->mpMenu )
			[pEmptyMenuBar->mpMenu performSelectorOnMainThread:@selector(setMenuAsMainMenu:) withObject:pEmptyMenuBar->mpMenu waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

#endif	// USE_NATIVE_WINDOW

//-----------------------------------------------------------------------------

BOOL JavaSalMenu::VisibleMenuBar()
{
	return TRUE;
}

//-----------------------------------------------------------------------------

void JavaSalMenu::SetFrame( const SalFrame *pFrame )
{
#ifdef USE_NATIVE_WINDOW
	if ( mbIsMenuBarMenu && mpMenu )
	{
		mpParentFrame = (JavaSalFrame *)pFrame;

		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLMenuWrapperArgs *pSetFrameArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithUnsignedLong:(unsigned long)mpParentFrame]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpMenu performSelectorOnMainThread:@selector(setFrame:) withObject:pSetFrameArgs waitUntilDone:NO modes:pModes];

		if ( mpParentFrame && mpParentFrame == GetSalData()->mpFocusFrame )
			SetMenuBarToFocusFrame();

		[pPool release];
	}
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
	JavaSalMenuItem *pJavaSalMenuItem = (JavaSalMenuItem *)pSalMenuItem;
#ifdef USE_NATIVE_WINDOW
	if ( mpMenu && pJavaSalMenuItem && pJavaSalMenuItem->mpMenuItem )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLMenuWrapperArgs *pInsertMenuItemArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObjects:pJavaSalMenuItem->mpMenuItem, [NSNumber numberWithUnsignedInt:nPos], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpMenu performSelectorOnMainThread:@selector(insertMenuItem:) withObject:pInsertMenuItemArgs waitUntilDone:NO modes:pModes];

		[pPool release];
	}
#else	// USE_NATIVE_WINDOW
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
	if ( mpMenu )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLMenuWrapperArgs *pRemoveMenuItemArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithUnsignedInt:nPos]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpMenu performSelectorOnMainThread:@selector(removeMenuItem:) withObject:pRemoveMenuItemArgs waitUntilDone:NO modes:pModes];

		[pPool release];
	}
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
	JavaSalMenu* pJavaSubMenu = (JavaSalMenu *)pSubMenu;
#ifdef USE_NATIVE_WINDOW
	if ( mpMenu && pJavaSubMenu && pJavaSubMenu && pJavaSubMenu->mpMenu )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLMenuWrapperArgs *pSetMenuItemSubmenuArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObjects:pJavaSubMenu->mpMenu, [NSNumber numberWithUnsignedInt:nPos], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpMenu performSelectorOnMainThread:@selector(setMenuItemSubmenu:) withObject:pSetMenuItemSubmenuArgs waitUntilDone:NO modes:pModes];

		[pPool release];
	}
#else	// USE_NATIVE_WINDOW
	if( mbIsMenuBarMenu && mpVCLMenuBar && pJavaSubMenu && pJavaSubMenu->mpVCLMenu )
	{
		mpVCLMenuBar->changeMenu( pJavaSubMenu->mpVCLMenu->getMenuItemDataObject(), nPos );
	}
	else if( mpVCLMenu && pJavaSubMenu )
	{
		mpVCLMenu->attachSubmenu( pJavaSubMenu->mpVCLMenu->getMenuItemDataObject(), nPos );
	}
#endif	// USE_NATIVE_WINDOW
	pJavaSalMenuItem->mpSalSubmenu = pJavaSubMenu;
}

//-----------------------------------------------------------------------------

void JavaSalMenu::CheckItem( unsigned nPos, BOOL bCheck )
{
#ifdef USE_NATIVE_WINDOW
	if ( mpMenu )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLMenuWrapperArgs *pCheckMenuItemArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithUnsignedInt:nPos], [NSNumber numberWithBool:bCheck], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpMenu performSelectorOnMainThread:@selector(checkMenuItem:) withObject:pCheckMenuItemArgs waitUntilDone:NO modes:pModes];

		[pPool release];
	}
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
	if ( mpMenu )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLMenuWrapperArgs *pEnableMenuItemArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithUnsignedInt:nPos], [NSNumber numberWithBool:bEnable], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpMenu performSelectorOnMainThread:@selector(enableMenuItem:) withObject:pEnableMenuItemArgs waitUntilDone:NO modes:pModes];

		[pPool release];
	}
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
	// assume pSalMenuItem is a pointer to the menu item object already at nPos
	JavaSalMenuItem *pJavaSalMenuItem = (JavaSalMenuItem *)pSalMenuItem;

	// remove accelerator character
	XubString theText(rText);
	theText.EraseAllChars('~');
	OUString aText( theText );

#ifdef USE_NATIVE_WINDOW
	if ( mpMenu && pJavaSalMenuItem && pJavaSalMenuItem->mpMenuItem )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSString *pTitle = [NSString stringWithCharacters:aText.getStr() length:aText.getLength()];
		VCLMenuWrapperArgs *pSetMenuItemTitleArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObjects:( pTitle ? pTitle : @"" ), pJavaSalMenuItem->mpMenuItem, nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpMenu performSelectorOnMainThread:@selector(setMenuItemTitle:) withObject:pSetMenuItemTitleArgs waitUntilDone:NO modes:pModes];

		[pPool release];
	}
#else	// USE_NATIVE_WINDOW
	if( pJavaSalMenuItem && pJavaSalMenuItem->mpVCLMenuItemData )
	{
		pJavaSalMenuItem->mpVCLMenuItemData->setTitle( aText );
	}
#endif	// USE_NATIVE_WINDOW
}

//-----------------------------------------------------------------------------

void JavaSalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName )
{
	// Only pass through keycodes that are using the command key as Java will
	// always add a command key to any shortcut. Also, ignore any shortcuts
	// that use modifiers other than the shift key as Java only allows adding
	// of the shift key and Alt and Control modifiers are likely to conflict
	// with standard Mac key actions. Also, exclude standard shortcuts
	// in the application menu. Also, fix bug 2886 by not allowing any
	// shortcuts with a space as Java will disable a tab and the shortcut
	// be unusable.
	if ( rKeyCode.IsMod1() && !rKeyCode.IsMod2() && !rKeyCode.IsMod3() && ! ( rKeyCode.GetCode() == KEY_H && !rKeyCode.IsShift() ) && rKeyCode.GetCode() != KEY_Q && rKeyCode.GetCode() != KEY_COMMA && rKeyCode.GetCode() != KEY_SPACE )
	{
		// assume pSalMenuItem is a pointer to the item to be associated with
		// the new shortcut
		JavaSalMenuItem *pJavaSalMenuItem = (JavaSalMenuItem *)pSalMenuItem;
#ifdef USE_NATIVE_WINDOW
		if ( pJavaSalMenuItem && pJavaSalMenuItem->mpMenuItem )
		{
			OUString aKeyEquivalent = JavaSalFrame::ConvertVCLKeyCode( rKeyCode.GetCode() );
			if ( !rKeyCode.IsShift() )
				aKeyEquivalent = aKeyEquivalent.toAsciiLowerCase();

			if ( aKeyEquivalent.getLength() )
			{
				NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

				NSString *pKeyEquivalent = [NSString stringWithCharacters:aKeyEquivalent.getStr() length:aKeyEquivalent.getLength()];
				VCLMenuWrapperArgs *pSetMenuItemKeyEquivalentArgs = [VCLMenuWrapperArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithUnsignedInt:nPos], ( pKeyEquivalent ? pKeyEquivalent : @"" ), [NSNumber numberWithBool:rKeyCode.IsShift()], nil]];
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				[mpMenu performSelectorOnMainThread:@selector(setMenuItemKeyEquivalent:) withObject:pSetMenuItemKeyEquivalentArgs waitUntilDone:NO modes:pModes];

				[pPool release];
			}
		}
#else	// USE_NATIVE_WINDOW
		if( pJavaSalMenuItem && pJavaSalMenuItem->mpVCLMenuItemData )
			pJavaSalMenuItem->mpVCLMenuItemData->setKeyboardShortcut( rKeyCode.GetCode(), rKeyCode.IsShift() );
#endif	// USE_NATIVE_WINDOW
	}
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
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMenuItem )
		[mpMenuItem release];

	[pPool release];
#else	// USE_NATIVE_WINDOW
	if( mpVCLMenuItemData )
	{
		mpVCLMenuItemData->dispose();
		delete mpVCLMenuItemData;
	}
#endif	// USE_NATIVE_WINDOW
}

//-----------------------------------------------------------------------------

SalMenu* JavaSalInstance::CreateMenu( BOOL bMenuBar, Menu *pVCLMenuWrapper )
{
#ifndef NO_NATIVE_MENUS
	JavaSalMenu *pSalMenu = new JavaSalMenu();
	pSalMenu->mbIsMenuBarMenu = bMenuBar;
#ifdef USE_NATIVE_WINDOW
	pSalMenu->mpMenu = NULL;
#else	// USE_NATIVE_WINDOW
	pSalMenu->mpVCLMenuBar=NULL;
	pSalMenu->mpVCLMenu=NULL;
#endif	// USE_NATIVE_WINDOW
	pSalMenu->mpParentVCLMenu=pVCLMenuWrapper;

#ifdef USE_NATIVE_WINDOW
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	VCLCreateMenu *pVCLCreateMenu = [VCLCreateMenu create:bMenuBar];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLCreateMenu performSelectorOnMainThread:@selector(createMenu:) withObject:pVCLCreateMenu waitUntilDone:YES modes:pModes];
	VCLMenuWrapper *pMenu = [pVCLCreateMenu menu];
	if ( pMenu )
	{
		[pMenu retain];
		pSalMenu->mpMenu = pMenu;
	}

	[pPool release];
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
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSString *pTitle = [NSString stringWithCharacters:aTitle.getStr() length:aTitle.getLength()];
	VCLCreateMenuItem *pVCLCreateMenuItem = [VCLCreateMenuItem createWithTitle:pTitle type:pItemData->eType id:pItemData->nId menu:pItemData->pMenu];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLCreateMenuItem performSelectorOnMainThread:@selector(createMenuItem:) withObject:pVCLCreateMenuItem waitUntilDone:YES modes:pModes];
	NSMenuItem *pMenuItem = [pVCLCreateMenuItem menuItem];
	if ( pMenuItem )
	{
		[pMenuItem retain];
		pSalMenuItem->mpMenuItem = pMenuItem;
	}

	[pPool release];
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
			Reference< XClipboard > aClipboard = pWindow->GetClipboard();
			if ( aClipboard.is() )
				aClipboard->getContents();
		}
	}

	// Post the SALEVENT_MENUACTIVATE event
	SalMenuEvent *pActivateEvent = new SalMenuEvent( 0, pVCLMenuWrapper );
	com_sun_star_vcl_VCLEvent aActivateEvent( SALEVENT_MENUACTIVATE, pFrame, pActivateEvent );
	aActivateEvent.dispatch();

	USHORT nCount = pVCLMenuWrapper->GetItemCount();
	for( USHORT i = 0; i < nCount; i++ )
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
	SalMenuEvent *pDeactivateEvent = new SalMenuEvent( 0, pVCLMenuWrapper );
	com_sun_star_vcl_VCLEvent aDeactivateEvent( SALEVENT_MENUDEACTIVATE, pFrame, pDeactivateEvent );
	aDeactivateEvent.dispatch();
#endif	// !NO_NATIVE_MENUS
}
