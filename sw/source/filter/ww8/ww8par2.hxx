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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_WW8PAR2_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_WW8PAR2_HXX

#include <swtypes.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <editeng/lrspitem.hxx>

#include "ww8scan.hxx"
#include "ww8par.hxx"

class WW8RStyle;

class WW8DupProperties
{
public:
    WW8DupProperties(SwDoc &rDoc, SwWW8FltControlStack *pStk);
    void Insert(const SwPosition &rPos);
private:
    //No copying
    WW8DupProperties(const WW8DupProperties&);
    WW8DupProperties& operator=(const WW8DupProperties&);
    SwWW8FltControlStack* pCtrlStck;
    SfxItemSet aChrSet,aParSet;
};

struct WW8SwFlyPara
{
    SwFlyFrmFmt* pFlyFmt;

                // 1. Teil: daraus abgeleitete Sw-Attribute
    sal_Int16 nXPos, nYPos;         // Position
    sal_Int16 nLeMgn, nRiMgn;       // Raender
    sal_Int16 nUpMgn, nLoMgn;       // Raender
    sal_Int16 nWidth, nHeight;      // Groesse
    sal_Int16 nNetWidth;

    SwFrmSize eHeightFix;       // Hoehe Fix oder Min
    RndStdIds eAnchor;          // Bindung
    short eHRel;     // Seite oder Seitenrand
    short eVRel;     // Seite oder Seitenrand
    sal_Int16 eVAlign;       // Oben, unten, mittig
    sal_Int16 eHAlign;       // links, rechts, mittig
    SwSurround eSurround;       // Wrap-Mode

    sal_uInt8 nXBind, nYBind;        // relativ zu was gebunden

                // 2.Teil: sich waehrend des Einlesens ergebende AEnderungen
    long nNewNetWidth;
    SwPosition* pMainTextPos;   // um nach Apo in Haupttext zurueckzukehren
    sal_uInt16 nLineSpace;          // LineSpace in tw fuer Graf-Apos
    bool bAutoWidth;
    bool bToggelPos;

    // add parameter <nWWPgTop> - WW8's page top margin
    WW8SwFlyPara( SwPaM& rPaM,
                  SwWW8ImplReader& rIo,
                  WW8FlyPara& rWW,
                  const sal_uInt32 nWWPgTop,
                  const sal_uInt32 nPgLeft,
                  const sal_uInt32 nPgWidth,
                  const sal_Int32 nIniFlyDx,
                  const sal_Int32 nIniFlyDy );

    void BoxUpWidth( long nWidth );
    SwWW8FltAnchorStack *pOldAnchorStck;
};

class WW8RStyle: public WW8Style
{
friend class SwWW8ImplReader;
    wwSprmParser maSprmParser;
    SwWW8ImplReader* pIo;   // Parser-Klasse
    SvStream* pStStrm;      // Input-File

    SwNumRule* pStyRule;    // Bullets und Aufzaehlungen in Styles

    sal_uInt8* pParaSprms;           // alle ParaSprms des UPX falls UPX.Papx
    sal_uInt16 nSprmsLen;           // Laenge davon

    sal_uInt8 nWwNumLevel;           // fuer Bullets und Aufzaehlungen in Styles

    bool bTxtColChanged;
    bool bFontChanged;      // For Simulating Default-Font
    bool bCJKFontChanged;   // For Simulating Default-CJK Font
    bool bCTLFontChanged;   // For Simulating Default-CTL Font
    bool bFSizeChanged;     // For Simulating Default-FontSize
    bool bFCTLSizeChanged;  // For Simulating Default-CTL FontSize
    bool bWidowsChanged;    // For Simulating Default-Widows / Orphans

    void ImportSprms(sal_Size nPosFc, short nLen, bool bPap);
    void ImportSprms(sal_uInt8 *pSprms, short nLen, bool bPap);
    void ImportGrupx(short nLen, bool bPara, bool bOdd);
    short ImportUPX(short nLen, bool bPAP, bool bOdd);

    void Set1StyleDefaults();
    void Import1Style(sal_uInt16 nNr);
    void RecursiveReg(sal_uInt16 nNr);

    void ImportStyles();

    void ImportNewFormatStyles();
    void ScanStyles();
    void ImportOldFormatStyles();

    bool PrepareStyle(SwWW8StyInf &rSI, ww::sti eSti, sal_uInt16 nThisStyle, sal_uInt16 nNextStyle);
    void PostStyle(SwWW8StyInf &rSI, bool bOldNoImp);

    //No copying
    WW8RStyle(const WW8RStyle&);
    WW8RStyle& operator=(const WW8RStyle&);
public:
    WW8RStyle( WW8Fib& rFib, SwWW8ImplReader* pI );
    void Import();
    void PostProcessStyles();
    const sal_uInt8* HasParaSprm( sal_uInt16 nId ) const;
};

class WW8FlySet: public SfxItemSet
{
private:
    //No copying
    const WW8FlySet& operator=(const WW8FlySet&);
    void Init(const SwWW8ImplReader& rReader, const SwPaM* pPaM);
public:
    WW8FlySet(SwWW8ImplReader& rReader, const WW8FlyPara* pFW,
        const WW8SwFlyPara* pFS, bool bGraf);
    WW8FlySet(SwWW8ImplReader& rReader, const SwPaM* pPaM, const WW8_PIC& rPic,
        long nWidth, long nHeight);
};

#ifndef NO_LIBO_WW8_TABLE_LEAK_FIX

class WW8SelBoxInfo
    : public std::vector<SwTableBox*>, private boost::noncopyable
{
public:
    short nGroupXStart;
    short nGroupWidth;
    bool bGroupLocked;

    WW8SelBoxInfo(short nXCenter, short nWidth)
        : nGroupXStart( nXCenter ), nGroupWidth( nWidth ), bGroupLocked(false)
    {}
};

typedef boost::ptr_vector<WW8SelBoxInfo> WW8MergeGroups;

class WW8TabDesc: private boost::noncopyable
{
    std::vector<OUString> aNumRuleNames;
    sw::util::RedlineStack *mpOldRedlineStack;

    SwWW8ImplReader* pIo;

    WW8TabBandDesc* pFirstBand;
    WW8TabBandDesc* pActBand;

#ifdef NO_LIBO_WW8_TABLE_LEAK_FIX
    SwPosition* pTmpPos;
#else	// NO_LIBO_WW8_TABLE_LEAK_FIX
    std::unique_ptr<SwPosition> m_xTmpPos;
#endif	// NO_LIBO_WW8_TABLE_LEAK_FIX

    SwTableNode* pTblNd;            // table node
    const SwTableLines* pTabLines;  // row array of node
    SwTableLine* pTabLine;          // current row
    SwTableBoxes* pTabBoxes;        // boxes array in current row
    SwTableBox* pTabBox;            // current cell

    WW8MergeGroups aMergeGroups;   // list of all cells to be merged

    WW8_TCell* pAktWWCell;

    short nRows;
    short nDefaultSwCols;
    short nBands;
    short nMinLeft;
    short nConvertedLeft;
    short nMaxRight;
    short nSwWidth;
    short nPreferredWidth;
    short nOrgDxaLeft;

    bool bOk;
    bool bClaimLineFmt;
    sal_Int16 eOri;
    bool bIsBiDi;
                                // 2. common admin info
    short nAktRow;
    short nAktBandRow;          // SW: row of current band
                                // 3. admin info for writer
    short nAktCol;

    sal_uInt16 nRowsToRepeat;

    // 4. methods

    sal_uInt16 GetLogicalWWCol() const;
    void SetTabBorders( SwTableBox* pBox, short nIdx );
    void SetTabShades( SwTableBox* pBox, short nWwIdx );
    void SetTabVertAlign( SwTableBox* pBox, short nWwIdx );
    void SetTabDirection( SwTableBox* pBox, short nWwIdx );
    void CalcDefaults();
    bool SetPamInCell(short nWwCol, bool bPam);
    void InsertCells( short nIns );
    void AdjustNewBand();

    WW8SelBoxInfo* FindMergeGroup(short nX1, short nWidth, bool bExact);

    // single box - maybe used in a merge group
    // (the merge groups are processed later at once)
    SwTableBox* UpdateTableMergeGroup(WW8_TCell& rCell,
        WW8SelBoxInfo* pActGroup, SwTableBox* pActBox, sal_uInt16 nCol  );
    void StartMiserableHackForUnsupportedDirection(short nWwCol);
    void EndMiserableHackForUnsupportedDirection(short nWwCol);

public:
    const SwTable* pTable;          // table
    SwPosition* pParentPos;
    SwFlyFrmFmt* pFlyFmt;
    SfxItemSet aItemSet;
    bool IsValidCell(short nCol) const;
    bool InFirstParaInCell() const;

    WW8TabDesc( SwWW8ImplReader* pIoClass, WW8_CP nStartCp );
    bool Ok() const { return bOk; }
    void CreateSwTable(SvxULSpaceItem* pULSpaceItem = 0);
    void UseSwTable();
    void SetSizePosition(SwFrmFmt* pFrmFmt);
    void TableCellEnd();
    void MoveOutsideTable();
    void ParkPaM();
    void FinishSwTable();
    void MergeCells();
    short GetMinLeft() const { return nConvertedLeft; }
    ~WW8TabDesc();

    const WW8_TCell* GetAktWWCell() const { return pAktWWCell; }
    short GetAktCol() const { return nAktCol; }
    // find name of numrule valid for current WW-COL
    OUString GetNumRuleName() const;
    void SetNumRuleName( const OUString& rName );

    sw::util::RedlineStack* getOldRedlineStack(){ return mpOldRedlineStack; }
};

#endif	// !NO_LIBO_WW8_TABLE_LEAK_FIX

enum WW8LvlType {WW8_None, WW8_Outline, WW8_Numbering, WW8_Sequence, WW8_Pause};

WW8LvlType GetNumType(sal_uInt8 nWwLevelNo);
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
