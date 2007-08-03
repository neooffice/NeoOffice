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
 *  Patrick Luby, August 2007
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2007 Planamesa Inc.
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
#import "crt_externs.h"
#import "main_cocoa.h"

static AEEventHandlerUPP pQuitHandlerUPP = nil;

static OSErr CarbonQuitEventHandler( const AppleEvent *pEvent, AppleEvent *pReply, long nRef )
{
	Application_queryExit();
	return noErr;
}

@interface DesktopApplicationDelegate : NSObject
- (BOOL)application:(NSApplication *)pApp openFile:(NSString *)pFilename;
- (void)application:(NSApplication *)pApp openFiles:(NSArray *)pFilenames;
- (BOOL)application:(NSApplication *)pApp printFile:(NSString *)pFilename;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)pApp;
@end

@implementation DesktopApplicationDelegate

- (BOOL)application:(NSApplication *)pApp openFile:(NSString *)pFilename
{
	if ( pFilename )
		Application_openOrPrintFile( (CFStringRef)pFilename, NO );
	return YES;
}

- (void)application:(NSApplication *)pApp openFiles:(NSArray *)pFilenames
{
	if ( pFilenames )
	{
		unsigned count = [pFilenames count];
		int i = 0;
		for ( ; i < count; i++ )
			[self application:pApp openFile:[pFilenames objectAtIndex:i]];
	}
}

- (BOOL)application:(NSApplication *)pApp printFile:(NSString *)pFilename
{
	if ( pFilename )
		Application_openOrPrintFile( (CFStringRef)pFilename, YES );
	return YES;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)pApp
{
	Application_queryExit();
	return NSTerminateCancel;
}

@end

@interface DesktopApplication : NSApplication
- (void)sendEvent:(NSEvent *)pEvent;
@end

@implementation DesktopApplication

- (void)sendEvent:(NSEvent *)pEvent
{
	[super sendEvent:pEvent];

	// Handle the Command-Q event
	if ( pEvent && [pEvent type] == NSKeyDown && ( [pEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask ) == NSCommandKeyMask && [@"q" isEqualToString:[pEvent characters]] )
		Application_queryExit();
}

@end

void NSApplication_run( CFRunLoopTimerRef aTimer, void *pInfo )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	// We need to override our own NSApplication methods to handle some menu
	// key equivalents in the default application menu
	[DesktopApplication poseAsClass:[NSApplication class]];

	// Load default application menu
	if ( NSApplicationLoad() )
	{
		// Make sure that X11.app is running
		NSTask *pTask = [[NSTask alloc] init];
		NSMutableArray *pArgs = [NSMutableArray array];
		if ( pTask && pArgs )
		{
			[pArgs addObject:@"-a"];
			[pArgs addObject:@"X11"];
			[pTask setArguments:pArgs];
			[pTask setLaunchPath:@"/usr/bin/open"];
			[pTask launch];
			[pTask waitUntilExit];
		}

		NSApplication *pApp = [NSApplication sharedApplication];
		if ( pApp )
		{
			[pApp setDelegate:[[DesktopApplicationDelegate alloc] init]];

			// Install event handler for the quit menu
			if ( !pQuitHandlerUPP )
			{
				pQuitHandlerUPP = NewAEEventHandlerUPP( CarbonQuitEventHandler );
				if ( pQuitHandlerUPP )
					AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, pQuitHandlerUPP, 0, NO );
			}

			// Run native event loop
			[pApp run];
		}
	}

	if ( aTimer )
	{
		CFRunLoopRemoveTimer( CFRunLoopGetCurrent(), aTimer, kCFRunLoopDefaultMode );
		CFRelease( aTimer );
	}

	[pPool release];
}
