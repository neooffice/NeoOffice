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
 *  Modified June 2004 by Patrick Luby. SISSL Removed. NeoOffice is
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
#ifndef _VOS_THREAD_HXX
#include <vos/thread.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif

#include <map>

class ImplDevFontList;
class SalBitmap;
struct ImplFontSelectData;
struct ImplFontMetricData;
struct ImplKernPairData;
struct ImplFontData;
class PolyPolygon;
class FontCharMap;
class SalLayout;
class ImplLayoutArgs;
class Rectangle;
struct FontSubsetInfo;
class OutputDevice;

// ---------------------
// - SalGraphics-Codes -
// ---------------------

#define SAL_SETFONT_REMOVEANDMATCHNEW       ((USHORT)0x0001)
#define SAL_SETFONT_USEDRAWTEXT             ((USHORT)0x0002)
#define SAL_SETFONT_USEDRAWTEXTARRAY        ((USHORT)0x0004)
#define SAL_SETFONT_UNICODE                 ((USHORT)0x0008)
#define SAL_SETFONT_BADFONT                 ((USHORT)0x1000)

#define SAL_COPYAREA_WINDOWINVALIDATE       ((USHORT)0x0001)

// ---------------
// - SalGraphics -
// ---------------

// note: if you add any new methods to class SalGraphics that work with coordinates 
//       make sure they are virtual and add them to class SalGraphicsLayout where you have
//       to perform coordinate mirroring if required, (see existing methods as sample)

class SalGraphics
{
    friend class SalFrame;
    friend class SalVirtualDevice;
    friend class SalPrinter;

public:                     // public for Sal Implementation
                            SalGraphics();
                            virtual ~SalGraphics();

public:                     // public for Sal Implementation
    SalGraphicsData         maGraphicsData;

    // to be overridden by derived class
    virtual int             GetLayout() { return 0; }	// base class supports left-to-right only
    virtual void            SetLayout( int ) {};

#ifdef _INCL_SAL_SALGDI_IMP
#include <salgdi.imp>
#endif

public:

    // this functions must be quick, because this data is query for all
    // GetGraphics()-Instances
    void                    GetResolution( long& rDPIX, long& rDPIY );
    void                    GetScreenFontResolution( long& rDPIX, long& rDPIY );
    USHORT                  GetBitCount();
    long                    GetGraphicsWidth();

    void                    ResetClipRegion();
    void                    BeginSetClipRegion( ULONG nCount );
    virtual BOOL            UnionClipRegion( long nX, long nY, long nWidth, long nHeight, const OutputDevice *pOutDev );
    void                    EndSetClipRegion();

    void                    SetLineColor();
    void                    SetLineColor( SalColor nSalColor );
    void                    SetFillColor();

    void                    SetFillColor( SalColor nSalColor );

    void                    SetXORMode( BOOL bSet );

    void                    SetROPLineColor( SalROPColor nROPColor );
    void                    SetROPFillColor( SalROPColor nROPColor );

    // all positions are in pixel and relative to
    // the top/left-position of the output area
    void                    SetTextColor( SalColor nSalColor );
    USHORT                  SetFont( ImplFontSelectData*, int nFallbackLevel );
    void                    GetFontMetric( ImplFontMetricData* );
                            // return only PairCount when (pKernPairs == NULL)
    ULONG                   GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs );
    ULONG                   GetFontCodeRanges( sal_uInt32* pCodePairs ) const;
    // graphics must fill supplied font list
    void                    GetDevFontList( ImplDevFontList* );
    // graphics should call ImplAddDevFontSubstitute on supplied
    // OutputDevice for all its device specific preferred font substitutions
    void					GetDevFontSubstList( OutputDevice* );
    ImplFontData*           AddTempDevFont( const String& rFileURL, const String& rFontName );
    static void             RemovingFont( ImplFontData* );

    // CreateFontSubset: a method to get a subset of glyhps of a font
    // inside a new valid font file
    // returns TRUE if creation of subset was successfull
    // parameters: rToFile: contains a osl file URL to write the subset to
    //             pFont: describes from which font to create a subset
    //             pGlyphIDs: the glyph ids to be extracted
    //             pEncoding: the character code corresponding to each glyph
    //             pWidths: the advance widths of the correspoding glyphs (in PS font units)
    //             nGlyphs: the number of glyphs
    //             rInfo: additional outgoing information
    // implementation note: encoding 0 with glyph id 0 should be added implicitly
    // as "undefined character"
    BOOL					CreateFontSubset( const rtl::OUString& rToFile,
                                              ImplFontData* pFont,
                                              long* pGlyphIDs,
                                              sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo // out parameter
                                              );

    // GetFontEncodingVector: a method to get the encoding map Unicode
	// to font encoded character; this is only used for type1 fonts and
    // may return NULL in case of unknown encoding vector
    // if ppNonEncoded is set and non encoded characters (that is type1
    // glyphs with only a name) exist it is set to the corresponding
    // map for non encoded glyphs; the encoding vector contains -1
    // as encoding for these cases
    const std::map< sal_Unicode, sal_Int32 >* GetFontEncodingVector( ImplFontData* pFont, const std::map< sal_Unicode, rtl::OString >** ppNonEncoded );

    // GetEmbedFontData: gets the font data for a font marked
    // embeddable by GetDevFontList or NULL in case of error
    // parameters: pFont: describes the font in question
    //             pUnicodes: contains the Unicodes assigned to
    //             code points 0 to 255; must contain at least 256 members
    //             pWidths: the widths of all glyphs from char code 0 to 255
    //                      pWidths MUST support at least 256 members;
    //             rInfo: additional outgoing information
    //             pDataLen: out parameter, contains the byte length of the returned buffer
    const void*                     GetEmbedFontData( ImplFontData* pFont,
                                                      const sal_Unicode* pUnicodes,
                                                      sal_Int32* pWidths,
                                                      FontSubsetInfo& rInfo,
                                                      long* pDataLen );
    // frees the font data again
    void                            FreeEmbedFontData( const void* pData, long nDataLen );

    virtual BOOL                    GetGlyphBoundRect( long nIndex, Rectangle&, const OutputDevice *pOutDev );
    virtual BOOL                    GetGlyphOutline( long nIndex, PolyPolygon&, const OutputDevice *pOutDev );

    virtual SalLayout*              GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
    virtual void                    DrawPixel( long nX, long nY, const OutputDevice *pOutDev );
    virtual void                    DrawPixel( long nX, long nY, SalColor nSalColor, const OutputDevice *pOutDev );
    virtual void                    DrawLine( long nX1, long nY1, long nX2, long nY2, const OutputDevice *pOutDev );
    virtual void                    DrawRect( long nX, long nY, long nWidth, long nHeight, const OutputDevice *pOutDev );
    virtual void                    DrawPolyLine( ULONG nPoints, const SalPoint* pPtAry, const OutputDevice *pOutDev );
    virtual void                    DrawPolygon( ULONG nPoints, const SalPoint* pPtAry, const OutputDevice *pOutDev );
    virtual void                    DrawPolyPolygon( ULONG nPoly, const ULONG* pPoints,
                                             PCONSTSALPOINT* pPtAry, const OutputDevice *pOutDev );
    virtual sal_Bool                DrawPolyLineBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry, const OutputDevice *pOutDev );
    virtual sal_Bool                DrawPolygonBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry, const OutputDevice *pOutDev );
    virtual sal_Bool                DrawPolyPolygonBezier( ULONG nPoly, const ULONG* pPoints,
                                                   const SalPoint* const* pPtAry, const BYTE* const* pFlgAry, const OutputDevice *pOutDev );

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void                    CopyArea( long nDestX, long nDestY,
                                      long nSrcX, long nSrcY,
                                      long nSrcWidth, long nSrcHeight,
                                      USHORT nFlags, const OutputDevice *pOutDev );

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    virtual void                    CopyBits( const SalTwoRect* pPosAry,
                                      SalGraphics* pSrcGraphics, const OutputDevice *pOutDev, const OutputDevice *pSrcOutDev );
    virtual void                    DrawBitmap( const SalTwoRect* pPosAry,
                                        const SalBitmap& rSalBitmap, const OutputDevice *pOutDev );
    virtual void                    DrawBitmap( const SalTwoRect* pPosAry,
                                        const SalBitmap& rSalBitmap,
                                        SalColor nTransparentColor, const OutputDevice *pOutDev );
    virtual void                    DrawBitmap( const SalTwoRect* pPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rTransparentBitmap, const OutputDevice *pOutDev );

    virtual void                    DrawMask( const SalTwoRect* pPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor, const OutputDevice *pOutDev );

    virtual SalBitmap*              GetBitmap( long nX, long nY, long nWidth, long nHeight, const OutputDevice *pOutDev );
    virtual SalColor                GetPixel( long nX, long nY, const OutputDevice *pOutDev );

    // invert --> ClipRegion (only Windows)
    virtual void                    Invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags, const OutputDevice *pOutDev );
    virtual void                    Invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nFlags, const OutputDevice *pOutDev );

    virtual BOOL                    DrawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize, const OutputDevice *pOutDev );
#ifdef USE_JAVA
	virtual void					SetAntialias( BOOL bAntialias );
#endif
};


// ---------------------
// - SalGraphicsLayout -
// ---------------------

// extend SalGraphics for RTL

class SalGraphicsLayout : public SalGraphics
{
	friend class SalFrame;
	friend class SalVirtualDevice;
	friend class SalPrinter;

private:
	int                             mnLayout;

public:
							SalGraphicsLayout();
							~SalGraphicsLayout();

	int                     GetLayout() { return mnLayout; }
	void					SetLayout( int aLayout)	{ mnLayout = aLayout;}

	void					mirror( long& nX, const OutputDevice *pOutDev );
	void					mirror( long& nX, long& nWidth, const OutputDevice *pOutDev );
	BOOL					mirror( sal_uInt32 nPoints, const SalPoint *pPtAry, SalPoint *pPtAry2, const OutputDevice *pOutDev );

	// overwrite those SalGraphics methods that require mirroring
    // note: text methods will not be mirrored here, this is handled in outdev3.cxx
	BOOL					UnionClipRegion( long nX, long nY, long nWidth, long nHeight, const OutputDevice *pOutDev );
	void					DrawPixel( long nX, long nY, const OutputDevice *pOutDev );
	void					DrawPixel( long nX, long nY, SalColor nSalColor, const OutputDevice *pOutDev );
	void					DrawLine( long nX1, long nY1, long nX2, long nY2, const OutputDevice *pOutDev );
	void					DrawRect( long nX, long nY, long nWidth, long nHeight, const OutputDevice *pOutDev );
	void					DrawPolyLine( ULONG nPoints, const SalPoint* pPtAry, const OutputDevice *pOutDev );
	void					DrawPolygon( ULONG nPoints, const SalPoint* pPtAry, const OutputDevice *pOutDev );
	void					DrawPolyPolygon( ULONG nPoly, const ULONG* pPoints,
											 PCONSTSALPOINT* pPtAry, const OutputDevice *pOutDev );
    sal_Bool                DrawPolyLineBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry, const OutputDevice *pOutDev );
    sal_Bool                DrawPolygonBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry, const OutputDevice *pOutDev );
    sal_Bool                DrawPolyPolygonBezier( ULONG nPoly, const ULONG* pPoints,
                                                   const SalPoint* const* pPtAry, const BYTE* const* pFlgAry, const OutputDevice *pOutDev );
	void					CopyArea( long nDestX, long nDestY,
									  long nSrcX, long nSrcY,
									  long nSrcWidth, long nSrcHeight,
									  USHORT nFlags, const OutputDevice *pOutDev );
	void					CopyBits( const SalTwoRect* pPosAry,
									  SalGraphics* pSrcGraphics, const OutputDevice *pOutDev, const OutputDevice *pSrcOutDev );
	void					DrawBitmap( const SalTwoRect* pPosAry,
										const SalBitmap& rSalBitmap, const OutputDevice *pOutDev );
	void					DrawBitmap( const SalTwoRect* pPosAry,
										const SalBitmap& rSalBitmap,
										SalColor nTransparentColor, const OutputDevice *pOutDev );
	void					DrawBitmap( const SalTwoRect* pPosAry,
										const SalBitmap& rSalBitmap,
										const SalBitmap& rTransparentBitmap, const OutputDevice *pOutDev );
	void					DrawMask( const SalTwoRect* pPosAry,
									  const SalBitmap& rSalBitmap,
									  SalColor nMaskColor, const OutputDevice *pOutDev );
	SalBitmap*				GetBitmap( long nX, long nY, long nWidth, long nHeight, const OutputDevice *pOutDev );
	SalColor				GetPixel( long nX, long nY, const OutputDevice *pOutDev );
	void					Invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags, const OutputDevice *pOutDev );
	void					Invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nFlags, const OutputDevice *pOutDev );
	BOOL					DrawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize, const OutputDevice *pOutDev );
#ifdef USE_JAVA
	void					SetAntialias( BOOL bAntialias );
#endif
};

#endif // _SV_SALGDI_HXX
