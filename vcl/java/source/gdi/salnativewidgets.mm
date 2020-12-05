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

#include <rtl/ustring.h>
#include <osl/module.h>
#include <vcl/decoview.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#import <objc/objc-class.h>
#include <postmac.h>
#undef check

#include "java/salbmp.h"
#include "java/saldata.hxx"
#include "java/salframe.h"
#include "java/salgdi.h"
#include "java/salinst.h"
#include "java/svsys.h"

#include "../java/VCLEventQueue_cocoa.h"

// Comment out the following line to disable native controls
#define USE_NATIVE_CONTROLS

// Comment out the following line to disable native frame
#define USE_NATIVE_CTRL_FRAME

// Uncomment the following line to enable native group box
// #define USE_NATIVE_CTRL_GROUPBOX

#define COMBOBOX_BUTTON_WIDTH			19
#define COMBOBOX_HEIGHT					28
#define COMBOBOX_HEIGHT_SLOP			1
#define CONTROL_TAB_PANE_LEFT_OFFSET	6
#define CONTROL_TAB_PANE_TOP_OFFSET		28
#define CONTROL_TAB_PANE_RIGHT_OFFSET	6
#define CONTROL_TAB_PANE_BOTTOM_OFFSET	9
// Fix bug 3378 by reducing the editbox height for low screen resolutions.
// Increase adjustment factor slightly to increase height in text fields in the
// Preference dialog's User Data panel.
#define EDITBOX_HEIGHT					( Application::GetSettings().GetStyleSettings().GetToolFont().GetFontHeight() * 3 )
#define EDITBOX_HEIGHT_SLOP				( IsRunningHighSierraOrLower() ? 0 : 1 )
#define EDITFRAMEPADDING_WIDTH			1
#define FOCUSRING_WIDTH					3
#define FRAME_TRIMWIDTH					1
#define LISTBOX_BUTTON_WIDTH			19
#define SPINNER_WIDTH_SLOP				1
#define SPINNER_FOCUSRING_LEFT_OFFSET	0
#define SPINNER_FOCUSRING_TOP_OFFSET	1
#define SPINNER_FOCUSRING_RIGHT_OFFSET	0
#define SPINNER_FOCUSRING_BOTTOM_OFFSET	-1
#define SPINNER_FOCUSRING_ROUNDED_RECT_RADIUS	4
#define PROGRESSBAR_HEIGHT_SLOP			0
#define PROGRESSBARPADDING_HEIGHT		1
// Fix most cases of checkbox and radio button clipping reported in the
// following NeoOffice forum post by setting their width and height to the
// minimum amount that will not result in a clipped focus ring when drawn:
// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=64288#64288
#define CHECKBOX_WIDTH					14
#define CHECKBOX_HEIGHT					14
#define RADIOBUTTON_WIDTH				14
#define RADIOBUTTON_HEIGHT				14
#define PUSHBUTTON_HEIGHT_SLOP			1
#define PUSHBUTTON_DEFAULT_ALPHA		0.5f
#define DISCLOSUREBTN_WIDTH_SLOP		-2
#define TABITEM_FOCUSRING_LEFT_OFFSET	2
#define TABITEM_FOCUSRING_TOP_OFFSET	1
#define TABITEM_FOCUSRING_RIGHT_OFFSET	2
#define TABITEM_FOCUSRING_BOTTOM_OFFSET	2
#define TABITEM_FOCUSRING_ROUNDED_RECT_RADIUS	5

using namespace osl;
using namespace vcl;

struct SAL_DLLPRIVATE VCLBitmapBuffer : BitmapBuffer
{
	CGContextRef			maContext;
	MutexGuard*				mpGraphicsMutexGuard;
	bool					mbLastDrawToPrintGraphics;
	bool					mbUseLayer;

							VCLBitmapBuffer();
	virtual					~VCLBitmapBuffer();

	bool					Create( long nX, long nY, long nWidth, long nHeight, JavaSalGraphics *pGraphics, bool bUseLayer = true );
	void					Destroy();
	void					DrawContextAndDestroy( JavaSalGraphics *pGraphics, CGRect aSrcRect, CGRect aDestRect );
	void					ReleaseContext();
};

static bool bIsRunningHighSierraOrLowerInitizalized  = false;
static bool bIsRunningHighSierraOrLower = false;
static bool bIsRunningCatalinaOrLowerInitizalized  = false;
static bool bIsRunningCatalinaOrLower = false;

static VCLBitmapBuffer aSharedComboBoxBuffer;
static VCLBitmapBuffer aSharedListBoxBuffer;
static VCLBitmapBuffer aSharedHorizontalScrollBarBuffer;
static VCLBitmapBuffer aSharedVerticalScrollBarBuffer;
static VCLBitmapBuffer aSharedScrollBarBuffer;
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

inline long Float32ToLong( Float32 f ) { return static_cast< long >( f + 0.5 ); }

// =======================================================================

static bool IsRunningHighSierraOrLower()
{
	if ( !bIsRunningHighSierraOrLowerInitizalized )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSProcessInfo *pProcessInfo = [NSProcessInfo processInfo];
		if ( pProcessInfo )
		{
			NSOperatingSystemVersion aVersion = pProcessInfo.operatingSystemVersion;
			if ( aVersion.majorVersion <= 10 && aVersion.minorVersion <= 13 )
				bIsRunningHighSierraOrLower = true;
		}

		bIsRunningHighSierraOrLowerInitizalized = true;

		[pPool release];
	}

	return bIsRunningHighSierraOrLower;
}

static bool IsRunningCatalinaOrLower()
{
	if ( !bIsRunningCatalinaOrLowerInitizalized )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSProcessInfo *pProcessInfo = [NSProcessInfo processInfo];
		if ( pProcessInfo )
		{
			NSOperatingSystemVersion aVersion = pProcessInfo.operatingSystemVersion;
			if ( aVersion.majorVersion <= 10 && aVersion.minorVersion <= 15 )
				bIsRunningCatalinaOrLower = true;
		}

		bIsRunningCatalinaOrLowerInitizalized = true;

		[pPool release];
	}

	return bIsRunningCatalinaOrLower;
}

// =======================================================================

@interface VCLNativeControlWindow : NSWindow
{
	BOOL					mbInactive;
}
+ (id)createAndAttachToView:(NSView *)pView controlState:(ControlState)nControlState;
- (BOOL)_hasActiveControls;
- (id)initWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nStyle backing:(NSBackingStoreType)nBufferingType defer:(BOOL)bDeferCreation;
- (id)initWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nStyle backing:(NSBackingStoreType)nBufferingType defer:(BOOL)bDeferCreation screen:(NSScreen *)pScreen;
- (BOOL)isKeyWindow;
- (void)orderFrontRegardless;
- (void)orderWindow:(NSWindowOrderingMode)nOrderingMode relativeTo:(NSInteger)nOtherWindowNumber;
- (void)setInactive:(BOOL)bInactive;
@end

@implementation VCLNativeControlWindow

+ (id)createAndAttachToView:(NSView *)pView controlState:(ControlState)nControlState
{
	VCLNativeControlWindow *pRet = nil;

	if ( pView && ( ![pView isKindOfClass:[NSControl class]] || [static_cast< NSControl* >( pView ) isEnabled] ) )
	{
		// Fix slowness on macOS 11 by only attaching inactive views as active
		// views don't need to be attached on macOS 10.14 and higher
		BOOL bInactive = ( nControlState & ControlState::INACTIVE ? YES : NO );
		if ( !bInactive && !IsRunningHighSierraOrLower() && ![pView isKindOfClass:[NSProgressIndicator class]] )
			return pRet;

		pRet = [[VCLNativeControlWindow alloc] initWithContentRect:[pView frame] styleMask:NSWindowStyleMaskBorderless backing:NSBackingStoreBuffered defer:YES];
		if ( pRet )
		{
			[pRet autorelease];
			[pRet setReleasedWhenClosed:NO];
			[pRet setContentView:pView];
			[pRet setInactive:bInactive];
		}
	}

	return pRet;
}

- (BOOL)_hasActiveControls
{
	BOOL bActive = !mbInactive;
	if ( bActive )
	{
		// If the application is not active, then force inactive state
		NSApplication *pApp = [NSApplication sharedApplication];
		if ( pApp && ![pApp isActive] )
			bActive = NO;
	}

	return bActive;
}

- (id)initWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nStyle backing:(NSBackingStoreType)nBufferingType defer:(BOOL)bDeferCreation
{
	[super initWithContentRect:aContentRect styleMask:nStyle backing:nBufferingType defer:bDeferCreation];

	mbInactive = NO;

	return self;
}

- (id)initWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nStyle backing:(NSBackingStoreType)nBufferingType defer:(BOOL)bDeferCreation screen:(NSScreen *)pScreen
{
	[super initWithContentRect:aContentRect styleMask:nStyle backing:nBufferingType defer:bDeferCreation screen:pScreen];

	mbInactive = NO;

	return self;
}

- (BOOL)isKeyWindow
{
	return [self _hasActiveControls];
}

- (void)orderFrontRegardless
{
	[self orderWindow:NSWindowOut relativeTo:0];
}

- (void)orderWindow:(NSWindowOrderingMode)nOrderingMode relativeTo:(NSInteger)nOtherWindowNumber
{
	(void)nOrderingMode;
	(void)nOtherWindowNumber;

	[super orderWindow:NSWindowOut relativeTo:0];
}

- (void)setInactive:(BOOL)bInactive
{
	mbInactive = bInactive;
}

@end

// =======================================================================

@interface VCLNativeControlView : NSView
{
	BOOL					mbFlipped;
}
+ (id)createWithControl:(NSControl *)pControl;
- (id)initWithControl:(NSControl *)pControl;
- (BOOL)isFlipped;
@end

@implementation VCLNativeControlView

+ (id)createWithControl:(NSControl *)pControl
{
	VCLNativeControlView *pRet = nil;

	if ( pControl )
	{
		pRet = [[VCLNativeControlView alloc] initWithControl:pControl];
		if ( pRet )
			[pRet autorelease];
	}

	return pRet;
}

- (id)initWithControl:(NSControl *)pControl
{
	[super initWithFrame:[pControl frame]];

	mbFlipped = [pControl isFlipped];

	return self;
}

- (BOOL)isFlipped
{
	return mbFlipped;
}

@end

// =======================================================================

@interface VCLNativeButton : NSObject
{
	NSButtonType			mnButtonType;
	NSBezelStyle			mnBezelStyle;
	NSControlSize			mnControlSize;
	NSInteger				mnButtonState;
	ControlState			mnControlState;
	BOOL					mbDrawRTL;
	VCLBitmapBuffer*		mpBuffer;
	JavaSalGraphics*		mpGraphics;
	CGRect					maDestRect;
	bool					mbDrawn;
	bool					mbRedraw;
	NSSize					maSize;
}
+ (id)createWithButtonType:(NSButtonType)nButtonType bezelStyle:(NSBezelStyle)nBezelStyle controlSize:(NSControlSize)nControlSize buttonState:(NSInteger)nButtonState controlState:(ControlState)nControlState drawRTL:(BOOL)bDrawRTL bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect;
- (NSButton *)button;
- (void)draw:(id)pObject;
- (bool)drawn;
- (void)getSize:(id)pObject;
- (id)initWithButtonType:(NSButtonType)nButtonType bezelStyle:(NSBezelStyle)nBezelStyle controlSize:(NSControlSize)nControlSize buttonState:(NSInteger)nButtonState controlState:(ControlState)nControlState drawRTL:(BOOL)bDrawRTL bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect;
- (bool)redraw;
- (NSSize)size;
@end

@implementation VCLNativeButton

+ (id)createWithButtonType:(NSButtonType)nButtonType bezelStyle:(NSBezelStyle)nBezelStyle controlSize:(NSControlSize)nControlSize buttonState:(NSInteger)nButtonState controlState:(ControlState)nControlState drawRTL:(BOOL)bDrawRTL bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect
{
	VCLNativeButton *pRet = [[VCLNativeButton alloc] initWithButtonType:nButtonType bezelStyle:nBezelStyle controlSize:nControlSize buttonState:nButtonState controlState:nControlState drawRTL:bDrawRTL bitmapBuffer:pBuffer graphics:pGraphics destRect:aDestRect];
	[pRet autorelease];
	return pRet;
}

- (NSButton *)button
{
	NSButton *pButton = [[NSButton alloc] initWithFrame:NSMakeRect( 0, 0, 1, 1 )];
	if ( !pButton )
		return nil;

	[pButton autorelease];

	NSCell *pCell = [pButton cell];
	if ( !pCell )
		return nil;

	[pButton setButtonType:mnButtonType];
	[pButton setBezelStyle:mnBezelStyle];
	[pButton setState:mnButtonState];
	[pButton setTitle:@""];
	[pCell setControlSize:mnControlSize];

	if ( mnControlState & ( ControlState::PRESSED | ControlState::SELECTED ) )
	{
		[pButton setEnabled:YES];
		[pButton highlight:YES];
	}
	else if ( mnControlState & ControlState::ENABLED )
	{
		[pButton setEnabled:YES];
		[pButton highlight:NO];
	}
	else
	{
		[pButton setEnabled:NO];
		[pButton highlight:NO];
	}

	if ( mnControlState & ControlState::FOCUSED )
		[pCell setShowsFirstResponder:YES];
	else
		[pCell setShowsFirstResponder:NO];

	// The enabled state is controlled by the [NSWindow _hasActiveControls]
	// selector so we need to attach a custom hidden window to draw enabled
	if ( mnButtonType == NSButtonTypeMomentaryLight || mnControlState & ControlState::INACTIVE )
		[VCLNativeControlWindow createAndAttachToView:pButton controlState:mnControlState];

	[pButton sizeToFit];

	return pButton;
}

- (void)draw:(id)pObject
{
	(void)pObject;

	if ( !mbDrawn && mpBuffer && mpGraphics && !CGRectIsEmpty( maDestRect ) )
	{
		mbRedraw = false;

		NSButton *pButton = [self button];
		if ( pButton )
		{
			NSCell *pCell = [pButton cell];
			if ( pCell )
			{
				float fCellHeight = [pCell cellSize].height;
				float fOffscreenHeight = maDestRect.size.height;
				BOOL bPlacard = NO;
				BOOL bAttachToKeyWindow = NO;
				if ( mnButtonType == NSButtonTypeMomentaryLight )
				{
					fCellHeight -= ( FOCUSRING_WIDTH * 2 );
					if ( fCellHeight <= 0 )
						fCellHeight = maDestRect.size.height;
					fOffscreenHeight = ( maDestRect.size.height > fCellHeight ? maDestRect.size.height : fCellHeight );
					bPlacard = ( fOffscreenHeight * 1.5 >= maDestRect.size.width );
					if ( bPlacard )
					{
						fOffscreenHeight = maDestRect.size.height;
						[pButton setBezelStyle:NSBezelStyleShadowlessSquare];
					}
					else
					{
						// The default adornment hides the pressed state so set
						// the adornment to none if the button is pressed. Also,
						// push buttons should never have a focus ring so treat
						// them as default buttons.
						[pCell setShowsFirstResponder:NO];

						if ( mnControlState & ( ControlState::DEFAULT | ControlState::FOCUSED ) && ! ( mnControlState & ( ControlState::PRESSED | ControlState::SELECTED ) ) )
						{
							[pButton setKeyEquivalent:@"\r"];
							if ( ! ( mnControlState & ControlState::INACTIVE ) )
								bAttachToKeyWindow = YES;
						}
					}
				}

				CGRect aAdjustedDestRect = CGRectMake( 0, 0, maDestRect.size.width, fOffscreenHeight );
				if ( mpBuffer->Create( static_cast< long >( maDestRect.origin.x ), static_cast< long >( maDestRect.origin.y ), static_cast< long >( maDestRect.size.width ), static_cast< long >( fOffscreenHeight ), mpGraphics, fOffscreenHeight == maDestRect.size.height ) )
				{
					CGContextSaveGState( mpBuffer->maContext );
					if ( [pButton isFlipped] )
					{
						CGContextTranslateCTM( mpBuffer->maContext, 0, aAdjustedDestRect.size.height );
						CGContextScaleCTM( mpBuffer->maContext, 1.0f, -1.0f );
					}

					if ( mbDrawRTL )
					{
						CGContextTranslateCTM( mpBuffer->maContext, aAdjustedDestRect.size.width, 0 );
						CGContextScaleCTM( mpBuffer->maContext, -1.0f, 1.0f );
					}

					NSGraphicsContext *pContext = [NSGraphicsContext graphicsContextWithCGContext:mpBuffer->maContext flipped:YES];
					if ( pContext )
					{
						NSRect aDrawRect = NSRectFromCGRect( aAdjustedDestRect );
						if ( mnButtonType == NSButtonTypeMomentaryLight && !bPlacard )
						{
							// Fix bug 1633 by vertically centering button
							aDrawRect.origin.y += ( ( fOffscreenHeight - fCellHeight ) / 2 ) + PUSHBUTTON_HEIGHT_SLOP;
							aDrawRect.size.height = fCellHeight;
						}
						else if ( mnBezelStyle == NSBezelStyleDisclosure )
						{
							// Horizontally align disclosure button outward
							aDrawRect.origin.x += DISCLOSUREBTN_WIDTH_SLOP;
							aDrawRect.size.width = [pCell cellSize].width;
						}

						NSGraphicsContext *pOldContext = [NSGraphicsContext currentContext];
						[NSGraphicsContext setCurrentContext:pContext];

						// Fix drawing of default button color on macOS 10.12 by
						// temporarily attaching the button to the key window
						if ( bAttachToKeyWindow )
						{
							NSApplication *pApp = [NSApplication sharedApplication];
							if ( pApp )
							{
								NSWindow *pKeyWindow = [pApp keyWindow];
								if ( pKeyWindow )
								{
									// Fix crash in Mac App Store version when
									// creating a new database by not attaching
									// to the window if the content view is an
									// instance of NSRemoteView
									VCLView *pContentView = [pKeyWindow contentView];
									if ( pContentView && [pContentView isKindOfClass:[VCLView class]] )
									{
										NSRect aBounds = [pButton bounds];
										[pButton removeFromSuperview];
										// Ensure that the button is not
										// visible in the key window
										[pButton setFrameOrigin:NSMakePoint( aBounds.size.width * -2, aBounds.size.height * -2 )];
										[pContentView addSubview:pButton positioned:NSWindowBelow relativeTo:nil];
									}
								}
							}
						}

						// Fix failure to draw on macOS 10.14 by passing a
						// regular view instead of the control itself. Fix
						// failure to draw control in inactive state on
						// macOS 10.14 by attaching view to the control's
						// window.
						NSView *pControlView = ( IsRunningHighSierraOrLower() ? nil : [VCLNativeControlView createWithControl:pButton] );
						if ( pControlView )
						{
							NSWindow *pWindow = [pButton window];
							if ( pWindow )
							{
								NSView *pContentView = [pWindow contentView];
								if ( pContentView )
									[pContentView addSubview:pControlView positioned:NSWindowBelow relativeTo:nil];
							}
						}

						CGContextClipToRect( mpBuffer->maContext, aAdjustedDestRect );
						// CGContextBeginTransparencyLayerWithRect( mpBuffer->maContext, aAdjustedDestRect, nullptr );
						[pCell drawWithFrame:aDrawRect inView:( pControlView ? pControlView : pButton )];
						// CGContextEndTransparencyLayer( mpBuffer->maContext );

						if ( mbRedraw )
						{
							// Emulate pulse by painting pressed button on top
							// of the default button with varying alpha
							float fAlpha = 0.15f;
							double fTime = CFAbsoluteTimeGetCurrent();
							fAlpha += fAlpha * sin( ( fTime - static_cast< long >( fTime ) ) * 2 * M_PI );
							fAlpha += PUSHBUTTON_DEFAULT_ALPHA;
							if ( fAlpha > 0 )
							{
								CGContextSetAlpha( mpBuffer->maContext, fAlpha > 1.0f ? 1.0f : fAlpha );
								// CGContextBeginTransparencyLayerWithRect( mpBuffer->maContext, aAdjustedDestRect, nullptr );

								[pButton highlight:YES];
								[pCell drawWithFrame:aDrawRect inView:pButton];

								// CGContextEndTransparencyLayer( mpBuffer->maContext );
							}
						}

						if ( pControlView )
							[pControlView removeFromSuperview];

						if ( bAttachToKeyWindow )
							[pButton removeFromSuperview];

						[NSGraphicsContext setCurrentContext:pOldContext];

						mbDrawn = true;
					}

					CGContextRestoreGState( mpBuffer->maContext );

					mpBuffer->ReleaseContext();

					if ( mbDrawn )
						mpBuffer->DrawContextAndDestroy( mpGraphics, aAdjustedDestRect, maDestRect );
				}
			}
		}
	}
}

- (bool)drawn
{
	return mbDrawn;
}

- (void)getSize:(id)pObject
{
	(void)pObject;

	if ( NSEqualSizes( maSize, NSZeroSize ) )
	{
		NSButton *pButton = [self button];
		if ( pButton )
		{
			NSCell *pCell = [pButton cell];
			if ( pCell )
				maSize = [pCell cellSize];
		}
	}
}

- (id)initWithButtonType:(NSButtonType)nButtonType bezelStyle:(NSBezelStyle)nBezelStyle controlSize:(NSControlSize)nControlSize buttonState:(NSInteger)nButtonState controlState:(ControlState)nControlState drawRTL:(BOOL)bDrawRTL bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect
{
	[super init];

	mnButtonType = nButtonType;
	mnBezelStyle = nBezelStyle;
	mnControlSize = nControlSize;
	mnButtonState = nButtonState;
	mnControlState = nControlState;
	mbDrawRTL = bDrawRTL;
	mpBuffer = pBuffer;
	mpGraphics = pGraphics;
	maDestRect = aDestRect;
	mbDrawn = false;
	mbRedraw = false;
	maSize = NSZeroSize;

	return self;
}

- (bool)redraw
{
	return mbRedraw;
}

- (NSSize)size
{
	return maSize;
}

@end

// =======================================================================

@interface VCLNativeComboBox : NSObject
{
	ControlState			mnControlState;
	BOOL					mbEditable;
	VCLBitmapBuffer*		mpBuffer;
	JavaSalGraphics*		mpGraphics;
	CGRect					maDestRect;
	bool					mbDrawn;
	BOOL					mbRTL;
	NSSize					maSize;
}
+ (id)createWithControlState:(ControlState)nControlState editable:(BOOL)bEditable bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect;
- (NSControl *)comboBox;
- (NSControl *)popUpButton;
- (void)draw:(id)pObject;
- (bool)drawn;
- (void)getSize:(id)pObject;
- (id)initWithControlState:(ControlState)nControlState editable:(BOOL)bEditable bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect;
- (BOOL)isRTL;
- (NSSize)size;
@end

static NSComboBox *pSharedComboBox = nil;

@implementation VCLNativeComboBox

+ (id)createWithControlState:(ControlState)nControlState editable:(BOOL)bEditable bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect
{
	VCLNativeComboBox *pRet = [[VCLNativeComboBox alloc] initWithControlState:nControlState editable:bEditable bitmapBuffer:pBuffer graphics:pGraphics destRect:aDestRect];
	[pRet autorelease];
	return pRet;
}

- (NSControl *)comboBox
{
	NSComboBox *pComboBox;
	if ( IsRunningHighSierraOrLower() )
	{
		pComboBox = [[NSComboBox alloc] initWithFrame:NSMakeRect( 0, 0, 1, 1 )];
		if ( !pComboBox )
			return nil;

		[pComboBox autorelease];
	}
	else
	{
		// Fix slowness on macOS 11 by reusing the same combobox
		if ( !pSharedComboBox )
			pSharedComboBox = [[NSComboBox alloc] initWithFrame:NSMakeRect( 0, 0, 1, 1 )];
		pComboBox = pSharedComboBox;
		if ( !pComboBox )
			return nil;

		[pComboBox setFrame:NSMakeRect( 0, 0, 1, 1 )];
	}

	NSCell *pCell = [pComboBox cell];
	if ( !pCell )
		return nil;

	NSButtonCell *pButtonCell = nil;
	@try
	{
		pButtonCell = [pCell valueForKey:@"_buttonCell"];
	}
	@catch ( NSException *pExc )
	{
	}

	if ( !pButtonCell || ![pButtonCell isKindOfClass:[NSButtonCell class]] )
		return nil;

	if ( mnControlState & ( ControlState::PRESSED | ControlState::SELECTED ) )
	{
		[pComboBox setEnabled:YES];
		[pButtonCell setHighlighted:YES];
	}
	else if ( mnControlState & ControlState::ENABLED )
	{
		[pComboBox setEnabled:YES];
		[pButtonCell setHighlighted:NO];
	}
	else
	{
		[pComboBox setEnabled:NO];
		[pButtonCell setHighlighted:NO];
	}

	if ( mnControlState & ControlState::FOCUSED )
		[pCell setShowsFirstResponder:YES];
	else
		[pCell setShowsFirstResponder:NO];

	// The enabled state is controlled by the [NSWindow _hasActiveControls]
	// selector so we need to attach a custom hidden window to draw enabled
	[VCLNativeControlWindow createAndAttachToView:pComboBox controlState:mnControlState];

	[pComboBox sizeToFit];

	return pComboBox;
}

- (NSControl *)popUpButton
{
	NSPopUpButton *pPopUpButton = [[NSPopUpButton alloc] initWithFrame:NSMakeRect( 0, 0, 1, 1 )];
	if ( !pPopUpButton )
		return nil;

	[pPopUpButton autorelease];

	NSCell *pCell = [pPopUpButton cell];
	if ( !pCell )
		return nil;

	[pPopUpButton setPullsDown:YES];

	if ( mnControlState & ( ControlState::PRESSED | ControlState::SELECTED ) )
	{
		[pPopUpButton setEnabled:YES];
		[pCell setHighlighted:YES];
	}
	else if ( mnControlState & ControlState::ENABLED )
	{
		[pPopUpButton setEnabled:YES];
		[pCell setHighlighted:NO];
	}
	else
	{
		[pPopUpButton setEnabled:NO];
		[pCell setHighlighted:NO];
	}

	if ( mnControlState & ControlState::FOCUSED )
		[pCell setShowsFirstResponder:YES];
	else
		[pCell setShowsFirstResponder:NO];

	// The enabled state is controlled by the [NSWindow _hasActiveControls]
	// selector so we need to attach a custom hidden window to draw enabled
	[VCLNativeControlWindow createAndAttachToView:pPopUpButton controlState:mnControlState];

	[pPopUpButton sizeToFit];

	return pPopUpButton;
}

- (void)draw:(id)pObject
{
	(void)pObject;

	if ( !mbDrawn && mpBuffer && mpGraphics && !CGRectIsEmpty( maDestRect ) )
	{
		NSControl *pControl = ( mbEditable ? [self comboBox] : [self popUpButton] );
		if ( pControl )
		{
			NSCell *pCell = [pControl cell];
			if ( pCell )
			{
				float fCellHeight = [pCell cellSize].height - ( FOCUSRING_WIDTH * 2 );
				if ( fCellHeight <= 0 )
					fCellHeight = maDestRect.size.height;
				float fOffscreenHeight = ( maDestRect.size.height > fCellHeight ? maDestRect.size.height : fCellHeight );
				CGRect aAdjustedDestRect = CGRectMake( 0, 0, maDestRect.size.width, fOffscreenHeight );
				if ( mpBuffer->Create( static_cast< long >( maDestRect.origin.x ), static_cast< long >( maDestRect.origin.y ), static_cast< long >( maDestRect.size.width ), static_cast< long >( fOffscreenHeight ), mpGraphics, fOffscreenHeight == maDestRect.size.height ) )
				{
					CGContextSaveGState( mpBuffer->maContext );
					if ( [pControl isFlipped] )
					{
						CGContextTranslateCTM( mpBuffer->maContext, 0, aAdjustedDestRect.size.height );
						CGContextScaleCTM( mpBuffer->maContext, 1.0f, -1.0f );
					}

					NSGraphicsContext *pContext = [NSGraphicsContext graphicsContextWithCGContext:mpBuffer->maContext flipped:YES];
					if ( pContext )
					{
						NSRect aDrawRect = NSRectFromCGRect( aAdjustedDestRect );
						// Shift combobox edge to right
						if ( mbEditable )
						{
							aDrawRect.origin.x += FOCUSRING_WIDTH;
							aDrawRect.size.width -= FOCUSRING_WIDTH;
						}

						// Vertically center control
						aDrawRect.origin.y += ( ( fOffscreenHeight - fCellHeight ) / 2 ) + COMBOBOX_HEIGHT_SLOP;
						aDrawRect.size.height = fCellHeight;

						NSGraphicsContext *pOldContext = [NSGraphicsContext currentContext];
						[NSGraphicsContext setCurrentContext:pContext];

						// Fix failure to draw on macOS 10.14 by passing a
						// regular view instead of the control itself. Fix
						// failure to draw control in inactive state on
						// macOS 10.14 by attaching view to the control's
						// window.
						NSView *pControlView = ( IsRunningHighSierraOrLower() ? nil : [VCLNativeControlView createWithControl:pControl] );
						if ( pControlView )
						{
							NSWindow *pWindow = [pControl window];
							if ( pWindow )
							{
								NSView *pContentView = [pWindow contentView];
								if ( pContentView )
									[pContentView addSubview:pControlView positioned:NSWindowBelow relativeTo:nil];
							}
						}

						CGContextClipToRect( mpBuffer->maContext, aAdjustedDestRect );
						// CGContextBeginTransparencyLayerWithRect( mpBuffer->maContext, aAdjustedDestRect, nullptr );
						[pCell drawWithFrame:aDrawRect inView:( pControlView ? pControlView : pControl )];
						// CGContextEndTransparencyLayer( mpBuffer->maContext );

						if ( pControlView )
							[pControlView removeFromSuperview];

						[NSGraphicsContext setCurrentContext:pOldContext];

						mbDrawn = true;
					}

					CGContextRestoreGState( mpBuffer->maContext );

					mpBuffer->ReleaseContext();

					if ( mbDrawn )
						mpBuffer->DrawContextAndDestroy( mpGraphics, aAdjustedDestRect, maDestRect );
				}
			}
		}
	}
}

- (bool)drawn
{
	return mbDrawn;
}

- (void)getSize:(id)pObject
{
	(void)pObject;

	if ( NSEqualSizes( maSize, NSZeroSize ) )
	{
		NSControl *pControl = ( mbEditable ? [self comboBox] : [self popUpButton] );
		if ( pControl )
		{
			NSCell *pCell = [pControl cell];
			if ( pCell )
			{
				mbRTL = ( [pCell userInterfaceLayoutDirection] == NSUserInterfaceLayoutDirectionRightToLeft );
				maSize = [pCell cellSize];
			}
		}
	}
}

- (id)initWithControlState:(ControlState)nControlState editable:(BOOL)bEditable bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect
{
	[super init];

	mnControlState = nControlState;
	mbEditable = bEditable;
	mpBuffer = pBuffer;
	mpGraphics = pGraphics;
	maDestRect = aDestRect;
	mbDrawn = false;
	mbRTL = NO;
	maSize = NSZeroSize;

	return self;
}

- (BOOL)isRTL
{
	return mbRTL;
}

- (NSSize)size
{
	return maSize;
}

@end

// =======================================================================

@interface NSScroller (VCLNativeScrollbar)
- (void)drawArrow:(NSUInteger)nArrow highlightPart:(NSUInteger)nPart;
- (id)scrollerImp;
- (void)scrollerImp:(id)pObject animateUIStateTransitionWithDuration:(double)fDuration;
@end

@interface VCLNativeScrollbar : NSObject
{
	ControlState			mnControlState;
	VCLBitmapBuffer*		mpBuffer;
	JavaSalGraphics*		mpGraphics;
	const ScrollbarValue*	mpScrollbarValue;
	BOOL					mbDoubleScrollbarArrows;
	CGRect					maDestRect;
	bool					mbDrawn;
	BOOL					mbHorizontal;
	BOOL					mbDrawOnlyTrack;
	NSRect					maDecrementArrowBounds;
	NSRect					maIncrementArrowBounds;
	NSRect					maDecrementPageBounds;
	NSRect					maIncrementPageBounds;
	NSRect					maThumbBounds;
	NSRect					maTrackBounds;
	NSRect					maTotalBounds;
}
+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics scrollbarValue:(const ScrollbarValue *)pScrollbarValue doubleScrollbarArrows:(BOOL)bDoubleScrollbarArrows destRect:(CGRect)aDestRect;
- (NSScroller *)scroller;
- (void)draw:(id)pObject;
- (bool)drawn;
- (void)getBounds:(id)pObject;
- (BOOL)horizontal;
- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics scrollbarValue:(const ScrollbarValue *)pScrollbarValue doubleScrollbarArrows:(BOOL)bDoubleScrollbarArrows destRect:(CGRect)aDestRect;
- (NSRect)decrementArrowBounds;
- (NSRect)incrementArrowBounds;
- (NSRect)decrementPageBounds;
- (NSRect)incrementPageBounds;
- (NSRect)thumbBounds;
- (NSRect)trackBounds;
- (NSRect)totalBounds;
@end

@implementation VCLNativeScrollbar

+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics scrollbarValue:(const ScrollbarValue *)pScrollbarValue doubleScrollbarArrows:(BOOL)bDoubleScrollbarArrows destRect:(CGRect)aDestRect
{
	VCLNativeScrollbar *pRet = [[VCLNativeScrollbar alloc] initWithControlState:nControlState bitmapBuffer:pBuffer graphics:pGraphics scrollbarValue:pScrollbarValue doubleScrollbarArrows:bDoubleScrollbarArrows destRect:aDestRect];
	[pRet autorelease];
	return pRet;
}

- (NSScroller *)scroller
{
	NSScroller *pScroller = [[NSScroller alloc] initWithFrame:NSMakeRect( 0, 0, maDestRect.size.width, maDestRect.size.height )];
	if ( !pScroller )
		return nil;

	[pScroller autorelease];

	// Always use NSScrollerStyleLegacy scrollbars as we do not support
	// autohiding scrollbars
	pScroller.scrollerStyle = NSScrollerStyleLegacy;

	// For scrollers, the inactive state is the same as the disabled state
	if ( mnControlState & ControlState::ENABLED && ! ( mnControlState & ControlState::INACTIVE ) )
		[pScroller setEnabled:YES];
	else
		[pScroller setEnabled:NO];

	if ( mpScrollbarValue )
	{
		float fTrackRange = static_cast< float >( mpScrollbarValue->mnMax - mpScrollbarValue->mnVisibleSize - mpScrollbarValue->mnMin );
		float fTrackPosition = static_cast< float >( mpScrollbarValue->mnCur - mpScrollbarValue->mnMin );
		if ( mpScrollbarValue->mnVisibleSize > 0 && fTrackRange > 0 )
		{
			mbDrawOnlyTrack = NO;
			[pScroller setDoubleValue:fTrackPosition / fTrackRange];
			[pScroller setKnobProportion:static_cast< float >( mpScrollbarValue->mnVisibleSize ) / ( fTrackRange + mpScrollbarValue->mnVisibleSize )];
		}
		else
		{
			// Set the value and knob proportion with "reasonable" values.
			// Fix bug 3359 by drawing on the scrollbar track.
			mbDrawOnlyTrack = YES;
			[pScroller setDoubleValue:0];
			[pScroller setKnobProportion:1.0];
		}

		if ( ( mpScrollbarValue->mnButton1State | mpScrollbarValue->mnButton2State | mpScrollbarValue->mnPage1State | mpScrollbarValue->mnPage2State | mpScrollbarValue->mnThumbState ) & ( ControlState::PRESSED | ControlState::ROLLOVER | ControlState::SELECTED ) )
		{
			// Darken thumb when mouse is within scroller on Mac OS X 10.7 and
			// higher
			if ( [pScroller respondsToSelector:@selector(scrollerImp)] && [pScroller respondsToSelector:@selector(scrollerImp:animateUIStateTransitionWithDuration:)] )
			{
				NSObject *pImp = [pScroller scrollerImp];
				if ( pImp )
					[pScroller scrollerImp:pImp animateUIStateTransitionWithDuration:1.0f];
			}
		}
	}

	// The enabled state is controlled by the [NSWindow _hasActiveControls]
	// selector so we need to attach a custom hidden window to draw enabled
	[VCLNativeControlWindow createAndAttachToView:pScroller controlState:mnControlState];

	[pScroller sizeToFit];

	// Force view size to match width of track otherwise the track will float
	// outward and downward
	NSRect aFrame = [pScroller frame];
	NSRect aTrackBounds = [pScroller rectForPart:NSScrollerKnobSlot];
	if ( aFrame.size.width > aFrame.size.height )
	{
		mbHorizontal = YES;
		aFrame.size.height = aTrackBounds.size.height;
	}
	else
	{
		mbHorizontal = NO;
		aFrame.size.width = aTrackBounds.size.width;
	}
	[pScroller setFrame:aFrame];

	return pScroller;
}

- (void)draw:(id)pObject
{
	(void)pObject;

	if ( !mbDrawn && mpBuffer && mpGraphics && !CGRectIsEmpty( maDestRect ) )
	{
		NSScroller *pScroller = [self scroller];
		if ( pScroller )
		{
			float fOffscreenHeight = maDestRect.size.height;
			CGRect aAdjustedDestRect = CGRectMake( 0, 0, maDestRect.size.width, fOffscreenHeight );
			if ( mpBuffer->Create( static_cast< long >( maDestRect.origin.x ), static_cast< long >( maDestRect.origin.y ), static_cast< long >( maDestRect.size.width ), static_cast< long >( fOffscreenHeight ), mpGraphics, fOffscreenHeight == maDestRect.size.height ) )
			{
				CGContextSaveGState( mpBuffer->maContext );
				if ( [pScroller isFlipped] )
				{
					CGContextTranslateCTM( mpBuffer->maContext, 0, aAdjustedDestRect.size.height );
					CGContextScaleCTM( mpBuffer->maContext, 1.0f, -1.0f );
				}

				NSGraphicsContext *pContext = [NSGraphicsContext graphicsContextWithCGContext:mpBuffer->maContext flipped:YES];
				if ( pContext )
				{
					NSGraphicsContext *pOldContext = [NSGraphicsContext currentContext];
					[NSGraphicsContext setCurrentContext:pContext];
					CGContextClipToRect( mpBuffer->maContext, aAdjustedDestRect );
					// CGContextBeginTransparencyLayerWithRect( mpBuffer->maContext, aAdjustedDestRect, nullptr );

					// Fix bug 2031 by always filling the background with white.
					// Fix incorrect dark mode drawing by filling with a system
					// color instead of white.
					if ( @available(macOS 10.14, * ) )
						[[NSColor unemphasizedSelectedContentBackgroundColor] set];
					else if ( class_getClassMethod( [NSColor class], @selector(scrollBarColor) ) )
						[[NSColor scrollBarColor] set];
					else
						[[NSColor controlBackgroundColor] set];
					[NSBezierPath fillRect:NSRectFromCGRect( aAdjustedDestRect )];

					[pScroller drawKnobSlotInRect:[pScroller rectForPart:NSScrollerKnobSlot] highlight:NO];
					if ( !mbDrawOnlyTrack )
						[pScroller drawKnob];

					// CGContextEndTransparencyLayer( mpBuffer->maContext );
					[NSGraphicsContext setCurrentContext:pOldContext];

					mbDrawn = true;
				}

				CGContextRestoreGState( mpBuffer->maContext );

				mpBuffer->ReleaseContext();

				if ( mbDrawn )
					mpBuffer->DrawContextAndDestroy( mpGraphics, aAdjustedDestRect, maDestRect );
			}
		}
	}
}

- (bool)drawn
{
	return mbDrawn;
}

- (void)getBounds:(id)pObject
{
	(void)pObject;

	if ( NSEqualRects( maTotalBounds, NSZeroRect ) )
	{
		NSScroller *pScroller = [self scroller];
		if ( pScroller )
		{
			BOOL bFlipped = [pScroller isFlipped];

			maDecrementPageBounds = [pScroller rectForPart:NSScrollerDecrementPage];
			if ( !bFlipped )
				maDecrementPageBounds.origin.y = maDestRect.size.height - maDecrementPageBounds.origin.y - maDecrementPageBounds.size.height;

			maIncrementPageBounds = [pScroller rectForPart:NSScrollerIncrementPage];
			if ( !bFlipped )
				maIncrementPageBounds.origin.y = maDestRect.size.height - maIncrementPageBounds.origin.y - maIncrementPageBounds.size.height;

			maThumbBounds = [pScroller rectForPart:NSScrollerKnob];
			if ( !bFlipped )
				maThumbBounds.origin.y = maDestRect.size.height - maThumbBounds.origin.y - maThumbBounds.size.height;

			maTrackBounds = [pScroller rectForPart:NSScrollerKnobSlot];
			if ( !bFlipped )
				maTrackBounds.origin.y = maDestRect.size.height - maTrackBounds.origin.y - maTrackBounds.size.height;

			maTotalBounds = [pScroller rectForPart:NSScrollerNoPart];
			if ( !bFlipped )
				maTotalBounds.origin.y = maDestRect.size.height - maTotalBounds.origin.y - maTotalBounds.size.height;
		}
	}
}

- (BOOL)horizontal
{
	return mbHorizontal;
}

- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics scrollbarValue:(const ScrollbarValue *)pScrollbarValue doubleScrollbarArrows:(BOOL)bDoubleScrollbarArrows destRect:(CGRect)aDestRect
{
	[super init];

	mnControlState = nControlState;
	mpBuffer = pBuffer;
	mpGraphics = pGraphics;
	mpScrollbarValue = pScrollbarValue;
	mbDoubleScrollbarArrows = bDoubleScrollbarArrows;
	maDestRect = aDestRect;
	mbDrawn = false;
	mbHorizontal = NO;
	maDecrementArrowBounds = NSZeroRect;
	maIncrementArrowBounds = NSZeroRect;
	maDecrementPageBounds = NSZeroRect;
	maIncrementPageBounds = NSZeroRect;
	maThumbBounds = NSZeroRect;
	maTrackBounds = NSZeroRect;
	maTotalBounds = NSZeroRect;

	return self;
}

- (NSRect)decrementArrowBounds
{
	return maDecrementArrowBounds;
}

- (NSRect)incrementArrowBounds
{
	return maIncrementArrowBounds;
}

- (NSRect)decrementPageBounds
{
	return maDecrementPageBounds;
}

- (NSRect)incrementPageBounds
{
	return maIncrementPageBounds;
}

- (NSRect)thumbBounds
{
	return maThumbBounds;
}

- (NSRect)trackBounds
{
	return maTrackBounds;
}

- (NSRect)totalBounds
{
	return maTotalBounds;
}

@end

// =======================================================================

@interface VCLNativeProgressbar : NSObject
{
	ControlState			mnControlState;
	NSControlSize			mnControlSize;
	VCLBitmapBuffer*		mpBuffer;
	JavaSalGraphics*		mpGraphics;
	const ImplControlValue*	mpControlValue;
	CGRect					maDestRect;
	bool					mbDrawn;
	NSSize					maSize;
}
+ (id)createWithControlState:(ControlState)nControlState controlSize:(NSControlSize)nControlSize bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics progressbarValue:(const ImplControlValue *)pControlValue destRect:(CGRect)aDestRect;
- (NSProgressIndicator *)progressIndicator;
- (void)draw:(id)pObject;
- (bool)drawn;
- (void)getSize:(id)pObject;
- (id)initWithControlState:(ControlState)nControlState controlSize:(NSControlSize)nControlSize bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics progressbarValue:(const ImplControlValue *)pControlValue destRect:(CGRect)aDestRect;
- (NSSize)size;
@end

@implementation VCLNativeProgressbar

+ (id)createWithControlState:(ControlState)nControlState controlSize:(NSControlSize)nControlSize bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics progressbarValue:(const ImplControlValue *)pControlValue destRect:(CGRect)aDestRect
{
	VCLNativeProgressbar *pRet = [[VCLNativeProgressbar alloc] initWithControlState:nControlState controlSize:static_cast< NSControlSize >( nControlSize ) bitmapBuffer:pBuffer graphics:pGraphics progressbarValue:pControlValue destRect:aDestRect];
	[pRet autorelease];
	return pRet;
}

- (NSProgressIndicator *)progressIndicator
{
	NSProgressIndicator *pProgressIndicator = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect( 0, 0, maDestRect.size.width, maDestRect.size.height / 2 )];
	if ( !pProgressIndicator )
		return nil;

	[pProgressIndicator autorelease];

	[pProgressIndicator setControlSize:mnControlSize];
	[pProgressIndicator setStyle:NSProgressIndicatorStyleBar];
	[pProgressIndicator setIndeterminate:NO];

	double fRange = [pProgressIndicator maxValue] - [pProgressIndicator minValue];
	if ( fRange < 1.0f )
		fRange = 1.0f;

	[pProgressIndicator setMinValue:0];
	[pProgressIndicator setMaxValue:fRange];
	[pProgressIndicator setDoubleValue:0];

	if ( mpControlValue )
	{
		double fValue = static_cast< double >( mpControlValue->getNumericVal() );
		if ( fValue > fRange )
			fValue = fRange;
		else if ( fValue < 0 )
			fValue = 0;
		[pProgressIndicator setDoubleValue:fValue];
	}

	// The enabled state is controlled by the [NSWindow _hasActiveControls]
	// selector so we need to attach a custom hidden window to draw enabled
	[VCLNativeControlWindow createAndAttachToView:pProgressIndicator controlState:mnControlState];

	[pProgressIndicator sizeToFit];

	return pProgressIndicator;
}

- (void)draw:(id)pObject
{
	(void)pObject;

	if ( !mbDrawn && mpBuffer && mpGraphics && !CGRectIsEmpty( maDestRect ) )
	{
		NSProgressIndicator *pProgressIndicator = [self progressIndicator];
		if ( pProgressIndicator )
		{
			float fOffscreenHeight = maDestRect.size.height;
			CGRect aAdjustedDestRect = CGRectMake( 0, 0, maDestRect.size.width, fOffscreenHeight );
			if ( mpBuffer->Create( static_cast< long >( maDestRect.origin.x ), static_cast< long >( maDestRect.origin.y ), static_cast< long >( maDestRect.size.width ), static_cast< long >( fOffscreenHeight ), mpGraphics, fOffscreenHeight == maDestRect.size.height ) )
			{
				CGContextSaveGState( mpBuffer->maContext );
				if ( [pProgressIndicator isFlipped] )
				{
					CGContextTranslateCTM( mpBuffer->maContext, 0, aAdjustedDestRect.size.height );
					CGContextScaleCTM( mpBuffer->maContext, 1.0f, -1.0f );
				}

				// Vertically center control. Note that we translate the
				// context because we must use the [NSView drawRect:] selector
				// to draw the control.
				CGContextTranslateCTM( mpBuffer->maContext, 0, ( ( fOffscreenHeight - [pProgressIndicator frame].size.height ) / 2 ) + PROGRESSBAR_HEIGHT_SLOP );

				NSGraphicsContext *pContext = [NSGraphicsContext graphicsContextWithCGContext:mpBuffer->maContext flipped:YES];
				if ( pContext )
				{
					NSGraphicsContext *pOldContext = [NSGraphicsContext currentContext];
					[NSGraphicsContext setCurrentContext:pContext];
					CGContextClipToRect( mpBuffer->maContext, aAdjustedDestRect );
					// CGContextBeginTransparencyLayerWithRect( mpBuffer->maContext, aAdjustedDestRect, nullptr );
					[pProgressIndicator drawRect:[pProgressIndicator frame]];
					// CGContextEndTransparencyLayer( mpBuffer->maContext );
					[NSGraphicsContext setCurrentContext:pOldContext];

					mbDrawn = true;
				}

				CGContextRestoreGState( mpBuffer->maContext );

				mpBuffer->ReleaseContext();

				if ( mbDrawn )
					mpBuffer->DrawContextAndDestroy( mpGraphics, aAdjustedDestRect, maDestRect );
			}
		}
	}
}

- (bool)drawn
{
	return mbDrawn;
}

- (void)getSize:(id)pObject
{
	(void)pObject;

	if ( NSEqualSizes( maSize, NSZeroSize ) )
	{
		NSProgressIndicator *pProgressIndicator = [self progressIndicator];
		if ( pProgressIndicator )
			maSize = [pProgressIndicator frame].size;
	}
}

- (id)initWithControlState:(ControlState)nControlState controlSize:(NSControlSize)nControlSize bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics progressbarValue:(const ImplControlValue *)pControlValue destRect:(CGRect)aDestRect
{
	[super init];

	mnControlState = nControlState;
	mnControlSize = nControlSize;
	mpBuffer = pBuffer;
	mpGraphics = pGraphics;
	mpControlValue = pControlValue;
	maDestRect = aDestRect;
	mbDrawn = false;
	maSize = NSZeroSize;

	return self;
}

- (NSSize)size
{
	return maSize;
}

@end

// =======================================================================

@interface VCLNativeBox : NSObject
{
	ControlState			mnControlState;
	VCLBitmapBuffer*		mpBuffer;
	JavaSalGraphics*		mpGraphics;
	CGRect					maDestRect;
	bool					mbDrawn;
}
+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect;
- (NSBox *)box;
- (void)draw:(id)pObject;
- (bool)drawn;
- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect;
@end

@implementation VCLNativeBox

+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect
{
	VCLNativeBox *pRet = [[VCLNativeBox alloc] initWithControlState:nControlState bitmapBuffer:pBuffer graphics:pGraphics destRect:aDestRect];
	[pRet autorelease];
	return pRet;
}

- (NSBox *)box
{
	NSBox *pBox = [[NSBox alloc] initWithFrame:NSMakeRect( 0, 0, maDestRect.size.width, maDestRect.size.height )];
	if ( !pBox )
		return nil;

	[pBox autorelease];

	[pBox setTitle:@""];
	[pBox setTitlePosition:NSNoTitle];

	return pBox;
}

- (void)draw:(id)pObject
{
	(void)pObject;

	if ( !mbDrawn && mpBuffer && mpGraphics && !CGRectIsEmpty( maDestRect ) )
	{
		NSBox *pBox = [self box];
		if ( pBox )
		{
			float fOffscreenHeight = maDestRect.size.height;
			CGRect aAdjustedDestRect = CGRectMake( 0, 0, maDestRect.size.width, fOffscreenHeight );
			if ( mpBuffer->Create( static_cast< long >( maDestRect.origin.x ), static_cast< long >( maDestRect.origin.y ), static_cast< long >( maDestRect.size.width ), static_cast< long >( fOffscreenHeight ), mpGraphics, fOffscreenHeight == maDestRect.size.height ) )
			{
				CGContextSaveGState( mpBuffer->maContext );
				if ( [pBox isFlipped] )
				{
					CGContextTranslateCTM( mpBuffer->maContext, 0, aAdjustedDestRect.size.height );
					CGContextScaleCTM( mpBuffer->maContext, 1.0f, -1.0f );
				}

				NSGraphicsContext *pContext = [NSGraphicsContext graphicsContextWithCGContext:mpBuffer->maContext flipped:YES];
				if ( pContext )
				{
					NSGraphicsContext *pOldContext = [NSGraphicsContext currentContext];
					[NSGraphicsContext setCurrentContext:pContext];
					CGContextClipToRect( mpBuffer->maContext, aAdjustedDestRect );
					// CGContextBeginTransparencyLayerWithRect( mpBuffer->maContext, aAdjustedDestRect, nullptr );
					[pBox drawRect:[pBox frame]];
					// CGContextEndTransparencyLayer( mpBuffer->maContext );
					[NSGraphicsContext setCurrentContext:pOldContext];

					mbDrawn = true;
				}

				CGContextRestoreGState( mpBuffer->maContext );

				mpBuffer->ReleaseContext();

				if ( mbDrawn )
					mpBuffer->DrawContextAndDestroy( mpGraphics, aAdjustedDestRect, maDestRect );
			}
		}
	}
}

- (bool)drawn
{
	return mbDrawn;
}

- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect
{
	[super init];

	mnControlState = nControlState;
	mpBuffer = pBuffer;
	mpGraphics = pGraphics;
	maDestRect = aDestRect;
	mbDrawn = false;

	return self;
}

@end

// =======================================================================

@interface VCLNativeBorderView : NSObject
{
	ControlState			mnControlState;
	VCLBitmapBuffer*		mpBuffer;
	JavaSalGraphics*		mpGraphics;
	CGRect					maDestRect;
	bool					mbDrawn;
}
+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect;
- (NSView *)borderView;
- (void)draw:(id)pObject;
- (bool)drawn;
- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect;
@end

@implementation VCLNativeBorderView

+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect
{
	VCLNativeBorderView *pRet = [[VCLNativeBorderView alloc] initWithControlState:nControlState bitmapBuffer:pBuffer graphics:pGraphics destRect:aDestRect];
	[pRet autorelease];
	return pRet;
}

- (NSView *)borderView
{
	NSBox *pBox = [[NSBox alloc] initWithFrame:NSMakeRect( 0, 0, maDestRect.size.width, maDestRect.size.height )];
	if ( !pBox )
		return nil;

	[pBox autorelease];

	[pBox setBoxType:NSBoxCustom];
	[pBox setBorderType:NSLineBorder];
	[pBox setBorderColor:[NSColor gridColor]];

	return pBox;
}

- (void)draw:(id)pObject
{
	(void)pObject;

	if ( !mbDrawn && mpBuffer && mpGraphics && !CGRectIsEmpty( maDestRect ) )
	{
		NSView *pView = [self borderView];
		if ( pView )
		{
			float fOffscreenHeight = maDestRect.size.height;
			CGRect aAdjustedDestRect = CGRectMake( 0, 0, maDestRect.size.width, fOffscreenHeight );
			if ( mpBuffer->Create( static_cast< long >( maDestRect.origin.x ), static_cast< long >( maDestRect.origin.y ), static_cast< long >( maDestRect.size.width ), static_cast< long >( fOffscreenHeight ), mpGraphics, fOffscreenHeight == maDestRect.size.height ) )
			{
				CGContextSaveGState( mpBuffer->maContext );
				if ( [pView isFlipped] )
				{
					CGContextTranslateCTM( mpBuffer->maContext, 0, aAdjustedDestRect.size.height );
					CGContextScaleCTM( mpBuffer->maContext, 1.0f, -1.0f );
				}

				NSGraphicsContext *pContext = [NSGraphicsContext graphicsContextWithCGContext:mpBuffer->maContext flipped:YES];
				if ( pContext )
				{
					NSGraphicsContext *pOldContext = [NSGraphicsContext currentContext];
					[NSGraphicsContext setCurrentContext:pContext];
					CGContextClipToRect( mpBuffer->maContext, aAdjustedDestRect );
					// CGContextBeginTransparencyLayerWithRect( mpBuffer->maContext, aAdjustedDestRect, nullptr );
					[pView drawRect:[pView frame]];
					// CGContextEndTransparencyLayer( mpBuffer->maContext );
					[NSGraphicsContext setCurrentContext:pOldContext];

					mbDrawn = true;
				}

				CGContextRestoreGState( mpBuffer->maContext );

				mpBuffer->ReleaseContext();

				if ( mbDrawn )
					mpBuffer->DrawContextAndDestroy( mpGraphics, aAdjustedDestRect, maDestRect );
			}
		}
	}
}

- (bool)drawn
{
	return mbDrawn;
}

- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect
{
	[super init];

	mnControlState = nControlState;
	mpBuffer = pBuffer;
	mpGraphics = pGraphics;
	maDestRect = aDestRect;
	mbDrawn = false;

	return self;
}

@end

// =======================================================================

@interface VCLNativeTableHeaderColumn : NSObject
{
	ControlState			mnControlState;
	VCLBitmapBuffer*		mpBuffer;
	JavaSalGraphics*		mpGraphics;
	const ListViewHeaderValue*	mpListViewHeaderValue;
	CGRect					maDestRect;
	bool					mbDrawn;
}
+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics listViewHeaderValue:(const ListViewHeaderValue *)pListViewHeaderValue destRect:(CGRect)aDestRect;
- (NSTableColumn *)tableColumn;
- (void)draw:(id)pObject;
- (bool)drawn;
- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics listViewHeaderValue:(const ListViewHeaderValue *)pListViewHeaderValue destRect:(CGRect)aDestRect;
@end

@implementation VCLNativeTableHeaderColumn

+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics listViewHeaderValue:(const ListViewHeaderValue *)pListViewHeaderValue destRect:(CGRect)aDestRect
{
	VCLNativeTableHeaderColumn *pRet = [[VCLNativeTableHeaderColumn alloc] initWithControlState:nControlState bitmapBuffer:pBuffer graphics:pGraphics listViewHeaderValue:pListViewHeaderValue destRect:aDestRect];
	[pRet autorelease];
	return pRet;
}

- (NSTableColumn *)tableColumn
{
	NSScrollView *pScrollView = [[NSScrollView alloc] initWithFrame:NSMakeRect( 0, 0, maDestRect.size.width, maDestRect.size.height )];
	if ( !pScrollView )
		return nil;

	[pScrollView autorelease];

	NSTableView *pTableView = [[NSTableView alloc] initWithFrame:[pScrollView frame]];
	if ( !pTableView )
		return nil;

	[pTableView autorelease];
	[pScrollView setDocumentView:pTableView];

	NSTableColumn *pTableColumn = [[NSTableColumn alloc] initWithIdentifier:@""];
	if ( !pTableColumn )
		return nil;

	[pTableColumn autorelease];
	[pTableColumn setWidth:[pTableView frame].size.width];
	[pTableView addTableColumn:pTableColumn];

	NSTableHeaderCell *pTableHeaderCell = [pTableColumn headerCell];
	if ( !pTableHeaderCell || ![pTableHeaderCell isKindOfClass:[NSTableHeaderCell class]] )
		return nil;

	[pTableHeaderCell setStringValue:@""];

	if ( mnControlState & ControlState::PRESSED )
		[pTableHeaderCell setState:NSControlStateValueOn];
	else
		[pTableHeaderCell setState:NSControlStateValueOff];

	// The enabled state is controlled by the [NSWindow _hasActiveControls]
	// selector so we need to attach a custom hidden window to draw enabled.
	// Fix hanging reported in the following NeoOffice forum topic by not
	// attaching a custom window on OS X 10.10:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8656

	return pTableColumn;
}

- (void)draw:(id)pObject
{
	(void)pObject;

	if ( !mbDrawn && mpBuffer && mpGraphics && !CGRectIsEmpty( maDestRect ) )
	{
		NSTableColumn *pTableColumn = [self tableColumn];
		if ( pTableColumn )
		{
			NSTableView *pTableView = [pTableColumn tableView];
			if ( pTableView )
			{
				NSTableHeaderView *pTableHeaderView = [pTableView headerView];
				NSTableHeaderCell *pTableHeaderCell = [pTableColumn headerCell];
				if ( pTableHeaderView && pTableHeaderCell && [pTableHeaderCell isKindOfClass:[NSTableHeaderCell class]] )
				{
					BOOL bHighlighted = ( ( mnControlState & ControlState::SELECTED ) || ( mpListViewHeaderValue && mpListViewHeaderValue->mbPrimarySortColumn ) );

					// Prevent clipping of left separator by extending width
					// to the left when drawing highlighted or pressed cells
					float fWidthAdjust = ( bHighlighted || mnControlState & ControlState::PRESSED ? 1.0f : 0 );
					CGRect aRealDrawRect = maDestRect;
					aRealDrawRect.origin.x -= fWidthAdjust;
					aRealDrawRect.size.width += fWidthAdjust;

					// Fix incorrect table header height on OS X 10.11 by
					// ignoring the cell height when calculating the drawing
					// bounds
					float fOffscreenHeight = aRealDrawRect.size.height;
					CGRect aAdjustedDestRect = CGRectMake( 0, 0, aRealDrawRect.size.width, fOffscreenHeight );
					if ( mpBuffer->Create( static_cast< long >( aRealDrawRect.origin.x ), static_cast< long >( aRealDrawRect.origin.y ), static_cast< long >( aRealDrawRect.size.width ), static_cast< long >( fOffscreenHeight ), mpGraphics, fOffscreenHeight == aRealDrawRect.size.height ) )
					{
						CGContextSaveGState( mpBuffer->maContext );
						CGContextTranslateCTM( mpBuffer->maContext, 0, 0 );
						if ( [pTableHeaderView isFlipped] )
						{
							CGContextTranslateCTM( mpBuffer->maContext, 0, aAdjustedDestRect.size.height );
							CGContextScaleCTM( mpBuffer->maContext, 1.0f, -1.0f );
						}

						NSGraphicsContext *pContext = [NSGraphicsContext graphicsContextWithCGContext:mpBuffer->maContext flipped:YES];
						if ( pContext )
						{
							// Shift control to right by same amount so that
							// the clipped bits will be drawn
							NSRect aDrawRect = NSRectFromCGRect( aAdjustedDestRect );
							aDrawRect.origin.x += fWidthAdjust;
							aDrawRect.size.width -= fWidthAdjust;

							NSGraphicsContext *pOldContext = [NSGraphicsContext currentContext];
							[NSGraphicsContext setCurrentContext:pContext];
							CGContextClipToRect( mpBuffer->maContext, aAdjustedDestRect );
							// CGContextBeginTransparencyLayerWithRect( mpBuffer->maContext, aAdjustedDestRect, nullptr );

							if ( bHighlighted )
								[pTableHeaderCell highlight:YES withFrame:aDrawRect inView:pTableHeaderView];
							else
								[pTableHeaderCell drawWithFrame:aDrawRect inView:pTableHeaderView];

							// Draw sort indicator
							if ( mpListViewHeaderValue )
							{
								BOOL bDrawSortIndicator = NO;
								BOOL bSortAscending = YES;
								if ( mpListViewHeaderValue->mnSortDirection == ListViewHeaderSortValue::SORT_ASCENDING )
								{
									bDrawSortIndicator = YES;
									bSortAscending = YES;
								}
								else if ( mpListViewHeaderValue->mnSortDirection == ListViewHeaderSortValue::SORT_DESCENDING )
								{
									bDrawSortIndicator = YES;
									bSortAscending = NO;
								}

								if ( bDrawSortIndicator )
									[pTableHeaderCell drawSortIndicatorWithFrame:aDrawRect inView:pTableHeaderView ascending:bSortAscending priority:0];
							}

							// CGContextEndTransparencyLayer( mpBuffer->maContext );
							[NSGraphicsContext setCurrentContext:pOldContext];

							mbDrawn = true;
						}

						CGContextRestoreGState( mpBuffer->maContext );

						mpBuffer->ReleaseContext();

						if ( mbDrawn )
							mpBuffer->DrawContextAndDestroy( mpGraphics, aAdjustedDestRect, aRealDrawRect );
					}
				}
			}
		}
	}
}

- (bool)drawn
{
	return mbDrawn;
}

- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics listViewHeaderValue:(const ListViewHeaderValue *)pListViewHeaderValue destRect:(CGRect)aDestRect
{
	[super init];

	mnControlState = nControlState;
	mpBuffer = pBuffer;
	mpGraphics = pGraphics;
	mpListViewHeaderValue = pListViewHeaderValue;
	maDestRect = aDestRect;
	mbDrawn = false;

	return self;
}

@end

// =======================================================================

@interface VCLNativeSpinbuttons : NSObject
{
	ControlState			mnControlState;
	VCLBitmapBuffer*		mpBuffer;
	JavaSalGraphics*		mpGraphics;
	const SpinbuttonValue*	mpSpinbuttonValue;
	CGRect					maDestRect;
	bool					mbDrawn;
	NSSize					maSize;
}
+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics spinbuttonValue:(const SpinbuttonValue *)pSpinbuttonValue destRect:(CGRect)aDestRect;
- (NSStepper *)stepper;
- (void)draw:(id)pObject;
- (bool)drawn;
- (void)getSize:(id)pObject;
- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics spinbuttonValue:(const SpinbuttonValue *)pSpinbuttonValue destRect:(CGRect)aDestRect;
- (NSSize)size;
@end

@implementation VCLNativeSpinbuttons

+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics spinbuttonValue:(const SpinbuttonValue *)pSpinbuttonValue destRect:(CGRect)aDestRect
{
	VCLNativeSpinbuttons *pRet = [[VCLNativeSpinbuttons alloc] initWithControlState:nControlState bitmapBuffer:pBuffer graphics:pGraphics spinbuttonValue:pSpinbuttonValue destRect:aDestRect];
	[pRet autorelease];
	return pRet;
}

- (NSStepper *)stepper
{
	NSStepper *pStepper = [[NSStepper alloc] initWithFrame:NSMakeRect( 0, 0, maDestRect.size.width, maDestRect.size.height )];
	if ( !pStepper )
		return nil;

	[pStepper autorelease];

	NSCell *pCell = [pStepper cell];
	if ( !pCell )
		return nil;

	[pStepper setAutorepeat:NO];

	if ( mpSpinbuttonValue )
	{
		if ( mpSpinbuttonValue->mnUpperState & ControlState::PRESSED )
		{
			[pStepper moveUp:self];
			[pCell setHighlighted:YES];
		}
		else if ( mpSpinbuttonValue->mnLowerState & ControlState::PRESSED )
		{
			[pStepper moveDown:self];
			[pCell setHighlighted:YES];
		}
	}

	if ( mnControlState & ( ControlState::PRESSED | ControlState::SELECTED | ControlState::ENABLED ) )
		[pStepper setEnabled:YES];
	else
		[pStepper setEnabled:NO];

	// Always suppress focus ring since it does not paint on some Mac OS X
	// versions
	[pCell setShowsFirstResponder:NO];

	// The enabled state is controlled by the [NSWindow _hasActiveControls]
	// selector so we need to attach a custom hidden window to draw enabled
	[VCLNativeControlWindow createAndAttachToView:pStepper controlState:mnControlState];

	[pStepper sizeToFit];

	return pStepper;
}

- (void)draw:(id)pObject
{
	(void)pObject;

	if ( !mbDrawn && mpBuffer && mpGraphics && !CGRectIsEmpty( maDestRect ) )
	{
		NSStepper *pStepper = [self stepper];
		if ( pStepper )
		{
			NSCell *pCell = [pStepper cell];
			if ( pCell )
			{
				float fCellWidth = [pCell cellSize].width;
				if ( fCellWidth <= 0 )
					fCellWidth = maDestRect.size.width;
				float fCellHeight = [pCell cellSize].height;
				if ( fCellHeight <= 0 )
					fCellHeight = maDestRect.size.height;
				float fOffscreenHeight = ( maDestRect.size.height > fCellHeight ? maDestRect.size.height : fCellHeight );
				CGRect aAdjustedDestRect = CGRectMake( 0, 0, maDestRect.size.width, fOffscreenHeight );
				if ( mpBuffer->Create( static_cast< long >( maDestRect.origin.x ), static_cast< long >( maDestRect.origin.y ), static_cast< long >( maDestRect.size.width ), static_cast< long >( fOffscreenHeight ), mpGraphics, fOffscreenHeight == maDestRect.size.height ) )
				{
					CGContextSaveGState( mpBuffer->maContext );
					if ( [pStepper isFlipped] )
					{
						CGContextTranslateCTM( mpBuffer->maContext, 0, aAdjustedDestRect.size.height );
						CGContextScaleCTM( mpBuffer->maContext, 1.0f, -1.0f );
					}

					CGContextClipToRect( mpBuffer->maContext, aAdjustedDestRect );

					// Horizontally right align and vertically center control
					float fXAdjust = aAdjustedDestRect.size.width - fCellWidth + SPINNER_WIDTH_SLOP;
					if ( fXAdjust < 0 )
						fXAdjust = 0;
					float fYAdjust = ( fOffscreenHeight - fCellHeight ) / 2;
					CGContextTranslateCTM( mpBuffer->maContext, fXAdjust, fYAdjust );

					NSGraphicsContext *pContext = [NSGraphicsContext graphicsContextWithCGContext:mpBuffer->maContext flipped:YES];
					if ( pContext )
					{
						NSGraphicsContext *pOldContext = [NSGraphicsContext currentContext];
						[NSGraphicsContext setCurrentContext:pContext];
						CGContextClipToRect( mpBuffer->maContext, aAdjustedDestRect );
						// CGContextBeginTransparencyLayerWithRect( mpBuffer->maContext, aAdjustedDestRect, nullptr );

						[pStepper drawRect:[pStepper frame]];

						// Draw focus ring
						if ( mnControlState & ControlState::FOCUSED && [pStepper isEnabled] )
						{
							NSRect aFocusRingRect = NSMakeRect( FOCUSRING_WIDTH + SPINNER_FOCUSRING_LEFT_OFFSET, FOCUSRING_WIDTH + SPINNER_FOCUSRING_TOP_OFFSET, [pCell cellSize].width - ( FOCUSRING_WIDTH * 2 ) - SPINNER_FOCUSRING_LEFT_OFFSET - SPINNER_FOCUSRING_RIGHT_OFFSET, [pCell cellSize].height - ( FOCUSRING_WIDTH * 2 ) - SPINNER_FOCUSRING_TOP_OFFSET - SPINNER_FOCUSRING_BOTTOM_OFFSET );
							NSSetFocusRingStyle( NSFocusRingBelow );
							[[NSColor clearColor] set];
							NSBezierPath *pPath = [NSBezierPath bezierPathWithRoundedRect:aFocusRingRect xRadius:SPINNER_FOCUSRING_ROUNDED_RECT_RADIUS yRadius:SPINNER_FOCUSRING_ROUNDED_RECT_RADIUS];
							if ( pPath )
								[pPath fill];
						}

						// CGContextEndTransparencyLayer( mpBuffer->maContext );
						[NSGraphicsContext setCurrentContext:pOldContext];

						mbDrawn = true;
					}

					CGContextRestoreGState( mpBuffer->maContext );

					mpBuffer->ReleaseContext();

					if ( mbDrawn )
						mpBuffer->DrawContextAndDestroy( mpGraphics, aAdjustedDestRect, maDestRect );
				}
			}
		}
	}
}

- (bool)drawn
{
	return mbDrawn;
}

- (void)getSize:(id)pObject
{
	(void)pObject;

	if ( NSEqualSizes( maSize, NSZeroSize ) )
	{
		NSStepper *pStepper = [self stepper];
		if ( pStepper )
			maSize = [pStepper frame].size;
	}
}

- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics spinbuttonValue:(const SpinbuttonValue *)pSpinbuttonValue destRect:(CGRect)aDestRect
{
	[super init];

	mnControlState = nControlState;
	mpBuffer = pBuffer;
	mpGraphics = pGraphics;
	mpSpinbuttonValue = pSpinbuttonValue;
	maDestRect = aDestRect;
	mbDrawn = false;
	maSize = NSZeroSize;

	return self;
}

- (NSSize)size
{
	return maSize;
}

@end

// =======================================================================

@interface VCLNativeTextField : NSObject
{
	ControlState			mnControlState;
	VCLBitmapBuffer*		mpBuffer;
	JavaSalGraphics*		mpGraphics;
	CGRect					maDestRect;
	bool					mbDrawn;
}
+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect;
- (NSTextField *)textField;
- (void)draw:(id)pObject;
- (bool)drawn;
- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect;
@end

@implementation VCLNativeTextField

+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect
{
	VCLNativeTextField *pRet = [[VCLNativeTextField alloc] initWithControlState:nControlState bitmapBuffer:pBuffer graphics:pGraphics destRect:aDestRect];
	[pRet autorelease];
	return pRet;
}

- (NSTextField *)textField
{
	NSTextField *pTextField = [[NSTextField alloc] initWithFrame:NSMakeRect( 0, 0, maDestRect.size.width, maDestRect.size.height )];
	if ( !pTextField )
		return nil;

	[pTextField autorelease];

	NSCell *pCell = [pTextField cell];
	if ( !pCell )
		return nil;

	if ( mnControlState & ( ControlState::PRESSED | ControlState::SELECTED | ControlState::ENABLED ) )
		[pTextField setEnabled:YES];
	else
		[pTextField setEnabled:NO];

	// Always suppress focus ring since it does not paint on some Mac OS X
	// versions
	[pCell setShowsFirstResponder:NO];

	return pTextField;
}

- (void)draw:(id)pObject
{
	(void)pObject;

	if ( !mbDrawn && mpBuffer && mpGraphics && !CGRectIsEmpty( maDestRect ) )
	{
		NSTextField *pTextField = [self textField];
		if ( pTextField )
		{
			NSCell *pCell = [pTextField cell];
			if ( pCell )
			{
				float fOffscreenHeight = maDestRect.size.height;
				CGRect aAdjustedDestRect = CGRectMake( 0, 0, maDestRect.size.width, fOffscreenHeight );
				if ( mpBuffer->Create( static_cast< long >( maDestRect.origin.x ), static_cast< long >( maDestRect.origin.y ), static_cast< long >( maDestRect.size.width ), static_cast< long >( fOffscreenHeight ), mpGraphics, fOffscreenHeight == maDestRect.size.height ) )
				{
					CGContextSaveGState( mpBuffer->maContext );
					if ( [pTextField isFlipped] )
					{
						CGContextTranslateCTM( mpBuffer->maContext, 0, aAdjustedDestRect.size.height );
						CGContextScaleCTM( mpBuffer->maContext, 1.0f, -1.0f );
					}

					NSGraphicsContext *pContext = [NSGraphicsContext graphicsContextWithCGContext:mpBuffer->maContext flipped:YES];
					if ( pContext )
					{
						NSRect aDrawRect = NSRectFromCGRect( aAdjustedDestRect );
						aDrawRect.origin.x += FOCUSRING_WIDTH;
						aDrawRect.origin.y += FOCUSRING_WIDTH;
						aDrawRect.size.width -= FOCUSRING_WIDTH * 2;
						aDrawRect.size.height -= FOCUSRING_WIDTH * 2;

						NSGraphicsContext *pOldContext = [NSGraphicsContext currentContext];
						[NSGraphicsContext setCurrentContext:pContext];
						CGContextClipToRect( mpBuffer->maContext, aAdjustedDestRect );
						// CGContextBeginTransparencyLayerWithRect( mpBuffer->maContext, aAdjustedDestRect, nullptr );

						// Fix height of text fields on macOS 10.14 by adding
						// height to the top of the text field
						NSRect aCellDrawRect = aDrawRect;
						aCellDrawRect.origin.y -= EDITBOX_HEIGHT_SLOP;
						aCellDrawRect.size.height += EDITBOX_HEIGHT_SLOP;
						[pCell drawWithFrame:aCellDrawRect inView:pTextField];

						// Draw focus ring
						if ( mnControlState & ControlState::FOCUSED && [pTextField isEnabled] )
						{
							NSSetFocusRingStyle( NSFocusRingAbove );
							[[NSColor clearColor] set];
							[NSBezierPath fillRect:aDrawRect];
						}

						// CGContextEndTransparencyLayer( mpBuffer->maContext );
						[NSGraphicsContext setCurrentContext:pOldContext];

						mbDrawn = true;
					}

					CGContextRestoreGState( mpBuffer->maContext );

					mpBuffer->ReleaseContext();

					if ( mbDrawn )
						mpBuffer->DrawContextAndDestroy( mpGraphics, aAdjustedDestRect, maDestRect );
				}
			}
		}
	}
}

- (bool)drawn
{
	return mbDrawn;
}

- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect
{
	[super init];

	mnControlState = nControlState;
	mpBuffer = pBuffer;
	mpGraphics = pGraphics;
	maDestRect = aDestRect;
	mbDrawn = false;

	return self;
}

@end

// =======================================================================

@interface VCLNativeMenuItemCell : NSObject
{
	ControlState			mnControlState;
	VCLBitmapBuffer*		mpBuffer;
	JavaSalGraphics*		mpGraphics;
	CGRect					maDestRect;
	bool					mbDrawn;
	BOOL					mbDrawSeparator;
}
+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect drawSeparator:(BOOL)bDrawSeparator;
- (NSControl *)menuItemCellControl;
- (void)draw:(id)pObject;
- (bool)drawn;
- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect drawSeparator:(BOOL)bDrawSeparator;
@end

@implementation VCLNativeMenuItemCell

+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect drawSeparator:(BOOL)bDrawSeparator
{
	VCLNativeMenuItemCell *pRet = [[VCLNativeMenuItemCell alloc] initWithControlState:nControlState bitmapBuffer:pBuffer graphics:pGraphics destRect:aDestRect drawSeparator:bDrawSeparator];
	[pRet autorelease];
	return pRet;
}

- (NSControl *)menuItemCellControl
{
	NSControl *pControl = [[NSControl alloc] initWithFrame:NSMakeRect( 0, 0, maDestRect.size.width, maDestRect.size.height )];
	if ( !pControl )
		return nil;

	[pControl autorelease];

	NSMenuItemCell *pCell = [[NSMenuItemCell alloc] initTextCell:@""];
	if ( !pCell )
		return nil;

	[pControl setCell:pCell];

	if ( mnControlState & ( ControlState::PRESSED | ControlState::SELECTED | ControlState::ENABLED ) )
		[pControl setEnabled:YES];
	else
		[pControl setEnabled:NO];

	// Always suppress focus ring in menu items
	[pCell setShowsFirstResponder:NO];

	return pControl;
}

- (void)draw:(id)pObject
{
	(void)pObject;

	if ( !mbDrawn && mpBuffer && mpGraphics && !CGRectIsEmpty( maDestRect ) )
	{
		NSControl *pControl = [self menuItemCellControl];
		if ( pControl )
		{
			NSMenuItemCell *pMenuItemCell = [pControl cell];
			if ( pMenuItemCell && [pMenuItemCell isKindOfClass:[NSMenuItemCell class]] )
			{
				float fOffscreenHeight = maDestRect.size.height;
				CGRect aAdjustedDestRect = CGRectMake( 0, 0, maDestRect.size.width, fOffscreenHeight );
				if ( mpBuffer->Create( static_cast< long >( maDestRect.origin.x ), static_cast< long >( maDestRect.origin.y ), static_cast< long >( maDestRect.size.width ), static_cast< long >( fOffscreenHeight ), mpGraphics, fOffscreenHeight == maDestRect.size.height ) )
				{
					CGContextSaveGState( mpBuffer->maContext );
					if ( [pControl isFlipped] )
					{
						CGContextTranslateCTM( mpBuffer->maContext, 0, aAdjustedDestRect.size.height );
						CGContextScaleCTM( mpBuffer->maContext, 1.0f, -1.0f );
					}

					NSGraphicsContext *pContext = [NSGraphicsContext graphicsContextWithCGContext:mpBuffer->maContext flipped:YES];
					if ( pContext )
					{
						NSRect aDrawRect = NSRectFromCGRect( aAdjustedDestRect );

						NSGraphicsContext *pOldContext = [NSGraphicsContext currentContext];
						[NSGraphicsContext setCurrentContext:pContext];
						CGContextClipToRect( mpBuffer->maContext, aAdjustedDestRect );
						// CGContextBeginTransparencyLayerWithRect( mpBuffer->maContext, aAdjustedDestRect, nullptr );

						if ( mbDrawSeparator )
						{
							[pMenuItemCell drawSeparatorItemWithFrame:aDrawRect inView:pControl];
						}
						else
						{
							if ( @available(macOS 10.14, * ) )
								[[NSColor unemphasizedSelectedContentBackgroundColor] set];
							else
								[[NSColor controlColor] set];
							[NSBezierPath fillRect:aDrawRect];
						}

						// CGContextEndTransparencyLayer( mpBuffer->maContext );
						[NSGraphicsContext setCurrentContext:pOldContext];

						mbDrawn = true;
					}

					CGContextRestoreGState( mpBuffer->maContext );

					mpBuffer->ReleaseContext();

					if ( mbDrawn )
						mpBuffer->DrawContextAndDestroy( mpGraphics, aAdjustedDestRect, maDestRect );
				}
			}
		}
	}
}

- (bool)drawn
{
	return mbDrawn;
}

- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect drawSeparator:(BOOL)bDrawSeparator
{
	[super init];

	mnControlState = nControlState;
	mpBuffer = pBuffer;
	mpGraphics = pGraphics;
	maDestRect = aDestRect;
	mbDrawn = false;
	mbDrawSeparator = bDrawSeparator;

	return self;
}

@end

// =======================================================================

@interface NSTabView (VCLNativeTabView)
- (void)_drawTabViewItem:(NSTabViewItem *)pItem inRect:(NSRect)aRect;
- (NSRect)_tabRectForTabViewItem:(NSTabViewItem *)pItem;
@end

@interface VCLNativeTabBorder : NSObject
{
	ControlState			mnControlState;
	VCLBitmapBuffer*		mpBuffer;
	JavaSalGraphics*		mpGraphics;
	CGRect					maDestRect;
	bool					mbDrawn;
}
+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect;
- (NSTabView *)tabView;
- (void)draw:(id)pObject;
- (bool)drawn;
- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect;
@end

@implementation VCLNativeTabBorder

+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect
{
	VCLNativeTabBorder *pRet = [[VCLNativeTabBorder alloc] initWithControlState:nControlState bitmapBuffer:pBuffer graphics:pGraphics destRect:aDestRect];
	[pRet autorelease];
	return pRet;
}

- (NSTabView *)tabView
{
	NSTabView *pTabView = [[NSTabView alloc] initWithFrame:NSMakeRect( 0, 0, maDestRect.size.width, maDestRect.size.height )];
	if ( !pTabView )
		return nil;

	[pTabView autorelease];
	[pTabView setTabViewType:NSTopTabsBezelBorder];

	// The enabled state is controlled by the [NSWindow _hasActiveControls]
	// selector so we need to attach a custom hidden window to draw enabled
	[VCLNativeControlWindow createAndAttachToView:pTabView controlState:mnControlState];

	return pTabView;
}

- (void)draw:(id)pObject
{
	(void)pObject;

	if ( !mbDrawn && mpBuffer && mpGraphics && !CGRectIsEmpty( maDestRect ) )
	{
		NSTabView *pTabView = [self tabView];
		if ( pTabView && [pTabView respondsToSelector:@selector(_drawTabViewItem:inRect:)] && [pTabView respondsToSelector:@selector(_tabRectForTabViewItem:)] )
		{
			float fOffscreenHeight = maDestRect.size.height;
			CGRect aAdjustedDestRect = CGRectMake( 0, 0, maDestRect.size.width, fOffscreenHeight );
			if ( mpBuffer->Create( static_cast< long >( maDestRect.origin.x ), static_cast< long >( maDestRect.origin.y ), static_cast< long >( maDestRect.size.width ), static_cast< long >( fOffscreenHeight ), mpGraphics, fOffscreenHeight == maDestRect.size.height ) )
			{
				CGContextSaveGState( mpBuffer->maContext );
				if ( [pTabView isFlipped] )
				{
					CGContextTranslateCTM( mpBuffer->maContext, 0, aAdjustedDestRect.size.height );
					CGContextScaleCTM( mpBuffer->maContext, 1.0f, -1.0f );
				}

				NSGraphicsContext *pContext = [NSGraphicsContext graphicsContextWithCGContext:mpBuffer->maContext flipped:YES];
				if ( pContext )
				{
					NSGraphicsContext *pOldContext = [NSGraphicsContext currentContext];
					[NSGraphicsContext setCurrentContext:pContext];
					CGContextClipToRect( mpBuffer->maContext, aAdjustedDestRect );
					// CGContextBeginTransparencyLayerWithRect( mpBuffer->maContext, aAdjustedDestRect, nullptr );
					[pTabView drawRect:[pTabView frame]];
					// CGContextEndTransparencyLayer( mpBuffer->maContext );
					[NSGraphicsContext setCurrentContext:pOldContext];

					mbDrawn = true;
				}

				CGContextRestoreGState( mpBuffer->maContext );

				mpBuffer->ReleaseContext();

				if ( mbDrawn )
					mpBuffer->DrawContextAndDestroy( mpGraphics, aAdjustedDestRect, maDestRect );
			}
		}
	}
}

- (bool)drawn
{
	return mbDrawn;
}

- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics destRect:(CGRect)aDestRect
{
	[super init];

	mnControlState = nControlState;
	mpBuffer = pBuffer;
	mpGraphics = pGraphics;
	maDestRect = aDestRect;
	mbDrawn = false;

	return self;
}

@end

// =======================================================================

@interface VCLNativeTabView : NSTabView
- (NSRect)_tabRectForTabViewItem:(NSTabViewItem *)pItem;
@end

@implementation VCLNativeTabView

- (NSRect)_tabRectForTabViewItem:(NSTabViewItem *)pItem
{
	// Force the item to fill the entire tab view
	NSRect aRet = [self frame];

	// Set height to item's height
	if ( [super respondsToSelector:@selector(_tabRectForTabViewItem:)] )
	{
		NSRect aTabRect = [super _tabRectForTabViewItem:pItem];
		aRet.size.height = aTabRect.size.height;
	}

	return aRet;
}

@end

// =======================================================================

@interface VCLNativeTabViewItem : NSTabViewItem
{
    NSTabState				mnTabState;
}
- (id)initWithIdentifier:(id)pIdentifier;
- (void)setTabState:(NSTabState)nState;
- (NSTabState)tabState;
@end

@implementation VCLNativeTabViewItem

- (id)initWithIdentifier:(id)pIdentifier
{
	[super initWithIdentifier:pIdentifier];

	mnTabState = NSBackgroundTab;

	return self;
}

- (void)setTabState:(NSTabState)nTabState
{
	mnTabState = nTabState;
}

- (NSTabState)tabState
{
	return mnTabState;
}

@end

// =======================================================================

@interface VCLNativeTabCell : NSObject
{
	ControlState			mnControlState;
	VCLBitmapBuffer*		mpBuffer;
	JavaSalGraphics*		mpGraphics;
	const TabitemValue*		mpTabitemValue;
	CGRect					maDestRect;
	bool					mbDrawn;
	NSSize					maSize;
}
+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics tabitemValue:(const TabitemValue *)pTabitemValue destRect:(CGRect)aDestRect;
- (VCLNativeTabViewItem *)tabViewItem;
- (void)draw:(id)pObject;
- (bool)drawn;
- (void)getSize:(id)pObject;
- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics tabitemValue:(const TabitemValue *)pTabitemValue destRect:(CGRect)aDestRect;
- (NSSize)size;
@end

@implementation VCLNativeTabCell

+ (id)createWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics tabitemValue:(const TabitemValue *)pTabitemValue destRect:(CGRect)aDestRect
{
	VCLNativeTabCell *pRet = [[VCLNativeTabCell alloc] initWithControlState:nControlState bitmapBuffer:pBuffer graphics:pGraphics tabitemValue:pTabitemValue destRect:aDestRect];
	[pRet autorelease];
	return pRet;
}

- (VCLNativeTabViewItem *)tabViewItem
{
	VCLNativeTabView *pTabView = [[VCLNativeTabView alloc] initWithFrame:NSMakeRect( 0, 0, maDestRect.size.width, maDestRect.size.height )];
	if ( !pTabView )
		return nil;

	[pTabView autorelease];
	[pTabView setTabViewType:NSTopTabsBezelBorder];

	if ( mpTabitemValue && !mpTabitemValue->isFirst() )
	{
		VCLNativeTabViewItem *pPreItem = [[VCLNativeTabViewItem alloc] initWithIdentifier:@""];
		if ( !pPreItem )
			return nil;

		[pPreItem autorelease];
		[pTabView addTabViewItem:pPreItem];

		// Fix tab divider line color on OS X 10.10 by adding a second tab item
		// to the left of the tab item to be drawn
		if ( ! ( mnControlState & ( ControlState::PRESSED | ControlState::SELECTED ) ) )
		{
			pPreItem = [[VCLNativeTabViewItem alloc] initWithIdentifier:@""];
			if ( !pPreItem )
				return nil;

			[pPreItem autorelease];
			[pTabView addTabViewItem:pPreItem];
		}
	}

	VCLNativeTabViewItem *pItem = [[VCLNativeTabViewItem alloc] initWithIdentifier:@""];
	if ( !pItem )
		return nil;

	[pItem autorelease];
	[pTabView addTabViewItem:pItem];

	if ( mpTabitemValue && !mpTabitemValue->isLast() )
	{
		VCLNativeTabViewItem *pPostItem = [[VCLNativeTabViewItem alloc] initWithIdentifier:@""];
		if ( !pPostItem )
			return nil;

		[pPostItem autorelease];
		[pTabView addTabViewItem:pPostItem];
	}

	if ( mnControlState & ControlState::PRESSED )
		[pItem setTabState:NSPressedTab];
	else if ( mnControlState & ControlState::SELECTED )
		[pItem setTabState:NSSelectedTab];
	else
		[pItem setTabState:NSBackgroundTab];

	// The enabled state is controlled by the [NSWindow _hasActiveControls]
	// selector so we need to attach a custom hidden window to draw enabled
	[VCLNativeControlWindow createAndAttachToView:pTabView controlState:mnControlState];

	return pItem;
}

- (void)draw:(id)pObject
{
	(void)pObject;

	if ( !mbDrawn && mpBuffer && mpGraphics && !CGRectIsEmpty( maDestRect ) )
	{
		VCLNativeTabViewItem *pItem = [self tabViewItem];
		if ( pItem )
		{
			NSTabView *pTabView = [pItem tabView];
			if ( pTabView && [pTabView respondsToSelector:@selector(_drawTabViewItem:inRect:)] && [pTabView respondsToSelector:@selector(_tabRectForTabViewItem:)] )
			{
				// Prevent clipping of left separator by extending width to
				// the left
				float fWidthAdjust = 1.0f;
				CGRect aRealDrawRect = maDestRect;
				aRealDrawRect.origin.x -= fWidthAdjust + FOCUSRING_WIDTH;
				aRealDrawRect.size.width += fWidthAdjust + ( FOCUSRING_WIDTH * 2 );

				float fCellHeight = [pTabView _tabRectForTabViewItem:pItem].size.height;
				float fOffscreenHeight = ( aRealDrawRect.size.height > fCellHeight ? aRealDrawRect.size.height : fCellHeight );
				CGRect aAdjustedDestRect = CGRectMake( 0, 0, aRealDrawRect.size.width, fOffscreenHeight );
				if ( mpBuffer->Create( static_cast< long >( aRealDrawRect.origin.x ), static_cast< long >( aRealDrawRect.origin.y ), static_cast< long >( aRealDrawRect.size.width ), static_cast< long >( fOffscreenHeight ), mpGraphics, fOffscreenHeight == aRealDrawRect.size.height ) )
				{
					CGContextSaveGState( mpBuffer->maContext );
					if ( [pTabView isFlipped] )
					{
						CGContextTranslateCTM( mpBuffer->maContext, 0, aAdjustedDestRect.size.height );
						CGContextScaleCTM( mpBuffer->maContext, 1.0f, -1.0f );
					}

					NSGraphicsContext *pContext = [NSGraphicsContext graphicsContextWithCGContext:mpBuffer->maContext flipped:YES];
					if ( pContext )
					{
						// Shift control to right by same amount so that the
						// clipped bits will be drawn
						NSRect aFrame = [pTabView frame];
						aFrame.origin.x += fWidthAdjust + FOCUSRING_WIDTH;
						aFrame.origin.y += FOCUSRING_WIDTH;
						[pTabView setFrame:aFrame];

						NSGraphicsContext *pOldContext = [NSGraphicsContext currentContext];
						[NSGraphicsContext setCurrentContext:pContext];
						CGContextClipToRect( mpBuffer->maContext, aAdjustedDestRect );
						// CGContextBeginTransparencyLayerWithRect( mpBuffer->maContext, aAdjustedDestRect, nullptr );

						[pTabView _drawTabViewItem:pItem inRect:[pTabView frame]];

						// Draw focus ring
						if ( mnControlState & ControlState::FOCUSED && mnControlState & ( ControlState::PRESSED | ControlState::SELECTED | ControlState::ENABLED ) )
						{
							NSRect aFocusRingRect = [pTabView _tabRectForTabViewItem:pItem];
							if ( mpTabitemValue && mpTabitemValue->isFirst() )
								aFocusRingRect.origin.x += TABITEM_FOCUSRING_LEFT_OFFSET;
							aFocusRingRect.origin.y += TABITEM_FOCUSRING_TOP_OFFSET;
							if ( mpTabitemValue && mpTabitemValue->isFirst() )
								aFocusRingRect.size.width -= TABITEM_FOCUSRING_LEFT_OFFSET;
							if ( mpTabitemValue && mpTabitemValue->isLast() )
								aFocusRingRect.size.width -= TABITEM_FOCUSRING_RIGHT_OFFSET;
							aFocusRingRect.size.height -= TABITEM_FOCUSRING_TOP_OFFSET + TABITEM_FOCUSRING_BOTTOM_OFFSET;

							NSSetFocusRingStyle( NSFocusRingBelow );
							[[NSColor clearColor] set];
							if ( mpTabitemValue && ( mpTabitemValue->isFirst() || mpTabitemValue->isLast() ) )
							{
								NSBezierPath *pPath = [NSBezierPath bezierPathWithRoundedRect:aFocusRingRect xRadius:TABITEM_FOCUSRING_ROUNDED_RECT_RADIUS yRadius:TABITEM_FOCUSRING_ROUNDED_RECT_RADIUS];
								if ( pPath )
								{
									if ( !mpTabitemValue->isFirst() )
										[pPath appendBezierPathWithRect:NSMakeRect( aFocusRingRect.origin.x, aFocusRingRect.origin.y, aFocusRingRect.size.width / 2, aFocusRingRect.size.height )];
									if ( !mpTabitemValue->isLast() )
										[pPath appendBezierPathWithRect:NSMakeRect( aFocusRingRect.origin.x + ( aFocusRingRect.size.width / 2 ), aFocusRingRect.origin.y, aFocusRingRect.size.width / 2, aFocusRingRect.size.height )];

									[pPath fill];
								}
							}
							else
							{
								[NSBezierPath fillRect:aFocusRingRect];
							}
						}

						// CGContextEndTransparencyLayer( mpBuffer->maContext );
						[NSGraphicsContext setCurrentContext:pOldContext];

						mbDrawn = true;
					}

					CGContextRestoreGState( mpBuffer->maContext );

					mpBuffer->ReleaseContext();

					if ( mbDrawn )
						mpBuffer->DrawContextAndDestroy( mpGraphics, aAdjustedDestRect, aRealDrawRect );
				}
			}
		}
	}
}

- (bool)drawn
{
	return mbDrawn;
}

- (void)getSize:(id)pObject
{
	(void)pObject;

	if ( NSEqualSizes( maSize, NSZeroSize ) )
	{
		VCLNativeTabViewItem *pItem = [self tabViewItem];
		if ( pItem )
		{
			NSTabView *pTabView = [pItem tabView];
			if ( pTabView && [pTabView respondsToSelector:@selector(_tabRectForTabViewItem:)] )
				maSize = [pTabView _tabRectForTabViewItem:pItem].size;
		}
	}
}

- (id)initWithControlState:(ControlState)nControlState bitmapBuffer:(VCLBitmapBuffer *)pBuffer graphics:(JavaSalGraphics *)pGraphics tabitemValue:(const TabitemValue *)pTabitemValue destRect:(CGRect)aDestRect
{
	[super init];

	mnControlState = nControlState;
	mpBuffer = pBuffer;
	mpGraphics = pGraphics;
	mpTabitemValue = pTabitemValue;
	maDestRect = aDestRect;
	mbDrawn = false;
	maSize = NSZeroSize;

	return self;
}

- (NSSize)size
{
	return maSize;
}

@end

// =======================================================================

VCLBitmapBuffer::VCLBitmapBuffer() :
	BitmapBuffer(),
	maContext( nullptr ),
	mpGraphicsMutexGuard( nullptr ),
	mbLastDrawToPrintGraphics( false ),
	mbUseLayer( false )
{
	mnFormat = ScanlineFormat::NONE;
	mnWidth = 0;
	mnHeight = 0;
	mnScanlineSize = 0;
	mnBitCount = 0;
	mpBits = nullptr;
}

// -----------------------------------------------------------------------

VCLBitmapBuffer::~VCLBitmapBuffer()
{
	Destroy();
}

// -----------------------------------------------------------------------

bool VCLBitmapBuffer::Create( long nX, long nY, long nWidth, long nHeight, JavaSalGraphics *pGraphics, bool bUseLayer )
{
	if ( nWidth <= 0 || nHeight <= 0 || !pGraphics )
		return false;

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
				mpGraphicsMutexGuard = nullptr;
			}
		}
	}

	if ( !mpBits && !mbUseLayer )
	{
		try
		{
			mpBits = new sal_uInt8[ mnScanlineSize * mnHeight ];
		}
		catch( const std::bad_alloc& ) {}
	}

	if ( !mpBits && !mbUseLayer )
	{
		Destroy();
		return false;
	}

	if ( !maContext && !mbUseLayer )
	{
		CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
		if ( !aColorSpace )
		{
			Destroy();
			return false;
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
		return false;
	}

	if ( mpBits )
		memset( mpBits, 0, mnScanlineSize * mnHeight );

	mbLastDrawToPrintGraphics = bDrawToPrintGraphics;

	return true;
}

// -----------------------------------------------------------------------

void VCLBitmapBuffer::Destroy()
{
	mnFormat = ScanlineFormat::NONE;
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
		maContext = nullptr;
	}

	if ( mpBits )
	{
		delete[] mpBits;
		mpBits = nullptr;
	}

	if ( mpGraphicsMutexGuard )
	{
		delete mpGraphicsMutexGuard;
		mpGraphicsMutexGuard = nullptr;
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
			CGDataProviderRef aProvider = CGDataProviderCreateWithData( nullptr, mpBits, mnScanlineSize * mnHeight, ReleaseBitmapBufferBytePointerCallback );
			if ( aProvider )
			{
				mpBits = nullptr;
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
		maContext = nullptr;
	}

	if ( mpGraphicsMutexGuard )
	{
		delete mpGraphicsMutexGuard;
		mpGraphicsMutexGuard = nullptr;
	}
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
 * @return true if successful, false on error
 */
static bool DrawNativeComboBox( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState, const OUString& /* rCaption */ )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeComboBox *pVCLNativeComboBox = [VCLNativeComboBox createWithControlState:nState editable:YES bitmapBuffer:&aSharedComboBoxBuffer graphics:pGraphics destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeComboBox performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeComboBox waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeComboBox drawn];

	[pPool release];

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
 * @return true if successful, false on error
 */
static bool DrawNativeListBox( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState, const OUString& /* rCaption */ )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeComboBox *pVCLNativeComboBox = [VCLNativeComboBox createWithControlState:nState editable:NO bitmapBuffer:&aSharedListBoxBuffer graphics:pGraphics destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeComboBox performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeComboBox waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeComboBox drawn];

	[pPool release];

	return bRet;
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
 * @return true if successful, false on error
 */
static bool DrawNativeScrollBar( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState, const ScrollbarValue *pScrollbarValue )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	VCLBitmapBuffer *pBuffer;
	if ( rDestBounds.GetWidth() > rDestBounds.GetHeight() )
		pBuffer = &aSharedHorizontalScrollBarBuffer;
	else
		pBuffer = &aSharedVerticalScrollBarBuffer;

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeScrollbar *pVCLNativeScrollbar = [VCLNativeScrollbar createWithControlState:nState bitmapBuffer:pBuffer graphics:pGraphics scrollbarValue:pScrollbarValue doubleScrollbarArrows:GetSalData()->mbDoubleScrollbarArrows destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeScrollbar performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeScrollbar waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeScrollbar drawn];

	[pPool release];

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
 * @return true if drawing successful, false if not
 */
static bool DrawNativeSpinbox( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState, const SpinbuttonValue *pValue )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	// Always disable focus in the spinbutton and let the edit box have focus
	VCLNativeSpinbuttons *pVCLNativeSpinbuttons = [VCLNativeSpinbuttons createWithControlState:nState & ~ControlState::FOCUSED bitmapBuffer:&aSharedSpinbuttonBuffer graphics:pGraphics spinbuttonValue:pValue destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeSpinbuttons performSelectorOnMainThread:@selector(getSize:) withObject:pVCLNativeSpinbuttons waitUntilDone:YES modes:pModes];
	NSSize aSize = [pVCLNativeSpinbuttons size];
	if ( !NSEqualSizes( aSize, NSZeroSize ) )
	{
		[pVCLNativeSpinbuttons performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeSpinbuttons waitUntilDone:YES modes:pModes];
		if ( [pVCLNativeSpinbuttons drawn] )
		{
			VCLNativeTextField *pVCLNativeTextField = [VCLNativeTextField createWithControlState:nState bitmapBuffer:&aSharedEditBoxBuffer graphics:pGraphics destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth() - aSize.width, rDestBounds.GetHeight() )];
			[pVCLNativeTextField performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeTextField waitUntilDone:YES modes:pModes];
			bRet = [pVCLNativeTextField drawn];
		}
	}

	[pPool release];

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
 * @return true if drawing successful, false if not
 */
static bool DrawNativeSpinbutton( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState, const SpinbuttonValue *pValue )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeSpinbuttons *pVCLNativeSpinbuttons = [VCLNativeSpinbuttons createWithControlState:nState bitmapBuffer:&aSharedSpinbuttonBuffer graphics:pGraphics spinbuttonValue:pValue destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeSpinbuttons performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeSpinbuttons waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeSpinbuttons drawn];

	[pPool release];

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
 * @param bSmall			true to use small progress bar otherwise use large
 * @return true if drawing successful, false if not
 */
static bool DrawNativeProgressbar( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState, const ImplControlValue *pValue, bool bSmall )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeProgressbar *pVCLNativeProgressbar = [VCLNativeProgressbar createWithControlState:nState controlSize:( bSmall ? NSControlSizeSmall : NSControlSizeRegular ) bitmapBuffer:&aSharedProgressbarBuffer graphics:pGraphics progressbarValue:pValue destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeProgressbar performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeProgressbar waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeProgressbar drawn];

	[pPool release];

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
 * @return true if drawing successful, false if not
 */
static bool DrawNativeTab( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState, const TabitemValue *pValue )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeTabCell *pVCLNativeTabCell = [VCLNativeTabCell createWithControlState:nState bitmapBuffer:&aSharedTabBuffer graphics:pGraphics tabitemValue:pValue destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeTabCell performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeTabCell waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeTabCell drawn];

	[pPool release];

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
static bool DrawNativeTabBoundingBox( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeTabBorder *pVCLNativeTabBorder = [VCLNativeTabBorder createWithControlState:nState bitmapBuffer:&aSharedTabBoundingBoxBuffer graphics:pGraphics destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeTabBorder performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeTabBorder waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeTabBorder drawn];

	[pPool release];

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
static bool DrawNativePrimaryGroupBox( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeBox *pVCLNativeBox = [VCLNativeBox createWithControlState:nState bitmapBuffer:&aSharedPrimaryGroupBoxBuffer graphics:pGraphics destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeBox performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeBox waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeBox drawn];

	[pPool release];

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
static bool DrawNativeMenuBackground( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	ControlState nState = ControlState::ENABLED;
	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeMenuItemCell *pVCLNativeMenuItemCell = [VCLNativeMenuItemCell createWithControlState:nState bitmapBuffer:&aSharedMenuBackgroundBuffer graphics:pGraphics destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) drawSeparator:NO];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeMenuItemCell performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeMenuItemCell waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeMenuItemCell drawn];

	[pPool release];

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
static bool DrawNativeEditBox( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeTextField *pVCLNativeTextField = [VCLNativeTextField createWithControlState:nState bitmapBuffer:&aSharedEditBoxBuffer graphics:pGraphics destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeTextField performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeTextField waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeTextField drawn];

	[pPool release];

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
static bool DrawNativeListBoxFrame( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeBorderView *pVCLNativeBorderView = [VCLNativeBorderView createWithControlState:nState bitmapBuffer:&aSharedListViewFrameBuffer graphics:pGraphics destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeBorderView performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeBorderView waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeBorderView drawn];

	[pPool release];

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
 * @return OK if successful, false on failure
 */
static bool DrawNativeDisclosureButton( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState, const ImplControlValue *pValue )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSInteger nButtonState = NSControlStateValueOff;
	if ( pValue && pValue->getTristateVal() == ButtonValue::On )
		nButtonState = NSControlStateValueOn;

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeButton *pVCLNativeButton = [VCLNativeButton createWithButtonType:NSButtonTypeOnOff bezelStyle:NSBezelStyleDisclosure controlSize:NSControlSizeRegular buttonState:nButtonState controlState:nState drawRTL:NO bitmapBuffer:&aSharedDisclosureBtnBuffer graphics:pGraphics destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeButton performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeButton waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeButton drawn];

	[pPool release];

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
static bool DrawNativeSeparatorLine( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeMenuItemCell *pVCLNativeMenuItemCell = [VCLNativeMenuItemCell createWithControlState:nState bitmapBuffer:&aSharedSeparatorLineBuffer graphics:pGraphics destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() ) drawSeparator:YES];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeMenuItemCell performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeMenuItemCell waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeMenuItemCell drawn];

	[pPool release];

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
static bool DrawNativeListViewHeader( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState, const ListViewHeaderValue *pValue )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	// If the mouse is pressed and in the list view header, set the pressed flag
	bool bRedraw = false;
	if ( pGraphics->mpFrame && GetSalData()->maLastPointerState.mnState & MOUSE_LEFT )
	{
		Point aScreenPoint( GetSalData()->maLastPointerState.maPos );
		if ( rDestBounds.IsInside( Point( aScreenPoint.X() - pGraphics->mpFrame->maGeometry.nX, aScreenPoint.Y() - pGraphics->mpFrame->maGeometry.nY ) ) )
			nState |= ControlState::PRESSED;
		bRedraw = true;
	}

	VCLNativeTableHeaderColumn *pVCLNativeTableHeaderColumn = [VCLNativeTableHeaderColumn createWithControlState:nState bitmapBuffer:&aSharedListViewHeaderBuffer graphics:pGraphics listViewHeaderValue:pValue destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeTableHeaderColumn performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeTableHeaderColumn waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeTableHeaderColumn drawn];

	if ( bRet && bRedraw && pGraphics->mpFrame )
	{
		// Invalidate bounds to force redraw
		Window *pWindow = Application::GetFirstTopLevelWindow();
		while ( pWindow && pWindow->ImplGetFrame() != pGraphics->mpFrame )
			pWindow = Application::GetNextTopLevelWindow( pWindow );

		// Include a few pixels of the adjacent header columns to ensure that
		// they get redrawn as well
		if ( pWindow && pWindow->IsReallyVisible() )
			pWindow->Invalidate( tools::Rectangle( Point( rDestBounds.Left() - 2, rDestBounds.Top() ), Size( rDestBounds.GetWidth() + 4, rDestBounds.GetHeight() ) ) );
	}

	[pPool release];

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
static bool DrawNativeBevelButton( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState, const ImplControlValue& aValue )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSInteger nButtonState;
	if ( aValue.getTristateVal() == ButtonValue::On )
		nButtonState = NSControlStateValueOn;
	else
		nButtonState = NSControlStateValueOff;

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeButton *pVCLNativeButton = [VCLNativeButton createWithButtonType:NSButtonTypeOnOff bezelStyle:NSBezelStyleShadowlessSquare controlSize:NSControlSizeRegular buttonState:nButtonState controlState:nState drawRTL:NO bitmapBuffer:&aSharedBevelButtonBuffer graphics:pGraphics destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeButton performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeButton waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeButton drawn];

	[pPool release];

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
static bool DrawNativeCheckbox( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState, const ImplControlValue& aValue )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSControlSize nControlSize = ( rDestBounds.GetWidth() < CHECKBOX_WIDTH - ( FOCUSRING_WIDTH * 2 ) || rDestBounds.GetHeight() < CHECKBOX_HEIGHT - ( FOCUSRING_WIDTH * 2 ) ? NSControlSizeSmall : NSControlSizeRegular );
	NSInteger nButtonState;
	if ( aValue.getTristateVal() == ButtonValue::On )
		nButtonState = NSControlStateValueOn;
	else if ( aValue.getTristateVal() == ButtonValue::Mixed )
		nButtonState = NSControlStateValueMixed;
	else
		nButtonState = NSControlStateValueOff;

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeButton *pVCLNativeButton = [VCLNativeButton createWithButtonType:NSButtonTypeSwitch bezelStyle:NSBezelStyleRounded controlSize:nControlSize buttonState:nButtonState controlState:nState drawRTL:NO bitmapBuffer:&aSharedCheckboxBuffer graphics:pGraphics destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeButton performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeButton waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeButton drawn];

	[pPool release];

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
static bool DrawNativeRadioButton( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState, const ImplControlValue& aValue )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSControlSize nControlSize = ( rDestBounds.GetWidth() < RADIOBUTTON_WIDTH - ( FOCUSRING_WIDTH * 2 ) || rDestBounds.GetHeight() < RADIOBUTTON_HEIGHT - ( FOCUSRING_WIDTH * 2 ) ? NSControlSizeSmall : NSControlSizeRegular );
	NSInteger nButtonState;
	if ( aValue.getTristateVal() == ButtonValue::On )
		nButtonState = NSControlStateValueOn;
	else if ( aValue.getTristateVal() == ButtonValue::Mixed )
		nButtonState = NSControlStateValueMixed;
	else
		nButtonState = NSControlStateValueOff;

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeButton *pVCLNativeButton = [VCLNativeButton createWithButtonType:NSButtonTypeRadio bezelStyle:NSBezelStyleRounded controlSize:nControlSize buttonState:nButtonState controlState:nState drawRTL:NO bitmapBuffer:&aSharedCheckboxBuffer graphics:pGraphics destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeButton performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeButton waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeButton drawn];

	[pPool release];


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
static bool DrawNativePushButton( JavaSalGraphics *pGraphics, const tools::Rectangle& rDestBounds, ControlState nState, const ImplControlValue& aValue )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSInteger nButtonState;
	if ( aValue.getTristateVal() == ButtonValue::On )
		nButtonState = NSControlStateValueOn;
	else if ( aValue.getTristateVal() == ButtonValue::Mixed )
		nButtonState = NSControlStateValueMixed;
	else
		nButtonState = NSControlStateValueOff;

	if ( pGraphics->mpFrame && !pGraphics->mpFrame->IsFloatingFrame() && pGraphics->mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	VCLNativeButton *pVCLNativeButton = [VCLNativeButton createWithButtonType:NSButtonTypeMomentaryLight bezelStyle:NSBezelStyleRounded controlSize:NSControlSizeRegular buttonState:nButtonState controlState:nState drawRTL:NO bitmapBuffer:&aSharedCheckboxBuffer graphics:pGraphics destRect:CGRectMake( rDestBounds.Left(), rDestBounds.Top(), rDestBounds.GetWidth(), rDestBounds.GetHeight() )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLNativeButton performSelectorOnMainThread:@selector(draw:) withObject:pVCLNativeButton waitUntilDone:YES modes:pModes];
	bRet = [pVCLNativeButton drawn];

	if ( bRet && pGraphics->mpFrame && [pVCLNativeButton redraw] )
	{
		// Invalidate bounds to force redraw
		Window *pWindow = Application::GetFirstTopLevelWindow();
		while ( pWindow && pWindow->ImplGetFrame() != pGraphics->mpFrame )
			pWindow = Application::GetNextTopLevelWindow( pWindow );
		if ( pWindow && pWindow->IsReallyVisible() )
			pWindow->Invalidate( rDestBounds );
	}

	[pPool release];

	return bRet;
}

// =======================================================================

/**
 * Determine if support exists for drawing a particular native widget in the
 * interface.
 *
 * @param nType         control flavor that is requsted to be drawn
 * @param nPart         subpart of the control that is requested to be drawn
 * @return true if supported, false if not
 */
bool JavaSalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
	bool isSupported = false;

#ifndef USE_NATIVE_CONTROLS
	return isSupported;
#endif	// !USE_NATIVE_CONTROLS

	switch( nType )
	{
		case ControlType::Pushbutton:
			if( nPart == ControlPart::Entire )
				isSupported = true;
			break;

		case ControlType::Radiobutton:
			if( nPart == ControlPart::Entire )
				isSupported = true;
			break;

		case ControlType::Checkbox:
			if( nPart == ControlPart::Entire )
				isSupported = true;
			break;

		case ControlType::Combobox:
			if( ( nPart == ControlPart::Entire ) || ( nPart == ControlPart::HasBackgroundTexture ) )
				isSupported = true;
			break;

		case ControlType::Listbox:
			if( ( nPart == ControlPart::Entire ) || ( nPart == ControlPart::HasBackgroundTexture ) )
				isSupported = true;
			break;

		case ControlType::Scrollbar:
			if( ( nPart == ControlPart::Entire ) || nPart == ControlPart::DrawBackgroundHorz || nPart == ControlPart::DrawBackgroundVert )
				isSupported = true;
			break;

		case ControlType::Spinbox:
			if( nPart == ControlPart::Entire || nPart == ControlPart::AllButtons || nPart == ControlPart::HasBackgroundTexture )
				isSupported = true;
			break;

		case ControlType::SpinButtons:
			if( nPart == ControlPart::Entire || nPart == ControlPart::AllButtons )
				isSupported = true;
			break;

		case ControlType::Progress:
		case ControlType::IntroProgress:
			if( nPart == ControlPart::Entire )
				isSupported = true;
			break;

		case ControlType::TabItem:
			if( nPart == ControlPart::Entire )
				isSupported = true;
			break;

		case ControlType::TabPane:
			if( nPart == ControlPart::Entire )
				isSupported = true;
			break;

		case ControlType::Groupbox:
#ifdef USE_NATIVE_CTRL_GROUPBOX
			if( nPart == ControlPart::Entire )
				isSupported = true;
#endif	// USE_NATIVE_CTRL_GROUPBOX
			break;

		case ControlType::MenuPopup:
			if( nPart == ControlPart::Entire )
				isSupported = true;
			break;

		case ControlType::Editbox:
			if( ( nPart == ControlPart::Entire ) || ( nPart == ControlPart::HasBackgroundTexture ) )
				isSupported = true;
			break;

		case ControlType::ListNode:
			if( nPart == ControlPart::Entire )
				isSupported = true;
			break;

		case ControlType::ListViewHeader:
			if( ( nPart == ControlPart::Entire ) || ( nPart == ControlPart::ListViewHeaderSortMark ) )
				isSupported = true;
			break;

		case ControlType::Fixedline:
			if( nPart == ControlPart::Entire )
				isSupported = true;
			break;

		case ControlType::ListViewBox:
			if( nPart == ControlPart::Entire )
				isSupported = true;
			break;

		case ControlType::Toolbar:
			// Suppress the non-native toolbar background by returning true
			// ControlPart::Entire even though we don't actually support
			// it in the size and drawing methods
			if( nPart == ControlPart::Entire || nPart == ControlPart::Button )
				isSupported = true;
			break;

		case ControlType::Frame:
#ifdef USE_NATIVE_CTRL_FRAME
			if ( nPart == ControlPart::Border )
				isSupported = true;
#endif	// USE_NATIVE_CTRL_FRAME
			break;

		case ControlType::Tooltip:
			if( nPart == ControlPart::Entire )
				isSupported = true;
			break;

		default:
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
 * @param rIsInside             return parameter indicating whether aPos was
 *                              within the control or not
 * @return true if the function performed hit testing, false if default OOo
 *      hit testing should be used instead
 */
bool JavaSalGraphics::hitTestNativeControl( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion, const Point& aPos, bool& rIsInside )
{
	rIsInside = false;

#ifndef USE_NATIVE_CONTROLS
	if ( !IsNativeControlSupported( nType, nPart ) )
		return false;
#endif	// !USE_NATIVE_CONTROLS

	// [ed] Scrollbars are a special case:  in order to get proper regions,
	// a full description of the scrollbar is required including its values
	// and visible width.  We'll rely on the VCL scrollbar, which queried
	// these regions, to perform our hit testing.

	if ( nType == ControlType::Scrollbar )
		return false;

	tools::Rectangle aNativeBoundingRegion;
	tools::Rectangle aNativeContentRegion;
	if ( getNativeControlRegion( nType, nPart, rControlRegion, ControlState::NONE, ImplControlValue(), OUString(), aNativeBoundingRegion, aNativeContentRegion ) )
	{
		rIsInside = aNativeBoundingRegion.IsInside( aPos );
		return true;
	}
	else
	{
		return false;
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
 * @param rCaption		Caption title or string for the control.
 *				Contains keyboard shortcuts prefixed with ~
 * @return true if drawing was successful, false if drawing was not successful
 */
bool JavaSalGraphics::drawNativeControl( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion, ControlState nState, const ImplControlValue& aValue, const OUString& rCaption )
{
	bool bOK = false;

#ifndef USE_NATIVE_CONTROLS
	if ( !IsNativeControlSupported( nType, nPart ) )
		return bOK;
#endif	// !USE_NATIVE_CONTROLS

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	switch( nType )
	{
		case ControlType::Pushbutton:
			if( nPart == ControlPart::Entire )
			{
				bOK = DrawNativePushButton( this, rControlRegion, nState, aValue );
			}
			break;

		case ControlType::Radiobutton:
			if( nPart == ControlPart::Entire )
			{
				bOK = DrawNativeRadioButton( this, rControlRegion, nState, aValue );
			}
			break;

		case ControlType::Checkbox:
			if( nPart == ControlPart::Entire )
			{
				bOK = DrawNativeCheckbox( this, rControlRegion, nState, aValue );
			}
			break;

		case ControlType::Combobox:
			if( nPart == ControlPart::Entire )
			{
				bOK = DrawNativeComboBox( this, rControlRegion, nState, rCaption );
			}
			break;

		case ControlType::Listbox:
			if( nPart == ControlPart::Entire )
			{
				bOK = DrawNativeListBox( this, rControlRegion, nState, rCaption );
			}
			break;

		case ControlType::Scrollbar:
			if( ( nPart == ControlPart::Entire) || ( nPart == ControlPart::DrawBackgroundHorz ) || ( nPart == ControlPart::DrawBackgroundVert ) )
			{
				const ScrollbarValue *pValue = ( aValue.getType() == ControlType::Scrollbar ? static_cast< const ScrollbarValue* >( &aValue ) : nullptr );
				bOK = DrawNativeScrollBar( this, rControlRegion, nState, pValue );
			}
			break;

		case ControlType::Spinbox:
			if( ( nPart == ControlPart::Entire ) || ( nPart == ControlPart::AllButtons ) )
			{
				const SpinbuttonValue *pValue = ( aValue.getType() == ControlType::SpinButtons ? static_cast< const SpinbuttonValue* >( &aValue ) : nullptr );
				bOK = DrawNativeSpinbox( this, rControlRegion, nState, pValue );
			}
			break;

		case ControlType::SpinButtons:
			if( ( nPart == ControlPart::Entire ) || ( nPart == ControlPart::AllButtons ) )
			{
				const SpinbuttonValue *pValue = ( aValue.getType() == ControlType::SpinButtons ? static_cast< const SpinbuttonValue* >( &aValue ) : nullptr );
				bOK = DrawNativeSpinbutton( this, rControlRegion, nState, pValue );
			}
			break;

		case ControlType::Progress:
		case ControlType::IntroProgress:
			if( nPart == ControlPart::Entire )
			{
				bOK = DrawNativeProgressbar( this, rControlRegion, nState, &aValue, nType == ControlType::IntroProgress ? true : false );
			}
			break;

		case ControlType::TabItem:
			if( nPart == ControlPart::Entire )
			{
				const TabitemValue *pValue = ( aValue.getType() == ControlType::TabItem ? static_cast< const TabitemValue* >( &aValue ) : nullptr );
				bOK = DrawNativeTab( this, rControlRegion, nState, pValue );
			}
			break;

		case ControlType::TabPane:
			if( nPart == ControlPart::Entire )
			{
				tools::Rectangle ctrlRect( rControlRegion );
				// hack - on 10.3+ tab panes visually need to intersect the
				// middle of the associated segmented control.  Subtract
				// 15 off the height to shoehorn the drawing in.
				ctrlRect.Left() -= CONTROL_TAB_PANE_LEFT_OFFSET;
				ctrlRect.Top() -= CONTROL_TAB_PANE_TOP_OFFSET;
				ctrlRect.Right() += CONTROL_TAB_PANE_RIGHT_OFFSET;
				ctrlRect.Bottom() += CONTROL_TAB_PANE_BOTTOM_OFFSET;
				bOK = DrawNativeTabBoundingBox( this, ctrlRect, nState );
			}
			break;

		case ControlType::Groupbox:
			if( nPart == ControlPart::Entire )
			{
				bOK = DrawNativePrimaryGroupBox( this, rControlRegion, nState );
			}
			break;

		case ControlType::MenuPopup:
		case ControlType::Tooltip:
			if ( nPart == ControlPart::Entire )
			{
				bOK = DrawNativeMenuBackground( this, rControlRegion );
			}
			break;

		case ControlType::Editbox:
			if( nPart == ControlPart::Entire )
			{
				bOK = DrawNativeEditBox( this, rControlRegion, nState );
			}
			break;

		case ControlType::ListNode:
			if( nPart == ControlPart::Entire )
			{
				bOK = DrawNativeDisclosureButton( this, rControlRegion, nState, &aValue );
			}
			break;

		case ControlType::ListViewHeader:
			if( nPart == ControlPart::Entire )
			{
				const ListViewHeaderValue *pValue = ( aValue.getType() == ControlType::ListViewHeader ? static_cast< const ListViewHeaderValue* >( &aValue ) : nullptr );
				bOK = DrawNativeListViewHeader( this, rControlRegion, nState, pValue );
			}
			break;

		case ControlType::Fixedline:
			if( nPart == ControlPart::Entire )
			{
				bOK = DrawNativeSeparatorLine( this, rControlRegion, nState );
			}
			break;

		case ControlType::ListViewBox:
			if( nPart == ControlPart::Entire )
			{
				bOK = DrawNativeListBoxFrame( this, rControlRegion, nState );
			}
			break;

		case ControlType::Toolbar:
			if( nPart == ControlPart::Button )
			{
				bOK = DrawNativeBevelButton( this, rControlRegion, nState, aValue );
			}
			break;

		case ControlType::Frame:
			if ( nPart == ControlPart::Border )
			{
				DrawFrameFlags nValue = static_cast< DrawFrameFlags >( aValue.getNumericVal() & 0xfff0 );
				if ( ! ( nValue & ( DrawFrameFlags::Menu | DrawFrameFlags::WindowBorder ) ) )
				{
					DrawFrameStyle nStyle = static_cast< DrawFrameStyle >( aValue.getNumericVal() & 0x000f );
					tools::Rectangle ctrlRect( rControlRegion );
					ctrlRect.Left() -= FRAME_TRIMWIDTH;
					ctrlRect.Top() -= FRAME_TRIMWIDTH;
					if ( nStyle == DrawFrameStyle::DoubleIn )
					{
						ctrlRect.Right() += FRAME_TRIMWIDTH;
						ctrlRect.Bottom() += FRAME_TRIMWIDTH;
					}
					bOK = DrawNativeListBoxFrame( this, ctrlRect, ControlState::ENABLED );
				}
			}
			break;

		default:
			break;
	}

	[pPool release];

	return bOK;
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
 * @param rCaption		Caption title or string for the control.
 *				Contains keyboard shortcuts prefixed with ~
 * @param rNativeBoundingRegion	return parameter that contains the true bounds
 *				of the control along with any platform specific
 *				control adornment (shadows, etc.)
 * @param rNativeContentRegion	return parameter that indicates the region which
 *				can safely be overdrawn with custom content
 *				without overdrawing the system adornments of
 *				the control
 * @return true if appropriate information was returned about any native widget
 *	drawing areas, false if the entire control region can be considered
 *	an accurate enough representation of the native widget area
 */
bool JavaSalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion, ControlState nState, const ImplControlValue& aValue, const OUString& /* rCaption */, tools::Rectangle& rNativeBoundingRegion, tools::Rectangle& rNativeContentRegion )
{
	bool bReturn = false;

#ifndef USE_NATIVE_CONTROLS
	if ( !IsNativeControlSupported( nType, nPart ) )
		return bReturn;
#endif	// !USE_NATIVE_CONTROLS

	switch( nType )
	{
		case ControlType::Pushbutton:
			if( nPart == ControlPart::Entire )
			{
				// If the button width is less than the preferred height,
				// assume that it's intended to be a "placard" type button with
				// an icon. In that case, return the requested height as it
				// will then draw it as a placard button instead of a rounded
				// button. This makes buttons used as parts of subcontrols
				// (combo boxes, small toolbar buttons) draw with the
				// appropriate style.
				NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

				VCLNativeButton *pVCLNativeButton = [VCLNativeButton createWithButtonType:NSButtonTypeMomentaryLight bezelStyle:NSBezelStyleRounded controlSize:NSControlSizeRegular buttonState:NSControlStateValueOff controlState:ControlState::NONE drawRTL:NO bitmapBuffer:nullptr graphics:nullptr destRect:CGRectZero];
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				[pVCLNativeButton performSelectorOnMainThread:@selector(getSize:) withObject:pVCLNativeButton waitUntilDone:YES modes:pModes];
				NSSize aSize = [pVCLNativeButton size];
				if ( !NSEqualSizes( aSize, NSZeroSize ) )
				{
					tools::Rectangle buttonRect( rControlRegion );
					long buttonWidth = buttonRect.GetWidth();
					long buttonHeight = static_cast< long >( aSize.height );
					if ( buttonHeight >= buttonWidth )
						buttonHeight = buttonRect.GetHeight();
					else if ( buttonHeight != buttonRect.GetHeight() && buttonRect.GetHeight() > 0 )
						buttonHeight = buttonRect.GetHeight();

					Point topLeft( static_cast< long >( buttonRect.Left() - FOCUSRING_WIDTH), (long)(buttonRect.Top() + ( ( buttonRect.GetHeight() - buttonHeight) / 2) - FOCUSRING_WIDTH ) );
					Size boundsSize( static_cast< long >( buttonWidth ) + ( FOCUSRING_WIDTH * 2 ), static_cast< long >( buttonHeight ) + ( FOCUSRING_WIDTH * 2 ) );
					rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
					rNativeContentRegion = rNativeBoundingRegion;
					bReturn = true;
				}

				[pPool release];
			}
			break;

		case ControlType::Radiobutton:
			if( nPart == ControlPart::Entire )
			{
				tools::Rectangle buttonRect( rControlRegion );
				Point topLeft( static_cast< long >( buttonRect.Left() - FOCUSRING_WIDTH ), static_cast< long >( buttonRect.Top() - FOCUSRING_WIDTH ) );
				Size boundsSize( static_cast< long >( RADIOBUTTON_WIDTH ) + ( FOCUSRING_WIDTH * 2 ), static_cast< long >( RADIOBUTTON_HEIGHT ) + ( FOCUSRING_WIDTH * 2 ) );
				rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
				rNativeContentRegion = rNativeBoundingRegion;
				bReturn = true;
			}
			break;

		case ControlType::Checkbox:
			if( nPart == ControlPart::Entire )
			{
				tools::Rectangle buttonRect( rControlRegion );
				Point topLeft( static_cast< long >( buttonRect.Left() - FOCUSRING_WIDTH ), static_cast< long >( buttonRect.Top() - FOCUSRING_WIDTH ) );
				Size boundsSize( static_cast< long >( CHECKBOX_WIDTH ) + ( FOCUSRING_WIDTH * 2 ), static_cast< long >( CHECKBOX_HEIGHT ) + ( FOCUSRING_WIDTH * 2 ) );
				rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
				rNativeContentRegion = rNativeBoundingRegion;
				bReturn = true;
			}
			break;

		case ControlType::Combobox:
		case ControlType::Listbox:
			{
				tools::Rectangle comboBoxRect( rControlRegion );
				long nHeightAdjust = ( COMBOBOX_HEIGHT - comboBoxRect.GetHeight() ) / 2;
				if ( nHeightAdjust > 0 )
				{
					comboBoxRect.Top() -= nHeightAdjust;
					comboBoxRect.Bottom() += nHeightAdjust;
				}

				bool bEditable = ( nType == ControlType::Combobox ? YES : NO );

				NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

				VCLNativeComboBox *pVCLNativeComboBox = [VCLNativeComboBox createWithControlState:nState editable:bEditable bitmapBuffer:nullptr graphics:nullptr destRect:CGRectMake( comboBoxRect.Left(), comboBoxRect.Top(), comboBoxRect.GetWidth(), comboBoxRect.GetHeight() )];
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				[pVCLNativeComboBox performSelectorOnMainThread:@selector(getSize:) withObject:pVCLNativeComboBox waitUntilDone:YES modes:pModes];
				NSSize aSize = [pVCLNativeComboBox size];
				if ( !NSEqualSizes( aSize, NSZeroSize ) )
				{
					// Vertically center the preferred bounds
					float fYAdjust = ( static_cast< float >( comboBoxRect.GetHeight() ) - aSize.height ) / 2;
					NSRect preferredRect = NSMakeRect( comboBoxRect.Left(), comboBoxRect.Top() + fYAdjust, aSize.width > comboBoxRect.GetWidth() ? aSize.width : comboBoxRect.GetWidth(), aSize.height );
					BOOL bRTL = [pVCLNativeComboBox isRTL];

					switch( nPart )
					{
						case ControlPart::Entire:
							{
								// Fix vertical position of font size combobox
								// in toolbar by returning the preferred size
								Point topLeft( static_cast< long >( preferredRect.origin.x ), static_cast< long >( preferredRect.origin.y ) );
								Size boundsSize( static_cast< long >( preferredRect.size.width ), static_cast< long >( preferredRect.size.height ) );
								rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
								rNativeContentRegion = rNativeBoundingRegion;
								bReturn = true;
							}
							break;

						case ControlPart::ButtonDown:
							{
								if ( bEditable )
								{
									Point topLeft;
									if ( bRTL )
										topLeft = Point( static_cast< long >( preferredRect.origin.x ), static_cast< long >( preferredRect.origin.y ) );
									else
										topLeft = Point( static_cast< long >( preferredRect.origin.x ) + static_cast< long >( preferredRect.size.width ) - COMBOBOX_BUTTON_WIDTH - FOCUSRING_WIDTH, static_cast< long >( preferredRect.origin.y ) );
									Size boundsSize( COMBOBOX_BUTTON_WIDTH + FOCUSRING_WIDTH, static_cast< long >( preferredRect.size.height ) );
									rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
								}
								else
								{
									Point topLeft( static_cast< long >( preferredRect.origin.x ), static_cast< long >( preferredRect.origin.y ) );
									Size boundsSize( static_cast< long >( preferredRect.size.width ), static_cast< long >( preferredRect.size.height ) );
									rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
								}

								rNativeContentRegion = rNativeBoundingRegion;
								bReturn = true;
							}
							break;

						case ControlPart::SubEdit:
							{
								Point topLeft;
								if ( bRTL )
									topLeft = Point( static_cast< long >( preferredRect.origin.x ) + ( bEditable ? COMBOBOX_BUTTON_WIDTH + FOCUSRING_WIDTH + EDITFRAMEPADDING_WIDTH : LISTBOX_BUTTON_WIDTH - EDITFRAMEPADDING_WIDTH ), static_cast< long >( preferredRect.origin.y ) + FOCUSRING_WIDTH + EDITFRAMEPADDING_WIDTH );
								else
									topLeft = Point( static_cast< long >( preferredRect.origin.x ) + FOCUSRING_WIDTH + EDITFRAMEPADDING_WIDTH, static_cast< long >( preferredRect.origin.y ) + FOCUSRING_WIDTH + EDITFRAMEPADDING_WIDTH );
								Size boundsSize( static_cast< long >( preferredRect.size.width ) - ( bEditable ? COMBOBOX_BUTTON_WIDTH : LISTBOX_BUTTON_WIDTH ) - ( ( FOCUSRING_WIDTH + EDITFRAMEPADDING_WIDTH ) * 2 ), static_cast< long >( preferredRect.size.height ) - ( ( FOCUSRING_WIDTH + EDITFRAMEPADDING_WIDTH ) * 2 ) );
								rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
								rNativeContentRegion = rNativeBoundingRegion;
								bReturn = true;
							}
							break;

						default:
							break;
					}
				}

				[pPool release];
			}
			break;

		case ControlType::Scrollbar:
			{
				// Fix bug 1600 by detecting if double arrows are at both ends
				tools::Rectangle scrollbarRect( rControlRegion );
				const ScrollbarValue *pValue = ( aValue.getType() == ControlType::Scrollbar ? static_cast< const ScrollbarValue* >( &aValue ) : nullptr );
				bool bDoubleScrollbarArrows = GetSalData()->mbDoubleScrollbarArrows;

				NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

				VCLNativeScrollbar *pVCLNativeScrollbar = [VCLNativeScrollbar createWithControlState:nState bitmapBuffer:nullptr graphics:nullptr scrollbarValue:pValue doubleScrollbarArrows:bDoubleScrollbarArrows destRect:CGRectMake( scrollbarRect.Left(), scrollbarRect.Top(), scrollbarRect.GetWidth(), scrollbarRect.GetHeight() )];
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				[pVCLNativeScrollbar performSelectorOnMainThread:@selector(getBounds:) withObject:pVCLNativeScrollbar waitUntilDone:YES modes:pModes];

				BOOL bHorizontal = [pVCLNativeScrollbar horizontal];
				NSRect aDecrementArrowBounds = [pVCLNativeScrollbar decrementArrowBounds];
				NSRect aIncrementArrowBounds = [pVCLNativeScrollbar incrementArrowBounds];
				NSRect aDecrementPageBounds = [pVCLNativeScrollbar decrementPageBounds];
				NSRect aIncrementPageBounds = [pVCLNativeScrollbar incrementPageBounds];
				NSRect aThumbBounds = [pVCLNativeScrollbar thumbBounds];
				NSRect aTrackBounds = [pVCLNativeScrollbar trackBounds];
				NSRect aTotalBounds = [pVCLNativeScrollbar totalBounds];
				aTotalBounds = NSUnionRect( aTotalBounds, aDecrementArrowBounds );
				aTotalBounds = NSUnionRect( aTotalBounds, aIncrementArrowBounds );
				aTotalBounds = NSUnionRect( aTotalBounds, aDecrementPageBounds );
				aTotalBounds = NSUnionRect( aTotalBounds, aIncrementPageBounds );
				aTotalBounds = NSUnionRect( aTotalBounds, aThumbBounds );
				aTotalBounds = NSUnionRect( aTotalBounds, aTrackBounds );

				NSRect bounds = NSZeroRect;
				switch ( nPart )
				{
					case ControlPart::Entire:
						bounds = aTotalBounds;
						break;

					case ControlPart::ButtonLeft:
					case ControlPart::ButtonUp:
						bounds = aDecrementArrowBounds;
						if ( bDoubleScrollbarArrows )
						{
							if ( bHorizontal )
								bounds.size.width *= 2;
							else
								bounds.size.height *= 2;
						}
						break;

					case ControlPart::ButtonRight:
					case ControlPart::ButtonDown:
						bounds = aIncrementArrowBounds;
						if ( bDoubleScrollbarArrows )
						{
							if ( bHorizontal )
							{
								bounds.origin.x -= bounds.size.width;
								bounds.size.width *= 2;
							}
							else
							{
								bounds.origin.y -= bounds.size.height;
								bounds.size.height *= 2;
							}
						}
						break;

					case ControlPart::TrackHorzLeft:
					case ControlPart::TrackVertUpper:
						// If bounds are empty, set to the entire track
						bounds = aDecrementPageBounds;
						if ( NSEqualRects( bounds, NSZeroRect ) )
							bounds = aTrackBounds;
						break;

					case ControlPart::TrackHorzRight:
					case ControlPart::TrackVertLower:
						// If bounds are empty, set to the entire track
						bounds = aIncrementPageBounds;
						if ( NSEqualRects( bounds, NSZeroRect ) )
							bounds = aTrackBounds;
						break;

					case ControlPart::ThumbHorz:
					case ControlPart::ThumbVert:
						bounds = aThumbBounds;
						break;
					
					case ControlPart::TrackHorzArea:
					case ControlPart::TrackVertArea:
						bounds = aTrackBounds;
						break;

					default:
						break;
				}

				// Fix bug 2031 by incrementing the scrollbar width slightly
				if ( !NSEqualRects( bounds, NSZeroRect ) )
				{
					if ( bHorizontal )
						bounds.size.height++;
					else
						bounds.size.width++;
				}

				Point topLeft( static_cast< long >( scrollbarRect.Left() + bounds.origin.x ), static_cast< long >( scrollbarRect.Top() + bounds.origin.y ) );
				Size boundsSize( static_cast< long >( bounds.size.width ), static_cast< long >( bounds.size.height ) );
				rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
				rNativeContentRegion = rNativeBoundingRegion;
				bReturn = true;

				[pPool release];
			}
			break;

		case ControlType::Spinbox:
			{
				tools::Rectangle spinboxRect( rControlRegion );
				const SpinbuttonValue *pValue = ( aValue.getType() == ControlType::SpinButtons ? static_cast< const SpinbuttonValue* >( &aValue ) : nullptr );

				NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

				VCLNativeSpinbuttons *pVCLNativeSpinbuttons = [VCLNativeSpinbuttons createWithControlState:nState bitmapBuffer:nullptr graphics:nullptr spinbuttonValue:pValue destRect:CGRectMake( spinboxRect.Left(), spinboxRect.Top(), spinboxRect.GetWidth(), spinboxRect.GetHeight() )];
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				[pVCLNativeSpinbuttons performSelectorOnMainThread:@selector(getSize:) withObject:pVCLNativeSpinbuttons waitUntilDone:YES modes:pModes];
				NSSize aSize = [pVCLNativeSpinbuttons size];
				if ( !NSEqualSizes( aSize, NSZeroSize ) )
				{
					// Fix excessive reduction in spinner height in Impress'
					// Line and Fill toolbar by setting the height to at least
					// the full height of the native spinners
					long nHeightAdjust = ( static_cast< long >( aSize.height ) - spinboxRect.GetHeight() ) / 2;
					if ( nHeightAdjust > 0 )
					{
						spinboxRect.Top() -= nHeightAdjust;
						spinboxRect.Bottom() += nHeightAdjust;
					}

					switch( nPart )
					{
						case ControlPart::Entire:
							{
								rNativeBoundingRegion = spinboxRect;
								rNativeContentRegion = rNativeBoundingRegion;
								bReturn = true;
							}
							break;

						case ControlPart::ButtonUp:
							{
								Point topLeft( static_cast< long >( spinboxRect.Right() - aSize.width - FOCUSRING_WIDTH ), static_cast< long >( spinboxRect.Top() + ( ( spinboxRect.GetHeight() - aSize.height ) / 2 ) - FOCUSRING_WIDTH ) );
								Size boundsSize( static_cast< long >( aSize.width + ( FOCUSRING_WIDTH * 2 ) ), static_cast< long >( ( aSize.height / 2 ) + FOCUSRING_WIDTH ) );
								rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
								rNativeContentRegion = rNativeBoundingRegion;
								bReturn = true;
							}
							break;

						case ControlPart::ButtonDown:
							{
								Point topLeft( static_cast< long >( spinboxRect.Right() - aSize.width - FOCUSRING_WIDTH ), static_cast< long >( spinboxRect.Top() + ( spinboxRect.GetHeight() / 2 ) ) );
								Size boundsSize( static_cast< long >( aSize.width + ( FOCUSRING_WIDTH * 2 ) ), static_cast< long >( ( aSize.height / 2 ) + FOCUSRING_WIDTH ) );
								rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
								rNativeContentRegion = rNativeBoundingRegion;
								bReturn = true;
							}
							break;

						case ControlPart::SubEdit:
							{
								Point topLeft( spinboxRect.Left() + FOCUSRING_WIDTH, spinboxRect.Top() + FOCUSRING_WIDTH );
								Size boundsSize( spinboxRect.GetWidth() - static_cast< long >( aSize.width ) - ( FOCUSRING_WIDTH * 2 ), spinboxRect.GetHeight() - ( FOCUSRING_WIDTH * 2 ) );
								rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
								rNativeContentRegion = rNativeBoundingRegion;
								bReturn = true;
							}
							break;

						default:
							break;
					}
				}

				[pPool release];
			}
			break;

		case ControlType::SpinButtons:
			{
				tools::Rectangle spinboxRect( rControlRegion );
				const SpinbuttonValue *pValue = ( aValue.getType() == ControlType::SpinButtons ? static_cast< const SpinbuttonValue* >( &aValue ) : nullptr );

				NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

				VCLNativeSpinbuttons *pVCLNativeSpinbuttons = [VCLNativeSpinbuttons createWithControlState:nState bitmapBuffer:nullptr graphics:nullptr spinbuttonValue:pValue destRect:CGRectMake( spinboxRect.Left(), spinboxRect.Top(), spinboxRect.GetWidth(), spinboxRect.GetHeight() )];
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				[pVCLNativeSpinbuttons performSelectorOnMainThread:@selector(getSize:) withObject:pVCLNativeSpinbuttons waitUntilDone:YES modes:pModes];
				NSSize aSize = [pVCLNativeSpinbuttons size];
				if ( !NSEqualSizes( aSize, NSZeroSize ) )
				{
					switch( nPart )
					{
						case ControlPart::Entire:
							{
								Point topLeft( static_cast< long >( spinboxRect.Right() - aSize.width - FOCUSRING_WIDTH ), static_cast< long >( spinboxRect.Top() + ( ( spinboxRect.GetHeight() - aSize.height ) / 2 ) - FOCUSRING_WIDTH ) );
								Size boundsSize( static_cast< long >( aSize.width + ( FOCUSRING_WIDTH * 2 ) ), static_cast< long >( aSize.height + ( FOCUSRING_WIDTH * 2 ) ) );
								rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
								rNativeContentRegion = rNativeBoundingRegion;
								bReturn = true;
							}
							break;

						case ControlPart::ButtonUp:
							{
								Point topLeft( static_cast< long >( spinboxRect.Right() - aSize.width - FOCUSRING_WIDTH ), static_cast< long >( spinboxRect.Top() + ( ( spinboxRect.GetHeight() - aSize.height ) / 2 ) - FOCUSRING_WIDTH ) );
								Size boundsSize( static_cast< long >( aSize.width + ( FOCUSRING_WIDTH * 2 ) ), static_cast< long >( ( aSize.height / 2 ) + FOCUSRING_WIDTH ) );
								rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
								rNativeContentRegion = rNativeBoundingRegion;
								bReturn = true;
							}
							break;

						case ControlPart::ButtonDown:
							{
								Point topLeft( static_cast< long >( spinboxRect.Right() - aSize.width - FOCUSRING_WIDTH ), static_cast< long >( spinboxRect.Top() + ( spinboxRect.GetHeight() / 2 ) ) );
								Size boundsSize( static_cast< long >( aSize.width + ( FOCUSRING_WIDTH * 2 ) ), static_cast< long >( ( aSize.height / 2 ) + FOCUSRING_WIDTH ) );
								rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
								rNativeContentRegion = rNativeBoundingRegion;
								bReturn = true;
							}
							break;

						default:
							break;
					}
				}

				[pPool release];
			}
			break;

		case ControlType::Progress:
		case ControlType::IntroProgress:
			if ( nPart == ControlPart::Entire )
			{
				tools::Rectangle controlRect( rControlRegion );

				NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

				VCLNativeProgressbar *pVCLNativeProgressbar = [VCLNativeProgressbar createWithControlState:nState controlSize:( nType == ControlType::IntroProgress ? NSControlSizeSmall : NSControlSizeRegular ) bitmapBuffer:nullptr graphics:nullptr progressbarValue:&aValue destRect:CGRectMake( controlRect.Left(), controlRect.Top(), controlRect.GetWidth(), controlRect.GetHeight() )];
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				[pVCLNativeProgressbar performSelectorOnMainThread:@selector(getSize:) withObject:pVCLNativeProgressbar waitUntilDone:YES modes:pModes];
				NSSize aSize = [pVCLNativeProgressbar size];
				if ( !NSEqualSizes( aSize, NSZeroSize ) )
				{
					// Vertically center the preferred bounds
					float fYAdjust = ( static_cast< float >( controlRect.GetHeight() ) - aSize.height ) / 2;
					NSRect preferredRect = NSMakeRect( controlRect.Left(), controlRect.Top() + fYAdjust, aSize.width > controlRect.GetWidth() ? aSize.width : controlRect.GetWidth(), aSize.height );

					// Fix clipping of small progress bar by adding padding
					Point topLeft( static_cast< long >( preferredRect.origin.x ), static_cast< long >( preferredRect.origin.y ) - PROGRESSBARPADDING_HEIGHT );
					Size boundsSize( static_cast< long >( preferredRect.size.width ), static_cast< long >( preferredRect.size.height ) + ( PROGRESSBARPADDING_HEIGHT * 2 ) );
					rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
					rNativeContentRegion = rNativeBoundingRegion;
					bReturn = true;
				}

				[pPool release];
			}
			break;

		case ControlType::TabItem:
			if ( nPart == ControlPart::Entire )
			{
				tools::Rectangle controlRect( rControlRegion );
				const TabitemValue *pValue = ( aValue.getType() == ControlType::TabItem ? static_cast< const TabitemValue* >( &aValue ) : nullptr );

				NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

				VCLNativeTabCell *pVCLNativeTabCell = [VCLNativeTabCell createWithControlState:nState bitmapBuffer:nullptr graphics:nullptr tabitemValue:pValue destRect:CGRectMake( controlRect.Left(), controlRect.Top(), controlRect.GetWidth(), controlRect.GetHeight() )];
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				[pVCLNativeTabCell performSelectorOnMainThread:@selector(getSize:) withObject:pVCLNativeTabCell waitUntilDone:YES modes:pModes];
				NSSize aSize = [pVCLNativeTabCell size];
				if ( !NSEqualSizes( aSize, NSZeroSize ) )
				{
					Point topLeft( controlRect.Left(), controlRect.Top() - FOCUSRING_WIDTH );
					Size boundsSize( static_cast< long >( aSize.width ), static_cast< long >( aSize.height ) + ( FOCUSRING_WIDTH * 2 ) );
					rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
					rNativeContentRegion = rNativeBoundingRegion;
					bReturn = true;
				}

				[pPool release];
			}
			break;

		case ControlType::TabPane:
			if ( nPart == ControlPart::Entire )
			{
				// for now, assume tab panes will occupy the full rectangle and
				// not require bound adjustment.
				rNativeBoundingRegion = rControlRegion;
				rNativeContentRegion = rNativeBoundingRegion;
				bReturn = true;
			}
			break;

		case ControlType::Groupbox:
			if ( nPart == ControlPart::Entire )
			{
				// for now, assume primary group boxes will occupy the full rectangle and
				// not require bound adjustment.
				rNativeBoundingRegion = rControlRegion;
				rNativeContentRegion = rNativeBoundingRegion;
				bReturn = true;
			}
			break;

		case ControlType::MenuPopup:
		case ControlType::Tooltip:
			if ( nPart == ControlPart::Entire )
			{
				// we can draw menu backgrounds for any size rectangular area
				rNativeBoundingRegion = rControlRegion;
				rNativeContentRegion = rNativeBoundingRegion;
				bReturn = true;
			}
			break;

		case ControlType::Editbox:
			{
				// fill entire control area with edit box
				tools::Rectangle controlRect( rControlRegion );
				long nHeightAdjust = ( EDITBOX_HEIGHT - controlRect.GetHeight() ) / 2;
				if ( nHeightAdjust > 0 )
				{
					controlRect.Top() -= nHeightAdjust;
					controlRect.Bottom() += nHeightAdjust;
				}

				switch( nPart )
				{
					case ControlPart::Entire:
						{
							rNativeBoundingRegion = controlRect;
							rNativeContentRegion = rNativeBoundingRegion;
							bReturn = true;
						}
						break;

					case ControlPart::SubEdit:
						{
							Point topLeft( controlRect.Left() + FOCUSRING_WIDTH, controlRect.Top() + FOCUSRING_WIDTH );
							Size boundsSize( controlRect.GetWidth() - ( FOCUSRING_WIDTH * 2 ), controlRect.GetHeight() - ( FOCUSRING_WIDTH * 2 ) );
							rNativeBoundingRegion = tools::Rectangle( topLeft, boundsSize );
							rNativeContentRegion = rNativeBoundingRegion;
							bReturn = true;
						}
						break;

					default:
						break;
				}

				bReturn = true;
			}
			break;

		case ControlType::ListNode:
			if ( nPart == ControlPart::Entire )
			{
				rNativeBoundingRegion = rControlRegion;
				rNativeContentRegion = rNativeBoundingRegion;
				bReturn = true;
			}
			break;

		case ControlType::ListViewHeader:
			if ( nPart == ControlPart::Entire )
			{
				rNativeBoundingRegion = rControlRegion;
				rNativeContentRegion = rNativeBoundingRegion;
				bReturn = true;
			}
			break;

		case ControlType::ListViewBox:
			if ( nPart == ControlPart::Entire )
			{
				rNativeBoundingRegion = rControlRegion;
				rNativeContentRegion = rNativeBoundingRegion;
				bReturn = true;
			}
			break;

		case ControlType::Toolbar:
			if ( nPart == ControlPart::Button )
			{
				rNativeBoundingRegion = rControlRegion;
				rNativeContentRegion = rNativeBoundingRegion;
				bReturn = true;
			}
			break;

		case ControlType::Frame:
			if ( nPart == ControlPart::Border )
			{
				DrawFrameFlags nValue = static_cast< DrawFrameFlags >( aValue.getNumericVal() & 0xfff0 );
				if ( ! ( nValue & ( DrawFrameFlags::Menu | DrawFrameFlags::WindowBorder ) ) )
				{
					DrawFrameStyle nStyle = static_cast< DrawFrameStyle >( aValue.getNumericVal() & 0x000f );
					tools::Rectangle controlRect( rControlRegion );
					controlRect.Left() += FRAME_TRIMWIDTH;
					controlRect.Top() += FRAME_TRIMWIDTH;
					if ( nStyle == DrawFrameStyle::DoubleIn )
					{
						controlRect.Right() -= FRAME_TRIMWIDTH;
						controlRect.Bottom() -= FRAME_TRIMWIDTH;
					}
					rNativeBoundingRegion = controlRect;
					rNativeContentRegion = rNativeBoundingRegion;
					bReturn = true;
				}
			}
			break;

		default:
			break;
	}

	return bReturn;
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
 * @param nTextColor	location in which color to draw text should be returned
 * @return true if a native widget text color is provided, false if the standard
 *	VCL text color should be used.
 */
bool JavaSalGraphics::getNativeControlTextColor( ControlType nType, ControlPart nPart, ControlState nState, const ImplControlValue& /* aValue */, SalColor& nTextColor )
{
#ifdef USE_NATIVE_CONTROLS
	(void)nPart;
#endif	// USE_NATIVE_CONTROLS

	bool bReturn = false;

#ifndef USE_NATIVE_CONTROLS
	if ( !IsNativeControlSupported( nType, nPart ) )
		return bReturn;
#endif	// !USE_NATIVE_CONTROLS

	if ( mpFrame && !mpFrame->IsFloatingFrame() && mpFrame != GetSalData()->mpFocusFrame )
		nState |= ControlState::INACTIVE;

	switch( nType )
	{
		case ControlType::Pushbutton:
			{
				if( nState & ( ControlState::DEFAULT | ControlState::FOCUSED | ControlState::PRESSED ) )
				{
					// Fix wrong color in the File > New > Database dialog's
					// "Next" button in the "Set up dBASE connection" panel
					if ( ! ( nState & ControlState::ENABLED ) )
						bReturn = JavaSalFrame::GetDisabledControlTextColor( nTextColor );
					// Fix text color when running on OS X 10.10 and our
					// application is not the active application. Fix wrong
					// color in the File > New > Database dialog's default
					// button when the help window has focus by not using the
					// selected color when the button is inactive.
					else if ( nState & ControlState::INACTIVE || !NSApplication_isActive() )
						bReturn = JavaSalFrame::GetSelectedControlTextColor( nTextColor );
					else
						bReturn = JavaSalFrame::GetAlternateSelectedControlTextColor( nTextColor );
				}
				else if ( ! ( nState & ControlState::ENABLED ) )
				{
					bReturn = JavaSalFrame::GetDisabledControlTextColor( nTextColor );
				}
				else
				{
					bReturn = JavaSalFrame::GetControlTextColor( nTextColor );
				}
			}
			break;

		case ControlType::Radiobutton:
		case ControlType::Checkbox:
		case ControlType::Listbox:
			{
				if( nState & ControlState::PRESSED )
					bReturn = JavaSalFrame::GetSelectedControlTextColor( nTextColor );
				else if ( ! ( nState & ControlState::ENABLED ) )
					bReturn = JavaSalFrame::GetDisabledControlTextColor( nTextColor );
				else
					bReturn = JavaSalFrame::GetControlTextColor( nTextColor );
			}
			break;

		case ControlType::TabItem:
			{
				if ( nState & ControlState::SELECTED )
				{
					if ( IsRunningCatalinaOrLower() )
					{
						if ( ! ( nState & ControlState::INACTIVE ) )
							bReturn = JavaSalFrame::GetAlternateSelectedControlTextColor( nTextColor );
						else
							bReturn = JavaSalFrame::GetSelectedTabTextColor( nTextColor );
					}
					else
					{
						if ( JavaSalFrame::UseDarkModeColors() )
							bReturn = JavaSalFrame::GetAlternateSelectedControlTextColor( nTextColor );
						else
							bReturn = JavaSalFrame::GetSelectedTabTextColor( nTextColor );
					}
				}
				else if ( nState & ControlState::PRESSED )
				{
					bReturn = JavaSalFrame::GetSelectedControlTextColor( nTextColor );
				}
				else if ( ! ( nState & ControlState::ENABLED ) )
				{
					bReturn = JavaSalFrame::GetDisabledControlTextColor( nTextColor );
				}
				else
				{
					bReturn = JavaSalFrame::GetControlTextColor( nTextColor );
				}
			}
			break;

		case ControlType::MenuPopup:
			{
				if ( nState & ControlState::SELECTED )
					bReturn = JavaSalFrame::GetSelectedMenuItemTextColor( nTextColor );
				else if ( ! ( nState & ControlState::ENABLED ) )
					bReturn = JavaSalFrame::GetDisabledControlTextColor( nTextColor );
				else
					bReturn = JavaSalFrame::GetControlTextColor( nTextColor );
			}
			break;

		default:
			break;
	}

	return bReturn;
}
