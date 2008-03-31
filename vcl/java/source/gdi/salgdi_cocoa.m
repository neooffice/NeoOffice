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

#import <Cocoa/Cocoa.h>
#import "salgdi_cocoa.h"

@interface DrawEPSInBitmap : NSObject
{
	int					mnDestHeight;
	int					mnDestWidth;
	int*				mpDestPtr;
	void*				mpEPSPtr;
	unsigned			mnEPSSize;
	BOOL				mbResult;
}
+ (id)epsWithPtr:(void *)pEPSPtr size:(unsigned)nEPSSize destPtr:(int *)pDestPtr destWidth:(int)nDestWidth destHeight:(int)nDestHeight;
- (void)drawEPSInBitmap:(id)pObject;
- (id)initWithPtr:(void *)pEPSPtr size:(unsigned)nEPSSize destPtr:(int *)pDestPtr destWidth:(int)nDestWidth destHeight:(int)nDestHeight;
- (BOOL)result;
@end

@implementation DrawEPSInBitmap

+ (id)epsWithPtr:(void *)pEPSPtr size:(unsigned)nEPSSize destPtr:(int *)pDestPtr destWidth:(int)nDestWidth destHeight:(int)nDestHeight
{
	DrawEPSInBitmap *pRet = [[DrawEPSInBitmap alloc] initWithPtr:pEPSPtr size:nEPSSize destPtr:pDestPtr destWidth:nDestWidth destHeight:nDestHeight];
	[pRet autorelease];
	return pRet;
}

- (void)drawEPSInBitmap:(id)pObject
{
	NSData *pEPSData = [NSData dataWithBytesNoCopy:mpEPSPtr length:mnEPSSize freeWhenDone:NO];
	if ( pEPSData )
	{
		NSImage *pEPSImage = [[NSImage alloc] initWithData:pEPSData];
		if ( pEPSImage )
		{
			// Add to autorelease pool as invoking alloc disables autorelease
			[pEPSImage autorelease];

			[pEPSImage setScalesWhenResized:YES];
			[pEPSImage setSize:NSMakeSize( mnDestWidth, mnDestHeight )];

			NSView *pFocusView = [NSView focusView];
			if ( pFocusView )
				[pFocusView unlockFocus];

			[pEPSImage lockFocus];

			NSBitmapImageRep *pBitmapImageRep = [[NSBitmapImageRep alloc] initWithFocusedViewRect:NSMakeRect(0, 0, mnDestWidth, mnDestHeight )];
			if ( pBitmapImageRep )
			{
				// Add to autorelease pool as invoking alloc disables
				// autorelease
				[pBitmapImageRep autorelease];

				int nBitsPerPixel = [pBitmapImageRep bitsPerPixel];
				if ( nBitsPerPixel == 24 )
				{
					// Fix bug 2218 by handling 24 bit EPS images
					char *pBitmapBuffer = (char *)[pBitmapImageRep bitmapData];
					if ( pBitmapBuffer )
					{
						// Pixels are in RBG format
						long nPixels = mnDestWidth * mnDestHeight;
						long i = 0;
						long j = 0;
						for ( ; i < nPixels; i++, j += 3 )
							mpDestPtr[ i ] = ( ( (int)pBitmapBuffer[ j ] & 0x000000ff ) << 16 ) | ( ( (int)pBitmapBuffer[ j + 1 ] & 0x000000ff ) << 8 ) | ( (int)pBitmapBuffer[ j + 2 ] & 0x000000ff ) | 0xff000000;
					}

					mbResult = YES;
				}
				else if ( nBitsPerPixel == 32 )
				{
					int *pBitmapBuffer = (int *)[pBitmapImageRep bitmapData];
					if ( pBitmapBuffer )
					{
						// Pixels are in RBGA format on PowerPC and in ABGR
						// format on Intel
						long nPixels = mnDestWidth * mnDestHeight;
						long i = 0;
						for ( ; i < nPixels; i++ )
#ifdef POWERPC
							mpDestPtr[ i ] = ( ( pBitmapBuffer[ i ] & 0xffffff00 ) >> 8 ) | ( ( pBitmapBuffer[ i ] & 0x000000ff ) << 24 );
#else	// POWERPC
							mpDestPtr[ i ] = ( pBitmapBuffer[ i ] & 0xff00ff00 ) | ( ( pBitmapBuffer[ i ] & 0x00ff0000 ) >> 16 ) | ( ( pBitmapBuffer[ i ] & 0x000000ff ) << 16 );
#endif	// POWERPC
					}

					mbResult = YES;
				}	
			}

			[pEPSImage unlockFocus];

			if ( pFocusView )
				[pFocusView lockFocus];
		}
	}
}

- (id)initWithPtr:(void *)pEPSPtr size:(unsigned)nEPSSize destPtr:(int *)pDestPtr destWidth:(int)nDestWidth destHeight:(int)nDestHeight
{
	[super init];

	mnDestWidth = nDestWidth;
	mpDestPtr = pDestPtr;
	mnDestHeight = nDestHeight;
	mpEPSPtr = pEPSPtr;
	mnEPSSize = nEPSSize;
	mbResult = NO;

	return self;
}

- (BOOL)result
{
	return mbResult;
}

@end

BOOL NSEPSImageRep_drawInBitmap( void *pEPSPtr, unsigned nEPSSize, int *pDestPtr, int nDestWidth, int nDestHeight )
{
	BOOL bRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pEPSPtr && nEPSSize && pDestPtr && nDestWidth && nDestHeight )
	{
		DrawEPSInBitmap *pDrawEPSInBitmap = [DrawEPSInBitmap epsWithPtr:pEPSPtr size:nEPSSize destPtr:pDestPtr destWidth:nDestWidth destHeight:nDestHeight];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pDrawEPSInBitmap performSelectorOnMainThread:@selector(drawEPSInBitmap:) withObject:pDrawEPSInBitmap waitUntilDone:YES modes:pModes];
		bRet = [pDrawEPSInBitmap result];
	}

	[pPool release];

	return bRet;
}
