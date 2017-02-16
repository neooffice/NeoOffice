/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 * 
 *   Modified December 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <sal/types.h>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <boost/scoped_array.hpp>

#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/settings.hxx>

#include "outdata.hxx"
#include "salgdi.hxx"

#ifdef USE_JAVA

#include <tools/fract.hxx>

#ifdef MACOSX
#include "java/salgdi.h"
#endif	// MACOSX
 
#define MAX_TRANSPARENT_GRADIENT_BMP_PIXELS ( 4 * 1024 * 1024 )
#ifdef MACOSX
#define MIN_TRANSPARENT_GRADIENT_RESOLUTION ( 2 * MIN_SCREEN_RESOLUTION )
#else	// MACOSX
#define MIN_TRANSPARENT_GRADIENT_RESOLUTION ( 2 * 72 )
#endif	// MACOSX

#endif	// USE_JAVA

namespace
{
    /**
     * Perform a safe approximation of a polygon from double-precision
     * coordinates to integer coordinates, to ensure that it has at least 2
     * pixels in both X and Y directions.
     */
    Polygon toPolygon( const basegfx::B2DPolygon& rPoly )
    {
        basegfx::B2DRange aRange = rPoly.getB2DRange();
        double fW = aRange.getWidth(), fH = aRange.getHeight();
        if (0.0 < fW && 0.0 < fH && (fW <= 1.0 || fH <= 1.0))
        {
            // This polygon not empty but is too small to display.  Approximate it
            // with a rectangle large enough to be displayed.
            double nX = aRange.getMinX(), nY = aRange.getMinY();
            double nW = std::max<double>(1.0, rtl::math::round(fW));
            double nH = std::max<double>(1.0, rtl::math::round(fH));

            Polygon aTarget;
            aTarget.Insert(0, Point(nX, nY));
            aTarget.Insert(1, Point(nX+nW, nY));
            aTarget.Insert(2, Point(nX+nW, nY+nH));
            aTarget.Insert(3, Point(nX, nY+nH));
            aTarget.Insert(4, Point(nX, nY));
            return aTarget;
        }
        return Polygon(rPoly);
    }

    tools::PolyPolygon toPolyPolygon( const basegfx::B2DPolyPolygon& rPolyPoly )
    {
        tools::PolyPolygon aTarget;
        for (sal_uInt32 i = 0; i < rPolyPoly.count(); ++i)
            aTarget.Insert(toPolygon(rPolyPoly.getB2DPolygon(i)));

        return aTarget;
    }
}

Color OutputDevice::ImplDrawModeToColor( const Color& rColor ) const
{
    Color aColor( rColor );
    sal_uLong  nDrawMode = GetDrawMode();

    if( nDrawMode & ( DRAWMODE_BLACKLINE | DRAWMODE_WHITELINE |
                      DRAWMODE_GRAYLINE | DRAWMODE_GHOSTEDLINE |
                      DRAWMODE_SETTINGSLINE ) )
    {
        if( !ImplIsColorTransparent( aColor ) )
        {
            if( nDrawMode & DRAWMODE_BLACKLINE )
            {
                aColor = Color( COL_BLACK );
            }
            else if( nDrawMode & DRAWMODE_WHITELINE )
            {
                aColor = Color( COL_WHITE );
            }
            else if( nDrawMode & DRAWMODE_GRAYLINE )
            {
                const sal_uInt8 cLum = aColor.GetLuminance();
                aColor = Color( cLum, cLum, cLum );
            }
            else if( nDrawMode & DRAWMODE_SETTINGSLINE )
            {
                aColor = GetSettings().GetStyleSettings().GetFontColor();
            }

            if( nDrawMode & DRAWMODE_GHOSTEDLINE )
            {
                aColor = Color( ( aColor.GetRed() >> 1 ) | 0x80,
                                ( aColor.GetGreen() >> 1 ) | 0x80,
                                ( aColor.GetBlue() >> 1 ) | 0x80);
            }
        }
    }
    return aColor;
}

sal_uInt16 OutputDevice::GetAlphaBitCount() const
{
    return 0;
}

bool OutputDevice::HasAlpha()
{
    return mpAlphaVDev != NULL;
}

void OutputDevice::ImplPrintTransparent( const Bitmap& rBmp, const Bitmap& rMask,
                                         const Point& rDestPt, const Size& rDestSize,
                                         const Point& rSrcPtPixel, const Size& rSrcSizePixel )
{
    Point       aPt;
    Point       aDestPt( LogicToPixel( rDestPt ) );
    Size        aDestSz( LogicToPixel( rDestSize ) );
    Rectangle   aSrcRect( rSrcPtPixel, rSrcSizePixel );

    aSrcRect.Justify();

    if( !rBmp.IsEmpty() && aSrcRect.GetWidth() && aSrcRect.GetHeight() && aDestSz.Width() && aDestSz.Height() )
    {
        Bitmap  aPaint( rBmp ), aMask( rMask );
        sal_uLong   nMirrFlags = 0UL;

        if( aMask.GetBitCount() > 1 )
            aMask.Convert( BMP_CONVERSION_1BIT_THRESHOLD );

        // mirrored horizontically
        if( aDestSz.Width() < 0L )
        {
            aDestSz.Width() = -aDestSz.Width();
            aDestPt.X() -= ( aDestSz.Width() - 1L );
            nMirrFlags |= BMP_MIRROR_HORZ;
        }

        // mirrored vertically
        if( aDestSz.Height() < 0L )
        {
            aDestSz.Height() = -aDestSz.Height();
            aDestPt.Y() -= ( aDestSz.Height() - 1L );
            nMirrFlags |= BMP_MIRROR_VERT;
        }

        // source cropped?
        if( aSrcRect != Rectangle( aPt, aPaint.GetSizePixel() ) )
        {
            aPaint.Crop( aSrcRect );
            aMask.Crop( aSrcRect );
        }

        // destination mirrored
        if( nMirrFlags )
        {
            aPaint.Mirror( nMirrFlags );
            aMask.Mirror( nMirrFlags );
        }

        // we always want to have a mask
        if( aMask.IsEmpty() )
        {
            aMask = Bitmap( aSrcRect.GetSize(), 1 );
            aMask.Erase( Color( COL_BLACK ) );
        }

        // do painting
        const long nSrcWidth = aSrcRect.GetWidth(), nSrcHeight = aSrcRect.GetHeight();
        long nX, nY; // , nWorkX, nWorkY, nWorkWidth, nWorkHeight;
        boost::scoped_array<long> pMapX(new long[ nSrcWidth + 1 ]);
        boost::scoped_array<long> pMapY(new long[ nSrcHeight + 1 ]);
        const bool bOldMap = mbMap;

        mbMap = false;

        // create forward mapping tables
        for( nX = 0L; nX <= nSrcWidth; nX++ )
            pMapX[ nX ] = aDestPt.X() + FRound( (double) aDestSz.Width() * nX / nSrcWidth );

        for( nY = 0L; nY <= nSrcHeight; nY++ )
            pMapY[ nY ] = aDestPt.Y() + FRound( (double) aDestSz.Height() * nY / nSrcHeight );

        // walk through all rectangles of mask
        const vcl::Region aWorkRgn(aMask.CreateRegion(COL_BLACK, Rectangle(Point(), aMask.GetSizePixel())));
        RectangleVector aRectangles;
        aWorkRgn.GetRegionRectangles(aRectangles);

        for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
        {
            const Point aMapPt(pMapX[aRectIter->Left()], pMapY[aRectIter->Top()]);
            const Size aMapSz( pMapX[aRectIter->Right() + 1] - aMapPt.X(),      // pMapX[L + W] -> L + ((R - L) + 1) -> R + 1
                               pMapY[aRectIter->Bottom() + 1] - aMapPt.Y());    // same for Y
            Bitmap aBandBmp(aPaint);

            aBandBmp.Crop(*aRectIter);
            DrawBitmap(aMapPt, aMapSz, Point(), aBandBmp.GetSizePixel(), aBandBmp, META_BMPSCALEPART_ACTION);
        }

        mbMap = bOldMap;
    }
}

// Caution: This method is nearly the same as
// void OutputDevice::DrawPolyPolygon( const basegfx::B2DPolyPolygon& rB2DPolyPoly )
// so when changes are made here do not forget to make changes there, too

void OutputDevice::DrawTransparent( const basegfx::B2DPolyPolygon& rB2DPolyPoly, double fTransparency)
{
    // AW: Do NOT paint empty PolyPolygons
    if(!rB2DPolyPoly.count())
        return;

    // we need a graphics
    if( !mpGraphics && !AcquireGraphics() )
        return;

    if( mbInitClipRegion )
        InitClipRegion();

    if( mbOutputClipped )
        return;

    if( mbInitLineColor )
        InitLineColor();

    if( mbInitFillColor )
        InitFillColor();

    if((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW) &&
       mpGraphics->supportsOperation(OutDevSupport_B2DDraw) &&
       (ROP_OVERPAINT == GetRasterOp()) )
    {
        // b2dpolygon support not implemented yet on non-UNX platforms
        const ::basegfx::B2DHomMatrix aTransform = ImplGetDeviceTransformation();
        basegfx::B2DPolyPolygon aB2DPolyPolygon(rB2DPolyPoly);

        // transform the polygon into device space and ensure it is closed
        aB2DPolyPolygon.transform( aTransform );
        aB2DPolyPolygon.setClosed( true );

        bool bDrawnOk = true;
        if( IsFillColor() )
            bDrawnOk = mpGraphics->DrawPolyPolygon( aB2DPolyPolygon, fTransparency, this );

        if( bDrawnOk && IsLineColor() )
        {
            const basegfx::B2DVector aHairlineWidth(1,1);
            const int nPolyCount = aB2DPolyPolygon.count();
            for( int nPolyIdx = 0; nPolyIdx < nPolyCount; ++nPolyIdx )
            {
                const ::basegfx::B2DPolygon aOnePoly = aB2DPolyPolygon.getB2DPolygon( nPolyIdx );
                mpGraphics->DrawPolyLine( aOnePoly, fTransparency, aHairlineWidth, ::basegfx::B2DLINEJOIN_NONE, com::sun::star::drawing::LineCap_BUTT, this );
            }
        }

        if( bDrawnOk )
        {
            if( mpMetaFile )
                mpMetaFile->AddAction( new MetaTransparentAction( tools::PolyPolygon( rB2DPolyPoly ), static_cast< sal_uInt16 >(fTransparency * 100.0)));

            return;
        }
    }

    // fallback to old polygon drawing if needed
    DrawTransparent(toPolyPolygon(rB2DPolyPoly), static_cast<sal_uInt16>(fTransparency * 100.0));
}

void OutputDevice::DrawInvisiblePolygon( const tools::PolyPolygon& rPolyPoly )
{
    // short circuit if the polygon border is invisible too
    if( !mbLineColor )
        return;

    // we assume that the border is NOT to be drawn transparently???
    Push( PushFlags::FILLCOLOR );
    SetFillColor();
    DrawPolyPolygon( rPolyPoly );
    Pop();
}

bool OutputDevice::DrawTransparentNatively ( const tools::PolyPolygon& rPolyPoly,
                                             sal_uInt16 nTransparencePercent )
{
    bool bDrawn = false;

    // debug helper:
#if defined USE_JAVA && defined MACOSX
    static const char* pDisableNative = NULL;
#else	// USE_JAVA && MACOSX
    static const char* pDisableNative = getenv( "SAL_DISABLE_NATIVE_ALPHA");
#endif	// USE_JAVA && MACOSX

    if( !pDisableNative &&
        mpGraphics->supportsOperation( OutDevSupport_B2DDraw )
#if defined UNX && ! defined MACOSX && ! defined IOS
        && GetBitCount() > 8
#endif
#ifdef WIN32
        // workaround bad dithering on remote displaying when using GDI+ with toolbar button highlighting
        && !rPolyPoly.IsRect()
#endif
        )
    {
        // prepare the graphics device
        if( mbInitClipRegion )
            InitClipRegion();

        if( mbOutputClipped )
            return false;

        if( mbInitLineColor )
            InitLineColor();

        if( mbInitFillColor )
            InitFillColor();

        // get the polygon in device coordinates
        basegfx::B2DPolyPolygon aB2DPolyPolygon( rPolyPoly.getB2DPolyPolygon() );
        const ::basegfx::B2DHomMatrix aTransform = ImplGetDeviceTransformation();
        aB2DPolyPolygon.transform( aTransform );

        const double fTransparency = 0.01 * nTransparencePercent;
        if( mbFillColor )
        {
            // #i121591#
            // CAUTION: Only non printing (pixel-renderer) VCL commands from OutputDevices
            // should be used when printing. Normally this is avoided by the printer being
            // non-AAed and thus e.g. on WIN GdiPlus calls are not used. It may be necessary
            // to figure out a way of moving this code to it's own function that is
            // overriden by the Print class, which will mean we deliberately override the
            // functionality and we use the fallback some lines below (which is not very good,
            // though. For now, WinSalGraphics::drawPolyPolygon will detect printer usage and
            // correct the wrong mapping (see there for details)
            bDrawn = mpGraphics->DrawPolyPolygon( aB2DPolyPolygon, fTransparency, this );
        }

        if( mbLineColor )
        {
            // disable the fill color for now
            mpGraphics->SetFillColor();
            // draw the border line
            const basegfx::B2DVector aLineWidths( 1, 1 );
            const int nPolyCount = aB2DPolyPolygon.count();
            for( int nPolyIdx = 0; nPolyIdx < nPolyCount; ++nPolyIdx )
            {
                const ::basegfx::B2DPolygon& rPolygon = aB2DPolyPolygon.getB2DPolygon( nPolyIdx );
                bDrawn = mpGraphics->DrawPolyLine( rPolygon, fTransparency, aLineWidths,
                                                   ::basegfx::B2DLINEJOIN_NONE, css::drawing::LineCap_BUTT, this );
            }
            // prepare to restore the fill color
            mbInitFillColor = mbFillColor;
        }
    }

    return bDrawn;
}

void OutputDevice::EmulateDrawTransparent ( const tools::PolyPolygon& rPolyPoly,
                                            sal_uInt16 nTransparencePercent )
{
#if !defined USE_JAVA || !defined MACOSX
    // debug helper:
    static const char* pDisableNative = getenv( "SAL_DISABLE_NATIVE_ALPHA" );
#endif	// !USE_JAVA || !MACOSX

    // #110958# Disable alpha VDev, we perform the necessary
    VirtualDevice* pOldAlphaVDev = mpAlphaVDev;

    // operation explicitly further below.
    if( mpAlphaVDev )
        mpAlphaVDev = NULL;

    GDIMetaFile* pOldMetaFile = mpMetaFile;
    mpMetaFile = NULL;

#if defined USE_JAVA && defined MACOSX
    if ( !mpGraphics )
        AcquireGraphics();
    if ( mpGraphics )
    {
        ((JavaSalGraphics *)mpGraphics)->setLineTransparency( nTransparencePercent );
        ((JavaSalGraphics *)mpGraphics)->setFillTransparency( nTransparencePercent );

        DrawPolyPolygon( rPolyPoly );

        ((JavaSalGraphics *)mpGraphics)->setLineTransparency( 0 );
        ((JavaSalGraphics *)mpGraphics)->setFillTransparency( 0 );
    }
#else	// USE_JAVA && MACOSX
    tools::PolyPolygon aPolyPoly( LogicToPixel( rPolyPoly ) );
    Rectangle aPolyRect( aPolyPoly.GetBoundRect() );
    Point aPoint;
    Rectangle aDstRect( aPoint, GetOutputSizePixel() );

    aDstRect.Intersection( aPolyRect );

    ClipToPaintRegion( aDstRect );

    if( !aDstRect.IsEmpty() )
    {
        bool bDrawn = false;

        // #i66849# Added fast path for exactly rectangular
        // polygons
        // #i83087# Naturally, system alpha blending cannot
        // work with separate alpha VDev
        if( !mpAlphaVDev && !pDisableNative && aPolyPoly.IsRect() )
        {
            // setup Graphics only here (other cases delegate
            // to basic OutDev methods)
            if ( mbInitClipRegion )
                InitClipRegion();

            if ( mbInitLineColor )
                InitLineColor();

            if ( mbInitFillColor )
                InitFillColor();

            Rectangle aLogicPolyRect( rPolyPoly.GetBoundRect() );
            Rectangle aPixelRect( ImplLogicToDevicePixel( aLogicPolyRect ) );

            if( !mbOutputClipped )
            {
                bDrawn = mpGraphics->DrawAlphaRect( aPixelRect.Left(), aPixelRect.Top(),
                    // #i98405# use methods with small g, else one pixel too much will be painted.
                    // This is because the source is a polygon which when painted would not paint
                    // the rightmost and lowest pixel line(s), so use one pixel less for the
                    // rectangle, too.
                                                    aPixelRect.getWidth(), aPixelRect.getHeight(),
                                                    sal::static_int_cast<sal_uInt8>(nTransparencePercent),
                                                    this );
            }
            else
            {
                bDrawn = true;
            }
        }

        if( !bDrawn )
        {
            VirtualDevice aVDev( *this, 1 );
            const Size aDstSz( aDstRect.GetSize() );
            const sal_uInt8 cTrans = (sal_uInt8) MinMax( FRound( nTransparencePercent * 2.55 ), 0, 255 );

            if( aDstRect.Left() || aDstRect.Top() )
                aPolyPoly.Move( -aDstRect.Left(), -aDstRect.Top() );

            if( aVDev.SetOutputSizePixel( aDstSz ) )
            {
                const bool bOldMap = mbMap;

                EnableMapMode( false );

                aVDev.SetLineColor( COL_BLACK );
                aVDev.SetFillColor( COL_BLACK );
                aVDev.DrawPolyPolygon( aPolyPoly );

                Bitmap aPaint( GetBitmap( aDstRect.TopLeft(), aDstSz ) );
                Bitmap aPolyMask( aVDev.GetBitmap( Point(), aDstSz ) );

                // #107766# check for non-empty bitmaps before accessing them
                if( !!aPaint && !!aPolyMask )
                {
                    BitmapWriteAccess* pW = aPaint.AcquireWriteAccess();
                    BitmapReadAccess* pR = aPolyMask.AcquireReadAccess();

                    if( pW && pR )
                    {
                        BitmapColor aPixCol;
                        const BitmapColor aFillCol( GetFillColor() );
                        const BitmapColor aWhite( pR->GetBestMatchingColor( Color( COL_WHITE ) ) );
                        const BitmapColor aBlack( pR->GetBestMatchingColor( Color( COL_BLACK ) ) );
                        const long nWidth = pW->Width();
                        const long nHeight = pW->Height();
                        const long nR = aFillCol.GetRed();
                        const long nG = aFillCol.GetGreen();
                        const long nB = aFillCol.GetBlue();
                        long nX, nY;

                        if( aPaint.GetBitCount() <= 8 )
                        {
                            const BitmapPalette& rPal = pW->GetPalette();
                            const sal_uInt16 nCount = rPal.GetEntryCount();
                            BitmapColor* pMap = (BitmapColor*) new sal_uInt8[ nCount * sizeof( BitmapColor ) ];

                            for( sal_uInt16 i = 0; i < nCount; i++ )
                            {
                                BitmapColor aCol( rPal[ i ] );
                                pMap[ i ] = BitmapColor( (sal_uInt8) rPal.GetBestIndex( aCol.Merge( aFillCol, cTrans ) ) );
                            }

                            if( pR->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL &&
                                pW->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
                            {
                                const sal_uInt8 cBlack = aBlack.GetIndex();

                                for( nY = 0; nY < nHeight; nY++ )
                                {
                                    Scanline pWScan = pW->GetScanline( nY );
                                    Scanline pRScan = pR->GetScanline( nY );
                                    sal_uInt8 cBit = 128;

                                    for( nX = 0; nX < nWidth; nX++, cBit >>= 1, pWScan++ )
                                    {
                                        if( !cBit )
                                        {
                                            cBit = 128;
                                            pRScan += 1;
                                        }
                                        if( ( *pRScan & cBit ) == cBlack )
                                        {
                                            *pWScan = (sal_uInt8) pMap[ *pWScan ].GetIndex();
                                        }
                                    }
                                }
                            }
                            else
                            {
                                for( nY = 0; nY < nHeight; nY++ )
                                {
                                    for( nX = 0; nX < nWidth; nX++ )
                                    {
                                        if( pR->GetPixel( nY, nX ) == aBlack )
                                        {
                                            pW->SetPixel( nY, nX, pMap[ pW->GetPixel( nY, nX ).GetIndex() ] );
                                        }
                                    }
                                }
                            }
                            delete[] (sal_uInt8*) pMap;
                        }
                        else
                        {
                            if( pR->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL &&
                                pW->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_BGR )
                            {
                                const sal_uInt8 cBlack = aBlack.GetIndex();

                                for( nY = 0; nY < nHeight; nY++ )
                                {
                                    Scanline pWScan = pW->GetScanline( nY );
                                    Scanline pRScan = pR->GetScanline( nY );
                                    sal_uInt8 cBit = 128;

                                    for( nX = 0; nX < nWidth; nX++, cBit >>= 1, pWScan += 3 )
                                    {
                                        if( !cBit )
                                        {
                                            cBit = 128;
                                            pRScan += 1;
                                        }
                                        if( ( *pRScan & cBit ) == cBlack )
                                        {
                                            pWScan[ 0 ] = COLOR_CHANNEL_MERGE( pWScan[ 0 ], nB, cTrans );
                                            pWScan[ 1 ] = COLOR_CHANNEL_MERGE( pWScan[ 1 ], nG, cTrans );
                                            pWScan[ 2 ] = COLOR_CHANNEL_MERGE( pWScan[ 2 ], nR, cTrans );
                                        }
                                    }
                                }
                            }
                            else
                            {
                                for( nY = 0; nY < nHeight; nY++ )
                                {
                                    for( nX = 0; nX < nWidth; nX++ )
                                    {
                                        if( pR->GetPixel( nY, nX ) == aBlack )
                                        {
                                            aPixCol = pW->GetColor( nY, nX );
                                            pW->SetPixel( nY, nX, aPixCol.Merge( aFillCol, cTrans ) );
                                        }
                                    }
                                }
                            }
                        }
                    }

                    aPolyMask.ReleaseAccess( pR );
                    aPaint.ReleaseAccess( pW );

                    DrawBitmap( aDstRect.TopLeft(), aPaint );

                    EnableMapMode( bOldMap );

                    if( mbLineColor )
                    {
                        Push( PushFlags::FILLCOLOR );
                        SetFillColor();
                        DrawPolyPolygon( rPolyPoly );
                        Pop();
                    }
                }
            }
            else
            {
                DrawPolyPolygon( rPolyPoly );
            }
        }
    }
#endif	// USE_JAVA && MACOSX

    mpMetaFile = pOldMetaFile;

    // #110958# Restore disabled alpha VDev
    mpAlphaVDev = pOldAlphaVDev;
}

void OutputDevice::DrawTransparent( const tools::PolyPolygon& rPolyPoly,
                                    sal_uInt16 nTransparencePercent )
{
    // short circuit for drawing an opaque polygon
    if( (nTransparencePercent < 1) || ((mnDrawMode & DRAWMODE_NOTRANSPARENCY) != 0) )
    {
        DrawPolyPolygon( rPolyPoly );
        return;
    }

    // short circuit for drawing an invisible polygon
    if( !mbFillColor || (nTransparencePercent >= 100) )
    {
        DrawInvisiblePolygon( rPolyPoly );
        return; // tdf#84294: do not record it in metafile
    }

    // handle metafile recording
    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaTransparentAction( rPolyPoly, nTransparencePercent ) );

    bool bDrawn = !IsDeviceOutputNecessary() || ImplIsRecordLayout();
    if( bDrawn )
        return;

    // get the device graphics as drawing target
    if( !mpGraphics && !AcquireGraphics() )
        return;

    // try hard to draw it directly, because the emulation layers are slower
    bDrawn = DrawTransparentNatively( rPolyPoly, nTransparencePercent );
    if( bDrawn )
        return;

    EmulateDrawTransparent( rPolyPoly, nTransparencePercent );

    // #110958# Apply alpha value also to VDev alpha channel
    if( mpAlphaVDev )
    {
        const Color aFillCol( mpAlphaVDev->GetFillColor() );
        mpAlphaVDev->SetFillColor( Color(sal::static_int_cast<sal_uInt8>(255*nTransparencePercent/100),
                                         sal::static_int_cast<sal_uInt8>(255*nTransparencePercent/100),
                                         sal::static_int_cast<sal_uInt8>(255*nTransparencePercent/100)) );

        mpAlphaVDev->DrawTransparent( rPolyPoly, nTransparencePercent );

        mpAlphaVDev->SetFillColor( aFillCol );
    }
}

void OutputDevice::DrawTransparent( const GDIMetaFile& rMtf, const Point& rPos,
                                    const Size& rSize, const Gradient& rTransparenceGradient )
{

    const Color aBlack( COL_BLACK );

    if( mpMetaFile )
    {
         // missing here is to map the data using the DeviceTransformation
        mpMetaFile->AddAction( new MetaFloatTransparentAction( rMtf, rPos, rSize, rTransparenceGradient ) );
    }

    if ( !IsDeviceOutputNecessary() )
        return;

    if( ( rTransparenceGradient.GetStartColor() == aBlack && rTransparenceGradient.GetEndColor() == aBlack ) ||
        ( mnDrawMode & ( DRAWMODE_NOTRANSPARENCY ) ) )
    {
        ( (GDIMetaFile&) rMtf ).WindStart();
        ( (GDIMetaFile&) rMtf ).Play( this, rPos, rSize );
        ( (GDIMetaFile&) rMtf ).WindStart();
    }
    else
    {
        GDIMetaFile* pOldMetaFile = mpMetaFile;
        Rectangle aOutRect( LogicToPixel( rPos ), LogicToPixel( rSize ) );
        Point aPoint;
        Rectangle aDstRect( aPoint, GetOutputSizePixel() );

        mpMetaFile = NULL;
        aDstRect.Intersection( aOutRect );

        ClipToPaintRegion( aDstRect );

        if( !aDstRect.IsEmpty() )
        {
            boost::scoped_ptr<VirtualDevice> pVDev(new VirtualDevice);

            ((OutputDevice*)pVDev.get())->mnDPIX = mnDPIX;
            ((OutputDevice*)pVDev.get())->mnDPIY = mnDPIY;

#ifdef USE_JAVA
            // Prevent runaway memory usage when drawing to the printer by
            // lowering the resolution of the temporary buffer if necessary
            Rectangle aVirDevRect( Point( 0, 0 ), aDstRect.GetSize() );
            float fExcessPixelRatio = (float)MAX_TRANSPARENT_GRADIENT_BMP_PIXELS / ( ( mnDPIX * aDstRect.GetWidth() ) + ( mnDPIY * aDstRect.GetHeight() ) );
            if ( fExcessPixelRatio < 1.0 )
            {
                ((OutputDevice*)pVDev.get())->mnDPIX = (sal_uInt32)( ( fExcessPixelRatio * mnDPIX ) + 0.5f );
                ((OutputDevice*)pVDev.get())->mnDPIY = (sal_uInt32)( ( fExcessPixelRatio * mnDPIY ) + 0.5f );

                if ( ((OutputDevice*)pVDev.get())->mnDPIX < MIN_TRANSPARENT_GRADIENT_RESOLUTION)
                    ((OutputDevice*)pVDev.get())->mnDPIX = MIN_TRANSPARENT_GRADIENT_RESOLUTION;
                if ( ((OutputDevice*)pVDev.get())->mnDPIY < MIN_TRANSPARENT_GRADIENT_RESOLUTION)
                    ((OutputDevice*)pVDev.get())->mnDPIY = MIN_TRANSPARENT_GRADIENT_RESOLUTION;
            }

            Fraction aScaleX( ((OutputDevice*)pVDev.get())->mnDPIX, mnDPIX );
            Fraction aScaleY( ((OutputDevice*)pVDev.get())->mnDPIY, mnDPIY );

            aVirDevRect = Rectangle( Point( 0, 0 ), Size( (long)( ( (double)aScaleX * aDstRect.GetWidth() ) + 0.5 ), (long)( ( (double)aScaleY * aDstRect.GetHeight() ) + 0.5 ) ) );

            if( pVDev->SetOutputSizePixel( aVirDevRect.GetSize() ) )
#else	// USE_JAVA
            if( pVDev->SetOutputSizePixel( aDstRect.GetSize() ) )
#endif	// USE_JAVA
            {
#if defined USE_JAVA && defined MACOSX
                // Do not draw gradients using a alpha mask even when
                // antialiasing is enabled as it will causes areas outside of
                // the clip region to be filled with gray
#else	// USE_JAVA && MACOSX
                if(GetAntialiasing())
                {
                    // #i102109#
                    // For MetaFile replay (see task) it may now be necessary to take
                    // into account that the content is AntiAlialised and needs to be masked
                    // like that. Instead of masking, i will use a copy-modify-paste cycle
                    // here (as i already use in the VclPrimiziveRenderer with successs)
                    pVDev->SetAntialiasing(GetAntialiasing());

                    // create MapMode for buffer (offset needed) and set
                    MapMode aMap(GetMapMode());
                    const Point aOutPos(PixelToLogic(aDstRect.TopLeft()));
                    aMap.SetOrigin(Point(-aOutPos.X(), -aOutPos.Y()));
#ifdef USE_JAVA
                    if ( (double)aScaleX > 1.0f )
                        aMap.SetScaleX( aScaleX );
                    if ( (double)aScaleY > 1.0f )
                        aMap.SetScaleY( aScaleY );
#endif	// USE_JAVA
                    pVDev->SetMapMode(aMap);

                    // copy MapMode state and disable for target
                    const bool bOrigMapModeEnabled(IsMapModeEnabled());
                    EnableMapMode(false);

                    // copy MapMode state and disable for buffer
                    const bool bBufferMapModeEnabled(pVDev->IsMapModeEnabled());
                    pVDev->EnableMapMode(false);

                    // copy content from original to buffer
                    pVDev->DrawOutDev( aPoint, pVDev->GetOutputSizePixel(), // dest
                                       aDstRect.TopLeft(), pVDev->GetOutputSizePixel(), // source
                                       *this);

                    // draw MetaFile to buffer
                    pVDev->EnableMapMode(bBufferMapModeEnabled);
                    ((GDIMetaFile&)rMtf).WindStart();
                    ((GDIMetaFile&)rMtf).Play(pVDev.get(), rPos, rSize);
                    ((GDIMetaFile&)rMtf).WindStart();

                    // get content bitmap from buffer
                    pVDev->EnableMapMode(false);

                    const Bitmap aPaint(pVDev->GetBitmap(aPoint, pVDev->GetOutputSizePixel()));

                    // create alpha mask from gradient and get as Bitmap
                    pVDev->EnableMapMode(bBufferMapModeEnabled);
                    pVDev->SetDrawMode(DRAWMODE_GRAYGRADIENT);
                    pVDev->DrawGradient(Rectangle(rPos, rSize), rTransparenceGradient);
                    pVDev->SetDrawMode(DRAWMODE_DEFAULT);
                    pVDev->EnableMapMode(false);

                    const AlphaMask aAlpha(pVDev->GetBitmap(aPoint, pVDev->GetOutputSizePixel()));

                    pVDev.reset();

                    // draw masked content to target and restore MapMode
                    DrawBitmapEx(aDstRect.TopLeft(), BitmapEx(aPaint, aAlpha));
                    EnableMapMode(bOrigMapModeEnabled);
                }
                else
#endif	// USE_JAVA && MACOSX
                {
                    Bitmap aPaint, aMask;
                    AlphaMask aAlpha;
                    MapMode aMap( GetMapMode() );
                    Point aOutPos( PixelToLogic( aDstRect.TopLeft() ) );
                    const bool bOldMap = mbMap;

                    aMap.SetOrigin( Point( -aOutPos.X(), -aOutPos.Y() ) );
#ifdef USE_JAVA
                    if ( (double)aScaleX > 1.0f )
                        aMap.SetScaleX( aScaleX );
                    if ( (double)aScaleY > 1.0f )
                        aMap.SetScaleY( aScaleY );
#endif	// USE_JAVA
                    pVDev->SetMapMode( aMap );
                    const bool bVDevOldMap = pVDev->IsMapModeEnabled();

                    // create paint bitmap
                    ( (GDIMetaFile&) rMtf ).WindStart();
                    ( (GDIMetaFile&) rMtf ).Play( pVDev.get(), rPos, rSize );
                    ( (GDIMetaFile&) rMtf ).WindStart();
                    pVDev->EnableMapMode( false );
                    aPaint = pVDev->GetBitmap( Point(), pVDev->GetOutputSizePixel() );
                    pVDev->EnableMapMode( bVDevOldMap ); // #i35331#: MUST NOT use EnableMapMode( sal_True ) here!

                    // create mask bitmap
                    pVDev->SetLineColor( COL_BLACK );
                    pVDev->SetFillColor( COL_BLACK );
                    pVDev->DrawRect( Rectangle( pVDev->PixelToLogic( Point() ), pVDev->GetOutputSize() ) );
                    pVDev->SetDrawMode( DRAWMODE_WHITELINE | DRAWMODE_WHITEFILL | DRAWMODE_WHITETEXT |
                                        DRAWMODE_WHITEBITMAP | DRAWMODE_WHITEGRADIENT );
                    ( (GDIMetaFile&) rMtf ).WindStart();
                    ( (GDIMetaFile&) rMtf ).Play( pVDev.get(), rPos, rSize );
                    ( (GDIMetaFile&) rMtf ).WindStart();
                    pVDev->EnableMapMode( false );
                    aMask = pVDev->GetBitmap( Point(), pVDev->GetOutputSizePixel() );
                    pVDev->EnableMapMode( bVDevOldMap ); // #i35331#: MUST NOT use EnableMapMode( sal_True ) here!

                    // create alpha mask from gradient
                    pVDev->SetDrawMode( DRAWMODE_GRAYGRADIENT );
                    pVDev->DrawGradient( Rectangle( rPos, rSize ), rTransparenceGradient );
                    pVDev->SetDrawMode( DRAWMODE_DEFAULT );
                    pVDev->EnableMapMode( false );
                    pVDev->DrawMask( Point(), pVDev->GetOutputSizePixel(), aMask, Color( COL_WHITE ) );

                    aAlpha = pVDev->GetBitmap( Point(), pVDev->GetOutputSizePixel() );

                    pVDev.reset();

#ifdef USE_JAVA
                    size_t nGradBeginPos = GDI_METAFILE_END;
                    size_t nGradEndPos = GDI_METAFILE_END;
                    size_t nTransGradPushClipBeginPos = GDI_METAFILE_END;
                    size_t nTransGradPushClipEndPos = GDI_METAFILE_END;
                    size_t nTransGradPopClipBeginPos = GDI_METAFILE_END;
                    size_t nTransGradPopClipEndPos = GDI_METAFILE_END;
                    size_t nCount = ( (GDIMetaFile&) rMtf ).GetActionSize();
                    size_t nPos;
                    for ( nPos = 0; nPos < nCount; nPos++ )
                    {
                        MetaAction *pAct = ( (GDIMetaFile&) rMtf ).GetAction( nPos );
                        if ( pAct && pAct->GetType() == META_COMMENT_ACTION )
                        {
                            if ( ((MetaCommentAction *)pAct)->GetComment().equalsIgnoreAsciiCase( "XGRAD_SEQ_BEGIN" ) )
                                nGradBeginPos = nPos;
                            else if ( ((MetaCommentAction *)pAct)->GetComment().equalsIgnoreAsciiCase( "XGRAD_SEQ_END" ) )
                                nGradEndPos = nPos;
                            else if ( ((MetaCommentAction *)pAct)->GetComment().equalsIgnoreAsciiCase( "XTRANSGRADPUSHCLIP_SEQ_BEGIN" ) )
                                nTransGradPushClipBeginPos = nPos;
                            else if ( ((MetaCommentAction *)pAct)->GetComment().equalsIgnoreAsciiCase( "XTRANSGRADPUSHCLIP_SEQ_END" ) )
                                nTransGradPushClipEndPos = nPos;
                            else if ( ((MetaCommentAction *)pAct)->GetComment().equalsIgnoreAsciiCase( "XTRANSGRADPOPCLIP_SEQ_BEGIN" ) )
                                nTransGradPopClipBeginPos = nPos;
                            else if ( ((MetaCommentAction *)pAct)->GetComment().equalsIgnoreAsciiCase( "XTRANSGRADPOPCLIP_SEQ_END" ) )
                                nTransGradPopClipEndPos = nPos;
                        }
                    }

                    // Only use metafile's clip if the clip commands are at the
                    // beginning and end of the metafile
                    if ( ( !nTransGradPushClipBeginPos || nTransGradPushClipBeginPos - 1 == nGradBeginPos ) &&
                        nTransGradPushClipBeginPos < nTransGradPushClipEndPos &&
                        nTransGradPushClipEndPos < nTransGradPopClipBeginPos &&
                        nTransGradPopClipBeginPos < nTransGradPopClipEndPos &&
                        ( nTransGradPopClipEndPos == nCount - 1 || nTransGradPopClipEndPos == nGradEndPos - 1 ) )
                    {
                        // Copy code from GDIMetaFile::Play for creating map mode
                        // to use when executing metafile commands
                        MapMode aOldMap( GetMapMode() );
                        MapMode aGDIMap( ( (GDIMetaFile&) rMtf ).GetPrefMapMode() );
                        if ( aOutRect.GetWidth() && aOutRect.GetHeight() )
                        {
                            Size aTmpPrefSize( LogicToPixel( ( (GDIMetaFile&) rMtf ).GetPrefSize(), aGDIMap ) );
                            if ( !aTmpPrefSize.Width() )
                                aTmpPrefSize.Width() = aOutRect.GetWidth();

                            if ( !aTmpPrefSize.Height() )
                                aTmpPrefSize.Height() = aOutRect.GetHeight();

                            Fraction aGDIScaleX( aOutRect.GetWidth(), aTmpPrefSize.Width() );
                            Fraction aGDIScaleY( aOutRect.GetHeight(), aTmpPrefSize.Height() );

                            aGDIScaleX *= aGDIMap.GetScaleX(); aGDIMap.SetScaleX( aGDIScaleX );
                            aGDIScaleY *= aGDIMap.GetScaleY(); aGDIMap.SetScaleY( aGDIScaleY );
        
                            const Size& rOldOffset( GetPixelOffset() );
                            const Size aEmptySize;
                            SetPixelOffset( aEmptySize );
                            aGDIMap.SetOrigin( PixelToLogic( LogicToPixel( rPos ), aGDIMap ) );
                            SetPixelOffset( rOldOffset );

                            Push();
                            SetMapMode( aGDIMap );

                            for ( nPos = nTransGradPushClipBeginPos; nPos < nTransGradPushClipEndPos; nPos++ )
                            {
                                MetaAction *pAct = ( (GDIMetaFile&) rMtf ).GetAction( nPos );
                                if ( pAct )
                                    pAct->Execute( this );
                            }

                            SetMapMode( aOldMap );

                            EnableMapMode( false );
                            DrawBitmapEx( aDstRect.TopLeft(), aDstRect.GetSize(), aVirDevRect.TopLeft(), aVirDevRect.GetSize(), BitmapEx( aPaint, aAlpha ) );
                            EnableMapMode( bOldMap );

                            SetMapMode( aGDIMap );

                            for ( nPos = nTransGradPopClipBeginPos; nPos < nTransGradPopClipEndPos; nPos++ )
                            {
                                MetaAction *pAct = ( (GDIMetaFile&) rMtf ).GetAction( nPos );
                                if ( pAct )
                                    pAct->Execute( this );
                            }


                            SetMapMode( aOldMap );
                            Pop();
                        }
                    }
                    else
                    {
                        EnableMapMode( false );
                        DrawBitmapEx( aDstRect.TopLeft(), aDstRect.GetSize(), aVirDevRect.TopLeft(), aVirDevRect.GetSize(), BitmapEx( aPaint, aAlpha ) );
                        EnableMapMode( bOldMap );
                    }
#else	// USE_JAVA
                    EnableMapMode( false );
                    DrawBitmapEx( aDstRect.TopLeft(), BitmapEx( aPaint, aAlpha ) );
                    EnableMapMode( bOldMap );
#endif	// USE_JAVA
                }
            }
        }

        mpMetaFile = pOldMetaFile;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
