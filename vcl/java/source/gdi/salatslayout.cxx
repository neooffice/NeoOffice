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

#define FIX2PIXEL_SHIFT 8
#define UNITS_PER_PIXEL ( 1 << FIX2PIXEL_SHIFT )

inline int Float32ToInt( Float32 f ) { return (int)( f + 0.5 ); }
inline long Fix2Pixel( Fixed f ) { return (long)( f >> FIX2PIXEL_SHIFT ); }

class ATSLayout : public SalLayout
{
	::vcl::com_sun_star_vcl_VCLFont*	mpVCLFont;
	ATSUStyle			maFontStyle;
	ATSUTextLayout		maLayout;
	sal_Unicode*		mpStr;
	UniCharArrayOffset	mnStart;
	UniCharCount		mnLen;
	bool				mbRTL;
	bool				mbVertical;
	mutable long		mnBaseAdvance;
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
	mpStr( NULL ),
	mnStart( 0 ),
	mnLen( 0 ),
	mbRTL( false ),
	mbVertical( false ),
	mnBaseAdvance( 0 ),
	mnWidth( 0 ),
	mpAdvances( NULL ),
	mpGlyphInfoArray( NULL )
{
	SetUnitsPerPixel( UNITS_PER_PIXEL );

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

	SalLayout::AdjustLayout( rArgs );

	Destroy();

	if ( !maFontStyle )
		return false;

	mnLen = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
	if ( !mnLen )
		return false;

	mnStart = rArgs.mnMinCharPos;
	mbRTL = ( mnLayoutFlags & SAL_LAYOUT_BIDI_STRONG && mnLayoutFlags & SAL_LAYOUT_BIDI_RTL );

	// Create a copy of the string. Note that we add an extra character as we
	// need it for RTL strings.
	int nBufSize = ( mnLen + 1 ) * sizeof( sal_Unicode );
	mpStr = (sal_Unicode *)rtl_allocateMemory( nBufSize );
	memcpy( mpStr, rArgs.mpStr + mnStart, nBufSize );
	mpStr[ mnLen ] = 0;

	// Swap characters in RTL runs
	int nRunStart;
	int nRunEnd;
	bool bRunRTL;
	rArgs.ResetPos();
	while ( rArgs.GetNextRun( &nRunStart, &nRunEnd, &bRunRTL ) )
	{
		if ( mbRTL && bRunRTL )
		{
			nRunEnd -= mnStart;
			for ( int i = nRunStart - mnStart; i < nRunEnd; i++ )
			{
				sal_Unicode nChar = GetMirroredChar( mpStr[ i ] );
				if ( nChar )
					mpStr[ i ] = nChar;
			}
		}
	}

	if ( ATSUCreateTextLayoutWithTextPtr( mpStr, kATSUFromTextBeginning, kATSUToTextEnd, mnLen, 1, &mnLen, &maFontStyle, &maLayout ) != noErr )
	{
		Destroy();
		return false;
	}

	if ( mnLayoutFlags & SAL_LAYOUT_VERTICAL )
	{
		// Set vertical
		ATSUVerticalCharacterType nVertical = kATSUStronglyVertical;
		ATSUAttributeTag nTag = kATSUVerticalCharacterTag;
		ByteCount nBytes = sizeof( ATSUVerticalCharacterType );
		ATSUAttributeValuePtr nVal = &nVertical;

		if ( ATSUSetAttributes( maFontStyle, 1, &nTag, &nBytes, &nVal ) == noErr )
			mbVertical = true;
	}

	if ( mbRTL )
	{
		ATSUVerticalCharacterType nDirection = kATSURightToLeftBaseDirection;
		ATSUAttributeTag nTag = kATSULineDirectionTag;
		ByteCount nBytes = sizeof( MacOSBoolean );
		ATSUAttributeValuePtr nVal = &nDirection;
		ATSUSetLayoutControls( maLayout, 1, &nTag, &nBytes, &nVal );
	}

	ATSLineLayoutOptions nLayoutOptions = kATSLineKeepSpacesOutOfMargin;
	ATSUAttributeTag nTag = kATSULineLayoutOptionsTag;
	ByteCount nBytes = sizeof( ATSLineLayoutOptions );
	ATSUAttributeValuePtr nVal = &nLayoutOptions;
	ATSUSetLayoutControls( maLayout, 1, &nTag, &nBytes, &nVal );

	// Set automatic font matching for missing glyphs
	ATSUSetTransientFontMatching( maLayout, true );

	ATSUTextMeasurement nStart;
	ATSUTextMeasurement nEnd;
	if ( ATSUGetUnjustifiedBounds( maLayout, kATSUFromTextBeginning, kATSUToTextEnd, &nStart, &nEnd, NULL, NULL ) == noErr )
		Justify( Fix2Pixel( nEnd - nStart ) );

	if ( !mnWidth )
	{
		Destroy();
		return false;
	}

	if ( mbRTL )
	{
		// The OOo code does not like the trailing kashida on the last
		// logical character to be included in the character advances as it
		// messed up their caret position tracking so we need to calculate
		// and save the trailing kashida width
		mpStr[ mnLen ] = mpStr[ mnLen - 1 ];
		if ( ATSUTextInserted( maLayout, mnLen, 1 ) == noErr )
		{
			ATSUCaret aLastCaret;
			ATSUCaret aNextToLastCaret;
			if ( ATSUOffsetToCursorPosition( maLayout, mnLen, true, kATSUByCharacter, &aLastCaret, NULL, NULL ) == noErr )
			{
				if ( mnLen > 1 )
				{
					if ( ATSUOffsetToCursorPosition( maLayout, mnLen - 1, true, kATSUByCharacter, &aNextToLastCaret, NULL, NULL ) == noErr )
						mnBaseAdvance = Fix2Pixel( ( aLastCaret.fX * 2 ) - aNextToLastCaret.fX );
				}
				else
				{
					if ( ATSUOffsetToCursorPosition( maLayout, mnLen + 1, true, kATSUByCharacter, &aNextToLastCaret, NULL, NULL ) == noErr )
						mnBaseAdvance = Fix2Pixel( ( aLastCaret.fX * 2 ) - aNextToLastCaret.fX );
				}

				if ( ATSUTextDeleted( maLayout, mnLen, 1 ) != noErr )
				{
					Destroy();
					return false;
				}
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------------

void ATSLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
	SalLayout::AdjustLayout( rArgs );

	if ( rArgs.mpDXArray )
	{
		// TODO: actually position individual glyphs instead of justifying it
		Justify( mnBaseAdvance + ( rArgs.mpDXArray[ mnLen - 1 ] * UNITS_PER_PIXEL ) );
	}
	else if ( rArgs.mnLayoutWidth )
	{
		Justify( mnBaseAdvance + ( rArgs.mnLayoutWidth * UNITS_PER_PIXEL ) );
	}
}

// ----------------------------------------------------------------------------

void ATSLayout::DrawText( com_sun_star_vcl_VCLGraphics *pGraphics, SalColor nColor ) const
{
	if ( !maLayout )
		return;

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
		Point aPos = GetDrawPosition( Point( ( mnBaseAdvance * -1 / UNITS_PER_PIXEL ), 0 ) );
		if ( mbVertical )
		{
			// Center text and round towards the top of the font
			long nDescent = mpVCLFont->getDescent();
			long nHeight = nDescent + mpVCLFont->getAscent() + 1;
			aPos.X() += ( nHeight / 2 ) - nDescent;
		}
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

	// Iterate through advances until the maximum width is reached
	long nCurrentWidth = 0;
	for ( int i = 0; i < mnLen; i++ )
	{
		nCurrentWidth += ( mpAdvances[ i ] * nFactor );
		if ( nCurrentWidth >= nMaxWidth )
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
	if ( pCaretXArray )
	{
		for ( int i = 0; i < nArraySize; i++ )
			pCaretXArray[ i ] = -1;
	}

	if ( !InitAdvances() )
		return;

	if ( pCaretXArray )
	{
		long nPreviousX = mnBaseAdvance;
		for ( int i = 0; i < nArraySize; i += 2 )
		{
			if ( mbRTL )
			{
				// Handle RTL carets
				pCaretXArray[ i + 1 ] = nPreviousX;
				nPreviousX += mpAdvances[ i / 2 ];
				pCaretXArray[ i ] = nPreviousX;
			}
			else
			{
				// Handle LTR carets
				pCaretXArray[ i ] = nPreviousX;
				nPreviousX += mpAdvances[ i / 2 ];
				pCaretXArray[ i + 1 ] = nPreviousX;
			}
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
	rPos.X() = pG->screenX * UNITS_PER_PIXEL;
	rPos.Y() = Float32ToInt( pG->deltaY * UNITS_PER_PIXEL );

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

	// Adjust for units per pixel
	rRect.nLeft *= UNITS_PER_PIXEL;
	rRect.nTop *= UNITS_PER_PIXEL;
	rRect.nRight *= UNITS_PER_PIXEL;
	rRect.nBottom *= UNITS_PER_PIXEL;
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

	if ( mpStr )
		rtl_freeMemory( mpStr );
	mpStr = NULL;

	mnStart = 0;
	mnLen = 0;
	mbRTL = false;
	mbVertical = false;
	mnBaseAdvance = 0;
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

	if ( !maLayout || !mnWidth )
		return false;

	int nBufSize = mnLen * sizeof( long );
	mpAdvances = (long *)rtl_allocateMemory( nBufSize );
	memset( mpAdvances, 0, nBufSize );

	// Calculate positions in logical order
	ATSUCaret aCaret;
	long nPreviousX = 0;
	if ( mbRTL )
	{
		for ( int i = mnLen - 1; i > 0; i-- )
		{
			if ( ATSUOffsetToCursorPosition( maLayout, i + 1, true, kATSUByCharacter, &aCaret, NULL, NULL ) == noErr )
				mpAdvances[ i ] = Fix2Pixel( aCaret.fX ) - nPreviousX;
			nPreviousX += mpAdvances[ i ];
		}

		// Force any remaining advance into the first character
		mpAdvances[ 0 ] = mnWidth - nPreviousX - mnBaseAdvance;
	}
	else
	{
		for ( int i = 0; i < mnLen - 1; i++ )
		{
			if ( ATSUOffsetToCursorPosition( maLayout, i + 1, true, kATSUByCharacter, &aCaret, NULL, NULL ) == noErr )
				mpAdvances[ i ] = Fix2Pixel( aCaret.fX ) - nPreviousX;
			nPreviousX += mpAdvances[ i ];
		}

		// Force any remaining advance into the last character
		mpAdvances[ mnLen - 1 ] = mnWidth - nPreviousX - mnBaseAdvance;
	}

	return true;
}

// ----------------------------------------------------------------------------

bool ATSLayout::InitGlyphInfoArray() const
{
	if ( mpGlyphInfoArray )
		return true;

	if ( !maLayout || !mnWidth )
		return false;

	// TODO: is there a good way to predict the maximum glyph count?
	ByteCount nBufSize = 3 * ( mnLen + 16 ) * sizeof( ATSUGlyphInfo );
	mpGlyphInfoArray = (ATSUGlyphInfoArray *)rtl_allocateMemory( nBufSize );
	memset( mpGlyphInfoArray, 0, nBufSize );

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

	ATSUAttributeTag nTags[2];
	ByteCount nBytes[2];
	ATSUAttributeValuePtr nVals[2];

	ATSUTextMeasurement nWidth = Long2Fix( nNewWidth / UNITS_PER_PIXEL );
	nTags[0] = kATSULineWidthTag;
	nBytes[0] = sizeof( ATSUTextMeasurement );
	nVals[0] = &nWidth;
	Fract nJustification = X2Frac( 1.0 );
	nTags[1] = kATSULineJustificationFactorTag;
	nBytes[1] = sizeof( Fract );
	nVals[1] = &nJustification;

	if ( ATSUSetLayoutControls( maLayout, 2, nTags, nBytes, nVals ) == noErr )
	{
		mnWidth = nNewWidth;

		if ( mpAdvances )
			rtl_freeMemory( mpAdvances );
		mpAdvances = NULL;

		if ( mpGlyphInfoArray )
			rtl_freeMemory( mpGlyphInfoArray );
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
