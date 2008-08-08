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

#ifndef _SV_SALGDI_H
#define _SV_SALGDI_H

#include <hash_map>

#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif
#ifndef _SV_SV_H
#include <sv.h>
#endif 

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>
#undef check

class ImplDevFontAttributes;
class ImplFontSelectData;
class JavaSalFrame;
class JavaSalPrinter;
class JavaSalVirtualDevice;
class SalATSLayout;

namespace vcl
{
class com_sun_star_vcl_VCLFont;
class com_sun_star_vcl_VCLGraphics;
}

// --------------------
// - JavaImplFontData -
// --------------------

class JavaImplFontData : public ImplFontData
{
public:
	::rtl::OUString			maVCLFontName;
	mutable sal_IntPtr		mnATSUFontID;
	::std::list< JavaImplFontData* >	maChildren;

							JavaImplFontData( const ImplDevFontAttributes& rAttibutes, ::rtl::OUString aVCLFontName, sal_IntPtr nATSUFontID );
	virtual					~JavaImplFontData();

	virtual ImplFontEntry*	CreateFontInstance( ImplFontSelectData& rData ) const;
	virtual ImplFontData*   Clone() const;
	virtual sal_IntPtr		GetFontId() const;
};

// -------------------
// - JavaSalGraphics -
// -------------------

class JavaSalGraphics : public SalGraphics
{
public:
	SalColor				mnFillColor;
	SalColor				mnLineColor;
	SalColor				mnTextColor;
	SalColor				mnFillTransparency;
	SalColor				mnLineTransparency;
	JavaSalFrame*			mpFrame;
	JavaSalPrinter*			mpPrinter;
	JavaSalVirtualDevice*	mpVirDev;
	::vcl::com_sun_star_vcl_VCLGraphics*	mpVCLGraphics;
	JavaImplFontData*		mpFontData;
	::vcl::com_sun_star_vcl_VCLFont*	mpVCLFont;
	::std::hash_map< int, ::vcl::com_sun_star_vcl_VCLFont* >	maFallbackFonts;
	ImplLayoutRuns			maFallbackRuns;
	FontFamily				mnFontFamily;
	FontWeight				mnFontWeight;
	bool					mbFontItalic;
	FontPitch				mnFontPitch;
	sal_Int32				mnDPIX;
	sal_Int32				mnDPIY;
	CGMutablePathRef		maNativeClipPath;

							JavaSalGraphics();
	virtual					~JavaSalGraphics();

	virtual BOOL			unionClipRegion( long nX, long nY, long nWidth, long nHeight );
	virtual BOOL			unionClipRegion( ULONG nPoly, const ULONG* pPoints, PCONSTSALPOINT* pPtAry, sal_Int32 nOffsetX, sal_Int32 nOffsetY );
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
	virtual long			GetGraphicsWidth() const;
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
	virtual BOOL			IsNativeControlSupported( ControlType nType, ControlPart nPart );
	virtual BOOL			hitTestNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion, const Point& aPos, SalControlHandle& rControlHandle, BOOL& rIsInside );
	virtual BOOL			drawNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle, const rtl::OUString& rCaption );
	virtual BOOL			drawNativeControlText( ControlType nType, ControlPart nPart, const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle, const rtl::OUString& rCaption );
	virtual BOOL			getNativeControlRegion( ControlType nType, ControlPart nPart, const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle, const rtl::OUString& rCaption, Region &rNativeBoundingRegion, Region &rNativeContentRegion );
    virtual BOOL			getNativeControlTextColor( ControlType nType, ControlPart nPart, ControlState nState, const ImplControlValue& aValue, SalColor& textColor );
	virtual bool			drawAlphaBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSourceBitmap, const SalBitmap& rAlphaBitmap );
	virtual bool			drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency );

	void					setLineTransparency( sal_uInt8 nTransparency );
	void					setFillTransparency( sal_uInt8 nTransparency );
};

#endif // _SV_SALGDI_H
