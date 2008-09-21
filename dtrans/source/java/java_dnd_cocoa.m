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
 *         - GNU General Public License Version 3
 *
 *  Patrick Luby, August 2008
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2009 Planamesa Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License version 3, as published by the Free Software Foundation.
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
#import "java_dnd_cocoa.h"

@interface GetWindowRefFromNSView : NSObject
{
	NSView*				mpView;
	WindowRef			maResult;
}
+ (id)createWithView:(NSView *)pView;
- (void)getWindowRef:(id)pObject;
- (id)initWithView:(void *)pView;
- (WindowRef)result;
@end

@implementation GetWindowRefFromNSView

+ (id)createWithView:(NSView *)pView
{
	GetWindowRefFromNSView *pRet = [[GetWindowRefFromNSView alloc] initWithView:pView];
	[pRet autorelease];
	return pRet;
}

- (void)getWindowRef:(id)pObject
{
	if ( mpView )
	{
		NSWindow *pWindow = [mpView window];
		if ( pWindow )
			maResult = [pWindow windowRef];
	}
}

- (id)initWithView:(void *)pView
{
	[super init];

	mpView = pView;
	maResult = nil;

	return self;
}

- (WindowRef)result
{
	return maResult;
}

@end

WindowRef NSView_windowRef( NSView *pView )
{
	WindowRef aRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pView )
	{
		GetWindowRefFromNSView *pGetWindowRefFromNSView = [GetWindowRefFromNSView createWithView:pView];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pGetWindowRefFromNSView performSelectorOnMainThread:@selector(getWindowRef:) withObject:pGetWindowRefFromNSView waitUntilDone:YES modes:pModes];
		aRet = [pGetWindowRefFromNSView result];
	}

	[pPool release];

	return aRet;
}
