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
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

inline int Float32ToInt( Float32 f ) { return (int)(f+0.5); }

// =======================================================================

class ATSLayout : public SalLayout
{
public:
				ATSLayout();
				~ATSLayout();

	virtual bool	LayoutText( ImplLayoutArgs& );
	virtual void	AdjustLayout( ImplLayoutArgs& );
	virtual void	DrawText( SalGraphics& ) const;

	virtual int    GetNextGlyphs( int nLen, long* pGlyphs, Point& rPos, int&,
						long* pGlyphAdvances, int* pCharIndexes ) const;

	virtual long	GetTextWidth() const;
	virtual long	FillDXArray( long* pDXArray ) const;
	virtual int    GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const;
	virtual void	GetCaretPositions( int nArraySize, long* pCaretXArray ) const;
	virtual bool	GetGlyphOutlines( SalGraphics&, PolyPolyVector& ) const;
	virtual bool	GetBoundRect( SalGraphics&, Rectangle& ) const;

	// for glyph+font+script fallback
	virtual void   InitFont();
	virtual void   MoveGlyph( int nStart, long nNewXPos );
	virtual void   DropGlyph( int nStart );
	virtual void   Simplify( bool bIsBase );

private:
	ATSUTextLayout		maATSULayout;
	ATSUStyle*			mpATSUStyles;
	int					mnRuns;
	UniCharArrayOffset  mnTextStart;
	UniCharCount		mnTextLen;
	UniCharCount		mnMinCharPos;
	UniCharCount		mnEndCharPos;
	int				mnBaseAdv;

private:
	mutable ATSUGlyphInfoArray *  mpGlyphInfoArray;

	bool		InitGlyphInfoArray() const;
	void		DestroyGlyphInfoArray( void );
};

// =======================================================================

ATSLayout::ATSLayout() :
	maATSULayout		( NULL ),
	mpATSUStyles		( NULL ),
	mpGlyphInfoArray    ( NULL ),
	mnRuns				( 0 ),
	mnTextStart		( 0 ),
	mnTextLen			( 0 ),
	mnBaseAdv			( 0 )
{}

// -----------------------------------------------------------------------

ATSLayout::~ATSLayout()
{
	if ( mpGlyphInfoArray )
		DestroyGlyphInfoArray();

	if ( maATSULayout )
		ATSUDisposeTextLayout( maATSULayout );

	if ( mpATSUStyles )
	{
		for ( int i = 0 ; i < mnRuns ; i++ )
		{
			if ( mpATSUStyles[ i ] )
				ATSUDisposeStyle( mpATSUStyles[ i ] );
		}
	}
	rtl_freeMemory( mpATSUStyles );
}

// -----------------------------------------------------------------------

bool ATSLayout::LayoutText( ImplLayoutArgs& rArgs )
{
	OSStatus		theErr;

	if ( mpGlyphInfoArray )
		DestroyGlyphInfoArray();

	if( maATSULayout )
		ATSUDisposeTextLayout( maATSULayout );

	if ( mpATSUStyles )
	{
		for ( int i = 0 ; i < mnRuns ; i++ )
		{
			if ( mpATSUStyles[ i ] )
				ATSUDisposeStyle( mpATSUStyles[ i ] );
		}
	}
	rtl_freeMemory( mpATSUStyles );

	mnMinCharPos = rArgs.mnMinCharPos;
	mnEndCharPos = rArgs.mnEndCharPos;
	mnTextStart = rArgs.mnMinCharPos;
	mnTextLen = rArgs.mnEndCharPos - rArgs.mnMinCharPos;

	mnRuns = 0;
	bool bRTL;
	int i, j;

	// Count the number of runs
	rArgs.ResetPos();
	while ( rArgs.GetNextRun( &i, &j, &bRTL ) )
		mnRuns++;

	mpATSUStyles = (ATSUStyle *)rtl_allocateMemory( mnRuns * sizeof( ATSUStyle ) );

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
			Fixed nSize = Long2Fix( 12 );
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
		mpATSUStyles[ nRunsProcessed ] = aStyle;
		nRunsProcessed++;	
	}

	theErr = ATSUCreateTextLayoutWithTextPtr( rArgs.mpStr, mnTextStart, mnTextLen, rArgs.mnLength, mnRuns, aRunLengths, mpATSUStyles, &maATSULayout );

	if( theErr != noErr )
	{
		fprintf( stderr, "ATSLayout::LayoutText() : "
			"Unable to create ATSUI text layout! "
			"nTextStart=%d, nTextLen=%d, nParaLen=%d => err = %d\n",
			mnTextStart, mnTextLen, rArgs.mnLength, theErr );
		return false;
	}

	ATSUSetTransientFontMatching( maATSULayout, true );

	return true;
}

// -----------------------------------------------------------------------

void ATSLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
	int nPixelWidth = rArgs.mnLayoutWidth;
	if( !nPixelWidth && rArgs.mpDXArray ) {
		// for now we are only interested in the layout width
		// TODO: account for individual logical widths
		nPixelWidth = rArgs.mpDXArray[ mnTextLen - 1 ];
	}
	// return early if there is nothing to do
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
	ATSUSetLayoutControls( maATSULayout, 2, nTags, nBytes, nVals );
}

// -----------------------------------------------------------------------

void ATSLayout::DrawText( SalGraphics& rGraphics ) const
{
	if( mnTextLen <= 0 )
		return;

	Point aPos = GetDrawPosition( Point(mnBaseAdv, 0) );
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

			theErr = ATSUSetLayoutControls( maATSULayout, 1, &theTag, &valSize, &valPtr );
			if( theErr != noErr )
				fprintf( stderr, "ATSLayout::DrawText(0x%X) : Unable to set layout font rotation\n", this );
		}
*/

		// Tell ATSUI to use CoreGraphics
		ATSUSetLayoutControls( maATSULayout, 1, &cgTag, &cgSize, &cgValPtr );

/*
		// Save the current CTM and state so we can munge it for ATSUI drawing
		CGContextSaveGState( aCGContext );
		GetGraphicsBoundsCGRect( mpGraphicsData, &viewRect );

		// Undo the coordinate translations we did to flip the view.
		// Because translations are CUMULATIVE, we can't simply cancel just the scale
		CGContextTranslateCTM( aCGContext, 0, viewRect.size.height );
		CGContextScaleCTM( aCGContext, 1.0, -1.0 );
*/

		// Draw the text
		SalColor nColor = rGraphics.maGraphicsData.mnTextColor;
		CGContextSetRGBFillColor( aCGContext, SALCOLOR_RED( nColor), SALCOLOR_GREEN( nColor ), SALCOLOR_BLUE( nColor), 1.0 );
		if( ATSUDrawText( maATSULayout, mnTextStart, mnTextLen, Long2Fix(aPos.X()), Long2Fix(aPos.Y() * -1)) != noErr )
		{
			fprintf( stderr, "ATSLayout::DrawText(0x%X) : ATSUDrawText failed!\n", this );
		}
		rGraphics.maGraphicsData.mpVCLGraphics->addToFlush( 0, 0, 1000, 1000);

/*
		// Restore OOo-correct CTM and state
		CGContextRestoreGState( aCGContext );
*/

		rGraphics.maGraphicsData.mpVCLGraphics->releaseNativeGraphics( aCGContext );
	}
}

// -----------------------------------------------------------------------

int ATSLayout::GetNextGlyphs( int nLen, long* pGlyphs, Point& rPos, int& nStart,
    long* pGlyphAdvances, int* pCharIndexes ) const
{
	if( !InitGlyphInfoArray() )
		return 0;

	if( nStart < 0 )                    // first glyph requested?
		nStart = 0;
	if( nStart >= mpGlyphInfoArray->numGlyphs )    // no glyph left?
		return 0;

	const ATSUGlyphInfo* pG = mpGlyphInfoArray->glyphs + nStart;
	rPos.X() = pG->screenX;
	rPos.Y() = Float32ToInt( pG->deltaY );

	int nCount = 0;
	for(; ++nCount <= nLen; ++pG )
	{
		*(pGlyphs++) = pG->glyphID;
		if( pCharIndexes )
			*(pCharIndexes++) = pG->charIndex;
		if( pGlyphAdvances )
			*(pGlyphAdvances++) = 0;	// TODO
        // TODO: break early if unexpected position
		break; // nCount=1 because we don't know the glyph width
	}

	nStart += nCount;
    return nCount;
}

// -----------------------------------------------------------------------

// get typographic bounds of the text
long ATSLayout::GetTextWidth() const
{
	ItemCount nBoundsCount = 0;
	ATSTrapezoid aTrapez;
	ATSUGetGlyphBounds( maATSULayout, 0, 0, mnTextStart, mnTextLen, kATSUseDeviceOrigins, 1, &aTrapez, &nBoundsCount );
	if( nBoundsCount != 1 )
		return 0;

	long nWidth = Fix2Long( aTrapez.lowerRight.x - aTrapez.lowerLeft.x );
	return ( nWidth );
}

// -----------------------------------------------------------------------

long ATSLayout::FillDXArray( long* pDXArray ) const
{
	long nWidth = GetTextWidth();

	if ( (NULL != pDXArray) && (mnTextLen > 0) )
	{
		// for now we split the whole width to fake the virtual charwidths
		// TODO: calculate the virtual character widths exactly
		for( int i = 0; i < mnTextLen; i++ )
			pDXArray[ i ] = ((i+1) * nWidth) / mnTextLen;
	}

	return( nWidth );
}

// -----------------------------------------------------------------------

int ATSLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
	// TODO: accout for nCharExtra
	ATSUTextMeasurement nATSUMaxWidth = Long2Fix( nMaxWidth / nFactor );

	// TODO: massage ATSUBreakLine to like inword breaks:
	//   we prefer BreakInWord instead of ATSUBreakLine trying to be smart
	//   and moving the soft break inbetween words, as the ATSUI API says
	UniCharArrayOffset nBreakPos = mnTextStart;
	OSStatus nStatus = ATSUBreakLine( maATSULayout, mnTextStart, nATSUMaxWidth, false, &nBreakPos );
	if( ((nStatus != noErr) && (nStatus != kATSULineBreakInWord)) || (nBreakPos==mnTextStart) )
	{
		fprintf(stderr,"ATSUBreakLine => %d\n", nStatus);
		return( STRING_LEN );
	}
	return( mnTextStart + nBreakPos );
}

// -----------------------------------------------------------------------

void ATSLayout::GetCaretPositions( int nMaxIndex, long* pCaretXArray ) const
{
	if ( !pCaretXArray )
	{
		fprintf( stderr, "GetCaretPosition() pCaretXArray == NULL\n" );
		return;
	}

	// prepare caret array
	int n;
	for( n = 0; n < nMaxIndex; ++n )
		pCaretXArray[n] = -1;

	if( !InitGlyphInfoArray() )
		return;

	// use glyph index array to fill the caret positions
	const ATSUGlyphInfo* pG = mpGlyphInfoArray->glyphs;
	for( int i = 0; i < mpGlyphInfoArray->numGlyphs; ++i )
	{
		n = 2 * pG->charIndex;
		if( n > nMaxIndex + 1 )
			continue;
		// TODO: recognize and deal with RTL glyphs
		// is it one the pG->layputFlags?
		bool bRTL = false;
		// TODO: pCaretXArray[ n+0 ] = ???;
		pCaretXArray[ n+1 ] = pG->caretX;
	}

	long nXPos = 0;
	for( n = 0; n < nMaxIndex; ++n )
	{
		if( pCaretXArray[n] >= 0 )
			nXPos = pCaretXArray[n];
		else
			pCaretXArray[n] = nXPos;
	}
}

// -----------------------------------------------------------------------

// get ink bounds of the text
bool ATSLayout::GetBoundRect( SalGraphics&, Rectangle& rVCLRect ) const
{
	Rect aMacRect;
	ATSUMeasureTextImage( maATSULayout,
		mnTextStart, mnTextLen, 0, 0, &aMacRect );
	rVCLRect.Left() 	= aMacRect.left;
	rVCLRect.Top()		= aMacRect.top;
	rVCLRect.Right() 	= aMacRect.right;
	rVCLRect.Bottom() 	= aMacRect.bottom;
	return true;
}

// -----------------------------------------------------------------------

// initialize glyph index array
bool ATSLayout::InitGlyphInfoArray() const
{
	if( mpGlyphInfoArray )
		return true;

	// TODO: is there a good way to predict the maximum glyph count?
	ByteCount nBufSize = sizeof(ATSUGlyphInfoArray);
	nBufSize += 3*(mnTextLen+16) * sizeof(ATSUGlyphInfo);
	char* pBuffer = new char[ nBufSize ];

	OSStatus theErr = ATSUGetGlyphInfo( maATSULayout,
						mnTextStart, mnTextLen, &nBufSize, (ATSUGlyphInfoArray *)(pBuffer) );
	if( theErr == noErr )
		mpGlyphInfoArray = reinterpret_cast<ATSUGlyphInfoArray*>( pBuffer );
	else
	{
		fprintf( stderr, "ATSUGetGlyphInfo => err=%d\n", theErr);
		delete[] pBuffer;
	}

	return (mpGlyphInfoArray != NULL);
}

// -----------------------------------------------------------------------

void ATSLayout::DestroyGlyphInfoArray( void )
{
	if ( mpGlyphInfoArray )
	{
		delete[] mpGlyphInfoArray;
		mpGlyphInfoArray = NULL;
	}
}

// =======================================================================

// helper class to convert ATSUI outlines to VCL PolyPolygons
class PolyArgs
{
public:
				PolyArgs();
				~PolyArgs();

	void		Init( PolyPolygon* pPolyPoly, long nXOffset, long nYOffset );
	void		AddPoint( const Float32Point&, PolyFlags );
	void		ClosePolygon();

private:
	PolyPolygon* mpPolyPoly;
	long		mnXOffset, mnYOffset;

	Point*		mpPointAry;
	BYTE*		mpFlagAry;
	USHORT		mnMaxPoints;

	USHORT		mnPointCount;
	USHORT		mnPolyCount;
	bool		mbHasOffline;
};

// -----------------------------------------------------------------------

PolyArgs::PolyArgs()
:	mpPolyPoly(NULL),
	mnPointCount(0),
	mnPolyCount(0),
	mbHasOffline(false)
{
	mnMaxPoints = 256;
	mpPointAry	= new Point[ mnMaxPoints ];
	mpFlagAry	= new BYTE [ mnMaxPoints ];
}

// -----------------------------------------------------------------------

PolyArgs::~PolyArgs()
{
	delete[] mpFlagAry;
	delete[] mpPointAry;
}

// -----------------------------------------------------------------------

void PolyArgs::Init( PolyPolygon* pPolyPoly, long nXOffset, long nYOffset )
{
	mnXOffset = nXOffset;
	mnYOffset = nYOffset;
	mpPolyPoly = pPolyPoly;

	mpPolyPoly->Clear();
	mnPointCount = 0;
	mnPolyCount  = 0;
}

// -----------------------------------------------------------------------

void PolyArgs::AddPoint( const Float32Point& rPoint, PolyFlags eFlags )
{
	if( mnPointCount >= mnMaxPoints )
	{
		// resize if needed (TODO: use STL?)
		mnMaxPoints *= 4;
		Point* mpNewPoints = new Point[ mnMaxPoints ];
		BYTE* mpNewFlags = new BYTE[ mnMaxPoints ];
		for( int i = 0; i < mnPointCount; ++i )
		{
			mpNewPoints[ i ] = mpPointAry[ i ];
			mpNewFlags[ i ] = mpFlagAry[ i ];
		}
		delete[] mpFlagAry;
		delete[] mpPointAry;
		mpPointAry = mpNewPoints;
		mpFlagAry = mpNewFlags;
	}

	// convert to pixels and add startpoint offset
	int nXPos = Float32ToInt( rPoint.x );
	int nYPos = Float32ToInt( rPoint.y );
	mpPointAry[ mnPointCount ] = Point( nXPos + mnXOffset, nYPos + mnYOffset );
	// set point flags
	mpFlagAry[ mnPointCount++ ]= eFlags;
	mbHasOffline |= (eFlags != POLY_NORMAL);
}

// -----------------------------------------------------------------------

void PolyArgs::ClosePolygon()
{
	if( !mnPolyCount++ )
		 return;

	// append finished polygon
	Polygon aPoly( mnPointCount, mpPointAry, (mbHasOffline ? mpFlagAry : NULL) );
	mpPolyPoly->Insert( aPoly );

	// prepare for new polygon
	mnPointCount = 0;
	mbHasOffline = false;
}

// =======================================================================

// helper functions for ATSLayout::GetGlyphOutlines()
OSStatus MyATSCubicMoveToCallback( const Float32Point *pt1, 
   void* pData )
{
	PolyArgs& rA = *reinterpret_cast<PolyArgs*>(pData);
	// MoveTo implies a new polygon => finish old polygon first
	rA.ClosePolygon();
	rA.AddPoint( *pt1, POLY_NORMAL );
}

OSStatus MyATSCubicLineToCallback( const Float32Point* pt1,
	void* pData )
{
	PolyArgs& rA = *reinterpret_cast<PolyArgs*>(pData);
	rA.AddPoint( *pt1, POLY_NORMAL );
}

OSStatus MyATSCubicCurveToCallback( const Float32Point* pt1,
	const Float32Point* pt2, const Float32Point* pt3, void* pData )
{
	PolyArgs& rA = *reinterpret_cast<PolyArgs*>(pData);
	rA.AddPoint( *pt1, POLY_CONTROL );
	rA.AddPoint( *pt2, POLY_CONTROL );
	rA.AddPoint( *pt3, POLY_NORMAL );
}

OSStatus MyATSCubicClosePathCallback (
   void *pData )
{
	PolyArgs& rA = *reinterpret_cast<PolyArgs*>(pData);
	rA.ClosePolygon();
}

// -----------------------------------------------------------------------

bool ATSLayout::GetGlyphOutlines( SalGraphics&, PolyPolyVector& rPPV ) const
{
	rPPV.clear();

	if( !InitGlyphInfoArray() )
		return false;

	rPPV.resize( mpGlyphInfoArray->numGlyphs );
	PolyArgs aPolyArgs;
	const ATSUGlyphInfo* pG = mpGlyphInfoArray->glyphs;
	for( int i = 0; i < mpGlyphInfoArray->numGlyphs; ++i, ++pG )
	{
		// convert glyphid at glyphpos to outline
		GlyphID nGlyphId = pG->glyphID;
		long nDeltaY = Float32ToInt( pG->deltaY );
		aPolyArgs.Init( &rPPV[i], pG->screenX, nDeltaY );
		OSStatus nStatus, nCBStatus;
		nStatus = ATSUGlyphGetCubicPaths( mpATSUStyles[0], nGlyphId, MyATSCubicMoveToCallback, MyATSCubicLineToCallback, MyATSCubicCurveToCallback, MyATSCubicClosePathCallback, &aPolyArgs, &nCBStatus );

		if( (nStatus != noErr) && (nCBStatus != noErr) )
		{
			fprintf( stderr,"ATSUCallback = %d,%d\n", nStatus, nCBStatus );
			rPPV.resize( i );
			break;
		}
	}

	return true;
}

// -----------------------------------------------------------------------

void ATSLayout::InitFont()
{
	// TODO to allow glyph fallback
}

// -----------------------------------------------------------------------

void ATSLayout::MoveGlyph( int nStart, long nNewXPos )
{
	// TODO to allow glyph fallback
}

// -----------------------------------------------------------------------

void ATSLayout::DropGlyph( int nStart )
{
	// TODO to allow glyph fallback
}

// -----------------------------------------------------------------------

void ATSLayout::Simplify( bool bIsBase )
{
	// TODO to allow glyph fallback
}

// =======================================================================

SalLayout* SalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
	return new ATSLayout();
}

// =======================================================================
