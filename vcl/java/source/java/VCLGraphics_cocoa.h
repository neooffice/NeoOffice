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
 *  Patrick Luby, September 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 Planamesa Inc.
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

#ifndef __VCLGRAPHICS_COCOA_H__
#define __VCLGRAPHICS_COCOA_H__

#include <salframe.h>
#include <salprn.h>
#include <salvd.h>
#include <sal/types.h>

#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING

#ifdef __cplusplus
#include <premac.h>
#endif
#include <ApplicationServices/ApplicationServices.h>
#ifdef __cplusplus
#include <postmac.h>
#endif

#ifdef __cplusplus
BEGIN_C
#endif
SAL_DLLPRIVATE void CGContext_drawGlyphs( float fX, float fY, size_t nCount, CGGlyph *pGlyphs, CGSize *pSizes, CGFontRef aFont, float fFontSize, int nColor, float fGlyphTranslateX, float fGlyphTranslateY, float fGlyphRotateAngle, float fGlyphScaleX, float fGlyphScaleY, CGPathRef aClipPath, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
SAL_DLLPRIVATE void CGContext_drawLine( float fX1, float fY1, float fX2, float fY2, int nColor, CGPathRef aClipPath, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
SAL_DLLPRIVATE void CGContext_drawPath( int nColor, BOOL bFill, CGPathRef aPath, CGPathRef aClipPath, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
SAL_DLLPRIVATE void CGContext_drawPathline( int nColor, float fLineWidth, CGLineJoin nJoin, CGPathRef aPath, CGPathRef aClipPath, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
SAL_DLLPRIVATE void CGContext_drawPolygon( int nPoints, float *pXPoints, float *pYPoints, int nColor, BOOL bFill, CGPathRef aClipPath, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
SAL_DLLPRIVATE void CGContext_drawPolyline( int nPoints, float *pXPoints, float *pYPoints, int nColor, CGPathRef aClipPath, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
SAL_DLLPRIVATE void CGContext_drawPolyPolygon( int nPoly, int *pNPoints, float **ppXPoints, float **ppYPoints, int nColor, BOOL bFill, CGPathRef aClipPath, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
SAL_DLLPRIVATE void CGContext_drawRect( float fX, float fY, float fWidth, float fHeight, int nColor, BOOL bFill, CGPathRef aClipPath, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
SAL_DLLPRIVATE void CGImageRef_drawInRect( CGImageRef aImage, float fX, float fY, float fWidth, float fHeight, CGPathRef aClipPath, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
SAL_DLLPRIVATE void NSEPSImageRep_drawInRect( void *pPtr, unsigned nSize, float fX, float fY, float fWidth, float fHeight, CGPathRef aClipPath, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
#ifdef __cplusplus
END_C
#endif

#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING

#endif
