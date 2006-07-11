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
 *    Modified May 2006 by Edward Peterlin. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

#define _SV_PRGSBAR_CXX

#ifndef _TOOLS_DEBUGS_HXX
#include <tools/debug.hxx>
#endif
#ifndef _VCL_STATUS_HXX
#include <vcl/status.hxx>
#endif
#ifndef _PRGSBAR_HXX
#include <prgsbar.hxx>
#endif
#ifdef USE_JAVA
#ifndef _SV_NATIVEWIDGETS_HXX
#include <vcl/salnativewidgets.hxx>
#endif
#endif

// =======================================================================

#define PROGRESSBAR_OFFSET			3
#define PROGRESSBAR_WIN_OFFSET		2

// =======================================================================

void ProgressBar::ImplInit()
{
	mnPercent	= 0;
	mbCalcNew	= TRUE;

	ImplInitSettings( TRUE, TRUE, TRUE );
}

// -----------------------------------------------------------------------

ProgressBar::ProgressBar( Window* pParent, WinBits nWinStyle ) :
	Window( pParent, nWinStyle )
{
	SetOutputSizePixel( Size( 150, 20 ) );
	ImplInit();
}

// -----------------------------------------------------------------------

ProgressBar::ProgressBar( Window* pParent, const ResId& rResId ) :
	Window( pParent, rResId )
{
	ImplInit();
}

// -----------------------------------------------------------------------

ProgressBar::~ProgressBar()
{
}

// -----------------------------------------------------------------------

void ProgressBar::ImplInitSettings( BOOL bFont,
									BOOL bForeground, BOOL bBackground )
{
	const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

/* !!! Derzeit unterstuetzen wir keine Textausgaben
	if ( bFont )
	{
		Font aFont;
		aFont = rStyleSettings.GetAppFont();
		if ( IsControlFont() )
			aFont.Merge( GetControlFont() );
		SetZoomedPointFont( aFont );
	}
*/

	if ( bBackground )
	{
		Color aColor;
		if ( IsControlBackground() )
			aColor = GetControlBackground();
		else
			aColor = rStyleSettings.GetFaceColor();
		SetBackground( aColor );
	}

	if ( bForeground || bFont )
	{
		Color aColor = rStyleSettings.GetHighlightColor();
		if ( IsControlForeground() )
			aColor = GetControlForeground();
		if ( aColor.IsRGBEqual( GetBackground().GetColor() ) )
		{
			if ( aColor.GetLuminance() > 100 )
				aColor.DecreaseLuminance( 64 );
			else
				aColor.IncreaseLuminance( 64 );
		}
		SetLineColor();
		SetFillColor( aColor );
/* !!! Derzeit unterstuetzen wir keine Textausgaben
		SetTextColor( aColor );
		SetTextFillColor();
*/
	}
}

// -----------------------------------------------------------------------

void ProgressBar::ImplDrawProgress( USHORT nOldPerc, USHORT nNewPerc )
{
#ifdef USE_JAVA
	if ( IsNativeControlSupported( CTRL_PROGRESSBAR, PART_ENTIRE_CONTROL ) )
	{
		ProgressbarValue aProgressbarValue;
		aProgressbarValue.mdPercentComplete = (double)nNewPerc;
		
		ImplControlValue aControlValue;
		aControlValue.setOptionalVal( &aProgressbarValue );
		
		ControlState	 nState = 0;
		if( Window::IsEnabled() )
			nState |= CTRL_STATE_ENABLED;
			
		Rectangle aCtrlRect( Point( 0, 0 ), GetOutputSizePixel() );
		Region aCtrlRegion( aCtrlRect );
		BOOL bOK = DrawNativeControl( CTRL_PROGRESSBAR, PART_ENTIRE_CONTROL, aCtrlRegion, nState, aControlValue, rtl::OUString() );
		if ( bOK )
			return;
	}
#endif

	if ( mbCalcNew )
	{
		mbCalcNew = FALSE;

		Size aSize = GetOutputSizePixel();
		mnPrgsHeight = aSize.Height()-(PROGRESSBAR_WIN_OFFSET*2);
		mnPrgsWidth = (mnPrgsHeight*2)/3;
		maPos.Y() = PROGRESSBAR_WIN_OFFSET;
		long nMaxWidth = (aSize.Width()-(PROGRESSBAR_WIN_OFFSET*2)+PROGRESSBAR_OFFSET);
		USHORT nMaxCount = (USHORT)(nMaxWidth / (mnPrgsWidth+PROGRESSBAR_OFFSET));
		if ( nMaxCount <= 1 )
			nMaxCount = 1;
		else
		{
			while ( ((10000/(10000/nMaxCount))*(mnPrgsWidth+PROGRESSBAR_OFFSET)) > nMaxWidth )
				nMaxCount--;
		}
		mnPercentCount = 10000/nMaxCount;
		nMaxWidth = ((10000/(10000/nMaxCount))*(mnPrgsWidth+PROGRESSBAR_OFFSET))-PROGRESSBAR_OFFSET;
		maPos.X() = (aSize.Width()-nMaxWidth)/2;
	}

	::DrawProgress( this, maPos, PROGRESSBAR_OFFSET, mnPrgsWidth, mnPrgsHeight,
					nOldPerc*100, nNewPerc*100, mnPercentCount );
}

// -----------------------------------------------------------------------

void ProgressBar::Paint( const Rectangle& )
{
	ImplDrawProgress( 0, mnPercent );
}

// -----------------------------------------------------------------------

void ProgressBar::Resize()
{
	mbCalcNew = TRUE;
	if ( IsReallyVisible() )
		Invalidate();
}

// -----------------------------------------------------------------------

void ProgressBar::SetValue( USHORT nNewPercent )
{
	DBG_ASSERTWARNING( nNewPercent <= 100, "StatusBar::SetProgressValue(): nPercent > 100" );

	if ( nNewPercent < mnPercent )
	{
		mbCalcNew = TRUE;
		mnPercent = nNewPercent;
		if ( IsReallyVisible() )
		{
			Invalidate();
			Update();
		}
	}
	else
	{
		ImplDrawProgress( mnPercent, nNewPercent );
		mnPercent = nNewPercent;
	}
}

// -----------------------------------------------------------------------

void ProgressBar::StateChanged( StateChangedType nType )
{
/* !!! Derzeit unterstuetzen wir keine Textausgaben
	if ( (nType == STATE_CHANGE_ZOOM) ||
		 (nType == STATE_CHANGE_CONTROLFONT) )
	{
		ImplInitSettings( TRUE, FALSE, FALSE );
		Invalidate();
	}
	else
*/
	if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
	{
		ImplInitSettings( FALSE, TRUE, FALSE );
		Invalidate();
	}
	else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
	{
		ImplInitSettings( FALSE, FALSE, TRUE );
		Invalidate();
	}

	Window::StateChanged( nType );
}

// -----------------------------------------------------------------------

void ProgressBar::DataChanged( const DataChangedEvent& rDCEvt )
{
	if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
		 (rDCEvt.GetFlags() & SETTINGS_STYLE) )
	{
		ImplInitSettings( TRUE, TRUE, TRUE );
		Invalidate();
	}

	Window::DataChanged( rDCEvt );
}

