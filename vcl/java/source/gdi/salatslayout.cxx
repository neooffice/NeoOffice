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
 *  Dan Williams, October 2003
 *  Patrick Luby, July 2004
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Dan Williams (danw@neooffice.org)
 *  Copyright 2004 by Patrick Luby (patrick.luby@planamesa.com)
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
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

inline int Float32ToInt( Float32 f ) { return (int)( f + 0.5 ); }

class ATSLayout : public SalLayout
{
	::vcl::com_sun_star_vcl_VCLFont*	mpVCLFont;
	ATSUTextLayout		maLayout;
	ItemCount			mnRuns;
	ATSUStyle*			mpStyles;
	UniCharArrayOffset	mnStart;
	UniCharCount		mnLen;
	mutable long		mnWidth;
	mutable long*		mpAdvances;
	mutable ATSUGlyphInfoArray*	mpGlyphInfoArray;

protected:
	void				Destroy();
	bool				InitAdvances() const;
	bool				InitGlyphInfoArray() const;

public:
						ATSLayout( ::vcl::com_sun_star_vcl_VCLFont *pVCLFont );
						~ATSLayout();

	virtual bool		LayoutText( ImplLayoutArgs& rArgs );
	virtual void		AdjustLayout( ImplLayoutArgs& rArgs );
	virtual void		DrawText( SalGraphics& rGraphics ) const;
	virtual int			GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const;
	virtual long		FillDXArray( long *pDXArray ) const;
	virtual void		GetCaretPositions( int nArraySize, long *pCaretXArray ) const;
	virtual int			GetNextGlyphs( int nLen, long *pGlyphs, Point& rPos, int& nStart, long *pGlyphAdvances, int *pCharIndexes ) const;
	virtual bool		GetOutline( SalGraphics& rGraphics, PolyPolyVector& rPPV ) const;
	virtual bool		GetBoundRect( SalGraphics& rGraphics, Rectangle& rRect ) const;
	virtual void		MoveGlyph( int nStart, long nNewXPos );
	virtual void		DropGlyph( int nStart );
	virtual void		Simplify( bool bIsBase );
};

class PolyArgs
{
	PolyPolygon*		mpPolyPoly;
	long				mnXOffset;
	long				mnYOffset;
	USHORT				mnMaxPoints;
	Point*				mpPointAry;
	BYTE*				mpFlagAry;
	USHORT				mnPointCount;
	USHORT				mnPolyCount;
	bool				mbHasOffline;

public:
						PolyArgs();
						~PolyArgs();

	void				Init( PolyPolygon *pPolyPoly, long nXOffset, long nYOffset );
	void				AddPoint( const Float32Point& rPoint, PolyFlags eFlags );
	void				ClosePolygon();
};

using namespace vcl;

// ============================================================================

OSStatus MyATSCubicMoveToCallback( const Float32Point *pt1, void* pData )
{
	PolyArgs& rA = *reinterpret_cast<PolyArgs*>(pData);
	// MoveTo implies a new polygon => finish old polygon first
	rA.ClosePolygon();
	rA.AddPoint( *pt1, POLY_NORMAL );
}

// ----------------------------------------------------------------------------

OSStatus MyATSCubicLineToCallback( const Float32Point* pt1, void* pData )
{
	PolyArgs& rA = *reinterpret_cast<PolyArgs*>(pData);
	rA.AddPoint( *pt1, POLY_NORMAL );
}

// ----------------------------------------------------------------------------

OSStatus MyATSCubicCurveToCallback( const Float32Point* pt1, const Float32Point* pt2, const Float32Point* pt3, void* pData )
{
	PolyArgs& rA = *reinterpret_cast<PolyArgs*>(pData);
	rA.AddPoint( *pt1, POLY_CONTROL );
	rA.AddPoint( *pt2, POLY_CONTROL );
	rA.AddPoint( *pt3, POLY_NORMAL );
}

// ----------------------------------------------------------------------------

OSStatus MyATSCubicClosePathCallback( void *pData )
{
	PolyArgs& rA = *reinterpret_cast<PolyArgs*>(pData);
	rA.ClosePolygon();
}

// ============================================================================

SalLayout *SalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
	return new ATSLayout( maGraphicsData.mpVCLFont );
}

// ============================================================================

ATSLayout::ATSLayout( com_sun_star_vcl_VCLFont *pVCLFont ) :
	maLayout( NULL ),
	mpStyles( NULL ),
	mnRuns( 0 ),
	mnStart( 0 ),
	mnLen( 0 ),
	mnWidth( 0 ),
	mpAdvances( NULL ),
	mpGlyphInfoArray( NULL )
{
	mpVCLFont = new com_sun_star_vcl_VCLFont( pVCLFont->getJavaObject() );
}

// ----------------------------------------------------------------------------

ATSLayout::~ATSLayout()
{
	Destroy();

	if ( mpVCLFont )
		delete mpVCLFont;
	mpVCLFont = NULL;
}

// ----------------------------------------------------------------------------

bool ATSLayout::LayoutText( ImplLayoutArgs& rArgs )
{
	OSStatus		theErr;

	Destroy();

	mnLen = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
	if ( !mnLen )
		return false;

	mnStart = rArgs.mnMinCharPos;
	mnRuns = 0;

	bool bRTL;
	int i, j;

	// Count the number of runs
	rArgs.ResetPos();
	while ( rArgs.GetNextRun( &i, &j, &bRTL ) )
		mnRuns++;

	mpStyles = (ATSUStyle *)rtl_allocateMemory( mnRuns * sizeof( ATSUStyle ) );

	// Populate run lengths and style arrays
	bool bVertical = ( ( rArgs.mnFlags & SAL_LAYOUT_VERTICAL ) != 0 );
	UniCharCount aRunLengths[ mnRuns ];
	ItemCount nRunsProcessed = 0;
	rArgs.ResetPos();
	while ( rArgs.GetNextRun( &i, &j, &bRTL ) && nRunsProcessed < mnRuns )
	{
		ATSUStyle aStyle;
		if ( ATSUCreateStyle( &aStyle ) == noErr )
		{
			ATSUAttributeTag nTags[5];
			ByteCount nBytes[5];
			ATSUAttributeValuePtr nVals[5];

			// Set font
			// TODO: Don't hardcode Geneva
			ATSUFontID nFontID = kFontIDGeneva;
			nTags[0] = kATSUFontTag;
			nBytes[0] = sizeof( ATSUFontID );
			nVals[0] = &nFontID;
			Fixed nSize = Long2Fix( mpVCLFont->getSize() );
			nTags[1] = kATSUSizeTag;
			nBytes[1] = sizeof( Fixed );
			nVals[1] = &nSize;

			// Set vertical or horizontal
			UInt16 nVerticalDirection;
			nTags[2] = kATSUVerticalCharacterTag;
			nBytes[2] = sizeof( UInt16 );
			if ( bVertical )
				nVerticalDirection = kATSUStronglyVertical;
			else
				nVerticalDirection = kATSUStronglyHorizontal;
			nVals[2] = &nVerticalDirection;

			// Set LTR or RTL
			UInt16 nLineDirection;
			nTags[3] = kATSULineDirectionTag;
			nBytes[3] = sizeof( UInt16 );
			if ( bRTL && !bVertical )
				nLineDirection = kATSURightToLeftBaseDirection;
			else
				nLineDirection = kATSULeftToRightBaseDirection;
			nVals[3] = &nLineDirection;

			// Set antialiasing
			ATSStyleRenderingOptions nOptions;
			if ( mpVCLFont->isAntialiased() )
				nOptions = kATSStyleApplyAntiAliasing;
			else
				nOptions = kATSStyleNoAntiAliasing;
			nTags[4] = kATSUStyleRenderingOptionsTag;
			nBytes[4] = sizeof( ATSStyleRenderingOptions );
			nVals[4] = &nOptions;

			ATSUSetAttributes( aStyle, 5, nTags, nBytes, nVals );
		}
		else
		{
			aStyle = NULL;
		}

		aRunLengths[ nRunsProcessed ] = j - i;
		mpStyles[ nRunsProcessed ] = aStyle;
		nRunsProcessed++;	
	}

	if ( ATSUCreateTextLayoutWithTextPtr( rArgs.mpStr + mnStart, kATSUFromTextBeginning, kATSUToTextEnd, mnLen, mnRuns, aRunLengths, mpStyles, &maLayout ) != noErr )
	{
		Destroy();
		return false;
	}

	// Set automatic font matching for missing glyphs
	ATSUSetTransientFontMatching( maLayout, true );

	return true;
}

// ----------------------------------------------------------------------------

void ATSLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
/*
	int nPixelWidth = rArgs.mnLayoutWidth;
	if( !nPixelWidth && rArgs.mpDXArray )
	{
		// For now we are only interested in the layout width
		// TODO: account for individual logical widths
		nPixelWidth = rArgs.mpDXArray[ mnLen - 1 ];
	}

	if( !nPixelWidth )
		return;

	ATSUAttributeTag nTags[2];
	ATSUAttributeValuePtr nVals[2];
	ByteCount nBytes[2];

	Fixed nFixedWidth = Long2Fix( nPixelWidth );
	Fixed nFixedOne = Long2Fix( 1 );
	nTags[0] = kATSULineWidthTag;
	nBytes[0] = sizeof( Fixed );
	nVals[0] = &nFixedWidth;
	nTags[1] = kATSULineJustificationFactorTag;
	nBytes[1] = sizeof( Fixed );
	nVals[1] = &nFixedOne;
	ATSUSetLayoutControls( maLayout, 2, nTags, nBytes, nVals );
*/
}

// ----------------------------------------------------------------------------

void ATSLayout::DrawText( SalGraphics& rGraphics ) const
{
	if ( !maLayout )
		return;

	Point aPos = GetDrawPosition();
	CGContextRef aCGContext = (CGContextRef)rGraphics.maGraphicsData.mpVCLGraphics->getNativeGraphics();
	if ( aCGContext )
	{
		// Set the layout's CGContext and orientation
		ATSUAttributeTag nTags[2];
		ByteCount nBytes[2];
		ATSUAttributeValuePtr nVals[2];
		nTags[0] = kATSUCGContextTag;
		nBytes[0] = sizeof( CGContextRef );
		nVals[0] = &aCGContext;
		Fixed nOrientation = Long2Fix( mpVCLFont->getOrientation() / 10 );
		nTags[1] = kATSULineRotationTag;
		nBytes[1] = sizeof( Fixed );
		nVals[1] = &nOrientation;
		ATSUSetLayoutControls( maLayout, 2, nTags, nBytes, nVals );

		// Since there is no CGrafPtr associated with the CGContext, we need
		// to set the font size, color, etc. ourselves
		CGContextSaveGState( aCGContext );
		CGContextTranslateCTM( aCGContext, aPos.X(), aPos.Y() * -1 );
		float nScaleFactor = (float)mpVCLFont->getSize() / 12;
		CGContextScaleCTM( aCGContext, nScaleFactor, nScaleFactor );
		SalColor nColor = rGraphics.maGraphicsData.mnTextColor;
		CGContextSetRGBFillColor( aCGContext, (float)SALCOLOR_RED( nColor ) / 256, (float)SALCOLOR_GREEN( nColor ) / 256, (float)SALCOLOR_BLUE( nColor ) / 256, 1.0 );
		CGContextSetShouldAntialias( aCGContext, mpVCLFont->isAntialiased() );

		// Draw the text
		ATSUDrawText( maLayout, kATSUFromTextBeginning, kATSUToTextEnd, 0, 0 );
		Rectangle aRect;
		if ( GetBoundRect( rGraphics, aRect ) )
			rGraphics.maGraphicsData.mpVCLGraphics->addToFlush( aPos.X() + aRect.nLeft - 1, aPos.Y() + aRect.nTop - 1, aRect.GetWidth() + 2, aRect.GetHeight() + 2 );

		// Restore the CGContext state
		CGContextRestoreGState( aCGContext );

		// Reset layout controls
		ATSUClearLayoutControls( maLayout, 2, nTags );

		rGraphics.maGraphicsData.mpVCLGraphics->releaseNativeGraphics( aCGContext );
	}
}

// ----------------------------------------------------------------------------

int ATSLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
	if ( !InitAdvances() )
		return STRING_LEN;

	if ( ( ( mnWidth * nFactor ) + ( mnLen * nCharExtra ) ) <= nMaxWidth )
		return -1;

	// Iterate through advances until the maximum width is reached
	long nCurrentWidth = 0;
	for ( int i = 0; i < mnLen; i++ )
	{
		nCurrentWidth += ( mpAdvances[ i ] * nFactor );
		if ( nCurrentWidth >= nMaxWidth )
			return mnStart + 1;
		nCurrentWidth += nCharExtra;
	}

	return -1;
}

// ----------------------------------------------------------------------------

long ATSLayout::FillDXArray( long* pDXArray ) const
{
	if ( !InitAdvances() )
		return 0;

	if ( pDXArray )
	{
		for ( int i = 0; i < mnLen; i++ )
			pDXArray[ i ] = mpAdvances[ i ];
	}

	return mnWidth;
}

// ----------------------------------------------------------------------------

void ATSLayout::GetCaretPositions( int nArraySize, long *pCaretXArray ) const
{
	if ( !InitAdvances() )
		return;

	if ( pCaretXArray )
	{
		long nPreviousX = 0;
		for ( int i = 0; i < nArraySize; i += 2)
		{
			pCaretXArray[ i ] = nPreviousX;
			nPreviousX += mpAdvances[ i / 2 ];
			pCaretXArray[ i + 1 ] = nPreviousX;
		}
	}
}

// ----------------------------------------------------------------------------

int ATSLayout::GetNextGlyphs( int nLen, long *pGlyphs, Point& rPos, int& nStart, long *pGlyphAdvances, int *pCharIndexes ) const
{
	if ( !InitGlyphInfoArray() )
		return 0;

	// First glyph requested?
	if ( nStart < 0 )
		nStart = 0;
	// No glyphs left?
	if ( nStart >= mpGlyphInfoArray->numGlyphs )
		return 0;

	const ATSUGlyphInfo *pG = mpGlyphInfoArray->glyphs + nStart;
	rPos.X() = pG->screenX;
	rPos.Y() = Float32ToInt( pG->deltaY );

	int nCount = 0;
	for (; ++nCount <= nLen; ++pG )
	{
		*pGlyphs++ = pG->glyphID;
		if ( pCharIndexes )
			*pCharIndexes++ = pG->charIndex;
		if ( pGlyphAdvances )
			*pGlyphAdvances++ = 0;
        // TODO: break early if unexpected position
		break;
	}
	nStart += nCount;

    return nCount;
}

// ----------------------------------------------------------------------------

bool ATSLayout::GetOutline( SalGraphics& rGraphics, PolyPolyVector& rPPV ) const
{
	rPPV.clear();

	if ( !InitGlyphInfoArray() )
		return false;

	rPPV.resize( mpGlyphInfoArray->numGlyphs );
	PolyArgs aPolyArgs;
	const ATSUGlyphInfo *pG = mpGlyphInfoArray->glyphs;
	for ( int i = 0; i < mpGlyphInfoArray->numGlyphs; ++i, ++pG )
	{
		// Convert glyph ID at glyph position to outline
		GlyphID nGlyphId = pG->glyphID;
		long nDeltaY = Float32ToInt( pG->deltaY );
		aPolyArgs.Init( &rPPV[i], pG->screenX, nDeltaY );
		OSStatus nStatus, nCBStatus;
		nStatus = ATSUGlyphGetCubicPaths( mpStyles[0], nGlyphId, MyATSCubicMoveToCallback, MyATSCubicLineToCallback, MyATSCubicCurveToCallback, MyATSCubicClosePathCallback, &aPolyArgs, &nCBStatus );

		if ( nStatus != noErr && nCBStatus != noErr )
		{
			rPPV.resize( i );
			break;
		}
	}

	return true;
}

// ----------------------------------------------------------------------------

bool ATSLayout::GetBoundRect( SalGraphics& rGraphics, Rectangle& rRect ) const
{
	Rect aMacRect;

	if ( ATSUMeasureTextImage( maLayout, kATSUFromTextBeginning, kATSUToTextEnd, 0, 0, &aMacRect ) == noErr )
	{
		float nScaleFactor = (float)mpVCLFont->getSize() / 12;
		rRect = Rectangle( 0, (long)( nScaleFactor * aMacRect.top), (long)( nScaleFactor * ( aMacRect.right - aMacRect.left ) ), (long)( nScaleFactor * aMacRect.bottom ) );
		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------------

void ATSLayout::MoveGlyph( int nStart, long nNewXPos )
{
#ifdef DEBUG
	fprintf( stderr, "SalATSLayout::MoveGlyph not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void ATSLayout::DropGlyph( int nStart )
{
#ifdef DEBUG
	fprintf( stderr, "SalATSLayout::DropGlyph not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void ATSLayout::Simplify( bool bIsBase )
{
#ifdef DEBUG
	fprintf( stderr, "SalATSLayout::Simplify not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void ATSLayout::Destroy()
{
	if ( maLayout )
		ATSUDisposeTextLayout( maLayout );
	maLayout = NULL;

	mnRuns = 0;

	if ( mpStyles )
	{
		for ( ItemCount i = 0; i < mnRuns; i++ )
		{
			if ( mpStyles[ i ] )
				ATSUDisposeStyle( mpStyles[ i ] );
		}
		rtl_freeMemory( mpStyles );
	}
	mpStyles = NULL;

	mnStart = 0;
	mnLen = 0;
	mnWidth = 0;

	if ( mpAdvances )
		rtl_freeMemory( mpAdvances );
	mpAdvances = NULL;

	if ( mpGlyphInfoArray )
		rtl_freeMemory( mpGlyphInfoArray );
	mpGlyphInfoArray = NULL;
}

// ----------------------------------------------------------------------------

bool ATSLayout::InitAdvances() const
{
	if ( mpAdvances && mnWidth )
		return true;

	if ( !maLayout )
		return false;

	if ( mpAdvances )
		rtl_freeMemory( mpAdvances );
	mpAdvances = (long *)rtl_allocateMemory( mnLen * sizeof( long ) );

	Fixed nScaleFactor = FixDiv( Long2Fix( mpVCLFont->getSize() ), Long2Fix( 12 ) );

	long nPreviousX = 0;
	for ( int i = 0; i < mnLen; i++ )
	{
		ATSUCaret aCaret;
		if ( ATSUOffsetToCursorPosition( maLayout, i + 1, true, kATSUByCharacter, &aCaret, NULL, NULL ) == noErr )
		{
			mpAdvances[ i ] = Fix2Long( FixMul( aCaret.fX, nScaleFactor ) ) - nPreviousX;
			nPreviousX += mpAdvances[ i ];
		}
		else
		{
			mpAdvances[ i ] = 0;
		}
			
	}

	mnWidth = nPreviousX;

	return true;
}

// ----------------------------------------------------------------------------

bool ATSLayout::InitGlyphInfoArray() const
{
	if ( mpGlyphInfoArray )
		return true;

	// TODO: is there a good way to predict the maximum glyph count?
	ByteCount nBufSize = 3 * ( mnLen + 16 ) * sizeof( ATSUGlyphInfo );
	mpGlyphInfoArray = (ATSUGlyphInfoArray *)rtl_allocateMemory( nBufSize );

	if ( ATSUGetGlyphInfo( maLayout, kATSUFromTextBeginning, kATSUToTextEnd, &nBufSize, mpGlyphInfoArray ) != noErr )
	{
		if ( mpGlyphInfoArray )
			rtl_freeMemory( mpGlyphInfoArray );
		mpGlyphInfoArray = NULL;
		return false;
	}
	else
	{
		return true;
	}
}

// ============================================================================

PolyArgs::PolyArgs() :
	mpPolyPoly( NULL ),
	mnXOffset( 0 ),
	mnYOffset( 0 ),
	mnMaxPoints( 256 ),
	mnPointCount( 0 ),
	mnPolyCount( 0 ),
	mbHasOffline( false )
{
	mpPointAry = new Point[ mnMaxPoints ];
	mpFlagAry = new BYTE [ mnMaxPoints ];
}

// ----------------------------------------------------------------------------

PolyArgs::~PolyArgs()
{
	delete[] mpFlagAry;
	delete[] mpPointAry;
}

// ----------------------------------------------------------------------------

void PolyArgs::Init( PolyPolygon *pPolyPoly, long nXOffset, long nYOffset )
{
	mpPolyPoly = pPolyPoly;
	mnXOffset = nXOffset;
	mnYOffset = nYOffset;

	mpPolyPoly->Clear();
	mnPointCount = 0;
	mnPolyCount = 0;
}

// ----------------------------------------------------------------------------

void PolyArgs::AddPoint( const Float32Point& rPoint, PolyFlags eFlags )
{
	if ( mnPointCount >= mnMaxPoints )
	{
		// Resize if needed (TODO: use STL?)
		mnMaxPoints *= 4;
		Point* mpNewPoints = new Point[ mnMaxPoints ];
		BYTE* mpNewFlags = new BYTE[ mnMaxPoints ];
		for ( int i = 0; i < mnPointCount; ++i )
		{
			mpNewPoints[ i ] = mpPointAry[ i ];
			mpNewFlags[ i ] = mpFlagAry[ i ];
		}
		delete[] mpFlagAry;
		delete[] mpPointAry;
		mpPointAry = mpNewPoints;
		mpFlagAry = mpNewFlags;
	}

	// Convert to pixels and add startpoint offset
	int nXPos = Float32ToInt( rPoint.x );
	int nYPos = Float32ToInt( rPoint.y );
	mpPointAry[ mnPointCount ] = Point( nXPos + mnXOffset, nYPos + mnYOffset );

	// Set point flags
	mpFlagAry[ mnPointCount++ ] = eFlags;
	mbHasOffline |= ( eFlags != POLY_NORMAL );
}

// ----------------------------------------------------------------------------

void PolyArgs::ClosePolygon()
{
	if ( !mnPolyCount++ )
		 return;

	// Append finished polygon
	Polygon aPoly( mnPointCount, mpPointAry, mbHasOffline ? mpFlagAry : NULL );
	mpPolyPoly->Insert( aPoly );

	// Prepare for new polygon
	mnPointCount = 0;
	mbHasOffline = false;
}
