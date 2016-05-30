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
 *   Modified May 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 4
 *   of the Apache License, Version 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <salgdi.hxx>

using namespace rtl;

/****************************************************************
 *  Placeholder for no native widgets
 ***************************************************************/


/*
 * IsNativeControlSupported()
 *
 *  Returns sal_True if the platform supports native
 *  drawing of the control defined by nPart
 */
sal_Bool SalGraphics::IsNativeControlSupported( ControlType, ControlPart )
{
	return( sal_False );
}


/*
 * HitTestNativeControl()
 *
 *  If the return value is sal_True, bIsInside contains information whether
 *  aPos was or was not inside the native widget specified by the
 *  nType/nPart combination.
 */
sal_Bool SalGraphics::hitTestNativeControl( ControlType,
							  ControlPart,
							  const Rectangle&,
							  const Point&,
							  sal_Bool& )
{
	return( sal_False );
}


/*
 * DrawNativeControl()
 *
 *  Draws the requested control described by nPart/nState.
 *
 *  rControlRegion:	The bounding region of the complete control in VCL frame coordinates.
 *  aValue:  		An optional value (tristate/numerical/string)
 *  aCaption:  	A caption or title string (like button text etc)
 */
sal_Bool SalGraphics::drawNativeControl(	ControlType,
							ControlPart,
							const Rectangle&,
							ControlState,
							const ImplControlValue&,
							const OUString& )
{
	return( sal_False );
}


/*
 * DrawNativeControlText()
 *
 *  OPTIONAL.  Draws the requested text for the control described by nPart/nState.
 *     Used if text not drawn by DrawNativeControl().
 *
 *  rControlRegion:	The bounding region of the complete control in VCL frame coordinates.
 *  aValue:  		An optional value (tristate/numerical/string)
 *  aCaption:  	A caption or title string (like button text etc)
 */
sal_Bool SalGraphics::drawNativeControlText(	ControlType,
								ControlPart,
								const Rectangle&,
								ControlState,
								const ImplControlValue&,
								const OUString& )
{
	return( sal_False );
}


/*
 * GetNativeControlRegion()
 *
 *  If the return value is sal_True, rNativeBoundingRegion
 *  contains the sal_True bounding region covered by the control
 *  including any adornment, while rNativeContentRegion contains the area
 *  within the control that can be safely drawn into without drawing over
 *  the borders of the control.
 *
 *  rControlRegion:	The bounding region of the control in VCL frame coordinates.
 *  aValue:		An optional value (tristate/numerical/string)
 *  aCaption:		A caption or title string (like button text etc)
 */
sal_Bool SalGraphics::getNativeControlRegion(  ControlType,
								ControlPart,
								const Rectangle&,
								ControlState,
								const ImplControlValue&,
								const OUString&,
								Rectangle &,
								Rectangle & )
{
	return( sal_False );
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
sal_Bool SalGraphics::getNativeControlTextColor( ControlType nType, ControlPart nPart, ControlState nState, const ImplControlValue& aValue, SalColor& nTextColor )
{
    return( sal_False );
}

#endif	// USE_JAVA

