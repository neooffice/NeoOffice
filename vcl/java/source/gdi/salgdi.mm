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

#include <basegfx/polygon/b2dpolygon.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>
#undef check

#include "java/saldata.hxx"
#include "java/salgdi.h"
#include "java/salinst.h"

#include "../app/salinst_cocoa.h"

class SAL_DLLPRIVATE JavaSalGraphicsCopyLayerOp : public JavaSalGraphicsOp
{
	CGLayerRef				maSrcLayer;
	CGRect					maSrcRect;
	CGRect					maRect;

public:
							JavaSalGraphicsCopyLayerOp( const CGPathRef aFrameClipPath, const CGPathRef aNativeClipPath, bool bInvert, bool bXOR, CGLayerRef aSrcLayer, const CGRect aSrcRect, const CGRect aRect );
	virtual					~JavaSalGraphicsCopyLayerOp();

	virtual	void			drawOp( JavaSalGraphics *pGraphics, CGContextRef aContext, CGRect aBounds );
};

class SAL_DLLPRIVATE JavaSalGraphicsDrawEPSOp : public JavaSalGraphicsOp
{
	CFDataRef				maData;
	CGRect					maRect;

public:
							JavaSalGraphicsDrawEPSOp( const CGPathRef aFrameClipPath, const CGPathRef aNativeClipPath, CFDataRef aData, const CGRect aRect );
	virtual					~JavaSalGraphicsDrawEPSOp();

	virtual	void			drawOp( JavaSalGraphics *pGraphics, CGContextRef aContext, CGRect aBounds );
};

using namespace osl;

// =======================================================================

bool AddPolygonToPaths( CGMutablePathRef aCGPath, const ::basegfx::B2DPolygon& rPolygon, bool bClosePath, CGRect aUnflippedBounds )
{
	if ( !aCGPath )
		return false;

	const sal_uInt32 nCount = rPolygon.count();
	if ( !nCount )
		return false;

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
		CGPoint aUnflippedPoint = UnflipFlippedPoint( CGPointMake( aPoint.getX(), aPoint.getY() ), aUnflippedBounds );

		if ( !nIndex )
		{
			CGPathMoveToPoint( aCGPath, NULL, aUnflippedPoint.x, aUnflippedPoint.y );
		}
		else if ( !bPendingCurve )
		{
			CGPathAddLineToPoint( aCGPath, NULL, aUnflippedPoint.x, aUnflippedPoint.y );
		}
		else
		{
			::basegfx::B2DPoint aFirstControlPoint = rPolygon.getNextControlPoint( nPreviousIndex );
			::basegfx::B2DPoint aSecondControlPoint = rPolygon.getPrevControlPoint( nClosedIndex );
			CGPoint aFirstUnflippedControlPoint = UnflipFlippedPoint( CGPointMake( aFirstControlPoint.getX(), aFirstControlPoint.getY() ), aUnflippedBounds );
			CGPoint aSecondUnflippedControlPoint = UnflipFlippedPoint( CGPointMake( aSecondControlPoint.getX(), aSecondControlPoint.getY() ), aUnflippedBounds );
			CGPathAddCurveToPoint( aCGPath, NULL, aFirstUnflippedControlPoint.x, aFirstUnflippedControlPoint.y, aSecondUnflippedControlPoint.x, aSecondUnflippedControlPoint.y, aUnflippedPoint.x, aUnflippedPoint.y );
		}

		if ( bHasCurves )
			bPendingCurve = rPolygon.isNextControlPointUsed( nClosedIndex );
	}

	if ( bClosePath )
		CGPathCloseSubpath( aCGPath );

	return !bHasCurves;
}

// -----------------------------------------------------------------------

bool AddPolyPolygonToPaths( CGMutablePathRef aCGPath, const ::basegfx::B2DPolyPolygon& rPolyPoly, CGRect aUnflippedBounds )
{
	bool bRet = false;

	const sal_uInt32 nCount = rPolyPoly.count();
	if ( !nCount )
		return bRet;

	bRet = true;
	for ( sal_uInt32 i = 0; i < nCount; i++ )
	{
		const ::basegfx::B2DPolygon rPolygon = rPolyPoly.getB2DPolygon( i );
		if ( !AddPolygonToPaths( aCGPath, rPolygon, true, aUnflippedBounds ) )
			bRet = false;
	}

	return bRet;
}

// -----------------------------------------------------------------------

CGColorRef CreateCGColorFromSalColor( SalColor nColor )
{
	return CGColorCreateGenericRGB( (float)( ( nColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( nColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( nColor & 0x000000ff ) / (float)0xff, (float)( ( nColor & 0xff000000 ) >> 24 ) / (float)0xff );
}

// -----------------------------------------------------------------------

CGPoint UnflipFlippedPoint( CGPoint aFlippedPoint, CGRect aUnflippedBounds )
{
	CGPoint aRet = aFlippedPoint;

	if ( !CGRectIsNull( aUnflippedBounds ) )
		aRet.y = aUnflippedBounds.origin.y + aUnflippedBounds.size.height - aRet.y;

	return aRet;
}

// -----------------------------------------------------------------------

CGRect UnflipFlippedRect( CGRect aFlippedRect, CGRect aUnflippedBounds )
{
	CGRect aRet = aFlippedRect;

	if ( !CGRectIsNull( aFlippedRect ) && !CGRectIsNull( aUnflippedBounds ) )
	{
		aRet = CGRectStandardize( aRet );
		aRet.origin.y = aUnflippedBounds.origin.y + aUnflippedBounds.size.height - aRet.origin.y - aRet.size.height;
	}

	return aRet;
}

// =======================================================================

JavaSalGraphicsCopyLayerOp::JavaSalGraphicsCopyLayerOp( const CGPathRef aFrameClipPath, const CGPathRef aNativeClipPath, bool bInvert, bool bXOR, CGLayerRef aSrcLayer, const CGRect aSrcRect, const CGRect aRect ) :
	JavaSalGraphicsOp( aFrameClipPath, aNativeClipPath, bInvert, bXOR ),
	maSrcLayer( aSrcLayer ),
	maSrcRect( aSrcRect ),
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
	if ( !pGraphics || !aContext || !maSrcLayer )
		return;

	CGRect aDrawBounds = maRect;
	if ( !CGRectIsEmpty( aBounds ) )
		aDrawBounds = CGRectIntersection( aDrawBounds, aBounds );
	if ( maFrameClipPath )
		aDrawBounds = CGRectIntersection( aDrawBounds, CGPathGetBoundingBox( maFrameClipPath ) );
	if ( maNativeClipPath )
		aDrawBounds = CGRectIntersection( aDrawBounds, CGPathGetBoundingBox( maNativeClipPath ) );
	if ( CGRectIsEmpty( aDrawBounds ) )
		return;

	float fScaleX = maRect.size.width / maSrcRect.size.width;
	float fScaleY = maRect.size.height / maSrcRect.size.height;

	// Check if the adjusted draw bounds matches any area of the source layer
	CGSize aSrcLayerSize = CGLayerGetSize( maSrcLayer );
	CGRect aSrcDrawBounds = CGRectMake( maSrcRect.origin.x + ( ( aDrawBounds.origin.x - maRect.origin.x ) / fScaleX ), maSrcRect.origin.y + ( ( aDrawBounds.origin.y - maRect.origin.y ) / fScaleY ), aDrawBounds.size.width / fScaleX, aDrawBounds.size.height / fScaleY );
	if ( CGRectIsEmpty( CGRectIntersection( aSrcDrawBounds, CGRectMake( 0, 0, aSrcLayerSize.width, aSrcLayerSize.height ) ) ) )
		return;

	aContext = saveClipXORGState( pGraphics, aContext, aDrawBounds );
	if ( !aContext )
		return;

	CGContextRef aSrcContext = CGLayerGetContext( maSrcLayer );
	if ( aSrcContext == aContext )
	{
		// Fix the stray drawing artifacts reported in the following forum post
		// by making the temporary layer large enough to handle the bleeding
		// for lines drawn in the edge of the temporary bounds:
		// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=62799#62799
		sal_uInt32 nTmpPadding = 0;
		if ( mfLineWidth > 0 )
			nTmpPadding = (sal_uInt32)( mfLineWidth + 0.5 );

		// Copying within the same context to a negative x destination or a
		// destination that overlaps with the source causes drawing to wrap
		// around to the right edge of the destination layer so make a temporary
		// copy of the source using a native layer created from the source
		// layer's context
		CGLayerRef aTmpLayer = CGLayerCreateWithContext( aSrcContext, CGSizeMake( maSrcRect.size.width + ( nTmpPadding * 2 ), maSrcRect.size.height + ( nTmpPadding * 2 ) ), NULL );
		if ( aTmpLayer )
		{
			CGContextRef aTmpContext = CGLayerGetContext( aTmpLayer );
			if ( aTmpContext )
			{
				CGContextTranslateCTM( aTmpContext, nTmpPadding, nTmpPadding );
				CGContextDrawLayerAtPoint( aTmpContext, CGPointMake( maSrcRect.origin.x * -1, maSrcRect.origin.y * -1 ), maSrcLayer );

				CGContextClipToRect( aContext, maRect );
				CGContextTranslateCTM( aContext, maRect.origin.x - nTmpPadding, maRect.origin.y - nTmpPadding );
				CGContextScaleCTM( aContext, fScaleX, fScaleY );
				CGContextDrawLayerAtPoint( aContext, CGPointMake( 0, 0 ), aTmpLayer );
			}

			CGLayerRelease( aTmpLayer );
		}
	}
	else
	{
		CGContextClipToRect( aContext, maRect );
		CGContextTranslateCTM( aContext, maRect.origin.x - maSrcRect.origin.x, maRect.origin.y - maSrcRect.origin.y );
		CGContextScaleCTM( aContext, fScaleX, fScaleY );
		CGContextDrawLayerAtPoint( aContext, CGPointMake( 0, 0 ), maSrcLayer );
	}

	restoreClipXORGState();
}

// =======================================================================

JavaSalGraphicsDrawEPSOp::JavaSalGraphicsDrawEPSOp( const CGPathRef aFrameClipPath, const CGPathRef aNativeClipPath, CFDataRef aData, const CGRect aRect ) :
	JavaSalGraphicsOp( aFrameClipPath, aNativeClipPath ),
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
	if ( !pGraphics || !aContext || !maData )
		return;

	CGRect aDrawBounds = maRect;
	if ( !CGRectIsEmpty( aBounds ) )
		aDrawBounds = CGRectIntersection( aDrawBounds, aBounds );
	if ( maFrameClipPath )
		aDrawBounds = CGRectIntersection( aDrawBounds, CGPathGetBoundingBox( maFrameClipPath ) );
	if ( maNativeClipPath )
		aDrawBounds = CGRectIntersection( aDrawBounds, CGPathGetBoundingBox( maNativeClipPath ) );
	if ( CGRectIsEmpty( aDrawBounds ) )
		return;

	aContext = saveClipXORGState( pGraphics, aContext, aDrawBounds );
	if ( !aContext )
		return;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSImageRep *pImageRep = [NSEPSImageRep imageRepWithData:(NSData *)maData];
	if ( !pImageRep )
		pImageRep = [NSPDFImageRep imageRepWithData:(NSData *)maData];
	if ( pImageRep )
	{
		// Fix crash after printing an EPS image by retaining the EPS image
		// until after the print operation has ended
		VCLInstance_retainIfInDragPrintLock( pImageRep );

		NSGraphicsContext *pContext = [NSGraphicsContext graphicsContextWithGraphicsPort:aContext flipped:NO];
		if ( pContext )
		{
			NSGraphicsContext *pOldContext = [NSGraphicsContext currentContext];
			[NSGraphicsContext setCurrentContext:pContext];
			[pImageRep drawInRect:NSRectFromCGRect( maRect )];
			[NSGraphicsContext setCurrentContext:pOldContext];
		}
	}

	[pPool release];

	restoreClipXORGState();

	if ( pGraphics->mpFrame )
		pGraphics->addNeedsDisplayRect( aDrawBounds, mfLineWidth );
}

// =======================================================================

JavaSalGraphicsDrawPathOp::JavaSalGraphicsDrawPathOp( const CGPathRef aFrameClipPath, const CGPathRef aNativeClipPath, bool bInvert, bool bXOR, bool bAntialias, SalColor nFillColor, SalColor nLineColor, const CGPathRef aPath, bool bShiftLines, float fLineWidth, ::basegfx::B2DLineJoin eLineJoin, bool bLineDash, com::sun::star::drawing::LineCap nLineCap ) :
	JavaSalGraphicsOp( aFrameClipPath, aNativeClipPath, bInvert, bXOR, fLineWidth ),
	mbAntialias( bAntialias ),
	mnFillColor( nFillColor ),
	mnLineColor( nLineColor ),
	maPath( NULL ),
	mbShiftLines( bShiftLines ),
	meLineJoin( eLineJoin ),
	mbLineDash( bLineDash ),
	mnLineCap( nLineCap )
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
	if ( !pGraphics || !aContext || !maPath )
		return;

	// Expand draw bounds by the line width
	if ( mfLineWidth <= 0 )
		mfLineWidth = pGraphics->getNativeLineWidth();

	// Eliminate drawing of unexpected circles when drawing a trapezoid with a
	// round line cap and a wide line width in Writer by not drawing anything
	// when the path is a single point
	CGRect aDrawBounds = CGPathGetBoundingBox( maPath );
	if ( !aDrawBounds.size.width && !aDrawBounds.size.height )
		return;

	aDrawBounds.origin.x -= mfLineWidth;
	aDrawBounds.origin.y -= mfLineWidth;
	aDrawBounds.size.width += mfLineWidth * 2;
	aDrawBounds.size.height += mfLineWidth * 2;
	if ( !CGRectIsEmpty( aBounds ) )
		aDrawBounds = CGRectIntersection( aDrawBounds, aBounds );
	if ( maFrameClipPath )
		aDrawBounds = CGRectIntersection( aDrawBounds, CGPathGetBoundingBox( maFrameClipPath ) );
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
				// Set line width
				CGContextSetLineWidth( aContext, mfLineWidth );

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

				// Set line cap
				switch ( mnLineCap )
				{
					case com::sun::star::drawing::LineCap_BUTT:
						CGContextSetLineCap( aContext, kCGLineCapButt );
						break;
					case com::sun::star::drawing::LineCap_ROUND:
						CGContextSetLineCap( aContext, kCGLineCapRound );
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

				if ( CGColorGetAlpha( aFillColor ) )
				{
					CGContextBeginPath( aContext );
					CGContextAddPath( aContext, maPath );

					// Smooth out image drawing for bug 2475 image
					CGContextSetAllowsAntialiasing( aContext, mbXOR || mbAntialias );

					CGContextSetFillColorWithColor( aContext, aFillColor );
					CGContextEOFillPath( aContext );
				}
				if ( CGColorGetAlpha( aLineColor ) )
				{
					// Shift line drawing downward slightly so that line
					// drawing favors pixels downward and to the right instead
					// of the CGContext default behavior of upward and to the
					// right since the OOo code favors the former, not the
					// latter behavior.
					// This download shift also fixes the bugs reported in the
					// following NeoOffice forum posts:
					// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=62777#62777
					// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=62805#62805
					// Shift half the line width to fix the bug reported in the
					// following NeoOffice forum post:
					// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8467
					// Fix overshifting of curves reported in the following
					// NeoOffice forum topic by shifting lines only when the
					// polygon consists of straight lines:
					// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=63692#63692
					if ( mbShiftLines && !pGraphics->mpPrinter )
						CGContextTranslateCTM( aContext, mbAntialias ? pGraphics->getNativeLineWidth() / 2 : 0, pGraphics->getNativeLineWidth() / -2 );

					CGContextBeginPath( aContext );
					CGContextAddPath( aContext, maPath );

					// Enable or disable antialiasing
					CGContextSetAllowsAntialiasing( aContext, mbAntialias );

					CGContextSetStrokeColorWithColor( aContext, aLineColor );
					CGContextStrokePath( aContext );
				}

				restoreClipXORGState();

				if ( pGraphics->mpFrame )
					pGraphics->addNeedsDisplayRect( aDrawBounds, mfLineWidth );
			}

			CGColorRelease( aLineColor );
		}

		CGColorRelease( aFillColor );
	}
}

// =======================================================================

float JavaSalGraphics::getContextBackingFactor( CGContextRef aContext )
{
	float fRet = 1.0f;

	if ( aContext )
	{
		CGRect aDeviceRect = CGContextConvertRectToDeviceSpace( aContext, CGRectMake( 0, 0, 1, 1 ) );
		if ( !CGRectIsEmpty( aDeviceRect ) )
		{
			// Use the higher of the two backing factors if they are different
			if ( fRet < aDeviceRect.size.width )
				fRet = aDeviceRect.size.width;
			if ( fRet < aDeviceRect.size.height )
				fRet = aDeviceRect.size.height;
		}
	}

	return fRet;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::setContextDefaultSettings( CGContextRef aContext, const CGPathRef aFrameClipPath, const CGPathRef aClipPath, float fLineWidth )
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

	// Set frame clip
	if ( aFrameClipPath )
	{
		CGContextBeginPath( aContext );
		CGContextAddPath( aContext, aFrameClipPath );
		CGContextClip( aContext );
	}

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
	mnBackgroundColor( 0x00000000 ),
	maLayer( NULL ),
	mnPixelContextData( 0 ),
	maPixelContext( NULL ),
	maNeedsDisplayRect( CGRectNull ),
	mnFillColor( MAKE_SALCOLOR( 0xff, 0xff, 0xff ) | 0xff000000 ),
	mnLineColor( MAKE_SALCOLOR( 0, 0, 0 ) | 0xff000000 ),
	mnTextColor( MAKE_SALCOLOR( 0, 0, 0 ) | 0xff000000 ),
	mnFillTransparency( 0xff000000 ),
	mnLineTransparency( 0xff000000 ),
	mpFrame( NULL ),
	mpPrinter( NULL ),
	mpVirDev( NULL ),
	mpFontData( NULL ),
	mpFont( NULL ),
	mnFontFamily( FAMILY_DONTKNOW ),
	mnFontWeight( WEIGHT_DONTKNOW ),
	mbFontItalic( false ),
	mnFontPitch( PITCH_DONTKNOW ),
	mnFontWidthType( WIDTH_DONTKNOW ),
	mnDPIX( 0 ),
	mnDPIY( 0 ),
	maFrameClipPath( NULL ),
	maNativeClipPath( NULL ),
	mbInvert( false ),
	mbXOR( false ),
	meOrientation( ORIENTATION_PORTRAIT ),
	mbPaperRotated( sal_False ),
	mfPageTranslateX( 0 ),
	mfPageTranslateY( 0 ),
	mfPageOutputWidth( 0 ),
	mfPageOutputHeight( 0 ),
	maNativeBounds( CGRectNull )
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

	if ( maLayer )
		CGLayerRelease( maLayer );

	if ( mpFontData )
		delete mpFontData;

	if ( mpFont )
		delete mpFont;

	for ( ::boost::unordered_map< int, JavaImplFont* >::const_iterator it = maFallbackFonts.begin(); it != maFallbackFonts.end(); ++it )
		delete it->second;

	if ( maFrameClipPath )
		CFRelease( maFrameClipPath );

	if ( maNativeClipPath )
		CFRelease( maNativeClipPath );

	if ( maPixelContext )
		CGContextRelease( maPixelContext );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY )
{
	rDPIX = mnDPIX;
	rDPIY = mnDPIY;
}

// -----------------------------------------------------------------------

sal_uInt16 JavaSalGraphics::GetBitCount() const
{
	return 32;
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

void JavaSalGraphics::ResetClipRegion()
{
	if ( maNativeClipPath )
	{
		CFRelease( maNativeClipPath );
		maNativeClipPath = NULL;
	}
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::setClipRegion( const vcl::Region& rRegion )
{
	if ( maNativeClipPath )
	{
		CFRelease( maNativeClipPath );
		maNativeClipPath = NULL;
	}

	maNativeClipPath = CGPathCreateMutable();
	if ( maNativeClipPath )
	{
		if ( rRegion.HasPolyPolygonOrB2DPolyPolygon() )
		{
			const ::basegfx::B2DPolyPolygon aPolyPoly = rRegion.GetAsB2DPolyPolygon();
			const sal_uInt32 nPoly = aPolyPoly.count();
			if ( nPoly )
			{
				CGMutablePathRef aCGPath = CGPathCreateMutable();
				AddPolyPolygonToPaths( aCGPath, aPolyPoly, maNativeBounds );
				CGPathAddPath( maNativeClipPath, NULL, aCGPath );
				CFRelease( aCGPath );
			}
		}
		else
		{
			RectangleVector aRects;
			rRegion.GetRegionRectangles( aRects );
			for ( RectangleVector::const_iterator it = aRects.begin(); it != aRects.end(); ++it )
			{
				if ( !(*it).IsEmpty() )
				{
					CGRect aRect = UnflipFlippedRect( CGRectMake( (*it).Left(), (*it).Top(), (*it).GetWidth(), (*it).GetHeight() ), maNativeBounds );
					if ( !CGRectIsEmpty( aRect ) )
						CGPathAddRect( maNativeClipPath, NULL, aRect );
				}
			}
		}
	}

	return true;
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
		CGMutablePathRef aPath = CGPathCreateMutable();
		if ( aPath )
		{
			CGRect aUnflippedRect = UnflipFlippedRect( CGRectMake( nX, nY, 1, 1 ), maNativeBounds );
			CGPathAddRect( aPath, NULL, aUnflippedRect );
			addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maFrameClipPath, maNativeClipPath, mbInvert, mbXOR, false, mnLineColor, 0x00000000, aPath ) );
			CGPathRelease( aPath );
		}
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
	CGMutablePathRef aPath = CGPathCreateMutable();
	if ( aPath )
	{
		CGRect aUnflippedRect = UnflipFlippedRect( CGRectMake( nX, nY, 1, 1 ), maNativeBounds );
		CGPathAddRect( aPath, NULL, aUnflippedRect );
		addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maFrameClipPath, maNativeClipPath, mbInvert, mbXOR, false, nSalColor | 0xff000000, 0x00000000, aPath ) );
		CGPathRelease( aPath );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
	if ( mnLineColor )
	{
		CGMutablePathRef aPath = CGPathCreateMutable();
		if ( aPath )
		{
			CGPoint aUnflippedPoint1 = UnflipFlippedPoint( CGPointMake( nX1, nY1 ), maNativeBounds );
			CGPoint aUnflippedPoint2 = UnflipFlippedPoint( CGPointMake( nX2, nY2 ), maNativeBounds );
			CGPathMoveToPoint( aPath, NULL, aUnflippedPoint1.x, aUnflippedPoint1.y );
			CGPathAddLineToPoint( aPath, NULL, aUnflippedPoint2.x, aUnflippedPoint2.y );
			addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maFrameClipPath, maNativeClipPath, mbInvert, mbXOR, false, 0x00000000, mnLineColor, aPath ) );
			CGPathRelease( aPath );
		}
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
	if ( mnFillColor || mnLineColor )
	{
		CGMutablePathRef aPath = CGPathCreateMutable();
		if ( aPath )
		{
			CGRect aUnflippedRect = UnflipFlippedRect( CGRectMake( nX, nY, nWidth, nHeight ), maNativeBounds );

			// The OOo code expects any line drawing to be drawn within the
			// inside edge of the rectangle so shrink the rectange by half the
			// line width if the line color is set
			if ( mnLineColor && !CGRectIsEmpty( aUnflippedRect ) )
			{
				float fNativeLineWidth = getNativeLineWidth();
				aUnflippedRect.origin.x += fNativeLineWidth / 2;
				aUnflippedRect.origin.y += fNativeLineWidth / 2;
				if ( aUnflippedRect.size.width < fNativeLineWidth )
					aUnflippedRect.size.width = 0;
				else
					aUnflippedRect.size.width -= fNativeLineWidth;
				if ( aUnflippedRect.size.height < fNativeLineWidth )
					aUnflippedRect.size.height = 0;
				else
					aUnflippedRect.size.height -= fNativeLineWidth;
			}
			CGPathAddRect( aPath, NULL, aUnflippedRect );
			if ( !mnLineColor && CGRectIsEmpty( aUnflippedRect ) )
			{
				CGPathRelease( aPath );
				aPath = NULL;
			}

			if ( aPath )
			{
				addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maFrameClipPath, maNativeClipPath, mbInvert, mbXOR, false, mnFillColor, mnLineColor, aPath ) );
				CGPathRelease( aPath );
			}
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

void JavaSalGraphics::drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
	if ( mnLineColor && nPoints && pPtAry )
	{
		CGMutablePathRef aPath = CGPathCreateMutable();
		if ( aPath )
		{
			CGPoint aUnflippedPoint = UnflipFlippedPoint( CGPointMake( pPtAry[ 0 ].mnX, pPtAry[ 0 ].mnY ), maNativeBounds );
			CGPathMoveToPoint( aPath, NULL, aUnflippedPoint.x, aUnflippedPoint.y );
			for ( sal_uInt32 i = 1 ; i < nPoints; i++ )
			{
				aUnflippedPoint = UnflipFlippedPoint( CGPointMake( pPtAry[ i ].mnX, pPtAry[ i ].mnY ), maNativeBounds );
				CGPathAddLineToPoint( aPath, NULL, aUnflippedPoint.x, aUnflippedPoint.y );
			}
			addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maFrameClipPath, maNativeClipPath, mbInvert, mbXOR, false, 0x00000000, mnLineColor, aPath ) );
			CGPathRelease( aPath );
		}
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
	if ( ( mnFillColor || mnLineColor ) && nPoints && pPtAry )
	{
		CGMutablePathRef aPath = CGPathCreateMutable();
		if ( aPath )
		{
			CGPoint aUnflippedPoint = UnflipFlippedPoint( CGPointMake( pPtAry[ 0 ].mnX, pPtAry[ 0 ].mnY ), maNativeBounds );
			CGPathMoveToPoint( aPath, NULL, aUnflippedPoint.x, aUnflippedPoint.y );
			for ( sal_uInt32 i = 1 ; i < nPoints; i++ )
			{
				aUnflippedPoint = UnflipFlippedPoint( CGPointMake( pPtAry[ i ].mnX, pPtAry[ i ].mnY ), maNativeBounds );
				CGPathAddLineToPoint( aPath, NULL, aUnflippedPoint.x, aUnflippedPoint.y );
			}
			CGPathCloseSubpath( aPath );
			CGRect aRect = CGPathGetBoundingBox( aPath );
			if ( !mnLineColor && CGRectIsEmpty( aRect ) )
			{
				CGPathRelease( aPath );
				aPath = NULL;
			}

			if ( aPath )
			{
				addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maFrameClipPath, maNativeClipPath, mbInvert, mbXOR, false, mnFillColor, mnLineColor, aPath ) );
				CGPathRelease( aPath );
			}
		}
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry )
{
	if ( ( mnFillColor || mnLineColor ) && nPoly && pPoints && pPtAry )
	{
		::basegfx::B2DPolyPolygon aPolyPoly;
		for ( sal_uInt32 i = 0 ; i < nPoly; i++ )
		{
			PCONSTSALPOINT pPolyPtAry = pPtAry[ i ];
			if ( pPolyPtAry )
			{
				::basegfx::B2DPolygon aPoly;
				for ( sal_uInt32 j = 0 ; j < pPoints[ i ]; j++ )
					aPoly.append( ::basegfx::B2DPoint( pPolyPtAry[ j ].mnX, pPolyPtAry[ j ].mnY ) );
				aPoly.setClosed( true );
				aPoly.removeDoublePoints();
				aPolyPoly.append( aPoly );
			}
		}

		CGMutablePathRef aPath = CGPathCreateMutable();
		if ( aPath )
		{
			AddPolyPolygonToPaths( aPath, aPolyPoly, maNativeBounds );
			CGRect aRect = CGPathGetBoundingBox( aPath );
			if ( !mnLineColor && CGRectIsEmpty( aRect ) )
			{
				CGPathRelease( aPath );
				aPath = NULL;
			}

			// Always disable invert and XOR for polypolygons like in the
			// Java code otherwise transparent non-rectangular gradients
			// will be drawn incorrectly when printed
			if ( aPath )
			{
				addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maFrameClipPath, maNativeClipPath, false, false, false, mnFillColor, mnLineColor, aPath ) );
				CGPathRelease( aPath );
			}
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

		CGMutablePathRef aPath = CGPathCreateMutable();
		if ( aPath )
		{
			bool bShiftLines = AddPolyPolygonToPaths( aPath, rPolyPoly, maNativeBounds );
			CGRect aRect = CGPathGetBoundingBox( aPath );
			if ( !mnLineColor && CGRectIsEmpty( aRect ) )
			{
				CGPathRelease( aPath );
				aPath = NULL;
			}

			if ( aPath )
			{
				addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maFrameClipPath, maNativeClipPath, mbInvert, mbXOR, getAntiAliasB2DDraw(), mnFillColor, mnLineColor, aPath, bShiftLines ) );
				CGPathRelease( aPath );
			}
		}

		setFillTransparency( 0 );
		setLineTransparency( 0 );
	}

	return bRet;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawPolyLine( const ::basegfx::B2DPolygon& rPoly, double fTransparency, const ::basegfx::B2DVector& rLineWidths, basegfx::B2DLineJoin eLineJoin, com::sun::star::drawing::LineCap nLineCap )
{
	bool bRet = true;

	if ( mnLineColor )
	{
		sal_uInt8 nTransparency = (sal_uInt8)( ( fTransparency * 100 ) + 0.5 );
		setLineTransparency( nTransparency );

		CGMutablePathRef aPath = CGPathCreateMutable();
		if ( aPath )
		{
			bool bShiftLines = AddPolygonToPaths( aPath, rPoly, rPoly.isClosed(), maNativeBounds );
			float fNativeLineWidth = rLineWidths.getX();
			if ( fNativeLineWidth <= 0 )
				fNativeLineWidth = getNativeLineWidth();
			addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maFrameClipPath, maNativeClipPath, mbInvert, mbXOR, getAntiAliasB2DDraw(), 0x00000000, mnLineColor, aPath, bShiftLines, fNativeLineWidth, eLineJoin, false, nLineCap ) );
			CGPathRelease( aPath );
		}

		setLineTransparency( 0 );
	}

	return bRet;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawPolyLineBezier( sal_uInt32 /* nPoints */, const SalPoint* /* pPtAry */, const sal_uInt8* /* pFlgAry */ )
{
	return false;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawPolygonBezier( sal_uInt32 /* nPoints */, const SalPoint* /* pPtAry */, const sal_uInt8* /* pFlgAry */ )
{
	return false;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawPolyPolygonBezier( sal_uInt32 /* nPoly */, const sal_uInt32* /* nPoints */, const SalPoint* const* /* pPtAry */, const sal_uInt8* const* /* pFlgAry */ )
{
	return false;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize )
{
	bool bRet = false;

	if ( pPtr && nSize )
	{
		void *pPtrCopy = rtl_allocateMemory( nSize );
		if ( pPtrCopy )
		{
			memcpy( pPtrCopy, pPtr, nSize );

			// Assign ownership of bits to a CFData instance
			CFDataRef aData = CFDataCreateWithBytesNoCopy( NULL, (UInt8 *)pPtrCopy, nSize, NULL );
			if ( aData )
			{
				CGRect aUnflippedRect = UnflipFlippedRect( CGRectMake( nX, nY, nWidth, nHeight ), maNativeBounds );
				addUndrawnNativeOp( new JavaSalGraphicsDrawEPSOp( maFrameClipPath, maNativeClipPath, aData, aUnflippedRect ) );
				CFRelease( aData );
			}

			bRet = true;
		}
	}

	return bRet;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::setLineTransparency( sal_uInt16 nTransparency )
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

void JavaSalGraphics::setFillTransparency( sal_uInt16 nTransparency )
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

void JavaSalGraphics::setFrameClipPath( CGPathRef aFrameClipPath )
{
	if ( aFrameClipPath != maFrameClipPath )
	{
		if ( maFrameClipPath )
			CGPathRelease( maFrameClipPath );
		maFrameClipPath = aFrameClipPath;
		if ( maFrameClipPath )
			CGPathRetain( maFrameClipPath );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::addNeedsDisplayRect( const CGRect aRect, float fLineWidth )
{
	if ( !mpFrame || CGRectIsEmpty( aRect ) )
		return;

	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	float fNativeLineWidth = fLineWidth;
	if ( fNativeLineWidth <= 0 )
		fNativeLineWidth = getNativeLineWidth();

	maNeedsDisplayRect = CGRectUnion( maNeedsDisplayRect, CGRectMake( aRect.origin.x - fNativeLineWidth, aRect.origin.y - fNativeLineWidth, aRect.size.width + ( fNativeLineWidth * 2 ), aRect.size.height + ( fNativeLineWidth * 2 ) ) );
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
			drawUndrawnNativeOps( aContext, maNativeBounds );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::copyFromGraphics( JavaSalGraphics *pSrcGraphics, CGRect aSrcRect, CGRect aDestRect, bool bAllowXOR )
{
	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	if ( !pSrcGraphics || !maLayer )
		return;

	CGContextRef aContext = CGLayerGetContext( maLayer );
	if ( aContext )
	{
		// Draw any undrawn operations so that we copy the latest bits
		drawUndrawnNativeOps( aContext, maNativeBounds );

		CGRect aDrawBounds = aDestRect;
		if ( !CGRectIsEmpty( maNativeBounds ) )
			aDrawBounds = CGRectIntersection( aDrawBounds, maNativeBounds );
		if ( maFrameClipPath )
			aDrawBounds = CGRectIntersection( aDrawBounds, CGPathGetBoundingBox( maFrameClipPath ) );
		if ( maNativeClipPath )
			aDrawBounds = CGRectIntersection( aDrawBounds, CGPathGetBoundingBox( maNativeClipPath ) );
		if ( CGRectIsEmpty( aDrawBounds ) )
			return;

		CGContextSaveGState( aContext );

		bool bInvert = ( mbInvert && bAllowXOR ? true : false );
		bool bXOR = ( mbXOR && bAllowXOR ? true : false );
		pSrcGraphics->copyToContext( maFrameClipPath, maNativeClipPath, bInvert, bXOR, aContext, maNativeBounds, aSrcRect, aDestRect, false, false );

		CGContextRestoreGState( aContext );

		if ( mpFrame )
			addNeedsDisplayRect( aDrawBounds, getNativeLineWidth() );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::copyToContext( const CGPathRef aFrameClipPath, const CGPathRef aNativeClipPath, bool bInvert, bool bXOR, CGContextRef aDestContext, CGRect aDestBounds, CGRect aSrcRect, CGRect aDestRect, bool bDestIsWindow, bool bDestIsUnflipped )
{
	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	if ( !aDestContext || !maLayer )
		return;

	// If source and destination height do not match, adjust so that they
	// align coordinates so that the source's top get copies to the
	// destination's top
	CGSize aLayerSize = CGLayerGetSize( maLayer );
	if ( bDestIsUnflipped && !CGRectIsNull( aDestBounds ) )
		aSrcRect.origin.y += aLayerSize.height - aDestBounds.size.height;

	// Draw any undrawn operations so that we copy the latest bits
	CGContextRef aContext = CGLayerGetContext( maLayer );
	if ( aContext )
		drawUndrawnNativeOps( aContext, maNativeBounds );

	if ( bDestIsWindow && mpFrame && mnBackgroundColor && !CGRectIsEmpty( aDestRect ) )
	{
		// If the layer does not fully cover the window, paint the fill color
		// in the uncovered areas so that no drawing artifacts remain after
		// resizing a window. Fix doubling of transparent and antialias pixels
		// reported in the following NeoOffice forum post by drawing the
		// background color in covered areas as well:
		// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=62905#62905
		CGColorRef aBackgroundColor = CreateCGColorFromSalColor( mnBackgroundColor );
		if ( aBackgroundColor )
		{
			CGContextSaveGState( aDestContext );
			CGContextBeginPath( aDestContext );
			CGContextAddRect( aDestContext, aDestRect );
			CGContextSetFillColorWithColor( aDestContext, aBackgroundColor );
			CGContextFillPath( aDestContext );
			CGContextRestoreGState( aDestContext );

			CGColorRelease( aBackgroundColor );
		}
	}

	// Do not queue this operation since we are copying to another context
	JavaSalGraphicsCopyLayerOp aOp( aFrameClipPath, aNativeClipPath, bInvert, bXOR, maLayer, aSrcRect, aDestRect );
	aOp.drawOp( this, aDestContext, aDestBounds );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawUndrawnNativeOps( CGContextRef aContext, CGRect aBounds )
{
	if ( !aContext )
		return;

	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	CGContextSaveGState( aContext );

	// Scale printer context to match OOo resolution
	if ( mpPrinter )
	{
		sal_Int32 nDPIX;
		sal_Int32 nDPIY;
		GetResolution( nDPIX, nDPIY );
		if ( nDPIX && nDPIY && !CGRectIsEmpty( aBounds ) )
		{
			float fScaleX = (float)72 / nDPIX;
			float fScaleY = (float)72 / nDPIY;
			CGContextScaleCTM( aContext, fScaleX, fScaleY );
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

sal_uLong JavaSalGraphics::getBitmapDirectionFormat()
{
	return JavaSalBitmap::GetNativeDirectionFormat();
}

// -----------------------------------------------------------------------

float JavaSalGraphics::getNativeLineWidth()
{
	// Fix printing bug reported in the following forum post by not using
	// thicker line widths for printers:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=63125#63125
	return 1.0f;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::setBackgroundColor( SalColor nBackgroundColor )
{
	if ( !mpFrame )
		return;

	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	mnBackgroundColor = nBackgroundColor;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::setLayer( CGLayerRef aLayer )
{
	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	if ( aLayer != maLayer )
	{
		if ( mpFrame && aLayer )
		{
			CGSize aLayerSize = CGLayerGetSize( aLayer );
			CGContextRef aContext = CGLayerGetContext( aLayer );
			if ( aContext )
			{
				// Clear background
				CGColorRef aBackgroundColor = CreateCGColorFromSalColor( mnBackgroundColor );
				if ( aBackgroundColor )
				{
					CGContextSaveGState( aContext );
					CGContextBeginPath( aContext );
					CGContextAddRect( aContext, CGRectMake( 0, 0, aLayerSize.width, aLayerSize.height ) );
					CGContextSetFillColorWithColor( aContext, aBackgroundColor );
					CGContextFillPath( aContext );
					CGContextRestoreGState( aContext );

					CGColorRelease( aBackgroundColor );
				}

				// Copy old layer to new layer
				if ( maLayer )
				{
					CGSize aOldLayerSize = CGLayerGetSize( maLayer );
					CGContextDrawLayerAtPoint( aContext, CGPointMake( 0, aLayerSize.height - aOldLayerSize.height ), maLayer );
				}
			}
		}

		if ( maLayer )
			CGLayerRelease( maLayer );
		maLayer = aLayer;
		if ( maLayer )
		{
			CGLayerRetain( maLayer );

			// Fix black window corners in presentation minimizer floating
			// toolbar when running on OS X 10.11 by forcing the entire layer
			// to be displayed
			addNeedsDisplayRect( maNativeBounds, 0 );
		}
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::setNeedsDisplay( NSView *pView )
{
	if ( !pView )
		return;

	NSWindow *pWindow = [pView window];
	if ( !pWindow || ![pWindow isVisible] )
	{
		[pView setNeedsDisplay:NO];
		return;
	}

	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	if ( maLayer && !CGRectIsEmpty( maNeedsDisplayRect ) )
	{
		CGSize aLayerSize = CGLayerGetSize( maLayer );
		NSRect aBounds = [pView bounds];
		NSRect aDirtyRect = NSRectFromCGRect( maNeedsDisplayRect );

		// If view is flipped, flip coordinates. If view is unflipped and
		// source and destination height do not match, adjust so that they
		// align coordinates so that the source's top matches the destination's
		// top
		if ( [pView isFlipped] )
			aDirtyRect.origin.y = aBounds.origin.y + aBounds.size.height - aDirtyRect.origin.y - aDirtyRect.size.height;
		else
			aDirtyRect.origin.y += aBounds.size.height - aLayerSize.height;

		aDirtyRect = NSIntersectionRect( aDirtyRect, aBounds );
		if ( aDirtyRect.size.width > 0 && aDirtyRect.size.height > 0 )
			[pView setNeedsDisplayInRect:aDirtyRect];
	}

	maNeedsDisplayRect = CGRectNull;
}

// =======================================================================

JavaSalGraphicsOp::JavaSalGraphicsOp( const CGPathRef aFrameClipPath, const CGPathRef aNativeClipPath, bool bInvert, bool bXOR, float fLineWidth ) :
	maFrameClipPath( NULL ),
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
	if ( aFrameClipPath )
		maFrameClipPath = CGPathCreateCopy( aFrameClipPath );

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

	if ( maFrameClipPath )
		CGPathRelease( maFrameClipPath );

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
						// Fix Retina display bug reported in the following
						// NeoOffice forum topic by making the temporary bitmaps
						// have the same bytes per pixel that the layer has:
						// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8493
						float fBackingFactor = JavaSalGraphics::getContextBackingFactor( maSavedContext );
						if ( fBackingFactor < 1.0f )
							fBackingFactor = 1.0f;

						CGContextDrawImage( maSavedContext, CGRectMake( maXORRect.origin.x - mnXORBitmapPadding, maXORRect.origin.y - mnXORBitmapPadding, (float)nBitmapWidth / fBackingFactor, (float)nBitmapHeight / fBackingFactor ), aImage );
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

		CGContextSetAllowsAntialiasing( maSavedContext, true );
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
					// Fix Retina display bug reported in the following
					// NeoOffice forum topic by making the temporary bitmaps
					// have the same bytes per pixel that the layer has:
					// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8493
					float fBackingFactor = JavaSalGraphics::getContextBackingFactor( aContext );
					if ( fBackingFactor < 1.0f )
						fBackingFactor = 1.0f;

					CGSize aBitmapSize = CGSizeMake( maXORRect.size.width + ( mnXORBitmapPadding * 2 ), maXORRect.size.height + ( mnXORBitmapPadding * 2 ) );
					aBitmapSize.width *= fBackingFactor;
					aBitmapSize.height *= fBackingFactor;
					long nScanlineSize = AlignedWidth4Bytes( 32 * aBitmapSize.width );
					mnBitmapCapacity = nScanlineSize * aBitmapSize.height;
					try
					{
						mpDrawBits = new sal_uInt8[ mnBitmapCapacity ];
						mpXORBits = new sal_uInt8[ mnBitmapCapacity ];
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
							CGContextTranslateCTM( maDrawBitmapContext, ( mnXORBitmapPadding - maXORRect.origin.x ) * fBackingFactor, ( mnXORBitmapPadding - maXORRect.origin.y ) * fBackingFactor );
							CGContextScaleCTM( maDrawBitmapContext, fBackingFactor, fBackingFactor );

							JavaSalGraphics::setContextDefaultSettings( maDrawBitmapContext, maFrameClipPath, maNativeClipPath, pGraphics->getNativeLineWidth() );

							// Copy layer to XOR context
							CGContextTranslateCTM( maXORBitmapContext, ( mnXORBitmapPadding - maXORRect.origin.x ) * fBackingFactor, ( mnXORBitmapPadding - maXORRect.origin.y ) * fBackingFactor );
							CGContextScaleCTM( maXORBitmapContext, fBackingFactor, fBackingFactor );
							CGContextDrawLayerAtPoint( maXORBitmapContext, CGPointMake( 0, 0 ), maXORLayer );

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

	JavaSalGraphics::setContextDefaultSettings( maSavedContext, maFrameClipPath, maNativeClipPath, pGraphics->getNativeLineWidth() );

	if ( mbInvert )
		CGContextSetBlendMode( maSavedContext, kCGBlendModeDifference );

	return maDrawBitmapContext ? maDrawBitmapContext : maSavedContext;
}
