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
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
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
 *  =================================================
 *  Modified September 2003 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

#ifndef _SV_SALGDI_HXX
#define _SV_SALGDI_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_SALGTYPE_HXX
#include <salgtype.hxx>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif

class ImplDevFontList;
class SalBitmap;
struct ImplFontSelectData;
struct ImplFontMetricData;
struct ImplKernPairData;
class FontCharMap;

// ---------------------
// - SalGraphics-Codes -
// ---------------------

#define SAL_SETFONT_REMOVEANDMATCHNEW		((USHORT)0x0001)
#define SAL_SETFONT_USEDRAWTEXT 			((USHORT)0x0002)
#define SAL_SETFONT_USEDRAWTEXTARRAY		((USHORT)0x0004)
#define SAL_SETFONT_UNICODE 				((USHORT)0x0008)

#define SAL_COPYAREA_WINDOWINVALIDATE		((USHORT)0x0001)

// ---------------
// - SalGraphics -
// ---------------

class SalGraphics
{
	friend class SalFrame;
	friend class SalVirtualDevice;
	friend class SalPrinter;

public: 					// public for Sal Implementation
							SalGraphics();
							~SalGraphics();

public: 					// public for Sal Implementation
	SalGraphicsData 		maGraphicsData;

#ifdef _INCL_SAL_SALGDI_IMP
#include <salgdi.imp>
#endif

public:
	// this functions must be quick, because this data is query for all
	// GetGraphics()-Instances
	void					GetResolution( long& rDPIX, long& rDPIY );
	void					GetScreenFontResolution( long& rDPIX, long& rDPIY );
	USHORT					GetBitCount();

	void					ResetClipRegion();
	void					BeginSetClipRegion( ULONG nCount );
	BOOL					UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
	void					EndSetClipRegion();

	void					SetLineColor();
	void					SetLineColor( SalColor nSalColor );
	void					SetFillColor();
	void					SetFillColor( SalColor nSalColor );

	void					SetXORMode( BOOL bSet );

	void					SetROPLineColor( SalROPColor nROPColor );
	void					SetROPFillColor( SalROPColor nROPColor );

    // all positions are in pixel and relative to
    // the top/left-position of the output area
    void                    SetTextColor( SalColor nSalColor );
    USHORT                  SetFont( ImplFontSelectData* pFont );
    long                    GetCharWidth( sal_Unicode nChar1, sal_Unicode nChar2, long* pWidthAry );
    void                    GetFontMetric( ImplFontMetricData* pMetric );
                            // pKernPairs == NULL, than only return PairCount
    ULONG                   GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs );
    ULONG                   GetFontCodeRanges( sal_uInt32* pCodePairs ) const;
    void                    GetDevFontList( ImplDevFontList* pList );

	BOOL					GetGlyphBoundRect( xub_Unicode cChar, long* pX, long* pY,
											   long* pWidth, long* pHeight );
	ULONG					GetGlyphOutline( xub_Unicode cChar, USHORT** pPolySizes,
											 SalPoint** ppPoints, BYTE** ppFlags );

	void					DrawText( long nX, long nY,
									  const xub_Unicode* pStr, xub_StrLen nLen );
	void					DrawTextArray( long nX, long nY,
										   const xub_Unicode* pStr, xub_StrLen nLen,
										   const long* pDXAry );

	// draw --> LineColor and FillColor and RasterOp and ClipRegion
	void					DrawPixel( long nX, long nY );
	void					DrawPixel( long nX, long nY, SalColor nSalColor );
	void					DrawLine( long nX1, long nY1, long nX2, long nY2 );
	void					DrawRect( long nX, long nY, long nWidth, long nHeight );
	void					DrawPolyLine( ULONG nPoints, const SalPoint* pPtAry );
	void					DrawPolygon( ULONG nPoints, const SalPoint* pPtAry );
	void					DrawPolyPolygon( ULONG nPoly, const ULONG* pPoints,
											 PCONSTSALPOINT* pPtAry );

	// CopyArea --> No RasterOp, but ClipRegion
	void					CopyArea( long nDestX, long nDestY,
									  long nSrcX, long nSrcY,
									  long nSrcWidth, long nSrcHeight,
									  USHORT nFlags );

	// CopyBits and DrawBitmap --> RasterOp and ClipRegion
	// CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
	void					CopyBits( const SalTwoRect* pPosAry,
									  SalGraphics* pSrcGraphics );
	void					DrawBitmap( const SalTwoRect* pPosAry,
										const SalBitmap& rSalBitmap );
	void					DrawBitmap( const SalTwoRect* pPosAry,
										const SalBitmap& rSalBitmap,
										SalColor nTransparentColor );
	void					DrawBitmap( const SalTwoRect* pPosAry,
										const SalBitmap& rSalBitmap,
										const SalBitmap& rTransparentBitmap );

	void					DrawMask( const SalTwoRect* pPosAry,
									  const SalBitmap& rSalBitmap,
									  SalColor nMaskColor );

	SalBitmap*				GetBitmap( long nX, long nY, long nWidth, long nHeight );
	SalColor				GetPixel( long nX, long nY );

	// invert --> ClipRegion (only Windows)
	void					Invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags );
	void					Invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nFlags );

	BOOL					DrawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize );
#ifdef USE_JAVA
	void					SetAntialias( BOOL bAntialias );
#endif
};

#endif // _SV_SALGDI_HXX
