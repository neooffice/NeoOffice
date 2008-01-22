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

@interface VCLChildSuperview : NSView
- (BOOL)isFlipped;
@end

@implementation VCLChildSuperview

- (BOOL)isFlipped
{
	return YES;
}

@end

@interface VCLChildView : VCLChildSuperview
{
	NSColor*				mpBackgroundColor;
	NSRect					maClipRect;
	NSView*					mpSuperview;
}
- (void)dealloc;
- (void)drawRect:(NSRect)aRect;
- (id)initWithFrame:(NSRect)aFrame;
- (BOOL)isOpaque;
- (void)release:(id)pObject;
- (void)setBackgroundColor:(NSColor *)pColor;
- (void)setBounds:(NSValue *)pValue;
- (void)setClip:(NSValue *)pValue;
@end

@implementation VCLChildView

- (void)dealloc
{
	NSArray *pSubviews = [self subviews];
	if ( pSubviews )
	{
		// Make a copy as the array will change when we remove subviews
		pSubviews = [NSArray arrayWithArray:pSubviews];
		if ( pSubviews )
		{
			unsigned int nCount = [pSubviews count];
			unsigned int i = 0;
			for ( ; i < nCount; i++ )
			{
				NSView *pSubview = (NSView *)[pSubviews objectAtIndex:i];
				if ( pSubview )
					[pSubview removeFromSuperview];
			}
		}
	}

	[self removeFromSuperview];

	if ( mpSuperview )
	{
		[mpSuperview removeFromSuperview];
		[mpSuperview release];
	}

	if ( mpBackgroundColor )
		[mpBackgroundColor release];

	[super dealloc];
}

- (void)drawRect:(NSRect)aRect
{
	[super drawRect:aRect];

	if ( mpBackgroundColor )
		[mpBackgroundColor set];
	else
		[[NSColor whiteColor] set];
	[NSBezierPath fillRect:aRect];
}

- (id)initWithFrame:(NSRect)aFrame
{
	NSRect aChildFrame = NSMakeRect( 0, 0, aFrame.size.width, aFrame.size.height );
	[super initWithFrame:aChildFrame];

	mpBackgroundColor = nil;
	maClipRect = NSZeroRect;

	// Create a superview that we use to control clipping
	mpSuperview = [[VCLChildSuperview alloc] initWithFrame:aFrame];
	if ( mpSuperview )
		[mpSuperview addSubview:self positioned:NSWindowAbove relativeTo:nil];

	return self;
}

- (BOOL)isOpaque
{
	return YES;
}

- (void)release:(id)pObject
{
	[self release];
}

- (void)setBackgroundColor:(NSColor *)pColor
{
	if ( mpBackgroundColor )
	{
		[mpBackgroundColor release];
		mpBackgroundColor = nil;
	}

	if ( pColor )
	{
		mpBackgroundColor = pColor;
		[mpBackgroundColor retain];
	}
}

- (void)setBounds:(NSValue *)pValue
{
	if ( pValue && mpSuperview )
	{
		NSRect aNewFrame = [pValue rectValue];
		if ( !NSIsEmptyRect( aNewFrame ) )
		{
			// Set superview to intersection of new frame and clip
			NSRect aParentFrame = [mpSuperview frame];
			NSRect aNewParentFrame = NSMakeRect( 0, 0, aNewFrame.size.width, aNewFrame.size.height );
			if ( !NSIsEmptyRect( maClipRect ) )
				aNewParentFrame = NSIntersectionRect( aNewParentFrame, maClipRect );
			aNewParentFrame.origin.x += aNewFrame.origin.x;
			aNewParentFrame.origin.y += aNewFrame.origin.y;
			[mpSuperview setFrame:aNewParentFrame];

			// Move child view's origin to account for origin of superview
			if ( !NSIsEmptyRect( maClipRect ) )
			{
				aNewFrame.origin.x = maClipRect.origin.x * -1;
				aNewFrame.origin.y = maClipRect.origin.y * -1;
			}
			else
			{
				aNewFrame.origin.x = 0;
				aNewFrame.origin.y = 0;
			}

			[self setFrame:aNewFrame];
			[self setNeedsDisplay:YES];

			// Force a repaint of the superview's superview in the old and new
			// frames
			NSView *pSuperSuperview = [mpSuperview superview];
			if ( pSuperSuperview )
				[pSuperSuperview setNeedsDisplayInRect:aParentFrame];
		}
	}
}

- (void)setClip:(NSValue *)pValue
{
	if ( pValue && mpSuperview )
	{
		NSRect aFrame = [self frame];
		NSRect aParentFrame = [mpSuperview frame];
		if ( !NSIsEmptyRect( maClipRect ) )
		{
			aParentFrame.origin.x -= maClipRect.origin.x;
			aParentFrame.origin.y -= maClipRect.origin.y;
			aParentFrame.size.width = aFrame.size.width;
			aParentFrame.size.height = aFrame.size.height;
		}

		maClipRect = [pValue rectValue];
		if ( NSIsEmptyRect( maClipRect ) )
			maClipRect = NSZeroRect;

		[self setBounds:[NSValue valueWithRect:aParentFrame]];
	}
}

@end

@interface InitVCLChildView : NSObject
{
	NSView*					mpView;
}
+ (id)childWithView:(VCLChildView *)pView;
- (id)initWithView:(VCLChildView *)pView;
- (void)initialize:(id)pObject;
@end

@implementation InitVCLChildView

+ (id)childWithView:(VCLChildView *)pView
{
	InitVCLChildView *pRet = [[InitVCLChildView alloc] initWithView:pView];
	[pRet autorelease];
	return pRet;
}

- (id)initWithView:(VCLChildView *)pView
{
	[super init];

	mpView = pView;

	return self;
}

- (void)initialize:(id)pObject
{
	[mpView initWithFrame:NSMakeRect( 0, 0, 1, 1 )];
}

@end

@interface ShowVCLChildView : NSObject
{
	NSWindow*				mpParentWindow;
	VCLChildView*			mpView;
}
+ (id)childWithView:(VCLChildView *)pView parentWindow:(NSWindow *)pParentWindow;
- (id)initWithView:(VCLChildView *)pView parentWindow:(NSWindow *)pParentWindow;
- (void)show:(id)pObject;
@end

@implementation ShowVCLChildView

+ (id)childWithView:(VCLChildView *)pView parentWindow:(NSWindow *)pParentWindow
{
	ShowVCLChildView *pRet = [[ShowVCLChildView alloc] initWithView:pView parentWindow:pParentWindow];
	[pRet autorelease];
	return pRet;
}

- (id)initWithView:(VCLChildView *)pView parentWindow:(NSWindow *)pParentWindow
{
	[super init];

	mpParentWindow = pParentWindow;
	mpView = pView;

	return self;
}

- (void)show:(id)pObject
{
	if ( mpView )
	{
		NSView *pSuperview = [mpView superview];
		if ( pSuperview )
		{
			NSArray *pSubviews = [mpView subviews];
			if ( pSubviews )
			{
				// Make a copy of the subviews so that we can reattach them
				pSubviews = [NSArray arrayWithArray:pSubviews];
				if ( pSubviews )
				{
					unsigned int nCount = [pSubviews count];
					unsigned int i = 0;
					for ( ; i < nCount; i++ )
					{
						NSView *pSubview = (NSView *)[pSubviews objectAtIndex:i];
						if ( pSubview )
							[pSubview removeFromSuperview];
					}
				}
			}

			// Detach from current parent view
			[pSuperview removeFromSuperview];

			if ( mpParentWindow && [mpParentWindow isVisible] )
			{
				NSView *pContentView = [mpParentWindow contentView];
				if ( pContentView )
				{
					// Always attach to the Java panel's view
					NSArray *pSubviews = [pContentView subviews];
					if ( pSubviews && [pSubviews count] )
						pContentView = (NSView *)[pSubviews objectAtIndex:0];
					else
						pContentView = nil;
				}

#ifndef DISABLE_VCLCHILDVIEWS
				if ( pContentView )
					[pContentView addSubview:pSuperview positioned:NSWindowAbove relativeTo:nil];
#endif	// DISABLE_VCLCHILDVIEWS
			}

			if ( pSubviews )
			{
				unsigned int nCount = [pSubviews count];
				unsigned int i = 0;
				for ( ; i < nCount; i++ )
				{
					NSView *pSubview = (NSView *)[pSubviews objectAtIndex:i];
					if ( pSubview )
						[mpView addSubview:pSubview positioned:NSWindowAbove relativeTo:nil];
				}
			}
		}
	}
}

@end

id VCLChildView_create()
{
	VCLChildView *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	// Do not retain as invoking alloc disables autorelease
	pRet = [VCLChildView alloc];
	if ( pRet )
	{
		InitVCLChildView *pInitVCLChildView = [InitVCLChildView childWithView:pRet];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pInitVCLChildView performSelectorOnMainThread:@selector(initialize:) withObject:pInitVCLChildView waitUntilDone:YES modes:pModes];
	}

	[pPool release];

	return pRet;
}

void VCLChildView_release( id pVCLChildView )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pVCLChildView )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(VCLChildView *)pVCLChildView performSelectorOnMainThread:@selector(release:) withObject:pVCLChildView waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void VCLChildView_setBackgroundColor( id pVCLChildView, int nColor )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pVCLChildView )
	{
		// Always force the background to be opaque
		NSColor *pColor = [NSColor colorWithDeviceRed:( (float)( ( nColor & 0x00ff0000 ) >> 16 ) / (float)0xff ) green:( (float)( ( nColor & 0x0000ff00 ) >> 8 ) / (float)0xff ) blue:( (float)( nColor & 0x000000ff ) / (float)0xff ) alpha:1.0f];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(VCLChildView *)pVCLChildView performSelectorOnMainThread:@selector(setBackgroundColor:) withObject:pColor waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void VCLChildView_setBounds( id pVCLChildView, long nX, long nY, long nWidth, long nHeight )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pVCLChildView )
	{
		if ( nWidth < 1 )
			nWidth = 1;
		if ( nHeight < 1 )
			nHeight = 1;
		NSValue *pValue = [NSValue valueWithRect:NSMakeRect( nX, nY, nWidth, nHeight )];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(VCLChildView *)pVCLChildView performSelectorOnMainThread:@selector(setBounds:) withObject:pValue waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void VCLChildView_setClip( id pVCLChildView, long nX, long nY, long nWidth, long nHeight )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pVCLChildView )
	{
		if ( nWidth < 0 )
			nWidth = 0;
		if ( nHeight < 0 )
			nHeight = 0;
		NSValue *pValue = [NSValue valueWithRect:NSMakeRect( nX, nY, nWidth, nHeight )];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(VCLChildView *)pVCLChildView performSelectorOnMainThread:@selector(setClip:) withObject:pValue waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void VCLChildView_show( id pVCLChildView, id pParentNSWindow, BOOL bShow )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pVCLChildView )
	{
		ShowVCLChildView *pShowVCLChildView = [ShowVCLChildView childWithView:pVCLChildView parentWindow:pParentNSWindow];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pShowVCLChildView performSelectorOnMainThread:@selector(show:) withObject:pShowVCLChildView waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}
