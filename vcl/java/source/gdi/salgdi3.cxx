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
	if ( maGraphicsData.mpVCLFont )
		delete maGraphicsData.mpVCLFont;

	com_sun_star_vcl_VCLFont *pVCLFont = (com_sun_star_vcl_VCLFont *)pFont->mpFontData->mpSysData;
	OUString aName = pFont->maName;
	sal_Bool bBold = ( pFont->meWeight > WEIGHT_MEDIUM );
	sal_Bool bItalic = ( pFont->meItalic != ITALIC_NONE );

#ifdef MACOSX
	if ( bBold || bItalic || pFont->maName != pFont->maFoundName )
	{
		FMFont nFontID = (FMFont)((com_sun_star_vcl_VCLFont *)pFont->mpFontData->mpSysData)->getNativeFont( bBold, bItalic );
		ATSFontRef aFont = FMGetATSFontRefFromFont( nFontID );
		CFStringRef aFontNameRef;
		if ( aFont && ATSFontGetName( aFont, kATSOptionFlagsDefault, &aFontNameRef ) == noErr )
		{
			sal_Int32 nBufSize = CFStringGetLength( aFontNameRef );
			sal_Unicode aBuf[ nBufSize + 1 ];
			CFRange aRange;

			aRange.location = 0;
			aRange.length = nBufSize;
			CFStringGetCharacters( aFontNameRef, aRange, aBuf );
			aBuf[ nBufSize ] = 0;
			aName = OUString( aBuf );

			CFRelease( aFontNameRef );
		}
	}
#endif	// MACOSX

	if ( XubString( aName ) != pFont->maName )
	{
		SalData *pSalData = GetSalData();

		if ( !pSalData->mpFontList )
			pSalData->mpFontList = com_sun_star_vcl_VCLFont::getAllFonts();

		com_sun_star_vcl_VCLFontList *pFontList = GetSalData()->mpFontList;

		// Iterate through fonts and find a font a matching font
		for ( jsize i = 0; i < pFontList->mnCount; i++ )
		{
			if ( aName == pFontList->mpFonts[ i ]->getName() )
			{
				pVCLFont = pFontList->mpFonts[ i ];
				break;
			}
		}
	}

	pFont->maFoundName = aName;
	maGraphicsData.mpVCLFont = pVCLFont->deriveFont( pFont->mnHeight, bBold, bItalic, pFont->mnOrientation, !pFont->mbNonAntialiased, pFont->mbVertical );

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

	if ( !pSalData->mpFontList )
		pSalData->mpFontList = com_sun_star_vcl_VCLFont::getAllFonts();

	com_sun_star_vcl_VCLFontList *pFontList = GetSalData()->mpFontList;

	// Iterate through fonts and add each to the font list
	for ( jsize i = 0; i < pFontList->mnCount; i++ )
	{
		// Set default values
		ImplFontData *pFontData = new ImplFontData();
		pFontData->mpNext = NULL;
		pFontData->mpSysData = (void *)pFontList->mpFonts[ i ];
		pFontData->maName = XubString( pFontList->mpFonts[ i ]->getName() );
		pFontData->mnWidth = 0;
		pFontData->mnHeight = 0;
		pFontData->meFamily = pFontList->mpFonts[ i ]->getFamilyType();
		pFontData->meCharSet = RTL_TEXTENCODING_UNICODE;
		if ( pFontData->meFamily == FAMILY_MODERN )
			pFontData->mePitch = PITCH_FIXED;
		else
			pFontData->mePitch = PITCH_VARIABLE;
		pFontData->meWidthType = WIDTH_DONTKNOW;
		if ( pFontList->mpFonts[ i ]->isBold() )
			pFontData->meWeight = WEIGHT_BOLD;
		else
			pFontData->meWeight = WEIGHT_NORMAL;
		if ( pFontList->mpFonts[ i ]->isItalic() )
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
