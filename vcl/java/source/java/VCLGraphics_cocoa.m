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

		// Fix bug 1990 by caching and reusing CGFontRefs
		if ( [fFont respondsToSelector:@selector(_atsFontID)] )
            fNativeCGFont = CreateCachedCGFont( (ATSFontRef)[fFont _atsFontID] );
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
	float				mfTranslateX;
	float				mfTranslateY;
	float				mfRotateAngle;
	float				mfScaleX;
	float				mfScaleY;
}
- (void)drawImageInRect:(id)pObject;
- (id)initWithImage:(CGImageRef)aImage x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
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
			NSRect aBounds = [pPrintView bounds];
			FlippedView *pFlippedView = [[FlippedView alloc] initWithFrame:NSMakeRect( 0, 0, aBounds.size.width, aBounds.size.height )];
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
						CGContextTranslateCTM( aContext, mfTranslateX, mfTranslateY );
						CGContextRotateCTM( aContext, mfRotateAngle );
						CGContextScaleCTM( aContext, mfScaleX, mfScaleY );
						CGContextClipToRect( aContext, CGRectMake( mfClipX, mfClipY, mfClipWidth, mfClipHeight ) );
						CGContextDrawImage( aContext, CGRectMake( mfX, mfY + mfHeight, mfWidth, mfHeight * -1 ), maImage );
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

- (id)initWithImage:(CGImageRef)aImage x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY
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
	mfTranslateX = fTranslateX;
	mfTranslateY = fTranslateY;
	mfRotateAngle = fRotateAngle;
	mfScaleX = fScaleX;
	mfScaleY = fScaleY;

	return self;
}

@end

void CGImageRef_drawInRect( CGImageRef aImage, float fX, float fY, float fWidth, float fHeight, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( aImage && fWidth && fHeight )
	{
		DrawImageInRect *pDrawImageInRect = [[DrawImageInRect alloc] initWithImage:aImage x:fX y:fY width:fWidth height:fHeight clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
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
