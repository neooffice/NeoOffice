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
 *   Modified November 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <vcl/virdev.hxx>
#include <vcl/metric.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/printwarningoptions.hxx>
#include <svtools/printoptions.hxx>
#include <vector>

#include <sfx2/printer.hxx>
#include <sfx2/printopt.hxx>
#include "sfxtypes.hxx"
#include <sfx2/prnmon.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/sfxresid.hxx>
#include "view.hrc"

#ifdef USE_JAVA

#ifdef MACOSX
#include <svtools/prnsetup.hxx>
#endif	// MACOSX

#include "printer.h"

static ::boost::unordered_map< const SfxPrinter*, const SfxPrinter* > aPrinterMap;

#endif	// USE_JAVA

// struct SfxPrinter_Impl ------------------------------------------------

struct SfxPrinter_Impl
{
    bool            mbAll;
    bool            mbSelection;
    bool            mbFromTo;
    bool            mbRange;

    SfxPrinter_Impl() :
        mbAll       ( true ),
        mbSelection ( true ),
        mbFromTo    ( true ),
        mbRange     ( true ) {}
    ~SfxPrinter_Impl() {}
};

struct SfxPrintOptDlg_Impl
{
    bool        mbHelpDisabled;
#if defined USE_JAVA && defined MACOSX
    bool        mbShowPrintSetupDialog;
#endif	// USE_JAVA && MACOSX

    SfxPrintOptDlg_Impl() :
#if defined USE_JAVA && defined MACOSX
        mbHelpDisabled  ( false ), mbShowPrintSetupDialog( true ) {}
#else	// USE_JAVA && MACOSX
        mbHelpDisabled  ( false ) {}
#endif	// USE_JAVA && MACOSX
};

// class SfxPrinter ------------------------------------------------------

SfxPrinter* SfxPrinter::Create( SvStream& rStream, SfxItemSet* pOptions )

/*  [Description]

    Creates a <SfxPrinter> from the stream. Loading is really only a jobsetup.
    If such a printer is not available on the system, then the original is
    marked as the original Job-setup and a comparable printer is selected from
    existing ones.

    The 'pOptions' are taken over in the generated SfxPrinter, the return
    value belongs to the caller.
*/

{
    // Load JobSetup
    JobSetup aFileJobSetup;
    ReadJobSetup( rStream, aFileJobSetup );

    // Get printers
    SfxPrinter *pPrinter = new SfxPrinter( pOptions, aFileJobSetup );
    return pPrinter;
}



SvStream& SfxPrinter::Store( SvStream& rStream ) const

/*  [Description]

    Saves the used JobSetup of <SfxPrinter>s.
*/

{
    return WriteJobSetup( rStream, GetJobSetup() );
}



SfxPrinter::SfxPrinter( SfxItemSet* pTheOptions ) :

/*  [Description]

    This constructor creates a default printer.
*/

    pOptions( pTheOptions ),
    bKnown(true)

{
    assert(pOptions);
    pImpl = new SfxPrinter_Impl;
#ifdef USE_JAVA
    aPrinterMap[ this ] = this;
#endif	// USE_JAVA
}



SfxPrinter::SfxPrinter( SfxItemSet* pTheOptions,
                        const JobSetup& rTheOrigJobSetup ) :

    Printer         ( rTheOrigJobSetup.GetPrinterName() ),
    pOptions        ( pTheOptions )

{
    assert(pOptions);
    pImpl = new SfxPrinter_Impl;
    bKnown = GetName() == rTheOrigJobSetup.GetPrinterName();

    if ( bKnown )
        SetJobSetup( rTheOrigJobSetup );

#ifdef USE_JAVA
    aPrinterMap[ this ] = this;
#endif	// USE_JAVA
}



SfxPrinter::SfxPrinter( SfxItemSet* pTheOptions,
                        const OUString& rPrinterName ) :

    Printer         ( rPrinterName ),
    pOptions        ( pTheOptions ),
    bKnown          ( GetName() == rPrinterName )

{
    assert(pOptions);
    pImpl = new SfxPrinter_Impl;
#ifdef USE_JAVA
    aPrinterMap[ this ] = this;
#endif	// USE_JAVA
}



SfxPrinter::SfxPrinter( const SfxPrinter& rPrinter ) :

    Printer ( rPrinter.GetName() ),
    pOptions( rPrinter.GetOptions().Clone() ),
    bKnown  ( rPrinter.IsKnown() )
{
    assert(pOptions);
    SetJobSetup( rPrinter.GetJobSetup() );
    SetPrinterProps( &rPrinter );
    SetMapMode( rPrinter.GetMapMode() );

    pImpl = new SfxPrinter_Impl;
    pImpl->mbAll = rPrinter.pImpl->mbAll;
    pImpl->mbSelection = rPrinter.pImpl->mbSelection;
    pImpl->mbFromTo = rPrinter.pImpl->mbFromTo;
    pImpl->mbRange = rPrinter.pImpl->mbRange;

#ifdef USE_JAVA
    aPrinterMap[ this ] = this;
#endif	// USE_JAVA
}



SfxPrinter* SfxPrinter::Clone() const
{
    if ( IsDefPrinter() )
    {
        SfxPrinter *pNewPrinter;
        pNewPrinter = new SfxPrinter( GetOptions().Clone() );
        pNewPrinter->SetJobSetup( GetJobSetup() );
        pNewPrinter->SetPrinterProps( this );
        pNewPrinter->SetMapMode( GetMapMode() );
        pNewPrinter->pImpl->mbAll = pImpl->mbAll;
        pNewPrinter->pImpl->mbSelection =pImpl->mbSelection;
        pNewPrinter->pImpl->mbFromTo = pImpl->mbFromTo;
        pNewPrinter->pImpl->mbRange =pImpl->mbRange;
        return pNewPrinter;
    }
    else
        return new SfxPrinter( *this );
}



SfxPrinter::~SfxPrinter()
{
#ifdef USE_JAVA
    ::boost::unordered_map< const SfxPrinter*, const SfxPrinter* >::iterator it = aPrinterMap.find( this );
    if ( it != aPrinterMap.end() )
        aPrinterMap.erase( it );
#endif	// USE_JAVA

    delete pOptions;
    delete pImpl;
}



void SfxPrinter::SetOptions( const SfxItemSet &rNewOptions )
{
    pOptions->Set(rNewOptions);
}



SfxPrintOptionsDialog::SfxPrintOptionsDialog(vcl::Window *pParent,
                                              SfxViewShell *pViewShell,
                                              const SfxItemSet *pSet)

    : ModalDialog(pParent, "PrinterOptionsDialog",
        "sfx/ui/printeroptionsdialog.ui")
    , pDlgImpl(new SfxPrintOptDlg_Impl)
    , pViewSh(pViewShell)
    , pOptions(pSet->Clone())
{
    VclContainer *pVBox = get_content_area();

    // Insert TabPage
    pPage = pViewSh->CreatePrintOptionsPage(pVBox, *pOptions);
    DBG_ASSERT( pPage, "CreatePrintOptions != SFX_VIEW_HAS_PRINTOPTIONS" );
    if( pPage )
    {
        pPage->Reset( pOptions );
        SetHelpId( pPage->GetHelpId() );
        pPage->Show();
    }
}



SfxPrintOptionsDialog::~SfxPrintOptionsDialog()
{
    delete pDlgImpl;
    delete pPage;
    delete pOptions;
}



short SfxPrintOptionsDialog::Execute()
{
    if( ! pPage )
#if defined USE_JAVA && defined MACOSX
    {
        if ( pDlgImpl->mbShowPrintSetupDialog )
        {
            PrinterSetupDialog *pSetupDlg = dynamic_cast< PrinterSetupDialog* >( GetParent() );
            if ( pSetupDlg )
            {
                Printer *pPrinter = pSetupDlg->GetPrinter();
                if ( pPrinter )
                {
                    pDlgImpl->mbShowPrintSetupDialog = false;

                    Printer aTempPrinter( pPrinter->GetJobSetup() );

                    // If the user presses the native page setup dialog's OK
                    // button, signal that action to the code in the
                    // svtools/source/dialog/prnsetup.cxx file by changing
                    // the dialog's printer
                    if ( aTempPrinter.Setup( this ) )
                    {
                        SfxPrinter *pViewPrinter = pViewSh->GetPrinter( sal_True );
                        if ( pViewPrinter )
                        {
                            pViewPrinter->SetJobSetup( aTempPrinter.GetJobSetup() );
                            pViewSh->SetPrinter( pViewPrinter, SFX_PRINTER_CHG_ORIENTATION | SFX_PRINTER_CHG_SIZE );
                        }
                    }
                }
            }
        }
#endif	// USE_JAVA && MACOSX
        return RET_CANCEL;
#if defined USE_JAVA && defined MACOSX
    }
#endif	// USE_JAVA && MACOSX

    short nRet = ModalDialog::Execute();
    if ( nRet == RET_OK )
        pPage->FillItemSet( pOptions );
    else
        pPage->Reset( pOptions );
    return nRet;
}



bool SfxPrintOptionsDialog::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        if ( rNEvt.GetKeyEvent()->GetKeyCode().GetCode() == KEY_F1 && pDlgImpl->mbHelpDisabled )
            return true; // help disabled -> <F1> does nothing
    }
#if defined USE_JAVA && defined MACOSX
    else if ( pDlgImpl->mbShowPrintSetupDialog && rNEvt.GetType() == EVENT_GETFOCUS )
    {
        PrinterSetupDialog *pSetupDlg = dynamic_cast< PrinterSetupDialog* >( GetParent() );
        if ( pSetupDlg )
        {
            Printer *pPrinter = pSetupDlg->GetPrinter();
            if ( pPrinter )
            {
                Printer *pTempPrinter = new Printer( pPrinter->GetJobSetup() );
                if ( pTempPrinter )
                {
                    pDlgImpl->mbShowPrintSetupDialog = false;

                    // If the user presses the native page setup dialog's OK
                    // button, signal that action to the code in the
                    // svtools/source/dialog/prnsetup.cxx file by changing the
                    // dialog's printer
                    if ( pTempPrinter->Setup( this ) )
                        pSetupDlg->SetPrinter( pTempPrinter );
                    else
                        delete pTempPrinter;
                }
            }
        }
    }
#endif	// USE_JAVA && MACOSX

    return ModalDialog::Notify( rNEvt );
}



void SfxPrintOptionsDialog::DisableHelp()
{
    pDlgImpl->mbHelpDisabled = true;

    get<HelpButton>("help")->Disable();
}

#ifdef USE_JAVA

bool ImplIsValidSfxPrinter( const SfxPrinter *pPrinter )
{
    ::boost::unordered_map< const SfxPrinter*, const SfxPrinter* >::const_iterator it = aPrinterMap.find( pPrinter );
    return ( it != aPrinterMap.end() ? true : false );
}

#endif	// USE_JAVA

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
