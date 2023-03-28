/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SV_SALATSLAYOUT_HXX
#define _SV_SALATSLAYOUT_HXX

#include <vector>

#include <boost/unordered_map.hpp>

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

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
	static void			GetGlyphBounds( sal_Int32 nGlyph, JavaImplFont *pFont, Rectangle &rRect );
	static void			ClearLayoutDataCache();

						SalATSLayout( JavaSalGraphics *pGraphics, int nFallbackLevel );
	virtual				~SalATSLayout();

	virtual void		AdjustLayout( ImplLayoutArgs& rArgs ) SAL_OVERRIDE;
	virtual bool		LayoutText( ImplLayoutArgs& rArgs ) SAL_OVERRIDE;
	virtual void		DrawText( SalGraphics& rGraphics ) const SAL_OVERRIDE;
	virtual bool		GetBoundRect( SalGraphics& rGraphics, Rectangle& rRect ) const SAL_OVERRIDE;
	virtual bool		GetOutline( SalGraphics& rGraphics, ::basegfx::B2DPolyPolygonVector& rVector ) const SAL_OVERRIDE;

	ImplATSLayoutData*	GetVerticalGlyphTranslation( sal_Int32 nGlyph, int nCharPos, DeviceCoordinate& fX, DeviceCoordinate& fY ) const;
	DeviceCoordinate	GetNativeGlyphWidth( sal_Int32 nGlyph, int nCharPos ) const;

	void				Destroy();
};

#endif // _SV_SALATSLAYOUT_HXX
