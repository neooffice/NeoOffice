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
	NSView *theView;
}
+ (id)createWithPath:(NSString *)path view:(NSView *)r;
- (id)initWithPath:(NSString *)path view:(NSView *)r;
- (void)setFilename:(id)pObject;
@end

@implementation DoSetRepresentedFilename

+ (id)createWithPath:(NSString *)path view:(NSView *)r
{
	DoSetRepresentedFilename *pRet = [[DoSetRepresentedFilename alloc] initWithPath:path view:r];
	[pRet autorelease];
	return pRet;
}

- (id)initWithPath:(NSString *)path view:(NSView *)r
{
	[super init];

	theView=r;
	thePath=path;

	return(self);
}

- (void)setFilename:(id)ignore
{
	NSWindow *theWin = [theView window];
	if (theWin)
		[theWin setRepresentedFilename: thePath];
}

@end

/**
 * Set the represented filename on an NSView that has been extracted from
 * a Cocoa window.
 */
void DoCocoaSetRepresentedFilename( void *pView, CFStringRef path )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pView )
	{
		DoSetRepresentedFilename *pDoSetRepresentedFilename = [DoSetRepresentedFilename createWithPath:(NSString *)path view:(NSView *)pView];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pDoSetRepresentedFilename performSelectorOnMainThread:@selector(setFilename:) withObject:pDoSetRepresentedFilename waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}
