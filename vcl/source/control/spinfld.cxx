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

#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/toolkit/spinfld.hxx>
#include <vcl/settings.hxx>
#include <vcl/uitest/uiobject.hxx>
#include <sal/log.hxx>

#include <spin.hxx>
#include <svdata.hxx>

namespace {

void ImplGetSpinbuttonValue(vcl::Window* pWin,
                            const tools::Rectangle& rUpperRect, const tools::Rectangle& rLowerRect,
                            bool bUpperIn, bool bLowerIn, bool bUpperEnabled, bool bLowerEnabled,
                            bool bHorz, SpinbuttonValue& rValue )
{
    // convert spinbutton data to a SpinbuttonValue structure for native painting

    rValue.maUpperRect = rUpperRect;
    rValue.maLowerRect = rLowerRect;

    Point aPointerPos = pWin->GetPointerPosPixel();

    ControlState nState = ControlState::ENABLED;
    if (bUpperIn)
        nState |= ControlState::PRESSED;
    if (!pWin->IsEnabled() || !bUpperEnabled)
        nState &= ~ControlState::ENABLED;
    if (pWin->HasFocus())
        nState |= ControlState::FOCUSED;
    if (pWin->IsMouseOver() && rUpperRect.Contains(aPointerPos))
        nState |= ControlState::ROLLOVER;
    rValue.mnUpperState = nState;

    nState = ControlState::ENABLED;
    if (bLowerIn)
        nState |= ControlState::PRESSED;
    if (!pWin->IsEnabled() || !bLowerEnabled)
        nState &= ~ControlState::ENABLED;
    if (pWin->HasFocus())
        nState |= ControlState::FOCUSED;
    // for overlapping spins: highlight only one
    if (pWin->IsMouseOver() && rLowerRect.Contains(aPointerPos) && !rUpperRect.Contains(aPointerPos))
        nState |= ControlState::ROLLOVER;
    rValue.mnLowerState = nState;

    rValue.mnUpperPart = bHorz ? ControlPart::ButtonLeft : ControlPart::ButtonUp;
    rValue.mnLowerPart = bHorz ? ControlPart::ButtonRight : ControlPart::ButtonDown;
}

#ifdef USE_JAVA
bool ImplDrawNativeSpinfield(vcl::RenderContext& rRenderContext, vcl::Window const * pWin, const SpinbuttonValue& rSpinbuttonValue, bool bInDropDown = false)
#else	// USE_JAVA
bool ImplDrawNativeSpinfield(vcl::RenderContext& rRenderContext, vcl::Window const * pWin, const SpinbuttonValue& rSpinbuttonValue)
#endif	// USE_JAVA
{
    bool bNativeOK = false;

    if (rRenderContext.IsNativeControlSupported(ControlType::Spinbox, ControlPart::Entire) &&
        // there is just no useful native support for spinfields with dropdown
        !(pWin->GetStyle() & WB_DROPDOWN))
    {
        if (rRenderContext.IsNativeControlSupported(ControlType::Spinbox, rSpinbuttonValue.mnUpperPart) &&
            rRenderContext.IsNativeControlSupported(ControlType::Spinbox, rSpinbuttonValue.mnLowerPart))
        {
            // only paint the embedded spin buttons, all buttons are painted at once
            tools::Rectangle aUpperAndLowerButtons( rSpinbuttonValue.maUpperRect.GetUnion( rSpinbuttonValue.maLowerRect ) );
            bNativeOK = rRenderContext.DrawNativeControl(ControlType::Spinbox, ControlPart::AllButtons, aUpperAndLowerButtons,
#ifdef USE_JAVA
                                                         pWin->IsEnabled() ? ControlState::ENABLED : 0, rSpinbuttonValue, OUString());
#else	// USE_JAVA
                                                         ControlState::ENABLED, rSpinbuttonValue, OUString());
#endif	// USE_JAVA
        }
        else
        {
            // paint the spinbox as a whole, use borderwindow to have proper clipping
            vcl::Window* pBorder = pWin->GetWindow(GetWindowType::Border);

            // to not overwrite everything, set the button region as clipregion to the border window
            tools::Rectangle aClipRect(rSpinbuttonValue.maLowerRect);
            aClipRect.Union(rSpinbuttonValue.maUpperRect);

            vcl::RenderContext* pContext = &rRenderContext;
            vcl::Region oldRgn;
            Point aPt;
            Size aSize(pBorder->GetOutputSizePixel());    // the size of the border window, i.e., the whole control
            tools::Rectangle aNatRgn(aPt, aSize);

            if (!pWin->SupportsDoubleBuffering())
            {
                // convert from screen space to borderwin space
                aClipRect.SetPos(pBorder->ScreenToOutputPixel(pWin->OutputToScreenPixel(aClipRect.TopLeft())));

                oldRgn = pBorder->GetOutDev()->GetClipRegion();
                pBorder->GetOutDev()->SetClipRegion(vcl::Region(aClipRect));

                pContext = pBorder->GetOutDev();
            }

            tools::Rectangle aBound, aContent;
            if (!ImplGetSVData()->maNWFData.mbCanDrawWidgetAnySize &&
                pContext->GetNativeControlRegion(ControlType::Spinbox, ControlPart::Entire,
                                                aNatRgn, ControlState::NONE, rSpinbuttonValue,
                                                aBound, aContent))
            {
                aSize = aContent.GetSize();
            }

            tools::Rectangle aRgn(aPt, aSize);
            if (pWin->SupportsDoubleBuffering())
            {
                // convert from borderwin space, to the pWin's space
                aRgn.SetPos(pWin->ScreenToOutputPixel(pBorder->OutputToScreenPixel(aRgn.TopLeft())));
            }

            bNativeOK = pContext->DrawNativeControl(ControlType::Spinbox, ControlPart::Entire, aRgn,
#ifdef USE_JAVA
                                                   pWin->IsEnabled() ? ControlState::ENABLED : 0, rSpinbuttonValue, OUString());
#else	// USE_JAVA
                                                   ControlState::ENABLED, rSpinbuttonValue, OUString());
#endif	// USE_JAVA

            if (!pWin->SupportsDoubleBuffering())
                pBorder->GetOutDev()->SetClipRegion(oldRgn);
        }
    }
#ifdef USE_JAVA
    // render spinfields with dropdowns as combo boxes
    else if( (pWin->GetStyle() & WB_DROPDOWN) && pWin->IsNativeControlSupported(ControlType::Combobox, ControlPart::Entire) )
    {
        vcl::Window *pBorder = pWin->GetWindow( GetWindowType::Border );
        Point aPt;
        Size aSize( pBorder->GetOutputSizePixel() );
        Rectangle aRgn( aPt, aSize );
        ImplControlValue aControlValue;
        ControlState nState = 0;
        if( pWin->IsEnabled() )
            nState |= ControlState::ENABLED;
        if( bInDropDown )
            nState |= ControlState::PRESSED;
        bNativeOK = pBorder->DrawNativeControl( ControlType::Combobox, ControlPart::Entire, aRgn, nState, aControlValue, OUString() );
    }
#endif	// USE_JAVA
    return bNativeOK;
}

bool ImplDrawNativeSpinbuttons(vcl::RenderContext& rRenderContext, const SpinbuttonValue& rSpinbuttonValue)
{
    bool bNativeOK = false;

    if (rRenderContext.IsNativeControlSupported(ControlType::SpinButtons, ControlPart::Entire))
    {
#ifdef USE_JAVA
        Point aPt;
        Size aSize( pWin->GetOutputSizePixel() );
        Rectangle aRgn( aPt, aSize );
        bNativeOK = pWin->DrawNativeControl( ControlType::SpinButtons, ControlPart::AllButtons, aRgn, ControlState::ENABLED,
        bNativeOK = rRenderContext.DrawNativeControl(ControlType::SpinButtons, ControlPart::AllButtons, aRgn,
#else	// USE_JAVA
        tools::Rectangle aArea = rSpinbuttonValue.maUpperRect.GetUnion(rSpinbuttonValue.maLowerRect);
        // only paint the standalone spin buttons, all buttons are painted at once
        bNativeOK = rRenderContext.DrawNativeControl(ControlType::SpinButtons, ControlPart::AllButtons, aArea,
#endif	// USE_JAVA
                                                     ControlState::ENABLED, rSpinbuttonValue, OUString());
    }
    return bNativeOK;
}

}

#ifdef USE_JAVA

static void ImplDrawSpinButton(vcl::RenderContext& rRenderContext, vcl::Window* pWindow,
                        const tools::Rectangle& rUpperRect, const tools::Rectangle& rLowerRect,
                        bool bUpperIn, bool bLowerIn, bool bUpperEnabled, bool bLowerEnabled,
                        bool bHorz, bool bMirrorHorz, bool bInDropDown = false);

#endif	// USE_JAVA

void ImplDrawSpinButton(vcl::RenderContext& rRenderContext, vcl::Window* pWindow,
                        const tools::Rectangle& rUpperRect, const tools::Rectangle& rLowerRect,
                        bool bUpperIn, bool bLowerIn, bool bUpperEnabled, bool bLowerEnabled,
                        bool bHorz, bool bMirrorHorz)
#ifdef USE_JAVA
{
    ImplDrawSpinButton(rRenderContext, pWindow, rUpperRect, rLowerRect, bUpperIn, bLowerIn, bUpperEnabled, bLowerEnabled, bHorz, bMirrorHorz, false);
}

static void ImplDrawSpinButton(vcl::RenderContext& rRenderContext, vcl::Window* pWindow,
                        const tools::Rectangle& rUpperRect, const tools::Rectangle& rLowerRect,
                        bool bUpperIn, bool bLowerIn, bool bUpperEnabled, bool bLowerEnabled,
                        bool bHorz, bool bMirrorHorz, bool bInDropDown)
#endif	// USE_JAVA
{
    bool bNativeOK = false;

    if (pWindow)
    {
        // are we drawing standalone spin buttons or members of a spinfield ?
        ControlType aControl = ControlType::SpinButtons;
        switch (pWindow->GetType())
        {
            case WindowType::EDIT:
            case WindowType::MULTILINEEDIT:
            case WindowType::PATTERNFIELD:
            case WindowType::METRICFIELD:
            case WindowType::CURRENCYFIELD:
            case WindowType::DATEFIELD:
            case WindowType::TIMEFIELD:
            case WindowType::SPINFIELD:
            case WindowType::FORMATTEDFIELD:
                aControl = ControlType::Spinbox;
                break;
            default:
                aControl = ControlType::SpinButtons;
                break;
        }

        SpinbuttonValue aValue;
        ImplGetSpinbuttonValue(pWindow, rUpperRect, rLowerRect,
                               bUpperIn, bLowerIn, bUpperEnabled, bLowerEnabled,
                               bHorz, aValue);

        if( aControl == ControlType::Spinbox )
#ifdef USE_JAVA
            bNativeOK = ImplDrawNativeSpinfield(rRenderContext, pWindow, aValue, bInDropDown);
#else	// USE_JAVA
            bNativeOK = ImplDrawNativeSpinfield(rRenderContext, pWindow, aValue);
#endif	// USE_JAVA
        else if( aControl == ControlType::SpinButtons )
            bNativeOK = ImplDrawNativeSpinbuttons(rRenderContext, aValue);
    }

    if (bNativeOK)
        return;

    ImplDrawUpDownButtons(rRenderContext,
                          rUpperRect, rLowerRect,
                          bUpperIn, bLowerIn, bUpperEnabled, bLowerEnabled,
                          bHorz, bMirrorHorz);
}

void ImplDrawUpDownButtons(vcl::RenderContext& rRenderContext,
                           const tools::Rectangle& rUpperRect, const tools::Rectangle& rLowerRect,
                           bool bUpperIn, bool bLowerIn, bool bUpperEnabled, bool bLowerEnabled,
                           bool bHorz, bool bMirrorHorz)
{
    DecorationView aDecoView(&rRenderContext);

    SymbolType eType1, eType2;

    if ( bHorz )
    {
        eType1 = bMirrorHorz ? SymbolType::SPIN_RIGHT : SymbolType::SPIN_LEFT;
        eType2 = bMirrorHorz ? SymbolType::SPIN_LEFT : SymbolType::SPIN_RIGHT;
    }
    else
    {
        eType1 = SymbolType::SPIN_UP;
        eType2 = SymbolType::SPIN_DOWN;
    }

    DrawButtonFlags nStyle = DrawButtonFlags::NoLeftLightBorder;
    // draw upper/left Button
    if (bUpperIn)
        nStyle |= DrawButtonFlags::Pressed;

    tools::Rectangle aUpRect = aDecoView.DrawButton(rUpperRect, nStyle);

    nStyle = DrawButtonFlags::NoLeftLightBorder;
    // draw lower/right Button
    if (bLowerIn)
        nStyle |= DrawButtonFlags::Pressed;

    tools::Rectangle aLowRect = aDecoView.DrawButton(rLowerRect, nStyle);

     // make use of additional default edge
    aUpRect.AdjustLeft( -1 );
    aUpRect.AdjustTop( -1 );
    aUpRect.AdjustRight( 1 );
    aUpRect.AdjustBottom( 1 );
    aLowRect.AdjustLeft( -1 );
    aLowRect.AdjustTop( -1 );
    aLowRect.AdjustRight( 1 );
    aLowRect.AdjustBottom( 1 );

    // draw into the edge, so that something is visible if the rectangle is too small
    if (aUpRect.GetHeight() < 4)
    {
        aUpRect.AdjustRight( 1 );
        aUpRect.AdjustBottom( 1 );
        aLowRect.AdjustRight( 1 );
        aLowRect.AdjustBottom( 1 );
    }

    // calculate Symbol size
    tools::Long nTempSize1 = aUpRect.GetWidth();
    tools::Long nTempSize2 = aLowRect.GetWidth();
    if (std::abs( nTempSize1-nTempSize2 ) == 1)
    {
        if (nTempSize1 > nTempSize2)
            aUpRect.AdjustLeft( 1 );
        else
            aLowRect.AdjustLeft( 1 );
    }
    nTempSize1 = aUpRect.GetHeight();
    nTempSize2 = aLowRect.GetHeight();
    if (std::abs(nTempSize1 - nTempSize2) == 1)
    {
        if (nTempSize1 > nTempSize2)
            aUpRect.AdjustTop( 1 );
        else
            aLowRect.AdjustTop( 1 );
    }

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    DrawSymbolFlags nSymStyle = DrawSymbolFlags::NONE;
    if (!bUpperEnabled)
        nSymStyle |= DrawSymbolFlags::Disable;
    aDecoView.DrawSymbol(aUpRect, eType1, rStyleSettings.GetButtonTextColor(), nSymStyle);

    nSymStyle = DrawSymbolFlags::NONE;
    if (!bLowerEnabled)
        nSymStyle |= DrawSymbolFlags::Disable;
    aDecoView.DrawSymbol(aLowRect, eType2, rStyleSettings.GetButtonTextColor(), nSymStyle);
}

void SpinField::ImplInitSpinFieldData()
{
    mpEdit.disposeAndClear();
    mbSpin          = false;
    mbRepeat        = false;
    mbUpperIn       = false;
    mbLowerIn       = false;
    mbInitialUp     = false;
    mbInitialDown   = false;
    mbInDropDown    = false;
    mbUpperEnabled  = true;
    mbLowerEnabled  = true;
}

void SpinField::ImplInit(vcl::Window* pParent, WinBits nWinStyle)
{
    Edit::ImplInit( pParent, nWinStyle );

    if (!(nWinStyle & (WB_SPIN | WB_DROPDOWN)))
        return;

    mbSpin = true;

    // Some themes want external spin buttons, therefore the main
    // spinfield should not overdraw the border between its encapsulated
    // edit field and the spin buttons
#ifdef USE_JAVA
    if (ImplUseNativeBorder(nWinStyle)) 
#else	// USE_JAVA
    if ((nWinStyle & WB_SPIN) && ImplUseNativeBorder(*GetOutDev(), nWinStyle))
#endif	// USE_JAVA
    {
        SetBackground();
        mpEdit.set(VclPtr<Edit>::Create(this, WB_NOBORDER));
        mpEdit->SetBackground();
    }
    else
        mpEdit.set(VclPtr<Edit>::Create(this, WB_NOBORDER));

    mpEdit->EnableRTL(false);
    mpEdit->SetPosPixel(Point());
    mpEdit->Show();

    SetSubEdit(mpEdit);

    maRepeatTimer.SetInvokeHandler(LINK( this, SpinField, ImplTimeout));
    maRepeatTimer.SetTimeout(MouseSettings::GetButtonStartRepeat());
    if (nWinStyle & WB_REPEAT)
        mbRepeat = true;

    SetCompoundControl(true);
}

SpinField::SpinField(vcl::Window* pParent, WinBits nWinStyle, WindowType nType) :
    Edit(nType), maRepeatTimer("SpinField maRepeatTimer")
{
    ImplInitSpinFieldData();
    ImplInit(pParent, nWinStyle);
}

SpinField::~SpinField()
{
    disposeOnce();
}

void SpinField::dispose()
{
    mpEdit.disposeAndClear();

    Edit::dispose();
}

void SpinField::Up()
{
    ImplCallEventListenersAndHandler( VclEventId::SpinfieldUp, [this] () { maUpHdlLink.Call(*this); } );
#ifdef USE_JAVA
    if ( IsNativeControlSupported( ControlType::Spinbox, ControlPart::Entire ) )
        ImplInvalidateOutermostBorder( this );
#endif	// USE_JAVA
}

void SpinField::Down()
{
    ImplCallEventListenersAndHandler( VclEventId::SpinfieldDown, [this] () { maDownHdlLink.Call(*this); } );
#ifdef USE_JAVA
    if ( IsNativeControlSupported( ControlType::Spinbox, ControlPart::Entire ) )
        ImplInvalidateOutermostBorder( this );
#endif	// USE_JAVA
}

void SpinField::First()
{
    ImplCallEventListenersAndHandler(VclEventId::SpinfieldFirst, nullptr);
#ifdef USE_JAVA
    if ( IsNativeControlSupported( ControlType::Spinbox, ControlPart::Entire ) )
        ImplInvalidateOutermostBorder( this );
#endif	// USE_JAVA
}

void SpinField::Last()
{
    ImplCallEventListenersAndHandler(VclEventId::SpinfieldLast, nullptr);
#ifdef USE_JAVA
    if ( IsNativeControlSupported( ControlType::Spinbox, ControlPart::Entire ) )
        ImplInvalidateOutermostBorder( this );
#endif	// USE_JAVA
}

void SpinField::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (!HasFocus() && (!mpEdit || !mpEdit->HasFocus()))
    {
        GrabFocus();
    }

    if (!IsReadOnly())
    {
        if (maUpperRect.Contains(rMEvt.GetPosPixel()))
        {
            mbUpperIn   = true;
            mbInitialUp = true;
#ifdef USE_JAVA
            if ( IsNativeControlSupported( ControlType::Spinbox, ControlPart::Entire ) )
                ImplInvalidateOutermostBorder( this );
            else
#endif	// USE_JAVA
            Invalidate(maUpperRect);
        }
        else if (maLowerRect.Contains(rMEvt.GetPosPixel()))
        {
            mbLowerIn    = true;
            mbInitialDown = true;
#ifdef USE_JAVA
            if ( IsNativeControlSupported( ControlType::Spinbox, ControlPart::Entire ) )
                ImplInvalidateOutermostBorder( this );
            else
#endif	// USE_JAVA
            Invalidate(maLowerRect);
        }
        else if (maDropDownRect.Contains(rMEvt.GetPosPixel()))
        {
            // put DropDownButton to the right
            mbInDropDown = ShowDropDown( !mbInDropDown );
#ifdef USE_JAVA
            if ( IsNativeControlSupported( ControlType::Combobox, ControlPart::Entire ) )
                GetParent()->Invalidate( Rectangle( GetPosPixel(), GetSizePixel() ) );
            else
#endif	// USE_JAVA
            Invalidate(tools::Rectangle(Point(), GetOutputSizePixel()));
        }

        if (mbUpperIn || mbLowerIn)
        {
            CaptureMouse();
            if (mbRepeat)
                maRepeatTimer.Start();
            return;
        }
    }

    Edit::MouseButtonDown(rMEvt);
}

void SpinField::MouseButtonUp(const MouseEvent& rMEvt)
{
    ReleaseMouse();
    mbInitialUp = mbInitialDown = false;
    maRepeatTimer.Stop();
    maRepeatTimer.SetTimeout(MouseSettings::GetButtonStartRepeat());

    if (mbUpperIn)
    {
        mbUpperIn = false;
#ifdef USE_JAVA
        if ( IsNativeControlSupported( ControlType::Spinbox, ControlPart::Entire ) )
            ImplInvalidateOutermostBorder( this );
        else
#endif	// USE_JAVA
        Invalidate(maUpperRect);
        Up();
    }
    else if (mbLowerIn)
    {
        mbLowerIn = false;
#ifdef USE_JAVA
        if ( IsNativeControlSupported( ControlType::Spinbox, ControlPart::Entire ) )
            ImplInvalidateOutermostBorder( this );
        else
#endif	// USE_JAVA
        Invalidate(maLowerRect);
        Down();
    }

    Edit::MouseButtonUp(rMEvt);
}

void SpinField::MouseMove(const MouseEvent& rMEvt)
{
    if (rMEvt.IsLeft())
    {
        if (mbInitialUp)
        {
            bool bNewUpperIn = maUpperRect.Contains(rMEvt.GetPosPixel());
            if (bNewUpperIn != mbUpperIn)
            {
                if (bNewUpperIn)
                {
                    if (mbRepeat)
                        maRepeatTimer.Start();
                }
                else
                    maRepeatTimer.Stop();

                mbUpperIn = bNewUpperIn;
#ifdef USE_JAVA
                if ( IsNativeControlSupported( ControlType::Spinbox, ControlPart::Entire ) )
                    ImplInvalidateOutermostBorder( this );
                else
#endif	// USE_JAVA
                Invalidate(maUpperRect);
            }
        }
        else if (mbInitialDown)
        {
            bool bNewLowerIn = maLowerRect.Contains(rMEvt.GetPosPixel());
            if (bNewLowerIn != mbLowerIn)
            {
                if (bNewLowerIn)
                {
                    if (mbRepeat)
                        maRepeatTimer.Start();
                }
                else
                    maRepeatTimer.Stop();

                mbLowerIn = bNewLowerIn;
#ifdef USE_JAVA
                if ( IsNativeControlSupported( ControlType::Spinbox, ControlPart::Entire ) )
                    ImplInvalidateOutermostBorder( this );
                else
#endif	// USE_JAVA
                Invalidate(maLowerRect);
            }
        }
    }

    Edit::MouseMove(rMEvt);
}

bool SpinField::EventNotify(NotifyEvent& rNEvt)
{
    bool bDone = false;
    if (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT)
    {
        const KeyEvent& rKEvt = *rNEvt.GetKeyEvent();
        if (!IsReadOnly())
        {
            sal_uInt16 nMod = rKEvt.GetKeyCode().GetModifier();
            switch (rKEvt.GetKeyCode().GetCode())
            {
                case KEY_UP:
                {
                    if (!nMod)
                    {
                        Up();
                        bDone = true;
                    }
                }
                break;
                case KEY_DOWN:
                {
                    if (!nMod)
                    {
                        Down();
                        bDone = true;
                    }
                    else if ((nMod == KEY_MOD2) && !mbInDropDown && (GetStyle() & WB_DROPDOWN))
                    {
                        mbInDropDown = ShowDropDown(true);
#ifdef USE_JAVA
                        if ( IsNativeControlSupported( ControlType::Spinbox, ControlPart::Entire ) )
                            ImplInvalidateOutermostBorder( this );
                        else
#endif	// USE_JAVA
                        Invalidate(tools::Rectangle(Point(), GetOutputSizePixel()));
                        bDone = true;
                    }
                }
                break;
                case KEY_PAGEUP:
                {
                    if (!nMod)
                    {
                        Last();
                        bDone = true;
                    }
                }
                break;
                case KEY_PAGEDOWN:
                {
                    if (!nMod)
                    {
                        First();
                        bDone = true;
                    }
                }
                break;
            }
        }
    }

    if (rNEvt.GetType() == MouseNotifyEvent::COMMAND)
    {
        if ((rNEvt.GetCommandEvent()->GetCommand() == CommandEventId::Wheel) && !IsReadOnly())
        {
            MouseWheelBehaviour nWheelBehavior(GetSettings().GetMouseSettings().GetWheelBehavior());
            if (nWheelBehavior == MouseWheelBehaviour::ALWAYS
               || (nWheelBehavior == MouseWheelBehaviour::FocusOnly && HasChildPathFocus()))
            {
                const CommandWheelData* pData = rNEvt.GetCommandEvent()->GetWheelData();
                if (pData->GetMode() == CommandWheelMode::SCROLL)
                {
                    if (pData->GetDelta() < 0)
                        Down();
                    else
                        Up();
                    bDone = true;

                    if (!HasChildPathFocus())
                        GrabFocus();
                }
            }
            else
                bDone = false;  // don't eat this event, let the default handling happen (i.e. scroll the context)
        }
    }

    return bDone || Edit::EventNotify(rNEvt);
}

void SpinField::FillLayoutData() const
{
    if (mbSpin)
    {
        mxLayoutData.emplace();
        AppendLayoutData(*GetSubEdit());
        GetSubEdit()->SetLayoutDataParent(this);
    }
    else
        Edit::FillLayoutData();
}

void SpinField::SetUpperEnabled(bool bEnabled)
{
    if (mbUpperEnabled == bEnabled)
        return;

    mbUpperEnabled = bEnabled;

    if (mbSpin)
        Invalidate(maUpperRect);
}

void SpinField::SetLowerEnabled(bool bEnabled)
{
    if (mbLowerEnabled == bEnabled)
        return;

    mbLowerEnabled = bEnabled;

    if (mbSpin)
        Invalidate(maLowerRect);
}

void SpinField::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if (mbSpin)
    {
        bool bEnabled = IsEnabled();
        bool bUpperEnabled = bEnabled && IsUpperEnabled();
        bool bLowerEnabled = bEnabled && IsLowerEnabled();
        ImplDrawSpinButton(rRenderContext, this, maUpperRect, maLowerRect,
                           mbUpperIn && bUpperEnabled, mbLowerIn && bLowerEnabled,
                           bUpperEnabled, bLowerEnabled);
    }

    if (GetStyle() & WB_DROPDOWN)
    {
#ifdef USE_JAVA
        if( IsNativeControlSupported( ControlType::Combobox, ControlPart::Entire ) )
        {
            bool bEnable = IsEnabled();
            ImplDrawSpinButton(rRenderContext, this, maUpperRect, maLowerRect, mbUpperIn, mbLowerIn, bEnable, bEnable, false, false, mbInDropDown);
        }
        else
        {
#endif	// USE_JAVA
        DecorationView aView(&rRenderContext);

        DrawButtonFlags nStyle = DrawButtonFlags::NoLightBorder;
        if (mbInDropDown)
            nStyle |= DrawButtonFlags::Pressed;
        tools::Rectangle aInnerRect = aView.DrawButton(maDropDownRect, nStyle);

        DrawSymbolFlags nSymbolStyle = IsEnabled() ? DrawSymbolFlags::NONE : DrawSymbolFlags::Disable;
        aView.DrawSymbol(aInnerRect, SymbolType::SPIN_DOWN, rRenderContext.GetSettings().GetStyleSettings().GetButtonTextColor(), nSymbolStyle);
#ifdef USE_JAVA
        }
#endif	// USE_JAVA
    }

    Edit::Paint(rRenderContext, rRect);
}

void SpinField::ImplCalcButtonAreas(const OutputDevice* pDev, const Size& rOutSz, tools::Rectangle& rDDArea,
                                    tools::Rectangle& rSpinUpArea, tools::Rectangle& rSpinDownArea)
{
    const StyleSettings& rStyleSettings = pDev->GetSettings().GetStyleSettings();

    Size aSize = rOutSz;
    Size aDropDownSize;

    if (GetStyle() & WB_DROPDOWN)
    {
        tools::Long nW = rStyleSettings.GetScrollBarSize();
        nW = GetDrawPixel( pDev, nW );
        aDropDownSize = Size( CalcZoom( nW ), aSize.Height() );
        aSize.AdjustWidth( -(aDropDownSize.Width()) );
        rDDArea = tools::Rectangle( Point( aSize.Width(), 0 ), aDropDownSize );
        rDDArea.AdjustTop( -1 );
    }
    else
        rDDArea.SetEmpty();

    // calculate sizes according to the height
    if (GetStyle() & WB_SPIN)
    {
        tools::Long nBottom1 = aSize.Height()/2;
        tools::Long nBottom2 = aSize.Height()-1;
        tools::Long nTop2 = nBottom1;
        if ( !(aSize.Height() & 0x01) )
            nBottom1--;

        bool bNativeRegionOK = false;
        tools::Rectangle aContentUp, aContentDown;

        if ((pDev->GetOutDevType() == OUTDEV_WINDOW) &&
            // there is just no useful native support for spinfields with dropdown
            ! (GetStyle() & WB_DROPDOWN) &&
            IsNativeControlSupported(ControlType::Spinbox, ControlPart::Entire))
        {
            vcl::Window *pWin = pDev->GetOwnerWindow();
            vcl::Window *pBorder = pWin->GetWindow( GetWindowType::Border );

            // get the system's spin button size
            ImplControlValue aControlValue;
            tools::Rectangle aBound;
            Point aPoint;

            // use the full extent of the control
            tools::Rectangle aArea( aPoint, pBorder->GetOutputSizePixel() );

            bNativeRegionOK =
                pWin->GetNativeControlRegion(ControlType::Spinbox, ControlPart::ButtonUp,
                    aArea, ControlState::NONE, aControlValue, aBound, aContentUp) &&
                pWin->GetNativeControlRegion(ControlType::Spinbox, ControlPart::ButtonDown,
                    aArea, ControlState::NONE, aControlValue, aBound, aContentDown);

            if (bNativeRegionOK)
            {
                // convert back from border space to local coordinates
                aPoint = pBorder->ScreenToOutputPixel( pWin->OutputToScreenPixel( aPoint ) );
                aContentUp.Move(-aPoint.X(), -aPoint.Y());
                aContentDown.Move(-aPoint.X(), -aPoint.Y());
            }
        }

        if (bNativeRegionOK)
        {
            rSpinUpArea = aContentUp;
            rSpinDownArea = aContentDown;
        }
        else
        {
            aSize.AdjustWidth( -(CalcZoom( GetDrawPixel( pDev, rStyleSettings.GetSpinSize() ) )) );

            rSpinUpArea = tools::Rectangle( aSize.Width(), 0, rOutSz.Width()-aDropDownSize.Width()-1, nBottom1 );
            rSpinDownArea = tools::Rectangle( rSpinUpArea.Left(), nTop2, rSpinUpArea.Right(), nBottom2 );
        }
    }
    else
    {
        rSpinUpArea.SetEmpty();
        rSpinDownArea.SetEmpty();
    }
}

void SpinField::Resize()
{
    if (!mbSpin)
        return;

    Control::Resize();
    Size aSize = GetOutputSizePixel();
    bool bSubEditPositioned = false;

    if (GetStyle() & (WB_SPIN | WB_DROPDOWN))
    {
        ImplCalcButtonAreas( GetOutDev(), aSize, maDropDownRect, maUpperRect, maLowerRect );

        ImplControlValue aControlValue;
        Point aPoint;
        tools::Rectangle aContent, aBound;

        // use the full extent of the control
        vcl::Window *pBorder = GetWindow( GetWindowType::Border );
        tools::Rectangle aArea( aPoint, pBorder->GetOutputSizePixel() );

        // adjust position and size of the edit field
        if (GetNativeControlRegion(ControlType::Spinbox, ControlPart::SubEdit, aArea, ControlState::NONE,
                                   aControlValue, aBound, aContent) &&
            // there is just no useful native support for spinfields with dropdown
            !(GetStyle() & WB_DROPDOWN))
        {
            // convert back from border space to local coordinates
            aPoint = pBorder->ScreenToOutputPixel(OutputToScreenPixel(aPoint));
            aContent.Move(-aPoint.X(), -aPoint.Y());

            // use the themes drop down size
            mpEdit->SetPosPixel( aContent.TopLeft() );
            bSubEditPositioned = true;
            aSize = aContent.GetSize();
        }
        else
        {
            if (maUpperRect.IsEmpty())
            {
                SAL_WARN_IF( maDropDownRect.IsEmpty(), "vcl", "SpinField::Resize: SPIN && DROPDOWN, but all empty rects?" );
                aSize.setWidth( maDropDownRect.Left() );
            }
            else
                aSize.setWidth( maUpperRect.Left() );
        }
    }

    if (!bSubEditPositioned)
    {
        // this moves our sub edit if RTL gets switched
        mpEdit->SetPosPixel(Point());
    }
    mpEdit->SetSizePixel(aSize);

#ifdef USE_JAVA
    if ( IsNativeControlSupported( ControlType::Spinbox, ControlPart::Entire ) )
    {
        ImplInvalidateOutermostBorder( this );
    }
    else
    {
#endif	// USE_JAVA
    if (GetStyle() & WB_SPIN)
        Invalidate(tools::Rectangle(maUpperRect.TopLeft(), maLowerRect.BottomRight()));
    if (GetStyle() & WB_DROPDOWN)
        Invalidate(maDropDownRect);
#ifdef USE_JAVA
    }
#endif	// USE_JAVA
}

void SpinField::StateChanged(StateChangedType nType)
{
    Edit::StateChanged(nType);

    if (nType == StateChangedType::Enable)
    {
        if (mbSpin || (GetStyle() & WB_DROPDOWN))
        {
            mpEdit->Enable(IsEnabled());

#ifdef USE_JAVA
            if ( IsNativeControlSupported( ControlType::Spinbox, ControlPart::Entire ) )
            {
                ImplInvalidateOutermostBorder( this );
            }
            else
            {
#endif	// USE_JAVA
            if (mbSpin)
            {
                Invalidate(maLowerRect);
                Invalidate(maUpperRect);
            }
            if (GetStyle() & WB_DROPDOWN)
                Invalidate(maDropDownRect);
#ifdef USE_JAVA
            }
#endif	// USE_JAVA
        }
    }
    else if (nType == StateChangedType::Style)
    {
        if (GetStyle() & WB_REPEAT)
            mbRepeat = true;
        else
            mbRepeat = false;
    }
    else if (nType == StateChangedType::Zoom)
    {
        Resize();
        if (mpEdit)
            mpEdit->SetZoom(GetZoom());
        Invalidate();
    }
    else if (nType == StateChangedType::ControlFont)
    {
        if (mpEdit)
            mpEdit->SetControlFont(GetControlFont());
        Invalidate();
    }
    else if (nType == StateChangedType::ControlForeground)
    {
        if (mpEdit)
            mpEdit->SetControlForeground(GetControlForeground());
        Invalidate();
    }
    else if (nType == StateChangedType::ControlBackground)
    {
        if (mpEdit)
            mpEdit->SetControlBackground(GetControlBackground());
        Invalidate();
    }
    else if( nType == StateChangedType::Mirroring )
    {
        if (mpEdit)
            mpEdit->CompatStateChanged(StateChangedType::Mirroring);
        Resize();
    }
}

void SpinField::DataChanged( const DataChangedEvent& rDCEvt )
{
    Edit::DataChanged(rDCEvt);

    if ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
        (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        Resize();
        Invalidate();
    }
}

tools::Rectangle* SpinField::ImplFindPartRect(const Point& rPt)
{
    if (maUpperRect.Contains(rPt))
        return &maUpperRect;
    else if (maLowerRect.Contains(rPt))
        return &maLowerRect;
    else
        return nullptr;
}

bool SpinField::PreNotify(NotifyEvent& rNEvt)
{
    if (rNEvt.GetType() == MouseNotifyEvent::MOUSEMOVE)
    {
        const MouseEvent* pMouseEvt = rNEvt.GetMouseEvent();
        if (pMouseEvt && !pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged())
        {
            // trigger redraw if mouse over state has changed
            if( IsNativeControlSupported(ControlType::Spinbox, ControlPart::Entire) ||
                IsNativeControlSupported(ControlType::Spinbox, ControlPart::AllButtons) )
            {
                tools::Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                tools::Rectangle* pLastRect = ImplFindPartRect( GetLastPointerPosPixel() );
                if( pRect != pLastRect || (pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow()) )
                {
#ifdef USE_JAVA
                    if ( IsNativeControlSupported( ControlType::Spinbox, ControlPart::Entire ) )
#else	// USE_JAVA
                    // FIXME: this is currently only on macOS
                    // check for other platforms that need similar handling
                    if (ImplGetSVData()->maNWFData.mbNoFocusRects && IsNativeWidgetEnabled() &&
                        IsNativeControlSupported(ControlType::Editbox, ControlPart::Entire))
#endif	// USE_JAVA
                    {
                        ImplInvalidateOutermostBorder(this);
                    }
                    else
                    {
                        // paint directly
                        vcl::Region aRgn( GetOutDev()->GetActiveClipRegion() );
                        if (pLastRect)
                        {
                            GetOutDev()->SetClipRegion(vcl::Region(*pLastRect));
                            Invalidate(*pLastRect);
                            GetOutDev()->SetClipRegion( aRgn );
                        }
                        if (pRect)
                        {
                            GetOutDev()->SetClipRegion(vcl::Region(*pRect));
                            Invalidate(*pRect);
                            GetOutDev()->SetClipRegion( aRgn );
                        }
                    }
                }
            }
        }
    }

    return Edit::PreNotify(rNEvt);
}

void SpinField::EndDropDown()
{
    mbInDropDown = false;
    Invalidate(tools::Rectangle(Point(), GetOutputSizePixel()));
}

bool SpinField::ShowDropDown( bool )
{
    return false;
}

Size SpinField::CalcMinimumSizeForText(const OUString &rString) const
{
    Size aSz = Edit::CalcMinimumSizeForText(rString);

    if ( GetStyle() & WB_DROPDOWN )
        aSz.AdjustWidth(GetSettings().GetStyleSettings().GetScrollBarSize() );
    if ( GetStyle() & WB_SPIN )
    {
        ImplControlValue aControlValue;
        tools::Rectangle aArea( Point(), Size(100, aSz.Height()));
        tools::Rectangle aEntireBound, aEntireContent, aEditBound, aEditContent;
        if (
               GetNativeControlRegion(ControlType::Spinbox, ControlPart::Entire,
                   aArea, ControlState::NONE, aControlValue, aEntireBound, aEntireContent) &&
               GetNativeControlRegion(ControlType::Spinbox, ControlPart::SubEdit,
                   aArea, ControlState::NONE, aControlValue, aEditBound, aEditContent)
           )
        {
            aSz.AdjustWidth(aEntireContent.GetWidth() - aEditContent.GetWidth());
        }
        else
        {
            aSz.AdjustWidth(maUpperRect.GetWidth() );
        }
    }

    return aSz;
}

Size SpinField::CalcMinimumSize() const
{
    return CalcMinimumSizeForText(GetText());
}

Size SpinField::GetOptimalSize() const
{
    return CalcMinimumSize();
}

Size SpinField::CalcSize(sal_Int32 nChars) const
{
    Size aSz = Edit::CalcSize( nChars );

    if ( GetStyle() & WB_DROPDOWN )
        aSz.AdjustWidth(GetSettings().GetStyleSettings().GetScrollBarSize() );
    if ( GetStyle() & WB_SPIN )
        aSz.AdjustWidth(GetSettings().GetStyleSettings().GetSpinSize() );

    return aSz;
}

IMPL_LINK( SpinField, ImplTimeout, Timer*, pTimer, void )
{
    if ( pTimer->GetTimeout() == static_cast<sal_uInt64>(MouseSettings::GetButtonStartRepeat()) )
    {
        pTimer->SetTimeout( GetSettings().GetMouseSettings().GetButtonRepeat() );
        pTimer->Start();
    }
    else
    {
        if ( mbInitialUp )
            Up();
        else
            Down();

#ifdef USE_JAVA
        if ( IsNativeControlSupported( ControlType::Spinbox, ControlPart::Entire ) )
            ImplInvalidateOutermostBorder( this );
#endif	// USE_JAVA
    }
}

void SpinField::Draw(OutputDevice* pDev, const Point& rPos, SystemTextColorFlags nFlags)
{
    Edit::Draw(pDev, rPos, nFlags);

    WinBits nFieldStyle = GetStyle();
    if ( (nFlags & SystemTextColorFlags::NoControls ) || !( nFieldStyle & (WB_SPIN|WB_DROPDOWN) ) )
        return;

    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = GetSizePixel();
    AllSettings aOldSettings = pDev->GetSettings();

    pDev->Push();
    pDev->SetMapMode();

    tools::Rectangle aDD, aUp, aDown;
    ImplCalcButtonAreas(pDev, aSize, aDD, aUp, aDown);
    aDD.Move(aPos.X(), aPos.Y());
    aUp.Move(aPos.X(), aPos.Y());
    aUp.AdjustTop( 1 );
    aDown.Move(aPos.X(), aPos.Y());

#ifndef USE_JAVA
    Color aButtonTextColor;
    if (nFlags & SystemTextColorFlags::Mono)
        aButtonTextColor = COL_BLACK;
    else
        aButtonTextColor = GetSettings().GetStyleSettings().GetButtonTextColor();
#endif	// !USE_JAVA

    if (GetStyle() & WB_DROPDOWN)
    {
#ifdef USE_JAVA
        ImplDrawSpinButton(*pDev, this, aUp, aDown, false, false, true, true, mbInDropDown );
#else	// USE_JAVA
        DecorationView aView( pDev );
        tools::Rectangle aInnerRect = aView.DrawButton( aDD, DrawButtonFlags::NoLightBorder );
        DrawSymbolFlags nSymbolStyle = IsEnabled() ? DrawSymbolFlags::NONE : DrawSymbolFlags::Disable;
        aView.DrawSymbol(aInnerRect, SymbolType::SPIN_DOWN, aButtonTextColor, nSymbolStyle);
#endif	// USE_JAVA
    }

    if (GetStyle() & WB_SPIN)
    {
        ImplDrawSpinButton(*pDev, this, aUp, aDown, false, false);
    }

    pDev->Pop();
    pDev->SetSettings(aOldSettings);

}

FactoryFunction SpinField::GetUITestFactory() const
{
    return SpinFieldUIObject::create;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
