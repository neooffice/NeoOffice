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

#include <salgdi.hxx>

/****************************************************************
 *  Placeholder for no native widgets
 ***************************************************************/

bool SalGraphics::IsNativeControlSupported( ControlType, ControlPart )
{
    return false;
}

bool SalGraphics::hitTestNativeControl( ControlType, ControlPart,
                                        const tools::Rectangle&, const Point&, bool& )
{
    return false;
}

bool SalGraphics::drawNativeControl( ControlType, ControlPart,
                                     const tools::Rectangle&, ControlState,
                                     const ImplControlValue&, const OUString& )
{
    return false;
}

bool SalGraphics::getNativeControlRegion( ControlType, ControlPart,
                                          const tools::Rectangle&, ControlState,
                                          const ImplControlValue&,
                                          const OUString&, tools::Rectangle&, tools::Rectangle& )
{
    return false;
}

#ifdef USE_JAVA

/*
 * getNativeControlTextColor()
 *
 *  If the return value is sal_True, nTextColor contains the color with which
 *	the text should be drawn given the control state and other values. If the
 *	return value is sal_False, the control text should be drawn with the
 *	platform independent control color.
 *
 *  aValue:		An optional value (tristate/numerical/string)
 */
bool SalGraphics::getNativeControlTextColor( ControlType /* nType */, ControlPart /* nPart */, ControlState /* nState */, const ImplControlValue& /* aValue */, SalColor& /* nTextColor */ )
{
    return false;
}

#endif	// USE_JAVA

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
