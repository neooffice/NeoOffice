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

struct ImplATSLayoutData;
class SalGraphics;

// ----------------
// - SalATSLayout -
// ----------------

class SalATSLayout : public GenericSalLayout
{
	SalGraphics*		mpGraphics;
	int					mnFallbackLevel;
	::vcl::com_sun_star_vcl_VCLFont*	mpVCLFont;
	mutable ImplLayoutRuns	maRuns;
	::std::vector< ImplATSLayoutData* >	maLayoutData;
	ImplATSLayoutData*	mpKashidaLayoutData;

public:
						SalATSLayout( SalGraphics *pGraphics, int nFallbackLevel );
	virtual				~SalATSLayout();

	virtual void		AdjustLayout( ImplLayoutArgs& rArgs );
	virtual bool		LayoutText( ImplLayoutArgs& rArgs );
	virtual void		DrawText( SalGraphics& rGraphics ) const;
	virtual bool		GetOutline( SalGraphics& rGraphics, PolyPolyVector& rVector ) const;

	void				Destroy();
	long				GetBaselineDelta() const;
	void				GetVerticalGlyphTranslation( long nGlyph, long& nX, long& nY ) const;
};

#endif // _SV_SALATSLAYOUT_HXX
