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

#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

class ATSLayout : public GenericSalLayout
{
	::vcl::com_sun_star_vcl_VCLFont*	mpVCLFont;
	ATSUStyle			maFontStyle;
	int					mnGlyphCount;
	long*				mpGlyphTransforms;

public:
						ATSLayout( ::vcl::com_sun_star_vcl_VCLFont *pVCLFont );
	virtual				~ATSLayout();

	virtual bool		LayoutText( ImplLayoutArgs& rArgs );
	virtual void		DrawText( SalGraphics& rGraphics ) const;
};

using namespace vcl;

// ============================================================================

SalLayout *SalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
	return new ATSLayout( maGraphicsData.mpVCLFont );
}

// ============================================================================

ATSLayout::ATSLayout( com_sun_star_vcl_VCLFont *pVCLFont ) :
	maFontStyle( NULL ),
	mnGlyphCount( 0 ),
	mpGlyphTransforms( NULL )
{
	mpVCLFont = new com_sun_star_vcl_VCLFont( pVCLFont->getJavaObject() );

	// Create font style
	if ( ATSUCreateStyle( &maFontStyle ) == noErr )
	{
		ATSUAttributeTag nTags[3];
		ByteCount nBytes[3];
		ATSUAttributeValuePtr nVals[3];

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

		if ( ATSUSetAttributes( maFontStyle, 3, nTags, nBytes, nVals ) != noErr )
		{
			ATSUDisposeStyle( maFontStyle );
			maFontStyle = NULL;
		}
	}
}

// ----------------------------------------------------------------------------

ATSLayout::~ATSLayout()
{
	if ( mpVCLFont )
		delete mpVCLFont;

	if ( maFontStyle )
		ATSUDisposeStyle( maFontStyle );

	if ( mpGlyphTransforms )
		rtl_freeMemory( mpGlyphTransforms );
}

// ----------------------------------------------------------------------------

bool ATSLayout::LayoutText( ImplLayoutArgs& rArgs )
{
	if ( !maFontStyle )
		return false;

	bool bRTL = ( rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG && rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL );

	// Create a copy of the string so that we can perform mirroring
	sal_Unicode aStr[ rArgs.mnLength ];

	// Copy characters
	int nRunStart;
	int nRunEnd;
	bool bRunRTL;
	rArgs.ResetPos();
	while ( rArgs.GetNextRun( &nRunStart, &nRunEnd, &bRunRTL ) )
	{
		for ( int i = nRunStart; i < nRunEnd; i++ )
		{
			int j = i - rArgs.mnMinCharPos;
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
	if ( ATSUCreateTextLayoutWithTextPtr( aStr, kATSUFromTextBeginning, kATSUToTextEnd, rArgs.mnLength, 1, (const UniCharCount *)&rArgs.mnLength, &maFontStyle, &aLayout ) != noErr )
		return false;

	MacOSBoolean nDirection;
	if ( bRTL )
		nDirection = kATSURightToLeftBaseDirection;
	else
		nDirection = kATSULeftToRightBaseDirection;
	ATSUAttributeTag nTag = kATSULineDirectionTag;
	ByteCount nBytes = sizeof( MacOSBoolean );
	ATSUAttributeValuePtr nVal = &nDirection;
	if ( ATSUSetLayoutControls( aLayout, 1, &nTag, &nBytes, &nVal ) != noErr )
	{
		ATSUDisposeTextLayout( aLayout );
		return false;
	}

	ByteCount nBufSize;
	if ( ATSUGetGlyphInfo( aLayout, kATSUFromTextBeginning, kATSUToTextEnd, &nBufSize, NULL ) != noErr )
	{
		ATSUDisposeTextLayout( aLayout );
		return false;
	}

	ATSUGlyphInfoArray *pGlyphInfoArray = (ATSUGlyphInfoArray *)rtl_allocateMemory( nBufSize );

	ByteCount nRetSize = nBufSize;
	if ( ATSUGetGlyphInfo( aLayout, kATSUFromTextBeginning, kATSUToTextEnd, &nRetSize, pGlyphInfoArray ) != noErr || nRetSize != nBufSize )
	{
		rtl_freeMemory( pGlyphInfoArray );
		ATSUDisposeTextLayout( aLayout );
		return false;
	}

	// Cache number of glyphs and the glyph tranforms
	mnGlyphCount = pGlyphInfoArray->numGlyphs;
	nBufSize = rArgs.mnLength * sizeof( int );
	if ( mpGlyphTransforms )
		rtl_freeMemory( mpGlyphTransforms );
	nBufSize = rArgs.mnLength * 2 * sizeof( long );
	mpGlyphTransforms = (long *)rtl_allocateMemory( nBufSize );
	memset( mpGlyphTransforms, 0, nBufSize );

	// Calculate and cache glyph advances
	bool bPosRTL;
	bool bVertical = ( rArgs.mnFlags & SAL_LAYOUT_VERTICAL );
	long nAscent = 0;
	long nDescent = 0;
	long nLeading = 0;
	long nAdjust = 0;
	if ( bVertical )
	{
		nAscent = mpVCLFont->getAscent();
		nDescent = mpVCLFont->getDescent();
		nLeading = mpVCLFont->getLeading();
		nAdjust = nLeading / 2;
	}
	Point aPos( 0, 0 );
	rArgs.ResetPos();
	int nCharPos = -1;
	while ( rArgs.GetNextPos( &nCharPos, &bPosRTL ) )
	{
		for ( int i = 0; i < mnGlyphCount; i++ )
		{
			if ( nCharPos != pGlyphInfoArray->glyphs[ i ].charIndex + rArgs.mnMinCharPos )
				continue;

			int nGlyph = pGlyphInfoArray->glyphs[ i ].glyphID;
			long nCharWidth = 0;
			ATSGlyphScreenMetrics aScreenMetrics;
			if ( ATSUGlyphGetScreenMetrics( pGlyphInfoArray->glyphs[ i ].style, 1, &pGlyphInfoArray->glyphs[ i ].glyphID, sizeof( GlyphID ), true, true, &aScreenMetrics ) == noErr )
			{
				if ( bVertical )
					nGlyph |= GetVerticalFlags( aStr[ pGlyphInfoArray->glyphs[ i ].charIndex ] );

				if ( nGlyph & ( GF_ROTL | GF_ROTR ) )
				{
					nCharWidth = (long)aScreenMetrics.height + nLeading;

					int j = nCharPos * 2;
					if ( nGlyph & GF_ROTL )
					{
						mpGlyphTransforms[ j ] = nDescent - ( ( nAscent + nDescent - (long)aScreenMetrics.width ) / 2 );
						mpGlyphTransforms[ j + 1 ] = (long)aScreenMetrics.topLeft.y + nAdjust;
					}
					else
					{
						mpGlyphTransforms[ j ] = ( ( nAscent + nDescent - (long)aScreenMetrics.width ) / 2 ) - nAscent;
						mpGlyphTransforms[ j + 1 ] = nCharWidth - (long)aScreenMetrics.topLeft.y - nAdjust;
					}
				}
				else
				{
					nCharWidth = (long)aScreenMetrics.deviceAdvance.x;
				}
			}

			int nGlyphFlags = ( nCharWidth > 0 ) ? 0 : GlyphItem::IS_IN_CLUSTER;
			if ( bPosRTL )
				nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;

			GlyphItem aGI( nCharPos, nGlyph, aPos, nGlyphFlags, nCharWidth );
			aGI.mnNewWidth = nCharWidth;
			AppendGlyph( aGI );

			aPos.X() += nCharWidth;
		}
	}
	SortGlyphItems();

	rtl_freeMemory( pGlyphInfoArray );
	ATSUDisposeTextLayout( aLayout );

	return ( nCharPos >= 0 );
}

// ----------------------------------------------------------------------------

void ATSLayout::DrawText( SalGraphics& rGraphics ) const
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

		int j = aCharPosArray[ 0 ] * 2;
		if ( aGlyphArray[ 0 ] & ( GF_ROTL | GF_ROTR ) )
		{
			// Draw rotated glyphs one at a time. Note that we rotated the
			// coordinates when the glyphs were added.
			int nGlyphRotation;
			if ( aGlyphArray[ 0 ] & GF_ROTL )
				nGlyphRotation = 900;
			else
				nGlyphRotation = -900;

			aGlyphArray[ 0 ] &= GF_IDXMASK;

			rGraphics.maGraphicsData.mpVCLGraphics->drawGlyphs( aPos.X(), aPos.Y(), 1, aGlyphArray, aDXArray, mpVCLFont, rGraphics.maGraphicsData.mnTextColor, nOrientation + nGlyphRotation, mpGlyphTransforms[ j ], mpGlyphTransforms[ j + 1 ]);

			nStart -= nGlyphCount - 1;
			continue;
		}
		else
		{
			for ( int i = 0; i < nGlyphCount; i++ )
				aGlyphArray[ i ] &= GF_IDXMASK;

			rGraphics.maGraphicsData.mpVCLGraphics->drawGlyphs( aPos.X(), aPos.Y(), nGlyphCount, aGlyphArray, aDXArray, mpVCLFont, rGraphics.maGraphicsData.mnTextColor, nOrientation, mpGlyphTransforms[ j ], mpGlyphTransforms[ j + 1 ] );
		}
	}
}
