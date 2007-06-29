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
 *  Edward Peterlin, April 2007
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
#import "objmisc_cocoa.h"

@interface DoSetModified : NSObject
{
	WindowRef theRef;
	BOOL theState;
}
- (id)initWithState:(BOOL)state winRef:(WindowRef)r;
- (void)setModified:(id)pObject;
@end

@implementation DoSetModified
- (id)initWithState:(BOOL)state winRef:(WindowRef)r
{
	[super init];

	theRef=r;
	theState=state;

	return(self);
}

- (void)setModified:(id)ignore
{
	NSEnumerator *windowIter=[[NSApp windows] objectEnumerator];
	NSWindow *theWin;
	while((theWin=(NSWindow *)[windowIter nextObject])!=nil)
	{
		if([theWin windowRef]==theRef)
		{
			[theWin setDocumentEdited: theState];
			break;
		}
	}
}
@end

/**
 * Perform a SetWindowModified on a WindowRef that has been extracted from
 * a Cocoa window.
 */
void DoCocoaSetWindowModifiedBit( unsigned long winRef, bool isModified )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( winRef )
	{
		DoSetModified *pDoSetModified = [[DoSetModified alloc] initWithState:((isModified) ? YES : NO) winRef:(WindowRef)winRef];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pDoSetModified performSelectorOnMainThread:@selector(setModified:) withObject:pDoSetModified waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}
