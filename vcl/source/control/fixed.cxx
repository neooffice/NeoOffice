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

#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <fixed.hxx>
#endif
#ifndef _VCL_CONTROLLAYOUT_HXX
#include <controllayout.hxx>
#endif

#include <tools/rc.h>



// =======================================================================

#define FIXEDLINE_TEXT_BORDER	 4

#define FIXEDTEXT_VIEW_STYLE	(WB_3DLOOK |						\
								 WB_LEFT | WB_CENTER | WB_RIGHT |	\
								 WB_TOP | WB_VCENTER | WB_BOTTOM |	\
								 WB_WORDBREAK | WB_NOLABEL |		\
								 WB_INFO | WB_PATHELLIPSIS)
#define FIXEDLINE_VIEW_STYLE	(WB_3DLOOK | WB_NOLABEL)
#define FIXEDBITMAP_VIEW_STYLE	(WB_3DLOOK |						\
								 WB_LEFT | WB_CENTER | WB_RIGHT |	\
								 WB_TOP | WB_VCENTER | WB_BOTTOM |	\
								 WB_SCALE)
#define FIXEDIMAGE_VIEW_STYLE	(WB_3DLOOK |						\
								 WB_LEFT | WB_CENTER | WB_RIGHT |	\
								 WB_TOP | WB_VCENTER | WB_BOTTOM |	\
								 WB_SCALE)

// =======================================================================

static Point ImplCalcPos( WinBits nStyle, const Point& rPos,
						  const Size& rObjSize, const Size& rWinSize )
{
	long	nX;
	long	nY;

	if ( nStyle & WB_LEFT )
		nX = 0;
	else if ( nStyle & WB_RIGHT )
		nX = rWinSize.Width()-rObjSize.Width();
	else
		nX = (rWinSize.Width()-rObjSize.Width())/2;

	if ( nStyle & WB_TOP )
		nY = 0;
	else if ( nStyle & WB_BOTTOM )
		nY = rWinSize.Height()-rObjSize.Height();
	else
		nY = (rWinSize.Height()-rObjSize.Height())/2;

	if ( nStyle & WB_TOPLEFTVISIBLE )
	{
		if ( nX < 0 )
			nX = 0;
		if ( nY < 0 )
			nY = 0;
	}

	Point aPos( nX+rPos.X(), nY+rPos.Y() );
	return aPos;
}

// =======================================================================

void FixedText::ImplInit( Window* pParent, WinBits nStyle )
{
	nStyle = ImplInitStyle( nStyle );
	Control::ImplInit( pParent, nStyle, NULL );
	ImplInitSettings( TRUE, TRUE, TRUE );
}

// -----------------------------------------------------------------------

WinBits FixedText::ImplInitStyle( WinBits nStyle )
{
	if ( !(nStyle & WB_NOGROUP) )
		nStyle |= WB_GROUP;
	return nStyle;
}

// -----------------------------------------------------------------------

void FixedText::ImplInitSettings( BOOL bFont,
								  BOOL bForeground, BOOL bBackground )
{
	const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

	if ( bFont )
	{
		Font aFont;
		if ( GetStyle() & WB_INFO )
			aFont = rStyleSettings.GetInfoFont();
		else
			aFont = rStyleSettings.GetLabelFont();
		if ( IsControlFont() )
			aFont.Merge( GetControlFont() );
		SetZoomedPointFont( aFont );
	}

	if ( bForeground || bFont )
	{
		Color aColor;
		if ( IsControlForeground() )
			aColor = GetControlForeground();
		else
		{
			if ( GetStyle() & WB_INFO )
				aColor = rStyleSettings.GetInfoTextColor();
			else
				aColor = rStyleSettings.GetLabelTextColor();
		}
		SetTextColor( aColor );
		SetTextFillColor();
	}

	if ( bBackground )
	{
		Window* pParent = GetParent();
		if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
		{
			EnableChildTransparentMode( TRUE );
			SetParentClipMode( PARENTCLIPMODE_NOCLIP );
			SetPaintTransparent( TRUE );
			SetBackground();
		}
		else
		{
			EnableChildTransparentMode( FALSE );
			SetParentClipMode( 0 );
			SetPaintTransparent( FALSE );

			if ( IsControlBackground() )
				SetBackground( GetControlBackground() );
			else
				SetBackground( pParent->GetBackground() );
		}
	}
#ifdef USE_JAVA
	if ( IsNativeControlSupported( CTRL_FIXEDBORDER, PART_ENTIRE_CONTROL ) && GetParent() )
	{
		if ( GetParent()->GetType() == WINDOW_FIXEDBORDER )
		{
			EnableChildTransparentMode( TRUE );
			GetParent()->Invalidate();
		}
		else
		{
			// dialogs will implement tabpages as peers of their borders, not
			// as contained
			for ( USHORT i = 0; i < GetParent()->GetChildCount(); i++ )
			{
				Window* pChild = GetParent()->GetChild( i );
				if ( pChild->GetType() == WINDOW_FIXEDBORDER )
				{
					EnableChildTransparentMode( TRUE );
					GetParent()->Invalidate();
					SetBackground();
					SetControlBackground();
					break;
				}
			}
		}
	}
#endif	// USE_JAVA
}

// -----------------------------------------------------------------------

FixedText::FixedText( Window* pParent, WinBits nStyle ) :
	Control( WINDOW_FIXEDTEXT )
{
	ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

FixedText::FixedText( Window* pParent, const ResId& rResId ) :
	Control( WINDOW_FIXEDTEXT )
{
	rResId.SetRT( RSC_TEXT );
	WinBits nStyle = ImplInitRes( rResId );
	ImplInit( pParent, nStyle );
	ImplLoadRes( rResId );

	if ( !(nStyle & WB_HIDE) )
		Show();
}

// -----------------------------------------------------------------------

USHORT FixedText::ImplGetTextStyle( WinBits nWinStyle )
{
	USHORT nTextStyle = TEXT_DRAW_MNEMONIC | TEXT_DRAW_ENDELLIPSIS;

    if( ! (nWinStyle & WB_NOMULTILINE) )
        nTextStyle |= TEXT_DRAW_MULTILINE;

	if ( nWinStyle & WB_RIGHT )
		nTextStyle |= TEXT_DRAW_RIGHT;
	else if ( nWinStyle & WB_CENTER )
		nTextStyle |= TEXT_DRAW_CENTER;
	else
		nTextStyle |= TEXT_DRAW_LEFT;
	if ( nWinStyle & WB_BOTTOM )
		nTextStyle |= TEXT_DRAW_BOTTOM;
	else if ( nWinStyle & WB_VCENTER )
		nTextStyle |= TEXT_DRAW_VCENTER;
	else
		nTextStyle |= TEXT_DRAW_TOP;
	if ( nWinStyle & WB_WORDBREAK )
		nTextStyle |= TEXT_DRAW_WORDBREAK;
	if ( nWinStyle & WB_NOLABEL )
		nTextStyle &= ~TEXT_DRAW_MNEMONIC;

	return nTextStyle;
}

// -----------------------------------------------------------------------

void FixedText::ImplDraw( OutputDevice* pDev, ULONG nDrawFlags,
						  const Point& rPos, const Size& rSize,
                          bool bFillLayout
                          ) const
{
	const StyleSettings&	rStyleSettings = GetSettings().GetStyleSettings();
	WinBits 				nWinStyle = GetStyle();
	XubString				aText( GetText() );
	USHORT					nTextStyle = FixedText::ImplGetTextStyle( nWinStyle );
    Point                   aPos = rPos;

    if ( nWinStyle & WB_EXTRAOFFSET )
        aPos.X() += 2;
    
	if ( nWinStyle & WB_PATHELLIPSIS )
	{
		nTextStyle &= ~(TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK);
		nTextStyle |= TEXT_DRAW_PATHELLIPSIS;
	}
	if ( nDrawFlags & WINDOW_DRAW_NOMNEMONIC )
	{
		if ( nTextStyle & TEXT_DRAW_MNEMONIC )
		{
			aText = GetNonMnemonicString( aText );
			nTextStyle &= ~TEXT_DRAW_MNEMONIC;
		}
	}
	if ( !(nDrawFlags & WINDOW_DRAW_NODISABLE) )
	{
		if ( !IsEnabled() )
			nTextStyle |= TEXT_DRAW_DISABLE;
	}
	if ( (nDrawFlags & WINDOW_DRAW_MONO) ||
		 (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
		nTextStyle |= TEXT_DRAW_MONO;

    if( bFillLayout )
    {
        mpLayoutData->m_aDisplayText = String();
        pDev->DrawText( Rectangle( aPos, rSize ),
                        aText,
                        nTextStyle,
                        &mpLayoutData->m_aUnicodeBoundRects,
                        &mpLayoutData->m_aDisplayText
                        );
    }
    else
        pDev->DrawText( Rectangle( aPos, rSize ), aText, nTextStyle );
}

// -----------------------------------------------------------------------

void FixedText::Paint( const Rectangle& rRect )
{
	ImplDraw( this, 0, Point(), GetOutputSizePixel() );
}

// -----------------------------------------------------------------------

void FixedText::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
					  ULONG nFlags )
{
	ImplInitSettings( TRUE, TRUE, TRUE );

	Point		aPos  = pDev->LogicToPixel( rPos );
	Size		aSize = pDev->LogicToPixel( rSize );
	Font		aFont = GetDrawPixelFont( pDev );

	pDev->Push();
	pDev->SetMapMode();
	pDev->SetFont( aFont );
	if ( nFlags & WINDOW_DRAW_MONO )
		pDev->SetTextColor( Color( COL_BLACK ) );
	else
		pDev->SetTextColor( GetTextColor() );
	pDev->SetTextFillColor();

	BOOL bBorder = !(nFlags & WINDOW_DRAW_NOBORDER ) && (GetStyle() & WB_BORDER);
	BOOL bBackground = !(nFlags & WINDOW_DRAW_NOBACKGROUND) && IsControlBackground();
	if ( bBorder || bBackground )
	{
		Rectangle aRect( aPos, aSize );
		if ( bBorder )
		{
            ImplDrawFrame( pDev, aRect );
		}
		if ( bBackground )
		{
			pDev->SetFillColor( GetControlBackground() );
			pDev->DrawRect( aRect );
		}
	}

	ImplDraw( pDev, nFlags, aPos, aSize );
	pDev->Pop();
}

// -----------------------------------------------------------------------

void FixedText::Resize()
{
    Control::Resize();
	Invalidate();
}

// -----------------------------------------------------------------------

void FixedText::StateChanged( StateChangedType nType )
{
	Control::StateChanged( nType );

	if ( (nType == STATE_CHANGE_ENABLE) ||
		 (nType == STATE_CHANGE_TEXT) ||
		 (nType == STATE_CHANGE_UPDATEMODE) )
	{
		if ( IsReallyVisible() && IsUpdateMode() )
#ifdef USE_JAVA
			if ( IsChildTransparentModeEnabled() || ( GetBackground().GetStyle() == WALLPAPER_NULL ) )
				GetParent()->Invalidate();
			else
#endif	// USE_JAVA
			Invalidate();
	}
	else if ( nType == STATE_CHANGE_STYLE )
	{
		SetStyle( ImplInitStyle( GetStyle() ) );
		if ( (GetPrevStyle() & FIXEDTEXT_VIEW_STYLE) !=
			 (GetStyle() & FIXEDTEXT_VIEW_STYLE) )
		{
			ImplInitSettings( TRUE, FALSE, FALSE );
			Invalidate();
		}
	}
	else if ( (nType == STATE_CHANGE_ZOOM)	||
			  (nType == STATE_CHANGE_CONTROLFONT) )
	{
		ImplInitSettings( TRUE, FALSE, FALSE );
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
#ifdef USE_JAVA
	else if ( ( nType == WINDOW_FIRSTOVERLAP ) && ( IsChildTransparentModeEnabled() || ( GetBackground().GetStyle() == WALLPAPER_NULL ) ) )
	{
		GetParent()->Invalidate();
	}
#endif	// USE_JAVA
}

// -----------------------------------------------------------------------

void FixedText::DataChanged( const DataChangedEvent& rDCEvt )
{
	Control::DataChanged( rDCEvt );

	if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
		 (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
		 ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
		  (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
	{
		ImplInitSettings( TRUE, TRUE, TRUE );
		Invalidate();
	}
}

// -----------------------------------------------------------------------

Size FixedText::CalcMinimumSize( long nMaxWidth ) const
{
	USHORT nStyle = ImplGetTextStyle( GetStyle() );
	if ( !( GetStyle() & WB_NOLABEL ) )
		nStyle |= TEXT_DRAW_MNEMONIC;

	Size aSize = GetTextRect( Rectangle( Point(), Size( (nMaxWidth ? nMaxWidth : 0x7fffffff), 0x7fffffff ) ),
							  GetText(), nStyle ).GetSize();

    if ( GetStyle() & WB_EXTRAOFFSET )
        aSize.Width() += 2;
    
	// GetTextRect verkraftet keinen leeren String:
	if ( aSize.Width() < 0 )
		aSize.Width() = 0;
	if ( aSize.Height() <= 0 )
		aSize.Height() = GetTextHeight();

	return CalcWindowSize( aSize );
}

// -----------------------------------------------------------------------

void  FixedText::FillLayoutData() const
{
    mpLayoutData = new vcl::ControlLayoutData();
	ImplDraw( const_cast<FixedText*>(this), 0, Point(), GetOutputSizePixel(), true );
}


// =======================================================================

void FixedLine::ImplInit( Window* pParent, WinBits nStyle )
{
	nStyle = ImplInitStyle( nStyle );
	Control::ImplInit( pParent, nStyle, NULL );
	ImplInitSettings( TRUE, TRUE, TRUE );
}

// -----------------------------------------------------------------------

WinBits FixedLine::ImplInitStyle( WinBits nStyle )
{
	if ( !(nStyle & WB_NOGROUP) )
		nStyle |= WB_GROUP;
	return nStyle;
}

// -----------------------------------------------------------------------

void FixedLine::ImplInitSettings( BOOL bFont,
								  BOOL bForeground, BOOL bBackground )
{
	const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

	if ( bFont )
	{
		Font aFont = rStyleSettings.GetGroupFont();
		if ( IsControlFont() )
			aFont.Merge( GetControlFont() );
		SetZoomedPointFont( aFont );
	}

	if ( bForeground || bFont )
	{
		Color aColor;
		if ( IsControlForeground() )
			aColor = GetControlForeground();
		else
			aColor = rStyleSettings.GetGroupTextColor();
		SetTextColor( aColor );
		SetTextFillColor();
	}

	if ( bBackground )
	{
		Window* pParent = GetParent();
		if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
		{
			EnableChildTransparentMode( TRUE );
			SetParentClipMode( PARENTCLIPMODE_NOCLIP );
			SetPaintTransparent( TRUE );
			SetBackground();
		}
		else
		{
			EnableChildTransparentMode( FALSE );
			SetParentClipMode( 0 );
			SetPaintTransparent( FALSE );

			if ( IsControlBackground() )
				SetBackground( GetControlBackground() );
			else
				SetBackground( pParent->GetBackground() );
		}
	}
}

// -----------------------------------------------------------------------

void FixedLine::ImplDraw( bool bLayout )
{
	Size					aOutSize = GetOutputSizePixel();
	String					aText = GetText();
	const StyleSettings&	rStyleSettings = GetSettings().GetStyleSettings();
	WinBits 				nWinStyle = GetStyle();
    MetricVector*			pVector = bLayout ? &mpLayoutData->m_aUnicodeBoundRects : NULL;
    String*					pDisplayText = bLayout ? &mpLayoutData->m_aDisplayText : NULL;
#ifdef USE_JAVA
	BOOL					bNativeOK = FALSE;
#endif	// USE_JAVA

	if ( rStyleSettings.GetOptions() & STYLE_OPTION_MONO )
		SetLineColor( Color( COL_BLACK ) );
	else
		SetLineColor( rStyleSettings.GetShadowColor() );

	if ( !aText.Len() || (nWinStyle & WB_VERT) )
	{
        if( !pVector )
        {
#ifdef USE_JAVA
			if ( IsNativeControlSupported( CTRL_FIXEDLINE, PART_ENTIRE_CONTROL ) )
			{
		        const ImplControlValue aControlValue( BUTTONVALUE_DONTKNOW, rtl::OUString(), 0 );

				ControlState nState = CTRL_STATE_ENABLED;
				int part = PART_ENTIRE_CONTROL;
				if ( !IsEnabled() )
					nState &= ~CTRL_STATE_ENABLED;
				if ( HasFocus() )
					nState |= CTRL_STATE_FOCUSED;
				
				Point aPoint;
				Size aSize;
				
				if ( nWinStyle & WB_VERT )
				{
					aPoint = Point( (aOutSize.Width()-1)/2, 0 );
					aSize = Size( 1, aOutSize.Height()-1 );
				}
				else
				{
					aPoint = Point( 0, (aOutSize.Height()-1)/2 );
					aSize = Size( aOutSize.Width()-1, 1 );
				}
				
				Region aCtrlRegion( Rectangle( aPoint, aSize ) );
				bNativeOK = DrawNativeControl( CTRL_FIXEDLINE, PART_ENTIRE_CONTROL, aCtrlRegion, nState, aControlValue, rtl::OUString() );
			}
			
			if ( ! bNativeOK )
			{
#endif	// USE_JAVA
            long nX = 0;
            long nY = 0;
            
            if ( nWinStyle & WB_VERT )
            {
                nX = (aOutSize.Width()-1)/2;
                DrawLine( Point( nX, 0 ), Point( nX, aOutSize.Height()-1 ) );
            }
            else
            {
                nY = (aOutSize.Height()-1)/2;
                DrawLine( Point( 0, nY ), Point( aOutSize.Width()-1, nY ) );
            }
            
            if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
            {
                SetLineColor( rStyleSettings.GetLightColor() );
                if ( nWinStyle & WB_VERT )
                    DrawLine( Point( nX+1, 0 ), Point( nX+1, aOutSize.Height()-1 ) );
                else
                    DrawLine( Point( 0, nY+1 ), Point( aOutSize.Width()-1, nY+1 ) );
            }
#ifdef USE_JAVA
			}
#endif	// USE_JAVA
        }
	}
	else
	{
		USHORT		nStyle = TEXT_DRAW_MNEMONIC | TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER | TEXT_DRAW_ENDELLIPSIS;
		Rectangle	aRect( 0, 0, aOutSize.Width(), aOutSize.Height() );

		if ( !IsEnabled() )
			nStyle |= TEXT_DRAW_DISABLE;
		if ( GetStyle() & WB_NOLABEL )
			nStyle &= ~TEXT_DRAW_MNEMONIC;
		if ( rStyleSettings.GetOptions() & STYLE_OPTION_MONO )
			nStyle |= TEXT_DRAW_MONO;

		aRect = GetTextRect( aRect, aText, nStyle );
		DrawText( aRect, aText, nStyle, pVector, pDisplayText );

        if( !pVector )
        {
            long nTop = aRect.Top() + ((aRect.GetHeight()-1)/2);
#ifdef USE_JAVA
			if ( IsNativeControlSupported( CTRL_FIXEDLINE, PART_ENTIRE_CONTROL ) )
			{
		        const ImplControlValue aControlValue( BUTTONVALUE_DONTKNOW, rtl::OUString(), 0 );

				ControlState nState = CTRL_STATE_ENABLED;
				int part = PART_ENTIRE_CONTROL;
				if ( !IsEnabled() )
					nState &= ~CTRL_STATE_ENABLED;
				if ( HasFocus() )
					nState |= CTRL_STATE_FOCUSED;
				
				Point aPoint( aRect.Right()+FIXEDLINE_TEXT_BORDER, nTop );
				Size aSize ( (aOutSize.Width()-1)-(aRect.Right()+FIXEDLINE_TEXT_BORDER), 1 ) ;
				
				Region aCtrlRegion( Rectangle( aPoint, aSize ) );
				bNativeOK = DrawNativeControl( CTRL_FIXEDLINE, PART_ENTIRE_CONTROL, aCtrlRegion, nState, aControlValue, rtl::OUString() );
			}
			
			if ( ! bNativeOK )
			{
#endif	// USE_JAVA
            DrawLine( Point( aRect.Right()+FIXEDLINE_TEXT_BORDER, nTop ), Point( aOutSize.Width()-1, nTop ) );
            if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
            {
                SetLineColor( rStyleSettings.GetLightColor() );
                DrawLine( Point( aRect.Right()+FIXEDLINE_TEXT_BORDER, nTop+1 ), Point( aOutSize.Width()-1, nTop+1 ) );
            }
#ifdef USE_JAVA
			}
#endif	// USE_JAVA
        }
    }
}

// -----------------------------------------------------------------------

FixedLine::FixedLine( Window* pParent, WinBits nStyle ) :
	Control( WINDOW_FIXEDLINE )
{
	ImplInit( pParent, nStyle );
	SetSizePixel( Size( 2, 2 ) );
}

// -----------------------------------------------------------------------

FixedLine::FixedLine( Window* pParent, const ResId& rResId ) :
	Control( WINDOW_FIXEDLINE )
{
	rResId.SetRT( RSC_FIXEDLINE );
	WinBits nStyle = ImplInitRes( rResId );
	ImplInit( pParent, nStyle );
	ImplLoadRes( rResId );

	if ( !(nStyle & WB_HIDE) )
		Show();
}

// -----------------------------------------------------------------------

void  FixedLine::FillLayoutData() const
{
    mpLayoutData = new vcl::ControlLayoutData();
	const_cast<FixedLine*>(this)->ImplDraw( true );
}


// -----------------------------------------------------------------------

void FixedLine::Paint( const Rectangle& rRect )
{
    ImplDraw();
}

// -----------------------------------------------------------------------

void FixedLine::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
					  ULONG nFlags )
{
}

// -----------------------------------------------------------------------

void FixedLine::Resize()
{
    Control::Resize();
	Invalidate();
}

// -----------------------------------------------------------------------

void FixedLine::StateChanged( StateChangedType nType )
{
	Control::StateChanged( nType );

	if ( (nType == STATE_CHANGE_ENABLE) ||
		 (nType == STATE_CHANGE_TEXT) ||
		 (nType == STATE_CHANGE_UPDATEMODE) )
	{
		if ( IsReallyVisible() && IsUpdateMode() )
			Invalidate();
	}
	else if ( nType == STATE_CHANGE_STYLE )
	{
		SetStyle( ImplInitStyle( GetStyle() ) );
		if ( (GetPrevStyle() & FIXEDLINE_VIEW_STYLE) !=
			 (GetStyle() & FIXEDLINE_VIEW_STYLE) )
			Invalidate();
	}
	else if ( (nType == STATE_CHANGE_ZOOM)	||
			  (nType == STATE_CHANGE_STYLE) ||
			  (nType == STATE_CHANGE_CONTROLFONT) )
	{
		ImplInitSettings( TRUE, FALSE, FALSE );
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
}

// -----------------------------------------------------------------------

void FixedLine::DataChanged( const DataChangedEvent& rDCEvt )
{
	Control::DataChanged( rDCEvt );

	if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
		 (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
		 ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
		  (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
	{
		ImplInitSettings( TRUE, TRUE, TRUE );
		Invalidate();
	}
}

// =======================================================================

void FixedBitmap::ImplInit( Window* pParent, WinBits nStyle )
{
	nStyle = ImplInitStyle( nStyle );
	Control::ImplInit( pParent, nStyle, NULL );
	ImplInitSettings();
}

// -----------------------------------------------------------------------

WinBits FixedBitmap::ImplInitStyle( WinBits nStyle )
{
	if ( !(nStyle & WB_NOGROUP) )
		nStyle |= WB_GROUP;
	return nStyle;
}

// -----------------------------------------------------------------------

void FixedBitmap::ImplInitSettings()
{
	Window* pParent = GetParent();
	if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
	{
		EnableChildTransparentMode( TRUE );
		SetParentClipMode( PARENTCLIPMODE_NOCLIP );
		SetPaintTransparent( TRUE );
		SetBackground();
	}
	else
	{
		EnableChildTransparentMode( FALSE );
		SetParentClipMode( 0 );
		SetPaintTransparent( FALSE );

		if ( IsControlBackground() )
			SetBackground( GetControlBackground() );
		else
			SetBackground( pParent->GetBackground() );
	}
}

// -----------------------------------------------------------------------

void FixedBitmap::ImplLoadRes( const ResId& rResId )
{
	Control::ImplLoadRes( rResId );

	ULONG nObjMask = ReadLongRes();

	if ( RSC_FIXEDBITMAP_BITMAP & nObjMask )
	{
		maBitmap = Bitmap( ResId( (RSHEADER_TYPE*)GetClassRes() ) );
		IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
	}
}

// -----------------------------------------------------------------------

FixedBitmap::FixedBitmap( Window* pParent, WinBits nStyle ) :
	Control( WINDOW_FIXEDBITMAP )
{
	ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

FixedBitmap::FixedBitmap( Window* pParent, const ResId& rResId ) :
	Control( WINDOW_FIXEDBITMAP )
{
	rResId.SetRT( RSC_FIXEDBITMAP );
	WinBits nStyle = ImplInitRes( rResId );
	ImplInit( pParent, nStyle );
	ImplLoadRes( rResId );

	if ( !(nStyle & WB_HIDE) )
		Show();
}

// -----------------------------------------------------------------------

FixedBitmap::~FixedBitmap()
{
}

// -----------------------------------------------------------------------

void FixedBitmap::ImplDraw( OutputDevice* pDev, ULONG /* nDrawFlags */,
							const Point& rPos, const Size& rSize )
{
	USHORT nStyle = 0;
	Bitmap* pBitmap = &maBitmap;
	Color aCol;
	if( !!maBitmapHC && ImplGetCurrentBackgroundColor( aCol ) )
	{
		if( aCol.IsDark() )
			pBitmap = &maBitmapHC;
        // #99902 no col transform required
		//if( aCol.IsBright() )
		//	nStyle |= IMAGE_DRAW_COLORTRANSFORM;
	}

	if( nStyle & IMAGE_DRAW_COLORTRANSFORM )
	{
		// only images support IMAGE_DRAW_COLORTRANSFORM
		Image aImage( maBitmap );
		if ( !(!aImage) )
		{
			if ( GetStyle() & WB_SCALE )
				pDev->DrawImage( rPos, rSize, aImage, nStyle );
			else
			{
				Point aPos = ImplCalcPos( GetStyle(), rPos, aImage.GetSizePixel(), rSize );
				pDev->DrawImage( aPos, aImage, nStyle );
			}
		}
	}
	else
	{
		// Haben wir ueberhaupt eine Bitmap
		if ( !(!(*pBitmap)) )
		{
			if ( GetStyle() & WB_SCALE )
				pDev->DrawBitmap( rPos, rSize, *pBitmap );
			else
			{
				Point aPos = ImplCalcPos( GetStyle(), rPos, pBitmap->GetSizePixel(), rSize );
				pDev->DrawBitmap( aPos, *pBitmap );
			}
		}
	}
}

// -----------------------------------------------------------------------

void FixedBitmap::Paint( const Rectangle& rRect )
{
	ImplDraw( this, 0, Point(), GetOutputSizePixel() );
}

// -----------------------------------------------------------------------

void FixedBitmap::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
						ULONG nFlags )
{
	Point		aPos  = pDev->LogicToPixel( rPos );
	Size		aSize = pDev->LogicToPixel( rSize );
	Rectangle	aRect( aPos, aSize );

	pDev->Push();
	pDev->SetMapMode();

	// Border
	if ( !(nFlags & WINDOW_DRAW_NOBORDER) && (GetStyle() & WB_BORDER) )
	{
		DecorationView aDecoView( pDev );
		aRect = aDecoView.DrawFrame( aRect, FRAME_DRAW_DOUBLEIN );
	}
	pDev->IntersectClipRegion( aRect );
	ImplDraw( pDev, nFlags, aRect.TopLeft(), aRect.GetSize() );

	pDev->Pop();
}

// -----------------------------------------------------------------------

void FixedBitmap::Resize()
{
    Control::Resize();
	Invalidate();
}

// -----------------------------------------------------------------------

void FixedBitmap::StateChanged( StateChangedType nType )
{
	Control::StateChanged( nType );

	if ( (nType == STATE_CHANGE_DATA) ||
		 (nType == STATE_CHANGE_UPDATEMODE) )
	{
		if ( IsReallyVisible() && IsUpdateMode() )
			Invalidate();
	}
	else if ( nType == STATE_CHANGE_STYLE )
	{
		SetStyle( ImplInitStyle( GetStyle() ) );
		if ( (GetPrevStyle() & FIXEDBITMAP_VIEW_STYLE) !=
			 (GetStyle() & FIXEDBITMAP_VIEW_STYLE) )
			Invalidate();
	}
	else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
	{
		ImplInitSettings();
		Invalidate();
	}
}

// -----------------------------------------------------------------------

void FixedBitmap::DataChanged( const DataChangedEvent& rDCEvt )
{
	Control::DataChanged( rDCEvt );

	if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
		 (rDCEvt.GetFlags() & SETTINGS_STYLE) )
	{
		ImplInitSettings();
		Invalidate();
	}
}

// -----------------------------------------------------------------------

void FixedBitmap::SetBitmap( const Bitmap& rBitmap )
{
	maBitmap = rBitmap;
	StateChanged( STATE_CHANGE_DATA );
}

// -----------------------------------------------------------------------

BOOL FixedBitmap::SetModeBitmap( const Bitmap& rBitmap, BmpColorMode eMode )
{
    if( eMode == BMP_COLOR_NORMAL )
        SetBitmap( rBitmap );
    else if( eMode == BMP_COLOR_HIGHCONTRAST )
    {
	    maBitmapHC = rBitmap;
	    StateChanged( STATE_CHANGE_DATA );
    }
    else
        return FALSE;
    return TRUE;
}

// -----------------------------------------------------------------------

const Bitmap& FixedBitmap::GetModeBitmap( BmpColorMode eMode) const
{
    if( eMode == BMP_COLOR_HIGHCONTRAST )
        return maBitmapHC;
    else
        return maBitmap;
}

// =======================================================================

void FixedImage::ImplInit( Window* pParent, WinBits nStyle )
{
	nStyle = ImplInitStyle( nStyle );
	mbInUserDraw = FALSE;
	Control::ImplInit( pParent, nStyle, NULL );
	ImplInitSettings();
}

// -----------------------------------------------------------------------

WinBits FixedImage::ImplInitStyle( WinBits nStyle )
{
	if ( !(nStyle & WB_NOGROUP) )
		nStyle |= WB_GROUP;
	return nStyle;
}

// -----------------------------------------------------------------------

void FixedImage::ImplInitSettings()
{
	Window* pParent = GetParent();
	if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
	{
		EnableChildTransparentMode( TRUE );
		SetParentClipMode( PARENTCLIPMODE_NOCLIP );
		SetPaintTransparent( TRUE );
		SetBackground();
	}
	else
	{
		EnableChildTransparentMode( FALSE );
		SetParentClipMode( 0 );
		SetPaintTransparent( FALSE );

		if ( IsControlBackground() )
			SetBackground( GetControlBackground() );
		else
			SetBackground( pParent->GetBackground() );
	}
#ifdef USE_JAVA
	if ( IsNativeControlSupported( CTRL_FIXEDBORDER, PART_ENTIRE_CONTROL ) && GetParent() )
	{
		if ( GetParent()->GetType() == WINDOW_FIXEDBORDER )
		{
			EnableChildTransparentMode( TRUE );
		}
		else
		{
			// dialogs will implement tabpages as peers of their borders, not
			// as contained
			for ( USHORT i = 0; i < GetParent()->GetChildCount(); i++ )
			{
				Window* pChild = GetParent()->GetChild( i );
				if ( pChild->GetType() == WINDOW_FIXEDBORDER )
				{
					EnableChildTransparentMode( TRUE );
					SetBackground();
					SetControlBackground();
					break;
				}
			}
		}
	}
#endif	// USE_JAVA
}

// -----------------------------------------------------------------------

void FixedImage::ImplLoadRes( const ResId& rResId )
{
	Control::ImplLoadRes( rResId );

	ULONG nObjMask = ReadLongRes();

	if ( RSC_FIXEDIMAGE_IMAGE & nObjMask )
	{
		maImage = Image( ResId( (RSHEADER_TYPE*)GetClassRes() ) );
		IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
	}
}

// -----------------------------------------------------------------------

FixedImage::FixedImage( Window* pParent, WinBits nStyle ) :
	Control( WINDOW_FIXEDIMAGE )
{
	ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

FixedImage::FixedImage( Window* pParent, const ResId& rResId ) :
	Control( WINDOW_FIXEDIMAGE )
{
	rResId.SetRT( RSC_FIXEDIMAGE );
	WinBits nStyle = ImplInitRes( rResId );
	ImplInit( pParent, nStyle );
	ImplLoadRes( rResId );

	if ( !(nStyle & WB_HIDE) )
		Show();
}

// -----------------------------------------------------------------------

FixedImage::~FixedImage()
{
}

// -----------------------------------------------------------------------

void FixedImage::ImplDraw( OutputDevice* pDev, ULONG nDrawFlags,
						   const Point& rPos, const Size& rSize )
{
	USHORT nStyle = 0;
	if ( !(nDrawFlags & WINDOW_DRAW_NODISABLE) )
	{
		if ( !IsEnabled() )
			nStyle |= IMAGE_DRAW_DISABLE;
	}

	Image *pImage = &maImage;
	Color aCol;
	if( !!maImageHC && ImplGetCurrentBackgroundColor( aCol ) )
	{
		if( aCol.IsDark() )
			pImage = &maImageHC;
        // #99902 no col transform required
		//if( aCol.IsBright() )
		//	nStyle |= IMAGE_DRAW_COLORTRANSFORM;
	}

	// Haben wir ueberhaupt ein Image
	if ( !(!(*pImage)) )
	{
		if ( GetStyle() & WB_SCALE )
			pDev->DrawImage( rPos, rSize, *pImage, nStyle );
		else
		{
			Point aPos = ImplCalcPos( GetStyle(), rPos, pImage->GetSizePixel(), rSize );
			pDev->DrawImage( aPos, *pImage, nStyle );
		}
	}

	mbInUserDraw = TRUE;
	UserDrawEvent aUDEvt( pDev, Rectangle( rPos, rSize ), 0, nStyle );
	UserDraw( aUDEvt );
	mbInUserDraw = FALSE;
}

// -----------------------------------------------------------------------

void FixedImage::Paint( const Rectangle& rRect )
{
	ImplDraw( this, 0, Point(), GetOutputSizePixel() );
}

// -----------------------------------------------------------------------

void FixedImage::UserDraw( const UserDrawEvent& )
{
}

// -----------------------------------------------------------------------

void FixedImage::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
					   ULONG nFlags )
{
	Point		aPos  = pDev->LogicToPixel( rPos );
	Size		aSize = pDev->LogicToPixel( rSize );
	Rectangle	aRect( aPos, aSize );

	pDev->Push();
	pDev->SetMapMode();

	// Border
	if ( !(nFlags & WINDOW_DRAW_NOBORDER) && (GetStyle() & WB_BORDER) )
	{
		ImplDrawFrame( pDev, aRect );
	}
	pDev->IntersectClipRegion( aRect );
	ImplDraw( pDev, nFlags, aRect.TopLeft(), aRect.GetSize() );

	pDev->Pop();
}

// -----------------------------------------------------------------------

void FixedImage::Resize()
{
    Control::Resize();
	Invalidate();
}

// -----------------------------------------------------------------------

void FixedImage::StateChanged( StateChangedType nType )
{
	Control::StateChanged( nType );

	if ( (nType == STATE_CHANGE_ENABLE) ||
		 (nType == STATE_CHANGE_DATA) ||
		 (nType == STATE_CHANGE_UPDATEMODE) )
	{
		if ( IsReallyVisible() && IsUpdateMode() )
#ifdef USE_JAVA
			if ( IsChildTransparentModeEnabled() || ( GetBackground().GetStyle() == WALLPAPER_NULL ) )
				GetParent()->Invalidate( Rectangle( GetPosPixel(), GetSizePixel() ) );
			else
#endif	// USE_JAVA
			Invalidate();
	}
	else if ( nType == STATE_CHANGE_STYLE )
	{
		SetStyle( ImplInitStyle( GetStyle() ) );
		if ( (GetPrevStyle() & FIXEDIMAGE_VIEW_STYLE) !=
			 (GetStyle() & FIXEDIMAGE_VIEW_STYLE) )
			Invalidate();
	}
	else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
	{
		ImplInitSettings();
		Invalidate();
	}
#ifdef USE_JAVA
	else if ( ( nType == WINDOW_FIRSTOVERLAP ) && ( IsChildTransparentModeEnabled() || ( GetBackground().GetStyle() == WALLPAPER_NULL ) ) )
	{
		Invalidate();
	}
#endif	// USE_JAVA
}

// -----------------------------------------------------------------------

void FixedImage::DataChanged( const DataChangedEvent& rDCEvt )
{
	Control::DataChanged( rDCEvt );

	if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
		 (rDCEvt.GetFlags() & SETTINGS_STYLE) )
	{
		ImplInitSettings();
		Invalidate();
	}
}

// -----------------------------------------------------------------------

void FixedImage::SetImage( const Image& rImage )
{
	if ( rImage != maImage )
	{
		maImage = rImage;
		StateChanged( STATE_CHANGE_DATA );
	}
}

// -----------------------------------------------------------------------

BOOL FixedImage::SetModeImage( const Image& rImage, BmpColorMode eMode )
{
    if( eMode == BMP_COLOR_NORMAL )
        SetImage( rImage );
    else if( eMode == BMP_COLOR_HIGHCONTRAST )
    {
		if( maImageHC != rImage )
		{
			maImageHC = rImage;
			StateChanged( STATE_CHANGE_DATA );
		}
    }
    else
        return FALSE;
    return TRUE;
}

// -----------------------------------------------------------------------

const Image& FixedImage::GetModeImage( BmpColorMode eMode ) const
{
    if( eMode == BMP_COLOR_HIGHCONTRAST )
        return maImageHC;
    else
        return maImage;
}

// -----------------------------------------------------------------------

Point FixedImage::CalcImagePos( const Point& rPos,
								const Size& rObjSize, const Size& rWinSize )
{
	return ImplCalcPos( GetStyle(), rPos, rObjSize, rWinSize );
}
