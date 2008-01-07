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
 *  Patrick Luby, December 2007
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
#import "salobj_cocoa.h"

@interface VCLChildWindow : NSWindow
{
	NSRect					maFlippedBounds;
}
- (NSRect)flippedBounds;
- (id)init:(id)pObject;
- (void)release:(id)pObject;
- (void)setBounds:(NSValue *)pValue;
@end

@implementation VCLChildWindow

- (NSRect)flippedBounds
{
	return maFlippedBounds;
}

- (id)init:(id)pObject
{
	// Create a borderless window and set the background to be transparent
	[super initWithContentRect:NSMakeRect( 0, 0, 1, 1 ) styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:YES];

	maFlippedBounds = [self frame];
	[self setOpaque:NO];
	[self setBackgroundColor:[NSColor colorWithDeviceRed:0 green:0 blue:0 alpha:0]];

	return self;
}

- (void)release:(id)pObject
{
	[self orderOut:self];
	[self release];
}

- (void)setBounds:(NSValue *)pValue
{
	if ( pValue )
	{
		maFlippedBounds = [pValue rectValue];

		NSWindow *pParentWindow = [self parentWindow];
		if ( pParentWindow && [pParentWindow isVisible] )
		{
			NSRect aParentFrame = [pParentWindow contentRectForFrameRect:[pParentWindow frame]];
			NSRect aBounds = NSIntersectionRect( aParentFrame, NSMakeRect( aParentFrame.origin.x + maFlippedBounds.origin.x, aParentFrame.origin.y + aParentFrame.size.height - maFlippedBounds.origin.y - maFlippedBounds.size.height, maFlippedBounds.size.width, maFlippedBounds.size.height ) );
			[self setFrame:aBounds display:NO];
		}
	}
}

@end

@interface ShowChildWindow : NSObject
{
	VCLChildWindow*			mpWindow;
	NSWindow*				mpParentWindow;
	WindowRef				maWindowRef;
}
- (WindowRef)getWindowRef;
- (id)initWithWindow:(VCLChildWindow *)pWindow parentWindow:(NSWindow *)pParentWindow;
- (void)show:(id)pObject;
@end

@implementation ShowChildWindow

- (WindowRef)getWindowRef
{
	return maWindowRef;
}

- (id)initWithWindow:(VCLChildWindow *)pWindow parentWindow:(NSWindow *)pParentWindow
{
	[super init];

	mpWindow = pWindow;
	mpParentWindow = pParentWindow;
	maWindowRef = nil;

	return self;
}

- (void)show:(id)pObject
{
	if ( mpWindow )
	{
		// Detach from current parent window
		NSWindow *pOldParentWindow = [mpWindow parentWindow];
		if ( pOldParentWindow )
			[pOldParentWindow removeChildWindow:mpWindow];

		if ( mpParentWindow && [mpParentWindow isVisible] )
		{
			NSRect aFlippedBounds = [mpWindow flippedBounds];
			NSRect aParentFrame = [mpParentWindow contentRectForFrameRect:[mpParentWindow frame]];
			NSRect aBounds = NSIntersectionRect( aParentFrame, NSMakeRect( aParentFrame.origin.x + aFlippedBounds.origin.x, aParentFrame.origin.y + aParentFrame.size.height - aFlippedBounds.origin.y - aFlippedBounds.size.height, aFlippedBounds.size.width, aFlippedBounds.size.height ) );
			[mpWindow setFrame:aBounds display:NO];
			[mpParentWindow addChildWindow:mpWindow ordered:NSWindowAbove];
			[mpWindow orderFront:self];
			maWindowRef = (WindowRef)[mpWindow windowRef];
		}
	}
}

@end

id VCLChildWindow_create()
{
	VCLChildWindow *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	pRet = [VCLChildWindow alloc];
	if ( pRet )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pRet performSelectorOnMainThread:@selector(init:) withObject:pRet waitUntilDone:YES modes:pModes];
		[pRet retain];
	}

	[pPool release];

	return pRet;
}

void VCLChildWindow_release( id pVCLChildWindow )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pVCLChildWindow )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(VCLChildWindow *)pVCLChildWindow performSelectorOnMainThread:@selector(release:) withObject:pVCLChildWindow waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void VCLChildWindow_setBackgroundColor( id pVCLChildWindow, int nColor )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pVCLChildWindow )
	{
		NSColor *pColor = [NSColor colorWithDeviceRed:( (float)( ( nColor & 0x00ff0000 ) >> 16 ) / (float)0xff ) green:( (float)( ( nColor & 0x0000ff00 ) >> 8 ) / (float)0xff ) blue:( (float)( nColor & 0x000000ff ) / (float)0xff ) alpha:( (float)( ( nColor & 0xff000000 ) >> 24 ) / (float)0xff )];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(VCLChildWindow *)pVCLChildWindow performSelectorOnMainThread:@selector(setBackgroundColor:) withObject:pColor waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void VCLChildWindow_setBounds( id pVCLChildWindow, long nX, long nY, long nWidth, long nHeight )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pVCLChildWindow )
	{
		if ( nWidth < 1 )
			nWidth = 1;
		if ( nHeight < 1 )
			nHeight = 1;
		NSValue *pValue = [NSValue valueWithRect:NSMakeRect( nX, nY, nWidth, nHeight )];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(VCLChildWindow *)pVCLChildWindow performSelectorOnMainThread:@selector(setBounds:) withObject:pValue waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

WindowRef VCLChildWindow_show( id pVCLChildWindow, id pParentNSWindow, BOOL bShow )
{
	WindowRef aRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pVCLChildWindow )
	{
		ShowChildWindow *pShowChildWindow = [[ShowChildWindow alloc] initWithWindow:pVCLChildWindow parentWindow:pParentNSWindow];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pShowChildWindow performSelectorOnMainThread:@selector(show:) withObject:pShowChildWindow waitUntilDone:YES modes:pModes];
		aRet = [pShowChildWindow getWindowRef];
	}

	[pPool release];

	return aRet;
}
