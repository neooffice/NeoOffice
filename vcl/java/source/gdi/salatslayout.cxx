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

inline int Float32ToInt( Float32 f ) { return (int)( f + 0.5 ); }

class ATSLayout : public SalLayout
{
	::vcl::com_sun_star_vcl_VCLFont*	mpVCLFont;
	ATSUStyle			maFontStyle;
	ATSUTextLayout		maLayout;
	ItemCount			mnRuns;
	UniCharArrayOffset	mnStart;
	UniCharCount		mnLen;
	bool				mbRTL;
	bool				mbVertical;
	mutable long		mnWidth;
	mutable long*		mpAdvances;
	mutable ATSUGlyphInfoArray*	mpGlyphInfoArray;

	void				Destroy();
	bool				GetBoundRect( Rectangle& rRect ) const;
	bool				InitAdvances() const;
	bool				InitGlyphInfoArray() const;
	void				Justify( long nNewWidth ) const;

public:
						ATSLayout( ::vcl::com_sun_star_vcl_VCLFont *pVCLFont );
	virtual				~ATSLayout();

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

	void				DrawText( ::vcl::com_sun_star_vcl_VCLGraphics *pGraphics, SalColor nColor ) const;
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

static void JNICALL Java_com_sun_star_vcl_VCLGraphics_drawTextLayout0( JNIEnv *pEnv, jobject object, jint pLayout, jint nColor )
{
	ATSLayout *pATSLayout = (ATSLayout *)pLayout;
	if ( pATSLayout )
	{
		com_sun_star_vcl_VCLGraphics *pGraphics = new com_sun_star_vcl_VCLGraphics( object );
		if ( pGraphics )
		{
			pATSLayout->DrawText( pGraphics, (SalColor)nColor );
			delete pGraphics;
		}
		pATSLayout->Release();
	}
}

// ----------------------------------------------------------------------------

static OSStatus MyATSCubicMoveToCallback( const Float32Point *pt1, void* pData )
{
	PolyArgs& rA = *reinterpret_cast<PolyArgs*>(pData);
	// MoveTo implies a new polygon => finish old polygon first
	rA.ClosePolygon();
	rA.AddPoint( *pt1, POLY_NORMAL );
}

// ----------------------------------------------------------------------------

static OSStatus MyATSCubicLineToCallback( const Float32Point* pt1, void* pData )
{
	PolyArgs& rA = *reinterpret_cast<PolyArgs*>(pData);
	rA.AddPoint( *pt1, POLY_NORMAL );
}

// ----------------------------------------------------------------------------

static OSStatus MyATSCubicCurveToCallback( const Float32Point* pt1, const Float32Point* pt2, const Float32Point* pt3, void* pData )
{
	PolyArgs& rA = *reinterpret_cast<PolyArgs*>(pData);
	rA.AddPoint( *pt1, POLY_CONTROL );
	rA.AddPoint( *pt2, POLY_CONTROL );
	rA.AddPoint( *pt3, POLY_NORMAL );
}

// ----------------------------------------------------------------------------

static OSStatus MyATSCubicClosePathCallback( void *pData )
{
	PolyArgs& rA = *reinterpret_cast<PolyArgs*>(pData);
	rA.ClosePolygon();
}

// ============================================================================

jclass com_sun_star_vcl_VCLGraphics::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLGraphics" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		// Register the VCLGraphics.drawTextLayout0 native method
		if ( t.pEnv && tempClass )
		{
			JNINativeMethod aMethod;
			aMethod.name = "drawTextLayout0";
			aMethod.signature = "(II)V";
			aMethod.fnPtr = (void *)Java_com_sun_star_vcl_VCLGraphics_drawTextLayout0;
			t.pEnv->RegisterNatives( tempClass, &aMethod, 1 );
		}
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ============================================================================

SalLayout *SalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
	return new ATSLayout( maGraphicsData.mpVCLFont );
}

// ============================================================================

ATSLayout::ATSLayout( com_sun_star_vcl_VCLFont *pVCLFont ) :
	maLayout( NULL ),
	maFontStyle( NULL ),
	mnRuns( 0 ),
	mnStart( 0 ),
	mnLen( 0 ),
	mbRTL( false ),
	mbVertical( false ),
	mnWidth( 0 ),
	mpAdvances( NULL ),
	mpGlyphInfoArray( NULL )
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
	Destroy();

	if ( maFontStyle )
		ATSUDisposeStyle( maFontStyle );
}

// ----------------------------------------------------------------------------

bool ATSLayout::LayoutText( ImplLayoutArgs& rArgs )
{
	OSStatus		theErr;

	Destroy();

	if ( !maFontStyle )
		return false;

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

	// Populate run lengths and style arrays
	UniCharCount aRunLengths[ mnRuns ];
	ATSUStyle aStyles[ mnRuns ];
	ItemCount nRunsProcessed = 0;
	bool bRTLRequested = false;
	rArgs.ResetPos();
	while ( rArgs.GetNextRun( &i, &j, &bRTL ) && nRunsProcessed < mnRuns )
	{
		if ( bRTL )
			bRTLRequested = true;
		aRunLengths[ nRunsProcessed ] = j - i;
		aStyles[ nRunsProcessed ] = maFontStyle;
		nRunsProcessed++;
	}

	if ( ATSUCreateTextLayoutWithTextPtr( rArgs.mpStr + mnStart, kATSUFromTextBeginning, kATSUToTextEnd, mnLen, mnRuns, aRunLengths, aStyles, &maLayout ) != noErr )
	{
		Destroy();
		return false;
	}

	if ( rArgs.mnFlags & SAL_LAYOUT_VERTICAL )
	{
		// Set vertical
		ATSUVerticalCharacterType nVertical = kATSUStronglyVertical;
		ATSUAttributeTag nTag = kATSUVerticalCharacterTag;
		ByteCount nBytes = sizeof( ATSUVerticalCharacterType );
		ATSUAttributeValuePtr nVal = &nVertical;

		if ( ATSUSetAttributes( maFontStyle, 1, &nTag, &nBytes, &nVal ) != noErr )
			mbVertical = true;
	}
	else if ( rArgs.mnFlags & ( SAL_LAYOUT_BIDI_STRONG | SAL_LAYOUT_BIDI_RTL ) )
	{
		if ( bRTLRequested )
			mbRTL = true;
	}

	// Set automatic font matching for missing glyphs
	ATSUSetTransientFontMatching( maLayout, true );

	return true;
}

// ----------------------------------------------------------------------------

void ATSLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
	if ( rArgs.mpDXArray )
	{
		// TODO: actually position individual glyphs instead of justifying it
		Justify( rArgs.mpDXArray[ mnLen - 1 ] );
	}
	else if ( rArgs.mnLayoutWidth )
	{
		Justify( rArgs.mnLayoutWidth );
	}
}

// ----------------------------------------------------------------------------

void ATSLayout::DrawText( com_sun_star_vcl_VCLGraphics *pGraphics, SalColor nColor ) const
{
	if ( !maLayout )
		return;

	Point aPos = GetDrawPosition();
	CGContextRef aCGContext = (CGContextRef)pGraphics->getNativeGraphics();
	if ( aCGContext )
	{
		CGContextSaveGState( aCGContext );

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

		// Set color
		CGContextSetRGBFillColor( aCGContext, (float)SALCOLOR_RED( nColor ) / 0xff, (float)SALCOLOR_GREEN( nColor ) / 0xff, (float)SALCOLOR_BLUE( nColor ) / 0xff, 1.0 );
		if ( maFontStyle )
		{
			ATSUAttributeTag nColorTags[2];
			nColorTags[0] = kATSUColorTag;
			nColorTags[1] = kATSURGBAlphaColorTag;
			ATSUClearAttributes( maFontStyle, 2, (const ATSUAttributeTag *)&nColorTags );
		}

		// Draw the text
		ATSUDrawText( maLayout, kATSUFromTextBeginning, kATSUToTextEnd, Long2Fix( aPos.X() ), Long2Fix( aPos.Y() * - 1 ) );

		// Add rotated text bounds (plus a little extra) to flush
		Rectangle aRect;
		if ( GetBoundRect( aRect ) )
			pGraphics->addToFlush( aPos.X() + aRect.nLeft - 5, aPos.Y() + aRect.nTop - 5, aRect.GetWidth() + 11, aRect.GetHeight() + 11 );

		// Reset layout controls
		ATSUClearLayoutControls( maLayout, 2, nTags );

		CGContextRestoreGState( aCGContext );

		pGraphics->releaseNativeGraphics( aCGContext );
	}
}

// ----------------------------------------------------------------------------

void ATSLayout::DrawText( SalGraphics& rGraphics ) const
{
	// Route call through Java so that when the VCLGraphics is using a drawing
	// queue, text can be drawn when this operation is dispatched from the
	// drawing queue. Note that the reference added here will be released
	// in the VCLGraphics native method.
	Reference();
	rGraphics.maGraphicsData.mpVCLGraphics->drawTextLayout( (void *)this, rGraphics.maGraphicsData.mnTextColor );
}

// ----------------------------------------------------------------------------

int ATSLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
	if ( !InitAdvances() )
		return STRING_LEN;

	if ( ( ( mnWidth * nFactor ) + ( mnLen * nCharExtra ) ) <= nMaxWidth )
		return STRING_LEN;

	// RTL text will have negative advances so also check for negative values
	long nMaxRTLWidth = nMaxWidth * -1;

	// Iterate through advances until the maximum width is reached
	long nCurrentWidth = 0;
	for ( int i = 0; i < mnLen; i++ )
	{
		nCurrentWidth += ( mpAdvances[ i ] * nFactor );
		if ( nCurrentWidth >= nMaxWidth || nCurrentWidth <= nMaxRTLWidth )
			return mnStart + i;
		nCurrentWidth += nCharExtra;
	}

	return STRING_LEN;
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
		nStatus = ATSUGlyphGetCubicPaths( maFontStyle, nGlyphId, MyATSCubicMoveToCallback, MyATSCubicLineToCallback, MyATSCubicCurveToCallback, MyATSCubicClosePathCallback, &aPolyArgs, &nCBStatus );

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
	GetBoundRect( rRect );
}

// ----------------------------------------------------------------------------

bool ATSLayout::GetBoundRect( Rectangle& rRect ) const
{
	Rect aRect;

	if ( ATSUMeasureTextImage( maLayout, kATSUFromTextBeginning, kATSUToTextEnd, 0, 0, &aRect ) == noErr )
	{
		rRect = Rectangle( aRect.left, aRect.top, aRect.right, aRect.bottom );
		rRect.Justify();
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
	if ( maFontStyle )
	{
		// Clear any special styles
		ATSUAttributeTag nTag = kATSUStronglyVertical;
		ATSUClearAttributes( maFontStyle, 1, &nTag );
	}

	if ( maLayout )
		ATSUDisposeTextLayout( maLayout );
	maLayout = NULL;

	mnRuns = 0;
	mnStart = 0;
	mnLen = 0;
	mbRTL = false;
	mbVertical = false;
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
	if ( mpAdvances )
		return true;

	if ( !maLayout )
		return false;

	// Force width to an integer since the OOo code cannot handle a fractional
	// layout width
	ATSUTextMeasurement nStart;
	ATSUTextMeasurement nEnd;
	ATSUTextMeasurement nAscent;
	ATSUTextMeasurement nDescent;
	if ( ATSUGetUnjustifiedBounds( maLayout, kATSUFromTextBeginning, kATSUToTextEnd, &nStart, &nEnd, &nAscent, &nDescent ) == noErr )
		Justify( Fix2Long( nEnd - nStart + 1 ) );

	if ( !mnWidth )
		return false;

	mpAdvances = (long *)rtl_allocateMemory( mnLen * sizeof( long ) );

	// RTL has negative advances so we need to set the beginning caret to the
	// layout width
	long nStartX = 0;
	if ( mbRTL )
		nStartX = mnWidth;

	ATSUCaret aCaret;
	long nPreviousX = nStartX;
	int nLen = mnLen - 1;
	for ( int i = 0; i < nLen; i++ )
	{
		if ( ATSUOffsetToCursorPosition( maLayout, i + 1, true, kATSUByCharacter, &aCaret, NULL, NULL ) == noErr )
		{
			mpAdvances[ i ] = Fix2Long( aCaret.fX ) - nPreviousX;
			nPreviousX += mpAdvances[ i ];
		}
		else
		{
			mpAdvances[ i ] = 0;
		}
	}

	// Force any remaining advance into the last character
	mpAdvances[ mnLen - 1 ] = mnWidth - ( nPreviousX - nStartX );

	return true;
}

// ----------------------------------------------------------------------------

bool ATSLayout::InitGlyphInfoArray() const
{
	if ( mpGlyphInfoArray )
		return true;

	if ( !maLayout )
		return false;

	// Force width to an integer since the OOo code cannot handle a fractional
	// layout width
	ATSUTextMeasurement nStart;
	ATSUTextMeasurement nEnd;
	ATSUTextMeasurement nAscent;
	ATSUTextMeasurement nDescent;
	if ( ATSUGetUnjustifiedBounds( maLayout, kATSUFromTextBeginning, kATSUToTextEnd, &nStart, &nEnd, &nAscent, &nDescent ) == noErr )
		Justify( Fix2Long( nEnd - nStart + 1 ) );

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

// ----------------------------------------------------------------------------

void ATSLayout::Justify( long nNewWidth ) const
{
	if ( nNewWidth == mnWidth )
		return;

	mnWidth = 0;

	if ( mpAdvances )
		rtl_freeMemory( mpAdvances );
	mpAdvances = NULL;

	if ( mpGlyphInfoArray )
		rtl_freeMemory( mpGlyphInfoArray );
	mpGlyphInfoArray = NULL;

	ATSUAttributeTag nTags[2];
	ByteCount nBytes[2];
	ATSUAttributeValuePtr nVals[2];

	ATSUTextMeasurement nWidth = Long2Fix( nNewWidth );
	nTags[0] = kATSULineWidthTag;
	nBytes[0] = sizeof( ATSUTextMeasurement );
	nVals[0] = &nWidth;
	Fract nJustification = kATSUFullJustification;
	nTags[1] = kATSULineJustificationFactorTag;
	nBytes[1] = sizeof( Fract );
	nVals[1] = &nJustification;

	if ( ATSUSetLayoutControls( maLayout, 2, nTags, nBytes, nVals ) == noErr )
		mnWidth = nNewWidth;

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
