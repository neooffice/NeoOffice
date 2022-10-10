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

#include <hintids.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IShellCursorSupplier.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>
#include <redline.hxx>

SwUndoInserts::SwUndoInserts( SwUndoId nUndoId, const SwPaM& rPam )
    : SwUndo( nUndoId ), SwUndRng( rPam ),
    pTxtFmtColl( 0 ), pLastNdColl(0), pFrmFmts( 0 ), pRedlData( 0 ),
    bSttWasTxtNd( true ), nNdDiff( 0 ), nSetPos( 0 )
{
    pHistory = new SwHistory;
    SwDoc* pDoc = (SwDoc*)rPam.GetDoc();

    SwTxtNode* pTxtNd = rPam.GetPoint()->nNode.GetNode().GetTxtNode();
    if( pTxtNd )
    {
        pTxtFmtColl = pTxtNd->GetTxtColl();
        pHistory->CopyAttr( pTxtNd->GetpSwpHints(), nSttNode,
                            0, pTxtNd->GetTxt().getLength(), false );
        if( pTxtNd->HasSwAttrSet() )
            pHistory->CopyFmtAttr( *pTxtNd->GetpSwAttrSet(), nSttNode );

#ifdef NO_LIBO_BUG_108124_FIX
        if( !nSttCntnt )    // than take the Flys along
#else	// NO_LIBO_BUG_108124_FIX
        // We may have some flys anchored to paragraph where we inserting.
        // These flys will be saved in pFrameFormats array (only flys which exist BEFORE insertion!)
        // Then in SwUndoInserts::SetInsertRange the flys saved in pFrameFormats will NOT create Undos.
        // m_FlyUndos will only be filled with newly inserted flys.

        const size_t nArrLen = pDoc->GetSpzFrmFmts()->size();
        for( size_t n = 0; n < nArrLen; ++n )
#endif	// NO_LIBO_BUG_108124_FIX
        {
#ifdef NO_LIBO_BUG_108124_FIX
            const size_t nArrLen = pDoc->GetSpzFrmFmts()->size();
            for( size_t n = 0; n < nArrLen; ++n )
#else	// NO_LIBO_BUG_108124_FIX
            SwFrmFmt* pFmt = (*pDoc->GetSpzFrmFmts())[n];
            SwFmtAnchor const*const  pAnchor = &pFmt->GetAnchor();
            const SwPosition* pAPos = pAnchor->GetCntntAnchor();
            if (pAPos &&
                (pAnchor->GetAnchorId() == RndStdIds::FLY_AT_PARA) &&
                 nSttNode == pAPos->nNode.GetIndex() )
#endif	// NO_LIBO_BUG_108124_FIX
            {
#ifdef NO_LIBO_BUG_108124_FIX
                SwFrmFmt* pFmt = (*pDoc->GetSpzFrmFmts())[n];
                SwFmtAnchor const*const  pAnchor = &pFmt->GetAnchor();
                const SwPosition* pAPos = pAnchor->GetCntntAnchor();
                if (pAPos &&
                    (pAnchor->GetAnchorId() == FLY_AT_PARA) &&
                     nSttNode == pAPos->nNode.GetIndex() )
                {
                    if( !pFrmFmts )
                        pFrmFmts = new std::vector<SwFrmFmt*>;
                    pFrmFmts->push_back( pFmt );
                }
#else	// NO_LIBO_BUG_108124_FIX
                if( !pFrmFmts )
                    pFrmFmts = new std::vector<SwFrmFmt*>;
                pFrmFmts->push_back( pFmt );
#endif	// NO_LIBO_BUG_108124_FIX
            }
        }
    }
    // consider Redline
    if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( nsRedlineType_t::REDLINE_INSERT, pDoc->getIDocumentRedlineAccess().GetRedlineAuthor() );
        SetRedlineMode( pDoc->getIDocumentRedlineAccess().GetRedlineMode() );
    }
}

#ifdef NO_LIBO_BUG_108124_FIX
// set destination after reading input
#else	// NO_LIBO_BUG_108124_FIX
// This method does two things:
// 1. Adjusts SwUndoRng members, required for Undo.
//  Members are:
//  SwUndoRng::nSttNode - all nodes starting from this node will be deleted during Undo (in SwUndoInserts::UndoImpl)
//  SwUndoRng::nSttContent - corresponding content index in SwUndoRng::nSttNode
//  SwUndoRng::nEndNode - end node for deletion
//  SwUndoRng::nEndContent - end content index
// All these members are filled in during construction of SwUndoInserts instance, and can be adjusted using this method
//
// 2. Fills in m_FlyUndos array with flys anchored ONLY to first and last paragraphs (first == rPam.Start(), last == rPam.End())
//  Flys, anchored to any paragraph, but not first and last, are handled by DelContentIndex (see SwUndoInserts::UndoImpl) and are not stored in m_FlyUndos.
#endif	// NO_LIBO_BUG_108124_FIX

void SwUndoInserts::SetInsertRange( const SwPaM& rPam, bool bScanFlys,
                                    bool bSttIsTxtNd )
{
    const SwPosition* pTmpPos = rPam.End();
    nEndNode = pTmpPos->nNode.GetIndex();
    nEndCntnt = pTmpPos->nContent.GetIndex();
    if( rPam.HasMark() )
    {
        if( pTmpPos == rPam.GetPoint() )
            pTmpPos = rPam.GetMark();
        else
            pTmpPos = rPam.GetPoint();

        nSttNode = pTmpPos->nNode.GetIndex();
        nSttCntnt = pTmpPos->nContent.GetIndex();

        if( !bSttIsTxtNd )      // if a table selection is added ...
        {
            ++nSttNode;         // ... than the CopyPam is not fully correct
            bSttWasTxtNd = false;
        }
    }

#ifdef NO_LIBO_BUG_108124_FIX
    if( bScanFlys && !nSttCntnt )
#else	// NO_LIBO_BUG_108124_FIX
    // Fill m_FlyUndos with flys anchored to first and last paragraphs

    if( bScanFlys)
#endif	// NO_LIBO_BUG_108124_FIX
    {
        // than collect all new Flys
        SwDoc* pDoc = (SwDoc*)rPam.GetDoc();
        const size_t nArrLen = pDoc->GetSpzFrmFmts()->size();
        for( size_t n = 0; n < nArrLen; ++n )
        {
            SwFrmFmt* pFmt = (*pDoc->GetSpzFrmFmts())[n];
            SwFmtAnchor const*const pAnchor = &pFmt->GetAnchor();
            SwPosition const*const pAPos = pAnchor->GetCntntAnchor();
            if (pAPos &&
                (pAnchor->GetAnchorId() == FLY_AT_PARA) &&
#ifdef NO_LIBO_BUG_108124_FIX
                nSttNode == pAPos->nNode.GetIndex() )
#else	// NO_LIBO_BUG_108124_FIX
                (nSttNode == pAPos->nNode.GetIndex() || nEndNode == pAPos->nNode.GetIndex()))
#endif	// NO_LIBO_BUG_108124_FIX
            {
                std::vector<SwFrmFmt*>::iterator it;
                if( !pFrmFmts ||
                    pFrmFmts->end() == ( it = std::find( pFrmFmts->begin(), pFrmFmts->end(), pFmt ) ) )
                {
                    ::boost::shared_ptr<SwUndoInsLayFmt> const pFlyUndo(
                        new SwUndoInsLayFmt(pFmt, 0, 0));
                    m_FlyUndos.push_back(pFlyUndo);
                }
                else
                    pFrmFmts->erase( it );
            }
        }
        delete pFrmFmts, pFrmFmts = 0;
    }
}

SwUndoInserts::~SwUndoInserts()
{
    if (m_pUndoNodeIndex) // delete also the section from UndoNodes array
    {
        // Insert saves content in IconSection
        SwNodes& rUNds = m_pUndoNodeIndex->GetNodes();
        rUNds.Delete(*m_pUndoNodeIndex,
            rUNds.GetEndOfExtras().GetIndex() - m_pUndoNodeIndex->GetIndex());
        m_pUndoNodeIndex.reset();
    }
    delete pFrmFmts;
    delete pRedlData;
}

#ifndef NO_LIBO_BUG_108124_FIX
// Undo Insert operation
//  It's important to note that Undo stores absolute node indexes. I.e. if during insertion, you insert nodes 31 to 33,
//  during Undo nodes with indices from 31 to 33 will be deleted. Undo doesn't check that nodes 31 to 33 are the same nodes which were inserted.
//  It just deletes them.
//  This may seem as bad programming practice, but Undo actions are strongly ordered. If you change your document in some way, a new Undo action is added.
//  During Undo most recent actions will be executed first. So during execution of particular Undo action indices will be correct.
//  But storing absolute indices leads to crashes if some action in Undo fails to roll back some modifications.

//  Has following main steps:
//  1. DelContentIndex to delete footnotes, flys, bookmarks (see comment for this function)
//     Deleted flys are stored in pHistory array.
//     First and last paragraphs flys are handled later in this function! They are not deleted by DelContentIndex!
//     For flys anchored to last paragraph, DelContentIndex re-anchors them to the last paragraph that will remain after Undo.
//     This is not fully correct, as everything between nSttNode and nEndNode should be deleted (these nodes marks range of inserted nodes).
//     But due to bug in paste (probably there), during paste all flys are anchored to last paragraph (see https://bugs.documentfoundation.org/show_bug.cgi?id=94225#c38).
//     So they should be re-anchored.
//  2. MoveToUndoNds moves nodes to Undo nodes array and removes them from document.
//  3. m_FlyUndos removes flys anchored to first and last paragraph in Undo range. This array may be empty.
//  4. Lastly (starting from if(pTextNode)), text from last paragraph is joined to last remaining paragraph and FormatColl for last paragraph is restored.
//     Format coll for last paragraph is removed during execution of UndoImpl
#endif	// !NO_LIBO_BUG_108124_FIX

void SwUndoInserts::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pPam = & AddUndoRedoPaM(rContext);

    if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
        pDoc->getIDocumentRedlineAccess().DeleteRedline( *pPam, true, USHRT_MAX );

    // if Point and Mark are different text nodes so a JoinNext has to be done
    bool bJoinNext = nSttNode != nEndNode &&
                pPam->GetMark()->nNode.GetNode().GetTxtNode() &&
                pPam->GetPoint()->nNode.GetNode().GetTxtNode();

    // Is there any content? (loading from template does not have content)
    if( nSttNode != nEndNode || nSttCntnt != nEndCntnt )
    {
        if( nSttNode != nEndNode )
        {
            SwTxtNode* pTxtNd = pDoc->GetNodes()[ nEndNode ]->GetTxtNode();
            if (pTxtNd && pTxtNd->GetTxt().getLength() == nEndCntnt)
                pLastNdColl = pTxtNd->GetTxtColl();
        }

        RemoveIdxFromRange( *pPam, false );
        SetPaM(*pPam);

        // are there Footnotes or CntntFlyFrames in text?
        nSetPos = pHistory->Count();
        nNdDiff = pPam->GetMark()->nNode.GetIndex();
        DelCntntIndex( *pPam->GetMark(), *pPam->GetPoint() );
        nNdDiff -= pPam->GetMark()->nNode.GetIndex();

        if( *pPam->GetPoint() != *pPam->GetMark() )
        {
            m_pUndoNodeIndex.reset(
                    new SwNodeIndex(pDoc->GetNodes().GetEndOfContent()));
            MoveToUndoNds(*pPam, m_pUndoNodeIndex.get());

            if( !bSttWasTxtNd )
                pPam->Move( fnMoveBackward, fnGoCntnt );
        }
    }

    if (m_FlyUndos.size())
    {
        sal_uLong nTmp = pPam->GetPoint()->nNode.GetIndex();
        for (size_t n = m_FlyUndos.size(); 0 < n; --n)
        {
            m_FlyUndos[ n-1 ]->UndoImpl(rContext);
        }
        nNdDiff += nTmp - pPam->GetPoint()->nNode.GetIndex();
    }

    SwNodeIndex& rIdx = pPam->GetPoint()->nNode;
    SwTxtNode* pTxtNode = rIdx.GetNode().GetTxtNode();
    if( pTxtNode )
    {
        if( !pTxtFmtColl ) // if 0 than it's no TextNode -> delete
        {
            SwNodeIndex aDelIdx( rIdx );
            ++rIdx;
            SwCntntNode* pCNd = rIdx.GetNode().GetCntntNode();
            pPam->GetPoint()->nContent.Assign( pCNd, pCNd ? pCNd->Len() : 0 );
            pPam->SetMark();
            pPam->DeleteMark();

            RemoveIdxRel( aDelIdx.GetIndex(), *pPam->GetPoint() );

            pDoc->GetNodes().Delete( aDelIdx, 1 );
        }
        else
        {
            if( bJoinNext && pTxtNode->CanJoinNext())
            {
                {
                    RemoveIdxRel( rIdx.GetIndex()+1, SwPosition( rIdx,
                        SwIndex( pTxtNode, pTxtNode->GetTxt().getLength() )));
                }
                pTxtNode->JoinNext();
            }
            // reset all text attributes in the paragraph!
            pTxtNode->RstTxtAttr( SwIndex(pTxtNode, 0), pTxtNode->Len(), 0, 0, true );

            pTxtNode->ResetAllAttr();

            if( USHRT_MAX != pDoc->GetTxtFmtColls()->GetPos( pTxtFmtColl ))
                pTxtFmtColl = (SwTxtFmtColl*)pTxtNode->ChgFmtColl( pTxtFmtColl );

            pHistory->SetTmpEnd( nSetPos );
            pHistory->TmpRollback( pDoc, 0, false );
        }
    }
}

#ifndef NO_LIBO_BUG_108124_FIX
// See SwUndoInserts::UndoImpl comments
// All actions here should be done in reverse order to what is done in SwUndoInserts::UndoImpl!
#endif	// !NO_LIBO_BUG_108124_FIX

void SwUndoInserts::RedoImpl(::sw::UndoRedoContext & rContext)
{
    // position cursor onto REDO section
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());
    SwDoc* pDoc = pPam->GetDoc();
    pPam->DeleteMark();
    pPam->GetPoint()->nNode = nSttNode - nNdDiff;
    SwCntntNode* pCNd = pPam->GetCntntNode();
    pPam->GetPoint()->nContent.Assign( pCNd, nSttCntnt );

    SwTxtFmtColl* pSavTxtFmtColl = pTxtFmtColl;
    if( pTxtFmtColl && pCNd && pCNd->IsTxtNode() )
        pSavTxtFmtColl = ((SwTxtNode*)pCNd)->GetTxtColl();

    pHistory->SetTmpEnd( nSetPos );

    // retrieve start position for rollback
    if( ( nSttNode != nEndNode || nSttCntnt != nEndCntnt ) && m_pUndoNodeIndex)
    {
        const bool bMvBkwrd = MovePtBackward( *pPam );

        // re-insert content again (first detach m_pUndoNodeIndex!)
        sal_uLong const nMvNd = m_pUndoNodeIndex->GetIndex();
        m_pUndoNodeIndex.reset();
        MoveFromUndoNds(*pDoc, nMvNd, *pPam->GetMark());
        if( bSttWasTxtNd )
            MovePtForward( *pPam, bMvBkwrd );
        pPam->Exchange();
    }

    if( USHRT_MAX != pDoc->GetTxtFmtColls()->GetPos( pTxtFmtColl ))
    {
        SwTxtNode* pTxtNd = pPam->GetMark()->nNode.GetNode().GetTxtNode();
        if( pTxtNd )
            pTxtNd->ChgFmtColl( pTxtFmtColl );
    }
    pTxtFmtColl = pSavTxtFmtColl;

    if( pLastNdColl && USHRT_MAX != pDoc->GetTxtFmtColls()->GetPos( pLastNdColl ) &&
        pPam->GetPoint()->nNode != pPam->GetMark()->nNode )
    {
        SwTxtNode* pTxtNd = pPam->GetPoint()->nNode.GetNode().GetTxtNode();
        if( pTxtNd )
            pTxtNd->ChgFmtColl( pLastNdColl );
    }

#ifdef NO_LIBO_BUG_108124_FIX
    for (size_t n = m_FlyUndos.size(); 0 < n; --n)
#else	// NO_LIBO_BUG_108124_FIX
    // tdf#108124 (10/25/2017)
    // During UNDO we call SwUndoInsLayFormat::UndoImpl in reverse order,
    //  firstly for m_FlyUndos[ m_FlyUndos.size()-1 ], etc.
    // As absolute node index of fly stored in SwUndoFlyBase::nNdPgPos we
    //  should recover from Undo in direct order (last should be recovered first)
    // During REDO we should recover Flys (Images) in direct order,
    //  firstly m_FlyUndos[0], then with m_FlyUndos[1] index, etc.

    for (size_t n = 0; m_FlyUndos.size() > n; ++n)
#endif	// NO_LIBO_BUG_108124_FIX
    {
#ifdef NO_LIBO_BUG_108124_FIX
        m_FlyUndos[ n-1 ]->RedoImpl(rContext);
#else	// NO_LIBO_BUG_108124_FIX
        m_FlyUndos[n]->RedoImpl(rContext);
#endif	// NO_LIBO_BUG_108124_FIX
    }

    pHistory->Rollback( pDoc, nSetPos );

    if( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
    {
        RedlineMode_t eOld = pDoc->getIDocumentRedlineAccess().GetRedlineMode();
        pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)( eOld & ~nsRedlineMode_t::REDLINE_IGNORE ));
        pDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( *pRedlData, *pPam ), true);
        pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
    }
    else if( !( nsRedlineMode_t::REDLINE_IGNORE & GetRedlineMode() ) &&
            !pDoc->getIDocumentRedlineAccess().GetRedlineTbl().empty() )
        pDoc->getIDocumentRedlineAccess().SplitRedline( *pPam );
}

void SwUndoInserts::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwPaM aPam( rContext.GetDoc().GetNodes().GetEndOfContent() );
    SetPaM( aPam );
    SwPaM & rRepeatPaM( rContext.GetRepeatPaM() );
    aPam.GetDoc()->getIDocumentContentOperations().CopyRange( aPam, *rRepeatPaM.GetPoint(), false );
}

SwUndoInsDoc::SwUndoInsDoc( const SwPaM& rPam )
    : SwUndoInserts( UNDO_INSDOKUMENT, rPam )
{
}

SwUndoCpyDoc::SwUndoCpyDoc( const SwPaM& rPam )
    : SwUndoInserts( UNDO_COPY, rPam )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
