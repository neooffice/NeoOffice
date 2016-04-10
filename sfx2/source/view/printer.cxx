/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 * This file incorporates work covered by the following license notice:
 * 
 *   Modified April 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 4
 *   of the Apache License, Version 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <vcl/virdev.hxx>
#include <vcl/metric.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/printwarningoptions.hxx>
#include <svtools/printoptions.hxx>
#include <vector>

#ifndef GCC
#endif

#include <sfx2/printer.hxx>
#include <sfx2/printopt.hxx>
#include "sfxtypes.hxx"
#include <sfx2/prnmon.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/tabdlg.hxx>
#include "sfx2/sfxresid.hxx"
#include "view.hrc"

#if defined USE_JAVA && defined MACOSX
#include <svtools/prnsetup.hxx>
#endif	// USE_JAVA && MACOSX

// struct SfxPrinter_Impl ------------------------------------------------

struct SfxPrinter_Impl
{
	sal_Bool			mbAll;
	sal_Bool			mbSelection;
	sal_Bool			mbFromTo;
	sal_Bool			mbRange;

	SfxPrinter_Impl() :
		mbAll		( sal_True ),
		mbSelection ( sal_True ),
		mbFromTo	( sal_True ),
		mbRange 	( sal_True ) {}
	~SfxPrinter_Impl() {}
};

struct SfxPrintOptDlg_Impl
{
	sal_Bool		mbHelpDisabled;
#if defined USE_JAVA && defined MACOSX
	sal_Bool		mbShowPrintSetupDialog;
#endif	// USE_JAVA && MACOSX

	SfxPrintOptDlg_Impl() :
#if defined USE_JAVA && defined MACOSX
		mbHelpDisabled	( sal_False ), mbShowPrintSetupDialog( sal_True ) {}
#else	// USE_JAVA && MACOSX
		mbHelpDisabled	( sal_False ) {}
#endif	// USE_JAVA && MACOSX
};

// class SfxPrinter ------------------------------------------------------

SfxPrinter* SfxPrinter::Create( SvStream& rStream, SfxItemSet* pOptions )

/* 	[Beschreibung]

	Erzeugt einen <SfxPrinter> aus dem Stream. Geladen wird genaugenommen
	nur ein JobSetup. Falls ein solcher Drucker auf dem System nicht
	verf"augbar ist, wird das Original als Orig-JobSetup gemerkt und
	ein "anhlicher exisitierender Drucker genommen.

	Die 'pOptions' werden in den erzeugten SfxPrinter "ubernommen,
	der Returnwert geh"ort dem Caller.
*/

{
	// JobSetup laden
	JobSetup aFileJobSetup;
	rStream >> aFileJobSetup;

	// Drucker erzeugen
	SfxPrinter *pPrinter = new SfxPrinter( pOptions, aFileJobSetup );
	return pPrinter;
}

//--------------------------------------------------------------------

SvStream& SfxPrinter::Store( SvStream& rStream ) const

/*	[Beschreibung]

	Speichert das verwendete JobSetup des <SfxPrinter>s.
*/

{
	return ( rStream << GetJobSetup() );
}

//--------------------------------------------------------------------

SfxPrinter::SfxPrinter( SfxItemSet* pTheOptions ) :

/*  [Beschreibung]

	Dieser Ctor erzeugt einen Standard-Drucker.
*/

	pOptions( pTheOptions ),
	bKnown(sal_True)

{
	pImpl = new SfxPrinter_Impl;
}

//--------------------------------------------------------------------

SfxPrinter::SfxPrinter( SfxItemSet* pTheOptions,
						const JobSetup& rTheOrigJobSetup ) :

	Printer			( rTheOrigJobSetup.GetPrinterName() ),
	pOptions		( pTheOptions )

{
	pImpl = new SfxPrinter_Impl;
	bKnown = GetName() == rTheOrigJobSetup.GetPrinterName();

	if ( bKnown )
		SetJobSetup( rTheOrigJobSetup );
}

//--------------------------------------------------------------------

SfxPrinter::SfxPrinter( SfxItemSet* pTheOptions,
						const String& rPrinterName ) :

	Printer			( rPrinterName ),
	pOptions		( pTheOptions ),
	bKnown			( GetName() == rPrinterName )

{
	pImpl = new SfxPrinter_Impl;
}

//--------------------------------------------------------------------

SfxPrinter::SfxPrinter( const SfxPrinter& rPrinter ) :

	Printer	( rPrinter.GetName() ),
	pOptions( rPrinter.GetOptions().Clone() ),
	bKnown	( rPrinter.IsKnown() )
{
	SetJobSetup( rPrinter.GetJobSetup() );
	SetPrinterProps( &rPrinter );
	SetMapMode( rPrinter.GetMapMode() );

	pImpl = new SfxPrinter_Impl;
	pImpl->mbAll = rPrinter.pImpl->mbAll;
	pImpl->mbSelection = rPrinter.pImpl->mbSelection;
	pImpl->mbFromTo = rPrinter.pImpl->mbFromTo;
	pImpl->mbRange = rPrinter.pImpl->mbRange;
}

//--------------------------------------------------------------------

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

//--------------------------------------------------------------------

SfxPrinter::~SfxPrinter()
{
	delete pOptions;
	delete pImpl;
}

//--------------------------------------------------------------------

void SfxPrinter::SetOptions( const SfxItemSet &rNewOptions )
{
	pOptions->Set(rNewOptions);
}

//--------------------------------------------------------------------

SfxPrintOptionsDialog::SfxPrintOptionsDialog( Window *pParent,
											  SfxViewShell *pViewShell,
											  const SfxItemSet *pSet ) :

	ModalDialog( pParent, WinBits( WB_STDMODAL | WB_3DLOOK ) ),

	aOkBtn		( this ),
	aCancelBtn	( this ),
	aHelpBtn	( this ),
	pDlgImpl	( new SfxPrintOptDlg_Impl ),
	pViewSh		( pViewShell ),
	pOptions	( pSet->Clone() ),
	pPage		( NULL )

{
	SetText( SfxResId( STR_PRINT_OPTIONS_TITLE ) );

	// TabPage einh"angen
	pPage = pViewSh->CreatePrintOptionsPage( this, *pOptions );
	DBG_ASSERT( pPage, "CreatePrintOptions != SFX_VIEW_HAS_PRINTOPTIONS" );
    if( pPage )
    {
        pPage->Reset( *pOptions );
        SetHelpId( pPage->GetHelpId() );
        pPage->Show();
    }

	// Dialoggr"o\se bestimmen
    Size a6Sz = LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
	Size aBtnSz = LogicToPixel( Size( 50, 14 ), MAP_APPFONT );
    Size aOutSz( pPage ? pPage->GetSizePixel() : Size() );
	aOutSz.Height() += 6;
    long nWidth = aBtnSz.Width();
    nWidth += a6Sz.Width();
    aOutSz.Width() += nWidth;
	if ( aOutSz.Height() < 90 )
		// mindestens die H"ohe der 3 Buttons
		aOutSz.Height() = 90;
	SetOutputSizePixel( aOutSz );

	// set position and size of the buttons
    Point aBtnPos( aOutSz.Width() - aBtnSz.Width() - a6Sz.Width(), a6Sz.Height() );
	aOkBtn.SetPosSizePixel( aBtnPos, aBtnSz );
	aBtnPos.Y() += aBtnSz.Height() + ( a6Sz.Height() / 2 );
	aCancelBtn.SetPosSizePixel( aBtnPos, aBtnSz );
	aBtnPos.Y() += aBtnSz.Height() + a6Sz.Height();
	aHelpBtn.SetPosSizePixel( aBtnPos, aBtnSz );

	aCancelBtn.Show();
	aOkBtn.Show();
	aHelpBtn.Show();
}

//--------------------------------------------------------------------

SfxPrintOptionsDialog::~SfxPrintOptionsDialog()
{
	delete pDlgImpl;
	delete pPage;
	delete pOptions;
}

//--------------------------------------------------------------------

short SfxPrintOptionsDialog::Execute()
{
    if( ! pPage )
        return RET_CANCEL;
    
	short nRet = ModalDialog::Execute();
	if ( nRet == RET_OK )
        pPage->FillItemSet( *pOptions );
	else
        pPage->Reset( *pOptions );
	return nRet;
}

//--------------------------------------------------------------------

long SfxPrintOptionsDialog::Notify( NotifyEvent& rNEvt )
{
	if ( rNEvt.GetType() == EVENT_KEYINPUT )
	{
		if ( rNEvt.GetKeyEvent()->GetKeyCode().GetCode() == KEY_F1 && pDlgImpl->mbHelpDisabled )
			return 1; // help disabled -> <F1> does nothing
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

//--------------------------------------------------------------------

void SfxPrintOptionsDialog::DisableHelp()
{
	pDlgImpl->mbHelpDisabled = sal_True;

	aHelpBtn.Disable();
}

