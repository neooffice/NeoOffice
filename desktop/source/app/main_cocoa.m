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
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
		[pApp terminate:pApp];

	[pPool release];

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
	BOOL bRet = NO;

	if ( pFilename )
		bRet = Application_openOrPrintFile( (CFStringRef)pFilename, NO );

	return bRet;
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
	BOOL bRet = NO;

	if ( pFilename )
		bRet = Application_openOrPrintFile( (CFStringRef)pFilename, YES );

	return bRet;
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
		[self terminate:self];
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

	[pPool release];

	if ( aTimer )
	{
		CFRunLoopRemoveTimer( CFRunLoopGetCurrent(), aTimer, kCFRunLoopDefaultMode );
		CFRelease( aTimer );
	}
}
