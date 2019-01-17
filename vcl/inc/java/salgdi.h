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
	mutable ::std::list< JavaPhysicalFontFace* >	maChildren;
	OUString				maFamilyName;
	mutable const JavaPhysicalFontFace*	mpParent;

	static void				ClearNativeFonts();
	static void				HandleBadFont( const JavaPhysicalFontFace *pFontData );
	static bool				IsBadFont( const JavaPhysicalFontFace *pFontData, bool bHandleIfBadFont = true );
	DECL_STATIC_LINK( JavaPhysicalFontFace, RunNativeFontsTimer, void*, void );

							JavaPhysicalFontFace( const ImplDevFontAttributes& rAttibutes, const OUString& rFontName, sal_IntPtr nNativeFontID, const OUString& rFamilyName );
	virtual					~JavaPhysicalFontFace();

	virtual LogicalFontInstance*	CreateFontInstance( FontSelectPattern& rData ) const override;
	virtual PhysicalFontFace*	Clone() const override;
	virtual sal_IntPtr		GetFontId() const override;
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
	com::sun::star::drawing::LineCap	mnLineCap;

public:
							JavaSalGraphicsDrawPathOp( const CGPathRef aFrameClip, const CGPathRef aNativeClipPath, bool bInvert, bool bXOR, bool bAntialias, SalColor nFillColor, SalColor nLineColor, const CGPathRef aPath, bool bShiftLines = true, float fLineWidth = 0, ::basegfx::B2DLineJoin eLineJoin = ::basegfx::B2DLineJoin::NONE, bool bLineDash = false, com::sun::star::drawing::LineCap nLineCap = com::sun::star::drawing::LineCap_SQUARE );
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

	virtual void			drawPixel( long nX, long nY ) override;
	virtual void			drawPixel( long nX, long nY, SalColor nSalColor ) override;
	virtual void			drawLine( long nX1, long nY1, long nX2, long nY2 ) override;
	virtual void			drawRect( long nX, long nY, long nWidth, long nHeight ) override;
	virtual void			drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;
	virtual void			drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry ) override;
	virtual void			drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry ) override;
	virtual bool			drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly, double fTransparency ) override;
	virtual bool			drawPolyLine( const ::basegfx::B2DPolygon& rPoly, double fTransparency, const ::basegfx::B2DVector& rLineWidths, basegfx::B2DLineJoin eLineJoin, com::sun::star::drawing::LineCap nLineCap, double fMiterMinimumAngle ) override;
	virtual bool			drawPolyLineBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const PolyFlags* pFlgAry ) override;
	virtual bool			drawPolygonBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const PolyFlags* pFlgAry ) override;
	virtual bool			drawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints, const SalPoint* const* pPtAry, const PolyFlags* const* pFlgAry ) override;
	virtual void			copyArea( long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth, long nSrcHeight, bool bWindowInvalidate ) override;
	virtual void			copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics ) override;
	virtual void			drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap ) override;
	virtual void			drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap, const SalBitmap& rMaskBitmap ) override;
	virtual void			drawMask( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap, SalColor nMaskColor ) override;
	virtual SalBitmap*		getBitmap( long nX, long nY, long nWidth, long nHeight ) override;
	virtual SalColor		getPixel( long nX, long nY ) override;
	virtual void			invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags ) override;
	virtual void			invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags ) override;
	virtual bool			drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize ) override;
	virtual void			GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) override;
	virtual sal_uInt16		GetBitCount() const override;
	virtual long			GetGraphicsWidth() const override;
	virtual void			ResetClipRegion() override;
	virtual bool			setClipRegion( const vcl::Region& rRegion ) override;
	virtual void			SetLineColor() override;
	virtual void			SetLineColor( SalColor nSalColor ) override;
	virtual void			SetFillColor() override;
	virtual void			SetFillColor( SalColor nSalColor ) override;
	virtual void			SetXORMode( bool bSet ) override;
	virtual void			SetROPLineColor( SalROPColor nROPColor ) override;
	virtual void			SetROPFillColor( SalROPColor nROPColor ) override;
	virtual void			SetTextColor( SalColor nSalColor ) override;
	virtual void			SetFont( FontSelectPattern* pFont, int nFallbackLevel ) override;
	virtual void			GetFontMetric( ImplFontMetricDataRef& rMetric, int nFallbackLevel ) override;
	virtual const FontCharMapRef	GetFontCharMap() const override;
	virtual void			GetDevFontList( PhysicalFontCollection* ) override;
	virtual bool			AddTempDevFont( PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) override;
	virtual bool			CreateFontSubset( const OUString& rToFile, const PhysicalFontFace* pFont, const sal_GlyphId* pGlyphIDs, const sal_uInt8* pEncoding, sal_Int32* pWidths, int nGlyphs, FontSubsetInfo& rInfo ) override;
	virtual const void*		GetEmbedFontData( const PhysicalFontFace* pFont, long* pDataLen ) override;
	virtual void			FreeEmbedFontData( const void* pData, long nDataLen ) override;
	virtual void			GetGlyphWidths( const PhysicalFontFace* pFont, bool bVertical, std::vector< sal_Int32 >& rWidths, Ucs2UIntMap& rUnicodeEnc ) override;
	virtual bool			GetGlyphBoundRect( const GlyphItem& rIndex, tools::Rectangle& ) override;
	virtual bool			GetGlyphOutline( const GlyphItem& rIndex, basegfx::B2DPolyPolygon& rPolyPoly ) override;
	virtual SalLayout*		GetTextLayout( ImplLayoutArgs&, int nFallbackLevel ) override;
	virtual bool			IsNativeControlSupported( ControlType nType, ControlPart nPart ) override;
	virtual bool			hitTestNativeControl( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion, const Point& aPos, bool& rIsInside ) override;
	virtual bool			drawNativeControl( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion, ControlState nState, const ImplControlValue& aValue, const OUString& rCaption ) override;
	virtual bool			getNativeControlRegion( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion, ControlState nState, const ImplControlValue& aValue, const OUString& rCaption, tools::Rectangle& rNativeBoundingRegion, tools::Rectangle& rNativeContentRegion ) override;
	virtual bool			getNativeControlTextColor( ControlType nType, ControlPart nPart, ControlState nState, const ImplControlValue& aValue, SalColor& textColor ) override;
	virtual bool			drawAlphaBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSourceBitmap, const SalBitmap& rAlphaBitmap ) override;
	virtual bool			drawTransformedBitmap( const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX, const basegfx::B2DPoint& rY, const SalBitmap& rSourceBitmap, const SalBitmap* pAlphaBitmap ) override;
	virtual bool			drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency ) override;
	virtual bool			supportsOperation( OutDevSupportType ) const override;
	virtual SystemGraphicsData	GetGraphicsData() const override;
	virtual bool			blendBitmap( const SalTwoRect& rPosAry, const SalBitmap& rBitmap ) override;
	virtual bool			blendAlphaBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSrcBitmap, const SalBitmap& rMaskBitmap, const SalBitmap& rAlphaBitmap ) override;
	virtual bool			drawGradient( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient ) override;
	virtual SalGraphicsImpl*	GetImpl() const override;
	virtual bool			GetFontCapabilities( vcl::FontCapabilities& rFontCapabilities ) const override;
	virtual void			ClearDevFontCache() override;

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
