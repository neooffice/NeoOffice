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

#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/ruler.hxx>
#include <svx/sidebar/ContextChangeEventMultiplexer.hxx>
#include <idxmrk.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <viewopt.hxx>
#include <docsh.hxx>
#include <globdoc.hxx>
#include <navipi.hxx>
#include <fldwrap.hxx>
#include <redlndlg.hxx>
#include <dpage.hxx>
#include <edtwin.hxx>
#include "formatclipboard.hxx"
#include <cmdid.h>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>

extern bool bDocSzUpdated;

void SwView::Activate(bool bMDIActivate)
{
    // fdo#40438 Update the layout to make sure everything is correct before showing the content
    m_pWrtShell->StartAction();
    m_pWrtShell->EndAction( true );

    // Register the current View at the DocShell.
    // The view remains active at the DocShell until it will
    // be destroyed or by Activate a new one will be set.
    SwDocShell* pDocSh = GetDocShell();
    if(pDocSh)
        pDocSh->SetView(this);
    SwModule* pSwMod = SW_MOD();
    pSwMod->SetView(this);

    // Document size has changed.
    if(!bDocSzUpdated)
        DocSzChgd(m_aDocSz);

    // make selection visible
    if(m_bMakeSelectionVisible)
    {
        m_pWrtShell->MakeSelVisible();
        m_bMakeSelectionVisible = false;
    }
    m_pHRuler->SetActive( true );
    m_pVRuler->SetActive( true );

    if ( bMDIActivate )
    {
        m_pWrtShell->ShGetFcs(false);     // Selections visible

        if( !m_sSwViewData.isEmpty() )
        {
            ReadUserData(m_sSwViewData, false);
            m_sSwViewData = "";
        }

        AttrChangedNotify(m_pWrtShell);

        // Initialize Flddlg newly if necessary (e.g. for TYP_SETVAR)
        sal_uInt16 nId = SwFldDlgWrapper::GetChildWindowId();
        SfxViewFrame* pVFrame = GetViewFrame();
        SwFldDlgWrapper *pWrp = (SwFldDlgWrapper*)pVFrame->GetChildWindow(nId);
        if (pWrp)
            pWrp->ReInitDlg(GetDocShell());

        // Initialize RedlineDlg newly if necessary
        nId = SwRedlineAcceptChild::GetChildWindowId();
        SwRedlineAcceptChild *pRed = (SwRedlineAcceptChild*)pVFrame->GetChildWindow(nId);
        if (pRed)
            pRed->ReInitDlg(GetDocShell());

        // reinit IdxMarkDlg
        nId = SwInsertIdxMarkWrapper::GetChildWindowId();
        SwInsertIdxMarkWrapper *pIdxMrk = (SwInsertIdxMarkWrapper*)pVFrame->GetChildWindow(nId);
        if (pIdxMrk)
            pIdxMrk->ReInitDlg(*m_pWrtShell);

        // reinit AuthMarkDlg
        nId = SwInsertAuthMarkWrapper::GetChildWindowId();
        SwInsertAuthMarkWrapper *pAuthMrk = (SwInsertAuthMarkWrapper*)pVFrame->
                                                                GetChildWindow(nId);
        if (pAuthMrk)
            pAuthMrk->ReInitDlg(*m_pWrtShell);
    }
    else
        // At least call the Notify (as a precaution because of the SlotFilter).
        AttrChangedNotify(m_pWrtShell);

    SfxViewShell::Activate(bMDIActivate);
}

void SwView::Deactivate(bool bMDIActivate)
{
    extern bool bFlushCharBuffer ;
        // Are Characters still in the input buffer?
    if( bFlushCharBuffer )
        GetEditWin().FlushInBuffer();

    if( bMDIActivate )
    {
        m_pWrtShell->ShLooseFcs();    // Selections invisible

        m_pHRuler->SetActive( false );
        m_pVRuler->SetActive( false );
    }
    SfxViewShell::Deactivate(bMDIActivate);
}

void SwView::MarginChanged()
{
    GetWrtShell().SetBrowseBorder( GetMargin() );
}

void SwView::ExecFormatPaintbrush(SfxRequest& rReq)
{
    if(!m_pFormatClipboard)
        return;

    if( m_pFormatClipboard->HasContent() )
    {
        m_pFormatClipboard->Erase();

        SwApplyTemplate aTemplate;
        GetEditWin().SetApplyTemplate(aTemplate);
    }
    else
    {
        bool bPersistentCopy = false;
        const SfxItemSet *pArgs = rReq.GetArgs();
        if( pArgs && pArgs->Count() >= 1 )
        {
            bPersistentCopy = static_cast<bool>(((SfxBoolItem &)pArgs->Get(
                                    SID_FORMATPAINTBRUSH)).GetValue());
        }

        m_pFormatClipboard->Copy( GetWrtShell(), GetPool(), bPersistentCopy );

        SwApplyTemplate aTemplate;
        aTemplate.m_pFormatClipboard = m_pFormatClipboard;
        GetEditWin().SetApplyTemplate(aTemplate);
    }
    GetViewFrame()->GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
}

void SwView::StateFormatPaintbrush(SfxItemSet &rSet)
{
    if(!m_pFormatClipboard)
        return;

    bool bHasContent = m_pFormatClipboard && m_pFormatClipboard->HasContent();
#ifdef NO_LIBO_FORMAT_PAINT_BRUSH_PUT_LEAK_FIX
    rSet.Put(SfxBoolItem(SID_FORMATPAINTBRUSH, bHasContent));
    if(!bHasContent)
#else	// NO_LIBO_FORMAT_PAINT_BRUSH_PUT_LEAK_FIX
    if( !bHasContent &&
        !m_pFormatClipboard->CanCopyThisType( GetWrtShell().GetSelectionType())
      )
#endif	// NO_LIBO_FORMAT_PAINT_BRUSH_PUT_LEAK_FIX
    {
#ifdef NO_LIBO_FORMAT_PAINT_BRUSH_PUT_LEAK_FIX
        if( !m_pFormatClipboard->CanCopyThisType( GetWrtShell().GetSelectionType() ) )
            rSet.DisableItem( SID_FORMATPAINTBRUSH );
#else	// NO_LIBO_FORMAT_PAINT_BRUSH_PUT_LEAK_FIX
        rSet.DisableItem( SID_FORMATPAINTBRUSH );
#endif	// NO_LIBO_FORMAT_PAINT_BRUSH_PUT_LEAK_FIX
    }
#ifndef NO_LIBO_FORMAT_PAINT_BRUSH_PUT_LEAK_FIX
    else
        rSet.Put(SfxBoolItem(SID_FORMATPAINTBRUSH, bHasContent));
#endif	// !NO_LIBO_FORMAT_PAINT_BRUSH_PUT_LEAK_FIX
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
