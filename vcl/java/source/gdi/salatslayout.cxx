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
 *  Sun Microsystems Inc., December, 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Sun Microsystems, Inc.
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
 *  Modified July 2004 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Original source obtained from patch submitted to OpenOffice.org in issue
 *  23283 (see http://qa.openoffice.org/issues/show_bug.cgi?id=23283).
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

#ifndef _SV_SALATSLAYOUT_HXX
#include <salatslayout.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif

inline int Float32ToLong( Float32 f ) { return (long)( f + 0.5 ); }

using namespace osl;
using namespace rtl;
using namespace vcl;

// ============================================================================

SalLayout *SalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
	return new SalATSLayout( maGraphicsData.mpVCLFont, !maGraphicsData.mpPrinter );
}

// ============================================================================

SalATSLayout::SalATSLayout( com_sun_star_vcl_VCLFont *pVCLFont, bool bUseScreenMetrics ) :
	maFontStyle( NULL ),
	mbUseScreenMetrics( bUseScreenMetrics ),
	mnGlyphCount( 0 ),
	mpGlyphInfoArray( NULL ),
	mpGlyphTranslations( NULL ),
	mpCharsToGlyphs( NULL ),
	mpVerticalFlags( NULL )
{
	SetUnitsPerPixel( 1000 );

	mpVCLFont = new com_sun_star_vcl_VCLFont( pVCLFont->getJavaObject() );

	// Create font style
	if ( ATSUCreateStyle( &maFontStyle ) == noErr )
	{
		ATSUAttributeTag nTags[5];
		ByteCount nBytes[5];
		ATSUAttributeValuePtr nVals[5];

		// Set font
		ATSUFontID nFontID = (ATSUFontID)mpVCLFont->getNativeFont();
		if ( !nFontID )
		{
			// Fall back to Geneva as a last resort
			if ( ATSUFindFontFromName( "Geneva", 6, kFontFullName, kFontNoPlatformCode, kFontNoScriptCode, kFontNoLanguageCode, &nFontID ) != noErr )
				nFontID = kATSUInvalidFontID;
		}
		nTags[0] = kATSUFontTag;
		nBytes[0] = sizeof( ATSUFontID );
		nVals[0] = &nFontID;

		// Set font size
		Fixed nSize = Long2Fix( mpVCLFont->getSize() );
		nTags[1] = kATSUSizeTag;
		nBytes[1] = sizeof( Fixed );
		nVals[1] = &nSize;

		// Set antialiasing
		ATSStyleRenderingOptions nOptions;
		if ( mpVCLFont->isAntialiased() )
			nOptions = kATSStyleApplyAntiAliasing;
		else
			nOptions = kATSStyleNoAntiAliasing;
		nTags[2] = kATSUStyleRenderingOptionsTag;
		nBytes[2] = sizeof( ATSStyleRenderingOptions );
		nVals[2] = &nOptions;

		// Set bold
		MacOSBoolean bBold = mpVCLFont->isBold();
		nTags[3] = kATSUQDBoldfaceTag;
		nBytes[3] = sizeof( MacOSBoolean );
		nVals[3] = &bBold;

		// Set italic
		MacOSBoolean bItalic = mpVCLFont->isItalic();
		nTags[4] = kATSUQDItalicTag;
		nBytes[4] = sizeof( MacOSBoolean );
		nVals[4] = &bItalic;

		if ( ATSUSetAttributes( maFontStyle, 3, nTags, nBytes, nVals ) != noErr )
		{
			ATSUDisposeStyle( maFontStyle );
			maFontStyle = NULL;
		}
	}
}

// ----------------------------------------------------------------------------

SalATSLayout::~SalATSLayout()
{
	Destroy();

	if ( mpVCLFont )
		delete mpVCLFont;

	if ( maFontStyle )
		ATSUDisposeStyle( maFontStyle );
}

// ----------------------------------------------------------------------------

void SalATSLayout::Destroy()
{
	mnGlyphCount = 0;

	if ( mpGlyphInfoArray )
		rtl_freeMemory( mpGlyphInfoArray );
	mpGlyphInfoArray = NULL;

	if ( mpGlyphTranslations )
		rtl_freeMemory( mpGlyphTranslations );
	mpGlyphTranslations = NULL;

	if ( mpCharsToGlyphs )
		rtl_freeMemory( mpCharsToGlyphs );
	mpCharsToGlyphs = NULL;

	if ( mpVerticalFlags )
		rtl_freeMemory( mpVerticalFlags );
	mpVerticalFlags = NULL;
}

// ----------------------------------------------------------------------------

bool SalATSLayout::LayoutText( ImplLayoutArgs& rArgs )
{
	if ( !maFontStyle )
		return false;

	int nLen = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
	if ( !nLen )
		return false;

	bool bRTL = ( rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG && rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL );
	bool bVertical = ( rArgs.mnFlags & SAL_LAYOUT_VERTICAL );
	long nAscent = bVertical ? mpVCLFont->getAscent() : 0;
	long nDescent = bVertical ? mpVCLFont->getDescent() : 0;
	long nAdjust = bVertical ? Float32ToLong( ( nAscent + nDescent ) * 0.05 ) : 0;
	long nDoubleAdjust = bVertical ? nAdjust * 2 : 0;

	if ( ! ( rArgs.mnFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING ) )
	{
		// Create a copy of the string so that we can perform mirroring. Note
		// that we add the leading and/or trailing characters if this is a
		// substring to ensure that we get the correct layout of glyphs.
		nLen += 2;
		sal_Unicode aStr[ nLen ];

		if ( rArgs.mnMinCharPos )
			aStr[ 0 ] = rArgs.mpStr[ rArgs.mnMinCharPos - 1 ];
		else
			aStr[ 0 ] = 0x0020;

		if ( rArgs.mnEndCharPos < rArgs.mnLength )
			aStr[ nLen - 1 ] = rArgs.mpStr[ rArgs.mnEndCharPos ];
		else
			aStr[ nLen - 1 ] = 0x0020;

		// Copy characters
		int nRunStart;
		int nRunEnd;
		bool bRunRTL;
		while ( rArgs.GetNextRun( &nRunStart, &nRunEnd, &bRunRTL ) )
		{
			for ( int i = nRunStart; i < nRunEnd; i++ )
			{
				int j = i - rArgs.mnMinCharPos + 1;
				aStr[ j ] = rArgs.mpStr[ i ];

				// Mirror RTL characters
				if ( bRTL && bRunRTL )
				{
					sal_Unicode nChar = GetMirroredChar( aStr[ j ] );
					if ( nChar )
						aStr[ j ] = nChar;
				}
			}
		}

		ATSUTextLayout aLayout;
		if ( ATSUCreateTextLayoutWithTextPtr( aStr, kATSUFromTextBeginning, kATSUToTextEnd, nLen, 1, (const UniCharCount *)&nLen, &maFontStyle, &aLayout ) != noErr )
		{
			Destroy();
			return false;
		}

		ATSUAttributeTag nTags[2];
		ByteCount nBytes[2];
		ATSUAttributeValuePtr nVals[2];
		MacOSBoolean nDirection;
		if ( bRTL )
			nDirection = kATSURightToLeftBaseDirection;
		else
			nDirection = kATSULeftToRightBaseDirection;
		nTags[0] = kATSULineDirectionTag;
		nBytes[0] = sizeof( MacOSBoolean );
		nVals[0] = &nDirection;
		ATSLineLayoutOptions nOptions = kATSLineKeepSpacesOutOfMargin;
		nTags[1] = kATSULineLayoutOptionsTag;
		nBytes[1] = sizeof( ATSLineLayoutOptions );
		nVals[1] = &nOptions;

		if ( ATSUSetLayoutControls( aLayout, 2, nTags, nBytes, nVals ) != noErr )
		{
			ATSUDisposeTextLayout( aLayout );
			Destroy();
			return false;
		}

		ByteCount nBufSize;
		if ( ATSUGetGlyphInfo( aLayout, kATSUFromTextBeginning, kATSUToTextEnd, &nBufSize, NULL ) != noErr )
		{
			ATSUDisposeTextLayout( aLayout );
			Destroy();
			return false;
		}

		if ( mpGlyphInfoArray )
			rtl_freeMemory( mpGlyphInfoArray );
		mpGlyphInfoArray = (ATSUGlyphInfoArray *)rtl_allocateMemory( nBufSize );

		ByteCount nRetSize = nBufSize;
		if ( ATSUGetGlyphInfo( aLayout, kATSUFromTextBeginning, kATSUToTextEnd, &nRetSize, mpGlyphInfoArray ) != noErr || nRetSize != nBufSize )
		{
			ATSUDisposeTextLayout( aLayout );
			Destroy();
			return false;
		}

		mnGlyphCount = mpGlyphInfoArray->numGlyphs;

		ATSUDisposeTextLayout( aLayout );

		if ( bVertical )
		{
			// Cache vertical flags and glyph translations
			nBufSize = nLen * sizeof( int );
			if ( mpVerticalFlags )
				rtl_freeMemory( mpVerticalFlags );
			mpVerticalFlags = (int *)rtl_allocateMemory( nBufSize );

			nBufSize = nLen * 2 * sizeof( long );
			if ( mpGlyphTranslations )
				rtl_freeMemory( mpGlyphTranslations );
			mpGlyphTranslations = (long *)rtl_allocateMemory( nBufSize );
			memset( mpGlyphTranslations, 0, nBufSize );

			int i;
			int j;
			for ( i = 0, j = 0; i < nLen; i++, j += 2 )
			{
				mpVerticalFlags[ i ] = GetVerticalFlags( aStr[ i ] );

				if ( mpVerticalFlags[ i ] & GF_ROTL )
				{
					ATSGlyphScreenMetrics aScreenMetrics;
					if ( ATSUGlyphGetScreenMetrics( mpGlyphInfoArray->glyphs[ i ].style, 1, &mpGlyphInfoArray->glyphs[ i ].glyphID, sizeof( GlyphID ), true, true, &aScreenMetrics ) == noErr )
					{
						mpGlyphTranslations[ j ] = Float32ToLong( ( nAscent + nDescent - aScreenMetrics.deviceAdvance.x + aScreenMetrics.sideBearing.x + aScreenMetrics.otherSideBearing.x ) / 2 ) - nDescent;
						mpGlyphTranslations[ j + 1 ] = Float32ToLong( aScreenMetrics.topLeft.y ) + nAdjust;
					}
				}
				else if ( mpVerticalFlags[ i ] & GF_ROTR )
				{
					ATSGlyphScreenMetrics aScreenMetrics;
					if ( ATSUGlyphGetScreenMetrics( mpGlyphInfoArray->glyphs[ i ].style, 1, &mpGlyphInfoArray->glyphs[ i ].glyphID, sizeof( GlyphID ), true, true, &aScreenMetrics ) == noErr )
					{
						mpGlyphTranslations[ j ] = nAscent - Float32ToLong( ( nAscent + nDescent - aScreenMetrics.deviceAdvance.x + aScreenMetrics.sideBearing.x + aScreenMetrics.otherSideBearing.x ) / 2 );
						mpGlyphTranslations[ j + 1 ] = Float32ToLong( aScreenMetrics.height - aScreenMetrics.topLeft.y ) + nAdjust;
					}
				}
			}
		}

		// Cache mapping of characters to glyphs
		nBufSize = nLen * sizeof( int );
		if ( mpCharsToGlyphs )
			rtl_freeMemory( mpCharsToGlyphs );
		mpCharsToGlyphs = (int *)rtl_allocateMemory( nBufSize );

		int i;
		for ( i = 0; i < nLen; i++ )
			mpCharsToGlyphs[ i ] = -1;
		for ( i = 0; i < mnGlyphCount; i++ )
		{
			int nIndex = mpGlyphInfoArray->glyphs[ i ].charIndex;
			if ( mpCharsToGlyphs[ nIndex ] < 0 || i < mpCharsToGlyphs[ nIndex ] )
				mpCharsToGlyphs[ nIndex ] = i;
		}

		rArgs.mnFlags |= SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
	}

	if ( !mnGlyphCount || !mpGlyphInfoArray || !mpCharsToGlyphs )
	{
		Destroy();
		return false;
	}

	// Calculate and cache glyph advances
	bool bPosRTL;
	Point aPos( 0, 0 );
	int nCharPos = -1;
	while ( rArgs.GetNextPos( &nCharPos, &bPosRTL ) )
	{
		int nIndex = nCharPos - rArgs.mnMinCharPos + 1;
		for ( int i = mpCharsToGlyphs[ nIndex ]; i < mnGlyphCount && mpGlyphInfoArray->glyphs[ i ].charIndex == nIndex; i++ )
		{
			int nGlyph = mpGlyphInfoArray->glyphs[ i ].glyphID;
			long nCharWidth = 0;
			if ( mpGlyphTranslations && mpVerticalFlags && mpVerticalFlags[ mpGlyphInfoArray->glyphs[ i ].charIndex ] & ( GF_ROTL | GF_ROTR ) )
			{
				ATSGlyphScreenMetrics aScreenMetrics;
				if ( ATSUGlyphGetScreenMetrics( mpGlyphInfoArray->glyphs[ i ].style, 1, &mpGlyphInfoArray->glyphs[ i ].glyphID, sizeof( GlyphID ), true, true, &aScreenMetrics ) == noErr )
					nCharWidth = Float32ToLong( ( aScreenMetrics.height + nDoubleAdjust ) * mnUnitsPerPixel );
			}
			else if ( mbUseScreenMetrics )
			{
				nCharWidth = Float32ToLong( ( mpGlyphInfoArray->glyphs[ i + 1 ].screenX - mpGlyphInfoArray->glyphs[ i ].screenX ) * mnUnitsPerPixel );
			}
			else
			{
				nCharWidth = Float32ToLong( ( mpGlyphInfoArray->glyphs[ i + 1 ].idealX - mpGlyphInfoArray->glyphs[ i ].idealX ) * mnUnitsPerPixel );
			}

			if ( mpGlyphInfoArray->glyphs[ i ].layoutFlags & ( kATSGlyphInfoIsWhiteSpace | kATSGlyphInfoTerminatorGlyph ) )
				nGlyph = GF_IDXMASK;

			int nGlyphFlags = nCharWidth ? 0 : GlyphItem::IS_IN_CLUSTER;

			if ( bPosRTL )
				nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;

			GlyphItem aGI( nCharPos, nGlyph, aPos, nGlyphFlags, nCharWidth );
			aGI.mnNewWidth = nCharWidth;
				AppendGlyph( aGI );

			aPos.X() += nCharWidth;
		}
	}

	return ( nCharPos >= 0 );
}

// ----------------------------------------------------------------------------

void SalATSLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
	GenericSalLayout::AdjustLayout( rArgs );

	// Asian kerning
	if ( rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN && ! ( rArgs.mnFlags & SAL_LAYOUT_VERTICAL ) )
		ApplyAsianKerning( rArgs.mpStr, rArgs.mnLength );
}

// ----------------------------------------------------------------------------

void SalATSLayout::DrawText( SalGraphics& rGraphics ) const
{
	if ( !mnGlyphCount )
		return;

	int nMaxGlyphs( mnGlyphCount );
	long aGlyphArray[ nMaxGlyphs ];
	long aDXArray[ nMaxGlyphs ];
	int aCharPosArray[ nMaxGlyphs ];

	Point aPos;
	for ( int nStart = 0; ; )
	{
		int nGlyphCount = GetNextGlyphs( nMaxGlyphs, aGlyphArray, aPos, nStart, aDXArray, aCharPosArray );

		if ( !nGlyphCount )
			break;

		int nOrientation = GetOrientation();

		int j = aCharPosArray[ 0 ] - mnMinCharPos + 1;
		if ( mpGlyphTranslations && mpVerticalFlags && mpVerticalFlags[ j ] & ( GF_ROTL | GF_ROTR ) )
		{
			// Draw rotated glyphs one at a time. Note that we rotated the
			// coordinates when the glyphs were added.
			int nGlyphRotation;
			if ( mpVerticalFlags[ j ] & GF_ROTL )
				nGlyphRotation = 900;
			else
				nGlyphRotation = -900;

			aGlyphArray[ 0 ] &= GF_IDXMASK;

			int k = j * 2;
			rGraphics.maGraphicsData.mpVCLGraphics->drawGlyphs( aPos.X(), aPos.Y(), 1, aGlyphArray, aDXArray, mpVCLFont, rGraphics.maGraphicsData.mnTextColor, nOrientation + nGlyphRotation, mpGlyphTranslations[ k ], mpGlyphTranslations[ k + 1 ], mnUnitsPerPixel );

			nStart -= nGlyphCount - 1;
			continue;
		}
		else
		{
			for ( int i = 0; i < nGlyphCount; i++ )
				aGlyphArray[ i ] &= GF_IDXMASK;

			rGraphics.maGraphicsData.mpVCLGraphics->drawGlyphs( aPos.X(), aPos.Y(), nGlyphCount, aGlyphArray, aDXArray, mpVCLFont, rGraphics.maGraphicsData.mnTextColor, nOrientation, 0, 0, mnUnitsPerPixel );
		}
	}
}

// ----------------------------------------------------------------------------

int SalATSLayout::GetNextGlyphs( int nLen, long *pGlyphIdxAry, Point& rPos, int& rStart, long *pGlyphAdvAry, int *pCharPosAry ) const
{
	int nGlyphCount;

	while ( ( nGlyphCount = GenericSalLayout::GetNextGlyphs( nLen, pGlyphIdxAry, rPos, rStart, pGlyphAdvAry, pCharPosAry ) ) )
	{
		// Don't pass on GF_IDXMASK glyphs as we don't want the upper
		// layers to paint them
		for ( int i = 0; i < nGlyphCount && ( pGlyphIdxAry[ i ] & GF_IDXMASK ) == GF_IDXMASK; i++ )
			;

		if ( i )
		{
			rStart -= nGlyphCount - i;
			continue;
		}

		for ( i = 0; i < nGlyphCount && ( pGlyphIdxAry[ i ] & GF_IDXMASK ) != GF_IDXMASK; i++ )
			;

		if ( i )
		{
			rStart -= nGlyphCount - i;
			nGlyphCount = i;
			break;
		}
	}

	return nGlyphCount;
}
