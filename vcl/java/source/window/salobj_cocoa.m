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
}
- (id)initWithContentRect:(NSRect)aContentRect backgroundColor:(NSColor *)pColor;
@end

@implementation VCLChildWindow

- (id)initWithContentRect:(NSRect)aContentRect backgroundColor:(NSColor *)pColor;
{
	// Create a borderless window and set the background to be transparent
	[super initWithContentRect:aContentRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:YES];
	[self setOpaque:NO];
	[self setBackgroundColor:pColor];

	return self;
}

@end

@interface ReleaseChildWindow : NSObject
{
	VCLChildWindow*			mpWindow;
}
- (id)initWithChildWindow:(VCLChildWindow *)pWindow;
- (void)releaseWindow:(id)pObject;
@end

@implementation ReleaseChildWindow

- (id)initWithChildWindow:(VCLChildWindow *)pWindow
{
	[super init];
 
	mpWindow = pWindow;
 
	return self;
}

- (void)releaseWindow:(id)pObject
{
	[mpWindow release];
}

@end

id VCLChildWindow_create( long nX, long nY, long nWidth, long nHeight, int nColor )
{
	if ( nWidth <= 0 )
		nWidth = 1;
	if ( nHeight <= 0 )
		nHeight = 1;
	return [[VCLChildWindow alloc] initWithContentRect:NSMakeRect( nX, nY, nWidth, nHeight ) backgroundColor:[NSColor colorWithDeviceRed:( (float)( ( nColor & 0x00ff0000 ) >> 16 ) / (float)0xff ) green:( (float)( ( nColor & 0x0000ff00 ) >> 8 ) / (float)0xff ) blue:( (float)( nColor & 0x000000ff ) / (float)0xff ) alpha:( (float)( ( nColor & 0xff000000 ) >> 24 ) / (float)0xff )]];
}

void VCLChildWindow_release( id *pVCLChildWindow )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pVCLChildWindow )
	{
		ReleaseChildWindow *pReleaseChildWindow = [[ReleaseChildWindow alloc] initWithChildWindow:(VCLChildWindow *)pVCLChildWindow];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pReleaseChildWindow performSelectorOnMainThread:@selector(releaseWindow:) withObject:pReleaseChildWindow waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void VCLChildWindow_setBackgroundColor( id *pVCLChildWindow, int nColor )
{
}

void VCLChildWindow_setBounds( id *pVCLChildWindow, long nX, long nY, long nWidth, long nHeight )
{
}

void VCLChildWindow_setParent( id *pParentNSWindow )
{
}

void VCLChildWindow_show( id *pVCLChildWindow, id *pNSParentWindow, BOOL bShow )
{
}
