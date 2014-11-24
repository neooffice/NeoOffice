/*************************************************************************
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of NeoOffice.
 *
 * NeoOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * NeoOffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 3 along with NeoOffice.  If not, see
 * <http://www.gnu.org/licenses/gpl-3.0.txt>
 * for a copy of the GPLv3 License.
 *
 * Modified January 2009 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */
#include <hintids.hxx>

#include <stack>

#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#include <tools/stream.hxx>
#include <svtools/itemiter.hxx>
#include <svtools/rtftoken.h>
#include <svtools/intitem.hxx>
#include <svx/fhgtitem.hxx>
#include <svx/ulspitem.hxx>
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#include <svx/lspcitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/escpitem.hxx>
#include <svx/fontitem.hxx>
#include <svx/frmdiritem.hxx>
#include <svx/hyznitem.hxx>
#include <fmtpdsc.hxx>
#include <fmtfld.hxx>
#include <fmthbsh.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <txtftn.hxx>
#include <fmtclds.hxx>
#include <fmtftn.hxx>
#include <fmtfsize.hxx>
#include <fmtflcnt.hxx>
#include <fmtanchr.hxx>
#include <frmatr.hxx>
#include <docstat.hxx>
#include <swtable.hxx>
#include <shellio.hxx>
#include <swtypes.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <mdiexp.hxx>           // ...Percent()
#include <swparrtf.hxx>
#include <charfmt.hxx>
#include <pagedesc.hxx>
#include <ftninfo.hxx>
#include <docufld.hxx>
#include <flddat.hxx>
#include <fltini.hxx>
#include <fchrfmt.hxx>
#include <paratr.hxx>
#ifndef _SECTIOM_HXX
#include <section.hxx>
#endif
#include <fmtclbl.hxx>
#include <viewsh.hxx>
#include <shellres.hxx>
#include <hfspacingitem.hxx>
#include <tox.hxx>
#include <swerror.h>
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>          // ResId fuer Statusleiste
#endif
#include <SwStyleNameMapper.hxx>
#include <tblsel.hxx>           // SwSelBoxes

#include <docsh.hxx>
#include <fmtlsplt.hxx> // SwLayoutSplit
#include <svx/keepitem.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdorect.hxx>


#include <fmtsrnd.hxx>
#include <fmtfollowtextflow.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/opaqitem.hxx>
#include "svx/svdograf.hxx"
#include <svx/xflclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/svdoutl.hxx>
#include <svx/outlobj.hxx>

#include <tools/stream.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <vcl/salbtype.hxx>     // FRound

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#if SUPD == 310
#include <comphelper/processfactory.hxx>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <sal/log.hxx>
#include <unotools/streamwrap.hxx>
#include <unoobj.hxx>
#endif	// SUPD == 310


using namespace ::com::sun::star;


// einige Hilfs-Funktionen
// char
inline const SvxFontHeightItem& GetSize(const SfxItemSet& rSet,BOOL bInP=TRUE)
    { return (const SvxFontHeightItem&)rSet.Get( RES_CHRATR_FONTSIZE,bInP); }
inline const SvxLRSpaceItem& GetLRSpace(const SfxItemSet& rSet,BOOL bInP=TRUE)
    { return (const SvxLRSpaceItem&)rSet.Get( RES_LR_SPACE,bInP); }

/*  */

#if SUPD != 310

extern "C" SAL_DLLPUBLIC_EXPORT Reader* SAL_CALL ImportRTF()
{
    return new RtfReader();
}

// Aufruf fuer die allg. Reader-Schnittstelle
ULONG RtfReader::Read( SwDoc &rDoc, const String& rBaseURL, SwPaM &rPam, const String &)
{
    if( !pStrm )
    {
        ASSERT( FALSE, "RTF-Read ohne Stream" );
        return ERR_SWG_READ_ERROR;
    }

    //JP 18.01.96: Alle Ueberschriften sind normalerweise ohne
    //              Kapitelnummer. Darum hier explizit abschalten
    //              weil das Default jetzt wieder auf AN ist.
    if( !bInsertMode )
    {
        Reader::SetNoOutlineNum( rDoc );

        // MIB 27.09.96: Umrandung uns Abstaende aus Frm-Vorlagen entf.
        Reader::ResetFrmFmts( rDoc );
    }

    ULONG nRet = 0;
    SwDocShell *pDocShell(rDoc.GetDocShell());
    DBG_ASSERT(pDocShell, "no SwDocShell");
    uno::Reference<document::XDocumentProperties> xDocProps;
    if (pDocShell) {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            pDocShell->GetModel(), uno::UNO_QUERY_THROW);
        xDocProps.set(xDPS->getDocumentProperties());
    }

    SvParserRef xParser = new SwRTFParser( &rDoc, xDocProps,
                                rPam, *pStrm, rBaseURL, !bInsertMode );
    SvParserState eState = xParser->CallParser();
    if( SVPAR_PENDING != eState && SVPAR_ACCEPTED != eState )
    {
        String sErr( String::CreateFromInt32( xParser->GetLineNr() ));
        sErr += ',';
        sErr += String::CreateFromInt32( xParser->GetLinePos() );

        nRet = *new StringErrorInfo( ERR_FORMAT_ROWCOL, sErr,
                                    ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
    }


    return nRet;
}

SwRTFParser::SwRTFParser(SwDoc* pD,
        uno::Reference<document::XDocumentProperties> i_xDocProps,
        const SwPaM& rCrsr, SvStream& rIn, const String& rBaseURL,
        int bReadNewDoc) :
    SvxRTFParser(pD->GetAttrPool(), rIn, i_xDocProps, bReadNewDoc),
    maParaStyleMapper(*pD),
    maCharStyleMapper(*pD),
    maSegments(*this),
    maInsertedTables(*pD),
    aMergeBoxes(0, 5),
    aTblFmts(0, 10),
    mpBookmarkStart(0),
    mpRedlineStack(0),
    pAuthorInfos(0),
    pGrfAttrSet(0),
    pTableNode(0),
    pOldTblNd(0),
    pSttNdIdx(0),
    pRegionEndIdx(0),
    pDoc(pD),
    pRelNumRule(new SwRelNumRuleSpaces(*pD, static_cast< BOOL >(bReadNewDoc))),
    pRedlineInsert(0),
    pRedlineDelete(0),
    sBaseURL( rBaseURL ),
    nAktPageDesc(0),
    nAktFirstPageDesc(0),
    nAktBox(0),
    nInsTblRow(USHRT_MAX),
    nNewNumSectDef(USHRT_MAX),
    nRowsToRepeat(0),
    // --> OD 2008-12-22 #i83368#
    mbReadCellWhileReadSwFly( false ),
    // <--
    bTrowdRead(0),
    nReadFlyDepth(0),
    nZOrder(0)
{
    mbIsFootnote = mbReadNoTbl = bReadSwFly = bSwPageDesc = bStyleTabValid =
    bInPgDscTbl = bNewNumList = false;
    bFirstContinue = true;
    bContainsPara = false;
    bContainsTablePara = false;
    bNestedField = false;
    bForceNewTable = false;

    pPam = new SwPaM( *rCrsr.GetPoint() );
    SetInsPos( SwxPosition( pPam ) );
    SetChkStyleAttr( 0 != bReadNewDoc );
    SetCalcValue( FALSE );
    SetReadDocInfo( TRUE );

    // diese sollen zusaetzlich ueber \pard zurueck gesetzt werden
    USHORT temp;
    temp = RES_TXTATR_CHARFMT;      AddPlainAttr( temp );
    temp = RES_PAGEDESC;            AddPardAttr( temp );
    temp = RES_BREAK;               AddPardAttr( temp );
    temp = RES_PARATR_NUMRULE;      AddPardAttr( temp );
    temp = FN_PARAM_NUM_LEVEL;          AddPardAttr( temp );
}

// Aufruf des Parsers
SvParserState SwRTFParser::CallParser()
{
    mbReadNoTbl = false;
    bFirstContinue = true;

    rInput.Seek(STREAM_SEEK_TO_BEGIN);
    rInput.ResetError();

    mpRedlineStack = new sw::util::RedlineStack(*pDoc);

    return SvxRTFParser::CallParser();
}

#endif	// SUPD != 310

bool lcl_UsedPara(SwPaM &rPam)
{
    const SwCntntNode* pCNd;
    const SfxItemSet* pSet;
    if( rPam.GetPoint()->nContent.GetIndex() ||
        ( 0 != ( pCNd = rPam.GetCntntNode()) &&
          0 != ( pSet = pCNd->GetpSwAttrSet()) &&
         ( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, FALSE ) ||
           SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, FALSE ))))
        return true;
    return false;
}

#if SUPD != 310

void SwRTFParser::Continue( int nToken )
{
    if( bFirstContinue )
    {
        bFirstContinue = FALSE;

        if (IsNewDoc())
        {
            //
            // COMPATIBILITY FLAGS START
            //
            pDoc->set(IDocumentSettingAccess::PARA_SPACE_MAX, true);
            pDoc->set(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES, true);
            pDoc->set(IDocumentSettingAccess::TAB_COMPAT, true);
            pDoc->set(IDocumentSettingAccess::USE_VIRTUAL_DEVICE, true);
            pDoc->set(IDocumentSettingAccess::USE_HIRES_VIRTUAL_DEVICE, true);
            pDoc->set(IDocumentSettingAccess::ADD_FLY_OFFSETS, true);
            pDoc->set(IDocumentSettingAccess::ADD_EXT_LEADING, true);
            pDoc->set(IDocumentSettingAccess::OLD_NUMBERING, false);
            pDoc->set(IDocumentSettingAccess::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING, false );
            pDoc->set(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK, false);
            pDoc->set(IDocumentSettingAccess::OLD_LINE_SPACING, false);
            pDoc->set(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS, true);
            pDoc->set(IDocumentSettingAccess::USE_FORMER_OBJECT_POS, false);
            pDoc->set(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING, false);
            pDoc->set(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION, true);
            pDoc->set(IDocumentSettingAccess::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT, false); // --> FME 2005-08-11 #i53199#
            // --> FME 2006-02-10 #131283#
            pDoc->set(IDocumentSettingAccess::TABLE_ROW_KEEP, true);
            pDoc->set(IDocumentSettingAccess::IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION, true);
	    pDoc->set(IDocumentSettingAccess::INVERT_BORDER_SPACING, true);
            //
            // COMPATIBILITY FLAGS END
            //
        }

        // einen temporaeren Index anlegen, auf Pos 0 so wird er nicht bewegt!
        pSttNdIdx = new SwNodeIndex( pDoc->GetNodes() );
        if( !IsNewDoc() )       // in ein Dokument einfuegen ?
        {
            const SwPosition* pPos = pPam->GetPoint();
            SwTxtNode* pSttNd = pPos->nNode.GetNode().GetTxtNode();

            pDoc->SplitNode( *pPos, false );

            *pSttNdIdx = pPos->nNode.GetIndex()-1;
            pDoc->SplitNode( *pPos, false );

            SwPaM aInsertionRangePam( *pPos );

            pPam->Move( fnMoveBackward );

            // #106634# split any redline over the insertion point
            aInsertionRangePam.SetMark();
            *aInsertionRangePam.GetPoint() = *pPam->GetPoint();
            aInsertionRangePam.Move( fnMoveBackward );
            pDoc->SplitRedline( aInsertionRangePam );

            pDoc->SetTxtFmtColl( *pPam, pDoc->GetTxtCollFromPool
                                 ( RES_POOLCOLL_STANDARD, false ));

            // verhinder das einlesen von Tabellen in Fussnoten / Tabellen
            ULONG nNd = pPos->nNode.GetIndex();
            mbReadNoTbl = 0 != pSttNd->FindTableNode() ||
                        ( nNd < pDoc->GetNodes().GetEndOfInserts().GetIndex() &&
                        pDoc->GetNodes().GetEndOfInserts().StartOfSectionIndex() < nNd );
        }

        // Laufbalken anzeigen, aber nur bei synchronem Call
        ULONG nCurrPos = rInput.Tell();
        rInput.Seek(STREAM_SEEK_TO_END);
        rInput.ResetError();
        ::StartProgress( STR_STATSTR_W4WREAD, 0, rInput.Tell(), pDoc->GetDocShell());
        rInput.Seek( nCurrPos );
        rInput.ResetError();
    }

    SvxRTFParser::Continue( nToken );

    if( SVPAR_PENDING == GetStatus() )
        return ;                // weiter gehts beim naechsten mal

    pRelNumRule->SetNumRelSpaces( *pDoc );

    // den Start wieder korrigieren
    if( !IsNewDoc() && pSttNdIdx->GetIndex() )
    {
        //die Flys muessen zuerst zurecht gerueckt werden, denn sonst wird
        // ein am 1. Absatz verankerter Fly falsch eingefuegt
        if( SVPAR_ACCEPTED == eState )
        {
            if( aFlyArr.Count() )
                SetFlysInDoc();
            pRelNumRule->SetOultineRelSpaces( *pSttNdIdx, pPam->GetPoint()->nNode );
        }

        SwTxtNode* pTxtNode = pSttNdIdx->GetNode().GetTxtNode();
        SwNodeIndex aNxtIdx( *pSttNdIdx );
        if( pTxtNode && pTxtNode->CanJoinNext( &aNxtIdx ))
        {
            xub_StrLen nStt = pTxtNode->GetTxt().Len();
            // wenn der Cursor noch in dem Node steht, dann setze in an das Ende
            if( pPam->GetPoint()->nNode == aNxtIdx )
            {
                pPam->GetPoint()->nNode = *pSttNdIdx;
                pPam->GetPoint()->nContent.Assign( pTxtNode, nStt );
            }

#ifndef PRODUCT
// !!! sollte nicht moeglich sein, oder ??
ASSERT( pSttNdIdx->GetIndex()+1 != pPam->GetBound( TRUE ).nNode.GetIndex(),
            "Pam.Bound1 steht noch im Node" );
ASSERT( pSttNdIdx->GetIndex()+1 != pPam->GetBound( FALSE ).nNode.GetIndex(),
            "Pam.Bound2 steht noch im Node" );

if( pSttNdIdx->GetIndex()+1 == pPam->GetBound( TRUE ).nNode.GetIndex() )
{
    xub_StrLen nCntPos = pPam->GetBound( TRUE ).nContent.GetIndex();
    pPam->GetBound( TRUE ).nContent.Assign( pTxtNode,
                    pTxtNode->GetTxt().Len() + nCntPos );
}
if( pSttNdIdx->GetIndex()+1 == pPam->GetBound( FALSE ).nNode.GetIndex() )
{
    xub_StrLen nCntPos = pPam->GetBound( FALSE ).nContent.GetIndex();
    pPam->GetBound( FALSE ).nContent.Assign( pTxtNode,
                    pTxtNode->GetTxt().Len() + nCntPos );
}
#endif
            // Zeichen Attribute beibehalten!
            SwTxtNode* pDelNd = aNxtIdx.GetNode().GetTxtNode();
            if( pTxtNode->GetTxt().Len() )
                pDelNd->FmtToTxtAttr( pTxtNode );
            else
                pTxtNode->ChgFmtColl( pDelNd->GetTxtColl() );
            pTxtNode->JoinNext();
        }
    }

    if( SVPAR_ACCEPTED == eState )
    {
        // den letzen Bereich wieder zumachen
        if( pRegionEndIdx )
        {
            // JP 06.01.00: Task 71411 - the last section in WW are not a
            //              balanced Section.
            if( !GetVersionNo() )
            {
                SwSectionNode* pSectNd = pRegionEndIdx->GetNode().
                                    StartOfSectionNode()->GetSectionNode();
                if( pSectNd )
                    pSectNd->GetSection().GetFmt()->SetFmtAttr(
                                    SwFmtNoBalancedColumns( TRUE ) );
            }

            DelLastNode();
            pPam->GetPoint()->nNode = *pRegionEndIdx;
            pPam->Move( fnMoveForward, fnGoNode );
            delete pRegionEndIdx, pRegionEndIdx = 0;
        }

        sal_uInt16 nPageDescOffset = pDoc->GetPageDescCnt();
        maSegments.InsertSegments(IsNewDoc());
        UpdatePageDescs(*pDoc, nPageDescOffset);
        //$flr folloing garbe collecting code has been moved from the previous procedure
        //     UpdatePageDescs to here in order to fix bug #117882#
        rtfSections::myrDummyIter aDEnd = maSegments.maDummyPageNos.rend();
        for (rtfSections::myrDummyIter aI = maSegments.maDummyPageNos.rbegin(); aI != aDEnd; ++aI)
            pDoc->DelPageDesc(*aI);


        if( aFlyArr.Count() )
            SetFlysInDoc();

        // jetzt noch den letzten ueberfluessigen Absatz loeschen
        SwPosition* pPos = pPam->GetPoint();
        if( !pPos->nContent.GetIndex() )
        {
            SwTxtNode* pAktNd;
            ULONG nNodeIdx = pPos->nNode.GetIndex();
            if( IsNewDoc() )
            {
                SwNode* pTmp = pDoc->GetNodes()[ nNodeIdx -1 ];
                if( pTmp->IsCntntNode() && !pTmp->FindTableNode() )
                {
                    // --> FME 2006-02-15 #131200# Do not delete the paragraph
                    // if it has anchored objects:
                    bool bAnchoredObjs = false;
                    const SwSpzFrmFmts* pFrmFmts = pDoc->GetSpzFrmFmts();
                    if ( pFrmFmts && pFrmFmts->Count() )
                    {
                        for ( USHORT nI = pFrmFmts->Count(); nI; --nI )
                        {
                            const SwFmtAnchor & rAnchor = (*pFrmFmts)[ nI - 1 ]->GetAnchor();
                            if ( FLY_AT_CNTNT == rAnchor.GetAnchorId() ||
                                 FLY_AUTO_CNTNT == rAnchor.GetAnchorId() )
                            {
                                const SwPosition * pObjPos = rAnchor.GetCntntAnchor();
                                if ( pObjPos && nNodeIdx == pObjPos->nNode.GetIndex() )
                                {
                                    bAnchoredObjs = true;
                                    break;
                                }
                            }
                        }
                    }
                    // <--

                    if ( !bAnchoredObjs )
                        DelLastNode();
                }
            }
            else if (0 != (pAktNd = pDoc->GetNodes()[nNodeIdx]->GetTxtNode()))
            {
                if( pAktNd->CanJoinNext( &pPos->nNode ))
                {
                    SwTxtNode* pNextNd = pPos->nNode.GetNode().GetTxtNode();
                    pPos->nContent.Assign( pNextNd, 0 );
                    pPam->SetMark(); pPam->DeleteMark();
                    pNextNd->JoinPrev();
                }
                else if( !pAktNd->GetTxt().Len() &&
                        pAktNd->StartOfSectionIndex()+2 <
                        pAktNd->EndOfSectionIndex() )
                {
                    pPos->nContent.Assign( 0, 0 );
                    pPam->SetMark(); pPam->DeleteMark();
                    pDoc->GetNodes().Delete( pPos->nNode, 1 );
                    pPam->Move( fnMoveBackward );
                }
            }
        }
        // nun noch das SplitNode vom Ende aufheben
        else if( !IsNewDoc() )
        {
            if( pPos->nContent.GetIndex() )     // dann gabs am Ende kein \par,
                pPam->Move( fnMoveForward, fnGoNode );  // als zum naechsten Node
            SwTxtNode* pTxtNode = pPos->nNode.GetNode().GetTxtNode();
            SwNodeIndex aPrvIdx( pPos->nNode );
            if( pTxtNode && pTxtNode->CanJoinPrev( &aPrvIdx ) &&
                *pSttNdIdx <= aPrvIdx )
            {
                // eigentlich muss hier ein JoinNext erfolgen, aber alle Cursor
                // usw. sind im pTxtNode angemeldet, so dass der bestehen
                // bleiben MUSS.

                // Absatz in Zeichen-Attribute umwandeln, aus dem Prev die
                // Absatzattribute und die Vorlage uebernehmen!
                SwTxtNode* pPrev = aPrvIdx.GetNode().GetTxtNode();
                pTxtNode->ChgFmtColl( pPrev->GetTxtColl() );
                pTxtNode->FmtToTxtAttr( pPrev );
                pTxtNode->ResetAllAttr();

                if( pPrev->HasSwAttrSet() )
                    pTxtNode->SetAttr( *pPrev->GetpSwAttrSet() );

                if( &pPam->GetBound(TRUE).nNode.GetNode() == pPrev )
                    pPam->GetBound(TRUE).nContent.Assign( pTxtNode, 0 );
                if( &pPam->GetBound(FALSE).nNode.GetNode() == pPrev )
                    pPam->GetBound(FALSE).nContent.Assign( pTxtNode, 0 );

                pTxtNode->JoinPrev();
            }
        }
    }
    delete pSttNdIdx, pSttNdIdx = 0;
    delete pRegionEndIdx, pRegionEndIdx = 0;
    RemoveUnusedNumRules();

    pDoc->SetUpdateExpFldStat(true);
    pDoc->SetInitDBFields(true);

    // Laufbalken bei asynchronen Call nicht einschalten !!!
    ::EndProgress( pDoc->GetDocShell() );
}

bool rtfSections::SetCols(SwFrmFmt &rFmt, const rtfSection &rSection,
    USHORT nNettoWidth)
{
    //sprmSCcolumns - Anzahl der Spalten - 1
    USHORT nCols = static_cast< USHORT >(rSection.NoCols());

    if (nCols < 2)
        return false;                   // keine oder bloedsinnige Spalten

    SwFmtCol aCol;                      // Erzeuge SwFmtCol

    //sprmSDxaColumns   - Default-Abstand 1.25 cm
    USHORT nColSpace = static_cast< USHORT >(rSection.StandardColSeperation());

    aCol.Init( nCols, nColSpace, nNettoWidth );

    // not SFEvenlySpaced
    if (rSection.maPageInfo.maColumns.size())
    {
        aCol._SetOrtho(false);
        USHORT nWishWidth = 0, nHalfPrev = 0;
        for(USHORT n=0, i=0; n < rSection.maPageInfo.maColumns.size() && i < nCols; n += 2, ++i )
        {
            SwColumn* pCol = aCol.GetColumns()[ i ];
            pCol->SetLeft( nHalfPrev );
            USHORT nSp = static_cast< USHORT >(rSection.maPageInfo.maColumns[ n+1 ]);
            nHalfPrev = nSp / 2;
            pCol->SetRight( nSp - nHalfPrev );
            pCol->SetWishWidth( static_cast< USHORT >(rSection.maPageInfo.maColumns[ n ]) +
                pCol->GetLeft() + pCol->GetRight());
            nWishWidth = nWishWidth + pCol->GetWishWidth();
        }
        aCol.SetWishWidth( nWishWidth );
    }

    rFmt.SetFmtAttr(aCol);
    return true;
}

void rtfSections::SetPage(SwPageDesc &rInPageDesc, SwFrmFmt &rFmt,
    const rtfSection &rSection, bool bIgnoreCols)
{
    // 1. Orientierung
    rInPageDesc.SetLandscape(rSection.IsLandScape());

    // 2. Papiergroesse
    SwFmtFrmSize aSz(rFmt.GetFrmSize());
    aSz.SetWidth(rSection.GetPageWidth());
    aSz.SetHeight(rSection.GetPageHeight());
    rFmt.SetFmtAttr(aSz);

    rFmt.SetFmtAttr(
        SvxLRSpaceItem(rSection.GetPageLeft(), rSection.GetPageRight(), 0, 0, RES_LR_SPACE));

    if (!bIgnoreCols)
    {
        SetCols(rFmt, rSection, static_cast< USHORT >(rSection.GetPageWidth() -
            rSection.GetPageLeft() - rSection.GetPageRight()));
    }

    rFmt.SetFmtAttr(rSection.maPageInfo.maBox);
}

bool HasHeader(const SwFrmFmt &rFmt)
{
    const SfxPoolItem *pHd;
    if (SFX_ITEM_SET == rFmt.GetItemState(RES_HEADER, false, &pHd))
        return ((const SwFmtHeader *)(pHd))->IsActive();
    return false;
}

bool HasFooter(const SwFrmFmt &rFmt)
{
    const SfxPoolItem *pFt;
    if (SFX_ITEM_SET == rFmt.GetItemState(RES_FOOTER, false, &pFt))
        return ((const SwFmtFooter *)(pFt))->IsActive();
    return false;
}

void rtfSections::GetPageULData(const rtfSection &rSection, bool bFirst,
    rtfSections::wwULSpaceData& rData)
{
    short nWWUp     = static_cast< short >(rSection.maPageInfo.mnMargtsxn);
    short nWWLo     = static_cast< short >(rSection.maPageInfo.mnMargbsxn);
    short nWWHTop   = static_cast< short >(rSection.maPageInfo.mnHeadery);
    short nWWFBot   = static_cast< short >(rSection.maPageInfo.mnFootery);

    if (bFirst)
    {
        if (
            rSection.mpTitlePage && HasHeader(rSection.mpTitlePage->GetMaster())
           )
        {
            rData.bHasHeader = true;
        }
    }
    else
    {
        if (rSection.mpPage &&
               (
               HasHeader(rSection.mpPage->GetMaster())
               || HasHeader(rSection.mpPage->GetLeft())
               )
           )
        {
            rData.bHasHeader = true;
        }
    }

    if( rData.bHasHeader )
    {
        rData.nSwUp  = nWWHTop;             // Header -> umrechnen, see ww8par6.cxx

        if ( nWWUp > 0 && nWWUp >= nWWHTop )
            rData.nSwHLo = nWWUp - nWWHTop;
        else
            rData.nSwHLo = 0;

        if (rData.nSwHLo < cMinHdFtHeight)
            rData.nSwHLo = cMinHdFtHeight;
    }
    else // kein Header -> Up einfach uebernehmen
        rData.nSwUp = Abs(nWWUp);

    if (bFirst)
    {
        if (
                rSection.mpTitlePage &&
                HasFooter(rSection.mpTitlePage->GetMaster())
           )
        {
            rData.bHasFooter = true;
        }
    }
    else
    {
        if (rSection.mpPage &&
           (
               HasFooter(rSection.mpPage->GetMaster())
               || HasFooter(rSection.mpPage->GetLeft())
           )
           )
        {
            rData.bHasFooter = true;
        }
    }

    if( rData.bHasFooter )
    {
        rData.nSwLo = nWWFBot;              // Footer -> Umrechnen
        if ( nWWLo > 0 && nWWLo >= nWWFBot )
            rData.nSwFUp = nWWLo - nWWFBot;
        else
            rData.nSwFUp = 0;

        if (rData.nSwFUp < cMinHdFtHeight)
            rData.nSwFUp = cMinHdFtHeight;
    }
    else // kein Footer -> Lo einfach uebernehmen
        rData.nSwLo = Abs(nWWLo);
}

void rtfSections::SetPageULSpaceItems(SwFrmFmt &rFmt,
    rtfSections::wwULSpaceData& rData)
{
    if (rData.bHasHeader)               // ... und Header-Lower setzen
    {
        //Kopfzeilenhoehe minimal sezten
        if (SwFrmFmt* pHdFmt = (SwFrmFmt*)rFmt.GetHeader().GetHeaderFmt())
        {
            pHdFmt->SetFmtAttr(SwFmtFrmSize(ATT_MIN_SIZE, 0, rData.nSwHLo));
            SvxULSpaceItem aHdUL(pHdFmt->GetULSpace());
            aHdUL.SetLower( rData.nSwHLo - cMinHdFtHeight );
            pHdFmt->SetFmtAttr(aHdUL);
            pHdFmt->SetFmtAttr(SwHeaderAndFooterEatSpacingItem(
                RES_HEADER_FOOTER_EAT_SPACING, true));
        }
    }

    if (rData.bHasFooter)               // ... und Footer-Upper setzen
    {
        if (SwFrmFmt* pFtFmt = (SwFrmFmt*)rFmt.GetFooter().GetFooterFmt())
        {
            pFtFmt->SetFmtAttr(SwFmtFrmSize(ATT_MIN_SIZE, 0, rData.nSwFUp));
            SvxULSpaceItem aFtUL(pFtFmt->GetULSpace());
            aFtUL.SetUpper( rData.nSwFUp - cMinHdFtHeight );
            pFtFmt->SetFmtAttr(aFtUL);
            pFtFmt->SetFmtAttr(SwHeaderAndFooterEatSpacingItem(
                RES_HEADER_FOOTER_EAT_SPACING, true));
        }
    }

    SvxULSpaceItem aUL(rData.nSwUp, rData.nSwLo, RES_UL_SPACE ); // Page-UL setzen
    rFmt.SetFmtAttr(aUL);
}

void rtfSections::SetSegmentToPageDesc(const rtfSection &rSection,
    bool bTitlePage, bool bIgnoreCols)
{
    SwPageDesc &rPage = bTitlePage ? *rSection.mpTitlePage : *rSection.mpPage;

//    SetNumberingType(rSection, rPage);

    SwFrmFmt &rFmt = rPage.GetMaster();
//    mrReader.SetDocumentGrid(rFmt, rSection);

    wwULSpaceData aULData;
    GetPageULData(rSection, bTitlePage, aULData);
    SetPageULSpaceItems(rFmt, aULData);

    SetPage(rPage, rFmt, rSection, bIgnoreCols);

    UseOnPage ePage = rPage.ReadUseOn();
    if(ePage & nsUseOnPage::PD_ALL)
    {
        SwFrmFmt &rFmtLeft = rPage.GetLeft();
        SetPageULSpaceItems(rFmtLeft, aULData);
        SetPage(rPage, rFmtLeft, rSection, bIgnoreCols);
    }

}

void rtfSections::CopyFrom(const SwPageDesc &rFrom, SwPageDesc &rDest)
{
    UseOnPage ePage = rFrom.ReadUseOn();
    rDest.WriteUseOn(ePage);

    mrReader.pDoc->CopyHeader(rFrom.GetMaster(), rDest.GetMaster());
    SwFrmFmt &rDestFmt = rDest.GetMaster();
    rDestFmt.SetFmtAttr(rFrom.GetMaster().GetHeader());
    mrReader.pDoc->CopyHeader(rFrom.GetLeft(), rDest.GetLeft());
    mrReader.pDoc->CopyFooter(rFrom.GetMaster(), rDest.GetMaster());
    mrReader.pDoc->CopyFooter(rFrom.GetLeft(), rDest.GetLeft());
}

void rtfSections::MoveFrom(SwPageDesc &rFrom, SwPageDesc &rDest)
{
    UseOnPage ePage = rFrom.ReadUseOn();
    rDest.WriteUseOn(ePage);

    SwFrmFmt &rDestMaster = rDest.GetMaster();
    SwFrmFmt &rFromMaster = rFrom.GetMaster();
    rDestMaster.SetFmtAttr(rFromMaster.GetHeader());
    rDestMaster.SetFmtAttr(rFromMaster.GetFooter());
    //rFromMaster.SetAttr(SwFmtHeader()); //$flr uncommented due to bug fix #117882#
    //rFromMaster.SetAttr(SwFmtFooter()); //$flr uncommented due to bug fix #117882#

    SwFrmFmt &rDestLeft = rDest.GetLeft();
    SwFrmFmt &rFromLeft = rFrom.GetLeft();
    rDestLeft.SetFmtAttr(rFromLeft.GetHeader());
    rDestLeft.SetFmtAttr(rFromLeft.GetFooter());
    //rFromLeft.SetAttr(SwFmtHeader()); //$flr uncommented due to bug fix #117882#
    //rFromLeft.SetAttr(SwFmtFooter()); //$flr uncommented due to bug fix #117882#
}

void rtfSections::SetHdFt(rtfSection &rSection)
{
    ASSERT(rSection.mpPage, "makes no sense to call without a main page");
    if (rSection.mpPage && rSection.maPageInfo.mpPageHdFt)
    {
        if (rSection.maPageInfo.mbPageHdFtUsed)
        {
            MoveFrom(*rSection.maPageInfo.mpPageHdFt, *rSection.mpPage);
            rSection.maPageInfo.mbPageHdFtUsed = false;
            rSection.maPageInfo.mpPageHdFt = rSection.mpPage;
        }
        else
            CopyFrom(*rSection.maPageInfo.mpPageHdFt, *rSection.mpPage);
    }

    if (rSection.mpTitlePage && rSection.maPageInfo.mpTitlePageHdFt)
    {
        if (rSection.maPageInfo.mbTitlePageHdFtUsed)
        {
            MoveFrom(*rSection.maPageInfo.mpTitlePageHdFt,
                    *rSection.mpTitlePage);
            rSection.maPageInfo.mbTitlePageHdFtUsed = false;
            rSection.maPageInfo.mpTitlePageHdFt = rSection.mpTitlePage;
        }
        else
        {
            CopyFrom(*rSection.maPageInfo.mpTitlePageHdFt,
                    *rSection.mpTitlePage);
        }
    }
}

SwSectionFmt *rtfSections::InsertSection(SwPaM& rMyPaM, rtfSection &rSection)
{
    SwSection aSection(CONTENT_SECTION, mrReader.pDoc->GetUniqueSectionName());

    SfxItemSet aSet( mrReader.pDoc->GetAttrPool(), aFrmFmtSetRange );

    sal_uInt8 nRTLPgn = maSegments.empty() ? 0 : maSegments.back().IsBiDi();
    aSet.Put(SvxFrameDirectionItem(
        nRTLPgn ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR));

    rSection.mpSection = mrReader.pDoc->Insert( rMyPaM, aSection, &aSet );
    ASSERT(rSection.mpSection, "section not inserted!");
    if (!rSection.mpSection)
        return 0;

    SwPageDesc *pPage = 0;
    mySegrIter aEnd = maSegments.rend();
    for (mySegrIter aIter = maSegments.rbegin(); aIter != aEnd; ++aIter)
    {
        pPage = aIter->mpPage;
        if (pPage)
            break;
    }

    ASSERT(pPage, "no page outside this section!");

    if (!pPage)
        pPage = &mrReader.pDoc->_GetPageDesc(0);

    if (!pPage)
        return 0;

    SwFrmFmt& rFmt = pPage->GetMaster();
    const SwFmtFrmSize&   rSz = rFmt.GetFrmSize();
    const SvxLRSpaceItem& rLR = rFmt.GetLRSpace();
    SwTwips nWidth = rSz.GetWidth();
    long nLeft  = rLR.GetTxtLeft();
    long nRight = rLR.GetRight();

    SwSectionFmt *pFmt = rSection.mpSection->GetFmt();
    ASSERT(pFmt, "impossible");
    if (!pFmt)
        return 0;
    SetCols(*pFmt, rSection, (USHORT)(nWidth - nLeft - nRight) );

    return pFmt;
}

void rtfSections::InsertSegments(bool bNewDoc)
{
    sal_uInt16 nDesc(0);
    mySegIter aEnd = maSegments.end();
    mySegIter aStart = maSegments.begin();
    for (mySegIter aIter = aStart; aIter != aEnd; ++aIter)
    {
        mySegIter aNext = aIter+1;

        bool bInsertSection = aIter != aStart ? aIter->IsContinous() : false;

        if (!bInsertSection)
        {
            /*
             If a cont section follow this section then we won't be
             creating a page desc with 2+ cols as we cannot host a one
             col section in a 2+ col pagedesc and make it look like
             word. But if the current section actually has columns then
             we are forced to insert a section here as well as a page
             descriptor.
            */

            /*
             Note for the future:
             If we want to import "protected sections" the here is
             where we would also test for that and force a section
             insertion if that was true.
            */
            bool bIgnoreCols = false;
            if (aNext != aEnd && aNext->IsContinous())
            {
                bIgnoreCols = true;
                if (aIter->NoCols() > 1)
                    bInsertSection = true;
            }

            if (aIter->HasTitlePage())
            {
                if (bNewDoc && aIter == aStart)
                {
                    aIter->mpTitlePage =
                        mrReader.pDoc->GetPageDescFromPool(RES_POOLPAGE_FIRST);
                }
                else
                {
                    USHORT nPos = mrReader.pDoc->MakePageDesc(
                        ViewShell::GetShellRes()->GetPageDescName(nDesc)
                        , 0, false);
                    aIter->mpTitlePage = &mrReader.pDoc->_GetPageDesc(nPos);
                }
                ASSERT(aIter->mpTitlePage, "no page!");
                if (!aIter->mpTitlePage)
                    continue;

                SetSegmentToPageDesc(*aIter, true, bIgnoreCols);
            }

            if (!bNewDoc && aIter == aStart)
                continue;
            else if (bNewDoc && aIter == aStart)
            {
                aIter->mpPage =
                    mrReader.pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD);
            }
            else
            {
                USHORT nPos = mrReader.pDoc->MakePageDesc(
                    ViewShell::GetShellRes()->GetPageDescName(nDesc,
                        false, aIter->HasTitlePage()),
                        aIter->mpTitlePage, false);
                aIter->mpPage = &mrReader.pDoc->_GetPageDesc(nPos);
            }
            ASSERT(aIter->mpPage, "no page!");
            if (!aIter->mpPage)
                continue;

            SetHdFt(*aIter);

            if (aIter->mpTitlePage)
                SetSegmentToPageDesc(*aIter, true, bIgnoreCols);
            SetSegmentToPageDesc(*aIter, false, bIgnoreCols);

            SwFmtPageDesc aPgDesc(aIter->HasTitlePage() ?
                 aIter->mpTitlePage : aIter->mpPage);

            if (aIter->mpTitlePage)
                aIter->mpTitlePage->SetFollow(aIter->mpPage);

            if (aIter->PageRestartNo() ||
                ((aIter == aStart) && aIter->PageStartAt() != 1))
                aPgDesc.SetNumOffset( static_cast< USHORT >(aIter->PageStartAt()) );

            /*
            If its a table here, apply the pagebreak to the table
            properties, otherwise we add it to the para at this
            position
            */
            if (aIter->maStart.GetNode().IsTableNode())
            {
                SwTable& rTable =
                    aIter->maStart.GetNode().GetTableNode()->GetTable();
                SwFrmFmt* pApply = rTable.GetFrmFmt();
                ASSERT(pApply, "impossible");
                if (pApply)
                    pApply->SetFmtAttr(aPgDesc);
            }
            else
            {
                SwPosition aPamStart(aIter->maStart);
                aPamStart.nContent.Assign(
                    aIter->maStart.GetNode().GetCntntNode(), 0);
                SwPaM aPage(aPamStart);

                mrReader.pDoc->Insert(aPage, aPgDesc, 0);
            }
            ++nDesc;
        }

        SwTxtNode* pTxtNd = 0;
        if (bInsertSection)
        {
            SwPaM aSectPaM(*mrReader.pPam);
            SwNodeIndex aAnchor(aSectPaM.GetPoint()->nNode);
            if (aNext != aEnd)
            {
                aAnchor = aNext->maStart;
                aSectPaM.GetPoint()->nNode = aAnchor;
                aSectPaM.GetPoint()->nContent.Assign(
                    aNext->maStart.GetNode().GetCntntNode(), 0);
                aSectPaM.Move(fnMoveBackward);
            }

            const SwPosition* pPos  = aSectPaM.GetPoint();
            const SwTxtNode* pSttNd =
                mrReader.pDoc->GetNodes()[ pPos->nNode ]->GetTxtNode();
            const SwTableNode* pTableNd = pSttNd ? pSttNd->FindTableNode() : 0;
            if (pTableNd)
            {
                pTxtNd =
                    mrReader.pDoc->GetNodes().MakeTxtNode(aAnchor,
                    mrReader.pDoc->GetTxtCollFromPool( RES_POOLCOLL_TEXT ));

                aSectPaM.GetPoint()->nNode = SwNodeIndex(*pTxtNd);
                aSectPaM.GetPoint()->nContent.Assign(
                    aSectPaM.GetCntntNode(), 0);
            }

            aSectPaM.SetMark();

            aSectPaM.GetPoint()->nNode = aIter->maStart;
            aSectPaM.GetPoint()->nContent.Assign(
                aSectPaM.GetCntntNode(), 0);

            SwSectionFmt *pRet = InsertSection(aSectPaM, *aIter);
            //The last section if continous is always unbalanced
            if (aNext == aEnd && pRet)
                pRet->SetFmtAttr(SwFmtNoBalancedColumns(true));
        }

        if (pTxtNd)
        {
            SwNodeIndex aIdx(*pTxtNd);
            SwPosition aPos(aIdx);
            SwPaM aTest(aPos);
            mrReader.pDoc->DelFullPara(aTest);
            pTxtNd = 0;
        }
    }
}

#endif	// SUPD != 310

namespace sw{
    namespace util{
    
InsertedTableClient::InsertedTableClient(SwTableNode & rNode)
{
    rNode.Add(this);
}

SwTableNode * InsertedTableClient::GetTableNode()
{
    return dynamic_cast<SwTableNode *> (pRegisteredIn);
}
    
InsertedTablesManager::InsertedTablesManager(const SwDoc &rDoc)
    : mbHasRoot(rDoc.GetRootFrm())
{
}

void InsertedTablesManager::DelAndMakeTblFrms()
{
    if (!mbHasRoot)
        return;
    TblMapIter aEnd = maTables.end();
    for (TblMapIter aIter = maTables.begin(); aIter != aEnd; ++aIter)
    {
        // exitiert schon ein Layout, dann muss an dieser Tabelle die BoxFrames
        // neu erzeugt
        SwTableNode *pTable = aIter->first->GetTableNode();
        ASSERT(pTable, "Why no expected table");
        if (pTable)
        {
            SwFrmFmt * pFrmFmt = pTable->GetTable().GetFrmFmt();
            
            if (pFrmFmt != NULL)
            {
                SwNodeIndex *pIndex = aIter->second;
                pTable->DelFrms();
                pTable->MakeFrms(pIndex);
            }
        }
    }
}

void InsertedTablesManager::InsertTable(SwTableNode &rTableNode, SwPaM &rPaM)
{
    if (!mbHasRoot)
        return;
    //Associate this tablenode with this after position, replace an //old
    //node association if necessary
    
    InsertedTableClient * pClient = new InsertedTableClient(rTableNode);

#ifdef USE_JAVA
    // Fix bug 3400 by detecting when the value for an existing key is changing
    TblMapIter it = maTables.find(pClient);
    if (it != maTables.end() && it->second != &(rPaM.GetPoint()->nNode))
    {
        mbHasRoot = false;
        maTables.clear();
        delete pClient;
        return;
    }
#endif  // USE_JAVA
    
    maTables.insert(TblMap::value_type(pClient, &(rPaM.GetPoint()->nNode)));
}
}
}

#if SUPD != 310

SwRTFParser::~SwRTFParser()
{
    maInsertedTables.DelAndMakeTblFrms();
    mpRedlineStack->closeall(*pPam->GetPoint());
    delete mpRedlineStack;

    delete pSttNdIdx;
    delete pRegionEndIdx;
    delete pPam;
    delete pRelNumRule;

    if (aFlyArr.Count())
        aFlyArr.DeleteAndDestroy( 0, aFlyArr.Count() );

    if (pGrfAttrSet)
        DELETEZ( pGrfAttrSet );

    DELETEZ( pAuthorInfos );
}

//i19718
void SwRTFParser::ReadShpRslt()
{
    int nToken;
    while ('}' != (nToken = GetNextToken() ) && IsParserWorking())
    {
        switch(nToken)
        {
            case RTF_PAR:
                break;
            default:
                NextToken(nToken);
                break;
        }
    }
    SkipToken(-1);
}

void SwRTFParser::ReadShpTxt(String& s)
{
  int nToken;
  int level=1;
  s.AppendAscii("{\\rtf");
  while (level>0 && IsParserWorking())
    {
      nToken = GetNextToken();
      switch(nToken)
    {
    case RTF_SN:
    case RTF_SV:
      SkipGroup();
      break;
    case RTF_TEXTTOKEN:
      s.Append(aToken);
      break;
    case '{':
      level++;
      s.Append(String::CreateFromAscii("{"));
      break;
    case '}':
      level--;
      s.Append(String::CreateFromAscii("}"));
      break;
    default:
      s.Append(aToken);
      if (bTokenHasValue) {
        s.Append(String::CreateFromInt64(nTokenValue));
      }
      s.Append(String::CreateFromAscii(" "));
      break;
    }
    }
  SkipToken(-1);
}

/*
 * #127429#. Very basic support for the "Buchhalternase".
 */
void SwRTFParser::ReadDrawingObject()
{
    int nToken;
    int level;
    level=1;
    Rectangle aRect;
    ::basegfx::B2DPolygon aPolygon;
    ::basegfx::B2DPoint aPoint;
    bool bPolygonActive(false);

    while (level>0 && IsParserWorking())
    {
        nToken = GetNextToken();
        switch(nToken)
        {
            case '}':
                level--;
                break;
            case '{':
                level++;
                break;
            case RTF_DPX:
                aRect.setX(nTokenValue);
                break;
            case RTF_DPXSIZE:
                aRect.setWidth(nTokenValue);
                break;
            case RTF_DPY:
                aRect.setY(nTokenValue);
                break;
            case RTF_DPYSIZE:
                aRect.setHeight(nTokenValue);
                break;
            case RTF_DPPOLYCOUNT:
                bPolygonActive = true;
                break;
            case RTF_DPPTX:
                aPoint.setX(nTokenValue);
                break;
            case RTF_DPPTY:
                aPoint.setY(nTokenValue);

                if(bPolygonActive)
                {
                    aPolygon.append(aPoint);
                }

                break;
            default:
                break;
        }
    }
    SkipToken(-1);
    /*
    const Point aPointC1( 0, 0 );
    const Point aPointC2( 100, 200 );
    const Point aPointC3( 300, 400 );
    XPolygon aPolygonC(3);
    aPolygonC[0] = aPointC1;
    aPolygonC[1] = aPointC2;
    aPolygonC[2] = aPointC3;
    */
    if(bPolygonActive && aPolygon.count())
    {
        SdrPathObj* pStroke = new SdrPathObj(OBJ_PLIN, ::basegfx::B2DPolyPolygon(aPolygon));
        SfxItemSet aFlySet(pDoc->GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1);
        SwFmtSurround aSur( SURROUND_PARALLEL );
        aSur.SetContour( false );
        aSur.SetOutside(true);
        aFlySet.Put( aSur );
        SwFmtFollowTextFlow aFollowTextFlow( FALSE );
        aFlySet.Put( aFollowTextFlow );
        /*
        sw::util::SetLayer aSetLayer(*pDoc);
        aSetLayer.SendObjectToHeaven(*pStroke);
        */
        /*
            FLY_AT_CNTNT,       //Absatzgebundener Rahmen <to paragraph>
        FLY_IN_CNTNT,       //Zeichengebundener Rahmen <as character>
        FLY_PAGE,           //Seitengebundener Rahmen <to page>
        FLY_AT_FLY,         //Rahmengebundener Rahmen ( LAYER_IMPL ) <to frame>
        FLY_AUTO_CNTNT,     //Automatisch positionierter, absatzgebundener Rahmen <to character>
        */
        SwFmtAnchor aAnchor( FLY_AT_CNTNT );
        aAnchor.SetAnchor( pPam->GetPoint() );
        aFlySet.Put( aAnchor );

        /*
        text::RelOrientation::FRAME,          // Absatz inkl. Raender
        text::RelOrientation::PRINT_AREA,     // Absatz ohne Raender
        text::RelOrientation::CHAR,       // an einem Zeichen
        text::RelOrientation::PAGE_LEFT,  // im linken Seitenrand
        text::RelOrientation::PAGE_RIGHT,   // im rechten Seitenrand
        text::RelOrientation::FRAME_LEFT,   // im linken Absatzrand
        text::RelOrientation::FRAME_RIGHT,  // im rechten Absatzrand
        text::RelOrientation::PAGE_FRAME, // Seite inkl. Raender, bei seitengeb. identisch mit text::RelOrientation::FRAME
        text::RelOrientation::PAGE_PRINT_AREA,    // Seite ohne Raender, bei seitengeb. identisch mit text::RelOrientation::PRTAREA
        // OD 11.11.2003 #i22341#
        text::RelOrientation::TEXT_LINE,  // vertical relative to top of text line, only for to-character
                        // anchored objects.


            text::HoriOrientation::NONE,      //Der Wert in nYPos gibt die RelPos direkt an.
        text::HoriOrientation::RIGHT,     //Der Rest ist fuer automatische Ausrichtung.
        text::HoriOrientation::CENTER,
        text::HoriOrientation::LEFT,
        text::HoriOrientation::INSIDE,
        text::HoriOrientation::OUTSIDE,
        text::HoriOrientation::FULL,          //Spezialwert fuer Tabellen
        text::HoriOrientation::LEFT_AND_WIDTH  //Auch fuer Tabellen
        */
        SwFmtHoriOrient aHori( 0, text::HoriOrientation::NONE, text::RelOrientation::PAGE_FRAME );
        aFlySet.Put( aHori );
        /*
        text::VertOrientation::NONE,  //Der Wert in nYPos gibt die RelPos direkt an.
        text::VertOrientation::TOP,   //Der Rest ist fuer automatische Ausrichtung.
        text::VertOrientation::CENTER,
        text::VertOrientation::BOTTOM,
        text::VertOrientation::CHAR_TOP,      //Ausrichtung _nur_ fuer Zeichengebundene Rahmen
        text::VertOrientation::CHAR_CENTER,   //wie der Name jew. sagt wird der RefPoint des Rahmens
        text::VertOrientation::CHAR_BOTTOM,   //entsprechend auf die Oberkante, Mitte oder Unterkante
        text::VertOrientation::LINE_TOP,      //der Zeile gesetzt. Der Rahmen richtet sich  dann
        text::VertOrientation::LINE_CENTER,   //entsprechend aus.
        text::VertOrientation::LINE_BOTTOM
        */
        SwFmtVertOrient aVert( 0, text::VertOrientation::NONE, text::RelOrientation::PAGE_FRAME );
        aFlySet.Put( aVert );

        pDoc->GetOrCreateDrawModel();
        SdrModel* pDrawModel  = pDoc->GetDrawModel();
        SdrPage* pDrawPg = pDrawModel->GetPage(0);
        pDrawPg->InsertObject(pStroke, 0);

        pStroke->SetSnapRect(aRect);

        /* SwFrmFmt* pRetFrmFmt = */pDoc->Insert(*pPam, *pStroke, &aFlySet, NULL);
    }
}

void SwRTFParser::InsertShpObject(SdrObject* pStroke, int _nZOrder)
{
        SfxItemSet aFlySet(pDoc->GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1);
        SwFmtSurround aSur( SURROUND_THROUGHT );
        aSur.SetContour( false );
        aSur.SetOutside(true);
        aFlySet.Put( aSur );
        SwFmtFollowTextFlow aFollowTextFlow( FALSE );
        aFlySet.Put( aFollowTextFlow );

        SwFmtAnchor aAnchor( FLY_AT_CNTNT );
        aAnchor.SetAnchor( pPam->GetPoint() );
        aFlySet.Put( aAnchor );


        SwFmtHoriOrient aHori( 0, text::HoriOrientation::NONE, text::RelOrientation::PAGE_FRAME );
        aFlySet.Put( aHori );

        SwFmtVertOrient aVert( 0, text::VertOrientation::NONE, text::RelOrientation::PAGE_FRAME );
        aFlySet.Put( aVert );

        aFlySet.Put(SvxOpaqueItem(RES_OPAQUE,false));

        pDoc->GetOrCreateDrawModel();
        SdrModel* pDrawModel  = pDoc->GetDrawModel();
        SdrPage* pDrawPg = pDrawModel->GetPage(0);
        pDrawPg->InsertObject(pStroke);
        pDrawPg->SetObjectOrdNum(pStroke->GetOrdNum(), _nZOrder);
        /* SwFrmFmt* pRetFrmFmt = */pDoc->Insert(*pPam, *pStroke, &aFlySet, NULL);
}

::basegfx::B2DPoint rotate(const ::basegfx::B2DPoint& rStart, const ::basegfx::B2DPoint& rEnd)
{
    const ::basegfx::B2DVector aVector(rStart - rEnd);
    return ::basegfx::B2DPoint(aVector.getY() + rEnd.getX(), -aVector.getX() + rEnd.getY());
}


void SwRTFParser::ReadShapeObject()
{
    int nToken;
    int level;
    level=1;
    ::basegfx::B2DPoint aPointLeftTop;
    ::basegfx::B2DPoint aPointRightBottom;
    String sn;
    sal_Int32 shapeType=-1;
    Graphic aGrf;
    bool bGrfValid=false;
    bool fFilled=true;
    Color fillColor(255, 255, 255);
    bool fLine=true;
    int lineWidth=9525/360;
    String shpTxt;
    bool bshpTxt=false;
    int txflTextFlow=0;


    while (level>0 && IsParserWorking())
    {
        nToken = GetNextToken();
        switch(nToken)
        {
            case '}':
                level--;
                break;
            case '{':
                level++;
                break;
            case RTF_SHPLEFT:
                aPointLeftTop.setX(nTokenValue);
                break;
            case RTF_SHPTOP:
                aPointLeftTop.setY(nTokenValue);
                break;
            case RTF_SHPBOTTOM:
                aPointRightBottom.setY(nTokenValue);
                break;
            case RTF_SHPRIGHT:
                aPointRightBottom.setX(nTokenValue);
                break;
            case RTF_SN:
                nToken = GetNextToken();
                ASSERT(nToken==RTF_TEXTTOKEN, "expected name");
                sn=aToken;
                break;
            case RTF_SV:
                nToken = GetNextToken();
                if (nToken==RTF_TEXTTOKEN)
                {
                    if (sn.EqualsAscii("shapeType"))
                    {
                        shapeType=aToken.ToInt32();

                    } else if (sn.EqualsAscii("fFilled"))
                    {
                        fFilled=aToken.ToInt32();

                    } else if (sn.EqualsAscii("fLine"))
                    {
                            fLine=aToken.ToInt32();
                    } else if (sn.EqualsAscii("lineWidth"))
                    {
                            lineWidth=aToken.ToInt32()/360;

                    } else if (sn.EqualsAscii("fillColor"))
                    {
                        sal_uInt32 nColor=aToken.ToInt32();
                        fillColor=Color( (sal_uInt8)nColor, (sal_uInt8)( nColor >> 8 ), (sal_uInt8)( nColor >> 16 ) );
                    }else if (sn.EqualsAscii("txflTextFlow"))
                      {
                        txflTextFlow=aToken.ToInt32();
                      }

                }
                break;
            case RTF_PICT:
                {
                        SvxRTFPictureType aPicType;
                        bGrfValid=ReadBmpData( aGrf, aPicType );
                }
                break;
            case RTF_SHPRSLT:
                if (shapeType!=1 && shapeType!=20 && shapeType!=75)
                {
                    ReadShpRslt();
                }
                break;
                     case RTF_SHPTXT:
               ReadShpTxt(shpTxt);
               bshpTxt=true;
               break;

            default:
                break;
        }
    }
    SkipToken(-1);

    switch(shapeType)
    {
        case 202: /* Text Box */
    case 1: /* Rectangle */
        {
            ::basegfx::B2DRange aRange(aPointLeftTop);
            aRange.expand(aPointRightBottom);

          if (txflTextFlow==2) {
            const ::basegfx::B2DPoint a(rotate(aRange.getMinimum(), aRange.getCenter()));
            const ::basegfx::B2DPoint b(rotate(aRange.getMaximum(), aRange.getCenter()));

            aRange.reset();
            aRange.expand(a);
            aRange.expand(b);
          }

            const Rectangle aRect(FRound(aRange.getMinX()), FRound(aRange.getMinY()), FRound(aRange.getMaxX()), FRound(aRange.getMaxY()));
            SdrRectObj* pStroke = new SdrRectObj(aRect);
            pStroke->SetSnapRect(aRect);
            pDoc->GetOrCreateDrawModel(); // create model
            InsertShpObject(pStroke, this->nZOrder++);
            SfxItemSet aSet(pStroke->GetMergedItemSet());
            if (fFilled)
            {
                aSet.Put(XFillStyleItem(XFILL_SOLID));
                aSet.Put(XFillColorItem( String(), fillColor ) );
            }
            else
            {
                aSet.Put(XFillStyleItem(XFILL_NONE));
            }
            if (!fLine) {
              aSet.Put(XLineStyleItem(XLINE_NONE));
            } else {
              aSet.Put( XLineWidthItem( lineWidth/2 ) ); // lineWidth are in 1000th mm, seems that XLineWidthItem expects 1/2 the line width
            }

            pStroke->SetMergedItemSet(aSet);
            if (bshpTxt) {
              SdrOutliner& rOutliner=pDoc->GetDrawModel()->GetDrawOutliner(pStroke);
              rOutliner.Clear();
              ByteString bs(shpTxt, RTL_TEXTENCODING_ASCII_US);
              SvMemoryStream aStream((sal_Char*)bs.GetBuffer(), bs.Len(), STREAM_READ);
              rOutliner.Read(aStream, String::CreateFromAscii(""), EE_FORMAT_RTF);
              OutlinerParaObject* pParaObject=rOutliner.CreateParaObject();
              pStroke->NbcSetOutlinerParaObject(pParaObject);
              //delete pParaObject;
              rOutliner.Clear();
            }
            if (txflTextFlow==2) {
              long nAngle = 90;
              double a = nAngle*100*nPi180;
              pStroke->Rotate(pStroke->GetCurrentBoundRect().Center(), nAngle*100, sin(a), cos(a) );

            }

        }
        break;
    case 20: /* Line */
        {
            ::basegfx::B2DPolygon aLine;
            aLine.append(aPointLeftTop);
            aLine.append(aPointRightBottom);

            SdrPathObj* pStroke = new SdrPathObj(OBJ_PLIN, ::basegfx::B2DPolyPolygon(aLine));
            //pStroke->SetSnapRect(aRect);

            InsertShpObject(pStroke, this->nZOrder++);
            SfxItemSet aSet(pStroke->GetMergedItemSet());
            if (!fLine) {
              aSet.Put(XLineStyleItem(XLINE_NONE));
            } else {
              aSet.Put( XLineWidthItem( lineWidth/2 ) ); // lineWidth are in 1000th mm, seems that XLineWidthItem expects 1/2 the line width
            }

            pStroke->SetMergedItemSet(aSet);
        }
        break;
    case 75 : /* Picture */
        if (bGrfValid) {
            ::basegfx::B2DRange aRange(aPointLeftTop);
            aRange.expand(aPointRightBottom);
            const Rectangle aRect(FRound(aRange.getMinX()), FRound(aRange.getMinY()), FRound(aRange.getMaxX()), FRound(aRange.getMaxY()));

            SdrRectObj* pStroke = new SdrGrafObj(aGrf);
            pStroke->SetSnapRect(aRect);

            InsertShpObject(pStroke, this->nZOrder++);
        }
    }
}

extern void sw3io_ConvertFromOldField( SwDoc& rDoc, USHORT& rWhich,
                                USHORT& rSubType, ULONG &rFmt,
                                USHORT nVersion );

USHORT SwRTFParser::ReadRevTbl()
{
    // rStr.Erase( 0 );
    int nNumOpenBrakets = 1, nToken;        // die erste wurde schon vorher erkannt !!
    USHORT nAuthorTableIndex = 0;

    while( nNumOpenBrakets && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case '}':   --nNumOpenBrakets;  break;
        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                else if( RTF_UNKNOWNCONTROL != GetNextToken() )
                    nToken = SkipToken( -2 );
                else
                {
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;
                }
                ++nNumOpenBrakets;
            }
            break;

        case RTF_TEXTTOKEN:
            aToken.EraseTrailingChars(';');

            USHORT nSWId = pDoc->InsertRedlineAuthor(aToken);
            // Store matchpair
            if( !pAuthorInfos )
                pAuthorInfos = new sw::util::AuthorInfos;
            sw::util::AuthorInfo* pAutorInfo = new sw::util::AuthorInfo( nAuthorTableIndex, nSWId );
            if( 0 == pAuthorInfos->Insert( pAutorInfo ) )
                delete pAutorInfo;

            aRevTbl.push_back(aToken);
            nAuthorTableIndex++;
            break;
        }
    }
    SkipToken( -1 );
    return nAuthorTableIndex;
}

// #117910# simulate words behaviour of \keepn in table rows
void fixKeepAndSplitAttributes(SwTableNode *pTableNode)
{
    ASSERT(pTableNode!=NULL, "no table node!");
    if (!pTableNode) return;
    SwDoc *pDoc=pTableNode->GetDoc();
    if (pTableNode==NULL) return;
    SwTable& rTable=pTableNode->GetTable();
    SwTableLines& rLns = rTable.GetTabLines();
    USHORT nLines=rLns.Count();
    if (nLines==0) return;
    // get first paragaph in left down-most box
    SwTableLine* pLastLine = rLns[ nLines-1 ];
    SwTableBox* pBox = pLastLine->GetTabBoxes()[ 0 ];
    ULONG iFirstParagraph=pBox->GetSttIdx()+1;
    SwTxtNode *pTxtNode=(SwTxtNode *)pDoc->GetNodes()[iFirstParagraph];
    SwFrmFmt* pFmt=rTable.GetFrmFmt();

    SwFmtLayoutSplit *pTableSplit=(SwFmtLayoutSplit *)pFmt->GetAttrSet().GetItem(RES_LAYOUT_SPLIT);
    BOOL isTableKeep = pTableSplit!=NULL && !pTableSplit->GetValue();
    SvxFmtKeepItem *pTableKeep=(SvxFmtKeepItem *)pFmt->GetAttrSet().GetItem(RES_KEEP);
    BOOL isTableKeepNext = pTableKeep!=NULL && pTableKeep->GetValue();
    SvxFmtKeepItem *pKeepNext = (SvxFmtKeepItem *)pTxtNode->GetSwAttrSet().GetItem(RES_KEEP);

    if (isTableKeepNext)
    {
        if (nLines>2 && !isTableKeep)
            { // split
                SwTableLine* pSplitLine = rLns[ nLines-2 ];
                SwTableBox* pSplitBox = pSplitLine->GetTabBoxes()[ 0 ];
                SwNodeIndex aSplitIdx( *pSplitBox->GetSttNd() );
                pDoc->SplitTable( aSplitIdx, HEADLINE_NONE, !isTableKeep );
                SwTable& rSplitTable=aSplitIdx.GetNode().FindTableNode()->GetTable();
                aSplitIdx-=2;
                pDoc->GetNodes().Delete(aSplitIdx);
                pFmt=rSplitTable.GetFrmFmt();
                pFmt->ResetFmtAttr(RES_PAGEDESC);
            }
        // set keep=1(i.e. split=0) attribut
        SwFmtLayoutSplit aSplit(0);
        SwAttrSet aNewSet(pFmt->GetAttrSet());
        aNewSet.Put(aSplit);
        pFmt->SetFmtAttr(aNewSet);
    }
    else // !isTableKeepNext
    {
        if (isTableKeep)
        {
            SwNodeIndex aTmpIdx( *pBox->GetSttNd() );
            pDoc->SplitTable( aTmpIdx, HEADLINE_NONE, FALSE );
            SwTable& rSplitTable=aTmpIdx.GetNode().FindTableNode()->GetTable();
            aTmpIdx-=2;
            pDoc->GetNodes().Delete(aTmpIdx);
            pFmt=rSplitTable.GetFrmFmt();
            pFmt->ResetFmtAttr(RES_PAGEDESC);
        }
        // set keep=0(i.e. split=1) attribut
        SwFmtLayoutSplit aSplit(1);
        SwAttrSet aNewSet(pFmt->GetAttrSet());
        aNewSet.Put(aSplit);
        pFmt->SetFmtAttr(aNewSet);
    }
    // move keepnext attribtue from last paragraph to table
    if (pKeepNext!=NULL)
    {
        SvxFmtKeepItem aNewKeepItem(pKeepNext->GetValue(), RES_KEEP);
        SwAttrSet aNewSet(pFmt->GetAttrSet());
        aNewSet.Put(aNewKeepItem);
        pFmt->SetFmtAttr(aNewSet);
    }
}

void SwRTFParser::NextToken( int nToken )
{
    USHORT eDateFmt;

    switch( nToken )
    {
    case RTF_FOOTNOTE:
        //We can only insert a footnote if we're not inside a footnote. e.g.
        //#i7713#
        if (!mbIsFootnote)
        {
            ReadHeaderFooter( nToken );
            SkipToken( -1 );        // Klammer wieder zurueck
        }
        break;
    case RTF_SWG_PRTDATA:
        ReadPrtData();
        break;
    case RTF_XE:
        ReadXEField();
        break;
    case RTF_FIELD:
        ReadField();
        break;
    case RTF_SHPRSLT:
        ReadShpRslt();
        break;
    case RTF_DO:
        ReadDrawingObject();
        break;
    case RTF_SHP:
        ReadShapeObject();
        break;
    case RTF_SHPPICT:
    case RTF_PICT:
        ReadBitmapData();
        break;
#ifdef READ_OLE_OBJECT
    case RTF_OBJECT:
        ReadOLEData();
        break;
#endif
    case RTF_TROWD:                 ReadTable( nToken );        break;
    case RTF_PGDSCTBL:
        if( !IsNewDoc() )
            SkipPageDescTbl();
        else
            ReadPageDescTbl();
        break;
    case RTF_LISTTABLE:             ReadListTable();            break;
    case RTF_LISTOVERRIDETABLE:     ReadListOverrideTable();    break;

    case RTF_LISTTEXT:
        GetAttrSet().Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, 0 ));
        SkipGroup();
        break;

    case RTF_PN:
        if( bNewNumList )
            SkipGroup();
        else
        {
            bStyleTabValid = TRUE;
            if (SwNumRule* pRule = ReadNumSecLevel( nToken ))
            {
                GetAttrSet().Put( SwNumRuleItem( pRule->GetName() ));

                if( SFX_ITEM_SET != GetAttrSet().GetItemState( FN_PARAM_NUM_LEVEL, FALSE ))
                    GetAttrSet().Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, 0 ));
            }
        }
        break;


    case RTF_BKMKSTART:
        if(RTF_TEXTTOKEN == GetNextToken())
            mpBookmarkStart = new BookmarkPosition(*pPam);
        else
            SkipToken(-1);

        SkipGroup();
        break;

    case RTF_BKMKEND:
        if(RTF_TEXTTOKEN == GetNextToken())
        {
            const String& sBookmark = aToken;
            KeyCode aEmptyKeyCode;
            if (mpBookmarkStart)
            {
                BookmarkPosition aBookmarkEnd(*pPam);
                SwPaM aBookmarkRegion(  mpBookmarkStart->maMkNode, mpBookmarkStart->mnMkCntnt,
                                        aBookmarkEnd.maMkNode, aBookmarkEnd.mnMkCntnt);
                if (*mpBookmarkStart == aBookmarkEnd)
                    aBookmarkRegion.DeleteMark();
                pDoc->makeBookmark(aBookmarkRegion, aEmptyKeyCode, sBookmark, aEmptyStr, IDocumentBookmarkAccess::BOOKMARK);
            }
            delete mpBookmarkStart, mpBookmarkStart = 0;
        }
        else
            SkipToken(-1);

        SkipGroup();
        break;


    case RTF_PNSECLVL:{
        if( bNewNumList)
            SkipGroup();
        else
            ReadNumSecLevel( nToken );
        break;
                      }

    case RTF_PNTEXT:
    case RTF_NONSHPPICT:
        SkipGroup();
        break;

    case RTF_DEFFORMAT:
    case RTF_DEFTAB:
    case RTF_DEFLANG:
        // sind zwar Dok-Controls, werden aber manchmal auch vor der
        // Font/Style/Color-Tabelle gesetzt!
        SvxRTFParser::NextToken( nToken );
        break;

    case RTF_PAGE:
        if (pTableNode==NULL) { //#117410#: A \page command within a table is ignored by Word.
            if (lcl_UsedPara(*pPam))
                InsertPara();
            CheckInsNewTblLine();
            pDoc->Insert(*pPam, SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE, RES_BREAK), 0);
        }
        break;

    case RTF_SECT:
        ReadSectControls( nToken );
        break;
    case RTF_CELL:
        // --> OD 2008-12-22 #i83368#
        mbReadCellWhileReadSwFly = bReadSwFly;
        // <--
        if (CantUseTables())
            InsertPara();
        else
        {
            // Tabelle nicht mehr vorhanden ?
            if (USHRT_MAX != nInsTblRow && !pTableNode)
                NewTblLine();               // evt. Line copieren
            GotoNextBox();
        }
        break;

    case RTF_ROW:
        bTrowdRead=false;
        if (!CantUseTables())
        {
            // aus der Line raus
            nAktBox = 0;
            pTableNode = 0;
            // noch in der Tabelle drin?
            SwNodeIndex& rIdx = pPam->GetPoint()->nNode;
            const SwTableNode* pTblNd = rIdx.GetNode().FindTableNode();
            if( pTblNd )
            {
                // search the end of this row
                const SwStartNode* pBoxStt =
                                    rIdx.GetNode().FindTableBoxStartNode();
                const SwTableBox* pBox = pTblNd->GetTable().GetTblBox(
                                                pBoxStt->GetIndex() );
                const SwTableLine* pLn = pBox->GetUpper();
                pBox = pLn->GetTabBoxes()[ pLn->GetTabBoxes().Count() - 1 ];
                rIdx = *pBox->GetSttNd()->EndOfSectionNode();
                pPam->Move( fnMoveForward, fnGoNode );
            }
            nInsTblRow = static_cast< USHORT >(GetOpenBrakets());
            SetPardTokenRead( FALSE );
            SwPaM aTmp(*pPam);
            aTmp.Move( fnMoveBackward, fnGoNode );
        }
        ::SetProgressState( rInput.Tell(), pDoc->GetDocShell() );
        break;

    case RTF_INTBL:
        if (!CantUseTables())
        {
            if( !pTableNode )           // Tabelle nicht mehr vorhanden ?
            {
                if (RTF_TROWD != GetNextToken())
                    NewTblLine();           // evt. Line copieren
                SkipToken(-1);
            }
            else
            {
                // Crsr nicht mehr in der Tabelle ?
                if( !pPam->GetNode()->FindTableNode() )
                {
                    // dann wieder in die letzte Box setzen
                    // (kann durch einlesen von Flys geschehen!)
                    pPam->GetPoint()->nNode = *pTableNode->EndOfSectionNode();
                    pPam->Move( fnMoveBackward );
                }
            }
        }
        break;

    case RTF_REVTBL:
        ReadRevTbl();
        break;

    case RTF_REVISED:
        pRedlineInsert = new SwFltRedline(nsRedlineType_t::REDLINE_INSERT, 0, DateTime(Date( 0 ), Time( 0 )));
        break;

    case RTF_DELETED:
        pRedlineDelete = new SwFltRedline(nsRedlineType_t::REDLINE_DELETE, 0, DateTime(Date( 0 ), Time( 0 )));
        break;

    case RTF_REVAUTH:
        {
            sw::util::AuthorInfo aEntry( static_cast< USHORT >(nTokenValue) );
            USHORT nPos;

            if(pRedlineInsert)
            {
                if (pAuthorInfos && pAuthorInfos->Seek_Entry(&aEntry, &nPos))
                {
                    if (const sw::util::AuthorInfo* pAuthor = pAuthorInfos->GetObject(nPos))
                    {
                        pRedlineInsert->nAutorNo = pAuthor->nOurId;
                    }
                }
            }
        }
        break;

    case RTF_REVAUTHDEL:
        {
            sw::util::AuthorInfo aEntry( static_cast< short >(nTokenValue) );
            USHORT nPos;

            if(pRedlineDelete)
            {
                if (pAuthorInfos && pAuthorInfos->Seek_Entry(&aEntry, &nPos))
                {
                    if (const sw::util::AuthorInfo* pAuthor = pAuthorInfos->GetObject(nPos))
                    {
                        pRedlineDelete->nAutorNo = pAuthor->nOurId;
                    }
                }
            }
        }
        break;

    case RTF_REVDTTM:
        pRedlineInsert->aStamp = sw::ms::DTTM2DateTime(nTokenValue);
        break;

    case RTF_REVDTTMDEL:
        pRedlineDelete->aStamp = sw::ms::DTTM2DateTime(nTokenValue);
        break;


    case RTF_FLY_INPARA:
            // \pard  und plain ueberlesen !
        if( '}' != GetNextToken() && '}' != GetNextToken() )
        {
            // Zeichengebundener Fly in Fly
            ReadHeaderFooter( nToken );
            SetPardTokenRead( FALSE );
        }
        break;

    case RTF_PGDSCNO:
        if( IsNewDoc() && bSwPageDesc &&
            USHORT(nTokenValue) < pDoc->GetPageDescCnt() )
        {
            const SwPageDesc* pPgDsc =
                &const_cast<const SwDoc *>(pDoc)
                ->GetPageDesc( USHORT(nTokenValue) );
            CheckInsNewTblLine();
            pDoc->Insert( *pPam, SwFmtPageDesc( pPgDsc ), 0);
        }
        break;

    case RTF_COLUM:
        pDoc->Insert( *pPam, SvxFmtBreakItem( SVX_BREAK_COLUMN_BEFORE, RES_BREAK ), 0);
        break;

    case RTF_DXFRTEXT:      // werden nur im Zusammenhang mit Flys ausgewertet
    case RTF_DFRMTXTX:
    case RTF_DFRMTXTY:
        break;

    case RTF_CHDATE:    eDateFmt = DF_SHORT;    goto SETCHDATEFIELD;
    case RTF_CHDATEA:   eDateFmt = DF_SSYS;     goto SETCHDATEFIELD;
    case RTF_CHDATEL:   eDateFmt = DF_LSYS;     goto SETCHDATEFIELD;
SETCHDATEFIELD:
        {
            USHORT nSubType = DATEFLD, nWhich = RES_DATEFLD;
            ULONG nFormat = eDateFmt;
            sw3io_ConvertFromOldField( *pDoc, nWhich, nSubType, nFormat, 0x0110 );

            SwDateTimeField aDateFld( (SwDateTimeFieldType*)
                                        pDoc->GetSysFldType( RES_DATETIMEFLD ), DATEFLD, nFormat);
            CheckInsNewTblLine();
            pDoc->Insert( *pPam, SwFmtFld( aDateFld ), 0);
        }
        break;

    case RTF_CHTIME:
        {
            USHORT nSubType = TIMEFLD, nWhich = RES_TIMEFLD;
            ULONG nFormat = TF_SSMM_24;
            sw3io_ConvertFromOldField( *pDoc, nWhich, nSubType, nFormat, 0x0110 );
            SwDateTimeField aTimeFld( (SwDateTimeFieldType*)
                    pDoc->GetSysFldType( RES_DATETIMEFLD ), TIMEFLD, nFormat);
            CheckInsNewTblLine();
            pDoc->Insert( *pPam, SwFmtFld( aTimeFld ), 0);
        }
        break;

    case RTF_CHPGN:
        {
            SwPageNumberField aPageFld( (SwPageNumberFieldType*)
                                    pDoc->GetSysFldType( RES_PAGENUMBERFLD ),
                                    PG_RANDOM, SVX_NUM_ARABIC );
            CheckInsNewTblLine();
            pDoc->Insert( *pPam, SwFmtFld( aPageFld), 0);
        }
        break;

    case RTF_CHFTN:
        bFootnoteAutoNum = TRUE;
        break;

    case RTF_NOFPAGES:
        if( IsNewDoc() && nTokenValue && -1 != nTokenValue )
            ((SwDocStat&)pDoc->GetDocStat()).nPage = (USHORT)nTokenValue;
        break;

    case RTF_NOFWORDS:
        if( IsNewDoc() && nTokenValue && -1 != nTokenValue )
            ((SwDocStat&)pDoc->GetDocStat()).nWord = (USHORT)nTokenValue;
        break;
    case RTF_NOFCHARS:
        if( IsNewDoc() && nTokenValue && -1 != nTokenValue )
            ((SwDocStat&)pDoc->GetDocStat()).nChar = (USHORT)nTokenValue;
        break;
    case RTF_LYTPRTMET:
        if (IsNewDoc())
            pDoc->set(IDocumentSettingAccess::USE_VIRTUAL_DEVICE, false);
        break;
    case RTF_U:
        {
            CheckInsNewTblLine();
            if( nTokenValue )
                aToken = (sal_Unicode )nTokenValue;
            pDoc->Insert( *pPam, aToken, true );
        }
        break;

    case RTF_USERPROPS:
        ReadUserProperties();       // #i28758 For now we don't support user properties
        break;

// RTF_SUBENTRYINDEX

    default:
        switch( nToken & ~(0xff | RTF_SWGDEFS) )
        {
        case RTF_DOCFMT:
            ReadDocControls( nToken );
            break;
        case RTF_SECTFMT:
            ReadSectControls( nToken );
            break;
        case RTF_APOCTL:
            if (nReadFlyDepth < 10)
            {
                nReadFlyDepth++;
                ReadFly( nToken );
                nReadFlyDepth--;
            }
            break;

        case RTF_BRDRDEF | RTF_TABLEDEF:
        case RTF_SHADINGDEF | RTF_TABLEDEF:
        case RTF_TABLEDEF:
            ReadTable( nToken );
            break;

        case RTF_INFO:
            ReadInfo();
            break;

        default:
            if( USHRT_MAX != nInsTblRow &&
                (nInsTblRow > GetOpenBrakets() || IsPardTokenRead() ))
                nInsTblRow = USHRT_MAX;

            SvxRTFParser::NextToken( nToken );
            break;
        }
    }
    if( USHRT_MAX != nInsTblRow &&
        (nInsTblRow > GetOpenBrakets() || IsPardTokenRead() ))
        nInsTblRow = USHRT_MAX;
}



void SwRTFParser::InsertText()
{
    bContainsPara = false;
    // dann fuege den String ein, ohne das Attribute am Ende
    // aufgespannt werden.
    CheckInsNewTblLine();

    if(pRedlineInsert)
        mpRedlineStack->open(*pPam->GetPoint(), *pRedlineInsert);
    if(pRedlineDelete)
        mpRedlineStack->open(*pPam->GetPoint(), *pRedlineDelete);

    pDoc->Insert( *pPam, aToken, true );

    if(pRedlineDelete)
    {
        mpRedlineStack->close(*pPam->GetPoint(), pRedlineDelete->eType);
    }

    if(pRedlineInsert)
    {
        mpRedlineStack->close(*pPam->GetPoint(), pRedlineInsert->eType);
    }


}


void SwRTFParser::InsertPara()
{
    bContainsPara = true;
    CheckInsNewTblLine();
    pDoc->AppendTxtNode(*pPam->GetPoint());

    // setze das default Style
    if( !bStyleTabValid )
        MakeStyleTab();

    SwTxtFmtColl* pColl = aTxtCollTbl.Get( 0 );
    if( !pColl )
        pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false );
    pDoc->SetTxtFmtColl( *pPam, pColl );

    ::SetProgressState( rInput.Tell(), pDoc->GetDocShell() );
}



void SwRTFParser::MovePos( int bForward )
{
    if( bForward )
        pPam->Move( fnMoveForward );
    else
        pPam->Move( fnMoveBackward );
}

int SwRTFParser::IsEndPara( SvxNodeIdx* pNd, xub_StrLen nCnt ) const
{
    SwCntntNode *pNode = pDoc->GetNodes()[pNd->GetIdx()]->GetCntntNode();
    return pNode && pNode->Len() == nCnt;
}

bool SwRTFParser::UncompressableStackEntry(const SvxRTFItemStackType &rSet)
    const
{
    /*
    #i21961#
    Seeing as CHARFMT sets all the properties of the charfmt itself, its not
    good enough to just see it as a single property from the point of
    compressing property sets. If bold and charfmt are in a child, and bold is
    in the parent, removing bold from the child will not result in the same
    thing, if the charfmt removes bold itself for example
    */
    bool bRet = false;
    if (rSet.GetAttrSet().Count())
    {

        if (SFX_ITEM_SET ==
                rSet.GetAttrSet().GetItemState(RES_TXTATR_CHARFMT, FALSE))
        {
            bRet = true;
        }
    }
    return bRet;
}

void SwRTFParser::SetEndPrevPara( SvxNodeIdx*& rpNodePos, xub_StrLen& rCntPos )
{
    SwNodeIndex aIdx( pPam->GetPoint()->nNode );
    SwCntntNode* pNode = pDoc->GetNodes().GoPrevious( &aIdx );
    if( !pNode )
    {
        ASSERT( FALSE, "keinen vorherigen ContentNode gefunden" );
    }

    rpNodePos = new SwNodeIdx( aIdx );
    rCntPos = pNode->Len();
}

void SwRTFParser::SetAttrInDoc( SvxRTFItemStackType &rSet )
{
    ULONG nSNd = rSet.GetSttNodeIdx(), nENd = rSet.GetEndNodeIdx();
    xub_StrLen nSCnt = rSet.GetSttCnt(), nECnt = rSet.GetEndCnt();

    SwPaM aPam( *pPam->GetPoint() );

#ifndef PRODUCT
    ASSERT( nSNd <= nENd, "Start groesser als Ende" );
    SwNode* pDebugNd = pDoc->GetNodes()[ nSNd ];
    ASSERT( pDebugNd->IsCntntNode(), "Start kein ContentNode" );
    pDebugNd = pDoc->GetNodes()[ nENd ];
    ASSERT( pDebugNd->IsCntntNode(), "Ende kein ContentNode" );
#endif

    SwCntntNode* pCNd = pDoc->GetNodes()[ nSNd ]->GetCntntNode();
    aPam.GetPoint()->nNode = nSNd;
    aPam.GetPoint()->nContent.Assign( pCNd, nSCnt );
    aPam.SetMark();
    if( nENd == nSNd )
        aPam.GetPoint()->nContent = nECnt;
    else
    {
        aPam.GetPoint()->nNode = nENd;
        pCNd = aPam.GetCntntNode();
        aPam.GetPoint()->nContent.Assign( pCNd, nECnt );
    }

    // setze ueber den Bereich das entsprechende Style
    if( rSet.StyleNo() )
    {
        // setze jetzt das Style
        if( !bStyleTabValid )
            MakeStyleTab();
        SwTxtFmtColl* pColl = aTxtCollTbl.Get( rSet.StyleNo() );
        if( pColl )
            pDoc->SetTxtFmtColl( aPam, pColl, false );
    }

    const SfxPoolItem* pItem;
    const SfxPoolItem* pCharFmt;
    if (rSet.GetAttrSet().Count() )
    {

        // falls eine Zeichenvorlage im Set steht, deren Attribute
        // aus dem Set loeschen. Sonst sind diese doppelt, was man ja
        // nicht will.
        if( SFX_ITEM_SET == rSet.GetAttrSet().GetItemState(
            RES_TXTATR_CHARFMT, FALSE, &pCharFmt ) &&
            ((SwFmtCharFmt*)pCharFmt)->GetCharFmt() )
        {
            const String& rName = ((SwFmtCharFmt*)pCharFmt)->GetCharFmt()->GetName();
            SvxRTFStyleType* pStyle = GetStyleTbl().First();
            do {
                if( pStyle->bIsCharFmt && pStyle->sName == rName )
                {
                    // alle Attribute, die schon vom Style definiert sind, aus dem
                    // akt. AttrSet entfernen
                    SfxItemSet &rAttrSet = rSet.GetAttrSet(),
                               &rStyleSet = pStyle->aAttrSet;
                    SfxItemIter aIter( rAttrSet );
                    USHORT nWhich = aIter.GetCurItem()->Which();
                    while( TRUE )
                    {
                        const SfxPoolItem* pI;
                        if( SFX_ITEM_SET == rStyleSet.GetItemState(
                            nWhich, FALSE, &pI ) && *pI == *aIter.GetCurItem())
                            rAttrSet.ClearItem( nWhich );       // loeschen

                        if( aIter.IsAtEnd() )
                            break;
                        nWhich = aIter.NextItem()->Which();
                    }
                    break;
                }
            } while( 0 != (pStyle = GetStyleTbl().Next()) );

            pDoc->Insert(aPam, *pCharFmt, 0);
            rSet.GetAttrSet().ClearItem(RES_TXTATR_CHARFMT);     //test hack
        }
        if (rSet.GetAttrSet().Count())
        {
            // dann setze ueber diesen Bereich die Attrbiute
            SetSwgValues(rSet.GetAttrSet());
            pDoc->Insert(aPam, rSet.GetAttrSet(), nsSetAttrMode::SETATTR_DONTCHGNUMRULE);
        }
    }

    if( SFX_ITEM_SET == rSet.GetAttrSet().GetItemState(
        FN_PARAM_NUM_LEVEL, FALSE, &pItem ))
    {
        // dann ueber den Bereich an den Nodes das NodeNum setzen
        for( ULONG n = nSNd; n <= nENd; ++n )
        {
            SwTxtNode* pTxtNd = pDoc->GetNodes()[ n ]->GetTxtNode();
            if( pTxtNd )
            {
                pTxtNd->SetAttrListLevel((BYTE) ((SfxUInt16Item*)pItem)->GetValue());
                // Update vom LR-Space abschalten?
            }
        }
    }

    if( SFX_ITEM_SET == rSet.GetAttrSet().GetItemState(
        RES_PARATR_NUMRULE, FALSE, &pItem ))
    {
        const SwNumRule* pRule = pDoc->FindNumRulePtr(
                                    ((SwNumRuleItem*)pItem)->GetValue() );
        if( pRule && ( pRule->IsContinusNum() || !bNewNumList ))
        {
            // diese Rule hat keinen Level, also muss die Einrueckung
            // erhalten bleiben!
            // dann ueber den Bereich an den Nodes das Flag zuruecksetzen
            for( ULONG n = nSNd; n <= nENd; ++n )
            {
                SwTxtNode* pTxtNd = pDoc->GetNodes()[ n ]->GetTxtNode();
                if( pTxtNd )
                {
                    // Update vom LR-Space abschalten
                    pTxtNd->SetNumLSpace( FALSE );
                }
            }
        }
    }

    bool bNoNum = true;
    if (
        (SFX_ITEM_SET == rSet.GetAttrSet().GetItemState(RES_PARATR_NUMRULE))
     || (SFX_ITEM_SET == rSet.GetAttrSet().GetItemState(FN_PARAM_NUM_LEVEL))
       )
    {
        bNoNum = false;
    }

    if (bNoNum)
    {
        for( ULONG n = nSNd; n <= nENd; ++n )
        {
            SwTxtNode* pTxtNd = pDoc->GetNodes()[ n ]->GetTxtNode();
            if( pTxtNd )
            {
                pTxtNd->SetAttr(
                    *GetDfltAttr(RES_PARATR_NUMRULE));
            }
        }
    }
}

DocPageInformation::DocPageInformation()
    : maBox( RES_BOX ),
    mnPaperw(12240), mnPaperh(15840), mnMargl(1800), mnMargr(1800),
    mnMargt(1440), mnMargb(1440), mnGutter(0), mnPgnStart(1), mbFacingp(false),
    mbLandscape(false), mbRTLdoc(false)
{
}

SectPageInformation::SectPageInformation(const DocPageInformation &rDoc)
    : maBox(rDoc.maBox), mpTitlePageHdFt(0), mpPageHdFt(0),
    mnPgwsxn(rDoc.mnPaperw), mnPghsxn(rDoc.mnPaperh), mnMarglsxn(rDoc.mnMargl),
    mnMargrsxn(rDoc.mnMargr), mnMargtsxn(rDoc.mnMargt),
    mnMargbsxn(rDoc.mnMargb), mnGutterxsn(rDoc.mnGutter), mnHeadery(720),
    mnFootery(720), mnPgnStarts(rDoc.mnPgnStart), mnCols(1), mnColsx(720),
    mnStextflow(rDoc.mbRTLdoc ? 3 : 0), mnBkc(2), mbLndscpsxn(rDoc.mbLandscape),
    mbTitlepg(false), mbFacpgsxn(rDoc.mbFacingp), mbRTLsection(rDoc.mbRTLdoc),
    mbPgnrestart(false), mbTitlePageHdFtUsed(false), mbPageHdFtUsed(false)
{
};

SectPageInformation::SectPageInformation(const SectPageInformation &rSect)
    : maColumns(rSect.maColumns), maBox(rSect.maBox),
    maNumType(rSect.maNumType), mpTitlePageHdFt(rSect.mpTitlePageHdFt),
    mpPageHdFt(rSect.mpPageHdFt), mnPgwsxn(rSect.mnPgwsxn),
    mnPghsxn(rSect.mnPghsxn), mnMarglsxn(rSect.mnMarglsxn),
    mnMargrsxn(rSect.mnMargrsxn), mnMargtsxn(rSect.mnMargtsxn),
    mnMargbsxn(rSect.mnMargbsxn), mnGutterxsn(rSect.mnGutterxsn),
    mnHeadery(rSect.mnHeadery), mnFootery(rSect.mnFootery),
    mnPgnStarts(rSect.mnPgnStarts), mnCols(rSect.mnCols),
    mnColsx(rSect.mnColsx), mnStextflow(rSect.mnStextflow), mnBkc(rSect.mnBkc),
    mbLndscpsxn(rSect.mbLndscpsxn), mbTitlepg(rSect.mbTitlepg),
    mbFacpgsxn(rSect.mbFacpgsxn), mbRTLsection(rSect.mbRTLsection),
    mbPgnrestart(rSect.mbPgnrestart),
    mbTitlePageHdFtUsed(rSect.mbTitlePageHdFtUsed),
    mbPageHdFtUsed(rSect.mbPageHdFtUsed)
{
};

rtfSection::rtfSection(const SwPosition &rPos,
    const SectPageInformation &rPageInfo)
    : maStart(rPos.nNode), maPageInfo(rPageInfo), mpSection(0), mpTitlePage(0),
    mpPage(0)
{
}

void rtfSections::push_back(const rtfSection &rSect)
{
    if (!maSegments.empty() && (maSegments.back().maStart == rSect.maStart))
        maSegments.pop_back();
    maSegments.push_back(rSect);
}

// lese alle Dokument-Controls ein
void SwRTFParser::SetPageInformationAsDefault(const DocPageInformation &rInfo)
{
    //If we are at the beginning of the document then start the document with
    //a segment with these properties. See #i14982# for this requirement
    rtfSection aSect(*pPam->GetPoint(), SectPageInformation(rInfo));
    if (maSegments.empty() || (maSegments.back().maStart == aSect.maStart))
        maSegments.push_back(aSect);

    if (!bSwPageDesc && IsNewDoc())
    {
        SwFmtFrmSize aFrmSize(ATT_FIX_SIZE, rInfo.mnPaperw, rInfo.mnPaperh);

        SvxLRSpaceItem aLR( static_cast< USHORT >(rInfo.mnMargl), static_cast< USHORT >(rInfo.mnMargr), 0, 0, RES_LR_SPACE );
        SvxULSpaceItem aUL( static_cast< USHORT >(rInfo.mnMargt), static_cast< USHORT >(rInfo.mnMargb), RES_UL_SPACE );

        UseOnPage eUseOn;
        if (rInfo.mbFacingp)
            eUseOn = UseOnPage(nsUseOnPage::PD_MIRROR | nsUseOnPage::PD_HEADERSHARE | nsUseOnPage::PD_FOOTERSHARE);
        else
            eUseOn = UseOnPage(nsUseOnPage::PD_ALL | nsUseOnPage::PD_HEADERSHARE | nsUseOnPage::PD_FOOTERSHARE);

        USHORT nPgStart = static_cast< USHORT >(rInfo.mnPgnStart);

        SvxFrameDirectionItem aFrmDir(rInfo.mbRTLdoc ?
            FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR);

        // direkt an der Standartseite drehen
        SwPageDesc& rPg = pDoc->_GetPageDesc( 0 );
        rPg.WriteUseOn( eUseOn );

        if (rInfo.mbLandscape)
            rPg.SetLandscape(true);

        SwFrmFmt &rFmt1 = rPg.GetMaster(), &rFmt2 = rPg.GetLeft();

        rFmt1.SetFmtAttr( aFrmSize );   rFmt2.SetFmtAttr( aFrmSize );
        rFmt1.SetFmtAttr( aLR );        rFmt2.SetFmtAttr( aLR );
        rFmt1.SetFmtAttr( aUL );       rFmt2.SetFmtAttr( aUL );
        rFmt1.SetFmtAttr( aFrmDir );   rFmt2.SetFmtAttr( aFrmDir );

        // StartNummer der Seiten setzen
        if (nPgStart  != 1)
        {
            SwFmtPageDesc aPgDsc( &rPg );
            aPgDsc.SetNumOffset( nPgStart );
            pDoc->Insert( *pPam, aPgDsc, 0 );
        }
    }
}

void SwRTFParser::SetBorderLine(SvxBoxItem& rBox, sal_uInt16 nLine)
{
    int bWeiter = true;
    short nLineThickness = 1;
    short nPageDistance = 0;
    BYTE nCol = 0;
    short nIdx = 0;

    int nToken = GetNextToken();
    do {
        switch( nToken )
        {
        case RTF_BRDRS:
            nIdx = 1;
            break;

        case RTF_BRDRDB:
            nIdx = 3;
            break;

        case RTF_BRDRTRIPLE:
            nIdx = 10;
            break;

        case RTF_BRDRTNTHSG:
            nIdx = 11;
            break;

        case RTF_BRDRTHTNSG:
            nIdx = 12;
            break;

        case RTF_BRDRTNTHTNSG:
            nIdx = 13;
            break;

        case RTF_BRDRTNTHMG:
            nIdx = 14;
            break;

        case RTF_BRDRTHTNMG:
            nIdx = 15;
            break;

        case RTF_BRDRTNTHTNMG:
            nIdx = 16;
            break;

        case RTF_BRDRTNTHLG:
            nIdx = 17;
            break;

        case RTF_BRDRTHTNLG:
            nIdx = 18;
            break;

        case RTF_BRDRTNTHTNLG:
            nIdx = 19;
            break;

        case RTF_BRDRWAVY:
            nIdx = 20;
            break;

        case RTF_BRDRWAVYDB:
            nIdx = 21;
            break;

        case RTF_BRDREMBOSS:
            nIdx = 24;
            break;

        case RTF_BRDRENGRAVE:
            nIdx = 25;
            break;

        case RTF_BRSP:
            nPageDistance = static_cast< short >(nTokenValue);
            break;

        case RTF_BRDRDOT:           // SO does not have dashed or dotted lines
        case RTF_BRDRDASH:
        case RTF_BRDRDASHSM:
        case RTF_BRDRDASHD:
        case RTF_BRDRDASHDD:
        case RTF_BRDRDASHDOTSTR:
        case RTF_BRDRSH:            // shading not supported
        case RTF_BRDRCF:            // colors not supported
            break;

        case RTF_BRDRW:
            nLineThickness = static_cast< short >(nTokenValue);
            break;
        default:
            bWeiter = false;
            SkipToken(-1);
            break;
        }
        if (bWeiter)
            nToken = GetNextToken();
    } while (bWeiter && IsParserWorking());

    GetLineIndex(rBox, nLineThickness, nPageDistance, nCol, nIdx, nLine, nLine, 0);
}

// lese alle Dokument-Controls ein
void SwRTFParser::ReadDocControls( int nToken )
{
    int bWeiter = true;

    SwFtnInfo aFtnInfo;
    SwEndNoteInfo aEndInfo;
    bool bSetHyph = false;

    BOOL bEndInfoChgd = FALSE, bFtnInfoChgd = FALSE;

    do {
        USHORT nValue = USHORT( nTokenValue );
        switch( nToken )
        {
        case RTF_RTLDOC:
            maPageDefaults.mbRTLdoc = true;
            break;
        case RTF_LTRDOC:
            maPageDefaults.mbRTLdoc = false;
            break;
        case RTF_LANDSCAPE:
            maPageDefaults.mbLandscape = true;
            break;
        case RTF_PAPERW:
            if( 0 < nTokenValue )
                maPageDefaults.mnPaperw = nTokenValue;
            break;
        case RTF_PAPERH:
            if( 0 < nTokenValue )
                maPageDefaults.mnPaperh = nTokenValue;
            break;
        case RTF_MARGL:
            if( 0 <= nTokenValue )
                maPageDefaults.mnMargl = nTokenValue;
            break;
        case RTF_MARGR:
            if( 0 <= nTokenValue )
                maPageDefaults.mnMargr = nTokenValue;
            break;
        case RTF_MARGT:
            if( 0 <= nTokenValue )
                maPageDefaults.mnMargt = nTokenValue;
            break;
        case RTF_MARGB:
            if( 0 <= nTokenValue )
                maPageDefaults.mnMargb = nTokenValue;
            break;
        case RTF_FACINGP:
            maPageDefaults.mbFacingp = true;
            break;
        case RTF_PGNSTART:
            maPageDefaults.mnPgnStart = nTokenValue;
            break;
        case RTF_ENDDOC:
        case RTF_ENDNOTES:
            aFtnInfo.ePos = FTNPOS_CHAPTER; bFtnInfoChgd = TRUE;
            break;
        case RTF_FTNTJ:
        case RTF_FTNBJ:
            aFtnInfo.ePos = FTNPOS_PAGE; bFtnInfoChgd = TRUE;
            break;

        case RTF_AENDDOC:
        case RTF_AENDNOTES:
        case RTF_AFTNTJ:
        case RTF_AFTNBJ:
        case RTF_AFTNRESTART:
        case RTF_AFTNRSTCONT:
            break;      // wir kenn nur am Doc Ende und Doc weite Num.!

        case RTF_FTNSTART:
            if( nValue )
            {
                aFtnInfo.nFtnOffset = nValue-1;
                bFtnInfoChgd = TRUE;
            }
            break;
        case RTF_AFTNSTART:
            if( nValue )
            {
                aEndInfo.nFtnOffset = nValue-1;
                bEndInfoChgd = TRUE;
            }
            break;
        case RTF_FTNRSTPG:
            aFtnInfo.eNum = FTNNUM_PAGE; bFtnInfoChgd = TRUE;
            break;
        case RTF_FTNRESTART:
            aFtnInfo.eNum = FTNNUM_CHAPTER; bFtnInfoChgd = TRUE;
            break;
        case RTF_FTNRSTCONT:
            aFtnInfo.eNum = FTNNUM_DOC; bFtnInfoChgd = TRUE;
            break;

        case RTF_FTNNAR:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_ARABIC); bFtnInfoChgd = TRUE; break;
        case RTF_FTNNALC:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER_N); bFtnInfoChgd = TRUE; break;
        case RTF_FTNNAUC:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_UPPER_LETTER_N); bFtnInfoChgd = TRUE; break;
        case RTF_FTNNRLC:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_ROMAN_LOWER); bFtnInfoChgd = TRUE; break;
        case RTF_FTNNRUC:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_ROMAN_UPPER); bFtnInfoChgd = TRUE; break;
        case RTF_FTNNCHI:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL); bFtnInfoChgd = TRUE; break;

        case RTF_AFTNNAR:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_ARABIC); bEndInfoChgd = TRUE; break;
        case RTF_AFTNNALC:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER_N);
            bEndInfoChgd = TRUE;
            break;
        case RTF_AFTNNAUC:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_UPPER_LETTER_N);
            bEndInfoChgd = TRUE;
            break;
        case RTF_AFTNNRLC:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_ROMAN_LOWER);
            bEndInfoChgd = TRUE;
            break;
        case RTF_AFTNNRUC:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_ROMAN_UPPER);
            bEndInfoChgd = TRUE;
            break;
        case RTF_AFTNNCHI:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            bEndInfoChgd = TRUE;
            break;
        case RTF_HYPHAUTO:
            if (nTokenValue)
                bSetHyph = true;
            //FOO//
            break;
        case RTF_PGBRDRT:
            SetBorderLine(maPageDefaults.maBox, BOX_LINE_TOP);
            break;

        case RTF_PGBRDRB:
            SetBorderLine(maPageDefaults.maBox, BOX_LINE_BOTTOM);
            break;

        case RTF_PGBRDRL:
            SetBorderLine(maPageDefaults.maBox, BOX_LINE_LEFT);
            break;

        case RTF_PGBRDRR:
            SetBorderLine(maPageDefaults.maBox, BOX_LINE_RIGHT);
            break;

        case '{':
            {
                short nSkip = 0;
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nSkip = -1;
                else if( RTF_DOCFMT != (( nToken = GetNextToken() )
                        & ~(0xff | RTF_SWGDEFS)) )
                    nSkip = -2;
                else
                {
                    SkipGroup();        // erstmal komplett ueberlesen
                    // ueberlese noch die schliessende Klammer
                    GetNextToken();
                }
                if( nSkip )
                {
                    SkipToken( nSkip );     // Ignore wieder zurueck
                    bWeiter = FALSE;
                }
            }
            break;

        default:
            if( RTF_DOCFMT == (nToken & ~(0xff | RTF_SWGDEFS)) ||
                RTF_UNKNOWNCONTROL == nToken )
                SvxRTFParser::NextToken( nToken );
            else
                bWeiter = FALSE;
            break;
        }
        if( bWeiter )
            nToken = GetNextToken();
    } while( bWeiter && IsParserWorking() );

    if (IsNewDoc())
    {
        if( bEndInfoChgd )
            pDoc->SetEndNoteInfo( aEndInfo );
        if( bFtnInfoChgd )
            pDoc->SetFtnInfo( aFtnInfo );
    }

    if (!bSwPageDesc)
    {
        SetPageInformationAsDefault(maPageDefaults);

        MakeStyleTab();

        SwTxtFmtColl* pColl = aTxtCollTbl.Get(0);
        if (!pColl)
        {
            pColl = pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false );
        }

        ASSERT(pColl, "impossible to have no standard style");

        if (pColl)
        {
            if (
                IsNewDoc() && bSetHyph &&
                SFX_ITEM_SET != pColl->GetItemState(RES_PARATR_HYPHENZONE,
                false)
               )
            {
                pColl->SetFmtAttr(SvxHyphenZoneItem(true, RES_PARATR_HYPHENZONE));
            }

            pDoc->SetTxtFmtColl( *pPam, pColl );
        }
    }

    SkipToken( -1 );
}

void SwRTFParser::MakeStyleTab()
{
	// dann erzeuge aus der SvxStyle-Tabelle die Swg-Collections
	if( GetStyleTbl().Count() )
	{
		USHORT nValidOutlineLevels = 0;
		if( !IsNewDoc() )
		{
			// search all outlined collections
			//BYTE nLvl;
			const SwTxtFmtColls& rColls = *pDoc->GetTxtFmtColls();
			for( USHORT n = rColls.Count(); n; )
				//if( MAXLEVEL > (nLvl = rColls[ --n ]->GetOutlineLevel() ))//#outline level,zhaojianwei
				//	nValidOutlineLevels |= 1 << nLvl;
				if( rColls[ --n ]->IsAssignedToListLevelOfOutlineStyle())	
					nValidOutlineLevels |= 1 << rColls[ n ]->GetAssignedOutlineStyleLevel();//<-end,zhaojianwei
		}

		SvxRTFStyleType* pStyle = GetStyleTbl().First();
		do {
			USHORT nNo = USHORT( GetStyleTbl().GetCurKey() );
			if( pStyle->bIsCharFmt )
			{
				if( !aCharFmtTbl.Get( nNo ) )
					// existiert noch nicht, also anlegen
					MakeCharStyle( nNo, *pStyle );
			}
			else if( !aTxtCollTbl.Get( nNo ) )
			{
				// existiert noch nicht, also anlegen
				MakeStyle( nNo, *pStyle );
			}

		} while( 0 != (pStyle = GetStyleTbl().Next()) );
		bStyleTabValid = TRUE;
	}
}

BOOL lcl_SetFmtCol( SwFmt& rFmt, USHORT nCols, USHORT nColSpace,
                    const SvUShorts& rColumns )
{
    BOOL bSet = FALSE;
    if( nCols && USHRT_MAX != nCols )
    {
        SwFmtCol aCol;
        if( USHRT_MAX == nColSpace )
            nColSpace = 720;

        aCol.Init( nCols, nColSpace, USHRT_MAX );
        if( nCols == ( rColumns.Count() / 2 ) )
        {
            aCol._SetOrtho( FALSE );
            USHORT nWishWidth = 0, nHalfPrev = 0;
            for( USHORT n = 0, i = 0; n < rColumns.Count(); n += 2, ++i )
            {
                SwColumn* pCol = aCol.GetColumns()[ i ];
                pCol->SetLeft( nHalfPrev );
                USHORT nSp = rColumns[ n+1 ];
                nHalfPrev = nSp / 2;
                pCol->SetRight( nSp - nHalfPrev );
                pCol->SetWishWidth( rColumns[ n ] +
                                    pCol->GetLeft() + pCol->GetRight() );
                nWishWidth = nWishWidth + pCol->GetWishWidth();
            }
            aCol.SetWishWidth( nWishWidth );
        }
        rFmt.SetFmtAttr( aCol );
        bSet = TRUE;
    }
    return bSet;
}

void SwRTFParser::DoHairyWriterPageDesc(int nToken)
{
    int bWeiter = TRUE;
    do {
        if( '{' == nToken )
        {
            switch( nToken = GetNextToken() )
            {
            case RTF_IGNOREFLAG:
                if( RTF_SECTFMT != (( nToken = GetNextToken() )
                    & ~(0xff | RTF_SWGDEFS)) )
                {
                    SkipToken( -2 );    // Ignore und Token wieder zurueck
                    bWeiter = FALSE;
                    break;
                }
                // kein break, Gruppe ueberspringen

            case RTF_FOOTER:
            case RTF_HEADER:
            case RTF_FOOTERR:
            case RTF_HEADERR:
            case RTF_FOOTERL:
            case RTF_HEADERL:
            case RTF_FOOTERF:
            case RTF_HEADERF:
                SkipGroup();        // erstmal komplett ueberlesen
                // ueberlese noch die schliessende Klammer
                GetNextToken();
                break;

            default:
                SkipToken( -1 );            // Ignore wieder zurueck
                bWeiter = FALSE;
                break;
            }
        }
        else if( RTF_SECTFMT == (nToken & ~(0xff | RTF_SWGDEFS)) ||
            RTF_UNKNOWNCONTROL == nToken )
            SvxRTFParser::NextToken( nToken );
        else
            bWeiter = FALSE;
        if( bWeiter )
            nToken = GetNextToken();
    } while( bWeiter && IsParserWorking() );
    SkipToken( -1 );                    // letztes Token wieder zurueck
    return;
}

void SwRTFParser::ReadSectControls( int nToken )
{
    //this is some hairy stuff to try and retain writer style page descriptors
    //in rtf, almost certainy a bad idea, but we've inherited it, so here it
    //stays
    if (bInPgDscTbl)
    {
        DoHairyWriterPageDesc(nToken);
        return;
    }

    ASSERT(!maSegments.empty(), "suspicious to have a section with no "
        "page info, though probably legal");
    if (maSegments.empty())
    {
        maSegments.push_back(rtfSection(*pPam->GetPoint(),
            SectPageInformation(maPageDefaults)));
    }

    SectPageInformation aNewSection(maSegments.back().maPageInfo);

    bool bNewSection = false;
    bool bNewSectionHeader = false;
    const SwFmtHeader* _pKeepHeader = NULL;
    const SwFmtFooter* _pKeepFooter = NULL;
    int bWeiter = true;
    bool bKeepFooter = false;
    do {
        USHORT nValue = USHORT( nTokenValue );
        switch( nToken )
        {
            case RTF_SECT:
                bNewSection = true;
                bForceNewTable = true; // #117882#
                break;
            case RTF_SECTD: {
                //Reset to page defaults
                SwPageDesc* oldPageDesc=aNewSection.mpPageHdFt;
                aNewSection = SectPageInformation(maPageDefaults);
                aNewSection.mpPageHdFt=oldPageDesc;
                _pKeepHeader = NULL;
                _pKeepFooter = NULL;
                } break;
            case RTF_PGWSXN:
                if (0 < nTokenValue)
                    aNewSection.mnPgwsxn = nTokenValue;
                break;
            case RTF_PGHSXN:
                if (0 < nTokenValue)
                    aNewSection.mnPghsxn = nTokenValue;
                break;
            case RTF_MARGLSXN:
                if (0 <= nTokenValue)
                    aNewSection.mnMarglsxn = nTokenValue;
                break;
            case RTF_MARGRSXN:
                if (0 <= nTokenValue)
                    aNewSection.mnMargrsxn = nTokenValue;
                break;
            case RTF_MARGTSXN:
                if (0 <= nTokenValue)
                    aNewSection.mnMargtsxn = nTokenValue;
                break;
            case RTF_MARGBSXN:
                if (0 <= nTokenValue)
                    aNewSection.mnMargbsxn = nTokenValue;
                break;
            case RTF_FACPGSXN:
                aNewSection.mbFacpgsxn = true;
                break;
            case RTF_HEADERY:
                aNewSection.mnHeadery = nTokenValue;
                break;
            case RTF_FOOTERY:
                aNewSection.mnFootery = nTokenValue;
                break;
            case RTF_LNDSCPSXN:
                aNewSection.mbLndscpsxn = true;
                break;
            case RTF_PGNSTARTS:
                aNewSection.mnPgnStarts = nTokenValue;
                break;
            case RTF_PGNDEC:
                aNewSection.maNumType.SetNumberingType(SVX_NUM_ARABIC);
                break;
            case RTF_PGNUCRM:
                aNewSection.maNumType.SetNumberingType(SVX_NUM_ROMAN_UPPER);
                break;
            case RTF_PGNLCRM:
                aNewSection.maNumType.SetNumberingType(SVX_NUM_ROMAN_LOWER);
                break;
            case RTF_PGNUCLTR:
                aNewSection.maNumType.SetNumberingType(
                    SVX_NUM_CHARS_UPPER_LETTER_N);
                break;
            case RTF_PGNLCLTR:
                aNewSection.maNumType.SetNumberingType(
                    SVX_NUM_CHARS_LOWER_LETTER_N);
                break;
            case RTF_SBKNONE:
                aNewSection.mnBkc = 0;
                break;
            case RTF_SBKCOL:
                aNewSection.mnBkc = 1;
                break;
            case RTF_PGBRDRT:
                SetBorderLine(aNewSection.maBox, BOX_LINE_TOP);
                break;

            case RTF_PGBRDRB:
                SetBorderLine(aNewSection.maBox, BOX_LINE_BOTTOM);
                break;

            case RTF_PGBRDRL:
                SetBorderLine(aNewSection.maBox, BOX_LINE_LEFT);
                break;

            case RTF_PGBRDRR:
                SetBorderLine(aNewSection.maBox, BOX_LINE_RIGHT);
                break;

            case RTF_PGBRDROPT:
            case RTF_ENDNHERE:
            case RTF_BINFSXN:
            case RTF_BINSXN:
            case RTF_SBKPAGE:
            case RTF_SBKEVEN:
            case RTF_SBKODD:
            case RTF_LINEBETCOL:
            case RTF_LINEMOD:
            case RTF_LINEX:
            case RTF_LINESTARTS:
            case RTF_LINERESTART:
            case RTF_LINEPAGE:
            case RTF_LINECONT:
            case RTF_GUTTERSXN:
            case RTF_PGNCONT:
            case RTF_PGNRESTART:
            case RTF_PGNX:
            case RTF_PGNY:
            case RTF_VERTALT:
            case RTF_VERTALB:
            case RTF_VERTALC:
            case RTF_VERTALJ:
                break;
            case RTF_TITLEPG:
                aNewSection.mbTitlepg = true;
                break;
            case RTF_HEADER:
            case RTF_HEADERL:
            case RTF_HEADERR:
                if (aNewSection.mpPageHdFt!=NULL)
                {
                    _pKeepHeader = NULL;
                    bKeepFooter = true; // #i82008
                    _pKeepFooter = &aNewSection.mpPageHdFt->GetMaster().GetFooter();
                }
            case RTF_FOOTER:
            case RTF_FOOTERL:
            case RTF_FOOTERR:
                if (aNewSection.mpPageHdFt!=NULL && !bKeepFooter )
                {
                    _pKeepFooter = NULL;
                    _pKeepHeader = &aNewSection.mpPageHdFt->GetMaster().GetHeader();
                }
                bKeepFooter = false;
                if (!bNewSectionHeader) { //see #117914# topic 2). If a header is redefined in a section
                    bNewSectionHeader=true;                    //  a new header must be created.
                    aNewSection.mpPageHdFt=NULL;
                }
                if (!aNewSection.mpPageHdFt)
                {
                    String aName(RTL_CONSTASCII_STRINGPARAM("rtfHdFt"));
                    aName += String::CreateFromInt32(maSegments.size());
                    sal_uInt16 nPageNo = pDoc->MakePageDesc(aName);
                    aNewSection.mpPageHdFt = &pDoc->_GetPageDesc(nPageNo);
                    aNewSection.mbPageHdFtUsed = true;
                    maSegments.maDummyPageNos.push_back(nPageNo);
                }
                ReadHeaderFooter(nToken, aNewSection.mpPageHdFt);
                if (_pKeepHeader) aNewSection.mpPageHdFt->GetMaster().SetFmtAttr(*_pKeepHeader);
                if (_pKeepFooter) aNewSection.mpPageHdFt->GetMaster().SetFmtAttr(*_pKeepFooter);
                break;
            case RTF_FOOTERF:
            case RTF_HEADERF:
                if (!aNewSection.mpTitlePageHdFt)
                {
                    String aTitle(RTL_CONSTASCII_STRINGPARAM("rtfTitleHdFt"));
                    aTitle += String::CreateFromInt32(maSegments.size());
                    sal_uInt16 nPageNo = pDoc->MakePageDesc(aTitle);
                    aNewSection.mpTitlePageHdFt = &pDoc->_GetPageDesc(nPageNo);
                    aNewSection.mbTitlePageHdFtUsed = true;
                    maSegments.maDummyPageNos.push_back(nPageNo);
                }
                ReadHeaderFooter(nToken, aNewSection.mpTitlePageHdFt);
                break;
            case RTF_COLS:
                aNewSection.mnCols = nTokenValue;
                break;
            case RTF_COLSX:
                aNewSection.mnColsx = nTokenValue;
                break;
            case RTF_COLNO:
                {
                    // next token must be either colw or colsr
                    unsigned long nAktCol = nValue;
                    long nWidth = 0, nSpace = 0;
                    int nColToken = GetNextToken();
                    if (RTF_COLW == nColToken)
                    {
                        // next token could be colsr (but not required)
                        nWidth = nTokenValue;
                        if( RTF_COLSR == GetNextToken() )
                            nSpace = nTokenValue;
                        else
                            SkipToken( -1 );        // put back token
                    }
                    else if (RTF_COLSR == nColToken)
                    {
                        // next token must be colw (what sense should it make to have colsr only?!)
                        nSpace = nTokenValue;
                        if( RTF_COLW == GetNextToken() )
                            nWidth = nTokenValue;
                        else
                            // what should we do if an isolated colsr without colw is found? Doesn't make sense!
                            SkipToken( -1 );        // put back token
                    }
                    else
                        break;

                    if (--nAktCol == (aNewSection.maColumns.size() / 2))
                    {
                        aNewSection.maColumns.push_back(nWidth);
                        aNewSection.maColumns.push_back(nSpace);
                    }
                }
                break;
            case RTF_STEXTFLOW:
                aNewSection.mnStextflow = nTokenValue;
                break;
            case RTF_RTLSECT:
                aNewSection.mbRTLsection = true;
                break;
            case RTF_LTRSECT:
                aNewSection.mbRTLsection = false;
                break;
            case '{':
                {
                    short nSkip = 0;
                    if( RTF_IGNOREFLAG != ( nToken = GetNextToken() ))
                        nSkip = -1;
                    else if( RTF_SECTFMT != (( nToken = GetNextToken() )
                             & ~(0xff | RTF_SWGDEFS)) &&
                            ( RTF_DOCFMT != ( nToken & ~(0xff | RTF_SWGDEFS))) )
                        nSkip = -2;
                    else
                    {
                        // erstmal komplett ueberlesen
                        SkipGroup();
                        // ueberlese noch die schliessende Klammer
                        GetNextToken();
                    }
                    if (nSkip)
                    {
                        bWeiter = ((-1 == nSkip) &&
                            (
                              RTF_FOOTER == nToken || RTF_HEADER == nToken ||
                              RTF_FOOTERR == nToken || RTF_HEADERR == nToken ||
                              RTF_FOOTERL == nToken || RTF_HEADERL == nToken ||
                              RTF_FOOTERF == nToken || RTF_HEADERF == nToken
                            ));
                        SkipToken (nSkip);      // Ignore wieder zurueck
                    }
                }
                break;
            case RTF_PAPERW:
            case RTF_PAPERH:
            case RTF_MARGL:
            case RTF_MARGR:
            case RTF_MARGT:
            case RTF_MARGB:
            case RTF_FACINGP:
                ASSERT(!this, "why are these tokens found in this section?");
                ReadDocControls( nToken );
                break;
            default:
                if (RTF_DOCFMT == (nToken & ~(0xff | RTF_SWGDEFS)))
                    ReadDocControls( nToken );
                else if (RTF_SECTFMT == (nToken & ~(0xff | RTF_SWGDEFS)) ||
                         RTF_UNKNOWNCONTROL == nToken)
                {
                    SvxRTFParser::NextToken(nToken);
                }
                else
                    bWeiter = false;
                break;
        }

        if (bWeiter)
            nToken = GetNextToken();
    } while (bWeiter && IsParserWorking());

    if (bNewSection || maSegments.empty())
    {
        AttrGroupEnd(); //#106493#
        if(!bContainsPara && !bContainsTablePara) //#117881#: bContainsTablePara is set in rtftbl.cxx
            pDoc->AppendTxtNode(*pPam->GetPoint());
        bContainsPara = false;
        bContainsTablePara = false;
        maSegments.push_back(rtfSection(*pPam->GetPoint(), aNewSection));
    }
    else //modifying/replacing the current section
    {
        SwPaM aPamStart(maSegments.back().maStart);
        maSegments.pop_back();
        maSegments.push_back(rtfSection(*aPamStart.GetPoint(), aNewSection));
    }

    SkipToken(-1);
}

void SwRTFParser::EnterEnvironment()
{
}


void SwRTFParser::LeaveEnvironment()
{
    if(pRedlineDelete)
    {
        delete pRedlineDelete;
        pRedlineDelete = 0;
    }

    if(pRedlineInsert)
    {
        delete pRedlineInsert;
        pRedlineInsert = 0;
    }
}

void SwRTFParser::SkipPageDescTbl()
{
    // M.M. #117907# I have to use this glorified SkipGroup because the
    // SvParser SkipGroup uses nNextCh which is not set correctly <groan>
    int nNumOpenBrakets = 1;

    while( nNumOpenBrakets && IsParserWorking() )
    {
        switch( GetNextToken() )
        {
        case '}':
            {
                --nNumOpenBrakets;
            }
            break;

        case '{':
            {
                nNumOpenBrakets++;
            }
            break;
        }
    }

    SkipToken( -1 );
}

void SwRTFParser::ReadPageDescTbl()
{
    // dann erzeuge aus der SvxStyle-Tabelle die Swg-Collections, damit
    // diese auch in den Headers/Footer benutzt werden koennen!
    MakeStyleTab();
    // das default-Style schon gleich am ersten Node setzen
    SwTxtFmtColl* pColl = aTxtCollTbl.Get( 0 );
    if( !pColl )
        pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false );
    pDoc->SetTxtFmtColl( *pPam, pColl );

    int nToken, bSaveChkStyleAttr = IsChkStyleAttr();
    int nNumOpenBrakets = 1;        // die erste wurde schon vorher erkannt !!

    SetChkStyleAttr(FALSE);     // Attribute nicht gegen die Styles checken

    bInPgDscTbl = true;
    USHORT nPos = 0;
    SwPageDesc* pPg = 0;
    SwFrmFmt* pPgFmt = 0;

    SvxULSpaceItem aUL( RES_UL_SPACE ), aHUL( RES_UL_SPACE ), aFUL( RES_UL_SPACE );
    SvxLRSpaceItem aLR( RES_LR_SPACE ), aHLR( RES_LR_SPACE ), aFLR( RES_LR_SPACE );
    SwFmtFrmSize aSz( ATT_FIX_SIZE, 11905, 16837 );     // DIN A4 defaulten
    SwFmtFrmSize aFSz( ATT_MIN_SIZE ), aHSz( ATT_MIN_SIZE );

    SvxFrameDirectionItem aFrmDir(FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR);

    USHORT nCols = USHRT_MAX, nColSpace = USHRT_MAX, nAktCol = 0;
    SvUShorts aColumns;

    while( nNumOpenBrakets && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case '{':
            ++nNumOpenBrakets;
            break;
        case '}':
            if (1 == --nNumOpenBrakets)
            {
                ASSERT(pPgFmt && pPg, "Serious problem here");
                if (pPgFmt && pPg)
                {
                    // PageDesc ist fertig, setze am Doc
                    pPgFmt->SetFmtAttr(aFrmDir);
                    pPgFmt->SetFmtAttr(aLR);
                    pPgFmt->SetFmtAttr(aUL);
                    pPgFmt->SetFmtAttr(aSz);
                    ::lcl_SetFmtCol(*pPgFmt, nCols, nColSpace, aColumns);
                    if (pPgFmt->GetHeader().GetHeaderFmt())
                    {
                        SwFrmFmt* pHFmt =
                            (SwFrmFmt*)pPgFmt->GetHeader().GetHeaderFmt();
                        pHFmt->SetFmtAttr(aHUL);
                        pHFmt->SetFmtAttr(aHLR);
                        pHFmt->SetFmtAttr(aHSz);
                    }
                    if (pPgFmt->GetFooter().GetFooterFmt())
                    {
                        SwFrmFmt* pFFmt =
                            (SwFrmFmt*)pPgFmt->GetFooter().GetFooterFmt();
                        pFFmt->SetFmtAttr(aHUL);
                        pFFmt->SetFmtAttr(aHLR);
                        pFFmt->SetFmtAttr(aHSz);
                    }
                    if( nPos < pDoc->GetPageDescCnt() )
                        pDoc->ChgPageDesc(nPos++, *pPg);
                }
            }
            break;
        case RTF_PGDSC:
            if (nPos)   // kein && wg MAC
            {
                if (nPos != pDoc->MakePageDesc(
                    String::CreateFromInt32(nTokenValue)))
                {
                    ASSERT( FALSE, "PageDesc an falscher Position" );
                }
            }
            pPg = &pDoc->_GetPageDesc(nPos);
            pPg->SetLandscape( FALSE );
            pPgFmt = &pPg->GetMaster();
#ifndef CFRONT
    SETPAGEDESC_DEFAULTS:
#endif
            // aSz = pPgFmt->GetFrmSize();
            aSz.SetWidth( 11905 ); aSz.SetHeight( 16837 );      // DIN A4 defaulten
            aLR.SetLeft( 0 );   aLR.SetRight( 0 );
            aUL.SetLower( 0 );  aUL.SetUpper( 0 );
            aHLR.SetLeft( 0 );  aHLR.SetRight( 0 );
            aHUL.SetLower( 0 ); aHUL.SetUpper( 0 );
            aFLR.SetLeft( 0 );  aFLR.SetRight( 0 );
            aFUL.SetLower( 0 ); aFUL.SetUpper( 0 );
            nCols = USHRT_MAX; nColSpace = USHRT_MAX; nAktCol = 0;
            aFSz.SetHeightSizeType( ATT_MIN_SIZE ); aFSz.SetHeight( 0 );
            aHSz.SetHeightSizeType( ATT_MIN_SIZE ); aHSz.SetHeight( 0 );
            break;

        case RTF_PGDSCUSE:
            pPg->WriteUseOn( (UseOnPage)nTokenValue );
            break;

        case RTF_PGDSCNXT:
            // setze erstmal nur die Nummer als Follow. Am Ende der
            // Tabelle wird diese entsprechend korrigiert !!
            if( nTokenValue )
                pPg->SetFollow( (const SwPageDesc*)nTokenValue );
            else
                pPg->SetFollow( & const_cast<const SwDoc *>(pDoc)
                                ->GetPageDesc( 0 ) );
            break;

        case RTF_FORMULA:   /* Zeichen "\|" !!! */
            pPgFmt->SetFmtAttr( aLR );
            pPgFmt->SetFmtAttr( aUL );
            pPgFmt->SetFmtAttr( aSz );
            ::lcl_SetFmtCol( *pPgFmt, nCols, nColSpace, aColumns );
            if( pPgFmt->GetHeader().GetHeaderFmt() )
            {
                SwFrmFmt* pHFmt = (SwFrmFmt*)pPgFmt->GetHeader().GetHeaderFmt();
                pHFmt->SetFmtAttr( aHUL );
                pHFmt->SetFmtAttr( aHLR );
                pHFmt->SetFmtAttr( aHSz );
            }
            if( pPgFmt->GetFooter().GetFooterFmt() )
            {
                SwFrmFmt* pFFmt = (SwFrmFmt*)pPgFmt->GetFooter().GetFooterFmt();
                pFFmt->SetFmtAttr( aHUL );
                pFFmt->SetFmtAttr( aHLR );
                pFFmt->SetFmtAttr( aHSz );
            }

            pPgFmt = &pPg->GetLeft();
#ifndef CFRONT
            goto SETPAGEDESC_DEFAULTS;
#else
            aLR.SetLeft( 0 );   aLR.SetRight( 0 );
            aUL.SetLower( 0 );  aUL.SetUpper( 0 );
            aHLR.SetLeft( 0 );  aHLR.SetRight( 0 );
            aHUL.SetLower( 0 ); aHUL.SetUpper( 0 );
            aFLR.SetLeft( 0 );  aFLR.SetRight( 0 );
            aFUL.SetLower( 0 ); aFUL.SetUpper( 0 );
//          aSz = pPgFmt->GetFrmSize();
            aSz.SetWidth( 11905 ); aSz.SetHeight( 16837 );      // DIN A4 defaulten
            nCols = USHRT_MAX; nColSpace = USHRT_MAX; nAktCol = 0;
            aFSz.SetHeightSizeType( ATT_MIN_SIZE ); aFSz.SetHeight( 0 );
            aHSz.SetHeightSizeType( ATT_MIN_SIZE ); aHSz.SetHeight( 0 );
            break;
#endif

        case RTF_RTLSECT:
            aFrmDir.SetValue(FRMDIR_HORI_RIGHT_TOP);
            break;

        case RTF_LTRSECT:
            aFrmDir.SetValue(FRMDIR_HORI_LEFT_TOP);
            break;

        // alt: LI/RI/SA/SB, neu: MARG?SXN
        case RTF_MARGLSXN:
        case RTF_LI:        aLR.SetLeft( (USHORT)nTokenValue );     break;
        case RTF_MARGRSXN:
        case RTF_RI:        aLR.SetRight( (USHORT)nTokenValue );    break;
        case RTF_MARGTSXN:
        case RTF_SA:        aUL.SetUpper( (USHORT)nTokenValue );    break;
        case RTF_MARGBSXN:
        case RTF_SB:        aUL.SetLower( (USHORT)nTokenValue );    break;
        case RTF_PGWSXN:    aSz.SetWidth( nTokenValue );            break;
        case RTF_PGHSXN:    aSz.SetHeight( nTokenValue );           break;

        case RTF_HEADERY:       aHUL.SetUpper( (USHORT)nTokenValue );   break;
        case RTF_HEADER_YB:     aHUL.SetLower( (USHORT)nTokenValue );   break;
        case RTF_HEADER_XL:     aHLR.SetLeft( (USHORT)nTokenValue );    break;
        case RTF_HEADER_XR:     aHLR.SetRight( (USHORT)nTokenValue );   break;
        case RTF_FOOTERY:       aFUL.SetLower( (USHORT)nTokenValue );   break;
        case RTF_FOOTER_YT:     aFUL.SetUpper( (USHORT)nTokenValue );   break;
        case RTF_FOOTER_XL:     aFLR.SetLeft( (USHORT)nTokenValue );    break;
        case RTF_FOOTER_XR:     aFLR.SetRight( (USHORT)nTokenValue );   break;

        case RTF_HEADER_YH:
                if( 0 > nTokenValue )
                {
                    aHSz.SetHeightSizeType( ATT_FIX_SIZE );
                    nTokenValue = -nTokenValue;
                }
                aHSz.SetHeight( (USHORT)nTokenValue );
                break;

        case RTF_FOOTER_YH:
                if( 0 > nTokenValue )
                {
                    aFSz.SetHeightSizeType( ATT_FIX_SIZE );
                    nTokenValue = -nTokenValue;
                }
                aFSz.SetHeight( (USHORT)nTokenValue );
                break;


        case RTF_LNDSCPSXN:     pPg->SetLandscape( TRUE );          break;

        case RTF_COLS:          nCols = (USHORT)nTokenValue;        break;
        case RTF_COLSX:         nColSpace = (USHORT)nTokenValue;    break;

        case RTF_COLNO:
            nAktCol = (USHORT)nTokenValue;
            if( RTF_COLW == GetNextToken() )
            {
                USHORT nWidth = USHORT( nTokenValue ), nSpace = 0;
                if( RTF_COLSR == GetNextToken() )
                    nSpace = USHORT( nTokenValue );
                else
                    SkipToken( -1 );        // wieder zurueck

                if( --nAktCol == ( aColumns.Count() / 2 ) )
                {
                    aColumns.Insert( nWidth, aColumns.Count() );
                    aColumns.Insert( nSpace, aColumns.Count() );
                }
            }
            break;

        case RTF_PAGEBB:
            {
                pPgFmt->SetFmtAttr( SvxFmtBreakItem( SVX_BREAK_PAGE_BEFORE, RES_BREAK ) );
            }
            break;

        case RTF_HEADER:
        case RTF_HEADERL:
        case RTF_HEADERR:
        case RTF_FOOTER:
        case RTF_FOOTERL:
        case RTF_FOOTERR:
        case RTF_FOOTERF:
        case RTF_HEADERF:
            ReadHeaderFooter(nToken, pPg);
            --nNumOpenBrakets;      // Klammer wird im ReadAttr ueberlesen!
            break;
        case RTF_TEXTTOKEN:
            if (!DelCharAtEnd(aToken, ';' ).Len())
                break;
            ASSERT(pPg, "Unexpected missing pPg");
            if (pPg)
            {
                pPg->SetName(aToken);

                // sollte es eine Vorlage aus dem Pool sein ??
                USHORT n = SwStyleNameMapper::GetPoolIdFromUIName(aToken,
                    nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC);
                if (USHRT_MAX != n)
                {
                    // dann setze bei der Neuen die entsp. PoolId
                    pPg->SetPoolFmtId(n);
                }
            }
            break;
        case RTF_BRDBOX:
            if (3 == nNumOpenBrakets)
            {
                ReadBorderAttr(SkipToken(-2),
                    (SfxItemSet&)pPgFmt->GetAttrSet());
                --nNumOpenBrakets;      // Klammer wird im ReadAttr ueberlesen!
            }
            break;
        case RTF_SHADOW:
            if( 3 == nNumOpenBrakets )
            {
                ReadAttr( SkipToken( -2 ), (SfxItemSet*)&pPgFmt->GetAttrSet() );
                --nNumOpenBrakets;      // Klammer wird im ReadAttr ueberlesen!
            }
            break;


        default:
            if( (nToken & ~0xff ) == RTF_SHADINGDEF )
                ReadBackgroundAttr( nToken, (SfxItemSet&)pPgFmt->GetAttrSet() );
            break;
        }
    }


    // setze jetzt noch bei allen die entsprechenden Follows !!
    // Die, die ueber die Tabelle eingelesen wurden und einen
    // Follow definiert haben, ist dieser als Tabposition im
    // Follow schon gesetzt.
    for( nPos = 0; nPos < pDoc->GetPageDescCnt(); ++nPos )
    {
        SwPageDesc* pPgDsc = &pDoc->_GetPageDesc( nPos );
        if( (USHORT)(long)pPgDsc->GetFollow() < pDoc->GetPageDescCnt() )
            pPgDsc->SetFollow(& const_cast<const SwDoc *>(pDoc)
                              ->GetPageDesc((USHORT)(long)
                                            pPgDsc->GetFollow()));
    }

    SetChkStyleAttr( bSaveChkStyleAttr );

    bInPgDscTbl = false;
    nAktPageDesc = 0;
    nAktFirstPageDesc = 0;
    bSwPageDesc = true;
    SkipToken( -1 );
}

// -------------- Methoden --------------------

/*
void SwRTFParser::ReadUnknownData()
{
    SvRTFParser::ReadUnknownData();
}

void SwRTFParser::ReadOLEData()
{
    SvRTFParser::ReadOLEData();
}
*/

void SwRTFParser::ReadPrtData()
{
    while( IsParserWorking() )
    {
        int nToken = GetNextToken();
        if( (RTF_TEXTTOKEN != nToken) && ('}' == nToken) )
            break;
    }

    SkipToken( -1 );        // schliessende Klammer wieder zurueck!!
}

static const SwNodeIndex* SetHeader(SwFrmFmt* pHdFtFmt, BOOL bReuseOld)
{
    ASSERT(pHdFtFmt, "Impossible, no header");
    const SwFrmFmt* pExisting = bReuseOld ?
        pHdFtFmt->GetHeader().GetHeaderFmt() : 0;
    if (!pExisting)
    {
        //No existing header, create a new one
        pHdFtFmt->SetFmtAttr(SwFmtHeader(TRUE));
        pExisting = pHdFtFmt->GetHeader().GetHeaderFmt();
    }
    return pExisting->GetCntnt().GetCntntIdx();
}

static const SwNodeIndex* SetFooter(SwFrmFmt* pHdFtFmt, BOOL bReuseOld)
{
    ASSERT(pHdFtFmt, "Impossible, no footer");
    const SwFrmFmt* pExisting = bReuseOld ?
        pHdFtFmt->GetFooter().GetFooterFmt() : 0;
    if (!pExisting)
    {
        //No exist footer, create a new one
        pHdFtFmt->SetFmtAttr(SwFmtFooter(TRUE));
        pExisting = pHdFtFmt->GetFooter().GetFooterFmt();
    }
    return pExisting->GetCntnt().GetCntntIdx();
}


void SwRTFParser::ReadHeaderFooter( int nToken, SwPageDesc* pPageDesc )
{
    ASSERT( RTF_FOOTNOTE == nToken ||
            RTF_FLY_INPARA == nToken ||
            pPageDesc, "PageDesc fehlt" );

    bool bContainsParaCache = bContainsPara;
    // alle wichtigen Sachen sichern
    SwPosition aSavePos( *pPam->GetPoint() );
    SvxRTFItemStack aSaveStack;
    aSaveStack.Insert( &GetAttrStack(), 0 );
    GetAttrStack().Remove( 0, GetAttrStack().Count() );

    // save the fly array - after read, all flys may be set into
    // the header/footer
    SwFlySaveArr aSaveArray( 255 < aFlyArr.Count() ? aFlyArr.Count() : 255 );
    aSaveArray.Insert( &aFlyArr, 0 );
    aFlyArr.Remove( 0, aFlyArr.Count() );
    BOOL bSetFlyInDoc = TRUE;

    const SwNodeIndex* pSttIdx = 0;
    SwFrmFmt* pHdFtFmt = 0;
    SwTxtAttr* pTxtAttr = 0;
    int bDelFirstChar = FALSE;
    bool bOldIsFootnote = mbIsFootnote;
    BOOL bOldGrpStt = sal::static_int_cast< BOOL, int >(IsNewGroup());

    int nNumOpenBrakets = GetOpenBrakets() - 1;

    switch( nToken )
    {
    case RTF_FOOTNOTE:
        {
            bool bIsEndNote = RTF_FTNALT == GetNextToken();
            if (!bIsEndNote)
                SkipToken(-1);

            SwTxtNode* pTxtNd = pPam->GetNode()->GetTxtNode();
            SwFmtFtn aFtnNote(bIsEndNote);
            xub_StrLen nPos = pPam->GetPoint()->nContent.GetIndex();

            if (nPos && !bFootnoteAutoNum)
            {
                pPam->GetPoint()->nContent--;
                nPos--;
                aFtnNote.SetNumStr( pTxtNd->GetTxt().GetChar( nPos ) );
                ((String&)pTxtNd->GetTxt()).SetChar( nPos, CH_TXTATR_INWORD );
                bDelFirstChar = TRUE;
            }

            pTxtAttr = pTxtNd->InsertItem( aFtnNote, nPos, nPos,
                        bDelFirstChar ? nsSetAttrMode::SETATTR_NOTXTATRCHR : 0 );

            ASSERT( pTxtAttr, "konnte die Fussnote nicht einfuegen/finden" );

            if( pTxtAttr )
                pSttIdx = ((SwTxtFtn*)pTxtAttr)->GetStartNode();
            mbIsFootnote = true;

            // wurde an der Position ein Escapement aufgespannt, so entferne
            // das jetzt. Fussnoten sind bei uns immer hochgestellt.
            SvxRTFItemStackTypePtr pTmp = aSaveStack.Top();
            if( pTmp && pTmp->GetSttNodeIdx() ==
                pPam->GetPoint()->nNode.GetIndex() &&
                pTmp->GetSttCnt() == nPos )
                pTmp->GetAttrSet().ClearItem( RES_CHRATR_ESCAPEMENT );
        }
        break;

    case RTF_FLY_INPARA:
        {
            xub_StrLen nPos = pPam->GetPoint()->nContent.GetIndex();
            SfxItemSet aSet( pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                                            RES_FRMATR_END-1 );
            aSet.Put( SwFmtAnchor( FLY_IN_CNTNT ));
            pHdFtFmt = pDoc->MakeFlySection( FLY_IN_CNTNT, pPam->GetPoint(), &aSet );

            pTxtAttr = pPam->GetNode()->GetTxtNode()->GetTxtAttr(
                                                nPos, RES_TXTATR_FLYCNT );
            ASSERT( pTxtAttr, "konnte den Fly nicht einfuegen/finden" );

            pSttIdx = pHdFtFmt->GetCntnt().GetCntntIdx();
            bSetFlyInDoc = FALSE;
        }
        break;

    case RTF_HEADERF:
    case RTF_HEADER:
        pPageDesc->WriteUseOn( (UseOnPage)(pPageDesc->ReadUseOn() | nsUseOnPage::PD_HEADERSHARE) );
        pHdFtFmt = &pPageDesc->GetMaster();
        pSttIdx = SetHeader( pHdFtFmt, FALSE );
        break;

    case RTF_HEADERL:
        // we cannot have left or right, must have always both
        pPageDesc->WriteUseOn( (UseOnPage)((pPageDesc->ReadUseOn() & ~nsUseOnPage::PD_HEADERSHARE) | nsUseOnPage::PD_ALL));
        SetHeader( pPageDesc->GetRightFmt(), TRUE );
        pHdFtFmt = pPageDesc->GetLeftFmt();
        pSttIdx = SetHeader(pHdFtFmt, FALSE );
        break;

    case RTF_HEADERR:
        // we cannot have left or right, must have always both
        pPageDesc->WriteUseOn( (UseOnPage)((pPageDesc->ReadUseOn() & ~nsUseOnPage::PD_HEADERSHARE) | nsUseOnPage::PD_ALL));
        SetHeader( pPageDesc->GetLeftFmt(), TRUE );
        pHdFtFmt = pPageDesc->GetRightFmt();
        pSttIdx = SetHeader(pHdFtFmt, FALSE );
        break;

    case RTF_FOOTERF:
    case RTF_FOOTER:
        pPageDesc->WriteUseOn( (UseOnPage)(pPageDesc->ReadUseOn() | nsUseOnPage::PD_FOOTERSHARE) );
        pHdFtFmt = &pPageDesc->GetMaster();
        pSttIdx = SetFooter(pHdFtFmt, FALSE );
        break;

    case RTF_FOOTERL:
        // we cannot have left or right, must have always both
        pPageDesc->WriteUseOn( (UseOnPage)((pPageDesc->ReadUseOn() & ~nsUseOnPage::PD_FOOTERSHARE) | nsUseOnPage::PD_ALL));
        SetFooter( pPageDesc->GetRightFmt(), TRUE );
        pHdFtFmt = pPageDesc->GetLeftFmt();
        pSttIdx = SetFooter(pHdFtFmt, FALSE );
        break;

    case RTF_FOOTERR:
        // we cannot have left or right, must have always both
        pPageDesc->WriteUseOn( (UseOnPage)((pPageDesc->ReadUseOn() & ~nsUseOnPage::PD_FOOTERSHARE) | nsUseOnPage::PD_ALL));
        SetFooter( pPageDesc->GetLeftFmt(), TRUE );
        pHdFtFmt = pPageDesc->GetRightFmt();
        pSttIdx = SetFooter(pHdFtFmt, FALSE );
        break;
    }

    USHORT nOldFlyArrCnt = aFlyArr.Count();
    if( !pSttIdx )
        SkipGroup();
    else
    {
        // es ist auf jedenfall jetzt ein TextNode im Kopf/Fusszeilen-Bereich
        // vorhanden. Dieser muss jetzt nur noch gefunden und der neue Cursor
        // dort hinein gesetzt werden.
        SwCntntNode *pNode = pDoc->GetNodes()[ pSttIdx->GetIndex()+1 ]->
                                GetCntntNode();

        // immer ans Ende der Section einfuegen !!
        pPam->GetPoint()->nNode = *pNode->EndOfSectionNode();
        pPam->Move( fnMoveBackward );

        SwTxtFmtColl* pColl = aTxtCollTbl.Get( 0 );
        if( !pColl )
            pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false );
        pDoc->SetTxtFmtColl( *pPam, pColl );

        SetNewGroup( TRUE );

        while( !( nNumOpenBrakets == GetOpenBrakets() && !GetStackPos()) && IsParserWorking() )
        {
            switch( nToken = GetNextToken() )
            {
            case RTF_U:
                if( bDelFirstChar )
                {
                    bDelFirstChar = FALSE;
                    nToken = 0;
                }
                break;

            case RTF_TEXTTOKEN:
                if( bDelFirstChar )
                {
                    if( !aToken.Erase( 0, 1 ).Len() )
                        nToken = 0;
                    bDelFirstChar = FALSE;
                }
                break;
            }
            if( nToken )
                NextToken( nToken );
        }

        SetAllAttrOfStk();
        if( aFlyArr.Count() && bSetFlyInDoc )
            SetFlysInDoc();

        // sollte der letze Node leer sein, dann loesche ihn
        // (\par heisst ja Absatzende und nicht neuer Absatz!)
        DelLastNode();
    }

    // vom FlyFmt noch die richtigen Attribute setzen
    if( pTxtAttr && RES_TXTATR_FLYCNT == pTxtAttr->Which() )
    {
        // is add a new fly ?
        if( nOldFlyArrCnt < aFlyArr.Count() )
        {
            SwFlySave* pFlySave = aFlyArr[ aFlyArr.Count()-1 ];
            pFlySave->aFlySet.ClearItem( RES_ANCHOR );
            pHdFtFmt->SetFmtAttr( pFlySave->aFlySet );
            aFlyArr.DeleteAndDestroy( aFlyArr.Count() - 1 );
        }
        else
        {
            // no, so remove the created textattribute
            SwFrmFmt* pFlyFmt = pTxtAttr->GetFlyCnt().GetFrmFmt();
            // remove the pam from the flynode
            *pPam->GetPoint() = aSavePos;
            pDoc->DelLayoutFmt( pFlyFmt );
        }
    }

    bFootnoteAutoNum = FALSE;       // default auf aus!

    // und alles wieder zurueck
    *pPam->GetPoint() = aSavePos;
    if (mbIsFootnote)
        SetNewGroup( bOldGrpStt );      // Status wieder zurueck
    else
        SetNewGroup( FALSE );           // { - Klammer war kein Group-Start!
    mbIsFootnote = bOldIsFootnote;
    GetAttrStack().Insert( &aSaveStack, 0 );

    aFlyArr.Insert( &aSaveArray, 0 );
    aSaveArray.Remove( 0, aSaveArray.Count() );
    bContainsPara = bContainsParaCache;
}

void SwRTFParser::SetSwgValues( SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    // Escapement korrigieren
    if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_ESCAPEMENT, FALSE, &pItem ))
    {
        /* prozentuale Veraenderung errechnen !
            * Formel :      (FontSize * 1/20 ) pts      Escapement * 2
            *               -----------------------  = ----------------
            *                     100%                          x
            */

        // die richtige
        long nEsc = ((SvxEscapementItem*)pItem)->GetEsc();

        // automatische Ausrichtung wurde schon richtig berechnet
        if( DFLT_ESC_AUTO_SUPER != nEsc && DFLT_ESC_AUTO_SUB != nEsc )
        {
            const SvxFontHeightItem& rFH = GetSize( rSet );
            nEsc *= 1000L;
            if(rFH.GetHeight()) nEsc /= long(rFH.GetHeight()); // #i77256#

            SvxEscapementItem aEsc( (short) nEsc,
                                ((SvxEscapementItem*)pItem)->GetProp(), RES_CHRATR_ESCAPEMENT);
            rSet.Put( aEsc );
        }
    }

    // TabStops anpassen
    if( SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_TABSTOP, FALSE, &pItem ))
    {
        const SvxLRSpaceItem& rLR = GetLRSpace( rSet );
        SvxTabStopItem aTStop( *(SvxTabStopItem*)pItem );

        long nOffset = rLR.GetTxtLeft();
        if( nOffset )
        {
            // Tabs anpassen !!
            SvxTabStop* pTabs = (SvxTabStop*)aTStop.GetStart();
            for( USHORT n = aTStop.Count(); n; --n, ++pTabs)
                if( SVX_TAB_ADJUST_DEFAULT != pTabs->GetAdjustment() )
                    pTabs->GetTabPos() -= nOffset;

            // negativer Einzug, dann auf 0 Pos einen Tab setzen
            if( rLR.GetTxtFirstLineOfst() < 0 )
                aTStop.Insert( SvxTabStop() );
        }

        if( !aTStop.Count() )
        {
            const SvxTabStopItem& rDflt = (const SvxTabStopItem&)rSet.
                                GetPool()->GetDefaultItem(RES_PARATR_TABSTOP);
            if( rDflt.Count() )
                aTStop.Insert( &rDflt, 0 );
        }
        rSet.Put( aTStop );
    }
    else if( SFX_ITEM_SET == rSet.GetItemState( RES_LR_SPACE, FALSE, &pItem )
            && ((SvxLRSpaceItem*)pItem)->GetTxtFirstLineOfst() < 0 )
    {
        // negativer Einzug, dann auf 0 Pos einen Tab setzen
        rSet.Put( SvxTabStopItem( 1, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP ));
    }

    // NumRules anpassen
    if( !bStyleTabValid &&
        SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_NUMRULE, FALSE, &pItem ))
    {
        // dann steht im Namen nur ein Verweis in das ListArray
        SwNumRule* pRule = GetNumRuleOfListNo( ((SwNumRuleItem*)pItem)->
                                                GetValue().ToInt32() );
        if( pRule )
            rSet.Put( SwNumRuleItem( pRule->GetName() ));
        else
            rSet.ClearItem( RES_PARATR_NUMRULE );

    }


/*
 ????????????????????????????????????????????????????????????????????
 ?? muss die LineSpacing Hoehe 200Twip betragen ??
 ?? in rtfitem.hxx wird es auf 0 defaultet. Wenn ja, dann muss hier
 ?? ein neues Item gesetzt werden!!!!
 ????????????????????????????????????????????????????????????????????

    // LineSpacing korrigieren
    if( SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_LINESPACING, FALSE, &pItem ))
    {
        const SvxLineSpacingItem* pLS = (const SvxLineSpacingItem*)pItem;
        SvxLineSpacingItem aNew;

        aNew.SetInterLineSpace( pLS->GetInterLineSpace() );
        aNew.GetLineSpaceRule() = pLS->GetLineSpaceRule();
        aNew.SetPropLineSpace( pLS->GetPropLineSpace() );
        aNew.GetInterLineSpaceRule() = pLS->GetInterLineSpaceRule();

        rSet.Put( aNew );
    }
?????????????????????????????????????????????????????????????????? */

}


SwTxtFmtColl* SwRTFParser::MakeColl(const String& rName, USHORT nPos,
    BYTE nOutlineLevel, bool& rbCollExist)
{
	if( BYTE(-1) == nOutlineLevel )
		//nOutlineLevel = NO_NUMBERING;
		nOutlineLevel = MAXLEVEL;//#outline level,zhaojianwei

	rbCollExist = false;
    SwTxtFmtColl* pColl;
    String aNm( rName );
    if( !aNm.Len() )
    {
        ASSERT(!this, "not a bug, but I (cmc) want to see an example of this");
        if( !nPos )
        {
            pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false );
			//pColl->SetOutlineLevel( nOutlineLevel );		//#outline level,removed by zhaojianwei
			if(nOutlineLevel < MAXLEVEL )							//->add by zhaojianwei	
				pColl->AssignToListLevelOfOutlineStyle( nOutlineLevel );
			else
				pColl->DeleteAssignmentToListLevelOfOutlineStyle();	//<-end,zhaojianwei
			return pColl;
		}

		// erzeuge einen Namen
		aNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "NoName(" ));
		aNm += String::CreateFromInt32( nPos );
		aNm += ')';
	}
    ww::sti eSti = ww::GetCanonicalStiFromEnglishName(rName);
    sw::util::ParaStyleMapper::StyleResult aResult =
        maParaStyleMapper.GetStyle(rName, eSti);
    pColl = aResult.first;
    rbCollExist = aResult.second;
    if (IsNewDoc() && rbCollExist)
    {
        // --> OD 2007-01-25 #i73790# - method renamed
        pColl->ResetAllFmtAttr();
        // <--
        rbCollExist = false;
    }

    if (!rbCollExist)
    	//pColl->SetOutlineLevel( nOutlineLevel );	//#outline level,removed by zhaojianwei
		if(nOutlineLevel < MAXLEVEL)						//->add by zhaojianwei
			pColl->AssignToListLevelOfOutlineStyle( nOutlineLevel );
		else
			pColl->DeleteAssignmentToListLevelOfOutlineStyle();	//<-end,zhaojianwei

    return pColl;
}

SwCharFmt* SwRTFParser::MakeCharFmt(const String& rName, USHORT nPos,
                                    int& rbCollExist)
{
    rbCollExist = FALSE;
    SwCharFmt* pFmt;
    String aNm( rName );
    if( !aNm.Len() )
    {
        ASSERT(!this, "not a bug, but I (cmc) want to see an example of this");
        aNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "NoName(" ));
        aNm += String::CreateFromInt32( nPos );
        aNm += ')';
    }

    ww::sti eSti = ww::GetCanonicalStiFromEnglishName(rName);
    sw::util::CharStyleMapper::StyleResult aResult =
        maCharStyleMapper.GetStyle(rName, eSti);
    pFmt = aResult.first;
    rbCollExist = aResult.second;
    if (IsNewDoc() && rbCollExist)
    {
        // --> OD 2007-01-25 #i73790# - method renamed
        pFmt->ResetAllFmtAttr();
        // <--
        rbCollExist = false;
    }
    return pFmt;
}

void SwRTFParser::SetStyleAttr( SfxItemSet& rCollSet,
                                const SfxItemSet& rStyleSet,
                                const SfxItemSet& rDerivedSet )
{
    rCollSet.Put( rStyleSet );
    if( rDerivedSet.Count() )
    {
        // suche alle Attribute, die neu gesetzt werden:
        const SfxPoolItem* pItem;
        SfxItemIter aIter( rDerivedSet );
        USHORT nWhich = aIter.GetCurItem()->Which();
        while( TRUE )
        {
            switch( rStyleSet.GetItemState( nWhich, FALSE, &pItem ) )
            {
            case SFX_ITEM_DEFAULT:
                // auf default zuruecksetzen
                if( RES_FRMATR_END > nWhich )
                    rCollSet.Put( rCollSet.GetPool()->GetDefaultItem( nWhich ));
                break;
            case SFX_ITEM_SET:
                if( *pItem == *aIter.GetCurItem() )     // gleiches Attribut?
                    // definition kommt aus dem Parent
                    rCollSet.ClearItem( nWhich );       // loeschen
                break;
            }

            if( aIter.IsAtEnd() )
                break;
            nWhich = aIter.NextItem()->Which();
        }
    }
    // und jetzt noch auf unsere Werte abgleichen
    SetSwgValues( rCollSet );
}

SwTxtFmtColl* SwRTFParser::MakeStyle( USHORT nNo, const SvxRTFStyleType& rStyle)
{
    bool bCollExist;
    SwTxtFmtColl* pColl = MakeColl( rStyle.sName, USHORT(nNo),
        rStyle.nOutlineNo, bCollExist);
    aTxtCollTbl.Insert( nNo, pColl );

    // in bestehendes Dok einfuegen, dann keine Ableitung usw. setzen
    if( bCollExist )
        return pColl;

    USHORT nStyleNo = rStyle.nBasedOn;
    if( rStyle.bBasedOnIsSet && nStyleNo != nNo )
    {
        SvxRTFStyleType* pDerivedStyle = GetStyleTbl().Get( nStyleNo );
        SwTxtFmtColl* pDerivedColl = aTxtCollTbl.Get( nStyleNo );
        if( !pDerivedColl )         // noch nicht vorhanden, also anlegen
        {
            // ist die ueberhaupt als Style vorhanden ?
            pDerivedColl = pDerivedStyle
                    ? MakeStyle( nStyleNo, *pDerivedStyle )
                    : pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false );
        }

        if( pColl == pDerivedColl )
            ((SfxItemSet&)pColl->GetAttrSet()).Put( rStyle.aAttrSet );
        else
        {
            pColl->SetDerivedFrom( pDerivedColl );

            // setze die richtigen Attribute
            const SfxItemSet* pDerivedSet;
            if( pDerivedStyle )
                pDerivedSet = &pDerivedStyle->aAttrSet;
            else
                pDerivedSet = &pDerivedColl->GetAttrSet();

            SetStyleAttr( (SfxItemSet&)pColl->GetAttrSet(),
                            rStyle.aAttrSet, *pDerivedSet );
        }
    }
    else
        ((SfxItemSet&)pColl->GetAttrSet()).Put( rStyle.aAttrSet );


    nStyleNo = rStyle.nNext;
    if( nStyleNo != nNo )
    {
        SwTxtFmtColl* pNext = aTxtCollTbl.Get( nStyleNo );
        if( !pNext )            // noch nicht vorhanden, also anlegen
        {
            // ist die ueberhaupt als Style vorhanden ?
            SvxRTFStyleType* pMkStyle = GetStyleTbl().Get( nStyleNo );
            pNext = pMkStyle
                    ? MakeStyle( nStyleNo, *pMkStyle )
                    : pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false );
        }
        pColl->SetNextTxtFmtColl( *pNext );
    }
    return pColl;
}

SwCharFmt* SwRTFParser::MakeCharStyle( USHORT nNo, const SvxRTFStyleType& rStyle )
{
    int bCollExist;
    SwCharFmt* pFmt = MakeCharFmt( rStyle.sName, USHORT(nNo), bCollExist );
    aCharFmtTbl.Insert( nNo, pFmt );

    // in bestehendes Dok einfuegen, dann keine Ableitung usw. setzen
    if( bCollExist )
        return pFmt;

    USHORT nStyleNo = rStyle.nBasedOn;
    if( rStyle.bBasedOnIsSet && nStyleNo != nNo )
    {
        SvxRTFStyleType* pDerivedStyle = GetStyleTbl().Get( nStyleNo );
        SwCharFmt* pDerivedFmt = aCharFmtTbl.Get( nStyleNo );
        if( !pDerivedFmt )          // noch nicht vorhanden, also anlegen
        {
            // ist die ueberhaupt als Style vorhanden ?
            pDerivedFmt = pDerivedStyle
                    ? MakeCharStyle( nStyleNo, *pDerivedStyle )
                    : pDoc->GetDfltCharFmt();
        }

        if( pFmt == pDerivedFmt )
            ((SfxItemSet&)pFmt->GetAttrSet()).Put( rStyle.aAttrSet );
        else
        {
            pFmt->SetDerivedFrom( pDerivedFmt );

            // setze die richtigen Attribute
            const SfxItemSet* pDerivedSet;
            if( pDerivedStyle )
                pDerivedSet = &pDerivedStyle->aAttrSet;
            else
                pDerivedSet = &pDerivedFmt->GetAttrSet();

            SetStyleAttr( (SfxItemSet&)pFmt->GetAttrSet(),
                            rStyle.aAttrSet, *pDerivedSet );
        }
    }
    else
        ((SfxItemSet&)pFmt->GetAttrSet()).Put( rStyle.aAttrSet );

    return pFmt;
}

// loesche den letzten Node (Tabelle/Fly/Ftn/..)
void SwRTFParser::DelLastNode()
{
    // sollte der letze Node leer sein, dann loesche ihn
    // (\par heisst ja Absatzende und nicht neuer Absatz!)

    if( !pPam->GetPoint()->nContent.GetIndex() )
    {
        ULONG nNodeIdx = pPam->GetPoint()->nNode.GetIndex();
        SwCntntNode* pCNd = pDoc->GetNodes()[ nNodeIdx ]->GetCntntNode();
        // paragraphs with page break information are not empty! see #117914# topic 1)
        if(const SfxPoolItem* pItem=&(pCNd->GetAttr( RES_PAGEDESC, FALSE)))
        {
            SwFmtPageDesc* pPageDescItem = ((SwFmtPageDesc*)pItem);
            if (pPageDescItem->GetPageDesc()!=NULL)
            return;
        }

        if( pCNd && pCNd->StartOfSectionIndex()+2 <
            pCNd->EndOfSectionIndex() )
        {
            if( GetAttrStack().Count() )
            {
                // Attribut Stack-Eintraege, muessen ans Ende des vorherigen
                // Nodes verschoben werden.
                BOOL bMove = FALSE;
                for( USHORT n = GetAttrStack().Count(); n; )
                {
                    SvxRTFItemStackType* pStkEntry = (SvxRTFItemStackType*)
                                                    GetAttrStack()[ --n ];
                    if( nNodeIdx == pStkEntry->GetSttNode().GetIdx() )
                    {
                        if( !bMove )
                        {
                            pPam->Move( fnMoveBackward );
                            bMove = TRUE;
                        }
                        pStkEntry->SetStartPos( SwxPosition( pPam ) );
                    }
                }
                if( bMove )
                    pPam->Move( fnMoveForward );
            }
            pPam->GetPoint()->nContent.Assign( 0, 0 );
            pPam->SetMark();
            pPam->DeleteMark();

            pDoc->GetNodes().Delete( pPam->GetPoint()->nNode );
        }
    }
}

    // fuer Tokens, die im ReadAttr nicht ausgewertet werden
void SwRTFParser::UnknownAttrToken( int nToken, SfxItemSet* pSet )
{
    switch( nToken )
    {
    case RTF_INTBL:
        {
            if( !pTableNode )           // Tabelle nicht mehr vorhanden ?
                NewTblLine();           // evt. Line copieren
            else
            {
                static int _do=0; //$flr See #117881# for explanation.
                // Crsr nicht mehr in der Tabelle ?
                if( !pPam->GetNode()->FindTableNode() && _do )
                {
                    ULONG nOldPos = pPam->GetPoint()->nNode.GetIndex();

                    // dann wieder in die letzte Box setzen
                    // (kann durch einlesen von Flys geschehen!)
                    pPam->GetPoint()->nNode = *pTableNode->EndOfSectionNode();
                    pPam->Move( fnMoveBackward );

                    // alle Attribute, die schon auf den nachfolgen zeigen
                    // auf die neue Box umsetzen !!
                    SvxRTFItemStack& rAttrStk = GetAttrStack();
                    const SvxRTFItemStackType* pStk;
                    for( USHORT n = 0; n < rAttrStk.Count(); ++n )
                        if( ( pStk = rAttrStk[ n ])->GetSttNodeIdx() == nOldPos &&
                            !pStk->GetSttCnt() )
                            ((SvxRTFItemStackType*)pStk)->SetStartPos( SwxPosition( pPam ) );
                }
            }
        }
        break;

    case RTF_PAGEBB:
        {
            pSet->Put( SvxFmtBreakItem( SVX_BREAK_PAGE_BEFORE, RES_BREAK ));
        }
        break;

    case RTF_PGBRK:
        {
            pSet->Put( SvxFmtBreakItem( 1 == nTokenValue ?
                                SVX_BREAK_PAGE_BOTH : SVX_BREAK_PAGE_AFTER, RES_BREAK ));
        }
        break;

    case RTF_PGDSCNO:
        if( IsNewDoc() && bSwPageDesc &&
            USHORT(nTokenValue) < pDoc->GetPageDescCnt() )
        {
            const SwPageDesc* pPgDsc = &const_cast<const SwDoc *>(pDoc)
                ->GetPageDesc( (USHORT)nTokenValue );
            pDoc->Insert( *pPam, SwFmtPageDesc( pPgDsc ), 0);
        }
        break;
    case RTF_CS:
        {
            SwCharFmt* pFmt = aCharFmtTbl.Get( nTokenValue );
            if( pFmt )
                pSet->Put( SwFmtCharFmt( pFmt ));
        }
        break;

    case RTF_LS:
        if( -1 != nTokenValue )
        {
            if( bStyleTabValid )
            {
                // dann ist auch die ListTabelle gueltig, also suche die
                // enstprechende NumRule
                SwNumRule* pRule = GetNumRuleOfListNo( nTokenValue );
                if( pRule )
                    pSet->Put( SwNumRuleItem( pRule->GetName() ));

                if( SFX_ITEM_SET != pSet->GetItemState( FN_PARAM_NUM_LEVEL, FALSE ))
                    pSet->Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, 0 ));
            }
            else
            {
                // wir sind in der Style-Definitions - Phase. Der Name
                // wird dann spaeter umgesetzt
                                //#117891# pSet->Put( SwNumRuleItem( String::CreateFromInt32( nTokenValue )));
            }

        }
        break;

    case RTF_ILVL:
    case RTF_SOUTLVL:
        {
            BYTE nLevel = MAXLEVEL <= nTokenValue ? MAXLEVEL - 1
                                                  : BYTE( nTokenValue );
            pSet->Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, nLevel ));
        }
        break;

/*
    case RTF_SBYS:
    case RTF_EXPND:
    case RTF_KEEP:
    case RTF_KEEPN:
*/

    }
}

void SwRTFParser::ReadInfo( const sal_Char* pChkForVerNo )
{
sal_Char __READONLY_DATA aChkForVerNo[] = "StarWriter";

    // falls nicht schon was vorgegeben wurde, setzen wir unseren Namen
    // rein. Wenn das im Kommentar match, wird im Parser die VersionNummer
    // gelesen und gesetzt
    if( !pChkForVerNo )
        pChkForVerNo = aChkForVerNo;

    SvxRTFParser::ReadInfo( pChkForVerNo );
}

void SwRTFParser::ReadUserProperties()
{
    // For now we don't support user properties but at least the parser is here.
    // At the moment it just swallows the tokens to prevent them being displayed
    int nNumOpenBrakets = 1, nToken;

    while( nNumOpenBrakets && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case '}':
             --nNumOpenBrakets;
             break;
        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                else if( RTF_UNKNOWNCONTROL != GetNextToken() )
                    nToken = SkipToken( -2 );
                else
                {
                    // gleich herausfiltern
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;
                }
                ++nNumOpenBrakets;
            }
            break;

        case RTF_PROPNAME:
            SkipGroup();
            break;

        case RTF_PROPTYPE:
            break;

        case RTF_STATICVAL:
            SkipGroup();
             break;

//      default:
        }
    }

    SkipToken( -1 );
}


#ifdef USED
void SwRTFParser::SaveState( int nToken )
{
    SvxRTFParser::SaveState( nToken );
}

void SwRTFParser::RestoreState()
{
    SvxRTFParser::RestoreState();
}
#endif

#endif	// SUPD != 310

/**/

BookmarkPosition::BookmarkPosition(const SwPaM &rPaM)
    : maMkNode(rPaM.GetMark()->nNode),
    mnMkCntnt(rPaM.GetMark()->nContent.GetIndex())
{
}

BookmarkPosition::BookmarkPosition(const BookmarkPosition &rEntry)
    : maMkNode(rEntry.maMkNode), mnMkCntnt(rEntry.mnMkCntnt)
{
}

bool BookmarkPosition::operator==(const BookmarkPosition rhs)
{
    return(maMkNode.GetIndex() == rhs.maMkNode.GetIndex() && mnMkCntnt == rhs.mnMkCntnt);
}

ULONG SwNodeIdx::GetIdx() const
{
    return aIdx.GetIndex();
}

SvxNodeIdx* SwNodeIdx::Clone() const
{
    return new SwNodeIdx( aIdx );
}

SvxPosition* SwxPosition::Clone() const
{
    return new SwxPosition( pPam );
}

SvxNodeIdx* SwxPosition::MakeNodeIdx() const
{
    return new SwNodeIdx( pPam->GetPoint()->nNode );
}

ULONG   SwxPosition::GetNodeIdx() const
{
    return pPam->GetPoint()->nNode.GetIndex();
}

xub_StrLen SwxPosition::GetCntIdx() const
{
    return pPam->GetPoint()->nContent.GetIndex();
}

#if SUPD == 310

/// Glue class to call RtfImport as an internal filter, needed by copy&paste support.
class SwRTFReader : public Reader
{
#ifdef USE_JAVA
    virtual ULONG Read( SwDoc &, const String& rBaseURL, SwPaM &,const String &) SAL_OVERRIDE;
#else	// USE_JAVA
    virtual sal_uLong Read( SwDoc &, const OUString& rBaseURL, SwPaM &,const OUString &) SAL_OVERRIDE;
#endif	// USE_JAVA
};

#ifdef USE_JAVA
ULONG SwRTFReader::Read( SwDoc &rDoc, const String& /*rBaseURL*/, SwPaM& rPam, const String &)
#else	// USE_JAVA
sal_uLong SwRTFReader::Read( SwDoc &rDoc, const OUString& /*rBaseURL*/, SwPaM& rPam, const OUString &)
#endif	// USE_JAVA
{
    if (!pStrm)
        return ERR_SWG_READ_ERROR;

    // We want to work in an empty paragraph.
    // Step 1: XTextRange will be updated when content is inserted, so we know
    // the end position.
    const uno::Reference<text::XTextRange> xInsertPosition =
#ifdef USE_JAVA
        SwXTextRange::CreateTextRangeFromPosition(&rDoc, *rPam.GetPoint(), 0);
#else	// USE_JAVA
        SwXTextRange::CreateXTextRange(rDoc, *rPam.GetPoint(), 0);
#endif	// USE_JAVA
    SwNodeIndex *pSttNdIdx = new SwNodeIndex(rDoc.GetNodes());
    const SwPosition* pPos = rPam.GetPoint();

    // Step 2: Split once and remember the node that has been split.
    rDoc.SplitNode( *pPos, false );
    *pSttNdIdx = pPos->nNode.GetIndex()-1;

    // Step 3: Split again.
    rDoc.SplitNode( *pPos, false );

    // Step 4: Insert all content into the new node
    rPam.Move( fnMoveBackward );
    rDoc.SetTxtFmtColl
        ( rPam, rDoc.GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ) );

    SwDocShell *pDocShell(rDoc.GetDocShell());
    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<uno::XInterface> xInterface(xMultiServiceFactory->createInstance(
        "com.sun.star.comp.Writer.RtfFilter"), uno::UNO_QUERY_THROW);

    uno::Reference<document::XImporter> xImporter(xInterface, uno::UNO_QUERY_THROW);
    uno::Reference<lang::XComponent> xDstDoc(pDocShell->GetModel(), uno::UNO_QUERY_THROW);
    xImporter->setTargetDocument(xDstDoc);

    const uno::Reference<text::XTextRange> xInsertTextRange =
#ifdef USE_JAVA
        SwXTextRange::CreateTextRangeFromPosition(&rDoc, *rPam.GetPoint(), 0);
#else	// USE_JAVA
        SwXTextRange::CreateXTextRange(rDoc, *rPam.GetPoint(), 0);
#endif	// USE_JAVA

    uno::Reference<document::XFilter> xFilter(xInterface, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aDescriptor(3);
    aDescriptor[0].Name = "InputStream";
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStrm));
    aDescriptor[0].Value <<= xStream;
    aDescriptor[1].Name = "InsertMode";
    aDescriptor[1].Value <<= sal_True;
    aDescriptor[2].Name = "TextInsertModeRange";
    aDescriptor[2].Value <<= xInsertTextRange;
#ifdef USE_JAVA
    ULONG ret(0);
#else	// USE_JAVA
    sal_uLong ret(0);
#endif	// USE_JAVA
    try {
        xFilter->filter(aDescriptor);
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("sw.rtf", "SwRTFReader::Read(): exception: " << e.Message);
        ret = ERR_SWG_READ_ERROR;
    }

    // Clean up the fake paragraphs.
    SwUnoInternalPaM aPam(rDoc);
#ifdef USE_JAVA
    SwXTextRange::XTextRangeToSwPaM(aPam, xInsertPosition);
#else	// USE_JAVA
    ::sw::XTextRangeToSwPaM(aPam, xInsertPosition);
#endif	// USE_JAVA
    if (pSttNdIdx->GetIndex())
    {
        // If we are in insert mode, join the split node that is in front
        // of the new content with the first new node. Or in other words:
        // Revert the first split node.
        SwTxtNode* pTxtNode = pSttNdIdx->GetNode().GetTxtNode();
        SwNodeIndex aNxtIdx( *pSttNdIdx );
        if( pTxtNode && pTxtNode->CanJoinNext( &aNxtIdx ) &&
                pSttNdIdx->GetIndex() + 1 == aNxtIdx.GetIndex() )
        {
            // If the PaM points to the first new node, move the PaM to the
            // end of the previous node.
            if( aPam.GetPoint()->nNode == aNxtIdx )
            {
                aPam.GetPoint()->nNode = *pSttNdIdx;
                aPam.GetPoint()->nContent.Assign( pTxtNode,
#ifdef USE_JAVA
                        pTxtNode->GetTxt().Len() );
#else	// USE_JAVA
                        pTxtNode->GetTxt().getLength() );
#endif	// USE_JAVA
            }
            // If the first new node isn't empty, convert  the node's text
            // attributes into hints. Otherwise, set the new node's
            // paragraph style at the previous (empty) node.
            SwTxtNode* pDelNd = aNxtIdx.GetNode().GetTxtNode();
#ifdef USE_JAVA
            if (pTxtNode->GetTxt().Len())
#else	// USE_JAVA
            if (pTxtNode->GetTxt().getLength())
#endif	// USE_JAVA
                pDelNd->FmtToTxtAttr( pTxtNode );
            else
                pTxtNode->ChgFmtColl( pDelNd->GetTxtColl() );
            pTxtNode->JoinNext();
        }
    }

    return ret;
}

extern "C" SAL_DLLPUBLIC_EXPORT Reader* SAL_CALL ImportRTF()
{
    return new SwRTFReader();
}

#endif	// SUPD == 310

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
