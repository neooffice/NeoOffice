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

#include <list>

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

#define UNITS_PER_PIXEL 1024

inline int Float32ToLong( Float32 f ) { return (long)( f + 0.5 ); }

using namespace osl;
using namespace rtl;
using namespace vcl;

// ============================================================================

static OSStatus SalATSCubicMoveToCallback( const Float32Point *pPoint, void *pData )
{
	::std::list< Polygon > *pPolygonList = (::std::list< Polygon > *)pData;

	Point aPoint( Float32ToLong( pPoint->x * UNITS_PER_PIXEL ), Float32ToLong( pPoint->y * UNITS_PER_PIXEL ) );
	pPolygonList->push_back( Polygon( 1, &aPoint ) );

	return noErr;
}

// ----------------------------------------------------------------------------

static OSStatus SalATSCubicLineToCallback( const Float32Point *pPoint, void *pData )
{
	::std::list< Polygon > *pPolygonList = (::std::list< Polygon > *)pData;

	pPolygonList->back().Insert( pPolygonList->back().GetSize(), Point( Float32ToLong( pPoint->x * UNITS_PER_PIXEL ), Float32ToLong( pPoint->y * UNITS_PER_PIXEL ) ) );

	return noErr;
}

// ----------------------------------------------------------------------------

static OSStatus SalATSCubicCurveToCallback( const Float32Point *pStart, const Float32Point *pOffCurve, const Float32Point *pEnd, void *pData )
{
	::std::list< Polygon > *pPolygonList = (::std::list< Polygon > *)pData;

	Point aStart( Float32ToLong( pStart->x * UNITS_PER_PIXEL ), Float32ToLong( pStart->y * UNITS_PER_PIXEL ) );
	Point aOffCurve( Float32ToLong( pOffCurve->x * UNITS_PER_PIXEL ), Float32ToLong( pOffCurve->y * UNITS_PER_PIXEL ) );
	Point aEnd( Float32ToLong( pEnd->x * UNITS_PER_PIXEL ), Float32ToLong( pEnd->y * UNITS_PER_PIXEL ) );

	USHORT nSize = pPolygonList->back().GetSize();
	pPolygonList->back().Insert( nSize++, aStart, POLY_CONTROL );
	pPolygonList->back().Insert( nSize++, aOffCurve, POLY_CONTROL );
	pPolygonList->back().Insert( nSize, aEnd );

	return noErr;
}

// ----------------------------------------------------------------------------

static OSStatus SalATSCubicClosePathCallback( void *pData )
{
	::std::list< Polygon > *pPolygonList = (::std::list< Polygon > *)pData;

	USHORT nSize = pPolygonList->back().GetSize();
	if ( nSize > 1 )
		pPolygonList->back().Insert( nSize, Point( pPolygonList->back().GetPoint( 0 ) ) );

	return noErr;
}

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
	mnGlyphCount( 0 ),
	mpGlyphInfoArray( NULL ),
	mpCharsToGlyphs( NULL ),
	maVerticalFontStyle( NULL )
{
	SetUnitsPerPixel( UNITS_PER_PIXEL );

	if ( mnFallbackLevel )
	{
		::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator it = mpGraphics->maGraphicsData.maFallbackFonts.find( mnFallbackLevel );
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

	if ( mpCharsToGlyphs )
		rtl_freeMemory( mpCharsToGlyphs );
	mpCharsToGlyphs = NULL;

	if ( maVerticalFontStyle )
		ATSUDisposeStyle( maVerticalFontStyle );
	maVerticalFontStyle = NULL;
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

	if ( ! ( rArgs.mnFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING ) )
	{
		ATSUAttributeTag nTags[2];
		ByteCount nBytes[2];
		ATSUAttributeValuePtr nVals[2];

		if ( rArgs.mnFlags & SAL_LAYOUT_VERTICAL )
		{
			if ( ATSUCreateAndCopyStyle( maFontStyle, &maVerticalFontStyle ) == noErr )
			{
				ATSUVerticalCharacterType nVertical;
				if ( maVerticalFontStyle )
					nVertical = kATSUStronglyVertical;
				else
					nVertical = kATSUStronglyHorizontal;
				nTags[0] = kATSUVerticalCharacterTag;
				nBytes[0] = sizeof( ATSUVerticalCharacterType );
				nVals[0] = &nVertical;

				if ( ATSUSetAttributes( maVerticalFontStyle, 1, nTags, nBytes, nVals ) != noErr )
				{
					ATSUDisposeStyle( maVerticalFontStyle );
					maVerticalFontStyle = NULL;
					return false;
				}
			}
		}

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
			}
		}

		ATSUTextLayout aLayout;
		if ( ATSUCreateTextLayoutWithTextPtr( aStr, kATSUFromTextBeginning, kATSUToTextEnd, nLen, 1, (const UniCharCount *)&nLen, &maFontStyle, &aLayout ) != noErr )
		{
			Destroy();
			return false;
		}

		if ( maVerticalFontStyle )
		{
			for ( int i = 0; i < nLen; i++ )
			{
				if ( GetVerticalFlags( aStr[ i ] ) & GF_ROTMASK && ATSUSetRunStyle( aLayout, maVerticalFontStyle, i, 1 ) != noErr )
				{
					ATSUDisposeTextLayout( aLayout );
					Destroy();
					return false;
				}
			}
		}

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
					::std::map< void*, ImplFontData* >::const_iterator it = pSalData->maNativeFontMapping.find( (void *)nFontID );
					if ( it != pSalData->maNativeFontMapping.end() )
					{
						int nNextLevel = mnFallbackLevel + 1;
						com_sun_star_vcl_VCLFont *pVCLFont = (com_sun_star_vcl_VCLFont *)it->second->mpSysData;
						::std::map< int, com_sun_star_vcl_VCLFont* >::const_iterator ffit = mpGraphics->maGraphicsData.maFallbackFonts.find( nNextLevel );
						if ( ffit != mpGraphics->maGraphicsData.maFallbackFonts.end() )
							delete ffit->second;
						mpGraphics->maGraphicsData.maFallbackFonts[ nNextLevel ] = pVCLFont->deriveFont( mpVCLFont->getSize(), mpVCLFont->isBold(), mpVCLFont->isItalic(), mpVCLFont->getOrientation(), mpVCLFont->isAntialiased(), mpVCLFont->isVertical(), mpVCLFont->getScaleX() );

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
			int nLastIndex = 0;
			rArgs.ResetPos();
			while ( rArgs.GetNextPos( &nCharPos, &bPosRTL ) )
			{
				int nIndex = nCharPos - rArgs.mnMinCharPos + 1;
				if ( pNeedFallback[ nIndex ] )
				{
					rArgs.NeedFallback( nCharPos, bPosRTL );
				}
				else if ( IsSpacingGlyph( aStr[ nIndex ] | GF_ISCHAR ) && pNeedFallback[ nLastIndex ] )
				{
					rArgs.NeedFallback( nCharPos, bPosRTL );
					pNeedFallback[ nIndex ] = true;
				}

				nLastIndex = nIndex;
			}

			rtl_freeMemory( pNeedFallback );
		}

		if ( !bFontSet )
			rArgs.mnFlags |= SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
	}

	if ( !mnGlyphCount || !mpGlyphInfoArray || !mpCharsToGlyphs )
	{
		Destroy();
		return false;
	}

	long nAscent = maVerticalFontStyle ? mpVCLFont->getAscent() * mnUnitsPerPixel : 0;
	long nBaselineDelta = 0;

	if ( maVerticalFontStyle )
	{
		BslnBaselineRecord aBaseline;
		memset( aBaseline, 0, sizeof( BslnBaselineRecord ) );
		if ( ATSUCalculateBaselineDeltas( maVerticalFontStyle, kBSLNRomanBaseline, aBaseline ) == noErr )
			nBaselineDelta = Float32ToLong( Fix2X( aBaseline[ kBSLNIdeographicCenterBaseline ] ) * mnUnitsPerPixel );
		if ( !nBaselineDelta )
			nBaselineDelta = ( ( ( mpVCLFont->getDescent() + mpVCLFont->getAscent() ) / 2 ) - mpVCLFont->getDescent() ) * mnUnitsPerPixel;
	}

	// Calculate and cache glyph advances
	double fUnitsPerPixel = mpVCLFont->getScaleX() * mnUnitsPerPixel;
	bool bPosRTL;
	Point aPos( 0, 0 );
	int nCharPos = -1;
	Float32 fCurrentWidth = 0;
	rArgs.ResetPos();
	while ( rArgs.GetNextPos( &nCharPos, &bPosRTL ) )
	{
		Float32 fHeight = 0;
		int nIndex = nCharPos - rArgs.mnMinCharPos + 1;
		for ( int i = mpCharsToGlyphs[ nIndex ]; i < mnGlyphCount && mpGlyphInfoArray->glyphs[ i ].charIndex == nIndex; i++ )
		{
			long nGlyph = mpGlyphInfoArray->glyphs[ i ].glyphID;
			long nCharWidth = 0;

			if ( maVerticalFontStyle )
				nGlyph |= GetVerticalFlags( rArgs.mpStr[ nCharPos ] );

			ATSTrapezoid aTrapezoid;
			if ( ATSUGetGlyphBounds( mpGlyphInfoArray->layout, 0, 0, nIndex, 1, kATSUseFractionalOrigins, 1, &aTrapezoid, NULL ) == noErr )
			{
				if ( nGlyph & GF_ROTMASK )
				{
					fCurrentWidth += Fix2X( aTrapezoid.lowerLeft.y - aTrapezoid.upperLeft.y ) * fUnitsPerPixel;
					aPos.Y() = nBaselineDelta - Float32ToLong( Fix2X( aTrapezoid.upperRight.x - aTrapezoid.upperLeft.x ) * mnUnitsPerPixel / 2 );
				}
				else
				{
					fCurrentWidth += Fix2X( aTrapezoid.upperRight.x - aTrapezoid.upperLeft.x ) * fUnitsPerPixel;
				}
			}

			nCharWidth = Float32ToLong( fCurrentWidth ) - aPos.X();
			if ( nCharWidth < 0 )
				nCharWidth = 0;

			// Mark whitespace glyphs
			if ( mpGlyphInfoArray->glyphs[ i ].glyphID == 0xffff || IsSpacingGlyph( rArgs.mpStr[ nCharPos ] | GF_ISCHAR ) || mpGlyphInfoArray->glyphs[ i ].layoutFlags & kATSGlyphInfoTerminatorGlyph )
				nGlyph = 0x0020 | GF_ISCHAR;

			int nGlyphFlags = nCharWidth ? 0 : GlyphItem::IS_IN_CLUSTER;
			if ( bPosRTL )
				nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;

			AppendGlyph( GlyphItem( nCharPos, nGlyph, aPos, nGlyphFlags, nCharWidth ) );

			aPos.X() += nCharWidth;
			aPos.Y() = 0;
			break;
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
	long nAscent = maVerticalFontStyle ? mpVCLFont->getAscent() : 0;

	Point aPos;
	for ( int nStart = 0; ; )
	{
		int nGlyphCount = GetNextGlyphs( nMaxGlyphs, aGlyphArray, aPos, nStart, aDXArray, NULL );

		if ( !nGlyphCount )
			break;

		int i;
		for ( i = 0; i < nGlyphCount && IsSpacingGlyph( aGlyphArray[ i ] ); i++ )
			;
		if ( i )
		{
			nStart -= nGlyphCount - i;
			continue;
		}

		for ( i = 0; i < nGlyphCount && !IsSpacingGlyph( aGlyphArray[ i ] ); i++ )
			;
		nGlyphCount = i;

		if ( !nGlyphCount )
			break;

		long nGlyphOrientation = aGlyphArray[ 0 ] & GF_ROTMASK;
		if ( nGlyphOrientation )
		{
			nStart -= nGlyphCount - 1;
			nGlyphCount = 1;
		}

		for ( i = 0; i < nGlyphCount; i++ )
			aGlyphArray[ i ] &= GF_IDXMASK;

		rGraphics.maGraphicsData.mpVCLGraphics->drawGlyphs( aPos.X(), aPos.Y(), nGlyphCount, aGlyphArray, aDXArray, mpVCLFont, rGraphics.maGraphicsData.mnTextColor, GetOrientation(), mnUnitsPerPixel, nGlyphOrientation );
	}
}

// ----------------------------------------------------------------------------

bool SalATSLayout::GetOutline( SalGraphics& rGraphics, PolyPolyVector& rVector ) const
{
	bool bRet = false;

	if ( !mnGlyphCount )
		return bRet;

	int nMaxGlyphs( 1 );
	long aGlyphArray[ nMaxGlyphs ];
	long aDXArray[ nMaxGlyphs ];
	int aCharPosArray[ nMaxGlyphs ];
	long nAscent = maVerticalFontStyle ? mpVCLFont->getAscent() * mnUnitsPerPixel : 0;
	long nBaselineDelta = 0;

	if ( maVerticalFontStyle )
	{
		BslnBaselineRecord aBaseline;
		memset( aBaseline, 0, sizeof( BslnBaselineRecord ) );
		if ( ATSUCalculateBaselineDeltas( maVerticalFontStyle, kBSLNRomanBaseline, aBaseline ) == noErr )
			nBaselineDelta = Float32ToLong( Fix2X( aBaseline[ kBSLNIdeographicCenterBaseline ] ) * mnUnitsPerPixel );
		if ( !nBaselineDelta )
			nBaselineDelta = ( ( ( mpVCLFont->getDescent() + mpVCLFont->getAscent() ) / 2 ) - mpVCLFont->getDescent() ) * mnUnitsPerPixel;
	}

	Point aPos;
	for ( int nStart = 0; ; )
	{
		int nGlyphCount = GetNextGlyphs( nMaxGlyphs, aGlyphArray, aPos, nStart, aDXArray, aCharPosArray );

		if ( !nGlyphCount )
			break;

		if ( IsSpacingGlyph( aGlyphArray[ 0 ] ) )
		{
			bRet = true;
			continue;
		}

		int nIndex = aCharPosArray[ 0 ] - mnMinCharPos + 1;
		for ( int i = mpCharsToGlyphs[ nIndex ]; i < mnGlyphCount && mpGlyphInfoArray->glyphs[ i ].charIndex == nIndex; i++ )
		{
			long nGlyph = mpGlyphInfoArray->glyphs[ i ].glyphID;
			if ( ( aGlyphArray[ 0 ] & GF_IDXMASK ) != nGlyph )
				continue;

			::std::list< Polygon > aPolygonList;
			OSStatus nErr;
			if ( ATSUGlyphGetCubicPaths( mpGlyphInfoArray->glyphs[ i ].style, mpGlyphInfoArray->glyphs[ i ].glyphID, SalATSCubicMoveToCallback, SalATSCubicLineToCallback, SalATSCubicCurveToCallback, SalATSCubicClosePathCallback, (void *)&aPolygonList, &nErr ) != noErr )
				continue;

			PolyPolygon aPolyPolygon;
			while ( aPolygonList.size() )
			{
				aPolyPolygon.Insert( aPolygonList.front() );
				aPolygonList.pop_front();
			}

			aPolyPolygon.Move( aPos.X() * mnUnitsPerPixel, aPos.Y() * mnUnitsPerPixel );

			if ( maVerticalFontStyle )
			{
				long nGlyphOrientation = aGlyphArray[ 0 ] & GF_ROTMASK;
				if ( nGlyphOrientation )
				{
					ATSGlyphScreenMetrics aScreenMetrics;
					if ( ATSUGlyphGetScreenMetrics( mpGlyphInfoArray->glyphs[ i ].style, 1, &mpGlyphInfoArray->glyphs[ i ].glyphID, sizeof( GlyphID ), false, false, &aScreenMetrics ) != noErr )
						continue;

					if ( nGlyphOrientation == GF_ROTL )
					{
						aPolyPolygon.Rotate( Point( 0, 0 ), 900 );
						aPolyPolygon.Move( ( Float32ToLong( aScreenMetrics.sideBearing.y * mnUnitsPerPixel ) + aPolyPolygon.GetBoundRect().nLeft ) * -1, nBaselineDelta * -1 );
					}
					else
					{
						aPolyPolygon.Rotate( Point( 0, 0 ), 2700 );
						aPolyPolygon.Move( aDXArray[ 0 ] - Float32ToLong( aScreenMetrics.otherSideBearing.y * mnUnitsPerPixel ) - aPolyPolygon.GetBoundRect().nRight, nBaselineDelta * -1 );
					}
				}
			}

			rVector.push_back( aPolyPolygon );
			bRet = true;
			break;
		}
	}

	return bRet;
}
