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

@interface CWindowDelegate : NSObject
{
	id						mpDelegate;
	NSWindow*				mpWindow;
}
- (void)dealloc;
- (id)delegate;
- (id)initWithWindow:(NSWindow *)pWindow delegate:(id)pDelegate;
- (NSRect)window:(NSWindow *)pWindow willPositionSheet:(NSWindow *)pSheet usingRect:(NSRect)aRect;
- (void)windowDidBecomeKey:(NSNotification *)pNotification;
- (void)windowDidBecomeMain:(NSNotification *)pNotification;
- (void)windowDidChangeScreen:(NSNotification *)pNotification;
- (void)windowDidChangeScreenProfile:(NSNotification *)pNotification;
- (void)windowDidDeminiaturize:(NSNotification *)pNotification;
- (void)windowDidEndSheet:(NSNotification *)pNotification;
- (void)windowDidExpose:(NSNotification *)pNotification;
- (void)windowDidMiniaturize:(NSNotification *)pNotification;
- (void)windowDidMove:(NSNotification *)pNotification;
- (void)windowDidResignKey:(NSNotification *)pNotification;
- (void)windowDidResignMain:(NSNotification *)pNotification;
- (void)windowDidResize:(NSNotification *)pNotification;
- (void)windowDidUpdate:(NSNotification *)pNotification;
- (BOOL)windowShouldClose:(id)pSender;
- (BOOL)windowShouldZoom:(NSWindow *)pWindow toFrame:(NSRect)aRect;
- (void)windowWillBeginSheet:(NSNotification *)pNotification;
- (void)windowWillClose:(NSNotification *)pNotification;
- (void)windowWillMiniaturize:(NSNotification *)pNotification;
- (void)windowWillMove:(NSNotification *)pNotification;
- (NSSize)windowWillResize:(NSWindow *)pWindow toSize:(NSSize)aSize;
- (NSRect)windowWillUseStandardFrame:(NSWindow *)pWindow defaultFrame:(NSRect)aRect;
- (id)windowWillReturnFieldEditor:(NSWindow *)pWindow toObject:(id)pObject;
- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)pWindow;
@end

@implementation CWindowDelegate

- (void)dealloc
{
	if ( mpDelegate )
		[mpDelegate release];

	if ( mpWindow )
		[mpWindow release];
}

- (id)delegate
{
	return mpDelegate;
}

- (id)initWithWindow:(NSWindow *)pWindow delegate:(id)pDelegate
{
	[super init];

	if ( pDelegate )
		mpDelegate = [pDelegate retain];

	if ( pWindow )
		mpWindow = [pWindow retain];

	return self;
}

- (NSRect)window:(NSWindow *)pWindow willPositionSheet:(NSWindow *)pSheet usingRect:(NSRect)aRect
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(window:willPositionSheet:usingRect:)] )
		return [mpDelegate window:pWindow willPositionSheet:pSheet usingRect:aRect];
	else
		return aRect;
}

- (void)windowDidBecomeKey:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowDidBecomeKey:)] )
		[mpDelegate windowDidBecomeKey:pNotification];
}

- (void)windowDidBecomeMain:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowDidBecomeMain:)] )
		[mpDelegate windowDidBecomeMain:pNotification];
}

- (void)windowDidChangeScreen:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowDidChangeScreen:)] )
		[mpDelegate windowDidChangeScreen:pNotification];
}

- (void)windowDidChangeScreenProfile:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowDidChangeScreenProfile:)] )
		[mpDelegate windowDidChangeScreenProfile:pNotification];
}

- (void)windowDidDeminiaturize:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowDidDeminiaturize:)] )
		[mpDelegate windowDidDeminiaturize:pNotification];
}

- (void)windowDidEndSheet:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowDidEndSheet:)] )
		[mpDelegate windowDidEndSheet:pNotification];
}

- (void)windowDidExpose:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowDidExpose:)] )
		[mpDelegate windowDidExpose:pNotification];
}

- (void)windowDidMiniaturize:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowDidMiniaturize:)] )
		[mpDelegate windowDidMiniaturize:pNotification];
}

- (void)windowDidMove:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowDidMove:)] )
		[mpDelegate windowDidMove:pNotification];
}

- (void)windowDidResignKey:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowDidResignKey:)] )
		[mpDelegate windowDidResignKey:pNotification];
}

- (void)windowDidResignMain:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowDidResignMain:)] )
		[mpDelegate windowDidResignMain:pNotification];
}

- (void)windowDidResize:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowDidResize:)] )
		[mpDelegate windowDidResize:pNotification];
}

- (void)windowDidUpdate:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowDidUpdate:)] )
		[mpDelegate windowDidUpdate:pNotification];
}

- (BOOL)windowShouldClose:(id)pSender
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowShouldClose:)] )
		return [mpDelegate windowShouldClose:pSender];
	else
		return YES;
}

- (BOOL)windowShouldZoom:(NSWindow *)pWindow toFrame:(NSRect)aRect
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowShouldZoom:toFront:)] )
		return [mpDelegate windowShouldZoom:pWindow toFront:aRect];
	else
		return YES;
}

- (void)windowWillBeginSheet:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowWillBeginSheet:)] )
		[mpDelegate windowWillBeginSheet:pNotification];
}

- (void)windowWillClose:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowWillClose:)] )
		[mpDelegate windowWillClose:pNotification];
}

- (void)windowWillMiniaturize:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowWillMiniaturize:)] )
		[mpDelegate windowWillMiniaturize:pNotification];
}

- (void)windowWillMove:(NSNotification *)pNotification
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowWillMove:)] )
		[mpDelegate windowWillMove:pNotification];
}

- (NSSize)windowWillResize:(NSWindow *)pWindow toSize:(NSSize)aSize
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowWillResize:toSize:)] )
		return [mpDelegate windowWillResize:pWindow toSize:aSize];
	else
		return aSize;
}

- (id)windowWillReturnFieldEditor:(NSWindow *)pWindow toObject:(id)pObject
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowWillReturnFieldEditor:toObject:)] )
		return [mpDelegate windowWillReturnFieldEditor:pWindow toObject:pObject];
	else
		return nil;
}

- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)pWindow
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowWillReturnUndoManager:)] )
		return [mpDelegate windowWillReturnUndoManager:pWindow];
	else
		return nil;
}

- (NSRect)windowWillUseStandardFrame:(NSWindow *)pWindow defaultFrame:(NSRect)aRect
{
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowWillUseStandardFrame:defaultFrame:)] )
		return [mpDelegate windowWillUseStandardFrame:pWindow defaultFrame:aRect];
	else
		return aRect;
}

@end

static EventLoopTimerUPP pCWindow_disposeDelegateTimerUPP = NULL;

static void CWindow_disposeDelegateTimerCallback( EventLoopTimerRef aTimer, void *pData )
{
	NSWindow *pNSWindow = (NSWindow *)pData;
	if ( pNSWindow )
	{
		CWindowDelegate *pDelegate = [pNSWindow delegate];
		if ( pDelegate )
		{
			[pNSWindow setDelegate:nil];
			[pDelegate release];
		}
	}
}

void CWindow_disposeDelegate( id pCWindow )
{
	if ( !pCWindow_disposeDelegateTimerUPP )
		pCWindow_disposeDelegateTimerUPP = NewEventLoopTimerUPP( CWindow_disposeDelegateTimerCallback );
	if ( pCWindow_disposeDelegateTimerUPP )
	{
		NSWindow *pNSWindow = (NSWindow *)CWindow_getNSWindow( pCWindow );
		if ( pNSWindow )
		{
			if ( GetCurrentEventLoop() != GetMainEventLoop() )
				InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pCWindow_disposeDelegateTimerUPP, (void *)pNSWindow, NULL );
			else
				CWindow_disposeDelegateTimerCallback( NULL, (void *)pNSWindow);
		}
	}
}

id CWindow_getNSWindow( id pCWindow )
{
	if ( pCWindow && [pCWindow respondsToSelector:@selector(getNSWindow)] )
		return [pCWindow getNSWindow];
	else
		return nil;
}

void CWindow_initDelegate( id pCWindow )
{
	NSWindow *pNSWindow = (NSWindow *)CWindow_getNSWindow( pCWindow );
	if ( pNSWindow )
	{
		CWindowDelegate *pNewDelegate = [[CWindowDelegate alloc] initWithWindow:pNSWindow delegate:[pNSWindow delegate]];
		[pNSWindow setDelegate:pNewDelegate];
	}
}

WindowRef CWindow_windowRef( id pCWindow )
{
	WindowRef aWindow = NULL;

	NSWindow *pNSWindow = (NSWindow *)CWindow_getNSWindow( pCWindow );
	if ( pNSWindow )
		aWindow = [pNSWindow windowRef];

	return aWindow;
}
