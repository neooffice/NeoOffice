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

inline int Float32ToInt( Float32 f ) { return (int)( f+0.5 ); }

class ATSLayout : public SalLayout
{
private:
	::vcl::com_sun_star_vcl_VCLFont*	mpVCLFont;
	ATSUTextLayout		maLayout;
	int					mnRuns;
	ATSUStyle*			mpStyles;
	UniCharArrayOffset	mnStart;
	UniCharCount		mnLen;
	mutable ATSUGlyphInfoArray*	mpGlyphInfoArray;

	bool				InitGlyphInfoArray() const;
	void				DestroyGlyphInfoArray();

public:
						ATSLayout( ::vcl::com_sun_star_vcl_VCLFont *pVCLFont );
						~ATSLayout();

	virtual bool		LayoutText( ImplLayoutArgs& rArgs );
	virtual void		AdjustLayout( ImplLayoutArgs& rArgs );
	virtual void		DrawText( SalGraphics& rGraphics ) const;
	virtual int			GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const;
	virtual long		FillDXArray( long *pDXArray ) const;
	virtual long		GetTextWidth() const;
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
	mpGlyphInfoArray( NULL )
{
	mpVCLFont = new com_sun_star_vcl_VCLFont( pVCLFont->getJavaObject() );
}

// ----------------------------------------------------------------------------

ATSLayout::~ATSLayout()
{
	if ( mpVCLFont )
		delete mpVCLFont;

	if ( mpGlyphInfoArray )
		DestroyGlyphInfoArray();

	if ( maLayout )
		ATSUDisposeTextLayout( maLayout );

	if ( mpStyles )
	{
		for ( int i = 0 ; i < mnRuns ; i++ )
		{
			if ( mpStyles[ i ] )
				ATSUDisposeStyle( mpStyles[ i ] );
		}
	}
	rtl_freeMemory( mpStyles );
}

// ----------------------------------------------------------------------------

bool ATSLayout::LayoutText( ImplLayoutArgs& rArgs )
{
	OSStatus		theErr;

	if ( mpGlyphInfoArray )
		DestroyGlyphInfoArray();

	if( maLayout )
		ATSUDisposeTextLayout( maLayout );

	if ( mpStyles )
	{
		for ( int i = 0 ; i < mnRuns ; i++ )
		{
			if ( mpStyles[ i ] )
				ATSUDisposeStyle( mpStyles[ i ] );
		}
	}
	rtl_freeMemory( mpStyles );

	mnStart = rArgs.mnMinCharPos;
	mnLen = rArgs.mnEndCharPos - rArgs.mnMinCharPos;

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
	int nRunsProcessed = 0;
	rArgs.ResetPos();
	while ( rArgs.GetNextRun( &i, &j, &bRTL ) && nRunsProcessed < mnRuns )
	{
		ATSUStyle aStyle;
		if ( ATSUCreateStyle( &aStyle ) == noErr )
		{
			ATSUAttributeTag nTags[4];
			ATSUAttributeValuePtr nVals[4];
			ByteCount nBytes[4];

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

			UInt16 nVerticalDirection;
			nTags[2] = kATSUVerticalCharacterTag;
			nBytes[2] = sizeof( UInt16 );
			if ( bVertical )
				nVerticalDirection = kATSUStronglyVertical;
			else
				nVerticalDirection = kATSUStronglyHorizontal;
			nVals[2] = &nVerticalDirection;

			UInt16 nLineDirection;
			nTags[3] = kATSULineDirectionTag;
			nBytes[3] = sizeof( UInt16 );
			if ( bRTL && !bVertical )
				nLineDirection = kATSURightToLeftBaseDirection;
			else
				nLineDirection = kATSULeftToRightBaseDirection;
			nVals[3] = &nLineDirection;

			ATSUSetAttributes( aStyle, 4, nTags, nBytes, nVals );
		}
		else
		{
			aStyle = NULL;
		}

		aRunLengths[ nRunsProcessed ] = j - i;
		mpStyles[ nRunsProcessed ] = aStyle;
		nRunsProcessed++;	
	}

	if ( ATSUCreateTextLayoutWithTextPtr( rArgs.mpStr, mnStart, mnLen, rArgs.mnLength, mnRuns, aRunLengths, mpStyles, &maLayout ) != noErr )
		return false;

	// Set automatic font matching for missing glyphs
	ATSUSetTransientFontMatching( maLayout, true );

	return true;
}

// ----------------------------------------------------------------------------

void ATSLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
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
}

// ----------------------------------------------------------------------------

void ATSLayout::DrawText( SalGraphics& rGraphics ) const
{
	if( mnLen <= 0 )
		return;

	Point aPos = GetDrawPosition();
	CGContextRef aCGContext = (CGContextRef)rGraphics.maGraphicsData.mpVCLGraphics->getNativeGraphics();
	if ( aCGContext )
	{
		ATSUAttributeTag		cgTag = kATSUCGContextTag;
		ByteCount				cgSize = sizeof( CGContextRef );
		ATSUAttributeValuePtr	cgValPtr = &aCGContext;
/*
		CGRect				viewRect;

		// [ed] 9/21 If we're not in a standard rotation, draw the rotated text
		if( mpGraphicsData->mnATSUIRotation != 0 )
		{
			Fixed				theAngle = Long2Fix( mpGraphicsData->mnATSUIRotation );
			ATSUAttributeTag		theTag = kATSULineRotationTag;
			ByteCount				valSize = sizeof( Fixed );
			ATSUAttributeValuePtr	valPtr = &theAngle;
			OSStatus				theErr;

			theErr = ATSUSetLayoutControls( maLayout, 1, &theTag, &valSize, &valPtr );
			if( theErr != noErr )
				fprintf( stderr, "ATSLayout::DrawText(0x%X) : Unable to set layout font rotation\n", this );
		}
*/

		// Since there is no CGrafPtr associated with the CGContext, we need
		// to set the font size, color, etc. ourselves
		CGContextSaveGState( aCGContext );
		CGContextTranslateCTM( aCGContext, aPos.X(), aPos.Y() * -1 );
		float nScaleFactor = (float)mpVCLFont->getSize() / 12;
		CGContextScaleCTM( aCGContext, nScaleFactor, nScaleFactor );
		ATSUSetLayoutControls( maLayout, 1, &cgTag, &cgSize, &cgValPtr );
		SalColor nColor = rGraphics.maGraphicsData.mnTextColor;
		CGContextSetRGBFillColor( aCGContext, SALCOLOR_RED( nColor), SALCOLOR_GREEN( nColor ), SALCOLOR_BLUE( nColor), 1.0 );

		// Draw the text
		ATSUDrawText( maLayout, mnStart, mnLen, 0, 0 );
		CGContextRestoreGState( aCGContext );
		rGraphics.maGraphicsData.mpVCLGraphics->addToFlush( 0, 0, 1000, 1000);

		rGraphics.maGraphicsData.mpVCLGraphics->releaseNativeGraphics( aCGContext );
	}
}

// ----------------------------------------------------------------------------

int ATSLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
	// TODO: account for nCharExtra
	ATSUTextMeasurement nATSUMaxWidth = Long2Fix( nMaxWidth / nFactor );

	// TODO: massage ATSUBreakLine to like inword breaks:
	//   we prefer BreakInWord instead of ATSUBreakLine trying to be smart
	//   and moving the soft break inbetween words, as the ATSUI API says
	UniCharArrayOffset nBreakPos = mnStart;
	OSStatus nStatus = ATSUBreakLine( maLayout, mnStart, nATSUMaxWidth, false, &nBreakPos );
	if( ( nStatus != noErr && nStatus != kATSULineBreakInWord ) || nBreakPos == mnStart )
		return STRING_LEN;
	else
		return mnStart + nBreakPos;
}

// ----------------------------------------------------------------------------

long ATSLayout::FillDXArray( long* pDXArray ) const
{
	long nWidth = GetTextWidth();

	if ( pDXArray && mnLen )
	{
		// For now we split the whole width to fake the virtual charwidths
		// TODO: calculate the virtual character widths exactly
		for ( int i = 0 ; i < mnLen ; i++ )
			pDXArray[ i ] = ( ( i + 1 ) * nWidth ) / mnLen;
	}

	return nWidth;
}

// ----------------------------------------------------------------------------

long ATSLayout::GetTextWidth() const
{
	Rect aMacRect;

	if ( ATSUMeasureTextImage( maLayout, mnStart, mnLen, 0, 0, &aMacRect ) == noErr )
		return aMacRect.right - aMacRect.left;
	else
		return 0;
}

// ----------------------------------------------------------------------------

void ATSLayout::GetCaretPositions( int nMaxIndex, long *pCaretXArray ) const
{
	if ( !pCaretXArray )
		return;

	// Prepare caret array
	int n;
	for ( n = 0 ; n < nMaxIndex ; n++ )
		pCaretXArray[n] = -1;

	if ( !InitGlyphInfoArray() )
		return;

	// Use glyph index array to fill the caret positions
	const ATSUGlyphInfo *pG = mpGlyphInfoArray->glyphs;
	for ( int i = 0 ; i < mpGlyphInfoArray->numGlyphs ; ++i )
	{
		n = 2 * pG->charIndex;
		if ( n > nMaxIndex + 1 )
			continue;
		// TODO: recognize and deal with RTL glyphs
		// is it one the pG->layputFlags?
		bool bRTL = false;
		// TODO: pCaretXArray[ n + 0 ] = ???;
		pCaretXArray[ n + 1 ] = pG->caretX;
	}

	long nXPos = 0;
	for ( n = 0 ; n < nMaxIndex ; n++ )
	{
		if ( pCaretXArray[n] >= 0 )
			nXPos = pCaretXArray[n];
		else
			pCaretXArray[n] = nXPos;
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
	for ( ; ++nCount <= nLen ; ++pG )
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
	for ( int i = 0 ; i < mpGlyphInfoArray->numGlyphs ; ++i, ++pG )
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

	if ( ATSUMeasureTextImage( maLayout, mnStart, mnLen, 0, 0, &aMacRect ) == noErr )
	{
		rRect = Rectangle( Point( aMacRect.left, aMacRect.top ), Size( aMacRect.right - aMacRect.left, aMacRect.bottom - aMacRect.top ) );
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

bool ATSLayout::InitGlyphInfoArray() const
{
	if ( mpGlyphInfoArray )
		return true;

	// TODO: is there a good way to predict the maximum glyph count?
	ByteCount nBufSize = sizeof( ATSUGlyphInfoArray );
	nBufSize += 3 * ( mnLen + 16) * sizeof( ATSUGlyphInfo );
	char* pBuffer = new char[ nBufSize ];

	OSStatus theErr = ATSUGetGlyphInfo( maLayout, mnStart, mnLen, &nBufSize, (ATSUGlyphInfoArray *)pBuffer );
	if ( theErr == noErr )
		mpGlyphInfoArray = reinterpret_cast<ATSUGlyphInfoArray*>( pBuffer );
	else
		delete[] pBuffer;

	return ( mpGlyphInfoArray != NULL );
}

// ----------------------------------------------------------------------------

void ATSLayout::DestroyGlyphInfoArray()
{
	if ( mpGlyphInfoArray )
	{
		delete[] mpGlyphInfoArray;
		mpGlyphInfoArray = NULL;
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
