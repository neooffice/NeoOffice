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

#define _SV_SALGDI_CXX

#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLBITMAP_HXX
#include <com/sun/star/vcl/VCLBitmap.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif

#include "salgdi_cocoa.h"

using namespace vcl;

// =======================================================================

JavaSalGraphics::JavaSalGraphics()
{
	mnFillColor = MAKE_SALCOLOR( 0xff, 0xff, 0xff ) | 0xff000000;
	mnLineColor = MAKE_SALCOLOR( 0, 0, 0 ) | 0xff000000;
	mnTextColor = MAKE_SALCOLOR( 0, 0, 0 ) | 0xff000000;
	mnFillTransparency = 0xff000000;
	mnLineTransparency = 0xff000000;
	mpFrame = NULL;
	mpPrinter = NULL;
	mpVirDev = NULL;
	mpFontData = NULL;
	mpVCLGraphics = NULL;
	mpVCLFont = NULL;
	mnFontFamily = FAMILY_DONTKNOW;
	mnFontWeight = WEIGHT_DONTKNOW;
	mnFontPitch = PITCH_DONTKNOW;
	mnDPIX = 0;
	mnDPIY = 0;
	maNativeClipPath = NULL;

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

void JavaSalGraphics::GetScreenFontResolution( long& rDPIX, long& rDPIY )
{
	GetResolution( rDPIX, rDPIY );
}

// -----------------------------------------------------------------------

USHORT JavaSalGraphics::GetBitCount()
{
	return mpVCLGraphics->getBitCount();
}

// -----------------------------------------------------------------------

void JavaSalGraphics::ResetClipRegion()
{
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
	if ( mpPrinter )
		ResetClipRegion();
	else
		mpVCLGraphics->beginSetClipRegion( sal_False );
}

// -----------------------------------------------------------------------

BOOL JavaSalGraphics::unionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
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
	return false;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::EndSetClipRegion()
{
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

void JavaSalGraphics::SetXORMode( BOOL bSet )
{
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
	if ( mnLineColor )
		mpVCLGraphics->setPixel( nX, nY, mnLineColor, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
	mpVCLGraphics->setPixel( nX, nY, nSalColor | 0xff000000, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
	if ( mnLineColor )
		mpVCLGraphics->drawLine( nX1, nY1, nX2, nY2, mnLineColor, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
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
	if ( mnLineColor )
		mpVCLGraphics->drawPolyline( nPoints, pPtAry, mnLineColor, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
	if ( mnFillColor )
		mpVCLGraphics->drawPolygon( nPoints, pPtAry, mnFillColor, TRUE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
	if ( mnLineColor )
		mpVCLGraphics->drawPolygon( nPoints, pPtAry, mnLineColor, FALSE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolyPolygon( ULONG nPoly, const ULONG* pPoints, PCONSTSALPOINT* pPtAry )
{
	if ( mnFillColor )
		mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pPtAry, mnFillColor, TRUE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
	if ( mnLineColor )
		mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pPtAry, mnLineColor, FALSE, mpPrinter && maNativeClipPath ? CGPathCreateCopy( maNativeClipPath ) : NULL );
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly, double fTransparency )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::drawPolyPolygon not implemented\n" );
#endif
	return false;
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawPolyLine( const ::basegfx::B2DPolygon& rPolyPoly, const ::basegfx::B2DVector& rLineWidths )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalGraphics::drawPolyLine not implemented\n" );
#endif
	return false;
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

// -----------------------------------------------------------------------

BOOL JavaSalGraphics::unionClipRegion( ULONG nPoly, const ULONG* pPoints, PCONSTSALPOINT* pPtAry, sal_Int32 nOffsetX, sal_Int32 nOffsetY )
{
	BOOL bRet = FALSE;

	if ( mpPrinter )
	{
		for ( ULONG i = 0; i < nPoly; i++ )
		{
			// CGMutablePathRef aPolyPath = CGPathCreateMutable();
			CGMutablePathRef aPolyPath = NULL;
			if ( !aPolyPath )
				continue;

			ULONG nCount = pPoints[ i ];
			if ( nCount )
			{
				CGPathMoveToPoint( aPolyPath, NULL, (float)( pPtAry[ i ][ 0 ].mnX + nOffsetX ), (float)( pPtAry[ i ][ 0 ].mnY + nOffsetY ) );
				for ( ULONG j = 1; j < nCount; j++ )
					CGPathAddLineToPoint( aPolyPath, NULL, (float)( pPtAry[ i ][ j ].mnX + nOffsetX ), (float)( pPtAry[ i ][ j ].mnY + nOffsetY ) );
				CGPathCloseSubpath( aPolyPath );
			}
				
			CGRect aBounds = CGPathGetBoundingBox( aPolyPath );
			if ( aBounds.size.width > 0 && aBounds.size.height > 0 )
			{
				if ( !maNativeClipPath )
					maNativeClipPath = CGPathCreateMutable();

				if ( maNativeClipPath )
				{
					CGPathAddPath( maNativeClipPath, NULL, aPolyPath );
					bRet = TRUE;
				}
			}

			CFRelease( aPolyPath );
		}
	}
	else
	{
		bRet = mpVCLGraphics->unionClipRegion( nPoly, pPoints, pPtAry, sal_False, nOffsetX, nOffsetY );
	}

	return bRet;
}
