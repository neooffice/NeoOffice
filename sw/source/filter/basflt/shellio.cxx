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
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <svl/urihelper.hxx>
#include <svl/fstathelper.hxx>
#include <unotools/moduleoptions.hxx>
#include <sfx2/docfile.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/paperinf.hxx>
#include <node.hxx>
#include <docary.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtpdsc.hxx>
#include <swtypes.hxx>
#include <shellio.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <pam.hxx>
#include <editsh.hxx>
#include <undobj.hxx>
#include <swundo.hxx>
#include <swtable.hxx>
#include <tblsel.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <fltini.hxx>
#include <docsh.hxx>
#include <redline.hxx>
#include <swerror.h>
#include <paratr.hxx>
#include <pausethreadstarting.hxx>

using namespace ::com::sun::star;

sal_uLong SwReader::Read( const Reader& rOptions )
{
    // copy variables
    Reader* po = (Reader*) &rOptions;
    po->pStrm = pStrm;
    po->pStg  = pStg;
    po->xStg  = xStg;
    po->bInsertMode = 0 != pCrsr;

    // if a Medium is selected, get its Stream
    if( 0 != (po->pMedium = pMedium ) &&
        !po->SetStrmStgPtr() )
    {
        po->SetReadUTF8( false );
        po->SetBlockMode( false );
        po->SetOrganizerMode( false );
        po->SetIgnoreHTMLComments( false );
        return ERR_SWG_FILE_FORMAT_ERROR;
    }

    sal_uLong nError = 0L;

    GetDoc();

    // while reading, do not call OLE-Modified
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    Link aOLELink( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    pDoc->SetInReading( true );
    pDoc->SetInXMLImport( 0 != dynamic_cast< XMLReader* >(po) );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    Link aOLELink( mxDoc->GetOle2Link() );
    mxDoc->SetOle2Link( Link() );

    mxDoc->SetInReading( true );
    mxDoc->SetInXMLImport( 0 != dynamic_cast< XMLReader* >(po) );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    SwPaM *pPam;
    if( pCrsr )
        pPam = pCrsr;
    else
    {
        // if the Reader was not called by a Shell, create a PaM ourselves
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SwNodeIndex nNode( pDoc->GetNodes().GetEndOfContent(), -1 );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SwNodeIndex nNode( mxDoc->GetNodes().GetEndOfContent(), -1 );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pPam = new SwPaM( nNode );
        // For Web documents the default template was set already by InitNew,
        // unless the filter is not HTML,
        // or a SetTemplateName was called in ConvertFrom.
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if( !pDoc->getIDocumentSettingAccess().get(IDocumentSettingAccess::HTML_MODE) || ReadHTML != po || !po->pTemplate  )
            po->SetTemplate( *pDoc );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if( !mxDoc->getIDocumentSettingAccess().get(IDocumentSettingAccess::HTML_MODE) || ReadHTML != po || !po->mxTemplate.is()  )
            po->SetTemplate( *mxDoc );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }

    // Pams are connected like rings; stop when we return to the 1st element
    SwPaM *pEnd = pPam;
    SwUndoInsDoc* pUndo = 0;

    bool bReadPageDescs = false;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    bool const bDocUndo = pDoc->GetIDocumentUndoRedo().DoesUndo();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    bool const bDocUndo = mxDoc->GetIDocumentUndoRedo().DoesUndo();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    bool bSaveUndo = bDocUndo && pCrsr;
    if( bSaveUndo )
    {
        // the reading of the page template cannot be undone!
        if( ( bReadPageDescs = po->aOpt.IsPageDescs() ) )
        {
            bSaveUndo = false;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pDoc->GetIDocumentUndoRedo().DelAllUndoObj();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mxDoc->GetIDocumentUndoRedo().DelAllUndoObj();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        }
        else
        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pDoc->GetIDocumentUndoRedo().ClearRedo();
            pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_INSDOKUMENT, NULL );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mxDoc->GetIDocumentUndoRedo().ClearRedo();
            mxDoc->GetIDocumentUndoRedo().StartUndo( UNDO_INSDOKUMENT, NULL );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        }
    }
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    pDoc->GetIDocumentUndoRedo().DoUndo(false);

    SwNodeIndex aSplitIdx( pDoc->GetNodes() );

    RedlineMode_t eOld = pDoc->getIDocumentRedlineAccess().GetRedlineMode();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mxDoc->GetIDocumentUndoRedo().DoUndo(false);

    SwNodeIndex aSplitIdx( mxDoc->GetNodes() );

    RedlineMode_t eOld = mxDoc->getIDocumentRedlineAccess().GetRedlineMode();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    RedlineMode_t ePostReadRedlineMode( nsRedlineMode_t::REDLINE_IGNORE );

    // Array of FlyFormats
    SwFrmFmts aFlyFrmArr;
    // only read templates? then ignore multi selection!
    bool bFmtsOnly = po->aOpt.IsFmtsOnly();

    while( true )
    {
        if( bSaveUndo )
            pUndo = new SwUndoInsDoc( *pPam );

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        SwPaM* pUndoPam = 0;
        if( bDocUndo || pCrsr )
        {
            // set Pam to the previous node, so that it is not also moved
            const SwNodeIndex& rTmp = pPam->GetPoint()->nNode;
            pUndoPam = new SwPaM( rTmp, rTmp, 0, -1 );
        }

        // store for now all Fly's
        if( pCrsr )
        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            std::copy(pDoc->GetSpzFrmFmts()->begin(),
                pDoc->GetSpzFrmFmts()->end(), std::back_inserter(aFlyFrmArr));
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            std::copy(mxDoc->GetSpzFrmFmts()->begin(),
                mxDoc->GetSpzFrmFmts()->end(), std::back_inserter(aFlyFrmArr));
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        }

        const sal_Int32 nSttCntnt = pPam->GetPoint()->nContent.GetIndex();

        // make sure the End position is correct for all Readers
        SwCntntNode* pCNd = pPam->GetCntntNode();
        sal_Int32 nEndCntnt = pCNd ? pCNd->Len() - nSttCntnt : 0;
        SwNodeIndex aEndPos( pPam->GetPoint()->nNode, 1 );

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        nError = po->Read( *pDoc, GetBaseURL(), *pPam, aFileName );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        nError = po->Read( *mxDoc, GetBaseURL(), *pPam, aFileName );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        // an ODF document may contain redline mode in settings.xml; save it!
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        ePostReadRedlineMode = pDoc->getIDocumentRedlineAccess().GetRedlineMode();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        ePostReadRedlineMode = mxDoc->getIDocumentRedlineAccess().GetRedlineMode();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        if( !IsError( nError ))     // set the End position already
        {
            aEndPos--;
            pCNd = aEndPos.GetNode().GetCntntNode();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            if( !pCNd && 0 == ( pCNd = pDoc->GetNodes().GoPrevious( &aEndPos ) ))
                pCNd = pDoc->GetNodes().GoNext( &aEndPos );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            if( !pCNd && 0 == ( pCNd = mxDoc->GetNodes().GoPrevious( &aEndPos ) ))
                pCNd = mxDoc->GetNodes().GoNext( &aEndPos );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

            pPam->GetPoint()->nNode = aEndPos;
            const sal_Int32 nLen = pCNd->Len();
            if( nLen < nEndCntnt )
                nEndCntnt = 0;
            else
                nEndCntnt = nLen - nEndCntnt;
            pPam->GetPoint()->nContent.Assign( pCNd, nEndCntnt );

            const SwStartNode* pTblBoxStart = pCNd->FindTableBoxStartNode();
            if ( pTblBoxStart )
            {
                SwTableBox* pBox = pTblBoxStart->GetTblBox();
                if ( pBox )
                {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    pDoc->ChkBoxNumFmt( *pBox, true );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    mxDoc->ChkBoxNumFmt( *pBox, true );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                }
            }
        }

        if( pCrsr )
        {
            *pUndoPam->GetMark() = *pPam->GetPoint();
            pUndoPam->GetPoint()->nNode++;
            SwNode& rNd = pUndoPam->GetNode();
            if( rNd.IsCntntNode() )
                pUndoPam->GetPoint()->nContent.Assign(
                                    (SwCntntNode*)&rNd, nSttCntnt );
            else
                pUndoPam->GetPoint()->nContent.Assign( 0, 0 );

            bool bChkHeaderFooter = rNd.FindHeaderStartNode() ||
                                   rNd.FindFooterStartNode();

            // search all new Fly's, and store them as individual Undo Objects
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            for( sal_uInt16 n = 0; n < pDoc->GetSpzFrmFmts()->size(); ++n )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            for( sal_uInt16 n = 0; n < mxDoc->GetSpzFrmFmts()->size(); ++n )
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                SwFrmFmt* pFrmFmt = (*pDoc->GetSpzFrmFmts())[ n ];
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                SwFrmFmt* pFrmFmt = (*mxDoc->GetSpzFrmFmts())[ n ];
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
                if( USHRT_MAX == aFlyFrmArr.GetPos( pFrmFmt) )
                {
                    SwPosition const*const pFrameAnchor(
                            rAnchor.GetCntntAnchor());
                    if  (   (FLY_AT_PAGE == rAnchor.GetAnchorId())
                        ||  (   pFrameAnchor
                            &&  (   (   (FLY_AT_PARA == rAnchor.GetAnchorId())
                                    &&  (   (pUndoPam->GetPoint()->nNode ==
                                             pFrameAnchor->nNode)
                                        ||  (pUndoPam->GetMark()->nNode ==
                                             pFrameAnchor->nNode)
                                        )
                                    )
                                // #i97570# also check frames anchored AT char
                                ||  (   (FLY_AT_CHAR == rAnchor.GetAnchorId())
                                    &&  !IsDestroyFrameAnchoredAtChar(
                                              *pFrameAnchor,
                                              *pUndoPam->GetPoint(),
#ifdef NO_LIBO_BUG_107975_FIX
                                              *pUndoPam->GetMark(),
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                              pDoc)
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                              mxDoc.get())
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
#else	// NO_LIBO_BUG_107975_FIX
                                              *pUndoPam->GetMark())
#endif	// NO_LIBO_BUG_107975_FIX
                                    )
                                )
                            )
                        )
                    {
                        if( bChkHeaderFooter &&
                            (FLY_AT_PARA == rAnchor.GetAnchorId()) &&
                            RES_DRAWFRMFMT == pFrmFmt->Which() )
                        {
                            // DrawObjects are not allowed in Headers/Footers!
                            pFrmFmt->DelFrms();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            pDoc->DelFrmFmt( pFrmFmt );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            mxDoc->DelFrmFmt( pFrmFmt );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            --n;
                        }
                        else
                        {
                            if( bSaveUndo )
                            {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                mxDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                // UGLY: temp. enable undo
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                pDoc->GetIDocumentUndoRedo().DoUndo(true);
                                pDoc->GetIDocumentUndoRedo().AppendUndo(
                                    new SwUndoInsLayFmt( pFrmFmt,0,0 ) );
                                pDoc->GetIDocumentUndoRedo().DoUndo(false);
                                pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                mxDoc->GetIDocumentUndoRedo().DoUndo(true);
                                mxDoc->GetIDocumentUndoRedo().AppendUndo(
                                    new SwUndoInsLayFmt( pFrmFmt,0,0 ) );
                                mxDoc->GetIDocumentUndoRedo().DoUndo(false);
                                mxDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            }
                            if( pFrmFmt->GetDepends() )
                            {
                                // Draw-Objects create a Frame when being inserted; thus delete them
                                pFrmFmt->DelFrms();
                            }

                            if (FLY_AT_PAGE == rAnchor.GetAnchorId())
                            {
                                if( !rAnchor.GetCntntAnchor() )
                                {
                                    pFrmFmt->MakeFrms();
                                }
                                else if( pCrsr )
                                {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                    pDoc->SetContainsAtPageObjWithContentAnchor( true );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                    mxDoc->SetContainsAtPageObjWithContentAnchor( true );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                }
                            }
                            else
                                pFrmFmt->MakeFrms();
                        }
                    }
                }
            }
            if( !aFlyFrmArr.empty() )
                aFlyFrmArr.clear();

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
            if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
                pDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, *pUndoPam ), true);
            else
                pDoc->getIDocumentRedlineAccess().SplitRedline( *pUndoPam );
            pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mxDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
            if( mxDoc->getIDocumentRedlineAccess().IsRedlineOn() )
                mxDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, *pUndoPam ), true);
            else
                mxDoc->getIDocumentRedlineAccess().SplitRedline( *pUndoPam );
            mxDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        }
        if( bSaveUndo )
        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mxDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pUndo->SetInsertRange( *pUndoPam, false );
            // UGLY: temp. enable undo
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pDoc->GetIDocumentUndoRedo().DoUndo(true);
            pDoc->GetIDocumentUndoRedo().AppendUndo( pUndo );
            pDoc->GetIDocumentUndoRedo().DoUndo(false);
            pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mxDoc->GetIDocumentUndoRedo().DoUndo(true);
            mxDoc->GetIDocumentUndoRedo().AppendUndo( pUndo );
            mxDoc->GetIDocumentUndoRedo().DoUndo(false);
            mxDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        }

        delete pUndoPam;

        pPam = (SwPaM *) pPam->GetNext();
        if( pPam == pEnd )
            break;

        // only read templates? then ignore multi selection! Bug 68593
        if( bFmtsOnly )
            break;

        /*
         * !!! The Status of the Stream has to be reset directly. !!!
         *     When Seeking, the current Status-, EOF- und bad-Bit is set;
         *     nobody knows why
         */
        if( pStrm )
        {
            pStrm->Seek(0);
            pStrm->ResetError();
        }
    }

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    pDoc->SetInReading( false );
    pDoc->SetInXMLImport( false );

    pDoc->InvalidateNumRules();
    pDoc->UpdateNumRule();
    pDoc->ChkCondColls();
    pDoc->SetAllUniqueFlyNames();
    pDoc->getIDocumentState().SetLoaded( true );

    pDoc->GetIDocumentUndoRedo().DoUndo(bDocUndo);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mxDoc->SetInReading( false );
    mxDoc->SetInXMLImport( false );

    mxDoc->InvalidateNumRules();
    mxDoc->UpdateNumRule();
    mxDoc->ChkCondColls();
    mxDoc->SetAllUniqueFlyNames();
    mxDoc->getIDocumentState().SetLoaded( true );

    mxDoc->GetIDocumentUndoRedo().DoUndo(bDocUndo);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (!bReadPageDescs)
    {
        if( bSaveUndo )
        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
            pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_INSDOKUMENT, NULL );
            pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mxDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
            mxDoc->GetIDocumentUndoRedo().EndUndo( UNDO_INSDOKUMENT, NULL );
            mxDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( nsRedlineMode_t::REDLINE_IGNORE );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        }
    }

    // delete Pam if it was created only for reading
    if( !pCrsr )
    {
        delete pPam;          // open a new one

        // #i42634# Moved common code of SwReader::Read() and
        // SwDocShell::UpdateLinks() to new SwDoc::UpdateLinks():
    // ATM still with Update
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pDoc->getIDocumentLinksAdministration().UpdateLinks();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->getIDocumentLinksAdministration().UpdateLinks();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        // not insert: set the redline mode read from settings.xml
        eOld = static_cast<RedlineMode_t>(
                ePostReadRedlineMode & ~nsRedlineMode_t::REDLINE_IGNORE);

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pDoc->getIDocumentFieldsAccess().SetFieldsDirty(false, NULL, 0);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->getIDocumentFieldsAccess().SetFieldsDirty(false, NULL, 0);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
    pDoc->SetOle2Link( aOLELink );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mxDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
    mxDoc->SetOle2Link( aOLELink );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    if( pCrsr )                 // das Doc ist jetzt modifiziert
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pDoc->getIDocumentState().SetModified();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->getIDocumentState().SetModified();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    // #i38810# - If links have been updated, the document
    // have to be modified. During update of links the OLE link at the document
    // isn't set. Thus, the document's modified state has to be set again after
    // the OLE link is restored - see above <pDoc->SetOle2Link( aOLELink )>.
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if ( pDoc->getIDocumentLinksAdministration().LinksUpdated() )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if ( mxDoc->getIDocumentLinksAdministration().LinksUpdated() )
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pDoc->getIDocumentState().SetModified();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->getIDocumentState().SetModified();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }

    po->SetReadUTF8( false );
    po->SetBlockMode( false );
    po->SetOrganizerMode( false );
    po->SetIgnoreHTMLComments( false );

    return nError;
}


SwReader::SwReader(SfxMedium& rMedium, const OUString& rFileName, SwDoc *pDocument)
    : SwDocFac(pDocument), pStrm(0), pMedium(&rMedium), pCrsr(0),
    aFileName(rFileName)
{
    SetBaseURL( rMedium.GetBaseURL() );
}


// Read into an existing document
SwReader::SwReader(SvStream& rStrm, const OUString& rFileName, const OUString& rBaseURL, SwPaM& rPam)
    : SwDocFac(rPam.GetDoc()), pStrm(&rStrm), pMedium(0), pCrsr(&rPam),
    aFileName(rFileName)
{
    SetBaseURL( rBaseURL );
}

SwReader::SwReader(SfxMedium& rMedium, const OUString& rFileName, SwPaM& rPam)
    : SwDocFac(rPam.GetDoc()), pStrm(0), pMedium(&rMedium),
    pCrsr(&rPam), aFileName(rFileName)
{
    SetBaseURL( rMedium.GetBaseURL() );
}

SwReader::SwReader( const uno::Reference < embed::XStorage > &rStg, const OUString& rFilename, SwPaM &rPam )
    : SwDocFac(rPam.GetDoc()), pStrm(0), xStg( rStg ), pMedium(0), pCrsr(&rPam), aFileName(rFilename)
{
}

Reader::Reader()
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    : pTemplate(0),
    aDStamp( Date::EMPTY ),
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    : aDStamp( Date::EMPTY ),
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    aTStamp( tools::Time::EMPTY ),
    aChkDateTime( DateTime::EMPTY ),
    pStrm(0), pMedium(0), bInsertMode(false),
    bTmplBrowseMode(false), bReadUTF8(false), bBlockMode(false), bOrganizerMode(false),
    bHasAskTemplateName(false), bIgnoreHTMLComments(false)
{
}

Reader::~Reader()
{
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    delete pTemplate;
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
}

OUString Reader::GetTemplateName() const
{
    return OUString();
}

// load the Filter template, set and release
SwDoc* Reader::GetTemplateDoc()
{
    if( !bHasAskTemplateName )
    {
        SetTemplateName( GetTemplateName() );
        bHasAskTemplateName = true;
    }

    if( aTemplateNm.isEmpty() )
        ClearTemplate();
    else
    {
        INetURLObject aTDir( aTemplateNm );
        const OUString aFileName = aTDir.GetMainURL( INetURLObject::NO_DECODE );
        OSL_ENSURE( !aTDir.HasError(), "No absolute path for template name!" );
        DateTime aCurrDateTime( DateTime::SYSTEM );
        bool bLoad = false;

        // if the template is already loaded, check once-a-minute if it has changed
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if( !pTemplate || aCurrDateTime >= aChkDateTime )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if( !mxTemplate.is() || aCurrDateTime >= aChkDateTime )
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        {
            Date aTstDate( Date::EMPTY );
            tools::Time aTstTime( tools::Time::EMPTY );
            if( FStatHelper::GetModifiedDateTimeOfFile(
                            aTDir.GetMainURL( INetURLObject::NO_DECODE ),
                            &aTstDate, &aTstTime ) &&
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                ( !pTemplate || aDStamp != aTstDate || aTStamp != aTstTime ))
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                ( !mxTemplate.is() || aDStamp != aTstDate || aTStamp != aTstTime ))
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            {
                bLoad = true;
                aDStamp = aTstDate;
                aTStamp = aTstTime;
            }

            // only one minute later check if it has changed
            aChkDateTime = aCurrDateTime;
            aChkDateTime += tools::Time( 0L, 1L );
        }

        if( bLoad )
        {
            ClearTemplate();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            OSL_ENSURE( !pTemplate, "Who holds the template doc?" );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            OSL_ENSURE( !mxTemplate.is(), "Who holds the template doc?" );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

                // If the writer module is not installed,
                // we cannot create a SwDocShell. We could create a
                // SwWebDocShell however, because this exists always
                // for the help.
                SvtModuleOptions aModuleOptions;
                if( aModuleOptions.IsWriter() )
                {
                    SwDocShell *pDocSh =
                        new SwDocShell ( SFX_CREATE_MODE_INTERNAL );
                    SfxObjectShellLock xDocSh = pDocSh;
                    if( pDocSh->DoInitNew( 0 ) )
                    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        pTemplate = pDocSh->GetDoc();
                        pTemplate->SetOle2Link( Link() );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        mxTemplate = pDocSh->GetDoc();
                        mxTemplate->SetOle2Link( Link() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        // always FALSE
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        pTemplate->GetIDocumentUndoRedo().DoUndo( false );
                        pTemplate->getIDocumentSettingAccess().set(IDocumentSettingAccess::BROWSE_MODE, bTmplBrowseMode );
                        pTemplate->RemoveAllFmtLanguageDependencies();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        mxTemplate->GetIDocumentUndoRedo().DoUndo( false );
                        mxTemplate->getIDocumentSettingAccess().set(IDocumentSettingAccess::BROWSE_MODE, bTmplBrowseMode );
                        mxTemplate->RemoveAllFmtLanguageDependencies();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

                        ReadXML->SetOrganizerMode( true );
                        SfxMedium aMedium( aFileName, sal_False );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        SwReader aRdr( aMedium, OUString(), pTemplate );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        SwReader aRdr( aMedium, OUString(), mxTemplate.get() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        aRdr.Read( *ReadXML );
                        ReadXML->SetOrganizerMode( false );

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                        pTemplate->acquire();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    }
                }
        }

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        OSL_ENSURE( !pTemplate || FStatHelper::IsDocument( aFileName ) || aTemplateNm=="$$Dummy$$",
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        OSL_ENSURE( !mxTemplate.is() || FStatHelper::IsDocument( aFileName ) || aTemplateNm=="$$Dummy$$",
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                "TemplatePtr but no template exist!" );
    }

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    return pTemplate;
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    return mxTemplate.get();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
}

bool Reader::SetTemplate( SwDoc& rDoc )
{
    bool bRet = false;

    GetTemplateDoc();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if( pTemplate )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if( mxTemplate.is() )
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    {
        rDoc.RemoveAllFmtLanguageDependencies();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        rDoc.ReplaceStyles( *pTemplate );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        rDoc.ReplaceStyles( *mxTemplate );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        rDoc.getIDocumentFieldsAccess().SetFixFields(false, NULL);
        bRet = true;
    }

    return bRet;
}

void Reader::ClearTemplate()
{
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if( pTemplate )
    {
        if( 0 == pTemplate->release() )
            delete pTemplate,
        pTemplate = 0;
    }
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mxTemplate.clear();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
}

void Reader::SetTemplateName( const OUString& rDir )
{
    if( !rDir.isEmpty() && aTemplateNm != rDir )
    {
        ClearTemplate();
        aTemplateNm = rDir;
    }
}

void Reader::MakeHTMLDummyTemplateDoc()
{
    ClearTemplate();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    pTemplate = new SwDoc;
    pTemplate->acquire();
    pTemplate->getIDocumentSettingAccess().set(IDocumentSettingAccess::BROWSE_MODE, bTmplBrowseMode );
    pTemplate->getIDocumentDeviceAccess().getPrinter( true );
    pTemplate->RemoveAllFmtLanguageDependencies();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mxTemplate = new SwDoc;
    mxTemplate->getIDocumentSettingAccess().set(IDocumentSettingAccess::BROWSE_MODE, bTmplBrowseMode );
    mxTemplate->getIDocumentDeviceAccess().getPrinter( true );
    mxTemplate->RemoveAllFmtLanguageDependencies();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    aChkDateTime = Date( 1, 1, 2300 );  // year 2300 should be sufficient
    aTemplateNm = "$$Dummy$$";
}

// Users that do not need to open these Streams / Storages,
// have to overload this method
bool Reader::SetStrmStgPtr()
{
    OSL_ENSURE( pMedium, "Where is the Media??" );

    if( pMedium->IsStorage() )
    {
        if( SW_STORAGE_READER & GetReaderType() )
        {
            xStg = pMedium->GetStorage();
            return true;
        }
    }
    else
    {
        pStrm = pMedium->GetInStream();
        if ( pStrm && SotStorage::IsStorageFile(pStrm) && (SW_STORAGE_READER & GetReaderType()) )
        {
            pStg = new SotStorage( *pStrm );
            pStrm = NULL;
        }
        else if ( !(SW_STREAM_READER & GetReaderType()) )
        {
            pStrm = NULL;
            return false;
        }

        return true;
    }
    return false;
}

int Reader::GetReaderType()
{
    return SW_STREAM_READER;
}

void Reader::SetFltName( const OUString& )
{
}

void Reader::ResetFrmFmtAttrs( SfxItemSet &rFrmSet )
{
    rFrmSet.Put( SvxLRSpaceItem(RES_LR_SPACE) );
    rFrmSet.Put( SvxULSpaceItem(RES_UL_SPACE) );
    rFrmSet.Put( SvxBoxItem(RES_BOX) );
}

void Reader::ResetFrmFmts( SwDoc& rDoc )
{
    sal_uInt16 const s_ids[3] = {
        RES_POOLFRM_FRAME, RES_POOLFRM_GRAPHIC, RES_POOLFRM_OLE
    };
    for (sal_uInt16 i = 0; i < SAL_N_ELEMENTS(s_ids); ++i)
    {
        SwFrmFmt *const pFrmFmt = rDoc.getIDocumentStylePoolAccess().GetFrmFmtFromPool( s_ids[i] );

        pFrmFmt->ResetFmtAttr( RES_LR_SPACE );
        pFrmFmt->ResetFmtAttr( RES_UL_SPACE );
        pFrmFmt->ResetFmtAttr( RES_BOX );
    }
}

    // read the sections of the document, which is equal to the medium.
    // returns the count of it
size_t Reader::GetSectionList( SfxMedium&, std::vector<OUString*>& ) const
{
    return 0;
}

bool SwReader::HasGlossaries( const Reader& rOptions )
{
    // copy variables
    Reader* po = (Reader*) &rOptions;
    po->pStrm = pStrm;
    po->pStg  = pStg;
    po->bInsertMode = false;

    // if a Medium is selected, get its Stream
    bool bRet = false;
    if( !( 0 != (po->pMedium = pMedium ) && !po->SetStrmStgPtr() ))
        bRet = po->HasGlossaries();
    return bRet;
}

bool SwReader::ReadGlossaries( const Reader& rOptions,
                                SwTextBlocks& rBlocks, bool bSaveRelFiles )
{
    // copy variables
    Reader* po = (Reader*) &rOptions;
    po->pStrm = pStrm;
    po->pStg  = pStg;
    po->bInsertMode = false;

    // if a Medium is selected, get its Stream
    bool bRet = false;
    if( !( 0 != (po->pMedium = pMedium ) && !po->SetStrmStgPtr() ))
        bRet = po->ReadGlossaries( rBlocks, bSaveRelFiles );
    return bRet;
}

bool Reader::HasGlossaries() const
{
    return false;
}

bool Reader::ReadGlossaries( SwTextBlocks&, bool ) const
{
    return false;
}

int StgReader::GetReaderType()
{
    return SW_STORAGE_READER;
}

/*
 * Writer
 */

/*
 * Constructors, Destructors are inline (inc/shellio.hxx).
 */

SwWriter::SwWriter(SvStream& rStrm, SwCrsrShell &rShell, bool bInWriteAll)
    : pStrm(&rStrm), pMedium(0), pOutPam(0), pShell(&rShell),
    rDoc(*rShell.GetDoc()), bWriteAll(bInWriteAll)
{
}

SwWriter::SwWriter(SvStream& rStrm,SwDoc &rDocument)
    : pStrm(&rStrm), pMedium(0), pOutPam(0), pShell(0), rDoc(rDocument),
    bWriteAll(true)
{
}

SwWriter::SwWriter(SvStream& rStrm, SwPaM& rPam, bool bInWriteAll)
    : pStrm(&rStrm), pMedium(0), pOutPam(&rPam), pShell(0),
    rDoc(*rPam.GetDoc()), bWriteAll(bInWriteAll)
{
}

SwWriter::SwWriter( const uno::Reference < embed::XStorage >& rStg, SwDoc &rDocument)
    : pStrm(0), xStg( rStg ), pMedium(0), pOutPam(0), pShell(0), rDoc(rDocument), bWriteAll(true)
{
}

SwWriter::SwWriter(SfxMedium& rMedium, SwCrsrShell &rShell, bool bInWriteAll)
    : pStrm(0), pMedium(&rMedium), pOutPam(0), pShell(&rShell),
    rDoc(*rShell.GetDoc()), bWriteAll(bInWriteAll)
{
}

SwWriter::SwWriter(SfxMedium& rMedium, SwDoc &rDocument)
    : pStrm(0), pMedium(&rMedium), pOutPam(0), pShell(0), rDoc(rDocument),
    bWriteAll(true)
{
}

sal_uLong SwWriter::Write( WriterRef& rxWriter, const OUString* pRealFileName )
{
    // #i73788#
    SwPauseThreadStarting aPauseThreadStarting;

    bool bHasMark = false;
    SwPaM * pPam;

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SwDoc *pDoc = 0;
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    rtl::Reference<SwDoc> xDoc;
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    if ( pShell && !bWriteAll && pShell->IsTableMode() )
    {
        bWriteAll = true;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pDoc = new SwDoc;
        pDoc->acquire();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        xDoc = new SwDoc;
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        // Copy parts of a table:
        // Create a table with the width of the original and copy the selected cells.
        // The sizes are corrected by ratio.

        // search the layout for cells
        SwSelBoxes aBoxes;
        GetTblSel( *pShell, aBoxes );
        SwTableNode* pTblNd = (SwTableNode*)aBoxes[0]->GetSttNd()->StartOfSectionNode();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfExtras(), 2 );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SwNodeIndex aIdx( xDoc->GetNodes().GetEndOfExtras(), 2 );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SwCntntNode *pNd = aIdx.GetNode().GetCntntNode();
        OSL_ENSURE( pNd, "Node not found" );
        SwPosition aPos( aIdx, SwIndex( pNd ) );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pTblNd->GetTable().MakeCopy( pDoc, aPos, aBoxes );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pTblNd->GetTable().MakeCopy( xDoc.get(), aPos, aBoxes );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }

    if( !bWriteAll && ( pShell || pOutPam ))
    {
        if( pShell )
            pPam = pShell->GetCrsr();
        else
            pPam = pOutPam;

        SwPaM *pEnd = pPam;

        // 1st round: Check if there is a selection
        while(true)
        {
            bHasMark = bHasMark || pPam->HasMark();
            pPam = (SwPaM *) pPam->GetNext();
            if(bHasMark || pPam == pEnd)
                break;
        }

        // if there is no selection, select the whole document
        if(!bHasMark)
        {
            if( pShell )
            {
                pShell->Push();
                pShell->SttEndDoc(true);
                pShell->SetMark();
                pShell->SttEndDoc(false);
            }
            else
            {
                pPam = new SwPaM( *pPam );
                pPam->Move( fnMoveBackward, fnGoDoc );
                pPam->SetMark();
                pPam->Move( fnMoveForward, fnGoDoc );
            }
        }
        // pPam is still the current Cursor !!
    }
    else
    {
        // no Shell or write-everything -> create a Pam
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SwDoc* pOutDoc = pDoc ? pDoc : &rDoc;
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SwDoc* pOutDoc = xDoc.is() ? xDoc.get() : &rDoc;
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pPam = new SwPaM( pOutDoc->GetNodes().GetEndOfContent() );
        if( pOutDoc->IsClipBoard() )
        {
            pPam->Move( fnMoveBackward, fnGoDoc );
            pPam->SetMark();
            pPam->Move( fnMoveForward, fnGoDoc );
        }
        else
        {
            pPam->SetMark();
            pPam->Move( fnMoveBackward, fnGoDoc );
        }
    }

    rxWriter->bWriteAll = bWriteAll;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SwDoc* pOutDoc = pDoc ? pDoc : &rDoc;
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SwDoc* pOutDoc = xDoc.is() ? xDoc.get() : &rDoc;
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    // If the default PageDesc has still the initial value,
    // (e.g. if no printer was set) then set it to DIN A4.
    // #i37248# - Modifications are only allowed at a new document.
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    // <pOutDoc> contains a new document, if <pDoc> is set - see above.
    if ( pDoc && !pOutDoc->getIDocumentDeviceAccess().getPrinter( false ) )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    // <pOutDoc> contains a new document, if <xDoc> is set - see above.
    if ( xDoc.is() && !pOutDoc->getIDocumentDeviceAccess().getPrinter( false ) )
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    {
        const SwPageDesc& rPgDsc = pOutDoc->GetPageDesc( 0 );
        //const SwPageDesc& rPgDsc = *pOutDoc->GetPageDescFromPool( RES_POOLPAGE_STANDARD );
        const SwFmtFrmSize& rSz = rPgDsc.GetMaster().GetFrmSize();
        // Clipboard-Document is always created w/o printer; thus the
        // default PageDesc is always aug LONG_MAX !! Set then to DIN A4
        if( LONG_MAX == rSz.GetHeight() || LONG_MAX == rSz.GetWidth() )
        {
            SwPageDesc aNew( rPgDsc );
            SwFmtFrmSize aNewSz( rSz );
            Size a4(SvxPaperInfo::GetPaperSize( PAPER_A4 ));
            aNewSz.SetHeight( a4.Width() );
            aNewSz.SetWidth( a4.Height() );
            aNew.GetMaster().SetFmtAttr( aNewSz );
            pOutDoc->ChgPageDesc( 0, aNew );
        }
    }

    bool bLockedView(false);
    SwEditShell* pESh = pOutDoc->GetEditShell();
    if( pESh )
    {
        bLockedView = pESh->IsViewLocked();
        pESh->LockView( true );    //lock visible section
        pESh->StartAllAction();
    }

    bool bWasPurgeOle = pOutDoc->getIDocumentSettingAccess().get(IDocumentSettingAccess::PURGE_OLE);
    pOutDoc->getIDocumentSettingAccess().set(IDocumentSettingAccess::PURGE_OLE, false);

    sal_uLong nError = 0;
    if( pMedium )
        nError = rxWriter->Write( *pPam, *pMedium, pRealFileName );
    else if( pStg )
        nError = rxWriter->Write( *pPam, *pStg, pRealFileName );
    else if( pStrm )
        nError = rxWriter->Write( *pPam, *pStrm, pRealFileName );
    else if( xStg.is() )
        nError = rxWriter->Write( *pPam, xStg, pRealFileName );

    pOutDoc->getIDocumentSettingAccess().set(IDocumentSettingAccess::PURGE_OLE, bWasPurgeOle );

#ifdef USE_JAVA
    // Attempt to fix Mac App Store crash by checking if the document's edit
    // shell has been deleted after it was fetched
    pESh = pOutDoc->GetEditShell();
#endif	// USE_JAVA
    if( pESh )
    {
        pESh->EndAllAction();
        pESh->LockView( bLockedView );
    }

    // If the selection was only created for printing, reset the old cursor before returning
    if( !bWriteAll && ( pShell || pOutPam ))
    {
        if(!bHasMark)
        {
            if( pShell )
                pShell->Pop( false );
            else
                delete pPam;
        }
    }
    else
    {
        delete pPam;            // delete the created Pam
        // Everything was written successfully? Tell the document!
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if ( !IsError( nError ) && !pDoc )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if ( !IsError( nError ) && !xDoc.is() )
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        {
            rDoc.getIDocumentState().ResetModified();
            // #i38810# - reset also flag, that indicates updated links
            rDoc.getIDocumentLinksAdministration().SetLinksUpdated( false );
        }
    }

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if ( pDoc )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if ( xDoc.is() )
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if ( !pDoc->release() )
            delete pDoc;
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        xDoc.clear();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        bWriteAll = false;
    }

    return nError;
}

bool SetHTMLTemplate( SwDoc & rDoc )
{
    // get template name of the Sfx-HTML-Filter !!!
    if( !ReadHTML->GetTemplateDoc() )
        ReadHTML->MakeHTMLDummyTemplateDoc();

    bool bRet = ReadHTML->SetTemplate( rDoc );

    SwNodes& rNds = rDoc.GetNodes();
    SwNodeIndex aIdx( rNds.GetEndOfExtras(), 1 );
    SwCntntNode* pCNd = rNds.GoNext( &aIdx );
    if( pCNd )
    {
        pCNd->SetAttr
            ( SwFmtPageDesc(rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_HTML, false) ) );
        pCNd->ChgFmtColl( rDoc.getIDocumentStylePoolAccess().GetTxtCollFromPool( RES_POOLCOLL_TEXT, false ));
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
