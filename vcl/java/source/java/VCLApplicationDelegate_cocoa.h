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
 *  Patrick Luby, August 2010
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2010 Planamesa Inc.
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

#ifndef __VCLAPPLICATIONDELEGATE_COCOA_H__
#define __VCLAPPLICATIONDELEGATE_COCOA_H__

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

@interface VCLMainMenuDidEndTracking : NSObject
+ (void)mainMenuDidEndTracking:(MacOSBOOL)bNoDelay;
- (void)handlePendingMainMenuChanges:(id)pObject;
@end

@interface VCLApplicationDelegate : NSObject
{
	MacOSBOOL				mbAppMenuInitialized;
	MacOSBOOL				mbCancelTracking;
	id						mpDelegate;
	NSMenu*					mpDockMenu;
	MacOSBOOL				mbInTermination;
	MacOSBOOL				mbInTracking;
}
+ (VCLApplicationDelegate *)sharedDelegate;
- (void)addMenuBarItem:(NSNotification *)pNotification;
- (MacOSBOOL)application:(NSApplication *)pApplication openFile:(NSString *)pFilename;
- (MacOSBOOL)application:(NSApplication *)pApplication printFile:(NSString *)pFilename;
- (void)applicationDidBecomeActive:(NSNotification *)pNotification;
- (void)applicationDidChangeScreenParameters:(NSNotification *)pNotification;
- (NSMenu *)applicationDockMenu:(NSApplication *)pApplication;
- (MacOSBOOL)applicationShouldHandleReopen:(NSApplication *)pApplication hasVisibleWindows:(MacOSBOOL)bFlag;
- (MacOSBOOL)applicationShouldOpenUntitledFile:(NSApplication *)pSender;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)pApplication;
- (void)applicationWillFinishLaunching:(NSNotification *)pNotification;
- (void)cancelTermination;
- (void)dealloc;
- (id)init;
- (MacOSBOOL)isInTracking;
- (void)menuNeedsUpdate:(NSMenu *)pMenu;
- (void)setDelegate:(id)pDelegate;
- (void)showAbout;
- (void)showPreferences;
- (void)trackMenuBar:(NSNotification *)pNotification;
- (MacOSBOOL)validateMenuItem:(NSMenuItem *)pMenuItem;
@end

#endif
