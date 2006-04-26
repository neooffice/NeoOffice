/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU General Public License Version 2.1
 *
 *  Edward Peterlin, April 2006
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2006 by Edward Peterlin (OPENSTEP@neooffice.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 ************************************************************************/

#ifdef GENESIS_OF_THE_NEW_WEAPONS

#define _SV_SALNATIVEWIDGETS_CXX

#ifndef _SV_SVSYS_H
#include <svsys.h>
#endif

#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALBMP_H
#include <salbmp.h>
#endif

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif
#include <osl/module.h>

#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif

#ifdef __cplusplus
#include <premac.h>
#endif
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#ifdef __cplusplus
#include <postmac.h>
#endif

using namespace vcl;
using namespace rtl;

#ifdef GENESIS_OF_THE_NEW_WEAPONS

#define COMBOBOX_BUTTON_WIDTH 22
#define COMBOBOX_BUTTON_TRIMWIDTH 3

static JavaSalBitmap *pComboBoxBitmap = NULL;
static JavaSalBitmap *pListBoxBitmap = NULL;

// =======================================================================

static BOOL InitButtonDrawInfo( HIThemeButtonDrawInfo *pButtonDrawInfo, ControlState nState )
{
	memset( pButtonDrawInfo, 0, sizeof( HIThemeButtonDrawInfo ) );
	pButtonDrawInfo->version = 0;
	pButtonDrawInfo->kind = kThemeComboBox;
	if ( nState & ( CTRL_STATE_PRESSED | CTRL_STATE_SELECTED ) )
		pButtonDrawInfo->state = kThemeStatePressed;
	else if ( nState & CTRL_STATE_ENABLED )
		pButtonDrawInfo->state = kThemeStateActive;
	else
		pButtonDrawInfo->state = kThemeStateInactive;

	if ( nState & CTRL_STATE_FOCUSED )
		pButtonDrawInfo->adornment = kThemeAdornmentFocus;
	else
		pButtonDrawInfo->adornment = kThemeAdornmentDefault;
}

// -----------------------------------------------------------------------

/**
 * (static) Draw a ComboBox into the graphics port at the specified location.
 * ComboBoxes are editable pulldowns, the left portion of which is an edit
 * field and the right portion a downward arrow button used to display the
 * full list contents.
 *
 * Due to VM implementation, JComboBox Swing elements cannot be drawn into
 * a Graphics unless the JComboBox is properly embedded into a visible JFrame.
 * The VM implementation draws these objects asynchronously.  Since we can't
 * easily handle it in Java, we'll use HITheme APIs to draw it into a
 * SalBitmap that we then blit into the graphics.
 *
 * @param pGraphics		pointer to the destination graphics where we'll
 *				be drawing
 * @param rDestBounds		eventual destination rectangle that encompasses
 *				the entire control, editing area as well as
 *				popup arrow
 * @param nState		state of the button to b drawn (enabled/pressed/etc.)
 * @param aCaption		text used for the control.  Presently ignored
 *				as we draw only the frame and let VCL draw
 *				the text
 * @return TRUE if successful, FALSE on error
 */
static BOOL DrawNativeComboBox( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState, OUString aCaption )
{
	BOOL bRet = FALSE;

	if ( !pComboBoxBitmap )
	{
		pComboBoxBitmap = new JavaSalBitmap();
		if ( !pComboBoxBitmap )
			return bRet;
	}

	if ( rDestBounds.GetWidth() > pComboBoxBitmap->GetSize().Width() || rDestBounds.GetHeight() > pComboBoxBitmap->GetSize().Height() )
		pComboBoxBitmap->Create( Size( rDestBounds.GetWidth(), rDestBounds.GetHeight() ), 32, BitmapPalette() );

	CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
	if ( !aColorSpace )
		return bRet;

	BitmapBuffer *pBuffer = pComboBoxBitmap->AcquireBuffer( false );
	if ( !pBuffer )
	{
		CGColorSpaceRelease( aColorSpace );
		return bRet;
	}

#ifdef POWERPC
	CGContextRef aContext = CGBitmapContextCreate( pBuffer->mpBits, pBuffer->mnWidth, pBuffer->mnHeight, 8, pBuffer->mnScanlineSize, aColorSpace, kCGImageAlphaPremultipliedFirst );
#else	// POWERPC
	CGContextRef aContext = CGBitmapContextCreate( pBuffer->mpBits, pBuffer->mnWidth, pBuffer->mnHeight, 8, pBuffer->mnScanlineSize, aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little );
#endif	// POWERPC
	if ( !aContext )
	{
		pComboBoxBitmap->ReleaseBuffer( pBuffer, false );
		CGColorSpaceRelease( aColorSpace );
		return bRet;
	}

	// Set the background to the fill color
	long nBits = pBuffer->mnWidth * pBuffer->mnHeight;
	int *pBits = (int *)pBuffer->mpBits;
	for ( long i = 0; i < nBits; i++ )
		pBits[ i ] = pGraphics->mnFillColor;

	HIThemeButtonDrawInfo aButtonDrawInfo;
	InitButtonDrawInfo( &aButtonDrawInfo, nState );

	HIRect destRect;
	destRect.origin.x = 0;
	destRect.origin.y = 0;
	destRect.size.width = rDestBounds.GetWidth() - COMBOBOX_BUTTON_TRIMWIDTH;
	destRect.size.height = rDestBounds.GetHeight();
	bRet = ( HIThemeDrawButton( &destRect, &aButtonDrawInfo, aContext, kHIThemeOrientationInverted, NULL ) == noErr );

	CGContextRelease( aContext );
	CGColorSpaceRelease( aColorSpace );

	pComboBoxBitmap->ReleaseBuffer( pBuffer, false );

	if ( bRet )
	{
		SalTwoRect aTwoRect;
		aTwoRect.mnSrcX = 0;
		aTwoRect.mnSrcY = 0;
		aTwoRect.mnSrcWidth = rDestBounds.GetWidth();
		aTwoRect.mnSrcHeight = rDestBounds.GetHeight();
		aTwoRect.mnDestX = rDestBounds.Left();
		aTwoRect.mnDestY = rDestBounds.Top();
		aTwoRect.mnDestWidth = aTwoRect.mnSrcWidth;
		aTwoRect.mnDestHeight = aTwoRect.mnSrcHeight;
		pGraphics->drawBitmap( &aTwoRect, *pComboBoxBitmap );
	}

	return TRUE;
}

/**
 * (static) Draw a popupmenu into the graphics port at the specified location.
 * Popup menus, a.k.a listboxes or non-editable combo boxes, consist of a
 * regular popup button and text.
 *
 * Due to VM implementation, JComboBox Swing elements cannot be drawn into
 * a Graphics unless the JComboBox is properly embedded into a visible JFrame.
 * The VM implementation draws these objects asynchronously.  Since we can't
 * easily handle it in Java, we'll use HITheme APIs to draw it into a
 * SalBitmap that we then blit into the graphics.
 *
 * @param pGraphics		pointer to the destination graphics where we'll
 *				be drawing
 * @param rDestBounds		eventual destination rectangle that encompasses
 *				the entire control, editing area as well as
 *				popup arrow
 * @param nState		state of the button to b drawn (enabled/pressed/etc.)
 * @param aCaption		text used for the control.  Presently ignored
 *				as we draw only the frame and let VCL draw
 *				the text
 * @return TRUE if successful, FALSE on error
 */
static BOOL DrawNativeListBox( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState, OUString aCaption )
{
	BOOL bRet = FALSE;

	if ( !pListBoxBitmap )
	{
		pListBoxBitmap = new JavaSalBitmap();
		if ( !pComboBoxBitmap )
			return bRet;
	}

	if ( rDestBounds.GetWidth() > pComboBoxBitmap->GetSize().Width() || rDestBounds.GetHeight() > pComboBoxBitmap->GetSize().Height() )
		pComboBoxBitmap->Create( Size( rDestBounds.GetWidth(), rDestBounds.GetHeight() ), 32, BitmapPalette() );

	CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
	if ( !aColorSpace )
		return bRet;

	BitmapBuffer *pBuffer = pComboBoxBitmap->AcquireBuffer( false );
	if ( !pBuffer )
	{
		CGColorSpaceRelease( aColorSpace );
		return bRet;
	}

#ifdef POWERPC
	CGContextRef aContext = CGBitmapContextCreate( pBuffer->mpBits, pBuffer->mnWidth, pBuffer->mnHeight, 8, pBuffer->mnScanlineSize, aColorSpace, kCGImageAlphaPremultipliedFirst );
#else	// POWERPC
	CGContextRef aContext = CGBitmapContextCreate( pBuffer->mpBits, pBuffer->mnWidth, pBuffer->mnHeight, 8, pBuffer->mnScanlineSize, aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little );
#endif	// POWERPC
	if ( !aContext )
	{
		pComboBoxBitmap->ReleaseBuffer( pBuffer, false );
		CGColorSpaceRelease( aColorSpace );
		return bRet;
	}

	// Set the background to the fill color
	long nBits = pBuffer->mnWidth * pBuffer->mnHeight;
	int *pBits = (int *)pBuffer->mpBits;
	for ( long i = 0; i < nBits; i++ )
		pBits[ i ] = pGraphics->mnFillColor;

	HIThemeButtonDrawInfo aButtonDrawInfo;
	InitButtonDrawInfo( &aButtonDrawInfo, nState );
	aButtonDrawInfo.kind = kThemePopupButton;
	
	HIRect destRect;
	destRect.origin.x = 0;
	destRect.origin.y = 0;
	destRect.size.width = rDestBounds.GetWidth()/* - COMBOBOX_BUTTON_TRIMWIDTH */;
	destRect.size.height = rDestBounds.GetHeight();
	bRet = ( HIThemeDrawButton( &destRect, &aButtonDrawInfo, aContext, kHIThemeOrientationInverted, NULL ) == noErr );

	CGContextRelease( aContext );
	CGColorSpaceRelease( aColorSpace );

	pComboBoxBitmap->ReleaseBuffer( pBuffer, false );

	if ( bRet )
	{
		SalTwoRect aTwoRect;
		aTwoRect.mnSrcX = 0;
		aTwoRect.mnSrcY = 0;
		aTwoRect.mnSrcWidth = rDestBounds.GetWidth();
		aTwoRect.mnSrcHeight = rDestBounds.GetHeight();
		aTwoRect.mnDestX = rDestBounds.Left();
		aTwoRect.mnDestY = rDestBounds.Top();
		aTwoRect.mnDestWidth = aTwoRect.mnSrcWidth;
		aTwoRect.mnDestHeight = aTwoRect.mnSrcHeight;
		pGraphics->drawBitmap( &aTwoRect, *pComboBoxBitmap );
	}

	return TRUE;
}

#endif // GENESIS_OF_THE_NEW_WEAPONS

// =======================================================================

/**
 * Determine if support exists for drawing a particular native widget in the
 * interface.
 *
 * @param nType         control flavor that is requsted to be drawn
 * @param nPart         subpart of the control that is requested to be drawn
 * @return TRUE if supported, FALSE if not
 */
BOOL JavaSalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
	BOOL isSupported = NULL;

	switch( nType )
	{
		case CTRL_PUSHBUTTON:
			if( nPart == PART_ENTIRE_CONTROL )
				isSupported = TRUE;
			break;

		case CTRL_RADIOBUTTON:
			if( nPart == PART_ENTIRE_CONTROL )
				isSupported = TRUE;
			break;

		case CTRL_CHECKBOX:
			if( nPart == PART_ENTIRE_CONTROL )
				isSupported = TRUE;
			break;

		case CTRL_COMBOBOX:
			if( ( nPart == PART_ENTIRE_CONTROL ) || ( nPart == HAS_BACKGROUND_TEXTURE ) )
				isSupported = TRUE;
			break;
		
		case CTRL_LISTBOX:
			if( ( nPart == PART_ENTIRE_CONTROL ) || ( nPart == HAS_BACKGROUND_TEXTURE ) )
				isSupported = TRUE;
			break;

		default:
			isSupported = FALSE;
			break;
	}

	return isSupported;
}

// =======================================================================

/**
 * Called to perform hit testing on native widgets.  If the widget itself
 * conforms to standard OpenOffice.org hit testing and subcontrol positions,
 * overloading the OOo hit testing should not be necessary.
 *
 * @param nType                 primary type of control to be hit tested
 * @param nPart                 subportion of the control to be hit tested
 * @param rControlRegion
 * @param aPos                  coordinate to hit test
 * @param rControlHandle
 * @param rIsInside             return parameter indicating whether aPos was
 *                              within the control or not
 * @return TRUE if the function performed hit testing, FALSE if default OOo
 *      hit testing should be used instead
 */
BOOL JavaSalGraphics::hitTestNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion, const Point& aPos, SalControlHandle& rControlHandle, BOOL& rIsInside )
{
	Region aNativeBoundingRegion;
	Region aNativeContentRegion;
	if ( getNativeControlRegion( nType, nPart, rControlRegion, 0, ImplControlValue(), rControlHandle, OUString(), aNativeBoundingRegion, aNativeContentRegion ) )
		return aNativeBoundingRegion.IsInside( aPos );
	else
		return FALSE;
}

// =======================================================================

/**
 * Draw a widget using the native platform appearance.
 *
 * @param nType                 type of control to draw
 * @param nPart			subpart of the control to draw
 * @param nState                current control state (e.g. pressed, disabled)
 * @param rControlRegion        bounding region of the entire control in VCL
 *				frame coordinates
 * @param aValue		An optional value used for certain control types
 * @param rControlHandle	Platform dependent control data
 * @param aCaption		Caption title or string for the control.
 *				Contains keyboard shortcuts prefixed with ~
 * @return TRUE if drawing was successful, FALSE if drawing was not successful
 */
BOOL JavaSalGraphics::drawNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle, OUString aCaption )
{
	BOOL bOK = FALSE;

	switch( nType )
	{
		case CTRL_PUSHBUTTON:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle buttonRect = rControlRegion.GetBoundRect();
				mpVCLGraphics->drawPushButton( buttonRect.Left(), buttonRect.Top(), buttonRect.GetWidth(), buttonRect.GetHeight(), aCaption, ( nState & CTRL_STATE_ENABLED ), ( nState & CTRL_STATE_FOCUSED ), ( nState & CTRL_STATE_PRESSED ), ( nState & CTRL_STATE_DEFAULT ) );
				bOK = TRUE;
			}
			break;

		case CTRL_RADIOBUTTON:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle buttonRect = rControlRegion.GetBoundRect();
				mpVCLGraphics->drawRadioButton( buttonRect.Left(), buttonRect.Top(), buttonRect.GetWidth(), buttonRect.GetHeight(), aCaption, ( nState & CTRL_STATE_ENABLED ), ( nState & CTRL_STATE_FOCUSED ), ( nState & CTRL_STATE_PRESSED ), aValue.getTristateVal() );
				bOK = TRUE;
			}
			break;

		case CTRL_CHECKBOX:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle buttonRect = rControlRegion.GetBoundRect();
				mpVCLGraphics->drawCheckBox( buttonRect.Left(), buttonRect.Top(), buttonRect.GetWidth(), buttonRect.GetHeight(), aCaption, ( nState & CTRL_STATE_ENABLED ), ( nState & CTRL_STATE_FOCUSED ), ( nState & CTRL_STATE_PRESSED ), aValue.getTristateVal() );
				bOK = TRUE;
			}
			break;

		case CTRL_COMBOBOX:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle buttonRect = rControlRegion.GetBoundRect();
				bOK = DrawNativeComboBox( this, buttonRect, nState, aCaption );
			}
			break;
		
		case CTRL_LISTBOX:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle buttonRect = rControlRegion.GetBoundRect();
				bOK = DrawNativeListBox( this, buttonRect, nState, aCaption );
			}
			break;
	}

	return bOK;
}

// =======================================================================

/**
 * Draws the requested text for a native control.  If the caption of the
 * control is drawn by drawNativeControl, this function does not need to be
 * implemented.
 *
 * @param nType			type of control to draw
 * @param nPart			subpart of the control to draw
 * @param rControlRegion	bounding region of the complete control in VCL frame coordiantes.
 * @param nState		current control state (e.g. pressed, disabled)
 * @param aValue		An optional value used for certain control types
 * @param rControlHandle	Platform dependent control data
 * @param aCaption		Caption title or string for the control.
 *				Contains keyboard shortcuts prefixed with ~
 * @return TRUE if the text was drawn, FALSE if the control had its text drawn
 *	with drawNativeControl()
 */
BOOL JavaSalGraphics::drawNativeControlText( ControlType nType, ControlPart nPart, const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle, OUString aCaption )
{
	return FALSE;
}

// =======================================================================

/**
 * Obtains information about the actual bounding area and available drawing
 * area of a native control.
 *
 * @param nType			type of control to query
 * @param nPart			subpart of the control to query
 * @param rControlRegion	bounding region of the control in VCL frame coordinates
 * @param nState		current control state (e.g. pressed, disabled)
 * @param aValue		An optional value used for certain control types
 * @param rControlHandle	Platform dependent control data
 * @param aCaption		Caption title or string for the control.
 *				Contains keyboard shortcuts prefixed with ~
 * @param rNativeBoundingRegion	return parameter that contains the true bounds
 *				of the control along with any platform specific
 *				control adornment (shadows, etc.)
 * @param rNativeContentRegion	return parameter that indicates the region which
 *				can safely be overdrawn with custom content
 *				without overdrawing the system adornments of
 *				the control
 * @return TRUE if appropriate information was returned about any native widget
 *	drawing areas, FALSE if the entire control region can be considered
 *	an accurate enough representation of the native widget area
 */
BOOL JavaSalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart, const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle, OUString aCaption, Region &rNativeBoundingRegion, Region &rNativeContentRegion )
{
	BOOL bReturn = FALSE;

	switch( nType )
	{
		case CTRL_PUSHBUTTON:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle buttonRect = rControlRegion.GetBoundRect();
				rNativeBoundingRegion = Region( mpVCLGraphics->getPreferredPushButtonBounds( buttonRect.Left(), buttonRect.Top(), buttonRect.GetWidth(), buttonRect.GetHeight(), aCaption ) );
				rNativeContentRegion = Region( rNativeBoundingRegion );
				bReturn = TRUE;
			}
			break;

		case CTRL_RADIOBUTTON:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle buttonRect = rControlRegion.GetBoundRect();
				rNativeBoundingRegion = Region( mpVCLGraphics->getPreferredRadioButtonBounds( buttonRect.Left(), buttonRect.Top(), buttonRect.GetWidth(), buttonRect.GetHeight(), aCaption ) );
				rNativeContentRegion = Region( rNativeBoundingRegion );
				bReturn = TRUE;
			}
			break;

		case CTRL_CHECKBOX:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle buttonRect = rControlRegion.GetBoundRect();
				rNativeBoundingRegion = Region( mpVCLGraphics->getPreferredCheckBoxBounds( buttonRect.Left(), buttonRect.Top(), buttonRect.GetWidth(), buttonRect.GetHeight(), aCaption ) );
				rNativeContentRegion = Region( rNativeBoundingRegion );
				bReturn = TRUE;
			}
			break;

		case CTRL_COMBOBOX:
		case CTRL_LISTBOX:
			{
				Rectangle comboBoxRect = rControlRegion.GetBoundRect();

				HIThemeButtonDrawInfo aButtonDrawInfo;
				InitButtonDrawInfo( &aButtonDrawInfo, nState );

				HIShapeRef preferredShape;
				HIRect destRect;
				destRect.origin.x = comboBoxRect.Left();
				destRect.origin.y = comboBoxRect.Top();
				destRect.size.width = comboBoxRect.GetWidth();
				destRect.size.height = comboBoxRect.GetHeight();
				if ( HIThemeGetButtonShape( &destRect, &aButtonDrawInfo, &preferredShape ) == noErr )
				{
					HIRect preferredRect;
					HIShapeGetBounds( preferredShape, &preferredRect );
					CFRelease( preferredShape );

					switch( nPart )
					{
						case PART_BUTTON_DOWN:
							{
								Point topLeft( (long)preferredRect.origin.x + (long)preferredRect.size.width - COMBOBOX_BUTTON_WIDTH, (long)preferredRect.origin.y );
								Size boundsSize( COMBOBOX_BUTTON_WIDTH + COMBOBOX_BUTTON_TRIMWIDTH, (long)preferredRect.size.height );
								rNativeBoundingRegion = Region( Rectangle( topLeft, boundsSize ) );
								rNativeContentRegion = Region( rNativeBoundingRegion );
								bReturn = TRUE;
							}
							break;

						case PART_SUB_EDIT:
							{
								Point topLeft( (long)preferredRect.origin.x, (long)preferredRect.origin.y );
								Size boundsSize( (long)preferredRect.size.width - COMBOBOX_BUTTON_WIDTH, (long)preferredRect.size.height );
								rNativeBoundingRegion = Region( Rectangle( topLeft, boundsSize ) );
								rNativeContentRegion = Region( rNativeBoundingRegion );
								bReturn = TRUE;
							}
							break;
					}
				}
			}
			break;
	}

	return bReturn;
}

#endif // GENESIS_OF_THE_NEW_WEAPONS
