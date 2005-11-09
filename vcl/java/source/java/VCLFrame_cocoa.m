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
#import "VCLFrame_cocoa.h"

@interface ToFront : NSObject
{
	id					mpCWindow;
}
- (void)toFront:(id)pObject;
- (id)initWithCWindow:(id)pCWindow;
@end

@implementation ToFront

- (void)toFront:(id)pObject
{
	if ( [mpCWindow respondsToSelector:@selector(getNSWindow)] )
	{
		NSWindow *pWindow = (NSWindow *)[mpCWindow getNSWindow];
		if ( pWindow && [pWindow isVisible] )
			[pWindow orderFront:self];
	}
}

- (id)initWithCWindow:(id)pCWindow
{
	[super init];

	mpCWindow = pCWindow;

	return self;
}

@end

@interface GetNSWindow : NSObject
{
	id					mpCWindow;
	NSWindow*			mpWindow;
}
- (void)getNSWindow:(id)pObject;
- (id)initWithCWindow:(id)pCWindow;
- (NSWindow *)window;
@end

@implementation GetNSWindow

- (void)getNSWindow:(id)pObject
{
	if ( [mpCWindow respondsToSelector:@selector(getNSWindow)] )
		mpWindow = (NSWindow *)[mpCWindow getNSWindow];
}

- (id)initWithCWindow:(id)pCWindow
{
	[super init];

	mpCWindow = pCWindow;
	mpWindow = nil;

	return self;
}

- (NSWindow *)window
{
	return mpWindow;
}

@end

@interface GetWindowRef : NSObject
{
	id					mpCWindow;
	WindowRef			maWindow;
}
- (void)getWindowRef:(id)pObject;
- (id)initWithCWindow:(id)pCWindow;
- (WindowRef)windowRef;
@end

@implementation GetWindowRef

- (void)getWindowRef:(id)pObject
{
	if ( [mpCWindow respondsToSelector:@selector(getNSWindow)] )
	{
		NSWindow *pWindow = (NSWindow *)[mpCWindow getNSWindow];
		if ( pWindow )
			maWindow = [pWindow windowRef];
	}
}

- (id)initWithCWindow:(id)pCWindow
{
	[super init];

	mpCWindow = pCWindow;
	maWindow = nil;

	return self;
}

- (WindowRef)windowRef
{
	return maWindow;
}

@end

void CWindow_toFront( id pCWindow )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pCWindow )
	{
		ToFront *pToFront = [[ToFront alloc] initWithCWindow:pCWindow];
		[pToFront performSelectorOnMainThread:@selector(toFront:) withObject:pToFront waitUntilDone:YES];
	}

	[pPool release];
}

id CWindow_getNSWindow( id pCWindow )
{
	NSWindow *pNSWindow = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pCWindow )
	{
		GetNSWindow *pGetNSWindow = [[GetNSWindow alloc] initWithCWindow:pCWindow];
		[pGetNSWindow performSelectorOnMainThread:@selector(getNSWindow:) withObject:pGetNSWindow waitUntilDone:YES];
		pNSWindow = [pGetNSWindow window];
	}

	[pPool release];

	return pNSWindow;
}

WindowRef CWindow_getWindowRef( id pCWindow )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	WindowRef aWindow = nil;

	if ( pCWindow )
	{
		GetWindowRef *pGetWindowRef = [[GetWindowRef alloc] initWithCWindow:pCWindow];
		[pGetWindowRef performSelectorOnMainThread:@selector(getWindowRef:) withObject:pGetWindowRef waitUntilDone:YES];
		aWindow = [pGetWindowRef windowRef];
	}

	[pPool release];

	return aWindow;
}
