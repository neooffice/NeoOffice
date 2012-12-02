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
 * Modified December 2012 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include "inscodlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


#ifdef USE_JAVA

#include <optutil.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include "miscuno.hxx"

// Defines - Paste Special Preserve Options
#define ALL_CHECK          "AllCheck"
#define CHECKS             "Checks"
#define FORMULA_CHECKS     "FormulaChecks"
#define CHECKS_2           "Checks2"
#define MOVE_MODE          "MoveMode"
#define SEP_PATH           "Office.CalcPasteSpecial/PasteSpecial"

using namespace com::sun::star::uno;

#endif	// USE_JAVA

//==================================================================

BOOL   ScInsertContentsDlg::bPreviousAllCheck = TRUE;
USHORT ScInsertContentsDlg::nPreviousChecks   = (IDF_DATETIME | IDF_STRING  |
												 IDF_NOTE     | IDF_FORMULA |
												 IDF_ATTRIB   | IDF_OBJECTS);
USHORT ScInsertContentsDlg::nPreviousFormulaChecks = PASTE_NOFUNC;
USHORT ScInsertContentsDlg::nPreviousChecks2 = 0;
USHORT ScInsertContentsDlg::nPreviousMoveMode = INS_NONE;	// enum InsCellCmd
#ifdef USE_JAVA
BOOL   ScInsertContentsDlg::bPreviousValuesInitialized = FALSE;
#endif	// USE_JAVA

//-----------------------------------------------------------------------

ScInsertContentsDlg::ScInsertContentsDlg( Window*		pParent,
										  USHORT		nCheckDefaults,
										  const String* pStrTitle )

 :  ModalDialog     ( pParent, ScResId( RID_SCDLG_INSCONT ) ),
	//
    aFlFrame        ( this, ScResId( FL_FRAME ) ),
	aBtnInsAll      ( this, ScResId( BTN_INSALL ) ),
	aBtnInsStrings  ( this, ScResId( BTN_INSSTRINGS ) ),
	aBtnInsNumbers  ( this, ScResId( BTN_INSNUMBERS ) ),
	aBtnInsDateTime ( this, ScResId( BTN_INSDATETIME ) ),
	aBtnInsFormulas ( this, ScResId( BTN_INSFORMULAS ) ),
	aBtnInsNotes    ( this, ScResId( BTN_INSNOTES ) ),
	aBtnInsAttrs    ( this, ScResId( BTN_INSATTRS ) ),
	aBtnInsObjects  ( this, ScResId( BTN_INSOBJECTS ) ),
    aFlSep1         ( this, ScResId( FL_SEP1 ) ),
    aFlOptions      ( this, ScResId( FL_OPTIONS ) ),
	aBtnSkipEmptyCells( this, ScResId(BTN_SKIP_EMPTY ) ),
	aBtnTranspose	( this, ScResId( BTN_TRANSPOSE ) ),
	aBtnLink        ( this, ScResId( BTN_LINK ) ),
    aFlOperation    ( this, ScResId( FL_OPERATION ) ),
	aRbNoOp         ( this, ScResId( BTN_OP_NOOP ) ),
	aRbAdd          ( this, ScResId( BTN_OP_ADD	) ),
	aRbSub          ( this, ScResId( BTN_OP_SUB  ) ),
	aRbMul          ( this, ScResId( BTN_OP_MUL  ) ),
	aRbDiv          ( this, ScResId( BTN_OP_DIV  ) ),
    aFlSep2         ( this, ScResId( FL_SEP2 ) ),
    aFlMove         ( this, ScResId( FL_MOVE ) ),
	aRbMoveNone     ( this, ScResId( BTN_MV_NONE ) ),
	aRbMoveDown     ( this, ScResId( BTN_MV_DOWN ) ),
	aRbMoveRight    ( this, ScResId( BTN_MV_RIGHT ) ),
	aBtnOk          ( this, ScResId( BTN_OK ) ),
	aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
	aBtnHelp        ( this, ScResId( BTN_HELP ) ),
	bOtherDoc		( FALSE ),
	bFillMode		( FALSE ),
	bChangeTrack	( FALSE ),
	bMoveDownDisabled( FALSE ),
	bMoveRightDisabled( FALSE )
{
#ifdef USE_JAVA
	if ( !bPreviousValuesInitialized )
	{
		bPreviousValuesInitialized = TRUE;

		Sequence< Any > aValues;
		const Any *pProperties;
		Sequence< ::rtl::OUString > aNames( 5 );
		::rtl::OUString *pNames = aNames.getArray();
		ScLinkConfigItem aItem( ::rtl::OUString::createFromAscii( SEP_PATH ) );

		pNames[0] = ::rtl::OUString::createFromAscii( ALL_CHECK );
		pNames[1] = ::rtl::OUString::createFromAscii( CHECKS );
		pNames[2] = ::rtl::OUString::createFromAscii( FORMULA_CHECKS);
		pNames[3] = ::rtl::OUString::createFromAscii( CHECKS_2 );
		pNames[4] = ::rtl::OUString::createFromAscii( MOVE_MODE );
		aValues = aItem.GetProperties( aNames );
		pProperties = aValues.getConstArray();
		if( pProperties[0].hasValue() )
		{
			sal_Bool bValue = true;
			pProperties[0] >>= bValue;
			ScInsertContentsDlg::bPreviousAllCheck = (BOOL)bValue;
		}
		if ( pProperties[1].hasValue() )
		{
			sal_Int32 nValue = -1;
			pProperties[1] >>= nValue;
			if ( nValue >= 0 )
				ScInsertContentsDlg::nPreviousChecks = (USHORT)nValue;
		}
		if ( pProperties[2].hasValue() )
		{
			sal_Int32 nValue = -1;
			pProperties[2] >>= nValue;
			if ( nValue >= 0 )
				ScInsertContentsDlg::nPreviousFormulaChecks = (USHORT)nValue;
		}
		if ( pProperties[3].hasValue() )
		{
			sal_Int32 nValue = -1;
			pProperties[3] >>= nValue;
			if ( nValue >= 0 )
				ScInsertContentsDlg::nPreviousChecks2 = (USHORT)nValue;
		}
		if ( pProperties[4].hasValue() )
		{
			sal_Int32 nValue = -1;
			pProperties[4] >>= nValue;
			if ( nValue >= 0 )
				ScInsertContentsDlg::nPreviousMoveMode = (USHORT)nValue;
		}
	}
#endif	// USE_JAVA

	if ( pStrTitle )
		SetText( *pStrTitle );

	if ( nCheckDefaults != 0 )
	{
		ScInsertContentsDlg::nPreviousChecks = nCheckDefaults;
		ScInsertContentsDlg::bPreviousAllCheck = FALSE;
		ScInsertContentsDlg::nPreviousChecks2 = 0;
	}

	aBtnInsAll.Check     ( ScInsertContentsDlg::bPreviousAllCheck );
	aBtnInsStrings.Check ( IS_SET( IDF_STRING,
								   ScInsertContentsDlg::nPreviousChecks ) );
	aBtnInsNumbers.Check ( IS_SET( IDF_VALUE,
								   ScInsertContentsDlg::nPreviousChecks ) );
	aBtnInsDateTime.Check( IS_SET( IDF_DATETIME,
								   ScInsertContentsDlg::nPreviousChecks ) );
	aBtnInsFormulas.Check( IS_SET( IDF_FORMULA,
								   ScInsertContentsDlg::nPreviousChecks ) );
	aBtnInsNotes.Check   ( IS_SET( IDF_NOTE,
								   ScInsertContentsDlg::nPreviousChecks ) );
	aBtnInsAttrs.Check   ( IS_SET( IDF_ATTRIB,
								   ScInsertContentsDlg::nPreviousChecks ) );
	aBtnInsObjects.Check ( IS_SET( IDF_OBJECTS,
								   ScInsertContentsDlg::nPreviousChecks ) );

	switch( ScInsertContentsDlg::nPreviousFormulaChecks )
	{
		case PASTE_NOFUNC: aRbNoOp.Check(TRUE); break;
		case PASTE_ADD:    aRbAdd.Check(TRUE); break;
		case PASTE_SUB:    aRbSub.Check(TRUE); break;
		case PASTE_MUL:    aRbMul.Check(TRUE); break;
		case PASTE_DIV:    aRbDiv.Check(TRUE); break;
	}

	switch( ScInsertContentsDlg::nPreviousMoveMode )
	{
		case INS_NONE:  	 aRbMoveNone.Check(TRUE); break;
		case INS_CELLSDOWN:	 aRbMoveDown.Check(TRUE); break;
		case INS_CELLSRIGHT: aRbMoveRight.Check(TRUE); break;
	}

	aBtnSkipEmptyCells.Check( ( ScInsertContentsDlg::nPreviousChecks2 & INS_CONT_NOEMPTY ) != 0);
	aBtnTranspose.Check( ( ScInsertContentsDlg::nPreviousChecks2 	& INS_CONT_TRANS ) != 0);
	aBtnLink.Check( ( ScInsertContentsDlg::nPreviousChecks2 			& INS_CONT_LINK  ) != 0);

	DisableChecks( aBtnInsAll.IsChecked() );

    aFlSep1.SetStyle( aFlSep1.GetStyle() | WB_VERT );
    aFlSep2.SetStyle( aFlSep2.GetStyle() | WB_VERT );

	aBtnInsAll.SetClickHdl( LINK( this, ScInsertContentsDlg, InsAllHdl ) );
	aBtnLink.SetClickHdl( LINK( this, ScInsertContentsDlg, LinkBtnHdl ) );

	//-------------
	FreeResource();
}

//------------------------------------------------------------------------

USHORT ScInsertContentsDlg::GetInsContentsCmdBits() const
{
	ScInsertContentsDlg::nPreviousChecks = 0;

	if ( aBtnInsStrings.IsChecked() )
		ScInsertContentsDlg::nPreviousChecks = IDF_STRING;
	if ( aBtnInsNumbers.IsChecked() )
		ScInsertContentsDlg::nPreviousChecks |= IDF_VALUE;
	if ( aBtnInsDateTime.IsChecked())
		ScInsertContentsDlg::nPreviousChecks |= IDF_DATETIME;
	if ( aBtnInsFormulas.IsChecked())
		ScInsertContentsDlg::nPreviousChecks |= IDF_FORMULA;
	if ( aBtnInsNotes.IsChecked()   )
		ScInsertContentsDlg::nPreviousChecks |= IDF_NOTE;
	if ( aBtnInsAttrs.IsChecked()   )
		ScInsertContentsDlg::nPreviousChecks |= IDF_ATTRIB;
	if ( aBtnInsObjects.IsChecked() )
		ScInsertContentsDlg::nPreviousChecks |= IDF_OBJECTS;

	ScInsertContentsDlg::bPreviousAllCheck = aBtnInsAll.IsChecked();

	return ( (ScInsertContentsDlg::bPreviousAllCheck)
				? IDF_ALL
				: ScInsertContentsDlg::nPreviousChecks );
}

//------------------------------------------------------------------------

InsCellCmd ScInsertContentsDlg::GetMoveMode()
{
	if ( aRbMoveDown.IsChecked() )
		return INS_CELLSDOWN;
	if ( aRbMoveRight.IsChecked() )
		return INS_CELLSRIGHT;

	return INS_NONE;
}

//------------------------------------------------------------------------

void ScInsertContentsDlg::DisableChecks( BOOL bInsAllChecked )
{
	if ( bInsAllChecked )
	{
		aBtnInsStrings.Disable();
		aBtnInsNumbers.Disable();
		aBtnInsDateTime.Disable();
		aBtnInsFormulas.Disable();
		aBtnInsNotes.Disable();
		aBtnInsAttrs.Disable();
		aBtnInsObjects.Disable();
	}
	else
	{
		aBtnInsStrings.Enable();
		aBtnInsNumbers.Enable();
		aBtnInsDateTime.Enable();
		aBtnInsFormulas.Enable();
		aBtnInsNotes.Enable();
		aBtnInsAttrs.Enable();

		//	"Objects" is disabled for "Fill Tables"
		if ( bFillMode )
			aBtnInsObjects.Disable();
		else
			aBtnInsObjects.Enable();
	}
}

// Link in anderes Dokument -> alles andere disabled

void ScInsertContentsDlg::TestModes()
{
	if ( bOtherDoc && aBtnLink.IsChecked() )
	{
		aBtnSkipEmptyCells.Disable();
		aBtnTranspose.Disable();
		aRbNoOp.Disable();
		aRbAdd.Disable();
		aRbSub.Disable();
		aRbMul.Disable();
		aRbDiv.Disable();
        aFlOperation.Disable();

		aRbMoveNone.Disable();
		aRbMoveDown.Disable();
		aRbMoveRight.Disable();
        aFlMove.Disable();

        aFlFrame.Disable();
		aBtnInsAll.Disable();
		DisableChecks(TRUE);
	}
	else
	{
		aBtnSkipEmptyCells.Enable();
		aBtnTranspose.Enable(!bFillMode);
		aRbNoOp.Enable();
		aRbAdd.Enable();
		aRbSub.Enable();
		aRbMul.Enable();
		aRbDiv.Enable();
        aFlOperation.Enable();

		aRbMoveNone.Enable(!bFillMode && !bChangeTrack && !(bMoveDownDisabled && bMoveRightDisabled));
		aRbMoveDown.Enable(!bFillMode && !bChangeTrack && !bMoveDownDisabled);
		aRbMoveRight.Enable(!bFillMode && !bChangeTrack && !bMoveRightDisabled);
        aFlMove.Enable(!bFillMode && !bChangeTrack && !(bMoveDownDisabled && bMoveRightDisabled));

        aFlFrame.Enable();
		aBtnInsAll.Enable();
		DisableChecks( aBtnInsAll.IsChecked() );
	}
}

void ScInsertContentsDlg::SetOtherDoc( BOOL bSet )
{
	if ( bSet != bOtherDoc )
	{
		bOtherDoc = bSet;
		TestModes();
		if ( bSet )
			aRbMoveNone.Check(TRUE);
	}
}

void ScInsertContentsDlg::SetFillMode( BOOL bSet )
{
	if ( bSet != bFillMode )
	{
		bFillMode = bSet;
		TestModes();
		if ( bSet )
			aRbMoveNone.Check(TRUE);
	}
}

void ScInsertContentsDlg::SetChangeTrack( BOOL bSet )
{
	if ( bSet != bChangeTrack )
	{
		bChangeTrack = bSet;
		TestModes();
		if ( bSet )
			aRbMoveNone.Check(TRUE);
	}
}

void ScInsertContentsDlg::SetCellShiftDisabled( int nDisable )
{
	BOOL bDown = ((nDisable & SC_CELL_SHIFT_DISABLE_DOWN) != 0);
	BOOL bRight = ((nDisable & SC_CELL_SHIFT_DISABLE_RIGHT) != 0);
	if ( bDown != bMoveDownDisabled || bRight != bMoveRightDisabled )
	{
		bMoveDownDisabled = bDown;
		bMoveRightDisabled = bRight;
		TestModes();
		if ( bMoveDownDisabled && aRbMoveDown.IsChecked() )
			aRbMoveNone.Check(TRUE);
		if ( bMoveRightDisabled && aRbMoveRight.IsChecked() )
			aRbMoveNone.Check(TRUE);
	}
}


//------------------------------------------------------------------------

IMPL_LINK( ScInsertContentsDlg, InsAllHdl, void*, EMPTYARG )
{
	DisableChecks( aBtnInsAll.IsChecked() );

	return 0;
}

IMPL_LINK( ScInsertContentsDlg, LinkBtnHdl, void*, EMPTYARG )
{
	TestModes();

	return 0;
}

__EXPORT ScInsertContentsDlg::~ScInsertContentsDlg()
{
	ScInsertContentsDlg::nPreviousChecks2 = 0;
	if(aBtnSkipEmptyCells.IsChecked())
		ScInsertContentsDlg::nPreviousChecks2 |= INS_CONT_NOEMPTY;
	if(	aBtnTranspose.IsChecked())
		ScInsertContentsDlg::nPreviousChecks2 |= INS_CONT_TRANS;
	if( aBtnLink.IsChecked() )
		ScInsertContentsDlg::nPreviousChecks2 |= INS_CONT_LINK;

	if (!bFillMode)		// im FillMode ist None gecheckt und alle 3 disabled
	{
		if ( aRbMoveNone.IsChecked() )
			ScInsertContentsDlg::nPreviousMoveMode = INS_NONE;
		else if ( aRbMoveDown.IsChecked() )
			ScInsertContentsDlg::nPreviousMoveMode = INS_CELLSDOWN;
		else if ( aRbMoveRight.IsChecked() )
			ScInsertContentsDlg::nPreviousMoveMode = INS_CELLSRIGHT;
	}

#ifdef USE_JAVA
	Sequence< Any > aValues;
	Any *pProperties;
	Sequence< ::rtl::OUString > aNames( 5 );
	::rtl::OUString *pNames = aNames.getArray();
	ScLinkConfigItem aItem( ::rtl::OUString::createFromAscii( SEP_PATH ) );

	pNames[0] = ::rtl::OUString::createFromAscii( ALL_CHECK );
	pNames[1] = ::rtl::OUString::createFromAscii( CHECKS );
	pNames[2] = ::rtl::OUString::createFromAscii( FORMULA_CHECKS);
	pNames[3] = ::rtl::OUString::createFromAscii( CHECKS_2 );
	pNames[4] = ::rtl::OUString::createFromAscii( MOVE_MODE );
	aValues = aItem.GetProperties( aNames );
    pProperties = aValues.getArray();
	pProperties[0] <<= static_cast< sal_Bool >( ScInsertContentsDlg::bPreviousAllCheck );
	pProperties[1] <<= static_cast< sal_Int32 >( ScInsertContentsDlg::nPreviousChecks );
	pProperties[2] <<= static_cast< sal_Int32 >( ScInsertContentsDlg::nPreviousFormulaChecks );
	pProperties[3] <<= static_cast< sal_Int32 >( ScInsertContentsDlg::nPreviousChecks2 );
	pProperties[4] <<= static_cast< sal_Int32 >( ScInsertContentsDlg::nPreviousMoveMode );

	aItem.PutProperties( aNames, aValues );
#endif	// USE_JAVA
}

USHORT	ScInsertContentsDlg::GetFormulaCmdBits() const
{
	ScInsertContentsDlg::nPreviousFormulaChecks = PASTE_NOFUNC;
	if(aRbAdd.IsChecked())
		ScInsertContentsDlg::nPreviousFormulaChecks = PASTE_ADD;
	else if(aRbSub.IsChecked())
		ScInsertContentsDlg::nPreviousFormulaChecks = PASTE_SUB;
	else if(aRbMul.IsChecked())
		ScInsertContentsDlg::nPreviousFormulaChecks = PASTE_MUL;
	else if(aRbDiv.IsChecked())
		ScInsertContentsDlg::nPreviousFormulaChecks = PASTE_DIV;
	// Bits fuer Checkboxen ausblenden
	return ScInsertContentsDlg::nPreviousFormulaChecks;
}



