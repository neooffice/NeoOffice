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
 *  Edward Peterlin, February 2008
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2008 Planamesa Inc.
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
#import "topfrm_cocoa.h"

@interface DoSetRepresentedFilename : NSObject
{
	NSString *thePath;
	WindowRef theRef;
}
+ (id)createWithPath:(NSString *)path winRef:(WindowRef)r;
- (id)initWithPath:(NSString *)path winRef:(WindowRef)r;
- (void)setFilename:(id)pObject;
@end

@implementation DoSetRepresentedFilename

+ (id)createWithPath:(NSString *)path winRef:(WindowRef)r
{
	DoSetRepresentedFilename *pRet = [[DoSetRepresentedFilename alloc] initWithPath:path winRef:r];
	[pRet autorelease];
	return pRet;
}

- (id)initWithPath:(NSString *)path winRef:(WindowRef)r
{
	[super init];

	theRef=r;
	thePath=path;

	return(self);
}

- (void)setFilename:(id)ignore
{
	NSEnumerator *windowIter=[[NSApp windows] objectEnumerator];
	NSWindow *theWin;
	while((theWin=(NSWindow *)[windowIter nextObject])!=nil)
	{
		if([theWin windowRef]==theRef)
		{
			[theWin setRepresentedFilename: thePath];
			break;
		}
	}
}

@end

/**
 * Set the represented filename on a WindowRef that has been extracted from
 * a Cocoa window.
 */
void DoCocoaSetRepresentedFilename( unsigned long winRef, CFStringRef path )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( winRef )
	{
		DoSetRepresentedFilename *pDoSetRepresentedFilename = [DoSetRepresentedFilename createWithPath:(NSString *)path winRef:(WindowRef)winRef];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pDoSetRepresentedFilename performSelectorOnMainThread:@selector(setFilename:) withObject:pDoSetRepresentedFilename waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}
