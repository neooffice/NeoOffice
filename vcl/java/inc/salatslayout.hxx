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

#ifndef _SV_SALATSLAYOUT_HXX
#define _SV_SALATSLAYOUT_HXX

#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

class SalGraphics;

// ----------------
// - SalATSLayout -
// ----------------

class SalATSLayout : public GenericSalLayout
{
	SalGraphics*		mpGraphics;
	int					mnFallbackLevel;
	::vcl::com_sun_star_vcl_VCLFont*	mpVCLFont;
	ATSUStyle			maFontStyle;
	int					mnGlyphCount;
	ATSUGlyphInfoArray*	mpGlyphInfoArray;
	int*				mpCharsToGlyphs;
	ATSUStyle			maVerticalFontStyle;
	long				mnBaselineDelta;

	void				Destroy();

public:
						SalATSLayout( SalGraphics *pGraphics, int nFallbackLevel );
	virtual				~SalATSLayout();

	virtual bool		LayoutText( ImplLayoutArgs& rArgs );
	virtual void		DrawText( SalGraphics& rGraphics ) const;
	virtual bool		GetOutline( SalGraphics& rGraphics, PolyPolyVector& rVector ) const;

	long				GetBaselineDelta() const { return mnBaselineDelta; }
	void				GetVerticalGlyphTranslation( long nGlyph, double& fX, double& fY ) const;
};

#endif // _SV_SALATSLAYOUT_HXX
