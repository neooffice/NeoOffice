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
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 Planamesa Inc.
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

#include <vcl/dialog.hxx>
#include <vcl/settings.hxx>
#include <vcl/status.hxx>
#include <vcl/svapp.hxx>

#include <premac.h>
#import <AppKit/AppKit.h>
#import <IOKit/pwr_mgt/IOPMLib.h>
#import <objc/objc-class.h>
#include <postmac.h>
#undef check

#include "java/saldata.hxx"
#include "java/salframe.h"
#include "java/salgdi.h"
#include "java/salinst.h"
#include "java/salmenu.h"
#include "java/salobj.h"
#include "java/salsys.h"

#include "../java/VCLEventQueue_cocoa.h"

#define MIN_CONTENT_WIDTH 130

static ::std::map< NSWindow*, JavaSalGraphics* > aNativeWindowMap;
static ::std::map< NSWindow*, NSCursor* > aNativeCursorMap;
static ::std::map< NSWindow*, NSTimer* > aNativeFlushTimerMap;
static bool bScreensInitialized = false;
static NSRect aTotalScreenBounds = NSZeroRect;
static ::std::vector< Rectangle > aVCLScreensFullBoundsList;
static ::std::vector< Rectangle > aVCLScreensVisibleBoundsList;
static ::osl::Mutex aScreensMutex;
static bool bSystemColorsInitialized = false;
static SalColor *pVCLControlTextColor = NULL;
static SalColor *pVCLTextColor = NULL;
static SalColor *pVCLHighlightColor = NULL;
static SalColor *pVCLHighlightTextColor = NULL;
static SalColor *pVCLDisabledControlTextColor = NULL;
static SalColor *pVCLBackColor = NULL;
static SalColor *pVCLAlternateSelectedControlTextColor = NULL;
static SalColor *pVCLSelectedControlTextColor = NULL;
static SalColor *pVCLSelectedMenuItemColor = NULL;
static SalColor *pVCLSelectedMenuItemTextColor = NULL;
static SalColor *pVCLShadowColor = NULL;
static long nVCLScrollbarSize = 0;

static ::osl::Mutex aSystemColorsMutex;
static NSString *pVCLTrackingAreaWindowKey = @"VCLTrackingAreaWindow";

inline long Float32ToLong( Float32 f ) { return (long)( f + 0.5 ); }

using namespace osl;
using namespace vcl;

@interface NSColor (VCLColor)
+ (NSColor *)unemphasizedSelectedTextBackgroundColor;
@end

static NSRect GetTotalScreenBounds()
{
	if ( NSIsEmptyRect( aTotalScreenBounds ) )
	{
		NSArray *pScreens = [NSScreen screens];
		if ( pScreens )
		{
			NSUInteger nCount = [pScreens count];
			NSUInteger i = 0;
			for ( ; i < nCount; i++ )
			{
				NSScreen *pScreen = [pScreens objectAtIndex:i];
				if ( pScreen )
				{
					NSRect aScreenFrame = [pScreen frame];
					if ( NSIsEmptyRect( aTotalScreenBounds ) )
						aTotalScreenBounds = aScreenFrame;
					else
						aTotalScreenBounds = NSUnionRect( aScreenFrame, aTotalScreenBounds );
				}
			}
		}
	}

	return aTotalScreenBounds;
}

static void HandleScreensChangedRequest()
{
	MutexGuard aGuard( aScreensMutex );

	bScreensInitialized = true;
	aTotalScreenBounds = NSZeroRect;
	aVCLScreensFullBoundsList.clear();
	aVCLScreensVisibleBoundsList.clear();

	NSArray *pScreens = [NSScreen screens];
	if ( pScreens )
	{
		// Calculate the total combined screen so that we can flip coordinates
		NSRect aTotalBounds = GetTotalScreenBounds();
		NSUInteger nCount = [pScreens count];
		NSUInteger i = 0;
		for ( ; i < nCount; i++ )
		{
			NSRect aLastFullFrame = NSMakeRect( 0, 0, 0, 0 );
			NSScreen *pScreen = [pScreens objectAtIndex:i];
			if ( pScreen )
			{
				NSRect aFullFrame = [pScreen frame];
				NSRect aVisibleFrame = [pScreen visibleFrame];

				// On some machines, there are two monitors for every mirrored
				// display so eliminate those duplicate monitors
				if ( i && NSEqualRects( aLastFullFrame, aFullFrame ) )
					continue;
				aLastFullFrame = aFullFrame;

				// Flip coordinates and cache bounds
				Rectangle aFullRect( Point( (long)aFullFrame.origin.x, (long)aTotalBounds.size.height - aFullFrame.origin.y - aFullFrame.size.height ), Size( (long)aFullFrame.size.width, (long)aFullFrame.size.height ) );
				Rectangle aVisibleRect( Point( (long)aVisibleFrame.origin.x, (long)aTotalBounds.size.height - aVisibleFrame.origin.y- aVisibleFrame.size.height ), Size( (long)aVisibleFrame.size.width, (long)aVisibleFrame.size.height ) );
				aFullRect.Justify();
				aVisibleRect.Justify();
				aVCLScreensFullBoundsList.push_back( aFullRect );
				aVCLScreensVisibleBoundsList.push_back( aVisibleRect );
			}
		}
	}
}

static sal_Bool SetSalColorFromNSColor( NSColor *pNSColor, SalColor **ppSalColor )
{
	sal_Bool bRet = sal_False;

	if ( ppSalColor )
	{
		if ( *ppSalColor )
		{
			delete *ppSalColor;
			*ppSalColor = NULL;
		}

		if ( pNSColor )
		{
			pNSColor = [pNSColor colorUsingColorSpaceName:NSDeviceRGBColorSpace];
			if ( pNSColor )
			{
				// Remove transparency by blending color with opaque gray
				float fAlpha = [pNSColor alphaComponent];
				float fRed = [pNSColor redComponent];
				float fGreen = [pNSColor greenComponent];
				float fBlue = [pNSColor blueComponent];
				*ppSalColor = new SalColor;
				**ppSalColor = MAKE_SALCOLOR( (unsigned char)( ( 0.5f + ( ( fRed - 0.5f ) * fAlpha ) ) * 0xff ), (unsigned char)( ( 0.5f + ( ( fGreen - 0.5f ) * fAlpha ) ) * 0xff ), (unsigned char)( ( 0.5f + ( ( fBlue - 0.5f ) * fAlpha ) ) * 0xff ) );
				bRet = sal_True;
			}
		}
	}

	return bRet;
}

static void HandleSystemColorsChangedRequest()
{
	MutexGuard aGuard( aSystemColorsMutex );

	bSystemColorsInitialized = true;

	SetSalColorFromNSColor( [NSColor controlTextColor], &pVCLControlTextColor );
	SetSalColorFromNSColor( [NSColor textColor], &pVCLTextColor );
	SetSalColorFromNSColor( [NSColor selectedTextBackgroundColor], &pVCLHighlightColor );
	SetSalColorFromNSColor( [NSColor selectedTextColor], &pVCLHighlightTextColor );
	SetSalColorFromNSColor( [NSColor disabledControlTextColor], &pVCLDisabledControlTextColor );
	if ( class_getClassMethod( [NSColor class], @selector(unemphasizedSelectedTextBackgroundColor) ) )
		SetSalColorFromNSColor( [NSColor unemphasizedSelectedTextBackgroundColor], &pVCLBackColor );
	else if ( class_getClassMethod( [NSColor class], @selector(controlHighlightColor) ) )
		SetSalColorFromNSColor( [NSColor controlHighlightColor], &pVCLBackColor );
	SetSalColorFromNSColor( [NSColor alternateSelectedControlTextColor], &pVCLAlternateSelectedControlTextColor );
	SetSalColorFromNSColor( [NSColor selectedControlTextColor], &pVCLSelectedControlTextColor );
	SetSalColorFromNSColor( [NSColor selectedMenuItemColor], &pVCLSelectedMenuItemColor );
	SetSalColorFromNSColor( [NSColor selectedMenuItemTextColor], &pVCLSelectedMenuItemTextColor );
	SetSalColorFromNSColor( [NSColor controlShadowColor], &pVCLShadowColor );

	// Always use NSScrollerStyleLegacy scrollbars as we always draw scrollbars 
	// with that style in vcl/java/source/gdi/salnativewidgets.mm
	nVCLScrollbarSize = Float32ToLong( [NSScroller scrollerWidthForControlSize:NSControlSizeRegular scrollerStyle:NSScrollerStyleLegacy] );
}

@interface VCLSetSystemUIMode : NSObject
{
	BOOL					mbFullScreen;
}
+ (id)createFullScreen:(BOOL)bFullScreen;
- (id)initFullScreen:(BOOL)bFullScreen;
- (void)setSystemUIMode:(id)pObject;
@end

static IOPMAssertionID nIOPMAssertionID;
static BOOL bIOPMAssertionIDSet = NO;

@implementation VCLSetSystemUIMode

+ (id)createFullScreen:(BOOL)bFullScreen
{
	VCLSetSystemUIMode *pRet = [[VCLSetSystemUIMode alloc] initFullScreen:bFullScreen];
	[pRet autorelease];
	return pRet;
}

- (id)initFullScreen:(BOOL)bFullScreen
{
	[super init];

	mbFullScreen = bFullScreen;

	return self;
}

- (void)setSystemUIMode:(id)pObject
{
	(void)pObject;

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		if ( mbFullScreen )
		{
			[pApp setPresentationOptions:NSApplicationPresentationHideDock | NSApplicationPresentationHideMenuBar | NSApplicationPresentationDisableAppleMenu | NSApplicationPresentationDisableProcessSwitching];

			// Block sleeping
			if ( !bIOPMAssertionIDSet )
			{
				NSString *pBundleDisplayName = nil;
				NSBundle *pBundle = [NSBundle mainBundle];
				if ( pBundle )
				{
					NSDictionary *pInfoDict = [pBundle infoDictionary];
					if ( pInfoDict )
						pBundleDisplayName = [pInfoDict objectForKey:@"CFBundleDisplayName"];
				}

				if ( IOPMAssertionCreateWithName( kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, pBundleDisplayName ? (CFStringRef)pBundleDisplayName : CFSTR( "" ), &nIOPMAssertionID ) == kIOReturnSuccess )
					bIOPMAssertionIDSet = YES;
			}
		}
		else
		{
			// Fix hidden menu after existing slide show mode to a full screen
			// mode window on OS X 10.11 by explicitly setting the presentation
			// options to NSApplicationPresentationAutoHideMenuBar and
			// NSApplicationPresentationAutoHideDock
			if ( [pApp presentationOptions] & NSApplicationPresentationFullScreen )
				[pApp setPresentationOptions:NSApplicationPresentationDefault | NSApplicationPresentationAutoHideMenuBar | NSApplicationPresentationAutoHideDock];
			else
				[pApp setPresentationOptions:NSApplicationPresentationDefault];

			// Stop blocking sleep
			if ( bIOPMAssertionIDSet )
			{
				bIOPMAssertionIDSet = NO;
				IOPMAssertionRelease( nIOPMAssertionID );
			}
		}
	}
}

@end

@interface VCLToggleFullScreen : NSObject
{
	BOOL					mbToggleToCurrentScreenMode;
	NSWindow*				mpWindow;
}
+ (id)createToggleFullScreen:(NSWindow *)pWindow toggleToCurrentScreenMode:(BOOL)bToggleToCurrentScreenMode;
- (id)initToggleFullScreen:(NSWindow *)pWindow toggleToCurrentScreenMode:(BOOL)bToggleToCurrentScreenMode;
- (void)dealloc;
- (void)toggleFullScreen:(id)pObject;
@end

@implementation VCLToggleFullScreen

+ (id)createToggleFullScreen:(NSWindow *)pWindow toggleToCurrentScreenMode:(BOOL)bToggleToCurrentScreenMode
{
	VCLToggleFullScreen *pRet = [[VCLToggleFullScreen alloc] initToggleFullScreen:pWindow toggleToCurrentScreenMode:bToggleToCurrentScreenMode];
	[pRet autorelease];
	return pRet;
}

- (id)initToggleFullScreen:(NSWindow *)pWindow toggleToCurrentScreenMode:(BOOL)bToggleToCurrentScreenMode
{
	[super init];

	mbToggleToCurrentScreenMode = bToggleToCurrentScreenMode;
	mpWindow = pWindow;
	if ( mpWindow )
		[mpWindow retain];

	return self;
}

- (void)dealloc
{
	if ( mpWindow )
		[mpWindow release];

	[super dealloc];
}

- (void)toggleFullScreen:(id)pObject
{
	(void)pObject;

	if ( mpWindow )
	{
		BOOL bToggle = !mbToggleToCurrentScreenMode;
		if ( !bToggle )
		{
			NSApplication *pApp = [NSApplication sharedApplication];
			if ( pApp )
			{
				BOOL bAppInFullScreen = ( [pApp presentationOptions] & NSApplicationPresentationFullScreen ? YES : NO );
				BOOL bWindowInFullScreen = ( [mpWindow styleMask] & NSWindowStyleMaskFullScreen ? YES : NO );
				if ( bAppInFullScreen != bWindowInFullScreen )
					bToggle = YES;
			}
		}

		if ( bToggle )
			[mpWindow toggleFullScreen:self];
	}
}

@end

@interface VCLViewGetGraphicsLayer : NSObject
{
	JavaSalGraphics*		mpGraphics;
	CGLayerRef				maLayer;
	NSView*					mpView;
}
+ (id)createGraphicsLayer:(JavaSalGraphics *)pGraphics view:(NSView *)pView;
- (id)initGraphicsLayer:(JavaSalGraphics *)pGraphics view:(NSView *)pView;
- (void)dealloc;
- (void)getGraphicsLayer:(id)pObject;
- (CGLayerRef)layer;
@end

@implementation VCLViewGetGraphicsLayer

+ (id)createGraphicsLayer:(JavaSalGraphics *)pGraphics view:(NSView *)pView
{
	VCLViewGetGraphicsLayer *pRet = [[VCLViewGetGraphicsLayer alloc] initGraphicsLayer:pGraphics view:pView];
	[pRet autorelease];
	return pRet;
}

- (id)initGraphicsLayer:(JavaSalGraphics *)pGraphics view:(NSView *)pView
{
	[super init];

	mpGraphics = pGraphics;
	maLayer = NULL;
	mpView = pView;
	if ( mpView )
		[mpView retain];

	return self;
}

- (void)dealloc
{
	if ( maLayer )
		CGLayerRelease( maLayer );

	if ( mpView )
		[mpView release];

	[super dealloc];
}

- (void)getGraphicsLayer:(id)pObject
{
	(void)pObject;

	if ( maLayer )
	{
		CGLayerRelease( maLayer );
		maLayer = NULL;
	}

	// Remove native window's entry
	NSWindow *pWindow = nil;
	if ( mpView )
	{
		pWindow = [mpView window];
		if ( pWindow )
		{
			::std::map< NSWindow*, JavaSalGraphics* >::iterator it = aNativeWindowMap.find( pWindow );
			if ( it != aNativeWindowMap.end() )
				aNativeWindowMap.erase( it );
		}
	}

	// Remove all entries for the graphics
	::std::map< NSWindow*, JavaSalGraphics* >::iterator it = aNativeWindowMap.begin();
	while ( it != aNativeWindowMap.end() )
	{
		if ( it->second == mpGraphics )
		{
			aNativeWindowMap.erase( it );
			it = aNativeWindowMap.begin();
			continue;
		}

		++it;
	}

	if ( mpGraphics && pWindow && [pWindow isVisible] )
	{
		NSRect aContentRect = [pWindow contentRectForFrameRect:[pWindow frame]];
		if ( aContentRect.size.width <= 1.0f )
			aContentRect.size.width = 1.0f;
		if ( aContentRect.size.height <= 1.0f )
			aContentRect.size.height = 1.0f;

		NSGraphicsContext *pContext = [pWindow graphicsContext];
		if ( pContext )
		{
			CGContextRef aContext = (CGContextRef)[pContext graphicsPort];
			if ( aContext )
			{
				maLayer = CGLayerCreateWithContext( aContext, CGSizeMake( aContentRect.size.width, aContentRect.size.height ), NULL );
				if ( maLayer )
					aNativeWindowMap[ pWindow ] = mpGraphics;
			}
		}
	}
}

- (CGLayerRef)layer
{
	return maLayer;
}

@end

@interface VCLWindowWrapperArgs : NSObject
{
	NSArray*				mpArgs;
	NSObject*				mpResult;
}
+ (id)argsWithArgs:(NSArray *)pArgs;
- (NSArray *)args;
- (void)dealloc;
- (id)initWithArgs:(NSArray *)pArgs;
- (NSObject *)result;
- (void)setResult:(NSObject *)pResult;
@end

@interface VCLWindowWrapper : NSObject
{
	JavaSalFrame*			mpFrame;
	BOOL					mbFullScreen;
	NSRect					maInsets;
	BOOL					mbInMouseEntered;
	BOOL					mbInMouseExited;
	BOOL					mbInMouseMoved;
	NSWindow*				mpParent;
	BOOL					mbShowOnlyMenus;
	NSRect					maShowOnlyMenusFrame;
	sal_uLong				mnStyle;
	BOOL					mbUndecorated;
	BOOL					mbUtility;
	NSProgressIndicator*	mpWaitingView;
	NSWindow*				mpWindow;
	NSUInteger				mnWindowStyleMask;
}
+ (void)updateShowOnlyMenusWindows;
- (void)addTrackingArea:(VCLWindowWrapperArgs *)pArgs;
- (void)adjustColorLevelAndShadow;
- (void)animateWaitingView:(BOOL)bAnimate;
- (id)initWithStyle:(sal_uLong)nStyle frame:(JavaSalFrame *)pFrame parent:(NSWindow *)pParent showOnlyMenus:(BOOL)bShowOnlyMenus utility:(BOOL)bUtility;
- (void)dealloc;
- (void)deminimize:(VCLWindowWrapperArgs *)pArgs;
- (void)destroy:(id)pObject;
- (void)flush:(id)pObject;
- (void)flushTimer:(id)pObject;
- (void)getContentView:(VCLWindowWrapperArgs *)pArgs;
- (void)getFrame:(VCLWindowWrapperArgs *)pArgs;
- (void)getState:(VCLWindowWrapperArgs *)pArgs;
- (const NSRect)insets;
- (BOOL)isFloatingWindow;
- (void)makeModal:(id)pObject;
- (void)mouseEntered:(NSEvent *)pEvent;
- (void)mouseExited:(NSEvent *)pEvent;
- (void)mouseMoved:(NSEvent *)pEvent;
- (void)removeTrackingArea:(VCLWindowWrapperArgs *)pArgs;
- (void)requestFocus:(VCLWindowWrapperArgs *)pArgs;
- (void)setContentMinSize:(NSSize)aContentMinSize;
- (void)setJavaFrame:(VCLWindowWrapperArgs *)pArgs;
- (void)setFullScreenMode:(VCLWindowWrapperArgs *)pArgs;
- (void)setMinSize:(VCLWindowWrapperArgs *)pArgs;
- (void)setMovable:(VCLWindowWrapperArgs *)pArgs;
- (void)setState:(VCLWindowWrapperArgs *)pArgs;
- (void)setTitle:(VCLWindowWrapperArgs *)pArgs;
- (void)setVisible:(VCLWindowWrapperArgs *)pArgs;
- (void)toFront:(VCLWindowWrapperArgs *)pArgs;
- (NSWindow *)window;
@end

static ::std::map< PointerStyle, NSCursor* > aVCLCustomCursors;

@interface VCLSetCursor : NSObject
{
	PointerStyle			mePointerStyle;
	VCLWindowWrapper*		mpWindowWrapper;
}
+ (id)createWithPointerStyle:(PointerStyle)ePointerStyle windowWrapper:(VCLWindowWrapper *)pWindowWrapper;
+ (void)loadCustomCursorWithPointerStyle:(PointerStyle)ePointerStyle hotSpot:(NSPoint)aHotSpot path:(NSString *)pPath;
- (void)dealloc;
- (id)initWithPointerStyle:(PointerStyle)ePointerStyle windowWrapper:(VCLWindowWrapper *)pWindowWrapper;
- (void)setCursor:(id)pObject;
@end

@implementation VCLSetCursor

+ (id)createWithPointerStyle:(PointerStyle)ePointerStyle windowWrapper:(VCLWindowWrapper *)pWindowWrapper
{
	VCLSetCursor *pRet = [[VCLSetCursor alloc] initWithPointerStyle:ePointerStyle windowWrapper:pWindowWrapper];
	[pRet autorelease];
	return pRet;
}

+ (void)loadCustomCursorWithPointerStyle:(PointerStyle)ePointerStyle hotSpot:(NSPoint)aHotSpot path:(NSString *)pPath
{
	if ( !pPath )
		return;

	::std::map< PointerStyle, NSCursor* >::const_iterator it = aVCLCustomCursors.find( ePointerStyle );
	if ( it != aVCLCustomCursors.end() )
		return;

	NSImage *pImage = [[NSImage alloc] initWithContentsOfFile:pPath];
	if ( pImage )
	{
		[pImage autorelease];

		if ( [pImage isValid] )
		{
			NSCursor *pCursor = [[NSCursor alloc] initWithImage:pImage hotSpot:aHotSpot];
			if ( pCursor )
				aVCLCustomCursors[ ePointerStyle ] = pCursor;
		}
	}
}

- (void)dealloc
{
	if ( mpWindowWrapper )
		[mpWindowWrapper release];

	[super dealloc];
}

- (id)initWithPointerStyle:(PointerStyle)ePointerStyle windowWrapper:(VCLWindowWrapper *)pWindowWrapper
{
	[super init];

	mePointerStyle = ePointerStyle;
	mpWindowWrapper = pWindowWrapper;
	if ( mpWindowWrapper )
		[mpWindowWrapper retain];
 
	return self;
}

- (void)setCursor:(id)pObject
{
	(void)pObject;

	if ( !mpWindowWrapper )
		return;

	NSWindow *pWindow = [mpWindowWrapper window];
	if ( !pWindow || ![pWindow isVisible] )
		return;

	NSView *pContentView = [pWindow contentView];
	if ( !pContentView )
		return;

	// Populate cached cursors
	if ( !aVCLCustomCursors.size() )
	{
		NSBundle *pBundle = [NSBundle mainBundle];
		if ( pBundle )
		{
			NSString *pPath = [pBundle bundlePath];
			if ( pPath )
			{
				pPath = [[[pPath stringByAppendingPathComponent:@"Contents"] stringByAppendingPathComponent:@"Resources"] stringByAppendingPathComponent:@"cursors"];
				if ( pPath )
				{
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AIRBRUSH hotSpot:NSMakePoint( 2, 31 ) path:[pPath stringByAppendingPathComponent:@"airbrush.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_E hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"ase.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_N hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"asn.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_NE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"asne.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_NS hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"asns.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_NSWE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"asnswe"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_NW hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"asnw.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_S hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"ass.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_SE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"asse.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_SW hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"assw.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_W hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"asw.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_WE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"aswe.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_CHAIN hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"chain.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_CHAIN_NOTALLOWED hotSpot:NSMakePoint( 10, 10 ) path:[pPath stringByAppendingPathComponent:@"chainnot.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_CHART hotSpot:NSMakePoint( 11, 11 ) path:[pPath stringByAppendingPathComponent:@"chart.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_COPYDATA hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"copydata.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_COPYDATALINK hotSpot:NSMakePoint( 10, 2 ) path:[pPath stringByAppendingPathComponent:@"copydlnk.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_COPYFILE hotSpot:NSMakePoint( 5, 10 ) path:[pPath stringByAppendingPathComponent:@"copyf.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_COPYFILES hotSpot:NSMakePoint( 9, 10 ) path:[pPath stringByAppendingPathComponent:@"copyf2.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_COPYFILELINK hotSpot:NSMakePoint( 9, 7 ) path:[pPath stringByAppendingPathComponent:@"copyflnk.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_CROOK hotSpot:NSMakePoint( 16, 17 ) path:[pPath stringByAppendingPathComponent:@"crook.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_CROP hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"crop.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_CROSS hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"cross.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_ARC hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"darc.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_BEZIER hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dbezier.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_CAPTION hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dcapt.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_CIRCLECUT hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dcirccut.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_CONNECT hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dconnect.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_ELLIPSE hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dellipse.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DETECTIVE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"detectiv.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_FREEHAND hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dfree.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_LINE hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dline.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_PIE hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dpie.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_POLYGON hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dpolygon.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_RECT hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"drect.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_TEXT hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dtext.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_FILL hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"fill.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_HAND hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"hand.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_HELP hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"help.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_HSHEAR hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"hshear.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_HSIZEBAR hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"hsizebar.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_HSPLIT hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"hsplit.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_LINKDATA hotSpot:NSMakePoint( 10, 2 ) path:[pPath stringByAppendingPathComponent:@"linkdata.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_LINKFILE hotSpot:NSMakePoint( 9, 7 ) path:[pPath stringByAppendingPathComponent:@"linkf.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MAGNIFY hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"magnify.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MIRROR hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"mirror.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"move.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVEBEZIERWEIGHT hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"movebw.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVEDATA hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"movedata.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVEDATALINK hotSpot:NSMakePoint( 3, 3 ) path:[pPath stringByAppendingPathComponent:@"movedlnk.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVEFILE hotSpot:NSMakePoint( 5, 10 ) path:[pPath stringByAppendingPathComponent:@"movef.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVEFILES hotSpot:NSMakePoint( 9, 10 ) path:[pPath stringByAppendingPathComponent:@"movef2.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVEFILELINK hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"moveflnk.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVEPOINT hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"movept.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_NESIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"neswsize.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_SWSIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"neswsize.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_WINDOW_NESIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"neswsize.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_WINDOW_SWSIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"neswsize.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_NOTALLOWED hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"notallow.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_NULL hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"nullptr.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_NWSIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"nwsesize.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_SESIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"nwsesize.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_WINDOW_NWSIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"nwsesize.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_WINDOW_SESIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"nwsesize.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_PEN hotSpot:NSMakePoint( 2, 31 ) path:[pPath stringByAppendingPathComponent:@"pen.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_PIVOT_COL hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"pivotcol.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_PIVOT_DELETE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"pivotdel.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_PIVOT_FIELD hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"pivotfld.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_PIVOT_ROW hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"pivotrow.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_PAINTBRUSH hotSpot:NSMakePoint( 4, 30 ) path:[pPath stringByAppendingPathComponent:@"pntbrsh.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_REFHAND hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"refhand.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_ROTATE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"rotate.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_TAB_SELECT_E hotSpot:NSMakePoint( 31, 16 ) path:[pPath stringByAppendingPathComponent:@"tblsele.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_TAB_SELECT_S hotSpot:NSMakePoint( 16, 31 ) path:[pPath stringByAppendingPathComponent:@"tblsels.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_TAB_SELECT_SE hotSpot:NSMakePoint( 31, 31 ) path:[pPath stringByAppendingPathComponent:@"tblselse.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_TAB_SELECT_SW hotSpot:NSMakePoint( 2, 31 ) path:[pPath stringByAppendingPathComponent:@"tblselsw.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_TAB_SELECT_W hotSpot:NSMakePoint( 2, 16 ) path:[pPath stringByAppendingPathComponent:@"tblselw.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_TIMEEVENT_MOVE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"timemove.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_TIMEEVENT_SIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"timesize.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_VSHEAR hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"vshear.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_VSIZEBAR hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"vsizebar.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_VSPLIT hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"vsplit.png"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_TEXT_VERTICAL hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"vtext.png"]];
				}
			}
		}
	}

	BOOL bAnimateWaitingView = NO;
	NSCursor *pCursor = nil;
	::std::map< PointerStyle, NSCursor* >::const_iterator it = aVCLCustomCursors.find( mePointerStyle );
	if ( it != aVCLCustomCursors.end() )
		pCursor = it->second;

	if ( !pCursor )
	{
		switch ( mePointerStyle )
		{
			case POINTER_NULL:
			case POINTER_TEXT:
				pCursor = [NSCursor IBeamCursor];
				break;
			case POINTER_CROSS:
			case POINTER_DRAW_LINE:
			case POINTER_DRAW_RECT:
			case POINTER_DRAW_POLYGON:
			case POINTER_DRAW_BEZIER:
			case POINTER_DRAW_ARC:
			case POINTER_DRAW_PIE:
			case POINTER_DRAW_CIRCLECUT:
			case POINTER_DRAW_ELLIPSE:
			case POINTER_DRAW_FREEHAND:
			case POINTER_DRAW_CONNECT:
			case POINTER_DRAW_TEXT:
			case POINTER_DRAW_CAPTION:
				pCursor = [NSCursor crosshairCursor];
				break;
			case POINTER_MOVE:
				pCursor = [NSCursor openHandCursor];
				break;
			case POINTER_NOTALLOWED:
				pCursor = [NSCursor operationNotAllowedCursor];
				break;
			case POINTER_NSIZE:
			case POINTER_WINDOW_NSIZE:
			case POINTER_SSIZE:
			case POINTER_WINDOW_SSIZE:
			case POINTER_VSPLIT:
			case POINTER_VSIZEBAR:
				pCursor = [NSCursor resizeUpDownCursor];
				break;
			case POINTER_WSIZE:
			case POINTER_WINDOW_WSIZE:
			case POINTER_ESIZE:
			case POINTER_WINDOW_ESIZE:
			case POINTER_HSPLIT:
			case POINTER_HSIZEBAR:
				pCursor = [NSCursor resizeLeftRightCursor];
				break;
			case POINTER_HAND:
			case POINTER_REFHAND:
				pCursor = [NSCursor pointingHandCursor];
				break;
			case POINTER_TEXT_VERTICAL:
				pCursor = [NSCursor IBeamCursorForVerticalLayout];
				break;
			case POINTER_WAIT:
				// Animate waiting view if available
				bAnimateWaitingView = YES;
				pCursor = [NSCursor arrowCursor];
				break;
			case POINTER_ARROW:
			case POINTER_HELP:
			case POINTER_PEN:
			case POINTER_MAGNIFY:
			case POINTER_FILL:
			case POINTER_ROTATE:
			case POINTER_HSHEAR:
			case POINTER_VSHEAR:
			case POINTER_MIRROR:
			case POINTER_CROOK:
			case POINTER_CROP:
			case POINTER_MOVEPOINT:
			case POINTER_MOVEBEZIERWEIGHT:
			case POINTER_MOVEDATA:
			case POINTER_COPYDATA:
			case POINTER_LINKDATA:
			case POINTER_MOVEDATALINK:
			case POINTER_COPYDATALINK:
			case POINTER_MOVEFILE:
			case POINTER_COPYFILE:
			case POINTER_LINKFILE:
			case POINTER_MOVEFILELINK:
			case POINTER_COPYFILELINK:
			case POINTER_MOVEFILES:
			case POINTER_COPYFILES:
			case POINTER_CHART:
			case POINTER_DETECTIVE:
			case POINTER_PIVOT_COL:
			case POINTER_PIVOT_ROW:
			case POINTER_PIVOT_FIELD:
			case POINTER_CHAIN:
			case POINTER_CHAIN_NOTALLOWED:
			case POINTER_TIMEEVENT_MOVE:
			case POINTER_TIMEEVENT_SIZE:
			case POINTER_AUTOSCROLL_N:
			case POINTER_AUTOSCROLL_S:
			case POINTER_AUTOSCROLL_W:
			case POINTER_AUTOSCROLL_E:
			case POINTER_AUTOSCROLL_NW:
			case POINTER_AUTOSCROLL_NE:
			case POINTER_AUTOSCROLL_SW:
			case POINTER_AUTOSCROLL_SE:
			case POINTER_AUTOSCROLL_NS:
			case POINTER_AUTOSCROLL_WE:
			case POINTER_AUTOSCROLL_NSWE:
			case POINTER_AIRBRUSH:
			case POINTER_PIVOT_DELETE:
			case POINTER_TAB_SELECT_S:
			case POINTER_TAB_SELECT_E:
			case POINTER_TAB_SELECT_SE:
			case POINTER_TAB_SELECT_W:
			case POINTER_TAB_SELECT_SW:
			case POINTER_NWSIZE:
			case POINTER_WINDOW_NWSIZE:
			case POINTER_NESIZE:
			case POINTER_WINDOW_NESIZE:
			case POINTER_SWSIZE:
			case POINTER_WINDOW_SWSIZE:
			case POINTER_SESIZE:
			case POINTER_WINDOW_SESIZE:
			default:
				pCursor = [NSCursor arrowCursor];
				break;
		}
	}

	[mpWindowWrapper animateWaitingView:bAnimateWaitingView];

	if ( pCursor )
	{
		::std::map< NSWindow*, NSCursor* >::iterator cit = aNativeCursorMap.find( pWindow );
		if ( cit != aNativeCursorMap.end() )
			[cit->second release];

		[pCursor retain];
		aNativeCursorMap[ pWindow ] = pCursor;
		[pWindow invalidateCursorRectsForView:pContentView];
	}
}

@end

@interface VCLSetNeedsDisplayAllViews : NSObject
{
}
+ (id)create;
- (id)init;
- (void)setNeedsDisplay:(id)pObject;
@end

@implementation VCLSetNeedsDisplayAllViews

+ (id)create
{
	VCLSetNeedsDisplayAllViews *pRet = [[VCLSetNeedsDisplayAllViews alloc] init];
	[pRet autorelease];
	return pRet;
}

- (id)init
{
	[super init];
 
	return self;
}

- (void)setNeedsDisplay:(id)pObject
{
	(void)pObject;

	for ( ::std::map< NSWindow*, JavaSalGraphics* >::const_iterator it = aNativeWindowMap.begin(); it != aNativeWindowMap.end(); ++it )
	{
		NSView *pContentView = [it->first contentView];
		if ( pContentView )
			it->second->setNeedsDisplay( pContentView );
	}

	// After using the Window menu to set focus to a non-full screen window and
	// after closing that window sets the focus to a full screen window,
	// closing the full screen focus will cause OS X to exit full screen mode
	// even if focus is set to another full screen window. To fix this bug,
	// invoke [NSWindow makeKeyAndOrderFront:] on the key window to ensure that
	// the application's and window's full screen modes are in sync.
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSWindow *pKeyWindow = [pApp keyWindow];
		if ( pKeyWindow && [pKeyWindow isVisible] && [pKeyWindow collectionBehavior] & NSWindowCollectionBehaviorFullScreenPrimary )
		{
			BOOL bAppInFullScreen = ( [pApp presentationOptions] & NSApplicationPresentationFullScreen ? YES : NO );
			BOOL bWindowInFullScreen = ( [pKeyWindow styleMask] & NSWindowStyleMaskFullScreen ? YES : NO );
			if ( bAppInFullScreen || bWindowInFullScreen )
			{
				// If there is a non-full screen window that can obtain focus,
				// do not set the focus. Otherwise, if there are full screen
				// windows with a modal dialog child window, focus can never be
				// set the modal dialog.
				NSArray *pWindows = [pApp orderedWindows];
				if ( pWindows )
				{
					NSUInteger nCount = [pWindows count];
					NSUInteger i = 0;
					for ( ; i < nCount; i++ )
					{
						NSWindow *pWindow = [pWindows objectAtIndex:i];
						if ( pWindow && [pWindow isVisible] && ! ( [pWindow styleMask] & NSWindowStyleMaskFullScreen ) && [pWindow canBecomeKeyWindow] )
							return;
					}
				}

				[pKeyWindow makeKeyAndOrderFront:pKeyWindow];
			}
		}
	}
}

@end

@interface VCLUpdateScreens : NSObject
{
}
+ (id)create;
- (id)init;
- (void)screensChanged:(NSNotification *)pNotification;
- (void)updateScreens:(id)pObject;
@end

static VCLUpdateScreens *pVCLUpdateScreens = nil;

@implementation VCLUpdateScreens

+ (id)create
{
	VCLUpdateScreens *pRet = [[VCLUpdateScreens alloc] init];
	[pRet autorelease];
	return pRet;
}

- (id)init
{
	[super init];
 
	return self;
}

- (void)screensChanged:(NSNotification *)pNotification
{
	(void)pNotification;

	HandleScreensChangedRequest();
}

- (void)updateScreens:(id)pObject
{
	(void)pObject;

	if ( !pVCLUpdateScreens )
	{
		NSNotificationCenter *pNotificationCenter = [NSNotificationCenter defaultCenter];
		if ( pNotificationCenter )
		{
			pVCLUpdateScreens = self;
			[pVCLUpdateScreens retain];
			[pNotificationCenter addObserver:pVCLUpdateScreens selector:@selector(screensChanged:) name:NSApplicationDidChangeScreenParametersNotification object:nil];
		}
	}

	HandleScreensChangedRequest();
}

@end

@interface VCLUpdateSystemColors : NSObject
{
	BOOL					mbInStartHandler;
}
+ (id)create;
- (id)init;
- (void)systemColorsChanged:(NSNotification *)pNotification;
- (void)updateSystemColors:(id)pObject;
@end

static VCLUpdateSystemColors *pVCLUpdateSystemColors = nil;

@implementation VCLUpdateSystemColors

+ (id)create
{
	VCLUpdateSystemColors *pRet = [[VCLUpdateSystemColors alloc] init];
	[pRet autorelease];
	return pRet;
}

- (id)init
{
	[super init];
 
	mbInStartHandler = NO;

	return self;
}

- (void)systemColorsChanged:(NSNotification *)pNotification
{
	(void)pNotification;

	HandleSystemColorsChangedRequest();

	JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_SYSTEMCOLORSCHANGED, NULL, NULL );
	JavaSalEventQueue::postCachedEvent( pEvent );
	pEvent->release();
}

- (void)updateSystemColors:(id)pObject
{
	(void)pObject;

	if ( !pVCLUpdateSystemColors )
	{
		NSNotificationCenter *pNotificationCenter = [NSNotificationCenter defaultCenter];
		if ( pNotificationCenter )
		{
			mbInStartHandler = YES;
			pVCLUpdateSystemColors = self;
			[pVCLUpdateSystemColors retain];
			[pNotificationCenter addObserver:pVCLUpdateSystemColors selector:@selector(systemColorsChanged:) name:NSSystemColorsDidChangeNotification object:nil];
			mbInStartHandler = NO;
		}
	}

	HandleSystemColorsChangedRequest();
}

@end

@implementation VCLWindowWrapperArgs

+ (id)argsWithArgs:(NSArray *)pArgs
{
	VCLWindowWrapperArgs *pRet = [[VCLWindowWrapperArgs alloc] initWithArgs:(NSArray *)pArgs];
	[pRet autorelease];
	return pRet;
}

- (NSArray *)args
{
	return mpArgs;
}

- (void)dealloc
{
	if ( mpArgs )
		[mpArgs release];

	if ( mpResult )
		[mpResult release];

	[super dealloc];
}

- (id)initWithArgs:(NSArray *)pArgs
{
	[super init];

	mpResult = nil;
	mpArgs = pArgs;
	if ( mpArgs )
		[mpArgs retain];

	return self;
}

- (NSObject *)result
{
	return mpResult;
}

- (void)setResult:(NSObject *)pResult
{
	if ( mpResult )
		[mpResult release];

	mpResult = pResult;

	if ( mpResult )
		[mpResult retain];
}

@end

static void CloseOrOrderOutWindow( NSWindow *pWindow )
{
	// Close, not order out the window because when the window is in full
	// screen mode, ordering out will leave the application in an empty full
	// screen mode state
	if ( pWindow )
	{
		NSWindow *pLastTabbedWindow = nil;
		NSArray<NSWindow*> *pTabbedWindows = pWindow.tabbedWindows;
		if ( pTabbedWindows )
		{
			NSUInteger nCount = [pTabbedWindows count];
			if ( nCount <= 2 )
			{
				NSUInteger i = 0;
				for ( ; i < nCount; i++ )
				{
					NSWindow *pTabbedWindow = [pTabbedWindows objectAtIndex:i];
					if ( pTabbedWindow && pTabbedWindow != pWindow )
					{
						pLastTabbedWindow = pTabbedWindow;
						[pLastTabbedWindow retain];
					}

				}
			}
		}
		
		[pWindow close];

		// Fix missized window contents when tabbed windows are closed using
		// the Close Other Tabs menu item in a tabs popover menu by calling
		// the windowDidResize: selector on the last remaining tabbed window
		if ( pLastTabbedWindow )
		{
			id<NSWindowDelegate> pDelegate = [pLastTabbedWindow delegate];
			if ( pDelegate && [pDelegate respondsToSelector:@selector(windowDidResize:)] )
				[pDelegate windowDidResize:[NSNotification notificationWithName:NSWindowDidResizeNotification object:pLastTabbedWindow]];
		}
	}
}

static ::std::map< NSWindow*, VCLWindow* > aShowOnlyMenusWindowMap;

@implementation VCLWindowWrapper

+ (void)updateShowOnlyMenusWindows
{
	// Fix bug 3032 by disabling focus for show only menus windows when
	// any frames are visible
	BOOL bShow = YES;
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSArray *pWindows = [pApp windows];
		if ( pWindows )
		{
			NSUInteger nCount = [pWindows count];
			NSUInteger i = 0;
			for ( ; i < nCount; i++ )
			{
				// Fix bug reported in the following NeoOffice forum post by
				// only letting the subset of windows that are VCLWindows stop
				// the show only menus windows from showing:
				// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=63338#63338
				NSWindow *pWindow = [pWindows objectAtIndex:i];
				if ( pWindow && [pWindow isKindOfClass:[VCLWindow class]] && ( [pWindow isVisible] || [pWindow isMiniaturized] ) )
				{
					::std::map< NSWindow*, VCLWindow* >::const_iterator it = aShowOnlyMenusWindowMap.find( pWindow );
					if ( it == aShowOnlyMenusWindowMap.end() )
					{
						bShow = NO;
						break;
					}
				}
			}
		}
	}

	for ( ::std::map< NSWindow*, VCLWindow* >::const_iterator it = aShowOnlyMenusWindowMap.begin(); it != aShowOnlyMenusWindowMap.end(); ++it )
	{
		// Fix bug reported in the following NeoOffice forum topic when
		// clicking on the application's Dock icon when all document windows
		// are minimized by hiding all show only menus windows when focus is
		// disabled:
		// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=63252#63252
		if ( bShow )
			[it->first makeKeyAndOrderFront:it->first];
		else
			[it->first orderOut:it->first];
	}

	// Fix bug reported in the following NeoOffice forum post by ensuring that
	// if no window has focus (usually when all document windows are
	// minimized and a visible window has just closed), we force one of the
	// visible windows to have focus or, if there are only minimized windows,
	// dispatch a get focus event to fill the menubar for one of the minimized
	// windows:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=63282#63282
	// Fix bug reported in the following NeoOffice forum topic by only forcing
	// a focus change when the application is active:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8556
	if ( !bShow && pApp && [pApp isActive] && ![pApp keyWindow] )
	{
		NSArray *pWindows = [pApp orderedWindows];
		if ( pWindows )
		{
			NSWindow *pVisibleWindow = nil;
			JavaSalFrame *pMinitiarizedFrame = NULL;
			NSUInteger nCount = [pWindows count];
			NSUInteger i = 0;
			for ( ; i < nCount; i++ )
			{
				NSWindow *pWindow = [pWindows objectAtIndex:i];
				if ( pWindow && ![pWindow parentWindow] && ! ( [pWindow styleMask] & NSWindowStyleMaskUtilityWindow ) && ( [pWindow isVisible] || [pWindow isMiniaturized] ) )
				{
					::std::map< NSWindow*, VCLWindow* >::const_iterator it = aShowOnlyMenusWindowMap.find( pWindow );
					if ( it == aShowOnlyMenusWindowMap.end() )
					{
						if ( [pWindow isVisible] && [pWindow canBecomeKeyWindow] )
						{
							if ( !pVisibleWindow )
							{
								pVisibleWindow = pWindow;
							}
							else if ( ! ( [pVisibleWindow styleMask] & NSWindowStyleMaskFullScreen ) && [pWindow styleMask] & NSWindowStyleMaskFullScreen )
							{
								pVisibleWindow = pWindow;
								break;
							}
						}
						else if ( !pMinitiarizedFrame && [pWindow isMiniaturized] )
						{
							::std::map< NSWindow*, JavaSalGraphics* >::iterator nwit = aNativeWindowMap.find( pWindow );
							if ( nwit != aNativeWindowMap.end() )
								pMinitiarizedFrame = nwit->second->mpFrame;
						}
					}
				}
			}

			if ( pVisibleWindow )
			{
				[pVisibleWindow makeKeyAndOrderFront:pVisibleWindow];
			}
			else if ( pMinitiarizedFrame )
			{
				JavaSalEvent *pGetFocusEvent = new JavaSalEvent( SALEVENT_GETFOCUS, pMinitiarizedFrame, NULL );
				JavaSalEventQueue::postCachedEvent( pGetFocusEvent );
				pGetFocusEvent->release();
			}
		}
	}
}

- (void)addTrackingArea:(VCLWindowWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

	NSValue *pWindow = (NSValue *)[pArgArray objectAtIndex:0];
	if ( !pWindow || ![pWindow pointerValue] )
		return;

	NSValue *pRect = (NSValue *)[pArgArray objectAtIndex:1];
	if ( !pRect || NSIsEmptyRect( [pRect rectValue] ) )
		return;

	// If any tracking area is already set for this pointer, remove it
	[self removeTrackingArea:pArgs];

	if ( mpWindow )
	{
		NSView *pContentView = [mpWindow contentView];
		if ( pContentView )
		{
			NSDictionary *pDict = [NSDictionary dictionaryWithObject:pWindow forKey:pVCLTrackingAreaWindowKey];
			if ( pDict )
			{
				NSTrackingArea *pTrackingArea = [[NSTrackingArea alloc] initWithRect:[pRect rectValue] options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveAlways owner:self userInfo:pDict];
				if ( pTrackingArea )
				{
					[pTrackingArea autorelease];
					[pContentView addTrackingArea:pTrackingArea];
				}
			}
		}
	}
}

- (void)animateWaitingView:(BOOL)bAnimate
{
	if ( mpWaitingView && mpWindow )
	{
		if ( bAnimate && [mpWindow isVisible] )
		{
			NSView *pContentView = [mpWindow contentView];
			if ( pContentView )
			{
				// Center in content view
				NSRect aContentBounds = [pContentView bounds];
				NSRect aWaitingFrame = [mpWaitingView frame];
				[mpWaitingView setFrameOrigin:NSMakePoint( ( aContentBounds.size.width - aWaitingFrame.size.width ) / 2, ( aContentBounds.size.height - aWaitingFrame.size.height ) / 2 )];
			}

			// The wait spinner is very slow so don't display it when the
			// spinner will only appear for a very short amount of time
			[mpWaitingView performSelector:@selector(startAnimation:) withObject:self afterDelay:0.5f];
		}
		else
		{
			[NSObject cancelPreviousPerformRequestsWithTarget:mpWaitingView selector:@selector(startAnimation:) object:self];
			[mpWaitingView stopAnimation:self];

		}
	}
}

- (void)adjustColorLevelAndShadow
{
	if ( mpWindow )
	{
		if ( mbFullScreen )
			[mpWindow setBackgroundColor:[NSColor blackColor]];
		else
			[mpWindow setBackgroundColor:[NSColor whiteColor]];

		if ( mbUtility )
		{
			[mpWindow setHasShadow:YES];
			[(VCLPanel *)mpWindow setFloatingPanel:YES];
		}
		else if ( mbFullScreen )
		{
			[mpWindow setHasShadow:NO];
			[mpWindow setLevel:NSNormalWindowLevel];
		}
		else if ( mbShowOnlyMenus )
		{
			[mpWindow setHasShadow:NO];
			[mpWindow setLevel:NSPopUpMenuWindowLevel];
		}
		else if ( mbUndecorated && mpParent )
		{
			[mpWindow setHasShadow:YES];
			[mpWindow setLevel:NSPopUpMenuWindowLevel];
		}
		else
		{
			[mpWindow setHasShadow:YES];
			[mpWindow setLevel:NSNormalWindowLevel];
		}
	}
}

- (id)initWithStyle:(sal_uLong)nStyle frame:(JavaSalFrame *)pFrame parent:(NSWindow *)pParent showOnlyMenus:(BOOL)bShowOnlyMenus utility:(BOOL)bUtility
{
	[super init];

	mpFrame = pFrame;
	mbFullScreen = NO;
	maInsets = NSMakeRect( 0, 0, 0, 0 );
	mbInMouseEntered = NO;
	mbInMouseExited = NO;
	mbInMouseMoved = NO;
	mpParent = pParent;
	if ( mpParent )
		[mpParent retain];
	mbShowOnlyMenus = bShowOnlyMenus;
	maShowOnlyMenusFrame = NSMakeRect( 0, 0, 1, 1 );
	mnStyle = nStyle;
	mbUtility = bUtility;
	mbUndecorated = NO;
	mpWaitingView = nil;
	mpWindow = nil;
	mnWindowStyleMask = NSWindowStyleMaskBorderless;

	if ( !mbUtility && ( mbShowOnlyMenus || ! ( mnStyle & ( SAL_FRAME_STYLE_DEFAULT | SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE ) ) ) )
		mbUndecorated = YES;

	if ( !mbUndecorated )
	{
		mnWindowStyleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable;
		if ( mbUtility )
			mnWindowStyleMask |= NSWindowStyleMaskUtilityWindow;
		if ( mnStyle & SAL_FRAME_STYLE_SIZEABLE )
		{
			mnWindowStyleMask |= NSWindowStyleMaskResizable;
			if ( !mbUtility )
				mnWindowStyleMask |= NSWindowStyleMaskMiniaturizable;
		}
	}

	VCLView *pContentView = [[VCLView alloc] initWithFrame:NSMakeRect( 0, 0, 1, 1 )];
	if ( pContentView )
	{
		[pContentView autorelease];

		if ( mbUtility || ( mbUndecorated && !mbShowOnlyMenus && !mbFullScreen )  || ( !mbUndecorated && mpParent ) )
			mpWindow = [[VCLPanel alloc] initWithContentRect:NSMakeRect( 0, 0, 1, 1 ) styleMask:mnWindowStyleMask backing:NSBackingStoreBuffered defer:YES];
		else
			mpWindow = [[VCLWindow alloc] initWithContentRect:NSMakeRect( 0, 0, 1, 1 ) styleMask:mnWindowStyleMask backing:NSBackingStoreBuffered defer:YES];

		if ( mpWindow )
		{
			[mpWindow setHidesOnDeactivate:NO];
			[mpWindow setContentView:pContentView];

			if ( mbUndecorated && !mbShowOnlyMenus && !mbFullScreen )
			{
				[(VCLPanel *)mpWindow setBecomesKeyOnlyIfNeeded:NO];
				[(VCLPanel *)mpWindow setCanBecomeKeyWindow:mnStyle & SAL_FRAME_STYLE_FLOAT_FOCUSABLE ? YES : NO];
			}
			else if ( mbUtility )
			{
				[mpWindow setHidesOnDeactivate:YES];
			}

			if ( [mpWindow isKindOfClass:[VCLPanel class]] )
				[(VCLPanel *)mpWindow setJavaFrame:mpFrame];
			else
				[(VCLWindow *)mpWindow setJavaFrame:mpFrame];

			// Cache the window's insets
			NSRect aContentRect = NSMakeRect( 0, 0, 1, 1 );
			NSRect aFrameRect = [NSWindow frameRectForContentRect:aContentRect styleMask:mnWindowStyleMask];
			maInsets = NSMakeRect( aContentRect.origin.x - aFrameRect.origin.x, aContentRect.origin.y - aFrameRect.origin.y, aFrameRect.origin.x + aFrameRect.size.width - aContentRect.origin.x - aContentRect.size.width, aFrameRect.origin.y + aFrameRect.size.height - aContentRect.origin.y - aContentRect.size.height );

			[self adjustColorLevelAndShadow];
			[self setContentMinSize:NSMakeSize( 1, 1 )];

			if ( mbShowOnlyMenus )
				aShowOnlyMenusWindowMap[ mpWindow ] = (VCLWindow *)mpWindow;

			if ( !mbUndecorated || mbFullScreen )
			{
				// Attach waiting view before window is visible because adding
				// and removing NSProgressIndicator instances to visble windows
				// can cause high CPU usage and hanging for several seconds on
				// OS X El Capitan
				mpWaitingView = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect( 0, 0, 1, 1 )];
				if ( mpWaitingView )
				{
					[mpWaitingView setIndeterminate:YES];
					[mpWaitingView setStyle:NSProgressIndicatorSpinningStyle];
					[mpWaitingView setDisplayedWhenStopped:NO];
					[mpWaitingView sizeToFit];
					[mpWaitingView setAutoresizingMask:NSViewMinXMargin | NSViewMaxXMargin | NSViewMinYMargin | NSViewMaxYMargin];

					[pContentView addSubview:mpWaitingView];
				}
			}

		}
	}

	return self;
}

- (void)deminimize:(VCLWindowWrapperArgs *)pArgs
{
	if ( mpWindow && [mpWindow isMiniaturized] && ![self isFloatingWindow] )
	{
		[mpWindow deminiaturize:self];
		if ( [mpWindow isVisible] )
			[pArgs setResult:[NSNumber numberWithBool:YES]];
	}
}

- (void)dealloc
{
	[self destroy:self];

	[super dealloc];
}

- (void)flush:(id)pObject
{
	(void)pObject;

	if ( mpWindow )
	{
		// Eliminate slow drawing on OS X 10.11 when the OOo code flushes
		// progress bars by doing the actual flushing in a native timer so that
		// OOo flushes can be coalesced
		::std::map< NSWindow*, NSTimer* >::const_iterator ftit = aNativeFlushTimerMap.find( mpWindow );
		if ( ftit == aNativeFlushTimerMap.end() )
		{
			NSTimer *pTimer = [NSTimer scheduledTimerWithTimeInterval:0.01f target:self selector:@selector(flushTimer:) userInfo:nil repeats:NO];
			if ( pTimer )
			{
				[pTimer retain];
				aNativeFlushTimerMap[ mpWindow ] = pTimer;
			}
		}
	}
}

- (void)flushTimer:(id)pObject
{
	(void)pObject;

	if ( mpWindow )
	{
		::std::map< NSWindow*, JavaSalGraphics* >::const_iterator nwit = aNativeWindowMap.find( mpWindow );
		if ( nwit != aNativeWindowMap.end() )
		{
			NSView *pContentView = [nwit->first contentView];
			if ( pContentView )
				nwit->second->setNeedsDisplay( pContentView );
		}

		::std::map< NSWindow*, NSTimer* >::iterator ftit = aNativeFlushTimerMap.find( mpWindow );
		if ( ftit != aNativeFlushTimerMap.end() )
		{
			[ftit->second invalidate];
			[ftit->second release];
			aNativeFlushTimerMap.erase( ftit );
		}
	}
}

- (void)destroy:(id)pObject
{
	(void)pObject;

	if ( mpParent )
	{
		[mpParent release];
		mpParent = nil;
	}

	if ( mpWindow )
	{
		NSWindow *pParentWindow = [mpWindow parentWindow];
		if ( pParentWindow )
			[pParentWindow removeChildWindow:mpWindow];

		CloseOrOrderOutWindow( mpWindow );

		::std::map< NSWindow*, JavaSalGraphics* >::iterator nwit = aNativeWindowMap.find( mpWindow );
		if ( nwit != aNativeWindowMap.end() )
			aNativeWindowMap.erase( nwit );

		::std::map< NSWindow*, NSCursor* >::iterator cit = aNativeCursorMap.find( mpWindow );
		if ( cit != aNativeCursorMap.end() )
		{
			[cit->second release];
			aNativeCursorMap.erase( cit );
		}

		::std::map< NSWindow*, NSTimer* >::iterator ftit = aNativeFlushTimerMap.find( mpWindow );
		if ( ftit != aNativeFlushTimerMap.end() )
		{
			[ftit->second invalidate];
			[ftit->second release];
			aNativeFlushTimerMap.erase( ftit );
		}

		::std::map< NSWindow*, VCLWindow* >::iterator somwit = aShowOnlyMenusWindowMap.find ( mpWindow );
		if ( somwit != aShowOnlyMenusWindowMap.end() )
			aShowOnlyMenusWindowMap.erase( somwit );

		[mpWindow release];
		mpWindow = nil;
	}

	if ( mpWaitingView )
	{
		[mpWaitingView release];
		mpWaitingView = nil;
	}
}

- (void)getContentView:(VCLWindowWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

    NSNumber *pTopLevelWindow = (NSNumber *)[pArgArray objectAtIndex:0];
    if ( !pTopLevelWindow )
        return;

	// Enable full screen feature for normal windows. Only enable this feature
	// if the window is not a panel and has a titlebar.
	if ( [pTopLevelWindow boolValue] && !mbUndecorated && mpWindow && [mpWindow isKindOfClass:[VCLWindow class]] )
		[mpWindow setCollectionBehavior:[mpWindow collectionBehavior] | NSWindowCollectionBehaviorFullScreenPrimary];
	else
		[mpWindow setCollectionBehavior:[mpWindow collectionBehavior] & ~NSWindowCollectionBehaviorFullScreenPrimary];

	// Only return content view if window is visible
	if ( mpWindow && [mpWindow isVisible] )
		[pArgs setResult:[mpWindow contentView]];
}

- (void)getFrame:(VCLWindowWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 3 )
		return;

    NSValue *pInLiveResize = (NSValue *)[pArgArray objectAtIndex:0];
    if ( !pInLiveResize )
        return;

    NSValue *pInFullScreenMode = (NSValue *)[pArgArray objectAtIndex:1];
    if ( !pInFullScreenMode )
        return;

    NSNumber *pFullScreen = (NSNumber *)[pArgArray objectAtIndex:2];
    if ( !pFullScreen )
        return;

	if ( mbShowOnlyMenus )
	{
		[pArgs setResult:[NSValue valueWithRect:maShowOnlyMenusFrame]];
	}
	else if ( mpWindow )
	{
		sal_Bool *pInLiveResizePointer = (sal_Bool *)[pInLiveResize pointerValue];
		if ( pInLiveResizePointer )
		{
			if ( [mpWindow inLiveResize] )
				*pInLiveResizePointer = YES;
			else
				*pInLiveResizePointer = NO;
		}

		sal_Bool *pInFullScreenModePointer = (sal_Bool *)[pInFullScreenMode pointerValue];
		NSRect aFrame = [mpWindow frame];

		// Check if we are in full screen mode
		if ( [mpWindow styleMask] & NSWindowStyleMaskFullScreen )
		{
			if ( pInFullScreenModePointer )
				*pInFullScreenModePointer = YES;

			// Reset insets to non-tabbed window insets
			NSRect aNonTabbedFrame = [NSWindow frameRectForContentRect:aFrame styleMask:[mpWindow styleMask] & ~NSWindowStyleMaskFullScreen];
			maInsets = NSMakeRect( aFrame.origin.x - aNonTabbedFrame.origin.x, aFrame.origin.y - aNonTabbedFrame.origin.y, aNonTabbedFrame.origin.x + aNonTabbedFrame.size.width - aFrame.origin.x - aFrame.size.width, aNonTabbedFrame.origin.y + aNonTabbedFrame.size.height - aFrame.origin.y - aFrame.size.height );

			if ( [pFullScreen boolValue] )
			{
				NSRect aNonFullScreenFrame;
				if ( [mpWindow isKindOfClass:[VCLPanel class]] )
					aNonFullScreenFrame = [(VCLPanel *)mpWindow nonFullScreenFrame];
				else
					aNonFullScreenFrame = [(VCLWindow *)mpWindow nonFullScreenFrame];
				if ( !NSIsEmptyRect( aNonFullScreenFrame ) )
					aFrame = aNonFullScreenFrame;
			}
			else
			{
				aFrame = aNonTabbedFrame;
			}
		}
		else
		{
			if ( pInFullScreenModePointer )
				*pInFullScreenModePointer = NO;

			// Update insets for non-full screen windows as tabbed windows have
			// a different inset than untabbed windows
			NSRect aContentRect = [mpWindow contentRectForFrameRect:aFrame];
			maInsets = NSMakeRect( aContentRect.origin.x - aFrame.origin.x, aContentRect.origin.y - aFrame.origin.y, aFrame.origin.x + aFrame.size.width - aContentRect.origin.x - aContentRect.size.width, aFrame.origin.y + aFrame.size.height - aContentRect.origin.y - aContentRect.size.height );
		}

		// Flip to OOo coordinates
		NSRect aTotalBounds = GetTotalScreenBounds();
		aFrame.origin.y = aTotalBounds.size.height - aFrame.origin.y - aFrame.size.height;

		[pArgs setResult:[NSValue valueWithRect:aFrame]];
	}
}

- (void)getState:(VCLWindowWrapperArgs *)pArgs
{
	if ( mpWindow )
	{
		unsigned long nState;
		if ( [mpWindow styleMask] & NSWindowStyleMaskMiniaturizable && [mpWindow isMiniaturized] )
			nState = WINDOWSTATE_STATE_MINIMIZED;
		else
			nState = WINDOWSTATE_STATE_NORMAL;

		[pArgs setResult:[NSNumber numberWithUnsignedLong:nState]];
	}
}

- (const NSRect)insets
{
	return maInsets;
}

- (BOOL)isFloatingWindow
{
	return ( mbUndecorated && !mbFullScreen && !mbShowOnlyMenus );
}

- (void)makeModal:(id)pObject
{
	(void)pObject;

	if ( mpWindow && [mpWindow styleMask] & NSWindowStyleMaskTitled && [mpWindow respondsToSelector:@selector(_setModalWindowLevel)] )
	{
		// Do not make window modal if any of its parent windows are in full
		// screen mode as this will cause any child windows, such as drop down
		// lists, to be displayed at a lower window level
		NSWindow *pParentWindow = mpWindow;
		while ( pParentWindow )
		{
			if ( [pParentWindow styleMask] & NSWindowStyleMaskFullScreen )
				return;
			pParentWindow = [pParentWindow parentWindow];
		}

		[mpWindow _setModalWindowLevel];

		// Run VCLWindow selector to ensure that the window level is set
		// correctly if the application is not active
		[VCLWindow clearModalWindowLevel];

		if ( [mpWindow isVisible] )
		{
			NSApplication *pApp = [NSApplication sharedApplication];
			if ( pApp )
				[pApp requestUserAttention:NSInformationalRequest];
		}
	}
}

- (void)mouseEntered:(NSEvent *)pEvent
{
	// Do not allow any recursion
	if ( mbInMouseEntered )
		return;

	mbInMouseEntered = YES;

	if ( pEvent && mpWindow && [mpWindow isVisible] && ![mpWindow isKeyWindow] )
		[mpWindow sendEvent:pEvent];

	mbInMouseEntered = NO;
}

- (void)mouseExited:(NSEvent *)pEvent
{
	// Do not allow any recursion
	if ( mbInMouseExited )
		return;

	mbInMouseExited = YES;

	if ( pEvent && mpWindow && [mpWindow isVisible] && ![mpWindow isKeyWindow] )
		[mpWindow sendEvent:pEvent];

	mbInMouseExited = NO;
}

- (void)mouseMoved:(NSEvent *)pEvent
{
	// Do not allow any recursion
	if ( mbInMouseMoved )
		return;

	mbInMouseMoved = YES;

	if ( pEvent && mpWindow && [mpWindow isVisible] && ![mpWindow isKeyWindow] )
		[mpWindow sendEvent:pEvent];

	mbInMouseMoved = NO;
}

- (void)removeTrackingArea:(VCLWindowWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSValue *pWindow = (NSValue *)[pArgArray objectAtIndex:0];
	if ( !pWindow || ![pWindow pointerValue] )
		return;

	if ( mpWindow )
	{
		NSView *pContentView = [mpWindow contentView];
		if ( pContentView )
		{
			NSArray *pTrackingAreas = [pContentView trackingAreas];
			if ( pTrackingAreas )
			{
				// Make a copy since we may remove some elements
				pTrackingAreas = [NSArray arrayWithArray:pTrackingAreas];
				if ( pTrackingAreas )
				{
					NSUInteger i = 0;
					NSUInteger nCount = [pTrackingAreas count];
					for ( ; i < nCount; i++ )
					{
						NSTrackingArea *pTrackingArea = [pTrackingAreas objectAtIndex:i];
						if ( pTrackingArea )
						{
							NSDictionary *pDict = [pTrackingArea userInfo];
							if ( pDict )
							{
								NSValue *pValue = [pDict objectForKey:pVCLTrackingAreaWindowKey];
								if ( pValue && [pValue pointerValue] == [pWindow pointerValue] )
									[pContentView removeTrackingArea:pTrackingArea];
							}
						}
					}
				}
			}
		}
	}
}

- (void)requestFocus:(VCLWindowWrapperArgs *)pArgs
{
	if ( mpWindow && [mpWindow isVisible] && ![self isFloatingWindow] )
	{
		[mpWindow makeKeyWindow];
		[pArgs setResult:[NSNumber numberWithBool:YES]];
	}
}

- (void)setContentMinSize:(NSSize)aContentMinSize
{
	if ( mpWindow )
	{
		// Make sure that there is a minimum amount of content area
		if ( mbUndecorated && aContentMinSize.width < 1 )
			aContentMinSize.width = 1;
		else if ( !mbUndecorated && aContentMinSize.width < MIN_CONTENT_WIDTH )
			aContentMinSize.width = MIN_CONTENT_WIDTH;
		if ( aContentMinSize.height < 1 )
			aContentMinSize.height = 1;

		[mpWindow setContentMinSize:aContentMinSize];

		NSSize aMinSize = NSMakeSize( aContentMinSize.width + maInsets.origin.x + maInsets.size.width, aContentMinSize.height + maInsets.origin.y + maInsets.size.height );
		[mpWindow setMinSize:aMinSize];
	}
}

- (void)setJavaFrame:(VCLWindowWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

    NSValue *pFrame = (NSValue *)[pArgArray objectAtIndex:0];
    if ( !pFrame )
        return;

    NSNumber *pInSetWindowState = (NSNumber *)[pArgArray objectAtIndex:1];
    if ( !pInSetWindowState )
        return;

	// Don't change size of windows in full screen mode or else the "update
	// links" dialog that appears when opening certain documents will leave
	// the window in a mixed state. Also, fix underlapping of the native tabbed
	// windows toolbar when switching to another application and back again
	// while in full screen mode.
	if ( mpWindow && ! ( [mpWindow styleMask] & NSWindowStyleMaskFullScreen ) )
	{
		// Fix bug 3012 by only returning a minimum size when the window is
		// visible
		NSSize aMinSize = NSMakeSize( 0, 0 );
		if ( [mpWindow isVisible] )
 			aMinSize = [mpWindow minSize];

		NSRect aFrame = [pFrame rectValue];
		if ( aFrame.size.width < aMinSize.width )
			aFrame.size.width = aMinSize.width;
		if ( aFrame.size.height < aMinSize.height )
			aFrame.size.height = aMinSize.height;

		// Always put showOnlyMenus windows under the main menubar
		if ( mbShowOnlyMenus )
		{
			maShowOnlyMenusFrame = aFrame;
			aFrame = NSMakeRect( 0, 0, 1, 1);
		}

		// Flip to native coordinates
		NSRect aTotalBounds = GetTotalScreenBounds();
		aFrame.origin.y = aTotalBounds.size.height - aFrame.origin.y - aFrame.size.height;

		BOOL bDisplay = NO;
		if ( [mpWindow isVisible] )
		{
			NSRect aOldFrame = [mpWindow frame];
			if ( aFrame.size.width != aOldFrame.size.width || aFrame.size.height != aOldFrame.size.height )
				bDisplay = YES;
		}

		[mpWindow setFrame:aFrame display:bDisplay];
	}
}

- (void)setFullScreenMode:(VCLWindowWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

    NSNumber *pFullScreen = (NSNumber *)[pArgArray objectAtIndex:0];
    if ( !pFullScreen )
        return;

	mbFullScreen = [pFullScreen boolValue];

	if ( mbUndecorated && mpWindow )
	{
		// Only adjust the color, level, and shadow if the window is
		// undecorated. Otherwise, a full screen window will not display the
		// current document in the versions browser on macOS 10.12.
		[self adjustColorLevelAndShadow];

		if ( [mpWindow isKindOfClass:[VCLPanel class]] )
			[(VCLPanel *)mpWindow setCanBecomeKeyWindow:( mbFullScreen || mnStyle & SAL_FRAME_STYLE_FLOAT_FOCUSABLE ) ? YES : NO];
		else
			[(VCLWindow *)mpWindow setCanBecomeKeyWindow:( mbFullScreen || mnStyle & SAL_FRAME_STYLE_FLOAT_FOCUSABLE ) ? YES : NO];
	}
}

- (void)setMinSize:(VCLWindowWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

    NSValue *pMinSize = (NSValue *)[pArgArray objectAtIndex:0];
    if ( !pMinSize )
        return;

	[self setContentMinSize:[pMinSize sizeValue]];
}

- (void)setMovable:(VCLWindowWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

    NSNumber *pMovable = (NSNumber *)[pArgArray objectAtIndex:0];
    if ( !pMovable )
        return;

	if ( mpWindow )
		[mpWindow setMovable:[pMovable boolValue]];
}

- (void)setState:(VCLWindowWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

    NSNumber *pState = (NSNumber *)[pArgArray objectAtIndex:0];
    if ( !pState )
        return;

	// Only change the state if the window is a visible window with a frame and
	// no parent window as this method can cause a deadlock with the native
	// menu handler on Mac OS X. Also, don't allow utility windows to be
	// minimized.
	unsigned long nState = [pState unsignedLongValue];
	if ( !mbUtility && !mbShowOnlyMenus && !mbUndecorated && !mpParent && mpWindow && ( [mpWindow isVisible] || [mpWindow isMiniaturized] ) )
	{
		if ( nState == WINDOWSTATE_STATE_MINIMIZED && [mpWindow styleMask] & NSWindowStyleMaskMiniaturizable )
			[mpWindow miniaturize:self];
		else if ( [mpWindow isMiniaturized] )
			[mpWindow deminiaturize:self];
	}
}

- (void)setTitle:(VCLWindowWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

    NSString *pTitle = (NSString *)[pArgArray objectAtIndex:0];
    if ( !pTitle )
        return;

	if ( mpWindow )
		[mpWindow setTitle:pTitle];
}

- (void)setVisible:(VCLWindowWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

    NSNumber *pVisible = (NSNumber *)[pArgArray objectAtIndex:0];
    if ( !pVisible )
        return;

    NSNumber *pNoActivate = (NSNumber *)[pArgArray objectAtIndex:1];
    if ( !pNoActivate )
        return;

	// Fix bug reported in the following NeoOffice forum post by treating
	// minimized windows the same as visible windows:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=63308#63308
	BOOL bVisible = [pVisible boolValue];
	if ( mpWindow && bVisible != ( [mpWindow isVisible] || [mpWindow isMiniaturized] ) )
	{
		if ( bVisible )
		{
			if ( [mpWindow isMiniaturized] )
				[mpWindow deminiaturize:self];

			// Fix bug 1012 by deiconifying the parent window. Fix bug 1388 by
			// skipping this step if the current window is a floating window.
			if ( ![self isFloatingWindow] && mpParent && [mpParent isMiniaturized] )
				[mpParent deminiaturize:self];

 			NSRect aFrame = [mpWindow frame];
			NSSize aMinSize = [mpWindow minSize];
			if ( aFrame.size.width < aMinSize.width )
				aFrame.size.width = aMinSize.width;
			if ( aFrame.size.height < aMinSize.height )
				aFrame.size.height = aMinSize.height;
			[mpWindow setFrame:aFrame display:NO];

			if ( mpParent && ![mpWindow parentWindow] )
			{
				// Check if the parent window's titlebar window is visible. If
				// so, do not attach the current window to parent window or
				// else it will cause the titlebar window to be drawn
				// incorrectly. To avoid this bug, either hide the titlebar
				// window before attaching the current window to the parent
				// window or abort display of the current window.
				NSResponder *pResponder = [mpParent firstResponder];
				if ( pResponder && [pResponder isKindOfClass:[NSView class]] )
				{
					NSWindow *pResponderWindow = [(NSView *)pResponder window];
					if ( pResponderWindow && [pResponderWindow parentWindow] == mpParent && ![pResponderWindow isKindOfClass:[VCLWindow class]] && ![pResponderWindow isKindOfClass:[VCLPanel class]] && [pResponderWindow isVisible] )
					{
						
						if ( mbUndecorated )
							return;
						else
							[pResponderWindow orderOut:self];
					}
				}
			}

			NSWindow *pKeyWindow = nil;
			NSApplication *pApp = [NSApplication sharedApplication];
			if ( pApp )
				pKeyWindow = [pApp keyWindow];

			[mpWindow orderWindow:NSWindowAbove relativeTo:( mpParent ? [mpParent windowNumber] : 0 )];
			BOOL bCanBecomeKeyWindow;
			if ( [mpWindow isKindOfClass:[VCLPanel class]] )
				bCanBecomeKeyWindow = [(VCLPanel *)mpWindow canBecomeKeyWindow];
			else
				bCanBecomeKeyWindow = [(VCLWindow *)mpWindow canBecomeKeyWindow];
			if ( bCanBecomeKeyWindow && ![pNoActivate boolValue] )
				[mpWindow makeKeyWindow];

			if ( mpParent && ![mpWindow parentWindow] )
			{
				// Fix the hidden "update links" modal dialog when opening a
				// document from a document that is already in full screen mode
				// by attaching titled windows to the last key window if the
				// last key window is in full screen mode
				NSWindow *pParentWindow = mpParent;
				if ( pKeyWindow && pKeyWindow != mpWindow && [mpWindow styleMask] & NSWindowStyleMaskTitled && [pKeyWindow styleMask] & NSWindowStyleMaskFullScreen )
					pParentWindow = pKeyWindow;
				[pParentWindow addChildWindow:mpWindow ordered:NSWindowAbove];
			}

			if ( pApp && [mpWindow level] == NSModalPanelWindowLevel )
				[pApp requestUserAttention:NSInformationalRequest];
		}
		else
		{
			[self animateWaitingView:NO];

			NSWindow *pParentWindow = [mpWindow parentWindow];
			if ( pParentWindow )
				[pParentWindow removeChildWindow:mpWindow];

			CloseOrOrderOutWindow( mpWindow );

			// Release cached cursor
			::std::map< NSWindow*, NSCursor* >::iterator cit = aNativeCursorMap.find( mpWindow );
			if ( cit != aNativeCursorMap.end() )
			{
				[cit->second release];
				aNativeCursorMap.erase( cit );
			}
		}
	}

	// Fix bug reported in the following NeoOffice forum topic by always
	// updating the show only menus even if there is no change in visibility
	// for the current window:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=63224#63224
	[VCLWindowWrapper updateShowOnlyMenusWindows];
}

- (void)toFront:(VCLWindowWrapperArgs *)pArgs
{
	if ( mpWindow && ( [mpWindow isVisible] || [mpWindow isMiniaturized] ) && ![self isFloatingWindow] )
	{
		// Fix bug reported in the following NeoOffice forum post by
		// unminiaturizing the window before making it the key window:
		// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=63241#63241
		if ( [mpWindow isMiniaturized] )
			[mpWindow deminiaturize:self];

		if ( [mpWindow isVisible] )
		{
			[mpWindow makeKeyAndOrderFront:self];
			[pArgs setResult:[NSNumber numberWithBool:YES]];
		}
	}
}

- (NSWindow *)window
{
	return mpWindow;
}

@end

@interface VCLCreateWindow : NSObject
{
	JavaSalFrame*			mpFrame;
	NSWindow*				mpParent;
	BOOL					mbShowOnlyMenus;
	sal_uLong				mnStyle;
	BOOL					mbUtility;
	VCLWindowWrapper*		mpWindow;
}
+ (id)createWithStyle:(sal_uLong)nStyle frame:(JavaSalFrame *)pFrame parent:(NSWindow *)pParent showOnlyMenus:(BOOL)bShowOnlyMenus utility:(BOOL)bUtility;
- (id)initWithStyle:(sal_uLong)nStyle frame:(JavaSalFrame *)pFrame parent:(NSWindow *)pParent showOnlyMenus:(BOOL)bShowOnlyMenus utility:(BOOL)bUtility;
- (void)dealloc;
- (void)createWindow:(id)pObject;
- (VCLWindowWrapper *)window;
@end

@implementation VCLCreateWindow

+ (id)createWithStyle:(sal_uLong)nStyle frame:(JavaSalFrame *)pFrame parent:(NSWindow *)pParent showOnlyMenus:(BOOL)bShowOnlyMenus utility:(BOOL)bUtility
{
	VCLCreateWindow *pRet = [[VCLCreateWindow alloc] initWithStyle:nStyle frame:pFrame parent:pParent showOnlyMenus:bShowOnlyMenus utility:bUtility];
	[pRet autorelease];
	return pRet;
}

- (id)initWithStyle:(sal_uLong)nStyle frame:(JavaSalFrame *)pFrame parent:(NSWindow *)pParent showOnlyMenus:(BOOL)bShowOnlyMenus utility:(BOOL)bUtility
{
	[super init];

	mpFrame = pFrame;
	mpParent = pParent;
	if ( mpParent )
		[mpParent retain];
	mbShowOnlyMenus = bShowOnlyMenus;
	mnStyle = nStyle;
	mbUtility = bUtility;
	mpWindow = nil;

	return self;
}

- (void)dealloc
{
	if ( mpParent )
		[mpParent release];

	if ( mpWindow )
		[mpWindow release];

	[super dealloc];
}

- (void)createWindow:(id)pObject
{
	(void)pObject;

	if ( mpWindow )
		return;

	mpWindow = [[VCLWindowWrapper alloc] initWithStyle:mnStyle frame:mpFrame parent:mpParent showOnlyMenus:mbShowOnlyMenus utility:mbUtility];
}

- (VCLWindowWrapper *)window
{
	return mpWindow;
}

@end

// =======================================================================

static void InitializeScreens()
{
	if ( !bScreensInitialized )
	{
		ClearableGuard< Mutex > aGuard( aSystemColorsMutex );

		// Set system screens and add observer for system screen changes
		if ( !bScreensInitialized )
		{
			NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

			VCLUpdateScreens *pVCLUpdateScreens = [VCLUpdateScreens create];
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			aGuard.clear();
			[pVCLUpdateScreens performSelectorOnMainThread:@selector(updateScreens:) withObject:pVCLUpdateScreens waitUntilDone:YES modes:pModes];

			[pPool release];
		}
	}
}

// -----------------------------------------------------------------------

static void InitializeSystemColors()
{
	if ( !bSystemColorsInitialized )
	{
		ClearableGuard< Mutex > aGuard( aSystemColorsMutex );

		// Set system colors and add observer for system color changes
		if ( !bSystemColorsInitialized )
		{
			NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

			VCLUpdateSystemColors *pVCLUpdateSystemColors = [VCLUpdateSystemColors create];
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			aGuard.clear();
			[pVCLUpdateSystemColors performSelectorOnMainThread:@selector(updateSystemColors:) withObject:pVCLUpdateSystemColors waitUntilDone:YES modes:pModes];

			[pPool release];
		}
	}
}

// =======================================================================

// Note: this must not be static as the symbol will be loaded by the framework
// module
sal_Bool IsShowOnlyMenusWindow( Window *pWindow )
{
	if ( !pWindow )
		return sal_False;

	SystemWindow *pSystemWindow = pWindow->GetSystemWindow();
	if ( !pSystemWindow )
		return sal_False;

	JavaSalFrame *pFrame = (JavaSalFrame *)pSystemWindow->ImplGetFrame();
	if ( !pFrame )
		return sal_False;

	return pFrame->mbShowOnlyMenus;
}

// -----------------------------------------------------------------------

// Note: this must not be static as the symbol will be loaded by the framework
// module
void ShowOnlyMenusForWindow( Window *pWindow, sal_Bool bShowOnlyMenus )
{
	if ( !pWindow )
		return;

	SystemWindow *pSystemWindow = pWindow->GetSystemWindow();
	if ( !pSystemWindow )
		return;

	JavaSalFrame *pFrame = (JavaSalFrame *)pSystemWindow->ImplGetFrame();
	if ( !pFrame || ( bShowOnlyMenus && pFrame->mpParent ) )
		return;

	pFrame->mbInShowOnlyMenus = sal_True;

	// Don't let the progress bar hiding override display set for top-level
	// windows
	sal_Bool bOldShowOnlyMenus = pFrame->mbShowOnlyMenus;
	StatusBar *pProgressBarWindow = dynamic_cast< StatusBar* >( pWindow );
	if ( pProgressBarWindow )
	{
		if ( pFrame->mbShowOnlyMenus )
			pFrame->mbShowOnlyMenus = bShowOnlyMenus;
	}
	else
	{
		pFrame->mbShowOnlyMenus = bShowOnlyMenus;
	}

	// Refresh Java frame
	if ( bOldShowOnlyMenus != pFrame->mbShowOnlyMenus )
		pFrame->SetParent( pFrame->mpParent );

	pFrame->mbInShowOnlyMenus = sal_False;
}

// -----------------------------------------------------------------------

void JavaSalFrame_drawToNSView( NSView *pView, NSRect aDirtyRect )
{
	if ( !pView )
		return;

	NSWindow *pWindow = [pView window];
	if ( !pWindow || ![pWindow isVisible] )
	{
		[pView setNeedsDisplay:NO];
		return;
	}

	// Eliminate black squares that sporadically appear in bottom corners of
	// windows on OS X 10.11 by filling with the window's background color
	NSColor *pColor = [pWindow backgroundColor];
	if ( pColor )
	{
		[pColor setFill];
		[NSBezierPath fillRect:aDirtyRect];
	}

	::std::map< NSWindow*, JavaSalGraphics* >::iterator it = aNativeWindowMap.find( pWindow );
	if ( it != aNativeWindowMap.end() && it->second )
	{
		CGRect aBounds = CGRectStandardize( NSRectToCGRect( [pView bounds] ) );
		CGRect aDestRect = CGRectStandardize( NSRectToCGRect( aDirtyRect ) );
		if ( CGRectIntersectsRect( aBounds, aDestRect ) )
		{
			NSGraphicsContext *pContext = [pWindow graphicsContext];
			if ( pContext )
			{
				NSGraphicsContext *pOldContext = [NSGraphicsContext currentContext];
				[NSGraphicsContext setCurrentContext:pContext];

				CGContextRef aContext = (CGContextRef)[pContext graphicsPort];
				if ( aContext )
				{
					CGContextSaveGState( aContext );
					CGContextTranslateCTM( aContext, aBounds.origin.x, aBounds.origin.y );
					BOOL bFlipped = [pView isFlipped];
					aDestRect.origin.y -= aBounds.origin.y;
					aDestRect.origin.x -= aBounds.origin.x;
					aBounds.origin.x = 0;
					aBounds.origin.y = 0;
					if ( bFlipped )
					{
						CGContextTranslateCTM( aContext, 0, aBounds.size.height );
						CGContextScaleCTM( aContext, 1.0, -1.0f );
						aDestRect.origin.y = aBounds.size.height - aDestRect.origin.y - aDestRect.size.height;
					}

					it->second->copyToContext( NULL, NULL, false, false, aContext, aBounds, aDestRect, aDestRect, true, !bFlipped );

					CGContextRestoreGState( aContext );
				}

				[NSGraphicsContext setCurrentContext:pOldContext];
			}
		}
	}
}

// -----------------------------------------------------------------------

NSCursor *JavaSalFrame_getCursor( NSView *pView )
{
	NSCursor *pRet = nil;

	if ( !pView )
		return pRet;

	NSWindow *pWindow = [pView window];
	if ( !pWindow || ![pWindow isVisible] )
		return pRet;

	::std::map< NSWindow*, NSCursor* >::const_iterator cit = aNativeCursorMap.find( pWindow );
	if ( cit != aNativeCursorMap.end() )
		pRet = cit->second;

	return pRet;
}

// =======================================================================

JavaSalFrame::JavaSalFrame( sal_uLong nSalFrameStyle, JavaSalFrame *pParent ) :
	maFrameLayer( NULL ),
	maFrameClipPath( NULL ),
	mpWindow( NULL ),
	mbAllowKeyBindings( true ),
	mpGraphics( new JavaSalGraphics() ),
	mnStyle( nSalFrameStyle ),
	mpParent( NULL ),
	mbGraphics( sal_False ),
	mbVisible( false ),
	mbCenter( sal_True ),
	mbFullScreen( false ),
	mbPresentation( false ),
	mpMenuBar( NULL ),
	mbInSetPosSize( sal_False ),
	mbInShow( sal_False ),
	mbShowOnlyMenus( sal_False ),
	mbInShowOnlyMenus( sal_False ),
	mbInShowFullScreen( sal_False ),
	mbInWindowDidExitFullScreen( sal_False ),
	mbInWindowWillEnterFullScreen( sal_False ),
	mbInSetWindowState( sal_False )
{
	memset( &maSysData, 0, sizeof( SystemEnvData ) );
	maSysData.nSize = sizeof( SystemEnvData );

	memset( &maGeometry, 0, sizeof( maGeometry ) );
	memset( &maOriginalGeometry, 0, sizeof( maOriginalGeometry ) );

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	VCLCreateWindow *pVCLCreateWindow = [VCLCreateWindow createWithStyle:mnStyle frame:this parent:nil showOnlyMenus:mbShowOnlyMenus utility:IsUtilityWindow()];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLCreateWindow performSelectorOnMainThread:@selector(createWindow:) withObject:pVCLCreateWindow waitUntilDone:YES modes:pModes];
	VCLWindowWrapper *pWindow = [pVCLCreateWindow window];
	if ( pWindow )
	{
		[pWindow retain];
		mpWindow = pWindow;
	}

	[pPool release];

	mpGraphics->mpFrame = this;
	mpGraphics->mnDPIX = MIN_SCREEN_RESOLUTION;
	mpGraphics->mnDPIY = MIN_SCREEN_RESOLUTION;
	mpGraphics->setLayer( NULL );

	// Set initial parent
	if ( pParent )
		SetParent( pParent );

	// Cache the insets
	Rectangle aRect = GetInsets();
	maGeometry.nLeftDecoration = aRect.Left();
	maGeometry.nTopDecoration = aRect.Top();
	maGeometry.nRightDecoration = aRect.Right();
	maGeometry.nBottomDecoration = aRect.Bottom();

	// Insert this window into the window list
	GetSalData()->maFrameList.push_front( this );
}

// -----------------------------------------------------------------------

JavaSalFrame::~JavaSalFrame()
{
	SalData *pSalData = GetSalData();

	// Remove this window from the window list
	pSalData->maFrameList.remove( this );

	// Make sure that no native drawing is possible
	maSysData.mpNSView = NULL;
	UpdateLayer();

	Show( false );
	StartPresentation( false );
	CaptureMouse( false );

	if ( mpMenuBar )
		mpMenuBar->SetFrame( NULL );

	// Detach child objects. Fix bug 3038 unsetting each object's parent.
	::std::list< JavaSalObject* > aObjects( maObjects );
	for ( ::std::list< JavaSalObject* >::const_iterator it = aObjects.begin(); it != aObjects.end(); ++it )
		(*it)->Destroy();

	// Detach child windows
	::std::list< JavaSalFrame* > aChildren( maChildren );
	for ( ::std::list< JavaSalFrame* >::const_iterator it = aChildren.begin(); it != aChildren.end(); ++it )
		(*it)->SetParent( NULL );

	// Detach from parent
	SetParent( NULL );

	if ( maFrameClipPath )
		CGPathRelease( maFrameClipPath );

	if ( maFrameLayer )
		CGLayerRelease( maFrameLayer );

	// Check for and remove any stale pointers to this instance
	if ( pSalData->mpFocusFrame == this )
		pSalData->mpFocusFrame = NULL;
	if ( pSalData->mpPresentationFrame == this )
		pSalData->mpPresentationFrame = NULL;
	if ( pSalData->mpNativeModalSheetFrame == this )
		pSalData->mpNativeModalSheetFrame = NULL;
	if ( pSalData->mpLastDragFrame == this )
		pSalData->mpLastDragFrame = NULL;
	if ( pSalData->mpCaptureFrame == this )
		pSalData->mpCaptureFrame = NULL;
	if ( pSalData->mpLastResizeFrame == this )
		pSalData->mpLastResizeFrame = NULL;
	if ( pSalData->mpLastMouseMoveFrame == this )
		pSalData->mpLastMouseMoveFrame = NULL;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpWindow )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(destroy:) withObject:mpWindow waitUntilDone:YES modes:pModes];
		[mpWindow release];
	}

	[pPool release];

	// Delete graphics last as it may be needed by a JavaSalBitmap
	if ( mpGraphics )
		delete mpGraphics;
}

// -----------------------------------------------------------------------
 
OUString JavaSalFrame::ConvertVCLKeyCode( sal_uInt16 nKeyCode, bool bIsMenuShortcut )
{
	OUString aRet;

	nKeyCode &= KEY_CODE_MASK;
	switch ( nKeyCode )
	{
		case KEY_0:
		case KEY_1:
		case KEY_2:
		case KEY_3:
		case KEY_4:
		case KEY_5:
		case KEY_6:
		case KEY_7:
		case KEY_8:
		case KEY_9:
			aRet = OUString( (sal_Unicode)( '0' + nKeyCode - KEY_0 ) );
			break;
		case KEY_A:
		case KEY_B:
		case KEY_C:
		case KEY_D:
		case KEY_E:
		case KEY_F:
		case KEY_G:
		case KEY_H:
		case KEY_I:
		case KEY_J:
		case KEY_K:
		case KEY_L:
		case KEY_M:
		case KEY_N:
		case KEY_O:
		case KEY_P:
		case KEY_Q:
		case KEY_R:
		case KEY_S:
		case KEY_T:
		case KEY_U:
		case KEY_V:
		case KEY_W:
		case KEY_X:
		case KEY_Y:
		case KEY_Z:
			aRet = OUString( (sal_Unicode)( 'A' + nKeyCode - KEY_A ) );
			break;
		case KEY_F1:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF1FunctionKey );
			else
				aRet = "F1";
			break;
		case KEY_F2:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF2FunctionKey );
			else
				aRet = "F2";
			break;
		case KEY_F3:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF3FunctionKey );
			else
				aRet = "F3";
			break;
		case KEY_F4:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF4FunctionKey );
			else
				aRet = "F4";
			break;
		case KEY_F5:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF5FunctionKey );
			else
				aRet = "F5";
			break;
		case KEY_F6:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF6FunctionKey );
			else
				aRet = "F6";
			break;
		case KEY_F7:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF7FunctionKey );
			else
				aRet = "F7";
			break;
		case KEY_F8:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF8FunctionKey );
			else
				aRet = "F8";
			break;
		case KEY_F9:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF9FunctionKey );
			else
				aRet = "F9";
			break;
		case KEY_F10:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF10FunctionKey );
			else
				aRet = "F10";
			break;
		case KEY_F11:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF11FunctionKey );
			else
				aRet = "F11";
			break;
		case KEY_F12:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF12FunctionKey );
			else
				aRet = "F12";
			break;
		case KEY_F13:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF13FunctionKey );
			else
				aRet = "F13";
			break;
		case KEY_F14:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF14FunctionKey );
			else
				aRet = "F14";
			break;
		case KEY_F15:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF15FunctionKey );
			else
				aRet = "F15";
			break;
		case KEY_F16:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF16FunctionKey );
			else
				aRet = "F16";
			break;
		case KEY_F17:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF17FunctionKey );
			else
				aRet = "F17";
			break;
		case KEY_F18:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF18FunctionKey );
			else
				aRet = "F18";
			break;
		case KEY_F19:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF19FunctionKey );
			else
				aRet = "F19";
			break;
		case KEY_F20:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF20FunctionKey );
			else
				aRet = "F20";
			break;
		case KEY_F21:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF21FunctionKey );
			else
				aRet = "F21";
			break;
		case KEY_F22:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF22FunctionKey );
			else
				aRet = "F22";
			break;
		case KEY_F23:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF23FunctionKey );
			else
				aRet = "F23";
			break;
		case KEY_F24:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF24FunctionKey );
			else
				aRet = "F24";
			break;
		case KEY_F25:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF25FunctionKey );
			else
				aRet = "F25";
			break;
		case KEY_F26:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSF26FunctionKey );
			else
				aRet = "F26";
			break;
		case KEY_DOWN:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSDownArrowFunctionKey );
			else
				aRet = OUString( (sal_Unicode)0x2193 );
			break;
		case KEY_UP:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSUpArrowFunctionKey );
			else
				aRet = OUString( (sal_Unicode)0x2191 );
			break;
		case KEY_LEFT:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSLeftArrowFunctionKey );
			else
				aRet = OUString( (sal_Unicode)0x2190);
			break;
		case KEY_RIGHT:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSRightArrowFunctionKey );
			else
				aRet = OUString( (sal_Unicode)0x2192 );
			break;
		case KEY_HOME:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSHomeFunctionKey );
			else
				aRet = OUString( (sal_Unicode)0x2196 );
			break;
		case KEY_END:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSEndFunctionKey );
			else
				aRet = OUString( (sal_Unicode)0x2198 );
			break;
		case KEY_PAGEUP:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSPageUpFunctionKey );
			else
				aRet = OUString( (sal_Unicode)0x21de );
			break;
		case KEY_PAGEDOWN:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSPageDownFunctionKey );
			else
				aRet = OUString( (sal_Unicode)0x21df );
			break;
		case KEY_RETURN:
			if ( !bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)0x23ce );
			break;
		case KEY_ESCAPE:
			if ( !bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)0x238b );
			break;
		case KEY_TAB:
			if ( !bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)0x21e5 );
			break;
		case KEY_BACKSPACE:
			if ( !bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)0x232b );
			break;
		case KEY_SPACE:
			if ( !bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)0x2423 );
			break;
		case KEY_INSERT:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSInsertFunctionKey );
			break;
		case KEY_DELETE:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSDeleteFunctionKey );
			else
				aRet = OUString( (sal_Unicode)0x2326 );
			break;
		case KEY_ADD:
			aRet = OUString( (sal_Unicode)'+' );
			break;
		case KEY_SUBTRACT:
			aRet = OUString( (sal_Unicode)'-' );
			break;
		case KEY_MULTIPLY:
			aRet = OUString( (sal_Unicode)'*' );
			break;
		case KEY_DIVIDE:
			aRet = OUString( (sal_Unicode)'/' );
			break;
		case KEY_DECIMAL:
		case KEY_POINT:
			aRet = OUString( (sal_Unicode)'.' );
			break;
		case KEY_COMMA:
			aRet = OUString( (sal_Unicode)',' );
			break;
		case KEY_LESS:
			aRet = OUString( (sal_Unicode)'<' );
			break;
		case KEY_GREATER:
			aRet = OUString( (sal_Unicode)'>' );
			break;
		case KEY_EQUAL:
			aRet = OUString( (sal_Unicode)'=' );
			break;
		case KEY_UNDO:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSUndoFunctionKey );
			break;
		case KEY_FIND:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSFindFunctionKey );
			break;
		case KEY_HELP:
			if ( bIsMenuShortcut )
				aRet = OUString( (sal_Unicode)NSHelpFunctionKey );
			break;
		case KEY_TILDE:
			aRet = OUString( (sal_Unicode)'~' );
			break;
		case KEY_QUOTELEFT:
			aRet = OUString( (sal_Unicode)'`' );
			break;
		case KEY_BRACKETLEFT:
			aRet = OUString( (sal_Unicode)'[' );
		case KEY_BRACKETRIGHT:
			aRet = OUString( (sal_Unicode)']' );
		case KEY_SEMICOLON:
			aRet = OUString( (sal_Unicode)';' );
		case KEY_QUOTERIGHT:
			aRet = OUString( (sal_Unicode)'\'' );
			break;
		case KEY_REPEAT:
		case KEY_CUT:
		case KEY_COPY:
		case KEY_PASTE:
		case KEY_PROPERTIES:
		case KEY_OPEN:
		case KEY_FRONT:
		case KEY_CONTEXTMENU:
		case KEY_MENU:
		case KEY_HANGUL_HANJA:
		case KEY_CAPSLOCK:
		case KEY_NUMLOCK:
		case KEY_SCROLLLOCK:
		default:
			break;
	}

	return aRet;
}

// -----------------------------------------------------------------------

void JavaSalFrame::FlushAllFrames()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	VCLSetNeedsDisplayAllViews *pVCLSetNeedsDisplayAllViews = [VCLSetNeedsDisplayAllViews create];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLSetNeedsDisplayAllViews performSelectorOnMainThread:@selector(setNeedsDisplay:) withObject:pVCLSetNeedsDisplayAllViews waitUntilDone:NO modes:pModes];

	[pPool release];
}

// -----------------------------------------------------------------------

unsigned int JavaSalFrame::GetDefaultScreenNumber()
{
	// Fix slideshow appearing underneath the presentation controller window
	// when running a slideshow immediately after moving the presentation
	// window to a different screen by always returning the first screen
	// instead of the the native main screen number
	return 0;
}

// -----------------------------------------------------------------------

const Rectangle JavaSalFrame::GetScreenBounds( long nX, long nY, long nWidth, long nHeight, sal_Bool bFullScreenMode )
{
	// Update if screens have not yet been set
	InitializeScreens();

	MutexGuard aGuard( aScreensMutex );

	// Fix bug 2671 by setting width and height greater than 0
	if ( nWidth <= 0 )
		nWidth = 1;
	if ( nHeight <= 0 )
		nHeight = 1;

	// Iterate through the screens and find the screen that the
	// point is inside of
	Point aPoint( nX, nY );
	unsigned int i;
	for ( i = 0; i < aVCLScreensFullBoundsList.size() && i < aVCLScreensVisibleBoundsList.size(); i++ )
	{
		// Test if the top left point is inside the screen
		if ( aVCLScreensFullBoundsList[ i ].IsInside( aPoint ) )
		{
			if ( bFullScreenMode )
				return aVCLScreensFullBoundsList[ i ];
			else
				return aVCLScreensVisibleBoundsList[ i ];
		}
	}

	// Iterate through the screens and find the closest screen
	unsigned long nClosestArea = ULONG_MAX;
	Rectangle aClosestBounds;
	for ( i = 0; i < aVCLScreensFullBoundsList.size() && i < aVCLScreensVisibleBoundsList.size(); i++ )
	{
		Rectangle aBounds;
		if ( bFullScreenMode )
			aBounds = aVCLScreensFullBoundsList[ i ];
		else
			aBounds = aVCLScreensVisibleBoundsList[ i ];

		// Test the closeness of the point to the center of the screen
		unsigned long nArea = labs( ( aBounds.Left() + ( aBounds.GetWidth() / 2 ) - nX ) * ( aBounds.Top() + ( aBounds.GetHeight() / 2 ) - nY ) );
		if ( nClosestArea > nArea )
		{
			nClosestArea = nArea;
			aClosestBounds = aBounds;
		}
	}

	if ( aClosestBounds.GetWidth() > 0 && aClosestBounds.GetHeight() > 0 )
		return aClosestBounds;
	else
		return Rectangle( Point( 0, 0 ), Size( 800, 600 ) );
}

// -----------------------------------------------------------------------

const Rectangle JavaSalFrame::GetScreenBounds( unsigned int nScreen, sal_Bool bFullScreenMode )
{
	// Update if screens have not yet been set
	InitializeScreens();

	MutexGuard aGuard( aScreensMutex );

	if ( bFullScreenMode && nScreen < aVCLScreensFullBoundsList.size() )
		return aVCLScreensFullBoundsList[ nScreen ];

	if ( !bFullScreenMode && nScreen < aVCLScreensVisibleBoundsList.size() )
		return aVCLScreensVisibleBoundsList[ nScreen ];
		
	return Rectangle( Point( 0, 0 ), Size( 0, 0 ) );
}

// -----------------------------------------------------------------------

unsigned int JavaSalFrame::GetScreenCount()
{
	// Update if screens have not yet been set
	InitializeScreens();

	MutexGuard aGuard( aScreensMutex );
	return ( aVCLScreensFullBoundsList.size() ? aVCLScreensFullBoundsList.size() : 1 );
}

// -----------------------------------------------------------------------

bool JavaSalFrame::GetAlternateSelectedControlTextColor( SalColor& rSalColor )
{
	bool bRet = false;

	// Update colors if any system colors have not yet been set
	InitializeSystemColors();

	if ( pVCLAlternateSelectedControlTextColor )
	{
		rSalColor = *pVCLAlternateSelectedControlTextColor;
		bRet = true;
	}

	return bRet;
}

// -----------------------------------------------------------------------

bool JavaSalFrame::GetControlTextColor( SalColor& rSalColor )
{
	bool bRet = false;

	// Update colors if any system colors have not yet been set
	InitializeSystemColors();

	if ( pVCLControlTextColor )
	{
		rSalColor = *pVCLControlTextColor;
		bRet = true;
	}

	return bRet;
}

// -----------------------------------------------------------------------

bool JavaSalFrame::GetDisabledControlTextColor( SalColor& rSalColor )
{
	bool bRet = false;

	// Update colors if any system colors have not yet been set
	InitializeSystemColors();

	if ( pVCLDisabledControlTextColor )
	{
		rSalColor = *pVCLDisabledControlTextColor;
		bRet = true;
	}

	return bRet;
}

// -----------------------------------------------------------------------

bool JavaSalFrame::GetSelectedControlTextColor( SalColor& rSalColor )
{
	bool bRet = false;

	// Update colors if any system colors have not yet been set
	InitializeSystemColors();

	if ( pVCLSelectedControlTextColor )
	{
		rSalColor = *pVCLSelectedControlTextColor;
		bRet = true;
	}

	return bRet;
}

// -----------------------------------------------------------------------

bool JavaSalFrame::GetSelectedMenuItemTextColor( SalColor& rSalColor )
{
	bool bRet = false;

	// Update colors if any system colors have not yet been set
	InitializeSystemColors();

	if ( pVCLSelectedMenuItemTextColor )
	{
		rSalColor = *pVCLSelectedMenuItemTextColor;
		bRet = true;
	}

	return bRet;
}

// -----------------------------------------------------------------------

void JavaSalFrame::AddObject( JavaSalObject *pObject, bool bVisible )
{
	if ( pObject )
	{
		maObjects.push_back( pObject );
		if ( bVisible )
			maVisibleObjects.push_back( pObject );
	}
}

// -----------------------------------------------------------------------

bool JavaSalFrame::Deminimize()
{
	bool bRet = false;

	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLWindowWrapperArgs *pRequestFocusArgs = [VCLWindowWrapperArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(deminimize:) withObject:pRequestFocusArgs waitUntilDone:YES modes:pModes];
		NSNumber *pResult = (NSNumber *)[pRequestFocusArgs result];
		if ( pResult && [pResult boolValue] )
			bRet = true;

		[pPool release];
	}

	return bRet;
}

// -----------------------------------------------------------------------

bool JavaSalFrame::IsFloatingFrame()
{
	return ( ! ( mnStyle & ( SAL_FRAME_STYLE_DEFAULT | SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE | SAL_FRAME_STYLE_FLOAT_FOCUSABLE ) ) && this != GetSalData()->mpPresentationFrame && !mbShowOnlyMenus );
}

// -----------------------------------------------------------------------

bool JavaSalFrame::IsUtilityWindow()
{
	return ( mnStyle & SAL_FRAME_STYLE_MOVEABLE && mnStyle & SAL_FRAME_STYLE_TOOLWINDOW && !IsFloatingFrame() );
}

// -----------------------------------------------------------------------

void JavaSalFrame::RemoveObject( JavaSalObject *pObject, bool bDeleted )
{
	if ( pObject )
	{
		if ( bDeleted )
			maObjects.remove( pObject );
		maVisibleObjects.remove( pObject );
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::FlushAllObjects()
{
	if ( mbVisible )
	{
		::std::list< JavaSalObject* > aObjects( maVisibleObjects );
		for ( ::std::list< JavaSalObject* >::const_iterator it = aObjects.begin(); it != aObjects.end(); ++it )
			(*it)->Flush();
	}
}

// -----------------------------------------------------------------------

const Rectangle JavaSalFrame::GetBounds( sal_Bool *pInLiveResize, sal_Bool *pInFullScreenMode, sal_Bool bUseFullScreenOriginalBounds )
{
	Rectangle aRet( Point( 0, 0 ), Size( 0, 0 ) );

	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLWindowWrapperArgs *pGetFrameArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObjects:[NSValue valueWithPointer:pInLiveResize], [NSValue valueWithPointer:pInFullScreenMode], [NSNumber numberWithBool:bUseFullScreenOriginalBounds], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(getFrame:) withObject:pGetFrameArgs waitUntilDone:YES modes:pModes];
		NSValue *pFrame = (NSValue *)[pGetFrameArgs result];
		if ( pFrame )
		{
			NSRect aFrame = [pFrame rectValue];
			aRet = Rectangle( Point( (long)aFrame.origin.x, (long)aFrame.origin.y ), Size( (long)aFrame.size.width, (long)aFrame.size.height ) );

			// Update insets for non-full screen windows as tabbed windows have
			// a different inset than untabbed windows
			const NSRect aInsets = [mpWindow insets];
			Rectangle aRect( (long)aInsets.origin.x, (long)aInsets.size.height, (long)aInsets.size.width, (long)aInsets.origin.y );
			maGeometry.nLeftDecoration = aRect.Left();
			maGeometry.nTopDecoration = aRect.Top();
			maGeometry.nRightDecoration = aRect.Right();
			maGeometry.nBottomDecoration = aRect.Bottom();
		}

		[pPool release];
	}

	return aRet;
}

// -----------------------------------------------------------------------

const Rectangle JavaSalFrame::GetInsets()
{
	// Insets use the rectangle's data members directly so set members directly
	Rectangle aRet( 0, 0, 0, 0 );

	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		// Flip native insets
		const NSRect aInsets = [mpWindow insets];
		aRet = Rectangle( (long)aInsets.origin.x, (long)aInsets.size.height, (long)aInsets.size.width, (long)aInsets.origin.y );

		[pPool release];
	}

	return aRet;
}

// -----------------------------------------------------------------------

id JavaSalFrame::GetNativeWindow()
{
	id pRet = nil;

	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		pRet = [mpWindow window];

		[pPool release];
	}

	return pRet;
}

// -----------------------------------------------------------------------

id JavaSalFrame::GetNativeWindowContentView( sal_Bool bTopLevelWindow )
{
	id pRet = nil;

	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLWindowWrapperArgs *pGetContentViewArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithBool:bTopLevelWindow]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(getContentView:) withObject:pGetContentViewArgs waitUntilDone:YES modes:pModes];
		pRet = [pGetContentViewArgs result];

		[pPool release];
	}

	return pRet;
}

// -----------------------------------------------------------------------

sal_uLong JavaSalFrame::GetState()
{
	sal_uLong nRet = 0;

	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLWindowWrapperArgs *pGetStateArgs = [VCLWindowWrapperArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(getState:) withObject:pGetStateArgs waitUntilDone:YES modes:pModes];
		NSNumber *pState = (NSNumber *)[pGetStateArgs result];
		if ( pState )
			nRet = [pState unsignedLongValue];

		[pPool release];
	}

	return nRet;
}

// -----------------------------------------------------------------------

void JavaSalFrame::MakeModal()
{
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(makeModal:) withObject:mpWindow waitUntilDone:YES modes:pModes];

		[pPool release];
	}
}

// -----------------------------------------------------------------------

bool JavaSalFrame::RequestFocus()
{
	bool bRet = false;

	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLWindowWrapperArgs *pRequestFocusArgs = [VCLWindowWrapperArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(requestFocus:) withObject:pRequestFocusArgs waitUntilDone:YES modes:pModes];
		NSNumber *pResult = (NSNumber *)[pRequestFocusArgs result];
		if ( pResult && [pResult boolValue] )
			bRet = true;

		[pPool release];
	}

	return bRet;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetState( sal_uLong nFrameState )
{
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		sal_uLong nState;
		if ( nFrameState & WINDOWSTATE_STATE_MINIMIZED )
			nState = WINDOWSTATE_STATE_MINIMIZED;
		else if ( nFrameState & WINDOWSTATE_STATE_MAXIMIZED )
			nState = WINDOWSTATE_STATE_MAXIMIZED;
		else
			nState = WINDOWSTATE_STATE_NORMAL;

		VCLWindowWrapperArgs *pSetStateArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithUnsignedLong:nState]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(setState:) withObject:pSetStateArgs waitUntilDone:YES modes:pModes];

		[pPool release];
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetVisible( bool bVisible, bool bNoActivate )
{
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLWindowWrapperArgs *pSetVisibleArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithBool:bVisible], [NSNumber numberWithBool:bNoActivate], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(setVisible:) withObject:pSetVisibleArgs waitUntilDone:YES modes:pModes];

		[pPool release];
	}
}

// -----------------------------------------------------------------------

bool JavaSalFrame::ToFront()
{
	bool bRet = false;

	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLWindowWrapperArgs *pToFrontArgs = [VCLWindowWrapperArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(toFront:) withObject:pToFrontArgs waitUntilDone:YES modes:pModes];
		NSNumber *pResult = (NSNumber *)[pToFrontArgs result];
		if ( pResult && [pResult boolValue] )
			bRet = true;

		[pPool release];
	}

	return bRet;
}

// -----------------------------------------------------------------------

void JavaSalFrame::UpdateLayer()
{
	CGSize aExpectedSize = CGSizeMake( maGeometry.nWidth, maGeometry.nHeight );
	if ( maFrameLayer && maSysData.mpNSView && CGSizeEqualToSize( CGLayerGetSize( maFrameLayer ), aExpectedSize ) )
		return;

	if ( maFrameLayer )
	{
		CGLayerRelease( maFrameLayer );
		maFrameLayer = NULL;
	}

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	VCLViewGetGraphicsLayer *pVCLViewGetGraphicsLayer = [VCLViewGetGraphicsLayer createGraphicsLayer:mpGraphics view:maSysData.mpNSView];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLViewGetGraphicsLayer performSelectorOnMainThread:@selector(getGraphicsLayer:) withObject:pVCLViewGetGraphicsLayer waitUntilDone:YES modes:pModes];
	maFrameLayer = [pVCLViewGetGraphicsLayer layer];
	if ( maFrameLayer )
		CGLayerRetain( maFrameLayer );

	[pPool release];

	if ( maFrameLayer )
	{
		CGSize aLayerSize = CGLayerGetSize( maFrameLayer );
		mpGraphics->maNativeBounds = CGRectMake( 0, 0, aLayerSize.width, aLayerSize.height );
		mpGraphics->setLayer( maFrameLayer );
		if ( mbFullScreen )
			mpGraphics->setBackgroundColor( 0xff000000 );
		else
			mpGraphics->setBackgroundColor( 0xffffffff );

		// If the layer size differs from the expected size, the window size is
		// changing so post a SALEVENT_MOVERESIZE event to notify the OOo code
		// of the change
		if ( !CGSizeEqualToSize( aLayerSize, aExpectedSize ) )
		{
			JavaSalEvent *pMoveResizeEvent = new JavaSalEvent( SALEVENT_MOVERESIZE, this, NULL );
			JavaSalEventQueue::postCachedEvent( pMoveResizeEvent );
			pMoveResizeEvent->release();
		}

		// Post a paint event
		JavaSalEvent *pPaintEvent = new JavaSalEvent( SALEVENT_PAINT, this, new SalPaintEvent( 0, 0, aLayerSize.width, aLayerSize.height ) );
		JavaSalEventQueue::postCachedEvent( pPaintEvent );
		pPaintEvent->release();
	}
	else
	{
		mpGraphics->maNativeBounds = CGRectNull;
		mpGraphics->setLayer( NULL );
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::AddTrackingRect( Window *pWindow )
{
	// If the frame is not visible, do nothing as it will result in flipped
	// tracking area bounds
	if ( pWindow && mpWindow && mbVisible )
	{
		CGRect aRect = UnflipFlippedRect( CGRectMake( pWindow->GetOutOffXPixel(), pWindow->GetOutOffYPixel(), pWindow->GetOutputWidthPixel(), pWindow->GetOutputHeightPixel() ), mpGraphics->maNativeBounds );
		if ( !CGRectIsEmpty( aRect ) )
		{
			NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

			VCLWindowWrapperArgs *pAddTrackingAreaArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObjects:[NSValue valueWithPointer:pWindow], [NSValue valueWithRect:NSRectFromCGRect( aRect )], nil]];
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[mpWindow performSelectorOnMainThread:@selector(addTrackingArea:) withObject:pAddTrackingAreaArgs waitUntilDone:YES modes:pModes];

			[pPool release];
		}
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::RemoveTrackingRect( Window *pWindow )
{
	if ( pWindow && mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLWindowWrapperArgs *pRemoveTrackingAreaArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObject:[NSValue valueWithPointer:pWindow]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(removeTrackingArea:) withObject:pRemoveTrackingAreaArgs waitUntilDone:YES modes:pModes];

		[pPool release];
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetMovable( bool bMoveable )
{
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLWindowWrapperArgs *pSetMovableArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithBool:bMoveable]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(setMovable:) withObject:pSetMovableArgs waitUntilDone:YES modes:pModes];

		[pPool release];
	}
}

// -----------------------------------------------------------------------

SalGraphics* JavaSalFrame::AcquireGraphics()
{
	if ( mbGraphics )
		return NULL;

	mbGraphics = sal_True;

	return mpGraphics;
}

// -----------------------------------------------------------------------

void JavaSalFrame::ReleaseGraphics( SalGraphics* pGraphics )
{
	if ( pGraphics != mpGraphics )
		return;

	mbGraphics = sal_False;
}

// -----------------------------------------------------------------------

bool JavaSalFrame::PostEvent( void *pData )
{
	JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_USEREVENT, this, pData );
	JavaSalEventQueue::postCachedEvent( pEvent );
	pEvent->release();
	return true;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetTitle( const OUString& rTitle )
{
	maTitle = rTitle;
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSString *pTitle = [NSString stringWithCharacters:maTitle.getStr() length:maTitle.getLength()];
		VCLWindowWrapperArgs *pSetTitleArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObject:pTitle]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(setTitle:) withObject:pSetTitleArgs waitUntilDone:YES modes:pModes];

		[pPool release];
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetIcon( sal_uInt16 /* nIcon */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::SetIcon not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalFrame::Show( bool bVisible, bool bNoActivate )
{
	// Don't allow floating children of a show only menus frame to ever show
	if ( mpParent && mpParent->mbShowOnlyMenus && IsFloatingFrame() )
		bVisible = sal_False;

	if ( bVisible == mbVisible )
		return;

	SalData *pSalData = GetSalData();

	// Fix bug 2501 by closing any dialogs that are child windows of this
	// window. This is necessary because in a few cases, the OOo code closes
	// a dialog's parent and expects the dialog to still remain showing!
	// Java, on the other hand, will forcefully close the window leaving the
	// the OOo code in an irrecoverable state.
	if ( !bVisible )
	{
		// Reset the frame's menu update list
		maUpdateMenuList.clear();

		// Close any attached objects
		::std::list< JavaSalObject* > aObjects( maVisibleObjects );
		for ( ::std::list< JavaSalObject* >::const_iterator it = aObjects.begin(); it != aObjects.end(); ++it )
			(*it)->Show( false );

		Window *pWindow = Application::GetFirstTopLevelWindow();
		while ( pWindow && pWindow->ImplGetFrame() != this )
			pWindow = Application::GetNextTopLevelWindow( pWindow );

		if ( pWindow )
		{
			// Fix bug 3356 without causing bugs 2501 or 3398 by ending all
			// dialogs whenever any of this frame's children are visible
			::std::list< JavaSalFrame* > aChildren( maChildren );
			for ( ::std::list< JavaSalFrame* >::const_iterator it = aChildren.begin(); it != aChildren.end(); ++it )
			{
				if ( (*it)->mbVisible )
				{
					Dialog::EndAllDialogs( pWindow );
					break;
				}
			}
		}
	}
	// Fix bug 3153 by setting parent to the focus frame for dialogs that
	// have a show only menus frame as their parent
	else if ( mpParent && mpParent->mbShowOnlyMenus && mpParent != pSalData->mpFocusFrame && !IsUtilityWindow() )
	{
		SetParent( pSalData->mpFocusFrame );
	}

	mbVisible = bVisible;

	SetVisible( mbVisible, bNoActivate );

	sal_Bool bTopLevelWindow = sal_False;
	if ( mbVisible )
	{
		mbInShow = sal_True;

		// Fix bug 3228 by setting the OOo modal dialogs to the native modal
		// window level
		ImplSVData *pSVData = ImplGetSVData();
		if ( pSVData->maWinData.mpLastExecuteDlg )
		{
			SystemWindow *pSystemWindow = pSVData->maWinData.mpLastExecuteDlg->GetSystemWindow();
			if ( pSystemWindow )
			{
				JavaSalFrame *pModalFrame = (JavaSalFrame *)pSystemWindow->ImplGetFrame();
				if ( pModalFrame && pModalFrame->mbVisible )
				{
					while ( pModalFrame->mpParent && pModalFrame->mpParent->mbVisible )
						pModalFrame = pModalFrame->mpParent;

					JavaSalFrame *pFrame = this;
					while ( pModalFrame != this && pFrame->mpParent && pFrame->mpParent->mbVisible )
						pFrame = pFrame->mpParent;

					if ( pModalFrame == pFrame )
						MakeModal();
				}
			}
		}

		UpdateMenusForFrame( this, NULL, false );

		// Get native window's content view since it won't be created until
		// first shown
		if ( !mpParent && !IsFloatingFrame() && !IsUtilityWindow() )
		{
			Window *pWindow = Application::GetFirstTopLevelWindow();
			while ( pWindow && pWindow->ImplGetFrame() != this )
				pWindow = Application::GetNextTopLevelWindow( pWindow );

			if ( pWindow )
				bTopLevelWindow = sal_True;
		}

		maSysData.mpNSView = (NSView *)GetNativeWindowContentView( bTopLevelWindow );
		mbCenter = sal_False;

		JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_MOVERESIZE, this, NULL );
		pEvent->dispatch();
		pEvent->release();

		// Reattach floating children
		::std::list< JavaSalFrame* > aChildren( maChildren );
		for ( ::std::list< JavaSalFrame* >::const_iterator it = aChildren.begin(); it != aChildren.end(); ++it )
		{
			if ( (*it)->mbVisible )
				(*it)->SetParent( this );
		}

		// Reattach visible objects
		::std::list< JavaSalObject* > aReshowObjects( maVisibleObjects );
		for ( ::std::list< JavaSalObject* >::const_iterator it = aReshowObjects.begin(); it != aReshowObjects.end(); ++it )
			(*it)->Show( true );

		// Explicitly set focus to this frame since Java may set the focus
		// to the child frame
		if ( !bNoActivate )
			ToTop( SAL_FRAME_TOTOP_RESTOREWHENMIN | SAL_FRAME_TOTOP_GRABFOCUS );

		mbInShow = sal_False;
	}
	else
	{
		// End composition
		JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_ENDEXTTEXTINPUT, this, NULL );
		pEvent->dispatch();
		pEvent->release();

		// Remove the native window since it is destroyed when hidden
		maSysData.mpNSView = NULL;

		// Unset focus but don't set focus to another frame as it will cause
		// menu shortcuts to be disabled if we go into show only menus mode
		// after closing a window whose child window had focus
		if ( pSalData->mpFocusFrame == this )
		{
			JavaSalEvent *pFocusEvent = new JavaSalEvent( SALEVENT_LOSEFOCUS, this, NULL );
			pFocusEvent->dispatch();
			pFocusEvent->release();
		}

		if ( pSalData->mpLastDragFrame == this )
			pSalData->mpLastDragFrame = NULL;

		if ( pSalData->mpLastMouseMoveFrame == this )
			pSalData->mpLastMouseMoveFrame = NULL;

		if ( maFrameLayer )
		{
			CGLayerRelease( maFrameLayer );
			maFrameLayer = NULL;
		}

		mpGraphics->setLayer( NULL );

		// Fix bug 3032 by showing one of the show only frames if no other
		// non-floating windows are visible
		JavaSalFrame *pShowOnlyMenusFrame = NULL;
		for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
		{
			if ( (*it)->mbShowOnlyMenus )
			{
				pShowOnlyMenusFrame = *it;
			}
			else if ( (*it)->mbVisible && !(*it)->IsFloatingFrame() )
			{
				pShowOnlyMenusFrame = NULL;
				break;
			}
		}

		if ( pShowOnlyMenusFrame && pShowOnlyMenusFrame != this )
		{
			// Fix empty menubar when the help window is displayed and then
			// closed and the show only menus frame's menubar fails to be
			// set by ensuring that the show only menus frame gets focus
			if ( pShowOnlyMenusFrame->mbVisible )
				pShowOnlyMenusFrame->ToTop( SAL_FRAME_TOTOP_RESTOREWHENMIN | SAL_FRAME_TOTOP_GRABFOCUS );
			else
				pShowOnlyMenusFrame->Show( true, false );
		}
	}

	UpdateLayer();

	// Fix bug reported in the following NeoOffice forum post by forcing
	// the window into full screen mode if the app is already in full
	// screen mode:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=65002#65002
	if ( mbVisible && bTopLevelWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSWindow *pNSWindow = (NSWindow *)GetNativeWindow();
		if ( pNSWindow )
		{
			VCLToggleFullScreen *pVCLToggleFullScreen = [VCLToggleFullScreen createToggleFullScreen:pNSWindow toggleToCurrentScreenMode:YES];
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pVCLToggleFullScreen performSelectorOnMainThread:@selector(toggleFullScreen:) withObject:pVCLToggleFullScreen waitUntilDone:YES modes:pModes];
		}

		[pPool release];
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetMinClientSize( long nWidth, long nHeight )
{
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSSize aMinSize = NSMakeSize( nWidth, nHeight );
		VCLWindowWrapperArgs *pSetMinSizeArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObject:[NSValue valueWithSize:aMinSize]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(setMinSize:) withObject:pSetMinSizeArgs waitUntilDone:YES modes:pModes];

		[pPool release];
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags )
{
	if ( mnStyle & SAL_FRAME_STYLE_SYSTEMCHILD )
		return;

	mbInSetPosSize = sal_True;

	Rectangle aPosSize( Point( maGeometry.nX - maGeometry.nLeftDecoration, maGeometry.nY - maGeometry.nTopDecoration ), Size( maGeometry.nWidth, maGeometry.nHeight ) );

	if ( ! ( nFlags & SAL_FRAME_POSSIZE_X ) )
		nX = aPosSize.Left();
	if ( ! ( nFlags & SAL_FRAME_POSSIZE_Y ) )
		nY = aPosSize.Top();
	if ( ! ( nFlags & SAL_FRAME_POSSIZE_WIDTH ) )
		nWidth = aPosSize.GetWidth();
	if ( ! ( nFlags & SAL_FRAME_POSSIZE_HEIGHT ) )
		nHeight = aPosSize.GetHeight();

	// Adjust position for RTL layout
	long nParentX = 0;
	long nParentY = 0;
	if ( mpParent )
	{
		Rectangle aParentBounds( mpParent->GetBounds() );
		nParentX = aParentBounds.Left() + mpParent->maGeometry.nLeftDecoration;
		nParentY = aParentBounds.Top() + mpParent->maGeometry.nTopDecoration;

		if ( nFlags & ( SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y ) && Application::GetSettings().GetLayoutRTL() )
			nX = mpParent->maGeometry.nWidth - nWidth - nX - 1;

		if ( nFlags & SAL_FRAME_POSSIZE_X )
			nX += nParentX;
		if ( nFlags & SAL_FRAME_POSSIZE_Y )
			nY += nParentY;
	}

	Rectangle aWorkArea;
	if ( mbCenter && ! ( nFlags & ( SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y ) ) )
	{
		if ( mpParent && (long)mpParent->maGeometry.nWidth >= nWidth && (long)mpParent->maGeometry.nHeight > nHeight)
		{
			nX = nParentX + ( mpParent->maGeometry.nWidth - nWidth ) / 2;
			nY = nParentY + ( mpParent->maGeometry.nHeight - nHeight ) / 2;

			aWorkArea = Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );
			GetWorkArea( aWorkArea );
		}
		else
		{
			aWorkArea = Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );
			GetWorkArea( aWorkArea );

			nX = aWorkArea.Left() + ( ( aWorkArea.GetWidth() - nWidth ) / 2 );
			nY = aWorkArea.Top() + ( ( aWorkArea.GetHeight() - nHeight ) / 2 );
		}

		mbCenter = sal_False;
	}
	else
	{
		aWorkArea = Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );
		GetWorkArea( aWorkArea );

		// Make sure that the work area intersects with the parent frame
		// so that dialogs don't show on a different monitor than the parent
		if ( mpParent )
		{
			Rectangle aParentBounds( mpParent->GetBounds() );
			if ( aWorkArea.GetIntersection( aParentBounds ).IsEmpty() )
			{
				aWorkArea = aParentBounds;
				GetWorkArea( aWorkArea );
			}
		}

		// Fix misplaced popup window when clicking the Line Spacing toolbar
		// button in Writer by not centering once both the X and Y are set
		if ( mbCenter && nFlags & ( SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y ) )
			mbCenter = sal_False;
	}

	// Make sure window does not spill off of the screen
	long nMinX = aWorkArea.Left();
	long nMinY = aWorkArea.Top();
	if ( mbPresentation )
	{
		nMinX -= 1;
		nMinY -= 1;
	}
	nWidth += maGeometry.nLeftDecoration + maGeometry.nRightDecoration;
	nHeight += maGeometry.nTopDecoration + maGeometry.nBottomDecoration;
	if ( nMinX + nWidth > aWorkArea.Left() + aWorkArea.GetWidth() )
		nWidth = aWorkArea.Left() + aWorkArea.GetWidth() - nMinX;
	if ( nMinY + nHeight > aWorkArea.Top() + aWorkArea.GetHeight() )
		nHeight = aWorkArea.Top() + aWorkArea.GetHeight() - nMinY;
	if ( nX < nMinX )
		nX = nMinX;
	if ( nY < nMinY )
		nY = nMinY;

	// Fix bug 1420 by not restricting width or height to work area for current
	// drag frame
	if ( this != GetSalData()->mpLastDragFrame )
	{
		if ( nX + nWidth > aWorkArea.Left() + aWorkArea.GetWidth() )
			nX = aWorkArea.Left() + aWorkArea.GetWidth() - nWidth;
		if ( nY + nHeight > aWorkArea.Top() + aWorkArea.GetHeight() )
			nY = aWorkArea.Top() + aWorkArea.GetHeight() - nHeight;
	}

	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSRect aFrame = NSMakeRect( nX, nY, nWidth, nHeight );
		VCLWindowWrapperArgs *pSetFrameArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObjects:[NSValue valueWithRect:aFrame], [NSNumber numberWithBool:mbInSetWindowState], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(setJavaFrame:) withObject:pSetFrameArgs waitUntilDone:YES modes:pModes];

		[pPool release];
	}

	// Update the cached position immediately
	unsigned long nOrigHeight = maGeometry.nHeight;
	JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_MOVERESIZE, this, NULL );
	pEvent->dispatch();
	pEvent->release();

	// When fitting most non-resizeable dialogs to the visible screen area,
	// the OOo code will not change the positions of the controls in dialogs
	// such as the File > Export as PDF menu item's dialog. This, in turn, will
	// hide any buttons and other controls at the bottom of the dialog so we
	// resize the height to fit all of the dialog's children so that the user
	// might be able to see all of the controls if they hide the OS X Dock or
	// drag the dialog to another monitor.
	if ( maGeometry.nHeight > nOrigHeight && mnStyle & ( SAL_FRAME_STYLE_DEFAULT | SAL_FRAME_STYLE_MOVEABLE ) && ! ( mnStyle & SAL_FRAME_STYLE_SIZEABLE ) )
	{
		Window *pWindow = Application::GetFirstTopLevelWindow();
		while ( pWindow && pWindow->ImplGetFrame() != this )
			pWindow = Application::GetNextTopLevelWindow( pWindow );

		if ( pWindow )
		{
			long nMinHeight = nOrigHeight;
			sal_uInt16 nCount = pWindow->GetChildCount();
			for ( sal_uInt16 i = 0; i < nCount; i++ )
			{
				Window *pChildWindow = pWindow->GetChild( i );
				if ( pChildWindow && pChildWindow->IsVisible() )
				{
					long nChildBottom = pChildWindow->GetOutOffYPixel() + pChildWindow->GetOutputHeightPixel();
					if ( nMinHeight < nChildBottom )
						nMinHeight = nChildBottom;
				}
			}

			if ( nMinHeight > (long)maGeometry.nHeight )
			{
				nHeight = nMinHeight + maGeometry.nTopDecoration + maGeometry.nBottomDecoration;

				NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

				NSRect aFrame = NSMakeRect( nX, nY, nWidth, nHeight );
				VCLWindowWrapperArgs *pSetFrameArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObjects:[NSValue valueWithRect:aFrame], [NSNumber numberWithBool:mbInSetWindowState], nil]];
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				[mpWindow performSelectorOnMainThread:@selector(setJavaFrame:) withObject:pSetFrameArgs waitUntilDone:YES modes:pModes];

				[pPool release];
			}

			// Update the cached position immediately
			pEvent = new JavaSalEvent( SALEVENT_MOVERESIZE, this, NULL );
			pEvent->dispatch();
			pEvent->release();
		}
	}

	mbInSetPosSize = sal_False;
}

// -----------------------------------------------------------------------

void JavaSalFrame::GetWorkArea( Rectangle &rRect )
{
	SalData *pSalData = GetSalData();
	// Fix unexpected resizing of window to the screen's visible bounds
	// reported in thefollowing NeoOffice forum topic by using the full screen
	// bounds when in full screen mode:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8663
	sal_Bool bFullScreenMode = ( mbFullScreen || pSalData->mpPresentationFrame || ( this == pSalData->mpLastDragFrame ) );
	long nX = rRect.Left();
	long nY = rRect.Top();
	long nWidth = rRect.GetWidth();
	long nHeight = rRect.GetHeight();
    
	// Use the parent frame's bounds if there is one. Fix bug 3163 by always
	// excluding the frame's insets
	if ( mpParent )
	{
		nX = mpParent->maGeometry.nX;
		nY = mpParent->maGeometry.nY;
		nWidth = mpParent->maGeometry.nWidth;
		nHeight = mpParent->maGeometry.nHeight;
	}
	else
	{
		nX += maGeometry.nLeftDecoration;
		nY += maGeometry.nTopDecoration;
		nWidth -= maGeometry.nLeftDecoration + maGeometry.nRightDecoration;
		nHeight -= maGeometry.nTopDecoration + maGeometry.nBottomDecoration;
	}

	Rectangle aRect( JavaSalFrame::GetScreenBounds( nX, nY, nWidth, nHeight, bFullScreenMode ) );
	if ( aRect.GetWidth() > 0 && aRect.GetHeight() > 0 )
		rRect = aRect;
}

// -----------------------------------------------------------------------

void JavaSalFrame::GetClientSize( long& rWidth, long& rHeight )
{
	rWidth = maGeometry.nWidth;
	rHeight = maGeometry.nHeight;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetWindowState( const SalFrameState* pState )
{
	sal_Bool bOldInSetWindowState = mbInSetWindowState;
	mbInSetWindowState = sal_True;

	sal_uInt16 nFlags = 0;
	if ( pState->mnMask & WINDOWSTATE_MASK_X )
		nFlags |= SAL_FRAME_POSSIZE_X;
	if ( pState->mnMask & WINDOWSTATE_MASK_Y )
		nFlags |= SAL_FRAME_POSSIZE_Y;
	if ( pState->mnMask & WINDOWSTATE_MASK_WIDTH )
		nFlags |= SAL_FRAME_POSSIZE_WIDTH;
	if ( pState->mnMask & WINDOWSTATE_MASK_HEIGHT )
		nFlags |= SAL_FRAME_POSSIZE_HEIGHT;
	if ( nFlags )
	{
		JavaSalFrame *pParent = mpParent;
		mpParent = NULL;
		SetPosSize( pState->mnX, pState->mnY, pState->mnWidth, pState->mnHeight, nFlags );
		mpParent = pParent;
	}

	// Fix bug 3078 by setting the state after setting the size
	if ( pState->mnMask & WINDOWSTATE_MASK_STATE )
		SetState( pState->mnState );

	mbInSetWindowState = bOldInSetWindowState;
}

// -----------------------------------------------------------------------

bool JavaSalFrame::GetWindowState( SalFrameState* pState )
{
	Rectangle aBounds( GetBounds( NULL, NULL, sal_True ) );
	pState->mnMask = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT | WINDOWSTATE_MASK_STATE;
	pState->mnX = aBounds.Left();
	pState->mnY = aBounds.Top();
	pState->mnWidth = aBounds.GetWidth() - maGeometry.nLeftDecoration - maGeometry.nRightDecoration;
	pState->mnHeight = aBounds.GetHeight() - maGeometry.nTopDecoration - maGeometry.nBottomDecoration;
	pState->mnState = GetState();

	// Fix bug 3012 by returning false if the frame size is not larger than
	// the frame's minimum size
	if ( !maGeometry.nWidth || !maGeometry.nHeight )
		return false;
	else
		return true;
}

// -----------------------------------------------------------------------

void JavaSalFrame::ShowFullScreen( bool bFullScreen, sal_Int32 nDisplay )
{
	if ( mbInShowFullScreen || bFullScreen == mbFullScreen )
		return;

	mbInShowFullScreen = sal_True;
	if ( !mbInWindowDidExitFullScreen && !mbInWindowWillEnterFullScreen )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSWindow *pNSWindow = (NSWindow *)GetNativeWindow();
		if ( pNSWindow )
		{
			VCLToggleFullScreen *pVCLToggleFullScreen = [VCLToggleFullScreen createToggleFullScreen:pNSWindow toggleToCurrentScreenMode:NO];
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pVCLToggleFullScreen performSelectorOnMainThread:@selector(toggleFullScreen:) withObject:pVCLToggleFullScreen waitUntilDone:YES modes:pModes];
		}

		[pPool release];

		// Do not set frame bounds when entering or exiting full screen mode
		// as that will cause the original size to be lost when a window that
		// is in full screen mode transitions to the versions browser
		sal_uInt16 nFlags = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT;
		if ( bFullScreen )
		{
			memcpy( &maOriginalGeometry, &maGeometry, sizeof( SalFrameGeometry ) );

			Rectangle aWorkArea( JavaSalFrame::GetScreenBounds( nDisplay, sal_True ) );
			if ( aWorkArea.IsEmpty() )
				aWorkArea = Rectangle( Point( maGeometry.nX - maGeometry.nLeftDecoration, maGeometry.nY - maGeometry.nTopDecoration ), Size( maGeometry.nWidth, maGeometry.nHeight ) );
			GetWorkArea( aWorkArea );
			SetPosSize( aWorkArea.Left(), aWorkArea.Top(), aWorkArea.GetWidth() - maGeometry.nLeftDecoration - maGeometry.nRightDecoration, aWorkArea.GetHeight() - maGeometry.nTopDecoration - maGeometry.nBottomDecoration, nFlags );
		}
		else
		{
			SetPosSize( maOriginalGeometry.nX - maOriginalGeometry.nLeftDecoration, maOriginalGeometry.nY - maOriginalGeometry.nTopDecoration, maOriginalGeometry.nWidth, maOriginalGeometry.nHeight, nFlags );
			memset( &maOriginalGeometry, 0, sizeof( SalFrameGeometry ) );
		}
	}

	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLWindowWrapperArgs *pSetFullScreenModeArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithBool:bFullScreen]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(setFullScreenMode:) withObject:pSetFullScreenModeArgs waitUntilDone:YES modes:pModes];

		[pPool release];
	}

	mbFullScreen = bFullScreen;
	mbInShowFullScreen = sal_False;

	if ( maFrameLayer )
	{
		if ( mbFullScreen )
			mpGraphics->setBackgroundColor( 0xff000000 );
		else
			mpGraphics->setBackgroundColor( 0xffffffff );
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::StartPresentation( bool bStart )
{
	if ( bStart == mbPresentation )
		return;

	SalData *pSalData = GetSalData();

	// Only allow one frame to be in presentation mode at any one time
	if ( bStart && pSalData->mpPresentationFrame )
		return;
	else if ( !bStart && pSalData->mpPresentationFrame != this )
		return;

	mbPresentation = bStart;

	if ( mbPresentation )
		pSalData->mpPresentationFrame = this;
	else
		pSalData->mpPresentationFrame = NULL;

	// Adjust window size if in full screen mode
	if ( mbFullScreen )
	{
		sal_uInt16 nFlags = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT;

		Rectangle aWorkArea( Point( maGeometry.nX - maGeometry.nLeftDecoration, maGeometry.nY - maGeometry.nTopDecoration ), Size( maGeometry.nWidth, maGeometry.nHeight ) );
		GetWorkArea( aWorkArea );

		SetPosSize( aWorkArea.Left(), aWorkArea.Top(), aWorkArea.GetWidth() - maGeometry.nLeftDecoration - maGeometry.nRightDecoration, aWorkArea.GetHeight() - maGeometry.nTopDecoration - maGeometry.nBottomDecoration, nFlags );
	}

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	// [ed] 2/15/05 Change the SystemUIMode via timers so we can trigger
	// it on the main runloop thread.  Bug 484
	// Always run the timer to ensure that the remote control feature works
	// when the presentation window is on a secondary screen
	VCLSetSystemUIMode *pVCLSetSystemUIMode = [VCLSetSystemUIMode createFullScreen:mbPresentation];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLSetSystemUIMode performSelectorOnMainThread:@selector(setSystemUIMode:) withObject:pVCLSetSystemUIMode waitUntilDone:YES modes:pModes];

	[pPool release];

}

// -----------------------------------------------------------------------

void JavaSalFrame::SetAlwaysOnTop( bool /* bOnTop */ )
{
}

// -----------------------------------------------------------------------

void JavaSalFrame::ToTop( sal_uInt16 nFlags )
{
	// Make sure frame is a top-level window
	JavaSalFrame *pFrame = this;
	while ( pFrame && pFrame->IsFloatingFrame() && pFrame->mpParent && pFrame->mpParent->mbVisible )
		pFrame = pFrame->mpParent;

	if ( !pFrame || pFrame->IsFloatingFrame() || !pFrame->mbVisible )
		return;

	bool bSuccess = false;
	if ( nFlags & SAL_FRAME_TOTOP_GRABFOCUS )
	{
		bSuccess = pFrame->ToFront();
	}
	else if ( nFlags & SAL_FRAME_TOTOP_GRABFOCUS_ONLY )
	{
		// Fix bug 3193 by invoking toFront() if the frame is a modal dialog
		bool bModal = false;
		ImplSVData *pSVData = ImplGetSVData();
		if ( pSVData->maWinData.mpLastExecuteDlg )
		{
			SystemWindow *pSystemWindow = pSVData->maWinData.mpLastExecuteDlg->GetSystemWindow();
			if ( pSystemWindow && pSystemWindow->ImplGetFrame() == pFrame )
				bModal = true;
		}

		if ( bModal || nFlags & SAL_FRAME_TOTOP_RESTOREWHENMIN )
			bSuccess = pFrame->ToFront();
		else
			bSuccess = pFrame->RequestFocus();
	}
	else if ( nFlags & SAL_FRAME_TOTOP_RESTOREWHENMIN )
	{
		bSuccess = pFrame->Deminimize();
	}
	else
	{
		bSuccess = false;
	}

	// If Java has set the focus, update it now in the OOo code as it may
	// take a while before the Java event shows up in the queue. Fix bug
	// 1203 by not doing this update if we are in the Show() method.
	if ( bSuccess && !mbInShow )
	{
		JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_GETFOCUS, pFrame, NULL );
		pEvent->dispatch();
		pEvent->release();
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetPointer( PointerStyle ePointerStyle )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	VCLSetCursor *pVCLSetCursor = [VCLSetCursor createWithPointerStyle:ePointerStyle windowWrapper:mpWindow];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLSetCursor performSelectorOnMainThread:@selector(setCursor:) withObject:pVCLSetCursor waitUntilDone:NO modes:pModes];

	[pPool release];
}

// -----------------------------------------------------------------------

void JavaSalFrame::CaptureMouse( bool bCapture )
{
	SalData *pSalData = GetSalData();
	if ( bCapture )
		pSalData->mpCaptureFrame = this;
	else
		pSalData->mpCaptureFrame = NULL;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetPointerPos( long /* nX */, long /* nY */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::SetPointerPos not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalFrame::Flush()
{
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(flush:) withObject:mpWindow waitUntilDone:NO modes:pModes];

		[pPool release];
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::Flush( const Rectangle& /* rRect */ )
{
	Flush();
}

// -----------------------------------------------------------------------

void JavaSalFrame::Sync()
{
	Flush();
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetInputContext( SalInputContext* pContext )
{
	// Only allow Mac OS X key bindings when the OOo application code says so
	if ( pContext && pContext->mnOptions & SAL_INPUTCONTEXT_TEXT )
		mbAllowKeyBindings = true;
	else
		mbAllowKeyBindings = false;
}

// -----------------------------------------------------------------------

void JavaSalFrame::EndExtTextInput( sal_uInt16 /* nFlags */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::EndExtTextInput not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

OUString JavaSalFrame::GetKeyName( sal_uInt16 nKeyCode )
{
	OUString aRet;

	OUString aKeyName( ConvertVCLKeyCode( nKeyCode, false ) );
	if ( aKeyName.getLength() )
	{
		if ( nKeyCode & KEY_SHIFT )
			aRet += OUString( (sal_Unicode)0x21e7 );
		if ( nKeyCode & KEY_MOD3 )
			aRet += OUString( (sal_Unicode)0x2303 );
		if ( nKeyCode & KEY_MOD1 )
			aRet += OUString( (sal_Unicode)0x2318 );
		if ( nKeyCode & KEY_MOD2 )
			aRet += OUString( (sal_Unicode)0x2325 );

		aRet += aKeyName;
	}

	return aRet;
}

// -----------------------------------------------------------------------

void JavaSalFrame::UpdateSettings( AllSettings& rSettings )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	MouseSettings aMouseSettings = rSettings.GetMouseSettings();
	NSTimeInterval fDoubleClickInterval = [NSEvent doubleClickInterval];
	if ( fDoubleClickInterval < 0.25 )
		fDoubleClickInterval = 0.25;
	aMouseSettings.SetDoubleClickTime( fDoubleClickInterval * 1000 );
	aMouseSettings.SetStartDragWidth( 6 );
	aMouseSettings.SetStartDragHeight( 6 );
	rSettings.SetMouseSettings( aMouseSettings );

	StyleSettings aStyleSettings( rSettings.GetStyleSettings() );

	long nBlinkRate = 500;
	NSUserDefaults *pDefaults = [NSUserDefaults standardUserDefaults];
	if ( pDefaults )
	{
		nBlinkRate = [pDefaults integerForKey:@"NSTextInsertionPointBlinkPeriod"];
		if ( nBlinkRate < 500 )
			nBlinkRate = 500;
	}
	aStyleSettings.SetCursorBlinkTime( nBlinkRate );

	// Update colors if any system colors have not yet been set
	InitializeSystemColors();

	MutexGuard aGuard( aSystemColorsMutex );
	if ( pVCLTextColor )
	{
		Color aTextColor( *pVCLTextColor );
		Color aThemeDialogColor = aTextColor;
		if ( pVCLControlTextColor )
			aThemeDialogColor = Color( *pVCLControlTextColor );

		aStyleSettings.SetDialogTextColor( aThemeDialogColor );
		aStyleSettings.SetMenuTextColor( aTextColor );
		aStyleSettings.SetMenuBarTextColor( aTextColor );
		aStyleSettings.SetMenuBarRolloverTextColor( aTextColor );
		aStyleSettings.SetButtonTextColor( aThemeDialogColor );
		aStyleSettings.SetRadioCheckTextColor( aThemeDialogColor );
		aStyleSettings.SetGroupTextColor( aThemeDialogColor );
		aStyleSettings.SetLabelTextColor( aThemeDialogColor );
		aStyleSettings.SetInfoTextColor( aTextColor );
		aStyleSettings.SetWindowTextColor( aTextColor );
		aStyleSettings.SetFieldTextColor( aTextColor );
	}

	if ( pVCLHighlightColor )
	{
		Color aHighlightColor( *pVCLHighlightColor );

		aStyleSettings.SetActiveBorderColor( aHighlightColor );
		aStyleSettings.SetActiveColor( aHighlightColor );
		aStyleSettings.SetActiveTextColor( aHighlightColor );
		aStyleSettings.SetHighlightColor( aHighlightColor );
		aStyleSettings.SetMenuHighlightColor( pVCLSelectedMenuItemColor ? Color( *pVCLSelectedMenuItemColor ) : aHighlightColor );
	}

	if ( pVCLHighlightTextColor )
	{
		Color aHighlightTextColor( *pVCLHighlightTextColor );

		aStyleSettings.SetHighlightTextColor( aHighlightTextColor );
		aStyleSettings.SetMenuHighlightTextColor( pVCLSelectedMenuItemTextColor ? Color( *pVCLSelectedMenuItemTextColor ) : aHighlightTextColor );
	}

	if ( pVCLBackColor )
	{
		Color aBackColor( *pVCLBackColor );
		Color aThemeDialogColor = aBackColor;
		if ( pVCLDisabledControlTextColor )
			aThemeDialogColor = Color( *pVCLDisabledControlTextColor );

		aStyleSettings.Set3DColors( aBackColor );
		aStyleSettings.SetDeactiveBorderColor( aBackColor );
		aStyleSettings.SetDeactiveColor( aBackColor );
		aStyleSettings.SetDeactiveTextColor( aThemeDialogColor );
		aStyleSettings.SetDialogColor( aBackColor );
		aStyleSettings.SetDisableColor( aBackColor );
		aStyleSettings.SetFaceColor( aBackColor );
		aStyleSettings.SetInactiveTabColor( aBackColor );
		aStyleSettings.SetLightBorderColor( aBackColor );
		aStyleSettings.SetMenuColor( aBackColor );
		aStyleSettings.SetMenuBarColor( aBackColor );

		if( aBackColor == COL_LIGHTGRAY )
		{
			aStyleSettings.SetCheckedColor( Color( 0xCC, 0xCC, 0xCC ) );
		}
		else
		{
			Color aColor2 = aStyleSettings.GetLightColor();
			aStyleSettings.SetCheckedColor( Color( (sal_uInt8)( ( (sal_uInt16)aBackColor.GetRed() + (sal_uInt16)aColor2.GetRed() ) / 2 ), (sal_uInt8)( ( (sal_uInt16)aBackColor.GetGreen() + (sal_uInt16)aColor2.GetGreen() ) / 2 ), (sal_uInt8)( ( (sal_uInt16)aBackColor.GetBlue() + (sal_uInt16)aColor2.GetBlue() ) / 2 ) ) );
		}
	}

	if ( pVCLShadowColor )
		aStyleSettings.SetShadowColor( Color( *pVCLShadowColor ) );

	// Mnemonics is needed for our code in OutputDevice::ImplDrawMnemonicLine()
	aStyleSettings.SetOptions( aStyleSettings.GetOptions() & ~STYLE_OPTION_NOMNEMONICS );

	// Use large icons by default
	aStyleSettings.SetToolbarIconSize( STYLE_TOOLBAR_ICONSIZE_LARGE );

	// Hide images in popup menus
	aStyleSettings.SetPreferredUseImagesInMenus( false );
	aStyleSettings.SetAcceleratorsInContextMenus( false );

	if ( nVCLScrollbarSize > 0 )
		aStyleSettings.SetScrollBarSize( nVCLScrollbarSize );

	SalData *pSalData = GetSalData();

	vcl::Font aSystemFont( pSalData->maSystemFont );
	aSystemFont.SetHeight( Float32ToLong( (float)aSystemFont.GetHeight() * 72 / mpGraphics->mnDPIY ) );
	if ( aSystemFont.GetHeight() > 0 )
	{
		aStyleSettings.SetAppFont( aSystemFont );
		aStyleSettings.SetHelpFont( aSystemFont );
		aStyleSettings.SetPushButtonFont( aSystemFont );
		aStyleSettings.SetToolFont( aSystemFont );
	}

	vcl::Font aLabelFont( pSalData->maLabelFont );
	aLabelFont.SetHeight( Float32ToLong( (float)aLabelFont.GetHeight() * 72 / mpGraphics->mnDPIY ) );
	if ( aLabelFont.GetHeight() <= 0 )
		aLabelFont = aSystemFont;
	if ( aLabelFont.GetHeight() > 0 )
	{
		aStyleSettings.SetFieldFont( aLabelFont );
		aStyleSettings.SetGroupFont( aLabelFont );
		aStyleSettings.SetIconFont( aLabelFont );
		aStyleSettings.SetInfoFont( aLabelFont );
		aStyleSettings.SetLabelFont( aLabelFont );
		aStyleSettings.SetRadioCheckFont( aLabelFont );
	}

	vcl::Font aMenuFont( pSalData->maMenuFont );
	aMenuFont.SetHeight( Float32ToLong( (float)aMenuFont.GetHeight() * 72 / mpGraphics->mnDPIY ) );
	if ( aMenuFont.GetHeight() <= 0 )
		aMenuFont = aSystemFont;
	if ( aMenuFont.GetHeight() > 0 )
		aStyleSettings.SetMenuFont( aMenuFont );

	vcl::Font aTitleBarFont( pSalData->maTitleBarFont );
	aTitleBarFont.SetHeight( Float32ToLong( (float)aTitleBarFont.GetHeight() * 72 / mpGraphics->mnDPIY ) );
	if ( aTitleBarFont.GetHeight() <= 0 )
		aTitleBarFont = aSystemFont;
	if ( aTitleBarFont.GetHeight() > 0 )
	{
		aStyleSettings.SetTitleFont( aTitleBarFont );
		aStyleSettings.SetFloatTitleFont( aTitleBarFont );
	}

	rSettings.SetStyleSettings( aStyleSettings );

	[pPool release];
}

// -----------------------------------------------------------------------

const SystemEnvData* JavaSalFrame::GetSystemData() const
{
	return &maSysData;
}

// -----------------------------------------------------------------------

void JavaSalFrame::Beep()
{
	NSBeep();
}

// -----------------------------------------------------------------------

SalFrame* JavaSalFrame::GetParent() const
{
	return mpParent;
}

// -----------------------------------------------------------------------

LanguageType JavaSalFrame::GetInputLanguage()
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::GetInputLanguage not implemented\n" );
#endif
	return LANGUAGE_DONTKNOW;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetParent( SalFrame* pNewParent )
{
	bool bUtilityWindow = IsUtilityWindow();
	if ( bUtilityWindow && pNewParent == mpParent )
		return;

	if ( mpParent )
		mpParent->maChildren.remove( this );

	mpParent = (JavaSalFrame *)pNewParent;

	// Utility windows should never be attached to a parent window.
	if ( !bUtilityWindow )
	{
		::std::list< JavaSalObject* > aReshowObjects( maVisibleObjects );
		bool bReshow = mbVisible;
		if ( bReshow )
			Show( false );

		// Fix bug 1310 by creating a new native window with the new parent
		maSysData.mpNSView = NULL;
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		// Prevent deadlock when opening a document when toolbars are
		// showing and we are in show only menus mode.
		NSWindow *pParentWindow = nil;
		if ( mpParent && mpParent->mpWindow && !mpParent->mbShowOnlyMenus )
			pParentWindow = [(VCLWindowWrapper *)mpParent->mpWindow window];

		VCLCreateWindow *pVCLCreateWindow = [VCLCreateWindow createWithStyle:mnStyle frame:this parent:pParentWindow showOnlyMenus:mbShowOnlyMenus utility:bUtilityWindow];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pVCLCreateWindow performSelectorOnMainThread:@selector(createWindow:) withObject:pVCLCreateWindow waitUntilDone:YES modes:pModes];
		VCLWindowWrapper *pWindow = [pVCLCreateWindow window];
		if ( pWindow )
		{
			// Release old window wrapper
			if ( mpWindow )
			{
				[mpWindow performSelectorOnMainThread:@selector(destroy:) withObject:mpWindow waitUntilDone:YES modes:pModes];
				[mpWindow release];
			}

			[pWindow retain];
			mpWindow = pWindow;
		}

		[pPool release];

		if ( mpParent )
			SetPosSize( maGeometry.nX - mpParent->maGeometry.nX - mpParent->maGeometry.nLeftDecoration, maGeometry.nY - mpParent->maGeometry.nY - mpParent->maGeometry.nTopDecoration, maGeometry.nWidth, maGeometry.nHeight, SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
		else
			SetPosSize( maGeometry.nX - maGeometry.nLeftDecoration, maGeometry.nY - maGeometry.nTopDecoration, maGeometry.nWidth, maGeometry.nHeight, SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );

		if ( bReshow )
		{
			Show( true, false );
			for ( ::std::list< JavaSalObject* >::const_iterator it = aReshowObjects.begin(); it != aReshowObjects.end(); ++it )
				(*it)->Show( true );
		}
	}

	if ( mpParent )
		mpParent->maChildren.push_back( this );

	if ( !bUtilityWindow )
	{
		// Reattach floating children
		::std::list< JavaSalFrame* >aChildren( maChildren );
		for ( ::std::list< JavaSalFrame* >::const_iterator it = aChildren.begin(); it != aChildren.end(); ++it )
			(*it)->SetParent( this );
	}
}

// -----------------------------------------------------------------------

bool JavaSalFrame::SetPluginParent( SystemParentData* /* pNewParent */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::SetPluginParent not implemented\n" );
#endif
	return false;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetMenu( SalMenu* pSalMenu )
{
	JavaSalMenu *pJavaSalMenu = (JavaSalMenu *)pSalMenu;
	if ( pJavaSalMenu && pJavaSalMenu->mbIsMenuBarMenu )
	{
		bool bUpdateMenus = ( mbVisible && pJavaSalMenu != mpMenuBar );
		mpMenuBar = pJavaSalMenu;

		// If the menu is being set, we need to update the new menus. Fix
		// bug 2577 by only updating the menubar and not its submenus. Fix
		// bug 3643 by only doing an update of the top level menus as a full
		// update causes changes to be excessively slow.
		if ( bUpdateMenus )
			UpdateMenusForFrame( this, NULL, false );
	}
	else
	{
		mpMenuBar = NULL;
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::DrawMenuBar()
{
}

// -----------------------------------------------------------------------

SalFrame::SalPointerState JavaSalFrame::GetPointerState()
{
	SalData *pSalData = GetSalData();

	SalPointerState aState;
	aState.mnState = pSalData->maLastPointerState.mnState;
	aState.maPos = Point( pSalData->maLastPointerState.maPos.X() - maGeometry.nX, pSalData->maLastPointerState.maPos.Y() - maGeometry.nY );

	return aState;
}

// -----------------------------------------------------------------------

bool JavaSalFrame::MapUnicodeToKeyCode( sal_Unicode /* aUnicode */, LanguageType /* aLangType */, KeyCode& /* rKeyCode */ )
{
	return false;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetExtendedFrameStyle( SalExtStyle /* nExtStyle */ )
{
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetMaxClientSize( long /* nWidth */, long /* nHeight */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::SetMaxClientSize not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalFrame::ResetClipRegion()
{
	if ( maFrameClipPath )
	{
		CGPathRelease( maFrameClipPath );
		maFrameClipPath = NULL;
		mpGraphics->setFrameClipPath( maFrameClipPath );
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::BeginSetClipRegion( sal_uLong /* nRects */ )
{
	ResetClipRegion();
}

// -----------------------------------------------------------------------

void JavaSalFrame::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
	CGRect aRect = UnflipFlippedRect( CGRectMake( nX, nY, nWidth, nHeight ), mpGraphics->maNativeBounds );
	if ( !CGRectIsEmpty( aRect ) )
	{
		if ( !maFrameClipPath )
			maFrameClipPath = CGPathCreateMutable();

		if ( maFrameClipPath )
			CGPathAddRect( maFrameClipPath, NULL, aRect );
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::EndSetClipRegion()
{
	mpGraphics->setFrameClipPath( maFrameClipPath );
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetScreenNumber( unsigned int /* nScreen */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::SetScreenNumber not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

SalFrame::SalIndicatorState JavaSalFrame::GetIndicatorState()
{
	SalFrame::SalIndicatorState aState;
	aState.mnState = 0;
	return aState;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SimulateKeyPress( sal_uInt16 /* nKeyCode */ )
{
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetApplicationID( const OUString& /* rApplicationID */ )
{
}
