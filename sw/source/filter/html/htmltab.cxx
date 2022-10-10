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

#include "hintids.hxx"
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/spltitem.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <svl/urihelper.hxx>
#ifndef NO_LIBO_HTML_TABLE_LEAK_FIX
#include <o3tl/make_unique.hxx>
#endif	// !NO_LIBO_HTML_TABLE_LEAK_FIX

#include <fmtornt.hxx>
#include <frmfmt.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include <fmtpdsc.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <fmtlsplt.hxx>
#include "frmatr.hxx"
#include "pam.hxx"
#include "doc.hxx"
#include <IDocumentLayoutAccess.hxx>
#include "ndtxt.hxx"
#include "shellio.hxx"
#include "poolfmt.hxx"
#include "swtable.hxx"
#include "cellatr.hxx"
#include "htmltbl.hxx"
#include "swtblfmt.hxx"
#include "htmlnum.hxx"
#include "swhtml.hxx"
#include "swcss1.hxx"
#include <numrule.hxx>

#define NETSCAPE_DFLT_BORDER 1
#define NETSCAPE_DFLT_CELLSPACING 2

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;

static HTMLOptionEnum aHTMLTblVAlignTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_VA_top,         text::VertOrientation::NONE       },
    { OOO_STRING_SVTOOLS_HTML_VA_middle,      text::VertOrientation::CENTER     },
    { OOO_STRING_SVTOOLS_HTML_VA_bottom,      text::VertOrientation::BOTTOM     },
    { 0,                    0               }
};

// table tags options

struct HTMLTableOptions
{
    sal_uInt16 nCols;
    sal_uInt16 nWidth;
    sal_uInt16 nHeight;
    sal_uInt16 nCellPadding;
    sal_uInt16 nCellSpacing;
    sal_uInt16 nBorder;
    sal_uInt16 nHSpace;
    sal_uInt16 nVSpace;

    SvxAdjust eAdjust;
    sal_Int16 eVertOri;
    HTMLTableFrame eFrame;
    HTMLTableRules eRules;

    bool bPrcWidth : 1;
    bool bTableAdjust : 1;
    bool bBGColor : 1;

    Color aBorderColor;
    Color aBGColor;

    OUString aBGImage, aStyle, aId, aClass, aDir;

    HTMLTableOptions( const HTMLOptions& rOptions, SvxAdjust eParentAdjust );
};

class _HTMLTableContext
{
    SwHTMLNumRuleInfo aNumRuleInfo; // Numbering valid before the table

    SwTableNode *pTblNd;            // table node
    SwFrmFmt *pFrmFmt;              // der Fly frame::Frame, containing the table
    SwPosition *pPos;               // position behind the table

    sal_uInt16 nContextStAttrMin;
    sal_uInt16 nContextStMin;

    bool    bRestartPRE : 1;
    bool    bRestartXMP : 1;
    bool    bRestartListing : 1;

public:

    _HTMLAttrTable aAttrTab;        // attributes

    _HTMLTableContext( SwPosition *pPs, sal_uInt16 nCntxtStMin,
                       sal_uInt16 nCntxtStAttrMin ) :
        pTblNd( 0 ),
        pFrmFmt( 0 ),
        pPos( pPs ),
        nContextStAttrMin( nCntxtStAttrMin ),
        nContextStMin( nCntxtStMin ),
        bRestartPRE( false ),
        bRestartXMP( false ),
        bRestartListing( false )
    {
        memset( &aAttrTab, 0, sizeof( _HTMLAttrTable ));
    }

    ~_HTMLTableContext();

    void SetNumInfo( const SwHTMLNumRuleInfo& rInf ) { aNumRuleInfo.Set(rInf); }
    const SwHTMLNumRuleInfo& GetNumInfo() const { return aNumRuleInfo; };

    void SavePREListingXMP( SwHTMLParser& rParser );
    void RestorePREListingXMP( SwHTMLParser& rParser );

    SwPosition *GetPos() const { return pPos; }

    void SetTableNode( SwTableNode *pNd ) { pTblNd = pNd; }
    SwTableNode *GetTableNode() const { return pTblNd; }

    void SetFrmFmt( SwFrmFmt *pFmt ) { pFrmFmt = pFmt; }
    SwFrmFmt *GetFrmFmt() const { return pFrmFmt; }

    sal_uInt16 GetContextStMin() const { return nContextStMin; }
    sal_uInt16 GetContextStAttrMin() const { return nContextStAttrMin; }
};

// Cell content is a linked list with SwStartNodes and
// HTMLTables.

class HTMLTableCnts
{
    HTMLTableCnts *pNext;               // next content

    // Only one of the next two pointers must be set!
    const SwStartNode *pStartNode;      // a paragraph
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTable *pTable;                  // a table

    SwHTMLTableLayoutCnts* pLayoutInfo;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::shared_ptr<HTMLTable> m_xTable;                  // a table

    std::shared_ptr<SwHTMLTableLayoutCnts> m_xLayoutInfo;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    bool bNoBreak;

    void InitCtor();

public:

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableCnts( const SwStartNode* pStNd );
    HTMLTableCnts( HTMLTable* pTab );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    explicit HTMLTableCnts(const SwStartNode* pStNd);
    explicit HTMLTableCnts(const std::shared_ptr<HTMLTable>& rTab);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    ~HTMLTableCnts();                   // only allowed in ~HTMLTableCell

    // Determine SwStartNode and HTMLTable respectively
    const SwStartNode *GetStartNode() const { return pStartNode; }
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    const HTMLTable *GetTable() const { return pTable; }
    HTMLTable *GetTable() { return pTable; }
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    const std::shared_ptr<HTMLTable>& GetTable() const { return m_xTable; }
    std::shared_ptr<HTMLTable>& GetTable() { return m_xTable; }
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    // Add a new node at the end of the list
    void Add( HTMLTableCnts* pNewCnts );

    // Determine next node
    const HTMLTableCnts *Next() const { return pNext; }
    HTMLTableCnts *Next() { return pNext; }

    inline void SetTableBox( SwTableBox *pBox );

    void SetNoBreak() { bNoBreak = true; }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    SwHTMLTableLayoutCnts *CreateLayoutInfo();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    const std::shared_ptr<SwHTMLTableLayoutCnts>& CreateLayoutInfo();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
};

// Cell of a HTML table
class HTMLTableCell
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    // !!!ATTENTION!!!!! For each new pointer the SetProtected
    // method (and the dtor) has to be executed.
    HTMLTableCnts *pContents;       // cell content
    SvxBrushItem *pBGBrush;         // cell background
    // !!!ATTENTION!!!!!
    ::boost::shared_ptr<SvxBoxItem> m_pBoxItem;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::shared_ptr<HTMLTableCnts> m_xContents;       // cell content
    std::shared_ptr<SvxBrushItem> m_xBGBrush;         // cell background
    std::shared_ptr<SvxBoxItem> m_xBoxItem;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    sal_uInt32 nNumFmt;
    sal_uInt16 nRowSpan;                // cell ROWSPAN
    sal_uInt16 nColSpan;                // cell COLSPAN
    sal_uInt16 nWidth;                  // cell WIDTH
    double nValue;
    sal_Int16 eVertOri;         // vertical alignment of the cell
    bool bProtected : 1;            // cell must not filled
    bool bRelWidth : 1;             // nWidth is given in %
    bool bHasNumFmt : 1;
    bool bHasValue : 1;
    bool bNoWrap : 1;
    bool mbCovered : 1;

public:

    HTMLTableCell();                // new cells always empty

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    ~HTMLTableCell();               // only allowed in ~HTMLTableRow
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    // Fill a not empty cell
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    void Set( HTMLTableCnts *pCnts, sal_uInt16 nRSpan, sal_uInt16 nCSpan,
              sal_Int16 eVertOri, SvxBrushItem *pBGBrush,
              ::boost::shared_ptr<SvxBoxItem> const pBoxItem,
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    void Set( std::shared_ptr<HTMLTableCnts> const& rCnts, sal_uInt16 nRSpan, sal_uInt16 nCSpan,
              sal_Int16 eVertOri, std::shared_ptr<SvxBrushItem> const& rBGBrush,
              std::shared_ptr<SvxBoxItem> const& rBoxItem,
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
              bool bHasNumFmt, sal_uInt32 nNumFmt,
              bool bHasValue, double nValue, bool bNoWrap, bool bCovered );

    // Protect an empty 1x1 cell
    void SetProtected();

    // Set/Get cell content
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    void SetContents( HTMLTableCnts *pCnts ) { pContents = pCnts; }
    HTMLTableCnts *GetContents() { return pContents; }
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    void SetContents(std::shared_ptr<HTMLTableCnts> const& rCnts) { m_xContents = rCnts; }
    const std::shared_ptr<HTMLTableCnts>& GetContents() const { return m_xContents; }
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    // Set/Get cell ROWSPAN/COLSPAN
    void SetRowSpan( sal_uInt16 nRSpan ) { nRowSpan = nRSpan; }
    sal_uInt16 GetRowSpan() const { return nRowSpan; }

    void SetColSpan( sal_uInt16 nCSpan ) { nColSpan = nCSpan; }
    sal_uInt16 GetColSpan() const { return nColSpan; }

    inline void SetWidth( sal_uInt16 nWidth, bool bRelWidth );

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    const SvxBrushItem *GetBGBrush() const { return pBGBrush; }
    ::boost::shared_ptr<SvxBoxItem> GetBoxItem() const { return m_pBoxItem; }
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    const std::shared_ptr<SvxBrushItem>& GetBGBrush() const { return m_xBGBrush; }
    const std::shared_ptr<SvxBoxItem>& GetBoxItem() const { return m_xBoxItem; }
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    inline bool GetNumFmt( sal_uInt32& rNumFmt ) const;
    inline bool GetValue( double& rValue ) const;

    sal_Int16 GetVertOri() const { return eVertOri; }

    // Is the cell filled or protected ?
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    bool IsUsed() const { return pContents!=0 || bProtected; }

    SwHTMLTableLayoutCell *CreateLayoutInfo();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    bool IsUsed() const { return m_xContents || bProtected; }

    std::unique_ptr<SwHTMLTableLayoutCell> CreateLayoutInfo();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    bool IsCovered() const { return mbCovered; }
};

// Row of a HTML table
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
typedef boost::ptr_vector<HTMLTableCell> HTMLTableCells;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
typedef std::vector<std::unique_ptr<HTMLTableCell>> HTMLTableCells;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

class HTMLTableRow
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableCells *pCells;             // cells of the row
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::unique_ptr<HTMLTableCells> m_xCells;   ///< cells of the row
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    bool bIsEndOfGroup : 1;

    sal_uInt16 nHeight;                     // options of <TR>/<TD>
    sal_uInt16 nEmptyRows;                  // number of empty rows are following

    SvxAdjust eAdjust;
    sal_Int16 eVertOri;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    SvxBrushItem *pBGBrush;             // background of cell from STYLE
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::unique_ptr<SvxBrushItem> xBGBrush;             // background of cell from STYLE
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

public:

    bool bBottomBorder;                 // Is there a line after the row?

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableRow( sal_uInt16 nCells=0 );    // cells of the row are empty

    ~HTMLTableRow();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    explicit HTMLTableRow( sal_uInt16 nCells ); // cells of the row are empty
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    inline void SetHeight( sal_uInt16 nHeight );
    sal_uInt16 GetHeight() const { return nHeight; }

    inline HTMLTableCell *GetCell( sal_uInt16 nCell ) const;

    inline void SetAdjust( SvxAdjust eAdj ) { eAdjust = eAdj; }
    inline SvxAdjust GetAdjust() const { return eAdjust; }

    inline void SetVertOri( sal_Int16 eV) { eVertOri = eV; }
    inline sal_Int16 GetVertOri() const { return eVertOri; }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    void SetBGBrush( SvxBrushItem *pBrush ) { pBGBrush = pBrush; }
    const SvxBrushItem *GetBGBrush() const { return pBGBrush; }
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    void SetBGBrush(std::unique_ptr<SvxBrushItem>& rBrush ) { xBGBrush = std::move(rBrush); }
    const std::unique_ptr<SvxBrushItem>& GetBGBrush() const { return xBGBrush; }
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    inline void SetEndOfGroup() { bIsEndOfGroup = true; }
    inline bool IsEndOfGroup() const { return bIsEndOfGroup; }

    void IncEmptyRows() { nEmptyRows++; }
    sal_uInt16 GetEmptyRows() const { return nEmptyRows; }

    // Expand row by adding empty cells
    void Expand( sal_uInt16 nCells, bool bOneCell=false );

    // Shrink row by deleting empty cells
    void Shrink( sal_uInt16 nCells );
};

// Column of a HTML table
class HTMLTableColumn
{
    bool bIsEndOfGroup;

    sal_uInt16 nWidth;                      // options of <COL>
    bool bRelWidth;

    SvxAdjust eAdjust;
    sal_Int16 eVertOri;

    SwFrmFmt *aFrmFmts[6];

    inline sal_uInt16 GetFrmFmtIdx( bool bBorderLine,
                                sal_Int16 eVertOri ) const;

public:

    bool bLeftBorder;                   // is there a line before the column

    HTMLTableColumn();

    inline void SetWidth( sal_uInt16 nWidth, bool bRelWidth);

    inline void SetAdjust( SvxAdjust eAdj ) { eAdjust = eAdj; }
    inline SvxAdjust GetAdjust() const { return eAdjust; }

    inline void SetVertOri( sal_Int16 eV) { eVertOri = eV; }
    inline sal_Int16 GetVertOri() const { return eVertOri; }

    inline void SetEndOfGroup() { bIsEndOfGroup = true; }
    inline bool IsEndOfGroup() const { return bIsEndOfGroup; }

    inline void SetFrmFmt( SwFrmFmt *pFmt, bool bBorderLine,
                           sal_Int16 eVertOri );
    inline SwFrmFmt *GetFrmFmt( bool bBorderLine,
                                sal_Int16 eVertOri ) const;

    SwHTMLTableLayoutColumn *CreateLayoutInfo();
};

// HTML table
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
typedef boost::ptr_vector<HTMLTableRow> HTMLTableRows;

typedef boost::ptr_vector<HTMLTableColumn> HTMLTableColumns;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
typedef std::vector<std::unique_ptr<HTMLTableRow>> HTMLTableRows;

typedef std::vector<std::unique_ptr<HTMLTableColumn>> HTMLTableColumns;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

typedef std::vector<SdrObject *> SdrObjects;

class HTMLTable
{
    OUString aId;
    OUString aStyle;
    OUString aClass;
    OUString aDir;

    SdrObjects *pResizeDrawObjs;// SDR objects
    std::vector<sal_uInt16> *pDrawObjPrcWidths;   // column of draw object and its rel. width

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableRows *pRows;           // table rows
    HTMLTableColumns *pColumns;     // table columns
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableRows *m_pRows;         ///< table rows
    HTMLTableColumns *m_pColumns;   ///< table columns
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    sal_uInt16 nRows;                   // number of rows
    sal_uInt16 nCols;                   // number of columns
    sal_uInt16 nFilledCols;             // number of filled columns

    sal_uInt16 nCurRow;                 // current Row
    sal_uInt16 nCurCol;                 // current Column

    sal_uInt16 nLeftMargin;             // Space to the left margin (from paragraph edge)
    sal_uInt16 nRightMargin;            // Space to the right margin (from paragraph edge)

    sal_uInt16 nCellPadding;            // Space from border to Text
    sal_uInt16 nCellSpacing;            // Space between two cells
    sal_uInt16 nHSpace;
    sal_uInt16 nVSpace;

    sal_uInt16 nBoxes;                  // number of boxes in the table

    const SwStartNode *pPrevStNd;   // the Table-Node or the Start-Node of the section before
    const SwTable *pSwTable;        // SW-Table (only on Top-Level)
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    SwTableBox *pBox1;              // TableBox, generated when the Top-Level-Table was build
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
public:
    std::unique_ptr<SwTableBox> m_xBox1;    // TableBox, generated when the Top-Level-Table was build
private:
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    SwTableBoxFmt *pBoxFmt;         // frame::Frame-Format from SwTableBox
    SwTableLineFmt *pLineFmt;       // frame::Frame-Format from SwTableLine
    SwTableLineFmt *pLineFrmFmtNoHeight;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    SvxBrushItem *pBGBrush;         // background of the table
    SvxBrushItem *pInhBGBrush;      // "inherited" background of the table
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::unique_ptr<SvxBrushItem> m_xBackgroundBrush;          // background of the table
    std::unique_ptr<SvxBrushItem> m_xInheritedBackgroundBrush; // "inherited" background of the table
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    const SwStartNode *pCaptionStartNode;   // Start-Node of the table-caption
    //lines for the border
    SvxBorderLine aTopBorderLine;
    SvxBorderLine aBottomBorderLine;
    SvxBorderLine aLeftBorderLine;
    SvxBorderLine aRightBorderLine;
    SvxBorderLine aBorderLine;
    SvxBorderLine aInhLeftBorderLine;
    SvxBorderLine aInhRightBorderLine;
    bool bTopBorder;                // is there a line on the top of the table
    bool bRightBorder;              // is there a line on the top right of the table
    bool bTopAlwd;                  // is it allowed to set the border?
    bool bRightAlwd;
    bool bFillerTopBorder;          // gets the left/right filler-cell a border on the
    bool bFillerBottomBorder;       // top or in the bottom
    bool bInhLeftBorder;
    bool bInhRightBorder;
    bool bBordersSet;               // the border is setted already
    bool bForceFrame;
    bool bTableAdjustOfTag;         // comes nTableAdjust from <TABLE>?
    sal_uInt32 nHeadlineRepeat;         // repeating rows
    bool bIsParentHead;
    bool bHasParentSection;
    bool bHasToFly;
    bool bFixedCols;
    bool bColSpec;                  // where there COL(GROUP)-elements?
    bool bPrcWidth;                 // width is declarated in %

    SwHTMLParser *pParser;          // the current parser
    HTMLTable *pTopTable;           // the table on the Top-Level
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableCnts *pParentContents;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::unique_ptr<HTMLTableCnts> m_xParentContents;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    _HTMLTableContext *pContext;    // the context of the table

    SwHTMLTableLayout *pLayoutInfo;
#ifndef NO_LIBO_MODIFIED_HTML_TABLE_LEAK_FIX
    bool bLayoutInfoOwner;
#endif	// !NO_LIBO_MODIFIED_HTML_TABLE_LEAK_FIX

    // the following parameters are from the <TABLE>-Tag
    sal_uInt16 nWidth;                  // width of the table
    sal_uInt16 nHeight;                 // absolute height of the table
    SvxAdjust eTableAdjust;         // drawing::Alignment of the table
    sal_Int16 eVertOri;         // Default vertical direction of the cells
    sal_uInt16 nBorder;                 // width of the external border
    HTMLTableFrame eFrame;          // frame around the table
    HTMLTableRules eRules;          // frame in the table
    bool bTopCaption;               // Caption of the table

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    void InitCtor( const HTMLTableOptions *pOptions );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    void InitCtor(const HTMLTableOptions& rOptions);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    // Correction of the Row-Spans for all cells above the chosen cell and the cell itself for the indicated content. The chosen cell gets the Row-Span 1
    void FixRowSpan( sal_uInt16 nRow, sal_uInt16 nCol, const HTMLTableCnts *pCnts );

    // Protects the chosen cell and the cells among
    void ProtectRowSpan( sal_uInt16 nRow, sal_uInt16 nCol, sal_uInt16 nRowSpan );

    // Looking for the SwStartNodes of the box ahead
    // If nRow==nCell==USHRT_MAX, return the last Start-Node of the table.
    const SwStartNode* GetPrevBoxStartNode( sal_uInt16 nRow, sal_uInt16 nCell ) const;

    sal_uInt16 GetTopCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                            bool bSwBorders=true ) const;
    sal_uInt16 GetBottomCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                               bool bSwBorders=true ) const;

    // Conforming of the frame::Frame-Format of the box
    void FixFrameFmt( SwTableBox *pBox, sal_uInt16 nRow, sal_uInt16 nCol,
                      sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                      bool bFirstPara=true, bool bLastPara=true ) const;
    void FixFillerFrameFmt( SwTableBox *pBox, bool bRight ) const;

    // Create a table with the content (lines/boxes)
    void _MakeTable( SwTableBox *pUpper=0 );

    // Gernerate a new SwTableBox, which contains a SwStartNode
    SwTableBox *NewTableBox( const SwStartNode *pStNd,
                             SwTableLine *pUpper ) const;

    // Generate a SwTableLine from the cells of the rectangle
    // (nTopRow/nLeftCol) inclusive to (nBottomRow/nRightRow) exclusive
    SwTableLine *MakeTableLine( SwTableBox *pUpper,
                                sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                                sal_uInt16 nBottomRow, sal_uInt16 nRightCol );

    // Generate a SwTableBox from the content of the cell
    SwTableBox *MakeTableBox( SwTableLine *pUpper,
                              HTMLTableCnts *pCnts,
                              sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                              sal_uInt16 nBootomRow, sal_uInt16 nRightCol );

    // Autolayout-Algorithm

    // Setting the border with the help of guidelines of the Parent-Table
    void InheritBorders( const HTMLTable *pParent,
                         sal_uInt16 nRow, sal_uInt16 nCol,
                         sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                         bool bFirstPara, bool bLastPara );

    // Inherit the left and the right border of the surrounding table
    void InheritVertBorders( const HTMLTable *pParent,
                             sal_uInt16 nCol, sal_uInt16 nColSpan );

    // Set the border with the help of the information from the user
    void SetBorders();

    // is the border already setted?
    bool BordersSet() const { return bBordersSet; }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    const SvxBrushItem *GetBGBrush() const { return pBGBrush; }
    const SvxBrushItem *GetInhBGBrush() const { return pInhBGBrush; }
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    const std::unique_ptr<SvxBrushItem>& GetBGBrush() const { return m_xBackgroundBrush; }
    const std::unique_ptr<SvxBrushItem>& GetInhBGBrush() const { return m_xInheritedBackgroundBrush; }
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    sal_uInt16 GetBorderWidth( const SvxBorderLine& rBLine,
                           bool bWithDistance=false ) const;

public:

    bool bFirstCell;                // wurde schon eine Zelle angelegt?

    HTMLTable( SwHTMLParser* pPars, HTMLTable *pTopTab,
               bool bParHead, bool bHasParentSec,
               bool bHasToFly,
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
               const HTMLTableOptions *pOptions );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
              const HTMLTableOptions& rOptions);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    ~HTMLTable();

    // Identifying of a cell
    inline HTMLTableCell *GetCell( sal_uInt16 nRow, sal_uInt16 nCell ) const;

    // set/determine caption
    inline void SetCaption( const SwStartNode *pStNd, bool bTop );
    const SwStartNode *GetCaptionStartNode() const { return pCaptionStartNode; }
    bool IsTopCaption() const { return bTopCaption; }

    SvxAdjust GetTableAdjust( bool bAny ) const
    {
        return (bTableAdjustOfTag || bAny) ? eTableAdjust : SVX_ADJUST_END;
    }

    sal_uInt16 GetHSpace() const { return nHSpace; }
    sal_uInt16 GetVSpace() const { return nVSpace; }

    // get inherited drawing::Alignment of rows and column
    SvxAdjust GetInheritedAdjust() const;
    sal_Int16 GetInheritedVertOri() const;

    // Insert a cell on the current position
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    void InsertCell( HTMLTableCnts *pCnts, sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    void InsertCell( std::shared_ptr<HTMLTableCnts> const& rCnts, sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                     sal_uInt16 nWidth, bool bRelWidth, sal_uInt16 nHeight,
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                     sal_Int16 eVertOri, SvxBrushItem *pBGBrush,
                     boost::shared_ptr<SvxBoxItem> const pBoxItem,
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                     sal_Int16 eVertOri, std::shared_ptr<SvxBrushItem> const& rBGBrushItem,
                     std::shared_ptr<SvxBoxItem> const& rBoxItem,
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                     bool bHasNumFmt, sal_uInt32 nNumFmt,
                     bool bHasValue, double nValue, bool bNoWrap );

    // announce the start/end of a new row
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    void OpenRow( SvxAdjust eAdjust, sal_Int16 eVertOri,
                  SvxBrushItem *pBGBrush );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    void OpenRow(SvxAdjust eAdjust, sal_Int16 eVertOri, std::unique_ptr<SvxBrushItem>& rBGBrush);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    void CloseRow( bool bEmpty );

    // announce the end of a new section
    inline void CloseSection( bool bHead );

    // announce the end of a column-group
    inline void CloseColGroup( sal_uInt16 nSpan, sal_uInt16 nWidth, bool bRelWidth,
                               SvxAdjust eAdjust, sal_Int16 eVertOri );

    // insert a new column
    void InsertCol( sal_uInt16 nSpan, sal_uInt16 nWidth, bool bRelWidth,
                    SvxAdjust eAdjust, sal_Int16 eVertOri );

    // Beenden einer Tab-Definition (MUSS fuer ALLE Tabs aufgerufen werden)
    void CloseTable();

    // SwTable konstruieren (inkl. der Child-Tabellen)
    void MakeTable( SwTableBox *pUpper, sal_uInt16 nAbsAvail,
                    sal_uInt16 nRelAvail=0, sal_uInt16 nAbsLeftSpace=0,
                    sal_uInt16 nAbsRightSpace=0, sal_uInt16 nInhAbsSpace=0 );

    inline bool IsNewDoc() const { return pParser->IsNewDoc(); }

    void SetHasParentSection( bool bSet ) { bHasParentSection = bSet; }
    bool HasParentSection() const { return bHasParentSection; }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    void SetParentContents( HTMLTableCnts *pCnts ) { pParentContents = pCnts; }
    HTMLTableCnts *GetParentContents() const { return pParentContents; }
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    void SetParentContents(HTMLTableCnts *pCnts) { m_xParentContents.reset(pCnts); }
    std::unique_ptr<HTMLTableCnts>& GetParentContents() { return m_xParentContents; }
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    void MakeParentContents();

    bool GetIsParentHeader() const { return bIsParentHead; }

    bool HasToFly() const { return bHasToFly; }

    void SetTable( const SwStartNode *pStNd, _HTMLTableContext *pCntxt,
                   sal_uInt16 nLeft, sal_uInt16 nRight,
                   const SwTable *pSwTab=0, bool bFrcFrame=false );

    _HTMLTableContext *GetContext() const { return pContext; }

    SwHTMLTableLayout *CreateLayoutInfo();

    bool HasColTags() const { return bColSpec; }

    sal_uInt16 IncGrfsThatResize() { return pSwTable ? ((SwTable *)pSwTable)->IncGrfsThatResize() : 0; }

    void RegisterDrawObject( SdrObject *pObj, sal_uInt8 nPrcWidth );

    const SwTable *GetSwTable() const { return pSwTable; }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    void SetBGBrush( const SvxBrushItem& rBrush ) { delete pBGBrush; pBGBrush = new SvxBrushItem( rBrush ); }
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    void SetBGBrush(const SvxBrushItem& rBrush) { m_xBackgroundBrush.reset(new SvxBrushItem(rBrush)); }
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    const OUString& GetId() const { return aId; }
    const OUString& GetClass() const { return aClass; }
    const OUString& GetStyle() const { return aStyle; }
    const OUString& GetDirection() const { return aDir; }

    void IncBoxCount() { nBoxes++; }
    bool IsOverflowing() const { return nBoxes > 64000; }
};

void HTMLTableCnts::InitCtor()
{
    pNext = 0;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pLayoutInfo = 0;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    m_xLayoutInfo.reset();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    bNoBreak = false;
}

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
HTMLTableCnts::HTMLTableCnts( const SwStartNode* pStNd ):
    pStartNode(pStNd), pTable(0)
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
HTMLTableCnts::HTMLTableCnts(const SwStartNode* pStNd)
    : pStartNode(pStNd)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
{
    InitCtor();
}

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
HTMLTableCnts::HTMLTableCnts( HTMLTable* pTab ):
    pStartNode(0), pTable(pTab)
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
HTMLTableCnts::HTMLTableCnts(const std::shared_ptr<HTMLTable>& rTab)
    : pStartNode(nullptr)
    , m_xTable(rTab)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
{
    InitCtor();
}

HTMLTableCnts::~HTMLTableCnts()
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    delete pTable;              // die Tabellen brauchen wir nicht mehr
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    m_xTable.reset();    // we don't need the tables anymore
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    delete pNext;
}

void HTMLTableCnts::Add( HTMLTableCnts* pNewCnts )
{
    HTMLTableCnts *pCnts = this;

    while( pCnts->pNext )
        pCnts = pCnts->pNext;

    pCnts->pNext = pNewCnts;
}

inline void HTMLTableCnts::SetTableBox( SwTableBox *pBox )
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    OSL_ENSURE( pLayoutInfo, "Da sit noch keine Layout-Info" );
    if( pLayoutInfo )
        pLayoutInfo->SetTableBox( pBox );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    OSL_ENSURE(m_xLayoutInfo.get(), "There is no layout info");
    if (m_xLayoutInfo)
        m_xLayoutInfo->SetTableBox(pBox);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
}

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
SwHTMLTableLayoutCnts *HTMLTableCnts::CreateLayoutInfo()
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
const std::shared_ptr<SwHTMLTableLayoutCnts>& HTMLTableCnts::CreateLayoutInfo()
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( !pLayoutInfo )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if (!m_xLayoutInfo)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        SwHTMLTableLayoutCnts *pNextInfo = pNext ? pNext->CreateLayoutInfo() : 0;
        SwHTMLTableLayout *pTableInfo = pTable ? pTable->CreateLayoutInfo() : 0;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        std::shared_ptr<SwHTMLTableLayoutCnts> xNextInfo;
        if (pNext)
            xNextInfo = pNext->CreateLayoutInfo();
        SwHTMLTableLayout *pTableInfo = m_xTable ? m_xTable->CreateLayoutInfo() : nullptr;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pLayoutInfo = new SwHTMLTableLayoutCnts( pStartNode, pTableInfo,
                                                 bNoBreak, pNextInfo );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        m_xLayoutInfo.reset(new SwHTMLTableLayoutCnts(pStartNode, pTableInfo, bNoBreak, xNextInfo));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    return pLayoutInfo;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    return m_xLayoutInfo;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
}

HTMLTableCell::HTMLTableCell():
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pContents(0),
    pBGBrush(0),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    nNumFmt(0),
    nRowSpan(1),
    nColSpan(1),
    nWidth( 0 ),
    nValue(0),
    eVertOri( text::VertOrientation::NONE ),
    bProtected(false),
    bRelWidth( false ),
    bHasNumFmt(false),
    bHasValue(false),
    bNoWrap(false),
    mbCovered(false)
{}

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX

HTMLTableCell::~HTMLTableCell()
{
    // der Inhalt ist in mehrere Zellen eingetragen, darf aber nur einmal
    // geloescht werden
    if( 1==nRowSpan && 1==nColSpan )
    {
        delete pContents;
        delete pBGBrush;
    }
}

void HTMLTableCell::Set( HTMLTableCnts *pCnts, sal_uInt16 nRSpan, sal_uInt16 nCSpan,
                         sal_Int16 eVert, SvxBrushItem *pBrush,
                         ::boost::shared_ptr<SvxBoxItem> const pBoxItem,
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
void HTMLTableCell::Set( std::shared_ptr<HTMLTableCnts> const& rCnts, sal_uInt16 nRSpan, sal_uInt16 nCSpan,
                         sal_Int16 eVert, std::shared_ptr<SvxBrushItem> const& rBrush,
                         std::shared_ptr<SvxBoxItem> const& rBoxItem,
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                         bool bHasNF, sal_uInt32 nNF, bool bHasV, double nVal,
                         bool bNWrap, bool bCovered )
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pContents = pCnts;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    m_xContents = rCnts;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    nRowSpan = nRSpan;
    nColSpan = nCSpan;
    bProtected = false;
    eVertOri = eVert;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pBGBrush = pBrush;
    m_pBoxItem = pBoxItem;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    m_xBGBrush = rBrush;
    m_xBoxItem = rBoxItem;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    bHasNumFmt = bHasNF;
    bHasValue = bHasV;
    nNumFmt = nNF;
    nValue = nVal;

    bNoWrap = bNWrap;
    mbCovered = bCovered;
}

inline void HTMLTableCell::SetWidth( sal_uInt16 nWdth, bool bRelWdth )
{
    nWidth = nWdth;
    bRelWidth = bRelWdth;
}

void HTMLTableCell::SetProtected()
{
    // Die Inhalte dieser Zelle mussen nich irgenwo anders verankert
    // sein, weil sie nicht geloescht werden!!!

    // Inhalt loeschen
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pContents = 0;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    m_xContents.reset();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    // Hintergrundfarbe kopieren.
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( pBGBrush )
        pBGBrush = new SvxBrushItem( *pBGBrush );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if (m_xBGBrush)
        m_xBGBrush.reset(new SvxBrushItem(*m_xBGBrush));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    nRowSpan = 1;
    nColSpan = 1;
    bProtected = true;
}

inline bool HTMLTableCell::GetNumFmt( sal_uInt32& rNumFmt ) const
{
    rNumFmt = nNumFmt;
    return bHasNumFmt;
}

inline bool HTMLTableCell::GetValue( double& rValue ) const
{
    rValue = nValue;
    return bHasValue;
}

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
SwHTMLTableLayoutCell *HTMLTableCell::CreateLayoutInfo()
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
std::unique_ptr<SwHTMLTableLayoutCell> HTMLTableCell::CreateLayoutInfo()
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    SwHTMLTableLayoutCnts *pCntInfo = pContents ? pContents->CreateLayoutInfo() : 0;

    return new SwHTMLTableLayoutCell( pCntInfo, nRowSpan, nColSpan, nWidth,
                                      bRelWidth, bNoWrap );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::shared_ptr<SwHTMLTableLayoutCnts> xCntInfo;
    if (m_xContents)
        xCntInfo = m_xContents->CreateLayoutInfo();
    return std::unique_ptr<SwHTMLTableLayoutCell>(new SwHTMLTableLayoutCell(xCntInfo, nRowSpan, nColSpan, nWidth,
                                      bRelWidth, bNoWrap));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
}

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
HTMLTableRow::HTMLTableRow( sal_uInt16 nCells ):
    pCells(new HTMLTableCells),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
HTMLTableRow::HTMLTableRow(sal_uInt16 const nCells)
    : m_xCells(new HTMLTableCells),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    bIsEndOfGroup(false),
    nHeight(0),
    nEmptyRows(0),
    eAdjust(SVX_ADJUST_END),
    eVertOri(text::VertOrientation::TOP),
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pBGBrush(0),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    bBottomBorder(false)
{
    for( sal_uInt16 i=0; i<nCells; i++ )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pCells->push_back( new HTMLTableCell );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        m_xCells->push_back(o3tl::make_unique<HTMLTableCell>());
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    OSL_ENSURE(nCells == pCells->size(),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    OSL_ENSURE(nCells == m_xCells->size(),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            "wrong Cell count in new HTML table row");
}

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX

HTMLTableRow::~HTMLTableRow()
{
    delete pCells;
    delete pBGBrush;
}

#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

inline void HTMLTableRow::SetHeight( sal_uInt16 nHght )
{
    if( nHght > nHeight  )
        nHeight = nHght;
}

inline HTMLTableCell *HTMLTableRow::GetCell( sal_uInt16 nCell ) const
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    OSL_ENSURE( nCell<pCells->size(),
        "ungueltiger Zellen-Index in HTML-Tabellenzeile" );
    return &(*pCells)[nCell];
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    OSL_ENSURE( nCell < m_xCells->size(),
        "invalid cell index in HTML table row" );
    return (*m_xCells)[nCell].get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
}

void HTMLTableRow::Expand( sal_uInt16 nCells, bool bOneCell )
{
    // die Zeile wird mit einer einzigen Zelle aufgefuellt, wenn
    // bOneCell gesetzt ist. Das geht, nur fuer Zeilen, in die keine
    // Zellen mehr eingefuegt werden!

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    sal_uInt16 nColSpan = nCells-pCells->size();
    for( sal_uInt16 i=pCells->size(); i<nCells; i++ )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    sal_uInt16 nColSpan = nCells - m_xCells->size();
    for (sal_uInt16 i = m_xCells->size(); i < nCells; ++i)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableCell *pCell = new HTMLTableCell;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        std::unique_ptr<HTMLTableCell> pCell(new HTMLTableCell);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if( bOneCell )
            pCell->SetColSpan( nColSpan );

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pCells->push_back( pCell );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        m_xCells->push_back(std::move(pCell));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        nColSpan--;
    }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    OSL_ENSURE(nCells == pCells->size(),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    OSL_ENSURE(nCells == m_xCells->size(),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            "wrong Cell count in expanded HTML table row");
}

void HTMLTableRow::Shrink( sal_uInt16 nCells )
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    OSL_ENSURE(nCells < pCells->size(), "number of cells too large");
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    OSL_ENSURE(nCells < m_xCells->size(), "number of cells too large");
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#if OSL_DEBUG_LEVEL > 0
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
     sal_uInt16 nEnd = pCells->size();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
     sal_uInt16 const nEnd = m_xCells->size();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
#endif
    // The colspan of empty cells at the end has to be fixed to the new
    // number of cells.
    sal_uInt16 i=nCells;
    while( i )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableCell *pCell = &(*pCells)[--i];
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableCell *pCell = (*m_xCells)[--i].get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if( !pCell->GetContents() )
        {
#if OSL_DEBUG_LEVEL > 0
            OSL_ENSURE( pCell->GetColSpan() == nEnd - i,
                    "invalid col span for empty cell at row end" );
#endif
            pCell->SetColSpan( nCells-i);
        }
        else
            break;
    }
#if OSL_DEBUG_LEVEL > 0
    for( i=nCells; i<nEnd; i++ )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableCell *pCell = &(*pCells)[i];
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableCell *pCell = (*m_xCells)[i].get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        OSL_ENSURE( pCell->GetRowSpan() == 1,
                "RowSpan von zu loesender Zelle ist falsch" );
        OSL_ENSURE( pCell->GetColSpan() == nEnd - i,
                    "ColSpan von zu loesender Zelle ist falsch" );
        OSL_ENSURE( !pCell->GetContents(), "Zu loeschende Zelle hat Inhalt" );
    }
#endif

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pCells->erase( pCells->begin() + nCells, pCells->end() );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    m_xCells->erase(m_xCells->begin() + nCells, m_xCells->end());
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
}

HTMLTableColumn::HTMLTableColumn():
    bIsEndOfGroup(false),
    nWidth(0), bRelWidth(false),
    eAdjust(SVX_ADJUST_END), eVertOri(text::VertOrientation::TOP),
    bLeftBorder(false)
{
    for( sal_uInt16 i=0; i<6; i++ )
        aFrmFmts[i] = 0;
}

inline void HTMLTableColumn::SetWidth( sal_uInt16 nWdth, bool bRelWdth )
{
    if( bRelWidth==bRelWdth )
    {
        if( nWdth > nWidth )
            nWidth = nWdth;
    }
    else
        nWidth = nWdth;
    bRelWidth = bRelWdth;
}

inline SwHTMLTableLayoutColumn *HTMLTableColumn::CreateLayoutInfo()
{
    return new SwHTMLTableLayoutColumn( nWidth, bRelWidth, bLeftBorder );
}

inline sal_uInt16 HTMLTableColumn::GetFrmFmtIdx( bool bBorderLine,
                                             sal_Int16 eVertOrient ) const
{
    OSL_ENSURE( text::VertOrientation::TOP != eVertOrient, "Top ist nicht erlaubt" );
    sal_uInt16 n = bBorderLine ? 3 : 0;
    switch( eVertOrient )
    {
    case text::VertOrientation::CENTER:   n+=1;   break;
    case text::VertOrientation::BOTTOM:   n+=2;   break;
    default:
        ;
    }
    return n;
}

inline void HTMLTableColumn::SetFrmFmt( SwFrmFmt *pFmt, bool bBorderLine,
                                        sal_Int16 eVertOrient )
{
    aFrmFmts[GetFrmFmtIdx(bBorderLine,eVertOrient)] = pFmt;
}

inline SwFrmFmt *HTMLTableColumn::GetFrmFmt( bool bBorderLine,
                                             sal_Int16 eVertOrient ) const
{
    return aFrmFmts[GetFrmFmtIdx(bBorderLine,eVertOrient)];
}

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
void HTMLTable::InitCtor( const HTMLTableOptions *pOptions )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
void HTMLTable::InitCtor(const HTMLTableOptions& rOptions)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
{
    pResizeDrawObjs = 0;
    pDrawObjPrcWidths = 0;

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pRows = new HTMLTableRows;
    pColumns = new HTMLTableColumns;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    m_pRows = new HTMLTableRows;
    m_pColumns = new HTMLTableColumns;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    nRows = 0;
    nCurRow = 0; nCurCol = 0;

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pBox1 = 0;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    m_xBox1.reset();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    pBoxFmt = 0; pLineFmt = 0;
    pLineFrmFmtNoHeight = 0;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pInhBGBrush = 0;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    m_xInheritedBackgroundBrush.reset();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    pPrevStNd = 0;
    pSwTable = 0;

    bTopBorder = false; bRightBorder = false;
    bTopAlwd = true; bRightAlwd = true;
    bFillerTopBorder = false; bFillerBottomBorder = false;
    bInhLeftBorder = false; bInhRightBorder = false;
    bBordersSet = false;
    bForceFrame = false;
    nHeadlineRepeat = 0;

    nLeftMargin = 0;
    nRightMargin = 0;

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    const Color& rBorderColor = pOptions->aBorderColor;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    const Color& rBorderColor = rOptions.aBorderColor;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    long nBorderOpt = (long)pOptions->nBorder;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    long nBorderOpt = static_cast<long>(rOptions.nBorder);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    long nPWidth = nBorderOpt==USHRT_MAX ? NETSCAPE_DFLT_BORDER
                                         : nBorderOpt;
    long nPHeight = nBorderOpt==USHRT_MAX ? 0 : nBorderOpt;
    SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );

    // nBorder gibt die Breite der Umrandung an, wie sie in die
    // Breitenberechnung in Netscape einfliesst. Wenn pOption->nBorder
    // == USHRT_MAX, wurde keine BORDER-Option angegeben. Trotzdem fliesst
    // eine 1 Pixel breite Umrandung in die Breitenberechnung mit ein.
    nBorder = (sal_uInt16)nPWidth;
    if( nBorderOpt==USHRT_MAX )
        nPWidth = 0;

    // HACK: ein Pixel-breite Linien sollen zur Haarlinie werden, wenn
    // wir mit doppelter Umrandung arbeiten
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( pOptions->nCellSpacing!=0 && nBorderOpt==1 )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if( rOptions.nCellSpacing!=0 && nBorderOpt==1 )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
        nPWidth = 1;
        nPHeight = 1;
    }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if ( pOptions->nCellSpacing != 0 )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if ( rOptions.nCellSpacing != 0 )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
        aTopBorderLine.SetBorderLineStyle(table::BorderLineStyle::DOUBLE);
    }
    aTopBorderLine.SetWidth( nPHeight );
    aTopBorderLine.SetColor( rBorderColor );
    aBottomBorderLine = aTopBorderLine;

    if( nPWidth == nPHeight )
    {
        aLeftBorderLine = aTopBorderLine;
    }
    else
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        if ( pOptions->nCellSpacing != 0 )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if ( rOptions.nCellSpacing != 0 )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        {
            aLeftBorderLine.SetBorderLineStyle(table::BorderLineStyle::DOUBLE);
        }
        aLeftBorderLine.SetWidth( nPWidth );
        aLeftBorderLine.SetColor( rBorderColor );
    }
    aRightBorderLine = aLeftBorderLine;

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( pOptions->nCellSpacing != 0 )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if( rOptions.nCellSpacing != 0 )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
        aBorderLine.SetBorderLineStyle(table::BorderLineStyle::DOUBLE);
        aBorderLine.SetWidth( DEF_LINE_WIDTH_0 );
    }
    else
    {
        aBorderLine.SetWidth( DEF_LINE_WIDTH_0 );
    }
    aBorderLine.SetColor( rBorderColor );

    if( nCellPadding )
    {
        if( nCellPadding==USHRT_MAX )
            nCellPadding = MIN_BORDER_DIST; // default
        else
        {
            nCellPadding = pParser->ToTwips( nCellPadding );
            if( nCellPadding<MIN_BORDER_DIST  )
                nCellPadding = MIN_BORDER_DIST;
        }
    }
    if( nCellSpacing )
    {
        if( nCellSpacing==USHRT_MAX )
            nCellSpacing = NETSCAPE_DFLT_CELLSPACING;
        nCellSpacing = pParser->ToTwips( nCellSpacing );
    }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    nPWidth = pOptions->nHSpace;
    nPHeight = pOptions->nVSpace;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    nPWidth = rOptions.nHSpace;
    nPHeight = rOptions.nVSpace;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
    nHSpace = (sal_uInt16)nPWidth;
    nVSpace = (sal_uInt16)nPHeight;

    bColSpec = false;

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pBGBrush = pParser->CreateBrushItem(
                    pOptions->bBGColor ? &(pOptions->aBGColor) : 0,
                    pOptions->aBGImage, aEmptyOUStr, aEmptyOUStr, aEmptyOUStr );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    m_xBackgroundBrush.reset(pParser->CreateBrushItem(
                    rOptions.bBGColor ? &(rOptions.aBGColor) : nullptr,
                    rOptions.aBGImage, aEmptyOUStr, aEmptyOUStr, aEmptyOUStr));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    pContext = 0;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pParentContents = 0;

    aId = pOptions->aId;
    aClass = pOptions->aClass;
    aStyle = pOptions->aStyle;
    aDir = pOptions->aDir;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    m_xParentContents.reset();

    aId = rOptions.aId;
    aClass = rOptions.aClass;
    aStyle = rOptions.aStyle;
    aDir = rOptions.aDir;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
}

HTMLTable::HTMLTable( SwHTMLParser* pPars, HTMLTable *pTopTab,
                      bool bParHead,
                      bool bHasParentSec, bool bHasToFlw,
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                      const HTMLTableOptions *pOptions ) :
    nCols( pOptions->nCols ),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                     const HTMLTableOptions& rOptions) :
    nCols(rOptions.nCols),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    nFilledCols( 0 ),
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    nCellPadding( pOptions->nCellPadding ),
    nCellSpacing( pOptions->nCellSpacing ),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    nCellPadding(rOptions.nCellPadding),
    nCellSpacing(rOptions.nCellSpacing),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    nBoxes( 1 ),
    pCaptionStartNode( 0 ),
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    bTableAdjustOfTag( !pTopTab && pOptions->bTableAdjust ),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    bTableAdjustOfTag( !pTopTab && rOptions.bTableAdjust ),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    bIsParentHead( bParHead ),
    bHasParentSection( bHasParentSec ),
    bHasToFly( bHasToFlw ),
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    bFixedCols( pOptions->nCols>0 ),
    bPrcWidth( pOptions->bPrcWidth ),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    bFixedCols( rOptions.nCols>0 ),
    bPrcWidth( rOptions.bPrcWidth ),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    pParser( pPars ),
    pTopTable( pTopTab ? pTopTab : this ),
    pLayoutInfo( 0 ),
#ifndef NO_LIBO_MODIFIED_HTML_TABLE_LEAK_FIX
    bLayoutInfoOwner( true ),
#endif	// !NO_LIBO_MODIFIED_HTML_TABLE_LEAK_FIX
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    nWidth( pOptions->nWidth ),
    nHeight( pTopTab ? 0 : pOptions->nHeight ),
    eTableAdjust( pOptions->eAdjust ),
    eVertOri( pOptions->eVertOri ),
    eFrame( pOptions->eFrame ),
    eRules( pOptions->eRules ),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    nWidth( rOptions.nWidth ),
    nHeight( pTopTab ? 0 : rOptions.nHeight ),
    eTableAdjust( rOptions.eAdjust ),
    eVertOri( rOptions.eVertOri ),
    eFrame( rOptions.eFrame ),
    eRules( rOptions.eRules ),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    bTopCaption( false ),
    bFirstCell( !pTopTab )
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    InitCtor( pOptions );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    InitCtor(rOptions);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    for( sal_uInt16 i=0; i<nCols; i++ )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pColumns->push_back( new HTMLTableColumn );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        m_pColumns->push_back(o3tl::make_unique<HTMLTableColumn>());
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#ifndef NO_LIBO_HTML_TABLE_LEAK_FIX
    pParser->RegisterHTMLTable(this);
#endif	// !NO_LIBO_HTML_TABLE_LEAK_FIX
}

#ifndef NO_LIBO_HTML_TABLE_LEAK_FIX

void SwHTMLParser::DeregisterHTMLTable(HTMLTable* pOld)
{
    if (pOld->m_xBox1)
        m_aOrphanedTableBoxes.emplace_back(std::move(pOld->m_xBox1));
    m_aTables.erase(std::remove(m_aTables.begin(), m_aTables.end(), pOld));
}

#endif	// !NO_LIBO_HTML_TABLE_LEAK_FIX

HTMLTable::~HTMLTable()
{
#ifndef NO_LIBO_HTML_TABLE_LEAK_FIX
    pParser->DeregisterHTMLTable(this);
#endif	// !NO_LIBO_HTML_TABLE_LEAK_FIX

    delete pResizeDrawObjs;
    delete pDrawObjPrcWidths;

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    delete pRows;
    delete pColumns;
    delete pBGBrush;
    delete pInhBGBrush;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    delete m_pRows;
    delete m_pColumns;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    delete pContext;

    // pLayoutInfo wurde entweder bereits geloescht oder muss aber es
    // in den Besitz der SwTable uebergegangen.
#ifndef NO_LIBO_MODIFIED_HTML_TABLE_LEAK_FIX
    if ( bLayoutInfoOwner )
        delete pLayoutInfo;
#endif	// !NO_LIBO_MODIFIED_HTML_TABLE_LEAK_FIX
}

SwHTMLTableLayout *HTMLTable::CreateLayoutInfo()
{
    sal_uInt16 nW = bPrcWidth ? nWidth : pParser->ToTwips( nWidth );

    sal_uInt16 nBorderWidth = GetBorderWidth( aBorderLine, true );
    sal_uInt16 nLeftBorderWidth =
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        ((*pColumns)[0]).bLeftBorder ? GetBorderWidth( aLeftBorderLine, true ) : 0;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        (*m_pColumns)[0]->bLeftBorder ? GetBorderWidth(aLeftBorderLine, true) : 0;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    sal_uInt16 nRightBorderWidth =
        bRightBorder ? GetBorderWidth( aRightBorderLine, true ) : 0;
    sal_uInt16 nInhLeftBorderWidth = 0;
    sal_uInt16 nInhRightBorderWidth = 0;

#ifndef NO_LIBO_MODIFIED_HTML_TABLE_LEAK_FIX
    if ( bLayoutInfoOwner )
        delete pLayoutInfo;
    bLayoutInfoOwner = true;
#endif	// !NO_LIBO_MODIFIED_HTML_TABLE_LEAK_FIX
    pLayoutInfo = new SwHTMLTableLayout(
                        pSwTable,
                        nRows, nCols, bFixedCols, bColSpec,
                        nW, bPrcWidth, nBorder, nCellPadding,
                        nCellSpacing, eTableAdjust,
                        nLeftMargin, nRightMargin,
                        nBorderWidth, nLeftBorderWidth, nRightBorderWidth,
                        nInhLeftBorderWidth, nInhRightBorderWidth );

    bool bExportable = true;
    sal_uInt16 i;
    for( i=0; i<nRows; i++ )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableRow *pRow = &(*pRows)[i];
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableRow *const pRow = (*m_pRows)[i].get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        for( sal_uInt16 j=0; j<nCols; j++ )
        {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            SwHTMLTableLayoutCell *pLayoutCell =
                pRow->GetCell(j)->CreateLayoutInfo();

            pLayoutInfo->SetCell( pLayoutCell, i, j );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            pLayoutInfo->SetCell( pRow->GetCell(j)->CreateLayoutInfo().release(), i, j );
            SwHTMLTableLayoutCell* pLayoutCell = pLayoutInfo->GetCell(i, j );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

            if( bExportable )
            {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                SwHTMLTableLayoutCnts *pLayoutCnts =
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                const std::shared_ptr<SwHTMLTableLayoutCnts>& rLayoutCnts =
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    pLayoutCell->GetContents();
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                bExportable = !pLayoutCnts ||
                              ( pLayoutCnts->GetStartNode() &&
                                !pLayoutCnts->GetNext() );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                bExportable = !rLayoutCnts ||
                              (rLayoutCnts->GetStartNode() && !rLayoutCnts->GetNext());
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            }
        }
    }

    pLayoutInfo->SetExportable( bExportable );

    for( i=0; i<nCols; i++ )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pLayoutInfo->SetColumn( ((*pColumns)[i]).CreateLayoutInfo(), i );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pLayoutInfo->SetColumn( (*m_pColumns)[i]->CreateLayoutInfo(), i );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    return pLayoutInfo;
}

inline void HTMLTable::SetCaption( const SwStartNode *pStNd, bool bTop )
{
    pCaptionStartNode = pStNd;
    bTopCaption = bTop;
}

void HTMLTable::FixRowSpan( sal_uInt16 nRow, sal_uInt16 nCol,
                            const HTMLTableCnts *pCnts )
{
    sal_uInt16 nRowSpan=1;
    HTMLTableCell *pCell;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    while( ( pCell=GetCell(nRow,nCol), pCell->GetContents()==pCnts ) )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    while (pCell=GetCell(nRow,nCol), pCell->GetContents().get() == pCnts)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
        pCell->SetRowSpan( nRowSpan );
        if( pLayoutInfo )
            pLayoutInfo->GetCell(nRow,nCol)->SetRowSpan( nRowSpan );

        if( !nRow ) break;
        nRowSpan++; nRow--;
    }
}

void HTMLTable::ProtectRowSpan( sal_uInt16 nRow, sal_uInt16 nCol, sal_uInt16 nRowSpan )
{
    for( sal_uInt16 i=0; i<nRowSpan; i++ )
    {
        GetCell(nRow+i,nCol)->SetProtected();
        if( pLayoutInfo )
            pLayoutInfo->GetCell(nRow+i,nCol)->SetProtected();
    }
}

// Suchen des SwStartNodes der letzten belegten Vorgaengerbox
const SwStartNode* HTMLTable::GetPrevBoxStartNode( sal_uInt16 nRow, sal_uInt16 nCol ) const
{
    const HTMLTableCnts *pPrevCnts = 0;

    if( 0==nRow )
    {
        // immer die Vorgaenger-Zelle
        if( nCol>0 )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            pPrevCnts = GetCell( 0, nCol-1 )->GetContents();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            pPrevCnts = GetCell( 0, nCol-1 )->GetContents().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        else
            return pPrevStNd;
    }
    else if( USHRT_MAX==nRow && USHRT_MAX==nCol )
        // der Contents der letzten Zelle
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pPrevCnts = GetCell( nRows-1, nCols-1 )->GetContents();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pPrevCnts = GetCell( nRows-1, nCols-1 )->GetContents().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    else
    {
        sal_uInt16 i;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableRow *pPrevRow = &(*pRows)[nRow-1];
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableRow *const pPrevRow = (*m_pRows)[nRow-1].get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        // evtl. eine Zelle in der aktuellen Zeile
        i = nCol;
        while( i )
        {
            i--;
            if( 1 == pPrevRow->GetCell(i)->GetRowSpan() )
            {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                pPrevCnts = GetCell(nRow,i)->GetContents();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                pPrevCnts = GetCell(nRow,i)->GetContents().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                break;
            }
        }

        // sonst die letzte gefuellte Zelle der Zeile davor suchen
        if( !pPrevCnts )
        {
            i = nCols;
            while( !pPrevCnts && i )
            {
                i--;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                pPrevCnts = pPrevRow->GetCell(i)->GetContents();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                pPrevCnts = pPrevRow->GetCell(i)->GetContents().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            }
        }
    }
    OSL_ENSURE( pPrevCnts, "keine gefuellte Vorgaenger-Zelle gefunden" );
    if( !pPrevCnts )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pPrevCnts = GetCell(0,0)->GetContents();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pPrevCnts = GetCell(0,0)->GetContents().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if( !pPrevCnts )
            return pPrevStNd;
    }

    while( pPrevCnts->Next() )
        pPrevCnts = pPrevCnts->Next();

    return ( pPrevCnts->GetStartNode() ? pPrevCnts->GetStartNode()
               : pPrevCnts->GetTable()->GetPrevBoxStartNode( USHRT_MAX, USHRT_MAX ) );
}

static bool IsBoxEmpty( const SwTableBox *pBox )
{
    const SwStartNode *pSttNd = pBox->GetSttNd();
    if( pSttNd &&
        pSttNd->GetIndex() + 2 == pSttNd->EndOfSectionIndex() )
    {
        const SwCntntNode *pCNd =
            pSttNd->GetNodes()[pSttNd->GetIndex()+1]->GetCntntNode();
        if( pCNd && !pCNd->Len() )
            return true;
    }

    return false;
}

sal_uInt16 HTMLTable::GetTopCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                                   bool bSwBorders ) const
{
    sal_uInt16 nSpace = nCellPadding;

    if( nRow == 0 )
    {
        nSpace += nBorder + nCellSpacing;
        if( bSwBorders )
        {
            sal_uInt16 nTopBorderWidth =
                GetBorderWidth( aTopBorderLine, true );
            if( nSpace < nTopBorderWidth )
                nSpace = nTopBorderWidth;
        }
    }
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    else if( bSwBorders && (*pRows)[nRow+nRowSpan-1].bBottomBorder &&
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    else if( bSwBorders && (*m_pRows)[nRow+nRowSpan-1]->bBottomBorder &&
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
             nSpace < MIN_BORDER_DIST )
    {
        OSL_ENSURE( !nCellPadding, "GetTopCellSpace: CELLPADDING!=0" );
        // Wenn die Gegenueberliegende Seite umrandet ist muessen
        // wir zumindest den minimalen Abstand zum Inhalt
        // beruecksichtigen. (Koennte man zusaetzlich auch an
        // nCellPadding festmachen.)
        nSpace = MIN_BORDER_DIST;
    }

    return nSpace;
}

sal_uInt16 HTMLTable::GetBottomCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                                      bool bSwBorders ) const
{
    sal_uInt16 nSpace = nCellSpacing + nCellPadding;

    if( nRow+nRowSpan == nRows )
    {
        nSpace = nSpace + nBorder;

        if( bSwBorders )
        {
            sal_uInt16 nBottomBorderWidth =
                GetBorderWidth( aBottomBorderLine, true );
            if( nSpace < nBottomBorderWidth )
                nSpace = nBottomBorderWidth;
        }
    }
    else if( bSwBorders )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        if( (*pRows)[nRow+nRowSpan+1].bBottomBorder )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if( (*m_pRows)[nRow+nRowSpan+1]->bBottomBorder )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        {
            sal_uInt16 nBorderWidth = GetBorderWidth( aBorderLine, true );
            if( nSpace < nBorderWidth )
                nSpace = nBorderWidth;
        }
        else if( nRow==0 && bTopBorder && nSpace < MIN_BORDER_DIST )
        {
            OSL_ENSURE( GetBorderWidth( aTopBorderLine, true ) > 0,
                    "GetBottomCellSpace: |aTopLine| == 0" );
            OSL_ENSURE( !nCellPadding, "GetBottomCellSpace: CELLPADDING!=0" );
            // Wenn die Gegenueberliegende Seite umrandet ist muessen
            // wir zumindest den minimalen Abstand zum Inhalt
            // beruecksichtigen. (Koennte man zusaetzlich auch an
            // nCellPadding festmachen.)
            nSpace = MIN_BORDER_DIST;
        }
    }

    return nSpace;
}

void HTMLTable::FixFrameFmt( SwTableBox *pBox,
                             sal_uInt16 nRow, sal_uInt16 nCol,
                             sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                             bool bFirstPara, bool bLastPara ) const
{
    SwFrmFmt *pFrmFmt = 0;      // frame::Frame-Format
    sal_Int16 eVOri = text::VertOrientation::NONE;
    const SvxBrushItem *pBGBrushItem = 0;   // Hintergrund
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    boost::shared_ptr<SvxBoxItem> pBoxItem;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::shared_ptr<SvxBoxItem> pBoxItem;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    bool bTopLine = false, bBottomLine = false, bLastBottomLine = false;
    bool bReUsable = false;     // Format nochmals verwendbar?
    sal_uInt16 nEmptyRows = 0;
    bool bHasNumFmt = false;
    bool bHasValue = false;
    sal_uInt32 nNumFmt = 0;
    double nValue = 0.0;

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableColumn *pColumn = &(*pColumns)[nCol];
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableColumn *const pColumn = (*m_pColumns)[nCol].get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    if( pBox->GetSttNd() )
    {
        // die Hintergrundfarbe/-grafik bestimmen
        const HTMLTableCell *pCell = GetCell( nRow, nCol );
        pBoxItem = pCell->GetBoxItem();
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pBGBrushItem = pCell->GetBGBrush();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pBGBrushItem = pCell->GetBGBrush().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if( !pBGBrushItem )
        {
            // Wenn die Zelle ueber mehrere Zeilen geht muss ein evtl.
            // an der Zeile gesetzter Hintergrund an die Zelle uebernommen
            // werden.
            // Wenn es sich um eine Tabelle in der Tabelle handelt und
            // die Zelle ueber die gesamte Heoehe der Tabelle geht muss
            // ebenfalls der Hintergrund der Zeile uebernommen werden, weil
            // die Line von der GC (zu Recht) wegoptimiert wird.
            if( nRowSpan > 1 || (this != pTopTable && nRowSpan==nRows) )
            {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                pBGBrushItem = (*pRows)[nRow].GetBGBrush();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                pBGBrushItem = (*m_pRows)[nRow]->GetBGBrush().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                if( !pBGBrushItem && this != pTopTable )
                {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                    pBGBrushItem = GetBGBrush();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    pBGBrushItem = GetBGBrush().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    if( !pBGBrushItem )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                        pBGBrushItem = GetInhBGBrush();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        pBGBrushItem = GetInhBGBrush().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                }
            }
        }

        bTopLine = 0==nRow && bTopBorder && bFirstPara;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        if( (*pRows)[nRow+nRowSpan-1].bBottomBorder && bLastPara )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if( (*m_pRows)[nRow+nRowSpan-1]->bBottomBorder && bLastPara )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            nEmptyRows = (*pRows)[nRow+nRowSpan-1].GetEmptyRows();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            nEmptyRows = (*m_pRows)[nRow+nRowSpan-1]->GetEmptyRows();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            if( nRow+nRowSpan == nRows )
                bLastBottomLine = true;
            else
                bBottomLine = true;
        }

        eVOri = pCell->GetVertOri();
        bHasNumFmt = pCell->GetNumFmt( nNumFmt );
        if( bHasNumFmt )
            bHasValue = pCell->GetValue( nValue );

        if( nColSpan==1 && !bTopLine && !bLastBottomLine && !nEmptyRows &&
            !pBGBrushItem && !bHasNumFmt && !pBoxItem)
        {
            pFrmFmt = pColumn->GetFrmFmt( bBottomLine, eVOri );
            bReUsable = !pFrmFmt;
        }
    }

    if( !pFrmFmt )
    {
        pFrmFmt = pBox->ClaimFrmFmt();

        // die Breite der Box berechnen
        SwTwips nFrmWidth = (SwTwips)pLayoutInfo->GetColumn(nCol)
                                                ->GetRelColWidth();
        for( sal_uInt16 i=1; i<nColSpan; i++ )
            nFrmWidth += (SwTwips)pLayoutInfo->GetColumn(nCol+i)
                                             ->GetRelColWidth();

        // die Umrandung nur an Edit-Boxen setzen (bei der oberen und unteren
        // Umrandung muss beruecks. werden, ob es sich um den ersten oder
        // letzen Absatz der Zelle handelt)
        if( pBox->GetSttNd() )
        {
            bool bSet = (nCellPadding > 0);

            SvxBoxItem aBoxItem( RES_BOX );
            long nInnerFrmWidth = nFrmWidth;

            if( bTopLine )
            {
                aBoxItem.SetLine( &aTopBorderLine, BOX_LINE_TOP );
                bSet = true;
            }
            if( bLastBottomLine )
            {
                aBoxItem.SetLine( &aBottomBorderLine, BOX_LINE_BOTTOM );
                bSet = true;
            }
            else if( bBottomLine )
            {
                if( nEmptyRows && !aBorderLine.GetInWidth() )
                {
                    // Leere Zeilen koennen zur Zeit nur dann ueber
                    // dicke Linien simuliert werden, wenn die Linie
                    // einfach ist.
                    SvxBorderLine aThickBorderLine( aBorderLine );

                    sal_uInt16 nBorderWidth = aBorderLine.GetOutWidth();
                    nBorderWidth *= (nEmptyRows + 1);
                    aThickBorderLine.SetBorderLineStyle(
                            table::BorderLineStyle::SOLID);
                    aThickBorderLine.SetWidth( nBorderWidth );
                    aBoxItem.SetLine( &aThickBorderLine, BOX_LINE_BOTTOM );
                }
                else
                {
                    aBoxItem.SetLine( &aBorderLine, BOX_LINE_BOTTOM );
                }
                bSet = true;
            }
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            if( ((*pColumns)[nCol]).bLeftBorder )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            if( ((*m_pColumns)[nCol])->bLeftBorder )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            {
                const SvxBorderLine& rBorderLine =
                    0==nCol ? aLeftBorderLine : aBorderLine;
                aBoxItem.SetLine( &rBorderLine, BOX_LINE_LEFT );
                nInnerFrmWidth -= GetBorderWidth( rBorderLine );
                bSet = true;
            }
            if( nCol+nColSpan == nCols && bRightBorder )
            {
                aBoxItem.SetLine( &aRightBorderLine, BOX_LINE_RIGHT );
                nInnerFrmWidth -= GetBorderWidth( aRightBorderLine );
                bSet = true;
            }

            if (pBoxItem)
            {
                pFrmFmt->SetFmtAttr( *pBoxItem );
            }
            else if (bSet)
            {
                // BorderDist nicht mehr Bestandteil einer Zelle mit fixer Breite
                sal_uInt16 nBDist = static_cast< sal_uInt16 >(
                    (2*nCellPadding <= nInnerFrmWidth) ? nCellPadding
                                                      : (nInnerFrmWidth / 2) );
                // wir setzen das Item nur, wenn es eine Umrandung gibt
                // oder eine sheet::Border-Distanz vorgegeben ist. Fehlt letztere,
                // dann gibt es eine Umrandung, und wir muessen die Distanz
                // setzen
                aBoxItem.SetDistance( nBDist ? nBDist : MIN_BORDER_DIST );
                pFrmFmt->SetFmtAttr( aBoxItem );
            }
            else
                pFrmFmt->ResetFmtAttr( RES_BOX );

            if( pBGBrushItem )
            {
                pFrmFmt->SetFmtAttr( *pBGBrushItem );
            }
            else
                pFrmFmt->ResetFmtAttr( RES_BACKGROUND );

            // Format nur setzten, wenn es auch einen Value gibt oder die Box leer ist.
            if( bHasNumFmt && (bHasValue || IsBoxEmpty(pBox)) )
            {
                bool bLock = pFrmFmt->GetDoc()->GetNumberFormatter()
                                     ->IsTextFormat( nNumFmt );
                SfxItemSet aItemSet( *pFrmFmt->GetAttrSet().GetPool(),
                                     RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
                SvxAdjust eAdjust = SVX_ADJUST_END;
                SwCntntNode *pCNd = 0;
                if( !bLock )
                {
                    const SwStartNode *pSttNd = pBox->GetSttNd();
                    pCNd = pSttNd->GetNodes()[pSttNd->GetIndex()+1]
                                 ->GetCntntNode();
                    const SfxPoolItem *pItem;
                    if( pCNd && pCNd->HasSwAttrSet() &&
                        SfxItemState::SET==pCNd->GetpSwAttrSet()->GetItemState(
                            RES_PARATR_ADJUST, false, &pItem ) )
                    {
                        eAdjust = ((const SvxAdjustItem *)pItem)
                            ->GetAdjust();
                    }
                }
                aItemSet.Put( SwTblBoxNumFormat(nNumFmt) );
                if( bHasValue )
                    aItemSet.Put( SwTblBoxValue(nValue) );

                if( bLock )
                    pFrmFmt->LockModify();
                pFrmFmt->SetFmtAttr( aItemSet );
                if( bLock )
                    pFrmFmt->UnlockModify();
                else if( pCNd && SVX_ADJUST_END != eAdjust )
                {
                    SvxAdjustItem aAdjItem( eAdjust, RES_PARATR_ADJUST );
                    pCNd->SetAttr( aAdjItem );
                }
            }
            else
                pFrmFmt->ResetFmtAttr( RES_BOXATR_FORMAT );

            OSL_ENSURE( eVOri != text::VertOrientation::TOP, "text::VertOrientation::TOP ist nicht erlaubt!" );
            if( text::VertOrientation::NONE != eVOri )
            {
                pFrmFmt->SetFmtAttr( SwFmtVertOrient( 0, eVOri ) );
            }
            else
                pFrmFmt->ResetFmtAttr( RES_VERT_ORIENT );

            if( bReUsable )
                pColumn->SetFrmFmt( pFrmFmt, bBottomLine, eVOri );
        }
        else
        {
            pFrmFmt->ResetFmtAttr( RES_BOX );
            pFrmFmt->ResetFmtAttr( RES_BACKGROUND );
            pFrmFmt->ResetFmtAttr( RES_VERT_ORIENT );
            pFrmFmt->ResetFmtAttr( RES_BOXATR_FORMAT );
        }
    }
    else
    {
        OSL_ENSURE( pBox->GetSttNd() ||
                SfxItemState::SET!=pFrmFmt->GetAttrSet().GetItemState(
                                    RES_VERT_ORIENT, false ),
                "Box ohne Inhalt hat vertikale Ausrichtung" );
        pBox->ChgFrmFmt( (SwTableBoxFmt*)pFrmFmt );
    }

}

void HTMLTable::FixFillerFrameFmt( SwTableBox *pBox, bool bRight ) const
{
    SwFrmFmt *pFrmFmt = pBox->ClaimFrmFmt();

    if( bFillerTopBorder || bFillerBottomBorder ||
        (!bRight && bInhLeftBorder) || (bRight && bInhRightBorder) )
    {
        SvxBoxItem aBoxItem( RES_BOX );
        if( bFillerTopBorder )
            aBoxItem.SetLine( &aTopBorderLine, BOX_LINE_TOP );
        if( bFillerBottomBorder )
            aBoxItem.SetLine( &aBottomBorderLine, BOX_LINE_BOTTOM );
        if( !bRight && bInhLeftBorder )
            aBoxItem.SetLine( &aInhLeftBorderLine, BOX_LINE_LEFT );
        if( bRight && bInhRightBorder )
            aBoxItem.SetLine( &aInhRightBorderLine, BOX_LINE_RIGHT );
        aBoxItem.SetDistance( MIN_BORDER_DIST );
        pFrmFmt->SetFmtAttr( aBoxItem );
    }
    else
    {
        pFrmFmt->ResetFmtAttr( RES_BOX );
    }

    if( GetInhBGBrush() )
        pFrmFmt->SetFmtAttr( *GetInhBGBrush() );
    else
        pFrmFmt->ResetFmtAttr( RES_BACKGROUND );

    pFrmFmt->ResetFmtAttr( RES_VERT_ORIENT );
    pFrmFmt->ResetFmtAttr( RES_BOXATR_FORMAT );
}

SwTableBox *HTMLTable::NewTableBox( const SwStartNode *pStNd,
                                    SwTableLine *pUpper ) const
{
    SwTableBox *pBox;

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( pTopTable->pBox1 &&
        pTopTable->pBox1->GetSttNd() == pStNd )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if( pTopTable->m_xBox1 &&
        pTopTable->m_xBox1->GetSttNd() == pStNd )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
        // wenn der StartNode dem StartNode der initial angelegten Box
        // entspricht nehmen wir diese Box
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pBox = pTopTable->pBox1;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pBox = pTopTable->m_xBox1.release();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pBox->SetUpper( pUpper );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pTopTable->pBox1 = 0;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }
    else
        pBox = new SwTableBox( pBoxFmt, *pStNd, pUpper );

    return pBox;
}

static void ResetLineFrmFmtAttrs( SwFrmFmt *pFrmFmt )
{
    pFrmFmt->ResetFmtAttr( RES_FRM_SIZE );
    pFrmFmt->ResetFmtAttr( RES_BACKGROUND );
    OSL_ENSURE( SfxItemState::SET!=pFrmFmt->GetAttrSet().GetItemState(
                                RES_VERT_ORIENT, false ),
            "Zeile hat vertikale Ausrichtung" );
}

// !!! kann noch vereinfacht werden
SwTableLine *HTMLTable::MakeTableLine( SwTableBox *pUpper,
                                       sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                                       sal_uInt16 nBottomRow, sal_uInt16 nRightCol )
{
    SwTableLine *pLine;
    if( this==pTopTable && !pUpper && 0==nTopRow )
        pLine = (pSwTable->GetTabLines())[0];
    else
        pLine = new SwTableLine( pLineFrmFmtNoHeight ? pLineFrmFmtNoHeight
                                                     : pLineFmt,
                                 0, pUpper );

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableRow *pTopRow = &(*pRows)[nTopRow];
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableRow *pTopRow = (*m_pRows)[nTopRow].get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    sal_uInt16 nRowHeight = pTopRow->GetHeight();
    const SvxBrushItem *pBGBrushItem = 0;
    if( this == pTopTable || nTopRow>0 || nBottomRow<nRows )
    {
        // An der Line eine Frabe zu setzen macht keinen Sinn, wenn sie
        // die auesserste und gleichzeitig einzige Zeile einer Tabelle in
        // der Tabelle ist.
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pBGBrushItem = pTopRow->GetBGBrush();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pBGBrushItem = pTopRow->GetBGBrush().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        if( !pBGBrushItem && this != pTopTable )
        {
            // Ein an einer Tabellen in der Tabelle gesetzter Hintergrund
            // wird an den Rows gesetzt. Das gilt auch fuer den Hintergrund
            // der Zelle, in dem die Tabelle vorkommt.
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            pBGBrushItem = GetBGBrush();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            pBGBrushItem = GetBGBrush().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            if( !pBGBrushItem )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                pBGBrushItem = GetInhBGBrush();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                pBGBrushItem = GetInhBGBrush().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        }
    }
    if( nTopRow==nBottomRow-1 && (nRowHeight || pBGBrushItem) )
    {
        SwTableLineFmt *pFrmFmt = (SwTableLineFmt*)pLine->ClaimFrmFmt();
        ResetLineFrmFmtAttrs( pFrmFmt );

        if( nRowHeight )
        {
            // Tabellenhoehe einstellen. Da es sich um eine
            // Mindesthoehe handelt, kann sie genauso wie in
            // Netscape berechnet werden, also ohne Beruecksichtigung
            // der tatsaechlichen Umrandungsbreite.
            nRowHeight += GetTopCellSpace( nTopRow, 1, false ) +
                       GetBottomCellSpace( nTopRow, 1, false );

            pFrmFmt->SetFmtAttr( SwFmtFrmSize( ATT_MIN_SIZE, 0, nRowHeight ) );
        }

        if( pBGBrushItem )
        {
            pFrmFmt->SetFmtAttr( *pBGBrushItem );
        }

    }
    else if( !pLineFrmFmtNoHeight )
    {
        // sonst muessen wir die Hoehe aus dem Attribut entfernen
        // und koennen uns das Format merken
        pLineFrmFmtNoHeight = (SwTableLineFmt*)pLine->ClaimFrmFmt();

        ResetLineFrmFmtAttrs( pLineFrmFmtNoHeight );
    }

    SwTableBoxes& rBoxes = pLine->GetTabBoxes();

    sal_uInt16 nStartCol = nLeftCol;
    while( nStartCol<nRightCol )
    {
        sal_uInt16 nCol = nStartCol;
        sal_uInt16 nSplitCol = nRightCol;
        bool bSplitted = false;
        while( !bSplitted )
        {
            OSL_ENSURE( nCol < nRightCol, "Zu weit gelaufen" );

            HTMLTableCell *pCell = GetCell(nTopRow,nCol);
            const bool bSplit = 1 == pCell->GetColSpan();

            OSL_ENSURE((nCol != nRightCol-1) || bSplit, "Split-Flag wrong");
            if( bSplit )
            {
                SwTableBox* pBox = 0;
                HTMLTableCell *pCell2 = GetCell( nTopRow, nStartCol );
                if( pCell2->GetColSpan() == (nCol+1-nStartCol) )
                {
                    // Die HTML-Tabellen-Zellen bilden genau eine Box.
                    // Dann muss hinter der Box gesplittet werden
                    nSplitCol = nCol + 1;

                    long nBoxRowSpan = pCell2->GetRowSpan();
                    if ( !pCell2->GetContents() || pCell2->IsCovered() )
                    {
                        if ( pCell2->IsCovered() )
                            nBoxRowSpan = -1 * nBoxRowSpan;

                        const SwStartNode* pPrevStartNd =
                            GetPrevBoxStartNode( nTopRow, nStartCol );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                        HTMLTableCnts *pCnts = new HTMLTableCnts(
                            pParser->InsertTableSection(pPrevStartNd) );
                        SwHTMLTableLayoutCnts *pCntsLayoutInfo =
                            pCnts->CreateLayoutInfo();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        std::shared_ptr<HTMLTableCnts> xCnts(new HTMLTableCnts(
                            pParser->InsertTableSection(pPrevStartNd)));
                        const std::shared_ptr<SwHTMLTableLayoutCnts> xCntsLayoutInfo =
                            xCnts->CreateLayoutInfo();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                        pCell2->SetContents( pCnts );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        pCell2->SetContents(xCnts);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        SwHTMLTableLayoutCell *pCurrCell = pLayoutInfo->GetCell( nTopRow, nStartCol );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                        pCurrCell->SetContents( pCntsLayoutInfo );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        pCurrCell->SetContents(xCntsLayoutInfo);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        if( nBoxRowSpan < 0 )
                            pCurrCell->SetRowSpan( 0 );

                        // ggf. COLSPAN beachten
                        for( sal_uInt16 j=nStartCol+1; j<nSplitCol; j++ )
                        {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                            GetCell(nTopRow,j)->SetContents( pCnts );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            GetCell(nTopRow,j)->SetContents(xCnts);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            pLayoutInfo->GetCell( nTopRow, j )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                                       ->SetContents( pCntsLayoutInfo );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                       ->SetContents(xCntsLayoutInfo);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        }
                    }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                    pBox = MakeTableBox( pLine, pCell2->GetContents(),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    pBox = MakeTableBox( pLine, pCell2->GetContents().get(),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                         nTopRow, nStartCol,
                                         nBottomRow, nSplitCol );

                    if ( 1 != nBoxRowSpan )
                        pBox->setRowSpan( nBoxRowSpan );

                    bSplitted = true;
                }

                OSL_ENSURE( pBox, "Colspan trouble" );

                if( pBox )
                    rBoxes.push_back( pBox );
            }
            nCol++;
        }
        nStartCol = nSplitCol;
    }

    return pLine;
}

SwTableBox *HTMLTable::MakeTableBox( SwTableLine *pUpper,
                                     HTMLTableCnts *pCnts,
                                     sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                                     sal_uInt16 nBottomRow, sal_uInt16 nRightCol )
{
    SwTableBox *pBox;
    sal_uInt16 nColSpan = nRightCol - nLeftCol;
    sal_uInt16 nRowSpan = nBottomRow - nTopRow;

    if( !pCnts->Next() )
    {
        // nur eine Inhalts-Section
        if( pCnts->GetStartNode() )
        {
            // und die ist keine Tabelle
            pBox = NewTableBox( pCnts->GetStartNode(), pUpper );
            pCnts->SetTableBox( pBox );
        }
#ifdef NO_LIBO_NULL_TABLE_NODE_FIX
        else
#else	// NO_LIBO_NULL_TABLE_NODE_FIX
        else if (HTMLTable* pTable = pCnts->GetTable().get())
#endif	// NO_LIBO_NULL_TABLE_NODE_FIX
        {
#ifdef NO_LIBO_NULL_TABLE_NODE_FIX
            pCnts->GetTable()->InheritVertBorders( this, nLeftCol,
#else	// NO_LIBO_NULL_TABLE_NODE_FIX
            pTable->InheritVertBorders( this, nLeftCol,
#endif	// NO_LIBO_NULL_TABLE_NODE_FIX
                                                   nRightCol-nLeftCol );
            // und die ist eine Tabelle: dann bauen wir eine neue
            // Box und fuegen die Zeilen der Tabelle in die Zeilen
            // der Box ein
            pBox = new SwTableBox( pBoxFmt, 0, pUpper );
            sal_uInt16 nAbs, nRel;
            pLayoutInfo->GetAvail( nLeftCol, nColSpan, nAbs, nRel );
            sal_uInt16 nLSpace = pLayoutInfo->GetLeftCellSpace( nLeftCol, nColSpan );
            sal_uInt16 nRSpace = pLayoutInfo->GetRightCellSpace( nLeftCol, nColSpan );
            sal_uInt16 nInhSpace = pLayoutInfo->GetInhCellSpace( nLeftCol, nColSpan );
            pCnts->GetTable()->MakeTable( pBox, nAbs, nRel, nLSpace, nRSpace,
                                          nInhSpace );
        }
#ifndef NO_LIBO_NULL_TABLE_NODE_FIX
        else
        {
            return nullptr;
        }
#endif	// !NO_LIBO_NULL_TABLE_NODE_FIX
    }
    else
    {
        // mehrere Inhalts Sections: dann brauchen wir eine Box mit Zeilen
        pBox = new SwTableBox( pBoxFmt, 0, pUpper );
        SwTableLines& rLines = pBox->GetTabLines();
        bool bFirstPara = true;

        while( pCnts )
        {
            if( pCnts->GetStartNode() )
            {
                // normale Absaetze werden zu einer Box in einer Zeile
                SwTableLine *pLine =
                    new SwTableLine( pLineFrmFmtNoHeight ? pLineFrmFmtNoHeight
                                                         : pLineFmt, 0, pBox );
                if( !pLineFrmFmtNoHeight )
                {
                    // Wenn es noch kein Line-Format ohne Hoehe gibt, koennen
                    // wir uns dieses her als soleches merken
                    pLineFrmFmtNoHeight = (SwTableLineFmt*)pLine->ClaimFrmFmt();

                    ResetLineFrmFmtAttrs( pLineFrmFmtNoHeight );
                }

                SwTableBox* pCntBox = NewTableBox( pCnts->GetStartNode(),
                                                   pLine );
                pCnts->SetTableBox( pCntBox );
                FixFrameFmt( pCntBox, nTopRow, nLeftCol, nRowSpan, nColSpan,
                             bFirstPara, 0==pCnts->Next() );
                pLine->GetTabBoxes().push_back( pCntBox );

                rLines.push_back( pLine );
            }
            else
            {
                pCnts->GetTable()->InheritVertBorders( this, nLeftCol,
                                                       nRightCol-nLeftCol );
                // Tabellen werden direkt eingetragen
                sal_uInt16 nAbs, nRel;
                pLayoutInfo->GetAvail( nLeftCol, nColSpan, nAbs, nRel );
                sal_uInt16 nLSpace = pLayoutInfo->GetLeftCellSpace( nLeftCol,
                                                                nColSpan );
                sal_uInt16 nRSpace = pLayoutInfo->GetRightCellSpace( nLeftCol,
                                                                 nColSpan );
                sal_uInt16 nInhSpace = pLayoutInfo->GetInhCellSpace( nLeftCol, nColSpan );
                pCnts->GetTable()->MakeTable( pBox, nAbs, nRel, nLSpace,
                                              nRSpace, nInhSpace );
            }

            pCnts = pCnts->Next();
            bFirstPara = false;
        }
    }

    FixFrameFmt( pBox, nTopRow, nLeftCol, nRowSpan, nColSpan );

    return pBox;
}

void HTMLTable::InheritBorders( const HTMLTable *pParent,
                                sal_uInt16 nRow, sal_uInt16 nCol,
                                sal_uInt16 nRowSpan, sal_uInt16 /*nColSpan*/,
                                bool bFirstPara, bool bLastPara )
{
    OSL_ENSURE( nRows>0 && nCols>0 && nCurRow==nRows,
            "Wurde CloseTable nicht aufgerufen?" );

    // Die Child-Tabelle muss einen Rahmen bekommen, wenn die umgebende
    // Zelle einen Rand an der betreffenden Seite besitzt.
    // Der obere bzw. untere Rand wird nur gesetzt, wenn die Tabelle
    // ale erster bzw. letzter Absatz in der Zelle vorkommt. Ansonsten
    // Fuer den linken/rechten Rand kann noch nicht entschieden werden,
    // ob eine Umrandung der Tabelle noetig/moeglich ist, weil das davon
    // abhaengt, ob "Filler"-Zellen eingefuegt werden. Hier werden deshalb
    // erstmal nur Informationen gesammelt

    if( 0==nRow && pParent->bTopBorder && bFirstPara )
    {
        bTopBorder = true;
        bFillerTopBorder = true; // auch Filler bekommt eine Umrandung
        aTopBorderLine = pParent->aTopBorderLine;
    }
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( (*pParent->pRows)[nRow+nRowSpan-1].bBottomBorder && bLastPara )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if( (*pParent->m_pRows)[nRow+nRowSpan-1]->bBottomBorder && bLastPara )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        (*pRows)[nRows-1].bBottomBorder = true;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        (*m_pRows)[nRows-1]->bBottomBorder = true;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        bFillerBottomBorder = true; // auch Filler bekommt eine Umrandung
        aBottomBorderLine =
            nRow+nRowSpan==pParent->nRows ? pParent->aBottomBorderLine
                                          : pParent->aBorderLine;
    }

    // Die Child Tabelle darf keinen oberen oder linken Rahmen bekommen,
    // wenn der bereits durch die umgebende Tabelle gesetzt ist.
    // Sie darf jedoch immer einen oberen Rand bekommen, wenn die Tabelle
    // nicht der erste Absatz in der Zelle ist.
    bTopAlwd = ( !bFirstPara || (pParent->bTopAlwd &&
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                 (0==nRow || !((*pParent->pRows)[nRow-1]).bBottomBorder)) );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                 (0==nRow || !((*pParent->m_pRows)[nRow-1])->bBottomBorder)) );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    // die Child-Tabelle muss die Farbe der Zelle erben, in der sie
    // vorkommt, wenn sie keine eigene besitzt
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    const SvxBrushItem *pInhBG = pParent->GetCell(nRow,nCol)->GetBGBrush();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    const SvxBrushItem *pInhBG = pParent->GetCell(nRow,nCol)->GetBGBrush().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if( !pInhBG && pParent != pTopTable &&
        pParent->GetCell(nRow,nCol)->GetRowSpan() == pParent->nRows )
    {
        // die ganze umgebende Tabelle ist eine Tabelle in der Tabelle
        // und besteht nur aus einer Line, die bei der GC (zu Recht)
        // wegoptimiert wird. Deshalb muss der Hintergrund der Line in
        // diese Tabelle uebernommen werden.
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pInhBG = (*pParent->pRows)[nRow].GetBGBrush();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pInhBG = (*pParent->m_pRows)[nRow]->GetBGBrush().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if( !pInhBG )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            pInhBG = pParent->GetBGBrush();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            pInhBG = pParent->GetBGBrush().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if( !pInhBG )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            pInhBG = pParent->GetInhBGBrush();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            pInhBG = pParent->GetInhBGBrush().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }
    if( pInhBG )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pInhBGBrush = new SvxBrushItem( *pInhBG );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        m_xInheritedBackgroundBrush.reset(new SvxBrushItem(*pInhBG));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
}

void HTMLTable::InheritVertBorders( const HTMLTable *pParent,
                                 sal_uInt16 nCol, sal_uInt16 nColSpan )
{
    sal_uInt16 nInhLeftBorderWidth = 0;
    sal_uInt16 nInhRightBorderWidth = 0;

    if( nCol+nColSpan==pParent->nCols && pParent->bRightBorder )
    {
        bInhRightBorder = true; // erstmal nur merken
        aInhRightBorderLine = pParent->aRightBorderLine;
        nInhRightBorderWidth =
            GetBorderWidth( aInhRightBorderLine, true ) + MIN_BORDER_DIST;
    }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( ((*pParent->pColumns)[nCol]).bLeftBorder )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if( ((*pParent->m_pColumns)[nCol])->bLeftBorder )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
        bInhLeftBorder = true;  // erstmal nur merken
        aInhLeftBorderLine = 0==nCol ? pParent->aLeftBorderLine
                                     : pParent->aBorderLine;
        nInhLeftBorderWidth =
            GetBorderWidth( aInhLeftBorderLine, true ) + MIN_BORDER_DIST;
    }

    if( !bInhLeftBorder && (bFillerTopBorder || bFillerBottomBorder) )
        nInhLeftBorderWidth = 2 * MIN_BORDER_DIST;
    if( !bInhRightBorder && (bFillerTopBorder || bFillerBottomBorder) )
        nInhRightBorderWidth = 2 * MIN_BORDER_DIST;
    pLayoutInfo->SetInhBorderWidths( nInhLeftBorderWidth,
                                     nInhRightBorderWidth );

    bRightAlwd = ( pParent->bRightAlwd &&
                  (nCol+nColSpan==pParent->nCols ||
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                   !((*pParent->pColumns)[nCol+nColSpan]).bLeftBorder) );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                   !((*pParent->m_pColumns)[nCol+nColSpan])->bLeftBorder) );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
}

void HTMLTable::SetBorders()
{
    sal_uInt16 i;
    for( i=1; i<nCols; i++ )
        if( HTML_TR_ALL==eRules || HTML_TR_COLS==eRules ||
            ((HTML_TR_ROWS==eRules || HTML_TR_GROUPS==eRules) &&
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
             ((*pColumns)[i-1]).IsEndOfGroup()) )
            ((*pColumns)[i]).bLeftBorder = true;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
             ((*m_pColumns)[i-1])->IsEndOfGroup()) )
            ((*m_pColumns)[i])->bLeftBorder = true;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    for( i=0; i<nRows-1; i++ )
        if( HTML_TR_ALL==eRules || HTML_TR_ROWS==eRules ||
            ((HTML_TR_COLS==eRules || HTML_TR_GROUPS==eRules) &&
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
             (*pRows)[i].IsEndOfGroup()) )
            (*pRows)[i].bBottomBorder = true;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
             (*m_pRows)[i]->IsEndOfGroup()) )
            (*m_pRows)[i]->bBottomBorder = true;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    if( bTopAlwd && (HTML_TF_ABOVE==eFrame || HTML_TF_HSIDES==eFrame ||
                     HTML_TF_BOX==eFrame) )
        bTopBorder = true;
    if( HTML_TF_BELOW==eFrame || HTML_TF_HSIDES==eFrame ||
        HTML_TF_BOX==eFrame )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        (*pRows)[nRows-1].bBottomBorder = true;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        (*m_pRows)[nRows-1]->bBottomBorder = true;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if( (HTML_TF_RHS==eFrame || HTML_TF_VSIDES==eFrame ||
                      HTML_TF_BOX==eFrame) )
        bRightBorder = true;
    if( HTML_TF_LHS==eFrame || HTML_TF_VSIDES==eFrame || HTML_TF_BOX==eFrame )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        ((*pColumns)[0]).bLeftBorder = true;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        ((*m_pColumns)[0])->bLeftBorder = true;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    for( i=0; i<nRows; i++ )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableRow *pRow = &(*pRows)[i];
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableRow *const pRow = (*m_pRows)[i].get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        for( sal_uInt16 j=0; j<nCols; j++ )
        {
            HTMLTableCell *pCell = pRow->GetCell(j);
            if( pCell->GetContents()  )
            {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                HTMLTableCnts *pCnts = pCell->GetContents();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                HTMLTableCnts *pCnts = pCell->GetContents().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                bool bFirstPara = true;
                while( pCnts )
                {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                    HTMLTable *pTable = pCnts->GetTable();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    HTMLTable *pTable = pCnts->GetTable().get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    if( pTable && !pTable->BordersSet() )
                    {
                        pTable->InheritBorders( this, i, j,
                                                pCell->GetRowSpan(),
                                                pCell->GetColSpan(),
                                                bFirstPara,
                                                0==pCnts->Next() );
                        pTable->SetBorders();
                    }
                    bFirstPara = false;
                    pCnts = pCnts->Next();
                }
            }
        }
    }

    bBordersSet = true;
}

sal_uInt16 HTMLTable::GetBorderWidth( const SvxBorderLine& rBLine,
                                  bool bWithDistance ) const
{
    sal_uInt16 nBorderWidth = rBLine.GetWidth();
    if( bWithDistance )
    {
        if( nCellPadding )
            nBorderWidth = nBorderWidth + nCellPadding;
        else if( nBorderWidth )
            nBorderWidth = nBorderWidth + MIN_BORDER_DIST;
    }

    return nBorderWidth;
}

inline HTMLTableCell *HTMLTable::GetCell( sal_uInt16 nRow,
                                          sal_uInt16 nCell ) const
{
    OSL_ENSURE(nRow < pRows->size(), "invalid row index in HTML table");
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    return (*pRows)[nRow].GetCell( nCell );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    return (*m_pRows)[nRow]->GetCell( nCell );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
}

SvxAdjust HTMLTable::GetInheritedAdjust() const
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    SvxAdjust eAdjust = (nCurCol<nCols ? ((*pColumns)[nCurCol]).GetAdjust()
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    SvxAdjust eAdjust = (nCurCol<nCols ? ((*m_pColumns)[nCurCol])->GetAdjust()
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                       : SVX_ADJUST_END );
    if( SVX_ADJUST_END==eAdjust )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        eAdjust = (*pRows)[nCurRow].GetAdjust();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        eAdjust = (*m_pRows)[nCurRow]->GetAdjust();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    return eAdjust;
}

sal_Int16 HTMLTable::GetInheritedVertOri() const
{
    // text::VertOrientation::TOP ist der default!
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    sal_Int16 eVOri = (*pRows)[nCurRow].GetVertOri();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    sal_Int16 eVOri = (*m_pRows)[nCurRow]->GetVertOri();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if( text::VertOrientation::TOP==eVOri && nCurCol<nCols )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        eVOri = ((*pColumns)[nCurCol]).GetVertOri();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        eVOri = ((*m_pColumns)[nCurCol])->GetVertOri();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if( text::VertOrientation::TOP==eVOri )
        eVOri = eVertOri;

    OSL_ENSURE( eVertOri != text::VertOrientation::TOP, "text::VertOrientation::TOP ist nicht erlaubt!" );
    return eVOri;
}

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
void HTMLTable::InsertCell( HTMLTableCnts *pCnts,
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
void HTMLTable::InsertCell( std::shared_ptr<HTMLTableCnts> const& rCnts,
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                            sal_uInt16 nCellWidth, bool bRelWidth, sal_uInt16 nCellHeight,
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                            sal_Int16 eVertOrient, SvxBrushItem *pBGBrushItem,
                            boost::shared_ptr<SvxBoxItem> const pBoxItem,
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            sal_Int16 eVertOrient, std::shared_ptr<SvxBrushItem> const& rBGBrushItem,
                            std::shared_ptr<SvxBoxItem> const& rBoxItem,
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            bool bHasNumFmt, sal_uInt32 nNumFmt,
                            bool bHasValue, double nValue, bool bNoWrap )
{
    if( !nRowSpan || (sal_uInt32)nCurRow + nRowSpan > USHRT_MAX )
        nRowSpan = 1;

    if( !nColSpan || (sal_uInt32)nCurCol + nColSpan > USHRT_MAX )
        nColSpan = 1;

    sal_uInt16 nColsReq = nCurCol + nColSpan;       // benoetigte Spalten
    sal_uInt16 nRowsReq = nCurRow + nRowSpan;       // benoetigte Zeilen
    sal_uInt16 i, j;

    // falls wir mehr Spalten benoetigen als wir zur Zeit haben,
    // muessen wir in allen Zeilen noch Zellen hinzufuegen
    if( nCols < nColsReq )
    {
        for( i=nCols; i<nColsReq; i++ )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            pColumns->push_back( new HTMLTableColumn );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            m_pColumns->push_back(o3tl::make_unique<HTMLTableColumn>());
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        for( i=0; i<nRows; i++ )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            (*pRows)[i].Expand( nColsReq, i<nCurRow );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            (*m_pRows)[i]->Expand( nColsReq, i<nCurRow );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        nCols = nColsReq;
        OSL_ENSURE(pColumns->size() == nCols,
                "wrong number of columns after expanding");
    }
    if( nColsReq > nFilledCols )
        nFilledCols = nColsReq;

    // falls wir mehr Zeilen benoetigen als wir zur Zeit haben,
    // muessen wir noch neue Zeilen hinzufuegen
    if( nRows < nRowsReq )
    {
        for( i=nRows; i<nRowsReq; i++ )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            pRows->push_back( new HTMLTableRow(nCols) );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            m_pRows->push_back(o3tl::make_unique<HTMLTableRow>(nCols));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        nRows = nRowsReq;
        OSL_ENSURE(nRows == pRows->size(), "wrong number of rows in Insert");
    }

    // Testen, ob eine Ueberschneidung vorliegt und diese
    // gegebenfalls beseitigen
    sal_uInt16 nSpanedCols = 0;
    if( nCurRow>0 )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableRow *pCurRow = &(*pRows)[nCurRow];
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableRow *const pCurRow = (*m_pRows)[nCurRow].get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        for( i=nCurCol; i<nColsReq; i++ )
        {
            HTMLTableCell *pCell = pCurRow->GetCell(i);
            if( pCell->GetContents() )
            {
                // Der Inhalt reicht von einer weiter oben stehenden Zelle
                // hier herein. Inhalt und Farbe der Zelle sind deshalb in
                // jedem Fall noch dort verankert und koennen deshalb
                // ueberschrieben werden bzw. von ProtectRowSpan geloescht
                // (Inhalt) oder kopiert (Farbe) werden.
                nSpanedCols = i + pCell->GetColSpan();
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                FixRowSpan( nCurRow-1, i, pCell->GetContents() );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                FixRowSpan( nCurRow-1, i, pCell->GetContents().get() );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                if( pCell->GetRowSpan() > nRowSpan )
                    ProtectRowSpan( nRowsReq, i,
                                    pCell->GetRowSpan()-nRowSpan );
            }
        }
        for( i=nColsReq; i<nSpanedCols; i++ )
        {
            // Auch diese Inhalte sind in jedem Fall nich in der Zeile
            // darueber verankert.
            HTMLTableCell *pCell = pCurRow->GetCell(i);
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            FixRowSpan( nCurRow-1, i, pCell->GetContents() );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            FixRowSpan( nCurRow-1, i, pCell->GetContents().get() );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            ProtectRowSpan( nCurRow, i, pCell->GetRowSpan() );
        }
    }

    // Fill the cells
    for( i=nColSpan; i>0; i-- )
    {
        for( j=nRowSpan; j>0; j-- )
        {
            const bool bCovered = i != nColSpan || j != nRowSpan;
            GetCell( nRowsReq-j, nColsReq-i )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                ->Set( pCnts, j, i, eVertOrient, pBGBrushItem, pBoxItem,
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                ->Set( rCnts, j, i, eVertOrient, rBGBrushItem, rBoxItem,
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                       bHasNumFmt, nNumFmt, bHasValue, nValue, bNoWrap, bCovered );
        }
    }

    Size aTwipSz( bRelWidth ? 0 : nCellWidth, nCellHeight );
    if( (aTwipSz.Width() || aTwipSz.Height()) && Application::GetDefaultDevice() )
    {
        aTwipSz = Application::GetDefaultDevice()
                    ->PixelToLogic( aTwipSz, MapMode( MAP_TWIP ) );
    }

    // die Breite nur in die erste Zelle setzen!
    if( nCellWidth )
    {
        sal_uInt16 nTmp = bRelWidth ? nCellWidth : (sal_uInt16)aTwipSz.Width();
        GetCell( nCurRow, nCurCol )->SetWidth( nTmp, bRelWidth );
    }

    // Ausserdem noch die Hoehe merken
    if( nCellHeight && 1==nRowSpan )
    {
        if( nCellHeight < MINLAY )
            nCellHeight = MINLAY;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        (*pRows)[nCurRow].SetHeight( (sal_uInt16)aTwipSz.Height() );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        (*m_pRows)[nCurRow]->SetHeight( (sal_uInt16)aTwipSz.Height() );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }

    // den Spaltenzaehler hinter die neuen Zellen setzen
    nCurCol = nColsReq;
    if( nSpanedCols > nCurCol )
        nCurCol = nSpanedCols;

    // und die naechste freie Zelle suchen
    while( nCurCol<nCols && GetCell(nCurRow,nCurCol)->IsUsed() )
        nCurCol++;
}

inline void HTMLTable::CloseSection( bool bHead )
{
    // die vorhergende Section beenden, falls es schon eine Zeile gibt
    OSL_ENSURE( nCurRow<=nRows, "ungeultige aktuelle Zeile" );
    if( nCurRow>0 && nCurRow<=nRows )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        (*pRows)[nCurRow-1].SetEndOfGroup();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        (*m_pRows)[nCurRow-1]->SetEndOfGroup();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if( bHead )
        nHeadlineRepeat = nCurRow;
}

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
void HTMLTable::OpenRow( SvxAdjust eAdjust, sal_Int16 eVertOrient,
                         SvxBrushItem *pBGBrushItem )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
void HTMLTable::OpenRow(SvxAdjust eAdjust, sal_Int16 eVertOrient,
                        std::unique_ptr<SvxBrushItem>& rBGBrushItem)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
{
    sal_uInt16 nRowsReq = nCurRow+1;    // Anzahl benoetigter Zeilen;

    // die naechste Zeile anlegen, falls sie nicht schon da ist
    if( nRows<nRowsReq )
    {
        for( sal_uInt16 i=nRows; i<nRowsReq; i++ )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            pRows->push_back( new HTMLTableRow(nCols) );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            m_pRows->push_back(o3tl::make_unique<HTMLTableRow>(nCols));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        nRows = nRowsReq;
        OSL_ENSURE( nRows==pRows->size(),
                "Zeilenzahl in OpenRow stimmt nicht" );
    }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableRow *pCurRow = &((*pRows)[nCurRow]);
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableRow *const pCurRow = (*m_pRows)[nCurRow].get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    pCurRow->SetAdjust( eAdjust );
    pCurRow->SetVertOri( eVertOrient );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( pBGBrushItem )
        (*pRows)[nCurRow].SetBGBrush( pBGBrushItem );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if (rBGBrushItem)
        (*m_pRows)[nCurRow]->SetBGBrush(rBGBrushItem);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    // den Spaltenzaehler wieder an den Anfang setzen
    nCurCol=0;

    // und die naechste freie Zelle suchen
    while( nCurCol<nCols && GetCell(nCurRow,nCurCol)->IsUsed() )
        nCurCol++;
}

void HTMLTable::CloseRow( bool bEmpty )
{
    OSL_ENSURE( nCurRow<nRows, "aktulle Zeile hinter dem Tabellenende" );

    // leere Zellen bekommen einfach einen etwas dickeren unteren Rand!
    if( bEmpty )
    {
        if( nCurRow > 0 )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            (*pRows)[nCurRow-1].IncEmptyRows();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            (*m_pRows)[nCurRow-1]->IncEmptyRows();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        return;
    }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableRow *pRow = &(*pRows)[nCurRow];
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableRow *const pRow = (*m_pRows)[nCurRow].get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    // den COLSPAN aller leeren Zellen am Zeilenende so anpassen, dass
    // eine Zelle daraus wird. Das kann man hier machen (und auf keinen
    // Fall frueher), weill jetzt keine Zellen mehr in die Zeile eingefuegt
    // werden.
    sal_uInt16 i=nCols;
    while( i )
    {
        HTMLTableCell *pCell = pRow->GetCell(--i);
        if( !pCell->GetContents() )
        {
            sal_uInt16 nColSpan = nCols-i;
            if( nColSpan > 1 )
                pCell->SetColSpan( nColSpan );
        }
        else
            break;
    }

    nCurRow++;
}

inline void HTMLTable::CloseColGroup( sal_uInt16 nSpan, sal_uInt16 _nWidth,
                                      bool bRelWidth, SvxAdjust eAdjust,
                                      sal_Int16 eVertOrient )
{
    if( nSpan )
        InsertCol( nSpan, _nWidth, bRelWidth, eAdjust, eVertOrient );

    OSL_ENSURE( nCurCol<=nCols, "ungueltige Spalte" );
    if( nCurCol>0 && nCurCol<=nCols )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        ((*pColumns)[nCurCol-1]).SetEndOfGroup();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        ((*m_pColumns)[nCurCol-1])->SetEndOfGroup();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
}

void HTMLTable::InsertCol( sal_uInt16 nSpan, sal_uInt16 nColWidth, bool bRelWidth,
                           SvxAdjust eAdjust, sal_Int16 eVertOrient )
{
    // #i35143# - no columns, if rows already exist.
    if ( nRows > 0 )
        return;

    sal_uInt16 i;

    if( !nSpan )
        nSpan = 1;

    sal_uInt16 nColsReq = nCurCol + nSpan;      // benoetigte Spalten

    if( nCols < nColsReq )
    {
        for( i=nCols; i<nColsReq; i++ )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            pColumns->push_back( new HTMLTableColumn );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            m_pColumns->push_back(o3tl::make_unique<HTMLTableColumn>());
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        nCols = nColsReq;
    }

    Size aTwipSz( bRelWidth ? 0 : nColWidth, 0 );
    if( aTwipSz.Width() && Application::GetDefaultDevice() )
    {
        aTwipSz = Application::GetDefaultDevice()
                    ->PixelToLogic( aTwipSz, MapMode( MAP_TWIP ) );
    }

    for( i=nCurCol; i<nColsReq; i++ )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableColumn *pCol = &(*pColumns)[i];
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableColumn *const pCol = (*m_pColumns)[i].get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        sal_uInt16 nTmp = bRelWidth ? nColWidth : (sal_uInt16)aTwipSz.Width();
        pCol->SetWidth( nTmp, bRelWidth );
        pCol->SetAdjust( eAdjust );
        pCol->SetVertOri( eVertOrient );
    }

    bColSpec = true;

    nCurCol = nColsReq;
}

void HTMLTable::CloseTable()
{
    sal_uInt16 i;

    // Die Anzahl der Tabellenzeilen richtet sich nur nach den
    // <TR>-Elementen (d.h. nach nCurRow). Durch ROWSPAN aufgespannte
    // Zeilen hinter Zeile nCurRow muessen wir deshalb loeschen
    // und vor allem aber den ROWSPAN in den darueberliegenden Zeilen
    // anpassen.
    if( nRows>nCurRow )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableRow *pPrevRow = &(*pRows)[nCurRow-1];
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableRow *const pPrevRow = (*m_pRows)[nCurRow-1].get();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTableCell *pCell;
        for( i=0; i<nCols; i++ )
            if( ( (pCell=(pPrevRow->GetCell(i))), (pCell->GetRowSpan()) > 1 ) )
            {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                FixRowSpan( nCurRow-1, i, pCell->GetContents() );
                ProtectRowSpan( nCurRow, i, (*pRows)[nCurRow].GetCell(i)->GetRowSpan() );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                FixRowSpan( nCurRow-1, i, pCell->GetContents().get() );
                ProtectRowSpan(nCurRow, i, (*m_pRows)[nCurRow]->GetCell(i)->GetRowSpan());
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            }
        for( i=nRows-1; i>=nCurRow; i-- )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            pRows->erase(pRows->begin() + i);
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            m_pRows->erase(m_pRows->begin() + i);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        nRows = nCurRow;
    }

    // falls die Tabelle keine Spalte hat, muessen wir eine hinzufuegen
    if( 0==nCols )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pColumns->push_back( new HTMLTableColumn );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        m_pColumns->push_back(o3tl::make_unique<HTMLTableColumn>());
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        for( i=0; i<nRows; i++ )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            (*pRows)[i].Expand(1);
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            (*m_pRows)[i]->Expand(1);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        nCols = 1;
        nFilledCols = 1;
    }

    // falls die Tabelle keine Zeile hat, muessen wir eine hinzufuegen
    if( 0==nRows )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pRows->push_back( new HTMLTableRow(nCols) );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        m_pRows->push_back(o3tl::make_unique<HTMLTableRow>(nCols));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        nRows = 1;
        nCurRow = 1;
    }

    if( nFilledCols < nCols )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pColumns->erase( pColumns->begin() + nFilledCols, pColumns->begin() + nCols );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        m_pColumns->erase( m_pColumns->begin() + nFilledCols, m_pColumns->begin() + nCols );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        for( i=0; i<nRows; i++ )
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            (*pRows)[i].Shrink( nFilledCols );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            (*m_pRows)[i]->Shrink( nFilledCols );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        nCols = nFilledCols;
    }
}

void HTMLTable::_MakeTable( SwTableBox *pBox )
{
    SwTableLines& rLines = (pBox ? pBox->GetTabLines()
                                 : ((SwTable *)pSwTable)->GetTabLines() );

    // jetzt geht's richtig los ...

    for( sal_uInt16 i=0; i<nRows; i++ )
    {
        SwTableLine *pLine = MakeTableLine( pBox, i, 0, i+1, nCols );
        if( pBox || i > 0 )
            rLines.push_back( pLine );
    }
}

/* Wie werden Tabellen ausgerichtet?

erste Zeile: ohne Absatz-Einzuege
zweite Zeile: mit Absatz-Einzuegen

ALIGN=          LEFT            RIGHT           CENTER          -
-------------------------------------------------------------------------
xxx bei Tabellen mit WIDTH=nn% ist die Prozent-Angabe von Bedeutung:
xxx nn = 100        text::HoriOrientation::FULL       text::HoriOrientation::FULL       text::HoriOrientation::FULL       text::HoriOrientation::FULL %
xxx             text::HoriOrientation::NONE       text::HoriOrientation::NONE       text::HoriOrientation::NONE %     text::HoriOrientation::NONE %
xxx nn < 100        Rahmen F        Rahmen F        text::HoriOrientation::CENTER %   text::HoriOrientation::LEFT %
xxx             Rahmen F        Rahmen F        text::HoriOrientation::CENTER %   text::HoriOrientation::NONE %

bei Tabellen mit WIDTH=nn% ist die Prozent-Angabe von Bedeutung:
nn = 100        text::HoriOrientation::LEFT       text::HoriOrientation::RIGHT      text::HoriOrientation::CENTER %   text::HoriOrientation::LEFT %
                text::HoriOrientation::LEFT_AND   text::HoriOrientation::RIGHT      text::HoriOrientation::CENTER %   text::HoriOrientation::LEFT_AND %
nn < 100        Rahmen F        Rahmen F        text::HoriOrientation::CENTER %   text::HoriOrientation::LEFT %
                Rahmen F        Rahmen F        text::HoriOrientation::CENTER %   text::HoriOrientation::NONE %

sonst die berechnete Breite w
w = avail*      text::HoriOrientation::LEFT       text::HoriOrientation::RIGHT      text::HoriOrientation::CENTER     text::HoriOrientation::LEFT
                HORI_LEDT_AND   text::HoriOrientation::RIGHT      text::HoriOrientation::CENTER     text::HoriOrientation::LEFT_AND
w < avail       Rahmen L        Rahmen L        text::HoriOrientation::CENTER     text::HoriOrientation::LEFT
                Rahmen L        Rahmen L        text::HoriOrientation::CENTER     text::HoriOrientation::NONE

xxx *) wenn fuer die Tabelle keine Groesse angegeben wurde, wird immer
xxx   text::HoriOrientation::FULL genommen

*/

void HTMLTable::MakeTable( SwTableBox *pBox, sal_uInt16 nAbsAvail,
                           sal_uInt16 nRelAvail, sal_uInt16 nAbsLeftSpace,
                           sal_uInt16 nAbsRightSpace, sal_uInt16 nInhAbsSpace )
{
    OSL_ENSURE( nRows>0 && nCols>0 && nCurRow==nRows,
            "Wurde CloseTable nicht aufgerufen?" );

    OSL_ENSURE( (pLayoutInfo==0) == (this==pTopTable),
            "Top-Tabelle hat keine Layout-Info oder umgekehrt" );

    if( this==pTopTable )
    {
        // Umrandung der Tabelle und aller in ihr enthaltenen berechnen
        SetBorders();

        // Schritt 1: Die benoetigten Layout-Strukturen werden angelegt
        // (inklusive Tabellen in Tabellen).
        CreateLayoutInfo();

        // Schritt 2: Die minimalen und maximalen Spaltenbreiten werden
        // berechnet (inklusive Tabellen in Tabellen). Da wir noch keine
        // Boxen haben, arabeiten wir noch auf den Start-Nodes.
        pLayoutInfo->AutoLayoutPass1();
    }

    // Schritt 3: Die tatsaechlichen Spaltenbreiten dieser Tabelle werden
    // berechnet (nicht von Tabellen in Tabellen). Dies muss jetzt schon
    // sein, damit wir entscheiden koennen ob Filler-Zellen benoetigt werden
    // oder nicht (deshalb war auch Pass1 schon noetig).
    pLayoutInfo->AutoLayoutPass2( nAbsAvail, nRelAvail, nAbsLeftSpace,
                                  nAbsRightSpace, nInhAbsSpace );

    if( this!=pTopTable )
    {
        // die linke und rechte Umrandung der Tabelle kann jetzt entgueltig
        // festgelegt werden
        if( pLayoutInfo->GetRelRightFill() == 0 )
        {
            if( !bRightBorder )
            {
                // linke Umrandung von auesserer Tabelle uebernehmen
                if( bInhRightBorder )
                {
                    bRightBorder = true;
                    aRightBorderLine = aInhRightBorderLine;
                }
            }
            else
            {
                // Umrandung nur setzen, wenn es erlaubt ist
                bRightBorder = bRightAlwd;
            }
        }

        if( pLayoutInfo->GetRelLeftFill() == 0 &&
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            !((*pColumns)[0]).bLeftBorder &&
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            !((*m_pColumns)[0])->bLeftBorder &&
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            bInhLeftBorder )
        {
            // ggf. rechte Umrandung von auesserer Tabelle uebernehmen
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            ((*pColumns)[0]).bLeftBorder = true;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            ((*m_pColumns)[0])->bLeftBorder = true;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            aLeftBorderLine = aInhLeftBorderLine;
        }
    }

    // Fuer die Top-Table muss die Ausrichtung gesetzt werden
    if( this==pTopTable )
    {
        sal_Int16 eHoriOri;
        if( bForceFrame )
        {
            // Die Tabelle soll in einen Rahmen und ist auch schmaler
            // als der verfuegbare Platz und nicht 100% breit.
            // Dann kommt sie in einen Rahmen
            eHoriOri = bPrcWidth ? text::HoriOrientation::FULL : text::HoriOrientation::LEFT;
        }
        else switch( eTableAdjust )
        {
            // Die Tabelle passt entweder auf die Seite, soll aber in keinen
            // Rahmen oder sie ist Breiter als die Seite und soll deshalb
            // in keinen Rahmen

        case SVX_ADJUST_RIGHT:
            // in rechtsbuendigen Tabellen kann nicht auf den rechten
            // Rand Ruecksicht genommen werden
            eHoriOri = text::HoriOrientation::RIGHT;
            break;
        case SVX_ADJUST_CENTER:
            // zentrierte Tabellen nehmen keine Ruecksicht auf Raender!
            eHoriOri = text::HoriOrientation::CENTER;
            break;
        case SVX_ADJUST_LEFT:
        default:
            // linksbuendige Tabellen nehmen nur auf den linken Rand
            // Ruecksicht
            eHoriOri = nLeftMargin ? text::HoriOrientation::LEFT_AND_WIDTH : text::HoriOrientation::LEFT;
            break;
        }

        // das Tabellenform holen und anpassen
        SwFrmFmt *pFrmFmt = pSwTable->GetFrmFmt();
        pFrmFmt->SetFmtAttr( SwFmtHoriOrient(0,eHoriOri) );
        if( text::HoriOrientation::LEFT_AND_WIDTH==eHoriOri )
        {
            OSL_ENSURE( nLeftMargin || nRightMargin,
                    "Da gibt's wohl noch Reste von relativen Breiten" );

            // The right margin will be ignored anyway.
            SvxLRSpaceItem aLRItem( pSwTable->GetFrmFmt()->GetLRSpace() );
            aLRItem.SetLeft( nLeftMargin );
            aLRItem.SetRight( nRightMargin );
            pFrmFmt->SetFmtAttr( aLRItem );
        }

        if( bPrcWidth && text::HoriOrientation::FULL!=eHoriOri )
        {
            pFrmFmt->LockModify();
            SwFmtFrmSize aFrmSize( pFrmFmt->GetFrmSize() );
            aFrmSize.SetWidthPercent( (sal_uInt8)nWidth );
            pFrmFmt->SetFmtAttr( aFrmSize );
            pFrmFmt->UnlockModify();
        }
    }

    // die Default Line- und Box-Formate holen
    if( this==pTopTable )
    {
        // die erste Box merken und aus der ersten Zeile ausketten
        SwTableLine *pLine1 = (pSwTable->GetTabLines())[0];
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pBox1 = (pLine1->GetTabBoxes())[0];
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        m_xBox1.reset((pLine1->GetTabBoxes())[0]);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pLine1->GetTabBoxes().erase(pLine1->GetTabBoxes().begin());

        pLineFmt = (SwTableLineFmt*)pLine1->GetFrmFmt();
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pBoxFmt = (SwTableBoxFmt*)pBox1->GetFrmFmt();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pBoxFmt = static_cast<SwTableBoxFmt*>(m_xBox1->GetFrmFmt());
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }
    else
    {
        pLineFmt = (SwTableLineFmt*)pTopTable->pLineFmt;
        pBoxFmt = (SwTableBoxFmt*)pTopTable->pBoxFmt;
    }

    // ggf. muessen fuer Tabellen in Tabellen "Filler"-Zellen eingefuegt
    // werden
    if( this != pTopTable &&
        ( pLayoutInfo->GetRelLeftFill() > 0  ||
          pLayoutInfo->GetRelRightFill() > 0 ) )
    {
        OSL_ENSURE( pBox, "kein TableBox fuer Tabelle in Tabelle" );

        SwTableLines& rLines = pBox->GetTabLines();

        // dazu brauchen wir erstmal ein eine neue Table-Line in der Box
        SwTableLine *pLine =
            new SwTableLine( pLineFrmFmtNoHeight ? pLineFrmFmtNoHeight
                                                 : pLineFmt, 0, pBox );
        rLines.push_back( pLine );

        // Sicherstellen, dass wie ein Format ohne Hoehe erwischt haben
        if( !pLineFrmFmtNoHeight )
        {
            // sonst muessen wir die Hoehe aus dem Attribut entfernen
            // und koennen uns das Format merken
            pLineFrmFmtNoHeight = (SwTableLineFmt*)pLine->ClaimFrmFmt();

            ResetLineFrmFmtAttrs( pLineFrmFmtNoHeight );
        }

        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        SwTableBox *pNewBox;

        // ggf. links eine Zelle einfuegen
        if( pLayoutInfo->GetRelLeftFill() > 0 )
        {
            // pPrevStNd ist der Vorgaenger-Start-Node der Tabelle. Den
            // "Filler"-Node fuegen wir einfach dahinter ein ...
            pPrevStNd = pParser->InsertTableSection( pPrevStNd );

            pNewBox = NewTableBox( pPrevStNd, pLine );
            rBoxes.push_back( pNewBox );
            FixFillerFrameFmt( pNewBox, false );
            pLayoutInfo->SetLeftFillerBox( pNewBox );
        }

        // jetzt die Tabelle bearbeiten
        pNewBox = new SwTableBox( pBoxFmt, 0, pLine );
        rBoxes.push_back( pNewBox );

        SwFrmFmt *pFrmFmt = pNewBox->ClaimFrmFmt();
        pFrmFmt->ResetFmtAttr( RES_BOX );
        pFrmFmt->ResetFmtAttr( RES_BACKGROUND );
        pFrmFmt->ResetFmtAttr( RES_VERT_ORIENT );
        pFrmFmt->ResetFmtAttr( RES_BOXATR_FORMAT );

        _MakeTable( pNewBox );

        // und noch ggf. rechts eine Zelle einfuegen
        if( pLayoutInfo->GetRelRightFill() > 0 )
        {
            const SwStartNode *pStNd =
                GetPrevBoxStartNode( USHRT_MAX, USHRT_MAX );
            pStNd = pParser->InsertTableSection( pStNd );

            pNewBox = NewTableBox( pStNd, pLine );
            rBoxes.push_back( pNewBox );

            FixFillerFrameFmt( pNewBox, true );
            pLayoutInfo->SetRightFillerBox( pNewBox );
        }
    }
    else
    {
        _MakeTable( pBox );
    }

    // zum Schluss fuehren wir noch eine Garbage-Collection fuer die
    // Top-Level-Tabelle durch
    if( this==pTopTable )
    {
        if( 1==nRows && nHeight && 1==pSwTable->GetTabLines().size() )
        {
            // Hoehe einer einzeiligen Tabelle als Mindesthoehe der
            // Zeile setzen. (War mal fixe Hoehe, aber das gibt manchmal
            // Probleme (fix #34972#) und ist auch nicht Netscape 4.0
            // konform
            nHeight = pParser->ToTwips( nHeight );
            if( nHeight < MINLAY )
                nHeight = MINLAY;

            (pSwTable->GetTabLines())[0]->ClaimFrmFmt();
            (pSwTable->GetTabLines())[0]->GetFrmFmt()
                ->SetFmtAttr( SwFmtFrmSize( ATT_MIN_SIZE, 0, nHeight ) );
        }

        if( GetBGBrush() )
            pSwTable->GetFrmFmt()->SetFmtAttr( *GetBGBrush() );

        ((SwTable *)pSwTable)->SetRowsToRepeat( static_cast< sal_uInt16 >(nHeadlineRepeat) );
        ((SwTable *)pSwTable)->GCLines();

        bool bIsInFlyFrame = pContext && pContext->GetFrmFmt();
        if( bIsInFlyFrame && !nWidth )
        {
            SvxAdjust eTblAdjust = GetTableAdjust(false);
            if( eTblAdjust != SVX_ADJUST_LEFT &&
                eTblAdjust != SVX_ADJUST_RIGHT )
            {
                // Wenn eine Tabelle ohne Breitenangabe nicht links oder
                // rechts umflossen werden soll, dann stacken wir sie
                // in einem Rahmen mit 100%-Breite, damit ihre Groesse
                // angepasst wird. Der Rahmen darf nicht angepasst werden.
                OSL_ENSURE( HasToFly(), "Warum ist die Tabelle in einem Rahmen?" );
                sal_uInt32 nMin = pLayoutInfo->GetMin();
                if( nMin > USHRT_MAX )
                    nMin = USHRT_MAX;
                SwFmtFrmSize aFlyFrmSize( ATT_VAR_SIZE, (SwTwips)nMin, MINLAY );
                aFlyFrmSize.SetWidthPercent( 100 );
                pContext->GetFrmFmt()->SetFmtAttr( aFlyFrmSize );
                bIsInFlyFrame = false;
            }
            else
            {
                // Links und rechts ausgerichtete Tabellen ohne Breite
                // duerfen leider nicht in der Breite angepasst werden, denn
                // sie wuerden nur schrumpfen aber nie wachsen.
                pLayoutInfo->SetMustNotRecalc( true );
                if( pContext->GetFrmFmt()->GetAnchor().GetCntntAnchor()
                    ->nNode.GetNode().FindTableNode() )
                {
                    sal_uInt32 nMax = pLayoutInfo->GetMax();
                    if( nMax > USHRT_MAX )
                        nMax = USHRT_MAX;
                    SwFmtFrmSize aFlyFrmSize( ATT_VAR_SIZE, (SwTwips)nMax, MINLAY );
                    pContext->GetFrmFmt()->SetFmtAttr( aFlyFrmSize );
                    bIsInFlyFrame = false;
                }
                else
                {
                    pLayoutInfo->SetMustNotResize( true );
                }
            }
        }
        pLayoutInfo->SetMayBeInFlyFrame( bIsInFlyFrame );

        // Nur Tabellen mit relativer Breite oder ohne Breite muessen
        // angepasst werden.
        pLayoutInfo->SetMustResize( bPrcWidth || !nWidth );

        pLayoutInfo->SetWidths();

#ifndef NO_LIBO_MODIFIED_HTML_TABLE_LEAK_FIX
        bLayoutInfoOwner = false;
#endif	// !NO_LIBO_MODIFIED_HTML_TABLE_LEAK_FIX
        ((SwTable *)pSwTable)->SetHTMLTableLayout( pLayoutInfo );

        if( pResizeDrawObjs )
        {
            sal_uInt16 nCount = pResizeDrawObjs->size();
            for( sal_uInt16 i=0; i<nCount; i++ )
            {
                SdrObject *pObj = (*pResizeDrawObjs)[i];
                sal_uInt16 nRow = (*pDrawObjPrcWidths)[3*i];
                sal_uInt16 nCol = (*pDrawObjPrcWidths)[3*i+1];
                sal_uInt8 nPrcWidth = (sal_uInt8)(*pDrawObjPrcWidths)[3*i+2];

                SwHTMLTableLayoutCell *pLayoutCell =
                    pLayoutInfo->GetCell( nRow, nCol );
                sal_uInt16 nColSpan = pLayoutCell->GetColSpan();

                sal_uInt16 nWidth2, nDummy;
                pLayoutInfo->GetAvail( nCol, nColSpan, nWidth2, nDummy );
                nWidth2 = nWidth2 - pLayoutInfo->GetLeftCellSpace( nCol, nColSpan );
                nWidth2 = nWidth2 - pLayoutInfo->GetRightCellSpace( nCol, nColSpan );
                nWidth2 = static_cast< sal_uInt16 >(((long)nWidth * nPrcWidth) / 100);

                pParser->ResizeDrawObject( pObj, nWidth2 );
            }
        }
    }
}

void HTMLTable::SetTable( const SwStartNode *pStNd, _HTMLTableContext *pCntxt,
                          sal_uInt16 nLeft, sal_uInt16 nRight,
                          const SwTable *pSwTab, bool bFrcFrame )
{
    pPrevStNd = pStNd;
    pSwTable = pSwTab;
    pContext = pCntxt;

    nLeftMargin = nLeft;
    nRightMargin = nRight;

    bForceFrame = bFrcFrame;
}

void HTMLTable::RegisterDrawObject( SdrObject *pObj, sal_uInt8 nPrcWidth )
{
    if( !pResizeDrawObjs )
        pResizeDrawObjs = new SdrObjects;
    pResizeDrawObjs->push_back( pObj );

    if( !pDrawObjPrcWidths )
        pDrawObjPrcWidths = new std::vector<sal_uInt16>;
    pDrawObjPrcWidths->push_back( nCurRow );
    pDrawObjPrcWidths->push_back( nCurCol );
    pDrawObjPrcWidths->push_back( (sal_uInt16)nPrcWidth );
}

void HTMLTable::MakeParentContents()
{
    if( !GetContext() && !HasParentSection() )
    {
        SetParentContents(
            pParser->InsertTableContents( GetIsParentHeader() ) );

        SetHasParentSection( true );
    }
}

_HTMLTableContext::~_HTMLTableContext()
{
    delete pPos;
}

void _HTMLTableContext::SavePREListingXMP( SwHTMLParser& rParser )
{
    bRestartPRE = rParser.IsReadPRE();
    bRestartXMP = rParser.IsReadXMP();
    bRestartListing = rParser.IsReadListing();
    rParser.FinishPREListingXMP();
}

void _HTMLTableContext::RestorePREListingXMP( SwHTMLParser& rParser )
{
    rParser.FinishPREListingXMP();

    if( bRestartPRE )
        rParser.StartPRE();

    if( bRestartXMP )
        rParser.StartXMP();

    if( bRestartListing )
        rParser.StartListing();
}

const SwStartNode *SwHTMLParser::InsertTableSection
    ( const SwStartNode *pPrevStNd )
{
    OSL_ENSURE( pPrevStNd, "Start-Node ist NULL" );

    pCSS1Parser->SetTDTagStyles();
    SwTxtFmtColl *pColl = pCSS1Parser->GetTxtCollFromPool( RES_POOLCOLL_TABLE );

    const SwStartNode *pStNd;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if (pTable->bFirstCell )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if (m_xTable->bFirstCell )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
        SwNode *const pNd = & pPam->GetPoint()->nNode.GetNode();
        pNd->GetTxtNode()->ChgFmtColl( pColl );
        pStNd = pNd->FindTableBoxStartNode();
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pTable->bFirstCell = false;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        m_xTable->bFirstCell = false;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }
    else
    {
        const SwNode* pNd;
        if( pPrevStNd->IsTableNode() )
            pNd = pPrevStNd;
        else
            pNd = pPrevStNd->EndOfSectionNode();
        SwNodeIndex nIdx( *pNd, 1 );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pStNd = pDoc->GetNodes().MakeTextSection( nIdx, SwTableBoxStartNode,
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pStNd = m_xDoc->GetNodes().MakeTextSection( nIdx, SwTableBoxStartNode,
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                                  pColl );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pTable->IncBoxCount();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        m_xTable->IncBoxCount();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }

    //Added defaults to CJK and CTL
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SwCntntNode *pCNd = pDoc->GetNodes()[pStNd->GetIndex()+1] ->GetCntntNode();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SwCntntNode *pCNd = m_xDoc->GetNodes()[pStNd->GetIndex()+1] ->GetCntntNode();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SvxFontHeightItem aFontHeight( 40, 100, RES_CHRATR_FONTSIZE );
    pCNd->SetAttr( aFontHeight );
    SvxFontHeightItem aFontHeightCJK( 40, 100, RES_CHRATR_CJK_FONTSIZE );
    pCNd->SetAttr( aFontHeightCJK );
    SvxFontHeightItem aFontHeightCTL( 40, 100, RES_CHRATR_CTL_FONTSIZE );
    pCNd->SetAttr( aFontHeightCTL );

    return pStNd;
}

const SwStartNode *SwHTMLParser::InsertTableSection( sal_uInt16 nPoolId )
{
    switch( nPoolId )
    {
    case RES_POOLCOLL_TABLE_HDLN:
        pCSS1Parser->SetTHTagStyles();
        break;
    case RES_POOLCOLL_TABLE:
        pCSS1Parser->SetTDTagStyles();
        break;
    }

    SwTxtFmtColl *pColl = pCSS1Parser->GetTxtCollFromPool( nPoolId );

    SwNode *const pNd = & pPam->GetPoint()->nNode.GetNode();
    const SwStartNode *pStNd;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if (pTable->bFirstCell)
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if (m_xTable->bFirstCell)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pNd->GetTxtNode()->ChgFmtColl( pColl );
        pTable->bFirstCell = false;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        SwTxtNode* pTextNd = pNd->GetTxtNode();
        if (!pTextNd)
        {
            eState = SVPAR_ERROR;
            return nullptr;
        }
        pTextNd->ChgFmtColl(pColl);
        m_xTable->bFirstCell = false;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pStNd = pNd->FindTableBoxStartNode();
    }
    else
    {
        SwTableNode *pTblNd = pNd->FindTableNode();
#ifndef NO_LIBO_NULL_TABLE_NODE_FIX
        if (!pTblNd)
        {
            eState = SVPAR_ERROR;
            return nullptr;
        }
#endif	// !NO_LIBO_NULL_TABLE_NODE_FIX
        if( pTblNd->GetTable().GetHTMLTableLayout() )
        { // if there is already a HTMTableLayout, this table is already finished
          // and we have to look for the right table in the environment
            SwTableNode *pOutTbl = pTblNd;
            do {
                pTblNd = pOutTbl;
                pOutTbl = pOutTbl->StartOfSectionNode()->FindTableNode();
            } while( pOutTbl && pTblNd->GetTable().GetHTMLTableLayout() );
        }
        SwNodeIndex aIdx( *pTblNd->EndOfSectionNode() );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pStNd = pDoc->GetNodes().MakeTextSection( aIdx, SwTableBoxStartNode,
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pStNd = m_xDoc->GetNodes().MakeTextSection( aIdx, SwTableBoxStartNode,
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                                  pColl );

        pPam->GetPoint()->nNode = pStNd->GetIndex() + 1;
        SwTxtNode *pTxtNd = pPam->GetPoint()->nNode.GetNode().GetTxtNode();
        pPam->GetPoint()->nContent.Assign( pTxtNd, 0 );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pTable->IncBoxCount();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        m_xTable->IncBoxCount();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }

#ifndef NO_LIBO_NULL_TEXTNODE_FIX
    if (!pStNd)
    {
        eState = SVPAR_ERROR;
    }
#endif	// !NO_LIBO_NULL_TEXTNODE_FIX

    return pStNd;
}

SwStartNode *SwHTMLParser::InsertTempTableCaptionSection()
{
    SwTxtFmtColl *pColl = pCSS1Parser->GetTxtCollFromPool( RES_POOLCOLL_TEXT );
    SwNodeIndex& rIdx = pPam->GetPoint()->nNode;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    rIdx = pDoc->GetNodes().GetEndOfExtras();
    SwStartNode *pStNd = pDoc->GetNodes().MakeTextSection( rIdx,
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    rIdx = m_xDoc->GetNodes().GetEndOfExtras();
    SwStartNode *pStNd = m_xDoc->GetNodes().MakeTextSection( rIdx,
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                          SwNormalStartNode, pColl );

    rIdx = pStNd->GetIndex() + 1;
    pPam->GetPoint()->nContent.Assign( rIdx.GetNode().GetTxtNode(), 0 );

    return pStNd;
}

sal_Int32 SwHTMLParser::StripTrailingLF()
{
    sal_Int32 nStripped = 0;

    const sal_Int32 nLen = pPam->GetPoint()->nContent.GetIndex();
    if( nLen )
    {
        SwTxtNode* pTxtNd = pPam->GetPoint()->nNode.GetNode().GetTxtNode();
        // vorsicht, wenn Kommentare nicht uebrlesen werden!!!
        if( pTxtNd )
        {
            sal_Int32 nPos = nLen;
            sal_Int32 nLFCount = 0;
            while (nPos && ('\x0a' == pTxtNd->GetTxt()[--nPos]))
                nLFCount++;

            if( nLFCount )
            {
                if( nLFCount > 2 )
                {
                    // Bei Netscape entspricht ein Absatz-Ende zwei LFs
                    // (mit einem kommt man in die naechste Zeile, das
                    // zweite erzeugt eine Leerzeile) Diesen Abstand
                    // erreichen wie aber schon mit dem unteren
                    // Absatz-Abstand. Wenn nach den <BR> ein neuer
                    // Absatz aufgemacht wird, wird das Maximum des Abstands,
                    // der sich aus den BR und dem P ergibt genommen.
                    // Deshalb muessen wir 2 bzw. alle bei weniger
                    // als zweien loeschen
                    nLFCount = 2;
                }

                nPos = nLen - nLFCount;
                SwIndex nIdx( pTxtNd, nPos );
                pTxtNd->EraseText( nIdx, nLFCount );
                nStripped = nLFCount;
            }
        }
    }

    return nStripped;
}

SvxBrushItem* SwHTMLParser::CreateBrushItem( const Color *pColor,
                                             const OUString& rImageURL,
                                             const OUString& rStyle,
                                             const OUString& rId,
                                             const OUString& rClass )
{
    SvxBrushItem *pBrushItem = 0;

    if( !rStyle.isEmpty() || !rId.isEmpty() || !rClass.isEmpty() )
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SfxItemSet aItemSet( pDoc->GetAttrPool(), RES_BACKGROUND,
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SfxItemSet aItemSet( m_xDoc->GetAttrPool(), RES_BACKGROUND,
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                                  RES_BACKGROUND );
        SvxCSS1PropertyInfo aPropInfo;

        if( !rClass.isEmpty() )
        {
            OUString aClass( rClass );
            SwCSS1Parser::GetScriptFromClass( aClass );
            const SvxCSS1MapEntry *pClass = pCSS1Parser->GetClass( aClass );
            if( pClass )
                aItemSet.Put( pClass->GetItemSet() );
        }

        if( !rId.isEmpty() )
        {
            const SvxCSS1MapEntry *pId = pCSS1Parser->GetId( rId );
            if( pId )
                aItemSet.Put( pId->GetItemSet() );
        }

        pCSS1Parser->ParseStyleOption( rStyle, aItemSet, aPropInfo );
        const SfxPoolItem *pItem = 0;
        if( SfxItemState::SET == aItemSet.GetItemState( RES_BACKGROUND, false,
                                                   &pItem ) )
        {
            pBrushItem = new SvxBrushItem( *((const SvxBrushItem *)pItem) );
        }
    }

    if( !pBrushItem && (pColor || !rImageURL.isEmpty()) )
    {
        pBrushItem = new SvxBrushItem(RES_BACKGROUND);

        if( pColor )
            pBrushItem->SetColor(*pColor);

        if( !rImageURL.isEmpty() )
        {
            pBrushItem->SetGraphicLink( URIHelper::SmartRel2Abs( INetURLObject(sBaseURL), rImageURL, Link(), false) );
            pBrushItem->SetGraphicPos( GPOS_TILED );
        }
    }

    return pBrushItem;
}

class _SectionSaveStruct : public SwPendingStackData
{
    sal_uInt16 nBaseFontStMinSave, nFontStMinSave, nFontStHeadStartSave;
    sal_uInt16 nDefListDeepSave, nContextStMinSave, nContextStAttrMinSave;

public:

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTable *pTable;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::shared_ptr<HTMLTable> m_xTable;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    _SectionSaveStruct( SwHTMLParser& rParser );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    virtual ~_SectionSaveStruct();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#if OSL_DEBUG_LEVEL > 0
    sal_uInt16 GetContextStAttrMin() const { return nContextStAttrMinSave; }
#endif
    void Restore( SwHTMLParser& rParser );
};

_SectionSaveStruct::_SectionSaveStruct( SwHTMLParser& rParser ) :
    nBaseFontStMinSave(0), nFontStMinSave(0), nFontStHeadStartSave(0),
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    nDefListDeepSave(0), nContextStMinSave(0), nContextStAttrMinSave(0),
    pTable( 0 )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    nDefListDeepSave(0), nContextStMinSave(0), nContextStAttrMinSave(0)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
{
    // Font-Stacks einfrieren
    nBaseFontStMinSave = rParser.nBaseFontStMin;
    rParser.nBaseFontStMin = rParser.aBaseFontStack.size();

    nFontStMinSave = rParser.nFontStMin;
    nFontStHeadStartSave = rParser.nFontStHeadStart;
    rParser.nFontStMin = rParser.aFontStack.size();

    // Kontext-Stack einfrieren
    nContextStMinSave = rParser.nContextStMin;
    nContextStAttrMinSave = rParser.nContextStAttrMin;
    rParser.nContextStMin = rParser.aContexts.size();
    rParser.nContextStAttrMin = rParser.nContextStMin;

    // und noch ein par Zaehler retten
    nDefListDeepSave = rParser.nDefListDeep;
    rParser.nDefListDeep = 0;
}

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX

_SectionSaveStruct::~_SectionSaveStruct()
{}

#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

void _SectionSaveStruct::Restore( SwHTMLParser& rParser )
{
    // Font-Stacks wieder auftauen
    sal_uInt16 nMin = rParser.nBaseFontStMin;
    if( rParser.aBaseFontStack.size() > nMin )
        rParser.aBaseFontStack.erase( rParser.aBaseFontStack.begin() + nMin,
                rParser.aBaseFontStack.end() );
    rParser.nBaseFontStMin = nBaseFontStMinSave;

    nMin = rParser.nFontStMin;
    if( rParser.aFontStack.size() > nMin )
        rParser.aFontStack.erase( rParser.aFontStack.begin() + nMin,
                rParser.aFontStack.end() );
    rParser.nFontStMin = nFontStMinSave;
    rParser.nFontStHeadStart = nFontStHeadStartSave;

    OSL_ENSURE( rParser.aContexts.size() == rParser.nContextStMin &&
            rParser.aContexts.size() == rParser.nContextStAttrMin,
            "The Context Stack was not cleaned up" );
    rParser.nContextStMin = nContextStMinSave;
    rParser.nContextStAttrMin = nContextStAttrMinSave;

    // und noch ein par Zaehler rekonstruieren
    rParser.nDefListDeep = nDefListDeepSave;

    // und ein par Flags zuruecksetzen
    rParser.bNoParSpace = false;
    rParser.nOpenParaToken = 0;

    if( !rParser.aParaAttrs.empty() )
        rParser.aParaAttrs.clear();
}

class _CellSaveStruct : public _SectionSaveStruct
{
    OUString aStyle, aId, aClass, aLang, aDir;
    OUString aBGImage;
    Color aBGColor;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    boost::shared_ptr<SvxBoxItem> m_pBoxItem;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::shared_ptr<SvxBoxItem> m_xBoxItem;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableCnts* pCnts;           // Liste aller Inhalte
    HTMLTableCnts* pCurrCnts;   // der aktuelle Inhalt oder 0
    SwNodeIndex *pNoBreakEndParaIdx;// Absatz-Index eines </NOBR>
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::shared_ptr<HTMLTableCnts> m_xCnts;              // List of all contents
    HTMLTableCnts* m_pCurrCnts;                          // current content or 0
    std::unique_ptr<SwNodeIndex> m_pNoBreakEndNodeIndex; // Paragraph index of a <NOBR>
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    double nValue;

    sal_uInt32 nNumFmt;

    sal_uInt16 nRowSpan, nColSpan, nWidth, nHeight;
    sal_Int32 nNoBreakEndCntntPos;     // Zeichen-Index eines </NOBR>

    SvxAdjust eAdjust;
    sal_Int16 eVertOri;

    bool bHead : 1;
    bool bPrcWidth : 1;
    bool bHasNumFmt : 1;
    bool bHasValue : 1;
    bool bBGColor : 1;
    bool bNoWrap : 1;       // NOWRAP-Option
    bool bNoBreak : 1;      // NOBREAK-Tag

public:

    _CellSaveStruct( SwHTMLParser& rParser, HTMLTable *pCurTable, bool bHd,
                     bool bReadOpt );

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    virtual ~_CellSaveStruct();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    void AddContents( HTMLTableCnts *pNewCnts );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTableCnts *GetFirstContents() { return pCnts; }

    void ClearIsInSection() { pCurrCnts = 0; }
    bool IsInSection() const { return pCurrCnts!=0; }
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    bool HasFirstContents() const { return m_xCnts.get(); }

    void ClearIsInSection() { m_pCurrCnts = nullptr; }
    bool IsInSection() const { return m_pCurrCnts!=nullptr; }
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    void InsertCell( SwHTMLParser& rParser, HTMLTable *pCurTable );

    bool IsHeaderCell() const { return bHead; }

    void StartNoBreak( const SwPosition& rPos );
    void EndNoBreak( const SwPosition& rPos );
    void CheckNoBreak( const SwPosition& rPos, SwDoc *pDoc );
};

_CellSaveStruct::_CellSaveStruct( SwHTMLParser& rParser, HTMLTable *pCurTable,
                                  bool bHd, bool bReadOpt ) :
    _SectionSaveStruct( rParser ),
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pCnts( 0 ),
    pCurrCnts( 0 ),
    pNoBreakEndParaIdx( 0 ),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    m_pCurrCnts( nullptr ),
    m_pNoBreakEndNodeIndex( nullptr ),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    nValue( 0.0 ),
    nNumFmt( 0 ),
    nRowSpan( 1 ),
    nColSpan( 1 ),
    nWidth( 0 ),
    nHeight( 0 ),
    nNoBreakEndCntntPos( 0 ),
    eAdjust( pCurTable->GetInheritedAdjust() ),
    eVertOri( pCurTable->GetInheritedVertOri() ),
    bHead( bHd ),
    bPrcWidth( false ),
    bHasNumFmt( false ),
    bHasValue( false ),
    bBGColor( false ),
    bNoWrap( false ),
    bNoBreak( false )
{
    OUString aNumFmt, aValue;

    if( bReadOpt )
    {
        const HTMLOptions& rOptions = rParser.GetOptions();
        for (size_t i = rOptions.size(); i; )
        {
            const HTMLOption& rOption = rOptions[--i];
            switch( rOption.GetToken() )
            {
            case HTML_O_ID:
                aId = rOption.GetString();
                break;
            case HTML_O_COLSPAN:
                nColSpan = (sal_uInt16)rOption.GetNumber();
                break;
            case HTML_O_ROWSPAN:
                nRowSpan = (sal_uInt16)rOption.GetNumber();
                break;
            case HTML_O_ALIGN:
                eAdjust = (SvxAdjust)rOption.GetEnum(
                                        aHTMLPAlignTable, static_cast< sal_uInt16 >(eAdjust) );
                break;
            case HTML_O_VALIGN:
                eVertOri = rOption.GetEnum(
                                        aHTMLTblVAlignTable, eVertOri );
                break;
            case HTML_O_WIDTH:
                nWidth = (sal_uInt16)rOption.GetNumber();   // nur fuer Netscape
                bPrcWidth = (rOption.GetString().indexOf('%') != -1);
                if( bPrcWidth && nWidth>100 )
                    nWidth = 100;
                break;
            case HTML_O_HEIGHT:
                nHeight = (sal_uInt16)rOption.GetNumber();  // nur fuer Netscape
                if( rOption.GetString().indexOf('%') != -1)
                    nHeight = 0;    // keine %-Angaben beruecksichtigen
                break;
            case HTML_O_BGCOLOR:
                // Leere BGCOLOR bei <TABLE>, <TR> und <TD>/<TH> wie Netscape
                // ignorieren, bei allen anderen Tags *wirklich* nicht.
                if( !rOption.GetString().isEmpty() )
                {
                    rOption.GetColor( aBGColor );
                    bBGColor = true;
                }
                break;
            case HTML_O_BACKGROUND:
                aBGImage = rOption.GetString();
                break;
            case HTML_O_STYLE:
                aStyle = rOption.GetString();
                break;
            case HTML_O_CLASS:
                aClass = rOption.GetString();
                break;
            case HTML_O_LANG:
                aLang = rOption.GetString();
                break;
            case HTML_O_DIR:
                aDir = rOption.GetString();
                break;
            case HTML_O_SDNUM:
                aNumFmt = rOption.GetString();
                bHasNumFmt = true;
                break;
            case HTML_O_SDVAL:
                bHasValue = true;
                aValue = rOption.GetString();
                break;
            case HTML_O_NOWRAP:
                bNoWrap = true;
                break;
            }
        }

        if( !aId.isEmpty() )
            rParser.InsertBookmark( aId );
    }

    if( bHasNumFmt )
    {
        LanguageType eLang;
        nValue = SfxHTMLParser::GetTableDataOptionsValNum(
                            nNumFmt, eLang, aValue, aNumFmt,
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            *rParser.pDoc->GetNumberFormatter() );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            *rParser.m_xDoc->GetNumberFormatter() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }

    // einen neuen Kontext anlegen, aber das drawing::Alignment-Attribut
    // nicht dort verankern, weil es noch ger keine Section gibt, in der
    // es gibt.
    sal_uInt16 nToken, nColl;
    if( bHead )
    {
        nToken = HTML_TABLEHEADER_ON;
        nColl = RES_POOLCOLL_TABLE_HDLN;
    }
    else
    {
        nToken = HTML_TABLEDATA_ON;
        nColl = RES_POOLCOLL_TABLE;
    }
#ifdef NO_LIBO_HTML_PARSER_LEAK_FIX
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( nToken, nColl, aEmptyOUStr, true );
#else	// NO_LIBO_HTML_PARSER_LEAK_FIX
    std::unique_ptr<_HTMLAttrContext> xCntxt(new _HTMLAttrContext(nToken, nColl, aEmptyOUStr, true));
#endif	// NO_LIBO_HTML_PARSER_LEAK_FIX
    if( SVX_ADJUST_END != eAdjust )
#ifdef NO_LIBO_HTML_PARSER_LEAK_FIX
        rParser.InsertAttr( &rParser.aAttrTab.pAdjust, SvxAdjustItem(eAdjust, RES_PARATR_ADJUST),
                            pCntxt );
#else	// NO_LIBO_HTML_PARSER_LEAK_FIX
        rParser.InsertAttr(&rParser.aAttrTab.pAdjust, SvxAdjustItem(eAdjust, RES_PARATR_ADJUST),
                           xCntxt.get());
#endif	// NO_LIBO_HTML_PARSER_LEAK_FIX

    if( rParser.HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SfxItemSet aItemSet( rParser.pDoc->GetAttrPool(),
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SfxItemSet aItemSet( rParser.m_xDoc->GetAttrPool(),
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                             rParser.pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( rParser.ParseStyleOptions( aStyle, aId, aClass, aItemSet,
                                       aPropInfo, &aLang, &aDir ) )
        {
            SfxPoolItem const* pItem;
            if (SfxItemState::SET == aItemSet.GetItemState(RES_BOX, false, &pItem))
            {   // fdo#41796: steal box item to set it in FixFrameFmt later!
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                m_pBoxItem.reset(dynamic_cast<SvxBoxItem *>(pItem->Clone()));
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                m_xBoxItem.reset(dynamic_cast<SvxBoxItem *>(pItem->Clone()));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                aItemSet.ClearItem(RES_BOX);
            }
#ifdef NO_LIBO_HTML_PARSER_LEAK_FIX
            rParser.InsertAttrs( aItemSet, aPropInfo, pCntxt );
#else	// NO_LIBO_HTML_PARSER_LEAK_FIX
            rParser.InsertAttrs(aItemSet, aPropInfo, xCntxt.get());
#endif	// NO_LIBO_HTML_PARSER_LEAK_FIX
        }
    }

#ifdef NO_LIBO_HTML_PARSER_LEAK_FIX
    rParser.SplitPREListingXMP( pCntxt );
#else	// NO_LIBO_HTML_PARSER_LEAK_FIX
    rParser.SplitPREListingXMP(xCntxt.get());
#endif	// NO_LIBO_HTML_PARSER_LEAK_FIX

#ifdef NO_LIBO_HTML_PARSER_LEAK_FIX
    rParser.PushContext( pCntxt );
#else	// NO_LIBO_HTML_PARSER_LEAK_FIX
    rParser.PushContext(xCntxt);
#endif	// NO_LIBO_HTML_PARSER_LEAK_FIX
}

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX

_CellSaveStruct::~_CellSaveStruct()
{
    delete pNoBreakEndParaIdx;
}

#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

void _CellSaveStruct::AddContents( HTMLTableCnts *pNewCnts )
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( pCnts )
        pCnts->Add( pNewCnts );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if (m_xCnts)
        m_xCnts->Add( pNewCnts );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    else
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pCnts = pNewCnts;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        m_xCnts.reset(pNewCnts);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pCurrCnts = pNewCnts;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    m_pCurrCnts = pNewCnts;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
}

void _CellSaveStruct::InsertCell( SwHTMLParser& rParser,
                                  HTMLTable *pCurTable )
{
#if OSL_DEBUG_LEVEL > 0
    // Die Attribute muessen schon beim Auefrauemen des Kontext-Stacks
    // entfernt worden sein, sonst ist etwas schiefgelaufen. Das
    // Checken wir mal eben ...
    // MIB 8.1.98: Wenn ausserhalb einer Zelle Attribute geoeffnet
    // wurden stehen diese noch in der Attribut-Tabelle und werden erst
    // ganz zum Schluss durch die CleanContext-Aufrufe in BuildTable
    // geloescht. Damit es in diesem Fall keine Asserts gibt findet dann
    // keine Ueberpruefung statt. Erkennen tut man diesen Fall an
    // nContextStAttrMin: Der gemerkte Wert nContextStAttrMinSave ist der
    // Wert, den nContextStAttrMin beim Start der Tabelle hatte. Und
    // der aktuelle Wert von nContextStAttrMin entspricht der Anzahl der
    // Kontexte, die beim Start der Zelle vorgefunden wurden. Sind beide
    // Werte unterschiedlich, wurden ausserhalb der Zelle Kontexte
    // angelegt und wir ueberpruefen nichts.

    if( rParser.nContextStAttrMin == GetContextStAttrMin() )
    {
        _HTMLAttr** pTbl = (_HTMLAttr**)&rParser.aAttrTab;

        for( sal_uInt16 nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
            nCnt--; ++pTbl )
        {
            OSL_ENSURE( !*pTbl, "Die Attribut-Tabelle ist nicht leer" );
        }
    }
#endif

    // jetzt muessen wir noch die Zelle an der aktuellen Position einfuegen
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    SvxBrushItem *pBrushItem =
        rParser.CreateBrushItem( bBGColor ? &aBGColor : 0, aBGImage,
                                 aStyle, aId, aClass );
    pCurTable->InsertCell( pCnts, nRowSpan, nColSpan, nWidth,
                           bPrcWidth, nHeight, eVertOri, pBrushItem, m_pBoxItem,
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::shared_ptr<SvxBrushItem> xBrushItem(
        rParser.CreateBrushItem(bBGColor ? &aBGColor : nullptr, aBGImage,
                                aStyle, aId, aClass));
    pCurTable->InsertCell( m_xCnts, nRowSpan, nColSpan, nWidth,
                           bPrcWidth, nHeight, eVertOri, xBrushItem, m_xBoxItem,
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                           bHasNumFmt, nNumFmt, bHasValue, nValue,
                           bNoWrap );
    Restore( rParser );
}

void _CellSaveStruct::StartNoBreak( const SwPosition& rPos )
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( !pCnts ||
        (!rPos.nContent.GetIndex() && pCurrCnts==pCnts &&
         pCnts->GetStartNode() &&
         pCnts->GetStartNode()->GetIndex() + 1 ==
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if( !m_xCnts ||
        (!rPos.nContent.GetIndex() && m_pCurrCnts == m_xCnts.get() &&
         m_xCnts->GetStartNode() &&
         m_xCnts->GetStartNode()->GetIndex() + 1 ==
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            rPos.nNode.GetIndex()) )
    {
        bNoBreak = true;
    }
}

void _CellSaveStruct::EndNoBreak( const SwPosition& rPos )
{
    if( bNoBreak )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        delete pNoBreakEndParaIdx;
        pNoBreakEndParaIdx = new SwNodeIndex( rPos.nNode );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        m_pNoBreakEndNodeIndex.reset( new SwNodeIndex( rPos.nNode ) );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        nNoBreakEndCntntPos = rPos.nContent.GetIndex();
        bNoBreak = false;
    }
}

void _CellSaveStruct::CheckNoBreak( const SwPosition& rPos, SwDoc * /*pDoc*/ )
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( pCnts && pCurrCnts==pCnts )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if (m_xCnts && m_pCurrCnts == m_xCnts.get())
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
        if( bNoBreak )
        {
            // <NOBR> wurde nicht beendet
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            pCnts->SetNoBreak();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            m_xCnts->SetNoBreak();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        }
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        else if( pNoBreakEndParaIdx &&
                 pNoBreakEndParaIdx->GetIndex() == rPos.nNode.GetIndex() )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        else if( m_pNoBreakEndNodeIndex &&
                 m_pNoBreakEndNodeIndex->GetIndex() == rPos.nNode.GetIndex() )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        {
            if( nNoBreakEndCntntPos == rPos.nContent.GetIndex() )
            {
                // <NOBR> wurde unmittelbar vor dem Zellen-Ende beendet
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                pCnts->SetNoBreak();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                m_xCnts->SetNoBreak();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            }
            else if( nNoBreakEndCntntPos + 1 == rPos.nContent.GetIndex() )
            {
                SwTxtNode const*const pTxtNd(rPos.nNode.GetNode().GetTxtNode());
                if( pTxtNd )
                {
                    sal_Unicode const cLast =
                            pTxtNd->GetTxt()[nNoBreakEndCntntPos];
                    if( ' '==cLast || '\x0a'==cLast )
                    {
                        // Zwischem dem </NOBR> und dem Zellen-Ende gibt es nur
                        // ein Blank oder einen Zeilenumbruch.
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                        pCnts->SetNoBreak();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        m_xCnts->SetNoBreak();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    }
                }
            }
        }
    }
}

HTMLTableCnts *SwHTMLParser::InsertTableContents(
                                        bool bHead )
{
    // eine neue Section anlegen, der PaM steht dann darin
    const SwStartNode *pStNd =
        InsertTableSection( static_cast< sal_uInt16 >(bHead ? RES_POOLCOLL_TABLE_HDLN
                                           : RES_POOLCOLL_TABLE) );

    if( GetNumInfo().GetNumRule() )
    {
        // 1. Absatz auf nicht numeriert setzen
        sal_uInt8 nLvl = GetNumInfo().GetLevel();

        SetNodeNum( nLvl, false );
    }

    // Attributierungs-Anfang neu setzen
    const SwNodeIndex& rSttPara = pPam->GetPoint()->nNode;
    sal_Int32 nSttCnt = pPam->GetPoint()->nContent.GetIndex();

    _HTMLAttr** pTbl = (_HTMLAttr**)&aAttrTab;
    for( sal_uInt16 nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
        nCnt--; ++pTbl )
    {

        _HTMLAttr *pAttr = *pTbl;
        while( pAttr )
        {
            OSL_ENSURE( !pAttr->GetPrev(), "Attribut hat Previous-Liste" );
            pAttr->nSttPara = rSttPara;
            pAttr->nEndPara = rSttPara;
            pAttr->nSttCntnt = nSttCnt;
            pAttr->nEndCntnt = nSttCnt;

            pAttr = pAttr->GetNext();
        }
    }

    return new HTMLTableCnts( pStNd );
}

sal_uInt16 SwHTMLParser::IncGrfsThatResizeTable()
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    return pTable ? pTable->IncGrfsThatResize() : 0;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    return m_xTable ? m_xTable->IncGrfsThatResize() : 0;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
}

void SwHTMLParser::RegisterDrawObjectToTable( HTMLTable *pCurTable,
                                        SdrObject *pObj, sal_uInt8 nPrcWidth )
{
    pCurTable->RegisterDrawObject( pObj, nPrcWidth );
}

void SwHTMLParser::BuildTableCell( HTMLTable *pCurTable, bool bReadOptions,
                                   bool bHead )
{
    if( !IsParserWorking() && !pPendStack )
        return;

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    _CellSaveStruct* pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::unique_ptr<_CellSaveStruct> xSaveStruct;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    int nToken = 0;
    bool bPending = false;
    if( pPendStack )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pSaveStruct = (_CellSaveStruct*)pPendStack->pData;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct.reset(static_cast<_CellSaveStruct*>(pPendStack->pData));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && pPendStack != 0;

        SaveState( nToken );
    }
    else
    {
        // <TH> bzw. <TD> wurde bereits gelesen
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        if( pTable->IsOverflowing() )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if (m_xTable->IsOverflowing())
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        {
            SaveState( 0 );
            return;
        }

        if( !pCurTable->GetContext() )
        {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            bool bTopTable = pTable==pCurTable;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            bool bTopTable = m_xTable.get() == pCurTable;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

            // die Tabelle besitzt noch keinen Inhalt, d.h. die eigentliche
            // Tabelle muss erst noch angelegt werden

            static sal_uInt16 aWhichIds[] =
            {
                RES_PARATR_SPLIT,   RES_PARATR_SPLIT,
                RES_PAGEDESC,       RES_PAGEDESC,
                RES_BREAK,          RES_BREAK,
                RES_BACKGROUND,     RES_BACKGROUND,
                RES_KEEP,           RES_KEEP,
                RES_LAYOUT_SPLIT,   RES_LAYOUT_SPLIT,
                RES_FRAMEDIR,       RES_FRAMEDIR,
                0
            };

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            SfxItemSet aItemSet( pDoc->GetAttrPool(), aWhichIds );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            SfxItemSet aItemSet( m_xDoc->GetAttrPool(), aWhichIds );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            SvxCSS1PropertyInfo aPropInfo;

            bool bStyleParsed = ParseStyleOptions( pCurTable->GetStyle(),
                                                   pCurTable->GetId(),
                                                   pCurTable->GetClass(),
                                                   aItemSet, aPropInfo,
                                                      0, &pCurTable->GetDirection() );
            const SfxPoolItem *pItem = 0;
            if( bStyleParsed )
            {
                if( SfxItemState::SET == aItemSet.GetItemState(
                                        RES_BACKGROUND, false, &pItem ) )
                {
                    pCurTable->SetBGBrush( *(const SvxBrushItem *)pItem );
                    aItemSet.ClearItem( RES_BACKGROUND );
                }
                if( SfxItemState::SET == aItemSet.GetItemState(
                                        RES_PARATR_SPLIT, false, &pItem ) )
                {
                    aItemSet.Put(
                        SwFmtLayoutSplit( ((const SvxFmtSplitItem *)pItem)
                                                ->GetValue() ) );
                    aItemSet.ClearItem( RES_PARATR_SPLIT );
                }
            }

            // Den linken/rechten Absatzeinzug ermitteln
            sal_uInt16 nLeftSpace = 0;
            sal_uInt16 nRightSpace = 0;
            short nIndent;
            GetMarginsFromContextWithNumBul( nLeftSpace, nRightSpace, nIndent );

            // die aktuelle Position an die wir irgendwann zurueckkehren
            SwPosition *pSavePos = 0;
            bool bForceFrame = false;
            bool bAppended = false;
            bool bParentLFStripped = false;
            if( bTopTable )
            {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                SvxAdjust eTblAdjust = pTable->GetTableAdjust(false);
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                SvxAdjust eTblAdjust = m_xTable->GetTableAdjust(false);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

                // Wenn die Tabelle links oder rechts ausgerivchtet ist,
                // oder in einen Rahmen soll, dann kommt sie auch in einen
                // solchen.
                bForceFrame = eTblAdjust == SVX_ADJUST_LEFT ||
                              eTblAdjust == SVX_ADJUST_RIGHT ||
                              pCurTable->HasToFly();

                // Entweder kommt die Tabelle in keinen Rahmen und befindet
                // sich in keinem Rahmen (wird also durch Zellen simuliert),
                // oder es gibt bereits Inhalt an der entsprechenden Stelle.
                OSL_ENSURE( !bForceFrame || pCurTable->HasParentSection(),
                        "Tabelle im Rahmen hat keine Umgebung!" );

                bool bAppend = false;
                if( bForceFrame )
                {
                    // Wenn die Tabelle in einen Rahmen kommt, muss
                    // nur ein neuer Absatz aufgemacht werden, wenn
                    // der Absatz Rahmen ohne Umlauf enthaelt.
                    bAppend = HasCurrentParaFlys(true);
                }
                else
                {
                    // Sonst muss ein neuer Absatz aufgemacht werden,
                    // wenn der Absatz nicht leer ist, oder Rahmen
                    // oder text::Bookmarks enthaelt.
                    bAppend =
                        pPam->GetPoint()->nContent.GetIndex() ||
                        HasCurrentParaFlys() ||
                        HasCurrentParaBookmarks();
                }
                if( bAppend )
                {
                    if( !pPam->GetPoint()->nContent.GetIndex() )
                    {
                        //Set default to CJK and CTL
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        pDoc->SetTxtFmtColl( *pPam,
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        m_xDoc->SetTxtFmtColl( *pPam,
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            pCSS1Parser->GetTxtCollFromPool(RES_POOLCOLL_STANDARD) );
                        SvxFontHeightItem aFontHeight( 40, 100, RES_CHRATR_FONTSIZE );

                        _HTMLAttr* pTmp =
                            new _HTMLAttr( *pPam->GetPoint(), aFontHeight );
                        aSetAttrTab.push_back( pTmp );

                        SvxFontHeightItem aFontHeightCJK( 40, 100, RES_CHRATR_CJK_FONTSIZE );
                        pTmp =
                            new _HTMLAttr( *pPam->GetPoint(), aFontHeightCJK );
                        aSetAttrTab.push_back( pTmp );

                        SvxFontHeightItem aFontHeightCTL( 40, 100, RES_CHRATR_CTL_FONTSIZE );
                        pTmp =
                            new _HTMLAttr( *pPam->GetPoint(), aFontHeightCTL );
                        aSetAttrTab.push_back( pTmp );

                        pTmp = new _HTMLAttr( *pPam->GetPoint(),
                                            SvxULSpaceItem( 0, 0, RES_UL_SPACE ) );
                        aSetAttrTab.push_front( pTmp ); // ja, 0, weil schon
                                                        // vom Tabellenende vorher
                                                        // was gesetzt sein kann.
                    }
                    AppendTxtNode( AM_NOSPACE );
                    bAppended = true;
                }
                else if( !aParaAttrs.empty() )
                {
                    if( !bForceFrame )
                    {
                        // Der Absatz wird gleich hinter die Tabelle
                        // verschoben. Deshalb entfernen wir alle harten
                        // Attribute des Absatzes

                        for( sal_uInt16 i=0; i<aParaAttrs.size(); i++ )
                            aParaAttrs[i]->Invalidate();
                    }

                    aParaAttrs.clear();
                }

                pSavePos = new SwPosition( *pPam->GetPoint() );
            }
            else if( pCurTable->HasParentSection() )
            {
                bParentLFStripped = StripTrailingLF() > 0;

                // Absaetze bzw. ueberschriften beeenden
                nOpenParaToken = 0;
                nFontStHeadStart = nFontStMin;

                // die harten Attribute an diesem Absatz werden nie mehr ungueltig
                if( !aParaAttrs.empty() )
                    aParaAttrs.clear();
            }

            // einen Tabellen Kontext anlegen
            _HTMLTableContext *pTCntxt =
                        new _HTMLTableContext( pSavePos, nContextStMin,
                                               nContextStAttrMin );

            // alle noch offenen Attribute beenden und hinter der Tabelle
            // neu aufspannen
            _HTMLAttrs *pPostIts = 0;
            if( !bForceFrame && (bTopTable || pCurTable->HasParentSection()) )
            {
                SplitAttrTab( pTCntxt->aAttrTab, bTopTable );
                // Wenn wir einen schon vorhandenen Absatz verwenden, duerfen
                // in den keine PostIts eingefuegt werden, weil der Absatz
                // ja hinter die Tabelle wandert. Sie werden deshalb in den
                // ersten Absatz der Tabelle verschoben.
                // Bei Tabellen in Tabellen duerfen ebenfalls keine PostIts
                // in einen noch leeren Absatz eingefuegt werden, weil
                // der sonat nicht geloescht wird.
                if( (bTopTable && !bAppended) ||
                    (!bTopTable && !bParentLFStripped &&
                     !pPam->GetPoint()->nContent.GetIndex()) )
                    pPostIts = new _HTMLAttrs;
                SetAttr( bTopTable, bTopTable, pPostIts );
            }
            else
            {
                SaveAttrTab( pTCntxt->aAttrTab );
                if( bTopTable && !bAppended )
                {
                    pPostIts = new _HTMLAttrs;
                    SetAttr( true, true, pPostIts );
                }
            }
            bNoParSpace = false;

            // Aktuelle Numerierung retten und auschalten.
            pTCntxt->SetNumInfo( GetNumInfo() );
            GetNumInfo().Clear();
            pTCntxt->SavePREListingXMP( *this );

            if( bTopTable )
            {
                if( bForceFrame )
                {
                    // Die Tabelle soll in einen Rahmen geschaufelt werden.

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    SfxItemSet aFrmSet( pDoc->GetAttrPool(),
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    SfxItemSet aFrmSet( m_xDoc->GetAttrPool(),
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                        RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
                    if( !pCurTable->IsNewDoc() )
                        Reader::ResetFrmFmtAttrs( aFrmSet );

                    SwSurround eSurround = SURROUND_NONE;
                    sal_Int16 eHori;

                    switch( pCurTable->GetTableAdjust(true) )
                    {
                    case SVX_ADJUST_RIGHT:
                        eHori = text::HoriOrientation::RIGHT;
                        eSurround = SURROUND_LEFT;
                        break;
                    case SVX_ADJUST_CENTER:
                        eHori = text::HoriOrientation::CENTER;
                        break;
                    case SVX_ADJUST_LEFT:
                        eSurround = SURROUND_RIGHT;
                        //fall-through
                    default:
                        eHori = text::HoriOrientation::LEFT;
                        break;
                    }
                    SetAnchorAndAdjustment( text::VertOrientation::NONE, eHori, aFrmSet,
                                            true );
                    aFrmSet.Put( SwFmtSurround(eSurround) );

                    SwFmtFrmSize aFrmSize( ATT_VAR_SIZE, 20*MM50, MINLAY );
                    aFrmSize.SetWidthPercent( 100 );
                    aFrmSet.Put( aFrmSize );

                    sal_uInt16 nSpace = pCurTable->GetHSpace();
                    if( nSpace )
                        aFrmSet.Put( SvxLRSpaceItem(nSpace,nSpace, 0, 0, RES_LR_SPACE) );
                    nSpace = pCurTable->GetVSpace();
                    if( nSpace )
                        aFrmSet.Put( SvxULSpaceItem(nSpace,nSpace, RES_UL_SPACE) );

                    RndStdIds eAnchorId = ((const SwFmtAnchor&)aFrmSet.
                                                Get( RES_ANCHOR )).
                                                GetAnchorId();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    SwFrmFmt *pFrmFmt =  pDoc->MakeFlySection(
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    SwFrmFmt *pFrmFmt =  m_xDoc->MakeFlySection(
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                eAnchorId, pPam->GetPoint(), &aFrmSet );

                    pTCntxt->SetFrmFmt( pFrmFmt );
                    const SwFmtCntnt& rFlyCntnt = pFrmFmt->GetCntnt();
                    pPam->GetPoint()->nNode = *rFlyCntnt.GetCntntIdx();
                    SwCntntNode *pCNd =
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        pDoc->GetNodes().GoNext( &(pPam->GetPoint()->nNode) );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        m_xDoc->GetNodes().GoNext( &(pPam->GetPoint()->nNode) );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    pPam->GetPoint()->nContent.Assign( pCNd, 0 );

                }

                // eine SwTable mit einer Box anlegen und den PaM in den
                // Inhalt der Box-Section bewegen (der Ausrichtungs-Parameter
                // ist erstmal nur ein Dummy und wird spaeter noch richtig
                // gesetzt)
                OSL_ENSURE( !pPam->GetPoint()->nContent.GetIndex(),
                        "Der Absatz hinter der Tabelle ist nicht leer!" );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                const SwTable* pSwTable = pDoc->InsertTable(
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                const SwTable* pSwTable = m_xDoc->InsertTable(
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        SwInsertTableOptions( tabopts::HEADLINE_NO_BORDER, 1 ),
                        *pPam->GetPoint(), 1, 1, text::HoriOrientation::LEFT );

                if( bForceFrame )
                {
                    SwNodeIndex aDstIdx( pPam->GetPoint()->nNode );
                    pPam->Move( fnMoveBackward );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    pDoc->GetNodes().Delete( aDstIdx );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    m_xDoc->GetNodes().Delete( aDstIdx );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                }
                else
                {
                    if( bStyleParsed )
                    {
                        pCSS1Parser->SetFmtBreak( aItemSet, aPropInfo );
                        pSwTable->GetFrmFmt()->SetFmtAttr( aItemSet );
                    }
                    pPam->Move( fnMoveBackward );
                }

                SwNode const*const pNd = & pPam->GetPoint()->nNode.GetNode();
#ifdef NO_LIBO_NULL_TEXTNODE_FIX
                if( !bAppended && !bForceFrame )
#else	// NO_LIBO_NULL_TEXTNODE_FIX
                SwTxtNode *const pOldTxtNd = (!bAppended && !bForceFrame) ?
                    pSavePos->nNode.GetNode().GetTxtNode() : nullptr;

                if (pOldTxtNd)
#endif	// NO_LIBO_NULL_TEXTNODE_FIX
                {
#ifdef NO_LIBO_NULL_TEXTNODE_FIX
                    SwTxtNode *const pOldTxtNd =
                        pSavePos->nNode.GetNode().GetTxtNode();
                    OSL_ENSURE( pOldTxtNd, "Wieso stehen wir in keinem Txt-Node?" );
#endif	// NO_LIBO_NULL_TEXTNODE_FIX
                    SwFrmFmt *pFrmFmt = pSwTable->GetFrmFmt();

                    const SfxPoolItem* pItem2;
                    if( SfxItemState::SET == pOldTxtNd->GetSwAttrSet()
                            .GetItemState( RES_PAGEDESC, false, &pItem2 ) &&
                        ((SwFmtPageDesc *)pItem2)->GetPageDesc() )
                    {
                        pFrmFmt->SetFmtAttr( *pItem2 );
                        pOldTxtNd->ResetAttr( RES_PAGEDESC );
                    }
                    if( SfxItemState::SET == pOldTxtNd->GetSwAttrSet()
                            .GetItemState( RES_BREAK, true, &pItem2 ) )
                    {
                        switch( ((SvxFmtBreakItem *)pItem2)->GetBreak() )
                        {
                        case SVX_BREAK_PAGE_BEFORE:
                        case SVX_BREAK_PAGE_AFTER:
                        case SVX_BREAK_PAGE_BOTH:
                            pFrmFmt->SetFmtAttr( *pItem2 );
                            pOldTxtNd->ResetAttr( RES_BREAK );
                        default:
                            ;
                        }
                    }
                }

                if( !bAppended && pPostIts )
                {
                    // noch vorhandene PostIts in den ersten Absatz
                    // der Tabelle setzen
                    InsertAttrs( *pPostIts );
                    delete pPostIts;
                    pPostIts = 0;
                }

                pTCntxt->SetTableNode( (SwTableNode *)pNd->FindTableNode() );

                pCurTable->SetTable( pTCntxt->GetTableNode(), pTCntxt,
                                     nLeftSpace, nRightSpace,
                                     pSwTable, bForceFrame );

                OSL_ENSURE( !pPostIts, "ubenutzte PostIts" );
            }
            else
            {
                // noch offene Bereiche muessen noch entfernt werden
                if( EndSections( bParentLFStripped ) )
                    bParentLFStripped = false;

                if( pCurTable->HasParentSection() )
                {
                    // dannach entfernen wir ein ggf. zu viel vorhandenen
                    // leeren Absatz, aber nur, wenn er schon vor dem
                    // entfernen von LFs leer war
                    if( !bParentLFStripped )
                        StripTrailingPara();

                    if( pPostIts )
                    {
                        // noch vorhandene PostIts an das Ende des jetzt
                        // aktuellen Absatzes schieben
                        InsertAttrs( *pPostIts );
                        delete pPostIts;
                        pPostIts = 0;
                    }
                }

                SwNode const*const pNd = & pPam->GetPoint()->nNode.GetNode();
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                const SwStartNode *pStNd = (pTable->bFirstCell ? pNd->FindTableNode()
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                const SwStartNode *pStNd = (m_xTable->bFirstCell ? pNd->FindTableNode()
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                                            : pNd->FindTableBoxStartNode() );

                pCurTable->SetTable( pStNd, pTCntxt, nLeftSpace, nRightSpace );
            }

            // Den Kontext-Stack einfrieren, denn es koennen auch mal
            // irgendwo ausserhalb von Zellen Attribute gesetzt werden.
            // Darf nicht frueher passieren, weil eventuell noch im
            // Stack gesucht wird!!!
            nContextStMin = aContexts.size();
            nContextStAttrMin = nContextStMin;
        }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pSaveStruct = new _CellSaveStruct( *this, pCurTable, bHead,
                                            bReadOptions );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct.reset(new _CellSaveStruct(*this, pCurTable, bHead, bReadOptions));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    // Token nach <TABLE>

    bool bDone = false;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        OSL_ENSURE( pPendStack || !bCallNextToken || pSaveStruct->IsInSection(),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        OSL_ENSURE( pPendStack || !bCallNextToken || xSaveStruct->IsInSection(),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                "Wo ist die Section gebieben?" );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        if( !pPendStack && bCallNextToken && pSaveStruct->IsInSection() )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if( !pPendStack && bCallNextToken && xSaveStruct->IsInSection() )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        {
            // NextToken direkt aufrufen (z.B. um den Inhalt von
            // Floating-Frames oder Applets zu ignorieren)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLEHEADER_ON:
        case HTML_TABLEDATA_ON:
        case HTML_TABLEROW_ON:
        case HTML_TABLEROW_OFF:
        case HTML_THEAD_ON:
        case HTML_THEAD_OFF:
        case HTML_TFOOT_ON:
        case HTML_TFOOT_OFF:
        case HTML_TBODY_ON:
        case HTML_TBODY_OFF:
        case HTML_TABLE_OFF:
            SkipToken(-1);
            //fall-through
        case HTML_TABLEHEADER_OFF:
        case HTML_TABLEDATA_OFF:
            bDone = true;
            break;
        case HTML_TABLE_ON:
            {
                bool bHasToFly = false;
                SvxAdjust eTabAdjust = SVX_ADJUST_END;
                if( !pPendStack )
                {
                    // nur wenn eine neue Tabelle aufgemacht wird, aber
                    // nicht wenn nach einem Pending in der Tabelle
                    // weitergelesen wird!
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                    pSaveStruct->pTable = pTable;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    xSaveStruct->m_xTable = m_xTable;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

                    // HACK: Eine Section fuer eine Tabelle anlegen, die
                    // in einen Rahmen kommt.
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                    if( !pSaveStruct->IsInSection() )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    if( !xSaveStruct->IsInSection() )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    {
                        // Diese Schleife muss vorwartes sein, weil die
                        // erste Option immer gewinnt.
                        bool bNeedsSection = false;
                        const HTMLOptions& rHTMLOptions = GetOptions();
                        for (size_t i = 0; i < rHTMLOptions.size(); ++i)
                        {
                            const HTMLOption& rOption = rHTMLOptions[i];
                            if( HTML_O_ALIGN==rOption.GetToken() )
                            {
                                SvxAdjust eAdjust =
                                    (SvxAdjust)rOption.GetEnum(
                                            aHTMLPAlignTable, SVX_ADJUST_END );
                                bNeedsSection = SVX_ADJUST_LEFT == eAdjust ||
                                                SVX_ADJUST_RIGHT == eAdjust;
                                break;
                            }
                        }
                        if( bNeedsSection )
                        {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                            pSaveStruct->AddContents(
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            xSaveStruct->AddContents(
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                InsertTableContents(bHead  ) );
                        }
                    }
                    else
                    {
                        // Wenn im aktuellen Absatz Flys verankert sind,
                        // muss die neue Tabelle in einen Rahmen.
                        bHasToFly = HasCurrentParaFlys(false,true);
                    }

                    // in der Zelle kann sich ein Bereich befinden!
                    eTabAdjust = aAttrTab.pAdjust
                        ? ((const SvxAdjustItem&)aAttrTab.pAdjust->GetItem()).
                                                 GetAdjust()
                        : SVX_ADJUST_END;
                }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                HTMLTable *pSubTable = BuildTable( eTabAdjust,
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                std::shared_ptr<HTMLTable> xSubTable = BuildTable(eTabAdjust,
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                                   bHead,
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                                                   pSaveStruct->IsInSection(),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                                                  xSaveStruct->IsInSection(),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                                   bHasToFly );
                if( SVPAR_PENDING != GetStatus() )
                {
                    // nur wenn die Tabelle wirklich zu Ende ist!
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                    if( pSubTable )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    if (xSubTable)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                        OSL_ENSURE( pSubTable->GetTableAdjust(false)!= SVX_ADJUST_LEFT &&
                                pSubTable->GetTableAdjust(false)!= SVX_ADJUST_RIGHT,
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        OSL_ENSURE( xSubTable->GetTableAdjust(false)!= SVX_ADJUST_LEFT &&
                                xSubTable->GetTableAdjust(false)!= SVX_ADJUST_RIGHT,
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                "links oder rechts ausgerichtete Tabellen gehoehren in Rahmen" );

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                        HTMLTableCnts *pParentContents =
                            pSubTable->GetParentContents();
                        if( pParentContents )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        auto& rParentContents = xSubTable->GetParentContents();
                        if (rParentContents)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                            OSL_ENSURE( !pSaveStruct->IsInSection(),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            OSL_ENSURE( !xSaveStruct->IsInSection(),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                    "Wo ist die Section geblieben" );

                            // Wenn jetzt keine Tabelle kommt haben wir eine
                            // Section
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                            pSaveStruct->AddContents( pParentContents );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            xSaveStruct->AddContents(rParentContents.release());
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        }

                        const SwStartNode *pCapStNd =
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                                pSubTable->GetCaptionStartNode();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                xSubTable->GetCaptionStartNode();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                        if( pSubTable->GetContext() )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        if (xSubTable->GetContext())
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                            OSL_ENSURE( !pSubTable->GetContext()->GetFrmFmt(),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            OSL_ENSURE( !xSubTable->GetContext()->GetFrmFmt(),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                    "Tabelle steht im Rahmen" );

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                            if( pCapStNd && pSubTable->IsTopCaption() )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            if( pCapStNd && xSubTable->IsTopCaption() )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                                pSaveStruct->AddContents(
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                xSaveStruct->AddContents(
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                    new HTMLTableCnts(pCapStNd) );
                            }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                            pSaveStruct->AddContents(
                                new HTMLTableCnts(pSubTable) );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            xSaveStruct->AddContents(
                                new HTMLTableCnts(xSubTable) );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                            if( pCapStNd && !pSubTable->IsTopCaption() )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            if( pCapStNd && !xSubTable->IsTopCaption() )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                                pSaveStruct->AddContents(
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                xSaveStruct->AddContents(
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                    new HTMLTableCnts(pCapStNd) );
                            }

                            // Jetzt haben wir keine Section mehr
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                            pSaveStruct->ClearIsInSection();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            xSaveStruct->ClearIsInSection();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        }
                        else if( pCapStNd )
                        {
                            // Da wir diese Sction nicht mehr loeschen
                            // koennen (sie koeente zur erster Box
                            // gehoeren), fuegen wir sie ein.
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                            pSaveStruct->AddContents(
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            xSaveStruct->AddContents(
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                new HTMLTableCnts(pCapStNd) );

                            // Jetzt haben wir keine Section mehr
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                            pSaveStruct->ClearIsInSection();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                            xSaveStruct->ClearIsInSection();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        }
                    }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                    pTable = pSaveStruct->pTable;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    m_xTable = xSaveStruct->m_xTable;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                }
            }
            break;

        case HTML_NOBR_ON:
            // HACK fuer MS: Steht das <NOBR> zu beginn der Zelle?
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            pSaveStruct->StartNoBreak( *pPam->GetPoint() );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            xSaveStruct->StartNoBreak( *pPam->GetPoint() );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            break;

        case HTML_NOBR_OFF:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                pSaveStruct->EndNoBreak( *pPam->GetPoint() );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                xSaveStruct->EndNoBreak( *pPam->GetPoint() );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            break;

        case HTML_COMMENT:
            // Mit Kommentar-Feldern werden Spaces nicht mehr geloescht
            // ausserdem wollen wir fuer einen Kommentar keine neue Zelle
            // anlegen !!!
            NextToken( nToken );
            break;

        case HTML_MARQUEE_ON:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            if( !pSaveStruct->IsInSection() )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            if( !xSaveStruct->IsInSection() )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            {
                // eine neue Section anlegen, der PaM steht dann darin
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                pSaveStruct->AddContents(
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                xSaveStruct->AddContents(
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    InsertTableContents( bHead ) );
            }
            bCallNextToken = true;
            NewMarquee( pCurTable );
            break;

        case HTML_TEXTTOKEN:
            // keine Section fuer einen leeren String anlegen
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            if( !pSaveStruct->IsInSection() && 1==aToken.getLength() &&
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            if( !xSaveStruct->IsInSection() && 1==aToken.getLength() &&
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                ' '==aToken[0] )
                break;
        default:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            if( !pSaveStruct->IsInSection() )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            if( !xSaveStruct->IsInSection() )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            {
                // eine neue Section anlegen, der PaM steht dann darin
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                pSaveStruct->AddContents(
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                xSaveStruct->AddContents(
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    InsertTableContents( bHead ) );
            }

            if( IsParserWorking() || bPending )
                NextToken( nToken );
            break;
        }

        OSL_ENSURE( !bPending || !pPendStack,
                "SwHTMLParser::BuildTableCell: Es gibt wieder einen Pend-Stack" );
        bPending = false;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        pPendStack = new SwPendingStack( bHead ? HTML_TABLEHEADER_ON
                                               : HTML_TABLEDATA_ON, pPendStack );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pPendStack->pData = pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pPendStack->pData = xSaveStruct.release();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        return;
    }

    // Falls der Inhalt der Zelle leer war, muessen wir noch einen
    // leeren Inhalt anlegen. Ausserdem legen wir einen leeren Inhalt
    // an, wenn die Zelle mit einer Tabelle aufgehoert hat und keine
    // COL-Tags hatte (sonst wurde sie wahrscheinlich von uns exportiert,
    // und dann wollen wir natuerlich keinen zusaetzlichen Absatz haben).
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( !pSaveStruct->GetFirstContents() ||
        (!pSaveStruct->IsInSection() && !pCurTable->HasColTags()) )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if( !xSaveStruct->HasFirstContents() ||
        (!xSaveStruct->IsInSection() && !pCurTable->HasColTags()) )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        OSL_ENSURE( pSaveStruct->GetFirstContents() ||
                !pSaveStruct->IsInSection(),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        OSL_ENSURE( xSaveStruct->HasFirstContents() ||
                !xSaveStruct->IsInSection(),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                "Section oder nicht, das ist hier die Frage" );
        const SwStartNode *pStNd =
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            InsertTableSection( static_cast< sal_uInt16 >(pSaveStruct->IsHeaderCell()
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            InsertTableSection( static_cast< sal_uInt16 >(xSaveStruct->IsHeaderCell()
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                        ? RES_POOLCOLL_TABLE_HDLN
                                        : RES_POOLCOLL_TABLE ));
#ifdef NO_LIBO_NULL_TABLE_NODE_FIX
        const SwEndNode *pEndNd = pStNd->EndOfSectionNode();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SwCntntNode *pCNd = pDoc->GetNodes()[pEndNd->GetIndex()-1] ->GetCntntNode();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SwCntntNode *pCNd = m_xDoc->GetNodes()[pEndNd->GetIndex()-1] ->GetCntntNode();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        //Added defaults to CJK and CTL
        SvxFontHeightItem aFontHeight( 40, 100, RES_CHRATR_FONTSIZE );
        pCNd->SetAttr( aFontHeight );
        SvxFontHeightItem aFontHeightCJK( 40, 100, RES_CHRATR_CJK_FONTSIZE );
        pCNd->SetAttr( aFontHeightCJK );
        SvxFontHeightItem aFontHeightCTL( 40, 100, RES_CHRATR_CTL_FONTSIZE );
        pCNd->SetAttr( aFontHeightCTL );
#else	// NO_LIBO_NULL_TABLE_NODE_FIX

        if (!pStNd)
            eState = SVPAR_ERROR;
        else
        {
            const SwEndNode *pEndNd = pStNd->EndOfSectionNode();
            SwCntntNode *pCNd = m_xDoc->GetNodes()[pEndNd->GetIndex()-1] ->GetCntntNode();
            //Added defaults to CJK and CTL
            SvxFontHeightItem aFontHeight( 40, 100, RES_CHRATR_FONTSIZE );
            pCNd->SetAttr( aFontHeight );
            SvxFontHeightItem aFontHeightCJK( 40, 100, RES_CHRATR_CJK_FONTSIZE );
            pCNd->SetAttr( aFontHeightCJK );
            SvxFontHeightItem aFontHeightCTL( 40, 100, RES_CHRATR_CTL_FONTSIZE );
            pCNd->SetAttr( aFontHeightCTL );
        }
#endif	// NO_LIBO_NULL_TABLE_NODE_FIX

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pSaveStruct->AddContents( new HTMLTableCnts(pStNd) );
        pSaveStruct->ClearIsInSection();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct->AddContents( new HTMLTableCnts(pStNd) );
        xSaveStruct->ClearIsInSection();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( pSaveStruct->IsInSection() )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if( xSaveStruct->IsInSection() )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pSaveStruct->CheckNoBreak( *pPam->GetPoint(), pDoc );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pSaveStruct->CheckNoBreak( *pPam->GetPoint(), m_xDoc.get() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        xSaveStruct->CheckNoBreak( *pPam->GetPoint(), pDoc );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        xSaveStruct->CheckNoBreak( *pPam->GetPoint(), m_xDoc.get() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        // Alle noch offenen Kontexte beenden. Wir nehmen hier
        // AttrMin, weil nContxtStMin evtl. veraendert wurde.
        // Da es durch EndContext wieder restauriert wird, geht das.
        while( (sal_uInt16)aContexts.size() > nContextStAttrMin+1 )
        {
#ifdef NO_LIBO_HTML_PARSER_LEAK_FIX
            _HTMLAttrContext *pCntxt = PopContext();
            EndContext( pCntxt );
            delete pCntxt;
#else	// NO_LIBO_HTML_PARSER_LEAK_FIX
            std::unique_ptr<_HTMLAttrContext> xCntxt(PopContext());
            EndContext(xCntxt.get());
#endif	// NO_LIBO_HTML_PARSER_LEAK_FIX
        }

        // LFs am Absatz-Ende entfernen
        if( StripTrailingLF()==0 && !pPam->GetPoint()->nContent.GetIndex() )
            StripTrailingPara();

        // falls fuer die Zelle eine Ausrichtung gesetzt wurde, muessen
        // wir die beenden
#ifdef NO_LIBO_HTML_PARSER_LEAK_FIX
        _HTMLAttrContext *pCntxt = PopContext();
        EndContext( pCntxt );
        delete pCntxt;
#else	// NO_LIBO_HTML_PARSER_LEAK_FIX
        std::unique_ptr<_HTMLAttrContext> xCntxt(PopContext());
        EndContext(xCntxt.get());
#endif	// NO_LIBO_HTML_PARSER_LEAK_FIX
    }
    else
    {
        // Alle noch offenen Kontexte beenden
        while( aContexts.size() > nContextStAttrMin )
        {
#ifdef NO_LIBO_HTML_PARSER_LEAK_FIX
            _HTMLAttrContext *pCntxt = PopContext();
            ClearContext( pCntxt );
            delete pCntxt;
#else	// NO_LIBO_HTML_PARSER_LEAK_FIX
            std::unique_ptr<_HTMLAttrContext> xCntxt(PopContext());
            ClearContext(xCntxt.get());
#endif	// NO_LIBO_HTML_PARSER_LEAK_FIX
        }
    }

    // auch eine Numerierung muss beendet werden
    GetNumInfo().Clear();

    SetAttr( false );

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pSaveStruct->InsertCell( *this, pCurTable );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    xSaveStruct->InsertCell( *this, pCurTable );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    // wir stehen jetzt (wahrschenlich) vor <TH>, <TD>, <TR> oder </TABLE>
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    delete pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    xSaveStruct.reset();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
}

class _RowSaveStruct : public SwPendingStackData
{
public:
    SvxAdjust eAdjust;
    sal_Int16 eVertOri;
    bool bHasCells;

    _RowSaveStruct() :
        eAdjust( SVX_ADJUST_END ), eVertOri( text::VertOrientation::TOP ), bHasCells( false )
    {}
};

void SwHTMLParser::BuildTableRow( HTMLTable *pCurTable, bool bReadOptions,
                                  SvxAdjust eGrpAdjust,
                                  sal_Int16 eGrpVertOri )
{
    // <TR> wurde bereist gelesen

    if( !IsParserWorking() && !pPendStack )
        return;

    int nToken = 0;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    _RowSaveStruct* pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::unique_ptr<_RowSaveStruct> xSaveStruct;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    bool bPending = false;
    if( pPendStack )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pSaveStruct = (_RowSaveStruct*)pPendStack->pData;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct.reset(static_cast<_RowSaveStruct*>(pPendStack->pData));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && pPendStack != 0;

        SaveState( nToken );
    }
    else
    {
        SvxAdjust eAdjust = eGrpAdjust;
        sal_Int16 eVertOri = eGrpVertOri;
        Color aBGColor;
        OUString aBGImage, aStyle, aId, aClass;
        bool bBGColor = false;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pSaveStruct = new _RowSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct.reset(new _RowSaveStruct);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        if( bReadOptions )
        {
            const HTMLOptions& rHTMLOptions = GetOptions();
            for (size_t i = rHTMLOptions.size(); i; )
            {
                const HTMLOption& rOption = rHTMLOptions[--i];
                switch( rOption.GetToken() )
                {
                case HTML_O_ID:
                    aId = rOption.GetString();
                    break;
                case HTML_O_ALIGN:
                    eAdjust = (SvxAdjust)rOption.GetEnum(
                                    aHTMLPAlignTable, static_cast< sal_uInt16 >(eAdjust) );
                    break;
                case HTML_O_VALIGN:
                    eVertOri = rOption.GetEnum(
                                    aHTMLTblVAlignTable, eVertOri );
                    break;
                case HTML_O_BGCOLOR:
                    // Leere BGCOLOR bei <TABLE>, <TR> und <TD>/<TH> wie Netsc.
                    // ignorieren, bei allen anderen Tags *wirklich* nicht.
                    if( !rOption.GetString().isEmpty() )
                    {
                        rOption.GetColor( aBGColor );
                        bBGColor = true;
                    }
                    break;
                case HTML_O_BACKGROUND:
                    aBGImage = rOption.GetString();
                    break;
                case HTML_O_STYLE:
                    aStyle = rOption.GetString();
                    break;
                case HTML_O_CLASS:
                    aClass= rOption.GetString();
                    break;
                }
            }
        }

        if( !aId.isEmpty() )
            InsertBookmark( aId );

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        SvxBrushItem *pBrushItem =
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        std::unique_ptr<SvxBrushItem> xBrushItem(
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            CreateBrushItem( bBGColor ? &aBGColor : 0, aBGImage, aStyle,
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                             aId, aClass );
        pCurTable->OpenRow( eAdjust, eVertOri, pBrushItem );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                             aId, aClass ));
        pCurTable->OpenRow(eAdjust, eVertOri, xBrushItem);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    // naechstes Token

    bool bDone = false;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( pPendStack || !bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Wo ist die Section gebieben?" );
        if( !pPendStack && bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            // NextToken direkt aufrufen (z.B. um den Inhalt von
            // Floating-Frames oder Applets zu ignorieren)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pCurTable->GetContext()  )
            {
                SkipToken( -1 );
                bDone = true;
            }

            break;
        case HTML_TABLEROW_ON:
        case HTML_THEAD_ON:
        case HTML_THEAD_OFF:
        case HTML_TBODY_ON:
        case HTML_TBODY_OFF:
        case HTML_TFOOT_ON:
        case HTML_TFOOT_OFF:
        case HTML_TABLE_OFF:
            SkipToken( -1 );
            //fall-through
        case HTML_TABLEROW_OFF:
            bDone = true;
            break;
        case HTML_TABLEHEADER_ON:
        case HTML_TABLEDATA_ON:
            BuildTableCell( pCurTable, true, HTML_TABLEHEADER_ON==nToken );
            if( SVPAR_PENDING != GetStatus() )
            {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                pSaveStruct->bHasCells = true;
                bDone = pTable->IsOverflowing();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                xSaveStruct->bHasCells = true;
                bDone = m_xTable->IsOverflowing();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            }
            break;
        case HTML_CAPTION_ON:
            BuildTableCaption( pCurTable );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            bDone = pTable->IsOverflowing();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            bDone = m_xTable->IsOverflowing();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            break;
        case HTML_CAPTION_OFF:
        case HTML_TABLEHEADER_OFF:
        case HTML_TABLEDATA_OFF:
        case HTML_COLGROUP_ON:
        case HTML_COLGROUP_OFF:
        case HTML_COL_ON:
        case HTML_COL_OFF:
            // wo keine Zelle anfing kann auch keine aufhoehren, oder?
            // und die ganzen anderen Tokens haben hier auch nicht zu
            // suchen und machen nur die Tabelle kaputt
            break;
        case HTML_MULTICOL_ON:
            // spaltige Rahmen koennen wir hier leider nicht einguegen
            break;
        case HTML_FORM_ON:
            NewForm( false );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_FORM_OFF:
            EndForm( false );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_COMMENT:
            NextToken( nToken );
            break;
        case HTML_MAP_ON:
            // eine Image-Map fuegt nichts ein, deshalb koennen wir sie
            // problemlos auch ohne Zelle parsen
            NextToken( nToken );
            break;
        case HTML_TEXTTOKEN:
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.getLength() && ' '==aToken[0] )
                break;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
            break;
        }

        OSL_ENSURE( !bPending || !pPendStack,
                "SwHTMLParser::BuildTableRow: Es gibt wieder einen Pend-Stack" );
        bPending = false;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        pPendStack = new SwPendingStack( HTML_TABLEROW_ON, pPendStack );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pPendStack->pData = pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pPendStack->pData = xSaveStruct.release();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }
    else
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pCurTable->CloseRow( !pSaveStruct->bHasCells );
        delete pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pCurTable->CloseRow(!xSaveStruct->bHasCells);
        xSaveStruct.reset();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }

    // wir stehen jetzt (wahrscheinlich) vor <TR> oder </TABLE>
}

void SwHTMLParser::BuildTableSection( HTMLTable *pCurTable,
                                      bool bReadOptions,
                                      bool bHead )
{
    // <THEAD>, <TBODY> bzw. <TFOOT> wurde bereits gelesen
    if( !IsParserWorking() && !pPendStack )
        return;

    int nToken = 0;
    bool bPending = false;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    _RowSaveStruct* pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::unique_ptr<_RowSaveStruct> xSaveStruct;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    if( pPendStack )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pSaveStruct = (_RowSaveStruct*)pPendStack->pData;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct.reset(static_cast<_RowSaveStruct*>(pPendStack->pData));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && pPendStack != 0;

        SaveState( nToken );
    }
    else
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pSaveStruct = new _RowSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct.reset(new _RowSaveStruct);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        if( bReadOptions )
        {
            const HTMLOptions& rHTMLOptions = GetOptions();
            for (size_t i = rHTMLOptions.size(); i; )
            {
                const HTMLOption& rOption = rHTMLOptions[--i];
                switch( rOption.GetToken() )
                {
                case HTML_O_ID:
                    InsertBookmark( rOption.GetString() );
                    break;
                case HTML_O_ALIGN:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                    pSaveStruct->eAdjust =
                        (SvxAdjust)rOption.GetEnum( aHTMLPAlignTable,
                                                     static_cast< sal_uInt16 >(pSaveStruct->eAdjust) );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    xSaveStruct->eAdjust =
                        (SvxAdjust)rOption.GetEnum( aHTMLPAlignTable, xSaveStruct->eAdjust );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    break;
                case HTML_O_VALIGN:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                    pSaveStruct->eVertOri =
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    xSaveStruct->eVertOri =
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        rOption.GetEnum( aHTMLTblVAlignTable,
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                                          pSaveStruct->eVertOri );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                          xSaveStruct->eVertOri );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    break;
                }
            }
        }

        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    // naechstes Token

    bool bDone = false;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( pPendStack || !bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Wo ist die Section gebieben?" );
        if( !pPendStack && bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            // NextToken direkt aufrufen (z.B. um den Inhalt von
            // Floating-Frames oder Applets zu ignorieren)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pCurTable->GetContext()  )
            {
                SkipToken( -1 );
                bDone = true;
            }

            break;
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
        case HTML_TABLE_OFF:
            SkipToken( -1 );
            //fall-through
        case HTML_THEAD_OFF:
        case HTML_TBODY_OFF:
        case HTML_TFOOT_OFF:
            bDone = true;
            break;
        case HTML_CAPTION_ON:
            BuildTableCaption( pCurTable );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            bDone = pTable->IsOverflowing();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            bDone = m_xTable->IsOverflowing();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            break;
        case HTML_CAPTION_OFF:
            break;
        case HTML_TABLEHEADER_ON:
        case HTML_TABLEDATA_ON:
            SkipToken( -1 );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            BuildTableRow( pCurTable, false, pSaveStruct->eAdjust,
                           pSaveStruct->eVertOri );
            bDone = pTable->IsOverflowing();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            BuildTableRow( pCurTable, false, xSaveStruct->eAdjust,
                           xSaveStruct->eVertOri );
            bDone = m_xTable->IsOverflowing();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            break;
        case HTML_TABLEROW_ON:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            BuildTableRow( pCurTable, true, pSaveStruct->eAdjust,
                           pSaveStruct->eVertOri );
            bDone = pTable->IsOverflowing();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            BuildTableRow( pCurTable, true, xSaveStruct->eAdjust,
                           xSaveStruct->eVertOri );
            bDone = m_xTable->IsOverflowing();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
             break;
            break;
        case HTML_MULTICOL_ON:
            // spaltige Rahmen koennen wir hier leider nicht einguegen
            break;
        case HTML_FORM_ON:
            NewForm( false );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_FORM_OFF:
            EndForm( false );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_TEXTTOKEN:
            // Blank-Strings sind Folge von CR+LF und kein Text
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.getLength() && ' ' == aToken[0] )
                break;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
        }

        OSL_ENSURE( !bPending || !pPendStack,
                "SwHTMLParser::BuildTableSection: Es gibt wieder einen Pend-Stack" );
        bPending = false;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        pPendStack = new SwPendingStack( bHead ? HTML_THEAD_ON
                                               : HTML_TBODY_ON, pPendStack );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pPendStack->pData = pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pPendStack->pData = xSaveStruct.release();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }
    else
    {
        pCurTable->CloseSection( bHead );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        delete pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct.reset();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }

    // now we stand (perhaps) in front of <TBODY>,... or </TABLE>
}

struct _TblColGrpSaveStruct : public SwPendingStackData
{
    sal_uInt16 nColGrpSpan;
    sal_uInt16 nColGrpWidth;
    bool bRelColGrpWidth;
    SvxAdjust eColGrpAdjust;
    sal_Int16 eColGrpVertOri;

    inline _TblColGrpSaveStruct();

    inline void CloseColGroup( HTMLTable *pTable );
};

inline _TblColGrpSaveStruct::_TblColGrpSaveStruct() :
    nColGrpSpan( 1 ), nColGrpWidth( 0 ),
    bRelColGrpWidth( false ), eColGrpAdjust( SVX_ADJUST_END ),
    eColGrpVertOri( text::VertOrientation::TOP )
{}

inline void _TblColGrpSaveStruct::CloseColGroup( HTMLTable *pTable )
{
    pTable->CloseColGroup( nColGrpSpan, nColGrpWidth,
                            bRelColGrpWidth, eColGrpAdjust, eColGrpVertOri );
}

void SwHTMLParser::BuildTableColGroup( HTMLTable *pCurTable,
                                       bool bReadOptions )
{
    // <COLGROUP> wurde bereits gelesen, wenn bReadOptions

    if( !IsParserWorking() && !pPendStack )
        return;

    int nToken = 0;
    bool bPending = false;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    _TblColGrpSaveStruct* pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::unique_ptr<_TblColGrpSaveStruct> xSaveStruct;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    if( pPendStack )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pSaveStruct = (_TblColGrpSaveStruct*)pPendStack->pData;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct.reset(static_cast<_TblColGrpSaveStruct*>(pPendStack->pData));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && pPendStack != 0;

        SaveState( nToken );
    }
    else
    {

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pSaveStruct = new _TblColGrpSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct.reset(new _TblColGrpSaveStruct);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if( bReadOptions )
        {
            const HTMLOptions& rColGrpOptions = GetOptions();
            for (size_t i = rColGrpOptions.size(); i; )
            {
                const HTMLOption& rOption = rColGrpOptions[--i];
                switch( rOption.GetToken() )
                {
                case HTML_O_ID:
                    InsertBookmark( rOption.GetString() );
                    break;
                case HTML_O_SPAN:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                    pSaveStruct->nColGrpSpan = (sal_uInt16)rOption.GetNumber();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    xSaveStruct->nColGrpSpan = (sal_uInt16)rOption.GetNumber();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    break;
                case HTML_O_WIDTH:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                    pSaveStruct->nColGrpWidth = (sal_uInt16)rOption.GetNumber();
                    pSaveStruct->bRelColGrpWidth =
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    xSaveStruct->nColGrpWidth = (sal_uInt16)rOption.GetNumber();
                    xSaveStruct->bRelColGrpWidth =
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        (rOption.GetString().indexOf('*') != -1);
                    break;
                case HTML_O_ALIGN:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                    pSaveStruct->eColGrpAdjust =
                        (SvxAdjust)rOption.GetEnum( aHTMLPAlignTable,
                                                static_cast< sal_uInt16 >(pSaveStruct->eColGrpAdjust) );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    xSaveStruct->eColGrpAdjust =
                        (SvxAdjust)rOption.GetEnum( aHTMLPAlignTable, xSaveStruct->eColGrpAdjust );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    break;
                case HTML_O_VALIGN:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                    pSaveStruct->eColGrpVertOri =
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    xSaveStruct->eColGrpVertOri =
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                        rOption.GetEnum( aHTMLTblVAlignTable,
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                                                pSaveStruct->eColGrpVertOri );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                          xSaveStruct->eColGrpVertOri );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    break;
                }
            }
        }
        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    // naechstes Token

    bool bDone = false;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( pPendStack || !bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Wo ist die Section gebieben?" );
        if( !pPendStack && bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            // NextToken direkt aufrufen (z.B. um den Inhalt von
            // Floating-Frames oder Applets zu ignorieren)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pCurTable->GetContext()  )
            {
                SkipToken( -1 );
                bDone = true;
            }

            break;
        case HTML_COLGROUP_ON:
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
        case HTML_TABLEROW_ON:
        case HTML_TABLE_OFF:
            SkipToken( -1 );
            //fall-through
        case HTML_COLGROUP_OFF:
            bDone = true;
            break;
        case HTML_COL_ON:
            {
                sal_uInt16 nColSpan = 1;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                sal_uInt16 nColWidth = pSaveStruct->nColGrpWidth;
                bool bRelColWidth = pSaveStruct->bRelColGrpWidth;
                SvxAdjust eColAdjust = pSaveStruct->eColGrpAdjust;
                sal_Int16 eColVertOri = pSaveStruct->eColGrpVertOri;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                sal_uInt16 nColWidth = xSaveStruct->nColGrpWidth;
                bool bRelColWidth = xSaveStruct->bRelColGrpWidth;
                SvxAdjust eColAdjust = xSaveStruct->eColGrpAdjust;
                sal_Int16 eColVertOri = xSaveStruct->eColGrpVertOri;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

                const HTMLOptions& rColOptions = GetOptions();
                for (size_t i = rColOptions.size(); i; )
                {
                    const HTMLOption& rOption = rColOptions[--i];
                    switch( rOption.GetToken() )
                    {
                    case HTML_O_ID:
                        InsertBookmark( rOption.GetString() );
                        break;
                    case HTML_O_SPAN:
                        nColSpan = (sal_uInt16)rOption.GetNumber();
                        break;
                    case HTML_O_WIDTH:
                        nColWidth = (sal_uInt16)rOption.GetNumber();
                        bRelColWidth =
                            (rOption.GetString().indexOf('*') != -1);
                        break;
                    case HTML_O_ALIGN:
                        eColAdjust =
                            (SvxAdjust)rOption.GetEnum( aHTMLPAlignTable,
                                                            static_cast< sal_uInt16 >(eColAdjust) );
                        break;
                    case HTML_O_VALIGN:
                        eColVertOri =
                            rOption.GetEnum( aHTMLTblVAlignTable,
                                                        eColVertOri );
                        break;
                    }
                }
                pCurTable->InsertCol( nColSpan, nColWidth, bRelColWidth,
                                      eColAdjust, eColVertOri );

                // die Angaben in <COLGRP> sollen ignoriert werden, wenn
                // <COL>-Elemente existieren
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                pSaveStruct->nColGrpSpan = 0;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                xSaveStruct->nColGrpSpan = 0;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            }
            break;
        case HTML_COL_OFF:
            break;      // Ignorieren
        case HTML_MULTICOL_ON:
            // spaltige Rahmen koennen wir hier leider nicht einguegen
            break;
        case HTML_TEXTTOKEN:
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.getLength() && ' '==aToken[0] )
                break;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
        }

        OSL_ENSURE( !bPending || !pPendStack,
                "SwHTMLParser::BuildTableColGrp: Es gibt wieder einen Pend-Stack" );
        bPending = false;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        pPendStack = new SwPendingStack( HTML_COL_ON, pPendStack );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pPendStack->pData = pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pPendStack->pData = xSaveStruct.release();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }
    else
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pSaveStruct->CloseColGroup( pCurTable );
        delete pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct->CloseColGroup( pCurTable );
        xSaveStruct.reset();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }
}

class _CaptionSaveStruct : public _SectionSaveStruct
{
    SwPosition aSavePos;
    SwHTMLNumRuleInfo aNumRuleInfo; // gueltige Numerierung

public:

    _HTMLAttrTable aAttrTab;        // und die Attribute

    _CaptionSaveStruct( SwHTMLParser& rParser, const SwPosition& rPos ) :
        _SectionSaveStruct( rParser ), aSavePos( rPos )
    {
        rParser.SaveAttrTab( aAttrTab );

        // Die aktuelle Numerierung wurde gerettet und muss nur
        // noch beendet werden.
        aNumRuleInfo.Set( rParser.GetNumInfo() );
        rParser.GetNumInfo().Clear();
    }

    const SwPosition& GetPos() const { return aSavePos; }

    void RestoreAll( SwHTMLParser& rParser )
    {
        // Die alten Stack wiederherstellen
        Restore( rParser );

        // Die alte Attribut-Tabelle wiederherstellen
        rParser.RestoreAttrTab( aAttrTab );

        // Die alte Numerierung wieder aufspannen
        rParser.GetNumInfo().Set( aNumRuleInfo );
    }

    virtual ~_CaptionSaveStruct();
};

_CaptionSaveStruct::~_CaptionSaveStruct()
{}

void SwHTMLParser::BuildTableCaption( HTMLTable *pCurTable )
{
    // <CAPTION> wurde bereits gelesen

    if( !IsParserWorking() && !pPendStack )
        return;

    int nToken = 0;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    _CaptionSaveStruct* pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::unique_ptr<_CaptionSaveStruct> xSaveStruct;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    if( pPendStack )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pSaveStruct = (_CaptionSaveStruct*)pPendStack->pData;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct.reset(static_cast<_CaptionSaveStruct*>(pPendStack->pData));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        OSL_ENSURE( !pPendStack, "Wo kommt hier ein Pending-Stack her?" );

        SaveState( nToken );
    }
    else
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        if( pTable->IsOverflowing() )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if (m_xTable->IsOverflowing())
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        {
            SaveState( 0 );
            return;
        }

        bool bTop = true;
        const HTMLOptions& rHTMLOptions = GetOptions();
        for ( size_t i = rHTMLOptions.size(); i; )
        {
            const HTMLOption& rOption = rHTMLOptions[--i];
            if( HTML_O_ALIGN == rOption.GetToken() )
            {
                if (rOption.GetString().equalsIgnoreAsciiCase(
                        OOO_STRING_SVTOOLS_HTML_VA_bottom))
                {
                    bTop = false;
                }
            }
        }

        // Alte PaM-Position retten.
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pSaveStruct = new _CaptionSaveStruct( *this, *pPam->GetPoint() );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct.reset(new _CaptionSaveStruct(*this, *pPam->GetPoint()));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        // Eine Text-Section im Icons-Bereich als Container fuer die
        // Ueberschrift anlegen und PaM dort reinstellen.
        const SwStartNode *pStNd;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        if( pTable == pCurTable )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if (m_xTable.get() == pCurTable)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            pStNd = InsertTempTableCaptionSection();
        else
            pStNd = InsertTableSection( RES_POOLCOLL_TEXT );

#ifdef NO_LIBO_HTML_PARSER_LEAK_FIX
        _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_CAPTION_ON );
#else	// NO_LIBO_HTML_PARSER_LEAK_FIX
        std::unique_ptr<_HTMLAttrContext> xCntxt(new _HTMLAttrContext(HTML_CAPTION_ON));
#endif	// NO_LIBO_HTML_PARSER_LEAK_FIX

        // Tabellen-Ueberschriften sind immer zentriert.
        NewAttr( &aAttrTab.pAdjust, SvxAdjustItem(SVX_ADJUST_CENTER, RES_PARATR_ADJUST) );

#ifdef NO_LIBO_HTML_PARSER_LEAK_FIX
        _HTMLAttrs &rAttrs = pCntxt->GetAttrs();
#else	// NO_LIBO_HTML_PARSER_LEAK_FIX
        _HTMLAttrs &rAttrs = xCntxt->GetAttrs();
#endif	// NO_LIBO_HTML_PARSER_LEAK_FIX
        rAttrs.push_back( aAttrTab.pAdjust );

#ifdef NO_LIBO_HTML_PARSER_LEAK_FIX
        PushContext( pCntxt );
#else	// NO_LIBO_HTML_PARSER_LEAK_FIX
        PushContext(xCntxt);
#endif	// NO_LIBO_HTML_PARSER_LEAK_FIX

        // StartNode der Section an der Tabelle merken.
        pCurTable->SetCaption( pStNd, bTop );

        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    // naechstes Token

    // </CAPTION> wird laut DTD benoetigt
    bool bDone = false;
    while( IsParserWorking() && !bDone )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pPendStack )
            {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                pSaveStruct->pTable = pTable;
                bool bHasToFly = pSaveStruct->pTable!=pCurTable;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                xSaveStruct->m_xTable = m_xTable;
                bool bHasToFly = xSaveStruct->m_xTable.get() != pCurTable;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                BuildTable( pCurTable->GetTableAdjust( true ),
                            false, true, bHasToFly );
            }
            else
            {
                BuildTable( SVX_ADJUST_END );
            }
            if( SVPAR_PENDING != GetStatus() )
            {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                pTable = pSaveStruct->pTable;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                m_xTable = xSaveStruct->m_xTable;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            }
            break;
        case HTML_TABLE_OFF:
        case HTML_COLGROUP_ON:
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
        case HTML_TABLEROW_ON:
            SkipToken( -1 );
            bDone = true;
            break;

        case HTML_CAPTION_OFF:
            bDone = true;
            break;
        default:
            if( pPendStack )
            {
                SwPendingStack* pTmp = pPendStack->pNext;
                delete pPendStack;
                pPendStack = pTmp;

                OSL_ENSURE( !pTmp, "weiter kann es nicht gehen!" );
            }

            if( IsParserWorking() )
                NextToken( nToken );
            break;
        }

        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING==GetStatus() )
    {
        pPendStack = new SwPendingStack( HTML_CAPTION_ON, pPendStack );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pPendStack->pData = pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pPendStack->pData = xSaveStruct.release();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        return;
    }

    // Alle noch offenen Kontexte beenden
    while( (sal_uInt16)aContexts.size() > nContextStAttrMin+1 )
    {
#ifdef NO_LIBO_HTML_PARSER_LEAK_FIX
        _HTMLAttrContext *pCntxt = PopContext();
        EndContext( pCntxt );
        delete pCntxt;
#else	// NO_LIBO_HTML_PARSER_LEAK_FIX
        std::unique_ptr<_HTMLAttrContext> xCntxt(PopContext());
        EndContext(xCntxt.get());
#endif	// NO_LIBO_HTML_PARSER_LEAK_FIX
    }

    // LF am Absatz-Ende entfernen
    bool bLFStripped = StripTrailingLF() > 0;

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( pTable==pCurTable )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if (m_xTable.get() == pCurTable)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
        // Beim spaeteren verschieben der Beschriftung vor oder hinter
        // die Tabelle wird der letzte Absatz nicht mitverschoben.
        // Deshalb muss sich am Ende der Section immer ein leerer
        // Absatz befinden.
        if( pPam->GetPoint()->nContent.GetIndex() || bLFStripped )
            AppendTxtNode( AM_NOSPACE );
    }
    else
    {
        // LFs am Absatz-Ende entfernen
        if( !pPam->GetPoint()->nContent.GetIndex() && !bLFStripped )
            StripTrailingPara();
    }

    // falls fuer die Zelle eine Ausrichtung gesetzt wurde, muessen
    // wir die beenden
#ifdef NO_LIBO_HTML_PARSER_LEAK_FIX
    _HTMLAttrContext *pCntxt = PopContext();
    EndContext( pCntxt );
    delete pCntxt;
#else	// NO_LIBO_HTML_PARSER_LEAK_FIX
    std::unique_ptr<_HTMLAttrContext> xCntxt(PopContext());
    EndContext(xCntxt.get());
    xCntxt.reset();
#endif	// NO_LIBO_HTML_PARSER_LEAK_FIX

    SetAttr( false );

    // Stacks und Attribut-Tabelle wiederherstellen
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pSaveStruct->RestoreAll( *this );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    xSaveStruct->RestoreAll(*this);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    // PaM wiederherstellen.
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    *pPam->GetPoint() = pSaveStruct->GetPos();

    delete pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    *pPam->GetPoint() = xSaveStruct->GetPos();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
}

class _TblSaveStruct : public SwPendingStackData
{
public:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTable *pCurTable;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::shared_ptr<HTMLTable> m_xCurrentTable;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    _TblSaveStruct( HTMLTable *pCurTbl ) :
        pCurTable( pCurTbl )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    explicit _TblSaveStruct(const std::shared_ptr<HTMLTable>& rCurTable)
        : m_xCurrentTable(rCurTable)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {}

    virtual ~_TblSaveStruct();

    // Aufbau der Tabelle anstossen und die Tabelle ggf. in einen
    // Rahmen packen. Wenn true zurueckgegeben wird muss noch ein
    // Absatz eingefuegt werden!
    void MakeTable( sal_uInt16 nWidth, SwPosition& rPos, SwDoc *pDoc );
};

_TblSaveStruct::~_TblSaveStruct()
{}

void _TblSaveStruct::MakeTable( sal_uInt16 nWidth, SwPosition& rPos, SwDoc *pDoc )
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    pCurTable->MakeTable( 0, nWidth );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    m_xCurrentTable->MakeTable(nullptr, nWidth);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    _HTMLTableContext *pTCntxt = pCurTable->GetContext();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    _HTMLTableContext *pTCntxt = m_xCurrentTable->GetContext();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    OSL_ENSURE( pTCntxt, "Wo ist der Tabellen-Kontext" );

    SwTableNode *pTblNd = pTCntxt->GetTableNode();
    OSL_ENSURE( pTblNd, "Wo ist der Tabellen-Node" );

    if( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() && pTblNd )
    {
        // Existiert schon ein Layout, dann muss an dieser Tabelle die
        // BoxFrames neu erzeugt werden.

        if( pTCntxt->GetFrmFmt() )
        {
            pTCntxt->GetFrmFmt()->DelFrms();
            pTblNd->DelFrms();
            pTCntxt->GetFrmFmt()->MakeFrms();
        }
        else
        {
            pTblNd->DelFrms();
            SwNodeIndex aIdx( *pTblNd->EndOfSectionNode(), 1 );
            OSL_ENSURE( aIdx.GetIndex() <= pTCntxt->GetPos()->nNode.GetIndex(),
                    "unerwarteter Node fuer das Tabellen-Layout" );
            pTblNd->MakeFrms( &aIdx );
        }
    }

    rPos = *pTCntxt->GetPos();
}

HTMLTableOptions::HTMLTableOptions( const HTMLOptions& rOptions,
                                    SvxAdjust eParentAdjust ) :
    nCols( 0 ),
    nWidth( 0 ), nHeight( 0 ),
    nCellPadding( USHRT_MAX ), nCellSpacing( USHRT_MAX ),
    nBorder( USHRT_MAX ),
    nHSpace( 0 ), nVSpace( 0 ),
    eAdjust( eParentAdjust ), eVertOri( text::VertOrientation::CENTER ),
    eFrame( HTML_TF_VOID ), eRules( HTML_TR_NONE ),
    bPrcWidth( false ),
    bTableAdjust( false ),
    bBGColor( false ),
    aBorderColor( COL_GRAY )
{
    bool bBorderColor = false;
    bool bHasFrame = false, bHasRules = false;

    for (size_t i = rOptions.size(); i; )
    {
        const HTMLOption& rOption = rOptions[--i];
        switch( rOption.GetToken() )
        {
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_COLS:
            nCols = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_WIDTH:
            nWidth = (sal_uInt16)rOption.GetNumber();
            bPrcWidth = (rOption.GetString().indexOf('%') != -1);
            if( bPrcWidth && nWidth>100 )
                nWidth = 100;
            break;
        case HTML_O_HEIGHT:
            nHeight = (sal_uInt16)rOption.GetNumber();
            if( rOption.GetString().indexOf('%') != -1 )
                nHeight = 0;    // keine %-Anagben benutzen!!!
            break;
        case HTML_O_CELLPADDING:
            nCellPadding = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_CELLSPACING:
            nCellSpacing = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_ALIGN:
            {
                sal_uInt16 nAdjust = static_cast< sal_uInt16 >(eAdjust);
                if( rOption.GetEnum( nAdjust, aHTMLPAlignTable ) )
                {
                    eAdjust = (SvxAdjust)nAdjust;
                    bTableAdjust = true;
                }
            }
            break;
        case HTML_O_VALIGN:
            eVertOri = rOption.GetEnum( aHTMLTblVAlignTable, eVertOri );
            break;
        case HTML_O_BORDER:
            // BORDER und BORDER=BORDER wie BORDER=1 behandeln
            if (!rOption.GetString().isEmpty() &&
                !rOption.GetString().equalsIgnoreAsciiCase(
                        OOO_STRING_SVTOOLS_HTML_O_border))
            {
                nBorder = (sal_uInt16)rOption.GetNumber();
            }
            else
                nBorder = 1;

            if( !bHasFrame )
                eFrame = ( nBorder ? HTML_TF_BOX : HTML_TF_VOID );
            if( !bHasRules )
                eRules = ( nBorder ? HTML_TR_ALL : HTML_TR_NONE );
            break;
        case HTML_O_FRAME:
            eFrame = rOption.GetTableFrame();
            bHasFrame = true;
            break;
        case HTML_O_RULES:
            eRules = rOption.GetTableRules();
            bHasRules = true;
            break;
        case HTML_O_BGCOLOR:
            // Leere BGCOLOR bei <TABLE>, <TR> und <TD>/<TH> wie Netscape
            // ignorieren, bei allen anderen Tags *wirklich* nicht.
            if( !rOption.GetString().isEmpty() )
            {
                rOption.GetColor( aBGColor );
                bBGColor = true;
            }
            break;
        case HTML_O_BACKGROUND:
            aBGImage = rOption.GetString();
            break;
        case HTML_O_BORDERCOLOR:
            rOption.GetColor( aBorderColor );
            bBorderColor = true;
            break;
        case HTML_O_BORDERCOLORDARK:
            if( !bBorderColor )
                rOption.GetColor( aBorderColor );
            break;
        case HTML_O_STYLE:
            aStyle = rOption.GetString();
            break;
        case HTML_O_CLASS:
            aClass = rOption.GetString();
            break;
        case HTML_O_DIR:
            aDir = rOption.GetString();
            break;
        case HTML_O_HSPACE:
            nHSpace = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_VSPACE:
            nVSpace = (sal_uInt16)rOption.GetNumber();
            break;
        }
    }

    if( nCols && !nWidth )
    {
        nWidth = 100;
        bPrcWidth = true;
    }

    // Wenn BORDER=0 oder kein BORDER gegeben ist, daan darf es auch
    // keine Umrandung geben
    if( 0==nBorder || USHRT_MAX==nBorder )
    {
        eFrame = HTML_TF_VOID;
        eRules = HTML_TR_NONE;
    }
}

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
HTMLTable *SwHTMLParser::BuildTable( SvxAdjust eParentAdjust,
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
std::shared_ptr<HTMLTable> SwHTMLParser::BuildTable(SvxAdjust eParentAdjust,
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                     bool bIsParentHead,
                                     bool bHasParentSection,
                                     bool bHasToFly )
{
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( !IsParserWorking() && !pPendStack )
        return 0;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if (!IsParserWorking() && !pPendStack)
        return std::shared_ptr<HTMLTable>();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    int nToken = 0;
    bool bPending = false;
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    _TblSaveStruct* pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::unique_ptr<_TblSaveStruct> xSaveStruct;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    if( pPendStack )
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pSaveStruct = (_TblSaveStruct*)pPendStack->pData;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct.reset(static_cast<_TblSaveStruct*>(pPendStack->pData));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && pPendStack != 0;

        SaveState( nToken );
    }
    else
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pTable = 0;
        HTMLTableOptions *pTblOptions =
            new HTMLTableOptions( GetOptions(), eParentAdjust );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        m_xTable.reset();
        HTMLTableOptions aTableOptions(GetOptions(), eParentAdjust);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        if( !pTblOptions->aId.isEmpty() )
            InsertBookmark( pTblOptions->aId );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if (!aTableOptions.aId.isEmpty())
            InsertBookmark(aTableOptions.aId);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        HTMLTable *pCurTable = new HTMLTable( this, pTable,
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        std::shared_ptr<HTMLTable> xCurTable(std::make_shared<HTMLTable>(this, m_xTable.get(),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                              bIsParentHead,
                                              bHasParentSection,
                                              bHasToFly,
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                                              pTblOptions );
        if( !pTable )
            pTable = pCurTable;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                                              aTableOptions));
        if (!m_xTable)
            m_xTable = xCurTable;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pSaveStruct = new _TblSaveStruct( pCurTable );

        delete pTblOptions;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct.reset(new _TblSaveStruct(xCurTable));
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTable *pCurTable = pSaveStruct->pCurTable;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::shared_ptr<HTMLTable> xCurTable = xSaveStruct->m_xCurrentTable;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

    // </TABLE> wird laut DTD benoetigt
    if( !nToken )
        nToken = GetNextToken();    // naechstes Token

    bool bDone = false;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( pPendStack || !bCallNextToken ||
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                pCurTable->GetContext() || pCurTable->HasParentSection(),
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                xCurTable->GetContext() || xCurTable->HasParentSection(),
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                "Wo ist die Section gebieben?" );
        if( !pPendStack && bCallNextToken &&
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            (xCurTable->GetContext() || xCurTable->HasParentSection()) )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
             /// Call NextToken directly (e.g. ignore the content of floating frames or applets)
        {
            // NextToken direkt aufrufen (z.B. um den Inhalt von
            // Floating-Frames oder Applets zu ignorieren)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            if( !pCurTable->GetContext() )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            if( !xCurTable->GetContext() )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            {
                // Wenn noch keine Tabelle eingefuegt wurde,
                // die naechste Tabelle lesen
                SkipToken( -1 );
                bDone = true;
            }

            break;
        case HTML_TABLE_OFF:
            bDone = true;
            break;
        case HTML_CAPTION_ON:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            BuildTableCaption( pCurTable );
            bDone = pTable->IsOverflowing();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            BuildTableCaption(xCurTable.get());
            bDone = m_xTable->IsOverflowing();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            break;
        case HTML_COL_ON:
            SkipToken( -1 );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            BuildTableColGroup( pCurTable, false );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            BuildTableColGroup(xCurTable.get(), false);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            break;
        case HTML_COLGROUP_ON:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            BuildTableColGroup( pCurTable, true );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            BuildTableColGroup(xCurTable.get(), true);
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            break;
        case HTML_TABLEROW_ON:
        case HTML_TABLEHEADER_ON:
        case HTML_TABLEDATA_ON:
            SkipToken( -1 );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            BuildTableSection( pCurTable, false, false );
            bDone = pTable->IsOverflowing();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            BuildTableSection(xCurTable.get(), false, false);
            bDone = m_xTable->IsOverflowing();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            break;
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            BuildTableSection( pCurTable, true, HTML_THEAD_ON==nToken );
            bDone = pTable->IsOverflowing();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            BuildTableSection(xCurTable.get(), true, HTML_THEAD_ON==nToken);
            bDone = m_xTable->IsOverflowing();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            break;
        case HTML_MULTICOL_ON:
            // spaltige Rahmen koennen wir hier leider nicht einguegen
            break;
        case HTML_FORM_ON:
            NewForm( false );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_FORM_OFF:
            EndForm( false );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_TEXTTOKEN:
            // Blank-Strings sind u. U. eine Folge von CR+LF und kein Text
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            if( (xCurTable->GetContext() ||
                 !xCurTable->HasParentSection()) &&
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                1==aToken.getLength() && ' '==aToken[0] )
                break;
        default:
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            pCurTable->MakeParentContents();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            xCurTable->MakeParentContents();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            NextToken( nToken );
            break;
        }

        OSL_ENSURE( !bPending || !pPendStack,
                "SwHTMLParser::BuildTable: Es gibt wieder einen Pend-Stack" );
        bPending = false;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        pPendStack = new SwPendingStack( HTML_TABLE_ON, pPendStack );
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pPendStack->pData = pSaveStruct;
        return 0;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        pPendStack->pData = xSaveStruct.release();
        return std::shared_ptr<HTMLTable>();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    _HTMLTableContext *pTCntxt = pCurTable->GetContext();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    _HTMLTableContext *pTCntxt = xCurTable->GetContext();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if( pTCntxt )
    {
        // Die Tabelle wurde auch angelegt

        // Tabellen-Struktur anpassen
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        pCurTable->CloseTable();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xCurTable->CloseTable();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

        // ausserhalb von Zellen begonnene Kontexte beenden
        // muss vor(!) dem Umsetzten der Attribut Tabelle existieren,
        // weil die aktuelle danach nicht mehr existiert
        while( aContexts.size() > nContextStAttrMin )
        {
#ifdef NO_LIBO_HTML_PARSER_LEAK_FIX
            _HTMLAttrContext *pCntxt = PopContext();
            ClearContext( pCntxt );
            delete pCntxt;
#else	// NO_LIBO_HTML_PARSER_LEAK_FIX
            std::unique_ptr<_HTMLAttrContext> xCntxt(PopContext());
            ClearContext(xCntxt.get());
#endif	// NO_LIBO_HTML_PARSER_LEAK_FIX
        }

        nContextStMin = pTCntxt->GetContextStMin();
        nContextStAttrMin = pTCntxt->GetContextStAttrMin();

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        if( pTable==pCurTable )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if (m_xTable == xCurTable)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        {
            // Tabellen-Beschriftung setzen
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            const SwStartNode *pCapStNd = pTable->GetCaptionStartNode();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            const SwStartNode *pCapStNd = m_xTable->GetCaptionStartNode();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            if( pCapStNd )
            {
                // Der letzte Absatz der Section wird nie mitkopiert. Deshalb
                // muss die Section mindestens zwei Absaetze enthalten.

                if( pCapStNd->EndOfSectionIndex() - pCapStNd->GetIndex() > 2 )
                {
                    // Start-Node und letzten Absatz nicht mitkopieren.
                    SwNodeRange aSrcRg( *pCapStNd, 1,
                                    *pCapStNd->EndOfSectionNode(), -1 );

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                    bool bTop = pTable->IsTopCaption();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    bool bTop = m_xTable->IsTopCaption();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
                    SwStartNode *pTblStNd = pTCntxt->GetTableNode();

                    OSL_ENSURE( pTblStNd, "Wo ist der Tabellen-Node" );
                    OSL_ENSURE( pTblStNd==pPam->GetNode().FindTableNode(),
                            "Stehen wir in der falschen Tabelle?" );

                    SwNode* pNd;
                    if( bTop )
                        pNd = pTblStNd;
                    else
                        pNd = pTblStNd->EndOfSectionNode();
                    SwNodeIndex aDstIdx( *pNd, bTop ? 0 : 1 );

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    pDoc->getIDocumentContentOperations().MoveNodeRange( aSrcRg, aDstIdx,
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    m_xDoc->getIDocumentContentOperations().MoveNodeRange( aSrcRg, aDstIdx,
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        IDocumentContentOperations::DOC_MOVEDEFAULT );

                    // Wenn die Caption vor der Tabelle eingefuegt wurde muss
                    // eine an der Tabelle gestzte Seitenvorlage noch in den
                    // ersten Absatz der Ueberschrift verschoben werden.
                    // Ausserdem muessen alle gemerkten Indizes, die auf den
                    // Tabellen-Node zeigen noch verschoben werden.
                    if( bTop )
                    {
                        MovePageDescAttrs( pTblStNd, aSrcRg.aStart.GetIndex(),
                                           false );
                    }
                }

                // Die Section wird jetzt nicht mehr gebraucht.
                pPam->SetMark();
                pPam->DeleteMark();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                pDoc->getIDocumentContentOperations().DeleteSection( (SwStartNode *)pCapStNd );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                m_xDoc->getIDocumentContentOperations().DeleteSection( (SwStartNode *)pCapStNd );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
                pTable->SetCaption( 0, false );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
                m_xTable->SetCaption( nullptr, false );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            }

            // SwTable aufbereiten
            sal_uInt16 nBrowseWidth = (sal_uInt16)GetCurrentBrowseWidth();
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pSaveStruct->MakeTable( nBrowseWidth, *pPam->GetPoint(), pDoc );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pSaveStruct->MakeTable( nBrowseWidth, *pPam->GetPoint(), m_xDoc.get() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            xSaveStruct->MakeTable(nBrowseWidth, *pPam->GetPoint(), pDoc);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            xSaveStruct->MakeTable(nBrowseWidth, *pPam->GetPoint(), m_xDoc.get());
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        }

        GetNumInfo().Set( pTCntxt->GetNumInfo() );
        pTCntxt->RestorePREListingXMP( *this );
        RestoreAttrTab( pTCntxt->aAttrTab );

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        if( pTable==pCurTable )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if (m_xTable == xCurTable)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        {
            // oberen Absatz-Abstand einstellen
            bUpperSpace = true;
            SetTxtCollAttrs();

            nParaCnt = nParaCnt - std::min(nParaCnt,
                pTCntxt->GetTableNode()->GetTable().GetTabSortBoxes().size());

            // ggfs. eine Tabelle anspringen
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            if( JUMPTO_TABLE == eJumpTo && pTable->GetSwTable() &&
                pTable->GetSwTable()->GetFrmFmt()->GetName() == sJmpMark )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            if( JUMPTO_TABLE == eJumpTo && m_xTable->GetSwTable() &&
                m_xTable->GetSwTable()->GetFrmFmt()->GetName() == sJmpMark )
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
            {
                bChkJumpMark = true;
                eJumpTo = JUMPTO_NONE;
            }

            // Wenn Import abgebrochen wurde kein erneutes Show
            // aufrufen, weil die SwViewShell schon geloescht wurde!
            // Genuegt nicht. Auch im ACCEPTING_STATE darf
            // kein Show aufgerufen werden, weil sonst waehrend des
            // Reschedules der Parser zerstoert wird, wenn noch ein
            // DataAvailable-Link kommt. Deshalb: Nur im WORKING-State.
            if( !nParaCnt && SVPAR_WORKING == GetStatus() )
                Show();
        }
    }
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    else if( pTable==pCurTable )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    else if (m_xTable == xCurTable)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
        // Es wurde gar keine Tabelle gelesen.

        // Dann muss eine evtl gelesene Beschriftung noch geloescht werden.
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        const SwStartNode *pCapStNd = pCurTable->GetCaptionStartNode();
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        const SwStartNode *pCapStNd = xCurTable->GetCaptionStartNode();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        if( pCapStNd )
        {
            pPam->SetMark();
            pPam->DeleteMark();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pDoc->getIDocumentContentOperations().DeleteSection( (SwStartNode *)pCapStNd );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            m_xDoc->getIDocumentContentOperations().DeleteSection( (SwStartNode *)pCapStNd );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
            pCurTable->SetCaption( 0, false );
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
            xCurTable->SetCaption( nullptr, false );
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
        }
    }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    if( pTable == pCurTable  )
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    if (m_xTable == xCurTable)
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    {
#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
        delete pSaveStruct->pCurTable;
        pSaveStruct->pCurTable = 0;
        pTable = 0;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
        xSaveStruct->m_xCurrentTable.reset();
        m_xTable.reset();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
    }

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    HTMLTable* pRetTbl = pSaveStruct->pCurTable;
    delete pSaveStruct;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    std::shared_ptr<HTMLTable> xRetTable = xSaveStruct->m_xCurrentTable;
    xSaveStruct.reset();
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX

#ifdef NO_LIBO_HTML_TABLE_LEAK_FIX
    return pRetTbl;
#else	// NO_LIBO_HTML_TABLE_LEAK_FIX
    return xRetTable;
#endif	// NO_LIBO_HTML_TABLE_LEAK_FIX
}

#ifndef NO_LIBO_DELETE_IN_CURRENT_TABLE_FIX

bool SwHTMLParser::PendingTableInPaM(SwPaM& rPam) const
{
    if (!m_xTable)
        return false;
    const SwTable *pTable = m_xTable->GetSwTable();
    if (!pTable)
        return false;
    const SwTableNode* pTableNode = pTable->GetTableNode();
    if (!pTableNode)
        return false;
    SwNodeIndex aTableNodeIndex(*pTableNode);
    return (aTableNodeIndex >= rPam.Start()->nNode && aTableNodeIndex <= rPam.End()->nNode);
}

#endif	// !NO_LIBO_DELETE_IN_CURRENT_TABLE_FIX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
