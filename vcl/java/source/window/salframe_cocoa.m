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
 *  Patrick Luby, August 2005
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
#import "salframe_cocoa.h"

@interface ScreenBounds : NSObject
{
	NSRect					maBounds;
	BOOL					mbFullScreenMode;
	BOOL					mbUseMainScreenOnly;
	long*					mpX;
	long*					mpY;
	long*					mpWidth;
	long*					mpHeight;
}
- (void)calcBounds:(id)pObject;
- (id)initWithX:(long *)nX y:(long *)nY width:(long *)nWidth height:(long *)nHeight fullScreenMode:(BOOL)bFullScreenMode useMainScreenOnly:(BOOL)bUseMainScreenOnly;
@end

@implementation ScreenBounds

- (void)calcBounds:(id)pObject
{
	VCLScreen_getScreenBounds( mpX, mpY, mpWidth, mpHeight, mbFullScreenMode, mbUseMainScreenOnly );
}

- (id)initWithX:(long *)nX y:(long *)nY width:(long *)nWidth height:(long *)nHeight fullScreenMode:(BOOL)bFullScreenMode useMainScreenOnly:(BOOL)bUseMainScreenOnly;
{
	[super init];

	maBounds = NSMakeRect( 0, 0, 0, 0 );
	mbFullScreenMode = bFullScreenMode;
	mbUseMainScreenOnly = bUseMainScreenOnly;
	mpX = nX;
	mpY = nY;
	mpWidth = nWidth;
	mpHeight = nHeight;

	return self;
}

@end

void NSScreen_getScreenBounds( long *nX, long *nY, long *nWidth, long *nHeight, BOOL bFullScreenMode, BOOL bUseMainScreenOnly )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	ScreenBounds *pScreenBounds = [[ScreenBounds alloc] initWithX:nX y:nY width:nWidth height:nHeight fullScreenMode:bFullScreenMode useMainScreenOnly:bUseMainScreenOnly];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pScreenBounds performSelectorOnMainThread:@selector(calcBounds:) withObject:pScreenBounds waitUntilDone:YES modes:pModes];

	[pPool release];
}
