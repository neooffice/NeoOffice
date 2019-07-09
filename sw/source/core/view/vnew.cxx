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
 *   Modified December 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sfx2/printer.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <docsh.hxx>
#include <viewsh.hxx>
#include <rootfrm.hxx>
#include <viewimp.hxx>
#include <viewopt.hxx>
#include <txtfrm.hxx>
#include <notxtfrm.hxx>
#include <fntcache.hxx>
#include <docufld.hxx>
#include <ptqueue.hxx>
#include <dview.hxx>
#include <ndgrf.hxx>
#include <ndindex.hxx>
#include <accessibilityoptions.hxx>
#include <calbck.hxx>

void SwViewShell::Init( const SwViewOption *pNewOpt )
{
    mbDocSizeChgd = false;

    // We play it safe: Remove old font information whenever the printer
    // resolution or the zoom factor changes. For that, Init() and Reformat()
    // are the most secure places.
     pFntCache->Flush( );

    // ViewOptions are created dynamically

    if( !mpOpt )
    {
        mpOpt = new SwViewOption;

        // ApplyViewOptions() does not need to be called
        if( pNewOpt )
        {
            *mpOpt = *pNewOpt;
            // Zoom factor needs to be set because there is no call to ApplyViewOptions() during
            // CTOR for performance reasons.
            if( GetWin() && 100 != mpOpt->GetZoom() )
            {
                MapMode aMode( mpWin->GetMapMode() );
                const Fraction aNewFactor( mpOpt->GetZoom(), 100 );
                aMode.SetScaleX( aNewFactor );
                aMode.SetScaleY( aNewFactor );
                mpWin->SetMapMode( aMode );
            }
        }
    }

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SwDocShell* pDShell = mpDoc->GetDocShell();
    mpDoc->GetDocumentSettingManager().set(DocumentSettingId::HTML_MODE, 0 != ::GetHtmlMode( pDShell ) );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SwDocShell* pDShell = mxDoc->GetDocShell();
    mxDoc->GetDocumentSettingManager().set(DocumentSettingId::HTML_MODE, 0 != ::GetHtmlMode( pDShell ) );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    // set readonly flag at ViewOptions before creating layout. Otherwise,
    // one would have to reformat again.

    if( pDShell && pDShell->IsReadOnly() )
        mpOpt->SetReadonly( true );

    SAL_INFO( "sw.core", "View::Init - before InitPrt" );
    OutputDevice* pPDFOut = nullptr;

    if ( mpOut && mpOut->GetPDFWriter() )
        pPDFOut = mpOut;

    // Only setup the printer if we need one:
    const bool bBrowseMode = mpOpt->getBrowseMode();
    if( pPDFOut )
        InitPrt( pPDFOut );

    // i#44963 Good occasion to check if page sizes in
    // page descriptions are still set to (LONG_MAX, LONG_MAX) (html import)
    if ( !bBrowseMode )
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mpDoc->CheckDefaultPageFormat();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->CheckDefaultPageFormat();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }

    SAL_INFO( "sw.core", "View::Init - after InitPrt" );
    if( GetWin() )
    {
        SwViewOption::Init( GetWin() );
        GetWin()->SetFillColor();
        GetWin()->SetBackground();
        GetWin()->SetLineColor();
    }

    // Create a new layout, if there is no one available
    if( !mpLayout )
    {
        // Here's the code which disables the usage of "multiple" layouts at the moment
        // If the problems with controls and groups objects are solved,
        // this code can be removed...
        SwViewShell *pCurrShell = GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
        if( pCurrShell )
            mpLayout = pCurrShell->mpLayout;
        // end of "disable multiple layouts"
        if( !mpLayout )
        {
            // switched to two step construction because creating the layout in SwRootFrame needs a valid pLayout set
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mpLayout = SwRootFramePtr(new SwRootFrame(mpDoc->GetDfltFrameFormat(), this),
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mpLayout = SwRootFramePtr(new SwRootFrame(mxDoc->GetDfltFrameFormat(), this),
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                                    &SwFrame::DestroyFrame);
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mpLayout->Init( mpDoc->GetDfltFrameFormat() );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mpLayout->Init( mxDoc->GetDfltFrameFormat() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        }
    }
    SizeChgNotify();

    // XForms mode: initialize XForms mode, based on design mode (draw view)
    //   MakeDrawView() requires layout
    if( GetDoc()->isXForms() )
    {
        if( ! HasDrawView() )
            MakeDrawView();
        mpOpt->SetFormView( ! GetDrawView()->IsDesignMode() );
    }
}

/// CTor for the first Shell.
SwViewShell::SwViewShell( SwDoc& rDocument, vcl::Window *pWindow,
                        const SwViewOption *pNewOpt, OutputDevice *pOutput,
                        long nFlags )
    :
    maBrowseBorder(),
    mpSfxViewShell( nullptr ),
    mpImp( new SwViewShellImp( this ) ),
    mpWin( pWindow ),
    mpOut( pOutput ? pOutput
                  : pWindow ? static_cast<OutputDevice*>(pWindow)
                            : static_cast<OutputDevice*>(rDocument.getIDocumentDeviceAccess().getPrinter( true ))),
    mpTmpRef( nullptr ),
    mpOpt( nullptr ),
    mpAccOptions( new SwAccessibilityOptions ),
    mbShowHeaderSeparator( false ),
    mbShowFooterSeparator( false ),
    mbHeaderFooterEdit( false ),
#ifdef USE_JAVA
    mbThumbnail(false),
    mbInCalcLayout(false),
#endif	// USE_JAVA
    mpTargetPaintWindow(nullptr),
    mpBufferedOut(nullptr),
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc( &rDocument ),
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mxDoc( &rDocument ),
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mnStartAction( 0 ),
    mnLockPaint( 0 ),
    mbSelectAll(false),
    mbOutputToWindow(false),
    mpPrePostOutDev(nullptr),
    maPrePostMapMode()
{
    // in order to suppress event handling in
    // <SwDrawContact::Changed> during construction of <SwViewShell> instance
    mbInConstructor = true;

    mbPaintInProgress = mbViewLocked = mbInEndAction = mbFrameView =
    mbEndActionByVirDev = false;
    mbPaintWorks = mbEnableSmooth = true;
    mbPreview = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );

    // i#38810 Do not reset modified state of document,
    // if it's already been modified.
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    const bool bIsDocModified( mpDoc->getIDocumentState().IsModified() );
    mpDoc->acquire();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    const bool bIsDocModified( mxDoc->getIDocumentState().IsModified() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    pOutput = mpOut;
    Init( pNewOpt );    // may change the Outdev (InitPrt())
    mpOut = pOutput;

    // initialize print preview layout after layout
    // is created in <SwViewShell::Init(..)> - called above.
    if ( mbPreview )
    {
        // init page preview layout
        mpImp->InitPagePreviewLayout();
    }

    SET_CURR_SHELL( this );

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    static_cast<SwHiddenTextFieldType*>(mpDoc->getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::HiddenText ))->
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    static_cast<SwHiddenTextFieldType*>(mxDoc->getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::HiddenText ))->
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SetHiddenFlag( !mpOpt->IsShowHiddenField() );

    // In Init a standard FrameFormat is created.
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (   !mpDoc->GetIDocumentUndoRedo().IsUndoNoResetModified()
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (   !mxDoc->GetIDocumentUndoRedo().IsUndoNoResetModified()
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        && !bIsDocModified )
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mpDoc->getIDocumentState().ResetModified();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->getIDocumentState().ResetModified();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }

    // extend format cache.
    if ( SwTextFrame::GetTextCache()->GetCurMax() < 2550 )
        SwTextFrame::GetTextCache()->IncreaseMax( 100 );
    if( mpOpt->IsGridVisible() || getIDocumentDrawModelAccess().GetDrawModel() )
        Imp()->MakeDrawView();

    mbInConstructor = false;
}

/// CTor for further Shells on a document.
SwViewShell::SwViewShell( SwViewShell& rShell, vcl::Window *pWindow,
                        OutputDevice * pOutput, long const nFlags)
    : Ring( &rShell ) ,
    maBrowseBorder( rShell.maBrowseBorder ),
    mpSfxViewShell( nullptr ),
    mpImp( new SwViewShellImp( this ) ),
    mpWin( pWindow ),
    mpOut( pOutput ? pOutput
                  : pWindow ? static_cast<OutputDevice*>(pWindow)
                            : static_cast<OutputDevice*>(rShell.GetDoc()->getIDocumentDeviceAccess().getPrinter( true ))),
    mpTmpRef( nullptr ),
    mpOpt( nullptr ),
    mpAccOptions( new SwAccessibilityOptions ),
    mbShowHeaderSeparator( false ),
    mbShowFooterSeparator( false ),
    mbHeaderFooterEdit( false ),
#ifdef USE_JAVA
    mbThumbnail(false),
    mbInCalcLayout(false),
#endif	// USE_JAVA
    mpTargetPaintWindow(nullptr),
    mpBufferedOut(nullptr),
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc( rShell.GetDoc() ),
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mxDoc( rShell.GetDoc() ),
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mnStartAction( 0 ),
    mnLockPaint( 0 ),
    mbSelectAll(false),
    mbOutputToWindow(false),
    mpPrePostOutDev(nullptr),
    maPrePostMapMode()
{
    // in order to suppress event handling in
    // <SwDrawContact::Changed> during construction of <SwViewShell> instance
    mbInConstructor = true;

    mbPaintWorks = mbEnableSmooth = true;
    mbPaintInProgress = mbViewLocked = mbInEndAction = mbFrameView =
    mbEndActionByVirDev = false;
    mbPreview = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );

    if( nFlags & VSHELLFLAG_SHARELAYOUT )
        mpLayout = rShell.mpLayout;

    SET_CURR_SHELL( this );

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc->acquire();
    bool bModified = mpDoc->getIDocumentState().IsModified();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    bool bModified = mxDoc->getIDocumentState().IsModified();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    pOutput = mpOut;
    Init( rShell.GetViewOptions() ); // might change Outdev (InitPrt())
    mpOut = pOutput;

    if ( mbPreview )
        mpImp->InitPagePreviewLayout();

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    static_cast<SwHiddenTextFieldType*>(mpDoc->getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::HiddenText ))->
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    static_cast<SwHiddenTextFieldType*>(mxDoc->getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::HiddenText ))->
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            SetHiddenFlag( !mpOpt->IsShowHiddenField() );

    // In Init a standard FrameFormat is created.
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if( !bModified && !mpDoc->GetIDocumentUndoRedo().IsUndoNoResetModified() )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if( !bModified && !mxDoc->GetIDocumentUndoRedo().IsUndoNoResetModified() )
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mpDoc->getIDocumentState().ResetModified();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->getIDocumentState().ResetModified();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }

    // extend format cache.
    if ( SwTextFrame::GetTextCache()->GetCurMax() < 2550 )
        SwTextFrame::GetTextCache()->IncreaseMax( 100 );
    if( mpOpt->IsGridVisible() || getIDocumentDrawModelAccess().GetDrawModel() )
        Imp()->MakeDrawView();

    mbInConstructor = false;

}

SwViewShell::~SwViewShell()
{
#ifndef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
#ifdef USE_JAVA
    IDocumentLayoutAccess * pLayoutAccess = mxDoc.get() ? &mxDoc->getIDocumentLayoutAccess() : nullptr;
#else	// USE_JAVA
    IDocumentLayoutAccess * const pLayoutAccess = mxDoc.get() ? &mxDoc->getIDocumentLayoutAccess() : nullptr;
#endif	// USE_JAVA
#endif	// !NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

    {
        SET_CURR_SHELL( this );
        mbPaintWorks = false;

        // i#9684 Stopping the animated graphics is not
        // necessary during printing or pdf export, because the animation
        // has not been started in this case.
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if( mpDoc && GetWin() )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if( mxDoc.get() && GetWin() )
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            SwNodes& rNds = mpDoc->GetNodes();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            SwNodes& rNds = mxDoc->GetNodes();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX

            SwStartNode *pStNd;
            SwNodeIndex aIdx( *rNds.GetEndOfAutotext().StartOfSectionNode(), 1 );
            while ( nullptr != (pStNd = aIdx.GetNode().GetStartNode()) )
            {
                ++aIdx;
                SwGrfNode *pGNd = aIdx.GetNode().GetGrfNode();
                if ( nullptr != pGNd )
                {
                    if( pGNd->IsAnimated() )
                    {
                        SwIterator<SwFrame,SwGrfNode> aIter( *pGNd );
                        for( SwFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next() )
                        {
                            OSL_ENSURE( pFrame->IsNoTextFrame(), "GraphicNode with Text?" );
                            static_cast<SwNoTextFrame*>(pFrame)->StopAnimation( mpOut );
                        }
                    }
                }
                aIdx.Assign( *pStNd->EndOfSectionNode(), +1 );
            }

            GetDoc()->StopNumRuleAnimations( mpOut );
        }

        delete mpImp; // Delete first, so that the LayoutViews are destroyed.
        mpImp = nullptr;   // Set to zero, because ~SwFrame relies on it.

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if ( mpDoc )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if ( mxDoc.get() )
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            if( !mpDoc->release() )
            {
                delete mpDoc;
                mpDoc = nullptr;
            }
            else
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            auto x = mxDoc->getReferenceCount();
            mxDoc.clear();
#ifdef USE_JAVA
            // Fix crash when printing comments by checking the current shell's
            // document is NULL
            if( x <= 1 )
                pLayoutAccess = nullptr;
            else
#else   // USE_JAVA 
            if( x > 1 )
#endif	// USE_JAVA 
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                GetLayout()->ResetNewLayout();
        }

        delete mpOpt;

        // resize format cache.
        if ( SwTextFrame::GetTextCache()->GetCurMax() > 250 )
            SwTextFrame::GetTextCache()->DecreaseMax( 100 );

        // Remove from PaintQueue if necessary
        SwPaintQueue::Remove( this );

        OSL_ENSURE( !mnStartAction, "EndAction() pending." );
    }

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if ( mpDoc )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if ( pLayoutAccess )
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    {
        GetLayout()->DeRegisterShell( this );
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        auto& rLayoutAccess(mpDoc->getIDocumentLayoutAccess());
        if(rLayoutAccess.GetCurrentViewShell()==this)
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if(pLayoutAccess->GetCurrentViewShell()==this)
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            rLayoutAccess.SetCurrentViewShell(nullptr);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            pLayoutAccess->SetCurrentViewShell(nullptr);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            for(SwViewShell& rShell : GetRingContainer())
            {
                if(&rShell != this)
                {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    rLayoutAccess.SetCurrentViewShell(&rShell);
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    pLayoutAccess->SetCurrentViewShell(&rShell);
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                    break;
                }
            }
        }
    }

    mpTmpRef.disposeAndClear();
    delete mpAccOptions;
}

bool SwViewShell::HasDrawView() const
{
    return Imp() && Imp()->HasDrawView();
}

void SwViewShell::MakeDrawView()
{
    Imp()->MakeDrawView( );
}

SdrView* SwViewShell::GetDrawView()
{
    return Imp()->GetDrawView();
}

SdrView* SwViewShell::GetDrawViewWithValidMarkList()
{
    SwDrawView* pDView = Imp()->GetDrawView();
    pDView->ValidateMarkList();
    return pDView;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
