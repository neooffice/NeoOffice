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

#include <salprn.h>
#include <salbmp.h>
#include <salvd.h>
#include <vcl/salgdi.hxx>
#include <vcl/outfont.hxx>
#include <vcl/sallayout.hxx>
#include <vcl/sv.h>

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>
#undef check

#ifdef USE_NATIVE_PRINTING
// Fix bug 3051 by setting the printer resolution to twips
#define MIN_PRINTER_RESOLUTION 1440
#endif	// USE_NATIVE_PRINTING

#ifdef USE_NATIVE_VIRTUAL_DEVICE
#define MIN_SCREEN_RESOLUTION 96
#endif	// USE_NATIVE_VIRTUAL_DEVICE

class ImplDevFontAttributes;
class ImplFontSelectData;
class JavaSalBitmap;
class JavaSalFrame;
class JavaSalPrinter;
class JavaSalVirtualDevice;
class SalATSLayout;

namespace vcl
{
class com_sun_star_vcl_VCLFont;
class com_sun_star_vcl_VCLGraphics;
class com_sun_star_vcl_VCLPath;
}

// --------------------
// - JavaImplFontData -
// --------------------

class JavaImplFontData : public ImplFontData
{
protected:
	static ::std::map< JavaImplFontData*, JavaImplFontData* >	maInstancesMap;

public:
	static ::std::map< sal_IntPtr, sal_IntPtr >	maBadNativeFontIDMap;
	::rtl::OUString			maVCLFontName;
	mutable sal_IntPtr		mnNativeFontID;
	::std::list< JavaImplFontData* >	maChildren;
	::rtl::OUString			maFamilyName;

	static void				ClearNativeFonts();
	static void				HandleBadFont( JavaImplFontData *pFontData );
	DECL_STATIC_LINK( JavaImplFontData, RunNativeFontsTimer, void* );

							JavaImplFontData( const ImplDevFontAttributes& rAttibutes, const ::rtl::OUString& rVCLFontName, sal_IntPtr nNativeFontID, const ::rtl::OUString& rFamilyName );
	virtual					~JavaImplFontData();

	virtual ImplFontEntry*	CreateFontInstance( ImplFontSelectData& rData ) const;
	virtual ImplFontData*   Clone() const;
	virtual sal_IntPtr		GetFontId() const;
};

// ----------------------
// - JavaSalGraphicsOp -
// ----------------------

class SAL_DLLPRIVATE JavaSalGraphicsOp
{
protected:
	CGPathRef				maNativeClipPath;
	bool					mbInvert;
	CGLayerRef				maXORLayer;
	CGContextRef			maSavedContext;
	size_t					mnBitmapCapacity;
	BYTE*					mpDrawBits;
	CGContextRef			maDrawBitmapContext;
	BYTE*					mpXORBits;
	CGContextRef			maXORBitmapContext;
	CGRect					maXORRect;
	
public:
							JavaSalGraphicsOp( const CGPathRef aNativeClipPath, bool bInvert = false, CGLayerRef aXORLayer = NULL );
	virtual					~JavaSalGraphicsOp();

	virtual	void			drawOp( CGContextRef aContext, CGRect aBounds ) {}

protected:
	void					restoreClipXORGState();
	CGContextRef			saveClipXORGState( CGContextRef aContext, CGRect aDrawBounds = CGRectNull );
};

// ------------------------------
// - JavaSalGraphicsDrawImageOp -
// ------------------------------

class SAL_DLLPRIVATE JavaSalGraphicsDrawImageOp : public JavaSalGraphicsOp
{
	CGImageRef				maImage;
	CGRect					maRect;

public:
							JavaSalGraphicsDrawImageOp( const CGPathRef aNativeClipPath, bool bInvert, CGLayerRef maXORLayer, CGDataProviderRef aProvider, int nDataBitCount, size_t nDataScanlineSize, size_t nDataWidth, size_t nDataHeight, const CGRect aSrcRect, const CGRect aRect );
	virtual					~JavaSalGraphicsDrawImageOp();

	virtual	void			drawOp( CGContextRef aContext, CGRect aBounds );
};

// -----------------------------
// - JavaSalGraphicsDrawPathOp -
// -----------------------------

class SAL_DLLPRIVATE JavaSalGraphicsDrawPathOp : public JavaSalGraphicsOp
{
	bool					mbAntialias;
	SalColor				mnFillColor;
	SalColor				mnLineColor;
	ULONG					mnPoints;
	CGPathRef				maPath;
	float					mfLineWidth;
	::basegfx::B2DLineJoin	meLineJoin;
	bool					mbLineDash;

public:
							JavaSalGraphicsDrawPathOp( const CGPathRef aNativeClipPath, bool bInvert, CGLayerRef aXORLayer, bool bAntialias, SalColor nFillColor, SalColor nLineColor, const CGPathRef aPath, float fLineWidth = 0, ::basegfx::B2DLineJoin eLineJoin = ::basegfx::B2DLINEJOIN_NONE, bool bLineDash = false );
	virtual					~JavaSalGraphicsDrawPathOp();

	virtual	void			drawOp( CGContextRef aContext, CGRect aBounds );
};

// -------------------
// - JavaSalGraphics -
// -------------------

class JavaSalGraphics : public SalGraphics
{
private:
	::osl::Mutex			maUndrawnNativeOpsMutex;
	::std::list< JavaSalGraphicsOp* >	maUndrawnNativeOpsList;
	::std::list< JavaSalBitmap* >	maGraphicsChangeListenerList;
	CGLayerRef				maLayer;

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
	bool					mbInvert;
	bool					mbXOR;
	Orientation				meOrientation;
	sal_Bool				mbPaperRotated;

							JavaSalGraphics();
	virtual					~JavaSalGraphics();

	virtual BOOL			unionClipRegion( long nX, long nY, long nWidth, long nHeight );
	virtual bool			unionClipRegion( const ::basegfx::B2DPolyPolygon& rPolyPoly );
	virtual void			drawPixel( long nX, long nY );
	virtual void			drawPixel( long nX, long nY, SalColor nSalColor );
	virtual void			drawLine( long nX1, long nY1, long nX2, long nY2 );
	virtual void			drawRect( long nX, long nY, long nWidth, long nHeight );
	virtual void			drawPolyLine( ULONG nPoints, const SalPoint* pPtAry );
	virtual void			drawPolygon( ULONG nPoints, const SalPoint* pPtAry );
	virtual void			drawPolyPolygon( ULONG nPoly, const ULONG* pPoints, PCONSTSALPOINT* pPtAry );
	virtual bool			drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly, double fTransparency );
	virtual bool			drawPolyLine( const ::basegfx::B2DPolygon& rPoly, const ::basegfx::B2DVector& rLineWidths, basegfx::B2DLineJoin eLineJoin );
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
	virtual USHORT			GetBitCount();
	virtual long			GetGraphicsWidth() const;
	virtual void			ResetClipRegion();
	virtual void			BeginSetClipRegion( ULONG nCount );
	virtual void			EndSetClipRegion();
	virtual void			SetLineColor();
	virtual void			SetLineColor( SalColor nSalColor );
	virtual void			SetFillColor();
	virtual void			SetFillColor( SalColor nSalColor );
	virtual void			SetXORMode( bool bSet, bool bInvertOnly );
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
	virtual BOOL			CreateFontSubset( const rtl::OUString& rToFile, const ImplFontData* pFont, sal_Int32* pGlyphIDs, sal_uInt8* pEncoding, sal_Int32* pWidths, int nGlyphs, FontSubsetInfo& rInfo );
	virtual const Ucs2SIntMap*	GetFontEncodingVector( const ImplFontData*, const Ucs2OStrMap** ppNonEncoded );
	virtual const void*		GetEmbedFontData( const ImplFontData* pFont, const sal_Ucs* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen );
	virtual void			FreeEmbedFontData( const void* pData, long nDataLen );
	virtual void			GetGlyphWidths( const ImplFontData* pFont, bool bVertical, Int32Vector& rWidths, Ucs2UIntMap& rUnicodeEnc );
	virtual BOOL			GetGlyphBoundRect( long nIndex, Rectangle& );
	virtual BOOL			GetGlyphOutline( long nIndex, basegfx::B2DPolyPolygon& rPolyPoly );
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
	virtual bool			supportsOperation( OutDevSupportType ) const;
	virtual SystemGraphicsData	GetGraphicsData() const;

	void					setLineTransparency( sal_uInt8 nTransparency );
	void					setFillTransparency( sal_uInt8 nTransparency );
	bool					useNativeDrawing();
	void					addGraphicsChangeListener( JavaSalBitmap *pBitmap );
	void					addUndrawnNativeOp( JavaSalGraphicsOp *pOp );
	void					copyFromGraphics( JavaSalGraphics *pSrcGraphics, CGPoint aSrcPoint, CGRect aDestRect, bool bAllowXOR );
	void					copyToContext( const CGPathRef aNativeClipPath, bool bInvert, CGLayerRef aXORLayer, CGContextRef aDestContext, CGRect aDestBounds, CGPoint aSrcPoint, CGRect aDestRect );
	void					drawUndrawnNativeOps( CGContextRef aContext, CGRect aRect );
	ULONG					getBitmapDirectionFormat();
	float					getNativeLineWidth();
	void					removeGraphicsChangeListener( JavaSalBitmap *pBitmap );
	void					setLayer( CGLayerRef aLayer );
};

SAL_DLLPRIVATE CGColorRef CreateCGColorFromSalColor( SalColor nColor );
SAL_DLLPRIVATE void AddPolygonToPaths( ::vcl::com_sun_star_vcl_VCLPath *pVCLPath, CGMutablePathRef aCGPath, const ::basegfx::B2DPolygon& rPolygon, bool bClosePath );
SAL_DLLPRIVATE void AddPolyPolygonToPaths( ::vcl::com_sun_star_vcl_VCLPath *pVCLPath, CGMutablePathRef aCGPath, const ::basegfx::B2DPolyPolygon& rPolyPoly );

#endif // _SV_SALGDI_H
