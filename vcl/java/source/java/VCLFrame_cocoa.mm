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
 *  Patrick Luby, July 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 Planamesa Inc.
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

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include "VCLEventQueue_cocoa.h"
#include "VCLFrame_cocoa.h"

#ifndef USE_NATIVE_EVENTS

#ifndef NSFullScreenWindowMask
#define NSFullScreenWindowMask ( 1 << 14 )
#endif

#ifndef NSWindowCollectionBehaviorFullScreenPrimary
#define NSWindowCollectionBehaviorFullScreenPrimary ( 1 << 7 )
#endif

@interface NSObject (CWindow)
- (NSWindow *)getNSWindow;
@end

@interface NSWindow (VCLWindow)
- (NSRect)_frameOnExitFromFullScreen;
- (void)_setModalWindowLevel;
- (BOOL)inLiveResize;
@end

@interface GetNSWindow : NSObject
{
	id					mpCWindow;
	NSRect				maFrame;
	BOOL				mbFullScreen;
	BOOL				mbInLiveResize;
	BOOL				mbTopLevelWindow;
	NSView*				mpView;
	NSWindow*			mpWindow;
}
+ (id)createWithCWindow:(id)pCWindow fullScreen:(BOOL)bFullScreen topLevelWindow:(BOOL)bTopLevelWindow;
- (NSView *)contentView;
- (NSRect)frame;
- (void)getNSWindow:(id)pObject;
- (id)initWithCWindow:(id)pCWindow fullScreen:(BOOL)bFullScreen topLevelWindow:(BOOL)bTopLevelWindow;
- (BOOL)inLiveResize;
- (NSWindow *)window;
@end

@implementation GetNSWindow

+ (id)createWithCWindow:(id)pCWindow fullScreen:(BOOL)bFullScreen topLevelWindow:(BOOL)bTopLevelWindow
{
	GetNSWindow *pRet = [[GetNSWindow alloc] initWithCWindow:pCWindow fullScreen:bFullScreen topLevelWindow:bTopLevelWindow];
	[pRet autorelease];
	return pRet;
}

- (NSView *)contentView
{
	return mpView;
}

- (NSRect)frame
{
	return maFrame;
}

- (void)getNSWindow:(id)pObject
{
	if ( [mpCWindow respondsToSelector:@selector(getNSWindow)] )
	{
		mpWindow = (NSWindow *)[mpCWindow getNSWindow];
		if ( mpWindow )
		{
			// Get flipped coordinates
			maFrame = [mpWindow frame];

#ifdef USE_NATIVE_FULL_SCREEN_MODE
			// Check if we are in full screen mode
			if ( [mpWindow styleMask] & NSFullScreenWindowMask && [mpWindow respondsToSelector:@selector(_frameOnExitFromFullScreen)] )
			{
				if ( mbFullScreen )
				{
					NSRect aFrame = [mpWindow _frameOnExitFromFullScreen];
					if ( !NSIsEmptyRect( aFrame ) )
						maFrame = aFrame;
				}
				else
				{
					maFrame = [NSWindow frameRectForContentRect:maFrame styleMask:[mpWindow styleMask] & ~NSFullScreenWindowMask];
				}
			}
#endif	// USE_NATIVE_FULL_SCREEN_MODE

			NSRect aRect = NSMakeRect( 0, 0, 0, 0 );
			NSArray *pScreens = [NSScreen screens];
			if ( pScreens )
			{
				NSUInteger i = 0;
				NSUInteger nCount = [pScreens count];
				for ( ; i < nCount; i++ )
				{
					NSScreen *pScreen = (NSScreen *)[pScreens objectAtIndex:i];
					if ( pScreen )
						aRect = NSUnionRect( aRect, [pScreen frame] );
				}
			}
			maFrame.origin.y = aRect.origin.y + aRect.size.height - maFrame.origin.y - maFrame.size.height;

			if ( [mpWindow respondsToSelector:@selector(inLiveResize)] && [mpWindow inLiveResize] )
				mbInLiveResize = YES;
			else
				mbInLiveResize = NO;

			mpView = [mpWindow contentView];

#ifdef USE_NATIVE_FULL_SCREEN_MODE
			// Enable full screen feature for normal windows
			if ( mbTopLevelWindow )
				[mpWindow setCollectionBehavior:[mpWindow collectionBehavior] | NSWindowCollectionBehaviorFullScreenPrimary];
#endif	// USE_NATIVE_FULL_SCREEN_MODE
		}
	}
}

- (id)initWithCWindow:(id)pCWindow fullScreen:(BOOL)bFullScreen topLevelWindow:(BOOL)bTopLevelWindow
{
	[super init];

	mpCWindow = pCWindow;
	maFrame = NSMakeRect( 0, 0, 0, 0 );
	mbFullScreen = bFullScreen;
	mbInLiveResize = NO;
	mbTopLevelWindow = bTopLevelWindow;
	mpView = nil;
	mpWindow = nil;

	return self;
}

- (BOOL)inLiveResize
{
	return mbInLiveResize;
}

- (NSWindow *)window
{
	return mpWindow;
}

@end

@interface MakeFloatingWindow : NSObject
{
	id					mpCWindow;
	int					mnTopInset;
}
+ (id)createWithCWindow:(id)pCWindow;
- (int)getTopInset;
- (id)initWithCWindow:(id)pCWindow;
- (void)makeFloatingWindow:(id)pObject;
@end

@implementation MakeFloatingWindow

+ (id)createWithCWindow:(id)pCWindow
{
	MakeFloatingWindow *pRet = [[MakeFloatingWindow alloc] initWithCWindow:pCWindow];
	[pRet autorelease];
	return pRet;
}

- (int)getTopInset
{
	return mnTopInset;
}

- (id)initWithCWindow:(id)pCWindow
{
	[super init];

	mpCWindow = pCWindow;
	mnTopInset = 0;

	return self;
}

- (void)makeFloatingWindow:(id)pObject
{
	if ( [mpCWindow respondsToSelector:@selector(getNSWindow)] )
	{
		NSWindow *pWindow = (NSWindow *)[mpCWindow getNSWindow];
		if ( pWindow && ![pWindow isVisible] )
		{
			NSView *pContentView = [pWindow contentView];
			if ( pContentView )
			{
				NSView *pSuperview = [pContentView superview];
				if ( pSuperview && [pSuperview respondsToSelector:@selector(_setUtilityWindow:)] )
				{
					if ( [pWindow styleMask] & NSTitledWindowMask )
					{
						[pWindow setLevel:NSFloatingWindowLevel];
						[pWindow setHidesOnDeactivate:YES];
					}
					else
					{
						[pWindow setLevel:NSPopUpMenuWindowLevel];
					}

					// Get the top inset for a utility window
					NSRect aFrameRect = NSMakeRect( 0, 0, 100, 100 );
					NSRect aContentRect = [NSWindow contentRectForFrameRect:aFrameRect styleMask:[pWindow styleMask] | NSUtilityWindowMask];
					mnTopInset = aFrameRect.origin.y + aFrameRect.size.height - aContentRect.origin.y - aContentRect.size.height;
				}
			}
		}
	}
}

@end

@interface MakeModalWindow : NSObject
{
	id					mpCWindow;
}
+ (id)createWithCWindow:(id)pCWindow;
- (id)initWithCWindow:(id)pCWindow;
- (void)makeModalWindow:(id)pObject;
@end

@implementation MakeModalWindow

+ (id)createWithCWindow:(id)pCWindow
{
	MakeModalWindow *pRet = [[MakeModalWindow alloc] initWithCWindow:pCWindow];
	[pRet autorelease];
	return pRet;
}

- (id)initWithCWindow:(id)pCWindow
{
	[super init];

	mpCWindow = pCWindow;

	return self;
}

- (void)makeModalWindow:(id)pObject
{
	if ( [mpCWindow respondsToSelector:@selector(getNSWindow)] )
	{
		NSWindow *pWindow = (NSWindow *)[mpCWindow getNSWindow];
		if ( pWindow && [pWindow styleMask] & NSTitledWindowMask && [pWindow respondsToSelector:@selector(_setModalWindowLevel)] )
		{
			[pWindow _setModalWindowLevel];

			// Run VCLWindow selector to ensure that the window level is set
			// correctly if the application is not active
			[VCLWindow clearModalWindowLevel];
		}
	}
}

@end

@interface MakeUnshadowedWindow : NSObject
{
	id					mpCWindow;
}
+ (id)createWithCWindow:(id)pCWindow;
- (id)initWithCWindow:(id)pCWindow;
- (void)makeUnshadowedWindow:(id)pObject;
@end

@implementation MakeUnshadowedWindow

+ (id)createWithCWindow:(id)pCWindow
{
	MakeUnshadowedWindow *pRet = [[MakeUnshadowedWindow alloc] initWithCWindow:pCWindow];
	[pRet autorelease];
	return pRet;
}

- (id)initWithCWindow:(id)pCWindow
{
	[super init];

	mpCWindow = pCWindow;

	return self;
}

- (void)makeUnshadowedWindow:(id)pObject
{
	if ( [mpCWindow respondsToSelector:@selector(getNSWindow)] )
	{
		NSWindow *pWindow = (NSWindow *)[mpCWindow getNSWindow];
		if ( pWindow && !([pWindow styleMask] & NSTitledWindowMask))
			[pWindow setHasShadow:NO];
	}
}

@end

@interface UpdateLocation : NSObject
{
	id					mpCWindow;
}
+ (id)createWithCWindow:(id)pCWindow;
- (id)initWithCWindow:(id)pCWindow;
- (void)updateLocation:(id)pObject;
@end

@implementation UpdateLocation

+ (id)createWithCWindow:(id)pCWindow
{
	UpdateLocation *pRet = [[UpdateLocation alloc] initWithCWindow:pCWindow];
	[pRet autorelease];
	return pRet;
}

- (id)initWithCWindow:(id)pCWindow
{
	[super init];

	mpCWindow = pCWindow;

	return self;
}

- (void)updateLocation:(id)pObject
{
	if ( [mpCWindow respondsToSelector:@selector(getNSWindow)] )
	{
		NSWindow *pWindow = (NSWindow *)[mpCWindow getNSWindow];
		if ( pWindow )
		{
			NSRect aBounds = [pWindow frame];
			NSPoint aPoint = NSMakePoint( aBounds.origin.x + 1, aBounds.origin.y + 1 );
			[pWindow setFrameOrigin:aPoint];
			[pWindow setFrameOrigin:aBounds.origin];
		}
	}
}

@end

id CWindow_getNSWindow( id pCWindow )
{
	NSWindow *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pCWindow )
	{
		GetNSWindow *pGetNSWindow = [GetNSWindow createWithCWindow:pCWindow fullScreen:NO topLevelWindow:NO];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pGetNSWindow performSelectorOnMainThread:@selector(getNSWindow:) withObject:pGetNSWindow waitUntilDone:YES modes:pModes];
		pRet = [pGetNSWindow window];
	}

	[pPool release];

	return pRet;
}

void CWindow_getNSWindowBounds( id pCWindow, float *pX, float *pY, float *pWidth, float *pHeight, BOOL *pInLiveResize, BOOL bFullScreen )
{
	if ( pX )
		*pX = 0;
	if ( pY )
		*pY = 0;
	if ( pWidth )
		*pWidth = 0;
	if ( pHeight )
		*pHeight = 0;
	if ( pInLiveResize )
		*pInLiveResize = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pCWindow )
	{
		GetNSWindow *pGetNSWindow = [GetNSWindow createWithCWindow:pCWindow fullScreen:bFullScreen topLevelWindow:NO];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pGetNSWindow performSelectorOnMainThread:@selector(getNSWindow:) withObject:pGetNSWindow waitUntilDone:YES modes:pModes];
		NSRect aRect = [pGetNSWindow frame];
		if ( pX )
			*pX = aRect.origin.x;
		if ( pY )
			*pY = aRect.origin.y;
		if ( pWidth )
			*pWidth = aRect.size.width;
		if ( pHeight )
			*pHeight = aRect.size.height;
		if ( pInLiveResize )
			*pInLiveResize = [pGetNSWindow inLiveResize];
	}

	[pPool release];
}

id CWindow_getNSWindowContentView( id pCWindow, BOOL bTopLevelWindow )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSView *pRet = nil;

	if ( pCWindow )
	{
		GetNSWindow *pGetNSWindow = [GetNSWindow createWithCWindow:pCWindow fullScreen:NO topLevelWindow:bTopLevelWindow];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pGetNSWindow performSelectorOnMainThread:@selector(getNSWindow:) withObject:pGetNSWindow waitUntilDone:YES modes:pModes];
		pRet = [pGetNSWindow contentView];
	}

	[pPool release];

	return pRet;
}

int CWindow_makeFloatingWindow( id pCWindow )
{
	int nRet = 0;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pCWindow )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		MakeFloatingWindow *pMakeFloatingWindow = [MakeFloatingWindow createWithCWindow:pCWindow];
		[pMakeFloatingWindow performSelectorOnMainThread:@selector(makeFloatingWindow:) withObject:pMakeFloatingWindow waitUntilDone:YES modes:pModes];
		nRet = [pMakeFloatingWindow getTopInset];
	}

	[pPool release];

	return nRet;
}

void CWindow_makeModalWindow( id pCWindow )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pCWindow )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		MakeModalWindow *pMakeModalWindow = [MakeModalWindow createWithCWindow:pCWindow];
		[pMakeModalWindow performSelectorOnMainThread:@selector(makeModalWindow:) withObject:pMakeModalWindow waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void CWindow_makeUnshadowedWindow( id pCWindow )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pCWindow )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		MakeUnshadowedWindow *pMakeUnshadowedWindow = [MakeUnshadowedWindow createWithCWindow:pCWindow];
		[pMakeUnshadowedWindow performSelectorOnMainThread:@selector(makeUnshadowedWindow:) withObject:pMakeUnshadowedWindow waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

void CWindow_updateLocation( id pCWindow )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pCWindow )
	{
		UpdateLocation *pUpdateLocation = [UpdateLocation createWithCWindow:pCWindow];
		[pUpdateLocation performSelectorOnMainThread:@selector(updateLocation:) withObject:pUpdateLocation waitUntilDone:NO];
	}

	[pPool release];
}

#endif	// !USE_NATIVE_EVENTS
