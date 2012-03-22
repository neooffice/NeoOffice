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

class SAL_DLLPRIVATE JavaSalGraphicsCopyLayerOp : public JavaSalGraphicsOp
{
	CGLayerRef				maSrcLayer;
	CGPoint					maSrcPoint;
	CGRect					maRect;

public:
JavaSalGraphicsCopyLayerOp::JavaSalGraphicsCopyLayerOp( const CGPathRef aNativeClipPath, bool bInvert, bool bXOR, CGLayerRef aSrcLayer, const CGPoint aSrcPoint, const CGRect aRect );
	virtual					~JavaSalGraphicsCopyLayerOp();

	virtual	void			drawOp( JavaSalGraphics *pGraphics, CGContextRef aContext, CGRect aBounds );
};

class SAL_DLLPRIVATE JavaSalGraphicsDrawEPSOp : public JavaSalGraphicsOp
{
	CFDataRef				maData;
	CGRect					maRect;

public:
							JavaSalGraphicsDrawEPSOp( const CGPathRef aNativeClipPath, CFDataRef aData, const CGRect aRect );
	virtual					~JavaSalGraphicsDrawEPSOp();

	virtual	void			drawOp( JavaSalGraphics *pGraphics, CGContextRef aContext, CGRect aBounds );
};

using namespace osl;
using namespace vcl;

// =======================================================================

void AddPolygonToPaths( com_sun_star_vcl_VCLPath *pVCLPath, CGMutablePathRef aCGPath, const ::basegfx::B2DPolygon& rPolygon, bool bClosePath )
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

void AddPolyPolygonToPaths( com_sun_star_vcl_VCLPath *pVCLPath, CGMutablePathRef aCGPath, const ::basegfx::B2DPolyPolygon& rPolyPoly )
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

JavaSalGraphicsCopyLayerOp::JavaSalGraphicsCopyLayerOp( const CGPathRef aNativeClipPath, bool bInvert, bool bXOR, CGLayerRef aSrcLayer, const CGPoint aSrcPoint, const CGRect aRect ) :
	JavaSalGraphicsOp( aNativeClipPath, bInvert, bXOR ),
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

void JavaSalGraphicsCopyLayerOp::drawOp( JavaSalGraphics *pGraphics, CGContextRef aContext, CGRect aBounds )
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

	aContext = saveClipXORGState( pGraphics, aContext, aDrawBounds );
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

void JavaSalGraphicsDrawEPSOp::drawOp( JavaSalGraphics *pGraphics, CGContextRef aContext, CGRect aBounds )
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

	aContext = saveClipXORGState( pGraphics, aContext, aDrawBounds );
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

JavaSalGraphicsDrawPathOp::JavaSalGraphicsDrawPathOp( const CGPathRef aNativeClipPath, bool bInvert, bool bXOR, bool bAntialias, SalColor nFillColor, SalColor nLineColor, const CGPathRef aPath, float fLineWidth, ::basegfx::B2DLineJoin eLineJoin, bool bLineDash ) :
	JavaSalGraphicsOp( aNativeClipPath, bInvert, bXOR, fLineWidth ),
	mbAntialias( bAntialias ),
	mnFillColor( nFillColor ),
	mnLineColor( nLineColor ),
	maPath( NULL ),
	meLineJoin( eLineJoin ),
	mbLineDash( bLineDash )
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

void JavaSalGraphicsDrawPathOp::drawOp( JavaSalGraphics *pGraphics, CGContextRef aContext, CGRect aBounds )
{
	if ( !aContext || !maPath )
		return;

	// Expand draw bounds by the line width
	float fNativeLineWidth = mfLineWidth;
	if ( fNativeLineWidth <= 0 )
		fNativeLineWidth = pGraphics->getNativeLineWidth();
	CGRect aDrawBounds = CGPathGetBoundingBox( maPath );
	aDrawBounds.origin.x -= fNativeLineWidth;
	aDrawBounds.origin.y -= fNativeLineWidth;
	aDrawBounds.size.width += fNativeLineWidth * 2;
	aDrawBounds.size.height += fNativeLineWidth * 2;
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
			aContext = saveClipXORGState( pGraphics, aContext, aDrawBounds );
			if ( aContext )
			{
				// Set line join
				switch ( meLineJoin )
				{
					case ::basegfx::B2DLINEJOIN_BEVEL:
						CGContextSetLineJoin( aContext, kCGLineJoinBevel );
						break;
					case ::basegfx::B2DLINEJOIN_ROUND:
						CGContextSetLineJoin( aContext, kCGLineJoinRound );
						break;
					default:
						break;
				}

				if ( mbLineDash )
				{
					CGFloat aLengths[ 2 ];
					aLengths[ 0 ] = 1;
					aLengths[ 1 ] = 1;
					CGContextSetLineDash( aContext, 0, aLengths, 2 );
				}

				CGContextAddPath( aContext, maPath );
				if ( CGColorGetAlpha( aFillColor ) )
				{
					// Smooth out image drawing for bug 2475 image
					CGContextSetAllowsAntialiasing( aContext, mbXOR || mbAntialias );

					CGContextSetFillColorWithColor( aContext, aFillColor );
					CGContextEOFillPath( aContext );
				}
				if ( CGColorGetAlpha( aLineColor ) )
				{
					// Enable or disable antialiasing
					CGContextSetAllowsAntialiasing( aContext, mbAntialias );

					CGContextSetStrokeColorWithColor( aContext, aLineColor );
					CGContextStrokePath( aContext );
				}

				restoreClipXORGState();
			}

			CGColorRelease( aLineColor );
		}

		CGColorRelease( aFillColor );
	}
}

// =======================================================================

void JavaSalGraphics::setContextDefaultSettings( CGContextRef aContext, CGPathRef aClipPath, float fLineWidth )
{
	if ( !aContext )
		return;

	if ( fLineWidth <= 0 )
		fLineWidth = 1.0f;

	// Scale line width, cap, and join. Note that the miter limit matches the
	// default miter limit specified in the Java 1.5 API BasicStroke class
	// documentation.
	CGContextSetLineWidth( aContext, fLineWidth );
	CGContextSetLineCap( aContext, kCGLineCapSquare );
	CGContextSetLineJoin( aContext, kCGLineJoinMiter );
	CGContextSetMiterLimit( aContext, 10.0 );

	// Turn off antialiasing by default since we did the same in the Java code
	CGContextSetAllowsAntialiasing( aContext, false );

	// Set clip
	if ( aClipPath )
	{
		CGContextBeginPath( aContext );
		CGContextAddPath( aContext, aClipPath );
		CGContextClip( aContext );
	}

	// Throw away any incomplete path
	CGContextBeginPath( aContext );
}

// -----------------------------------------------------------------------

JavaSalGraphics::JavaSalGraphics() :
	maLayer( NULL ),
	mnPixelContextData( 0 ),
	maPixelContext( NULL ),
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

	if ( maPixelContext )
		CGContextRelease( maPixelContext );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
	if ( ( !mnDPIX || !mnDPIY ) && mpVCLGraphics )
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
		{
			CGMutablePathRef aPath = CGPathCreateMutable();
			if ( aPath )
			{
				CGPathAddRect( aPath, NULL, CGRectMake( nX, nY, 1, 1 ) );
				addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbInvert, mbXOR, false, mnLineColor, 0x00000000, aPath ) );
				CGPathRelease( aPath );
			}
		}
		else if ( mpVCLGraphics )
			mpVCLGraphics->setPixel( nX, nY, mnLineColor, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
	if ( useNativeDrawing() )
	{
		CGMutablePathRef aPath = CGPathCreateMutable();
		if ( aPath )
		{
			CGPathAddRect( aPath, NULL, CGRectMake( nX, nY, 1, 1 ) );
			addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbInvert, mbXOR, false, nSalColor | 0xff000000, 0x00000000, aPath ) );
			CGPathRelease( aPath );
		}
	}
	else if ( mpVCLGraphics )
	{
		mpVCLGraphics->setPixel( nX, nY, nSalColor | 0xff000000, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
	if ( mnLineColor )
	{
		if ( useNativeDrawing() )
		{
			CGMutablePathRef aPath = CGPathCreateMutable();
			if ( aPath )
			{
				CGPathMoveToPoint( aPath, NULL, nX1, nY1 );
				CGPathAddLineToPoint( aPath, NULL, nX2, nY2 );
				addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbInvert, mbXOR, false, 0x00000000, mnLineColor, aPath ) );
				CGPathRelease( aPath );
			}
		}
		else if ( mpVCLGraphics )
		{
			mpVCLGraphics->drawLine( nX1, nY1, nX2, nY2, mnLineColor, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
		}
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
	if ( mnFillColor || mnLineColor )
	{
		if ( useNativeDrawing() )
		{
			CGMutablePathRef aPath = CGPathCreateMutable();
			if ( aPath )
			{
				CGRect aRect = CGRectStandardize( CGRectMake( nX, nY, nWidth, nHeight ) );
				CGPathAddRect( aPath, NULL, aRect );
				float fNativeLineWidth = getNativeLineWidth();
				if ( aRect.size.width < fNativeLineWidth )
					aRect.size.width = 0;
				if ( aRect.size.height < fNativeLineWidth )
					aRect.size.height = 0;
				if ( CGRectIsEmpty( aRect ) )
				{
					CGPathRelease( aPath );
					aPath = CGPathCreateMutable();
					if ( aPath )
					{
						CGPathMoveToPoint( aPath, NULL, aRect.origin.x, aRect.origin.y );
						CGPathAddLineToPoint( aPath, NULL, aRect.origin.x + aRect.size.width, aRect.origin.y + aRect.size.height );
					}
				}

				if ( aPath )
					addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbInvert, mbXOR, false, mnFillColor, mnLineColor, aPath ) );

				CGPathRelease( aPath );
			}
		}
		else if ( mpVCLGraphics )
		{
			if ( mnFillColor )
				mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, mnFillColor, TRUE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
			if ( mnLineColor )
				mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, mnLineColor, FALSE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
		}
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
	if ( mnLineColor && nPoints && pPtAry )
	{
		if ( useNativeDrawing() )
		{
			::basegfx::B2DPolygon aPoly;
			for ( ULONG i = 0 ; i < nPoints; i++ )
				aPoly.append( ::basegfx::B2DPoint( pPtAry[ i ].mnX, pPtAry[ i ].mnY ) );
			aPoly.removeDoublePoints();

			CGMutablePathRef aPath = CGPathCreateMutable();
			if ( aPath )
			{
				AddPolygonToPaths( NULL, aPath, aPoly, aPoly.isClosed() );
				CGRect aRect = CGPathGetBoundingBox( aPath );
				float fNativeLineWidth = getNativeLineWidth();
				if ( aRect.size.width < fNativeLineWidth )
					aRect.size.width = 0;
				if ( aRect.size.height < fNativeLineWidth )
					aRect.size.height = 0;
				if ( CGRectIsEmpty( aRect ) )
				{
					CGPathRelease( aPath );
					aPath = CGPathCreateMutable();
					if ( aPath )
					{
						CGPathMoveToPoint( aPath, NULL, aRect.origin.x, aRect.origin.y );
						CGPathAddLineToPoint( aPath, NULL, aRect.origin.x + aRect.size.width, aRect.origin.y + aRect.size.height );
					}
				}

				if ( aPath )
					addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbInvert, mbXOR, false, 0x00000000, mnLineColor, aPath ) );

				CGPathRelease( aPath );
			}
		}
		else if ( mpVCLGraphics )
		{
			mpVCLGraphics->drawPolyline( nPoints, pPtAry, mnLineColor, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
		}
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
	if ( ( mnFillColor || mnLineColor ) && nPoints && pPtAry )
	{
		if ( useNativeDrawing() )
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
				CGRect aRect = CGPathGetBoundingBox( aPath );
				float fNativeLineWidth = getNativeLineWidth();
				if ( aRect.size.width < fNativeLineWidth )
					aRect.size.width = 0;
				if ( aRect.size.height < fNativeLineWidth )
					aRect.size.height = 0;
				if ( CGRectIsEmpty( aRect ) )
				{
					CGPathRelease( aPath );
					aPath = CGPathCreateMutable();
					if ( aPath )
					{
						CGPathMoveToPoint( aPath, NULL, aRect.origin.x, aRect.origin.y );
						CGPathAddLineToPoint( aPath, NULL, aRect.origin.x + aRect.size.width, aRect.origin.y + aRect.size.height );
					}
				}

				if ( aPath )
					addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbInvert, mbXOR, false, mnFillColor, mnLineColor, aPath ) );

				CGPathRelease( aPath );
			}
		}
		else if ( mpVCLGraphics )
		{
			if ( mnFillColor )
				mpVCLGraphics->drawPolygon( nPoints, pPtAry, mnFillColor, TRUE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
			if ( mnLineColor )
				mpVCLGraphics->drawPolygon( nPoints, pPtAry, mnLineColor, FALSE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
		}
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolyPolygon( ULONG nPoly, const ULONG* pPoints, PCONSTSALPOINT* pPtAry )
{
	if ( nPoly && pPoints && pPtAry )
	{
		if ( useNativeDrawing() )
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
				CGRect aRect = CGPathGetBoundingBox( aPath );
				float fNativeLineWidth = getNativeLineWidth();
				if ( aRect.size.width < fNativeLineWidth )
					aRect.size.width = 0;
				if ( aRect.size.height < fNativeLineWidth )
					aRect.size.height = 0;
				if ( CGRectIsEmpty( aRect ) )
				{
					CGPathRelease( aPath );
					aPath = CGPathCreateMutable();
					if ( aPath )
					{
						CGPathMoveToPoint( aPath, NULL, aRect.origin.x, aRect.origin.y );
						CGPathAddLineToPoint( aPath, NULL, aRect.origin.x + aRect.size.width, aRect.origin.y + aRect.size.height );
					}
				}

				// Always disable invert and XOR for polypolygons like in the
				// Java code otherwise transparent non-rectangular gradients
				// will be drawn incorrectly when printed
				if ( aPath )
					addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, false, false, false, mnFillColor, mnLineColor, aPath ) );

				CGPathRelease( aPath );
			}
		}
		else if ( mpVCLGraphics )
		{
			if ( mnFillColor )
				mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pPtAry, mnFillColor, TRUE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
			if ( mnLineColor )
				mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pPtAry, mnLineColor, FALSE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
		}
	}
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly, double fTransparency )
{
	bool bRet = true;

	if ( ( mnFillColor || mnLineColor ) && rPolyPoly.count() )
	{
		sal_uInt8 nTransparency = (sal_uInt8)( ( fTransparency * 100 ) + 0.5 );
		setFillTransparency( nTransparency );
		setLineTransparency( nTransparency );

		if ( useNativeDrawing() )
		{
			CGMutablePathRef aPath = CGPathCreateMutable();
			if ( aPath )
			{
				AddPolyPolygonToPaths( NULL, aPath, rPolyPoly );
				CGRect aRect = CGPathGetBoundingBox( aPath );
				float fNativeLineWidth = getNativeLineWidth();
				if ( aRect.size.width < fNativeLineWidth )
					aRect.size.width = 0;
				if ( aRect.size.height < fNativeLineWidth )
					aRect.size.height = 0;
				if ( CGRectIsEmpty( aRect ) )
				{
					CGPathRelease( aPath );
					aPath = CGPathCreateMutable();
					if ( aPath )
					{
						CGPathMoveToPoint( aPath, NULL, aRect.origin.x, aRect.origin.y );
						CGPathAddLineToPoint( aPath, NULL, aRect.origin.x + aRect.size.width, aRect.origin.y + aRect.size.height );
					}
				}

				if ( aPath )
					addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbInvert, mbXOR, getAntiAliasB2DDraw(), mnFillColor, mnLineColor, aPath ) );

				CGPathRelease( aPath );
			}
		}
		else if ( mpVCLGraphics )
		{
			com_sun_star_vcl_VCLPath aPath;
			CGMutablePathRef aCGPath = NULL;
			if ( mpPrinter )
				aCGPath = CGPathCreateMutable();
			AddPolyPolygonToPaths( &aPath, aCGPath, rPolyPoly );

			if ( mnFillColor )
				mpVCLGraphics->drawPath( &aPath, mnFillColor, TRUE, getAntiAliasB2DDraw(), aCGPath, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
			if ( mnLineColor )
				mpVCLGraphics->drawPath( &aPath, mnFillColor, FALSE, getAntiAliasB2DDraw(), aCGPath, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
		}

		setFillTransparency( 0 );
		setLineTransparency( 0 );
	}

	return bRet;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawPolyLine( const ::basegfx::B2DPolygon& rPoly, const ::basegfx::B2DVector& rLineWidths, ::basegfx::B2DLineJoin eLineJoin )
{
	bool bRet = true;

	if ( mnLineColor )
	{
		if ( useNativeDrawing() )
		{
			CGMutablePathRef aPath = CGPathCreateMutable();
			if ( aPath )
			{
				AddPolygonToPaths( NULL, aPath, rPoly, rPoly.isClosed() );
				CGRect aRect = CGPathGetBoundingBox( aPath );
				float fNativeLineWidth = rLineWidths.getX();
				if ( fNativeLineWidth <= 0 )
					fNativeLineWidth = getNativeLineWidth();
				if ( aRect.size.width < fNativeLineWidth )
					aRect.size.width = 0;
				if ( aRect.size.height < fNativeLineWidth )
					aRect.size.height = 0;
				if ( CGRectIsEmpty( aRect ) )
				{
					CGPathRelease( aPath );
					aPath = CGPathCreateMutable();
					if ( aPath )
					{
						CGPathMoveToPoint( aPath, NULL, aRect.origin.x, aRect.origin.y );
						CGPathAddLineToPoint( aPath, NULL, aRect.origin.x + aRect.size.width, aRect.origin.y + aRect.size.height );
					}
				}

				if ( aPath )
					addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbInvert, mbXOR, getAntiAliasB2DDraw(), 0x00000000, mnLineColor, aPath, fNativeLineWidth, eLineJoin ) );

				CGPathRelease( aPath );
			}
		}
		else if ( mpPrinter )
		{
			bRet = false;
		}
		else if ( mpVCLGraphics )
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
#ifndef USE_NATIVE_PRINTING
		if ( !mpPrinter )
#endif	// !USE_NATIVE_PRINTING
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
	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	if ( !pSrcGraphics || !maLayer )
		return;

	CGContextRef aContext = CGLayerGetContext( maLayer );
	if ( aContext )
	{
		// Draw any undrawn operations so that we copy the latest bits
		CGSize aLayerSize = CGLayerGetSize( maLayer );
		CGRect aLayerBounds = CGRectMake( 0, 0, aLayerSize.width, aLayerSize.height );
		drawUndrawnNativeOps( aContext, aLayerBounds );

		CGContextSaveGState( aContext );

		pSrcGraphics->copyToContext( maNativeClipPath, mbInvert && bAllowXOR ? true : false, mbXOR && bAllowXOR ? true : false, aContext, aLayerBounds, aSrcPoint, aDestRect );

		CGContextRestoreGState( aContext );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::copyToContext( const CGPathRef aNativeClipPath, bool bInvert, bool bXOR, CGContextRef aDestContext, CGRect aDestBounds, CGPoint aSrcPoint, CGRect aDestRect )
{
	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	if ( !aDestContext || !maLayer )
		return;

	// Draw any undrawn operations so that we copy the latest bits
	CGSize aLayerSize = CGLayerGetSize( maLayer );
	drawUndrawnNativeOps( aDestContext, CGRectMake( 0, 0, aLayerSize.width, aLayerSize.height ) );

	// Do not queue this operation since we are copying to another context
	JavaSalGraphicsCopyLayerOp aOp( aNativeClipPath, bInvert, bXOR, maLayer, aSrcPoint, aDestRect );
	aOp.drawOp( this, aDestContext, aDestBounds );
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

	while ( maUndrawnNativeOpsList.size() )
	{
		JavaSalGraphicsOp *pOp = maUndrawnNativeOpsList.front();
		maUndrawnNativeOpsList.pop_front();
		pOp->drawOp( this, aContext, aBounds );
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

JavaSalGraphicsOp::JavaSalGraphicsOp( const CGPathRef aNativeClipPath, bool bInvert, bool bXOR, float fLineWidth ) :
	maNativeClipPath( NULL ),
	mbInvert( bInvert ),
	mbXOR( bXOR ),
	mfLineWidth( fLineWidth ),
	mnXORBitmapPadding( 0 ),
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
	if ( mbInvert )
		mbXOR = false;
}

// -----------------------------------------------------------------------

JavaSalGraphicsOp::~JavaSalGraphicsOp()
{
	restoreClipXORGState();

	if ( maNativeClipPath )
		CGPathRelease( maNativeClipPath );
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
						CGContextDrawImage( maSavedContext, CGRectMake( maXORRect.origin.x - mnXORBitmapPadding, maXORRect.origin.y - mnXORBitmapPadding, nBitmapWidth, nBitmapHeight ), aImage );
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

	if ( maXORLayer )
	{
		CGLayerRelease( maXORLayer );
		maXORLayer = NULL;
	}

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

CGContextRef JavaSalGraphicsOp::saveClipXORGState( JavaSalGraphics *pGraphics, CGContextRef aContext, CGRect aDrawBounds )
{
	restoreClipXORGState();

	if ( !aContext || !pGraphics )
		return NULL;

	if ( mfLineWidth <= 0 )
		mfLineWidth = pGraphics->getNativeLineWidth();

	if ( mfLineWidth > 0 )
		mnXORBitmapPadding = (sal_uInt32)( mfLineWidth + 0.5 );

	if ( mbXOR )
	{
		// Mac OS X's XOR blend mode does not do real XORing of bits so we
		// reimplement our own XORing
		bool bXORDrawable = false;

		maXORLayer = pGraphics->getLayer();
		if ( maXORLayer )
		{
			CGLayerRetain( maXORLayer );

			// Trust that the draw bounds has already been intersected against
			// the graphics bounds and clip
			maXORRect = CGRectStandardize( aDrawBounds );
			if ( !CGRectIsEmpty( maXORRect ) )
			{
				CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
				if ( aColorSpace )
				{
					CGSize aBitmapSize = CGSizeMake( maXORRect.size.width + ( mnXORBitmapPadding * 2 ), maXORRect.size.height + ( mnXORBitmapPadding * 2 ) );
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
							// Translate the drawing context
							CGContextTranslateCTM( maDrawBitmapContext, mnXORBitmapPadding - maXORRect.origin.x, mnXORBitmapPadding - maXORRect.origin.y );

							JavaSalGraphics::setContextDefaultSettings( maDrawBitmapContext, maNativeClipPath, pGraphics->getNativeLineWidth() );

							// Copy layer to XOR context
							CGContextDrawLayerAtPoint( maXORBitmapContext, CGPointMake( mnXORBitmapPadding - maXORRect.origin.x, mnXORBitmapPadding - maXORRect.origin.y ), maXORLayer );

							bXORDrawable = true;
						}
					}

					CGColorSpaceRelease( aColorSpace );
				}
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

	JavaSalGraphics::setContextDefaultSettings( maSavedContext, maNativeClipPath, pGraphics->getNativeLineWidth() );

	if ( mbInvert )
		CGContextSetBlendMode( maSavedContext, kCGBlendModeDifference );

	return maDrawBitmapContext ? maDrawBitmapContext : maSavedContext;
}
