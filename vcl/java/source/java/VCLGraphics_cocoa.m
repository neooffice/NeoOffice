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
#import "VCLGraphics_cocoa.h"

@interface DrawEPSInRect : NSObject
{
	void*				mpPtr;
	unsigned			mnSize;
	float				mfX;
	float				mfY;
	float				mfWidth;
	float				mfHeight;
}
- (void)drawEPSInRect:(id)pObject;
- (id)initWithPtr:(void *)pPtr size:(unsigned)nSize x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight;
@end

@implementation DrawEPSInRect

- (void)drawEPSInRect:(id)pObject
{
	NSPrintOperation *pOperation = [NSPrintOperation currentOperation];
	if ( pOperation )
	{
		NSView *pPrintView = [pOperation view];
		if ( pPrintView )
		{
			NSView *pFocusView = [NSView focusView];
			if ( pFocusView )
				[pFocusView unlockFocus];

			[pPrintView lockFocus];

			NSData *pData = [NSData dataWithBytesNoCopy:mpPtr length:mnSize freeWhenDone:NO];
			if ( pData )
			{
				NSEPSImageRep *pImage = [NSEPSImageRep imageRepWithData:pData];
				if ( pImage )
					[pImage drawInRect:NSMakeRect( mfX, mfY, mfWidth, mfHeight )];
			}

			[pPrintView unlockFocus];

			if ( pFocusView )
				[pFocusView lockFocus];
		}
	}
}

- (id)initWithPtr:(void *)pPtr size:(unsigned)nSize x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight
{
	[super init];

	mpPtr = pPtr;
	mnSize = nSize;
	mfX = fX;
	mfY = fY;
	mfWidth = fWidth;
	mfHeight = fHeight;

	return self;
}

@end

@interface DrawImageInRect : NSObject
{
	CGImageRef			maImage;
	float				mfX;
	float				mfY;
	float				mfWidth;
	float				mfHeight;
}
- (void)drawImageInRect:(id)pObject;
- (id)initWithImage:(CGImageRef)aImage x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight;
@end

@implementation DrawImageInRect

- (void)drawImageInRect:(id)pObject
{
	NSPrintOperation *pOperation = [NSPrintOperation currentOperation];
	if ( pOperation )
	{
		NSView *pPrintView = [pOperation view];
		if ( pPrintView )
		{
			NSView *pFocusView = [NSView focusView];
			if ( pFocusView )
				[pFocusView unlockFocus];

			[pPrintView lockFocus];

			NSGraphicsContext *pContext = [NSGraphicsContext currentContext];
			if ( pContext )
			{
				CGContextRef aContext = (CGContextRef)[pContext graphicsPort];
				if ( aContext )
					CGContextDrawImage( aContext, CGRectMake( mfX, mfY, mfWidth, mfHeight ), maImage );
			}

			[pPrintView unlockFocus];

			if ( pFocusView )
				[pFocusView lockFocus];
		}
	}
}

- (id)initWithImage:(CGImageRef)aImage x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight
{
	[super init];

	maImage = aImage;
	mfX = fX;
	mfY = fY;
	mfWidth = fWidth;
	mfHeight = fHeight;

	return self;
}

@end

void CGImageRef_drawInRect( CGImageRef aImage, float fX, float fY, float fWidth, float fHeight )
{
	if ( aImage && fWidth && fHeight )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, @"AWTRunLoopMode", nil];
		DrawImageInRect *pDrawImageInRect = [[DrawImageInRect alloc] initWithImage:aImage x:fX y:fY width:fWidth height:fHeight];
		[pDrawImageInRect performSelectorOnMainThread:@selector(drawImageInRect:) withObject:pDrawImageInRect waitUntilDone:YES modes:pModes];
		[pDrawImageInRect release];
	}
}

void NSEPSImageRep_drawInRect( void *pPtr, unsigned nSize, float fX, float fY, float fWidth, float fHeight )
{
	if ( pPtr && nSize && fWidth && fHeight )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, @"AWTRunLoopMode", nil];
		DrawEPSInRect *pDrawEPSInRect = [[DrawEPSInRect alloc] initWithPtr:pPtr size:nSize x:fX y:fY width:fWidth height:fHeight];
		[pDrawEPSInRect performSelectorOnMainThread:@selector(drawEPSInRect:) withObject:pDrawEPSInRect waitUntilDone:YES modes:pModes];
		[pDrawEPSInRect release];
	}
}
