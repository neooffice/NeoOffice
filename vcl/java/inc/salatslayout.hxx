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

namespace vcl
{
	class PDFWriterImpl;
}

// ----------------
// - SalATSLayout -
// ----------------

class SalATSLayout : public GenericSalLayout
{
    friend class		::vcl::PDFWriterImpl;

	::vcl::com_sun_star_vcl_VCLFont*	mpVCLFont;
	bool				mbUseScreenMetrics;
	ATSUStyle			maFontStyle;
	int					mnGlyphCount;
	ATSUGlyphInfoArray*	mpGlyphInfoArray;
	long*				mpGlyphTranslations;
	int*				mpCharsToGlyphs;
	int*				mpVerticalFlags;

	void				Destroy();

public:
						SalATSLayout( ::vcl::com_sun_star_vcl_VCLFont *pVCLFont, bool bUseScreenMetrics );
	virtual				~SalATSLayout();

	virtual bool		LayoutText( ImplLayoutArgs& rArgs );
	virtual void		AdjustLayout( ImplLayoutArgs& rArgs );
	virtual void		DrawText( SalGraphics& rGraphics ) const;
	virtual int			GetNextGlyphs( int nLen, long *pGlyphIdxAry, Point& rPos, int& rStart, long *pGlyphAdvAry = NULL, int *pCharPosAry = NULL ) const;
};

#endif // _SV_SALATSLAYOUT_HXX
