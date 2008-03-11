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
+ (id)createWithPtr:(void *)pPtr size:(unsigned)nSize x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
- (void)drawEPSInRect:(id)pObject;
- (id)initWithPtr:(void *)pPtr size:(unsigned)nSize x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
@end

@implementation DrawEPSInRect

+ (id)createWithPtr:(void *)pPtr size:(unsigned)nSize x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY
{
	DrawEPSInRect *pRet = [[DrawEPSInRect alloc] initWithPtr:pPtr size:nSize x:fX y:fY width:fWidth height:fHeight clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
	[pRet autorelease];
	return pRet;
}

- (void)drawEPSInRect:(id)pObject
{
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
			if ( mfClipWidth && mfClipHeight )
				CGContextClipToRect( aContext, CGRectMake( mfClipX * mfScaleX, mfClipY * mfScaleY, mfClipWidth * mfScaleX, mfClipHeight * mfScaleY ) );

			NSData *pData = [NSData dataWithBytesNoCopy:mpPtr length:mnSize freeWhenDone:NO];
			if ( pData )
			{
				NSImageRep *pImage = [NSEPSImageRep imageRepWithData:pData];
				if ( pImage )
					[pImage drawInRect:NSMakeRect( mfX * mfScaleX, ( mfY + mfHeight ) * mfScaleY, mfWidth * mfScaleX, mfHeight * mfScaleY * -1 )];
			}

			CGContextRestoreGState( aContext );
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
+ (id)createWithImage:(CGImageRef)aImage x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
- (void)drawImageInRect:(id)pObject;
- (id)initWithImage:(CGImageRef)aImage x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
@end

@implementation DrawImageInRect

+ (id)createWithImage:(CGImageRef)aImage x:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY
{
	DrawImageInRect *pRet = [[DrawImageInRect alloc] initWithImage:aImage x:fX y:fY width:fWidth height:fHeight clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
	[pRet autorelease];
	return pRet;
}

- (void)drawImageInRect:(id)pObject
{
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
			if ( mfClipWidth && mfClipHeight )
				CGContextClipToRect( aContext, CGRectMake( mfClipX * mfScaleX, mfClipY * mfScaleY, mfClipWidth * mfScaleX, mfClipHeight * mfScaleY ) );
			CGContextDrawImage( aContext, CGRectMake( mfX * mfScaleX, ( mfY + mfHeight ) * mfScaleY, mfWidth * mfScaleX, mfHeight * mfScaleY * -1 ), maImage );
			CGContextRestoreGState( aContext );
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
+ (id)create:(float)fX y:(float)fY count:(size_t)nCount glyphs:(CGGlyph *)pGlyphs sizes:(CGSize *)pSizes font:(CGFontRef)aFont fontSize:(int)nFontSize color:(int)nColor glyphTranslateX:(float)fGlyphTranslateX glyphTranslateY:(float)fGlyphTranslateY glyphRotateAngle:(float)fGlyphRotateAngle glyphScaleX:(float)fGlyphScaleX glyphScaleY:(float)fGlyphScaleY clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
- (void)drawGlyphs:(id)pObject;
- (id)init:(float)fX y:(float)fY count:(size_t)nCount glyphs:(CGGlyph *)pGlyphs sizes:(CGSize *)pSizes font:(CGFontRef)aFont fontSize:(int)nFontSize color:(int)nColor glyphTranslateX:(float)fGlyphTranslateX glyphTranslateY:(float)fGlyphTranslateY glyphRotateAngle:(float)fGlyphRotateAngle glyphScaleX:(float)fGlyphScaleX glyphScaleY:(float)fGlyphScaleY clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
@end

@implementation DrawGlyphs

+ (id)create:(float)fX y:(float)fY count:(size_t)nCount glyphs:(CGGlyph *)pGlyphs sizes:(CGSize *)pSizes font:(CGFontRef)aFont fontSize:(int)nFontSize color:(int)nColor glyphTranslateX:(float)fGlyphTranslateX glyphTranslateY:(float)fGlyphTranslateY glyphRotateAngle:(float)fGlyphRotateAngle glyphScaleX:(float)fGlyphScaleX glyphScaleY:(float)fGlyphScaleY clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY
{
	DrawGlyphs *pRet = [[DrawGlyphs alloc] init:fX y:fY count:nCount glyphs:pGlyphs sizes:pSizes font:aFont fontSize:nFontSize color:nColor glyphTranslateX:fGlyphTranslateX glyphTranslateY:fGlyphTranslateY glyphRotateAngle:fGlyphRotateAngle glyphScaleX:fGlyphScaleX glyphScaleY:fGlyphScaleY clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
	[pRet autorelease];
	return pRet;
}

- (void)drawGlyphs:(id)pObject
{
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
			if ( mfClipWidth && mfClipHeight )
				CGContextClipToRect( aContext, CGRectMake( mfClipX * mfScaleX, mfClipY * mfScaleY, mfClipWidth * mfScaleX, mfClipHeight * mfScaleY ) );

			CGContextTranslateCTM( aContext, mfX * mfScaleX, mfY * mfScaleY );
			CGContextRotateCTM( aContext, mfGlyphRotateAngle );
			CGContextTranslateCTM( aContext, mfGlyphTranslateX * mfGlyphScaleX * mfScaleX, mfGlyphTranslateY * mfGlyphScaleY * mfScaleY );
			CGContextScaleCTM( aContext, 1.0f, -1.0f );

			CGContextSetRGBStrokeColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
			CGContextSetRGBFillColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );

			// Fix bug 2674 by setting all translation, rotation, and scaling
			// in the CGContext and not in the text matrix. Fix bug 2957 by
			// moving the glyph scale back into the font transform.
			CGAffineTransform aTransform = CGAffineTransformMakeScale( mfGlyphScaleX, mfGlyphScaleY );
			CGContextSetTextMatrix( aContext, aTransform );

			CGContextSetFont( aContext, maFont );
			CGContextSetFontSize( aContext, (float)mnFontSize * mfScaleY );
			CGSize *pAdjustedSizes = (CGSize *)malloc( mnCount * sizeof( CGSize ) );
			if ( pAdjustedSizes )
			{
				size_t i;
				for ( i = 0; i < mnCount; i++ )
				{
					pAdjustedSizes[ i ].width = mpSizes[ i ].width * mfScaleX;
					pAdjustedSizes[ i ].height = mpSizes[ i ].height * mfScaleY;
				}
				CGContextShowGlyphsWithAdvances( aContext, mpGlyphs, pAdjustedSizes, mnCount );
				free( pAdjustedSizes );
			}

			CGContextRestoreGState( aContext );
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
+ (id)create:(float)fX1 y1:(float)fY1 x2:(float)fX2 y2:(float)fY2 color:(int)nColor clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
- (void)drawLine:(id)pObject;
- (id)init:(float)fX1 y1:(float)fY1 x2:(float)fX2 y2:(float)fY2 color:(int)nColor clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
@end

@implementation DrawLine

+ (id)create:(float)fX1 y1:(float)fY1 x2:(float)fX2 y2:(float)fY2 color:(int)nColor clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY
{
	DrawLine *pRet = [[DrawLine alloc] init:fX1 y1:fY1 x2:fX2 y2:fY2 color:nColor clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
	[pRet autorelease];
	return pRet;
}

- (void)drawLine:(id)pObject
{
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
			if ( mfClipWidth && mfClipHeight )
				CGContextClipToRect( aContext, CGRectMake( mfClipX * mfScaleX, mfClipY * mfScaleY, mfClipWidth * mfScaleX, mfClipHeight * mfScaleY ) );

			CGContextBeginPath( aContext );
			CGContextMoveToPoint( aContext, mfX1 * mfScaleX, mfY1 * mfScaleY );
			CGContextAddLineToPoint( aContext, mfX2 * mfScaleX, mfY2 * mfScaleY );
			CGContextSetRGBStrokeColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
			CGContextSetLineWidth( aContext, mfScaleX );
			CGContextStrokePath( aContext );

			CGContextRestoreGState( aContext );
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
+ (id)createWithPoints:(int)nPoints xPoints:(float *)pXPoints yPoints:(float *)pYPoints color:(int)nColor fill:(BOOL)bFill clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
- (void)drawPolygon:(id)pObject;
- (id)initWithPoints:(int)nPoints xPoints:(float *)pXPoints yPoints:(float *)pYPoints color:(int)nColor fill:(BOOL)bFill clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
@end

@implementation DrawPolygon

+ (id)createWithPoints:(int)nPoints xPoints:(float *)pXPoints yPoints:(float *)pYPoints color:(int)nColor fill:(BOOL)bFill clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY
{
	DrawPolygon *pRet = [[DrawPolygon alloc] initWithPoints:nPoints xPoints:pXPoints yPoints:pYPoints color:nColor fill:bFill clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
	[pRet autorelease];
	return pRet;
}

- (void)drawPolygon:(id)pObject
{
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
			if ( mfClipWidth && mfClipHeight )
				CGContextClipToRect( aContext, CGRectMake( mfClipX * mfScaleX, mfClipY * mfScaleY, mfClipWidth * mfScaleX, mfClipHeight * mfScaleY ) );

			CGContextBeginPath( aContext );
			CGContextMoveToPoint( aContext, mpXPoints[ 0 ] * mfScaleX, mpYPoints[ 0 ] * mfScaleY );
			int i = 1;
			for ( ; i < mnPoints; i++ )
				CGContextAddLineToPoint( aContext, mpXPoints[ i ] * mfScaleX, mpYPoints[ i ] * mfScaleY );
			CGContextClosePath( aContext );
			if ( mbFill )
			{
				CGContextSetRGBFillColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
				CGContextFillPath( aContext );
			}
			else
			{
				CGContextSetRGBStrokeColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
				CGContextSetLineWidth( aContext, mfScaleX );
				CGContextStrokePath( aContext );
			}

			CGContextRestoreGState( aContext );
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
+ (id)createWithPoints:(int)nPoints xPoints:(float *)pXPoints yPoints:(float *)pYPoints color:(int)nColor clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
- (void)drawPolyline:(id)pObject;
- (id)initWithPoints:(int)nPoints xPoints:(float *)pXPoints yPoints:(float *)pYPoints color:(int)nColor clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
@end

@implementation DrawPolyline

+ (id)createWithPoints:(int)nPoints xPoints:(float *)pXPoints yPoints:(float *)pYPoints color:(int)nColor clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY
{
	DrawPolyline *pRet = [[DrawPolyline alloc] initWithPoints:nPoints xPoints:pXPoints yPoints:pYPoints color:nColor clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
	[pRet autorelease];
	return pRet;
}

- (void)drawPolyline:(id)pObject
{
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
			if ( mfClipWidth && mfClipHeight )
				CGContextClipToRect( aContext, CGRectMake( mfClipX * mfScaleX, mfClipY * mfScaleY, mfClipWidth * mfScaleX, mfClipHeight * mfScaleY ) );

			CGContextBeginPath( aContext );
			CGContextMoveToPoint( aContext, mpXPoints[ 0 ] * mfScaleX, mpYPoints[ 0 ] * mfScaleY );
			int i = 1;
			for ( ; i < mnPoints; i++ )
				CGContextAddLineToPoint( aContext, mpXPoints[ i ] * mfScaleX, mpYPoints[ i ] * mfScaleY );
			CGContextSetRGBStrokeColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
			CGContextSetLineWidth( aContext, mfScaleX );
			CGContextStrokePath( aContext );

			CGContextRestoreGState( aContext );
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

@interface DrawPolyPolygon : NSObject
{
	int					mnPoly;
	int*				mpPoints;
	float**				mppXPoints;
	float**				mppYPoints;
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
+ (id)createWithPolygons:(int)nPoly points:(int *)pPoints xPoints:(float **)ppXPoints yPoints:(float **)ppYPoints color:(int)nColor fill:(BOOL)bFill clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
- (void)drawPolyPolygon:(id)pObject;
- (id)initWithPolygons:(int)nPoly points:(int *)pPoints xPoints:(float **)ppXPoints yPoints:(float **)ppYPoints color:(int)nColor fill:(BOOL)bFill clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
@end

@implementation DrawPolyPolygon

+ (id)createWithPolygons:(int)nPoly points:(int *)pPoints xPoints:(float **)ppXPoints yPoints:(float **)ppYPoints color:(int)nColor fill:(BOOL)bFill clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY
{
	DrawPolygon *pRet = [[DrawPolyPolygon alloc] initWithPolygons:nPoly points:pPoints xPoints:ppXPoints yPoints:ppYPoints color:nColor fill:bFill clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
	[pRet autorelease];
	return pRet;
}

- (void)drawPolyPolygon:(id)pObject
{
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
			if ( mfClipWidth && mfClipHeight )
				CGContextClipToRect( aContext, CGRectMake( mfClipX * mfScaleX, mfClipY * mfScaleY, mfClipWidth * mfScaleX, mfClipHeight * mfScaleY ) );

			CGContextBeginPath( aContext );
			int i = 0;
			for ( ; i < mnPoly; i++ )
			{
				if ( mpPoints[ i ] )
				{
					CGContextMoveToPoint( aContext, mppXPoints[ i ][ 0 ] * mfScaleX, mppYPoints[ i ][ 0 ] * mfScaleY );
					int j = 1;
					for ( ; j < mpPoints[ i ]; j++ )
						CGContextAddLineToPoint( aContext, mppXPoints[ i ][ j ] * mfScaleX, mppYPoints[ i ][ j ] * mfScaleY );
					CGContextClosePath( aContext );
				}
			}
			CGContextClosePath( aContext );

			if ( mbFill )
			{
				CGContextSetRGBFillColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
				CGContextEOFillPath( aContext );
			}
			else
			{
				CGContextSetRGBStrokeColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
				CGContextSetLineWidth( aContext, mfScaleX );
				CGContextStrokePath( aContext );
			}

			CGContextRestoreGState( aContext );
		}
	}
}

- (id)initWithPolygons:(int)nPoly points:(int *)pPoints xPoints:(float **)ppXPoints yPoints:(float **)ppYPoints color:(int)nColor fill:(BOOL)bFill clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY
{
	[super init];

	mnPoly = nPoly;
	mpPoints = pPoints;
	mppXPoints = ppXPoints;
	mppYPoints = ppYPoints;
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
+ (id)create:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight color:(int)nColor fill:(BOOL)bFill clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
- (void)drawRect:(id)pObject;
- (id)init:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight color:(int)nColor fill:(BOOL)bFill clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY;
@end

@implementation DrawRect

+ (id)create:(float)fX y:(float)fY width:(float)fWidth height:(float)fHeight color:(int)nColor fill:(BOOL)bFill clipX:(float)fClipX clipY:(float)fClipY clipWidth:(float)fClipWidth clipHeight:(float)fClipHeight translateX:(float)fTranslateX translateY:(float)fTranslateY rotateAngle:(float)fRotateAngle scaleX:(float)fScaleX scaleY:(float)fScaleY
{
	DrawRect *pRet = [[DrawRect alloc] init:fX y:fY width:fWidth height:fHeight color:nColor fill:bFill clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
	[pRet autorelease];
	return pRet;
}

- (void)drawRect:(id)pObject
{
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
			if ( mfClipWidth && mfClipHeight )
				CGContextClipToRect( aContext, CGRectMake( mfClipX * mfScaleX, mfClipY * mfScaleY, mfClipWidth * mfScaleX, mfClipHeight * mfScaleY ) );

			if ( mbFill )
			{
				CGContextSetRGBFillColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
				CGContextFillRect( aContext, CGRectMake( mfX * mfScaleX, mfY * mfScaleY, mfWidth * mfScaleX, mfHeight * mfScaleY ) );
			}
			else
			{
				CGContextSetRGBStrokeColor( aContext, (float)( ( mnColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( mnColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( mnColor & 0x000000ff ) / (float)0xff, (float)( ( mnColor & 0xff000000 ) >> 24 ) / (float)0xff );
				CGContextStrokeRect( aContext, CGRectMake( mfX * mfScaleX, mfY * mfScaleY, mfWidth * mfScaleX, mfHeight * mfScaleY ) );
			}

			CGContextRestoreGState( aContext );
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
		DrawGlyphs *pDrawGlyphs = [DrawGlyphs create:fX y:fY count:nCount glyphs:pGlyphs sizes:pSizes font:aFont fontSize:nFontSize color:nColor glyphTranslateX:fGlyphTranslateX glyphTranslateY:fGlyphTranslateY glyphRotateAngle:fGlyphRotateAngle glyphScaleX:fGlyphScaleX glyphScaleY:fGlyphScaleY clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
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

	DrawLine *pDrawLine = [DrawLine create:fX1 y1:fY1 x2:fX2 y2:fY2 color:nColor clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
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
		DrawPolygon *pDrawPolygon = [DrawPolygon createWithPoints:nPoints xPoints:pXPoints yPoints:pYPoints color:nColor fill:bFill clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
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
		DrawPolyline *pDrawPolyline = [DrawPolyline createWithPoints:nPoints xPoints:pXPoints yPoints:pYPoints color:nColor clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
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

void CGContext_drawPolyPolygon( int nPoly, int *pNPoints, float **ppXPoints, float **ppYPoints, int nColor, BOOL bFill, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( nPoly && pNPoints && ppXPoints && ppYPoints && *ppXPoints && *ppYPoints )
	{
		DrawPolyPolygon *pDrawPolyPolygon = [DrawPolyPolygon createWithPolygons:nPoly points:pNPoints xPoints:ppXPoints yPoints:ppYPoints color:nColor fill:bFill clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
		if ( bDrawInMainThread )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pDrawPolyPolygon performSelectorOnMainThread:@selector(drawPolyPolygon:) withObject:pDrawPolyPolygon waitUntilDone:YES modes:pModes];
		}
		else
		{
			[pDrawPolyPolygon drawPolyPolygon:pDrawPolyPolygon];
		}
	}

	[pPool release];
}

void CGContext_drawRect( float fX, float fY, float fWidth, float fHeight, int nColor, BOOL bFill, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	DrawRect *pDrawRect = [DrawRect create:fX y:fY width:fWidth height:fHeight color:nColor fill:bFill clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
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
		DrawImageInRect *pDrawImageInRect = [DrawImageInRect createWithImage:aImage x:fX y:fY width:fWidth height:fHeight clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
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
		DrawEPSInRect *pDrawEPSInRect = [DrawEPSInRect createWithPtr:pPtr size:nSize x:fX y:fY width:fWidth height:fHeight clipX:fClipX clipY:fClipY clipWidth:fClipWidth clipHeight:fClipHeight translateX:fTranslateX translateY:fTranslateY rotateAngle:fRotateAngle scaleX:fScaleX scaleY:fScaleY];
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
