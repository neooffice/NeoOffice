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
#import "VCLGraphics_cocoa.h"

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
- (void)drawEPSInRect:(id)pObject;
- (id)initWithPtr:(void *)pPtr size:(unsigned)nSize x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
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
						if ( mfClipWidth && mfClipHeight )
							CGContextClipToRect( aContext, CGRectMake( mfClipX, mfClipY, mfClipWidth, mfClipHeight ) );

						NSData *pData = [NSData dataWithBytesNoCopy:mpPtr length:mnSize freeWhenDone:NO];
						if ( pData )
						{
							NSImageRep *pImage = [NSEPSImageRep imageRepWithData:pData];
							if ( pImage )
								[pImage drawInRect:NSMakeRect( mfX, mfY + mfHeight, mfWidth, mfHeight * -1 )];
						}

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

- (id)initWithPtr:(void *)pPtr size:(unsigned)nSize x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY
{
	[super init];

	mpPtr = pPtr;
	mnSize = nSize;
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
						if ( mfClipWidth && mfClipHeight )
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

@interface DrawGlyphs : NSObject
{
	float				mfX;
	float				mfY;
	size_t				mnCount;
	CGGlyph*			mpGlyphs;
	CGSize*				mpSizes;
	CGFontRef			maFont;
	int					mnFontSize;
	int					mnColor;
	float				mfGlyphTranslateX;
	float				mfGlyphTranslateY;
	float				mfGlyphRotateAngle;
	float				mfGlyphScaleX;
	float				mfGlyphScaleY;
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
- (void)drawGlyphs:(id)pObject;
- (id)init:(float)fX y:(float)fY count:(size_t)nCount glyphs:(CGGlyph *)pGlyphs sizes:(CGSize *)pSizes font:(CGFontRef)aFont fontSize:(int)nFontSize color:(int)nColor glyphTranslateX:(float)fGlyphTranslateX glyphTranslateY:(float)fGlyphTranslateY glyphRotateAngle:(float)fGlyphRotateAngle glyphScaleX:(float)fGlyphScaleX glyphScaleY:(float)fGlyphScaleY clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
@end

@implementation DrawGlyphs

- (void)drawGlyphs:(id)pObject
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
						if ( mfClipWidth && mfClipHeight )
							CGContextClipToRect( aContext, CGRectMake( mfClipX, mfClipY, mfClipWidth, mfClipHeight ) );
						CGContextTranslateCTM( aContext, mfX, mfY);
						CGContextRotateCTM( aContext, mfGlyphRotateAngle );
						CGContextScaleCTM( aContext, mfGlyphScaleX, mfGlyphScaleY * -1 );
						CGContextTranslateCTM( aContext, mfGlyphTranslateX, mfGlyphTranslateY * -1 );

						CGContextSetRGBStrokeColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
						CGContextSetRGBFillColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );

						// Fix bug 2674 by setting all translation, rotation, and
						// scale in the CGContext and not in the text matrix
						CGAffineTransform aTransform = CGAffineTransformMake( 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f );
						CGContextSetTextMatrix( aContext, aTransform );

						CGContextSetFont( aContext, maFont );
						CGContextSetFontSize( aContext, mnFontSize );
						CGContextShowGlyphsWithAdvances( aContext, mpGlyphs, mpSizes, mnCount );

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

- (id)init:(float)fX y:(float)fY count:(size_t)nCount glyphs:(CGGlyph *)pGlyphs sizes:(CGSize *)pSizes font:(CGFontRef)aFont fontSize:(int)nFontSize color:(int)nColor glyphTranslateX:(float)fGlyphTranslateX glyphTranslateY:(float)fGlyphTranslateY glyphRotateAngle:(float)fGlyphRotateAngle glyphScaleX:(float)fGlyphScaleX glyphScaleY:(float)fGlyphScaleY clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
{
	[super init];

	mfX = fX;
	mfY = fY;
	mnCount = nCount;
	mpGlyphs = pGlyphs;
	mpSizes = pSizes;
	maFont = aFont;
	mnFontSize = nFontSize;
	mnColor = nColor;
	mfGlyphTranslateX = fGlyphTranslateX;
	mfGlyphTranslateY = fGlyphTranslateY;
	mfGlyphRotateAngle = fGlyphRotateAngle;
	mfGlyphScaleX = fGlyphScaleX;
	mfGlyphScaleY = fGlyphScaleY;
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

@interface DrawLine : NSObject
{
	float				mfX1;
	float				mfY1;
	float				mfX2;
	float				mfY2;
	int					mnColor;
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
- (void)drawLine:(id)pObject;
- (id)init:(float)fX1 y1:(float)fY1 x2:(float)fX2 y2:(float)fY2 color:(int)nColor clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
@end

@implementation DrawLine

- (void)drawLine:(id)pObject
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
						if ( mfClipWidth && mfClipHeight )
							CGContextClipToRect( aContext, CGRectMake( mfClipX, mfClipY, mfClipWidth, mfClipHeight ) );

						CGContextBeginPath( aContext );
						CGContextMoveToPoint( aContext, mfX1, mfY1 );
						CGContextAddLineToPoint( aContext, mfX2, mfY2 );
						CGContextSetRGBStrokeColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
						CGContextStrokePath( aContext );

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

- (id)init:(float)fX1 y1:(float)fY1 x2:(float)fX2 y2:(float)fY2 color:(int)nColor clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY
{
	[super init];

	mfX1 = fX1;
	mfY1 = fY1;
	mfX2 = fX2;
	mfY2 = fY2;
	mnColor = nColor;
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

@interface DrawPolygon : NSObject
{
	int					mnPoints;
	float*				mpXPoints;
	float*				mpYPoints;
	int					mnColor;
	BOOL				mbFill;
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
- (void)drawPolygon:(id)pObject;
- (id)initWithPoints:(int)nPoints xPoints:(float *)pXPoints yPoints:(float *)pYPoints color:(int)nColor fill:(BOOL)bFill clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
@end

@implementation DrawPolygon

- (void)drawPolygon:(id)pObject
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
						if ( mfClipWidth && mfClipHeight )
							CGContextClipToRect( aContext, CGRectMake( mfClipX, mfClipY, mfClipWidth, mfClipHeight ) );

						CGContextBeginPath( aContext );
						CGContextMoveToPoint( aContext, mpXPoints[ 0 ], mpYPoints[ 0 ] );
						int i = 1;
						for ( ; i < mnPoints; i++ )
							CGContextAddLineToPoint( aContext, mpXPoints[ i ], mpYPoints[ i ] );
						CGContextClosePath( aContext );
						if ( mbFill )
						{
							CGContextSetRGBFillColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
							CGContextFillPath( aContext );
						}
						else
						{
							CGContextSetRGBStrokeColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
							CGContextStrokePath( aContext );
						}

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

- (id)initWithPoints:(int)nPoints xPoints:(float *)pXPoints yPoints:(float *)pYPoints color:(int)nColor fill:(BOOL)bFill clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY
{
	[super init];

	mnPoints = nPoints;
	mpXPoints = pXPoints;
	mpYPoints = pYPoints;
	mnColor = nColor;
	mbFill = bFill;
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

@interface DrawPolyline : NSObject
{
	int					mnPoints;
	float*				mpXPoints;
	float*				mpYPoints;
	int					mnColor;
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
- (void)drawPolyline:(id)pObject;
- (id)initWithPoints:(int)nPoints xPoints:(float *)pXPoints yPoints:(float *)pYPoints color:(int)nColor clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
@end

@implementation DrawPolyline

- (void)drawPolyline:(id)pObject
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
						if ( mfClipWidth && mfClipHeight )
							CGContextClipToRect( aContext, CGRectMake( mfClipX, mfClipY, mfClipWidth, mfClipHeight ) );

						CGContextBeginPath( aContext );
						CGContextMoveToPoint( aContext, mpXPoints[ 0 ], mpYPoints[ 0 ] );
						int i = 1;
						for ( ; i < mnPoints; i++ )
							CGContextAddLineToPoint( aContext, mpXPoints[ i ], mpYPoints[ i ] );
						CGContextSetRGBStrokeColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
						CGContextStrokePath( aContext );

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

- (id)initWithPoints:(int)nPoints xPoints:(float *)pXPoints yPoints:(float *)pYPoints color:(int)nColor clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY
{
	[super init];

	mnPoints = nPoints;
	mpXPoints = pXPoints;
	mpYPoints = pYPoints;
	mnColor = nColor;
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

@interface DrawRect : NSObject
{
	float				mfX;
	float				mfY;
	float				mfWidth;
	float				mfHeight;
	int					mnColor;
	BOOL				mbFill;
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
- (void)drawRect:(id)pObject;
- (id)init:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight color:(int)nColor fill:(BOOL)bFill clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
@end

@implementation DrawRect

- (void)drawRect:(id)pObject
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
						if ( mfClipWidth && mfClipHeight )
							CGContextClipToRect( aContext, CGRectMake( mfClipX, mfClipY, mfClipWidth, mfClipHeight ) );

						if ( mbFill )
						{
							CGContextSetRGBFillColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
							CGContextFillRect( aContext, CGRectMake( mfX, mfY, mfWidth, mfHeight ) );
						}
						else
						{
							CGContextSetRGBStrokeColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
							CGContextStrokeRect( aContext, CGRectMake( mfX, mfY, mfWidth, mfHeight ) );
						}

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

- (id)init:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight color:(int)nColor fill:(BOOL)bFill clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY
{
	[super init];

	mfX = fX;
	mfY = fY;
	mfWidth = fWidth;
	mfHeight = fHeight;
	mnColor = nColor;
	mbFill = bFill;
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

void CGContext_drawGlyphs( float fX, float fY, size_t nCount, CGGlyph *pGlyphs, CGSize *pSizes, CGFontRef aFont, int nFontSize, int nColor, float fGlyphTranslateX, float fGlyphTranslateY, float fGlyphRotateAngle, float fGlyphScaleX, float fGlyphScaleY, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( nCount && pGlyphs && pSizes && aFont && nFontSize )
	{
		DrawGlyphs *pDrawGlyphs = [[DrawGlyphs alloc] init:fX y:fY count:nCount glyphs:pGlyphs sizes:pSizes font:aFont fontSize:nFontSize color:nColor glyphTranslateX:fGlyphTranslateX glyphTranslateY:fGlyphTranslateY glyphRotateAngle:fGlyphRotateAngle glyphScaleX:fGlyphScaleX glyphScaleY:fGlyphScaleY clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
		if ( bDrawInMainThread )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pDrawGlyphs performSelectorOnMainThread:@selector(drawGlyphs:) withObject:pDrawGlyphs waitUntilDone:YES modes:pModes];
		}
		else
		{
			[pDrawGlyphs drawGlyphs:pDrawGlyphs];
		}
	}

	[pPool release];
}

void CGContext_drawLine( float fX1, float fY1, float fX2, float fY2, int nColor, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	DrawLine *pDrawLine = [[DrawLine alloc] init:fX1 y1:fY1 x2:fX2 y2:fY2 color:nColor clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
	if ( bDrawInMainThread )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pDrawLine performSelectorOnMainThread:@selector(drawLine:) withObject:pDrawLine waitUntilDone:YES modes:pModes];
	}
	else
	{
		[pDrawLine drawLine:pDrawLine];
	}

	[pPool release];
}

void CGContext_drawPolygon( int nPoints, float *pXPoints, float *pYPoints, int nColor, BOOL bFill, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( nPoints && pXPoints && pYPoints )
	{
		DrawPolygon *pDrawPolygon = [[DrawPolygon alloc] initWithPoints:nPoints xPoints:pXPoints yPoints:pYPoints color:nColor fill:bFill clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
		if ( bDrawInMainThread )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pDrawPolygon performSelectorOnMainThread:@selector(drawPolygon:) withObject:pDrawPolygon waitUntilDone:YES modes:pModes];
		}
		else
		{
			[pDrawPolygon drawPolygon:pDrawPolygon];
		}
	}

	[pPool release];
}

void CGContext_drawPolyline( int nPoints, float *pXPoints, float *pYPoints, int nColor, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( nPoints && pXPoints && pYPoints )
	{
		DrawPolyline *pDrawPolyline = [[DrawPolyline alloc] initWithPoints:nPoints xPoints:pXPoints yPoints:pYPoints color:nColor clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
		if ( bDrawInMainThread )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pDrawPolyline performSelectorOnMainThread:@selector(drawPolyline:) withObject:pDrawPolyline waitUntilDone:YES modes:pModes];
		}
		else
		{
			[pDrawPolyline drawPolyline:pDrawPolyline];
		}
	}

	[pPool release];
}

void CGContext_drawRect( float fX, float fY, float fWidth, float fHeight, int nColor, BOOL bFill, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	DrawRect *pDrawRect = [[DrawRect alloc] init:fX y:fY width:fWidth height:fHeight color:nColor fill:bFill clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
	if ( bDrawInMainThread )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pDrawRect performSelectorOnMainThread:@selector(drawRect:) withObject:pDrawRect waitUntilDone:YES modes:pModes];
	}
	else
	{
		[pDrawRect drawRect:pDrawRect];
	}

	[pPool release];
}

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

void NSEPSImageRep_drawInRect( void *pPtr, unsigned nSize, float fX, float fY, float fWidth, float fHeight, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pPtr && nSize && fWidth && fHeight )
	{
		DrawEPSInRect *pDrawEPSInRect = [[DrawEPSInRect alloc] initWithPtr:pPtr size:nSize x:fX y:fY width:fWidth height:fHeight clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
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
