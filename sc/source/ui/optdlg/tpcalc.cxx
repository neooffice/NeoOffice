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
 * Modified January 2010 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include <math.h>

#include "scitems.hxx"
#include <vcl/msgbox.hxx>

#include "global.hxx"
#include "globstr.hrc"
#include "uiitems.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "docoptio.hxx"
#include "scresid.hxx"
#include "sc.hrc"       // -> Slot-IDs
#include "optdlg.hrc"

#define _TPCALC_CXX
#include "tpcalc.hxx"
#undef _TPCALC_CXX

// STATIC DATA -----------------------------------------------------------

static USHORT pCalcOptRanges[] =
{
	SID_SCDOCOPTIONS,
	SID_SCDOCOPTIONS,
	0
};

//========================================================================

ScTpCalcOptions::ScTpCalcOptions( Window*			pParent,
								  const SfxItemSet& rCoreAttrs )

	:	SfxTabPage		( pParent,
						  ScResId( RID_SCPAGE_CALC ),
						  rCoreAttrs ),

		aGbZRefs		( this, ScResId( GB_ZREFS ) ),
		aBtnIterate 	( this, ScResId( BTN_ITERATE ) ),
		aFtSteps		( this, ScResId( FT_STEPS ) ),
		aEdSteps		( this, ScResId( ED_STEPS ) ),
		aFtEps			( this, ScResId( FT_EPS ) ),
		aEdEps			( this, ScResId( ED_EPS ) ),
        aSeparatorFL    ( this, ScResId( FL_SEPARATOR ) ),
		aGbDate 		( this, ScResId( GB_DATE ) ),
		aBtnDateStd 	( this, ScResId( BTN_DATESTD ) ),
		aBtnDateSc10	( this, ScResId( BTN_DATESC10 ) ),
		aBtnDate1904	( this, ScResId( BTN_DATE1904 ) ),
        aHSeparatorFL   ( this, ScResId( FL_H_SEPARATOR ) ),
		aBtnCase	 	( this, ScResId( BTN_CASE ) ),
		aBtnCalc	 	( this, ScResId( BTN_CALC ) ),
		aBtnMatch	 	( this, ScResId( BTN_MATCH ) ),
        aBtnRegex       ( this, ScResId( BTN_REGEX ) ),
		aBtnLookUp   	( this, ScResId( BTN_LOOKUP ) ),
#ifdef USE_JAVA
        // Fix bug 3587 by disabling Excel syntax
        aFtFormulaSyntax( this ),
        aLBFormulaSyntax( this ),
#else	// USE_JAVA
        aFtFormulaSyntax( this, ScResId( FT_FORMULA_SYNTAX ) ),
        aLBFormulaSyntax( this, ScResId( LB_FORMULA_SYNTAX ) ),
#endif	// USE_JAVA
		aFtPrec 		( this, ScResId( FT_PREC ) ),
		aEdPrec 		( this, ScResId( ED_PREC ) ),
		pOldOptions 	( new ScDocOptions(
							((const ScTpCalcItem&)rCoreAttrs.Get(
								GetWhich( SID_SCDOCOPTIONS ))).
									GetDocOptions() ) ),
		pLocalOptions	( new ScDocOptions ),
		nWhichCalc		( GetWhich( SID_SCDOCOPTIONS ) )
{
    aSeparatorFL.SetStyle( aSeparatorFL.GetStyle() | WB_VERT );
    Init();
	FreeResource();
	SetExchangeSupport();
}

//-----------------------------------------------------------------------

__EXPORT ScTpCalcOptions::~ScTpCalcOptions()
{
	delete pOldOptions;
	delete pLocalOptions;
}

//-----------------------------------------------------------------------

void ScTpCalcOptions::Init()
{
	aBtnIterate .SetClickHdl( LINK( this, ScTpCalcOptions, CheckClickHdl ) );
	aBtnDateStd .SetClickHdl( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
	aBtnDateSc10.SetClickHdl( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
	aBtnDate1904.SetClickHdl( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
}

//-----------------------------------------------------------------------

USHORT* __EXPORT ScTpCalcOptions::GetRanges()
{
	return pCalcOptRanges;
}

//-----------------------------------------------------------------------

SfxTabPage* __EXPORT ScTpCalcOptions::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
	return ( new ScTpCalcOptions( pParent, rAttrSet ) );
}

//-----------------------------------------------------------------------

void __EXPORT ScTpCalcOptions::Reset( const SfxItemSet& /* rCoreAttrs */ )
{
	USHORT	d,m,y;

	*pLocalOptions	= *pOldOptions;

	aBtnCase   .Check( !pLocalOptions->IsIgnoreCase() );
	aBtnCalc   .Check( pLocalOptions->IsCalcAsShown() );
	aBtnMatch  .Check( pLocalOptions->IsMatchWholeCell() );
    aBtnRegex  .Check( pLocalOptions->IsFormulaRegexEnabled() );
	aBtnLookUp .Check( pLocalOptions->IsLookUpColRowNames() );
	aBtnIterate.Check( pLocalOptions->IsIter() );
	aEdSteps   .SetValue( pLocalOptions->GetIterCount() );
	aEdPrec    .SetValue( pLocalOptions->GetStdPrecision() );
    aEdEps     .SetValue( pLocalOptions->GetIterEps(), 6 );

#ifndef USE_JAVA
    ScGrammar::Grammar eGram = pLocalOptions->GetFormulaSyntax();
    switch (eGram)
    {
        case ScGrammar::GRAM_NATIVE:
            aLBFormulaSyntax.SelectEntryPos(0);
        break;
        case ScGrammar::GRAM_NATIVE_XL_A1:
            aLBFormulaSyntax.SelectEntryPos(1);
        break;
        case ScGrammar::GRAM_NATIVE_XL_R1C1:
            aLBFormulaSyntax.SelectEntryPos(2);
        break;
        default:
            aLBFormulaSyntax.SelectEntryPos(0);
    }
#endif	// !USE_JAVA

	pLocalOptions->GetDate( d, m, y );

	switch ( y )
	{
		case 1899:
			aBtnDateStd.Check();
			break;
		case 1900:
			aBtnDateSc10.Check();
			break;
		case 1904:
			aBtnDate1904.Check();
			break;
	}

	CheckClickHdl( &aBtnIterate );
}


//-----------------------------------------------------------------------

BOOL __EXPORT ScTpCalcOptions::FillItemSet( SfxItemSet& rCoreAttrs )
{
	// alle weiteren Optionen werden in den Handlern aktualisiert
	pLocalOptions->SetIterCount( (USHORT)aEdSteps.GetValue() );
    pLocalOptions->SetStdPrecision( (USHORT)aEdPrec.GetValue() );
	pLocalOptions->SetIgnoreCase( !aBtnCase.IsChecked() );
	pLocalOptions->SetCalcAsShown( aBtnCalc.IsChecked() );
	pLocalOptions->SetMatchWholeCell( aBtnMatch.IsChecked() );
    pLocalOptions->SetFormulaRegexEnabled( aBtnRegex.IsChecked() );
	pLocalOptions->SetLookUpColRowNames( aBtnLookUp.IsChecked() );
    ScGrammar::Grammar eGram = ScGrammar::GRAM_DEFAULT;
    switch (aLBFormulaSyntax.GetSelectEntryPos())
    {
        case 0:
            eGram = ScGrammar::GRAM_NATIVE;
        break;
#ifdef USE_JAVA
        // Fix bug 3587 by disabling Excel syntax
        default:
            eGram = ScGrammar::GRAM_NATIVE;
        break;
#else	// USE_JAVA
        case 1:
            eGram = ScGrammar::GRAM_NATIVE_XL_A1;
        break;
        case 2:
            eGram = ScGrammar::GRAM_NATIVE_XL_R1C1;
        break;
#endif	// USE_JAVA
    }
    pLocalOptions->SetFormulaSyntax(eGram);

	if ( *pLocalOptions != *pOldOptions )
	{
		rCoreAttrs.Put( ScTpCalcItem( nWhichCalc, *pLocalOptions ) );
		return TRUE;
	}
	else
		return FALSE;
}

//------------------------------------------------------------------------

int __EXPORT ScTpCalcOptions::DeactivatePage( SfxItemSet* pSetP )
{
    int nReturn = KEEP_PAGE;

    double fEps;
    if( aEdEps.GetValue( fEps ) && (fEps > 0.0) )
    {
        pLocalOptions->SetIterEps( fEps );
        nReturn = LEAVE_PAGE;
    }

	if ( nReturn == KEEP_PAGE )
	{
		ErrorBox( this,
				  WinBits( WB_OK | WB_DEF_OK ),
				  ScGlobal::GetRscString( STR_INVALID_EPS )
				).Execute();

		aEdEps.GrabFocus();
	}
    else if ( pSetP )
        FillItemSet( *pSetP );

	return nReturn;
}

//-----------------------------------------------------------------------
// Handler:

IMPL_LINK( ScTpCalcOptions, RadioClickHdl, RadioButton*, pBtn )
{
	if ( pBtn == &aBtnDateStd )
	{
		pLocalOptions->SetDate( 30, 12, 1899 );
	}
	else if ( pBtn == &aBtnDateSc10 )
	{
		pLocalOptions->SetDate( 1, 1, 1900 );
	}
	else if ( pBtn == &aBtnDate1904 )
	{
		pLocalOptions->SetDate( 1, 1, 1904 );
	}

	return 0;
}

//-----------------------------------------------------------------------

IMPL_LINK(  ScTpCalcOptions, CheckClickHdl, CheckBox*, pBtn )
{
	if ( pBtn->IsChecked() )
	{
		pLocalOptions->SetIter( TRUE );
		aFtSteps.Enable();	aEdSteps.Enable();
		aFtEps	.Enable();	aEdEps	.Enable();
	}
	else
	{
		pLocalOptions->SetIter( FALSE );
		aFtSteps.Disable(); aEdSteps.Disable();
		aFtEps	.Disable(); aEdEps	.Disable();
	}

	return 0;
}




