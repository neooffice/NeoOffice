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
	BOOL					mbAttachChildren;
	NSMutableSet*			mpChildWindows;
	id						mpDelegate;
	NSWindow*				mpWindow;
}
- (void)addChildWindow:(NSWindow *)pWindow;
- (void)attachWindows;
- (void)dealloc;
- (id)delegate;
- (id)initWithWindow:(NSWindow *)pWindow delegate:(id)pDelegate;
- (void)removeChildWindow:(NSWindow *)pWindow;
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

- (void)addChildWindow:(NSWindow *)pWindow
{
	if ( mpWindow && pWindow )
	{
		[mpChildWindows addObject:pWindow];
		mbAttachChildren = TRUE;
		[self attachWindows];
	}
}

- (void)attachWindows
{
	if ( mbAttachChildren && mpWindow && [mpWindow isVisible])
	{
		NSWindow *pWindow;
		NSEnumerator *pEnum = [mpChildWindows objectEnumerator];
		while ( ( pWindow = [pEnum nextObject] ) != nil )
		{
			if ( [pWindow isVisible] )
			{
				NSWindow *pParent = [pWindow parentWindow];
				if ( pParent != mpWindow )
				{
					if ( pParent )
						[pParent removeChildWindow:pWindow];
					[mpWindow addChildWindow:pWindow ordered:NSWindowAbove];
				}
			}
		}

		mbAttachChildren = FALSE;
	}
}

- (void)dealloc
{
	if ( mpChildWindows )
		[mpChildWindows release];

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

	mbAttachChildren = FALSE;
	mpChildWindows = [[NSMutableSet alloc] init];

	if ( pDelegate )
		mpDelegate = [pDelegate retain];

	if ( pWindow )
		mpWindow = [pWindow retain];

	return self;
}

- (void)removeChildWindow:(NSWindow *)pWindow
{
	if ( pWindow )
	{
		NSWindow *pParent = [pWindow parentWindow];
		if ( pParent == mpWindow )
			[mpWindow removeChildWindow:pWindow];

		[mpChildWindows removeObject:pWindow];
	}
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
	[self attachWindows];

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
	if ( mpDelegate && [mpDelegate respondsToSelector:@selector(windowShouldZoom:toFrame:)] )
		return [mpDelegate windowShouldZoom:pWindow toFrame:aRect];
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
	if ( mpWindow )
	{
		// Detach all child windows
		NSWindow *pWindow;
		NSEnumerator *pEnum = [mpChildWindows objectEnumerator];
		while ( ( pWindow = [pEnum nextObject] ) != nil )
		{
			NSWindow *pParent = [pWindow parentWindow];
			if ( pParent == mpWindow )
				[mpWindow removeChildWindow:pWindow];
		}

		// Detach this window from parent
		NSWindow *pParent = [mpWindow parentWindow];
		if ( pParent )
			[pParent removeChildWindow:mpWindow];

		mbAttachChildren = FALSE;
	}

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSWindow *pKeyWindow = [pApp keyWindow];
		if ( pKeyWindow && pKeyWindow != [pApp mainWindow] )
			[pKeyWindow makeMainWindow];
	}

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

struct CWindow_addChildWindowTimerParams
{
	NSWindow*				mpWindow;
	NSWindow*				mpChildWindow;
};

struct CWindow_removeChildWindowTimerParams
{
	NSWindow*				mpWindow;
	NSWindow*				mpChildWindow;
};

static EventLoopTimerUPP pCWindow_addChildWindowTimerUPP = NULL;
static EventLoopTimerUPP pCWindow_disposeDelegateTimerUPP = NULL;
static EventLoopTimerUPP pCWindow_removeChildWindowTimerUPP = NULL;

static void CWindow_addChildWindowTimerCallback( EventLoopTimerRef aTimer, void *pData )
{
	struct CWindow_addChildWindowTimerParams *pParams = (struct CWindow_addChildWindowTimerParams *)pData;
	if ( pParams )
	{
		if ( pParams->mpWindow && pParams->mpChildWindow )
		{
			CWindowDelegate *pDelegate = [pParams->mpWindow delegate];
			if ( pDelegate )
				[pDelegate addChildWindow:pParams->mpChildWindow];
		}

		rtl_freeMemory( pParams );
	}
}

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

static void CWindow_removeChildWindowTimerCallback( EventLoopTimerRef aTimer, void *pData )
{
	struct CWindow_removeChildWindowTimerParams *pParams = (struct CWindow_removeChildWindowTimerParams *)pData;
	if ( pParams )
	{
		if ( pParams->mpWindow && pParams->mpChildWindow )
		{
			CWindowDelegate *pDelegate = [pParams->mpWindow delegate];
			if ( pDelegate )
				[pDelegate removeChildWindow:pParams->mpChildWindow];
		}

		rtl_freeMemory( pParams );
	}
}

void CWindow_addChildWindow( id pCWindow, id pChildCWindow )
{
	if ( !pCWindow_addChildWindowTimerUPP )
		pCWindow_addChildWindowTimerUPP = NewEventLoopTimerUPP( CWindow_addChildWindowTimerCallback );
	if ( pCWindow_addChildWindowTimerUPP )
	{
		NSWindow *pNSWindow = (NSWindow *)CWindow_getNSWindow( pCWindow );
		NSWindow *pChildNSWindow = (NSWindow *)CWindow_getNSWindow( pChildCWindow );
		if ( pNSWindow && pChildNSWindow )
		{
			struct CWindow_addChildWindowTimerParams *pParams = (struct CWindow_addChildWindowTimerParams *)rtl_allocateMemory( sizeof( struct CWindow_addChildWindowTimerParams ) );
			pParams->mpWindow = pNSWindow;
			pParams->mpChildWindow = pChildNSWindow;

			if ( GetCurrentEventLoop() != GetMainEventLoop() )
				InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pCWindow_addChildWindowTimerUPP, (void *)pParams, NULL );
			else
				CWindow_addChildWindowTimerCallback( NULL, (void *)pParams );
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

void CWindow_removeChildWindow( id pCWindow, id pChildCWindow )
{
	if ( !pCWindow_removeChildWindowTimerUPP )
		pCWindow_removeChildWindowTimerUPP = NewEventLoopTimerUPP( CWindow_removeChildWindowTimerCallback );
	if ( pCWindow_removeChildWindowTimerUPP )
	{
		NSWindow *pNSWindow = (NSWindow *)CWindow_getNSWindow( pCWindow );
		NSWindow *pChildNSWindow = (NSWindow *)CWindow_getNSWindow( pChildCWindow );
		if ( pNSWindow && pChildNSWindow )
		{
			struct CWindow_removeChildWindowTimerParams *pParams = (struct CWindow_removeChildWindowTimerParams *)rtl_allocateMemory( sizeof( struct CWindow_removeChildWindowTimerParams ) );
			pParams->mpWindow = pNSWindow;
			pParams->mpChildWindow = pChildNSWindow;

			if ( GetCurrentEventLoop() != GetMainEventLoop() )
				InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pCWindow_removeChildWindowTimerUPP, (void *)pParams, NULL );
			else
				CWindow_removeChildWindowTimerCallback( NULL, (void *)pParams );
		}
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
