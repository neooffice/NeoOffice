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
 *		 - GNU General Public License Version 2.1
 *
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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

#ifndef _SV_SALGDI_H
#define _SV_SALGDI_H

#include <map>

#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif
#ifndef _SV_SV_H
#include <sv.h>
#endif 

class SalATSLayout;
class JavaSalFrame;
class SalPrinter;
class SalVirtualDevice;

namespace vcl
{
class com_sun_star_vcl_VCLFont;
class com_sun_star_vcl_VCLGraphics;
}

// -------------------
// - JavaSalGraphics -
// -------------------

class JavaSalGraphics : public SalGraphics
{
public:
	SalColor				mnFillColor;
	SalColor				mnLineColor;
	SalColor				mnTextColor;
	SalFrame*				mpFrame;
	SalPrinter*				mpPrinter;
	SalVirtualDevice*		mpVirDev;
	::vcl::com_sun_star_vcl_VCLGraphics*	mpVCLGraphics;
	::vcl::com_sun_star_vcl_VCLFont*	mpVCLFont;
	::std::map< int, ::vcl::com_sun_star_vcl_VCLFont* >	maFallbackFonts;
	ImplLayoutRuns			maFallbackRuns;

							JavaSalGraphics();
	virtual					~JavaSalGraphics();

	virtual BOOL			unionClipRegion( long nX, long nY, long nWidth, long nHeight );
	virtual void			drawPixel( long nX, long nY );
	virtual void			drawPixel( long nX, long nY, SalColor nSalColor );
	virtual void			drawLine( long nX1, long nY1, long nX2, long nY2 );
	virtual void			drawRect( long nX, long nY, long nWidth, long nHeight );
	virtual void			drawPolyLine( ULONG nPoints, const SalPoint* pPtAry );
	virtual void			drawPolygon( ULONG nPoints, const SalPoint* pPtAry );
	virtual void			drawPolyPolygon( ULONG nPoly, const ULONG* pPoints, PCONSTSALPOINT* pPtAry );
	virtual sal_Bool		drawPolyLineBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry );
	virtual sal_Bool		drawPolygonBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry );
	virtual sal_Bool		drawPolyPolygonBezier( ULONG nPoly, const ULONG* pPoints, const SalPoint* const* pPtAry, const BYTE* const* pFlgAry );
	virtual void			copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth, long nSrcHeight, USHORT nFlags );
	virtual void			copyBits( const SalTwoRect* pPosAry, SalGraphics* pSrcGraphics );
	virtual void			drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap );
	virtual void			drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap, SalColor nTransparentColor );
	virtual void			drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap, const SalBitmap& rTransparentBitmap );
	virtual void			drawMask( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap, SalColor nMaskColor );
	virtual SalBitmap*		getBitmap( long nX, long nY, long nWidth, long nHeight );
	virtual SalColor		getPixel( long nX, long nY );
	virtual void			invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags);
	virtual void			invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nFlags );
	virtual BOOL			drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize );
	virtual void			GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
	virtual void			GetScreenFontResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
	virtual USHORT			GetBitCount();
	virtual long			GetGraphicsWidth();
	virtual void			ResetClipRegion();
	virtual void			BeginSetClipRegion( ULONG nCount );
	virtual void			EndSetClipRegion();
	virtual void			SetLineColor();
	virtual void			SetLineColor( SalColor nSalColor );
	virtual void			SetFillColor();
	virtual void			SetFillColor( SalColor nSalColor );
	virtual void			SetXORMode( BOOL bSet );
	virtual void			SetROPLineColor( SalROPColor nROPColor );
	virtual void			SetROPFillColor( SalROPColor nROPColor );
	virtual void			SetTextColor( SalColor nSalColor );
	virtual USHORT			SetFont( ImplFontSelectData*, int nFallbackLevel );
	virtual void			GetFontMetric( ImplFontMetricData* );
	virtual ULONG			GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs );
	virtual ImplFontCharMap*	GetImplFontCharMap() const;
	virtual void			GetDevFontList( ImplDevFontList* );
	virtual void			GetDevFontSubstList( OutputDevice* );
	virtual bool			AddTempDevFont( ImplDevFontList*, const String& rFileURL, const String& rFontName );
	virtual BOOL			CreateFontSubset( const rtl::OUString& rToFile, ImplFontData* pFont, sal_Int32* pGlyphIDs, sal_uInt8* pEncoding, sal_Int32* pWidths, int nGlyphs, FontSubsetInfo& rInfo );
	virtual const std::map< sal_Unicode, sal_Int32 >*	GetFontEncodingVector( ImplFontData* pFont, const std::map< sal_Unicode, rtl::OString >** ppNonEncoded );
	virtual const void*		GetEmbedFontData( ImplFontData* pFont, const sal_Unicode* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen );
	virtual void			FreeEmbedFontData( const void* pData, long nDataLen );
	virtual BOOL			GetGlyphBoundRect( long nIndex, Rectangle& );
	virtual BOOL			GetGlyphOutline( long nIndex, basegfx::B2DPolyPolygon& );
	virtual SalLayout*		GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
	virtual void			DrawServerFontLayout( const ServerFontLayout& );
	virtual bool			filterText( const String& rOrigText, String& rNewText, xub_StrLen nIndex, xub_StrLen& rLen, xub_StrLen& rCutStart, xub_StrLen& rCutStop );
};

#endif // _SV_SALGDI_H
