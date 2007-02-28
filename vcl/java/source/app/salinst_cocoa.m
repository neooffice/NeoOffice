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
 *  Patrick Luby, September 2005
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
#import "salinst_cocoa.h"

@interface GetModalWindow : NSObject
{
	NSWindow*			mpModalWindow;
}
- (void)getModalWindow:(id)pObject;
- (id)init;
- (NSWindow *)modalWindow;
@end

@implementation GetModalWindow

- (void)getModalWindow:(id)pObject
{
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
		mpModalWindow = [pApp modalWindow];
}

- (id)init
{
	[super init];
 
	mpModalWindow = nil;
 
	return self;
}

- (NSWindow *)modalWindow
{
	return mpModalWindow;
}

@end

@interface EnableFlushing : NSObject
{
	BOOL				mbEnable;
}
- (void)enableFlushing:(id)pObject;
- (id)initWithEnable:(BOOL)bEnable;
@end

@implementation EnableFlushing

- (void)enableFlushing:(id)pObject
{
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSArray *pArray = [pApp windows];
		if ( pArray )
		{
			int nCount = [pArray count];
			int i = 0;
			for ( ; i < nCount; i++ )
			{
				NSWindow *pWindow = (NSWindow *)[pArray objectAtIndex:i];
				if ( pWindow && [pWindow isVisible] && [pWindow isFlushWindowDisabled] != mbEnable )
				{
					if ( mbEnable )
						[pWindow enableFlushWindow];
					else
						[pWindow disableFlushWindow];
				}
 			}
 		}
	}
}

- (id)initWithEnable:(BOOL)bEnable;
{
	[super init];
 
	mbEnable = bEnable;
 
	return self;
}

@end

void NSApplication_enableFlushing( BOOL bEnable )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	EnableFlushing *pEnableFlushing = [[EnableFlushing alloc] initWithEnable:bEnable];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pEnableFlushing performSelectorOnMainThread:@selector(enableFlushing:) withObject:pEnableFlushing waitUntilDone:NO modes:pModes];

	[pPool release];
}

id NSApplication_getModalWindow()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSWindow *pModalWindow = nil;

	GetModalWindow *pGetModalWindow = [[GetModalWindow alloc] init];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pGetModalWindow performSelectorOnMainThread:@selector(getModalWindow:) withObject:pGetModalWindow waitUntilDone:YES modes:pModes];
	pModalWindow = [pGetModalWindow modalWindow];

	[pPool release];

	return pModalWindow;
}
