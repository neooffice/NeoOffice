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

#define _SV_SALGDI3_CXX

#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif

using namespace rtl;
using namespace vcl;

// =======================================================================

void SalGraphics::SetTextColor( SalColor nSalColor )
{
	maGraphicsData.mnTextColor = nSalColor;
}

// -----------------------------------------------------------------------

USHORT SalGraphics::SetFont( ImplFontSelectData* pFont )
{
	if ( maGraphicsData.mpVCLFont )
		delete maGraphicsData.mpVCLFont;

	maGraphicsData.mpVCLFont = ((com_sun_star_vcl_VCLFont *)pFont->mpFontData->mpSysData)->deriveFont( pFont->mnHeight, pFont->meWeight <= WEIGHT_MEDIUM ? FALSE : TRUE, pFont->meItalic == ITALIC_NONE ? FALSE : TRUE, pFont->mnOrientation, !pFont->mbNonAntialiased );

	if ( maGraphicsData.mpPrinter )
		return SAL_SETFONT_USEDRAWTEXTARRAY;
	else
		return 0;
}

// -----------------------------------------------------------------------

long SalGraphics::GetCharWidth( sal_Unicode nChar1, sal_Unicode nChar2, long* pWidthAry )
{
	if ( maGraphicsData.mpVCLFont )
		maGraphicsData.mpVCLFont->getCharWidth( nChar1, nChar2, pWidthAry );
	else
		memset( pWidthAry, 0, ( nChar2 - nChar1 + 1 ) * sizeof( sal_Unicode ) );

	return 1;
}

// -----------------------------------------------------------------------

void SalGraphics::GetFontMetric( ImplFontMetricData* pMetric )
{
	pMetric->mnWidth = 0;
	if ( maGraphicsData.mpVCLFont )
	{
		pMetric->mnAscent = maGraphicsData.mpVCLFont->getAscent();
		pMetric->mnDescent = maGraphicsData.mpVCLFont->getDescent();
		pMetric->mnLeading = maGraphicsData.mpVCLFont->getLeading();
	}
	else
	{
		pMetric->mnAscent = 0;
		pMetric->mnDescent = 0;
		pMetric->mnLeading = 0;
	}
	pMetric->mnSlant = 0;
	pMetric->mnFirstChar = 0;
	pMetric->mnLastChar = 255;
	if ( maGraphicsData.mpVCLFont )
	{
		pMetric->maName = maGraphicsData.mpVCLFont->getName();
		pMetric->mnOrientation = maGraphicsData.mpVCLFont->getOrientation();
		pMetric->meFamily = maGraphicsData.mpVCLFont->getFamilyType();
	}
	else
	{
		pMetric->maName = OUString();
		pMetric->mnOrientation = 0;
		pMetric->meFamily = FAMILY_DONTKNOW;
	}
	pMetric->meCharSet = gsl_getSystemTextEncoding();
	if ( maGraphicsData.mpVCLFont && maGraphicsData.mpVCLFont->isBold() )
		pMetric->meWeight = WEIGHT_BOLD;
	else
		pMetric->meWeight = WEIGHT_NORMAL;
	if ( maGraphicsData.mpVCLFont && maGraphicsData.mpVCLFont->isItalic() )
		pMetric->meItalic = ITALIC_NORMAL;
	else
		pMetric->meItalic = ITALIC_NONE;
	if ( pMetric->meFamily == FAMILY_MODERN )
		pMetric->mePitch = PITCH_FIXED;
	else
		pMetric->mePitch = PITCH_VARIABLE;
	pMetric->meType = TYPE_SCALABLE;
	pMetric->mbDevice = FALSE;
}

// -----------------------------------------------------------------------

ULONG SalGraphics::GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs )
{
	if ( maGraphicsData.mpVCLFont )
		return 0;
	
	ImplKernPairData *pPair = pKernPairs;
	for ( ULONG i = 0; i < nPairs; i++ )
	{
		pPair->mnKern = maGraphicsData.mpVCLFont->getKerning( pPair->mnChar1, pPair->mnChar2 );
		pPair++;
	}

	return nPairs;
}

// -----------------------------------------------------------------------

ULONG SalGraphics::GetFontCodeRanges( sal_uInt32* pCodePairs ) const
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::GetFontCodeRanges not implemented\n" );
#endif
	return 0;
}

// -----------------------------------------------------------------------

void SalGraphics::GetDevFontList( ImplDevFontList* pList )
{
	com_sun_star_vcl_VCLFontList *pFontList = GetSalData()->mpFontList;

	// Iterate through fonts and add each to the font list
	for ( jsize i = 0; i < pFontList->nCount; i++ )
	{
		// Set default values
		ImplFontData *pFontData = new ImplFontData();
		pFontData->mpNext = NULL;
		pFontData->mpSysData = (void *)&pFontList->pFonts[ i ];
		pFontData->maName = XubString( pFontList->pFonts[ i ].getName() );
		pFontData->mnWidth = 0;
		pFontData->mnHeight = 0;
		pFontData->meFamily = pFontList->pFonts[ i ].getFamilyType();
		pFontData->meCharSet = gsl_getSystemTextEncoding();
		if ( pFontData->meFamily == FAMILY_MODERN )
			pFontData->mePitch = PITCH_FIXED;
		else
			pFontData->mePitch = PITCH_VARIABLE;
		pFontData->meWidthType = WIDTH_DONTKNOW;
		if ( pFontList->pFonts[ i ].isBold() )
			pFontData->meWeight = WEIGHT_BOLD;
		else
			pFontData->meWeight = WEIGHT_NORMAL;
		if ( pFontList->pFonts[ i ].isItalic() )
			pFontData->meItalic = ITALIC_NORMAL;
		else
			pFontData->meItalic = ITALIC_NONE;
		pFontData->meType = TYPE_SCALABLE;
		pFontData->mnVerticalOrientation = 0;
		pFontData->mbOrientation = TRUE;
		pFontData->mbDevice = FALSE;
		pFontData->mnQuality = 0;

		// Add to list
		pList->Add( pFontData );
	}
}

// -----------------------------------------------------------------------

void SalGraphics::DrawText( long nX, long nY,
                            const xub_Unicode* pStr, xub_StrLen nLen )
{
	if ( maGraphicsData.mpVCLFont )
		maGraphicsData.mpVCLGraphics->drawText( nX, nY, pStr, nLen, maGraphicsData.mpVCLFont, maGraphicsData.mnTextColor );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawTextArray( long nX, long nY,
                                 const xub_Unicode* pStr, xub_StrLen nLen,
                                 const long* pDXAry )
{
	if ( maGraphicsData.mpVCLFont )
		maGraphicsData.mpVCLGraphics->drawTextArray( nX, nY, pStr, nLen, maGraphicsData.mpVCLFont, maGraphicsData.mnTextColor, pDXAry );
}

// -----------------------------------------------------------------------

BOOL SalGraphics::GetGlyphBoundRect( xub_Unicode cChar, long* pX, long* pY,
                                     long* pWidth, long* pHeight )
{
	if ( maGraphicsData.mpVCLFont )
	{
		Size aSize( maGraphicsData.mpVCLGraphics->getGlyphSize( cChar, maGraphicsData.mpVCLFont ) );
		*pX = 0;
		*pY = 0;
		*pWidth = aSize.Width();
		*pHeight = aSize.Height();
		return TRUE;
	}
	else
		return FALSE;
}

// -----------------------------------------------------------------------

ULONG SalGraphics::GetGlyphOutline( xub_Unicode cChar, USHORT** ppPolySizes,
                                    SalPoint** ppPoints, BYTE** ppFlags )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::GetGlyphOutline not implemented\n" );
#endif
	*ppPolySizes = NULL;
	*ppPoints = NULL;
	*ppFlags = NULL;
	return 0;
}
