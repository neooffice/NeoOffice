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
 *  Patrick Luby, August 2004
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2004 Planamesa Inc.
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

#ifndef _SV_SALATSLAYOUT_HXX
#define _SV_SALATSLAYOUT_HXX

#include <hash_map>
#include <vector>

#include <vcl/sallayout.hxx>

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>
#undef check

// Comment out the following line to disable subpixel text rendering
#define USE_SUBPIXEL_TEXT_RENDERING

// Comment out the following line to disable the OS X 10.11 Indic font hack
#define USE_INDIC_FONT_HACK

struct ImplATSLayoutData;
class JavaImplFont;
class JavaSalGraphics;

// ----------------
// - SalATSLayout -
// ----------------

class SalATSLayout : public GenericSalLayout
{
	JavaSalGraphics*	mpGraphics;
	int					mnFallbackLevel;
	JavaImplFont*		mpFont;
	mutable ImplLayoutRuns	maRuns;
	::std::vector< ImplATSLayoutData* >	maLayoutData;
	::std::vector< int >	maLayoutMinCharPos;
	ImplATSLayoutData*	mpKashidaLayoutData;
	::std::hash_map< sal_Unicode, ImplATSLayoutData* >	maMirroredLayoutData;
	long				mnOrigWidth;
	float				mfGlyphScaleX;

public:
	static void			GetGlyphBounds( sal_Int32 nGlyph, JavaImplFont *pFont, Rectangle &rRect );
	static void			ClearLayoutDataCache();

						SalATSLayout( JavaSalGraphics *pGraphics, int nFallbackLevel );
	virtual				~SalATSLayout();

	virtual void		AdjustLayout( ImplLayoutArgs& rArgs );
	virtual bool		LayoutText( ImplLayoutArgs& rArgs );
	virtual void		DrawText( SalGraphics& rGraphics ) const;
	virtual bool		GetBoundRect( SalGraphics& rGraphics, Rectangle& rRect ) const;
	virtual bool		GetOutline( SalGraphics& rGraphics, ::basegfx::B2DPolyPolygonVector& rVector ) const;

	ImplATSLayoutData*	GetVerticalGlyphTranslation( sal_Int32 nGlyph, int nCharPos, long& nX, long& nY ) const;
	sal_Int32			GetNativeGlyphWidth( sal_Int32 nGlyph, int nCharPos ) const;

	void				Destroy();
#ifdef USE_INDIC_FONT_HACK
	bool				SetIndicFontHack( const sal_Unicode *pStr, int nMinCharPos, int nEndCharPos );
#endif	// USE_INDIC_FONT_HACK
};

#endif // _SV_SALATSLAYOUT_HXX
