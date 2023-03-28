/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __VCLAPPLICATIONDELEGATE_COCOA_H__
#define __VCLAPPLICATIONDELEGATE_COCOA_H__

#include <premac.h>
#import <Cocoa/Cocoa.h>
#import <apple_remote/RemoteMainController.h>
#include <postmac.h>

@interface VCLMainMenuDidEndTracking : NSObject
+ (void)mainMenuDidEndTracking:(BOOL)bNoDelay;
- (void)handlePendingMainMenuChanges:(id)pObject;
@end

@interface VCLApplicationDelegate : NSObject <NSApplicationDelegate, NSMenuDelegate>
{
	BOOL					mbAppMenuInitialized;
	BOOL					mbAwaitingTracking;
	BOOL					mbCancelTracking;
	id						mpDelegate;
	NSMenu*					mpDockMenu;
	BOOL					mbInPerformKeyEquivalent;
	BOOL					mbInTermination;
	BOOL					mbInTracking;
	AppleRemoteMainController*	mpAppleRemoteMainController;
}
+ (VCLApplicationDelegate *)sharedDelegate;
- (void)addMenuBarItem:(NSNotification *)pNotification;
- (BOOL)application:(NSApplication *)pApplication openFile:(NSString *)pFilename;
- (BOOL)application:(NSApplication *)pApplication printFile:(NSString *)pFilename;
- (void)applicationDidBecomeActive:(NSNotification *)pNotification;
- (void)applicationDidChangeScreenParameters:(NSNotification *)pNotification;
- (NSMenu *)applicationDockMenu:(NSApplication *)pApplication;
- (BOOL)applicationShouldHandleReopen:(NSApplication *)pApplication hasVisibleWindows:(BOOL)bFlag;
- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)pSender;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)pApplication;
- (void)applicationWillBecomeActive:(NSNotification *)pNotification;
- (void)applicationWillFinishLaunching:(NSNotification *)pNotification;
- (void)applicationWillResignActive:(NSNotification *)pNotification;
- (void)cancelTermination;
- (void)dealloc;
- (id)init;
- (BOOL)isInPerformKeyEquivalent;
- (BOOL)isInTermination;
- (BOOL)isInTracking;
- (void)menuNeedsUpdate:(NSMenu *)pMenu;
- (void)setDelegate:(id)pDelegate;
- (void)setInPerformKeyEquivalent:(BOOL)bInPerformKeyEquivalent;
- (void)showAbout;
- (void)showPreferences;
- (void)trackMenuBar:(NSNotification *)pNotification;
- (BOOL)validateMenuItem:(NSMenuItem *)pMenuItem;
@end

#endif
