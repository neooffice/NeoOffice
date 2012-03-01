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

#ifdef USE_NATIVE_PRINTING

class SAL_DLLPRIVATE JavaSalGraphicsDrawRectOp : public JavaSalGraphicsOp
{
	CGRect					maRect;
	SalColor				mnColor;
	bool					mbFill;

public:
							JavaSalGraphicsDrawRectOp( const CGPathRef aNativeClipPath, bool bXOR, const CGRect aRect, SalColor nColor, bool bFill ) : JavaSalGraphicsOp( aNativeClipPath, bXOR ), maRect( aRect ), mnColor( nColor ), mbFill( bFill ) {}
	virtual					~JavaSalGraphicsDrawRectOp() {}

	virtual	void			drawOp( CGContextRef aContext );
};

#endif	// USE_NATIVE_PRINTING

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

#ifdef USE_NATIVE_PRINTING

// =======================================================================

CGColorRef CreateCGColorFromSalColor( SalColor nColor )
{
	return CGColorCreateGenericRGB( (float)( ( nColor & 0x00ff0000 ) >> 16 ) / (float)0xff, (float)( ( nColor & 0x0000ff00 ) >> 8 ) / (float)0xff, (float)( nColor & 0x000000ff ) / (float)0xff, (float)( ( nColor & 0xff000000 ) >> 24 ) / (float)0xff );
}

// =======================================================================

void JavaSalGraphicsDrawRectOp::drawOp( CGContextRef aContext )
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

	CGColorRef aColor = CreateCGColorFromSalColor( mnColor );
	if ( aColor )
	{
		if ( mbXOR )
			CGContextSetBlendMode( aContext, kCGBlendModeXOR );

		if ( mbFill )
		{
			CGContextSetFillColorWithColor( aContext, aColor );
			CGContextFillRect( aContext, maRect );
		}
		else
		{
			CGContextSetStrokeColorWithColor( aContext, aColor );
			CGContextStrokeRect( aContext, maRect );
		}

		CGColorRelease( aColor );
	}

	CGContextRestoreGState( aContext );
}

#endif	// USE_NATIVE_PRINTING

// =======================================================================

JavaSalGraphics::JavaSalGraphics() :
	mnFillColor( MAKE_SALCOLOR( 0xff, 0xff, 0xff ) | 0xff000000 ),
	mnLineColor( MAKE_SALCOLOR( 0, 0, 0 ) | 0xff000000 ),
	mnTextColor( MAKE_SALCOLOR( 0, 0, 0 ) | 0xff000000 ),
	mnFillTransparency( 0xff000000 ),
	mnLineTransparency( 0xff000000 ),
	mpFrame( NULL ),
#ifdef USE_NATIVE_PRINTING
	mpInfoPrinter( NULL ),
#endif	// USE_NATIVE_PRINTING
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
#ifdef USE_NATIVE_PRINTING
	, meOrientation( ORIENTATION_PORTRAIT )
	, mbPaperRotated( sal_False )
#endif	// USE_NATIVE_PRINTING
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

#ifdef USE_NATIVE_PRINTING
	while ( maUndrawnNativeOpsList.size() )
	{
		JavaSalGraphicsOp *pOp = maUndrawnNativeOpsList.front();
		delete pOp;
		maUndrawnNativeOpsList.pop_front();
	}
#endif	// USE_NATIVE_PRINTING
}

// -----------------------------------------------------------------------

void JavaSalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter || mpPrinter )
	{
		rDPIX = MIN_PRINTER_RESOLUTION;
		rDPIY = MIN_PRINTER_RESOLUTION;
		return;
	}
	else
	{
#endif	// USE_NATIVE_PRINTING
	if ( !mnDPIX || !mnDPIY )
	{
		Size aSize( mpVCLGraphics->getResolution() );
		mnDPIX = aSize.Width();
		mnDPIY = aSize.Height();
	}

	rDPIX = mnDPIX;
	rDPIY = mnDPIY;
#ifdef USE_NATIVE_PRINTING
	}
#endif	// USE_NATIVE_PRINTING
}

// -----------------------------------------------------------------------

USHORT JavaSalGraphics::GetBitCount()
{
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter || mpPrinter )
		return 32;
#endif	// USE_NATIVE_PRINTING

	return mpVCLGraphics->getBitCount();
}

// -----------------------------------------------------------------------

void JavaSalGraphics::ResetClipRegion()
{
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter )
		return;
#endif	// USE_NATIVE_PRINTING

	if ( mpPrinter )
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
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter )
		return;
#endif	// USE_NATIVE_PRINTING

	if ( mpPrinter )
		ResetClipRegion();
	else
		mpVCLGraphics->beginSetClipRegion( sal_False );
}

// -----------------------------------------------------------------------

BOOL JavaSalGraphics::unionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter )
		return FALSE;
#endif	// USE_NATIVE_PRINTING

	if ( mpPrinter )
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

	return TRUE;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::unionClipRegion( const ::basegfx::B2DPolyPolygon& rPolyPoly )
{
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter )
		return false;
#endif	// USE_NATIVE_PRINTING

	const sal_uInt32 nPoly = rPolyPoly.count();
	if ( nPoly )
	{
		if ( mpPrinter )
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

	return true;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::EndSetClipRegion()
{
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter )
		return;
#endif	// USE_NATIVE_PRINTING

	if ( !mpPrinter )
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
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter || mpPrinter )
#else	// USE_NATIVE_PRINTING
	if ( mpPrinter )
#endif	// USE_NATIVE_PRINTING
		return;

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
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter )
		return;

	if ( mpPrinter )
	{
		fprintf( stderr, "JavaSalGraphics::drawPixel not implemented\n" );
		return;
	}
#endif	// USE_NATIVE_PRINTING

	if ( mnLineColor )
		mpVCLGraphics->setPixel( nX, nY, mnLineColor, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter )
		return;

	if ( mpPrinter )
	{
		fprintf( stderr, "JavaSalGraphics::drawPixel2 not implemented\n" );
		return;
	}
#endif	// USE_NATIVE_PRINTING

	mpVCLGraphics->setPixel( nX, nY, nSalColor | 0xff000000, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter )
		return;

	if ( mpPrinter )
	{
		fprintf( stderr, "JavaSalGraphics::drawLine not implemented\n" );
		return;
	}
#endif	// USE_NATIVE_PRINTING

	if ( mnLineColor )
		mpVCLGraphics->drawLine( nX1, nY1, nX2, nY2, mnLineColor, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter )
		return;

	if ( mpPrinter )
	{
		if ( mnFillColor )
			addToUndrawnNativeOps( new JavaSalGraphicsDrawRectOp( maNativeClipPath, false, CGRectMake( nX, nY, nWidth, nHeight ), mnFillColor, true ) );
		if ( mnLineColor )
			addToUndrawnNativeOps( new JavaSalGraphicsDrawRectOp( maNativeClipPath, false, CGRectMake( nX, nY, nWidth, nHeight ), mnLineColor, false ) );
		return;
	}
#endif	// USE_NATIVE_PRINTING

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
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter )
		return;

	if ( mpPrinter )
	{
		fprintf( stderr, "JavaSalGraphics::drawPolyLine not implemented\n" );
		return;
	}
#endif	// USE_NATIVE_PRINTING

	if ( mnLineColor )
		mpVCLGraphics->drawPolyline( nPoints, pPtAry, mnLineColor, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter )
		return;

	if ( mpPrinter )
	{
		fprintf( stderr, "JavaSalGraphics::drawPolygon not implemented\n" );
		return;
	}
#endif	// USE_NATIVE_PRINTING

	if ( mnFillColor )
		mpVCLGraphics->drawPolygon( nPoints, pPtAry, mnFillColor, TRUE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
	if ( mnLineColor )
		mpVCLGraphics->drawPolygon( nPoints, pPtAry, mnLineColor, FALSE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolyPolygon( ULONG nPoly, const ULONG* pPoints, PCONSTSALPOINT* pPtAry )
{
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter )
		return;

	if ( mpPrinter )
	{
		fprintf( stderr, "JavaSalGraphics::drawPolyPolygon not implemented\n" );
		return;
	}
#endif	// USE_NATIVE_PRINTING

	if ( mnFillColor )
		mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pPtAry, mnFillColor, TRUE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
	if ( mnLineColor )
		mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pPtAry, mnLineColor, FALSE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly, double fTransparency )
{
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter )
		return true;

	if ( mpPrinter )
	{
		fprintf( stderr, "JavaSalGraphics::drawPolyPolygon2 not implemented\n" );
		return true;
	}
#endif	// USE_NATIVE_PRINTING

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

	return true;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawPolyLine( const ::basegfx::B2DPolygon& rPoly, const ::basegfx::B2DVector& rLineWidths, ::basegfx::B2DLineJoin eLineJoin )
{
#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter || mpPrinter )
#else	// USE_NATIVE_PRINTING
	if ( mpPrinter )
#endif	// USE_NATIVE_PRINTING
		return false;

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

	return true;
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

#ifdef USE_NATIVE_PRINTING
	if ( mpInfoPrinter )
		return bRet;

	if ( mpPrinter )
	{
		fprintf( stderr, "JavaSalGraphics::drawEPS not implemented\n" );
		return bRet;
	}
#endif	// USE_NATIVE_PRINTING

	if ( pPtr && nSize )
	{
		if ( mpPrinter )
		{
			void *pPtrCopy = rtl_allocateMemory( nSize );
			if ( pPtrCopy )
			{
				// Don't delete the copied buffer and let the Java native
				// method print the buffer directly
				memcpy( pPtrCopy, pPtr, nSize );
				mpVCLGraphics->drawEPS( pPtrCopy, nSize, nX, nY, nWidth, nHeight, maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
				bRet = TRUE;
			}
		}

		if ( !bRet )
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

#ifdef USE_NATIVE_PRINTING

// -----------------------------------------------------------------------

void JavaSalGraphics::addToUndrawnNativeOps( JavaSalGraphicsOp *pOp )
{
	if ( !pOp )
		return;

	MutexGuard aGuard( maUndrawnNativeOpsMutex );

	maUndrawnNativeOpsList.push_back( pOp );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawUndrawnNativeOps( CGContextRef aContext )
{
	if ( !aContext )
		return;

	ClearableMutexGuard aGuard( maUndrawnNativeOpsMutex );
	::std::list< JavaSalGraphicsOp* > aOpsList( maUndrawnNativeOpsList );
	maUndrawnNativeOpsList.clear();
	aGuard.clear();

	CGContextSaveGState( aContext );

	// Scale context to match OOo resolution
	long nDPIX;
	long nDPIY;
	GetResolution( nDPIX, nDPIY );
	CGContextScaleCTM( aContext, (float)72 / (float)nDPIX, (float)72 / (float)nDPIY );

	while ( aOpsList.size() )
	{
		JavaSalGraphicsOp *pOp = aOpsList.front();
		pOp->drawOp( aContext );
		delete pOp;
		aOpsList.pop_front();
	}

	CGContextRestoreGState( aContext );
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

#endif	// USE_NATIVE_PRINTING
