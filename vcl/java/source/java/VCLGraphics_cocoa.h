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
void CGContect_drawGlyphs( float fX, float fY, size_t nCount, CGGlyph *pGlyphs, CGSize *pSizes, CGFontRef aFont, int nFontSize, float fFontScaleX, int nColor, float fGlyphTranslateX, float fGlyphTranslateY, float fGlyphRotateAngle, float fGlyphScaleX, float fGlyphScaleY, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
void CGContext_drawLine( float fX1, float fY1, float fX2, float fY2, int nColor, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
void CGContext_drawPolygon( int nPoints, float *pXPoints, float *pYPoints, int nColor, BOOL bFill, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
void CGContext_drawPolyline( int nPoints, float *pXPoints, float *pYPoints, int nColor, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
void CGContext_drawRect( float fX, float fY, float fWidth, float fHeight, int nColor, BOOL bFill, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
void CGImageRef_drawInRect( CGImageRef aImage, float fX, float fY, float fWidth, float fHeight, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
void NSEPSImageRep_drawInRect( void *pPtr, unsigned nSize, float fX, float fY, float fWidth, float fHeight, float fClipX, float fClipY, float fClipWidth, float fClipHeight, BOOL bDrawInMainThread, float fTranslateX, float fTranslateY, float fRotateAngle, float fScaleX, float fScaleY );
#ifdef __cplusplus
END_C
#endif

#endif
