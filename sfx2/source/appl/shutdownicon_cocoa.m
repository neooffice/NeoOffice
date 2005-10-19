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
 *  Patrick Luby, July 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 by Patrick Luby (patrick.luby@planamesa.com)
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

#import <Cocoa/Cocoa.h>
#import "shutdownicon_cocoa.h"

/*
 * Create a class that is a facade for the application delegate set by the JVM.
 * Note that this class only implement the delegate methods in the JVM's
 * ApplicationDelegate and AWTApplicationDelegate classes.
 */
@interface ShutdownIconDelegate : NSObject
{
	NSMenu*				mpDockMenu;
	id					mpDelegate;
}
- (BOOL)application:(NSApplication *)pApplication openFile:(NSString *)pFilename;
- (BOOL)application:(NSApplication *)pApplication printFile:(NSString *)pFilename;
- (void)applicationDidChangeScreenParameters:(NSNotification *)pNotification;
- (NSMenu *)applicationDockMenu:(NSApplication *)pApplication;
- (BOOL)applicationShouldHandleReopen:(NSApplication *)pApplication hasVisibleWindows:(BOOL)bFlag;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)pApplication;
- (void)applicationWillFinishLaunching:(NSNotification *)pNotification;
- (id)init;
- (void)handleCalcCommand:(id)pObject;
- (void)handleDrawCommand:(id)pObject;
- (void)handleFileOpenCommand:(id)pObject;
- (void)handleFromTemplateCommand:(id)pObject;
- (void)handleImpressCommand:(id)pObject;
- (void)handleMathCommand:(id)pObject;
- (void)handleWriterCommand:(id)pObject;
- (void)setDelegate:(id)pDelegate;
@end

@implementation ShutdownIconDelegate

- (BOOL)application:(NSApplication *)pApplication openFile:(NSString *)pFilename
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(application:openFile:)] )
		return [mpDelegate application:pApplication openFile:pFilename];
	else
		return NO;
}

- (BOOL)application:(NSApplication *)pApplication printFile:(NSString *)pFilename
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(application:printFile:)] )
		return [mpDelegate application:pApplication printFile:pFilename];
	else
		return NO;
}

- (NSMenu *)applicationDockMenu:(NSApplication *)pApplication
{   
    return mpDockMenu;
} 

- (void)applicationDidChangeScreenParameters:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationDidChangeScreenParameters:)] )
		[mpDelegate applicationDidChangeScreenParameters:pNotification];
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)pApplication hasVisibleWindows:(BOOL)bFlag
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationShouldHandleReopen:hasVisibleWindows:)] )
		return [mpDelegate applicationShouldHandleReopen:pApplication hasVisibleWindows:bFlag];
	else
		return NO;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)pApplication
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationShouldTerminate:)] )
		return [mpDelegate applicationShouldTerminate:pApplication];
	else
		return NSTerminateCancel;
}

- (void)applicationWillFinishLaunching:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationWillFinishLaunching:)] )
		[mpDelegate applicationWillFinishLaunching:pNotification];
}

- (id)init
{
	[super init];

	mpDockMenu = [[NSMenu alloc] initWithTitle:@""];

	return self;
}

- (void)handleCalcCommand:(id)pObject
{
	ProcessShutdownIconCommand( CALC_COMMAND_ID );
}

- (void)handleDrawCommand:(id)pObject
{
	ProcessShutdownIconCommand( DRAW_COMMAND_ID );
}

- (void)handleFileOpenCommand:(id)pObject
{
	ProcessShutdownIconCommand( FILEOPEN_COMMAND_ID );
}

- (void)handleFromTemplateCommand:(id)pObject
{
	ProcessShutdownIconCommand( FROMTEMPLATE_COMMAND_ID );
}

- (void)handleImpressCommand:(id)pObject
{
	ProcessShutdownIconCommand( IMPRESS_COMMAND_ID );
}

- (void)handleMathCommand:(id)pObject
{
	ProcessShutdownIconCommand( MATH_COMMAND_ID );
}

- (void)handleWriterCommand:(id)pObject
{
	ProcessShutdownIconCommand( WRITER_COMMAND_ID );
}


- (void)setDelegate:(id)pDelegate
{
	if ( pDelegate )
    	mpDelegate = pDelegate;
}

@end

@interface QuickstartMenuItems : NSObject
{
	int					mnCount;
	MenuCommand*		mpIDs;
	CFStringRef*		mpStrings;
}
- (void)addMenuItems:(id)pObject;
- (id)initWithCount:(int)nCount menuCommands:(MenuCommand *)pIDs strings:(CFStringRef *)pStrings;
@end

@implementation QuickstartMenuItems

- (void)addMenuItems:(id)pObject
{
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSMenu *pAppMenu = nil;
		NSMenu *pDockMenu = nil;

		NSMenu *pMainMenu = [pApp mainMenu];
		if ( pMainMenu && [pMainMenu numberOfItems] > 0 )
		{
			NSMenuItem *pItem = [pMainMenu itemAtIndex:0];
			if ( pItem )
				pAppMenu = [pItem submenu];
		}

		NSObject *pDelegate = [pApp delegate];
		if ( pDelegate && [pDelegate respondsToSelector:@selector(applicationDockMenu:)] )
			pDockMenu = [pDelegate applicationDockMenu:pApp];

		if ( !pDockMenu )
		{
			ShutdownIconDelegate *pNewDelegate = [[ShutdownIconDelegate alloc] init];
			if ( pDelegate )
				[pNewDelegate setDelegate:pDelegate];
			[pApp setDelegate: pNewDelegate];
			pDockMenu = [pNewDelegate applicationDockMenu:pApp];
		}

		if ( pAppMenu && pDockMenu )
		{
			// Insert a separator menu item (only in the application menu)
			[pAppMenu insertItem:[NSMenuItem separatorItem] atIndex:2];

			// Work the list of menu items is reverse order
			SEL aSelector;
			int i;
			for ( i = mnCount - 1; i >= 0; i-- )
			{
				switch ( mpIDs[ i ] )
				{
					case CALC_COMMAND_ID:
						aSelector = @selector(handleCalcCommand:);
						break;
					case DRAW_COMMAND_ID:
						aSelector = @selector(handleDrawCommand:);
						break;
					case FILEOPEN_COMMAND_ID:
						aSelector = @selector(handleFileOpenCommand:);
						break;
					case FROMTEMPLATE_COMMAND_ID:
						aSelector = @selector(handleFromTemplateCommand:);
						break;
					case IMPRESS_COMMAND_ID:
						aSelector = @selector(handleImpressCommand:);
						break;
					case MATH_COMMAND_ID:
						aSelector = @selector(handleMathCommand:);
						break;
					case WRITER_COMMAND_ID:
						aSelector = @selector(handleWriterCommand:);
						break;
					default:
						aSelector = nil;
						break;
				}

				// Insert and release string
				if ( mpStrings[ i ] )
				{
					[pAppMenu insertItemWithTitle:(NSString *)mpStrings[ i ] action:aSelector keyEquivalent:@"" atIndex:2];
					[pDockMenu insertItemWithTitle:(NSString *)mpStrings[ i ] action:aSelector keyEquivalent:@"" atIndex:0];
				}
			}

			mpIDs = nil;
			mpStrings = nil;
		}
	}
}

- (id)initWithCount:(int)nCount menuCommands:(MenuCommand *)pIDs strings:(CFStringRef *)pStrings;
{
	mnCount = nCount;
	mpIDs = pIDs;
	mpStrings = pStrings;
}

@end

void AddQuickstartMenuItems( int nCount, MenuCommand *pIDs, CFStringRef *pStrings )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	QuickstartMenuItems *pItems = [[QuickstartMenuItems alloc] initWithCount:nCount menuCommands:pIDs strings:pStrings];
	[pItems performSelectorOnMainThread:@selector(addMenuItems:) withObject:pItems waitUntilDone:YES];

	[pPool release];
}
