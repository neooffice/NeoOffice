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
#include <switerator.hxx>

#ifdef USE_JAVA

#include <unordered_map>

static ::std::unordered_map< const SwViewShell*, const SwViewShell* > aViewShellMap;

#endif	// USE_JAVA

void SwViewShell::Init( const SwViewOption *pNewOpt )
{
    mbDocSizeChgd = false;

    // We play it save: Remove old font information whenever the printer
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
    mpDoc->GetDocumentSettingManager().set(IDocumentSettingAccess::HTML_MODE, 0 != ::GetHtmlMode( pDShell ) );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    SwDocShell* pDShell = mxDoc->GetDocShell();
    mxDoc->GetDocumentSettingManager().set(IDocumentSettingAccess::HTML_MODE, 0 != ::GetHtmlMode( pDShell ) );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    // JP 02.02.99: Bug 61335 - set readonly flag at ViewOptions before creating layout. Otherwise,
    //                          one would have to reformat again.

    if( pDShell && pDShell->IsReadOnly() )
        mpOpt->SetReadonly( true );

    SAL_INFO( "sw.core", "View::Init - before InitPrt" );
    // --> FME 2007-11-06 #i82967#
    OutputDevice* pPDFOut = 0;

    if ( mpOut && mpOut->GetPDFWriter() )
        pPDFOut = mpOut;
    // <--

    // --> FME 2005-01-21 #i41075#
    // Only setup the printer if we need one:
    const bool bBrowseMode = mpOpt->getBrowseMode();
    if( pPDFOut )
        InitPrt( pPDFOut );
    // <--

    // --> FME 2005-03-16 #i44963# Good occasion to check if page sizes in
    // page descriptions are still set to (LONG_MAX, LONG_MAX) (html import)
    if ( !bBrowseMode )
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mpDoc->CheckDefaultPageFmt();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->CheckDefaultPageFmt();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }
    // <--

    SAL_INFO( "sw.core", "View::Init - after InitPrt" );
    if( GetWin() )
    {
        mpOpt->Init( GetWin() );
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
            // switched to two step construction because creating the layout in SwRootFrm needs a valid pLayout set
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mpLayout = SwRootFrmPtr(new SwRootFrm( mpDoc->GetDfltFrmFmt(), this ));
            mpLayout->Init( mpDoc->GetDfltFrmFmt() );
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            mpLayout = SwRootFrmPtr(new SwRootFrm(mxDoc->GetDfltFrmFmt(), this));
            mpLayout->Init( mxDoc->GetDfltFrmFmt() );
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        }
    }
    SizeChgNotify();

    // --> #i31958#
    // XForms mode: initialize XForms mode, based on design mode (draw view)
    //   MakeDrawView() requires layout
    if( GetDoc()->isXForms() )
    {
        if( ! HasDrawView() )
            MakeDrawView();
        mpOpt->SetFormView( ! GetDrawView()->IsDesignMode() );
    }
    // <-- #i31958#
}

/// CTor for the first Shell.
SwViewShell::SwViewShell( SwDoc& rDocument, vcl::Window *pWindow,
                        const SwViewOption *pNewOpt, OutputDevice *pOutput,
                        long nFlags )
    :
    maBrowseBorder(),
    mpSfxViewShell( 0 ),
    mpImp( new SwViewImp( this ) ),
    mpWin( pWindow ),
    mpOut( pOutput ? pOutput
                  : pWindow ? (OutputDevice*)pWindow
                            : (OutputDevice*)rDocument.getIDocumentDeviceAccess().getPrinter( true )),
    mpTmpRef( 0 ),
    mpOpt( 0 ),
    mpAccOptions( new SwAccessibilityOptions ),
    mbShowHeaderSeparator( false ),
    mbShowFooterSeparator( false ),
    mbHeaderFooterEdit( false ),
    mbTiledRendering(false),
#ifdef USE_JAVA
    mbThumbnail(false),
    mbInCalcLayout(false),
#endif	// USE_JAVA
    mpTargetPaintWindow(0), // #i74769#
    mpBufferedOut(0), // #i74769#
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc( &rDocument ),
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mxDoc( &rDocument ),
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mnStartAction( 0 ),
    mnLockPaint( 0 ),
    mbSelectAll(false),
    mpPrePostOutDev(0), // #i72754#
    maPrePostMapMode()
{
    // OD 2004-06-01 #i26791# - in order to suppress event handling in
    // <SwDrawContact::Changed> during construction of <SwViewShell> instance
    mbInConstructor = true;

    mbPaintInProgress = mbViewLocked = mbInEndAction = mbFrameView =
    mbEndActionByVirDev = false;
    mbPaintWorks = mbEnableSmooth = true;
    mbPreview = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );

    // --> OD 2005-02-11 #i38810# - Do not reset modified state of document,
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

    // OD 28.03.2003 #108470# - initialize print preview layout after layout
    // is created in <SwViewShell::Init(..)> - called above.
    if ( mbPreview )
    {
        // OD 12.12.2002 #103492# - init page preview layout
        mpImp->InitPagePreviewLayout();
    }

    SET_CURR_SHELL( this );

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    ((SwHiddenTxtFieldType*)mpDoc->getIDocumentFieldsAccess().GetSysFldType( RES_HIDDENTXTFLD ))->
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    static_cast<SwHiddenTxtFieldType*>(mxDoc->getIDocumentFieldsAccess().GetSysFldType( RES_HIDDENTXTFLD ))->
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        SetHiddenFlag( !mpOpt->IsShowHiddenField() );

    // In Init a standard FrmFmt is created.
    // --> OD 2005-02-11 #i38810#
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (   !mpDoc->GetIDocumentUndoRedo().IsUndoNoResetModified()
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    if (   !mxDoc->GetIDocumentUndoRedo().IsUndoNoResetModified()
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        && !bIsDocModified )
    // <--
    {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mpDoc->getIDocumentState().ResetModified();
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        mxDoc->getIDocumentState().ResetModified();
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    }

    // extend format cache.
    if ( SwTxtFrm::GetTxtCache()->GetCurMax() < 2550 )
        SwTxtFrm::GetTxtCache()->IncreaseMax( 100 );
    if( mpOpt->IsGridVisible() || getIDocumentDrawModelAccess()->GetDrawModel() )
        Imp()->MakeDrawView();

    // OD 2004-06-01 #i26791#
    mbInConstructor = false;

#ifdef USE_JAVA
    aViewShellMap[ this ] = this;
#endif	// USE_JAVA
}

/// CTor for further Shells on a document.
SwViewShell::SwViewShell( SwViewShell& rShell, vcl::Window *pWindow,
                        OutputDevice *pOutput, long nFlags ) :
    Ring( &rShell ),
    maBrowseBorder( rShell.maBrowseBorder ),
    mpSfxViewShell( 0 ),
    mpImp( new SwViewImp( this ) ),
    mpWin( pWindow ),
    mpOut( pOutput ? pOutput
                  : pWindow ? (OutputDevice*)pWindow
                            : (OutputDevice*)rShell.GetDoc()->getIDocumentDeviceAccess().getPrinter( true )),
    mpTmpRef( 0 ),
    mpOpt( 0 ),
    mpAccOptions( new SwAccessibilityOptions ),
    mbShowHeaderSeparator( false ),
    mbShowFooterSeparator( false ),
    mbHeaderFooterEdit( false ),
    mbTiledRendering(false),
#ifdef USE_JAVA
    mbThumbnail(false),
    mbInCalcLayout(false),
#endif	// USE_JAVA
    mpTargetPaintWindow(0), // #i74769#
    mpBufferedOut(0), // #i74769#
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mpDoc( rShell.GetDoc() ),
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mxDoc( rShell.GetDoc() ),
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    mnStartAction( 0 ),
    mnLockPaint( 0 ),
    mbSelectAll(false),
    mpPrePostOutDev(0), // #i72754#
    maPrePostMapMode()
{
    // OD 2004-06-01 #i26791# - in order to suppress event handling in
    // <SwDrawContact::Changed> during construction of <SwViewShell> instance
    mbInConstructor = true;

    mbPaintWorks = mbEnableSmooth = true;
    mbPaintInProgress = mbViewLocked = mbInEndAction = mbFrameView =
    mbEndActionByVirDev = false;
    mbPreview = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );
    // OD 12.12.2002 #103492#
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

    // OD 12.12.2002 #103492#
    if ( mbPreview )
        mpImp->InitPagePreviewLayout();

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    ((SwHiddenTxtFieldType*)mpDoc->getIDocumentFieldsAccess().GetSysFldType( RES_HIDDENTXTFLD ))->
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
    static_cast<SwHiddenTxtFieldType*>(mxDoc->getIDocumentFieldsAccess().GetSysFldType( RES_HIDDENTXTFLD ))->
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            SetHiddenFlag( !mpOpt->IsShowHiddenField() );

    // In Init a standard FrmFmt is created.
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
    if ( SwTxtFrm::GetTxtCache()->GetCurMax() < 2550 )
        SwTxtFrm::GetTxtCache()->IncreaseMax( 100 );
    if( mpOpt->IsGridVisible() || getIDocumentDrawModelAccess()->GetDrawModel() )
        Imp()->MakeDrawView();

    // OD 2004-06-01 #i26791#
    mbInConstructor = false;

#ifdef USE_JAVA
    aViewShellMap[ this ] = this;
#endif	// USE_JAVA
}

SwViewShell::~SwViewShell()
{
#ifdef USE_JAVA
    ::std::unordered_map< const SwViewShell*, const SwViewShell* >::iterator it = aViewShellMap.find( this );
    if ( it != aViewShellMap.end() )
        aViewShellMap.erase( it );
#endif	// USE_JAVA

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

        // FME 2004-06-21 #i9684# Stopping the animated graphics is not
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
            SwGrfNode *pGNd;

            SwStartNode *pStNd;
            SwNodeIndex aIdx( *rNds.GetEndOfAutotext().StartOfSectionNode(), 1 );
            while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
            {
                ++aIdx;
                if ( 0 != ( pGNd = aIdx.GetNode().GetGrfNode() ) )
                {
                    if( pGNd->IsAnimated() )
                    {
                        SwIterator<SwFrm,SwGrfNode> aIter( *pGNd );
                        for( SwFrm* pFrm = aIter.First(); pFrm; pFrm = aIter.Next() )
                        {
                            OSL_ENSURE( pFrm->IsNoTxtFrm(), "GraphicNode with Text?" );
                            ((SwNoTxtFrm*)pFrm)->StopAnimation( mpOut );
                        }
                    }
                }
                aIdx.Assign( *pStNd->EndOfSectionNode(), +1 );
            }

            GetDoc()->StopNumRuleAnimations( mpOut );
        }

        delete mpImp; // Delete first, so that the LayoutViews are destroyed.
        mpImp = 0;   // Set to zero, because ~SwFrm relies on it.

#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if ( mpDoc )
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if ( mxDoc.get() )
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        {
#ifdef NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            if( !mpDoc->release() )
                delete mpDoc, mpDoc = 0;
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
#else	// USE_JAVA
            if( x > 1 )
#endif	// USE_JAVA
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
                GetLayout()->ResetNewLayout();
        }

        delete mpOpt;

        // resize format cache.
        if ( SwTxtFrm::GetTxtCache()->GetCurMax() > 250 )
            SwTxtFrm::GetTxtCache()->DecreaseMax( 100 );

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
        if(mpDoc->getIDocumentLayoutAccess().GetCurrentViewShell()==this)
            mpDoc->getIDocumentLayoutAccess().SetCurrentViewShell( this->GetNext()!=this ?
#else	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
        if(pLayoutAccess->GetCurrentViewShell()==this)
            pLayoutAccess->SetCurrentViewShell( this->GetNext()!=this ?
#endif	// NO_LIBO_SWDOC_ACQUIRE_LEAK_FIX
            (SwViewShell*)this->GetNext() : NULL );
    }

    delete mpTmpRef;
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

#ifdef USE_JAVA

bool ImplIsValidSwViewShell( const SwViewShell *pViewShell )
{
    ::std::unordered_map< const SwViewShell*, const SwViewShell* >::const_iterator it = aViewShellMap.find( pViewShell );
    return ( it != aViewShellMap.end() ? true : false );
}

#endif	// USE_JAVA

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
