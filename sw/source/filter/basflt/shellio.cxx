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
 * 
 *   Modified April 2018 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include <tblafmt.hxx>
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
    Reader* po = const_cast<Reader*>(&rOptions);
    po->pStrm = pStrm;
    po->pStg  = pStg;
    po->xStg  = xStg;
    po->bInsertMode = nullptr != pCursor;
    po->bSkipImages = mbSkipImages;

    // if a Medium is selected, get its Stream
    if( nullptr != (po->pMedium = pMedium ) &&
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
    Link<bool,void> aOLELink( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link<bool,void>() );

    pDoc->SetInReading( true );
    pDoc->SetInXMLImport( dynamic_cast< XMLReader* >(po) !=  nullptr );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    Link<bool,void> aOLELink( mxDoc->GetOle2Link() );
    mxDoc->SetOle2Link( Link<bool,void>() );

    mxDoc->SetInReading( true );
    mxDoc->SetInXMLImport( dynamic_cast< XMLReader* >(po) !=  nullptr );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    SwPaM *pPam;
    if( pCursor )
        pPam = pCursor;
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
        if( !pDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) || ReadHTML != po || !po->mxTemplate.is()  )
            po->SetTemplate( *pDoc );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if( !mxDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) || ReadHTML != po || !po->mxTemplate.is()  )
            po->SetTemplate( *mxDoc );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }

    // Pams are connected like rings; stop when we return to the 1st element
    SwPaM *pEnd = pPam;
    SwUndoInsDoc* pUndo = nullptr;

    bool bReadPageDescs = false;
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    bool const bDocUndo = pDoc->GetIDocumentUndoRedo().DoesUndo();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    bool const bDocUndo = mxDoc->GetIDocumentUndoRedo().DoesUndo();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    bool bSaveUndo = bDocUndo && pCursor;
    if( bSaveUndo )
    {
        // the reading of the page template cannot be undone!
        bReadPageDescs = po->aOpt.IsPageDescs();
        if( bReadPageDescs )
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
            pDoc->GetIDocumentUndoRedo().StartUndo( SwUndoId::INSDOKUMENT, nullptr );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mxDoc->GetIDocumentUndoRedo().ClearRedo();
            mxDoc->GetIDocumentUndoRedo().StartUndo( SwUndoId::INSDOKUMENT, nullptr );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        }
    }
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    pDoc->GetIDocumentUndoRedo().DoUndo(false);

    SwNodeIndex aSplitIdx( pDoc->GetNodes() );

    RedlineFlags eOld = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mxDoc->GetIDocumentUndoRedo().DoUndo(false);

    SwNodeIndex aSplitIdx( mxDoc->GetNodes() );

    RedlineFlags eOld = mxDoc->getIDocumentRedlineAccess().GetRedlineFlags();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    RedlineFlags ePostReadRedlineFlags( RedlineFlags::Ignore );

    // Array of FlyFormats
    SwFrameFormatsV aFlyFrameArr;
    // only read templates? then ignore multi selection!
    bool bFormatsOnly = po->aOpt.IsFormatsOnly();

    while( true )
    {
        if( bSaveUndo )
            pUndo = new SwUndoInsDoc( *pPam );

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::Ignore );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::Ignore );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        SwPaM* pUndoPam = nullptr;
        if( bDocUndo || pCursor )
        {
            // set Pam to the previous node, so that it is not also moved
            const SwNodeIndex& rTmp = pPam->GetPoint()->nNode;
            pUndoPam = new SwPaM( rTmp, rTmp, 0, -1 );
        }

        // store for now all Fly's
        if( pCursor )
        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            std::copy(pDoc->GetSpzFrameFormats()->begin(),
                pDoc->GetSpzFrameFormats()->end(), std::back_inserter(aFlyFrameArr));
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            std::copy(mxDoc->GetSpzFrameFormats()->begin(),
                mxDoc->GetSpzFrameFormats()->end(), std::back_inserter(aFlyFrameArr));
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        }

        const sal_Int32 nSttContent = pPam->GetPoint()->nContent.GetIndex();

        // make sure the End position is correct for all Readers
        SwContentNode* pCNd = pPam->GetContentNode();
        sal_Int32 nEndContent = pCNd ? pCNd->Len() - nSttContent : 0;
        SwNodeIndex aEndPos( pPam->GetPoint()->nNode, 1 );

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        nError = po->Read( *pDoc, sBaseURL, *pPam, aFileName );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        nError = po->Read( *mxDoc, sBaseURL, *pPam, aFileName );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        // an ODF document may contain redline mode in settings.xml; save it!
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        ePostReadRedlineFlags = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        ePostReadRedlineFlags = mxDoc->getIDocumentRedlineAccess().GetRedlineFlags();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::Ignore );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::Ignore );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

        if( !IsError( nError ))     // set the End position already
        {
            --aEndPos;
            pCNd = aEndPos.GetNode().GetContentNode();
            if( !pCNd && nullptr == ( pCNd = SwNodes::GoPrevious( &aEndPos ) ))
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                pCNd = pDoc->GetNodes().GoNext( &aEndPos );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                pCNd = mxDoc->GetNodes().GoNext( &aEndPos );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

            pPam->GetPoint()->nNode = aEndPos;
            const sal_Int32 nLen = pCNd->Len();
            if( nLen < nEndContent )
                nEndContent = 0;
            else
                nEndContent = nLen - nEndContent;
            pPam->GetPoint()->nContent.Assign( pCNd, nEndContent );

            const SwStartNode* pTableBoxStart = pCNd->FindTableBoxStartNode();
            if ( pTableBoxStart )
            {
                SwTableBox* pBox = pTableBoxStart->GetTableBox();
                if ( pBox )
                {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    pDoc->ChkBoxNumFormat( *pBox, true );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    mxDoc->ChkBoxNumFormat( *pBox, true );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                }
            }
        }

        if( pCursor )
        {
            *pUndoPam->GetMark() = *pPam->GetPoint();
            ++pUndoPam->GetPoint()->nNode;
            SwNode& rNd = pUndoPam->GetNode();
            if( rNd.IsContentNode() )
                pUndoPam->GetPoint()->nContent.Assign(
                                    static_cast<SwContentNode*>(&rNd), nSttContent );
            else
                pUndoPam->GetPoint()->nContent.Assign( nullptr, 0 );

            bool bChkHeaderFooter = rNd.FindHeaderStartNode() ||
                                   rNd.FindFooterStartNode();

            // search all new Fly's, and store them as individual Undo Objects
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            for( SwFrameFormats::size_type n = 0; n < pDoc->GetSpzFrameFormats()->size(); ++n )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            for( SwFrameFormats::size_type n = 0; n < mxDoc->GetSpzFrameFormats()->size(); ++n )
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                SwFrameFormat* pFrameFormat = (*pDoc->GetSpzFrameFormats())[ n ];
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                SwFrameFormat* pFrameFormat = (*mxDoc->GetSpzFrameFormats())[ n ];
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
                // ok, here IsAlive is a misnomer...
                if (!aFlyFrameArr.IsAlive(pFrameFormat))
                {
                    SwPosition const*const pFrameAnchor(
                            rAnchor.GetContentAnchor());
                    if  (   (RndStdIds::FLY_AT_PAGE == rAnchor.GetAnchorId())
                        ||  (   pFrameAnchor
                            &&  (   (   (RndStdIds::FLY_AT_PARA == rAnchor.GetAnchorId())
                                    &&  (   (pUndoPam->GetPoint()->nNode ==
                                             pFrameAnchor->nNode)
                                        ||  (pUndoPam->GetMark()->nNode ==
                                             pFrameAnchor->nNode)
                                        )
                                    )
                                // #i97570# also check frames anchored AT char
                                ||  (   (RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId())
                                    &&  !IsDestroyFrameAnchoredAtChar(
                                              *pFrameAnchor,
                                              *pUndoPam->GetPoint(),
                                              *pUndoPam->GetMark())
                                    )
                                )
                            )
                        )
                    {
                        if( bChkHeaderFooter &&
                            (RndStdIds::FLY_AT_PARA == rAnchor.GetAnchorId()) &&
                            RES_DRAWFRMFMT == pFrameFormat->Which() )
                        {
                            // DrawObjects are not allowed in Headers/Footers!
                            pFrameFormat->DelFrames();
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            pDoc->DelFrameFormat( pFrameFormat );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            mxDoc->DelFrameFormat( pFrameFormat );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            --n;
                        }
                        else
                        {
                            if( bSaveUndo )
                            {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                mxDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                // UGLY: temp. enable undo
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                pDoc->GetIDocumentUndoRedo().DoUndo(true);
                                pDoc->GetIDocumentUndoRedo().AppendUndo(
                                    new SwUndoInsLayFormat( pFrameFormat,0,0 ) );
                                pDoc->GetIDocumentUndoRedo().DoUndo(false);
                                pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::Ignore );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                mxDoc->GetIDocumentUndoRedo().DoUndo(true);
                                mxDoc->GetIDocumentUndoRedo().AppendUndo(
                                    new SwUndoInsLayFormat( pFrameFormat,0,0 ) );
                                mxDoc->GetIDocumentUndoRedo().DoUndo(false);
                                mxDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::Ignore );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                            }
                            if( pFrameFormat->HasWriterListeners() )
                            {
                                // Draw-Objects create a Frame when being inserted; thus delete them
                                pFrameFormat->DelFrames();
                            }

                            if (RndStdIds::FLY_AT_PAGE == rAnchor.GetAnchorId())
                            {
                                if( !rAnchor.GetContentAnchor() )
                                {
                                    pFrameFormat->MakeFrames();
                                }
                                else if( pCursor )
                                {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                    pDoc->SetContainsAtPageObjWithContentAnchor( true );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                    mxDoc->SetContainsAtPageObjWithContentAnchor( true );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                }
                            }
                            else
                                pFrameFormat->MakeFrames();
                        }
                    }
                }
            }
            if( !aFlyFrameArr.empty() )
                aFlyFrameArr.clear();

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
            if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
                pDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, *pUndoPam ), true);
            else
                pDoc->getIDocumentRedlineAccess().SplitRedline( *pUndoPam );
            pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::Ignore );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mxDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
            if( mxDoc->getIDocumentRedlineAccess().IsRedlineOn() )
                mxDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, *pUndoPam ), true);
            else
                mxDoc->getIDocumentRedlineAccess().SplitRedline( *pUndoPam );
            mxDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::Ignore );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        }
        if( bSaveUndo )
        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mxDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pUndo->SetInsertRange( *pUndoPam, false );
            // UGLY: temp. enable undo
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pDoc->GetIDocumentUndoRedo().DoUndo(true);
            pDoc->GetIDocumentUndoRedo().AppendUndo( pUndo );
            pDoc->GetIDocumentUndoRedo().DoUndo(false);
            pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::Ignore );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mxDoc->GetIDocumentUndoRedo().DoUndo(true);
            mxDoc->GetIDocumentUndoRedo().AppendUndo( pUndo );
            mxDoc->GetIDocumentUndoRedo().DoUndo(false);
            mxDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::Ignore );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        }

        delete pUndoPam;

        pPam = pPam->GetNext();
        if( pPam == pEnd )
            break;

        // only read templates? then ignore multi selection! Bug 68593
        if( bFormatsOnly )
            break;

        /*
         * !!! The Status of the Stream has to be reset directly. !!!
         *     When Seeking, the current Status-, EOF- and bad-Bit is set;
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
    pDoc->getIDocumentState().SetLoaded();
    // Clear unassigned cell styles, because they aren't needed anymore.
    pDoc->GetCellStyles().clear();

    pDoc->GetIDocumentUndoRedo().DoUndo(bDocUndo);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mxDoc->SetInReading( false );
    mxDoc->SetInXMLImport( false );

    mxDoc->InvalidateNumRules();
    mxDoc->UpdateNumRule();
    mxDoc->ChkCondColls();
    mxDoc->SetAllUniqueFlyNames();
    mxDoc->getIDocumentState().SetLoaded();
    // Clear unassigned cell styles, because they aren't needed anymore.
    mxDoc->GetCellStyles().clear();

    mxDoc->GetIDocumentUndoRedo().DoUndo(bDocUndo);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (!bReadPageDescs)
    {
        if( bSaveUndo )
        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
            pDoc->GetIDocumentUndoRedo().EndUndo( SwUndoId::INSDOKUMENT, nullptr );
            pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::Ignore );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mxDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
            mxDoc->GetIDocumentUndoRedo().EndUndo( SwUndoId::INSDOKUMENT, nullptr );
            mxDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::Ignore );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        }
    }

    // delete Pam if it was created only for reading
    if( !pCursor )
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
        eOld = ePostReadRedlineFlags & ~RedlineFlags::Ignore;

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        pDoc->getIDocumentFieldsAccess().SetFieldsDirty(false, nullptr, 0);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->getIDocumentFieldsAccess().SetFieldsDirty(false, nullptr, 0);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    pDoc->SetOle2Link( aOLELink );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mxDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    mxDoc->SetOle2Link( aOLELink );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    if( pCursor )                 // the document is now modified
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
    : SwDocFac(pDocument), pStrm(nullptr), pMedium(&rMedium), pCursor(nullptr),
    aFileName(rFileName), mbSkipImages(false)
{
    SetBaseURL( rMedium.GetBaseURL() );
    SetSkipImages( rMedium.IsSkipImages() );
}


// Read into an existing document
SwReader::SwReader(SvStream& rStrm, const OUString& rFileName, const OUString& rBaseURL, SwPaM& rPam)
    : SwDocFac(rPam.GetDoc()), pStrm(&rStrm), pMedium(nullptr), pCursor(&rPam),
    aFileName(rFileName), mbSkipImages(false)
{
    SetBaseURL( rBaseURL );
}

SwReader::SwReader(SfxMedium& rMedium, const OUString& rFileName, SwPaM& rPam)
    : SwDocFac(rPam.GetDoc()), pStrm(nullptr), pMedium(&rMedium),
    pCursor(&rPam), aFileName(rFileName), mbSkipImages(false)
{
    SetBaseURL( rMedium.GetBaseURL() );
}

SwReader::SwReader( const uno::Reference < embed::XStorage > &rStg, const OUString& rFilename, SwPaM &rPam )
    : SwDocFac(rPam.GetDoc()), pStrm(nullptr), xStg( rStg ), pMedium(nullptr), pCursor(&rPam), aFileName(rFilename), mbSkipImages(false)
{
}

Reader::Reader()
  : aDStamp( Date::EMPTY ),
    aTStamp( tools::Time::EMPTY ),
    aChkDateTime( DateTime::EMPTY ),
    pStrm(nullptr), pMedium(nullptr), bInsertMode(false),
    bTmplBrowseMode(false), bReadUTF8(false), bBlockMode(false), bOrganizerMode(false),
    bHasAskTemplateName(false), bIgnoreHTMLComments(false), bSkipImages(false)
{
}

Reader::~Reader()
{
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
        const OUString aFileName = aTDir.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        OSL_ENSURE( !aTDir.HasError(), "No absolute path for template name!" );
        DateTime aCurrDateTime( DateTime::SYSTEM );
        bool bLoad = false;

        // if the template is already loaded, check once-a-minute if it has changed
        if( !mxTemplate.is() || aCurrDateTime >= aChkDateTime )
        {
            Date aTstDate( Date::EMPTY );
            tools::Time aTstTime( tools::Time::EMPTY );
            if( FStatHelper::GetModifiedDateTimeOfFile(
                            aTDir.GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                            &aTstDate, &aTstTime ) &&
                ( !mxTemplate.is() || aDStamp != aTstDate || aTStamp != aTstTime ))
            {
                bLoad = true;
                aDStamp = aTstDate;
                aTStamp = aTstTime;
            }

            // only one minute later check if it has changed
            aChkDateTime = aCurrDateTime;
            aChkDateTime += tools::Time( 0L, 1L );
        }

        if (bLoad)
        {
            ClearTemplate();
            OSL_ENSURE( !mxTemplate.is(), "Who holds the template doc?" );

            // If the writer module is not installed,
            // we cannot create a SwDocShell. We could create a
            // SwWebDocShell however, because this exists always
            // for the help.
            SvtModuleOptions aModuleOptions;
            if (aModuleOptions.IsWriter())
            {
                SwDocShell *pDocSh = new SwDocShell(SfxObjectCreateMode::INTERNAL);
                SfxObjectShellLock xDocSh = pDocSh;
                if (pDocSh->DoInitNew())
                {
                    mxTemplate = pDocSh->GetDoc();
                    mxTemplate->SetOle2Link( Link<bool,void>() );
                    // always FALSE
                    mxTemplate->GetIDocumentUndoRedo().DoUndo( false );
                    mxTemplate->getIDocumentSettingAccess().set(DocumentSettingId::BROWSE_MODE, bTmplBrowseMode );
                    mxTemplate->RemoveAllFormatLanguageDependencies();

                    ReadXML->SetOrganizerMode( true );
                    SfxMedium aMedium( aFileName, StreamMode::NONE );
                    SwReader aRdr( aMedium, OUString(), mxTemplate.get() );
                    aRdr.Read( *ReadXML );
                    ReadXML->SetOrganizerMode( false );
                }
            }
        }

        OSL_ENSURE( !mxTemplate.is() || FStatHelper::IsDocument( aFileName ) || aTemplateNm=="$$Dummy$$",
                "TemplatePtr but no template exist!" );
    }

    return mxTemplate.get();
}

bool Reader::SetTemplate( SwDoc& rDoc )
{
    bool bRet = false;

    GetTemplateDoc();
    if( mxTemplate.is() )
    {
        rDoc.RemoveAllFormatLanguageDependencies();
        rDoc.ReplaceStyles( *mxTemplate );
        rDoc.getIDocumentFieldsAccess().SetFixFields(nullptr);
        bRet = true;
    }

    return bRet;
}

void Reader::ClearTemplate()
{
    mxTemplate.clear();
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
    mxTemplate = new SwDoc;
    mxTemplate->getIDocumentSettingAccess().set(DocumentSettingId::BROWSE_MODE, bTmplBrowseMode );
    mxTemplate->getIDocumentDeviceAccess().getPrinter( true );
    mxTemplate->RemoveAllFormatLanguageDependencies();
    aChkDateTime = Date( 1, 1, 2300 );  // year 2300 should be sufficient
    aTemplateNm = "$$Dummy$$";
}

// Users that do not need to open these Streams / Storages,
// have to override this method
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
            pStrm = nullptr;
        }
        else if ( !(SW_STREAM_READER & GetReaderType()) )
        {
            pStrm = nullptr;
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

void Reader::ResetFrameFormatAttrs( SfxItemSet &rFrameSet )
{
    rFrameSet.Put( SvxLRSpaceItem(RES_LR_SPACE) );
    rFrameSet.Put( SvxULSpaceItem(RES_UL_SPACE) );
    rFrameSet.Put( SvxBoxItem(RES_BOX) );
}

void Reader::ResetFrameFormats( SwDoc& rDoc )
{
    sal_uInt16 const s_ids[3] = {
        RES_POOLFRM_FRAME, RES_POOLFRM_GRAPHIC, RES_POOLFRM_OLE
    };
    for (sal_uInt16 i : s_ids)
    {
        SwFrameFormat *const pFrameFormat = rDoc.getIDocumentStylePoolAccess().GetFrameFormatFromPool( i );

        pFrameFormat->ResetFormatAttr( RES_LR_SPACE );
        pFrameFormat->ResetFormatAttr( RES_UL_SPACE );
        pFrameFormat->ResetFormatAttr( RES_BOX );
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
    Reader* po = const_cast<Reader*>(&rOptions);
    po->pStrm = pStrm;
    po->pStg  = pStg;
    po->bInsertMode = false;

    // if a Medium is selected, get its Stream
    bool bRet = false;
    if( !( nullptr != (po->pMedium = pMedium ) && !po->SetStrmStgPtr() ))
        bRet = po->HasGlossaries();
    return bRet;
}

bool SwReader::ReadGlossaries( const Reader& rOptions,
                                SwTextBlocks& rBlocks, bool bSaveRelFiles )
{
    // copy variables
    Reader* po = const_cast<Reader*>(&rOptions);
    po->pStrm = pStrm;
    po->pStg  = pStg;
    po->bInsertMode = false;

    // if a Medium is selected, get its Stream
    bool bRet = false;
    if( !( nullptr != (po->pMedium = pMedium ) && !po->SetStrmStgPtr() ))
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

SwWriter::SwWriter(SvStream& rStrm, SwCursorShell &rShell, bool bInWriteAll)
    : pStrm(&rStrm), pMedium(nullptr), pOutPam(nullptr), pShell(&rShell),
    rDoc(*rShell.GetDoc()), bWriteAll(bInWriteAll)
{
}

SwWriter::SwWriter(SvStream& rStrm,SwDoc &rDocument)
    : pStrm(&rStrm), pMedium(nullptr), pOutPam(nullptr), pShell(nullptr), rDoc(rDocument),
    bWriteAll(true)
{
}

SwWriter::SwWriter(SvStream& rStrm, SwPaM& rPam, bool bInWriteAll)
    : pStrm(&rStrm), pMedium(nullptr), pOutPam(&rPam), pShell(nullptr),
    rDoc(*rPam.GetDoc()), bWriteAll(bInWriteAll)
{
}

SwWriter::SwWriter( const uno::Reference < embed::XStorage >& rStg, SwDoc &rDocument)
    : pStrm(nullptr), xStg( rStg ), pMedium(nullptr), pOutPam(nullptr), pShell(nullptr), rDoc(rDocument), bWriteAll(true)
{
}

SwWriter::SwWriter(SfxMedium& rMedium, SwCursorShell &rShell, bool bInWriteAll)
    : pStrm(nullptr), pMedium(&rMedium), pOutPam(nullptr), pShell(&rShell),
    rDoc(*rShell.GetDoc()), bWriteAll(bInWriteAll)
{
}

SwWriter::SwWriter(SfxMedium& rMedium, SwDoc &rDocument)
    : pStrm(nullptr), pMedium(&rMedium), pOutPam(nullptr), pShell(nullptr), rDoc(rDocument),
    bWriteAll(true)
{
}

sal_uLong SwWriter::Write( WriterRef& rxWriter, const OUString* pRealFileName )
{
    // #i73788#
    SwPauseThreadStarting aPauseThreadStarting;

    bool bHasMark = false;
    SwPaM * pPam;

    rtl::Reference<SwDoc> xDoc;

    if ( pShell && !bWriteAll && pShell->IsTableMode() )
    {
        bWriteAll = true;
        xDoc = new SwDoc;

        // Copy parts of a table:
        // Create a table with the width of the original and copy the selected cells.
        // The sizes are corrected by ratio.

        // search the layout for cells
        SwSelBoxes aBoxes;
        GetTableSel( *pShell, aBoxes );
        const SwTableNode* pTableNd = static_cast<const SwTableNode*>(aBoxes[0]->GetSttNd()->StartOfSectionNode());
        SwNodeIndex aIdx( xDoc->GetNodes().GetEndOfExtras(), 2 );
        SwContentNode *pNd = aIdx.GetNode().GetContentNode();
        OSL_ENSURE( pNd, "Node not found" );
        SwPosition aPos( aIdx, SwIndex( pNd ) );
        pTableNd->GetTable().MakeCopy( xDoc.get(), aPos, aBoxes );
    }

    if( !bWriteAll && ( pShell || pOutPam ))
    {
        if( pShell )
            pPam = pShell->GetCursor();
        else
            pPam = pOutPam;

        SwPaM *pEnd = pPam;

        // 1st round: Check if there is a selection
        while(true)
        {
            bHasMark = bHasMark || pPam->HasMark();
            pPam = pPam->GetNext();
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
                pPam = new SwPaM( *pPam, pPam );
                pPam->Move( fnMoveBackward, GoInDoc );
                pPam->SetMark();
                pPam->Move( fnMoveForward, GoInDoc );
            }
        }
        // pPam is still the current Cursor !!
    }
    else
    {
        // no Shell or write-everything -> create a Pam
        SwDoc* pOutDoc = xDoc.is() ? xDoc.get() : &rDoc;
        pPam = new SwPaM( pOutDoc->GetNodes().GetEndOfContent() );
        if( pOutDoc->IsClipBoard() )
        {
            pPam->Move( fnMoveBackward, GoInDoc );
            pPam->SetMark();
            pPam->Move( fnMoveForward, GoInDoc );
        }
        else
        {
            pPam->SetMark();
            pPam->Move( fnMoveBackward, GoInDoc );
        }
    }

    rxWriter->bWriteAll = bWriteAll;
    SwDoc* pOutDoc = xDoc.is() ? xDoc.get() : &rDoc;

    // If the default PageDesc has still the initial value,
    // (e.g. if no printer was set) then set it to DIN A4.
    // #i37248# - Modifications are only allowed at a new document.
    // <pOutDoc> contains a new document, if <xDoc> is set - see above.
    if ( xDoc.is() && !pOutDoc->getIDocumentDeviceAccess().getPrinter( false ) )
    {
        const SwPageDesc& rPgDsc = pOutDoc->GetPageDesc( 0 );
        //const SwPageDesc& rPgDsc = *pOutDoc->GetPageDescFromPool( RES_POOLPAGE_STANDARD );
        const SwFormatFrameSize& rSz = rPgDsc.GetMaster().GetFrameSize();
        // Clipboard-Document is always created w/o printer; thus the
        // default PageDesc is always aug LONG_MAX !! Set then to DIN A4
        if( LONG_MAX == rSz.GetHeight() || LONG_MAX == rSz.GetWidth() )
        {
            SwPageDesc aNew( rPgDsc );
            SwFormatFrameSize aNewSz( rSz );
            Size a4(SvxPaperInfo::GetPaperSize( PAPER_A4 ));
            aNewSz.SetHeight( a4.Width() );
            aNewSz.SetWidth( a4.Height() );
            aNew.GetMaster().SetFormatAttr( aNewSz );
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

    const bool bOrigPurgeOle = pOutDoc->getIDocumentSettingAccess().get(DocumentSettingId::PURGE_OLE);
    pOutDoc->getIDocumentSettingAccess().set(DocumentSettingId::PURGE_OLE, false);

    sal_uLong nError = 0;
    if( pMedium )
        nError = rxWriter->Write( *pPam, *pMedium, pRealFileName );
    else if( pStg.is() )
        nError = rxWriter->Write( *pPam, *pStg, pRealFileName );
    else if( pStrm )
        nError = rxWriter->Write( *pPam, *pStrm, pRealFileName );
    else if( xStg.is() )
        nError = rxWriter->Write( *pPam, xStg, pRealFileName );

    pOutDoc->getIDocumentSettingAccess().set(DocumentSettingId::PURGE_OLE, bOrigPurgeOle );

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
                pShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
            else
                delete pPam;
        }
    }
    else
    {
        delete pPam;            // delete the created Pam
        // Everything was written successfully? Tell the document!
        if ( !IsError( nError ) && !xDoc.is() )
        {
            rDoc.getIDocumentState().ResetModified();
            // #i38810# - reset also flag, that indicates updated links
            rDoc.getIDocumentLinksAdministration().SetLinksUpdated( false );
        }
    }

    if ( xDoc.is() )
    {
        xDoc.clear();
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
    SwContentNode* pCNd = rNds.GoNext( &aIdx );
    if( pCNd )
    {
        pCNd->SetAttr
            ( SwFormatPageDesc(rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_HTML, false) ) );
        pCNd->ChgFormatColl( rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TEXT, false ));
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
