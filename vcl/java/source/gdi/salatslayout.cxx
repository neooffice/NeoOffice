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
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
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
	return new SalATSLayout( this, nFallbackLevel );
}

// ============================================================================

SalATSLayout::SalATSLayout( SalGraphics *pGraphics, int nFallbackLevel ) :
	mpGraphics( pGraphics ),
	mnFallbackLevel( nFallbackLevel ),
	mpVCLFont( NULL ),
	maFontStyle( NULL ),
	mbUseScreenMetrics( !pGraphics->maGraphicsData.mpPrinter ),
	mnGlyphCount( 0 ),
	mpGlyphInfoArray( NULL ),
	mpGlyphTranslations( NULL ),
	mpCharsToGlyphs( NULL ),
	mpVerticalFlags( NULL )
{
	SetUnitsPerPixel( 1000 );

	if ( mnFallbackLevel )
	{
		::std::map< int, com_sun_star_vcl_VCLFont* >::iterator it = mpGraphics->maGraphicsData.maFallbackFonts.find( mnFallbackLevel );
		if ( it != mpGraphics->maGraphicsData.maFallbackFonts.end() )
			mpVCLFont = new com_sun_star_vcl_VCLFont( it->second->getJavaObject() );
	}
	else
	{
		mpVCLFont = new com_sun_star_vcl_VCLFont( mpGraphics->maGraphicsData.mpVCLFont->getJavaObject() );
	}

	// Create font style
	if ( mpVCLFont && ATSUCreateStyle( &maFontStyle ) == noErr )
	{
		ATSUAttributeTag nTags[3];
		ByteCount nBytes[3];
		ATSUAttributeValuePtr nVals[3];

		// Set font
		ATSUFontID nFontID = (ATSUFontID)mpVCLFont->getNativeFont();
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
	{
		ATSUDisposeTextLayout( mpGlyphInfoArray->layout );
		rtl_freeMemory( mpGlyphInfoArray );
	}
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

		// Find positions that require fallback fonts
		bool *pNeedFallback = NULL;
		UniCharArrayOffset nCurrentPos = 0;
		UniCharCount nOffset;
		ATSUFontID nFontID;
		bool bFontSet = false;
		for ( ; ; )
		{
			OSStatus nErr = ATSUMatchFontsToText( aLayout, nCurrentPos, kATSUToTextEnd, &nFontID, &nCurrentPos, &nOffset );
			if ( nErr == kATSUFontsNotMatched )
			{
				nCurrentPos += nOffset;
			}
			else if ( nErr == kATSUFontsMatched )
			{
				if ( !pNeedFallback )
				{
					nBufSize = nLen * sizeof( bool );
					pNeedFallback = (bool *)rtl_allocateMemory( nBufSize );
					memset( pNeedFallback, 0, nBufSize );
				}

				int nOffsetPos = nCurrentPos + nOffset;
				for ( ; nCurrentPos < nOffsetPos; nCurrentPos++ )
					pNeedFallback[ nCurrentPos ] = true;

				// Update font for next pass through
				if ( !bFontSet )
				{
					SalData *pSalData = GetSalData();
					::std::map< void*, ImplFontData* >::iterator it = pSalData->maNativeFontMapping.find( (void *)nFontID );
					if ( it != pSalData->maNativeFontMapping.end() )
					{
						int nNextLevel = mnFallbackLevel + 1;
						com_sun_star_vcl_VCLFont *pVCLFont = (com_sun_star_vcl_VCLFont *)it->second->mpSysData;
						::std::map< int, com_sun_star_vcl_VCLFont* >::iterator ffit = mpGraphics->maGraphicsData.maFallbackFonts.find( nNextLevel );
						if ( ffit != mpGraphics->maGraphicsData.maFallbackFonts.end() )
							delete ffit->second;
						mpGraphics->maGraphicsData.maFallbackFonts[ nNextLevel ] = pVCLFont->deriveFont( mpVCLFont->getSize(), mpVCLFont->isBold(), mpVCLFont->isItalic(), mpVCLFont->getOrientation(), mpVCLFont->isAntialiased(), mpVCLFont->isVertical() );

						bFontSet = true;
					}
				}
			}
			else
			{
				break;
			}
		}

		// Create fallback runs
		if ( pNeedFallback )
		{
			bool bPosRTL;
			int nCharPos = -1;
			rArgs.ResetPos();
			while ( rArgs.GetNextPos( &nCharPos, &bPosRTL ) )
			{
				int nIndex = nCharPos - rArgs.mnMinCharPos + 1;
				if ( pNeedFallback[ nIndex ] || IsSpacingGlyph( aStr[ nIndex ] | GF_ISCHAR ) )
					rArgs.NeedFallback( nCharPos, bPosRTL );
			}

			rtl_freeMemory( pNeedFallback );
		}

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
				if ( !mpGlyphInfoArray->glyphs[ i ].glyphID )
					continue;

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

		if ( !bFontSet )
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
	rArgs.ResetPos();
	while ( rArgs.GetNextPos( &nCharPos, &bPosRTL ) )
	{
		int nIndex = nCharPos - rArgs.mnMinCharPos + 1;
		for ( int i = mpCharsToGlyphs[ nIndex ]; i < mnGlyphCount && mpGlyphInfoArray->glyphs[ i ].charIndex == nIndex; i++ )
		{
			int nGlyph = mpGlyphInfoArray->glyphs[ i ].glyphID;
			long nCharWidth = 0;

			if ( mpGlyphTranslations && mpVerticalFlags && mpVerticalFlags[ mpGlyphInfoArray->glyphs[ i ].charIndex ] & GF_ROTMASK )
			{
				nGlyph |= mpVerticalFlags[ mpGlyphInfoArray->glyphs[ i ].charIndex ] & GF_ROTMASK;

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

			int nGlyphFlags = nCharWidth ? 0 : GlyphItem::IS_IN_CLUSTER;
			if ( bPosRTL )
			{
				nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;
				if ( ! ( nGlyphFlags & GlyphItem::IS_IN_CLUSTER ) )
				{
					AppendGlyph( GlyphItem( nCharPos, 3, aPos, nGlyphFlags, 0 ) );
					nGlyphFlags |= GlyphItem::IS_IN_CLUSTER;
				}
			}

			AppendGlyph( GlyphItem( nCharPos, nGlyph, aPos, nGlyphFlags, nCharWidth ) );

			aPos.X() += nCharWidth;
		}
	}

	return ( nCharPos >= 0 );
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

		if ( mpGlyphTranslations && aGlyphArray[ 0 ] & GF_ROTMASK )
		{
			// Draw rotated glyphs one at a time. Note that we rotated the
			// coordinates when the glyphs were added.
			int j = aCharPosArray[ 0 ] - mnMinCharPos + 1;
			int nGlyphRotation;
			if ( aGlyphArray[ 0 ] & GF_ROTL )
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
