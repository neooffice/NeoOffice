/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Portions of this file are part of the LibreOffice project.
 *
 *   This Source Code Form is subject to the terms of the Mozilla Public
 *   License, v. 2.0. If a copy of the MPL was not distributed with this
 *   file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ************************************************************************/
#ifndef _SWFNTCCH_HXX
#define _SWFNTCCH_HXX

#include <tools/mempool.hxx>

#if SUPD == 310
#define NUM_DEFAULT_VALUES 39
#else	// SUPD == 310
#define NUM_DEFAULT_VALUES 36
#endif	// SUPD == 310

#include "swcache.hxx"
#include "swfont.hxx"

class ViewShell;
class SfxPoolItem;

/*************************************************************************
 *                      class SwFontCache
 *************************************************************************/

class SwFontCache : public SwCache
{
public:

	inline SwFontCache() : SwCache(50,50
#ifndef PRODUCT
	, "Globaler AttributSet/Font-Cache pSwFontCache"
#endif
	) {}

};

// AttributSet/Font-Cache, globale Variable, in FontCache.Cxx angelegt
extern SwFontCache *pSwFontCache;

/*************************************************************************
 *                      class SwFontObj
 *************************************************************************/

class SwFontObj : public SwCacheObj
{
	friend class SwFontAccess;

private:
	SwFont aSwFont;
    const SfxPoolItem* pDefaultArray[ NUM_DEFAULT_VALUES ];

public:
	DECL_FIXEDMEMPOOL_NEWDEL(SwFontObj)

	SwFontObj( const void* pOwner, ViewShell *pSh );

	virtual ~SwFontObj();

	inline 		 SwFont *GetFont()		{ return &aSwFont; }
	inline const SwFont *GetFont() const  { return &aSwFont; }
    inline const SfxPoolItem** GetDefault() { return pDefaultArray; }
};

/*************************************************************************
 *                      class SwFontAccess
 *************************************************************************/


class SwFontAccess : public SwCacheAccess
{
	ViewShell *pShell;
protected:
	virtual SwCacheObj *NewObj( );

public:
	SwFontAccess( const void *pOwner, ViewShell *pSh );
	SwFontObj *Get();
};

#endif
