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
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 Planamesa Inc.
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

#include <salgdi.h>
#include <saldata.hxx>
#include <salframe.h>
#include <com/sun/star/vcl/VCLBitmap.hxx>
#include <com/sun/star/vcl/VCLGraphics.hxx>
#include <com/sun/star/vcl/VCLFont.hxx>
#include <com/sun/star/vcl/VCLPath.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include "salgdi_cocoa.h"

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#define XOR_BITMAP_BOUNDS_PADDING 2

class SAL_DLLPRIVATE JavaSalGraphicsCopyLayerOp : public JavaSalGraphicsOp
{
	CGLayerRef				maSrcLayer;
	CGPoint					maSrcPoint;
	CGRect					maRect;

public:
JavaSalGraphicsCopyLayerOp::JavaSalGraphicsCopyLayerOp( const CGPathRef aNativeClipPath, bool bInvert, CGLayerRef aXORLayer, CGLayerRef aSrcLayer, const CGPoint aSrcPoint, const CGRect aRect );
	virtual					~JavaSalGraphicsCopyLayerOp();

	virtual	void			drawOp( CGContextRef aContext, CGRect aBounds );
};

class SAL_DLLPRIVATE JavaSalGraphicsDrawEPSOp : public JavaSalGraphicsOp
{
	CFDataRef				maData;
	CGRect					maRect;

public:
							JavaSalGraphicsDrawEPSOp( const CGPathRef aNativeClipPath, CFDataRef aData, const CGRect aRect );
	virtual					~JavaSalGraphicsDrawEPSOp();

	virtual	void			drawOp( CGContextRef aContext, CGRect aBounds );
};

class SAL_DLLPRIVATE JavaSalGraphicsDrawLineOp : public JavaSalGraphicsOp
{
	float					mfX1;
	float					mfY1;
	float					mfX2;
	float					mfY2;
	SalColor				mnColor;

public:
							JavaSalGraphicsDrawLineOp( const CGPathRef aNativeClipPath, bool bInvert, CGLayerRef aXORLayer, float fX1, float fY1, float fX2, float fY2, SalColor nColor ) : JavaSalGraphicsOp( aNativeClipPath, bInvert, aXORLayer ), mfX1( fX1 ), mfY1( fY1 ), mfX2( fX2 ), mfY2( fY2 ), mnColor( nColor ) {}
	virtual					~JavaSalGraphicsDrawLineOp() {}

	virtual	void			drawOp( CGContextRef aContext, CGRect aBounds );
};

class SAL_DLLPRIVATE JavaSalGraphicsDrawPathOp : public JavaSalGraphicsOp
{
	bool					mbAntialias;
	SalColor				mnFillColor;
	SalColor				mnLineColor;
	ULONG					mnPoints;
	CGPathRef				maPath;
	float					mfLineWidth;
	::basegfx::B2DLineJoin	meLineJoin;

public:
							JavaSalGraphicsDrawPathOp( const CGPathRef aNativeClipPath, bool bInvert, CGLayerRef aXORLayer, bool bAntialias, SalColor nFillColor, SalColor nLineColor, const CGPathRef aPath, float fLineWidth = 0, ::basegfx::B2DLineJoin eLineJoin = ::basegfx::B2DLINEJOIN_NONE );
	virtual					~JavaSalGraphicsDrawPathOp();

	virtual	void			drawOp( CGContextRef aContext, CGRect aBounds );
};

class SAL_DLLPRIVATE JavaSalGraphicsDrawRectOp : public JavaSalGraphicsOp
{
	CGRect					maRect;
	SalColor				mnFillColor;
	SalColor				mnLineColor;

public:
							JavaSalGraphicsDrawRectOp( const CGPathRef aNativeClipPath, bool bInvert, CGLayerRef aXORLayer, const CGRect aRect, SalColor nFillColor, SalColor nLineColor ) : JavaSalGraphicsOp( aNativeClipPath, bInvert, aXORLayer ), maRect( aRect ), mnFillColor( nFillColor ), mnLineColor( nLineColor ) {}
	virtual					~JavaSalGraphicsDrawRectOp() {}

	virtual	void			drawOp( CGContextRef aContext, CGRect aBounds );
};

using namespace osl;
using namespace vcl;

// =======================================================================

static void AddPolygonToPaths( com_sun_star_vcl_VCLPath *pVCLPath, CGMutablePathRef aCGPath, const ::basegfx::B2DPolygon& rPolygon, bool bClosePath )
{
	const sal_uInt32 nCount = rPolygon.count();
	if ( !nCount )
		return;

	const bool bHasCurves = rPolygon.areControlPointsUsed();
	bool bPendingCurve = false;
	sal_uInt32 nIndex = 0;
	sal_uInt32 nPreviousIndex = 0;
	for ( ; ; nPreviousIndex = nIndex++ )
	{
		sal_uInt32 nClosedIndex = nIndex;
		if( nIndex >= nCount )
		{
			// Prepare to close last curve segment if needed
			if( bClosePath && ( nIndex == nCount ) )
				nClosedIndex = 0;
			else
				break;
		}

		::basegfx::B2DPoint aPoint = rPolygon.getB2DPoint( nClosedIndex );

		if ( !nIndex )
		{
			if ( pVCLPath )
				pVCLPath->moveTo( aPoint.getX(), aPoint.getY() );
			if ( aCGPath )
				CGPathMoveToPoint( aCGPath, NULL, aPoint.getX(), aPoint.getY() );
		}
		else if ( !bPendingCurve )
		{
			if ( pVCLPath )
				pVCLPath->lineTo( aPoint.getX(), aPoint.getY() );
			if ( aCGPath )
				CGPathAddLineToPoint( aCGPath, NULL, aPoint.getX(), aPoint.getY() );
		}
		else
		{
			::basegfx::B2DPoint aFirstControlPoint = rPolygon.getNextControlPoint( nPreviousIndex );
			::basegfx::B2DPoint aSecondControlPoint = rPolygon.getPrevControlPoint( nClosedIndex );
			if ( pVCLPath )
				pVCLPath->curveTo( aFirstControlPoint.getX(), aFirstControlPoint.getY(), aSecondControlPoint.getX(), aSecondControlPoint.getY(), aPoint.getX(), aPoint.getY() );
			if ( aCGPath )
				CGPathAddCurveToPoint( aCGPath, NULL, aFirstControlPoint.getX(), aFirstControlPoint.getY(), aSecondControlPoint.getX(), aSecondControlPoint.getY(), aPoint.getX(), aPoint.getY() );
		}

		if ( bHasCurves )
			bPendingCurve = rPolygon.isNextControlPointUsed( nClosedIndex );
	}

	if ( bClosePath )
	{
		if ( pVCLPath )
			pVCLPath->closePath();
		if ( aCGPath )
			CGPathCloseSubpath( aCGPath );
	}
}

// -----------------------------------------------------------------------

static void AddPolyPolygonToPaths( com_sun_star_vcl_VCLPath *pVCLPath, CGMutablePathRef aCGPath, const ::basegfx::B2DPolyPolygon& rPolyPoly )
{
	const sal_uInt32 nCount = rPolyPoly.count();
	if ( !nCount )
		return;

	for ( sal_uInt32 i = 0; i < nCount; i++ )
	{
		const ::basegfx::B2DPolygon rPolygon = rPolyPoly.getB2DPolygon( i );
		AddPolygonToPaths( pVCLPath, aCGPath, rPolygon, true );
	}
}

// =======================================================================

CGColorRef CreateCGColorFromSalColor( SalColor nColor )
{
	return CGColorCreateGenericRGB( (float)( ( nColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( nColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( nColor & 0x000000ff ) / (float)0xff, (float)( ( nColor & 0xff000000 ) >> 24 ) / (float)0xff );
}

// =======================================================================

JavaSalGraphicsCopyLayerOp::JavaSalGraphicsCopyLayerOp( const CGPathRef aNativeClipPath, bool bInvert, CGLayerRef aXORLayer, CGLayerRef aSrcLayer, const CGPoint aSrcPoint, const CGRect aRect ) :
	JavaSalGraphicsOp( aNativeClipPath, bInvert, aXORLayer ),
	maSrcLayer( aSrcLayer ),
	maSrcPoint( aSrcPoint ),
	maRect( aRect )
{
	if ( maSrcLayer )
		CGLayerRetain( maSrcLayer );
}

// -----------------------------------------------------------------------

JavaSalGraphicsCopyLayerOp::~JavaSalGraphicsCopyLayerOp()
{
	if ( maSrcLayer )
		CGLayerRelease( maSrcLayer );
}

// -----------------------------------------------------------------------

void JavaSalGraphicsCopyLayerOp::drawOp( CGContextRef aContext, CGRect aBounds )
{
	if ( !aContext || !maSrcLayer )
		return;

	// Shrink destination to handle source over or underflow
	CGSize aLayerSize = CGLayerGetSize( maSrcLayer );
	CGRect aSrcRect = CGRectMake( maSrcPoint.x, maSrcPoint.y, maRect.size.width, maRect.size.height );
	if ( aSrcRect.origin.x < 0 )
	{
		aSrcRect.size.width += aSrcRect.origin.x;
		maRect.size.width += aSrcRect.origin.x;
		aSrcRect.origin.x = 0;
	}
	if ( aSrcRect.origin.y < 0 )
	{
		aSrcRect.size.height += aSrcRect.origin.y;
		maRect.size.height += aSrcRect.origin.y;
		aSrcRect.origin.y = 0;
	}
	if ( aSrcRect.size.width > aLayerSize.width - aSrcRect.origin.x )
	{
		aSrcRect.size.width = aLayerSize.width - aSrcRect.origin.x;
		maRect.size.width = aSrcRect.size.width;
	}
	if ( aSrcRect.size.height > aLayerSize.height - aSrcRect.origin.y )
	{
		aSrcRect.size.height = aLayerSize.height - aSrcRect.origin.y;
		maRect.size.height = aSrcRect.size.height;
	}
	if ( maRect.origin.x < 0 )
	{
		aSrcRect.origin.x -= maRect.origin.x;
		aSrcRect.size.width += maRect.origin.x;
		maRect.size.width += maRect.origin.x;
		maRect.origin.x = 0;
	}
	if ( maRect.origin.y < 0 )
	{
		aSrcRect.origin.y -= maRect.origin.y;
		aSrcRect.size.height += maRect.origin.y;
		maRect.size.height += maRect.origin.y;
		maRect.origin.y = 0;
	}
	if ( maRect.size.width <= 0 || maRect.size.height <= 0 )
		return;

	CGRect aDrawBounds = maRect;
	if ( !CGRectIsEmpty( aBounds ) )
		aDrawBounds = CGRectIntersection( aDrawBounds, aBounds );
	if ( maNativeClipPath )
		aDrawBounds = CGRectIntersection( aDrawBounds, CGPathGetBoundingBox( maNativeClipPath ) );
	if ( CGRectIsEmpty( aDrawBounds ) )
		return;

	aContext = saveClipXORGState( aContext, aDrawBounds );
	if ( !aContext )
		return;

	CGContextRef aSrcContext = CGLayerGetContext( maSrcLayer );
	if ( aSrcContext == aContext && maRect.origin.x < aSrcRect.origin.x )
	{
		// Drawing to a negative x destination causes drawing to wrap around
		// to the right edge of the destination layer so make a temporary
		// copy of the source
		CGLayerRef aTmpLayer = CGLayerCreateWithContext( aContext, maRect.size, NULL );
		if ( aTmpLayer )
		{
			CGContextRef aTmpContext = CGLayerGetContext( aTmpLayer );
			if ( aTmpContext )
			{
				CGContextDrawLayerAtPoint( aTmpContext, CGPointMake( aSrcRect.origin.x * -1, aSrcRect.origin.y * -1 ), maSrcLayer );

				CGContextClipToRect( aContext, maRect );
				CGContextDrawLayerAtPoint( aContext, maRect.origin, aTmpLayer );
			}

			CGLayerRelease( aTmpLayer );
		}
	}
	else
	{
		CGContextClipToRect( aContext, maRect );
		CGContextDrawLayerAtPoint( aContext, CGPointMake( maRect.origin.x - aSrcRect.origin.x, maRect.origin.y - aSrcRect.origin.y ), maSrcLayer );
	}

	restoreClipXORGState();
}

// =======================================================================

JavaSalGraphicsDrawEPSOp::JavaSalGraphicsDrawEPSOp( const CGPathRef aNativeClipPath, CFDataRef aData, const CGRect aRect ) :
	JavaSalGraphicsOp( aNativeClipPath ),
	maData( aData ),
	maRect( aRect )
{
	if ( maData )
		CFRetain( maData );
}

// -----------------------------------------------------------------------

JavaSalGraphicsDrawEPSOp::~JavaSalGraphicsDrawEPSOp()
{
	if ( maData )
		CFRelease( maData );
}

// -----------------------------------------------------------------------

void JavaSalGraphicsDrawEPSOp::drawOp( CGContextRef aContext, CGRect aBounds )
{
	if ( !aContext || !maData )
		return;

	CGRect aDrawBounds = maRect;
	if ( !CGRectIsEmpty( aBounds ) )
		aDrawBounds = CGRectIntersection( aDrawBounds, aBounds );
	if ( maNativeClipPath )
		aDrawBounds = CGRectIntersection( aDrawBounds, CGPathGetBoundingBox( maNativeClipPath ) );
	if ( CGRectIsEmpty( aDrawBounds ) )
		return;

	aContext = saveClipXORGState( aContext, aDrawBounds );
	if ( !aContext )
		return;

	// CGImage's assume flipped coordinates when drawing so draw from the
	// bottom up
	NSImageRep *pImageRep = [NSEPSImageRep imageRepWithData:(NSData *)maData];
	if ( !pImageRep )
		pImageRep = [NSPDFImageRep imageRepWithData:(NSData *)maData];
	if ( pImageRep )
	{
		NSGraphicsContext *pContext = [NSGraphicsContext graphicsContextWithGraphicsPort:aContext flipped:NO];
		if ( pContext )
		{
			
			NSGraphicsContext *pOldContext = [NSGraphicsContext currentContext];
			[NSGraphicsContext setCurrentContext:pContext];
			[pImageRep drawInRect:NSMakeRect( maRect.origin.x, maRect.origin.y + maRect.size.height, maRect.size.width, maRect.size.height * -1 )];
			[NSGraphicsContext setCurrentContext:pOldContext];
		}
	}

	restoreClipXORGState();
}

// =======================================================================

void JavaSalGraphicsDrawLineOp::drawOp( CGContextRef aContext, CGRect aBounds )
{
	if ( !aContext )
		return;

	CGRect aDrawBounds = CGRectMake( mfX1, mfY1, mfX2 - mfX1, mfY2 - mfY1 );
	if ( !CGRectIsEmpty( aBounds ) )
		aDrawBounds = CGRectIntersection( aDrawBounds, aBounds );
	if ( maNativeClipPath )
		aDrawBounds = CGRectIntersection( aDrawBounds, CGPathGetBoundingBox( maNativeClipPath ) );
	if ( CGRectIsEmpty( aDrawBounds ) )
		return;

	CGColorRef aColor = CreateCGColorFromSalColor( mnColor );
	if ( aColor )
	{
		if ( CGColorGetAlpha( aColor ) )
		{
			aContext = saveClipXORGState( aContext, aDrawBounds );
			if ( aContext )
			{
				CGContextSetStrokeColorWithColor( aContext, aColor );
				CGContextMoveToPoint( aContext, mfX1, mfY1 );
				CGContextAddLineToPoint( aContext, mfX2, mfY2 );
				CGContextStrokePath( aContext );

				restoreClipXORGState();
			}
		}

		CGColorRelease( aColor );
	}
}

// =======================================================================

JavaSalGraphicsDrawPathOp::JavaSalGraphicsDrawPathOp( const CGPathRef aNativeClipPath, bool bInvert, CGLayerRef aXORLayer, bool bAntialias, SalColor nFillColor, SalColor nLineColor, const CGPathRef aPath, float fLineWidth, ::basegfx::B2DLineJoin eLineJoin ) :
	JavaSalGraphicsOp( aNativeClipPath, bInvert, aXORLayer ),
	mbAntialias( bAntialias ),
	mnFillColor( nFillColor ),
	mnLineColor( nLineColor ),
	maPath( NULL ),
	mfLineWidth( fLineWidth ),
	meLineJoin( eLineJoin )
{
	if ( aPath )
		maPath = CGPathCreateCopy( aPath );
}

// -----------------------------------------------------------------------

JavaSalGraphicsDrawPathOp::~JavaSalGraphicsDrawPathOp()
{
	if ( maPath )
		CGPathRelease( maPath );
}

// -----------------------------------------------------------------------

void JavaSalGraphicsDrawPathOp::drawOp( CGContextRef aContext, CGRect aBounds )
{
	if ( !aContext || !maPath )
		return;

	CGRect aDrawBounds = CGPathGetBoundingBox( maPath );
	if ( !CGRectIsEmpty( aBounds ) )
		aDrawBounds = CGRectIntersection( aDrawBounds, aBounds );
	if ( maNativeClipPath )
		aDrawBounds = CGRectIntersection( aDrawBounds, CGPathGetBoundingBox( maNativeClipPath ) );
	if ( CGRectIsEmpty( aDrawBounds ) )
		return;

	CGColorRef aFillColor = CreateCGColorFromSalColor( mnFillColor );
	if ( aFillColor )
	{
		CGColorRef aLineColor = CreateCGColorFromSalColor( mnLineColor );
		if ( aLineColor )
		{
			aContext = saveClipXORGState( aContext, aDrawBounds );
			if ( aContext )
			{
				// Set line width
				if ( mfLineWidth > 0 )
					CGContextSetLineWidth( aContext, mfLineWidth );

				// Set line join
				CGLineJoin nJoin = kCGLineJoinMiter;
				switch ( meLineJoin )
				{
					case ::basegfx::B2DLINEJOIN_BEVEL:
						nJoin = kCGLineJoinBevel;
						break;
					case ::basegfx::B2DLINEJOIN_ROUND:
						nJoin = kCGLineJoinRound;
						break;
					default:
						break;
				}
				CGContextSetLineJoin( aContext, nJoin );

				// Enable or disable antialiasing
				CGContextSetAllowsAntialiasing( aContext, mbAntialias );

				// Smooth out image drawing for bug 2475 image
				if ( maXORLayer && !mbAntialias )
					CGContextSetAllowsAntialiasing( aContext, true );

				CGContextAddPath( aContext, maPath );
				if ( CGColorGetAlpha( aFillColor ) )
				{
					CGContextSetFillColorWithColor( aContext, aFillColor );
					CGContextEOFillPath( aContext );
				}
				if ( CGColorGetAlpha( aLineColor ) )
				{
					CGContextSetStrokeColorWithColor( aContext, aLineColor );
					CGContextStrokePath( aContext );
				}

				CGContextSetAllowsAntialiasing( aContext, mbAntialias );

				restoreClipXORGState();
			}

			CGColorRelease( aLineColor );
		}

		CGColorRelease( aFillColor );
	}
}

// =======================================================================

void JavaSalGraphicsDrawRectOp::drawOp( CGContextRef aContext, CGRect aBounds )
{
	if ( !aContext )
		return;

	CGRect aDrawBounds = maRect;
	if ( !CGRectIsEmpty( aBounds ) )
		aDrawBounds = CGRectIntersection( aDrawBounds, aBounds );
	if ( maNativeClipPath )
		aDrawBounds = CGRectIntersection( aDrawBounds, CGPathGetBoundingBox( maNativeClipPath ) );
	if ( CGRectIsEmpty( aDrawBounds ) )
		return;

	CGColorRef aFillColor = CreateCGColorFromSalColor( mnFillColor );
	if ( aFillColor )
	{
		CGColorRef aLineColor = CreateCGColorFromSalColor( mnLineColor );
		if ( aLineColor )
		{
			if ( CGColorGetAlpha( aFillColor ) || CGColorGetAlpha( aLineColor ) )
			{
				aContext = saveClipXORGState( aContext, aDrawBounds );
				if ( aContext )
				{
					if ( CGColorGetAlpha( aFillColor ) )
					{
						CGContextSetFillColorWithColor( aContext, aFillColor );
						CGContextFillRect( aContext, maRect );
					}
					if ( CGColorGetAlpha( aLineColor ) )
					{
						CGContextSetStrokeColorWithColor( aContext, aLineColor );
						CGContextStrokeRect( aContext, maRect );
					}

					restoreClipXORGState();
				}
			}

			CGColorRelease( aLineColor );
		}

		CGColorRelease( aFillColor );
	}
}

// =======================================================================

JavaSalGraphics::JavaSalGraphics() :
	maLayer( NULL ),
	mnFillColor( MAKE_SALCOLOR( 0xff, 0xff, 0xff ) | 0xff000000 ),
	mnLineColor( MAKE_SALCOLOR( 0, 0, 0 ) | 0xff000000 ),
	mnTextColor( MAKE_SALCOLOR( 0, 0, 0 ) | 0xff000000 ),
	mnFillTransparency( 0xff000000 ),
	mnLineTransparency( 0xff000000 ),
	mpFrame( NULL ),
	mpPrinter( NULL ),
	mpVirDev( NULL ),
	mpVCLGraphics( NULL ),
	mpFontData( NULL ),
	mpVCLFont( NULL ),
	mnFontFamily( FAMILY_DONTKNOW ),
	mnFontWeight( WEIGHT_DONTKNOW ),
	mnFontPitch( PITCH_DONTKNOW ),
	mnDPIX( 0 ),
	mnDPIY( 0 ),
	maNativeClipPath( NULL ),
	mbInvert( false ),
	mbXOR( false ),
	meOrientation( ORIENTATION_PORTRAIT ),
	mbPaperRotated( sal_False )
{
	GetSalData()->maGraphicsList.push_back( this );
}

// -----------------------------------------------------------------------

JavaSalGraphics::~JavaSalGraphics()
{
	GetSalData()->maGraphicsList.remove( this );

	while ( maUndrawnNativeOpsList.size() )
	{
		JavaSalGraphicsOp *pOp = maUndrawnNativeOpsList.front();
		maUndrawnNativeOpsList.pop_front();
		delete pOp;
	}

	// Notify graphics change listeners
	while ( maGraphicsChangeListenerList.size() )
	{
		JavaSalBitmap *pBitmap = maGraphicsChangeListenerList.front();
		maGraphicsChangeListenerList.pop_front();
		pBitmap->NotifyGraphicsChanged( true );
	}

	if ( maLayer )
		CGLayerRelease( maLayer );

	if ( mpVCLGraphics )
		delete mpVCLGraphics;

	if ( mpFontData )
		delete mpFontData;

	if ( mpVCLFont )
		delete mpVCLFont;

	for ( ::std::hash_map< int, com_sun_star_vcl_VCLFont* >::const_iterator it = maFallbackFonts.begin(); it != maFallbackFonts.end(); ++it )
		delete it->second;

	if ( maNativeClipPath )
		CFRelease( maNativeClipPath );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
	if ( !mnDPIX || !mnDPIY )
	{
		Size aSize( mpVCLGraphics->getResolution() );
		mnDPIX = aSize.Width();
		mnDPIY = aSize.Height();
	}

	rDPIX = mnDPIX;
	rDPIY = mnDPIY;
}

// -----------------------------------------------------------------------

USHORT JavaSalGraphics::GetBitCount()
{
	if ( mpVCLGraphics )
		return mpVCLGraphics->getBitCount();
	else
		return 32;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::ResetClipRegion()
{
	if ( mpPrinter || useNativeDrawing() )
	{
		if ( maNativeClipPath )
		{
			CFRelease( maNativeClipPath );
			maNativeClipPath = NULL;
		}
	}
	else if ( mpVCLGraphics )
	{
		mpVCLGraphics->resetClipRegion( sal_False );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::BeginSetClipRegion( ULONG nRectCount )
{
	if ( mpPrinter || useNativeDrawing() )
		ResetClipRegion();
	else if ( mpVCLGraphics )
		mpVCLGraphics->beginSetClipRegion( sal_False );
}

// -----------------------------------------------------------------------

BOOL JavaSalGraphics::unionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
	BOOL bRet = TRUE;

	if ( mpPrinter || useNativeDrawing() )
	{
		if ( nWidth > 0 && nHeight > 0 )
		{
			if ( !maNativeClipPath )
				maNativeClipPath = CGPathCreateMutable();

			if ( maNativeClipPath )
				CGPathAddRect( maNativeClipPath, NULL, CGRectMake( (float)nX, (float)nY, (float)nWidth, (float)nHeight ) );
		}
	}
	else if ( mpVCLGraphics )
	{
		mpVCLGraphics->unionClipRegion( nX, nY, nWidth, nHeight, sal_False );
	}

	return bRet;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::unionClipRegion( const ::basegfx::B2DPolyPolygon& rPolyPoly )
{
	bool bRet = true;

	const sal_uInt32 nPoly = rPolyPoly.count();
	if ( nPoly )
	{
		if ( mpPrinter || useNativeDrawing() )
		{
			if ( !maNativeClipPath )
				maNativeClipPath = CGPathCreateMutable();

			if ( maNativeClipPath )
			{
				CGMutablePathRef aCGPath = CGPathCreateMutable();
				AddPolyPolygonToPaths( NULL, aCGPath, rPolyPoly );
				CGPathAddPath( maNativeClipPath, NULL, aCGPath );
				CFRelease( aCGPath );
			}
		}
		else
		{
			com_sun_star_vcl_VCLPath aPath;
			AddPolyPolygonToPaths( &aPath, NULL, rPolyPoly );
			mpVCLGraphics->unionClipPath( &aPath, sal_False );
		}
	}

	return bRet;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::EndSetClipRegion()
{
	if ( !mpPrinter && !useNativeDrawing() )
		mpVCLGraphics->endSetClipRegion( sal_False );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::SetLineColor()
{
	mnLineColor = 0x00000000;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::SetLineColor( SalColor nSalColor )
{
	mnLineColor = nSalColor | mnLineTransparency;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::SetFillColor()
{
	mnFillColor = 0x00000000;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::SetFillColor( SalColor nSalColor )
{
	mnFillColor = nSalColor | mnFillTransparency;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::SetXORMode( bool bSet, bool bInvertOnly )
{
	// Don't do anything if this is a printer
	if ( mpPrinter )
		bSet = false;

	if ( useNativeDrawing() )
	{
		if ( bSet && bInvertOnly )
		{
			mbInvert = true;
			mbXOR = false;
		}
		else
		{
			mbInvert = false;
			mbXOR = bSet;
		}
	}
	else if ( mpVCLGraphics )
	{
		// Ignore the bInvertOnly parameter as it is not used by Windows or X11
		// platforms
		mpVCLGraphics->setXORMode( bSet );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
	if ( nROPColor == SAL_ROP_0 )
		SetLineColor( MAKE_SALCOLOR( 0, 0, 0 ) );
	else
		SetLineColor( MAKE_SALCOLOR( 0xff, 0xff, 0xff ) );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
	if ( nROPColor == SAL_ROP_0 )
		SetFillColor( MAKE_SALCOLOR( 0, 0, 0 ) );
	else
		SetFillColor( MAKE_SALCOLOR( 0xff, 0xff, 0xff ) );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPixel( long nX, long nY )
{
	if ( mnLineColor )
	{
		if ( useNativeDrawing() )
			addUndrawnNativeOp( new JavaSalGraphicsDrawRectOp( maNativeClipPath, mbInvert, mbXOR ? maLayer : NULL, CGRectMake( nX, nY, 1, 1 ), 0x00000000, mnLineColor ) );
		else if ( mpVCLGraphics )
			mpVCLGraphics->setPixel( nX, nY, mnLineColor, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
	if ( useNativeDrawing() )
		addUndrawnNativeOp( new JavaSalGraphicsDrawRectOp( maNativeClipPath, mbInvert, mbXOR ? maLayer : NULL, CGRectMake( nX, nY, 1, 1 ), 0x00000000, nSalColor | 0xff000000 ) );
	else if ( mpVCLGraphics )
		mpVCLGraphics->setPixel( nX, nY, nSalColor | 0xff000000, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
	if ( mnLineColor )
	{
		if ( useNativeDrawing() )
			addUndrawnNativeOp( new JavaSalGraphicsDrawLineOp( maNativeClipPath, mbInvert, mbXOR ? maLayer : NULL, (float)nX1, (float)nY1, (float)nX2, (float)nY2, mnLineColor ) );
		else if ( mpVCLGraphics )
			mpVCLGraphics->drawLine( nX1, nY1, nX2, nY2, mnLineColor, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
	if ( useNativeDrawing() )
	{
		CGRect aRect = CGRectStandardize( CGRectMake( nX, nY, nWidth, nHeight ) );
		if ( CGRectIsEmpty( aRect ) )
		{
			drawLine( nX, nY, nX + nWidth, nY + nHeight );
			return;
		}

		addUndrawnNativeOp( new JavaSalGraphicsDrawRectOp( maNativeClipPath, mbInvert, mbXOR ? maLayer : NULL, aRect, mnFillColor, mnLineColor ) );
	}
	else
	{
		if ( mnFillColor )
			mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, mnFillColor, TRUE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
		if ( mnLineColor )
			mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, mnLineColor, FALSE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
	}
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency )
{
	setLineTransparency( nTransparency );
	setFillTransparency( nTransparency );

	drawRect( nX, nY, nWidth, nHeight );

	setLineTransparency( 0 );
	setFillTransparency( 0 );

	return true;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolyLine( ULONG nPoints, const SalPoint* pPtAry )
{
	if ( useNativeDrawing() )
	{
		if ( nPoints && pPtAry )
		{
			::basegfx::B2DPolygon aPoly;
			for ( ULONG i = 0 ; i < nPoints; i++ )
				aPoly.append( ::basegfx::B2DPoint( pPtAry[ i ].mnX, pPtAry[ i ].mnY ) );
			aPoly.removeDoublePoints();

			CGMutablePathRef aPath = CGPathCreateMutable();
			if ( aPath )
			{
				AddPolygonToPaths( NULL, aPath, aPoly, aPoly.isClosed() );
				addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbInvert, mbXOR ? maLayer : NULL, false, 0x00000000, mnLineColor, aPath ) );
				CGPathRelease( aPath );
			}
		}
	}
	else if ( mnLineColor )
	{
		mpVCLGraphics->drawPolyline( nPoints, pPtAry, mnLineColor, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
	if ( useNativeDrawing() )
	{
		if ( nPoints && pPtAry )
		{
			::basegfx::B2DPolygon aPoly;
			for ( ULONG i = 0 ; i < nPoints; i++ )
				aPoly.append( ::basegfx::B2DPoint( pPtAry[ i ].mnX, pPtAry[ i ].mnY ) );
			aPoly.removeDoublePoints();
			aPoly.setClosed( true );

			CGMutablePathRef aPath = CGPathCreateMutable();
			if ( aPath )
			{
				AddPolygonToPaths( NULL, aPath, aPoly, aPoly.isClosed() );
				addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbInvert, mbXOR ? maLayer : NULL, false, mnFillColor, mnLineColor, aPath ) );
				CGPathRelease( aPath );
			}
		}
	}
	else
	{
		if ( mnFillColor )
			mpVCLGraphics->drawPolygon( nPoints, pPtAry, mnFillColor, TRUE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
		if ( mnLineColor )
			mpVCLGraphics->drawPolygon( nPoints, pPtAry, mnLineColor, FALSE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolyPolygon( ULONG nPoly, const ULONG* pPoints, PCONSTSALPOINT* pPtAry )
{
	if ( useNativeDrawing() )
	{
		if ( nPoly && pPoints && pPtAry )
		{
			::basegfx::B2DPolyPolygon aPolyPoly;
			for ( ULONG i = 0 ; i < nPoly; i++ )
			{
				PCONSTSALPOINT pPolyPtAry = pPtAry[ i ];
				if ( pPolyPtAry )
				{
					::basegfx::B2DPolygon aPoly;
					for ( ULONG j = 0 ; j < pPoints[ i ]; j++ )
						aPoly.append( ::basegfx::B2DPoint( pPolyPtAry[ j ].mnX, pPolyPtAry[ j ].mnY ) );
					aPoly.setClosed( true );
					aPoly.removeDoublePoints();
					aPolyPoly.append( aPoly );
				}
			}

			CGMutablePathRef aPath = CGPathCreateMutable();
			if ( aPath )
			{
				AddPolyPolygonToPaths( NULL, aPath, aPolyPoly );
				addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbInvert, mbXOR ? maLayer : NULL, false, mnFillColor, mnLineColor, aPath ) );
				CGPathRelease( aPath );
			}
		}
	}
	else
	{
		if ( mnFillColor )
			mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pPtAry, mnFillColor, TRUE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
		if ( mnLineColor )
			mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pPtAry, mnLineColor, FALSE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
	}
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly, double fTransparency )
{
	bool bRet = true;

	if ( useNativeDrawing() )
	{
		CGMutablePathRef aPath = CGPathCreateMutable();
		if ( aPath )
		{
			sal_uInt8 nTransparency = (sal_uInt8)( ( fTransparency * 100 ) + 0.5 );
			setFillTransparency( nTransparency );
			setLineTransparency( nTransparency );
			AddPolyPolygonToPaths( NULL, aPath, rPolyPoly );
			addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbInvert, mbXOR ? maLayer : NULL, getAntiAliasB2DDraw(), mnFillColor, mnLineColor, aPath ) );
			setFillTransparency( 0 );
			setLineTransparency( 0 );
			CGPathRelease( aPath );
		}
	}
	else if ( mpVCLGraphics )
	{
		const sal_uInt32 nPoly = rPolyPoly.count();
		if ( nPoly && ( mnFillColor || mnLineColor ) )
		{
			com_sun_star_vcl_VCLPath aPath;
			CGMutablePathRef aCGPath = NULL;
			if ( mpPrinter )
				aCGPath = CGPathCreateMutable();
			AddPolyPolygonToPaths( &aPath, aCGPath, rPolyPoly );

			sal_uInt8 nTransparency = (sal_uInt8)( ( fTransparency * 100 ) + 0.5 );
			setFillTransparency( nTransparency );
			setLineTransparency( nTransparency );
			if ( mnFillColor )
				mpVCLGraphics->drawPath( &aPath, mnFillColor, TRUE, getAntiAliasB2DDraw(), aCGPath, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
			if ( mnLineColor )
				mpVCLGraphics->drawPath( &aPath, mnFillColor, FALSE, getAntiAliasB2DDraw(), aCGPath, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
			setFillTransparency( 0 );
			setLineTransparency( 0 );
		}
	}

	return bRet;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawPolyLine( const ::basegfx::B2DPolygon& rPoly, const ::basegfx::B2DVector& rLineWidths, ::basegfx::B2DLineJoin eLineJoin )
{
	bool bRet = true;

	if ( useNativeDrawing() )
	{
		CGMutablePathRef aPath = CGPathCreateMutable();
		if ( aPath )
		{
			AddPolygonToPaths( NULL, aPath, rPoly, rPoly.isClosed() );
			addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbInvert, mbXOR ? maLayer : NULL, getAntiAliasB2DDraw(), 0x00000000, mnLineColor, aPath, rLineWidths.getX(), eLineJoin ) );
			CGPathRelease( aPath );
		}
	}
	else if ( mpPrinter )
	{
		bRet = false;
	}
	else if ( mpVCLGraphics )
	{
		const sal_uInt32 nCount= rPoly.count();
		if ( nCount && mnLineColor )
		{
			com_sun_star_vcl_VCLPath aPath;
			CGMutablePathRef aCGPath = NULL;
			if ( mpPrinter )
				aCGPath = CGPathCreateMutable();
			AddPolygonToPaths( &aPath, aCGPath, rPoly, rPoly.isClosed() );

			mpVCLGraphics->drawPathline( &aPath, mnLineColor, getAntiAliasB2DDraw(), rLineWidths.getX(), eLineJoin, aCGPath, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
		}
	}

	return bRet;
}

// -----------------------------------------------------------------------

sal_Bool JavaSalGraphics::drawPolyLineBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
	return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool JavaSalGraphics::drawPolygonBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
	return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool JavaSalGraphics::drawPolyPolygonBezier( ULONG nPoly, const ULONG* nPoints, const SalPoint* const* pPtAry, const BYTE* const* pFlgAry )
{
	return sal_False;
}

// -----------------------------------------------------------------------

BOOL JavaSalGraphics::drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize )
{
	BOOL bRet = FALSE;

	if ( pPtr && nSize )
	{
		if ( mpPrinter || useNativeDrawing() )
		{
			void *pPtrCopy = rtl_allocateMemory( nSize );
			if ( pPtrCopy )
			{
				memcpy( pPtrCopy, pPtr, nSize );

				if ( useNativeDrawing() )
				{
					// Assign ownership of bits to a CFData instance
					CFDataRef aData = CFDataCreateWithBytesNoCopy( NULL, (UInt8 *)pPtrCopy, nSize, NULL );
					if ( aData )
					{
						addUndrawnNativeOp( new JavaSalGraphicsDrawEPSOp( maNativeClipPath, aData, CGRectMake( nX, nY, nWidth, nHeight ) ) );
						CFRelease( aData );
					}
					bRet = TRUE;
				}
				else if ( mpVCLGraphics )
				{
					// Don't delete the copied buffer and let the Java native
					// method print the buffer directly
					mpVCLGraphics->drawEPS( pPtrCopy, nSize, nX, nY, nWidth, nHeight, maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
					bRet = TRUE;
				}
			}
		}

		if ( !bRet && mpVCLGraphics )
		{
			com_sun_star_vcl_VCLBitmap aVCLBitmap( nWidth, nHeight, 32 );
			if ( aVCLBitmap.getJavaObject() )
			{
				java_lang_Object *pData = aVCLBitmap.getData();
				if ( pData )
				{
					VCLThreadAttach t;
					if ( t.pEnv )
					{
						jboolean bCopy( sal_False );
						jint *pBits = (jint *)t.pEnv->GetPrimitiveArrayCritical( (jintArray)pData->getJavaObject(), &bCopy );
						if ( pBits )
						{
							bRet = NSEPSImageRep_drawInBitmap( pPtr, nSize, (int *)pBits, nWidth, nHeight );
							if ( bRet )
							{
								t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)pData->getJavaObject(), pBits, 0 );
								mpVCLGraphics->drawBitmap( &aVCLBitmap, 0, 0, nWidth, nHeight, nX, nY, nWidth, nHeight, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
							}
							else
							{
								t.pEnv->ReleasePrimitiveArrayCritical( (jintArray)pData->getJavaObject(), pBits, JNI_ABORT );
							}
						}
					}

					delete pData;
				}

				aVCLBitmap.dispose();
			}
		}
	}

	return bRet;
}

// -----------------------------------------------------------------------

long JavaSalGraphics::GetGraphicsWidth() const
{
	if ( mpFrame )
		return mpFrame->maGeometry.nWidth;
	else
		return 0;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::setLineTransparency( sal_uInt8 nTransparency )
{
	if ( nTransparency > 100 )
		nTransparency = 100;
	mnLineTransparency = ( ( (SalColor)( 100 - nTransparency ) * 0xff ) / 100 ) << 24;

	// Reset current color. Fix bug 2692 by not resetting when the color is
	// already transparent.
	if ( mnLineColor )
		SetLineColor( mnLineColor & 0x00ffffff );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::setFillTransparency( sal_uInt8 nTransparency )
{
	if ( nTransparency > 100 )
		nTransparency = 100;
	mnFillTransparency = ( ( (SalColor)( 100 - nTransparency ) * 0xff ) / 100 ) << 24;

	// Reset current color. Fix bug 2692 by not resetting when the color is
	// already transparent.
	if ( mnFillColor )
		SetFillColor( mnFillColor & 0x00ffffff );
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::useNativeDrawing()
{
	bool bRet = false;

	if ( !mpVCLGraphics )
	{
		if ( mpPrinter )
#ifdef USE_NATIVE_PRINTING
			bRet = true;
#else	// USE_NATIVE_PRINTING
			bRet = false;
#endif	// USE_NATIVE_PRINTING
		else if ( maLayer )
			bRet = true;
	}

	return bRet;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::addGraphicsChangeListener( JavaSalBitmap *pBitmap )
{
	if ( !pBitmap )
		return;

	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	maGraphicsChangeListenerList.push_back( pBitmap );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::addUndrawnNativeOp( JavaSalGraphicsOp *pOp )
{
	if ( !pOp )
		return;

	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	maUndrawnNativeOpsList.push_back( pOp );

	if ( maLayer )
	{
		CGContextRef aContext = CGLayerGetContext( maLayer );
		if ( aContext )
		{
			CGSize aSize = CGLayerGetSize( maLayer );
			drawUndrawnNativeOps( aContext, CGRectMake( 0, 0, aSize.width, aSize.height ) );
		}
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::copyFromGraphics( JavaSalGraphics *pSrcGraphics, CGPoint aSrcPoint, CGRect aDestRect, bool bAllowXOR )
{
	if ( !pSrcGraphics || !maLayer )
		return;

	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	CGContextRef aContext = CGLayerGetContext( maLayer );
	if ( aContext )
	{
		// Draw any undrawn operations so that we copy the latest bits
		CGSize aLayerSize = CGLayerGetSize( maLayer );
		CGRect aLayerBounds = CGRectMake( 0, 0, aLayerSize.width, aLayerSize.height );
		drawUndrawnNativeOps( aContext, aLayerBounds );

		CGContextSaveGState( aContext );

		pSrcGraphics->copyToContext( maNativeClipPath, mbInvert && bAllowXOR ? true : false, mbXOR && bAllowXOR ? maLayer : NULL, aContext, aLayerBounds, aSrcPoint, aDestRect );

		CGContextRestoreGState( aContext );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::copyToContext( const CGPathRef aNativeClipPath, bool bInvert, CGLayerRef aXORLayer, CGContextRef aDestContext, CGRect aDestBounds, CGPoint aSrcPoint, CGRect aDestRect )
{
	if ( !aDestContext || !maLayer )
		return;

	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	// Draw any undrawn operations so that we copy the latest bits
	CGSize aLayerSize = CGLayerGetSize( maLayer );
	drawUndrawnNativeOps( aDestContext, CGRectMake( 0, 0, aLayerSize.width, aLayerSize.height ) );

	// Do not queue this operation since we are copying to another context
	JavaSalGraphicsCopyLayerOp aOp( aNativeClipPath, bInvert, aXORLayer, maLayer, aSrcPoint, aDestRect );
	aOp.drawOp( aDestContext, aDestBounds );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawUndrawnNativeOps( CGContextRef aContext, CGRect aBounds )
{
	if ( !aContext )
		return;

	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	// Notify graphics change listeners
	while ( maGraphicsChangeListenerList.size() )
	{
		JavaSalBitmap *pBitmap = maGraphicsChangeListenerList.front();
		maGraphicsChangeListenerList.pop_front();
		pBitmap->NotifyGraphicsChanged( false );
	}

	CGContextSaveGState( aContext );

	// Scale printer context to match OOo resolution
	if ( mpPrinter )
	{
		long nDPIX;
		long nDPIY;
		GetResolution( nDPIX, nDPIY );
		if ( nDPIX && nDPIY )
		{
			float fScaleX = (float)72 / nDPIX;
			float fScaleY = (float)72 / nDPIY;
			CGContextScaleCTM( aContext, fScaleX, fScaleY );
			if ( !CGRectIsEmpty( aBounds ) )
				aBounds = CGRectMake( aBounds.origin.x / fScaleX, aBounds.origin.y / fScaleY, aBounds.size.width / fScaleX, aBounds.size.height / fScaleY );
		}
	}

	// Scale line width
	CGContextSetLineWidth( aContext, getNativeLineWidth() );

	while ( maUndrawnNativeOpsList.size() )
	{
		JavaSalGraphicsOp *pOp = maUndrawnNativeOpsList.front();
		maUndrawnNativeOpsList.pop_front();
		pOp->drawOp( aContext, aBounds );
		delete pOp;
	}

	CGContextRestoreGState( aContext );
}

// -----------------------------------------------------------------------

ULONG JavaSalGraphics::getBitmapDirectionFormat()
{
	if ( useNativeDrawing() )
		return JavaSalBitmap::GetNativeDirectionFormat();
	else
		return BMP_FORMAT_TOP_DOWN;
}

// -----------------------------------------------------------------------

float JavaSalGraphics::getNativeLineWidth()
{
	if ( mpPrinter )
		return (float)MIN_PRINTER_RESOLUTION / 72;
	else
		return 1.0f;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::removeGraphicsChangeListener( JavaSalBitmap *pBitmap )
{
	if ( !pBitmap )
		return;

	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	maGraphicsChangeListenerList.remove( pBitmap );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::setLayer( CGLayerRef aLayer )
{
	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	if ( aLayer != maLayer )
	{
		if ( maLayer )
			CGLayerRelease( maLayer );
		maLayer = aLayer;
		if ( maLayer )
			CGLayerRetain( maLayer );
	}
}

// =======================================================================

JavaSalGraphicsOp::JavaSalGraphicsOp( const CGPathRef aNativeClipPath, bool bInvert, CGLayerRef aXORLayer ) :
	maNativeClipPath( NULL ),
	mbInvert( bInvert ),
	maXORLayer( NULL ),
	maSavedContext( NULL ),
	mnBitmapCapacity( 0 ),
	mpDrawBits( NULL ),
	maDrawBitmapContext( NULL ),
	mpXORBits( NULL ),
	maXORBitmapContext( NULL ),
	maXORRect( CGRectNull )
{
	if ( aNativeClipPath )
		maNativeClipPath = CGPathCreateCopy( aNativeClipPath );

	// Inverting always takes precedence of XORing
	if ( !mbInvert )
	{
		maXORLayer = aXORLayer;
		if ( maXORLayer )
			CGLayerRetain( maXORLayer );
	}
}

// -----------------------------------------------------------------------

JavaSalGraphicsOp::~JavaSalGraphicsOp()
{
	restoreClipXORGState();

	if ( maNativeClipPath )
		CGPathRelease( maNativeClipPath );

	if ( maXORLayer )
		CGLayerRelease( maXORLayer );
}

// -----------------------------------------------------------------------

void JavaSalGraphicsOp::restoreClipXORGState()
{
	if ( maSavedContext )
	{
		// If there are XOR bitmaps, XOR them and then draw to this context
		if ( mnBitmapCapacity && mpDrawBits && maDrawBitmapContext && mpXORBits && maXORBitmapContext )
		{
			size_t nBitmapWidth = CGBitmapContextGetWidth( maDrawBitmapContext );
			size_t nBitmapHeight = CGBitmapContextGetHeight( maDrawBitmapContext );
			CGContextRelease( maDrawBitmapContext );
			maDrawBitmapContext = NULL;

			CGContextRelease( maXORBitmapContext );
			maXORBitmapContext = NULL;

			CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
			if ( aColorSpace )
			{
				size_t nPixels = mnBitmapCapacity / sizeof( sal_uInt32 );
				sal_uInt32 *pDrawBits = (sal_uInt32 *)mpDrawBits;
				sal_uInt32 *pXORBits = (sal_uInt32 *)mpXORBits;
				for ( size_t i = 0; i < nPixels; i++ )
				{
					if ( ( pXORBits[ i ] & 0xff000000 ) == 0xff000000 )
						pDrawBits[ i ] = ( pDrawBits[ i ] ^ pXORBits[ i ] ) | 0xff000000;
				}

				delete[] mpXORBits;
				mpXORBits = NULL;

				// Assign ownership of bits to a CGDataProvider instance
				CGDataProviderRef aProvider = CGDataProviderCreateWithData( NULL, mpDrawBits, mnBitmapCapacity, ReleaseBitmapBufferBytePointerCallback );
				if ( aProvider )
				{
					mpDrawBits = NULL;

					CGImageRef aImage = CGImageCreate( nBitmapWidth, nBitmapHeight, 8, 32, AlignedWidth4Bytes( 32 * nBitmapWidth ), aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little, aProvider, NULL, false, kCGRenderingIntentDefault );
					if ( aImage )
					{
						CGContextDrawImage( maSavedContext, CGRectMake( maXORRect.origin.x - XOR_BITMAP_BOUNDS_PADDING, maXORRect.origin.y - XOR_BITMAP_BOUNDS_PADDING, nBitmapWidth, nBitmapHeight ), aImage );
						CGImageRelease( aImage );
					}

					CGDataProviderRelease( aProvider );
				}
				else
				{
					delete[] mpDrawBits;
					mpDrawBits = NULL;
				}

				CGColorSpaceRelease( aColorSpace );
			}
		}

		CGContextRestoreGState( maSavedContext );
		CGContextRelease( maSavedContext );
		maSavedContext = NULL;
	}

	mnBitmapCapacity = 0;

	if ( maDrawBitmapContext )
	{
		CGContextRelease( maDrawBitmapContext );
		maDrawBitmapContext = NULL;
	}

	if ( mpDrawBits )
	{
		delete[] mpDrawBits;
		mpDrawBits = NULL;
	}

	if ( maXORBitmapContext )
	{
		CGContextRelease( maXORBitmapContext );
		maXORBitmapContext = NULL;
	}

	if ( mpXORBits )
	{
		delete[] mpXORBits;
		mpXORBits = NULL;
	}

	maXORRect = CGRectNull;
}

// -----------------------------------------------------------------------

CGContextRef JavaSalGraphicsOp::saveClipXORGState( CGContextRef aContext, CGRect aDrawBounds )
{
	restoreClipXORGState();

	if ( !aContext )
		return NULL;

	if ( maXORLayer )
	{
		// Mac OS X's XOR blend mode does not do real XORing of bits so we
		// reimplement our own XORing
		bool bXORDrawable = false;

		// Trust that the draw bounds has already been intersected against the
		// graphics bounds and clip
		maXORRect = CGRectStandardize( aDrawBounds );
		if ( !CGRectIsEmpty( maXORRect ) )
		{
			CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
			if ( aColorSpace )
			{
				CGSize aBitmapSize = CGSizeMake( maXORRect.size.width + ( XOR_BITMAP_BOUNDS_PADDING * 2 ), maXORRect.size.height + ( XOR_BITMAP_BOUNDS_PADDING * 2 ) );
				long nScanlineSize = AlignedWidth4Bytes( 32 * aBitmapSize.width );
				mnBitmapCapacity = nScanlineSize * aBitmapSize.height;
				try
				{
					mpDrawBits = new BYTE[ mnBitmapCapacity ];
					mpXORBits = new BYTE[ mnBitmapCapacity ];
				}
				catch( const std::bad_alloc& ) {}

				if ( mpDrawBits && mpXORBits )
				{
					memset( mpDrawBits, 0, mnBitmapCapacity );
					memset( mpXORBits, 0, mnBitmapCapacity );
					maDrawBitmapContext = CGBitmapContextCreate( mpDrawBits, aBitmapSize.width, aBitmapSize.height, 8, nScanlineSize, aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little );
					maXORBitmapContext = CGBitmapContextCreate( mpXORBits, aBitmapSize.width, aBitmapSize.height, 8, nScanlineSize, aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little );
					if ( maDrawBitmapContext && maXORBitmapContext )
					{
						// Turn off antialiasing by default since we did the
						// same in the Java code
						CGContextSetAllowsAntialiasing( maDrawBitmapContext, false );

						// Translate and clip the drawing context
						CGContextTranslateCTM( maDrawBitmapContext, XOR_BITMAP_BOUNDS_PADDING - maXORRect.origin.x, XOR_BITMAP_BOUNDS_PADDING - maXORRect.origin.y );

						if ( maNativeClipPath )
						{
							CGContextBeginPath( maDrawBitmapContext );
							CGContextAddPath( maDrawBitmapContext, maNativeClipPath );
							CGContextClip( maDrawBitmapContext );
						}

						// Copy layer to XOR context
						CGContextDrawLayerAtPoint( maXORBitmapContext, CGPointMake( XOR_BITMAP_BOUNDS_PADDING - maXORRect.origin.x, XOR_BITMAP_BOUNDS_PADDING - maXORRect.origin.y ), maXORLayer );

						bXORDrawable = true;
					}
				}

				CGColorSpaceRelease( aColorSpace );
			}
		}

		if ( !bXORDrawable )
		{
			restoreClipXORGState();
			return NULL;
		}
	}

	maSavedContext = aContext;
	CGContextRetain( maSavedContext );
	CGContextSaveGState( maSavedContext );

	if ( mbInvert )
		CGContextSetBlendMode( maSavedContext, kCGBlendModeDifference );

	// Turn off antialiasing by default since we did the same in the Java code
	CGContextSetAllowsAntialiasing( maSavedContext, false );

	if ( maNativeClipPath )
	{
		CGContextBeginPath( maSavedContext );
		CGContextAddPath( maSavedContext, maNativeClipPath );
		CGContextClip( maSavedContext );
	}

	// Throw away any incomplete path
	CGContextBeginPath( maSavedContext );

	return maDrawBitmapContext ? maDrawBitmapContext : maSavedContext;
}
