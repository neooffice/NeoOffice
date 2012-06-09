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
 *  Copyright 2006 Planamesa Inc.
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

#define _SV_SALNATIVEWIDGETS_CXX

#include <dlfcn.h>

#include <salgdi.h>
#include <saldata.hxx>
#include <salbmp.h>
#include <salframe.h>
#include <salinst.h>
#include <svsys.h>
#include <rtl/ustring.h>
#include <osl/module.h>
#include <vcl/svapp.hxx>
#include <vcl/decoview.hxx>

#ifdef __cplusplus
#include <premac.h>
#endif
#include <ApplicationServices/ApplicationServices.h>
// Need to include for HITheme constants but we don't link to it
#include <Carbon/Carbon.h>
#ifdef __cplusplus
#include <postmac.h>
#endif

// Comment out the following line to disable native controls
#define USE_NATIVE_CONTROLS

#define COMBOBOX_BUTTON_WIDTH			( ( IsRunningLeopard() || IsRunningSnowLeopard() ) ? 25 : 24 )
#define COMBOBOX_BUTTON_HEIGHT_SLOP		0
#define COMBOBOX_BUTTON_TRIMWIDTH		3
#define COMBOBOX_BUTTON_TRIMHEIGHT		( ( IsRunningLeopard() || IsRunningSnowLeopard() ) ? 0 : 1 )
#define COMBOBOX_HEIGHT					( ( IsRunningLeopard() || IsRunningSnowLeopard() ) ? 28 : 29 )
#define CONTROL_TAB_PANE_TOP_OFFSET		12
// Fix bug 3378 by reducing the editbox height for low screen resolutions
#define EDITBOX_HEIGHT					( 24 * Application::GetSettings().GetStyleSettings().GetToolFont().GetHeight() / 10 )
#define FOCUSRING_WIDTH					3
#define LISTBOX_BUTTON_HORIZ_TRIMWIDTH	0
#define LISTBOX_BUTTON_VERT_TRIMWIDTH	1
#define LISTVIEWFRAME_TRIMWIDTH			1
#define SCROLLBAR_ARROW_TRIMX			13
#define SCROLLBAR_ARROW_TRIMY			14
#define SCROLLBAR_ARROW_TRIMWIDTH		11
#define SCROLLBAR_ARROW_TOP_TRIMHEIGHT	10
#define SCROLLBAR_ARROW_BOTTOM_TRIMHEIGHT	13
#define SCROLLBAR_THUMB_MIN_WIDTH		( ( IsRunningLeopard() || IsRunningSnowLeopard() ) ? 0 : 20 )
#define SCROLLBAR_THUMB_TRIMWIDTH		( ( IsRunningLeopard() || IsRunningSnowLeopard() ) ? 0 : 1 )
#define SCROLLBAR_SUPPRESS_ARROWS		( ( IsRunningLeopard() || IsRunningSnowLeopard() ) ? false : true )
#define SCROLLBAR_WIDTH_SLOP			0
#define SPINNER_TRIMWIDTH				3
#define SPINNER_TRIMHEIGHT				1
#define PROGRESS_HEIGHT_SLOP			( ( IsRunningLeopard() || IsRunningSnowLeopard() ) ? 0 : 1 )
#define TABITEM_HEIGHT_SLOP				4
#define CHECKBOX_WIDTH					16
#define CHECKBOX_HEIGHT					20
#define RADIOBUTTON_WIDTH				16
#define RADIOBUTTON_HEIGHT				16
#define PUSHBUTTON_HEIGHT_SLOP			4

using namespace osl;
using namespace rtl;
using namespace vcl;

struct SAL_DLLPRIVATE VCLBitmapBuffer : BitmapBuffer
{
	CGContextRef			maContext;
	MutexGuard*				mpGraphicsMutexGuard;
	HIThemeOrientation		mnHIThemeOrientationFlags;
	bool					mbLastDrawToPrintGraphics;
	bool					mbUseLayer;

							VCLBitmapBuffer();
	virtual					~VCLBitmapBuffer();

	BOOL					Create( long nX, long nY, long nWidth, long nHeight, JavaSalGraphics *pGraphics, bool bUseLayer = true );
	void					Destroy();
	void					DrawContextAndDestroy( JavaSalGraphics *pGraphics, CGRect aSrcRect, CGRect aDestRect );
	void					ReleaseContext();
};

typedef OSStatus GetThemeMetric_Type( ThemeMetric nMetric, SInt32 *pMetric);
typedef OSStatus GetThemeTextColor_Type( ThemeTextColor nColor, SInt16 nDepth, MacOSBoolean nColorDev, RGBColor *pColor );
typedef OSStatus HIThemeDrawButton_Type( const HIRect *pBounds, const HIThemeButtonDrawInfo *pDrawInfo, CGContextRef aContext, HIThemeOrientation nOrientation, HIRect *pLabelRect);
typedef OSStatus HIThemeDrawFrame_Type( const HIRect *pRect, const HIThemeFrameDrawInfo *pDrawInfo, CGContextRef aContext, HIThemeOrientation nOrientation);
typedef OSStatus HIThemeDrawGroupBox_Type( const HIRect *pRect, const HIThemeGroupBoxDrawInfo *pDrawInfo, CGContextRef aContext, HIThemeOrientation nOrientation );
typedef OSStatus HIThemeDrawMenuBackground_Type( const HIRect *pMenuRect, const HIThemeMenuDrawInfo *pMenuDrawInfo, CGContextRef aContext, HIThemeOrientation nOrientation);
typedef OSStatus HIThemeDrawSeparator_Type( const HIRect *pRect, const HIThemeSeparatorDrawInfo *pDrawInfo, CGContextRef aContext, HIThemeOrientation nOrientation);
typedef OSStatus HIThemeDrawTab_Type( const HIRect *pRect, const HIThemeTabDrawInfo *pDrawInfo, CGContextRef aContext, HIThemeOrientation nOrientation, HIRect *pLabelRect);
typedef OSStatus HIThemeDrawTabPane_Type( const HIRect *pRect, const HIThemeTabPaneDrawInfo *pDrawInfo, CGContextRef aContext, HIThemeOrientation nOrientation);
typedef OSStatus HIThemeDrawTrack_Type( const HIThemeTrackDrawInfo *pDrawInfo, const HIRect *pGhostRect, CGContextRef aContext, HIThemeOrientation nOrientation);
typedef OSStatus HIThemeGetButtonBackgroundBounds_Type( const HIRect *pBounds, const HIThemeButtonDrawInfo *pDrawInfo, HIRect *pBounds);
typedef OSStatus HIThemeGetGrowBoxBounds_Type( const HIPoint *pOrigin, const HIThemeGrowBoxDrawInfo *pDrawInfo, HIRect *pBounds);
typedef OSStatus HIThemeGetScrollBarTrackRect_Type( const HIRect *pBounds, const HIScrollBarTrackInfo *pTrackInfo, MacOSBoolean bIsHoriz, HIRect *pTrackBounds);
typedef OSStatus HIThemeGetTabShape_Type( const HIRect *pRect, const HIThemeTabDrawInfo *pDrawInfo, HIShapeRef *pShape);
typedef OSStatus HIThemeGetTrackBounds_Type( const HIThemeTrackDrawInfo *pDrawInfo, HIRect *pBounds);
typedef OSStatus HIThemeGetTrackPartBounds_Type( const HIThemeTrackDrawInfo *pDrawInfo, ControlPartCode nPartCode, HIRect *pPartBounds);

static bool bHIThemeInitialized = false;
static GetThemeMetric_Type *pGetThemeMetric = NULL;
static GetThemeTextColor_Type *pGetThemeTextColor = NULL;
static HIThemeDrawGroupBox_Type *pHIThemeDrawGroupBox = NULL;
static HIThemeDrawButton_Type *pHIThemeDrawButton = NULL;
static HIThemeDrawFrame_Type *pHIThemeDrawFrame = NULL;
static HIThemeDrawMenuBackground_Type *pHIThemeDrawMenuBackground = NULL;
static HIThemeDrawSeparator_Type *pHIThemeDrawSeparator = NULL;
static HIThemeDrawTab_Type *pHIThemeDrawTab = NULL;
static HIThemeDrawTabPane_Type *pHIThemeDrawTabPane = NULL;
static HIThemeDrawTrack_Type *pHIThemeDrawTrack = NULL;
static HIThemeGetButtonBackgroundBounds_Type *pHIThemeGetButtonBackgroundBounds = NULL;
static HIThemeGetGrowBoxBounds_Type *pHIThemeGetGrowBoxBounds = NULL;
static HIThemeGetScrollBarTrackRect_Type *pHIThemeGetScrollBarTrackRect = NULL;
static HIThemeGetTabShape_Type *pHIThemeGetTabShape = NULL;
static HIThemeGetTrackBounds_Type *pHIThemeGetTrackBounds = NULL;
static HIThemeGetTrackPartBounds_Type *pHIThemeGetTrackPartBounds = NULL;

static VCLBitmapBuffer aSharedComboBoxBuffer;
static VCLBitmapBuffer aSharedListBoxBuffer;
static VCLBitmapBuffer aSharedHorizontalScrollBarBuffer;
static VCLBitmapBuffer aSharedVerticalScrollBarBuffer;
static VCLBitmapBuffer aSharedScrollBarBuffer;
static VCLBitmapBuffer aSharedSpinboxBuffer;
static VCLBitmapBuffer aSharedSpinbuttonBuffer;
static VCLBitmapBuffer aSharedProgressbarBuffer;
static VCLBitmapBuffer aSharedTabBuffer;
static VCLBitmapBuffer aSharedTabBoundingBoxBuffer;
static VCLBitmapBuffer aSharedPrimaryGroupBoxBuffer;
static VCLBitmapBuffer aSharedMenuBackgroundBuffer;
static VCLBitmapBuffer aSharedEditBoxBuffer;
static VCLBitmapBuffer aSharedListViewFrameBuffer;
static VCLBitmapBuffer aSharedDisclosureBtnBuffer;
static VCLBitmapBuffer aSharedSeparatorLineBuffer;
static VCLBitmapBuffer aSharedListViewHeaderBuffer;
static VCLBitmapBuffer aSharedBevelButtonBuffer;
static VCLBitmapBuffer aSharedCheckboxBuffer;

inline long Float32ToLong( Float32 f ) { return (long)( f + 0.5 ); }

// =======================================================================

VCLBitmapBuffer::VCLBitmapBuffer() :
	BitmapBuffer(),
	maContext( NULL ),
	mpGraphicsMutexGuard( NULL ),
	mnHIThemeOrientationFlags( kHIThemeOrientationInverted ),
	mbLastDrawToPrintGraphics( false ),
	mbUseLayer( false )
{
	mnFormat = 0;
	mnWidth = 0;
	mnHeight = 0;
	mnScanlineSize = 0;
	mnBitCount = 0;
	mpBits = NULL;
}

// -----------------------------------------------------------------------

VCLBitmapBuffer::~VCLBitmapBuffer()
{
	Destroy();
}

// -----------------------------------------------------------------------

BOOL VCLBitmapBuffer::Create( long nX, long nY, long nWidth, long nHeight, JavaSalGraphics *pGraphics, bool bUseLayer )
{
	if ( nWidth <= 0 || nHeight <= 0 || !pGraphics )
		return FALSE;

	bool bDrawToPrintGraphics = ( pGraphics->mpPrinter ? true : false );
	mbUseLayer = false;

	// Note that we cannot draw to a frame's layer as it the native window's
	// flipped graphics context will cause the HITheme images to be flipped
	// regardless of the HITTheme orientation or context scaling used
	if ( bUseLayer && bDrawToPrintGraphics )
		bUseLayer = false;

	Destroy();

	mnFormat = JavaSalBitmap::Get32BitNativeFormat() | pGraphics->getBitmapDirectionFormat();
	mnWidth = nWidth;
	mnHeight = nHeight;
	mnScanlineSize = mnWidth * sizeof( sal_uInt32 );
	mnBitCount = 32;

	// If a layer is requested and there is a layer, draw to it directly
	if ( bUseLayer )
	{
		mpGraphicsMutexGuard = new MutexGuard( &pGraphics->getUndrawnNativeOpsMutex() );
		if ( mpGraphicsMutexGuard )
		{
			CGLayerRef aLayer = pGraphics->getLayer();
			if ( aLayer )
			{
				maContext = CGLayerGetContext( aLayer );
				if ( maContext )
				{
					CGContextRetain( maContext );
					CGContextSaveGState( maContext );
					JavaSalGraphics::setContextDefaultSettings( maContext, pGraphics->maFrameClipPath, pGraphics->maNativeClipPath, pGraphics->getNativeLineWidth() );
					CGRect aUnflippedRect = UnflipFlippedRect( CGRectMake( nX, nY, nWidth, nHeight ), pGraphics->maNativeBounds );
					CGContextTranslateCTM( maContext, aUnflippedRect.origin.x, aUnflippedRect.origin.y );
					mbUseLayer = true;
				}
			}

			if ( !mbUseLayer )
			{
				delete mpGraphicsMutexGuard;
				mpGraphicsMutexGuard = NULL;
			}
		}
	}

	if ( !mpBits && !mbUseLayer )
	{
		try
		{
			mpBits = new BYTE[ mnScanlineSize * mnHeight ];
		}
		catch( const std::bad_alloc& ) {}
	}

	if ( !mpBits && !mbUseLayer )
	{
		Destroy();
		return FALSE;
	}

	if ( !maContext && !mbUseLayer )
	{
		CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
		if ( !aColorSpace )
		{
			Destroy();
			return FALSE;
		}

		// Use requested width and height, not actual width and height of the
		// bitmap buffer
		maContext = CGBitmapContextCreate( mpBits, nWidth, nHeight, 8, mnScanlineSize, aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little );
		if ( maContext )
			CGContextSaveGState( maContext );

		CGColorSpaceRelease( aColorSpace );
	}

	if ( !maContext )
	{
		Destroy();
		return FALSE;
	}

	if ( mpBits )
		memset( mpBits, 0, mnScanlineSize * mnHeight );

	mbLastDrawToPrintGraphics = bDrawToPrintGraphics;

	return TRUE;
}

// -----------------------------------------------------------------------

void VCLBitmapBuffer::Destroy()
{
	mnHIThemeOrientationFlags = kHIThemeOrientationInverted;
	mnFormat = 0;
	mnWidth = 0;
	mnHeight = 0;
	mnScanlineSize = 0;
	mnBitCount = 0;
	mbLastDrawToPrintGraphics = false;
	mbUseLayer = false;

	if ( maContext )
	{
		CGContextRestoreGState( maContext );
		CGContextRelease( maContext );
		maContext = NULL;
	}

	if ( mpBits )
	{
		delete[] mpBits;
		mpBits = NULL;
	}

	if ( mpGraphicsMutexGuard )
	{
		delete mpGraphicsMutexGuard;
		mpGraphicsMutexGuard = NULL;
	}
}

// -----------------------------------------------------------------------

void VCLBitmapBuffer::DrawContextAndDestroy( JavaSalGraphics *pGraphics, CGRect aSrcRect, CGRect aDestRect )
{
	if ( pGraphics )
	{
		CGRect aUnflippedRect = UnflipFlippedRect( aDestRect, pGraphics->maNativeBounds );
		if ( mpBits && !mbUseLayer )
		{
			// Assign ownership of bits to a CGDataProvider instance
			CGDataProviderRef aProvider = CGDataProviderCreateWithData( NULL, mpBits, mnScanlineSize * mnHeight, ReleaseBitmapBufferBytePointerCallback );
			if ( aProvider )
			{
				mpBits = NULL;
				pGraphics->addUndrawnNativeOp( new JavaSalGraphicsDrawImageOp( pGraphics->maFrameClipPath, pGraphics->maNativeClipPath, false, false, aProvider, mnBitCount, mnScanlineSize, mnWidth, mnHeight, aSrcRect, aUnflippedRect ) );
				CGDataProviderRelease( aProvider );
			}
		}
		else if ( pGraphics->mpFrame )
		{
			pGraphics->addNeedsDisplayRect( aUnflippedRect, pGraphics->getNativeLineWidth() );
		}
	}

	Destroy();
}
		
// -----------------------------------------------------------------------

void VCLBitmapBuffer::ReleaseContext()
{
	mbUseLayer = false;

	if ( maContext )
	{
		CGContextRestoreGState( maContext );
		CGContextRelease( maContext );
		maContext = NULL;
	}

	if ( mpGraphicsMutexGuard )
	{
		delete mpGraphicsMutexGuard;
		mpGraphicsMutexGuard = NULL;
	}
}

// =======================================================================

static bool HIThemeInitialize()
{
	if ( !bHIThemeInitialized )
	{
		void *pLib = dlopen( NULL, RTLD_LAZY | RTLD_LOCAL );
		if ( pLib )
		{
			pGetThemeMetric = (GetThemeMetric_Type *)dlsym( pLib, "GetThemeMetric" );
			pGetThemeTextColor = (GetThemeTextColor_Type *)dlsym( pLib, "GetThemeTextColor" );
			pHIThemeDrawButton = (HIThemeDrawButton_Type *)dlsym( pLib, "HIThemeDrawButton" );
			pHIThemeDrawFrame = (HIThemeDrawFrame_Type *)dlsym( pLib, "HIThemeDrawFrame" );
			pHIThemeDrawGroupBox = (HIThemeDrawGroupBox_Type *)dlsym( pLib, "HIThemeDrawGroupBox" );
			pHIThemeDrawMenuBackground = (HIThemeDrawMenuBackground_Type *)dlsym( pLib, "HIThemeDrawMenuBackground" );
			pHIThemeDrawSeparator = (HIThemeDrawSeparator_Type *)dlsym( pLib, "HIThemeDrawSeparator" );
			pHIThemeDrawTab = (HIThemeDrawTab_Type *)dlsym( pLib, "HIThemeDrawTab" );
			pHIThemeDrawTabPane = (HIThemeDrawTabPane_Type *)dlsym( pLib, "HIThemeDrawTabPane" );
			pHIThemeDrawTrack = (HIThemeDrawTrack_Type *)dlsym( pLib, "HIThemeDrawTrack" );
			pHIThemeGetButtonBackgroundBounds = (HIThemeGetButtonBackgroundBounds_Type *)dlsym( pLib, "HIThemeGetButtonBackgroundBounds" );
			pHIThemeGetGrowBoxBounds = (HIThemeGetGrowBoxBounds_Type *)dlsym( pLib, "HIThemeGetGrowBoxBounds" );
			pHIThemeGetScrollBarTrackRect = (HIThemeGetScrollBarTrackRect_Type *)dlsym( pLib, "HIThemeGetScrollBarTrackRect" );
			pHIThemeGetTabShape = (HIThemeGetTabShape_Type *)dlsym( pLib, "HIThemeGetTabShape" );
			pHIThemeGetTrackBounds = (HIThemeGetTrackBounds_Type *)dlsym( pLib, "HIThemeGetTrackBounds" );
			pHIThemeGetTrackPartBounds = (HIThemeGetTrackPartBounds_Type *)dlsym( pLib, "HIThemeGetTrackPartBounds" );

			dlclose( pLib );
		}

#ifdef DEBUG
		fprintf( stderr, "pGetThemeMetric: %p\n", pGetThemeMetric );
		fprintf( stderr, "pGetThemeTextColor: %p\n", pGetThemeTextColor );
		fprintf( stderr, "pHIThemeDrawGroupBox: %p\n", pHIThemeDrawGroupBox );
		fprintf( stderr, "pHIThemeDrawButton: %p\n", pHIThemeDrawButton );
		fprintf( stderr, "pHIThemeDrawFrame: %p\n", pHIThemeDrawFrame );
		fprintf( stderr, "pHIThemeDrawMenuBackground: %p\n", pHIThemeDrawMenuBackground );
		fprintf( stderr, "pHIThemeDrawSeparator: %p\n", pHIThemeDrawSeparator );
		fprintf( stderr, "pHIThemeDrawTab: %p\n", pHIThemeDrawTab );
		fprintf( stderr, "pHIThemeDrawTabPane: %p\n", pHIThemeDrawTabPane );
		fprintf( stderr, "pHIThemeDrawTrack: %p\n", pHIThemeDrawTrack );
		fprintf( stderr, "pHIThemeGetButtonBackgroundBounds: %p\n", pHIThemeGetButtonBackgroundBounds );
		fprintf( stderr, "pHIThemeGetGrowBoxBounds: %p\n", pHIThemeGetGrowBoxBounds );
		fprintf( stderr, "pHIThemeGetScrollBarTrackRect: %p\n", pHIThemeGetScrollBarTrackRect );
		fprintf( stderr, "pHIThemeGetTabShape: %p\n", pHIThemeGetTabShape );
		fprintf( stderr, "pHIThemeGetTrackBounds: %p\n", pHIThemeGetTrackBounds );
		fprintf( stderr, "pHIThemeGetTrackPartBounds: %p\n", pHIThemeGetTrackPartBounds );
#endif	// DEBUG

		bHIThemeInitialized = true;
	}

	return ( pGetThemeMetric && pGetThemeTextColor && pHIThemeDrawGroupBox && pHIThemeDrawButton && pHIThemeDrawFrame && pHIThemeDrawMenuBackground && pHIThemeDrawSeparator && pHIThemeDrawTab && pHIThemeDrawTabPane && pHIThemeDrawTrack && pHIThemeGetButtonBackgroundBounds && pHIThemeGetGrowBoxBounds && pHIThemeGetScrollBarTrackRect && pHIThemeGetTabShape && pHIThemeGetTrackBounds && pHIThemeGetTrackPartBounds );
}

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
		pButtonDrawInfo->adornment = kThemeAdornmentNone;

	return TRUE;
}

// =======================================================================

/**
 * (static) Initialize an HITheme button structure to draw spinner arrows for a
 * spinbox, an editable control with arrow controls for incrementing and
 * decrementing a value.
 *
 * @param pButtonDrawInfo		HITheme button structure for drawing spinner
 * @param nState				overall conrol state of the control; overall
 *								disabled control state will override individual
 *								button state
 * @param pSpinbuttonValue		VCL structure holding enable state of the
 * @return TRUE if successful, FALSE on failure
 */
static BOOL InitSpinbuttonDrawInfo( HIThemeButtonDrawInfo *pButtonDrawInfo, ControlState nState, SpinbuttonValue *pSpinbuttonValue )
{
	memset( pButtonDrawInfo, 0, sizeof( HIThemeButtonDrawInfo ) );
	pButtonDrawInfo->version = 0;
	pButtonDrawInfo->kind = kThemeIncDecButton;
	if( pSpinbuttonValue )
	{
		if( ! ( nState & ( CTRL_STATE_ENABLED | CTRL_STATE_PRESSED ) ) )
		{
			// entire control is disabled, trumps sub-part state
			pButtonDrawInfo->state = kThemeStateInactive;
		}
		else
		{
			// use individual arrow state
			if( pSpinbuttonValue->mnUpperState & CTRL_STATE_PRESSED )
				pButtonDrawInfo->state = kThemeStatePressedUp;
			else if( pSpinbuttonValue->mnLowerState & CTRL_STATE_PRESSED )
				pButtonDrawInfo->state = kThemeStatePressedDown;
			else if( ( ! ( pSpinbuttonValue->mnUpperState & CTRL_STATE_ENABLED ) ) && ( ! ( pSpinbuttonValue->mnLowerState & CTRL_STATE_ENABLED ) ) )
				pButtonDrawInfo->state = kThemeStateInactive;
			else
				pButtonDrawInfo->state = kThemeStateActive;
		}
	}
	else
	{
		if( ! ( nState & CTRL_STATE_ENABLED ) )
			pButtonDrawInfo->state = kThemeStateInactive;
		else
			pButtonDrawInfo->state = kThemeStateActive;
	}
	return TRUE;
}

// =======================================================================

/**
 * (static) Convert a VCL scrollbar value structure into HITheme structures.
 *
 * @param pScrollBarTrackInfo		HITheme scrollbar structure
 * @param pHITrackInfo				alternative HITheme scrollbar structure used
 *									by some calls
 * @param nState					overall control state of the scrollbar.
 *									states of subparts are contained in the
 *									scrollbar value structure.
 * @param bounds					drawing bounds of the scrollbar
 * @param pScrollbarValue			VCL implcontrolvalue containing scrollbar
 *									value
 * @return TRUE on success, FALSE on failure
 */
static BOOL InitScrollBarTrackInfo( HIThemeTrackDrawInfo *pTrackDrawInfo, HIScrollBarTrackInfo *pHITrackInfo, ControlState nState, Rectangle bounds, ScrollbarValue *pScrollbarValue )
{
	memset( pTrackDrawInfo, 0, sizeof( HIThemeTrackDrawInfo ) );
	if( pHITrackInfo )
		memset( pHITrackInfo, 0, sizeof( HIScrollBarTrackInfo ) );
	pTrackDrawInfo->version = 0;
	pTrackDrawInfo->kind = kThemeScrollBarMedium;
	pTrackDrawInfo->bounds.origin.x = 0;
	pTrackDrawInfo->bounds.origin.y = 0;
	pTrackDrawInfo->bounds.size.width = bounds.GetWidth();
	pTrackDrawInfo->bounds.size.height = bounds.GetHeight();
	if( bounds.GetWidth() > bounds.GetHeight() )
		pTrackDrawInfo->attributes |= kThemeTrackHorizontal;
	pTrackDrawInfo->attributes |= kThemeTrackShowThumb;
	if( nState & CTRL_STATE_ENABLED )
		pTrackDrawInfo->enableState = kThemeTrackActive;
	else
		pTrackDrawInfo->enableState = kThemeTrackInactive;
	if( pScrollbarValue )
	{
		pTrackDrawInfo->min = pScrollbarValue->mnMin;
		pTrackDrawInfo->max = pScrollbarValue->mnMax-pScrollbarValue->mnVisibleSize;
		pTrackDrawInfo->value = pScrollbarValue->mnCur;
		pTrackDrawInfo->trackInfo.scrollbar.viewsize = pScrollbarValue->mnVisibleSize;
		if( pScrollbarValue->mnButton1State & CTRL_STATE_PRESSED )
		{
			// We need to draw both inside and outside buttons if single arrow
			// sets are used as there are drawing problems on Panther
			if( pScrollbarValue->mnButton1State & CTRL_STATE_SELECTED )
				pTrackDrawInfo->trackInfo.scrollbar.pressState |= kThemeRightInsideArrowPressed;
			else
				pTrackDrawInfo->trackInfo.scrollbar.pressState |= ( kThemeLeftOutsideArrowPressed | kThemeLeftOutsideArrowPressed );
		}
		if( pScrollbarValue->mnButton2State & CTRL_STATE_PRESSED )
		{
			if( pScrollbarValue->mnButton2State & CTRL_STATE_SELECTED )
				pTrackDrawInfo->trackInfo.scrollbar.pressState |= kThemeLeftInsideArrowPressed;
			else
				pTrackDrawInfo->trackInfo.scrollbar.pressState |= kThemeRightOutsideArrowPressed;
		}
		if( pScrollbarValue->mnPage1State & CTRL_STATE_PRESSED )
			pTrackDrawInfo->trackInfo.scrollbar.pressState |= kThemeLeftTrackPressed;
		if( pScrollbarValue->mnPage2State & CTRL_STATE_PRESSED )
			pTrackDrawInfo->trackInfo.scrollbar.pressState |= kThemeRightTrackPressed;
		if( pScrollbarValue->mnThumbState & CTRL_STATE_PRESSED )
			pTrackDrawInfo->trackInfo.scrollbar.pressState |= kThemeThumbPressed;
	}

	if( pTrackDrawInfo->min == pTrackDrawInfo->max )
	{
		// we need to seed the min, max, and value with "reasonable" values
		// in order for scrollbar metrics to be computed properly by HITheme.
		// If the values are all equal, HITheme will return a NULL rectangle
		// for all potential scrollbar parts.

		pTrackDrawInfo->min = 0;
		pTrackDrawInfo->max = 1;
		pTrackDrawInfo->value = 0;
		// Fix bug 3359 by disabling the scrollbar. Note that we set it to
		// disabled because the "nothing to scroll" setting will make the
		// arrow buttons have NULL bounds.
		pTrackDrawInfo->enableState = kThemeTrackDisabled;
		pTrackDrawInfo->trackInfo.scrollbar.viewsize = 0;
		pTrackDrawInfo->trackInfo.scrollbar.pressState = 0;
	}
	if( pHITrackInfo )
	{
		pHITrackInfo->enableState = pTrackDrawInfo->enableState;
		pHITrackInfo->pressState = pTrackDrawInfo->trackInfo.scrollbar.pressState;
		pHITrackInfo->viewsize = pTrackDrawInfo->trackInfo.scrollbar.viewsize;
	}
	return TRUE;
}

// =======================================================================

/**
 * (static) Convert VCL progress bar codes into an HITheme progress bar
 * structure
 *
 * @param pTrackDrawInfo	HITheme progress bar structure
 * @param nState			control state of the progress bar (enabled vs. disabled)
 * @param bounds			drawing bounds of the progress bar
 * @param pProgressbarValue	optional value providing progress bar specific
 *							info
 * @param bSmall			TRUE to use small progress bar otherwise use large
 * @return TRUE on success, FALSE on failure
 */
static BOOL InitProgressbarTrackInfo( HIThemeTrackDrawInfo *pTrackDrawInfo, ControlState nState, Rectangle bounds, ProgressbarValue *pProgressbarValue, BOOL bSmall )
{
	static UInt8 phase = 0x1;

	memset( pTrackDrawInfo, 0, sizeof( HIThemeTrackDrawInfo ) );
	pTrackDrawInfo->version = 0;
	if ( bSmall )
		pTrackDrawInfo->kind = ( ( pProgressbarValue && pProgressbarValue->mbIndeterminate ) ? kThemeIndeterminateBarMedium : kThemeProgressBarMedium );
	else
		pTrackDrawInfo->kind = ( ( pProgressbarValue && pProgressbarValue->mbIndeterminate ) ? kThemeIndeterminateBarLarge : kThemeProgressBarLarge );
	pTrackDrawInfo->bounds.origin.x = 0;
	pTrackDrawInfo->bounds.origin.y = 0;
	pTrackDrawInfo->bounds.size.width = bounds.GetWidth();
	pTrackDrawInfo->bounds.size.height = bounds.GetHeight();
	if( bounds.GetWidth() > bounds.GetHeight() )
		pTrackDrawInfo->attributes |= kThemeTrackHorizontal;
	pTrackDrawInfo->enableState = kThemeTrackActive;
	pTrackDrawInfo->min = 0;
	pTrackDrawInfo->max = 100;
	if( pProgressbarValue )
	{
		pTrackDrawInfo->value = (int)pProgressbarValue->mdPercentComplete;
		if( pProgressbarValue->mbIndeterminate )
		{
			pTrackDrawInfo->trackInfo.progress.phase = phase++;
		}
	}
	return TRUE;
}

// =======================================================================

/**
 * (static) Convert VCL tab information structure into an HITheme tab
 * description structure.  This structure can be used to draw an
 * individual tab/segmented control into a window.
 *
 * @param pTabDrawInfo		pointer to HITheme tab drawing structure
 * @param nState			overall control state of the tab item.
 * @param pTabValue			VCL structure containing information about
 * 							the tab's position
 * @return TRUE on success, FALSE on failure
 */
static BOOL InitTabDrawInfo( HIThemeTabDrawInfo *pTabDrawInfo, ControlState nState, TabitemValue *pTabValue )
{
	memset( pTabDrawInfo, 0, sizeof( HIThemeTabDrawInfo ) );
	pTabDrawInfo->version = 1;
	if( nState & CTRL_STATE_SELECTED )
		pTabDrawInfo->style = kThemeTabFront;
	else if( nState & CTRL_STATE_PRESSED )
		pTabDrawInfo->style = kThemeTabNonFrontPressed;
	else if( nState & CTRL_STATE_ENABLED )
		pTabDrawInfo->style = kThemeTabNonFront;
	else
		pTabDrawInfo->style = kThemeTabNonFrontInactive;
	pTabDrawInfo->direction = kThemeTabNorth;	// always assume tabs are at top of tab controls in standard position
	pTabDrawInfo->size = kHIThemeTabSizeNormal;
	if( nState & CTRL_STATE_FOCUSED )
		pTabDrawInfo->adornment = kHIThemeTabAdornmentFocus;
	else
		pTabDrawInfo->adornment = kHIThemeTabAdornmentNone;
	pTabDrawInfo->kind = kHIThemeTabKindNormal;
	pTabDrawInfo->position = kHIThemeTabPositionMiddle;
	if( pTabValue )
	{
		if ( pTabValue->isFirst() && pTabValue->isLast() )
			pTabDrawInfo->position = kHIThemeTabPositionOnly;
		else if( pTabValue->isFirst() )
			pTabDrawInfo->position = kHIThemeTabPositionFirst;
		else if( pTabValue->isLast() )
			pTabDrawInfo->position = kHIThemeTabPositionLast;
	}
	switch( pTabDrawInfo->position )
	{
		case kHIThemeTabPositionMiddle:
		case kHIThemeTabPositionFirst:
			pTabDrawInfo->adornment |= kHIThemeTabAdornmentTrailingSeparator;
			break;
	}
	return TRUE;
}

// =======================================================================

/**
 * (static) Initialize the HITheme strucutre used to draw the bounding box
 * for a tab control background bounding box
 *
 * @param pTabPaneDrawInfo	pointer to HITheme tab drawing structure
 * @param nState			overall control state of the tab item.
 * @return TRUE on success, FALSE on failure
 */
static BOOL InitTabPaneDrawInfo( HIThemeTabPaneDrawInfo *pTabPaneDrawInfo, ControlState nState )
{
	memset( pTabPaneDrawInfo, 0, sizeof( HIThemeTabPaneDrawInfo ) );
	pTabPaneDrawInfo->version = 1;
	pTabPaneDrawInfo->direction = kThemeTabNorth;
	pTabPaneDrawInfo->size = kHIThemeTabSizeNormal;
	pTabPaneDrawInfo->adornment = kHIThemeTabAdornmentNone;
	if( ! ( nState & CTRL_STATE_ENABLED ) )
		pTabPaneDrawInfo->state = kThemeStateInactive;
	else
		pTabPaneDrawInfo->state = kThemeStateActive;
	return TRUE;
}

// =======================================================================

/**
 * (static) Initialize an HITheme groupbox draw structure to draw a primary
 * group box.
 *
 * @param pDrawInfo		pointer to the HITheme group box draw structure to
 *						initialize
 * @param nState		overall control state of the group box
 * @return TRUE on success, FALSE on failure
 */
static BOOL InitPrimaryGroupBoxDrawInfo( HIThemeGroupBoxDrawInfo *pDrawInfo, ControlState nState )
{
	memset( pDrawInfo, 0, sizeof( HIThemeGroupBoxDrawInfo ) );
	pDrawInfo->version = 0;
	if( ! ( nState & CTRL_STATE_ENABLED ) )
		pDrawInfo->state = kThemeStateInactive;
	else
		pDrawInfo->state = kThemeStateActive;
	pDrawInfo->kind = kHIThemeGroupBoxKindPrimary;
	return TRUE;
}

// =======================================================================

/**
 * (static) Initialize HITheme structures used to draw the frame of an
 * edit field.
 *
 * @param pFrameInfo		pointer to the HITheme frame info structure
 *							to be initialized
 * @param nState			control state of the edit field
 * @return TRUE on success, FALSE on failure
 */
static BOOL InitEditFieldDrawInfo( HIThemeFrameDrawInfo *pFrameInfo, ControlState nState )
{
	memset( pFrameInfo, 0, sizeof( HIThemeFrameDrawInfo ) );
	pFrameInfo->version = 0;
	pFrameInfo->kind = kHIThemeFrameTextFieldSquare;
	if( ! ( nState & CTRL_STATE_ENABLED ) )
		pFrameInfo->state = kThemeStateInactive;
	else
		pFrameInfo->state = kThemeStateActive;
	if( nState & CTRL_STATE_FOCUSED )
	{
		pFrameInfo->isFocused = true;
		pFrameInfo->state |= kThemeStateActive;	// logically we can't have a focused edit field that's inactive
	}
	return TRUE;
}

// =======================================================================

/**
 * (static) Initialize HITheme structures used to draw the frame of an
 * list box.
 *
 * @param pFrameInfo		pointer to the HITheme frame info structure
 *							to be initialized
 * @param nState			control state of the list box
 * @return TRUE on success, FALSE on failure
 */
static BOOL InitListBoxDrawInfo( HIThemeFrameDrawInfo *pFrameInfo, ControlState nState )
{
	memset( pFrameInfo, 0, sizeof( HIThemeFrameDrawInfo ) );
	pFrameInfo->version = 0;
	pFrameInfo->kind = kHIThemeFrameListBox;
	if( ! ( nState & CTRL_STATE_ENABLED ) )
		pFrameInfo->state = kThemeStateInactive;
	else
		pFrameInfo->state = kThemeStateActive;
	if( nState & CTRL_STATE_FOCUSED )
	{
		pFrameInfo->isFocused = true;
		pFrameInfo->state |= kThemeStateActive;
	}
	return TRUE;
}

// =======================================================================

/**
 * (static) Initialize HITheme structures used to draw a disclosure arrow
 *
 * @param pButtonInfo	pointer to HITheme button info structure to be
 *						initialized
 * @param nState		control state of the disclosure button
 * @param pValue		pointer to VCL disclosure button value structure
 * @return TRUE on success, FALSE on failure
 */
static BOOL InitDisclosureButtonDrawInfo( HIThemeButtonDrawInfo *pButtonInfo, ControlState nState, DisclosureBtnValue *pValue )
{
	memset( pButtonInfo, 0, sizeof( HIThemeButtonDrawInfo ) );
	pButtonInfo->version = 0;
	pButtonInfo->kind = kThemeDisclosureTriangle;
	if ( pValue->mnOpenCloseState == DISCLOSUREBTN_OPEN )
	{
		pButtonInfo->value = kThemeDisclosureDown;
	}
	else
	{
		if ( pValue->mnAlignment == DISCLOSUREBTN_ALIGN_LEFT )
			pButtonInfo->value = kThemeDisclosureRight; // if left of container, point to the right
		else
			pButtonInfo->value = kThemeDisclosureLeft;
	}
	if ( nState & ( CTRL_STATE_PRESSED | CTRL_STATE_SELECTED ) )
		pButtonInfo->state = kThemeStatePressed;
	else if ( nState & CTRL_STATE_ENABLED )
		pButtonInfo->state = kThemeStateActive;
	else
		pButtonInfo->state = kThemeStateInactive;
	return TRUE;
}

// =======================================================================

/**
 * (static) Initialize HITheme structures used to draw a list view header
 *
 * @param pButtonInfo	pointer to HITheme button info structure to be
 *						initialized
 * @param nState		control state of the disclosure button
 * @param pValue		pointer to VCL list header button value structure
 * @return TRUE on success, FALSE on failure
 */
static BOOL InitListViewHeaderButtonDrawInfo( HIThemeButtonDrawInfo *pButtonInfo, ControlState nState, ListViewHeaderValue *pValue )
{
	memset( pButtonInfo, 0, sizeof( HIThemeButtonDrawInfo ) );
	pButtonInfo->version = 0;
	pButtonInfo->kind = kThemeListHeaderButton;
	if ( pValue->mbPrimarySortColumn )
		pButtonInfo->value = kThemeButtonOn;
	else
		pButtonInfo->value = kThemeButtonOff;
	if ( nState & ( CTRL_STATE_PRESSED | CTRL_STATE_SELECTED ) )
		pButtonInfo->state = kThemeStatePressed;
	else if ( nState & CTRL_STATE_ENABLED )
		pButtonInfo->state = kThemeStateActive;
	else
		pButtonInfo->state = kThemeStateInactive;
	switch ( pValue->mnSortDirection )
	{
		case LISTVIEWHEADER_SORT_ASCENDING:
			pButtonInfo->adornment = kThemeAdornmentHeaderButtonSortUp;
			break;

		case LISTVIEWHEADER_SORT_DESCENDING:
			// default is to have downward pointing arrow
			break;

		default:
			// for unknown sort orders
			pButtonInfo->adornment = kThemeAdornmentHeaderButtonNoSortArrow;
			break;
	}
	return TRUE;
}

// =======================================================================

/**
 * (static) Initialize HITheme structures used to draw a separator line
 *
 * @param pSepInfo	pointer to HITheme separator info structure to be
 *						initialized
 * @param nState		control state of the separator
 * @return TRUE on success, FALSE on failure
 */
static BOOL InitSeparatorDrawInfo( HIThemeSeparatorDrawInfo *pSepInfo, ControlState nState )
{
	memset( pSepInfo, 0, sizeof( HIThemeSeparatorDrawInfo ) );
	pSepInfo->version = 0;
	if ( nState & CTRL_STATE_ENABLED )
		pSepInfo->state = kThemeStateActive;
	else
		pSepInfo->state = kThemeStateInactive;
	return TRUE;
}

// =======================================================================

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
 * @param rCaption		text used for the control.  Presently ignored
 *				as we draw only the frame and let VCL draw
 *				the text
 * @return TRUE if successful, FALSE on error
 */
static BOOL DrawNativeComboBox( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState, const OUString& rCaption )
{
	VCLBitmapBuffer *pBuffer = &aSharedComboBoxBuffer;
	BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight(), pGraphics );
	if ( bRet )
	{
		if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
			nState = 0;

		HIThemeButtonDrawInfo aButtonDrawInfo;
		InitButtonDrawInfo( &aButtonDrawInfo, nState );

		HIRect destRect;
		destRect.origin.x = FOCUSRING_WIDTH;
		destRect.origin.y = FOCUSRING_WIDTH + COMBOBOX_BUTTON_TRIMHEIGHT;
		destRect.size.width = rDestBounds.GetWidth() - COMBOBOX_BUTTON_TRIMWIDTH - ( FOCUSRING_WIDTH * 2 );
		destRect.size.height = rDestBounds.GetHeight() - ( FOCUSRING_WIDTH * 2 );

		if ( COMBOBOX_BUTTON_HEIGHT_SLOP )
		{
			CGContextSaveGState( pBuffer->maContext );
			CGContextClipToRect( pBuffer->maContext, CGRectMake( 0, 0, rDestBounds.GetWidth() - COMBOBOX_BUTTON_WIDTH, rDestBounds.GetWidth() ) );
			bRet = ( pHIThemeDrawButton( &destRect, &aButtonDrawInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags, NULL ) == noErr );
			CGContextRestoreGState( pBuffer->maContext );

			if ( bRet )
			{
				CGContextSaveGState( pBuffer->maContext );
				CGContextClipToRect( pBuffer->maContext, CGRectMake( rDestBounds.GetWidth() - COMBOBOX_BUTTON_WIDTH, 0, COMBOBOX_BUTTON_WIDTH, rDestBounds.GetWidth() ) );
				destRect.origin.y += COMBOBOX_BUTTON_HEIGHT_SLOP;
				bRet = ( pHIThemeDrawButton( &destRect, &aButtonDrawInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags, NULL ) == noErr );
				CGContextRestoreGState( pBuffer->maContext );
			}
		}
		else
		{
			bRet = ( pHIThemeDrawButton( &destRect, &aButtonDrawInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags, NULL ) == noErr );
		}
	}

	pBuffer->ReleaseContext();

	if ( bRet )
		pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), rDestBounds.GetHeight() ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );

	return bRet;
}

// =======================================================================

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
 * @param rCaption		text used for the control.  Presently ignored
 *				as we draw only the frame and let VCL draw
 *				the text
 * @return TRUE if successful, FALSE on error
 */
static BOOL DrawNativeListBox( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState, const OUString& rCaption )
{
	VCLBitmapBuffer *pBuffer = &aSharedListBoxBuffer;
	BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight(), pGraphics );
	if ( bRet )
	{
		if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
			nState = 0;

		HIThemeButtonDrawInfo aButtonDrawInfo;
		InitButtonDrawInfo( &aButtonDrawInfo, nState );
		aButtonDrawInfo.kind = kThemePopupButton;

		HIRect destRect;
		destRect.origin.x = LISTBOX_BUTTON_HORIZ_TRIMWIDTH + FOCUSRING_WIDTH;
		destRect.origin.y = LISTBOX_BUTTON_VERT_TRIMWIDTH + FOCUSRING_WIDTH;
		destRect.size.width = rDestBounds.GetWidth() - LISTBOX_BUTTON_HORIZ_TRIMWIDTH - ( FOCUSRING_WIDTH * 2 );
		destRect.size.height = rDestBounds.GetHeight() - LISTBOX_BUTTON_VERT_TRIMWIDTH - ( FOCUSRING_WIDTH * 2 );
		bRet = ( pHIThemeDrawButton( &destRect, &aButtonDrawInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags, NULL ) == noErr );
	}

	pBuffer->ReleaseContext();

	if ( bRet )
		pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), rDestBounds.GetHeight() ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );

	return TRUE;
}

// =======================================================================

/**
 * (static) Draw a scrollbar into the graphics port at the specified location.
 * Swing scrollbars can lead to some odd tracking issues and don't provide
 * easy access to all of the individual subparts of the control proper,
 * so we'll draw them using HIThemes and SalBitmaps.
 *
 * @param pGraphics			pointer to the destination graphics where we'll be drawing
 * @param rDestBounds		eventual destination rectangle for the scrollbar
 * @param nState			overall scrollbar state (active vs. disabled)
 * @param pScrollbarValue	VCL scrollbar info value
 * @return TRUE if successful, FALSE on error
 */
static BOOL DrawNativeScrollBar( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState, ScrollbarValue *pScrollbarValue )
{
	BOOL bHorizontal = FALSE;
	VCLBitmapBuffer *pBuffer;
	if ( rDestBounds.GetWidth() > rDestBounds.GetHeight() )
	{
		pBuffer = &aSharedHorizontalScrollBarBuffer;
		bHorizontal = TRUE;
	}
	else
	{
		pBuffer = &aSharedVerticalScrollBarBuffer;
	}

	BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight(), pGraphics );
	if ( bRet )
	{
		if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
			nState = 0;

		// Fix bug 2031 by always filling the background with white
		HIRect destRect;
		destRect.origin.x = 0;
		destRect.origin.y = 0;
		destRect.size.width = rDestBounds.GetWidth();
		destRect.size.height = rDestBounds.GetHeight();
		float whiteColor[] = { 1.0, 1.0, 1.0, 1.0 };
		CGContextSetFillColor( pBuffer->maContext, whiteColor );
		CGContextFillRect( pBuffer->maContext, destRect );

		HIThemeTrackDrawInfo pTrackDrawInfo;
		InitScrollBarTrackInfo( &pTrackDrawInfo, NULL, nState, rDestBounds, pScrollbarValue );
		if ( bHorizontal )
			pTrackDrawInfo.bounds.size.width -= SCROLLBAR_WIDTH_SLOP;
		else
			pTrackDrawInfo.bounds.origin.x += SCROLLBAR_WIDTH_SLOP;

		// Fix bug 3359 by drawing disabled scrollbar as nothing to scroll
		if ( pTrackDrawInfo.enableState == kThemeTrackDisabled )
			pTrackDrawInfo.enableState = kThemeTrackNothingToScroll;

		bRet = ( pHIThemeDrawTrack( &pTrackDrawInfo, NULL, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags ) == noErr );
	}

	pBuffer->ReleaseContext();

	if ( bRet )
		pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), rDestBounds.GetHeight() ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );

	return bRet;
}

// =======================================================================

/**
 * (static) Draw a spinbox using native controls.  This consists of a set of
 * stepper arrows on the right portion of the control and an edit field on
 * the left.
 *
 * @param pGraphics			pointer into graphics object where spinbox should
 *							be painted
 * @param rDestBounds		destination rectangle where object will be painted
 * @param nState			overall control state
 * @param pValue			optional value giving enabled & pressed state for
 *							subcontrols.
 * @return TRUE if drawing successful, FALSE if not
 */
static BOOL DrawNativeSpinbox( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState, SpinbuttonValue *pValue )
{
	SInt32 spinnerThemeWidth;
	SInt32 spinnerThemeHeight;
	BOOL bRet = ( pGetThemeMetric( kThemeMetricLittleArrowsWidth, &spinnerThemeWidth ) == noErr && pGetThemeMetric( kThemeMetricLittleArrowsHeight, &spinnerThemeHeight) == noErr );
	if ( bRet )
	{
		spinnerThemeHeight += SPINNER_TRIMHEIGHT * 2;
		long offscreenHeight = ( ( rDestBounds.GetHeight() > spinnerThemeHeight ) ? rDestBounds.GetHeight() : spinnerThemeHeight );

		VCLBitmapBuffer *pBuffer = &aSharedSpinboxBuffer;
		bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), offscreenHeight, pGraphics, offscreenHeight == rDestBounds.GetHeight() );
		if ( bRet )
		{
			if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
				nState = 0;

			HIThemeButtonDrawInfo aButtonDrawInfo;
			InitSpinbuttonDrawInfo( &aButtonDrawInfo, nState, pValue );

			HIRect arrowRect;
			arrowRect.origin.x = rDestBounds.GetWidth() - spinnerThemeWidth - SPINNER_TRIMWIDTH;
			if( arrowRect.origin.x < 0 )
				arrowRect.origin.x = 0;
			arrowRect.origin.y = ( ( offscreenHeight - spinnerThemeHeight ) / 2 ) - SPINNER_TRIMHEIGHT;
			arrowRect.size.width = spinnerThemeWidth + ( SPINNER_TRIMWIDTH * 2 );
			arrowRect.size.height = spinnerThemeHeight;

			bRet = ( pHIThemeDrawButton( &arrowRect, &aButtonDrawInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags, NULL ) == noErr );

			if( bRet )
			{
				HIRect editRect;
				editRect.origin.x = FOCUSRING_WIDTH;
				editRect.origin.y = FOCUSRING_WIDTH;
				editRect.size.width = rDestBounds.GetWidth() - arrowRect.size.width - ( FOCUSRING_WIDTH * 2 );
				editRect.size.height = offscreenHeight - ( FOCUSRING_WIDTH * 2 );

				// erase out our background first

				float whiteColor[] = { 1.0, 1.0, 1.0, 1.0 };
				CGContextSetFillColor( pBuffer->maContext, whiteColor );
				CGContextFillRect( pBuffer->maContext, editRect );

				// draw our edit frame

				HIThemeFrameDrawInfo aFrameInfo;
				memset( &aFrameInfo, 0, sizeof( HIThemeFrameDrawInfo ) );

				aFrameInfo.kind = kHIThemeFrameTextFieldSquare;
				if( ! nState )
					aFrameInfo.state = kThemeStateInactive;
				else
					aFrameInfo.state = kThemeStateActive;
				if( nState & CTRL_STATE_FOCUSED )
					aFrameInfo.isFocused = TRUE;
				else
					aFrameInfo.isFocused = FALSE;

				bRet = ( pHIThemeDrawFrame( &editRect, &aFrameInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags ) == noErr );
			}
		}

		pBuffer->ReleaseContext();

		if ( bRet )
			pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), offscreenHeight ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );
	}

	return bRet;
}

// =======================================================================

/**
 * (static) Draw a spinbutton using Aqua control scaling.  This is a set of
 * stepper arrows 
 *
 * @param pGraphics			pointer into graphics object where spinbutton should
 *							be painted
 * @param rDestBounds		destination rectangle where object will be painted
 * @param nState			overall control state
 * @param pValue			optional value giving enabled & pressed state for
 *							subcontrols.
 * @return TRUE if drawing successful, FALSE if not
 */
static BOOL DrawNativeSpinbutton( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState, SpinbuttonValue *pValue )
{
	SInt32 spinnerThemeWidth;
	SInt32 spinnerThemeHeight;
	BOOL bRet = ( pGetThemeMetric( kThemeMetricLittleArrowsWidth, &spinnerThemeWidth ) == noErr && pGetThemeMetric( kThemeMetricLittleArrowsHeight, &spinnerThemeHeight) == noErr );
	if ( bRet )
	{
		spinnerThemeHeight += SPINNER_TRIMHEIGHT * 2;
		long offscreenHeight = ( ( rDestBounds.GetHeight() > spinnerThemeHeight ) ? rDestBounds.GetHeight() : spinnerThemeHeight );

		VCLBitmapBuffer *pBuffer = &aSharedSpinbuttonBuffer;
		bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), offscreenHeight, pGraphics, offscreenHeight == rDestBounds.GetHeight() );
		if ( bRet )
		{
			if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
				nState = 0;

			HIThemeButtonDrawInfo aButtonDrawInfo;
			InitSpinbuttonDrawInfo( &aButtonDrawInfo, nState, pValue );

			HIRect arrowRect;
			arrowRect.origin.x = rDestBounds.GetWidth() - spinnerThemeWidth - SPINNER_TRIMWIDTH;
			if( arrowRect.origin.x < 0 )
				arrowRect.origin.x = 0;
			arrowRect.origin.y = ( ( offscreenHeight - spinnerThemeHeight ) / 2 ) - SPINNER_TRIMHEIGHT;
			arrowRect.size.width = spinnerThemeWidth + ( SPINNER_TRIMWIDTH * 2 );
			arrowRect.size.height = spinnerThemeHeight;

			bRet = ( pHIThemeDrawButton( &arrowRect, &aButtonDrawInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags, NULL ) == noErr );
		}

		pBuffer->ReleaseContext();

		if ( bRet )
			pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), offscreenHeight ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );
	}

	return bRet;
}

// =======================================================================

/**
 * (static) Draw a progress bar using the native widget appearance.  The
 * progressbar indicates percentage of task completion.
 *
 * @param pGraphics			pointer into graphics object where spinbox should
 *							be painted
 * @param rDestBounds		destination rectangle where object will be painted
 * @param nState			overall control state
 * @param pValue			value providing the percentage completion and other
 *							progressbar state
 * @param bSmall			TRUE to use small progress bar otherwise use large
 * @return TRUE if drawing successful, FALSE if not
 */
static BOOL DrawNativeProgressbar( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState, ProgressbarValue *pValue, BOOL bSmall )
{
	VCLBitmapBuffer *pBuffer = &aSharedProgressbarBuffer;
	BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight(), pGraphics );
	if ( bRet )
	{
		if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
			nState = 0;

		HIThemeTrackDrawInfo aTrackDrawInfo;
		InitProgressbarTrackInfo( &aTrackDrawInfo, nState, rDestBounds, pValue, bSmall );

		HIRect destRect;
		destRect.origin.x = 0;
		destRect.origin.y = 0;
		destRect.size.width = rDestBounds.GetWidth();
		destRect.size.height = rDestBounds.GetHeight();

		// clear the background of the control with the fill color
		CGColorRef aFillColor = CreateCGColorFromSalColor( pGraphics->mnFillColor );
		if ( aFillColor )
		{
			CGContextSetFillColorWithColor( pBuffer->maContext, aFillColor );
			CGContextFillRect( pBuffer->maContext, destRect );
			CGColorRelease( aFillColor );
		}

		bRet = ( pHIThemeDrawTrack( &aTrackDrawInfo, NULL, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags ) == noErr );
	}

	pBuffer->ReleaseContext();

	if ( bRet )
		pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), rDestBounds.GetHeight() ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );

	return bRet;
}

// =======================================================================

/**
 * (static) Draw an individual tab control using the native widget appearance.
 * The tab consists of only a single element in a list of tabs that appear
 * at the top of a full tab group.  Each tab gets drawn independently.
 *
 * @param pGraphics			pointer into graphics object where spinbox should
 *							be painted
 * @param rDestBounds		destination rectangle where object will be painted
 * @param nState			overall control state
 * @param pValue			value providing the tab style, its position in
 *							the tab order relative to other tabs, and other
 *							tab-specific values
 * @return TRUE if drawing successful, FALSE if not
 */
static BOOL DrawNativeTab( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState, TabitemValue *pValue )
{
	VCLBitmapBuffer *pBuffer = &aSharedTabBuffer;
	BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight(), pGraphics );
	if ( bRet )
	{
		if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
			nState = 0;

		HIThemeTabDrawInfo pTabDrawInfo;
		InitTabDrawInfo( &pTabDrawInfo, nState, pValue );

		HIRect destRect;
		destRect.origin.x = 0;
		destRect.origin.y = 0;
		destRect.size.width = rDestBounds.GetWidth();
		destRect.size.height = rDestBounds.GetHeight();

		HIRect labelRect; // ignored

		bRet = ( pHIThemeDrawTab( &destRect, (HIThemeTabDrawInfo *)&pTabDrawInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags, &labelRect ) == noErr );
	}

	pBuffer->ReleaseContext();

	if ( bRet )
		pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), rDestBounds.GetHeight() ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );

	return bRet;
}

// =======================================================================

/**
 * (static) Draw a native tab box.  This includes the rounded boundary and
 * flat grey tab background.
 *
 * @param pGraphics			pointer into graphics object where spinbox should
 *							be painted
 * @param rDestBounds		destination rectangle where object will be painted
 * @param nState			overall control state
 */
static BOOL DrawNativeTabBoundingBox( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState )
{
	VCLBitmapBuffer *pBuffer = &aSharedTabBoundingBoxBuffer;
	BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight(), pGraphics );
	if ( bRet )
	{
		if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
			nState = 0;

		HIThemeTabPaneDrawInfo pTabPaneDrawInfo;
		InitTabPaneDrawInfo( &pTabPaneDrawInfo, nState );

		HIRect destRect;
		destRect.origin.x = 0;
		destRect.origin.y = 0;
		destRect.size.width = rDestBounds.GetWidth();
		destRect.size.height = rDestBounds.GetHeight();

		bRet = ( pHIThemeDrawTabPane( &destRect, (HIThemeTabPaneDrawInfo *)&pTabPaneDrawInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags ) == noErr );
	}

	pBuffer->ReleaseContext();

	if ( bRet )
		pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), rDestBounds.GetHeight() ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );

	return bRet;
}

// =======================================================================

/**
 * (static) Draw a primary group box.  This includes the rounded boundary used
 * to group controls together.
 *
 * @param pGraphics			pointer into graphics object where box should
 *							be painted
 * @param rDestBounds		destination rectangle where object will be painted
 * @param nState			overall control state
 */
static BOOL DrawNativePrimaryGroupBox( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState )
{
	VCLBitmapBuffer *pBuffer = &aSharedPrimaryGroupBoxBuffer;
	BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight(), pGraphics );
	if ( bRet )
	{
		if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
			nState = 0;

		HIThemeGroupBoxDrawInfo pGroupBoxDrawInfo;
		InitPrimaryGroupBoxDrawInfo( &pGroupBoxDrawInfo, nState );

		HIRect destRect;
		destRect.origin.x = 0;
		destRect.origin.y = 0;
		destRect.size.width = rDestBounds.GetWidth();
		destRect.size.height = rDestBounds.GetHeight();

		bRet = ( pHIThemeDrawGroupBox( &destRect, &pGroupBoxDrawInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags ) == noErr );
	}

	pBuffer->ReleaseContext();

	if ( bRet )
		pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), rDestBounds.GetHeight() ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );

	return bRet;
}

// =======================================================================

/**
 * (static) Draw the background for a native menu.
 *
 * @param pGraphics			pointer into graphics object where box should
 *							be painted
 * @param rDestBounds		destination rectangle where object will be painted
 */
static BOOL DrawNativeMenuBackground( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds )
{
	VCLBitmapBuffer *pBuffer = &aSharedMenuBackgroundBuffer;
	BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight(), pGraphics );
	if ( bRet )
	{
		HIThemeMenuDrawInfo pMenuDrawInfo;
		memset( &pMenuDrawInfo, 0, sizeof( pMenuDrawInfo ) );
		pMenuDrawInfo.version = 0;
		pMenuDrawInfo.menuType = kThemeMenuTypePopUp;

		HIRect destRect;
		destRect.origin.x = 0;
		destRect.origin.y = 0;
		destRect.size.width = rDestBounds.GetWidth();
		destRect.size.height = rDestBounds.GetHeight();

		bRet = ( pHIThemeDrawMenuBackground( &destRect, &pMenuDrawInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags ) == noErr );
	}

	pBuffer->ReleaseContext();

	if ( bRet )
		pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), rDestBounds.GetHeight() ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );

	return bRet;
}

// =======================================================================

/**
 * (static) Draw the background for a native edit field.
 *
 * @param pGraphics			pointer into graphics object where box should
 *							be painted
 * @param rDestBounds		destination rectangle where object will be painted
 */
static BOOL DrawNativeEditBox( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState )
{
	VCLBitmapBuffer *pBuffer = &aSharedEditBoxBuffer;
	BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight(), pGraphics );
	if ( bRet )
	{
		if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
			nState = 0;

		HIThemeFrameDrawInfo pFrameInfo;
		InitEditFieldDrawInfo( &pFrameInfo, nState );

		HIRect destRect;
		destRect.origin.x = FOCUSRING_WIDTH;
		destRect.origin.y = FOCUSRING_WIDTH;
		destRect.size.width = rDestBounds.GetWidth() - 2*FOCUSRING_WIDTH;
		destRect.size.height = rDestBounds.GetHeight() - 2*FOCUSRING_WIDTH;

		// clear the active editing portion of the control
		float whiteColor[] = { 1.0, 1.0, 1.0, 1.0 };
		CGContextSetFillColor( pBuffer->maContext, whiteColor );
		CGContextFillRect( pBuffer->maContext, destRect );
		// draw frame around the background
		bRet = ( pHIThemeDrawFrame( &destRect, &pFrameInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags ) == noErr );
	}

	pBuffer->ReleaseContext();

	if ( bRet )
		pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), rDestBounds.GetHeight() ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );

	return bRet;
}

// =======================================================================

/**
 * (static) Draw the background for a native lis view widget.
 *
 * @param pGraphics			pointer into graphics object where box should
 *							be painted
 * @param rDestBounds		destination rectangle where object will be painted
 */
static BOOL DrawNativeListBoxFrame( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState )
{
	VCLBitmapBuffer *pBuffer = &aSharedListViewFrameBuffer;
	BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight(), pGraphics );
	if ( bRet )
	{
		if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
			nState = 0;

		HIThemeFrameDrawInfo pFrameInfo;
		InitListBoxDrawInfo( &pFrameInfo, nState );

		HIRect destRect;
		destRect.origin.x = LISTVIEWFRAME_TRIMWIDTH;
		destRect.origin.y = LISTVIEWFRAME_TRIMWIDTH;
		destRect.size.width = rDestBounds.GetWidth() - 2*LISTVIEWFRAME_TRIMWIDTH;
		destRect.size.height = rDestBounds.GetHeight() - 2*LISTVIEWFRAME_TRIMWIDTH;

		bRet = ( pHIThemeDrawFrame( &destRect, &pFrameInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags ) == noErr );
	}

	pBuffer->ReleaseContext();

	if ( bRet )
		pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), rDestBounds.GetHeight() ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );

	return bRet;
}

// =======================================================================

/**
 * (static) Draw a native disclosure button used on the side of items in
 * hierarchical lists that can collapse and expand a container.
 *
 * @param pGraphics		pointer to the graphics object where the button should
 *						be painted
 * @param rDestBounds	destination drawing rectangle for the disclosure button
 * @param nState		current control enabled/disabled/focused state
 * @param pValue		NWF structure providing additional disclosure
 *						button state including alignment and collapsed/expanded
 *						state
 * @return OK if successful, FALSE on failure
 */
static BOOL DrawNativeDisclosureBtn( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState, DisclosureBtnValue *pValue )
{
	VCLBitmapBuffer *pBuffer = &aSharedDisclosureBtnBuffer;
	BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight(), pGraphics );
	if ( bRet )
	{
		if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
			nState = 0;

		HIThemeButtonDrawInfo pButtonInfo;
		InitDisclosureButtonDrawInfo( &pButtonInfo, nState, pValue );

		HIRect destRect;
		destRect.origin.x = 0;
		destRect.origin.y = 0;
		destRect.size.width = rDestBounds.GetWidth();
		destRect.size.height = rDestBounds.GetHeight();

		bRet = ( pHIThemeDrawButton( &destRect, &pButtonInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags, NULL ) == noErr );
	}

	pBuffer->ReleaseContext();

	if ( bRet )
		pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), rDestBounds.GetHeight() ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );

	return bRet;
}

// =======================================================================

/**
 * (static) Draw a native separator line.
 *
 * @param pGraphics		pointer to the graphics object where the line should
 *						be painted
 * @param rDestBounds	destination drawing rectangle for the separator
 * @param nState		current control enabled/disabled/focused state.
 *						really, this is meaningless for separators but is used
 *						by both VCL and Carbon, so we go for it
 */
static BOOL DrawNativeSeparatorLine( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState )
{
	VCLBitmapBuffer *pBuffer = &aSharedSeparatorLineBuffer;
	BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight(), pGraphics );
	if ( bRet )
	{
		if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
			nState = 0;

		HIThemeSeparatorDrawInfo pSepInfo;
		InitSeparatorDrawInfo( &pSepInfo, nState );

		HIRect destRect;
		destRect.origin.x = 0;
		destRect.origin.y = 0;
		destRect.size.width = rDestBounds.GetWidth();
		destRect.size.height = rDestBounds.GetHeight();

		bRet = ( pHIThemeDrawSeparator( &destRect, &pSepInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags ) == noErr );
	}

	pBuffer->ReleaseContext();

	if ( bRet )
		pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), rDestBounds.GetHeight() ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );

	return bRet;
}

// =======================================================================

/**
 * (static) Draw a native header button for a list view.  The header buttons
 * are drawn using the theme brushes for primary sort columns and non-primary
 * columns.  Sort indicators are not drawn at present.
 *
 * @param pGraphics		pointer to the graphics object where the button should
 *						be painted
 * @param rDestBounds	destination drawing rectangle for the disclosure button
 * @param nState		current control enabled/disabled/focused state
 * @param pValue		NWF structure providing information about primary
 *						sort column and additional sort order state
 */
static BOOL DrawNativeListViewHeader( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState, ListViewHeaderValue *pValue )
{
	SInt32 themeListViewHeaderHeight;
	BOOL bRet = ( pGetThemeMetric( kThemeMetricListHeaderHeight, &themeListViewHeaderHeight ) == noErr );

	if ( bRet )
	{
		VCLBitmapBuffer *pBuffer = &aSharedListViewHeaderBuffer;
		bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), themeListViewHeaderHeight, pGraphics );
		if ( bRet )
		{
			if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
				nState = 0;

			HIThemeButtonDrawInfo pButtonInfo;
			InitListViewHeaderButtonDrawInfo( &pButtonInfo, nState, pValue );

			HIRect destRect;
			destRect.origin.x = 0;
			destRect.origin.y = 0;
			destRect.size.width = rDestBounds.GetWidth();
			destRect.size.height = themeListViewHeaderHeight;

			bRet = ( pHIThemeDrawButton( &destRect, &pButtonInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags, NULL ) == noErr );
		}

		pBuffer->ReleaseContext();

		if ( bRet )
			pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), themeListViewHeaderHeight ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );
	}

	return bRet;
}

// =======================================================================

/**
 * (static) Draw a native bevel button frame.  This is used for buttons in
 * toolbars.
 *
 * @param pGraphics		pointer to the graphics object where the button should
 *						be painted
 * @param rDestBounds	destination drawing rectangle for the disclosure button
 * @param nState		current control enabled/disabled/focused state
 * @param aValue		control value
 */
static BOOL DrawNativeBevelButton( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState, const ImplControlValue& aValue )
{
	VCLBitmapBuffer *pBuffer = &aSharedBevelButtonBuffer;
	BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight(), pGraphics );
	if ( bRet )
	{
		if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
			nState = 0;

		HIThemeButtonDrawInfo aButtonDrawInfo;
		InitButtonDrawInfo( &aButtonDrawInfo, nState );

		aButtonDrawInfo.kind = kThemeBevelButton;
		if ( aValue.getTristateVal() == BUTTONVALUE_ON )
			aButtonDrawInfo.value = kThemeButtonOn;

		HIRect destRect;
		destRect.origin.x = 0;
		destRect.origin.y = 0;
		destRect.size.width = rDestBounds.GetWidth();
		destRect.size.height = rDestBounds.GetHeight();
		bRet = ( pHIThemeDrawButton( &destRect, &aButtonDrawInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags, NULL ) == noErr );
	}

	pBuffer->ReleaseContext();

	if ( bRet )
		pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), rDestBounds.GetHeight() ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );

	return bRet;
}

// =======================================================================

/**
 * (static) Draw a native checkbox.
 *
 * @param pGraphics		pointer to the graphics object where the checkbox should
 *						be painted
 * @param rDestBounds	destination drawing rectangle for the checkbox
 * @param nState		current control enabled/disabled/focused state
 * @param aValue		control value
 */
static BOOL DrawNativeCheckbox( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState, const ImplControlValue& aValue )
{
	VCLBitmapBuffer *pBuffer = &aSharedCheckboxBuffer;
	BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight(), pGraphics );
	if ( bRet )
	{
		if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
			nState = 0;

		HIThemeButtonDrawInfo aButtonDrawInfo;
		InitButtonDrawInfo( &aButtonDrawInfo, nState );

		if ( rDestBounds.GetWidth() < CHECKBOX_WIDTH - ( FOCUSRING_WIDTH * 2 ) || rDestBounds.GetHeight() < CHECKBOX_HEIGHT - ( FOCUSRING_WIDTH * 2 ) )
			aButtonDrawInfo.kind = kThemeCheckBoxSmall;
		else
			aButtonDrawInfo.kind = kThemeCheckBox;
		if ( aValue.getTristateVal() == BUTTONVALUE_ON )
			aButtonDrawInfo.value = kThemeButtonOn;
		else if ( aValue.getTristateVal() == BUTTONVALUE_MIXED )
			aButtonDrawInfo.value = kThemeButtonMixed;

		HIRect destRect;
		destRect.origin.x = FOCUSRING_WIDTH;
		destRect.origin.y = FOCUSRING_WIDTH;
		destRect.size.width = rDestBounds.GetWidth() - ( FOCUSRING_WIDTH * 2 );
		destRect.size.height = rDestBounds.GetHeight() - ( FOCUSRING_WIDTH * 2 );
		bRet = ( pHIThemeDrawButton( &destRect, &aButtonDrawInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags, NULL ) == noErr );
	}

	pBuffer->ReleaseContext();

	if ( bRet )
		pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), rDestBounds.GetHeight() ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );

	return bRet;
}

// =======================================================================

/**
 * (static) Draw a native radio button.
 *
 * @param pGraphics		pointer to the graphics object where the button should
 *						be painted
 * @param rDestBounds	destination drawing rectangle for the button
 * @param nState		current control enabled/disabled/focused state
 * @param aValue		control value
 */
static BOOL DrawNativeRadioButton( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState, const ImplControlValue& aValue )
{
	VCLBitmapBuffer *pBuffer = &aSharedCheckboxBuffer;
	BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight(), pGraphics );
	if ( bRet )
	{
		if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
			nState = 0;

		HIThemeButtonDrawInfo aButtonDrawInfo;
		InitButtonDrawInfo( &aButtonDrawInfo, nState );

		if ( rDestBounds.GetWidth() < RADIOBUTTON_WIDTH - ( FOCUSRING_WIDTH * 2 ) || rDestBounds.GetHeight() < RADIOBUTTON_HEIGHT - ( FOCUSRING_WIDTH * 2 ) )
			aButtonDrawInfo.kind = kThemeRadioButtonSmall;
		else
			aButtonDrawInfo.kind = kThemeRadioButton;
		if ( aValue.getTristateVal() == BUTTONVALUE_ON )
			aButtonDrawInfo.value = kThemeButtonOn;
		else if ( aValue.getTristateVal() == BUTTONVALUE_MIXED )
			aButtonDrawInfo.value = kThemeButtonMixed;

		HIRect destRect;
		destRect.origin.x = FOCUSRING_WIDTH;
		destRect.origin.y = FOCUSRING_WIDTH;
		destRect.size.width = rDestBounds.GetWidth() - ( FOCUSRING_WIDTH * 2 );
		destRect.size.height = rDestBounds.GetHeight() - ( FOCUSRING_WIDTH * 2 );
		bRet = ( pHIThemeDrawButton( &destRect, &aButtonDrawInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags, NULL ) == noErr );
	}

	pBuffer->ReleaseContext();

	if ( bRet )
		pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), rDestBounds.GetHeight() ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );

	return bRet;
}

// =======================================================================

/**
 * (static) Draw a native push button.
 *
 * @param pGraphics		pointer to the graphics object where the button should
 *						be painted
 * @param rDestBounds	destination drawing rectangle for the button
 * @param nState		current control enabled/disabled/focused state
 * @param aValue		control value
 */
static BOOL DrawNativePushButton( JavaSalGraphics *pGraphics, const Rectangle& rDestBounds, ControlState nState, const ImplControlValue& aValue )
{
    SInt32 pushButtonThemeHeight;
    BOOL bRet = ( pGetThemeMetric( kThemeMetricPushButtonHeight, &pushButtonThemeHeight) == noErr );
	if ( bRet )
	{
		pushButtonThemeHeight += FOCUSRING_WIDTH * 2;
		long offscreenHeight = ( rDestBounds.GetHeight() > pushButtonThemeHeight ? rDestBounds.GetHeight() : pushButtonThemeHeight );
		bool bPlacard = ( offscreenHeight >= rDestBounds.GetWidth() - 1 );
		if ( bPlacard )
			offscreenHeight = rDestBounds.GetHeight();

		VCLBitmapBuffer *pBuffer = &aSharedCheckboxBuffer;
		BOOL bRet = pBuffer->Create( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), offscreenHeight, pGraphics, offscreenHeight == rDestBounds.GetHeight() );
		if ( bRet )
		{
			if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
				nState = 0;

			HIThemeButtonDrawInfo aButtonDrawInfo;
			InitButtonDrawInfo( &aButtonDrawInfo, nState );

			// Detect placard buttons
			if ( bPlacard )
			{
				aButtonDrawInfo.kind = kThemeBevelButton;
			}
			else
			{
				aButtonDrawInfo.kind = kThemePushButton;

				// The default adornment hides the pressed state so set the
				// adornment to none if the button is pressed. Also, push
				// buttons should never have a focus ring so treat them as
				// default buttons.
				if ( nState & CTRL_STATE_DEFAULT || aButtonDrawInfo.adornment == kThemeAdornmentFocus )
				{
					if ( aButtonDrawInfo.state == kThemeStatePressed )
						aButtonDrawInfo.adornment = kThemeAdornmentNone;
					else
						aButtonDrawInfo.adornment = kThemeAdornmentDefault;
				}
			}

			if ( aValue.getTristateVal() == BUTTONVALUE_ON )
				aButtonDrawInfo.value = kThemeButtonOn;
			else if ( aValue.getTristateVal() == BUTTONVALUE_MIXED )
				aButtonDrawInfo.value = kThemeButtonMixed;

			HIRect destRect;
			if ( bPlacard )
			{
				destRect.origin.x = 0;
				destRect.origin.y = 0;
				destRect.size.width = rDestBounds.GetWidth() - 1;
				destRect.size.height = offscreenHeight - 1;
			}
			else
			{
				// Fix bug 1633 by vertically centering button
				destRect.origin.x = FOCUSRING_WIDTH;
				destRect.origin.y = ( ( offscreenHeight - pushButtonThemeHeight ) / 2 ) + PUSHBUTTON_HEIGHT_SLOP;
				destRect.size.width = rDestBounds.GetWidth() - ( FOCUSRING_WIDTH * 2 );
				destRect.size.height = pushButtonThemeHeight - ( FOCUSRING_WIDTH * 2 );
			}

			bRet = ( pHIThemeDrawButton( &destRect, &aButtonDrawInfo, pBuffer->maContext, pBuffer->mnHIThemeOrientationFlags, NULL ) == noErr );
		}

		pBuffer->ReleaseContext();

		if ( bRet )
			pBuffer->DrawContextAndDestroy( pGraphics, CGRectMake( 0, 0, rDestBounds.GetWidth(), offscreenHeight ), CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) );
	}

	return bRet;
}

// =======================================================================

/**
 * (static) Exclude the window's grow box region from the specified region if
 * the window has a grow box.
 *
 * @param pGraphics		pointer to the graphics object where the region should
 *						be painted
 * @param nType         control flavor that is requsted to be drawn
 * @param rControlRegion the control's region
 */
static const Region GetRegionAdjustedForGrowBox( JavaSalGraphics *pGraphics, ControlType nType, const Region &rControlRegion )
{
	Region aRegion( rControlRegion );

	// Only adjust for grow box on pre-Mac OS X 10.7 releases
	if ( ( IsRunningLeopard() || IsRunningSnowLeopard() ) && pGraphics->mpFrame && pGraphics->mpFrame->mnStyle & SAL_FRAME_STYLE_SIZEABLE )
	{
		HIPoint origin;
		origin.x = 0;
		origin.y = 0;
		HIThemeGrowBoxDrawInfo growBoxInfo;
		memset( &growBoxInfo, 0, sizeof( HIThemeGrowBoxDrawInfo ) );
		growBoxInfo.version = 0;
		growBoxInfo.state = kThemeStateActive;
		growBoxInfo.kind = kHIThemeGrowBoxKindNormal;
		growBoxInfo.direction = kThemeGrowLeft | kThemeGrowRight | kThemeGrowUp | kThemeGrowDown;
		growBoxInfo.size = kHIThemeGrowBoxSizeNormal;
		HIRect bounds;
		if ( pHIThemeGetGrowBoxBounds( &origin, &growBoxInfo, &bounds ) == noErr )
		{
			Rectangle boundingRect = aRegion.GetBoundRect();
			long nExcessWidth = boundingRect.Right() - pGraphics->mpFrame->maGeometry.nWidth + (long)bounds.size.width + 1;
			long nExcessHeight = boundingRect.Bottom() - pGraphics->mpFrame->maGeometry.nHeight + (long)bounds.size.height + 1;
			if ( nExcessWidth > 0 && nExcessHeight > 0 )
			{
				if ( nType == CTRL_SCROLLBAR && boundingRect.GetHeight() > boundingRect.GetWidth() )
				{
					if ( boundingRect.GetHeight() - nExcessHeight > 0 )
						boundingRect.Bottom() -= nExcessHeight;
				}
				else if ( nExcessWidth > 0 && boundingRect.GetWidth() - nExcessWidth > 0 )
				{
					boundingRect.Right() -= nExcessWidth;
				}
				else if ( nExcessHeight > 0 && boundingRect.GetHeight() - nExcessHeight > 0 )
				{
					boundingRect.Bottom() -= nExcessHeight;
				}
				aRegion = Region( boundingRect );
			}
		}
	}

	return aRegion;
}

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
	BOOL isSupported = FALSE;

#ifndef USE_NATIVE_CONTROLS
	return isSupported;
#endif	// !USE_NATIVE_CONTROLS

	if ( !HIThemeInitialize() )
		return isSupported;

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

		case CTRL_SCROLLBAR:
			if( ( nPart == PART_ENTIRE_CONTROL ) || nPart == PART_DRAW_BACKGROUND_HORZ || nPart == PART_DRAW_BACKGROUND_VERT )
				isSupported = TRUE;
			break;

		case CTRL_SPINBOX:
			if( nPart == PART_ENTIRE_CONTROL || nPart == PART_ALL_BUTTONS || nPart == HAS_BACKGROUND_TEXTURE )
				isSupported = TRUE;
			break;

		case CTRL_SPINBUTTONS:
			if( nPart == PART_ENTIRE_CONTROL || nPart == PART_ALL_BUTTONS )
				isSupported = TRUE;
			break;

		case CTRL_PROGRESS:
		case CTRL_INTROPROGRESS:
			if( nPart == PART_ENTIRE_CONTROL )
				isSupported = TRUE;
			break;

		case CTRL_TAB_ITEM:
			if( nPart == PART_ENTIRE_CONTROL )
				isSupported = TRUE;
			break;

		case CTRL_TAB_PANE:
			if( nPart == PART_ENTIRE_CONTROL )
				isSupported = TRUE;
			break;

		case CTRL_FIXEDBORDER:
		case CTRL_GROUPBOX:
			if( nPart == PART_ENTIRE_CONTROL )
				isSupported = TRUE;
			break;

		case CTRL_MENU_POPUP:
			if( nPart == PART_ENTIRE_CONTROL )
				isSupported = TRUE;
			break;

		case CTRL_EDITBOX:
			if( ( nPart == PART_ENTIRE_CONTROL ) || ( nPart == HAS_BACKGROUND_TEXTURE ) )
				isSupported = TRUE;
			break;

		case CTRL_DISCLOSUREBTN:
			if( nPart == PART_ENTIRE_CONTROL )
				isSupported = TRUE;
			break;

		case CTRL_LISTVIEWHEADER:
			if( ( nPart == PART_ENTIRE_CONTROL ) || ( nPart == PART_LISTVIEWHEADER_SORT_MARK ) )
				isSupported = TRUE;
			break;

		case CTRL_FIXEDLINE:
			if( nPart == PART_ENTIRE_CONTROL )
				isSupported = TRUE;
			break;

		case CTRL_LISTVIEWBOX:
			if( nPart == PART_ENTIRE_CONTROL )
				isSupported = TRUE;
			break;

		case CTRL_TOOLBAR:
			// Suppress the non-native toolbar background
			if( nPart == CTRL_PUSHBUTTON || nPart == PART_BUTTON )
				isSupported = TRUE;
			break;

		case CTRL_FRAME:
			if ( nPart == PART_BORDER )
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
	rIsInside = FALSE;

#ifndef USE_NATIVE_CONTROLS
	if ( !IsNativeControlSupported( nType, nPart ) )
		return rIsInside;
#endif	// !USE_NATIVE_CONTROLS

	if ( !HIThemeInitialize() )
		return rIsInside;

	// [ed] Scrollbars are a special case:  in order to get proper regions,
	// a full description of the scrollbar is required including its values
	// and visible width.  We'll rely on the VCL scrollbar, which queried
	// these regions, to perform our hit testing.

	if ( nType == CTRL_SCROLLBAR )
		return FALSE;

	Region aNativeBoundingRegion;
	Region aNativeContentRegion;
	if ( getNativeControlRegion( nType, nPart, rControlRegion, 0, ImplControlValue(), rControlHandle, OUString(), aNativeBoundingRegion, aNativeContentRegion ) )
	{
		rIsInside = aNativeBoundingRegion.IsInside( aPos );
		return TRUE;
	}
	else
	{
		return FALSE;
	}
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
 * @param rCaption		Caption title or string for the control.
 *				Contains keyboard shortcuts prefixed with ~
 * @return TRUE if drawing was successful, FALSE if drawing was not successful
 */
BOOL JavaSalGraphics::drawNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle, const OUString& rCaption )
{
	BOOL bOK = FALSE;

#ifndef USE_NATIVE_CONTROLS
	if ( !IsNativeControlSupported( nType, nPart ) )
		return bOK;
#endif	// !USE_NATIVE_CONTROLS

	if ( !HIThemeInitialize() )
		return bOK;

	const Region &rRealControlRegion = GetRegionAdjustedForGrowBox( this, nType, rControlRegion );

	switch( nType )
	{
		case CTRL_PUSHBUTTON:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				if ( mpFrame && !mpFrame->IsFloatingFrame() && mpFrame != GetSalData()->mpFocusFrame )
					nState = 0;

				Rectangle buttonRect = rRealControlRegion.GetBoundRect();
				bOK = DrawNativePushButton( this, buttonRect, nState, aValue );
			}
			break;

		case CTRL_RADIOBUTTON:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				if ( mpFrame && !mpFrame->IsFloatingFrame() && mpFrame != GetSalData()->mpFocusFrame )
					nState = 0;

				Rectangle buttonRect = rRealControlRegion.GetBoundRect();
				bOK = DrawNativeRadioButton( this, buttonRect, nState, aValue );
			}
			break;

		case CTRL_CHECKBOX:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				if ( mpFrame && !mpFrame->IsFloatingFrame() && mpFrame != GetSalData()->mpFocusFrame )
					nState = 0;

				Rectangle buttonRect = rRealControlRegion.GetBoundRect();
				bOK = DrawNativeCheckbox( this, buttonRect, nState, aValue );
			}
			break;

		case CTRL_COMBOBOX:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle buttonRect = rRealControlRegion.GetBoundRect();
				bOK = DrawNativeComboBox( this, buttonRect, nState, rCaption );
			}
			break;

		case CTRL_LISTBOX:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle buttonRect = rRealControlRegion.GetBoundRect();
				bOK = DrawNativeListBox( this, buttonRect, nState, rCaption );
			}
			break;

		case CTRL_SCROLLBAR:
			if( ( nPart == PART_ENTIRE_CONTROL) || ( nPart == PART_DRAW_BACKGROUND_HORZ ) || ( nPart == PART_DRAW_BACKGROUND_VERT ) )
			{
				Rectangle buttonRect = rRealControlRegion.GetBoundRect();
				ScrollbarValue *pValue = static_cast<ScrollbarValue *> ( aValue.getOptionalVal() );
				bOK = DrawNativeScrollBar( this, buttonRect, nState, pValue );
			}
			break;

		case CTRL_SPINBOX:
			if( ( nPart == PART_ENTIRE_CONTROL ) || ( nPart == PART_ALL_BUTTONS ) )
			{
				Rectangle buttonRect = rRealControlRegion.GetBoundRect();
				SpinbuttonValue *pValue = static_cast<SpinbuttonValue *> ( aValue.getOptionalVal() );
				bOK = DrawNativeSpinbox( this, buttonRect, nState, pValue );
			}
			break;

		case CTRL_SPINBUTTONS:
			if( ( nPart == PART_ENTIRE_CONTROL ) || ( nPart == PART_ALL_BUTTONS ) )
			{
				Rectangle buttonRect = rRealControlRegion.GetBoundRect();
				SpinbuttonValue *pValue = static_cast<SpinbuttonValue *> ( aValue.getOptionalVal() );
				bOK = DrawNativeSpinbutton( this, buttonRect, nState, pValue );
			}
			break;

		case CTRL_PROGRESS:
		case CTRL_INTROPROGRESS:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle ctrlRect = rRealControlRegion.GetBoundRect();
				ProgressbarValue aProgressbarValue;
				ProgressbarValue *pValue = static_cast<ProgressbarValue *> ( aValue.getOptionalVal() );
				if ( pValue )
				{
					aProgressbarValue.mbIndeterminate = pValue->mbIndeterminate;
					aProgressbarValue.mdPercentComplete = pValue->mdPercentComplete;
				}
				else
				{
					aProgressbarValue.mdPercentComplete = (double)( aValue.getNumericVal() * 100 / ctrlRect.GetWidth() );
				}

				bOK = DrawNativeProgressbar( this, ctrlRect, nState, &aProgressbarValue, nType == CTRL_INTROPROGRESS ? TRUE : FALSE );
			}
			break;

		case CTRL_TAB_ITEM:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle ctrlRect = rRealControlRegion.GetBoundRect();
				TabitemValue *pValue = static_cast<TabitemValue *> ( aValue.getOptionalVal() );
				bOK = DrawNativeTab( this, ctrlRect, nState, pValue );
			}
			break;

		case CTRL_TAB_PANE:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle ctrlRect = rRealControlRegion.GetBoundRect();
				// hack - on 10.3+ tab panes visually need to intersect the
				// middle of the associated segmented control.  Subtract
				// 15 off the height to shoehorn the drawing in.
				ctrlRect.setY( ctrlRect.getY() - CONTROL_TAB_PANE_TOP_OFFSET );
				ctrlRect.setHeight( ctrlRect.getHeight() + CONTROL_TAB_PANE_TOP_OFFSET );
				bOK = DrawNativeTabBoundingBox( this, ctrlRect, nState );
			}
			break;

		case CTRL_FIXEDBORDER:
		case CTRL_GROUPBOX:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle ctrlRect = rRealControlRegion.GetBoundRect();
				bOK = DrawNativePrimaryGroupBox( this, ctrlRect, nState );
			}
			break;

		case CTRL_MENU_POPUP:
			if ( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle ctrlRect = rRealControlRegion.GetBoundRect();
				bOK = DrawNativeMenuBackground( this, ctrlRect );
			}
			break;

		case CTRL_EDITBOX:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle ctrlRect = rRealControlRegion.GetBoundRect();
				bOK = DrawNativeEditBox( this, ctrlRect, nState );
			}
			break;

		case CTRL_DISCLOSUREBTN:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle ctrlRect = rRealControlRegion.GetBoundRect();
				DisclosureBtnValue *pValue = static_cast<DisclosureBtnValue *> ( aValue.getOptionalVal() );
				bOK = DrawNativeDisclosureBtn( this, ctrlRect, nState, pValue );
			}
			break;

		case CTRL_LISTVIEWHEADER:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle ctrlRect = rRealControlRegion.GetBoundRect();
				ListViewHeaderValue *pValue = static_cast<ListViewHeaderValue *> ( aValue.getOptionalVal() );
				bOK = DrawNativeListViewHeader( this, ctrlRect, nState, pValue );
			}
			break;

		case CTRL_FIXEDLINE:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle ctrlRect = rRealControlRegion.GetBoundRect();
				bOK = DrawNativeSeparatorLine( this, ctrlRect, nState );
			}
			break;

		case CTRL_LISTVIEWBOX:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle ctrlRect = rRealControlRegion.GetBoundRect();
				bOK = DrawNativeListBoxFrame( this, ctrlRect, nState );
			}
			break;

		case CTRL_TOOLBAR:
			if( nPart == PART_BUTTON )
			{
				Rectangle ctrlRect = rRealControlRegion.GetBoundRect();
				bOK = DrawNativeBevelButton( this, ctrlRect, nState, aValue );
			}
			break;

		case CTRL_FRAME:
			if ( nPart == PART_BORDER )
			{
				USHORT nValue = (USHORT)aValue.getNumericVal();
				if ( ! ( nValue & ( FRAME_DRAW_MENU | FRAME_DRAW_WINDOWBORDER ) ) )
				{
					Rectangle ctrlRect = rRealControlRegion.GetBoundRect();
					ctrlRect.Left() -= LISTVIEWFRAME_TRIMWIDTH;
					ctrlRect.Top() -= LISTVIEWFRAME_TRIMWIDTH;
					if ( nValue & FRAME_DRAW_DOUBLEIN )
					{
						ctrlRect.Right() += LISTVIEWFRAME_TRIMWIDTH;
						ctrlRect.Bottom() += LISTVIEWFRAME_TRIMWIDTH;
					}
					bOK = DrawNativeListBoxFrame( this, ctrlRect, CTRL_STATE_ENABLED );
				}
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
 * @param rCaption		Caption title or string for the control.
 *				Contains keyboard shortcuts prefixed with ~
 * @return TRUE if the text was drawn, FALSE if the control had its text drawn
 *	with drawNativeControl()
 */
BOOL JavaSalGraphics::drawNativeControlText( ControlType nType, ControlPart nPart, const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle, const OUString& rCaption )
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
 * @param rCaption		Caption title or string for the control.
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
BOOL JavaSalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart, const Region& rControlRegion, ControlState nState, const ImplControlValue& aValue, SalControlHandle& rControlHandle, const OUString& rCaption, Region &rNativeBoundingRegion, Region &rNativeContentRegion )
{
	BOOL bReturn = FALSE;

#ifndef USE_NATIVE_CONTROLS
	if ( !IsNativeControlSupported( nType, nPart ) )
		return bReturn;
#endif	// !USE_NATIVE_CONTROLS

	if ( !HIThemeInitialize() )
		return bReturn;

	const Region &rRealControlRegion = GetRegionAdjustedForGrowBox( this, nType, rControlRegion );

	switch( nType )
	{
		case CTRL_PUSHBUTTON:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				// If the button width is less than the preferred height,
				// assume that it's intended to be a "placard" type button with
				// an icon. In that case, return the requested height as it
				// will then draw it as a placard button instead of a rounded
				// button. This makes buttons used as parts of subcontrols
				// (combo boxes, small toolbar buttons) draw with the
				// appropriate style.
    			SInt32 pushButtonThemeHeight;
    			bReturn = ( pGetThemeMetric( kThemeMetricPushButtonHeight, &pushButtonThemeHeight) == noErr );
				if ( ! bReturn )
					return bReturn;

				Rectangle buttonRect = rRealControlRegion.GetBoundRect();
				long buttonWidth = buttonRect.GetWidth();
				long buttonHeight = pushButtonThemeHeight;
				if ( buttonHeight >= buttonWidth )
				{
					buttonWidth++;
					buttonHeight = buttonRect.GetHeight() + 1;
				}
				else if ( buttonHeight != buttonRect.GetHeight() && buttonRect.GetHeight() > 0 )
				{
					buttonHeight = buttonRect.GetHeight();
				}
				Point topLeft( (long)(buttonRect.Left() - FOCUSRING_WIDTH), (long)(buttonRect.Top() + ((buttonRect.GetHeight() - buttonHeight) / 2) - FOCUSRING_WIDTH) );
				Size boundsSize( (long)buttonWidth + ( FOCUSRING_WIDTH * 2 ), (long)buttonHeight + ( FOCUSRING_WIDTH * 2 ) );
				rNativeBoundingRegion = Region( Rectangle( topLeft, boundsSize ) );
				rNativeContentRegion = Region( rNativeBoundingRegion );
				bReturn = TRUE;
			}
			break;

		case CTRL_RADIOBUTTON:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle buttonRect = rRealControlRegion.GetBoundRect();
				Point topLeft( (long)(buttonRect.Left() - FOCUSRING_WIDTH), (long)(buttonRect.Top() - FOCUSRING_WIDTH) );
				Size boundsSize( (long)RADIOBUTTON_WIDTH + ( FOCUSRING_WIDTH * 2 ), (long)RADIOBUTTON_HEIGHT + ( FOCUSRING_WIDTH * 2 ) );
				rNativeBoundingRegion = Region( Rectangle( topLeft, boundsSize ) );
				rNativeContentRegion = Region( rNativeBoundingRegion );
				bReturn = TRUE;
			}
			break;

		case CTRL_CHECKBOX:
			if( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle buttonRect = rRealControlRegion.GetBoundRect();
				Point topLeft( (long)(buttonRect.Left() - FOCUSRING_WIDTH), (long)(buttonRect.Top() - FOCUSRING_WIDTH) );
				Size boundsSize( (long)CHECKBOX_WIDTH + ( FOCUSRING_WIDTH * 2 ), (long)CHECKBOX_HEIGHT + ( FOCUSRING_WIDTH * 2 ) );
				rNativeBoundingRegion = Region( Rectangle( topLeft, boundsSize ) );
				rNativeContentRegion = Region( rNativeBoundingRegion );
				bReturn = TRUE;
			}
			break;

		case CTRL_COMBOBOX:
		case CTRL_LISTBOX:
			{
				Rectangle comboBoxRect = rRealControlRegion.GetBoundRect();
				long nHeightAdjust = ( COMBOBOX_HEIGHT - comboBoxRect.GetHeight() ) / 2;
				if ( nHeightAdjust > 0 )
				{
					comboBoxRect.Top() -= nHeightAdjust;
					comboBoxRect.Bottom() += nHeightAdjust;
				}

				HIThemeButtonDrawInfo aButtonDrawInfo;
				InitButtonDrawInfo( &aButtonDrawInfo, nState );

				HIRect preferredRect;
				HIRect destRect;
				destRect.origin.x = comboBoxRect.Left();
				destRect.origin.y = comboBoxRect.Top();
				destRect.size.width = comboBoxRect.GetWidth();
				destRect.size.height = comboBoxRect.GetHeight();
				if ( pHIThemeGetButtonBackgroundBounds( &destRect, &aButtonDrawInfo, &preferredRect ) == noErr )
				{
					// Vertically center the preferred bounds
					float fHeightAdjust = ( preferredRect.size.height - destRect.size.height ) / 2;
					if ( fHeightAdjust < 0 )
					{
						preferredRect.origin.y -= fHeightAdjust;
					}
					else
					{
						preferredRect.origin.y = destRect.origin.y;
						preferredRect.size.height = destRect.size.height;
					}

					switch( nPart )
					{
						case PART_ENTIRE_CONTROL:
							{
								// Fix clipping of edges by returning the
								// passed in bounds
								rNativeBoundingRegion = Region( comboBoxRect );
								rNativeContentRegion = Region( rNativeBoundingRegion );
								bReturn = TRUE;
							}
							break;

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
								SInt32 editFramePadding;
								bReturn = ( pGetThemeMetric( kThemeMetricEditTextFrameOutset, &editFramePadding) == noErr );
								if ( ! bReturn )
									return bReturn;

								Point topLeft( (long)preferredRect.origin.x + editFramePadding, (long)preferredRect.origin.y + editFramePadding );
								Size boundsSize( (long)preferredRect.size.width - COMBOBOX_BUTTON_WIDTH  - ( editFramePadding * 2 ), (long)preferredRect.size.height - ( editFramePadding * 2 ) );
								rNativeBoundingRegion = Region( Rectangle( topLeft, boundsSize ) );
								rNativeContentRegion = Region( rNativeBoundingRegion );
								bReturn = TRUE;
							}
							break;
					}
				}
			}
			break;

		case CTRL_SCROLLBAR:
			{
				// Fix bug 1600 by detecting if double arrows are at both ends
				Rectangle comboBoxRect = rRealControlRegion.GetBoundRect();

				ScrollbarValue *pValue = static_cast<ScrollbarValue *> ( aValue.getOptionalVal() );

				HIThemeTrackDrawInfo pTrackDrawInfo;
				HIScrollBarTrackInfo pScrollBarTrackInfo;
				InitScrollBarTrackInfo( &pTrackDrawInfo, &pScrollBarTrackInfo, nState, comboBoxRect, pValue );

				HIRect bounds;

				bool bHorizontal = ( ( comboBoxRect.GetWidth() > comboBoxRect.GetHeight() ) ? true : false );
				long nStart = 0;
				long nVisibleSize = 0;
				if ( SCROLLBAR_SUPPRESS_ARROWS )
				{
					pHIThemeGetScrollBarTrackRect( &pTrackDrawInfo.bounds, &pScrollBarTrackInfo, bHorizontal, &bounds );
					if ( pValue )
					{
						if ( bHorizontal )
						{
							nStart = Float32ToLong( bounds.origin.x + ( ( bounds.size.width * pValue->mnCur ) / ( pValue->mnMax - pValue->mnMin ) ) );
							nVisibleSize = Float32ToLong( ( bounds.size.width * pValue->mnVisibleSize ) / ( pValue->mnMax - pValue->mnMin ) ) + SCROLLBAR_THUMB_TRIMWIDTH;

							if ( nVisibleSize > bounds.size.width )
							{
								nVisibleSize = bounds.size.width;
							}
							else if ( nVisibleSize < SCROLLBAR_THUMB_MIN_WIDTH )
							{
								nStart -= Float32ToLong( (float)( ( SCROLLBAR_THUMB_MIN_WIDTH - nVisibleSize - SCROLLBAR_THUMB_TRIMWIDTH ) * pValue->mnCur ) / ( pValue->mnMax - pValue->mnMin - pValue->mnVisibleSize ) );
								nVisibleSize = SCROLLBAR_THUMB_MIN_WIDTH;
							}

							if ( nStart < bounds.origin.x )
								nStart = bounds.origin.x;
							else if ( nStart > bounds.origin.x + bounds.size.width )
								nStart = bounds.origin.x + bounds.size.width;
							if ( nStart + nVisibleSize > bounds.origin.x + bounds.size.width )
								nStart = bounds.origin.x + bounds.size.width - nVisibleSize;
						}
						else
						{
							nStart = Float32ToLong( bounds.origin.y + ( ( bounds.size.height * pValue->mnCur ) / ( pValue->mnMax - pValue->mnMin ) ) );
							nVisibleSize = Float32ToLong( ( bounds.size.height * pValue->mnVisibleSize ) / ( pValue->mnMax - pValue->mnMin ) ) + SCROLLBAR_THUMB_TRIMWIDTH;

							if ( nVisibleSize > bounds.size.height )
							{
								nVisibleSize = bounds.size.height;
							}
							else if ( nVisibleSize < SCROLLBAR_THUMB_MIN_WIDTH )
							{
								nStart -= Float32ToLong( (float)( ( SCROLLBAR_THUMB_MIN_WIDTH - nVisibleSize - SCROLLBAR_THUMB_TRIMWIDTH ) * pValue->mnCur ) / ( pValue->mnMax - pValue->mnMin - pValue->mnVisibleSize ) );
								nVisibleSize = SCROLLBAR_THUMB_MIN_WIDTH;
							}

							if ( nStart < bounds.origin.y )
								nStart = bounds.origin.y;
							else if ( nStart > bounds.origin.y + bounds.size.height )
								nStart = bounds.origin.y + bounds.size.height;

							if ( nStart + nVisibleSize > bounds.origin.y + bounds.size.height )
								nStart = bounds.origin.y + bounds.size.height - nVisibleSize;
						}
					}
					else
					{
						if ( bHorizontal )
							nStart = bounds.origin.x;
						else
							nStart = bounds.origin.y;
					}
				}

				switch ( nPart )
				{
					case PART_ENTIRE_CONTROL:
						pHIThemeGetTrackBounds( &pTrackDrawInfo, &bounds );
						break;

					case PART_BUTTON_LEFT:
						{
							HIRect trackBounds;
							pHIThemeGetTrackBounds( &pTrackDrawInfo, &trackBounds );
							pHIThemeGetTrackPartBounds( &pTrackDrawInfo, kAppearancePartLeftButton, &bounds );
							if ( SCROLLBAR_SUPPRESS_ARROWS )
							{
								bounds.origin.x = trackBounds.origin.x;
								bounds.size.width = 0;
							}
							else if ( GetSalData()->mbDoubleScrollbarArrows )
							{
								bounds.origin.x = trackBounds.origin.x;
								bounds.size.width *= 2;
							}
							else
							{
								if ( bounds.origin.x > trackBounds.origin.x )
									bounds.origin.x += SCROLLBAR_ARROW_TRIMX;
								bounds.size.width -= SCROLLBAR_ARROW_TRIMWIDTH;
							}
						}
						break;

					case PART_BUTTON_UP:
						{
							HIRect trackBounds;
							pHIThemeGetTrackBounds( &pTrackDrawInfo, &trackBounds );
							if ( SCROLLBAR_SUPPRESS_ARROWS )
							{
								bounds.origin.y = trackBounds.origin.y;
								bounds.size.height = 0;
							}
							else
							{
								pHIThemeGetTrackPartBounds( &pTrackDrawInfo, kAppearancePartUpButton, &bounds );
								if ( GetSalData()->mbDoubleScrollbarArrows )
								{
									bounds.origin.y = trackBounds.origin.y;
									bounds.size.height *= 2;
								}
								else
								{
									if ( bounds.origin.y > trackBounds.origin.y )
									{
										bounds.origin.y += SCROLLBAR_ARROW_TRIMY;
										bounds.size.height -= SCROLLBAR_ARROW_BOTTOM_TRIMHEIGHT;
									}
									else
									{
										bounds.size.height -= SCROLLBAR_ARROW_TOP_TRIMHEIGHT;
									}
								}
							}
						}
						break;

					case PART_BUTTON_RIGHT:
						{
							HIRect trackBounds;
							pHIThemeGetTrackBounds( &pTrackDrawInfo, &trackBounds );
							if ( SCROLLBAR_SUPPRESS_ARROWS )
							{
								bounds.origin.x = trackBounds.origin.x + trackBounds.size.width;
								bounds.size.width = 0;
							}
							else
							{
								pHIThemeGetTrackPartBounds( &pTrackDrawInfo, kAppearancePartRightButton, &bounds );
								if ( GetSalData()->mbDoubleScrollbarArrows )
								{
									bounds.size.width *= 2;
									bounds.origin.x = trackBounds.origin.x + trackBounds.size.width - bounds.size.width;
								}
								else
								{
									HIRect otherBounds;
									pHIThemeGetTrackPartBounds( &pTrackDrawInfo, kAppearancePartLeftButton, &otherBounds );
									if ( otherBounds.origin.x <= trackBounds.origin.x )
									{
										bounds.origin.x += SCROLLBAR_ARROW_TRIMX;
										bounds.size.width -= SCROLLBAR_ARROW_TRIMWIDTH;
									}
								}
							}
						}
						break;

					case PART_BUTTON_DOWN:
						{
							HIRect trackBounds;
							pHIThemeGetTrackBounds( &pTrackDrawInfo, &trackBounds );
							if ( SCROLLBAR_SUPPRESS_ARROWS )
							{
								bounds.origin.y = trackBounds.origin.y + trackBounds.size.height;
								bounds.size.height = 0;
							}
							else
							{
								pHIThemeGetTrackPartBounds( &pTrackDrawInfo, kAppearancePartDownButton, &bounds );
								if ( GetSalData()->mbDoubleScrollbarArrows )
								{
									bounds.size.height *= 2;
									bounds.origin.y = trackBounds.origin.y + trackBounds.size.height - bounds.size.height;
								}
								else
								{
									HIRect otherBounds;
									pHIThemeGetTrackPartBounds( &pTrackDrawInfo, kAppearancePartUpButton, &otherBounds );
									if ( otherBounds.origin.y <= trackBounds.origin.y )
									{
										bounds.origin.y += SCROLLBAR_ARROW_TRIMY;
										bounds.size.height -= SCROLLBAR_ARROW_BOTTOM_TRIMHEIGHT;
									}
								}
							}
						}
						break;

					case PART_TRACK_HORZ_LEFT:
					case PART_TRACK_VERT_UPPER:
						if ( SCROLLBAR_SUPPRESS_ARROWS )
						{
							if ( bHorizontal )
								bounds.size.width = nStart;
							else
								bounds.size.height = nStart;
						}
						else
						{
							pHIThemeGetTrackPartBounds( &pTrackDrawInfo, kAppearancePartPageUpArea, &bounds );
							if( ! bounds.size.width && ! bounds.size.height )
							{
								// disabled control or other invalid settings.  Set to the entire
								// track.

								pHIThemeGetScrollBarTrackRect( &pTrackDrawInfo.bounds, &pScrollBarTrackInfo, ( ( comboBoxRect.GetWidth() > comboBoxRect.GetHeight() ) ? true : false ), &bounds );
							}
						}
						break;

					case PART_TRACK_HORZ_RIGHT:
					case PART_TRACK_VERT_LOWER:
						if ( SCROLLBAR_SUPPRESS_ARROWS )
						{
							if ( bHorizontal )
							{
								bounds.size.width -= nStart + nVisibleSize - bounds.origin.x;
								bounds.origin.x = nStart + nVisibleSize;
							}
							else
							{
								bounds.size.height -= nStart + nVisibleSize - bounds.origin.y;
								bounds.origin.y = nStart + nVisibleSize;
							}
						}
						else
						{
							pHIThemeGetTrackPartBounds( &pTrackDrawInfo, kAppearancePartPageDownArea, &bounds );
							if( ! bounds.size.width && ! bounds.size.height )
							{
								// disabled control or other invalid settings.  Set to the entire
								// track.

								pHIThemeGetScrollBarTrackRect( &pTrackDrawInfo.bounds, &pScrollBarTrackInfo, ( ( comboBoxRect.GetWidth() > comboBoxRect.GetHeight() ) ? true : false ), &bounds );
							}
						}
						break;

					case PART_THUMB_HORZ:
					case PART_THUMB_VERT:
						if ( SCROLLBAR_SUPPRESS_ARROWS )
						{
							if ( bHorizontal )
							{
								bounds.origin.x = nStart;
								bounds.size.width = nVisibleSize;
							}
							else
							{
								bounds.origin.y = nStart;
								bounds.size.height = nVisibleSize;
							}
						}
						else
						{
							pHIThemeGetTrackPartBounds( &pTrackDrawInfo, kAppearancePartIndicator, &bounds );
							if( ! bounds.size.width && ! bounds.size.height )
							{
								// disabled control or other invalid settings.  Set to the entire
								// track.

								pHIThemeGetScrollBarTrackRect( &pTrackDrawInfo.bounds, &pScrollBarTrackInfo, ( ( comboBoxRect.GetWidth() > comboBoxRect.GetHeight() ) ? true : false ), &bounds );
							}
						}
						break;
					
					case PART_TRACK_HORZ_AREA:
					case PART_TRACK_VERT_AREA:
						// [ed] 11/9/08 3.0 has new controls to obtain the
						// entire track area.  This includes page up area,
						// page down area, and thumb area.
						
						if ( SCROLLBAR_SUPPRESS_ARROWS )
						{
							pHIThemeGetScrollBarTrackRect( &pTrackDrawInfo.bounds, &pScrollBarTrackInfo, bHorizontal, &bounds );
						}
						else
						{
							HIRect upBounds;
							HIRect downBounds;
							HIRect thumbBounds;
							
							pHIThemeGetTrackPartBounds( &pTrackDrawInfo, kAppearancePartIndicator, &thumbBounds );
							if( ! thumbBounds.size.width && ! thumbBounds.size.height )
							{
								// disabled control or other invalid settings.  Set to the entire
								// track.

								pHIThemeGetScrollBarTrackRect( &pTrackDrawInfo.bounds, &pScrollBarTrackInfo, ( ( comboBoxRect.GetWidth() > comboBoxRect.GetHeight() ) ? true : false ), &bounds );
								break;
							}
							
							pHIThemeGetTrackPartBounds( &pTrackDrawInfo, kAppearancePartPageDownArea, &downBounds );
							if( ! downBounds.size.width && ! downBounds.size.height )
							{
								// disabled control or other invalid settings.  Set to the entire
								// track.

								pHIThemeGetScrollBarTrackRect( &pTrackDrawInfo.bounds, &pScrollBarTrackInfo, ( ( comboBoxRect.GetWidth() > comboBoxRect.GetHeight() ) ? true : false ), &bounds );
								break;
							}
							
							pHIThemeGetTrackPartBounds( &pTrackDrawInfo, kAppearancePartPageUpArea, &upBounds );
							if( ! upBounds.size.width && ! upBounds.size.height )
							{
								// disabled control or other invalid settings.  Set to the entire
								// track.

								pHIThemeGetScrollBarTrackRect( &pTrackDrawInfo.bounds, &pScrollBarTrackInfo, ( ( comboBoxRect.GetWidth() > comboBoxRect.GetHeight() ) ? true : false ), &bounds );
							}
							
							bounds=CGRectUnion(upBounds, thumbBounds);
							bounds=CGRectUnion(bounds, downBounds);
						}
						break;
				}

				// Fix bug 2031 by incrementing the scrollbar width slightly
				if ( comboBoxRect.GetWidth() > comboBoxRect.GetHeight() )
					bounds.size.height++;
				else
					bounds.size.width++;
				Point topLeft( (long)(comboBoxRect.Left()+bounds.origin.x), (long)(comboBoxRect.Top()+bounds.origin.y) );
				Size boundsSize( (long)bounds.size.width, (long)bounds.size.height );
				rNativeBoundingRegion = Region( Rectangle( topLeft, boundsSize ) );
				rNativeContentRegion = Region( rNativeBoundingRegion );

				bReturn = TRUE;
			}
			break;

		case CTRL_SPINBOX:
			{
				Rectangle spinboxRect = rRealControlRegion.GetBoundRect();
				long nHeightAdjust = ( EDITBOX_HEIGHT - spinboxRect.GetHeight() ) / 2;
				if ( nHeightAdjust > 0 )
				{
					spinboxRect.Top() -= nHeightAdjust;
					spinboxRect.Bottom() += nHeightAdjust;
				}

				// leave room for left edge adornments

				SInt32 spinnerThemeWidth;
				SInt32 spinnerThemeHeight;
				bReturn = ( pGetThemeMetric( kThemeMetricLittleArrowsWidth, &spinnerThemeWidth ) == noErr && pGetThemeMetric( kThemeMetricLittleArrowsHeight, &spinnerThemeHeight ) == noErr );
				if ( ! bReturn )
					return bReturn;

				spinnerThemeWidth += SPINNER_TRIMWIDTH * 2;

				switch( nPart )
				{
					case PART_ENTIRE_CONTROL:
						{
							rNativeBoundingRegion = Region( spinboxRect );
							rNativeContentRegion = Region( rNativeBoundingRegion );
							bReturn = TRUE;
						}
						break;

					case PART_BUTTON_UP:
						{
							Point topLeft( (long)( spinboxRect.Right() - spinnerThemeWidth ), (long)( spinboxRect.Top() + ( spinboxRect.GetHeight() / 2 ) - spinnerThemeHeight ) );
							Size boundsSize( (long)spinnerThemeWidth, (long)spinnerThemeHeight );
							rNativeBoundingRegion = Region( Rectangle( topLeft, boundsSize ) );
							rNativeContentRegion = Region( rNativeBoundingRegion );
							bReturn = TRUE;
						}
						break;

					case PART_BUTTON_DOWN:
						{
							Point topLeft( (long)( spinboxRect.Right() - spinnerThemeWidth ), (long)( spinboxRect.Top() + ( spinboxRect.GetHeight() / 2 ) ) );
							Size boundsSize( (long)spinnerThemeWidth, (long)spinnerThemeHeight );
							rNativeBoundingRegion = Region( Rectangle( topLeft, boundsSize ) );
							rNativeContentRegion = Region( rNativeBoundingRegion );
							bReturn = TRUE;
						}
						break;

					case PART_SUB_EDIT:
						{
							SInt32 editFramePadding;
							bReturn = ( pGetThemeMetric( kThemeMetricEditTextFrameOutset, &editFramePadding) == noErr );
							if ( ! bReturn )
								return bReturn;

							rNativeBoundingRegion = Region( Rectangle( Point( spinboxRect.Left() + editFramePadding, spinboxRect.Top() + editFramePadding ), Size( (long)( spinboxRect.GetWidth() - spinnerThemeWidth - 4 - ( editFramePadding * 2 ) ), spinboxRect.GetHeight() - ( editFramePadding * 2 ) ) ) );
							rNativeContentRegion = Region( rNativeBoundingRegion );
							bReturn = TRUE;
						}
						break;
				}
			}
			break;

		case CTRL_SPINBUTTONS:
			{
				Rectangle spinboxRect = rRealControlRegion.GetBoundRect();

				// leave room for left edge adornments

				SInt32 spinnerThemeWidth;
				SInt32 spinnerThemeHeight;
				bReturn = ( pGetThemeMetric( kThemeMetricLittleArrowsWidth, &spinnerThemeWidth ) == noErr && pGetThemeMetric( kThemeMetricLittleArrowsHeight, &spinnerThemeHeight ) == noErr );
				if ( ! bReturn )
					return bReturn;

				spinnerThemeWidth += SPINNER_TRIMWIDTH * 2;

				switch( nPart )
				{
					case PART_BUTTON_UP:
						{
							Point topLeft( (long)( spinboxRect.Right() - spinnerThemeWidth ), (long)( spinboxRect.Top() + ( spinboxRect.GetHeight() / 2 ) - spinnerThemeHeight ) );
							Size boundsSize( (long)spinnerThemeWidth, (long)spinnerThemeHeight );
							rNativeBoundingRegion = Region( Rectangle( topLeft, boundsSize ) );
							rNativeContentRegion = Region( rNativeBoundingRegion );
							bReturn = TRUE;
						}
						break;

					case PART_BUTTON_DOWN:
						{
							Point topLeft( (long)( spinboxRect.Right() - spinnerThemeWidth ), (long)( spinboxRect.Top() + ( spinboxRect.GetHeight() / 2 ) ) );
							Size boundsSize( (long)spinnerThemeWidth, (long)spinnerThemeHeight );
							rNativeBoundingRegion = Region( Rectangle( topLeft, boundsSize ) );
							rNativeContentRegion = Region( rNativeBoundingRegion );
							bReturn = TRUE;
						}
						break;
				}
			}
			break;

		case CTRL_PROGRESS:
		case CTRL_INTROPROGRESS:
			if ( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle controlRect = rRealControlRegion.GetBoundRect();

				ProgressbarValue aProgressbarValue;
				ProgressbarValue *pValue = static_cast<ProgressbarValue *> ( aValue.getOptionalVal() );
				if ( pValue )
				{
					aProgressbarValue.mbIndeterminate = pValue->mbIndeterminate;
					aProgressbarValue.mdPercentComplete = pValue->mdPercentComplete;
				}
				else
				{
					aProgressbarValue.mdPercentComplete = (double)( aValue.getNumericVal() * 100 / controlRect.GetWidth() );
				}

				HIThemeTrackDrawInfo pTrackDrawInfo;
				InitProgressbarTrackInfo( &pTrackDrawInfo, nState, controlRect, &aProgressbarValue, nType == CTRL_INTROPROGRESS ? TRUE : FALSE );

				HIRect bounds;
				pHIThemeGetTrackBounds( &pTrackDrawInfo, &bounds );

				bounds.size.height += PROGRESS_HEIGHT_SLOP;

				Point topLeft( (long)(controlRect.Left()+bounds.origin.x), (long)(controlRect.Top()+bounds.origin.y) );
				Size boundsSize( (long)bounds.size.width, (long)bounds.size.height );
				rNativeBoundingRegion = Region( Rectangle( topLeft, boundsSize ) );
				rNativeContentRegion = Region( rNativeBoundingRegion );

				bReturn = TRUE;
			}
			break;

		case CTRL_TAB_ITEM:
			if ( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle controlRect = rRealControlRegion.GetBoundRect();

				TabitemValue *pValue = static_cast<TabitemValue *> ( aValue.getOptionalVal() );

				HIThemeTabDrawInfo pTabDrawInfo;
				InitTabDrawInfo( &pTabDrawInfo, nState, pValue );

				HIRect proposedBounds;
				proposedBounds.origin.x = 0;
				proposedBounds.origin.y = 0;
				proposedBounds.size.width = controlRect.Right() - controlRect.Left();
				proposedBounds.size.height = controlRect.Bottom() - controlRect.Top();

				HIShapeRef tabShape;
				pHIThemeGetTabShape( &proposedBounds, (HIThemeTabDrawInfo *)&pTabDrawInfo, &tabShape );

				HIRect preferredRect;
				HIShapeGetBounds( tabShape, &preferredRect );
				CFRelease( tabShape );

				preferredRect.size.height += TABITEM_HEIGHT_SLOP;

				Point topLeft( controlRect.Left(), controlRect.Top() );
				Size boundsSize( (long)preferredRect.size.width, (long)preferredRect.size.height );
				rNativeBoundingRegion = Region( Rectangle( topLeft, boundsSize ) );
				rNativeContentRegion = Region( rNativeBoundingRegion );

				bReturn = TRUE;
			}
			break;

		case CTRL_TAB_PANE:
			if ( nPart == PART_ENTIRE_CONTROL )
			{
				// for now, assume tab panes will occupy the full rectangle and
				// not require bound adjustment.
				Rectangle controlRect = rRealControlRegion.GetBoundRect();
				rNativeBoundingRegion = Region( controlRect );
				rNativeContentRegion = Region( rNativeBoundingRegion );

				bReturn = TRUE;
			}
			break;

		case CTRL_FIXEDBORDER:
		case CTRL_GROUPBOX:
			if ( nPart == PART_ENTIRE_CONTROL )
			{
				// for now, assume primary group boxes will occupy the full rectangle and
				// not require bound adjustment.
				Rectangle controlRect = rRealControlRegion.GetBoundRect();
				rNativeBoundingRegion = Region( controlRect );
				rNativeContentRegion = Region( rNativeBoundingRegion );

				bReturn = TRUE;
			}
			break;

		case CTRL_MENU_POPUP:
			if ( nPart == PART_ENTIRE_CONTROL )
			{
				// we can draw menu backgrounds for any size rectangular area
				Rectangle controlRect = rRealControlRegion.GetBoundRect();
				rNativeBoundingRegion = Region( controlRect );
				rNativeContentRegion = Region( rNativeBoundingRegion );

				bReturn = TRUE;
			}
			break;

		case CTRL_EDITBOX:
			if ( nPart == PART_ENTIRE_CONTROL )
			{
				// fill entire control area with edit box
				Rectangle controlRect = rRealControlRegion.GetBoundRect();
				long nHeightAdjust = ( EDITBOX_HEIGHT - controlRect.GetHeight() ) / 2;
				if ( nHeightAdjust > 0 )
				{
					controlRect.Top() -= nHeightAdjust;
					controlRect.Bottom() += nHeightAdjust;
				}

				rNativeBoundingRegion = Region( controlRect );
				rNativeContentRegion = Region( rNativeBoundingRegion );

				bReturn = TRUE;
			}
			break;

		case CTRL_DISCLOSUREBTN:
			if ( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle controlRect = rRealControlRegion.GetBoundRect();
				rNativeBoundingRegion = Region( controlRect );
				rNativeContentRegion = Region( rNativeBoundingRegion );

				bReturn = TRUE;
			}
			break;

		case CTRL_LISTVIEWHEADER:
			if ( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle controlRect = rRealControlRegion.GetBoundRect();
				rNativeBoundingRegion = Region( controlRect );
				rNativeContentRegion = Region( rNativeBoundingRegion );

				bReturn = TRUE;
			}
			break;

		case CTRL_LISTVIEWBOX:
			if ( nPart == PART_ENTIRE_CONTROL )
			{
				Rectangle controlRect = rRealControlRegion.GetBoundRect();
				rNativeBoundingRegion = Region( controlRect );
				rNativeContentRegion = Region( rNativeBoundingRegion );

				bReturn = TRUE;
			}
			break;

		case CTRL_TOOLBAR:
			if ( nPart == PART_BUTTON )
			{
				Rectangle controlRect = rRealControlRegion.GetBoundRect();
				rNativeBoundingRegion = Region( controlRect );
				rNativeContentRegion = Region( rNativeBoundingRegion );

				bReturn = TRUE;
			}
			break;

		case CTRL_FRAME:
			if ( nPart == PART_BORDER )
			{
				USHORT nValue = (USHORT)aValue.getNumericVal();
				if ( ! ( nValue & ( FRAME_DRAW_MENU | FRAME_DRAW_WINDOWBORDER ) ) )
				{
					Rectangle controlRect = rRealControlRegion.GetBoundRect();
					controlRect.Left() += LISTVIEWFRAME_TRIMWIDTH;
					controlRect.Top() += LISTVIEWFRAME_TRIMWIDTH;
					if ( nValue & FRAME_DRAW_DOUBLEIN )
					{
						controlRect.Right() -= LISTVIEWFRAME_TRIMWIDTH;
						controlRect.Bottom() -= LISTVIEWFRAME_TRIMWIDTH;
					}
					rNativeBoundingRegion = Region( controlRect );
					rNativeContentRegion = Region( rNativeBoundingRegion );
					bReturn = TRUE;
				}
			}
			break;
	}

	return bReturn;
}

/**
 * (static) Convert a Mac RGBColor value into a SalColor.
 *
 * @param macColor 	Macintosh RGBColor struct
 * @return appropriate SalColor struct
 */
static SalColor ConvertRGBColorToSalColor( const RGBColor& theColor )
{
	return( MAKE_SALCOLOR( ((double)theColor.red/(double)USHRT_MAX)*0xFF, ((double)theColor.green/(double)USHRT_MAX)*0xFF, ((double)theColor.blue/(double)USHRT_MAX)*0xFF ) );
}

/**
 * Get the color that should be used to draw the textual element of a control.
 * This allows VCL controls that use widget renderig to get control backgrounds
 * and parts to use the correct color for the VCL rendered control text.
 *
 * @param nType		type of control whose text is being drawn
 * @param nPart		part of the control whose text is being drawn
 * @param nState	current state of the control
 * @param aValue	extra control-specific data of the ucrrent control state
 * @param textColor	location in which color to draw text should be returned
 * @return TRUE if a native widget text color is provided, FALSE if the standard
 *	VCL text color should be used.
 */
BOOL JavaSalGraphics::getNativeControlTextColor( ControlType nType, ControlPart nPart, ControlState nState, const ImplControlValue& aValue, SalColor& textColor )
{
	BOOL bReturn = FALSE;

#ifndef USE_NATIVE_CONTROLS
	if ( !IsNativeControlSupported( nType, nPart ) )
		return bReturn;
#endif	// !USE_NATIVE_CONTROLS

	if ( !HIThemeInitialize() )
		return bReturn;

	RGBColor nativeColor;

	switch( nType )
	{

		case CTRL_PUSHBUTTON:
		case CTRL_RADIOBUTTON:
		case CTRL_CHECKBOX:
			{				
				if( nState & CTRL_STATE_PRESSED )
				{
					bReturn = ( pGetThemeTextColor(kThemeTextColorPushButtonPressed, 32, true, &nativeColor) == noErr);
				}
				else if ( ! ( nState & CTRL_STATE_ENABLED ) )
				{
					bReturn = ( pGetThemeTextColor(kThemeTextColorPushButtonInactive, 32, true, &nativeColor) == noErr);
				}
				else
				{
					bReturn = ( pGetThemeTextColor(kThemeTextColorPushButtonActive, 32, true, &nativeColor) == noErr);
				}
			}
			break;

		case CTRL_LISTBOX:
			{
				if( nState & CTRL_STATE_PRESSED )
				{
					bReturn = ( pGetThemeTextColor(kThemeTextColorPopupButtonPressed, 32, true, &nativeColor) == noErr);
				}
				else if ( ! ( nState & CTRL_STATE_ENABLED ) )
				{
					bReturn = ( pGetThemeTextColor(kThemeTextColorPopupButtonInactive, 32, true, &nativeColor) == noErr);
				}
				else
				{
					bReturn = ( pGetThemeTextColor(kThemeTextColorPopupButtonActive, 32, true, &nativeColor) == noErr);
				}
			}
			break;

		case CTRL_TAB_ITEM:
			{
				if( nState & CTRL_STATE_SELECTED )
				{
					bReturn = ( pGetThemeTextColor(kThemeTextColorTabFrontActive, 32, true, &nativeColor) == noErr);
				}
				else if( nState & CTRL_STATE_PRESSED )
				{
					bReturn = ( pGetThemeTextColor(kThemeTextColorTabNonFrontPressed, 32, true, &nativeColor) == noErr);
				}
				else if ( ! ( nState & CTRL_STATE_ENABLED ) )
				{
					bReturn = ( pGetThemeTextColor(kThemeTextColorTabNonFrontInactive, 32, true, &nativeColor) == noErr);
				}
				else
				{
					bReturn = ( pGetThemeTextColor(kThemeTextColorTabNonFrontActive, 32, true, &nativeColor) == noErr);
				}
			}
			break;

		case CTRL_MENU_POPUP:
			{
				if( nState & CTRL_STATE_SELECTED )
				{
					bReturn = ( pGetThemeTextColor(kThemeTextColorMenuItemSelected , 32, true, &nativeColor) == noErr);
				}
				else if ( ! ( nState & CTRL_STATE_ENABLED ) )
				{
					bReturn = ( pGetThemeTextColor(kThemeTextColorMenuItemDisabled, 32, true, &nativeColor) == noErr);
				}
				else
				{
					bReturn = ( pGetThemeTextColor(kThemeTextColorMenuItemActive, 32, true, &nativeColor) == noErr);
				}
			}
			break;
	}

	if( bReturn )
		textColor = ConvertRGBColorToSalColor( nativeColor );

	return bReturn;
}
