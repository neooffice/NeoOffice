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
	BOOL					mbFullScreenWindow;
	NSPoint					maPoint;
}
- (NSRect)bounds;
- (void)calcBounds:(id)pObject;
- (id)initWithPoint:(NSPoint)aPoint fullScreenMode:(BOOL)bFullScreenMode fullScreenWindow:(BOOL)bFullScreenWindow;
@end

@implementation ScreenBounds

- (NSRect)bounds
{
	return maBounds;
}

- (void)calcBounds:(id)pObject
{
	NSArray *pScreens = [NSScreen screens];
	if ( pScreens )
	{
		unsigned nCount = [pScreens count];
		NSScreen *pScreen = (NSScreen *)[pScreens objectAtIndex:0];
		NSRect aVirtualBounds = [pScreen frame];

		// Iterate through the screen devices and calculate the virtual screen
		// size
		unsigned i;
		for ( i = 1; i < nCount; i++ )
		{
			pScreen = (NSScreen *)[pScreens objectAtIndex:i];
			aVirtualBounds = NSUnionRect( aVirtualBounds, [pScreen frame] );
		}

		if ( mbFullScreenWindow )
		{
			// Use only the first screen for full screen mode
			NSScreen *pScreen = (NSScreen *)[pScreens objectAtIndex:0];
			NSRect aBounds = [pScreen frame];
	
			// Flip the coordinate system to match the VCL coordinate system
			aBounds.origin.y = aVirtualBounds.size.height - aBounds.origin.y - aBounds.size.height;
			maBounds = aBounds;
		}
		else
		{
			// Calculate the size of the visible portion of the virtual screen
			pScreen = (NSScreen *)[pScreens objectAtIndex:0];
			NSRect aBounds = ( mbFullScreenMode ? [pScreen frame] : [pScreen visibleFrame] );

			unsigned i;
			for ( i = 1; i < nCount; i++ )
			{
				pScreen = (NSScreen *)[pScreens objectAtIndex:i];
				aBounds = NSUnionRect( aBounds, mbFullScreenMode ? [pScreen frame] : [pScreen visibleFrame] );
			}

			// Flip the coordinate system to match the VCL coordinate system
			aBounds.origin.y = aVirtualBounds.origin.y + aVirtualBounds.size.height - aBounds.origin.y - aBounds.size.height;
			maBounds = aBounds;
		}
	}
}

- (id)initWithPoint:(NSPoint)aPoint fullScreenMode:(BOOL)bFullScreenMode fullScreenWindow:(BOOL)bFullScreenWindow;
{
	[super init];

	maBounds = NSMakeRect( 0, 0, 800, 600 );
	mbFullScreenMode = bFullScreenMode;
	mbFullScreenWindow = bFullScreenWindow;
	maPoint = aPoint;

	return self;
}

@end

void NSScreen_getScreenBounds( long *nX, long *nY, long *nWidth, long *nHeight, BOOL bFullScreenMode, BOOL bFullScreenWindow )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	ScreenBounds *pScreenBounds = [[ScreenBounds alloc] initWithPoint:NSMakePoint( (float)( *nX ), (float)( *nY ) ) fullScreenMode:bFullScreenMode fullScreenWindow:bFullScreenWindow];
	[pScreenBounds performSelectorOnMainThread:@selector(calcBounds:) withObject:pScreenBounds waitUntilDone:YES];

	NSRect aBounds = [pScreenBounds bounds];
	*nX = (long)aBounds.origin.x;
	*nY = (long)aBounds.origin.y;
	*nWidth = (long)( aBounds.origin.x + aBounds.size.width );
	*nHeight = (long)( aBounds.origin.y + aBounds.size.height );

	[pPool release];
}
