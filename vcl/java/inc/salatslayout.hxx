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

#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>
#undef check

struct ImplATSLayoutData;
class JavaSalGraphics;

// ----------------
// - SalATSLayout -
// ----------------

class SalATSLayout : public GenericSalLayout
{
	JavaSalGraphics*	mpGraphics;
	int					mnFallbackLevel;
	::vcl::com_sun_star_vcl_VCLFont*	mpVCLFont;
	mutable ImplLayoutRuns	maRuns;
	::std::vector< ImplATSLayoutData* >	maLayoutData;
	::std::vector< int >	maLayoutMinCharPos;
	ImplATSLayoutData*	mpKashidaLayoutData;
	::std::hash_map< sal_Unicode, ImplATSLayoutData* >	maMirroredLayoutData;
	long				mnOrigWidth;
	float				mfGlyphScaleX;

public:
	static void			SetFontFallbacks();
	static void			ClearLayoutDataCache();

						SalATSLayout( JavaSalGraphics *pGraphics, int nFallbackLevel );
	virtual				~SalATSLayout();

	virtual void		AdjustLayout( ImplLayoutArgs& rArgs );
	virtual bool		LayoutText( ImplLayoutArgs& rArgs );
	virtual void		DrawText( SalGraphics& rGraphics ) const;
	virtual bool		GetOutline( SalGraphics& rGraphics, ::basegfx::B2DPolyPolygonVector& rVector ) const;

	ImplATSLayoutData*	GetVerticalGlyphTranslation( sal_Int32 nGlyph, int nCharPos, long& nX, long& nY ) const;

	void				Destroy();
};

#endif // _SV_SALATSLAYOUT_HXX
