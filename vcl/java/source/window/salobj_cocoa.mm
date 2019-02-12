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

#include <tools/gen.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include "salobj_cocoa.h"

@interface NSView (VCLChildView)
- (void)setFillColor:(NSColor *)pColor;
@end

@interface VCLChildView : NSView
{
	NSColor*				mpBackgroundColor;
	NSRect					maClipRect;
	NSView*					mpSuperview;
}
- (void)addSubview:(NSView *)pView;
- (void)addSubview:(NSView *)pView positioned:(NSWindowOrderingMode)nPlace relativeTo:(NSView *)pOtherView;
- (void)dealloc;
- (void)destroy:(id)pObject;
- (void)drawRect:(NSRect)aDirtyRect;
- (id)initWithFrame:(NSRect)aFrame;
- (BOOL)isOpaque;
- (void)setBackgroundColor:(NSColor *)pColor;
- (void)setBounds:(NSValue *)pValue;
- (void)setClip:(NSValue *)pValue;
- (void)setFillColorInSubviews;
- (void)viewWillStartLiveResize;
@end

@implementation VCLChildView

- (void)addSubview:(NSView *)pView
{
	[super addSubview:pView];

	[self setFillColorInSubviews];
}

- (void)addSubview:(NSView *)pView positioned:(NSWindowOrderingMode)nPlace relativeTo:(NSView *)pOtherView
{
	[super addSubview:pView positioned:nPlace relativeTo:pOtherView];

	[self setFillColorInSubviews];
}

- (void)dealloc
{
	if ( mpSuperview )
	{
		[mpSuperview removeFromSuperview];
		[mpSuperview release];
	}

	if ( mpBackgroundColor )
		[mpBackgroundColor release];

	[super dealloc];
}

- (void)destroy:(id)pObject
{
	(void)pObject;

	// Detach any children
	NSArray *pSubviews = [self subviews];
	if ( pSubviews )
	{
		// Make a temporary copy for iteration
		pSubviews = [NSArray arrayWithArray:pSubviews];
		if ( pSubviews )
		{
			unsigned int nCount = [pSubviews count];
			unsigned int i = 0;
			for ( ; i < nCount; i++ )
			{
				NSView *pSubview = static_cast< NSView* >( [pSubviews objectAtIndex:i] );
				if ( pSubview )
					[pSubview removeFromSuperview];
			}
		}
	}

	[self removeFromSuperview];
}

- (void)drawRect:(NSRect)aDirtyRect
{
	if ( mpBackgroundColor )
	{
		[mpBackgroundColor set];
		[NSBezierPath fillRect:aDirtyRect];
	}
}

- (id)initWithFrame:(NSRect)aFrame
{
	NSRect aChildFrame = NSMakeRect( 0, 0, aFrame.size.width, aFrame.size.height );
	[super initWithFrame:aChildFrame];

	mpBackgroundColor = [NSColor whiteColor];
	if ( mpBackgroundColor )
		[mpBackgroundColor retain];
	maClipRect = NSZeroRect;

	// Create a superview that we use to control clipping
	mpSuperview = [[NSView alloc] initWithFrame:aFrame];
	if ( mpSuperview )
		[mpSuperview addSubview:self positioned:NSWindowAbove relativeTo:nil];

	return self;
}

- (BOOL)isOpaque
{
	return ( mpBackgroundColor && [mpBackgroundColor alphaComponent] == 1.0f );
}

- (void)setBackgroundColor:(NSColor *)pColor
{
	if ( mpBackgroundColor )
	{
		[mpBackgroundColor release];
		mpBackgroundColor = nil;
	}

	if ( !pColor )
		pColor = [NSColor whiteColor];
	else if ( [pColor alphaComponent] != 1.0f )
		pColor = [pColor colorWithAlphaComponent:1.0f];

	if ( pColor )
	{
		mpBackgroundColor = pColor;
		[mpBackgroundColor retain];
		[self setFillColorInSubviews];
	}
}

- (void)setBounds:(NSValue *)pValue
{
	if ( pValue && mpSuperview )
	{
		NSRect aNewFrame = [pValue rectValue];
		if ( NSIsEmptyRect( aNewFrame ) )
			return;

		// Set superview to intersection of new frame and clip
		NSRect aNewParentFrame = NSMakeRect( 0, 0, aNewFrame.size.width, aNewFrame.size.height );
		if ( !NSIsEmptyRect( maClipRect ) )
			aNewParentFrame = NSIntersectionRect( aNewParentFrame, maClipRect );
		aNewParentFrame.origin.x += aNewFrame.origin.x;
		aNewParentFrame.origin.y += aNewFrame.origin.y;
		if ( NSIsEmptyRect( aNewParentFrame ) )
			return;

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

		NSRect aParentFrame = [mpSuperview frame];
		NSRect aFrame = [self frame];
		if ( !NSEqualRects( aNewParentFrame, aParentFrame ) || !NSEqualRects( aNewFrame, aFrame ) )
		{
			NSView *pSuperSuperview = [mpSuperview superview];
			if ( !NSEqualSizes( aNewParentFrame.size, aParentFrame.size ) )
				[mpSuperview removeFromSuperview];
			else
				pSuperSuperview = nil;

			if ( !NSEqualRects( aNewParentFrame, aParentFrame ) )
				[mpSuperview setFrame:aNewParentFrame];
			if ( !NSEqualRects( aNewFrame, aFrame ) )
				[self setFrame:aNewFrame];

			if ( pSuperSuperview )
				[pSuperSuperview addSubview:mpSuperview positioned:NSWindowAbove relativeTo:nil];
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

- (void)setFillColorInSubviews
{
	// Set the fill color for any QTMovieView subviews
	NSArray *pSubviews = [self subviews];
	if ( pSubviews )
	{
		unsigned int nCount = [pSubviews count];
		unsigned int i = 0;
		for ( ; i < nCount; i++ )
		{
			NSView *pSubview = [pSubviews objectAtIndex:i];
			if ( pSubview )
			{
				if ( [pSubview respondsToSelector:@selector(setFillColor:)] )
					[pSubview setFillColor:mpBackgroundColor];
			}
		}
	}
}

- (void)viewWillStartLiveResize
{
	[super viewWillStartLiveResize];

	[mpSuperview removeFromSuperview];
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
	(void)pObject;

	[mpView initWithFrame:NSMakeRect( 0, 0, 1, 1 )];
}

@end

@interface ShowVCLChildView : NSObject
{
	NSWindow*				mpParentWindow;
	BOOL					mbShow;
	VCLChildView*			mpView;
}
+ (id)childWithView:(VCLChildView *)pView parentWindow:(NSWindow *)pParentWindow show:(BOOL)bShow;
- (id)initWithView:(VCLChildView *)pView parentWindow:(NSWindow *)pParentWindow show:(BOOL)bShow;
- (void)show:(id)pObject;
@end

@implementation ShowVCLChildView

+ (id)childWithView:(VCLChildView *)pView parentWindow:(NSWindow *)pParentWindow show:(BOOL)bShow
{
	ShowVCLChildView *pRet = [[ShowVCLChildView alloc] initWithView:pView parentWindow:pParentWindow show:bShow];
	[pRet autorelease];
	return pRet;
}

- (id)initWithView:(VCLChildView *)pView parentWindow:(NSWindow *)pParentWindow show:(BOOL)bShow
{
	[super init];

	mpParentWindow = pParentWindow;
	mbShow = bShow;
	mpView = pView;

	return self;
}

- (void)show:(id)pObject
{
	(void)pObject;

	if ( mpView )
	{
		NSView *pSuperview = [mpView superview];
		if ( pSuperview )
		{
			NSView *pSuperSuperview = [pSuperview superview];

			if ( mbShow && mpParentWindow && [mpParentWindow isVisible] )
			{
				// Always attach to content view
				NSView *pContentView = [mpParentWindow contentView];
				if ( pContentView && pContentView != pSuperSuperview )
				{
					[pSuperview removeFromSuperview];

					if ( ![pContentView inLiveResize] )
						[pContentView addSubview:pSuperview positioned:NSWindowAbove relativeTo:nil];
				}
			}
			else
			{
				[pSuperview removeFromSuperview];
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
		[static_cast< VCLChildView* >( pVCLChildView ) performSelectorOnMainThread:@selector(destroy:) withObject:pVCLChildView waitUntilDone:NO modes:pModes];
		[pVCLChildView release];
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
		[static_cast< VCLChildView* >( pVCLChildView ) performSelectorOnMainThread:@selector(setBackgroundColor:) withObject:pColor waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

SAL_DLLPRIVATE void VCLChildView_setBounds( id pVCLChildView, NSRect aBounds )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pVCLChildView )
	{
		if ( aBounds.size.width < 0 )
			aBounds.size.width = 0;
		if ( aBounds.size.height < 0 )
			aBounds.size.height = 0;
		NSValue *pValue = [NSValue valueWithRect:aBounds];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< VCLChildView* >( pVCLChildView ) performSelectorOnMainThread:@selector(setBounds:) withObject:pValue waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

SAL_DLLPRIVATE void VCLChildView_setClip( id pVCLChildView, NSRect aClipRect )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pVCLChildView )
	{
		if ( aClipRect.size.width < 0 )
			aClipRect.size.width = 0;
		if ( aClipRect.size.height < 0 )
			aClipRect.size.height = 0;
		NSValue *pValue = [NSValue valueWithRect:aClipRect];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< VCLChildView* >( pVCLChildView ) performSelectorOnMainThread:@selector(setClip:) withObject:pValue waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void VCLChildView_show( id pVCLChildView, id pParentNSWindow, sal_Bool bShow )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pVCLChildView )
	{
		ShowVCLChildView *pShowVCLChildView = [ShowVCLChildView childWithView:pVCLChildView parentWindow:pParentNSWindow show:bShow];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pShowVCLChildView performSelectorOnMainThread:@selector(show:) withObject:pShowVCLChildView waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}
