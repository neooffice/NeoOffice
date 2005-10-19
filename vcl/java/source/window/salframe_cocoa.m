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
	BOOL					mbFullScreen;
	NSPoint					maPoint;
	NSRect					maBounds;
}
- (NSRect)bounds;
- (void)calcBounds:(id)pObject;
- (id)initWithPoint:(NSPoint)aPoint fullScreen:(BOOL)bFullScreen;
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
		NSRect aVirtualBounds = NSMakeRect( 0, 0, 0, 0 );
		unsigned nCount = [pScreens count];

		// Iterate through the screen devices and calculate the virtual screen
		// size
		unsigned i;
		for ( i = 0; i < nCount; i++ )
		{
			NSScreen *pScreen = (NSScreen *)[pScreens objectAtIndex:i];
			aVirtualBounds = NSUnionRect( aVirtualBounds, [pScreen frame] );
		}

		// Iterate through the screen devices in reverse order and find the
		// screen that the top left corner of the frame is in and if we get to
		// the first screen always use that as the default
		for ( i = nCount - 1; i >= 0; i-- )
		{
			NSScreen *pScreen = (NSScreen *)[pScreens objectAtIndex:i];
			NSRect aBounds = [pScreen frame];

			// Flip the coordinate system to match the VCL coordinate system
			aBounds.origin.y = aVirtualBounds.size.height - aBounds.origin.y - aBounds.size.height;
			
			if ( !i || NSPointInRect( maPoint, aBounds ) )
			{
				if ( !mbFullScreen )
				{
					// Flip the coordinate system to match the VCL coordinate
					// system
					NSRect aVisibleBounds = [pScreen visibleFrame];
					aVisibleBounds.origin.y = aBounds.origin.y + aBounds.size.height - aVisibleBounds.origin.y - aVisibleBounds.size.height;
					maBounds = aVisibleBounds;
				}
				else
				{
					maBounds = aBounds;
				}

				break;
			}
		}
	}
}

- (id)initWithPoint:(NSPoint)aPoint fullScreen:(BOOL)bFullScreen
{
	[super init];

	mbFullScreen = bFullScreen;
	maPoint = aPoint;
	maBounds = NSMakeRect( 0, 0, 800, 600 );

	return self;
}

@end

void NSScreen_getScreenBounds( long *nX, long *nY, long *nWidth, long *nHeight, BOOL bFullScreen )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	ScreenBounds *pScreenBounds = [[ScreenBounds alloc] initWithPoint:NSMakePoint( (float)( *nX ), (float)( *nY ) ) fullScreen:bFullScreen];
	[pScreenBounds performSelectorOnMainThread:@selector(calcBounds:) withObject:pScreenBounds waitUntilDone:YES];

	NSRect aBounds = [pScreenBounds bounds];
	*nX = (long)aBounds.origin.x;
	*nY = (long)aBounds.origin.y;
	*nWidth = (long)( aBounds.origin.x + aBounds.size.width );
	*nHeight = (long)( aBounds.origin.y + aBounds.size.height );

	[pPool release];
}
