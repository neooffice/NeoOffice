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
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif

#ifdef MACOSX
#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>
#endif	// MACOSX

using namespace rtl;
using namespace vcl;

// =======================================================================

void SalGraphics::SetTextColor( SalColor nSalColor )
{
	maGraphicsData.mnTextColor = nSalColor;
}

// -----------------------------------------------------------------------

USHORT SalGraphics::SetFont( ImplFontSelectData* pFont, int nFallbackLevel )
{
	SalData *pSalData = GetSalData();
	sal_Bool bBold = ( pFont->meWeight > WEIGHT_MEDIUM );
	sal_Bool bItalic = ( pFont->meItalic != ITALIC_NONE );

	com_sun_star_vcl_VCLFont *pVCLFont = (com_sun_star_vcl_VCLFont *)pFont->mpFontData->mpSysData;
	XubString aName( pVCLFont->getName() );

#ifdef MACOSX
	// Don't change the font for fallback levels as we need the first font
	// to properly determine the fallback font
	if ( !nFallbackLevel )
	{
		// Handle remapping to and from bold and italic fonts
		if ( bBold || bItalic || aName != pFont->maFoundName )
		{
			ATSUFontID nFontID = (ATSUFontID)pVCLFont->getNativeFont( bBold, bItalic );
			::std::map< void*, ImplFontData* >::iterator it = pSalData->maNativeFontMapping.find( (void *)nFontID );
			if ( it != pSalData->maNativeFontMapping.end() )
				aName = it->second->maName;
		}
	}
	else
	{
		// Retrieve the fallback font if one has been set by a text layout
		::std::map< int, com_sun_star_vcl_VCLFont* >::iterator ffit = maGraphicsData.maFallbackFonts.find( nFallbackLevel );
		if ( ffit != maGraphicsData.maFallbackFonts.end() )
			pVCLFont = ffit->second;
		aName = XubString( pVCLFont->getName() );
	}

#endif	// MACOSX

	// Find the matching font
	::std::map< OUString, com_sun_star_vcl_VCLFont* >::iterator it = pSalData->maFontMapping.find( aName );
	if ( it != pSalData->maFontMapping.end() )
		pVCLFont = it->second;

	// Update font in upper layers
	::std::map< void*, ImplFontData* >::iterator fit = pSalData->maNativeFontMapping.find( pVCLFont->getNativeFont() );
	if ( fit != pSalData->maNativeFontMapping.end() )
		pFont->mpFontData = fit->second;

	pFont->maFoundName = aName;

	if ( nFallbackLevel )
	{
		// Cache font select data for layout
		maGraphicsData.maFallbackFontSelectData[ nFallbackLevel ] = pFont;
	}
	else
	{
		// Set font for graphics device
		if ( maGraphicsData.mpVCLFont )
			delete maGraphicsData.mpVCLFont;
		maGraphicsData.mpVCLFont = pVCLFont->deriveFont( pFont->mnHeight, bBold, bItalic, pFont->mnOrientation, !pFont->mbNonAntialiased, pFont->mbVertical );
	}

	if ( maGraphicsData.mpPrinter )
		return SAL_SETFONT_USEDRAWTEXTARRAY;
	else
		return 0;
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
	pMetric->meCharSet = RTL_TEXTENCODING_UNICODE;
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
	SalData *pSalData = GetSalData();

	// Iterate through fonts and add each to the font list
    for ( ::std::map< OUString, com_sun_star_vcl_VCLFont* >::iterator it = pSalData->maFontMapping.begin(); it != pSalData->maFontMapping.end(); ++it )
	{
		com_sun_star_vcl_VCLFont *pVCLFont = it->second;
		void *pNativeFont = pVCLFont->getNativeFont();
		if ( !pNativeFont )
			continue;

		// Set default values
		ImplFontData *pFontData = new ImplFontData();
		pFontData->mpNext = NULL;
		pFontData->mpSysData = (void *)pVCLFont;
		pFontData->maName = XubString( it->first );
		pFontData->mnWidth = 0;
		pFontData->mnHeight = 0;
		pFontData->meFamily = pVCLFont->getFamilyType();
		pFontData->meCharSet = RTL_TEXTENCODING_UNICODE;
		if ( pFontData->meFamily == FAMILY_MODERN )
			pFontData->mePitch = PITCH_FIXED;
		else
			pFontData->mePitch = PITCH_VARIABLE;
		pFontData->meWidthType = WIDTH_DONTKNOW;
		if ( pVCLFont->isBold() )
			pFontData->meWeight = WEIGHT_BOLD;
		else
			pFontData->meWeight = WEIGHT_NORMAL;
		if ( pVCLFont->isItalic() )
			pFontData->meItalic = ITALIC_NORMAL;
		else
			pFontData->meItalic = ITALIC_NONE;
		pFontData->meType = TYPE_SCALABLE;
		pFontData->mnVerticalOrientation = 0;
		pFontData->mbOrientation = TRUE;
		pFontData->mbDevice = FALSE;
		pFontData->mnQuality = 0;
		pFontData->mbSubsettable = TRUE;
		pFontData->mbEmbeddable = FALSE;

		// Add to list
		pList->Add( pFontData );
	}
}

// -----------------------------------------------------------------------

BOOL SalGraphics::GetGlyphBoundRect( long nIndex, Rectangle& rRect,
                                     const OutputDevice *pOutDev )

{
	if ( maGraphicsData.mpVCLFont )
	{
		rRect = maGraphicsData.mpVCLGraphics->getGlyphBounds( nIndex, maGraphicsData.mpVCLFont );
		return TRUE;
	}
	else
	{
		rRect.SetEmpty();
		return FALSE;
	}
}

// -----------------------------------------------------------------------

BOOL SalGraphics::GetGlyphOutline( long nIndex, PolyPolygon& rPolyPoly,
                                   const OutputDevice *pOutDev )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::GetGlyphOutline not implemented\n" );
#endif
	rPolyPoly.Clear();
	return FALSE;
}

// -----------------------------------------------------------------------

void SalGraphics::GetDevFontSubstList( OutputDevice* pOutDev )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::GetDevFontSubstList not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

ImplFontData* SalGraphics::AddTempDevFont( const String& rFontFileURL,
                                           const String& rFontName )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::AddTempDevFont not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

void SalGraphics::RemovingFont( ImplFontData* )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::RemovingFont not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

BOOL SalGraphics::CreateFontSubset( const rtl::OUString& rToFile,
                                    ImplFontData* pFont, long* pGlyphIDs,
                                    sal_uInt8* pEncoding, sal_Int32* pWidths,
                                    int nGlyphs, FontSubsetInfo& rInfo )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::CreateFontSubset not implemented\n" );
#endif
	return FALSE;
}

// -----------------------------------------------------------------------

const void* SalGraphics::GetEmbedFontData( ImplFontData* pFont,
                                           const sal_Unicode* pUnicodes,
                                           sal_Int32* pWidths,
                                           FontSubsetInfo& rInfo,
                                           long* pDataLen )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::GetEmbedFontData not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

void SalGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::FreeEmbedFontData not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

const std::map< sal_Unicode, sal_Int32 >* SalGraphics::GetFontEncodingVector(
                ImplFontData* pFont,
                const std::map< sal_Unicode, rtl::OString >** pNonEncoded )
{
#ifdef DEBUG
	fprintf( stderr, "SalGraphics::GetFontEncodingVector not implemented\n" );
#endif
	if ( pNonEncoded )
		*pNonEncoded = NULL;
	return NULL;
}
