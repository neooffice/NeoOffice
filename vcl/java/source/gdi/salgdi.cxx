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
	mpFrame = NULL;
	mpPrinter = NULL;
	mpVirDev = NULL;
	mpVCLGraphics = NULL;
	mpVCLFont = NULL;

	GetSalData()->maGraphicsList.push_back( this );
}

// -----------------------------------------------------------------------

JavaSalGraphics::~JavaSalGraphics()
{
	GetSalData()->maGraphicsList.remove( this );

	if ( mpVCLFont )
		delete mpVCLFont;

	for ( ::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator it = maFallbackFonts.begin(); it != maFallbackFonts.end(); ++it )
		delete it->second;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
	Size aSize( mpVCLGraphics->getResolution() );
	rDPIX = aSize.Width();
	rDPIY = aSize.Height();
}

// -----------------------------------------------------------------------

void JavaSalGraphics::GetScreenFontResolution( long& rDPIX, long& rDPIY )
{
	Size aSize( mpVCLGraphics->getScreenFontResolution() );
	rDPIX = aSize.Width();
	rDPIY = aSize.Height();
}

// -----------------------------------------------------------------------

USHORT JavaSalGraphics::GetBitCount()
{
	return mpVCLGraphics->getBitCount();
}

// -----------------------------------------------------------------------

void JavaSalGraphics::ResetClipRegion()
{
	mpVCLGraphics->resetClipRegion();
}

// -----------------------------------------------------------------------

void JavaSalGraphics::BeginSetClipRegion( ULONG nRectCount )
{
	mpVCLGraphics->beginSetClipRegion();
}

// -----------------------------------------------------------------------

BOOL JavaSalGraphics::unionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
	mpVCLGraphics->unionClipRegion( nX, nY, nWidth, nHeight );
	return TRUE;
}

// -----------------------------------------------------------------------

BOOL JavaSalGraphics::unionClipRegion( ULONG nPoly, const ULONG* pPoints, PCONSTSALPOINT* pPtAry )
{
	return mpVCLGraphics->unionClipRegion( nPoly, pPoints, pPtAry );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::EndSetClipRegion()
{
	mpVCLGraphics->endSetClipRegion();
}

// -----------------------------------------------------------------------

void JavaSalGraphics::SetLineColor()
{
	mnLineColor = 0x00000000;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::SetLineColor( SalColor nSalColor )
{
	mnLineColor = nSalColor | 0xff000000;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::SetFillColor()
{
	mnFillColor = 0x00000000;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::SetFillColor( SalColor nSalColor )
{
	mnFillColor = nSalColor | 0xff000000;
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
		mpVCLGraphics->setPixel( nX, nY, mnLineColor );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
	mpVCLGraphics->setPixel( nX, nY, nSalColor | 0xff000000 );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
	if ( mnLineColor )
		mpVCLGraphics->drawLine( nX1, nY1, nX2, nY2, mnLineColor );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
	if ( mnFillColor )
		mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, mnFillColor, TRUE );
	if ( mnLineColor )
		mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, mnLineColor, FALSE );
}

// -----------------------------------------------------------------------

bool JavaSalGraphics::drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency )
{
	SalColor nTransColor = ( ( 0xff - nTransparency ) << 24 );
	if ( nTransColor )
	{
		if ( mnFillColor )
			mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, ( mnFillColor & 0x00ffffff ) | nTransColor, TRUE );
		if ( mnLineColor )
			mpVCLGraphics->drawRect( nX, nY, nWidth, nHeight, ( mnLineColor & 0x00ffffff ) | nTransColor, FALSE );
	}

	return true;
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolyLine( ULONG nPoints, const SalPoint* pPtAry )
{
	if ( mnLineColor )
		mpVCLGraphics->drawPolyline( nPoints, pPtAry, mnLineColor );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
	if ( mnFillColor )
		mpVCLGraphics->drawPolygon( nPoints, pPtAry, mnFillColor, TRUE );
	if ( mnLineColor )
		mpVCLGraphics->drawPolygon( nPoints, pPtAry, mnLineColor, FALSE );
}

// -----------------------------------------------------------------------

void JavaSalGraphics::drawPolyPolygon( ULONG nPoly, const ULONG* pPoints, PCONSTSALPOINT* pPtAry )
{
	if ( mnFillColor )
		mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pPtAry, mnFillColor, TRUE );
	if ( mnLineColor )
		mpVCLGraphics->drawPolyPolygon( nPoly, pPoints, pPtAry, mnLineColor, FALSE);
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
				mpVCLGraphics->drawEPS( pPtrCopy, nSize, nX, nY, nWidth, nHeight );
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
								mpVCLGraphics->drawBitmap( &aVCLBitmap, 0, 0, nWidth, nHeight, nX, nY, nWidth, nHeight );
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
