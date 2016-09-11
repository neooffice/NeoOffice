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
#undef check

#include <saldata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vos/mutex.hxx>

#include "salinst_cocoa.h"

@interface GetModalWindow : NSObject
{
	NSWindow*			mpModalWindow;
}
+ (id)create;
- (void)getModalWindow:(id)pObject;
- (id)init;
- (NSWindow *)modalWindow;
@end

@implementation GetModalWindow

+ (id)create
{
	GetModalWindow *pRet = [[GetModalWindow alloc] init];
	[pRet autorelease];
	return pRet;
}

- (void)getModalWindow:(id)pObject
{
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		mpModalWindow = [pApp modalWindow];
		if ( !mpModalWindow )
		{
			NSArray *pWindows = [pApp windows];
			if ( pWindows )
			{
				NSUInteger nCount = [pWindows count];
				NSUInteger i = 0;
				for ( ; i < nCount ; i++ )
				{
					NSWindow *pWindow = [pWindows objectAtIndex:i];
					if ( [pWindow isSheet] && [pWindow isVisible] )
					{
						mpModalWindow = pWindow;
						break;
					}
				}
			}
		}
	}
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

void NSApplication_dispatchPendingEvents( BOOL bInNativeDrag, BOOL bWait )
{
	// Do not dispatch any native events in a native drag session as it causes
	// the [NSView dragImage:at:offset:event:pasteboard:source:slideBack:]
	// selector to never return
	if ( bInNativeDrag || CFRunLoopGetCurrent() != CFRunLoopGetMain() )
		return;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		// Fix excessive CPU usage when this is called from by adding a slight
		// wait if there are no pending events
		NSDate *pDate = [NSDate date];
		if ( pDate && bWait )
			pDate = [NSDate dateWithTimeInterval:0.05f sinceDate:pDate];

		NSEvent *pEvent;
		while ( ( pEvent = [pApp nextEventMatchingMask:NSAnyEventMask untilDate:pDate inMode:( [pApp modalWindow] ? NSModalPanelRunLoopMode : NSDefaultRunLoopMode ) dequeue:YES] ) != nil )
			[pApp sendEvent:pEvent];
	}

	[pPool release];
}

id NSApplication_getModalWindow()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSWindow *pModalWindow = nil;

	GetModalWindow *pGetModalWindow = [GetModalWindow create];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pGetModalWindow performSelectorOnMainThread:@selector(getModalWindow:) withObject:pGetModalWindow waitUntilDone:YES modes:pModes];
	pModalWindow = [pGetModalWindow modalWindow];

	[pPool release];

	return pModalWindow;
}

sal_Bool Application_beginModalSheet( id *pNSWindowForSheet )
{
	SalData *pSalData = GetSalData();

	// Do not allow more than one window to display a modal sheet
	if ( pSalData->mbInNativeModalSheet || !pNSWindowForSheet )
		return false;

	JavaSalFrame *pFocusFrame = NULL;

	// Get the active document window
	Window *pWindow = Application::GetActiveTopWindow();
	if ( pWindow )
		pFocusFrame = (JavaSalFrame *)pWindow->ImplGetFrame();

	if ( !pFocusFrame )
		pFocusFrame = pSalData->mpFocusFrame;

	// Fix bug 3294 by not attaching to utility windows
	while ( pFocusFrame && ( pFocusFrame->IsFloatingFrame() || pFocusFrame->IsUtilityWindow() || pFocusFrame->mbShowOnlyMenus ) )
		pFocusFrame = pFocusFrame->mpParent;

	// Fix bug 1106. If the focus frame is not set or is not visible, find the
	// first visible non-floating, non-utility frame.
	if ( !pFocusFrame || !pFocusFrame->mbVisible )
	{
		pFocusFrame = NULL;
		for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
		{
			if ( (*it)->mbVisible && !(*it)->IsFloatingFrame() && !(*it)->IsUtilityWindow() && !(*it)->mbShowOnlyMenus )
			{
				pFocusFrame = *it;
				break;
			}
		}
	}

	pSalData->mbInNativeModalSheet = true;
	pSalData->mpNativeModalSheetFrame = pFocusFrame;

	if ( pFocusFrame )
	{
		pSalData->mpNativeModalSheetFrame = pFocusFrame;
		*pNSWindowForSheet = pFocusFrame->GetNativeWindow();
	}
	else
	{
		*pNSWindowForSheet = nil;
	}

	return sal_True;
}

void Application_endModalSheet()
{
	SalData *pSalData = GetSalData();
	pSalData->mbInNativeModalSheet = false;
	pSalData->mpNativeModalSheetFrame = NULL;
}

void Application_postWakeUpEvent()
{
	if ( !Application::IsShutDown() )
	{
		JavaSalEvent *pUserEvent = new JavaSalEvent( SALEVENT_WAKEUP, NULL, NULL );
		JavaSalEventQueue::postCachedEvent( pUserEvent );
		pUserEvent->release();
	}
}
