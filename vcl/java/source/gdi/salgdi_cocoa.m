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
#import "salgdi_cocoa.h"

@interface DrawEPSInBitmap : NSObject
{
	int					mnDestHeight;
	int					mnDestWidth;
	int*				mpDestPtr;
	void*				mpEPSPtr;
	unsigned			mnEPSSize;
}
- (void)drawEPSInBitmap;
- (id)initWithPtr:(void *)pEPSPtr size:(unsigned)nEPSSize destPtr:(int *)pDestPtr destWidth:(int)nDestWidth destHeight:(int)nDestHeight;
@end

@implementation DrawEPSInBitmap

- (BOOL)drawEPSInBitmap
{
	BOOL bRet = NO;

	NSData *pEPSData = [NSData dataWithBytesNoCopy:mpEPSPtr length:mnEPSSize freeWhenDone:NO];
	if ( pEPSData )
	{
		NSImage *pEPSImage = [[NSImage alloc] initWithData:pEPSData];
		if ( pEPSImage )
		{
			[pEPSImage setScalesWhenResized:YES];
			[pEPSImage setSize:NSMakeSize( mnDestWidth, mnDestHeight )];

			NSView *pFocusView = [NSView focusView];
			if ( pFocusView )
				[pFocusView unlockFocus];

			[pEPSImage lockFocus];

			NSBitmapImageRep *pBitmapImageRep = [[NSBitmapImageRep alloc] initWithFocusedViewRect:NSMakeRect(0, 0, mnDestWidth, mnDestHeight )];
			if ( pBitmapImageRep && [pBitmapImageRep bitsPerPixel] == 32 )
			{
				int *pBitmapBuffer = (int *)[pBitmapImageRep bitmapData];
				if ( pBitmapBuffer )
				{
					// Pixels are in RBGA format on PowerPC and in ABGR format
					// on Intel
					long nPixels = mnDestWidth * mnDestHeight;
					long i = 0;
					for ( ; i < nPixels; i++ )
#ifdef POWERPC
						mpDestPtr[ i ] = ( ( pBitmapBuffer[ i ] & 0xffffff00 ) >> 8 ) | ( ( pBitmapBuffer[ i ] & 0x000000ff ) << 24 );
#else	// POWERPC
						mpDestPtr[ i ] = ( pBitmapBuffer[ i ] & 0xff00ff00 ) | ( ( pBitmapBuffer[ i ] & 0x00ff0000 ) >> 16 ) | ( ( pBitmapBuffer[ i ] & 0x000000ff ) << 16 );
#endif	// POWERPC

					bRet = YES;
				}	
			}

			[pEPSImage unlockFocus];

			if ( pFocusView )
				[pFocusView lockFocus];
		}
	}

	return bRet;
}

- (id)initWithPtr:(void *)pEPSPtr size:(unsigned)nEPSSize destPtr:(int *)pDestPtr destWidth:(int)nDestWidth destHeight:(int)nDestHeight
{
	[super init];

	mnDestWidth = nDestWidth;
	mpDestPtr = pDestPtr;
	mnDestHeight = nDestHeight;
	mpEPSPtr = pEPSPtr;
	mnEPSSize = nEPSSize;

	return self;
}

@end

BOOL NSEPSImageRep_drawInBitmap( void *pEPSPtr, unsigned nEPSSize, int *pDestPtr, int nDestWidth, int nDestHeight )
{
	BOOL bRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pEPSPtr && nEPSSize && pDestPtr && nDestWidth && nDestHeight )
	{
		DrawEPSInBitmap *pDrawEPSInBitmap = [[DrawEPSInBitmap alloc] initWithPtr:pEPSPtr size:nEPSSize destPtr:pDestPtr destWidth:nDestWidth destHeight:nDestHeight];
		bRet = [pDrawEPSInBitmap drawEPSInBitmap];
	}

	[pPool release];

	return bRet;
}
