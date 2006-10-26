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

// Fix for bug 1928. Java 1.5 and higher will try to set its own arbitrary
// italic angle so we create a custom implementation of the JVM's private
// AWTFont class to ignore the custom transform that Java passes in.

@interface AWTFont : NSObject
{
	NSFont*				fFont;
	float*				fTransform;
	float				fPointSize;
	NSCharacterSet*		fCharacterSet;
	CGFontRef			fNativeCGFont;
}
+ (id)fontWithFont:(NSFont *)pFont matrix:(float *)pTransform;
- (id)initWithFont:(NSFont *)pFont matrix:(float *)pTransform;
- (void)dealloc;
@end

@implementation AWTFont

+ (id)fontWithFont:(NSFont *)pFont matrix:(float *)pTransform
{
	return [[AWTFont alloc] initWithFont:pFont matrix:pTransform];
}

- (id)initWithFont:(NSFont *)pFont matrix:(float *)pTransform
{
	[super init];

	fFont = pFont;
	if ( fFont )
	{
		[fFont retain];
		fPointSize = [fFont pointSize];
		fCharacterSet = [fFont coveredCharacterSet];
		if ( fCharacterSet )
			[fCharacterSet retain];

		if ( [fFont respondsToSelector:@selector(_atsFontID)] )
		{
            ATSFontRef aATSFont = (ATSFontRef)[fFont _atsFontID];
			fNativeCGFont = CGFontCreateWithPlatformFont( (void *)&aATSFont );
		}
	}

	fTransform = nil;

	return self;
}

- (void)dealloc
{
	if ( fFont )
		[fFont release];

	if ( fCharacterSet )
		[fCharacterSet release];

	CGFontRelease( fNativeCGFont );

	[super dealloc];
}

@end

@interface FlippedView : NSView
- (BOOL)isFlipped;
@end

@implementation FlippedView

- (BOOL)isFlipped
{
	return YES;
}

@end

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
			NSRect aFrame = [pPrintView frame];
			NSRect aBounds = [pPrintView bounds];
			FlippedView *pFlippedView = [[FlippedView alloc] initWithFrame:NSMakeRect( aBounds.origin.x * -1, aBounds.origin.y * -1, aFrame.size.width, aFrame.size.height )];
			if ( pFlippedView )
			{
				NSView *pFocusView = [NSView focusView];
				if ( pFocusView )
					[pFocusView unlockFocus];

				[pPrintView addSubview:pFlippedView];
				[pFlippedView lockFocus];

				NSData *pData = [NSData dataWithBytesNoCopy:mpPtr length:mnSize freeWhenDone:NO];
				if ( pData )
				{
					NSEPSImageRep *pImage = [NSEPSImageRep imageRepWithData:pData];
					if ( pImage )
						[pImage drawInRect:NSMakeRect( mfX, mfY, mfWidth, mfHeight )];
				}

				[pFlippedView unlockFocus];
				[pFlippedView removeFromSuperviewWithoutNeedingDisplay];

				if ( pFocusView )
					[pFocusView lockFocus];
			}
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
	float				mfClipX;
	float				mfClipY;
	float				mfClipWidth;
	float				mfClipHeight;
}
- (void)drawImageInRect:(id)pObject;
- (id)initWithImage:(CGImageRef)aImage x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight;
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
			NSRect aFrame = [pPrintView frame];
			NSRect aBounds = [pPrintView bounds];
			FlippedView *pFlippedView = [[FlippedView alloc] initWithFrame:NSMakeRect( aBounds.origin.x * -1, aBounds.origin.y * -1, aFrame.size.width, aFrame.size.height )];
			if ( pFlippedView )
			{
				NSView *pFocusView = [NSView focusView];
				if ( pFocusView )
					[pFocusView unlockFocus];

				[pPrintView addSubview:pFlippedView];
				[pFlippedView lockFocus];

				NSGraphicsContext *pContext = [NSGraphicsContext currentContext];
				if ( pContext )
				{
					CGContextRef aContext = (CGContextRef)[pContext graphicsPort];
					if ( aContext )
					{
						// Fix bug 1218 by setting the clip here and not in Java
						CGContextSaveGState( aContext );
						CGContextClipToRect( aContext, CGRectMake( mfClipX, mfClipY, mfClipWidth, mfClipHeight ) );
						CGContextDrawImage( aContext, CGRectMake( mfX, mfY, mfWidth, mfHeight ), maImage );
						CGContextRestoreGState( aContext );
					}
				}

				[pFlippedView unlockFocus];
				[pFlippedView removeFromSuperviewWithoutNeedingDisplay];

				if ( pFocusView )
					[pFocusView lockFocus];
			}
		}
	}
}

- (id)initWithImage:(CGImageRef)aImage x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight
{
	[super init];

	maImage = aImage;
	mfX = fX;
	mfY = fY;
	mfWidth = fWidth;
	mfHeight = fHeight;
	mfClipX = fClipX;
	mfClipY = fClipY;
	mfClipWidth = fClipWidth;
	mfClipHeight = fClipHeight;

	return self;
}

@end

void CGImageRef_drawInRect( CGImageRef aImage, float fX, float fY, float fWidth, float fHeight, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( aImage && fWidth && fHeight )
	{
		DrawImageInRect *pDrawImageInRect = [[DrawImageInRect alloc] initWithImage:aImage x:fX y:fY width:fWidth height:fHeight clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight];
		if ( bDrawInMainThread )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pDrawImageInRect performSelectorOnMainThread:@selector(drawImageInRect:) withObject:pDrawImageInRect waitUntilDone:YES modes:pModes];
		}
		else
		{
			[pDrawImageInRect drawImageInRect:pDrawImageInRect];
		}
	}

	[pPool release];
}

void NSEPSImageRep_drawInRect( void *pPtr, unsigned nSize, float fX, float fY, float fWidth, float fHeight, BOOL bDrawInMainThread )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pPtr && nSize && fWidth && fHeight )
	{
		DrawEPSInRect *pDrawEPSInRect = [[DrawEPSInRect alloc] initWithPtr:pPtr size:nSize x:fX y:fY width:fWidth height:fHeight];
		if ( bDrawInMainThread )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pDrawEPSInRect performSelectorOnMainThread:@selector(drawEPSInRect:) withObject:pDrawEPSInRect waitUntilDone:YES modes:pModes];
		}
		else
		{
			[pDrawEPSInRect drawEPSInRect:pDrawEPSInRect];
		}
	}

	[pPool release];
}
