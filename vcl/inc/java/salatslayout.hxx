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

#include <vector>

#include <boost/unordered_map.hpp>

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>
#undef check

#include "sallayout.hxx"

// Comment out the following line to disable subpixel text rendering
#define USE_SUBPIXEL_TEXT_RENDERING

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
	::boost::unordered_map< sal_Unicode, ImplATSLayoutData* >	maMirroredLayoutData;
	DeviceCoordinate	mfOrigWidth;
	float				mfGlyphScaleX;

public:
	static void			GetGlyphBounds( sal_Int32 nGlyph, JavaImplFont *pFont, tools::Rectangle &rRect );
	static void			ClearLayoutDataCache();

						SalATSLayout( JavaSalGraphics *pGraphics, int nFallbackLevel );
	virtual				~SalATSLayout();

	virtual void		AdjustLayout( ImplLayoutArgs& rArgs ) override;
	virtual bool		LayoutText( ImplLayoutArgs& rArgs ) override;
	virtual void		DrawText( SalGraphics& rGraphics ) const override;
	virtual bool		GetBoundRect( SalGraphics& rGraphics, tools::Rectangle& rRect ) const override;
	virtual bool		GetOutline( SalGraphics& rGraphics, ::basegfx::B2DPolyPolygonVector& rVector ) const override;

	ImplATSLayoutData*	GetVerticalGlyphTranslation( sal_Int32 nGlyph, int nCharPos, DeviceCoordinate& fX, DeviceCoordinate& fY ) const;
	DeviceCoordinate	GetNativeGlyphWidth( sal_Int32 nGlyph, int nCharPos ) const;

	void				Destroy();
};

#endif // _SV_SALATSLAYOUT_HXX
