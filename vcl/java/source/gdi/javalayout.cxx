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
 *  Patrick Luby, June 2004
 *
 *  GNU General Public License Version 2.1
 *  =============================================
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

#ifndef _SV_JAVALAYOUT_HXX
#include <javalayout.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGLYPHVECTOR_HXX
#include <com/sun/star/vcl/VCLTextLayout.hxx>
#endif

using namespace vcl;

// =======================================================================

JavaLayout::JavaLayout( const SalGraphics *pGraphics )
{
	mpVCLTextLayout = new com_sun_star_vcl_VCLTextLayout( pGraphics->maGraphicsData.mpVCLGraphics, pGraphics->maGraphicsData.mpVCLFont );
}

// -----------------------------------------------------------------------

JavaLayout::~JavaLayout()
{
	if ( mpVCLTextLayout )
		delete mpVCLTextLayout;
}

// -----------------------------------------------------------------------

bool JavaLayout::LayoutText( ImplLayoutArgs& rArgs )
{
	mpVCLTextLayout->layoutText( rArgs );

	return true;
}

// -----------------------------------------------------------------------

void JavaLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
	if ( rArgs.mpDXArray )
		mpVCLTextLayout->setDXArray( rArgs.mpDXArray, rArgs.mnEndCharPos - rArgs.mnMinCharPos );
	else if ( rArgs.mnLayoutWidth )
		mpVCLTextLayout->justify( rArgs.mnLayoutWidth );
}

// -----------------------------------------------------------------------

void JavaLayout::DrawText( SalGraphics& rGraphics ) const
{
	Point aPoint( GetDrawPosition() );
	mpVCLTextLayout->drawText( aPoint.X(), aPoint.Y(), GetOrientation(), rGraphics.maGraphicsData.mnTextColor );
}

// -----------------------------------------------------------------------

int JavaLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
	int nRet = mpVCLTextLayout->getTextBreak( nMaxWidth, nCharExtra, nFactor );
	return ( nRet < 0 ? STRING_LEN : nRet );
}

// -----------------------------------------------------------------------

long JavaLayout::FillDXArray( long *pDXArray ) const
{
	return mpVCLTextLayout->fillDXArray( pDXArray );
}

// -----------------------------------------------------------------------

void JavaLayout::GetCaretPositions( int nArraySize, long *pCaretXArray ) const
{
	mpVCLTextLayout->getCaretPositions( nArraySize, pCaretXArray );
}

// -----------------------------------------------------------------------

int JavaLayout::GetNextGlyphs( int nLen, long *pGlyphs, Point& rPos, int& nStart, long *pGlyphAdvances, int *pCharIndexes ) const
{
#ifdef DEBUG
	fprintf( stderr, "JavaLayout::GetNextGlyphs not implemented\n" );
#endif
	return 0;
}

// -----------------------------------------------------------------------

bool JavaLayout::GetBoundRect( SalGraphics& rGraphics, Rectangle& rRect ) const
{
	return mpVCLTextLayout->getBounds( rRect );
}

// -----------------------------------------------------------------------

void JavaLayout::MoveGlyph( int nStart, long nNewPos )
{
#ifdef DEBUG
	fprintf( stderr, "JavaLayout::MoveGlyph not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaLayout::DropGlyph( int nStart )
{
#ifdef DEBUG
	fprintf( stderr, "JavaLayout::DropGlyph not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaLayout::Simplify( bool isBase )
{
#ifdef DEBUG
	fprintf( stderr, "JavaLayout::Simplify not implemented\n" );
#endif
}
