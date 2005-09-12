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
#import "VCLPrintJob_cocoa.h"

@interface RunNativeTimers : NSObject
- (void)runNativeTimers:(id)pObject;
@end

@implementation RunNativeTimers

- (void)runNativeTimers:(id)pObject
{
	NSView *pFocusView = [NSView focusView];
	if ( pFocusView )
		[pFocusView unlockFocus];

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSArray *pWindows = [pApp windows];
		if ( pWindows )
		{
			NSEnumerator *pEnum = [pWindows objectEnumerator];
			NSWindow *pWindow;
			while ( ( pWindow = [pEnum nextObject] ) != nil )
				[pWindow setAutodisplay:NO];
		}

		NSEvent *pEvent;
		while ( ( pEvent = [pApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate date] inMode:NSDefaultRunLoopMode dequeue:YES] ) != nil )
			[pApp sendEvent:pEvent];

		pWindows = [pApp windows];
		if ( pWindows )
		{
			NSEnumerator *pEnum = [pWindows objectEnumerator];
			NSWindow *pWindow;
			while ( ( pWindow = [pEnum nextObject] ) != nil )
				[pWindow setAutodisplay:YES];
		}
	}

	if ( pFocusView )
		[pFocusView lockFocus];
}

@end

void NSPrintOperation_runNativeTimers()
{
	RunNativeTimers *pRunNativeTimers = [[RunNativeTimers alloc] init];
	[pRunNativeTimers performSelectorOnMainThread:@selector(runNativeTimers:) withObject:pRunNativeTimers waitUntilDone:YES];
	[pRunNativeTimers release];
}

BOOL NSPrintInfo_pageRange( id pNSPrintInfo, int *nFirst, int *nLast )
{
	if ( pNSPrintInfo && nFirst && nLast )
	{
		NSMutableDictionary *pDictionary = [(NSPrintInfo *)pNSPrintInfo dictionary];
		if ( pDictionary )
		{
			NSNumber *pNumber = [pDictionary objectForKey:NSPrintAllPages];
			if ( !pNumber || ![pNumber boolValue] )
			{
				NSNumber *pFirst = [pDictionary objectForKey:NSPrintFirstPage];
				NSNumber *pLast = [pDictionary objectForKey:NSPrintLastPage];
				if ( pFirst )
				{
					*nFirst = [pFirst intValue];
					*nLast = [pLast intValue];
					if ( nFirst > 0 && nLast > nFirst )
						return YES;
				}
			}
		}
	}

	return NO;
}
