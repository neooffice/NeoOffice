/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU General Public License Version 2.1.
 *
 *
 *    GNU General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *    Modified July 2006 by Edward Peterlin. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <settings.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <scrbar.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <help.hxx>
#endif
#ifndef _SV_LSTBOX_H
#include <lstbox.h>
#endif
#ifndef _SV_ILSTBOX_HXX
#include <ilstbox.hxx>
#endif
#ifndef _VCL_I18NHELP_HXX
#include <i18nhelp.hxx>
#endif
#ifndef _VCL_CONTROLLAYOUT_HXX
#include <controllayout.hxx>
#endif
#ifndef _VCL_UNOHELP_HXX
#include <unohelp.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCOLLATOR_HPP_
#include <com/sun/star/i18n/XCollator.hpp>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HDL_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif

#ifdef USE_JAVA

#ifndef _SV_LSTBOX_HXX
#include <lstbox.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <combobox.hxx>
#endif

#endif	// USE_JAVA


using namespace ::com::sun::star;

// =======================================================================

void ImplInitFieldSettings( Window* pWin, BOOL bFont, BOOL bForeground, BOOL bBackground )
{
	const StyleSettings& rStyleSettings = pWin->GetSettings().GetStyleSettings();

	if ( bFont )
	{
		Font aFont = rStyleSettings.GetFieldFont();
		if ( pWin->IsControlFont() )
			aFont.Merge( pWin->GetControlFont() );
		pWin->SetZoomedPointFont( aFont );
	}

	if ( bFont || bForeground )
	{
		Color aTextColor = rStyleSettings.GetFieldTextColor();
		if ( pWin->IsControlForeground() )
			aTextColor = pWin->GetControlForeground();
		pWin->SetTextColor( aTextColor );
	}

	if ( bBackground )
	{
		if( pWin->IsControlBackground() )
			pWin->SetBackground( pWin->GetControlBackground() );
		else
			pWin->SetBackground( rStyleSettings.GetFieldColor() );
	}
}

// -----------------------------------------------------------------------

void ImplInitDropDownButton( PushButton* pButton )
{
	if ( pButton->GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_SPINUPDOWN )
		pButton->SetSymbol( SYMBOL_SPIN_UPDOWN );
	else
		pButton->SetSymbol( SYMBOL_SPIN_DOWN );

	if ( pButton->IsNativeControlSupported(CTRL_LISTBOX, PART_ENTIRE_CONTROL)
			&& ! pButton->IsNativeControlSupported(CTRL_LISTBOX, PART_BUTTON_DOWN) )
		pButton->SetBackground();
}

// =======================================================================

ImplEntryList::ImplEntryList( Window* pWindow )
{
    mpWindow = pWindow;
	mnLastSelected = LISTBOX_ENTRY_NOTFOUND;
	mnSelectionAnchor = LISTBOX_ENTRY_NOTFOUND;
	mnImages = 0;
	mbCallSelectionChangedHdl = TRUE;

	mnMRUCount = 0;
	mnMaxMRUCount = 0;
}

// -----------------------------------------------------------------------

ImplEntryList::~ImplEntryList()
{
	Clear();
}

// -----------------------------------------------------------------------

void ImplEntryList::Clear()
{
	mnImages = 0;
	for ( USHORT n = GetEntryCount(); n; )
	{
		ImplEntryType* pImplEntry = GetEntry( --n );
		delete pImplEntry;
	}
	List::Clear();
}

// -----------------------------------------------------------------------

void ImplEntryList::SelectEntry( USHORT nPos, BOOL bSelect )
{
	ImplEntryType* pImplEntry = GetEntry( nPos );
	if ( pImplEntry &&
	   ( pImplEntry->mbIsSelected != bSelect ) &&
	   ( (pImplEntry->mnFlags & LISTBOX_ENTRY_FLAG_DISABLE_SELECTION) == 0  ) )
	{
		pImplEntry->mbIsSelected = bSelect;
		if ( mbCallSelectionChangedHdl )
			maSelectionChangedHdl.Call( (void*)nPos );
	}
}

// -----------------------------------------------------------------------

uno::Reference< i18n::XCollator > ImplGetCollator (lang::Locale &rLocale)
{
	static uno::Reference< i18n::XCollator > xCollator;
	if ( !xCollator.is() )
		xCollator = vcl::unohelper::CreateCollator();
	if( xCollator.is() )
		xCollator->loadDefaultCollator (rLocale, 0);

	return xCollator;
}

USHORT ImplEntryList::InsertEntry( USHORT nPos, ImplEntryType* pNewEntry, BOOL bSort )
{
	if ( !!pNewEntry->maImage )
		mnImages++;

	if ( !bSort || !Count() )
	{
		Insert( pNewEntry, nPos );
	}
	else
	{
		lang::Locale aLocale = Application::GetSettings().GetLocale();
		uno::Reference< i18n::XCollator > xCollator = ImplGetCollator(aLocale);

		const XubString& rStr = pNewEntry->maStr;
		ULONG nLow, nHigh, nMid;

		nHigh = Count();

		ImplEntryType* pTemp = GetEntry( (USHORT)(nHigh-1) );

		try
		{
			// XXX even though XCollator::compareString returns a sal_Int32 the only
			// defined values are {-1, 0, 1} which is compatible with StringCompare
			StringCompare eComp = xCollator.is() ?
				(StringCompare)xCollator->compareString (rStr, pTemp->maStr)
				: COMPARE_EQUAL;

			// Schnelles Einfuegen bei sortierten Daten
			if ( eComp != COMPARE_LESS )
			{
				Insert( pNewEntry, LIST_APPEND );
			}
			else
			{
				nLow  = mnMRUCount;
				pTemp = (ImplEntryType*)GetEntry( (USHORT)nLow );

				eComp = (StringCompare)xCollator->compareString (rStr, pTemp->maStr);
				if ( eComp != COMPARE_GREATER )
				{
					Insert( pNewEntry, (ULONG)0 );
				}
				else
				{
					// Binaeres Suchen
					nHigh--;
					do
					{
						nMid = (nLow + nHigh) / 2;
						pTemp = (ImplEntryType*)GetObject( nMid );

						eComp = (StringCompare)xCollator->compareString (rStr, pTemp->maStr);

						if ( eComp == COMPARE_LESS )
							nHigh = nMid-1;
						else
						{
							if ( eComp == COMPARE_GREATER )
								nLow = nMid + 1;
							else
								break;
						}
					}
					while ( nLow <= nHigh );

					if ( eComp != COMPARE_LESS )
						nMid++;

					Insert( pNewEntry, nMid );
				}
			}
		}
		catch (uno::RuntimeException& )
		{
			// XXX this is arguable, if the exception occured because pNewEntry is
			// garbage you wouldn't insert it. If the exception occured because the
			// Collator implementation is garbage then give the user a chance to see
			// his stuff
			Insert( pNewEntry, (ULONG)0 );
		}

	}

	return (USHORT)GetPos( pNewEntry );
}

// -----------------------------------------------------------------------

void ImplEntryList::RemoveEntry( USHORT nPos )
{
	ImplEntryType* pImplEntry = (ImplEntryType*)List::Remove( nPos );
	if ( pImplEntry )
	{
		if ( !!pImplEntry->maImage )
			mnImages--;

		delete pImplEntry;
	}
}

// -----------------------------------------------------------------------

USHORT ImplEntryList::FindEntry( const XubString& rString, BOOL bSearchMRUArea ) const
{
    USHORT nEntries = GetEntryCount();
    for ( USHORT n = bSearchMRUArea ? 0 : GetMRUCount(); n < nEntries; n++ )
	{
		ImplEntryType* pImplEntry = GetEntry( n );
        if ( pImplEntry->maStr == rString )
            return n;
    }
    return LISTBOX_ENTRY_NOTFOUND;
}

    // -----------------------------------------------------------------------

USHORT ImplEntryList::FindMatchingEntry( const XubString& rStr, USHORT nStart, BOOL bForward, BOOL bLazy ) const
{
	USHORT	nPos = LISTBOX_ENTRY_NOTFOUND;
	USHORT	nEntryCount = GetEntryCount();
	if ( !bForward )
		nStart++;	// wird sofort dekrementiert

    const vcl::I18nHelper& rI18nHelper = mpWindow->GetSettings().GetLocaleI18nHelper();
    for ( USHORT n = nStart; bForward ? ( n < nEntryCount ) : n; )
	{
		if ( !bForward )
			n--;

		ImplEntryType* pImplEntry = GetEntry( n );
        BOOL bMatch = bLazy ? rI18nHelper.MatchString( rStr, pImplEntry->maStr ) : ( rStr.Match( pImplEntry->maStr ) == STRING_MATCH );
		if ( bMatch )
		{
			nPos = n;
			break;
		}

		if ( bForward )
			n++;
	}

    return nPos;
}

// -----------------------------------------------------------------------

USHORT ImplEntryList::FindEntry( const void* pData ) const
{
	USHORT nPos = LISTBOX_ENTRY_NOTFOUND;
	for ( USHORT n = GetEntryCount(); n; )
	{
		ImplEntryType* pImplEntry = GetEntry( --n );
		if ( pImplEntry->mpUserData == pData )
		{
			nPos = n;
			break;
		}
	}
	return nPos;
}

// -----------------------------------------------------------------------

XubString ImplEntryList::GetEntryText( USHORT nPos ) const
{
	XubString aEntryText;
	ImplEntryType* pImplEntry = GetEntry( nPos );
	if ( pImplEntry )
		aEntryText = pImplEntry->maStr;
	return aEntryText;
}

// -----------------------------------------------------------------------

BOOL ImplEntryList::HasEntryImage( USHORT nPos ) const
{
	BOOL bImage = FALSE;
	ImplEntryType* pImplEntry = (ImplEntryType*)List::GetObject( nPos );
	if ( pImplEntry )
		bImage = !!pImplEntry->maImage;
	return bImage;
}

// -----------------------------------------------------------------------

Image ImplEntryList::GetEntryImage( USHORT nPos ) const
{
	Image aImage;
	ImplEntryType* pImplEntry = (ImplEntryType*)List::GetObject( nPos );
	if ( pImplEntry )
		aImage = pImplEntry->maImage;
	return aImage;
}

// -----------------------------------------------------------------------

void ImplEntryList::SetEntryData( USHORT nPos, void* pNewData )
{
	ImplEntryType* pImplEntry = (ImplEntryType*)List::GetObject( nPos );
	if ( pImplEntry )
		pImplEntry->mpUserData = pNewData;
}

// -----------------------------------------------------------------------

void* ImplEntryList::GetEntryData( USHORT nPos ) const
{
	ImplEntryType* pImplEntry = (ImplEntryType*)List::GetObject( nPos );
	return pImplEntry ? pImplEntry->mpUserData : NULL;
}

// -----------------------------------------------------------------------

void ImplEntryList::SetEntryFlags( USHORT nPos, long nFlags )
{
	ImplEntryType* pImplEntry = (ImplEntryType*)List::GetObject( nPos );
	if ( pImplEntry )
		pImplEntry->mnFlags = nFlags;
}

// -----------------------------------------------------------------------

long ImplEntryList::GetEntryFlags( USHORT nPos ) const
{
	ImplEntryType* pImplEntry = (ImplEntryType*)List::GetObject( nPos );
	return pImplEntry ? pImplEntry->mnFlags : 0;
}

// -----------------------------------------------------------------------

USHORT ImplEntryList::GetSelectEntryCount() const
{
	USHORT nSelCount = 0;
	for ( USHORT n = GetEntryCount(); n; )
	{
		ImplEntryType* pImplEntry = GetEntry( --n );
		if ( pImplEntry->mbIsSelected )
			nSelCount++;
	}
	return nSelCount;
}

// -----------------------------------------------------------------------

XubString ImplEntryList::GetSelectEntry( USHORT nIndex ) const
{
	return GetEntryText( GetSelectEntryPos( nIndex ) );
}

// -----------------------------------------------------------------------

USHORT ImplEntryList::GetSelectEntryPos( USHORT nIndex ) const
{
	USHORT nSelEntryPos = LISTBOX_ENTRY_NOTFOUND;
	USHORT nSel = 0;
	USHORT nEntryCount = GetEntryCount();

	for ( USHORT n = 0; n < nEntryCount; n++ )
	{
		ImplEntryType* pImplEntry = GetEntry( n );
		if ( pImplEntry->mbIsSelected )
		{
			if ( nSel == nIndex )
			{
				nSelEntryPos = n;
				break;
			}
			nSel++;
		}
	}

	return nSelEntryPos;
}

// -----------------------------------------------------------------------

BOOL ImplEntryList::IsEntrySelected( const XubString& rStr ) const
{
	return IsEntryPosSelected( FindEntry( rStr ) );
}

// -----------------------------------------------------------------------

BOOL ImplEntryList::IsEntryPosSelected( USHORT nIndex ) const
{
	ImplEntryType* pImplEntry = GetEntry( nIndex );
	return pImplEntry ? pImplEntry->mbIsSelected : FALSE;
}

// -----------------------------------------------------------------------

bool ImplEntryList::IsEntrySelectable( USHORT nPos ) const
{
	ImplEntryType* pImplEntry = GetEntry( nPos );
	return pImplEntry ? ((pImplEntry->mnFlags & LISTBOX_ENTRY_FLAG_DISABLE_SELECTION) == 0) : true;
}

// -----------------------------------------------------------------------

USHORT ImplEntryList::FindFirstSelectable( USHORT nPos, bool bForward /* = true */ )
{
	if( IsEntrySelectable( nPos ) )
		return nPos;

	if( bForward )
	{
		for( nPos = nPos + 1; nPos < GetEntryCount(); nPos++ )
		{
			if( IsEntrySelectable( nPos ) )
				return nPos;
		}
	}
	else
	{
		while( nPos )
		{
			nPos--;
			if( IsEntrySelectable( nPos ) )
				return nPos;
		}
	}

	return LISTBOX_ENTRY_NOTFOUND;
}

// =======================================================================

ImplListBoxWindow::ImplListBoxWindow( Window* pParent, WinBits nWinStyle ) :
	Control( pParent, 0 )
{
	mpEntryList 		= new ImplEntryList( this );

	mnTop				= 0;
	mnLeft				= 0;
	mnBorder			= 1;
	mnSelectModifier	= 0;
	mnUserDrawEntry 	= LISTBOX_ENTRY_NOTFOUND;
	mbTrack 			= FALSE;
	mbImgsDiffSz		= FALSE;
	mbTravelSelect		= FALSE;
	mbTrackingSelect	= FALSE;
	mbSelectionChanged	= FALSE;
	mbMouseMoveSelect	= FALSE;
	mbMulti 			= FALSE;
	mbStackMode 		= FALSE;
	mbGrabFocus 		= FALSE;
	mbUserDrawEnabled	= FALSE;
	mbInUserDraw		= FALSE;
	mbReadOnly			= FALSE;
    mbHasFocusRect      = FALSE;
    mbRight             = ( nWinStyle & WB_RIGHT )      ? TRUE : FALSE;
    mbCenter            = ( nWinStyle & WB_CENTER )     ? TRUE : FALSE;
	mbSimpleMode		= ( nWinStyle & WB_SIMPLEMODE ) ? TRUE : FALSE;
	mbSort				= ( nWinStyle & WB_SORT )       ? TRUE : FALSE;

	// pb: #106948# explicit mirroring for calc
	mbMirroring			= FALSE;

	mnCurrentPos			= LISTBOX_ENTRY_NOTFOUND;
	mnTrackingSaveSelection = LISTBOX_ENTRY_NOTFOUND;
	mnSeparatorPos			= LISTBOX_ENTRY_NOTFOUND;

	SetLineColor();
	SetTextFillColor();
	SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFieldColor() ) );

	maSearchTimeout.SetTimeout( 2500 );
	maSearchTimeout.SetTimeoutHdl( LINK( this, ImplListBoxWindow, SearchStringTimeout ) );

	ImplInitSettings( TRUE, TRUE, TRUE );
	ImplCalcMetrics();
}

// -----------------------------------------------------------------------

ImplListBoxWindow::~ImplListBoxWindow()
{
	maSearchTimeout.Stop();
	delete mpEntryList;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground )
{
	ImplInitFieldSettings( this, bFont, bForeground, bBackground );
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::ImplCalcMetrics()
{
	mnMaxWidth		= 0;
	mnMaxTxtWidth	= 0;
	mnMaxImgWidth	= 0;
	mnMaxImgTxtWidth= 0;
	mnMaxImgHeight	= 0;

	mnTextHeight = (USHORT)GetTextHeight();
	mnMaxTxtHeight = mnTextHeight + mnBorder;
	mnMaxHeight = mnMaxTxtHeight;

	if ( maUserItemSize.Height() > mnMaxHeight )
		mnMaxHeight = (USHORT) maUserItemSize.Height();
	if ( maUserItemSize.Width() > mnMaxWidth )
		mnMaxWidth= (USHORT) maUserItemSize.Width();

	for ( USHORT n = mpEntryList->GetEntryCount(); n; )
	{
		const ImplEntryType* pEntry = mpEntryList->GetEntryPtr( --n );
		ImplCalcEntryMetrics( *pEntry, TRUE );
	}

	Size aSz( GetOutputSizePixel().Width(), mnMaxHeight );
	maFocusRect.SetSize( aSz );
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplListBoxWindow, SearchStringTimeout, Timer*, EMPTYARG )
{
	maSearchStr.Erase();
	return 1;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::Clear()
{
	mpEntryList->Clear();

	mnMaxHeight 	= mnMaxTxtHeight;
	mnMaxWidth		= 0;
	mnMaxTxtWidth	= 0;
	mnMaxImgTxtWidth= 0;
	mnMaxImgWidth	= 0;
	mnMaxImgHeight	= 0;
	mnTop			= 0;
	mnLeft			= 0;
	mbImgsDiffSz	= FALSE;
    delete mpLayoutData, mpLayoutData = NULL;

    mnCurrentPos = LISTBOX_ENTRY_NOTFOUND;

	Invalidate();
}

void ImplListBoxWindow::SetUserItemSize( const Size& rSz )
{
    delete mpLayoutData, mpLayoutData = NULL;
	maUserItemSize = rSz;
	ImplCalcMetrics();
}

// -----------------------------------------------------------------------

struct ImplEntryMetrics
{
	BOOL	bText;
	BOOL	bImage;
	USHORT	nEntryWidth;
	USHORT	nEntryHeight;
	USHORT	nTextWidth;
	USHORT	nImgWidth;
	USHORT	nImgHeight;
};

// -----------------------------------------------------------------------

void ImplListBoxWindow::ImplCalcEntryMetrics( const ImplEntryType& rEntry, BOOL bUpdateMetrics )
{
	// bUpdateMetrics: Wenn Entry groesser als Max-Wert, dann uebernehmen

	// Diese Methode war mal auch fuer RemoveRentry gedacht, deshalb die
	// ImplEntryMetrics-Struktur, damit die Werte zurueckgegeben werden.
	// Jetzt werden die aktuellen Metriken aber doch immer angepasst, weil
	// bei RemoveEntry einmal komplett durchgegangen wird.

	ImplEntryMetrics aMetrics;
	aMetrics.bText = rEntry.maStr.Len() ? TRUE : FALSE;
	aMetrics.bImage = !!rEntry.maImage;
	aMetrics.nEntryWidth = 0;
	aMetrics.nEntryHeight = 0;
	aMetrics.nTextWidth = 0;
	aMetrics.nImgWidth = 0;
	aMetrics.nImgHeight = 0;

	if ( aMetrics.bText )
	{
		aMetrics.nTextWidth = (USHORT)GetTextWidth( rEntry.maStr );
		if( bUpdateMetrics && ( aMetrics.nTextWidth > mnMaxTxtWidth ) )
			mnMaxTxtWidth = aMetrics.nTextWidth;
		aMetrics.nEntryWidth = mnMaxTxtWidth;
	}
	if ( aMetrics.bImage )
	{
		Size aImgSz = rEntry.maImage.GetSizePixel();
		aMetrics.nImgWidth	= (USHORT) CalcZoom( aImgSz.Width() );
		aMetrics.nImgHeight = (USHORT) CalcZoom( aImgSz.Height() );

		if ( bUpdateMetrics )
		{
			if( mnMaxImgWidth && ( aMetrics.nImgWidth != mnMaxImgWidth ) )
				mbImgsDiffSz = TRUE;
			else if ( mnMaxImgHeight && ( aMetrics.nImgHeight != mnMaxImgHeight ) )
				mbImgsDiffSz = TRUE;

			if( aMetrics.nImgWidth > mnMaxImgWidth )
				mnMaxImgWidth = aMetrics.nImgWidth;
			if( aMetrics.nImgHeight > mnMaxImgHeight )
				mnMaxImgHeight = aMetrics.nImgHeight;

			mnMaxImgTxtWidth = Max( mnMaxImgTxtWidth, aMetrics.nTextWidth );
		}
	}
	if ( IsUserDrawEnabled() || aMetrics.bImage )
	{
		aMetrics.nEntryWidth = Max( aMetrics.nImgWidth, (USHORT)maUserItemSize.Width() );
		if ( aMetrics.bText )
			aMetrics.nEntryWidth += aMetrics.nTextWidth + IMG_TXT_DISTANCE;
		aMetrics.nEntryHeight = Max( mnMaxImgHeight, (USHORT)maUserItemSize.Height() ) + 2;
	}

	if ( bUpdateMetrics )
	{
		if ( aMetrics.nEntryWidth > mnMaxWidth )
			mnMaxWidth = aMetrics.nEntryWidth;
		if ( aMetrics.nEntryHeight > mnMaxHeight )
			mnMaxHeight = aMetrics.nEntryHeight;
	}
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::ImplCallSelect()
{
	if ( !IsTravelSelect() && GetEntryList()->GetMaxMRUCount() )
	{
		// Insert the selected entry as MRU, if not allready first MRU
		USHORT nSelected = GetEntryList()->GetSelectEntryPos( 0 );
		USHORT nMRUCount = GetEntryList()->GetMRUCount();
		String aSelected = GetEntryList()->GetEntryText( nSelected );
		USHORT nFirstMatchingEntryPos = GetEntryList()->FindEntry( aSelected, TRUE );
		if ( nFirstMatchingEntryPos || !nMRUCount )
		{
			BOOL bSelectNewEntry = FALSE;
			if ( nFirstMatchingEntryPos < nMRUCount )
			{
				RemoveEntry( nFirstMatchingEntryPos );
				nMRUCount--;
				if ( nFirstMatchingEntryPos == nSelected )
					bSelectNewEntry = TRUE;
			}
			else if ( nMRUCount == GetEntryList()->GetMaxMRUCount() )
			{
				RemoveEntry( nMRUCount - 1 );
				nMRUCount--;
			}

            delete mpLayoutData, mpLayoutData = NULL;

			ImplEntryType* pNewEntry = new ImplEntryType( aSelected );
			pNewEntry->mbIsSelected = bSelectNewEntry;
			GetEntryList()->InsertEntry( 0, pNewEntry, FALSE );
			GetEntryList()->SetMRUCount( ++nMRUCount );
			SetSeparatorPos( nMRUCount ? nMRUCount-1 : 0 );
			maMRUChangedHdl.Call( NULL );
		}
	}

	maSelectHdl.Call( NULL );
	mbSelectionChanged = FALSE;
}

// -----------------------------------------------------------------------

USHORT ImplListBoxWindow::InsertEntry( USHORT nPos, ImplEntryType* pNewEntry )
{
    delete mpLayoutData, mpLayoutData = NULL;
	USHORT nNewPos = mpEntryList->InsertEntry( nPos, pNewEntry, mbSort );

	ImplCalcEntryMetrics( *pNewEntry, TRUE );
	return nNewPos;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::RemoveEntry( USHORT nPos )
{
    delete mpLayoutData, mpLayoutData = NULL;
	mpEntryList->RemoveEntry( nPos );
	ImplCalcMetrics();
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::ImplShowFocusRect()
{
    if ( mbHasFocusRect )
        HideFocus();
    ShowFocus( maFocusRect );
    mbHasFocusRect = TRUE;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::ImplHideFocusRect()
{
    if ( mbHasFocusRect )
    {
        HideFocus();
        mbHasFocusRect = FALSE;
    }
}


// -----------------------------------------------------------------------

USHORT ImplListBoxWindow::GetEntryPosForPoint( const Point& rPoint ) const
{
    USHORT nSelect = (USHORT) ( ( rPoint.Y() + mnBorder ) / mnMaxHeight ) + (USHORT) mnTop;
    if( nSelect < mnTop || nSelect >= mpEntryList->GetEntryCount() )
        nSelect = LISTBOX_ENTRY_NOTFOUND;
    return nSelect;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
	mbMouseMoveSelect = FALSE;	// Nur bis zum ersten MouseButtonDown
	maSearchStr.Erase();

	if ( !IsReadOnly() )
	{
		if( rMEvt.GetClicks() == 1 )
		{
			USHORT nSelect = GetEntryPosForPoint( rMEvt.GetPosPixel() );
			if( nSelect != LISTBOX_ENTRY_NOTFOUND )
			{
				if ( !mbMulti && GetEntryList()->GetSelectEntryCount() )
					mnTrackingSaveSelection = GetEntryList()->GetSelectEntryPos( 0 );
				else
					mnTrackingSaveSelection = LISTBOX_ENTRY_NOTFOUND;

				mnCurrentPos = nSelect;
				mbTrackingSelect = TRUE;
				SelectEntries( nSelect, LET_MBDOWN, rMEvt.IsShift(), rMEvt.IsMod1() );
				mbTrackingSelect = FALSE;
				if ( mbGrabFocus )
					GrabFocus();

				StartTracking( STARTTRACK_SCROLLREPEAT );
			}
		}
		if( rMEvt.GetClicks() == 2 )
		{
			maDoubleClickHdl.Call( this );
		}
	}
	else // if ( mbGrabFocus )
	{
		GrabFocus();
	}
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::MouseMove( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeaveWindow() )
    {
		if ( mbStackMode && IsMouseMoveSelect() && IsReallyVisible() )
		{
			if ( rMEvt.GetPosPixel().Y() < 0 )
			{
				DeselectAll();
				mnCurrentPos = LISTBOX_ENTRY_NOTFOUND;
                SetTopEntry( 0 );
				if ( mbStackMode ) // #87072#, #92323#
				{
					mbTravelSelect = TRUE;
					mnSelectModifier = rMEvt.GetModifier();
					ImplCallSelect();
					mbTravelSelect = FALSE;
				}

			}
		}
    }
    else if ( ( ( !mbMulti && IsMouseMoveSelect() ) || mbStackMode ) && mpEntryList->GetEntryCount() )
	{
		Point aPoint;
		Rectangle aRect( aPoint, GetOutputSizePixel() );
		if( aRect.IsInside( rMEvt.GetPosPixel() ) )
		{
			if ( IsMouseMoveSelect() )
			{
				USHORT nSelect = (USHORT) ( ( rMEvt.GetPosPixel().Y() + mnBorder ) / mnMaxHeight ) + (USHORT) mnTop;
				nSelect = Min( nSelect, (USHORT) ( mnTop + mnMaxVisibleEntries ) );
				nSelect = Min( nSelect, (USHORT) ( mpEntryList->GetEntryCount() - 1 ) );
				// Select only visible Entries with MouseMove, otherwise Tracking...
				if ( IsVisible( nSelect ) && 
					mpEntryList->IsEntrySelectable( nSelect ) &&
					( ( nSelect != mnCurrentPos ) || !GetEntryList()->GetSelectEntryCount() || ( nSelect != GetEntryList()->GetSelectEntryPos( 0 ) ) ) )
				{
					mbTrackingSelect = TRUE;
					if ( SelectEntries( nSelect, LET_TRACKING, FALSE, FALSE ) )
		            {
                        if ( mbStackMode ) // #87072#
                        {
			                mbTravelSelect = TRUE;
			                mnSelectModifier = rMEvt.GetModifier();
			                ImplCallSelect();
			                mbTravelSelect = FALSE;
                        }
		            }
					mbTrackingSelect = FALSE;
				}
			}

			// Falls der DD-Button gedrueckt wurde und jemand mit gedrueckter
			// Maustaste in die ListBox faehrt...
			if ( rMEvt.IsLeft() && !rMEvt.IsSynthetic() )
			{
				if ( !mbMulti && GetEntryList()->GetSelectEntryCount() )
					mnTrackingSaveSelection = GetEntryList()->GetSelectEntryPos( 0 );
				else
					mnTrackingSaveSelection = LISTBOX_ENTRY_NOTFOUND;

				if ( mbStackMode && ( mpEntryList->GetSelectionAnchor() == LISTBOX_ENTRY_NOTFOUND ) )
					mpEntryList->SetSelectionAnchor( 0 );

				StartTracking( STARTTRACK_SCROLLREPEAT );
			}
		}
	}
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::DeselectAll()
{
	while ( GetEntryList()->GetSelectEntryCount() )
	{
		USHORT nS = GetEntryList()->GetSelectEntryPos( 0 );
		SelectEntry( nS, FALSE );
	}
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::SelectEntry( USHORT nPos, BOOL bSelect )
{
	if( (mpEntryList->IsEntryPosSelected( nPos ) != bSelect) && mpEntryList->IsEntrySelectable( nPos ) )
	{
		ImplHideFocusRect();
		if( bSelect )
		{
			if( !mbMulti )
			{
				// Selektierten Eintrag deselektieren
				USHORT nDeselect = GetEntryList()->GetSelectEntryPos( 0 );
				if( nDeselect != LISTBOX_ENTRY_NOTFOUND )
				{
					//SelectEntryPos( nDeselect, FALSE );
					GetEntryList()->SelectEntry( nDeselect, FALSE );
					if ( IsUpdateMode() && IsReallyVisible() )
						ImplPaint( nDeselect, TRUE );
				}
			}
			mpEntryList->SelectEntry( nPos, TRUE );
			mnCurrentPos = nPos;
			if ( ( nPos != LISTBOX_ENTRY_NOTFOUND ) && IsUpdateMode() )
			{
				ImplPaint( nPos );
				if ( !IsVisible( nPos ) )
                {
                    delete mpLayoutData, mpLayoutData = NULL;
                    if ( !mnMaxVisibleEntries || !IsReallyVisible() || ( nPos < GetTopEntry() ) )
                    {
                        Resize();
					    SetTopEntry( nPos );
                    }
                    else
                    {
                        SetTopEntry( nPos-mnMaxVisibleEntries+1 );
                    }
                }
			}
		}
		else
		{
			mpEntryList->SelectEntry( nPos, FALSE );
			ImplPaint( nPos, TRUE );
		}
		mbSelectionChanged = TRUE;
	}
}

// -----------------------------------------------------------------------

BOOL ImplListBoxWindow::SelectEntries( USHORT nSelect, LB_EVENT_TYPE eLET, BOOL bShift, BOOL bCtrl )
{
	BOOL bFocusChanged = FALSE;
	BOOL bSelectionChanged = FALSE;

	if( IsEnabled() && mpEntryList->IsEntrySelectable( nSelect ) )
	{
		// Hier (Single-ListBox) kann nur ein Eintrag deselektiert werden
		if( !mbMulti )
		{
			USHORT nDeselect = mpEntryList->GetSelectEntryPos( 0 );
			if( nSelect != nDeselect )
			{
				SelectEntry( nSelect, TRUE );
				mpEntryList->SetLastSelected( nSelect );
				bFocusChanged = TRUE;
				bSelectionChanged = TRUE;
			}
		}
		// MultiListBox ohne Modifier
		else if( mbSimpleMode && !bCtrl && !bShift )
		{
			USHORT nEntryCount = mpEntryList->GetEntryCount();
			for ( USHORT nPos = 0; nPos < nEntryCount; nPos++ )
			{
				BOOL bSelect = nPos == nSelect;
				if ( mpEntryList->IsEntryPosSelected( nPos ) != bSelect )
				{
					SelectEntry( nPos, bSelect );
					bFocusChanged = TRUE;
					bSelectionChanged = TRUE;
				}
			}
			mpEntryList->SetLastSelected( nSelect );
			mpEntryList->SetSelectionAnchor( nSelect );
		}
		// MultiListBox nur mit CTRL/SHIFT oder nicht im SimpleMode
		else if( ( !mbSimpleMode /* && !bShift */ ) || ( mbSimpleMode && ( bCtrl || bShift ) || mbStackMode ) )
		{
			// Space fuer Selektionswechsel
			if( !bShift && ( ( eLET == LET_KEYSPACE ) || ( eLET == LET_MBDOWN ) ) )
			{
				BOOL bSelect = ( mbStackMode && IsMouseMoveSelect() ) ? TRUE : !mpEntryList->IsEntryPosSelected( nSelect );
				if ( mbStackMode )
				{
					USHORT n;
					if ( bSelect )
					{
						// All entries before nSelect must be selected...
						for ( n = 0; n < nSelect; n++ )
							SelectEntry( n, TRUE );
					}
					if ( !bSelect )
					{
						for ( n = nSelect+1; n < mpEntryList->GetEntryCount(); n++ )
							SelectEntry( n, FALSE );
					}
				}
				SelectEntry( nSelect, bSelect );
				mpEntryList->SetLastSelected( nSelect );
				mpEntryList->SetSelectionAnchor( mbStackMode ? 0 : nSelect );
				if ( !mpEntryList->IsEntryPosSelected( nSelect ) )
					mpEntryList->SetSelectionAnchor( LISTBOX_ENTRY_NOTFOUND );
				bFocusChanged = TRUE;
				bSelectionChanged = TRUE;
			}
			else if( ( ( eLET == LET_TRACKING ) && ( nSelect != mnCurrentPos ) ) ||
					 ( (bShift||mbStackMode) && ( ( eLET == LET_KEYMOVE ) || ( eLET == LET_MBDOWN ) ) ) )
			{
				mnCurrentPos = nSelect;
				bFocusChanged = TRUE;

				USHORT nAnchor = mpEntryList->GetSelectionAnchor();
				if( ( nAnchor == LISTBOX_ENTRY_NOTFOUND ) && ( mpEntryList->GetSelectEntryCount() || mbStackMode ) )
				{
					nAnchor = mbStackMode ? 0 : mpEntryList->GetSelectEntryPos( mpEntryList->GetSelectEntryCount() - 1 );
				}
				if( nAnchor != LISTBOX_ENTRY_NOTFOUND )
				{
					// Alle Eintraege vom Anchor bis nSelect muessen selektiert sein
					USHORT nStart = Min( nSelect, nAnchor );
					USHORT nEnd = Max( nSelect, nAnchor );
					for ( USHORT n = nStart; n <= nEnd; n++ )
					{
						if ( !mpEntryList->IsEntryPosSelected( n ) )
						{
							SelectEntry( n, TRUE );
							bSelectionChanged = TRUE;
						}
					}

					// Ggf. muss noch was deselektiert werden...
					USHORT nLast = mpEntryList->GetLastSelected();
					if ( nLast != LISTBOX_ENTRY_NOTFOUND )
					{
						if ( ( nLast > nSelect ) && ( nLast > nAnchor ) )
						{
							for ( USHORT n = nSelect+1; n <= nLast; n++ )
							{
								if ( mpEntryList->IsEntryPosSelected( n ) )
								{
									SelectEntry( n, FALSE );
									bSelectionChanged = TRUE;
								}
							}
						}
						else if ( ( nLast < nSelect ) && ( nLast < nAnchor ) )
						{
							for ( USHORT n = nLast; n < nSelect; n++ )
							{
								if ( mpEntryList->IsEntryPosSelected( n ) )
								{
									SelectEntry( n, FALSE );
									bSelectionChanged = TRUE;
								}
							}
						}
					}
					mpEntryList->SetLastSelected( nSelect );
				}
			}
			else if( eLET != LET_TRACKING )
			{
				ImplHideFocusRect();
				ImplPaint( nSelect, TRUE );
				bFocusChanged = TRUE;
			}
		}
		else if( bShift )
		{
			bFocusChanged = TRUE;
		}

		if( bSelectionChanged )
			mbSelectionChanged = TRUE;

		if( bFocusChanged )
		{
			maFocusRect.SetPos( Point( 0, ( nSelect - mnTop ) * mnMaxHeight ) );
			if( HasFocus() )
				ImplShowFocusRect();
		}
        delete mpLayoutData, mpLayoutData = NULL;
	}
	return bSelectionChanged;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::Tracking( const TrackingEvent& rTEvt )
{
	Point aPoint;
	Rectangle aRect( aPoint, GetOutputSizePixel() );
	BOOL bInside = aRect.IsInside( rTEvt.GetMouseEvent().GetPosPixel() );

	if( rTEvt.IsTrackingCanceled() || rTEvt.IsTrackingEnded() ) // MouseButtonUp
	{
		if ( bInside && !rTEvt.IsTrackingCanceled() )
		{
			mnSelectModifier = rTEvt.GetMouseEvent().GetModifier();
			ImplCallSelect();
		}
		else
		{
			maCancelHdl.Call( NULL );
			if ( !mbMulti )
			{
				mbTrackingSelect = TRUE;
				SelectEntry( mnTrackingSaveSelection, TRUE );
				mbTrackingSelect = FALSE;
				if ( mnTrackingSaveSelection != LISTBOX_ENTRY_NOTFOUND )
				{
					maFocusRect.SetPos( Point( 0, ( mnCurrentPos - mnTop ) * mnMaxHeight ) );
					ImplShowFocusRect();
				}
			}
		}

		mbTrack = FALSE;
	}
	else
	{
		BOOL bTrackOrQuickClick = mbTrack;
		if( !mbTrack )
		{
			if ( bInside )
			{
				mbTrack = TRUE;
			}

			// Folgender Fall tritt nur auf, wenn man ganz kurz die Maustaste drueckt
			if( rTEvt.IsTrackingEnded() && mbTrack )
			{
				bTrackOrQuickClick = TRUE;
				mbTrack = FALSE;
			}
		}

		if( bTrackOrQuickClick )
		{
			MouseEvent aMEvt = rTEvt.GetMouseEvent();
			Point aPt( aMEvt.GetPosPixel() );
			BOOL bShift = aMEvt.IsShift();
			BOOL bCtrl	= aMEvt.IsMod1();

			USHORT nSelect = LISTBOX_ENTRY_NOTFOUND;
			if( aPt.Y() < 0 )
			{
                if ( mnCurrentPos != LISTBOX_ENTRY_NOTFOUND )
                {
				    nSelect = mnCurrentPos ? ( mnCurrentPos - 1 ) : 0;
				    if( nSelect < mnTop )
					    SetTopEntry( mnTop-1 );
                }
			}
			else if( aPt.Y() > GetOutputSizePixel().Height() )
			{
                if ( mnCurrentPos != LISTBOX_ENTRY_NOTFOUND )
                {
				    nSelect = Min(	(USHORT)(mnCurrentPos+1), (USHORT)(mpEntryList->GetEntryCount()-1) );
				    if( nSelect >= mnTop + mnMaxVisibleEntries )
					    SetTopEntry( mnTop+1 );
                }
			}
			else
			{
				nSelect = (USHORT) ( ( aPt.Y() + mnBorder ) / mnMaxHeight ) + (USHORT) mnTop;
				nSelect = Min( nSelect, (USHORT) ( mnTop + mnMaxVisibleEntries ) );
				nSelect = Min( nSelect, (USHORT) ( mpEntryList->GetEntryCount() - 1 ) );
			}

			if ( bInside )
			{
				if ( ( nSelect != mnCurrentPos ) || !GetEntryList()->GetSelectEntryCount() )
				{
					mbTrackingSelect = TRUE;
					if ( SelectEntries( nSelect, LET_TRACKING, bShift, bCtrl ) )
		            {
                        if ( mbStackMode ) // #87734# (#87072#)
                        {
			                mbTravelSelect = TRUE;
			                mnSelectModifier = rTEvt.GetMouseEvent().GetModifier();
			                ImplCallSelect();
			                mbTravelSelect = FALSE;
                        }
		            }
					mbTrackingSelect = FALSE;
				}
			}
			else
			{
				if ( !mbMulti && GetEntryList()->GetSelectEntryCount() )
				{
					mbTrackingSelect = TRUE;
					SelectEntry( GetEntryList()->GetSelectEntryPos( 0 ), FALSE );
					mbTrackingSelect = FALSE;
				}
				else if ( mbStackMode )
                {
                    if ( ( rTEvt.GetMouseEvent().GetPosPixel().X() > 0 )  && ( rTEvt.GetMouseEvent().GetPosPixel().X() < aRect.Right() ) )
                    {
				        if ( ( rTEvt.GetMouseEvent().GetPosPixel().Y() < 0 ) || ( rTEvt.GetMouseEvent().GetPosPixel().Y() > GetOutputSizePixel().Height() ) )
				        {
                            BOOL bSelectionChanged = FALSE;
                            if ( ( rTEvt.GetMouseEvent().GetPosPixel().Y() < 0 )
                                   && !mnCurrentPos )
                            {
                                if ( mpEntryList->IsEntryPosSelected( 0 ) )
                                {
					                SelectEntry( 0, FALSE );
                                    bSelectionChanged = TRUE;
                                    nSelect = LISTBOX_ENTRY_NOTFOUND;

                                }
                            }
                            else
                            {
					            mbTrackingSelect = TRUE;
                                bSelectionChanged = SelectEntries( nSelect, LET_TRACKING, bShift, bCtrl );
					            mbTrackingSelect = FALSE;
                            }

                            if ( bSelectionChanged )
		                    {
                                mbSelectionChanged = TRUE;
			                    mbTravelSelect = TRUE;
			                    mnSelectModifier = rTEvt.GetMouseEvent().GetModifier();
			                    ImplCallSelect();
			                    mbTravelSelect = FALSE;
		                    }
				        }
                    }
                }
			}
			mnCurrentPos = nSelect;
            if ( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
            {
                ImplHideFocusRect();
            }
            else
            {
				maFocusRect.SetPos( Point( 0, ( mnCurrentPos - mnTop ) * mnMaxHeight ) );
				ImplShowFocusRect();
            }
		}
	}
}


// -----------------------------------------------------------------------

void ImplListBoxWindow::KeyInput( const KeyEvent& rKEvt )
{
	if( !ProcessKeyInput( rKEvt ) )
		Control::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

#define IMPL_SELECT_NODIRECTION	0
#define IMPL_SELECT_UP			1
#define IMPL_SELECT_DOWN		2

BOOL ImplListBoxWindow::ProcessKeyInput( const KeyEvent& rKEvt )
{
	// zu selektierender Eintrag
	USHORT nSelect = LISTBOX_ENTRY_NOTFOUND;
	LB_EVENT_TYPE eLET = LET_KEYMOVE;

	KeyCode aKeyCode = rKEvt.GetKeyCode();

	BOOL bShift = aKeyCode.IsShift();
	BOOL bCtrl	= aKeyCode.IsMod1();
	BOOL bMod2 = aKeyCode.IsMod2();
	BOOL bDone = FALSE;
	USHORT nSelectDirection = IMPL_SELECT_NODIRECTION;

	switch( aKeyCode.GetCode() )
	{
		case KEY_UP:
		{
			if ( IsReadOnly() )
			{
				if ( GetTopEntry() )
					SetTopEntry( GetTopEntry()-1 );
			}
			else if ( !bMod2 )
			{
				if( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
				{
					nSelect = mpEntryList->FindFirstSelectable( 0, true );
				}
				else if ( mnCurrentPos )
				{
					// search first selectable above the current position
					nSelect = mpEntryList->FindFirstSelectable( mnCurrentPos - 1, false );
				}

				if( ( nSelect != LISTBOX_ENTRY_NOTFOUND ) && ( nSelect < mnTop ) )
					SetTopEntry( mnTop-1 );

				bDone = TRUE;
			}
			maSearchStr.Erase();
		}
		break;

		case KEY_DOWN:
		{
			if ( IsReadOnly() )
			{
				SetTopEntry( GetTopEntry()+1 );
			}
			else if ( !bMod2 )
			{
				if( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
				{
					nSelect = mpEntryList->FindFirstSelectable( 0, true );
				}
				else if ( (mnCurrentPos+1) < mpEntryList->GetEntryCount() )
				{
					// search first selectable below the current position
					nSelect = mpEntryList->FindFirstSelectable( mnCurrentPos + 1, true );
				}

				if( ( nSelect != LISTBOX_ENTRY_NOTFOUND ) && ( nSelect >= ( mnTop + mnMaxVisibleEntries ) ) )
					SetTopEntry( mnTop+1 );

				bDone = TRUE;
			}
			maSearchStr.Erase();
		}
		break;

		case KEY_PAGEUP:
		{
			if ( IsReadOnly() )
			{
				SetTopEntry( ( mnTop > mnMaxVisibleEntries ) ?
								(mnTop-mnMaxVisibleEntries) : 0 );
			}
			else if ( !bCtrl && !bMod2 )
			{
				if( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
				{
					nSelect = mpEntryList->FindFirstSelectable( 0, true );
				}
				else if ( mnCurrentPos )
				{
					if( mnCurrentPos == mnTop )
						SetTopEntry( ( mnTop > mnMaxVisibleEntries ) ? ( mnTop-mnMaxVisibleEntries+1 ) : 0 );
					
					// find first selectable starting from mnTop looking foreward
					nSelect = mpEntryList->FindFirstSelectable( mnTop, true );
				}
				bDone = TRUE;
			}
			maSearchStr.Erase();
		}
		break;

		case KEY_PAGEDOWN:
		{
			if ( IsReadOnly() )
			{
				SetTopEntry( mnTop + mnMaxVisibleEntries );
			}
			else if ( !bCtrl && !bMod2 )
			{
				if( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
				{
					nSelect = mpEntryList->FindFirstSelectable( 0, true );
				}
				else if ( (mnCurrentPos+1) < mpEntryList->GetEntryCount() )
				{
					USHORT nCount = mpEntryList->GetEntryCount();
					USHORT nTmp = Min( mnMaxVisibleEntries, nCount );
					nTmp += mnTop - 1;
					if( mnCurrentPos == nTmp && mnCurrentPos != nCount - 1 )
					{
						long nTmp2 = Min( (long)(nCount-mnMaxVisibleEntries), (long)((long)mnTop+(long)mnMaxVisibleEntries-1) );
						nTmp2 = Max( (long)0 , nTmp2 );
						nTmp = (USHORT)(nTmp2+(mnMaxVisibleEntries-1) );
						SetTopEntry( (USHORT)nTmp2 );
					}
					// find first selectable starting from nTmp looking backwards
					nSelect = mpEntryList->FindFirstSelectable( nTmp, false );
				}
				bDone = TRUE;
			}
			maSearchStr.Erase();
		}
		break;

		case KEY_HOME:
		{
			if ( IsReadOnly() )
			{
				SetTopEntry( 0 );
			}
			else if ( !bCtrl && !bMod2 )
			{
				if ( mnCurrentPos )
				{
					nSelect = mpEntryList->FindFirstSelectable( mpEntryList->GetEntryCount() ? 0 : LISTBOX_ENTRY_NOTFOUND, true );
					if( mnTop != 0 )
						SetTopEntry( 0 );

					bDone = TRUE;
				}
			}
			maSearchStr.Erase();
		}
		break;

		case KEY_END:
		{
			if ( IsReadOnly() )
			{
				SetTopEntry( 0xFFFF );
			}
			else if ( !bCtrl && !bMod2 )
			{
				if( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
				{
					nSelect = mpEntryList->FindFirstSelectable( 0, true );
				}
				else if ( (mnCurrentPos+1) < mpEntryList->GetEntryCount() )
				{
					USHORT nCount = mpEntryList->GetEntryCount();
					nSelect = mpEntryList->FindFirstSelectable( nCount - 1, false );
					if( nCount > mnMaxVisibleEntries )
						SetTopEntry( nCount - mnMaxVisibleEntries );
				}
				bDone = TRUE;
			}
			maSearchStr.Erase();
		}
		break;

		case KEY_LEFT:
		{
			if ( !bCtrl && !bMod2 )
			{
				ScrollHorz( -HORZ_SCROLL );
				bDone = TRUE;
			}
			maSearchStr.Erase();
		}
		break;

		case KEY_RIGHT:
		{
			if ( !bCtrl && !bMod2 )
			{
				ScrollHorz( HORZ_SCROLL );
				bDone = TRUE;
			}
			maSearchStr.Erase();
		}
		break;

		case KEY_RETURN:
		{
			if ( !bMod2 && !IsReadOnly() )
			{
				mnSelectModifier = rKEvt.GetKeyCode().GetModifier();
				ImplCallSelect();
				bDone = FALSE;	// RETURN nicht abfangen.
			}
			maSearchStr.Erase();
		}
		break;

		case KEY_SPACE:
		{
			if ( !bMod2 && !IsReadOnly() )
			{
				if( mbMulti && ( !mbSimpleMode || ( mbSimpleMode && bCtrl && !bShift ) || mbStackMode ) )
				{
					nSelect = mnCurrentPos;
					eLET = LET_KEYSPACE;
				}
				bDone = TRUE;
			}
			maSearchStr.Erase();
		}
		break;

		default:
		{
			xub_Unicode c = rKEvt.GetCharCode();

			if ( !IsReadOnly() && (c >= 32) && (c != 127) &&
				 !rKEvt.GetKeyCode().IsControlMod() )
			{
				maSearchStr += c;
				XubString aTmpSearch( maSearchStr );

				nSelect = mpEntryList->FindMatchingEntry( aTmpSearch, mnCurrentPos );
				if ( (nSelect == LISTBOX_ENTRY_NOTFOUND) && (aTmpSearch.Len() > 1) )
				{
					// Wenn alles die gleichen Buchstaben, dann anderer Such-Modus
					BOOL bAllEqual = TRUE;
					for ( USHORT n = aTmpSearch.Len(); n && bAllEqual; )
						bAllEqual = aTmpSearch.GetChar( --n ) == c;
					if ( bAllEqual )
					{
						aTmpSearch = c;
						nSelect = mpEntryList->FindMatchingEntry( aTmpSearch, mnCurrentPos+1 );
					}
				}
				if ( nSelect == LISTBOX_ENTRY_NOTFOUND )
					nSelect = mpEntryList->FindMatchingEntry( aTmpSearch, 0 );

				if ( nSelect != LISTBOX_ENTRY_NOTFOUND )
				{
					if( nSelect < mnTop )
						SetTopEntry( nSelect );
					else if( nSelect >= (mnTop + mnMaxVisibleEntries) )
						SetTopEntry( nSelect - mnMaxVisibleEntries + 1 );

					if ( nSelect == mnCurrentPos )
						nSelect = LISTBOX_ENTRY_NOTFOUND;

					maSearchTimeout.Start();
				}
				else
					maSearchStr.Erase();
				bDone = TRUE;
			}
		}
	}

	if ( (nSelect != LISTBOX_ENTRY_NOTFOUND) &&
		 ((nSelect != mnCurrentPos ) || ( eLET == LET_KEYSPACE)) )
	{
		DBG_ASSERT( (nSelect != mnCurrentPos) || mbMulti, "ImplListBox: Selecting same Entry" );
	    if( nSelect >= mpEntryList->GetEntryCount() )
            nSelect = mpEntryList->GetEntryCount()-1;
		mnCurrentPos = nSelect;
		if ( SelectEntries( nSelect, eLET, bShift, bCtrl ) )
		{
			mbTravelSelect = TRUE;
			mnSelectModifier = rKEvt.GetKeyCode().GetModifier();
			ImplCallSelect();
			mbTravelSelect = FALSE;
		}
	}

	return bDone;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::ImplPaint( USHORT nPos, BOOL bErase, bool bLayout )
{
	const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

	long nWidth  = GetOutputSizePixel().Width();
	long nY = ( nPos - mnTop ) * mnMaxHeight;
	Rectangle aRect( Point( 0, nY ), Size( nWidth, mnMaxHeight ) );

    if( ! bLayout )
    {
        if( mpEntryList->IsEntryPosSelected( nPos ) )
        {
            SetTextColor( !IsEnabled() ? rStyleSettings.GetDisableColor() : rStyleSettings.GetHighlightTextColor() );
            SetFillColor( rStyleSettings.GetHighlightColor() );
            SetTextFillColor( rStyleSettings.GetHighlightColor() );
            DrawRect( aRect );
        }
        else
        {
            ImplInitSettings( FALSE, TRUE, FALSE );
            if( !IsEnabled() )
                SetTextColor( rStyleSettings.GetDisableColor() );
            SetTextFillColor();
            if( bErase )
                Erase( aRect );
        }
    }

    if ( IsUserDrawEnabled() )
    {
        mbInUserDraw = TRUE;
		mnUserDrawEntry = nPos;
		aRect.Left() -= mnLeft;
		if ( nPos < GetEntryList()->GetMRUCount() )
			nPos = GetEntryList()->FindEntry( GetEntryList()->GetEntryText( nPos ) );
		nPos -= GetEntryList()->GetMRUCount();
		UserDrawEvent aUDEvt( this, aRect, nPos, 0 );
		maUserDrawHdl.Call( &aUDEvt );
		mbInUserDraw = FALSE;
	}
	else
	{
		DrawEntry( nPos, TRUE, TRUE, FALSE, bLayout );
	}
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::DrawEntry( USHORT nPos, BOOL bDrawImage, BOOL bDrawText, BOOL bDrawTextAtImagePos, bool bLayout )
{
	// Bei Aenderungen in dieser Methode ggf. auch ImplWin::DrawEntry() anpassen.

	if ( mbInUserDraw )
		nPos = mnUserDrawEntry; // real entry, not the matching entry from MRU

    long nY = ( nPos - mnTop ) * mnMaxHeight;
	Size aImgSz;

	if( bDrawImage && mpEntryList->HasImages() && !bLayout )
	{
		Image aImage = mpEntryList->GetEntryImage( nPos );
		if( !!aImage )
		{
            aImgSz = aImage.GetSizePixel();
			Point aPtImg( mnBorder - mnLeft, nY + ( ( mnMaxHeight - aImgSz.Height() ) / 2 ) );

			// pb: #106948# explicit mirroring for calc
			if ( mbMirroring )
				// right aligned
				aPtImg.X() = mnMaxWidth + mnBorder - aImgSz.Width() - mnLeft;

			if ( !IsZoom() )
			{
				DrawImage( aPtImg, aImage );
			}
			else
			{
				aImgSz.Width() = CalcZoom( aImgSz.Width() );
				aImgSz.Height() = CalcZoom( aImgSz.Height() );
				DrawImage( aPtImg, aImgSz, aImage );
			}
		}
	}

	if( bDrawText )
	{
        MetricVector* pVector = bLayout ? &mpLayoutData->m_aUnicodeBoundRects : NULL;
        String* pDisplayText = bLayout ? &mpLayoutData->m_aDisplayText : NULL;
		XubString aStr( mpEntryList->GetEntryText( nPos ) );
		if ( aStr.Len() )
		{
            long nMaxWidth = Max( static_cast< long >( mnMaxWidth ),
                                  GetOutputSizePixel().Width() - 2*mnBorder );

            Rectangle aTextRect( Point( mnBorder - mnLeft, nY ),
                                 Size( nMaxWidth, mnMaxHeight ) );

            if( !bDrawTextAtImagePos && ( mpEntryList->HasEntryImage(nPos) || IsUserDrawEnabled() ) )
			{
				USHORT nMaxWidth = Max( mnMaxImgWidth, (USHORT)maUserItemSize.Width() );
                aTextRect.Left() += nMaxWidth + IMG_TXT_DISTANCE;
			}

            if( bLayout )
                mpLayoutData->m_aLineIndices.push_back( mpLayoutData->m_aDisplayText.Len() );

			// pb: #106948# explicit mirroring for calc
			if ( mbMirroring )
			{
				// right aligned
                aTextRect.Left() = nMaxWidth + mnBorder - GetTextWidth( aStr ) - mnLeft;
				if ( aImgSz.Width() > 0 )
					aTextRect.Left() -= ( aImgSz.Width() + IMG_TXT_DISTANCE );
			}

            DrawText( aTextRect, aStr, ImplGetTextStyle(), pVector, pDisplayText );
		}
	}

    if( !bLayout )
    {
        if ( ( mnSeparatorPos != LISTBOX_ENTRY_NOTFOUND ) &&
             ( ( nPos == mnSeparatorPos ) || ( nPos == mnSeparatorPos+1 ) ) )
        {
            Color aOldLineColor( GetLineColor() );
            SetLineColor( ( GetBackground().GetColor() != COL_LIGHTGRAY ) ? COL_LIGHTGRAY : COL_GRAY );
            Point aStartPos( 0, nY );
            if ( nPos == mnSeparatorPos )
                aStartPos.Y() += mnMaxHeight-1;
            Point aEndPos( aStartPos );
            aEndPos.X() = GetOutputSizePixel().Width();
            DrawLine( aStartPos, aEndPos );
            SetLineColor( aOldLineColor );
        }
    }
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::FillLayoutData() const
{
    mpLayoutData = new vcl::ControlLayoutData();
    const_cast<ImplListBoxWindow*>(this)->
        ImplDoPaint( Rectangle( Point( 0, 0 ), GetOutputSize() ), true );
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::ImplDoPaint( const Rectangle& rRect, bool bLayout )
{
	USHORT nCount = mpEntryList->GetEntryCount();

    BOOL bShowFocusRect = mbHasFocusRect;
    if ( mbHasFocusRect && ! bLayout )
        ImplHideFocusRect();

	long nY = 0; // + mnBorder;
	long nHeight = GetOutputSizePixel().Height();// - mnMaxHeight + mnBorder;

	for( USHORT i = (USHORT)mnTop; i < nCount && nY < nHeight + mnMaxHeight; i++ )
	{
		if( nY + mnMaxHeight >= rRect.Top() &&
			nY <= rRect.Bottom() + mnMaxHeight )
		{
			ImplPaint( i, FALSE, bLayout );
		}
		nY += mnMaxHeight;
	}

	maFocusRect.SetPos( Point( 0, ( mnCurrentPos - mnTop ) * mnMaxHeight ) );
	if( HasFocus() && bShowFocusRect && !bLayout )
		ImplShowFocusRect();
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::Paint( const Rectangle& rRect )
{
    ImplDoPaint( rRect );
}

// -----------------------------------------------------------------------

USHORT ImplListBoxWindow::GetDisplayLineCount() const
{
	USHORT nCount = mpEntryList->GetEntryCount();
	long nHeight = GetOutputSizePixel().Height();// - mnMaxHeight + mnBorder;
    USHORT nEntries = static_cast< USHORT >( ( nHeight + mnMaxHeight - 1 ) / mnMaxHeight );
    if( nEntries > nCount-mnTop )
        nEntries = nCount-mnTop;

    return nEntries;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::Resize()
{
    Control::Resize();

    BOOL bShowFocusRect = mbHasFocusRect;
    if ( bShowFocusRect )
        ImplHideFocusRect();

    Size aSz( GetOutputSizePixel().Width(), mnMaxHeight );
	maFocusRect.SetSize( aSz );
	mnMaxVisibleEntries = (USHORT) ( GetOutputSizePixel().Height() / mnMaxHeight );

    if ( bShowFocusRect )
        ImplShowFocusRect();

    delete mpLayoutData, mpLayoutData = NULL;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::CalcMaxVisibleEntries( const Size& rFloatSize )
{
    if( mnMaxHeight )
        mnMaxVisibleEntries = (USHORT) ( (rFloatSize.Height()-4) / mnMaxHeight );
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::GetFocus()
{
	USHORT nPos = mnCurrentPos;
	if ( nPos == LISTBOX_ENTRY_NOTFOUND )
		nPos = 0;
	maFocusRect.SetPos( Point( 0, ( nPos - mnTop ) * mnMaxHeight ) );
	ImplShowFocusRect();
	Control::GetFocus();
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::LoseFocus()
{
	ImplHideFocusRect();
	Control::LoseFocus();
}

// -----------------------------------------------------------------------

/*
void ImplListBoxWindow::RequestHelp( const HelpEvent& rHEvt )
{
	if ( rHEvt.GetMode() & HELPMODE_BALLOON )
		Help::ShowBalloon( this, rHEvt.GetMousePosPixel(), String() );

	Window::RequestHelp( rHEvt );
}
*/

// -----------------------------------------------------------------------

void ImplListBoxWindow::SetTopEntry( USHORT nTop )
{
	USHORT nMaxTop = 0;
	if ( GetEntryList()->GetEntryCount() > mnMaxVisibleEntries )
		nMaxTop = GetEntryList()->GetEntryCount() - mnMaxVisibleEntries;
	nTop = Min( nTop, nMaxTop );
	if ( nTop != mnTop )
	{
        delete mpLayoutData, mpLayoutData = NULL;
		long nDiff = ( mnTop - nTop ) * mnMaxHeight;
        Update();
		ImplHideFocusRect();
		mnTop = nTop;
		Scroll( 0, nDiff );
        Update();
		maFocusRect.Top() += nDiff;
		maFocusRect.Bottom() += nDiff;
		if( HasFocus() )
			ImplShowFocusRect();
		maScrollHdl.Call( this );
	}
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::SetLeftIndent( USHORT n )
{
	ScrollHorz( n - mnLeft );
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::ScrollHorz( short n )
{
	short nDiff = 0;
	if ( n > 0 )
	{
		long nWidth = GetOutputSizePixel().Width();
		if( ( mnMaxWidth - mnLeft + n ) > nWidth )
			nDiff = n;
	}
	else if ( n < 0 )
	{
		if( mnLeft )
		{
			USHORT nAbs = -n;
			nDiff = - ( ( mnLeft > nAbs ) ? nAbs : mnLeft );
		}
	}

	if ( nDiff )
	{
        delete mpLayoutData, mpLayoutData = NULL;
		mnLeft += nDiff;
        Update();
		ImplHideFocusRect();
		Scroll( -nDiff, 0 );
        Update();
		if( HasFocus() )
			ImplShowFocusRect();
		maScrollHdl.Call( this );
	}
}

// -----------------------------------------------------------------------

Size ImplListBoxWindow::CalcSize( USHORT nMaxLines ) const
{
	Size aSz;
//	USHORT nL = Min( nMaxLines, mpEntryList->GetEntryCount() );
	aSz.Height() =	nMaxLines * mnMaxHeight;
	aSz.Width() = mnMaxWidth + 2*mnBorder;
	return aSz;
}

// -----------------------------------------------------------------------

Rectangle ImplListBoxWindow::GetBoundingRectangle( USHORT nItem ) const
{
    Size aSz( GetSizePixel().Width(), GetEntryHeight() );
    long nY = ( nItem - GetTopEntry() ) * GetEntryHeight();
    Rectangle aRect( Point( 0, nY ), aSz );
    return aRect;
}


// -----------------------------------------------------------------------

void ImplListBoxWindow::StateChanged( StateChangedType nType )
{
	Control::StateChanged( nType );

	if ( nType == STATE_CHANGE_ZOOM )
	{
		ImplInitSettings( TRUE, FALSE, FALSE );
		ImplCalcMetrics();
		Invalidate();
	}
	else if ( nType == STATE_CHANGE_UPDATEMODE )
	{
		if ( IsUpdateMode() && IsReallyVisible() )
			Invalidate();
	}
	else if ( nType == STATE_CHANGE_CONTROLFONT )
	{
		ImplInitSettings( TRUE, FALSE, FALSE );
		ImplCalcMetrics();
		Invalidate();
	}
	else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
	{
		ImplInitSettings( FALSE, TRUE, FALSE );
		Invalidate();
	}
	else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
	{
		ImplInitSettings( FALSE, FALSE, TRUE );
		Invalidate();
	}
    delete mpLayoutData, mpLayoutData = NULL;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
	Control::DataChanged( rDCEvt );

	if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
		 (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
		 ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
		  (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
	{
        delete mpLayoutData, mpLayoutData = NULL;
		ImplInitSettings( TRUE, TRUE, TRUE );
		ImplCalcMetrics();
		Invalidate();
	}
}

// -----------------------------------------------------------------------

USHORT ImplListBoxWindow::ImplGetTextStyle() const
{
    USHORT nTextStyle = TEXT_DRAW_VCENTER;

    if ( mpEntryList->HasImages() )
        nTextStyle |= TEXT_DRAW_LEFT;
    else if ( mbCenter )
        nTextStyle |= TEXT_DRAW_CENTER;
    else if ( mbRight )
        nTextStyle |= TEXT_DRAW_RIGHT;
    else
        nTextStyle |= TEXT_DRAW_LEFT;

    return nTextStyle;
}

// =======================================================================

ImplListBox::ImplListBox( Window* pParent, WinBits nWinStyle ) :
	Control( pParent, nWinStyle ),
	maLBWindow( this, nWinStyle&(~WB_BORDER) )
{
    // for native widget rendering we must be able to detect this window type
    SetType( WINDOW_LISTBOXWINDOW );

	mpVScrollBar	= new ScrollBar( this, WB_VSCROLL | WB_DRAG );
	mpHScrollBar	= new ScrollBar( this, WB_HSCROLL | WB_DRAG );
	mpScrollBarBox	= new ScrollBarBox( this );

	Link aLink( LINK( this, ImplListBox, ScrollBarHdl ) );
	mpVScrollBar->SetScrollHdl( aLink );
	mpHScrollBar->SetScrollHdl( aLink );

	mbVScroll		= FALSE;
	mbHScroll		= FALSE;
	mbAutoHScroll	= ( nWinStyle & WB_AUTOHSCROLL ) ? TRUE : FALSE;

	maLBWindow.SetScrollHdl( LINK( this, ImplListBox, LBWindowScrolled ) );
	maLBWindow.SetMRUChangedHdl( LINK( this, ImplListBox, MRUChanged ) );
	maLBWindow.Show();
}

// -----------------------------------------------------------------------

ImplListBox::~ImplListBox()
{
	delete mpHScrollBar;
	delete mpVScrollBar;
	delete mpScrollBarBox;
}

// -----------------------------------------------------------------------

void ImplListBox::Clear()
{
	maLBWindow.Clear();
	if ( GetEntryList()->GetMRUCount() )
	{
		maLBWindow.GetEntryList()->SetMRUCount( 0 );
		maLBWindow.SetSeparatorPos( LISTBOX_ENTRY_NOTFOUND );
	}
	mpVScrollBar->SetThumbPos( 0 );
	mpHScrollBar->SetThumbPos( 0 );
	StateChanged( STATE_CHANGE_DATA );
}

// -----------------------------------------------------------------------

USHORT ImplListBox::InsertEntry( USHORT nPos, const XubString& rStr )
{
	ImplEntryType* pNewEntry = new ImplEntryType( rStr );
	USHORT nNewPos = maLBWindow.InsertEntry( nPos, pNewEntry );
	StateChanged( STATE_CHANGE_DATA );
	return nNewPos;
}

// -----------------------------------------------------------------------

USHORT ImplListBox::InsertEntry( USHORT nPos, const Image& rImage )
{
	ImplEntryType* pNewEntry = new ImplEntryType( rImage );
	USHORT nNewPos = maLBWindow.InsertEntry( nPos, pNewEntry );
	StateChanged( STATE_CHANGE_DATA );
	return nNewPos;
}

// -----------------------------------------------------------------------

USHORT ImplListBox::InsertEntry( USHORT nPos, const XubString& rStr, const Image& rImage )
{
	ImplEntryType* pNewEntry = new ImplEntryType( rStr, rImage );
	USHORT nNewPos = maLBWindow.InsertEntry( nPos, pNewEntry );
	StateChanged( STATE_CHANGE_DATA );
	return nNewPos;
}

// -----------------------------------------------------------------------

void ImplListBox::RemoveEntry( USHORT nPos )
{
	maLBWindow.RemoveEntry( nPos );
	StateChanged( STATE_CHANGE_DATA );
}

// -----------------------------------------------------------------------

void ImplListBox::SetEntryFlags( USHORT nPos, long nFlags )
{
	maLBWindow.GetEntryList()->SetEntryFlags( nPos, nFlags );
}

// -----------------------------------------------------------------------

long ImplListBox::GetEntryFlags( USHORT nPos ) const
{
	return maLBWindow.GetEntryList()->GetEntryFlags( nPos );
}

// -----------------------------------------------------------------------

void ImplListBox::SelectEntry( USHORT nPos, BOOL bSelect )
{
	maLBWindow.SelectEntry( nPos, bSelect );
}

// -----------------------------------------------------------------------

void ImplListBox::SetNoSelection()
{
	maLBWindow.DeselectAll();
}

// -----------------------------------------------------------------------

void ImplListBox::GetFocus()
{
	maLBWindow.GrabFocus();
}

// -----------------------------------------------------------------------

Window* ImplListBox::GetPreferredKeyInputWindow()
{
    return &maLBWindow;
}

// -----------------------------------------------------------------------

void ImplListBox::Resize()
{
    Control::Resize();
	ImplResizeControls();
	ImplCheckScrollBars();
}


// -----------------------------------------------------------------------

IMPL_LINK( ImplListBox, MRUChanged, void*, EMPTYARG )
{
	StateChanged( STATE_CHANGE_DATA );
	return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplListBox, LBWindowScrolled, void*, EMPTYARG )
{
	mpVScrollBar->SetThumbPos( GetTopEntry() );
	mpHScrollBar->SetThumbPos( GetLeftIndent() );

	maScrollHdl.Call( this );

	return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplListBox, ScrollBarHdl, ScrollBar*, pSB )
{
	USHORT nPos = (USHORT) pSB->GetThumbPos();
	if( pSB == mpVScrollBar )
		SetTopEntry( nPos );
	else if( pSB == mpHScrollBar )
		SetLeftIndent( nPos );

	return 1;
}

// -----------------------------------------------------------------------

void ImplListBox::ImplCheckScrollBars()
{
	BOOL bArrange = FALSE;

	Size aOutSz = GetOutputSizePixel();
	USHORT nEntries = GetEntryList()->GetEntryCount();
	USHORT nMaxVisEntries = (USHORT) (aOutSz.Height() / GetEntryHeight());

	// vert. ScrollBar
	if( nEntries > nMaxVisEntries )
	{
		if( !mbVScroll )
			bArrange = TRUE;
		mbVScroll = TRUE;

		// Ueberpruefung des rausgescrollten Bereichs
		SetTopEntry( GetTopEntry() );	// MaxTop wird geprueft...
	}
	else
	{
		if( mbVScroll )
			bArrange = TRUE;
		mbVScroll = FALSE;
		SetTopEntry( 0 );
	}

	// horz. ScrollBar
	if( mbAutoHScroll )
	{
		long nWidth = (USHORT) aOutSz.Width();
		if ( mbVScroll )
			nWidth -= mpVScrollBar->GetSizePixel().Width();

		long nMaxWidth = GetMaxEntryWidth();
		if( nWidth < nMaxWidth )
		{
			if( !mbHScroll )
				bArrange = TRUE;
			mbHScroll = TRUE;

			if ( !mbVScroll )	// ggf. brauchen wir jetzt doch einen
			{
				nMaxVisEntries = (USHORT) ( ( aOutSz.Height() - mpHScrollBar->GetSizePixel().Height() ) / GetEntryHeight() );
				if( nEntries > nMaxVisEntries )
				{
					bArrange = TRUE;
					mbVScroll = TRUE;

					// Ueberpruefung des rausgescrollten Bereichs
					SetTopEntry( GetTopEntry() );	// MaxTop wird geprueft...
				}
			}

			// Ueberpruefung des rausgescrollten Bereichs
			USHORT nMaxLI = (USHORT) (nMaxWidth - nWidth);
			if ( nMaxLI < GetLeftIndent() )
				SetLeftIndent( nMaxLI );
		}
		else
		{
			if( mbHScroll )
				bArrange = TRUE;
			mbHScroll = FALSE;
			SetLeftIndent( 0 );
		}
	}

	if( bArrange )
		ImplResizeControls();

	ImplInitScrollBars();
}

// -----------------------------------------------------------------------

void ImplListBox::ImplInitScrollBars()
{
	Size aOutSz = maLBWindow.GetOutputSizePixel();

	if ( mbVScroll )
	{
		USHORT nEntries = GetEntryList()->GetEntryCount();
		USHORT nVisEntries = (USHORT) (aOutSz.Height() / GetEntryHeight());
		mpVScrollBar->SetRangeMax( nEntries );
		mpVScrollBar->SetVisibleSize( nVisEntries );
		mpVScrollBar->SetPageSize( nVisEntries - 1 );
	}

	if ( mbHScroll )
	{
		mpHScrollBar->SetRangeMax( GetMaxEntryWidth() + HORZ_SCROLL );
		mpHScrollBar->SetVisibleSize( (USHORT)aOutSz.Width() );
		mpHScrollBar->SetLineSize( HORZ_SCROLL );
		mpHScrollBar->SetPageSize( aOutSz.Width() - HORZ_SCROLL );
	}
}

// -----------------------------------------------------------------------

void ImplListBox::ImplResizeControls()
{
	// Hier werden die Controls nur angeordnet, ob die Scrollbars
	// sichtbar sein sollen wird bereits in ImplCheckScrollBars ermittelt.

	Size aOutSz = GetOutputSizePixel();
	long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
	nSBWidth = CalcZoom( nSBWidth );

	Size aInnerSz( aOutSz );
	if ( mbVScroll )
		aInnerSz.Width() -= nSBWidth;
	if ( mbHScroll )
		aInnerSz.Height() -= nSBWidth;

	// pb: #106948# explicit mirroring for calc
	// Scrollbar on left or right side?
	BOOL bMirroring = maLBWindow.IsMirroring();
	Point aWinPos( bMirroring && mbVScroll ? nSBWidth : 0, 0 );
	maLBWindow.SetPosSizePixel( aWinPos, aInnerSz );

	// ScrollBarBox
	if( mbVScroll && mbHScroll )
	{
		Point aBoxPos( bMirroring ? 0 : aInnerSz.Width(), aInnerSz.Height() );
		mpScrollBarBox->SetPosSizePixel( aBoxPos, Size( nSBWidth, nSBWidth ) );
		mpScrollBarBox->Show();
	}
	else
	{
		mpScrollBarBox->Hide();
	}

	// vert. ScrollBar
	if( mbVScroll )
	{
		// Scrollbar on left or right side?
		Point aVPos( bMirroring ? 0 : aOutSz.Width() - nSBWidth, 0 );
		mpVScrollBar->SetPosSizePixel( aVPos, Size( nSBWidth, aInnerSz.Height() ) );
		mpVScrollBar->Show();
	}
	else
	{
		mpVScrollBar->Hide();
        // #107254# Don't reset top entry after resize, but check for max top entry
		SetTopEntry( GetTopEntry() );
	}

	// horz. ScrollBar
	if( mbHScroll )
	{
		Point aHPos( ( bMirroring && mbVScroll ) ? nSBWidth : 0, aOutSz.Height() - nSBWidth );
		mpHScrollBar->SetPosSizePixel( aHPos, Size( aInnerSz.Width(), nSBWidth ) );
		mpHScrollBar->Show();
	}
	else
	{
		mpHScrollBar->Hide();
		SetLeftIndent( 0 );
	}
}

// -----------------------------------------------------------------------

void ImplListBox::StateChanged( StateChangedType nType )
{
	if ( nType == STATE_CHANGE_INITSHOW )
	{
		ImplCheckScrollBars();
	}
	else if ( ( nType == STATE_CHANGE_UPDATEMODE ) || ( nType == STATE_CHANGE_DATA ) )
	{
		BOOL bUpdate = IsUpdateMode();
		maLBWindow.SetUpdateMode( bUpdate );
//		mpHScrollBar->SetUpdateMode( bUpdate );
//		mpVScrollBar->SetUpdateMode( bUpdate );
		if ( bUpdate && IsReallyVisible() )
			ImplCheckScrollBars();
	}
	else if( nType == STATE_CHANGE_ENABLE )
	{
		mpHScrollBar->Enable( IsEnabled() );
		mpVScrollBar->Enable( IsEnabled() );
		mpScrollBarBox->Enable( IsEnabled() );
		Invalidate();
	}
	else if ( nType == STATE_CHANGE_ZOOM )
	{
		maLBWindow.SetZoom( GetZoom() );
		Resize();
	}
	else if ( nType == STATE_CHANGE_CONTROLFONT )
	{
		maLBWindow.SetControlFont( GetControlFont() );
	}
	else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
	{
		maLBWindow.SetControlForeground( GetControlForeground() );
	}
	else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
	{
		maLBWindow.SetControlBackground( GetControlBackground() );
	}

	Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

void ImplListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
//	if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
//		 (rDCEvt.GetFlags() & SETTINGS_STYLE) )
//	{
//		maLBWindow.SetSettings( GetSettings() );
//		Resize();
//	}
//	else
		Control::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------

long ImplListBox::Notify( NotifyEvent& rNEvt )
{
	long nDone = 0;
	if ( rNEvt.GetType() == EVENT_COMMAND )
	{
		const CommandEvent& rCEvt = *rNEvt.GetCommandEvent();
		if ( rCEvt.GetCommand() == COMMAND_WHEEL )
		{
			const CommandWheelData* pData = rCEvt.GetWheelData();
			if( !pData->GetModifier() && ( pData->GetMode() == COMMAND_WHEEL_SCROLL ) )
			{
				nDone = HandleScrollCommand( rCEvt, mpHScrollBar, mpVScrollBar );
			}
		}
	}

	return nDone ? nDone : Window::Notify( rNEvt );
}

// -----------------------------------------------------------------------

const Wallpaper& ImplListBox::GetDisplayBackground() const
{
    return maLBWindow.GetDisplayBackground();
}

// -----------------------------------------------------------------------

BOOL ImplListBox::HandleWheelAsCursorTravel( const CommandEvent& rCEvt )
{
	BOOL bDone = FALSE;
	if ( rCEvt.GetCommand() == COMMAND_WHEEL )
	{
		const CommandWheelData* pData = rCEvt.GetWheelData();
		if( !pData->GetModifier() && ( pData->GetMode() == COMMAND_WHEEL_SCROLL ) )
		{
			USHORT nKey = ( pData->GetDelta() < 0 ) ? KEY_DOWN : KEY_UP;
			KeyEvent aKeyEvent( 0, KeyCode( nKey ) );
			bDone = ProcessKeyInput( aKeyEvent );
		}
	}
	return bDone;
}

// -----------------------------------------------------------------------

void ImplListBox::SetMRUEntries( const XubString& rEntries, xub_Unicode cSep )
{
	BOOL bChanges = GetEntryList()->GetMRUCount() ? TRUE : FALSE;

	// Remove old MRU entries
	for ( USHORT n = GetEntryList()->GetMRUCount();n; )
		maLBWindow.RemoveEntry( --n );

	USHORT nMRUCount = 0;
	USHORT nEntries = rEntries.GetTokenCount( cSep );
	for ( USHORT nEntry = 0; nEntry < nEntries; nEntry++ )
	{
		XubString aEntry = rEntries.GetToken( nEntry, cSep );
		// Accept only existing entries
		if ( GetEntryList()->FindEntry( aEntry ) != LISTBOX_ENTRY_NOTFOUND )
		{
			ImplEntryType* pNewEntry = new ImplEntryType( aEntry );
			maLBWindow.GetEntryList()->InsertEntry( nMRUCount++, pNewEntry, FALSE );
			bChanges = TRUE;
		}
	}

	if ( bChanges )
	{
		maLBWindow.GetEntryList()->SetMRUCount( nMRUCount );
		SetSeparatorPos( nMRUCount ? nMRUCount-1 : 0 );
		StateChanged( STATE_CHANGE_DATA );
	}
}

// -----------------------------------------------------------------------

XubString ImplListBox::GetMRUEntries( xub_Unicode cSep ) const
{
	String aEntries;
	for ( USHORT n = 0; n < GetEntryList()->GetMRUCount(); n++ )
	{
		aEntries += GetEntryList()->GetEntryText( n );
		if( n < ( GetEntryList()->GetMRUCount() - 1 ) )
			aEntries += cSep;
	}
	return aEntries;
}

// =======================================================================

ImplWin::ImplWin( Window* pParent, WinBits nWinStyle ) :
	Control ( pParent, nWinStyle )
{
	if ( IsNativeControlSupported(CTRL_LISTBOX, PART_ENTIRE_CONTROL)
			&& ! IsNativeControlSupported(CTRL_LISTBOX, PART_BUTTON_DOWN) )
		SetBackground();
	else
		SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFieldColor() ) );

	mbInUserDraw = FALSE;
	mbUserDrawEnabled = FALSE;
	mnItemPos = LISTBOX_ENTRY_NOTFOUND;
}

// -----------------------------------------------------------------------

BOOL ImplWin::SetModeImage( const Image& rImage, BmpColorMode eMode )
{
    if( eMode == BMP_COLOR_NORMAL )
        SetImage( rImage );
    else if( eMode == BMP_COLOR_HIGHCONTRAST )
		maImageHC = rImage;
    else
        return FALSE;
    return TRUE;
}

// -----------------------------------------------------------------------

const Image& ImplWin::GetModeImage( BmpColorMode eMode ) const
{
    if( eMode == BMP_COLOR_HIGHCONTRAST )
        return maImageHC;
    else
        return maImage;
}

// -----------------------------------------------------------------------

void ImplWin::MBDown()
{
	if( IsEnabled() )
		maMBDownHdl.Call( this );
}

// -----------------------------------------------------------------------

void ImplWin::MouseButtonDown( const MouseEvent& rMEvt )
{
	if( IsEnabled() )
	{
//		Control::MouseButtonDown( rMEvt );
		MBDown();
	}
}

// -----------------------------------------------------------------------

void ImplWin::FillLayoutData() const
{
    mpLayoutData = new vcl::ControlLayoutData();
    const_cast<ImplWin*>(this)->ImplDraw( true );
}

// -----------------------------------------------------------------------

long ImplWin::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    const MouseEvent* pMouseEvt = NULL;

    if( (rNEvt.GetType() == EVENT_MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) )
    {
        if( pMouseEvt->IsEnterWindow() || pMouseEvt->IsLeaveWindow() )
        {
            // trigger redraw as mouse over state has changed
            if ( IsNativeControlSupported(CTRL_LISTBOX, PART_ENTIRE_CONTROL)
			&& ! IsNativeControlSupported(CTRL_LISTBOX, PART_BUTTON_DOWN) )
            {
                GetParent()->GetWindow( WINDOW_BORDER )->Invalidate( INVALIDATE_NOERASE );
                GetParent()->GetWindow( WINDOW_BORDER )->Update();
            }
        }
    }

    return nDone ? nDone : Control::PreNotify(rNEvt);
}

// -----------------------------------------------------------------------

void ImplWin::ImplDraw( bool bLayout )
{
	const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    BOOL bNativeOK = FALSE;

    if( ! bLayout )
    {
        ControlState nState = CTRL_STATE_ENABLED;
        if ( IsNativeControlSupported(CTRL_LISTBOX, PART_ENTIRE_CONTROL)
			&& IsNativeControlSupported(CTRL_LISTBOX, HAS_BACKGROUND_TEXTURE) )
        {
	        // Repaint the (focused) area similarly to
	        // ImplSmallBorderWindowView::DrawWindow() in
	        // vcl/source/window/brdwin.cxx
	        Window *pWin = GetParent();

	        ImplControlValue aControlValue;
	        if ( !pWin->IsEnabled() )
		    nState &= ~CTRL_STATE_ENABLED;
	        if ( pWin->HasFocus() )
		    nState |= CTRL_STATE_FOCUSED;
    	    
	        // The listbox is painted over the entire control including the
	        // border, but ImplWin does not contain the border => correction
	        // needed.
	        sal_Int32 nLeft, nTop, nRight, nBottom;
	        pWin->GetBorder( nLeft, nTop, nRight, nBottom );
	        Point aPoint( -nLeft, -nTop );
	        Region aCtrlRegion( Rectangle( aPoint - GetPosPixel(), pWin->GetSizePixel() ) );

            BOOL bMouseOver = FALSE;
            if( GetParent() )
            {
                Window *pChild = GetParent()->GetWindow( WINDOW_FIRSTCHILD );
                while( pChild && !(bMouseOver = pChild->IsMouseOver()) )
                    pChild = pChild->GetWindow( WINDOW_NEXT );
            }
    	    
            if( bMouseOver )
                nState |= CTRL_STATE_ROLLOVER;
            
#ifdef USE_JAVA
            ListBox *pListBox = dynamic_cast< ListBox* >( pWin );
            if ( pListBox )
            {
                const ImplBtn *pImplBtn = pListBox->GetImplBtn();
                if ( pImplBtn && pImplBtn->IsPressed() )
                    nState |= CTRL_STATE_PRESSED;
            }
#else	// USE_JAVA
            // if parent has no border, then nobody has drawn the background
            // since no border window exists. so draw it here.
            WinBits nParentStyle = pWin->GetStyle();
            if( ! (nParentStyle & WB_BORDER) || (nParentStyle & WB_NOBORDER) )
            {
                Rectangle aParentRect( Point( 0, 0 ), pWin->GetSizePixel() );
                Region aParentReg( aParentRect );
                pWin->DrawNativeControl( CTRL_LISTBOX, PART_ENTIRE_CONTROL, aParentReg,
                                         nState, aControlValue, rtl::OUString() );
            }
#endif	// USE_JAVA
            
	        bNativeOK = DrawNativeControl( CTRL_LISTBOX, PART_ENTIRE_CONTROL, aCtrlRegion, nState,
		        aControlValue, rtl::OUString() );
	    }

        if( IsEnabled() )
        {
            if( HasFocus() )
            {
                SetTextColor( rStyleSettings.GetHighlightTextColor() );
#ifndef USE_JAVA
                SetFillColor( rStyleSettings.GetHighlightColor() );
                DrawRect( maFocusRect );
#endif	// !USE_JAVA
            }
            else
            {
                Color aColor;
                if( bNativeOK && (nState & CTRL_STATE_ROLLOVER) )
                    aColor = rStyleSettings.GetFieldRolloverTextColor();
                else
                    aColor = rStyleSettings.GetFieldTextColor();
                if( IsControlForeground() )
                    aColor = GetControlForeground();
                SetTextColor( aColor );
		        if ( !bNativeOK )
		            Erase( maFocusRect );
            }
        }
        else // Disabled
        {
            SetTextColor( rStyleSettings.GetDisableColor() );
	        if ( !bNativeOK )
		        Erase( maFocusRect );
        }
    }

	if ( IsUserDrawEnabled() )
	{
		mbInUserDraw = TRUE;
		UserDrawEvent aUDEvt( this, maFocusRect, mnItemPos, 0 );
		maUserDrawHdl.Call( &aUDEvt );
		mbInUserDraw = FALSE;
	}
	else
	{
		DrawEntry( TRUE, TRUE, FALSE, bLayout );
	}
}

// -----------------------------------------------------------------------

void ImplWin::Paint( const Rectangle& rRect )
{
    ImplDraw();
}

// -----------------------------------------------------------------------

void ImplWin::DrawEntry( BOOL bDrawImage, BOOL bDrawText, BOOL bDrawTextAtImagePos, bool bLayout )
{
	long nBorder = 1;
	Size aOutSz = GetOutputSizePixel();

#ifdef USE_JAVA
	nBorder = 6;
#endif

	BOOL bImage = !!maImage;
	if( bDrawImage && bImage && !bLayout )
	{
		USHORT nStyle = 0;
		Size aImgSz = maImage.GetSizePixel();
		Point aPtImg( nBorder, ( ( aOutSz.Height() - aImgSz.Height() ) / 2 ) );

		// check for HC mode
		Image *pImage = &maImage;

		if( !!maImageHC )
		{
			// determine backgroundcolor as done in Paint()
			Color aBackCol;
			if( IsEnabled() )
			{
				if( HasFocus() )
					aBackCol = GetSettings().GetStyleSettings().GetHighlightColor();
				else
					aBackCol = GetBackground().GetColor();
			}
			else // Disabled
				aBackCol = GetBackground().GetColor();

			if( aBackCol.IsDark() )
				pImage = &maImageHC;
            // #99902 no col transform required
			//if( aBackCol.IsBright() )
			//	nStyle |= IMAGE_DRAW_COLORTRANSFORM;
		}

		if ( !IsZoom() )
		{
			DrawImage( aPtImg, *pImage, nStyle );
		}
		else
		{
			aImgSz.Width() = CalcZoom( aImgSz.Width() );
			aImgSz.Height() = CalcZoom( aImgSz.Height() );
			DrawImage( aPtImg, aImgSz, *pImage, nStyle );
		}
	}

	if( bDrawText && maString.Len() )
	{
        USHORT nTextStyle = TEXT_DRAW_VCENTER;

        if ( bDrawImage && bImage && !bLayout )
            nTextStyle |= TEXT_DRAW_LEFT;
        else if ( GetStyle() & WB_CENTER )
            nTextStyle |= TEXT_DRAW_CENTER;
        else if ( GetStyle() & WB_RIGHT )
            nTextStyle |= TEXT_DRAW_RIGHT;
        else
            nTextStyle |= TEXT_DRAW_LEFT;

        Rectangle aTextRect( Point( nBorder, 0 ), Size( aOutSz.Width()-2*nBorder, aOutSz.Height() ) );

        if ( !bDrawTextAtImagePos && ( bImage || IsUserDrawEnabled() ) )
		{
			long nMaxWidth = Max( maImage.GetSizePixel().Width(), maUserItemSize.Width() );
			aTextRect.Left() += nMaxWidth + IMG_TXT_DISTANCE;
		}

        MetricVector* pVector = bLayout ? &mpLayoutData->m_aUnicodeBoundRects : NULL;
        String* pDisplayText = bLayout ? &mpLayoutData->m_aDisplayText : NULL;
		DrawText( aTextRect, maString, nTextStyle, pVector, pDisplayText );
	}

	if( HasFocus() && !bLayout )
		ShowFocus( maFocusRect );
}

// -----------------------------------------------------------------------

void ImplWin::Resize()
{
    Control::Resize();
#if defined( USE_JAVA )
    maFocusRect.Top() = 2;
    maFocusRect.Left() = 4;
    maFocusRect.Bottom() = GetOutputSizePixel().Height() - 2;
    maFocusRect.Right() = GetOutputSizePixel().Width() - 4;
#else
	maFocusRect.SetSize( GetOutputSizePixel() );
#endif
	Invalidate();
}

// -----------------------------------------------------------------------

void ImplWin::GetFocus()
{
	ShowFocus( maFocusRect );
	Invalidate();
	Control::GetFocus();
}

// -----------------------------------------------------------------------

void ImplWin::LoseFocus()
{
	HideFocus();
	Invalidate();
	Control::LoseFocus();
}

// =======================================================================

ImplBtn::ImplBtn( Window* pParent, WinBits nWinStyle ) :
	PushButton(  pParent, nWinStyle ),
	mbDown	( FALSE )
{
}

// -----------------------------------------------------------------------

void ImplBtn::MBDown()
{
	if( IsEnabled() )
	   maMBDownHdl.Call( this );
}

// -----------------------------------------------------------------------

void ImplBtn::MouseButtonDown( const MouseEvent& rMEvt )
{
	//PushButton::MouseButtonDown( rMEvt );
	if( IsEnabled() )
	{
		MBDown();
		mbDown = TRUE;
	}
}

// =======================================================================

ImplListBoxFloatingWindow::ImplListBoxFloatingWindow( Window* pParent ) :
	FloatingWindow( pParent, WB_BORDER | WB_SYSTEMWINDOW | WB_NOSHADOW )    // no drop shadow for list boxes
{
	mpImplLB = NULL;
	mnDDLineCount = 0;
	mbAutoWidth = FALSE;

    mnPopupModeStartSaveSelection = LISTBOX_ENTRY_NOTFOUND;

	EnableSaveBackground();
}

// -----------------------------------------------------------------------

uno::Reference< ::com::sun::star::accessibility::XAccessible > ImplListBoxFloatingWindow::CreateAccessible()
{
	// Hide Accessible for this Window, because it's a top window we don't want to see as a top window.
	// Must be handled in the ListBox/ComboBox Accessibility Implementation
	return NULL;
}

// -----------------------------------------------------------------------

long ImplListBoxFloatingWindow::PreNotify( NotifyEvent& rNEvt )
{
	if( rNEvt.GetType() == EVENT_LOSEFOCUS )
	{
		if( !GetParent()->HasChildPathFocus( TRUE ) )
			EndPopupMode();
	}

	return FloatingWindow::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

void ImplListBoxFloatingWindow::SetPosSizePixel( long nX, long nY, long nWidth, long nHeight, USHORT nFlags )
{
	FloatingWindow::SetPosSizePixel( nX, nY, nWidth, nHeight, nFlags );

	// Fix #60890# ( MBA ): um auch im aufgeklappten Zustand der Listbox die Gr"o\se einfach zu einen
	// Aufruf von Resize() "andern zu k"onnen, wird die Position hier ggf. angepa\t
	if ( IsReallyVisible() && ( nFlags & WINDOW_POSSIZE_HEIGHT ) )
	{
		Point aPos = GetParent()->GetPosPixel();
		aPos = GetParent()->GetParent()->OutputToScreenPixel( aPos );

		if ( nFlags & WINDOW_POSSIZE_X )
			aPos.X() = nX;

		if ( nFlags & WINDOW_POSSIZE_Y )
			aPos.Y() = nY;

		USHORT nIndex;
		SetPosPixel( ImplCalcPos( this, Rectangle( aPos, GetParent()->GetSizePixel() ), FLOATWIN_POPUPMODE_DOWN, nIndex ) );
	}

//	if( !IsReallyVisible() )
	{
		// Die ImplListBox erhaelt kein Resize, weil nicht sichtbar.
		// Die Fenster muessen aber ein Resize() erhalten, damit die
		// Anzahl der sichtbaren Eintraege fuer PgUp/PgDown stimmt.
		// Die Anzahl kann auch nicht von List/Combobox berechnet werden,
		// weil hierfuer auch die ggf. vorhandene vertikale Scrollbar
		// beruecksichtigt werden muss.
		mpImplLB->SetSizePixel( GetOutputSizePixel() );
		((Window*)mpImplLB)->Resize();
		((Window*)mpImplLB->GetMainWindow())->Resize();
	}
}

// -----------------------------------------------------------------------

void ImplListBoxFloatingWindow::Resize()
{
    mpImplLB->GetMainWindow()->ImplClearLayoutData();
    FloatingWindow::Resize();
}

// -----------------------------------------------------------------------

Size ImplListBoxFloatingWindow::CalcFloatSize()
{
	Size aFloatSz( maPrefSz );

	sal_Int32 nLeft, nTop, nRight, nBottom;
	GetBorder( nLeft, nTop, nRight, nBottom );

	USHORT nLines = mpImplLB->GetEntryList()->GetEntryCount();
	if ( mnDDLineCount && ( nLines > mnDDLineCount ) )
		nLines = mnDDLineCount;

	Size aSz = mpImplLB->CalcSize( nLines );
	long nMaxHeight = aSz.Height() + nTop + nBottom;

	if ( mnDDLineCount )
		aFloatSz.Height() = nMaxHeight;

	if( mbAutoWidth )
	{
		// AutoSize erstmal nur fuer die Breite...

		aFloatSz.Width() = aSz.Width() + nLeft + nRight;
		aFloatSz.Width() += nRight; // etwas mehr Platz sieht besser aus...

		if ( ( aFloatSz.Height() < nMaxHeight ) || ( mnDDLineCount && ( mnDDLineCount < mpImplLB->GetEntryList()->GetEntryCount() ) ) )
		{
			// dann wird noch der vertikale Scrollbar benoetigt
			long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
			aFloatSz.Width() += nSBWidth;
		}
	}

	if ( aFloatSz.Height() > nMaxHeight )
		aFloatSz.Height() = nMaxHeight;

	// Minimale Hoehe, falls Hoehe nicht auf Float-Hoehe eingestellt wurde.
	// Der Parent vom FloatWin muss die DropDown-Combo/Listbox sein.
	Size aParentSz = GetParent()->GetSizePixel();
	if( !mnDDLineCount && ( aFloatSz.Height() < aParentSz.Height() ) )
		aFloatSz.Height() = aParentSz.Height();

	// Nicht schmaler als der Parent werden...
	if( aFloatSz.Width() < aParentSz.Width() )
		aFloatSz.Width() = aParentSz.Width();

	// Hoehe auf Entries alignen...
	long nInnerHeight = aFloatSz.Height() - nTop - nBottom;
	long nEntryHeight = mpImplLB->GetEntryHeight();
	if ( nInnerHeight % nEntryHeight )
	{
		nInnerHeight /= nEntryHeight;
		nInnerHeight++;
		nInnerHeight *= nEntryHeight;
		aFloatSz.Height() = nInnerHeight + nTop + nBottom;
	}

	return aFloatSz;
}

// -----------------------------------------------------------------------

void ImplListBoxFloatingWindow::StartFloat( BOOL bStartTracking )
{
	if( !IsInPopupMode() )
	{
		Size aFloatSz = CalcFloatSize();

		SetSizePixel( aFloatSz );
		mpImplLB->SetSizePixel( GetOutputSizePixel() );

		USHORT nPos = mpImplLB->GetEntryList()->GetSelectEntryPos( 0 );
        mnPopupModeStartSaveSelection = nPos;

        Size aSz = GetParent()->GetSizePixel();
#ifdef USE_JAVA
		// respect NWF preferred drawing bounds.  Bug #1769
		if( dynamic_cast<ComboBox *>(GetParent()) && IsNativeControlSupported( CTRL_COMBOBOX, PART_ENTIRE_CONTROL ) )
		{
			ImplControlValue aControlValue;
			Region aBoundingRgn, aContentRgn;
			Rectangle aRect( GetParent()->GetPosPixel(), GetParent()->GetSizePixel() );
			Region aArea( aRect );
			if ( GetNativeControlRegion( CTRL_COMBOBOX, PART_ENTIRE_CONTROL, aArea, 0, aControlValue, rtl::OUString(), aBoundingRgn, aContentRgn ) )
			{
				Rectangle preferredRect = aContentRgn.GetBoundRect();
				aSz = Size( preferredRect.GetWidth(), preferredRect.GetHeight() );
			}
		}
		if( dynamic_cast<ListBox *>(GetParent()) && IsNativeControlSupported( CTRL_LISTBOX, PART_ENTIRE_CONTROL ) )
		{
			ImplControlValue aControlValue;
			Region aBoundingRgn, aContentRgn;
			Rectangle aRect( GetParent()->GetPosPixel(), GetParent()->GetSizePixel() );
			Region aArea( aRect );
			if ( GetNativeControlRegion( CTRL_LISTBOX, PART_ENTIRE_CONTROL, aArea, 0, aControlValue, rtl::OUString(), aBoundingRgn, aContentRgn ) )
			{
				Rectangle preferredRect = aContentRgn.GetBoundRect();
				aSz = Size( preferredRect.GetWidth(), preferredRect.GetHeight() );
			}
		}
#endif
		Point aPos = GetParent()->GetPosPixel();
		aPos = GetParent()->GetParent()->OutputToScreenPixel( aPos );
		Rectangle aRect( aPos, aSz );

        // check if the control's parent is un-mirrored which is the case for form controls in a mirrored UI
        // where the document is unmirrored
        // because StartPopupMode() expects a rectangle in mirrored coordinates we have to re-mirror
        if( GetParent()->GetParent()->ImplHasMirroredGraphics() && !GetParent()->GetParent()->IsRTLEnabled() )
            GetParent()->GetParent()->ImplReMirror( aRect );

		StartPopupMode( aRect, FLOATWIN_POPUPMODE_DOWN );

        if( nPos != LISTBOX_ENTRY_NOTFOUND )
			mpImplLB->SetTopEntry( nPos );

		if( bStartTracking )
			mpImplLB->GetMainWindow()->EnableMouseMoveSelect( TRUE );

		if ( mpImplLB->GetMainWindow()->IsGrabFocusAllowed() )
			mpImplLB->GetMainWindow()->GrabFocus();

        mpImplLB->GetMainWindow()->ImplClearLayoutData();
	}
}
