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

@interface ShutdownIconDelegate : NSObject
{
	NSMenu*				mpDockMenu;
	id					mpDelegate;
}
- (BOOL)application:(NSApplication *)pApplication delegateHandlesKey:(NSString *)pKey;
- (BOOL)application:(NSApplication *)pApplication openFile:(NSString *)pFilename;
- (void)application:(NSApplication *)pApplication openFiles:(NSArray *)pFilenames;
- (BOOL)application:(NSApplication *)pApplication openFileWithoutUI:(NSString *)pFilename;
- (BOOL)application:(NSApplication *)pApplication openTempFile:(NSString *)pFilename;
- (BOOL)application:(NSApplication *)pApplication printFile:(NSString *)pFilename;
- (void)application:(NSApplication *)pApplication printFiles:(NSArray *)pFilenames;
#if (BUILD_OS_MAJOR >= 10) && (BUILD_OS_MINOR >= 4)
- (NSApplicationPrintReply)application:(NSApplication *)pApplication printFiles:(NSArray *)pFilenames withSettings:(NSDictionary *)pPrintSettings showPrintPanels:(BOOL)bShowPrintPanels;
#endif
- (void)applicationDidBecomeActive:(NSNotification *)pNotification;
- (void)applicationDidChangeScreenParameters:(NSNotification *)pNotification;
- (void)applicationDidFinishLaunching:(NSNotification *)pNotification;
- (void)applicationDidHide:(NSNotification *)pNotification;
- (void)applicationDidResignActive:(NSNotification *)pNotification;
- (void)applicationDidUnhide:(NSNotification *)pNotification;
- (void)applicationDidUpdate:(NSNotification *)pNotification;
- (NSMenu *)applicationDockMenu:(NSApplication *)pApplication;
- (BOOL)applicationOpenUntitledFile:(NSApplication *)pApplication;
- (BOOL)applicationShouldHandleReopen:(NSApplication *)pApplication hasVisibleWindows:(BOOL)bFlag;
- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)pApplication;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)pApplication;
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)pApplication;
- (void)applicationWillBecomeActive:(NSNotification *)pNotification;
- (void)applicationWillFinishLaunching:(NSNotification *)pNotification;
- (void)applicationWillHide:(NSNotification *)pNotification;
- (void)applicationWillResignActive:(NSNotification *)pNotification;
- (void)applicationWillTerminate:(NSNotification *)pNotification;
- (void)applicationWillUnhide:(NSNotification *)pNotification;
- (void)applicationWillUpdate:(NSNotification *)pNotification;
- (void)dealloc;
- (id)init;
- (void)handleCalcCommand:(id)pObject;
- (void)handleDrawCommand:(id)pObject;
- (void)handleFileOpenCommand:(id)pObject;
- (void)handleFromTemplateCommand:(id)pObject;
- (void)handleImpressCommand:(id)pObject;
- (void)handleMathCommand:(id)pObject;
- (void)handleWriterCommand:(id)pObject;
- (void)setDelegate:(id)pObject;
@end

@implementation ShutdownIconDelegate

- (BOOL)application:(NSApplication *)pApplication delegateHandlesKey:(NSString *)pKey;
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(application:delegateHandlesKey:)] )
		return [mpDelegate application:pApplication delegateHandlesKey:pKey];
	else
		return NO;
}

- (BOOL)application:(NSApplication *)pApplication openFile:(NSString *)pFilename
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(application:openFile:)] )
		return [mpDelegate application:pApplication openFile:pFilename];
	else
		return NO;
}

- (void)application:(NSApplication *)pApplication openFiles:(NSArray *)pFilenames
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(application:openFile:)] )
		[mpDelegate application:pApplication openFiles:pFilenames];
}

- (BOOL)application:(NSApplication *)pApplication openFileWithoutUI:(NSString *)pFilename
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(application:openFileWithoutUI:)] )
		return [mpDelegate application:pApplication openFileWithoutUI:pFilename];
	else
		return NO;
}

- (BOOL)application:(NSApplication *)pApplication openTempFile:(NSString *)pFilename
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(application:openTempFile:)] )
		return [mpDelegate application:pApplication openTempFile:pFilename];
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

- (void)application:(NSApplication *)pApplication printFiles:(NSArray *)pFilenames
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(application:printFile:)] )
		[mpDelegate application:pApplication printFiles:pFilenames];
}

#if (BUILD_OS_MAJOR >= 10) && (BUILD_OS_MINOR >= 4)
- (NSApplicationPrintReply)application:(NSApplication *)pApplication printFiles:(NSArray *)pFilenames withSettings:(NSDictionary *)pPrintSettings showPrintPanels:(BOOL)bShowPrintPanels
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(application:printFiles:withSettings:showPrintPanels:)] )
		return [mpDelegate application:pApplication printFiles:pFilenames withSettings:pPrintSettings showPrintPanels:bShowPrintPanels];
	else
		return NSPrintingFailure;
}
#endif

- (void)applicationDidBecomeActive:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationDidBecomeActive:)] )
		[mpDelegate applicationDidBecomeActive:pNotification];
}

- (void)applicationDidChangeScreenParameters:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationDidChangeScreenParameters:)] )
		[mpDelegate applicationDidChangeScreenParameters:pNotification];
}

- (void)applicationDidFinishLaunching:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationDidFinishLaunching:)] )
		[mpDelegate applicationDidFinishLaunching:pNotification];
}

- (void)applicationDidHide:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationDidHide:)] )
		[mpDelegate applicationDidHide:pNotification];
}

- (void)applicationDidResignActive:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationDidResignActive:)] )
		[mpDelegate applicationDidResignActive:pNotification];
}

- (void)applicationDidUnhide:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationDidUnhide:)] )
		[mpDelegate applicationDidUnhide:pNotification];
}

- (void)applicationDidUpdate:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationDidUpdate:)] )
		[mpDelegate applicationDidUpdate:pNotification];
}

- (NSMenu *)applicationDockMenu:(NSApplication *)pApplication
{   
    return mpDockMenu;
} 

- (BOOL)applicationOpenUntitledFile:(NSApplication *)pApplication
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationOpenUntitledFile:)] )
		return [mpDelegate applicationOpenUntitledFile:pApplication];
	else
		return NO;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)pApplication hasVisibleWindows:(BOOL)bFlag
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationShouldHandleReopen:hasVisibleWindows:)] )
		return [mpDelegate applicationShouldHandleReopen:pApplication hasVisibleWindows:bFlag];
	else
		return NO;
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)pApplication
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationShouldOpenUntitledFile:)] )
		return [mpDelegate applicationShouldOpenUntitledFile:pApplication];
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

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)pApplication
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationShouldTerminateAfterLastWindowClosed:)] )
		return [mpDelegate applicationShouldTerminateAfterLastWindowClosed:pApplication];
	else
		return NO;
}

- (void)applicationWillBecomeActive:(NSNotification *)pNotification;
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationWillBecomeActive:)] )
		[mpDelegate applicationWillBecomeActive:pNotification];
}

- (void)applicationWillFinishLaunching:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationWillFinishLaunching:)] )
		[mpDelegate applicationWillFinishLaunching:pNotification];
}

- (void)applicationWillHide:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationWillHide:)] )
		[mpDelegate applicationWillHide:pNotification];
}

- (void)applicationWillResignActive:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationWillResignActive:)] )
		[mpDelegate applicationWillResignActive:pNotification];
}

- (void)applicationWillTerminate:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationWillTerminate:)] )
		[mpDelegate applicationWillTerminate:pNotification];
}

- (void)applicationWillUnhide:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationWillUnhide:)] )
		[mpDelegate applicationWillUnhide:pNotification];
}

- (void)applicationWillUpdate:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(applicationWillUpdate:)] )
		[mpDelegate applicationWillUpdate:pNotification];
}

- (void)dealloc
{
	if ( mpDockMenu )
		[mpDockMenu release];
	[super dealloc];
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


- (void)setDelegate:(id)pObject
{   
    mpDelegate = pObject;
}

@end

void AddQuickstartMenuItems( int nCount, MenuCommand *pIDs, CFStringRef *pStrings )
{
	if ( !nCount )
		return;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

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
			for ( i = nCount - 1; i >= 0; i-- )
			{
				switch ( pIDs[ i ] )
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

				[pAppMenu insertItemWithTitle:(NSString *)pStrings[ i ] action:aSelector keyEquivalent:@"" atIndex:2];
				[pDockMenu insertItemWithTitle:(NSString *)pStrings[ i ] action:aSelector keyEquivalent:@"" atIndex:0];
			}
		}
	}

	[pPool release];
}
