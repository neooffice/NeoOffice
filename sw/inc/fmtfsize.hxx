/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Portions of this file are part of the LibreOffice project.
 *
 *   This Source Code Form is subject to the terms of the Mozilla Public
 *   License, v. 2.0. If a copy of the MPL was not distributed with this
 *   file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ************************************************************************/
#ifndef _FMTFSIZE_HXX
#define _FMTFSIZE_HXX

#include <tools/gen.hxx>
#include <svtools/poolitem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <swtypes.hxx>
#include <format.hxx>

class IntlWrapper;

//Die Framesize ---------------------------------

enum SwFrmSize
{
	ATT_VAR_SIZE,		//Frm ist in der Var-Richtung variabel
	ATT_FIX_SIZE,		//Frm ist in der Var-Richtung unbeweglich
	ATT_MIN_SIZE		//Der Wert in der Var-Richtung beschreibt eine
						//Minimalgroesse, die nicht unter- wohl aber
						//ueberschritten werden kann.
};

class SW_DLLPUBLIC SwFmtFrmSize: public SfxPoolItem
{
	Size	  aSize;
    SwFrmSize eFrmHeightType;
    SwFrmSize eFrmWidthType;
	BYTE 	  nWidthPercent;	//Fuer Tabellen kann die Breite in Prozent
#if SUPD == 310
    sal_Int16 m_eWidthPercentRelation;
#endif	// SUPD == 310
	BYTE	  nHeightPercent;	//angegeben sein.
#if SUPD == 310
    sal_Int16 m_eHeightPercentRelation;
#endif	// SUPD == 310
								//Fuer Rahmen koennen Hoehe und/oder Breite
								//in Prozent angegeben sein. Wenn nur eine
								//der Angaben in Prozent angeben ist, kann
								//durch den ausgezeichneten Wert 0xFF in der
								//anderen Prozentangabe bestimmt werden, das
								//sich diese Richtung proportional zur anderen
								//verhaelt. Basis fuer die Umrechnung sind fuer
								//diesen Fall die Angaben in der Size.
								//Die Prozentwerte beziehen sich immer auf die
								//Umgebung in der das Objekt steht (PrtArea)
								//Auf die Bildschirmbreite abzueglich Raender
								//in der BrowseView wenn die Umgebung die Seite
								//ist.
public:
	SwFmtFrmSize( SwFrmSize eSize = ATT_VAR_SIZE,
				  SwTwips nWidth = 0, SwTwips nHeight = 0 );
	SwFmtFrmSize& operator=( const SwFmtFrmSize& rCpy );

	// "pure virtual Methoden" vom SfxPoolItem
	virtual int             operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*	Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
									SfxMapUnit eCoreMetric,
									SfxMapUnit ePresMetric,
									String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;
	virtual	BOOL        	 QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
	virtual	BOOL			 PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    SwFrmSize GetHeightSizeType() const { return eFrmHeightType; }
    void SetHeightSizeType( SwFrmSize eSize ) { eFrmHeightType = eSize; }

    SwFrmSize GetWidthSizeType() const { return eFrmWidthType; }
    void SetWidthSizeType( SwFrmSize eSize ) { eFrmWidthType = eSize; }

    const Size& GetSize() const { return aSize; }
		  void  SetSize( const Size &rNew ) { aSize = rNew; }

	SwTwips GetHeight() const { return aSize.Height(); }
	SwTwips GetWidth()  const { return aSize.Width();  }
	void	SetHeight( const SwTwips nNew ) { aSize.Height() = nNew; }
	void	SetWidth ( const SwTwips nNew ) { aSize.Width()  = nNew; }

	BYTE    GetHeightPercent() const{ return nHeightPercent; }
#if SUPD == 310
    sal_Int16   GetHeightPercentRelation() const { return m_eHeightPercentRelation;  }
#endif	// SUPD == 310
	BYTE	GetWidthPercent() const { return nWidthPercent;  }
#if SUPD == 310
    sal_Int16   GetWidthPercentRelation() const { return m_eWidthPercentRelation;  }
#endif	// SUPD == 310
	void	SetHeightPercent( BYTE n ) { nHeightPercent = n; }
#if SUPD == 310
    void    SetHeightPercentRelation ( sal_Int16 n ) { m_eHeightPercentRelation  = n; }
#endif	// SUPD == 310
	void	SetWidthPercent ( BYTE n ) { nWidthPercent  = n; }
#if SUPD == 310
    void    SetWidthPercentRelation ( sal_Int16 n ) { m_eWidthPercentRelation  = n; }
#endif	// SUPD == 310
};

inline const SwFmtFrmSize &SwAttrSet::GetFrmSize(BOOL bInP) const
	{ return (const SwFmtFrmSize&)Get( RES_FRM_SIZE,bInP); }

inline const SwFmtFrmSize &SwFmt::GetFrmSize(BOOL bInP) const
	{ return aSet.GetFrmSize(bInP); }

#endif

