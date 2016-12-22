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

#include "vcl/window.hxx"
#include "vcl/waitobj.hxx"
#include "vcl/button.hxx"

#ifdef USE_JAVA
#include "salgdi.hxx"
#endif	// USE_JAVA

WaitObject::~WaitObject()
{
    if ( mpWindow )
        mpWindow->LeaveWait();
}

namespace vcl {

Size Window::GetOptimalSize() const
{
    return Size();
}

void Window::ImplAdjustNWFSizes()
{
    switch( GetType() )
    {
    case WINDOW_CHECKBOX:
        static_cast<CheckBox*>(this)->ImplSetMinimumNWFSize();
        break;
    case WINDOW_RADIOBUTTON:
        static_cast<RadioButton*>(this)->ImplSetMinimumNWFSize();
        break;
    default:
        {
            // iterate over children
            vcl::Window* pWin = GetWindow( WINDOW_FIRSTCHILD );
            while( pWin )
            {
                pWin->ImplAdjustNWFSizes();
                pWin = pWin->GetWindow( WINDOW_NEXT );
            }
        }
        break;
    }
}

#ifdef USE_JAVA

bool Window::GetNativeControlTextColor( ControlType nType, ControlPart nPart, ControlState nState, const ImplControlValue& aValue, Color& nTextColor )
{
    if( !IsNativeWidgetEnabled() )
        return false;

    if ( !mpGraphics && !AcquireGraphics() )
        return false;

    return mpGraphics->GetNativeControlTextColor( nType, nPart, nState, aValue, nTextColor );
}

#endif	// USE_JAVA

} /* namespace vcl */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
