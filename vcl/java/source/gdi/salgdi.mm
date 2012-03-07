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

#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

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
							JavaSalGraphicsDrawLineOp( const CGPathRef aNativeClipPath, bool bXOR, float fX1, float fY1, float fX2, float fY2, SalColor nColor ) : JavaSalGraphicsOp( aNativeClipPath, bXOR ), mfX1( fX1 ), mfY1( fY1 ), mfX2( fX2 ), mfY2( fY2 ), mnColor( nColor ) {}
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
							JavaSalGraphicsDrawPathOp( const CGPathRef aNativeClipPath, bool bXOR, bool bAntialias, SalColor nFillColor, SalColor nLineColor, const CGPathRef aPath, float fLineWidth = 0, ::basegfx::B2DLineJoin eLineJoin = ::basegfx::B2DLINEJOIN_NONE );
	virtual					~JavaSalGraphicsDrawPathOp();

	virtual	void			drawOp( CGContextRef aContext, CGRect aBounds );
};

class SAL_DLLPRIVATE JavaSalGraphicsDrawRectOp : public JavaSalGraphicsOp
{
	CGRect					maRect;
	SalColor				mnFillColor;
	SalColor				mnLineColor;

public:
							JavaSalGraphicsDrawRectOp( const CGPathRef aNativeClipPath, bool bXOR, const CGRect aRect, SalColor nFillColor, SalColor nLineColor ) : JavaSalGraphicsOp( aNativeClipPath, bXOR ), maRect( aRect ), mnFillColor( nFillColor ), mnLineColor( nLineColor ) {}
	virtual					~JavaSalGraphicsDrawRectOp() {}

	virtual	void			drawOp( CGContextRef aContext, CGRect aBounds );
};

#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE

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

#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE

// =======================================================================

CGColorRef CreateCGColorFromSalColor( SalColor nColor )
{
	return CGColorCreateGenericRGB( (float)( ( nColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( nColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( nColor & 0x000000ff ) / (float)0xff, (float)( ( nColor & 0xff000000 ) >> 24 ) / (float)0xff );
}

// =======================================================================

JavaSalGraphicsDrawEPSOp::JavaSalGraphicsDrawEPSOp( const CGPathRef aNativeClipPath, CFDataRef aData, const CGRect aRect ) :
	JavaSalGraphicsOp( aNativeClipPath, false ),
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

	if ( !CGRectIsNull( aBounds ) )
	{
		if ( !CGRectIntersectsRect( aBounds, maRect ) )
			return;
		else if ( maNativeClipPath && !CGRectIntersectsRect( aBounds, CGPathGetBoundingBox( maNativeClipPath ) ) )
			return;
	}

	saveClipXORGState( aContext );

	// CGImage's assume flipped coordinates when drawing so draw from the
	// bottom up
	NSImageRep *pImageRep = [NSEPSImageRep imageRepWithData:(NSData *)maData];
	if ( !pImageRep )
		pImageRep = [NSPDFImageRep imageRepWithData:(NSData *)maData];
	if ( pImageRep )
		[pImageRep drawInRect:NSMakeRect( maRect.origin.x, maRect.origin.y + maRect.size.height, maRect.size.width, maRect.size.height * -1 )];

	restoreGState( aContext );
}

// =======================================================================

void JavaSalGraphicsDrawLineOp::drawOp( CGContextRef aContext, CGRect aBounds )
{
	if ( !aContext )
		return;

	if ( !CGRectIsNull( aBounds ) )
	{
		if ( !CGRectContainsPoint( aBounds, CGPointMake( mfX1, mfY1 ) ) && !CGRectContainsPoint( aBounds, CGPointMake( mfX2, mfY2 ) ) )
			return;
		else if ( maNativeClipPath && !CGRectIntersectsRect( aBounds, CGPathGetBoundingBox( maNativeClipPath ) ) )
			return;
	}

	CGColorRef aColor = CreateCGColorFromSalColor( mnColor );
	if ( aColor )
	{
		if ( CGColorGetAlpha( aColor ) )
		{
			saveClipXORGState( aContext );

			CGContextSetStrokeColorWithColor( aContext, aColor );
			CGContextMoveToPoint( aContext, mfX1, mfY1 );
			CGContextAddLineToPoint( aContext, mfX2, mfY2 );
			CGContextStrokePath( aContext );

			restoreGState( aContext );
		}

		CGColorRelease( aColor );
	}
}

// =======================================================================

JavaSalGraphicsDrawPathOp::JavaSalGraphicsDrawPathOp( const CGPathRef aNativeClipPath, bool bXOR, bool bAntialias, SalColor nFillColor, SalColor nLineColor, const CGPathRef aPath, float fLineWidth, ::basegfx::B2DLineJoin eLineJoin ) :
	JavaSalGraphicsOp( aNativeClipPath, bXOR ),
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

	if ( !CGRectIsNull( aBounds ) )
	{
		if ( !CGRectIntersectsRect( aBounds, CGPathGetBoundingBox( maPath ) ) )
			return;
		else if ( maNativeClipPath && !CGRectIntersectsRect( aBounds, CGPathGetBoundingBox( maNativeClipPath ) ) )
			return;
	}

	CGColorRef aFillColor = CreateCGColorFromSalColor( mnFillColor );
	if ( aFillColor )
	{
		CGColorRef aLineColor = CreateCGColorFromSalColor( mnLineColor );
		if ( aLineColor )
		{
			saveClipXORGState( aContext );

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

			CGContextAddPath( aContext, maPath );
			if ( CGColorGetAlpha( aFillColor ) )
			{
				// Smooth out image drawing for bug 2475 image
				if ( mbXOR && !mbAntialias )
					CGContextSetAllowsAntialiasing( aContext, true );
				CGContextSetFillColorWithColor( aContext, aFillColor );
				CGContextEOFillPath( aContext );
				CGContextSetAllowsAntialiasing( aContext, mbAntialias );
			}
			if ( CGColorGetAlpha( aLineColor ) )
			{
				CGContextSetStrokeColorWithColor( aContext, aLineColor );
				CGContextStrokePath( aContext );
			}

			restoreGState( aContext );

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

	if ( !CGRectIsNull( aBounds ) )
	{
		if ( !CGRectIntersectsRect( aBounds, maRect ) )
			return;
		else if ( maNativeClipPath && !CGRectIntersectsRect( aBounds, CGPathGetBoundingBox( maNativeClipPath ) ) )
			return;
	}

	CGColorRef aFillColor = CreateCGColorFromSalColor( mnFillColor );
	if ( aFillColor )
	{
		CGColorRef aLineColor = CreateCGColorFromSalColor( mnLineColor );
		if ( aLineColor )
		{
			saveClipXORGState( aContext );

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

			restoreGState( aContext );

			CGColorRelease( aLineColor );
		}

		CGColorRelease( aFillColor );
	}
}

#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE

// =======================================================================

JavaSalGraphics::JavaSalGraphics() :
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
	maNativeClipPath( NULL )
#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE
	, mbXOR( false )
	, meOrientation( ORIENTATION_PORTRAIT )
	, mbPaperRotated( sal_False )
	, maLayer( NULL )
#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE
{
	GetSalData()->maGraphicsList.push_back( this );
}

// -----------------------------------------------------------------------

JavaSalGraphics::~JavaSalGraphics()
{
	GetSalData()->maGraphicsList.remove( this );

	if ( mpFontData )
		delete mpFontData;

	if ( mpVCLFont )
		delete mpVCLFont;

	for ( ::std::hash_map< int, com_sun_star_vcl_VCLFont* >::const_iterator it = maFallbackFonts.begin(); it != maFallbackFonts.end(); ++it )
		delete it->second;

	if ( maNativeClipPath )
		CFRelease( maNativeClipPath );

#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE
	while ( maUndrawnNativeOpsList.size() )
	{
		JavaSalGraphicsOp *pOp = maUndrawnNativeOpsList.front();
		delete pOp;
		maUndrawnNativeOpsList.pop_front();
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
#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE
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
#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE
	if ( useNativeDrawing() )
		return 32;
#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE

	return mpVCLGraphics->getBitCount();
}

// -----------------------------------------------------------------------

void JavaSalGraphics::ResetClipRegion()
{
#ifdef USE_NATIVE_VIRTUAL_DEVICE
	if ( mpPrinter || mpVirDev )
#else	// USE_NATIVE_VIRTUAL_DEVICE
	if ( mpPrinter )
#endif	// USE_NATIVE_VIRTUAL_DEVICE
	{
		if ( maNativeClipPath )
		{
			CFRelease( maNativeClipPath );
			maNativeClipPath = NULL;
		}
	}
	else
	{
		mpVCLGraphics->resetClipRegion( sal_False );
	}
}

// -----------------------------------------------------------------------

void JavaSalGraphics::BeginSetClipRegion( ULONG nRectCount )
{
#ifdef USE_NATIVE_VIRTUAL_DEVICE
	if ( mpPrinter || mpVirDev )
#else	// USE_NATIVE_VIRTUAL_DEVICE
	if ( mpPrinter )
#endif	// USE_NATIVE_VIRTUAL_DEVICE
		ResetClipRegion();
	else
		mpVCLGraphics->beginSetClipRegion( sal_False );
}

// -----------------------------------------------------------------------

BOOL JavaSalGraphics::unionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
	BOOL bRet = TRUE;

#ifdef USE_NATIVE_VIRTUAL_DEVICE
	if ( mpPrinter || mpVirDev )
#else	// USE_NATIVE_VIRTUAL_DEVICE
	if ( mpPrinter )
#endif	// USE_NATIVE_VIRTUAL_DEVICE
	{
		if ( nWidth > 0 && nHeight > 0 )
		{
			if ( !maNativeClipPath )
				maNativeClipPath = CGPathCreateMutable();

			if ( maNativeClipPath )
				CGPathAddRect( maNativeClipPath, NULL, CGRectMake( (float)nX, (float)nY, (float)nWidth, (float)nHeight ) );
		}
	}
	else
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
#ifdef USE_NATIVE_VIRTUAL_DEVICE
		if ( mpPrinter || mpVirDev )
#else	// USE_NATIVE_VIRTUAL_DEVICE
		if ( mpPrinter )
#endif	// USE_NATIVE_VIRTUAL_DEVICE
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
#ifdef USE_NATIVE_VIRTUAL_DEVICE
	if ( !mpPrinter && !mpVirDev )
#else	// USE_NATIVE_VIRTUAL_DEVICE
	if ( !mpPrinter )
#endif	// USE_NATIVE_VIRTUAL_DEVICE
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
	{
#ifdef USE_NATIVE_PRINTING
		mbXOR = false;
#endif	// USE_NATIVE_PRINTING
		return;
	}

#ifdef USE_NATIVE_VIRTUAL_DEVICE
	if ( mpVirDev )
	{
		mbXOR = bSet;
		return;
	}
#endif	// USE_NATIVE_VIRTUAL_DEVICE

	// Ignore the bInvertOnly parameter as it is not used by Windows or X11
	// platforms
	mpVCLGraphics->setXORMode( bSet );
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
#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE
	if ( useNativeDrawing() )
	{
		addUndrawnNativeOp( new JavaSalGraphicsDrawRectOp( maNativeClipPath, mbXOR, CGRectMake( nX, nY, 1, 1 ), 0x00000000, mnLineColor ) );
		return;
	}
#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE

	if ( mnLineColor )
		mpVCLGraphics->setPixel( nX, nY, mnLineColor, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE
	if ( useNativeDrawing() )
	{
		addUndrawnNativeOp( new JavaSalGraphicsDrawRectOp( maNativeClipPath, mbXOR, CGRectMake( nX, nY, 1, 1 ), 0x00000000, nSalColor | 0xff000000 ) );
		return;
	}
#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE

	mpVCLGraphics->setPixel( nX, nY, nSalColor | 0xff000000, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE
	if ( useNativeDrawing() )
	{
		addUndrawnNativeOp( new JavaSalGraphicsDrawLineOp( maNativeClipPath, mbXOR, (float)nX1, (float)nY1, (float)nX2, (float)nY2, mnLineColor ) );
		return;
	}
#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE

	if ( mnLineColor )
		mpVCLGraphics->drawLine( nX1, nY1, nX2, nY2, mnLineColor, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE
	if ( useNativeDrawing() )
	{
		CGRect aRect = CGRectStandardize( CGRectMake( nX, nY, nWidth, nHeight ) );
		if ( CGRectIsEmpty( aRect ) )
		{
			drawLine( nX, nY, nX + nWidth, nY + nHeight );
			return;
		}

		addUndrawnNativeOp( new JavaSalGraphicsDrawRectOp( maNativeClipPath, mbXOR, aRect, mnFillColor, mnLineColor ) );
		return;
	}
#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE

	if ( mnFillColor )
		mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, mnFillColor, TRUE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
	if ( mnLineColor )
		mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, mnLineColor, FALSE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
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
#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE
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
				addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbXOR, false, 0x00000000, mnLineColor, aPath ) );
				CGPathRelease( aPath );
			}
		}
		return;
	}
#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE

	if ( mnLineColor )
		mpVCLGraphics->drawPolyline( nPoints, pPtAry, mnLineColor, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE
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
				addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbXOR, false, mnFillColor, mnLineColor, aPath ) );
				CGPathRelease( aPath );
			}
		}
		return;
	}
#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE

	if ( mnFillColor )
		mpVCLGraphics->drawPolygon( nPoints, pPtAry, mnFillColor, TRUE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
	if ( mnLineColor )
		mpVCLGraphics->drawPolygon( nPoints, pPtAry, mnLineColor, FALSE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolyPolygon( ULONG nPoly, const ULONG* pPoints, PCONSTSALPOINT* pPtAry )
{
#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE
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
				addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbXOR, false, mnFillColor, mnLineColor, aPath ) );
				CGPathRelease( aPath );
			}
		}
		return;
	}
#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE

	if ( mnFillColor )
		mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pPtAry, mnFillColor, TRUE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
	if ( mnLineColor )
		mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pPtAry, mnLineColor, FALSE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly, double fTransparency )
{
	bool bRet = true;

#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE
	if ( useNativeDrawing() )
	{
		CGMutablePathRef aPath = CGPathCreateMutable();
		if ( aPath )
		{
			sal_uInt8 nTransparency = (sal_uInt8)( ( fTransparency * 100 ) + 0.5 );
			setFillTransparency( nTransparency );
			setLineTransparency( nTransparency );
			AddPolyPolygonToPaths( NULL, aPath, rPolyPoly );
			addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbXOR, getAntiAliasB2DDraw(), mnFillColor, mnLineColor, aPath ) );
			setFillTransparency( 0 );
			setLineTransparency( 0 );
			CGPathRelease( aPath );
		}
		return bRet;
	}
#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE

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

	return bRet;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawPolyLine( const ::basegfx::B2DPolygon& rPoly, const ::basegfx::B2DVector& rLineWidths, ::basegfx::B2DLineJoin eLineJoin )
{
	bool bRet = true;

#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE
	if ( useNativeDrawing() )
	{
		CGMutablePathRef aPath = CGPathCreateMutable();
		if ( aPath )
		{
			AddPolygonToPaths( NULL, aPath, rPoly, rPoly.isClosed() );
			addUndrawnNativeOp( new JavaSalGraphicsDrawPathOp( maNativeClipPath, mbXOR, getAntiAliasB2DDraw(), 0x00000000, mnLineColor, aPath, rLineWidths.getX(), eLineJoin ) );
			CGPathRelease( aPath );
		}
		return bRet;
	}
#else	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE
	if ( mpPrinter )
		return false;
#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE

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
#ifdef USE_NATIVE_VIRTUAL_DEVICE
		if ( mpPrinter || mpVirDev )
#else	// USE_NATIVE_VIRTUAL_DEVICE
		if ( mpPrinter )
#endif	// USE_NATIVE_VIRTUAL_DEVICE
		{
			void *pPtrCopy = rtl_allocateMemory( nSize );
			if ( pPtrCopy )
			{
				memcpy( pPtrCopy, pPtr, nSize );
#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE
				// Assign ownership of bits to a CFData instance
				CFDataRef aData = CFDataCreateWithBytesNoCopy( NULL, (UInt8 *)pPtrCopy, nSize, NULL );
				if ( aData )
				{
					addUndrawnNativeOp( new JavaSalGraphicsDrawEPSOp( maNativeClipPath, aData, CGRectMake( nX, nY, nWidth, nHeight ) ) );
					bRet = TRUE;
					CFRelease( aData );
				}
#else	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE
				// Don't delete the copied buffer and let the Java native
				// method print the buffer directly
				mpVCLGraphics->drawEPS( pPtrCopy, nSize, nX, nY, nWidth, nHeight, maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
				bRet = TRUE;
#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE
			}
		}

#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE
		if ( !bRet && !useNativeDrawing() )
#else	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE
		if ( !bRet )
#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE
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

#ifdef USE_NATIVE_PRINTING
	if ( mpPrinter )
		bRet = true;
#endif	// USE_NATIVE_PRINTING
#ifdef USE_NATIVE_VIRTUAL_DEVICE
	if ( mpVirDev )
		bRet = true;
#endif	// USE_NATIVE_VIRTUAL_DEVICE

	return bRet;
}

#if defined USE_NATIVE_PRINTING || defined USE_NATIVE_VIRTUAL_DEVICE

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

void JavaSalGraphics::drawUndrawnNativeOps( CGContextRef aContext, CGRect aBounds )
{
	if ( !aContext )
		return;

	ClearableMutexGuard aGuard( maUndrawnNativeOpsMutex );

	// Notify graphics change listeners
	while ( maGraphicsChangeListenerList.size() )
	{
		JavaSalBitmap *pBitmap = maGraphicsChangeListenerList.front();
		maGraphicsChangeListenerList.pop_front();
		pBitmap->NotifyGraphicsChanged( false );
	}

	::std::list< JavaSalGraphicsOp* > aOpsList( maUndrawnNativeOpsList );
	maUndrawnNativeOpsList.clear();

	aGuard.clear();

	CGContextSaveGState( aContext );

	// Turn off antialiasing by default since we did the same in the Java code
	CGContextSetAllowsAntialiasing( aContext, false );

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
			if ( !CGRectIsNull( aBounds ) )
				aBounds = CGRectMake( aBounds.origin.x / fScaleX, aBounds.origin.y / fScaleY, aBounds.size.width / fScaleX, aBounds.size.height / fScaleY );
		}
	}

	// Scale line width
	CGContextSetLineWidth( aContext, getNativeLineWidth() );

	while ( aOpsList.size() )
	{
		JavaSalGraphicsOp *pOp = aOpsList.front();
		pOp->drawOp( aContext, aBounds );
		delete pOp;
		aOpsList.pop_front();
	}

	CGContextRestoreGState( aContext );
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

	if ( maLayer )
		CGLayerRelease( maLayer );
	maLayer = aLayer;
	if ( maLayer )
		CGLayerRetain( maLayer );
}

// =======================================================================

JavaSalGraphicsOp::JavaSalGraphicsOp( const CGPathRef aNativeClipPath, bool bXOR ) :
	maNativeClipPath( NULL ),
	mbXOR( bXOR )
{
	if ( aNativeClipPath )
		maNativeClipPath = CGPathCreateCopy( aNativeClipPath );
}

// -----------------------------------------------------------------------

JavaSalGraphicsOp::~JavaSalGraphicsOp()
{
	if ( maNativeClipPath )
		CGPathRelease( maNativeClipPath );
}

// -----------------------------------------------------------------------

void JavaSalGraphicsOp::restoreGState( CGContextRef aContext )
{
	if ( !aContext )
		return;

	CGContextRestoreGState( aContext );
}

// -----------------------------------------------------------------------

void JavaSalGraphicsOp::saveClipXORGState( CGContextRef aContext )
{
	if ( !aContext )
		return;

	CGContextSaveGState( aContext );

	if ( maNativeClipPath )
	{
		CGContextBeginPath( aContext );
		CGContextAddPath( aContext, maNativeClipPath );
		CGContextClip( aContext );
	}

	if ( mbXOR )
	{
		// Mac OS X's XOR blend mode does not do real XORing of bits so we
		// reimplement our own XORing
		fprintf( stderr, "JavaSalGraphicsOp::saveClipXORGState XOR not implemented\n" );
	}

	// Throw away any incomplete path
	CGContextBeginPath( aContext );
}

#endif	// USE_NATIVE_PRINTING || USE_NATIVE_VIRTUAL_DEVICE
