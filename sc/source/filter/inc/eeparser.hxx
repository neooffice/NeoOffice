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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_EEPARSER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_EEPARSER_HXX

#include <tools/gen.hxx>
#include <vcl/graph.hxx>
#include <svl/itemset.hxx>
#include <editeng/editdata.hxx>
#include <address.hxx>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>

const sal_Char nHorizontal = 1;
const sal_Char nVertical = 2;
const sal_Char nHoriVerti = nHorizontal | nVertical;

struct ScHTMLImage
{
    OUString       aURL;
    Size                aSize;
    Point               aSpace;
    OUString       aFilterName;
    Graphic*            pGraphic;       // wird von WriteToDocument uebernommen
    sal_Char            nDir;           // 1==hori, 2==verti, 3==beides

    ScHTMLImage() :
        aSize( 0, 0 ), aSpace( 0, 0 ), pGraphic( NULL ),
        nDir( nHorizontal )
        {}

    ~ScHTMLImage() { delete pGraphic; }
};

struct ScEEParseEntry
{
    SfxItemSet          aItemSet;
    ESelection          aSel;           // Selection in EditEngine
    OUString*      pValStr;        // HTML evtl. SDVAL String
    OUString*      pNumStr;        // HTML evtl. SDNUM String
    OUString*      pName;          // HTML evtl. Anchor/RangeName
    OUString       aAltText;       // HTML IMG ALT Text
    boost::ptr_vector< ScHTMLImage > maImageList;       // Grafiken in dieser Zelle
    SCCOL               nCol;           // relativ zum Beginn des Parse
    SCROW               nRow;
    sal_uInt16          nTab;           // HTML TableInTable
    sal_uInt16          nTwips;         // RTF ColAdjust etc.
    SCCOL               nColOverlap;    // merged cells wenn >1
    SCROW               nRowOverlap;    // merged cells wenn >1
    sal_uInt16          nOffset;        // HTML PixelOffset
    sal_uInt16          nWidth;         // HTML PixelWidth
    bool                bHasGraphic:1;  // HTML any image loaded
    bool                bEntirePara:1;  // true = use entire paragraph, false = use selection

    ScEEParseEntry( SfxItemPool* pPool ) :
        aItemSet( *pPool ), pValStr( NULL ),
        pNumStr( NULL ), pName( NULL ),
        nCol(SCCOL_MAX), nRow(SCROW_MAX), nTab(0),
        nTwips(0), nColOverlap(1), nRowOverlap(1),
        nOffset(0), nWidth(0), bHasGraphic(false), bEntirePara(true)
        {}

    ScEEParseEntry( const SfxItemSet& rItemSet ) :
        aItemSet( rItemSet ), pValStr( NULL ),
        pNumStr( NULL ), pName( NULL ),
        nCol(SCCOL_MAX), nRow(SCROW_MAX), nTab(0),
        nTwips(0), nColOverlap(1), nRowOverlap(1),
        nOffset(0), nWidth(0), bHasGraphic(false), bEntirePara(true)
        {}

    ~ScEEParseEntry()
    {
        delete pValStr;
        delete pNumStr;
        delete pName;
        maImageList.clear();
    }
};

class EditEngine;

typedef std::map<SCCOL, sal_uInt16> ColWidthsMap;

class ScEEParser
{
protected:
    EditEngine*         pEdit;
    SfxItemPool*        pPool;
    SfxItemPool*        pDocPool;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    ::std::vector< ScEEParseEntry* > maList;
    ScEEParseEntry*     pActEntry;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::vector<std::shared_ptr<ScEEParseEntry>> maList;
    std::shared_ptr<ScEEParseEntry> mxActEntry;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    ColWidthsMap        maColWidths;
    int                 nLastToken;
    SCCOL               nColCnt;
    SCROW               nRowCnt;
    SCCOL               nColMax;
    SCROW               nRowMax;

    void                NewActEntry( ScEEParseEntry* );

public:
                        ScEEParser( EditEngine* );
    virtual             ~ScEEParser();

    virtual sal_uLong       Read( SvStream&, const OUString& rBaseURL ) = 0;

    const ColWidthsMap&     GetColWidths() const { return maColWidths; }
    ColWidthsMap&           GetColWidths() { return maColWidths; }
    void                    GetDimensions( SCCOL& nCols, SCROW& nRows ) const
                                { nCols = nColMax; nRows = nRowMax; }

    inline size_t           ListSize() const{ return maList.size(); }
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    ScEEParseEntry*         ListEntry( size_t index ) { return maList[ index ]; }
    const ScEEParseEntry*   ListEntry( size_t index ) const { return maList[ index ]; }
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    ScEEParseEntry*         ListEntry( size_t index ) { return maList[index].get(); }
    const ScEEParseEntry*   ListEntry( size_t index ) const { return maList[index].get(); }
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
