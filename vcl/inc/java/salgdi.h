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

#include <boost/unordered_map.hpp>

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>
#undef check

#include "PhysicalFontFace.hxx"
#include "impfont.hxx"
#include "salgdi.hxx"
#include "sallayout.hxx"
#include "java/salbmp.h"
#include "java/salframe.h"
#include "java/salprn.h"
#include "java/salvd.h"

// Fix bug 3051 by setting the printer resolution to twips
#define MIN_PRINTER_RESOLUTION 1440

#define MIN_SCREEN_RESOLUTION 96

class ImplDevFontAttributes;
class ImplFontSelectData;
class SalATSLayout;

// ------------------------
// - JavaPhysicalFontFace -
// ------------------------

class JavaPhysicalFontFace : public PhysicalFontFace
{
public:
	static ::std::map< sal_IntPtr, sal_IntPtr >	maBadNativeFontCheckedMap;
	static ::std::map< sal_IntPtr, sal_IntPtr >	maBadNativeFontIDMap;
	static ::std::map< OUString, OUString >	maBadNativeFontNameMap;
	OUString				maFontName;
	mutable sal_IntPtr		mnNativeFontID;
	::std::list< JavaPhysicalFontFace* >	maChildren;
	OUString				maFamilyName;

	static void				ClearNativeFonts();
	static void				HandleBadFont( const JavaPhysicalFontFace *pFontData );
	static bool				IsBadFont( const JavaPhysicalFontFace *pFontData, bool bHandleIfBadFont = true );
	DECL_STATIC_LINK( JavaPhysicalFontFace, RunNativeFontsTimer, void* );

							JavaPhysicalFontFace( const ImplDevFontAttributes& rAttibutes, const OUString& rFontName, sal_IntPtr nNativeFontID, const OUString& rFamilyName );
	virtual					~JavaPhysicalFontFace();

	virtual ImplFontEntry*	CreateFontInstance( FontSelectPattern& rData ) const SAL_OVERRIDE;
	virtual PhysicalFontFace*	Clone() const SAL_OVERRIDE;
	virtual sal_IntPtr		GetFontId() const SAL_OVERRIDE;
};

// ----------------------
// - JavaSalGraphicsOp -
// ----------------------

class JavaSalGraphicsOp
{
protected:
	CGPathRef				maFrameClipPath;
	CGPathRef				maNativeClipPath;
	bool					mbInvert;
	bool					mbXOR;
	float					mfLineWidth;
	size_t					mnXORBitmapPadding;
	CGLayerRef				maXORLayer;
	CGContextRef			maSavedContext;
	size_t					mnBitmapCapacity;
	sal_uInt8*				mpDrawBits;
	CGContextRef			maDrawBitmapContext;
	sal_uInt8*				mpXORBits;
	CGContextRef			maXORBitmapContext;
	CGRect					maXORRect;
	
public:
							JavaSalGraphicsOp( const CGPathRef aFrameClip, const CGPathRef aNativeClipPath, bool bInvert = false, bool bXOR = false, float fLineWidth = 0 );
	virtual					~JavaSalGraphicsOp();

	virtual	void			drawOp( JavaSalGraphics *, CGContextRef, CGRect ) {}

protected:
	void					restoreClipXORGState();
	CGContextRef			saveClipXORGState( JavaSalGraphics *pGraphics, CGContextRef aContext, CGRect aDrawBounds = CGRectNull );
};

// ------------------------------
// - JavaSalGraphicsDrawImageOp -
// ------------------------------

class SAL_DLLPRIVATE JavaSalGraphicsDrawImageOp : public JavaSalGraphicsOp
{
	CGImageRef				maImage;
	CGRect					maRect;

public:
							JavaSalGraphicsDrawImageOp( const CGPathRef aFrameClip, const CGPathRef aNativeClipPath, bool bInvert, bool bXOR, CGDataProviderRef aProvider, int nDataBitCount, size_t nDataScanlineSize, size_t nDataWidth, size_t nDataHeight, const CGRect aSrcRect, const CGRect aRect );
	virtual					~JavaSalGraphicsDrawImageOp();

	virtual	void			drawOp( JavaSalGraphics *pGraphics, CGContextRef aContext, CGRect aBounds );
};

// -----------------------------
// - JavaSalGraphicsDrawPathOp -
// -----------------------------

class SAL_DLLPRIVATE JavaSalGraphicsDrawPathOp : public JavaSalGraphicsOp
{
	bool					mbAntialias;
	SalColor				mnFillColor;
	SalColor				mnLineColor;
	CGPathRef				maPath;
	bool					mbShiftLines;
	::basegfx::B2DLineJoin	meLineJoin;
	bool					mbLineDash;

public:
							JavaSalGraphicsDrawPathOp( const CGPathRef aFrameClip, const CGPathRef aNativeClipPath, bool bInvert, bool bXOR, bool bAntialias, SalColor nFillColor, SalColor nLineColor, const CGPathRef aPath, bool bShiftLines = true, float fLineWidth = 0, ::basegfx::B2DLineJoin eLineJoin = ::basegfx::B2DLINEJOIN_NONE, bool bLineDash = false );
	virtual					~JavaSalGraphicsDrawPathOp();

	virtual	void			drawOp( JavaSalGraphics *pGraphics, CGContextRef aContext, CGRect aBounds );
};

// ----------------
// - JavaImplFont -
// ----------------

class JavaImplFont
{
	OUString				maPSName;
	sal_IntPtr				mnNativeFont;
	short					mnOrientation;
	double					mfScaleX;
	float					mfSize;
	sal_Bool				mbAntialiased;
	sal_Bool				mbVertical;
	sal_Bool				mbNativeFontOwner;

public:
	static void				clearNativeFonts();

							JavaImplFont( OUString aName, float fSize, short nOrientation, sal_Bool bAntialiased, sal_Bool bVertical, double fScaleX );
							JavaImplFont( JavaImplFont *pFont );
	virtual					~JavaImplFont();

	sal_IntPtr				getNativeFont();
	short					getOrientation();
	OUString				getPSName();
	double					getScaleX();
	float					getSize();
	sal_Bool				isAntialiased();
	sal_Bool				isVertical();
};

// -------------------
// - JavaSalGraphics -
// -------------------

class JavaSalGraphics : public SalGraphics
{
private:
	::osl::Mutex			maUndrawnNativeOpsMutex;
	::std::list< JavaSalGraphicsOp* >	maUndrawnNativeOpsList;
	SalColor				mnBackgroundColor;
	CGLayerRef				maLayer;
	sal_uInt32				mnPixelContextData;
	CGContextRef			maPixelContext;
	CGRect					maNeedsDisplayRect;

public:
	SalColor				mnFillColor;
	SalColor				mnLineColor;
	SalColor				mnTextColor;
	SalColor				mnFillTransparency;
	SalColor				mnLineTransparency;
	JavaSalFrame*			mpFrame;
	JavaSalPrinter*			mpPrinter;
	JavaSalVirtualDevice*	mpVirDev;
	JavaPhysicalFontFace*	mpFontData;
	JavaImplFont*			mpFont;
	::boost::unordered_map< int, JavaImplFont* >	maFallbackFonts;
	::boost::unordered_map< int, Size >	maFallbackFontSizes;
	ImplLayoutRuns			maFallbackRuns;
	FontFamily				mnFontFamily;
	FontWeight				mnFontWeight;
	bool					mbFontItalic;
	FontPitch				mnFontPitch;
	FontWidth				mnFontWidthType;
	sal_Int32				mnDPIX;
	sal_Int32				mnDPIY;
	CGPathRef				maFrameClipPath;
	CGMutablePathRef		maNativeClipPath;
	bool					mbInvert;
	bool					mbXOR;
	Orientation				meOrientation;
	sal_Bool				mbPaperRotated;
	float					mfPageTranslateX;
	float					mfPageTranslateY;
	float					mfPageOutputWidth;
	float					mfPageOutputHeight;
	CGRect					maNativeBounds;

	static float			getContextBackingFactor( CGContextRef aContext );
	static void				setContextDefaultSettings( CGContextRef aContext, const CGPathRef aFrameClipPath, const CGPathRef aClipPath, float fLineWidth );

							JavaSalGraphics();
	virtual					~JavaSalGraphics();

	virtual void			drawPixel( long nX, long nY ) SAL_OVERRIDE;
	virtual void			drawPixel( long nX, long nY, SalColor nSalColor ) SAL_OVERRIDE;
	virtual void			drawLine( long nX1, long nY1, long nX2, long nY2 ) SAL_OVERRIDE;
	virtual void			drawRect( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
	virtual void			drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) SAL_OVERRIDE;
	virtual void			drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) SAL_OVERRIDE;
	virtual void			drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry ) SAL_OVERRIDE;
	virtual bool			drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly, double fTransparency ) SAL_OVERRIDE;
	virtual bool			drawPolyLine( const ::basegfx::B2DPolygon& rPoly, double fTransparency, const ::basegfx::B2DVector& rLineWidths, basegfx::B2DLineJoin eLineJoin, com::sun::star::drawing::LineCap nLineCap ) SAL_OVERRIDE;
	virtual bool			drawPolyLineBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry ) SAL_OVERRIDE;
	virtual bool			drawPolygonBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry ) SAL_OVERRIDE;
	virtual bool			drawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints, const SalPoint* const* pPtAry, const sal_uInt8* const* pFlgAry ) SAL_OVERRIDE;
	virtual void			copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth, long nSrcHeight, sal_uInt16 nFlags ) SAL_OVERRIDE;
	virtual void			copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics ) SAL_OVERRIDE;
	virtual void			drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap ) SAL_OVERRIDE;
	virtual void			drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap, SalColor nTransparentColor ) SAL_OVERRIDE;
	virtual void			drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap, const SalBitmap& rTransparentBitmap ) SAL_OVERRIDE;
	virtual void			drawMask( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap, SalColor nMaskColor ) SAL_OVERRIDE;
	virtual SalBitmap*		getBitmap( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
	virtual SalColor		getPixel( long nX, long nY ) SAL_OVERRIDE;
	virtual void			invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags ) SAL_OVERRIDE;
	virtual void			invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags ) SAL_OVERRIDE;
	virtual bool			drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize ) SAL_OVERRIDE;
	virtual void			GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) SAL_OVERRIDE;
	virtual sal_uInt16		GetBitCount() const SAL_OVERRIDE;
	virtual long			GetGraphicsWidth() const SAL_OVERRIDE;
	virtual void			ResetClipRegion() SAL_OVERRIDE;
	virtual bool			setClipRegion( const vcl::Region& rRegion ) SAL_OVERRIDE;
	virtual void			SetLineColor() SAL_OVERRIDE;
	virtual void			SetLineColor( SalColor nSalColor ) SAL_OVERRIDE;
	virtual void			SetFillColor() SAL_OVERRIDE;
	virtual void			SetFillColor( SalColor nSalColor ) SAL_OVERRIDE;
	virtual void			SetXORMode( bool bSet, bool bInvertOnly ) SAL_OVERRIDE;
	virtual void			SetROPLineColor( SalROPColor nROPColor ) SAL_OVERRIDE;
	virtual void			SetROPFillColor( SalROPColor nROPColor ) SAL_OVERRIDE;
	virtual void			SetTextColor( SalColor nSalColor ) SAL_OVERRIDE;
	virtual sal_uInt16		SetFont( FontSelectPattern* pFont, int nFallbackLevel ) SAL_OVERRIDE;
	virtual void			GetFontMetric( ImplFontMetricData* pMetric, int nFallbackLevel = 0 ) SAL_OVERRIDE;
	virtual const FontCharMapPtr	GetFontCharMap() const SAL_OVERRIDE;
	virtual void			GetDevFontList( PhysicalFontCollection* ) SAL_OVERRIDE;
	virtual bool			AddTempDevFont( PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) SAL_OVERRIDE;
	virtual bool			CreateFontSubset( const OUString& rToFile, const PhysicalFontFace* pFont, sal_GlyphId* pGlyphIDs, sal_uInt8* pEncoding, sal_Int32* pWidths, int nGlyphs, FontSubsetInfo& rInfo ) SAL_OVERRIDE;
	virtual const Ucs2SIntMap*	GetFontEncodingVector( const PhysicalFontFace*, const Ucs2OStrMap** ppNonEncoded, std::set<sal_Unicode> const** ppPriority ) SAL_OVERRIDE;
	virtual const void*		GetEmbedFontData( const PhysicalFontFace* pFont, const sal_Ucs* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen ) SAL_OVERRIDE;
	virtual void			FreeEmbedFontData( const void* pData, long nDataLen ) SAL_OVERRIDE;
	virtual void			GetGlyphWidths( const PhysicalFontFace* pFont, bool bVertical, Int32Vector& rWidths, Ucs2UIntMap& rUnicodeEnc ) SAL_OVERRIDE;
	virtual bool			GetGlyphBoundRect( sal_GlyphId nIndex, Rectangle& ) SAL_OVERRIDE;
	virtual bool			GetGlyphOutline( sal_GlyphId nIndex, basegfx::B2DPolyPolygon& rPolyPoly ) SAL_OVERRIDE;
	virtual SalLayout*		GetTextLayout( ImplLayoutArgs&, int nFallbackLevel ) SAL_OVERRIDE;
	virtual void			DrawServerFontLayout( const ServerFontLayout& ) SAL_OVERRIDE;
	virtual bool			IsNativeControlSupported( ControlType nType, ControlPart nPart ) SAL_OVERRIDE;
	virtual bool			hitTestNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, const Point& aPos, bool& rIsInside ) SAL_OVERRIDE;
	virtual bool			drawNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState nState, const ImplControlValue& aValue, const OUString& rCaption ) SAL_OVERRIDE;
	virtual bool			getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState nState, const ImplControlValue& aValue, const OUString& rCaption, Rectangle& rNativeBoundingRegion, Rectangle& rNativeContentRegion ) SAL_OVERRIDE;
	virtual bool			getNativeControlTextColor( ControlType nType, ControlPart nPart, ControlState nState, const ImplControlValue& aValue, SalColor& textColor ) SAL_OVERRIDE;
	virtual bool			drawAlphaBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSourceBitmap, const SalBitmap& rAlphaBitmap ) SAL_OVERRIDE;
	virtual bool			drawTransformedBitmap( const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX, const basegfx::B2DPoint& rY, const SalBitmap& rSourceBitmap, const SalBitmap* pAlphaBitmap ) SAL_OVERRIDE;
	virtual bool			drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency ) SAL_OVERRIDE;
	virtual bool			supportsOperation( OutDevSupportType ) const SAL_OVERRIDE;
	virtual SystemGraphicsData	GetGraphicsData() const SAL_OVERRIDE;
	virtual SystemFontData	GetSysFontData( int nFallbacklevel ) const SAL_OVERRIDE;
	virtual bool			blendBitmap( const SalTwoRect& rPosAry, const SalBitmap& rBitmap ) SAL_OVERRIDE;
	virtual bool			blendAlphaBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSrcBitmap, const SalBitmap& rMaskBitmap, const SalBitmap& rAlphaBitmap ) SAL_OVERRIDE;
	virtual bool			drawGradient( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient ) SAL_OVERRIDE;
	virtual SalGraphicsImpl*	GetImpl() const SAL_OVERRIDE;
	virtual bool			GetFontCapabilities( vcl::FontCapabilities& rFontCapabilities ) const SAL_OVERRIDE;
	virtual void			ClearDevFontCache() SAL_OVERRIDE;
	virtual void 			BeginPaint() SAL_OVERRIDE {};
	virtual void			EndPaint() SAL_OVERRIDE {};

	void					setLineTransparency( sal_uInt16 nTransparency );
	void					setFillTransparency( sal_uInt16 nTransparency );
	void					setFrameClipPath( CGPathRef aFrameClipPath );
	void					addNeedsDisplayRect( const CGRect aRect, float fLineWidth );
	void					addUndrawnNativeOp( JavaSalGraphicsOp *pOp );
	void					copyFromGraphics( JavaSalGraphics *pSrcGraphics, CGRect aSrcRect, CGRect aDestRect, bool bAllowXOR );
	void					copyToContext( const CGPathRef aFrameClipPath, const CGPathRef aNativeClipPath, bool bInvert, bool bXOR, CGContextRef aDestContext, CGRect aDestBounds, CGRect aSrcRect, CGRect aDestRect, bool bDestIsWindow = false, bool bDestIsUnflipped = false );
	void					drawUndrawnNativeOps( CGContextRef aContext, CGRect aRect );
	sal_uLong				getBitmapDirectionFormat();
	CGLayerRef				getLayer() { return maLayer; }
	float					getNativeLineWidth();
	::osl::Mutex&			getUndrawnNativeOpsMutex() { return maUndrawnNativeOpsMutex; }
	void					setBackgroundColor( SalColor nBackgroundColor );
	void					setLayer( CGLayerRef aLayer );
	void					setNeedsDisplay( NSView *pView );
};

SAL_DLLPRIVATE bool AddPolygonToPaths( CGMutablePathRef aCGPath, const ::basegfx::B2DPolygon& rPolygon, bool bClosePath, CGRect aUnflippedBounds );
SAL_DLLPRIVATE bool AddPolyPolygonToPaths( CGMutablePathRef aCGPath, const ::basegfx::B2DPolyPolygon& rPolyPoly, CGRect aUnflippedBounds );
SAL_DLLPRIVATE CGColorRef CreateCGColorFromSalColor( SalColor nColor );
SAL_DLLPRIVATE CGPoint UnflipFlippedPoint( CGPoint aFlippedPoint, CGRect aUnflippedBounds );
SAL_DLLPRIVATE CGRect UnflipFlippedRect( CGRect aFlippedRect, CGRect aUnflippedBounds );

#endif // _SV_SALGDI_H
