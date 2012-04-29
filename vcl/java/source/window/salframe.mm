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

#include <dlfcn.h>

#include <salframe.h>
#include <salgdi.h>
#include <salinst.h>
#include <saldata.hxx>
#include <salmenu.h>
#include <salobj.h>
#include <salsys.h>
#include <vcl/dialog.hxx>
#include <vcl/settings.hxx>
#include <vcl/status.hxx>
#include <vcl/svapp.hxx>
#ifndef USE_NATIVE_EVENTS
#include <com/sun/star/vcl/VCLFrame.hxx>
#endif	// !USE_NATIVE_EVENTS
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
#include <com/sun/star/vcl/VCLGraphics.hxx>
#include <com/sun/star/vcl/VCLScreen.hxx>
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING

#include <premac.h>
#import <AppKit/AppKit.h>
#ifdef USE_NATIVE_WINDOW
#import <AudioToolbox/AudioToolbox.h>
#endif	// USE_NATIVE_WINDOW
// Need to include for SetSystemUIMode constants but we don't link to it
#import <Carbon/Carbon.h>
#import <objc/objc-class.h>
#include <postmac.h>

#if defined USE_NATIVE_WINDOW || defined USE_NATIVE_EVENTS
#include "../java/VCLEventQueue_cocoa.h"
#endif	// USE_NATIVE_WINDOW || USE_NATIVE_EVENTS

typedef UInt32 GetDblTime_Type();
typedef OSStatus SetSystemUIMode_Type( SystemUIMode nMode, SystemUIOptions nOptions );

#ifdef USE_NATIVE_WINDOW
static ::std::map< NSWindow*, JavaSalGraphics* > aNativeWindowMap;
static ::std::map< NSWindow*, NSCursor* > aNativeCursorMap;
#endif	// USE_NATIVE_WINDOW
#if defined USE_NATIVE_WINDOW && defined USE_NATIVE_VIRTUAL_DEVICE && defined USE_NATIVE_PRINTING
static unsigned int nMainScreen = 0;
static ::std::vector< Rectangle > aVCLScreensFullBoundsList;
static ::std::vector< Rectangle > aVCLScreensVisibleBoundsList;
static ::osl::Mutex aScreensMutex;
#endif	// USE_NATIVE_WINDOW && USE_NATIVE_VIRTUAL_DEVICE && USE_NATIVE_PRINTING
static NSColor *pVCLControlTextColor = nil;
static NSColor *pVCLTextColor = nil;
static NSColor *pVCLHighlightColor = nil;
static NSColor *pVCLHighlightTextColor = nil;
static NSColor *pVCLDisabledControlTextColor = nil;
static NSColor *pVCLBackColor = nil;
static ::osl::Mutex aSystemColorsMutex;

using namespace osl;
using namespace rtl;
using namespace vcl;

#ifdef USE_NATIVE_WINDOW

static NSRect GetTotalScreenBounds()
{
	NSRect aRet = NSMakeRect( 0, 0, 0, 0 );

	NSArray *pScreens = [NSScreen screens];
	if ( pScreens )
	{
		NSUInteger nCount = [pScreens count];
		NSUInteger i = 0;
		for ( ; i < nCount; i++ )
		{
			NSScreen *pScreen = [pScreens objectAtIndex:i];
			if ( pScreen )
				aRet = NSUnionRect( [pScreen frame], aRet );
		}
	}

	return aRet;
}

static void HandleScreensChangedRequest()
{
	MutexGuard aGuard( aScreensMutex );

	nMainScreen = 0;
	aVCLScreensFullBoundsList.clear();
	aVCLScreensVisibleBoundsList.clear();

	NSArray *pScreens = [NSScreen screens];
	NSScreen *pMainScreen = [NSScreen mainScreen];
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

				// Check if this is the main screen
				if ( pMainScreen && aVCLScreensFullBoundsList.size() && NSEqualRects( [pMainScreen frame], aFullFrame ) )
					nMainScreen = aVCLScreensFullBoundsList.size() - 1;
			}
		}
	}
}

#endif	// USE_NATIVE_WINDOW

static void HandleSystemColorsChangedRequest()
{
	MutexGuard aGuard( aSystemColorsMutex );

	if ( pVCLControlTextColor )
		[pVCLControlTextColor release];
	pVCLControlTextColor = [NSColor controlTextColor];
	if ( pVCLControlTextColor )
	{
		pVCLControlTextColor = [pVCLControlTextColor colorUsingColorSpaceName:NSDeviceRGBColorSpace];
		if ( pVCLControlTextColor )
			[pVCLControlTextColor retain];
	}

	if ( pVCLTextColor )
		[pVCLTextColor release];
	pVCLTextColor = [NSColor textColor];
	if ( pVCLTextColor )
	{
		pVCLTextColor = [pVCLTextColor colorUsingColorSpaceName:NSDeviceRGBColorSpace];
		if ( pVCLTextColor )
			[pVCLTextColor retain];
	}

	if ( pVCLHighlightColor )
		[pVCLHighlightColor release];
	pVCLHighlightColor = [NSColor selectedTextBackgroundColor];
	if ( pVCLHighlightColor )
	{
		pVCLHighlightColor = [pVCLHighlightColor colorUsingColorSpaceName:NSDeviceRGBColorSpace];
		if ( pVCLHighlightColor )
			[pVCLHighlightColor retain];
	}

	if ( pVCLHighlightTextColor )
		[pVCLHighlightTextColor release];
	pVCLHighlightTextColor = [NSColor selectedTextColor];
	if ( pVCLHighlightTextColor )
	{
		pVCLHighlightTextColor = [pVCLHighlightTextColor colorUsingColorSpaceName:NSDeviceRGBColorSpace];
		if ( pVCLHighlightTextColor )
			[pVCLHighlightTextColor retain];
	}

	if ( pVCLDisabledControlTextColor )
		[pVCLDisabledControlTextColor release];
	pVCLDisabledControlTextColor = [NSColor disabledControlTextColor];
	if ( pVCLDisabledControlTextColor )
	{
		pVCLDisabledControlTextColor = [pVCLDisabledControlTextColor colorUsingColorSpaceName:NSDeviceRGBColorSpace];
		if ( pVCLDisabledControlTextColor )
			[pVCLDisabledControlTextColor retain];
	}

	if ( pVCLBackColor )
		[pVCLBackColor release];
	pVCLBackColor = [NSColor controlHighlightColor];
	if ( pVCLBackColor )
	{
		pVCLBackColor = [pVCLBackColor colorUsingColorSpaceName:NSDeviceRGBColorSpace];
		if ( pVCLBackColor )
			[pVCLBackColor retain];
	}
}

@interface VCLSetSystemUIMode : NSObject
{
	BOOL					mbFullScreen;
}
+ (id)createFullScreen:(BOOL)bFullScreen;
- (id)initFullScreen:(BOOL)bFullScreen;
- (void)setSystemUIMode:(id)pObject;
- (void)updateSystemActivity;
@end

static NSTimer *pUpdateTimer = nil;

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
	void *pLib = dlopen( NULL, RTLD_LAZY | RTLD_LOCAL );
	if ( pLib )
	{
		SetSystemUIMode_Type *pSetSystemUIMode = (SetSystemUIMode_Type *)dlsym( pLib, "SetSystemUIMode" );
		if ( pSetSystemUIMode )
		{
			if ( mbFullScreen )
			{
				pSetSystemUIMode( kUIModeAllHidden, kUIOptionDisableAppleMenu | kUIOptionDisableProcessSwitch );

				// Run the update timer every 15 seconds
				if ( !pUpdateTimer )
				{
					SEL aSelector = @selector(updateSystemActivity);
					NSMethodSignature *pSignature = [[self class] instanceMethodSignatureForSelector:aSelector];
					if ( pSignature )
					{
						NSInvocation *pInvocation = [NSInvocation invocationWithMethodSignature:pSignature];
						if ( pInvocation )
						{
							[pInvocation setSelector:aSelector];
							[pInvocation setTarget:self];
							pUpdateTimer = [NSTimer scheduledTimerWithTimeInterval:15 invocation:pInvocation repeats:YES];
							if ( pUpdateTimer )
							{
								[pUpdateTimer retain];
								[self updateSystemActivity];
							}
						}
					}
				}
			}
			else
			{
				pSetSystemUIMode( kUIModeNormal, 0 );

				// Stop the update timer
				if ( pUpdateTimer )
				{
					[pUpdateTimer invalidate];
					[pUpdateTimer release];
					pUpdateTimer = nil;
				}
			}
		}

		dlclose( pLib );
	}
}

- (void)updateSystemActivity
{
	UpdateSystemActivity( OverallAct );
}

@end

@interface NSWindow (VCLToggleFullScreen)
- (void)toggleFullScreen:(id)pObject;
@end

@interface VCLToggleFullScreen : NSObject
{
	NSWindow*				mpWindow;
}
+ (id)createToggleFullScreen:(NSWindow *)pWindow;
- (id)initToggleFullScreen:(NSWindow *)pWindow;
- (void)dealloc;
- (void)toggleFullScreen:(id)pObject;
@end

@implementation VCLToggleFullScreen

+ (id)createToggleFullScreen:(NSWindow *)pWindow
{
	VCLToggleFullScreen *pRet = [[VCLToggleFullScreen alloc] initToggleFullScreen:pWindow];
	[pRet autorelease];
	return pRet;
}

- (id)initToggleFullScreen:(NSWindow *)pWindow
{
	[super init];

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
	if ( mpWindow && [mpWindow respondsToSelector:@selector(toggleFullScreen:)] )
		[mpWindow toggleFullScreen:self];
}

@end

#ifdef USE_NATIVE_WINDOW

@interface VCLViewGetGraphicsLayer : NSObject
{
	JavaSalGraphics*		mpGraphics;
	CGLayerRef				maLayer;
	CGSize					maSize;
	NSView*					mpView;
}
+ (id)createGraphicsLayer:(JavaSalGraphics *)pGraphics view:(NSView *)pView size:(CGSize)aSize;
- (id)initGraphicsLayer:(JavaSalGraphics *)pGraphics view:(NSView *)pView size:(CGSize)aSize;
- (void)dealloc;
- (void)getGraphicsLayer:(id)pObject;
- (CGLayerRef)layer;
@end

@implementation VCLViewGetGraphicsLayer

+ (id)createGraphicsLayer:(JavaSalGraphics *)pGraphics view:(NSView *)pView size:(CGSize)aSize
{
	VCLViewGetGraphicsLayer *pRet = [[VCLViewGetGraphicsLayer alloc] initGraphicsLayer:pGraphics view:pView size:aSize];
	[pRet autorelease];
	return pRet;
}

- (id)initGraphicsLayer:(JavaSalGraphics *)pGraphics view:(NSView *)pView size:(CGSize)aSize
{
	[super init];

	mpGraphics = pGraphics;
	maLayer = NULL;
	maSize = aSize;
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
			{
				[it->first release];
				aNativeWindowMap.erase( it );
			}
		}
	}

	// Remove all entries for the graphics
	::std::map< NSWindow*, JavaSalGraphics* >::iterator it = aNativeWindowMap.begin();
	while ( it != aNativeWindowMap.end() )
	{
		if ( it->second == mpGraphics )
		{
			[it->first release];
			aNativeWindowMap.erase( it );
			it = aNativeWindowMap.begin();
			continue;
		}

		++it;
	}

	if ( mpGraphics && maSize.width > 0 && maSize.height > 0 && pWindow && [pWindow isVisible] )
	{
		NSGraphicsContext *pContext = [pWindow graphicsContext];
		if ( pContext )
		{
			CGContextRef aContext = (CGContextRef)[pContext graphicsPort];
			if ( aContext )
			{
				maLayer = CGLayerCreateWithContext( aContext, maSize, NULL );
				if ( maLayer )
				{
					[pWindow retain];
					aNativeWindowMap[ pWindow ] = mpGraphics;
				}
			}
		}
	}
}

- (CGLayerRef)layer
{
	return maLayer;
}

@end

@interface NSCursor (VCLSetCursor)

+ (NSCursor *)IBeamCursorForVerticalLayout;
+ (NSCursor *)operationNotAllowedCursor;

@end

static ::std::map< PointerStyle, NSCursor* > aVCLCustomCursors;

@interface VCLSetCursor : NSObject
{
	PointerStyle			mePointerStyle;
	NSView*					mpView;
}
+ (id)createWithPointerStyle:(PointerStyle)ePointerStyle view:(NSView *)pView;
+ (void)loadCustomCursorWithPointerStyle:(PointerStyle)ePointerStyle hotSpot:(NSPoint)aHotSpot path:(NSString *)pPath;
- (void)dealloc;
- (id)initWithPointerStyle:(PointerStyle)ePointerStyle view:(NSView *)pView;
- (void)setCursor:(id)pObject;
@end

@implementation VCLSetCursor

+ (id)createWithPointerStyle:(PointerStyle)ePointerStyle view:(NSView *)pView
{
	VCLSetCursor *pRet = [[VCLSetCursor alloc] initWithPointerStyle:ePointerStyle view:pView];
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
	if ( mpView )
		[mpView release];

	[super dealloc];
}

- (id)initWithPointerStyle:(PointerStyle)ePointerStyle view:(NSView *)pView
{
	[super init];

	mePointerStyle = ePointerStyle;
	mpView = pView;
	if ( mpView )
		[mpView retain];
 
	return self;
}

- (void)setCursor:(id)pObject
{
	if ( !mpView )
		return;

	NSWindow *pWindow = [mpView window];
	if ( !pWindow || ![pWindow isVisible] )
		return;

	// Remove any hidden windows from cursor map
	::std::map< NSWindow*, NSCursor* >::iterator cit = aNativeCursorMap.begin();
	while ( cit != aNativeCursorMap.end() )
	{
		if ( ![cit->first isVisible] )
		{
			[cit->first release];
			[cit->second release];
			aNativeCursorMap.erase( cit );
			cit = aNativeCursorMap.begin();
			continue;
		}

		++cit;
	}

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
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AIRBRUSH hotSpot:NSMakePoint( 2, 31 ) path:[pPath stringByAppendingPathComponent:@"airbrush.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_E hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"ase.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_N hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"asn.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_NE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"asne.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_NS hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"asns.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_NSWE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"asnswe"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_NW hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"asnw.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_S hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"ass.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_SE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"asse.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_SW hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"assw.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_W hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"asw.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_AUTOSCROLL_WE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"aswe.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_CHAIN hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"chain.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_CHAIN_NOTALLOWED hotSpot:NSMakePoint( 10, 10 ) path:[pPath stringByAppendingPathComponent:@"chainnot.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_CHART hotSpot:NSMakePoint( 11, 11 ) path:[pPath stringByAppendingPathComponent:@"chart.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_COPYDATA hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"copydata.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_COPYDATALINK hotSpot:NSMakePoint( 10, 2 ) path:[pPath stringByAppendingPathComponent:@"copydlnk.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_COPYFILE hotSpot:NSMakePoint( 5, 10 ) path:[pPath stringByAppendingPathComponent:@"copyf.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_COPYFILES hotSpot:NSMakePoint( 9, 10 ) path:[pPath stringByAppendingPathComponent:@"copyf2.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_COPYFILELINK hotSpot:NSMakePoint( 9, 7 ) path:[pPath stringByAppendingPathComponent:@"copyflnk.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_CROOK hotSpot:NSMakePoint( 16, 17 ) path:[pPath stringByAppendingPathComponent:@"crook.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_CROP hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"crop.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_CROSS hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"cross.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_ARC hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"darc.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_BEZIER hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dbezier.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_CAPTION hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dcapt.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_CIRCLECUT hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dcirccut.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_CONNECT hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dconnect.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_ELLIPSE hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dellipse.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DETECTIVE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"detectiv.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_FREEHAND hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dfree.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_LINE hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dline.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_PIE hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dpie.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_POLYGON hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dpolygon.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_RECT hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"drect.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_DRAW_TEXT hotSpot:NSMakePoint( 8, 8 ) path:[pPath stringByAppendingPathComponent:@"dtext.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_FILL hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"fill.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_HAND hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"hand.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_HELP hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"help.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_HSHEAR hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"hshear.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_HSIZEBAR hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"hsizebar.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_HSPLIT hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"hsplit.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_LINKDATA hotSpot:NSMakePoint( 10, 2 ) path:[pPath stringByAppendingPathComponent:@"linkdata.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_LINKFILE hotSpot:NSMakePoint( 9, 7 ) path:[pPath stringByAppendingPathComponent:@"linkf.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MAGNIFY hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"magnify.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MIRROR hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"mirror.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"move.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVEBEZIERWEIGHT hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"movebw.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVEDATA hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"movedata.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVEDATALINK hotSpot:NSMakePoint( 3, 3 ) path:[pPath stringByAppendingPathComponent:@"movedlnk.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVEFILE hotSpot:NSMakePoint( 5, 10 ) path:[pPath stringByAppendingPathComponent:@"movef.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVEFILES hotSpot:NSMakePoint( 9, 10 ) path:[pPath stringByAppendingPathComponent:@"movef2.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVEFILELINK hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"moveflnk.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_MOVEPOINT hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"movept.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_NESIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"neswsize.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_SWSIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"neswsize.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_WINDOW_NESIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"neswsize.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_WINDOW_SWSIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"neswsize.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_NOTALLOWED hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"notallow.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_NULL hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"nullptr.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_NWSIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"nwsesize.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_SESIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"nwsesize.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_WINDOW_NWSIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"nwsesize.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_WINDOW_SESIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"nwsesize.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_PEN hotSpot:NSMakePoint( 2, 31 ) path:[pPath stringByAppendingPathComponent:@"pen.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_PIVOT_COL hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"pivotcol.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_PIVOT_DELETE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"pivotdel.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_PIVOT_FIELD hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"pivotfld.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_PIVOT_ROW hotSpot:NSMakePoint( 2, 3 ) path:[pPath stringByAppendingPathComponent:@"pivotrow.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_PAINTBRUSH hotSpot:NSMakePoint( 4, 30 ) path:[pPath stringByAppendingPathComponent:@"pntbrsh.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_REFHAND hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"refhand.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_ROTATE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"rotate.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_TAB_SELECT_E hotSpot:NSMakePoint( 31, 16 ) path:[pPath stringByAppendingPathComponent:@"tblsele.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_TAB_SELECT_S hotSpot:NSMakePoint( 16, 31 ) path:[pPath stringByAppendingPathComponent:@"tblsels.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_TAB_SELECT_SE hotSpot:NSMakePoint( 31, 31 ) path:[pPath stringByAppendingPathComponent:@"tblselse.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_TAB_SELECT_SW hotSpot:NSMakePoint( 2, 31 ) path:[pPath stringByAppendingPathComponent:@"tblselsw.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_TAB_SELECT_W hotSpot:NSMakePoint( 2, 16 ) path:[pPath stringByAppendingPathComponent:@"tblselw.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_TIMEEVENT_MOVE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"timemove.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_TIMEEVENT_SIZE hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"timesize.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_VSHEAR hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"vshear.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_VSIZEBAR hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"vsizebar.gif"]];
					[VCLSetCursor loadCustomCursorWithPointerStyle:POINTER_VSPLIT hotSpot:NSMakePoint( 16, 16 ) path:[pPath stringByAppendingPathComponent:@"vsplit.gif"]];
					// Note: POINTER_TEXT_VERTICAL vtext.gif icon unused due to
					// white background in icon
				}
			}
		}
	}

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
				if ( class_getClassMethod( [NSCursor class], @selector(operationNotAllowedCursor) ) )
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
				if ( class_getClassMethod( [NSCursor class], @selector(IBeamCursorForVerticalLayout) ) )
				pCursor = [NSCursor IBeamCursorForVerticalLayout];
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
			case POINTER_WAIT:
			default:
				pCursor = [NSCursor arrowCursor];
				break;
		}
	}

	if ( pCursor )
	{
		aNativeCursorMap[ pWindow ] = pCursor;
		[pWindow retain];
		[pCursor retain];
		[pWindow invalidateCursorRectsForView:mpView];
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
	for ( ::std::map< NSWindow*, JavaSalGraphics* >::const_iterator it = aNativeWindowMap.begin(); it != aNativeWindowMap.end(); ++it )
	{
		NSView *pContentView = [it->first contentView];
		if ( pContentView )
			it->second->setNeedsDisplay( pContentView );
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
	HandleScreensChangedRequest();
}

- (void)updateScreens:(id)pObject
{
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

#endif	// USE_NATIVE_WINDOW

@interface VCLUpdateSystemColors : NSObject
{
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
 
	return self;
}

- (void)systemColorsChanged:(NSNotification *)pNotification
{
	HandleSystemColorsChangedRequest();
}

- (void)updateSystemColors:(id)pObject
{
	if ( !pVCLUpdateSystemColors )
	{
		NSNotificationCenter *pNotificationCenter = [NSNotificationCenter defaultCenter];
		if ( pNotificationCenter )
		{
			pVCLUpdateSystemColors = self;
			[pVCLUpdateSystemColors retain];
			[pNotificationCenter addObserver:pVCLUpdateSystemColors selector:@selector(systemColorsChanged:) name:NSSystemColorsDidChangeNotification object:nil];
		}
	}

	HandleSystemColorsChangedRequest();
}

@end

#ifdef USE_NATIVE_EVENTS

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

@interface VCLWindowWrapper : NSObject
{
	JavaSalFrame*			mpFrame;
	MacOSBOOL				mbFullScreen;
	NSRect					maInsets;
	VCLWindow*				mpParent;
	MacOSBOOL				mbShowOnlyMenus;
	NSRect					maShowOnlyMenusFrame;
	ULONG					mnStyle;
	MacOSBOOL				mbUndecorated;
	MacOSBOOL				mbUtility;
	VCLWindow*				mpWindow;
	NSUInteger				mnWindowStyleMask;
}
+ (void)updateShowOnlyMenusWindows;
- (void)adjustColorLevelAndShadow;
- (id)initWithStyle:(ULONG)nStyle frame:(JavaSalFrame *)pFrame parent:(VCLWindow *)pParent showOnlyMenus:(MacOSBOOL)bShowOnlyMenus utility:(MacOSBOOL)bUtility;
- (void)dealloc;
- (void)destroy:(id)pObject;
- (void)flush:(id)pObject;
- (void)getContentView:(VCLWindowWrapperArgs *)pArgs;
- (void)getFrame:(VCLWindowWrapperArgs *)pArgs;
- (void)getState:(VCLWindowWrapperArgs *)pArgs;
- (const NSRect)insets;
- (MacOSBOOL)isFloatingWindow;
- (void)makeModal:(id)pObject;
- (void)requestFocus:(VCLWindowWrapperArgs *)pArgs;
- (void)setFrame:(VCLWindowWrapperArgs *)pArgs;
- (void)setFullScreenMode:(VCLWindowWrapperArgs *)pArgs;
- (void)setMinSize:(VCLWindowWrapperArgs *)pArgs;
- (void)setState:(VCLWindowWrapperArgs *)pArgs;
- (void)setTitle:(VCLWindowWrapperArgs *)pArgs;
- (void)setVisible:(VCLWindowWrapperArgs *)pArgs;
- (void)toFront:(VCLWindowWrapperArgs *)pArgs;
- (VCLWindow *)window;
@end

static ::std::map< VCLWindow*, VCLWindow* > aShowOnlyMenusWindowMap;

@implementation VCLWindowWrapper

+ (void)updateShowOnlyMenusWindows
{
	// Fix bug 3032 by disabling focus for show only menus windows when
	// any frames are visible
	MacOSBOOL bEnableFocus = NO;
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
				NSWindow *pWindow = [pWindows objectAtIndex:i];
				if ( !pWindow || ![pWindow isVisible] || aShowOnlyMenusWindowMap.find( pWindow ) != aShowOnlyMenusWindowMap.end() )
					continue;

				if ( [pWindow isKindOfClass:[VCLWindow class]] )
				{
					if ( [(VCLWindow *)pWindow canBecomeKeyOrMainWindow] )
					{
						bEnableFocus = YES;
						break;
					}
				}
				else if ( [pWindow canBecomeKeyWindow] )
				{
					bEnableFocus = YES;
					break;
				}
			}
		}
	}

	for ( ::std::map< VCLWindow*, VCLWindow* >::const_iterator it = aShowOnlyMenusWindowMap.begin(); it != aShowOnlyMenusWindowMap.end(); ++it )
	{
		[it->first setCanBecomeKeyOrMainWindow:bEnableFocus];
		if ( bEnableFocus && [it->first isVisible] )
			[it->first makeKeyWindow];
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
			[mpWindow setLevel:NSFloatingWindowLevel];
		}
		else if ( mbShowOnlyMenus || ( mbUndecorated && mpParent ) )
		{
			[mpWindow setHasShadow:YES];
			[mpWindow setLevel:NSPopUpMenuWindowLevel];
		}
		else if ( mbUndecorated && [mpWindow canBecomeKeyOrMainWindow] && !mpParent )
		{
			[mpWindow setHasShadow:NO];
			[mpWindow setLevel:NSNormalWindowLevel];
		}
		else
		{
			[mpWindow setHasShadow:YES];
			[mpWindow setLevel:NSNormalWindowLevel];
		}
	}
}

- (id)initWithStyle:(ULONG)nStyle frame:(JavaSalFrame *)pFrame parent:(VCLWindow *)pParent showOnlyMenus:(MacOSBOOL)bShowOnlyMenus utility:(MacOSBOOL)bUtility
{
	[super init];

	mpFrame = pFrame;
	mbFullScreen = NO;
	maInsets = NSMakeRect( 0, 0, 0, 0 );
	mpParent = pParent;
	if ( mpParent )
		[mpParent retain];
	mbShowOnlyMenus = bShowOnlyMenus;
	maShowOnlyMenusFrame = NSMakeRect( 0, 0, 1, 1 );
	mnStyle = nStyle;
	mbUtility = bUtility;
	mbUndecorated = NO;
	mpWindow = nil;
	mnWindowStyleMask = NSBorderlessWindowMask;

	if ( !mbUtility && ( mbShowOnlyMenus || ! ( mnStyle & ( SAL_FRAME_STYLE_DEFAULT | SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE ) ) ) )
		mbUndecorated = YES;

	if ( !mbUndecorated )
	{
		mnWindowStyleMask = NSTitledWindowMask | NSClosableWindowMask;
		if ( mbUtility )
			mnWindowStyleMask |= NSUtilityWindowMask;
		if ( mnStyle & SAL_FRAME_STYLE_SIZEABLE )
		{
			mnWindowStyleMask |= NSResizableWindowMask;
			if ( !mbUtility )
				mnWindowStyleMask |= NSMiniaturizableWindowMask;
		}
	}

	VCLView *pContentView = [[VCLView alloc] initWithFrame:NSMakeRect( 0, 0, 1, 1 )];
	if ( pContentView )
	{
		mpWindow = [[VCLWindow alloc] initWithContentRect:NSMakeRect( 0, 0, 1, 1 ) styleMask:( mnWindowStyleMask & ~NSUtilityWindowMask ) backing:NSBackingStoreBuffered defer:YES];
		if ( mpWindow )
		{
			if ( mbUtility )
			{
				// Set to utility window type
				if ( [mpWindow respondsToSelector:@selector(_setUtilityWindow:)] )
					[mpWindow _setUtilityWindow:YES];
				[mpWindow setHidesOnDeactivate:YES];
			}
			else if ( mbUndecorated && !mbShowOnlyMenus && !mbFullScreen )
			{
				[mpWindow setCanBecomeKeyOrMainWindow:NO];
			}

			[mpWindow setContentView:pContentView];
			[mpWindow setFrame:mpFrame];

			[self adjustColorLevelAndShadow];

			// Cache the window's insets
			NSRect aContentRect = NSMakeRect( 0, 0, 1, 1 );
			NSRect aFrameRect = [NSWindow frameRectForContentRect:aContentRect styleMask:mnWindowStyleMask];
			maInsets = NSMakeRect( aContentRect.origin.x - aFrameRect.origin.x, aContentRect.origin.y - aFrameRect.origin.y, aFrameRect.origin.x + aFrameRect.size.width - aContentRect.origin.x - aContentRect.size.width, aFrameRect.origin.y + aFrameRect.size.height - aContentRect.origin.y - aContentRect.size.height );

			if ( mbShowOnlyMenus )
				aShowOnlyMenusWindowMap[ mpWindow ] = mpWindow;
		}
	}

	return self;
}

- (void)dealloc
{
	[self destroy:self];

	[super dealloc];
}

- (void)flush:(id)pObject
{
	if ( mpWindow )
	{
		::std::map< NSWindow*, JavaSalGraphics* >::const_iterator it = aNativeWindowMap.find( mpWindow );
		if ( it != aNativeWindowMap.end() )
		{
			NSView *pContentView = [it->first contentView];
			if ( pContentView )
				it->second->setNeedsDisplay( pContentView );
		}
	}
}

- (void)destroy:(id)pObject
{
	if ( mpParent )
	{
		[mpParent release];
		mpParent = nil;
	}

	if ( mpWindow )
	{
		[mpWindow orderOut:self];

		::std::map< VCLWindow*, VCLWindow* >::iterator it = aShowOnlyMenusWindowMap.find ( mpWindow );
		if ( it != aShowOnlyMenusWindowMap.end() )
			aShowOnlyMenusWindowMap.erase( it );

		[mpWindow release];
		mpWindow = nil;
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

#ifdef USE_NATIVE_FULL_SCREEN_MODE
	// Enable full screen feature for normal windows
	if ( mpWindow && [pTopLevelWindow boolValue] )
		[mpWindow setCollectionBehavior:[mpWindow collectionBehavior] | NSWindowCollectionBehaviorFullScreenPrimary];
#endif	// USE_NATIVE_FULL_SCREEN_MODE

	// Only return content view if window is visible
	if ( mpWindow && [mpWindow isVisible] )
		[pArgs setResult:[mpWindow contentView]];
}

- (void)getFrame:(VCLWindowWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

    NSValue *pInLiveResize = (NSValue *)[pArgArray objectAtIndex:0];
    if ( !pInLiveResize )
        return;

    NSValue *pFullScreen = (NSValue *)[pArgArray objectAtIndex:1];
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
			if ( [mpWindow respondsToSelector:@selector(inLiveResize)] && [mpWindow inLiveResize] )
				*pInLiveResizePointer = YES;
			else
				*pInLiveResizePointer = NO;
		}

		NSRect aFrame = [mpWindow frame];

#ifdef USE_NATIVE_FULL_SCREEN_MODE
		// Check if we are in full screen mode
		if ( [mpWindow styleMask] & NSFullScreenWindowMask && [mpWindow respondsToSelector:@selector(_frameOnExitFromFullScreen)] )
		{
			if ( [pFullScreen boolValue] )
			{
				NSRect aNonFullScreenFrame = [mpWindow _frameOnExitFromFullScreen];
				if ( !NSIsEmptyRect( aNonFullScreenFrame ) )
					aFrame = aNonFullScreenFrame;
			}
			else
			{
				aFrame = [NSWindow frameRectForContentRect:maFrame styleMask:[mpWindow styleMask] & ~NSFullScreenWindowMask];
			}
		}
#endif	// USE_NATIVE_FULL_SCREEN_MODE

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
		if ( [mpWindow styleMask] & NSMiniaturizableWindowMask && [mpWindow isMiniaturized] )
			nState = SAL_FRAMESTATE_MINIMIZED;
		else if ( [mpWindow styleMask] & NSResizableWindowMask && [mpWindow isZoomed] )
			nState = SAL_FRAMESTATE_MAXIMIZED;
		else
			nState = SAL_FRAMESTATE_NORMAL;

		[pArgs setResult:[NSNumber numberWithUnsignedLong:nState]];
	}
}

- (const NSRect)insets
{
	return maInsets;
}

- (MacOSBOOL)isFloatingWindow
{
	return ( mbUndecorated && !mbFullScreen && !mbShowOnlyMenus );
}

- (void)makeModal:(id)pObject
{
	if ( mpWindow && [mpWindow styleMask] & NSTitledWindowMask && [mpWindow respondsToSelector:@selector(_setModalWindowLevel)] )
	{
		[mpWindow _setModalWindowLevel];

		// Run VCLWindow selector to ensure that the window level is set
		// correctly if the application is not active
		[VCLWindow clearModalWindowLevel];
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

- (void)setFrame:(VCLWindowWrapperArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

    NSValue *pFrame = (NSValue *)[pArgArray objectAtIndex:0];
    if ( !pFrame )
        return;

	if ( mpWindow )
	{
		// Fix bug 3012 by only returning a minimum size when the window is
		// visible
		NSSize aMinSize = [mpWindow minSize];
		if ( ![mpWindow isVisible] )
			aMinSize = NSMakeSize( 0, 0 );

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

		[mpWindow setFrame:aFrame display:NO];
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
	[self adjustColorLevelAndShadow];

	if ( mpWindow )
	{
		if ( mbUndecorated && !mbShowOnlyMenus && !mbFullScreen )
			[mpWindow setCanBecomeKeyOrMainWindow:NO];
		else
			[mpWindow setCanBecomeKeyOrMainWindow:YES];
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

	if ( mpWindow )
	{
		NSSize aMinSize = [pMinSize sizeValue];

		// Make sure that there is a minimum amount of content area
		if ( mbUndecorated && aMinSize.width < 1 )
			aMinSize.width = 1;
		else if ( !mbUndecorated && aMinSize.width < 200 )
			aMinSize.width = 200;
		if ( aMinSize.height < 1 )
			aMinSize.height = 1;

		const NSRect aInsets = [self insets];
		aMinSize.width += aInsets.origin.x + aInsets.size.width;
		aMinSize.height += aInsets.origin.y + aInsets.size.height;

		[mpWindow setMinSize:aMinSize];
	}
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
	if ( !mbShowOnlyMenus && !mbUndecorated && !mpParent && mpWindow && ( [mpWindow isVisible] || [mpWindow isMiniaturized] ) )
	{
		if ( nState == SAL_FRAMESTATE_MINIMIZED && [mpWindow styleMask] & NSMiniaturizableWindowMask )
		{
			[mpWindow miniaturize:self];
		}
		else if ( nState == SAL_FRAMESTATE_MAXIMIZED && [mpWindow styleMask] & NSResizableWindowMask )
		{
			if ( [mpWindow isMiniaturized] )
				[mpWindow deminiaturize:self];
			if ( ![mpWindow isZoomed] )
				[mpWindow zoom:self];
		}
		else
		{
			if ( [mpWindow isMiniaturized] )
				[mpWindow deminiaturize:self];
			if ( [mpWindow isZoomed] )
				[mpWindow zoom:self];
		}
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

	MacOSBOOL bVisible = [pVisible boolValue];
	if ( mpWindow && bVisible != [mpWindow isVisible] )
	{
		if ( bVisible )
		{
			// Fix bug 1012 by deiconifying the parent window. Fix bug 1388 by
			// skipping this step if the current window is a floating window.
			if ( ![self isFloatingWindow] && mpParent && [mpParent isMiniaturized] )
				[mpParent deminiaturize:self];

			[mpWindow orderWindow:NSWindowAbove relativeTo:( mpParent ? [mpParent windowNumber] : 0 )];
			if ( [mpWindow canBecomeKeyOrMainWindow] && ![pNoActivate boolValue] )
				[mpWindow makeKeyWindow];
		}
		else
		{
			[mpWindow orderOut:self];
		}

		[VCLWindowWrapper updateShowOnlyMenusWindows];
	}
}

- (void)toFront:(VCLWindowWrapperArgs *)pArgs
{
	if ( mpWindow && [mpWindow isVisible] && ![self isFloatingWindow] )
	{
		[mpWindow makeKeyAndOrderFront:self];
		[pArgs setResult:[NSNumber numberWithBool:YES]];
	}
}

- (VCLWindow *)window
{
	return mpWindow;
}

@end

@interface VCLCreateWindow : NSObject
{
	JavaSalFrame*			mpFrame;
	VCLWindow*				mpParent;
	MacOSBOOL				mbShowOnlyMenus;
	ULONG					mnStyle;
	MacOSBOOL				mbUtility;
	VCLWindowWrapper*		mpWindow;
}
+ (id)createWithStyle:(ULONG)nStyle frame:(JavaSalFrame *)pFrame parent:(VCLWindow *)pParent showOnlyMenus:(MacOSBOOL)bShowOnlyMenus utility:(MacOSBOOL)bUtility;
- (id)initWithStyle:(ULONG)nStyle frame:(JavaSalFrame *)pFrame parent:(VCLWindow *)pParent showOnlyMenus:(MacOSBOOL)bShowOnlyMenus utility:(MacOSBOOL)bUtility;
- (void)dealloc;
- (void)createWindow:(id)pObject;
- (VCLWindowWrapper *)window;
@end

@implementation VCLCreateWindow

+ (id)createWithStyle:(ULONG)nStyle frame:(JavaSalFrame *)pFrame parent:(VCLWindow *)pParent showOnlyMenus:(MacOSBOOL)bShowOnlyMenus utility:(MacOSBOOL)bUtility
{
	VCLCreateWindow *pRet = [[VCLCreateWindow alloc] initWithStyle:nStyle frame:pFrame parent:pParent showOnlyMenus:bShowOnlyMenus utility:bUtility];
	[pRet autorelease];
	return pRet;
}

- (id)initWithStyle:(ULONG)nStyle frame:(JavaSalFrame *)pFrame parent:(VCLWindow *)pParent showOnlyMenus:(MacOSBOOL)bShowOnlyMenus utility:(MacOSBOOL)bUtility
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
	if ( mpWindow )
		return;

	mpWindow = [[VCLWindowWrapper alloc] initWithStyle:mnStyle frame:mpFrame parent:mpParent showOnlyMenus:mbShowOnlyMenus utility:mbUtility];
}

- (VCLWindowWrapper *)window
{
	return mpWindow;
}

@end

#endif	// USE_NATIVE_EVENTS

// =======================================================================

long ImplSalCallbackDummy( void*, SalFrame*, USHORT, const void* )
{
	return 0;
}

// -----------------------------------------------------------------------

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

	pFrame->mbInShowOnlyMenus = TRUE;

	// Don't let the progress bar hiding override display set for top-level
	// windows
	BOOL bOldShowOnlyMenus = pFrame->mbShowOnlyMenus;
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

	pFrame->mbInShowOnlyMenus = FALSE;
}

#ifdef USE_NATIVE_WINDOW

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

	::std::map< NSWindow*, JavaSalGraphics* >::iterator it = aNativeWindowMap.find( pWindow );
	if ( it != aNativeWindowMap.end() && it->second )
	{
		CGRect aBounds = CGRectStandardize( NSRectToCGRect( [pView bounds] ) );
		CGRect aDestRect = CGRectStandardize( NSRectToCGRect( aDirtyRect ) );
		if ( CGRectIntersectsRect( aBounds, aDestRect ) )
		{
			NSGraphicsContext *pContext = [NSGraphicsContext currentContext];
			if ( pContext )
			{
				CGContextRef aContext = (CGContextRef)[pContext graphicsPort];
				if ( aContext )
				{
					CGContextSaveGState( aContext );
					if ( ![pView isFlipped] )
					{
						CGContextTranslateCTM( aContext, 0, aBounds.origin.y + aBounds.size.height );
						CGContextScaleCTM( aContext, 1.0, -1.0f );
						aDestRect.origin.y = aBounds.origin.y + aBounds.size.height - aDestRect.origin.y - aDestRect.size.height;
					}

					it->second->copyToContext( NULL, NULL, false, false, aContext, aBounds, aDestRect.origin, aDestRect );

					CGContextRestoreGState( aContext );
				}
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

#endif	// USE_NATIVE_WINDOW

// =======================================================================

JavaSalFrame::JavaSalFrame( ULONG nSalFrameStyle, JavaSalFrame *pParent ) :
#ifdef USE_NATIVE_WINDOW
	mnHiddenBit( 0 ),
    maHiddenContext( NULL ),
    maHiddenLayer( NULL ),
	maFrameLayer( NULL ),
	maFrameClipPath( NULL ),
#endif  // USE_NATIVE_WINDOW
#ifdef USE_NATIVE_EVENTS
	mpWindow( NULL ),
#else	// USE_NATIVE_EVENTS
	mpVCLFrame( NULL ),
#endif	// USE_NATIVE_EVENTS
	mpGraphics( new JavaSalGraphics() ),
	mnStyle( nSalFrameStyle ),
	mpParent( NULL ),
	mbGraphics( FALSE ),
	mbVisible( FALSE ),
	mbCenter( TRUE ),
	mbFullScreen( FALSE ),
	mbPresentation( FALSE ),
	mpMenuBar( NULL ),
	mbInSetPosSize( FALSE ),
	mbInShow( FALSE ),
	mbShowOnlyMenus( FALSE ),
	mbInShowOnlyMenus( FALSE ),
	mbInShowFullScreen( FALSE ),
	mbInWindowDidExitFullScreen( FALSE ),
	mbInWindowWillEnterFullScreen( FALSE )
{
	memset( &maSysData, 0, sizeof( SystemEnvData ) );
	maSysData.nSize = sizeof( SystemEnvData );

	memset( &maGeometry, 0, sizeof( maGeometry ) );
	memset( &maOriginalGeometry, 0, sizeof( maOriginalGeometry ) );

#ifdef USE_NATIVE_EVENTS
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
#else	// USE_NATIVE_EVENTS
    mpVCLFrame = new com_sun_star_vcl_VCLFrame( mnStyle, this, NULL, mbShowOnlyMenus, IsUtilityWindow() );
#endif	// USE_NATIVE_EVENTS

	mpGraphics->mpFrame = this;
#ifdef USE_NATIVE_WINDOW
	mpGraphics->mnDPIX = MIN_SCREEN_RESOLUTION;
	mpGraphics->mnDPIY = MIN_SCREEN_RESOLUTION;

	// Make a native layer backed by a 1 x 1 pixel native bitmap
	CGColorSpaceRef aColorSpace = CGColorSpaceCreateDeviceRGB();
	if ( aColorSpace )
	{
		maHiddenContext = CGBitmapContextCreate( &mnHiddenBit, 1, 1, 8, sizeof( mnHiddenBit ), aColorSpace, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little );
		if ( maHiddenContext )
			maHiddenLayer = CGLayerCreateWithContext( maHiddenContext, CGSizeMake( 1, 1 ), NULL );
		CGColorSpaceRelease( aColorSpace );
	}

	mpGraphics->setLayer( maHiddenLayer );
#endif  // USE_NATIVE_WINDOW

	// Set initial parent
	if ( pParent )
		SetParent( pParent );

	// Cache the insets
	Rectangle aRect = GetInsets();
	maGeometry.nLeftDecoration = aRect.nLeft;
	maGeometry.nTopDecoration = aRect.nTop;
	maGeometry.nRightDecoration = aRect.nRight;
	maGeometry.nBottomDecoration = aRect.nBottom;

	// Insert this window into the window list
	GetSalData()->maFrameList.push_front( this );
}

// -----------------------------------------------------------------------

JavaSalFrame::~JavaSalFrame()
{
	// Remove this window from the window list
	GetSalData()->maFrameList.remove( this );

#ifdef USE_NATIVE_WINDOW
	// Make sure that no native drawing is possible
	maSysData.pView = NULL;
	UpdateLayer();
#endif	// USE_NATIVE_WINDOW

	Show( FALSE );
	StartPresentation( FALSE );
	CaptureMouse( FALSE );

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

#ifdef USE_NATIVE_WINDOW
	if ( maFrameClipPath )
		CGPathRelease( maFrameClipPath );

	if ( maFrameLayer )
		CGLayerRelease( maFrameLayer );

	if ( maHiddenLayer )
		CGLayerRelease( maHiddenLayer );

	if ( maHiddenContext )
		CGContextRelease( maHiddenContext );
#endif  // USE_NATIVE_WINDOW

#ifdef USE_NATIVE_EVENTS
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpWindow )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(destroy:) withObject:mpWindow waitUntilDone:YES modes:pModes];
		[mpWindow release];
	}

	[pPool release];
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
	{
		mpVCLFrame->dispose();
		delete mpVCLFrame;
	}
#endif	// USE_NATIVE_EVENTS

	// Delete graphics last as it may be needed by a JavaSalBitmap
	if ( mpGraphics )
		delete mpGraphics;
}

#ifdef USE_NATIVE_WINDOW

// -----------------------------------------------------------------------

::rtl::OUString JavaSalFrame::ConvertVCLKeyCode( USHORT nKeyCode )
{
	sal_Unicode nChar = 0;

	nKeyCode &= KEY_CODE;
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
			nChar = '0' + nKeyCode - KEY_0;
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
			nChar = 'A' + nKeyCode - KEY_A;
			break;
		case KEY_F1:
			nChar = NSF1FunctionKey;
			break;
		case KEY_F2:
			nChar = NSF2FunctionKey;
			break;
		case KEY_F3:
			nChar = NSF3FunctionKey;
			break;
		case KEY_F4:
			nChar = NSF4FunctionKey;
			break;
		case KEY_F5:
			nChar = NSF5FunctionKey;
			break;
		case KEY_F6:
			nChar = NSF6FunctionKey;
			break;
		case KEY_F7:
			nChar = NSF7FunctionKey;
			break;
		case KEY_F8:
			nChar = NSF8FunctionKey;
			break;
		case KEY_F9:
			nChar = NSF9FunctionKey;
			break;
		case KEY_F10:
			nChar = NSF10FunctionKey;
			break;
		case KEY_F11:
			nChar = NSF11FunctionKey;
			break;
		case KEY_F12:
			nChar = NSF12FunctionKey;
			break;
		case KEY_F13:
			nChar = NSF13FunctionKey;
			break;
		case KEY_F14:
			nChar = NSF14FunctionKey;
			break;
		case KEY_F15:
			nChar = NSF15FunctionKey;
			break;
		case KEY_F16:
			nChar = NSF16FunctionKey;
			break;
		case KEY_F17:
			nChar = NSF17FunctionKey;
			break;
		case KEY_F18:
			nChar = NSF18FunctionKey;
			break;
		case KEY_F19:
			nChar = NSF19FunctionKey;
			break;
		case KEY_F20:
			nChar = NSF20FunctionKey;
			break;
		case KEY_F21:
			nChar = NSF21FunctionKey;
			break;
		case KEY_F22:
			nChar = NSF22FunctionKey;
			break;
		case KEY_F23:
			nChar = NSF23FunctionKey;
			break;
		case KEY_F24:
			nChar = NSF24FunctionKey;
			break;
		case KEY_F25:
			nChar = NSF25FunctionKey;
			break;
		case KEY_F26:
			nChar = NSF26FunctionKey;
			break;
		case KEY_DOWN:
			nChar = NSDownArrowFunctionKey;
			break;
		case KEY_UP:
			nChar = NSUpArrowFunctionKey;
			break;
		case KEY_LEFT:
			nChar = NSLeftArrowFunctionKey;
			break;
		case KEY_RIGHT:
			nChar = NSRightArrowFunctionKey;
			break;
		case KEY_HOME:
			nChar = NSHomeFunctionKey;
			break;
		case KEY_END:
			nChar = NSEndFunctionKey;
			break;
		case KEY_PAGEUP:
			nChar = NSPageUpFunctionKey;
			break;
		case KEY_PAGEDOWN:
			nChar = NSPageDownFunctionKey;
			break;
		case KEY_INSERT:
			nChar = NSInsertFunctionKey;
			break;
		case KEY_DELETE:
			nChar = NSDeleteFunctionKey;
			break;
		case KEY_ADD:
			nChar = '+';
			break;
		case KEY_SUBTRACT:
			nChar = '-';
			break;
		case KEY_MULTIPLY:
			nChar = '*';
			break;
		case KEY_DIVIDE:
			nChar = '/';
			break;
		case KEY_DECIMAL:
		case KEY_POINT:
			nChar = '.';
			break;
		case KEY_COMMA:
			nChar = ',';
			break;
		case KEY_LESS:
			nChar = '<';
			break;
		case KEY_GREATER:
			nChar = '>';
			break;
		case KEY_EQUAL:
			nChar = '=';
			break;
		case KEY_UNDO:
			nChar = NSUndoFunctionKey;
			break;
		case KEY_FIND:
			nChar = NSFindFunctionKey;
			break;
		case KEY_HELP:
			nChar = NSHelpFunctionKey;
			break;
		case KEY_TILDE:
			nChar = '~';
			break;
		case KEY_QUOTELEFT:
			nChar = '`';
			break;
		case KEY_RETURN:
		case KEY_ESCAPE:
		case KEY_SPACE:
		case KEY_TAB:
		case KEY_BACKSPACE:
		case KEY_REPEAT:
		case KEY_CUT:
		case KEY_COPY:
		case KEY_PASTE:
		case KEY_PROPERTIES:
		case KEY_OPEN:
		case KEY_FRONT:
		case KEY_CONTEXTMENU:
		case KEY_MENU:
		case KEY_CODE:
		case KEY_HANGUL_HANJA:
		default:
			break;
	}

	if ( nChar )
		return OUString( nChar );
	else
		return OUString();
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

#endif	// USE_NATIVE_WINDOW

// -----------------------------------------------------------------------

unsigned int JavaSalFrame::GetDefaultScreenNumber()
{
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	return com_sun_star_vcl_VCLScreen::getDefaultScreenNumber();
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	// Update if screens have not yet been set
	ResettableGuard< Mutex > aGuard( aScreensMutex );
	if ( !aVCLScreensFullBoundsList.size() || !aVCLScreensVisibleBoundsList.size() )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLUpdateScreens *pVCLUpdateScreens = [VCLUpdateScreens create];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		aGuard.clear();
		[pVCLUpdateScreens performSelectorOnMainThread:@selector(updateScreens:) withObject:pVCLUpdateScreens waitUntilDone:YES modes:pModes];
		aGuard.reset();

		[pPool release];
	}

	return nMainScreen;
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
}

// -----------------------------------------------------------------------

const Rectangle JavaSalFrame::GetScreenBounds( long nX, long nY, long nWidth, long nHeight, sal_Bool bFullScreenMode )
{
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	return com_sun_star_vcl_VCLScreen::getScreenBounds( nX, nY, nWidth, nHeight, bFullScreenMode );
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	// Update if screens have not yet been set
	ResettableGuard< Mutex > aGuard( aScreensMutex );
	if ( !aVCLScreensFullBoundsList.size() || !aVCLScreensVisibleBoundsList.size() )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLUpdateScreens *pVCLUpdateScreens = [VCLUpdateScreens create];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		aGuard.clear();
		[pVCLUpdateScreens performSelectorOnMainThread:@selector(updateScreens:) withObject:pVCLUpdateScreens waitUntilDone:YES modes:pModes];
		aGuard.reset();

		[pPool release];
	}

	// Fix bug 2671 by setting width and height greater than 0
	if ( nWidth <= 0 )
		nWidth = 1;
	if ( nHeight <= 0 )
		nHeight = 1;

	// Iterate through the screens and find the screen that the
	// point is inside of
	Point aPoint( nX, nY );
	::std::vector< Rectangle >::const_iterator it = aVCLScreensFullBoundsList.begin();
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
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
}

// -----------------------------------------------------------------------

const Rectangle JavaSalFrame::GetScreenBounds( unsigned int nScreen, sal_Bool bFullScreenMode )
{
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	return com_sun_star_vcl_VCLScreen::getScreenBounds( nScreen, bFullScreenMode );
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	// Update if screens have not yet been set
	ResettableGuard< Mutex > aGuard( aScreensMutex );
	if ( !aVCLScreensFullBoundsList.size() || !aVCLScreensVisibleBoundsList.size() )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLUpdateScreens *pVCLUpdateScreens = [VCLUpdateScreens create];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		aGuard.clear();
		[pVCLUpdateScreens performSelectorOnMainThread:@selector(updateScreens:) withObject:pVCLUpdateScreens waitUntilDone:YES modes:pModes];
		aGuard.reset();

		[pPool release];
	}

	if ( bFullScreenMode && nScreen < aVCLScreensFullBoundsList.size() )
		return aVCLScreensFullBoundsList[ nScreen ];

	if ( !bFullScreenMode && nScreen < aVCLScreensVisibleBoundsList.size() )
		return aVCLScreensVisibleBoundsList[ nScreen ];
		
	return Rectangle( Point( 0, 0 ), Size( 0, 0 ) );
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
}

// -----------------------------------------------------------------------

unsigned int JavaSalFrame::GetScreenCount()
{
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	return com_sun_star_vcl_VCLScreen::getScreenCount();
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	// Update if screens have not yet been set
	ResettableGuard< Mutex > aGuard( aScreensMutex );
	if ( !aVCLScreensFullBoundsList.size() || !aVCLScreensVisibleBoundsList.size() )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLUpdateScreens *pVCLUpdateScreens = [VCLUpdateScreens create];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		aGuard.clear();
		[pVCLUpdateScreens performSelectorOnMainThread:@selector(updateScreens:) withObject:pVCLUpdateScreens waitUntilDone:YES modes:pModes];
		aGuard.reset();

		[pPool release];
	}

	return ( aVCLScreensFullBoundsList.size() ? aVCLScreensFullBoundsList.size() : 1 );
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
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

bool JavaSalFrame::IsFloatingFrame()
{
	return ( ! ( mnStyle & ( SAL_FRAME_STYLE_DEFAULT | SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE ) ) && this != GetSalData()->mpPresentationFrame && !mbShowOnlyMenus );
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

const Rectangle JavaSalFrame::GetBounds( sal_Bool *pInLiveResize, sal_Bool bUseFullScreenOriginalBounds )
{
	Rectangle aRet( Point( 0, 0 ), Size( 0, 0 ) );

#ifdef USE_NATIVE_EVENTS
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLWindowWrapperArgs *pGetFrameArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObjects:[NSValue valueWithPointer:pInLiveResize], [NSNumber numberWithBool:bUseFullScreenOriginalBounds], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(getFrame:) withObject:pGetFrameArgs waitUntilDone:YES modes:pModes];
		NSValue *pFrame = (NSValue *)[pGetFrameArgs result];
		if ( pFrame )
		{
			NSRect aFrame = [pFrame rectValue];
			aRet = Rectangle( Point( (long)aFrame.origin.x, (long)aFrame.origin.y ), Size( (long)aFrame.size.width, (long)aFrame.size.height ) );
		}

		[pPool release];
	}
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
		aRet = mpVCLFrame->getBounds( pInLiveResize, bUseFullScreenOriginalBounds );
#endif	// USE_NATIVE_EVENTS

	return aRet;
}

// -----------------------------------------------------------------------

const Rectangle JavaSalFrame::GetInsets()
{
	// Insets use the rectangle's data members directly so set members directly
	Rectangle aRet( 0, 0, 0, 0 );

#ifdef USE_NATIVE_EVENTS
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		// Flip native insets
		const NSRect aInsets = [mpWindow insets];
		aRet = Rectangle( (long)aInsets.origin.x, (long)aInsets.size.height, (long)aInsets.size.width, (long)aInsets.origin.y );

		[pPool release];
	}
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
		aRet = mpVCLFrame->getInsets();
#endif	// USE_NATIVE_EVENTS

	return aRet;
}

// -----------------------------------------------------------------------

id JavaSalFrame::GetNativeWindow()
{
	id pRet = nil;

#ifdef USE_NATIVE_EVENTS
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		pRet = [mpWindow window];

		[pPool release];
	}
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
		pRet = (id)mpVCLFrame->getNativeWindow();
#endif	// USE_NATIVE_EVENTS

	return pRet;
}

// -----------------------------------------------------------------------

id JavaSalFrame::GetNativeWindowContentView( sal_Bool bTopLevelWindow )
{
	id pRet = nil;

#ifdef USE_NATIVE_EVENTS
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLWindowWrapperArgs *pGetContentViewArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithBool:bTopLevelWindow]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(getContentView:) withObject:pGetContentViewArgs waitUntilDone:YES modes:pModes];
		pRet = [pGetContentViewArgs result];

		[pPool release];
	}
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
		pRet = (id)mpVCLFrame->getNativeWindowContentView( bTopLevelWindow );
#endif	// USE_NATIVE_EVENTS

	return pRet;
}

// -----------------------------------------------------------------------

ULONG JavaSalFrame::GetState()
{
	ULONG nRet = SAL_FRAMESTATE_MINIMIZED;

#ifdef USE_NATIVE_EVENTS
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLWindowWrapperArgs *pGetStateArgs = [VCLWindowWrapperArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(getContentView:) withObject:pGetStateArgs waitUntilDone:YES modes:pModes];
		NSNumber *pState = (NSNumber *)[pGetStateArgs result];
		if ( pState )
			nRet = [pState unsignedLongValue];

		[pPool release];
	}
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
		nRet = mpVCLFrame->getState();
#endif	// USE_NATIVE_EVENTS

	return nRet;
}

// -----------------------------------------------------------------------

void JavaSalFrame::MakeModal()
{
#ifdef USE_NATIVE_EVENTS
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(makeModal:) withObject:mpWindow waitUntilDone:YES modes:pModes];

		[pPool release];
	}
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
		mpVCLFrame->makeModal();
#endif	// USE_NATIVE_EVENTS
}

// -----------------------------------------------------------------------

bool JavaSalFrame::RequestFocus()
{
	bool bRet = false;

#ifdef USE_NATIVE_EVENTS
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
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
		bRet = mpVCLFrame->requestFocus();
#endif	// USE_NATIVE_EVENTS

	return bRet;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetState( ULONG nFrameState )
{
#ifdef USE_NATIVE_EVENTS
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		ULONG nState;
		if ( nFrameState & SAL_FRAMESTATE_MINIMIZED )
			nState = SAL_FRAMESTATE_MINIMIZED;
		else if ( nFrameState & SAL_FRAMESTATE_MAXIMIZED )
			nState = SAL_FRAMESTATE_MAXIMIZED;
		else
			nState = SAL_FRAMESTATE_NORMAL;

		VCLWindowWrapperArgs *pSetStateArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithUnsignedLong:nState]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(setState:) withObject:pSetStateArgs waitUntilDone:YES modes:pModes];

		[pPool release];
	}
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
	{
		if ( nFrameState & SAL_FRAMESTATE_MINIMIZED )
			mpVCLFrame->setState( SAL_FRAMESTATE_MINIMIZED );
		else
			mpVCLFrame->setState( SAL_FRAMESTATE_NORMAL );
	}
#endif	// USE_NATIVE_EVENTS
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetVisible( sal_Bool bVisible, sal_Bool bNoActivate )
{
#ifdef USE_NATIVE_EVENTS
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLWindowWrapperArgs *pSetVisibleArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithBool:bVisible], [NSNumber numberWithBool:bNoActivate], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(setVisible:) withObject:pSetVisibleArgs waitUntilDone:YES modes:pModes];

		[pPool release];
	}
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
		mpVCLFrame->setVisible( bVisible, bNoActivate );
#endif	// USE_NATIVE_EVENTS
}

// -----------------------------------------------------------------------

bool JavaSalFrame::ToFront()
{
	bool bRet = false;

#ifdef USE_NATIVE_EVENTS
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
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
		bRet = mpVCLFrame->toFront();
#endif	// USE_NATIVE_EVENTS

	return bRet;
}

#ifdef USE_NATIVE_WINDOW

// -----------------------------------------------------------------------

void JavaSalFrame::UpdateLayer()
{
#ifdef USE_NATIVE_EVENTS
	CGSize aLayerSize = CGSizeMake( maGeometry.nWidth, maGeometry.nHeight );
#else	// USE_NATIVE_EVENTS
	CGSize aLayerSize = CGSizeMake( maGeometry.nWidth + maGeometry.nLeftDecoration + maGeometry.nRightDecoration, maGeometry.nHeight + maGeometry.nTopDecoration + maGeometry.nBottomDecoration );
#endif	// USE_NATIVE_EVENTS
	if ( maFrameLayer && maSysData.pView && CGSizeEqualToSize( CGLayerGetSize( maFrameLayer ), aLayerSize ) )
		return;

	if ( maFrameLayer )
	{
		CGLayerRelease( maFrameLayer );
		maFrameLayer = NULL;
	}

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	VCLViewGetGraphicsLayer *pVCLViewGetGraphicsLayer = [VCLViewGetGraphicsLayer createGraphicsLayer:mpGraphics view:maSysData.pView size:aLayerSize];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLViewGetGraphicsLayer performSelectorOnMainThread:@selector(getGraphicsLayer:) withObject:pVCLViewGetGraphicsLayer waitUntilDone:YES modes:pModes];
	maFrameLayer = [pVCLViewGetGraphicsLayer layer];
	if ( maFrameLayer )
		CGLayerRetain( maFrameLayer );

	[pPool release];

	if ( maFrameLayer )
	{
		mpGraphics->setLayer( maFrameLayer );

		// Post a paint event
		JavaSalEvent aEvent( SALEVENT_PAINT, this, new SalPaintEvent( 0, 0, aLayerSize.width, aLayerSize.height ) );
		JavaSalEventQueue::postCachedEvent( &aEvent );
	}
	else
	{
		mpGraphics->setLayer( maHiddenLayer );
	}
}

#endif	// USE_NATIVE_WINDOW

// -----------------------------------------------------------------------

SalGraphics* JavaSalFrame::GetGraphics()
{
	if ( mbGraphics )
		return NULL;

#ifndef USE_NATIVE_WINDOW
	if ( !mpGraphics->mpVCLGraphics && mpVCLFrame )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
#endif	// !USE_NATIVE_WINDOW
	mbGraphics = TRUE;

	return mpGraphics;
}

// -----------------------------------------------------------------------

void JavaSalFrame::ReleaseGraphics( SalGraphics* pGraphics )
{
	if ( pGraphics != mpGraphics )
		return;

	mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL JavaSalFrame::PostEvent( void *pData )
{
	JavaSalEvent aEvent( SALEVENT_USEREVENT, this, pData );
	JavaSalEventQueue::postCachedEvent( &aEvent );
	return TRUE;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetTitle( const XubString& rTitle )
{
	maTitle = OUString( rTitle );
#ifdef USE_NATIVE_EVENTS
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSString *pTitle = [NSString stringWithCharacters:maTitle.getStr() length:maTitle.getLength()];
		VCLWindowWrapperArgs *pSetTitleArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObject:pTitle]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(setTitle:) withObject:pSetTitleArgs waitUntilDone:YES modes:pModes];

		[pPool release];
	}
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
		mpVCLFrame->setTitle( maTitle );
#endif	// USE_NATIVE_EVENTS
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetIcon( USHORT nIcon )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::SetIcon not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalFrame::Show( BOOL bVisible, BOOL bNoActivate )
{
	// Don't allow floating children of a show only menus frame to ever show
	if ( mpParent && mpParent->mbShowOnlyMenus && IsFloatingFrame() )
		bVisible = FALSE;

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
			(*it)->Show( FALSE );

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

#ifndef USE_NATIVE_WINDOW
	// Make sure there is a graphics available to avoid crashing when the OOo
	// code tries to draw while updating the menus
	if ( !mpGraphics->mpVCLGraphics && mpVCLFrame )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
#endif	// !USE_NATIVE_WINDOW

	SetVisible( mbVisible, bNoActivate );

#ifndef USE_NATIVE_WINDOW
	// Reset graphics
	if ( mpGraphics->mpVCLGraphics )
		mpGraphics->mpVCLGraphics->resetGraphics();
#endif	// !USE_NATIVE_WINDOW

	if ( mbVisible )
	{
		mbInShow = TRUE;

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
		sal_Bool bTopLevelWindow = sal_False;
		if ( !mpParent && !IsFloatingFrame() && !IsUtilityWindow() )
		{
			Window *pWindow = Application::GetFirstTopLevelWindow();
			while ( pWindow && pWindow->ImplGetFrame() != this )
				pWindow = Application::GetNextTopLevelWindow( pWindow );

			if ( pWindow )
				bTopLevelWindow = sal_True;
		}

		maSysData.pView = (NSView *)GetNativeWindowContentView( bTopLevelWindow );
		mbCenter = FALSE;

		JavaSalEvent aEvent( SALEVENT_MOVERESIZE, this, NULL );
		aEvent.dispatch();

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
			(*it)->Show( TRUE );

		// Explicitly set focus to this frame since Java may set the focus
		// to the child frame
		if ( !bNoActivate )
			ToTop( SAL_FRAME_TOTOP_RESTOREWHENMIN | SAL_FRAME_TOTOP_GRABFOCUS );

		mbInShow = FALSE;
	}
	else
	{
		// End composition
		JavaSalEvent aEvent( SALEVENT_ENDEXTTEXTINPUT, this, NULL );
		aEvent.dispatch();

		// Remove the native window since it is destroyed when hidden
		maSysData.pView = NULL;

		// Unset focus but don't set focus to another frame as it will cause
		// menu shortcuts to be disabled if we go into show only menus mode
		// after closing a window whose child window had focus
		if ( pSalData->mpFocusFrame == this )
		{
			JavaSalEvent aFocusEvent( SALEVENT_LOSEFOCUS, this, NULL );
			aFocusEvent.dispatch();
		}

		if ( pSalData->mpLastDragFrame == this )
			pSalData->mpLastDragFrame = NULL;

#ifdef USE_NATIVE_WINDOW
		if ( maFrameLayer )
		{
			CGLayerRelease( maFrameLayer );
			maFrameLayer = NULL;
		}

		mpGraphics->setLayer( maHiddenLayer );
#endif	// USE_NATIVE_WINDOW

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
			pShowOnlyMenusFrame->Show( TRUE, FALSE );
	}

#ifdef USE_NATIVE_WINDOW
	UpdateLayer();
#endif	// USE_NATIVE_WINDOW
}

// -----------------------------------------------------------------------

void JavaSalFrame::Enable( BOOL bEnable )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::Enable not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetMinClientSize( long nWidth, long nHeight )
{
#ifdef USE_NATIVE_EVENTS
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSSize aMinSize = NSMakeSize( nWidth, nHeight );
		VCLWindowWrapperArgs *pSetMinSizeArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObject:[NSValue valueWithSize:aMinSize]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(setMinSize:) withObject:pSetMinSizeArgs waitUntilDone:YES modes:pModes];

		[pPool release];
	}
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
		mpVCLFrame->setMinClientSize( nWidth, nHeight );
#endif	// USE_NATIVE_EVENTS
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight,
							USHORT nFlags )
{
	if ( mnStyle & SAL_FRAME_STYLE_SYSTEMCHILD )
		return;

	mbInSetPosSize = TRUE;

	Rectangle aPosSize( Point( maGeometry.nX - maGeometry.nLeftDecoration, maGeometry.nY - maGeometry.nTopDecoration ), Size( maGeometry.nWidth, maGeometry.nHeight ) );

	if ( ! ( nFlags & SAL_FRAME_POSSIZE_X ) )
		nX = aPosSize.nLeft;
	if ( ! ( nFlags & SAL_FRAME_POSSIZE_Y ) )
		nY = aPosSize.nTop;
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
		nParentX = aParentBounds.nLeft + mpParent->maGeometry.nLeftDecoration;
		nParentY = aParentBounds.nTop + mpParent->maGeometry.nTopDecoration;

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

			nX = aWorkArea.nLeft + ( ( aWorkArea.GetWidth() - nWidth ) / 2 );
			nY = aWorkArea.nTop + ( ( aWorkArea.GetHeight() - nHeight ) / 2 );
		}

		mbCenter = FALSE;
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
	}

	// Make sure window does not spill off of the screen
	long nMinX = aWorkArea.nLeft;
	long nMinY = aWorkArea.nTop;
	if ( mbPresentation )
	{
		nMinX -= 1;
		nMinY -= 1;
	}
	nWidth += maGeometry.nLeftDecoration + maGeometry.nRightDecoration;
	nHeight += maGeometry.nTopDecoration + maGeometry.nBottomDecoration;
	if ( nMinX + nWidth > aWorkArea.nLeft + aWorkArea.GetWidth() )
		nWidth = aWorkArea.nLeft + aWorkArea.GetWidth() - nMinX;
	if ( nMinY + nHeight > aWorkArea.nTop + aWorkArea.GetHeight() )
		nHeight = aWorkArea.nTop + aWorkArea.GetHeight() - nMinY;
	if ( nX < nMinX )
		nX = nMinX;
	if ( nY < nMinY )
		nY = nMinY;

	// Fix bug 1420 by not restricting width or height to work area for current
	// drag frame
	if ( this != GetSalData()->mpLastDragFrame )
	{
		if ( nX + nWidth > aWorkArea.nLeft + aWorkArea.GetWidth() )
			nX = aWorkArea.nLeft + aWorkArea.GetWidth() - nWidth;
		if ( nY + nHeight > aWorkArea.nTop + aWorkArea.GetHeight() )
			nY = aWorkArea.nTop + aWorkArea.GetHeight() - nHeight;
	}

#ifdef USE_NATIVE_EVENTS
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSRect aFrame = NSMakeRect( nX, nY, nWidth, nHeight );
		VCLWindowWrapperArgs *pSetFrameArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObject:[NSValue valueWithRect:aFrame]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(setFrame:) withObject:pSetFrameArgs waitUntilDone:YES modes:pModes];

		[pPool release];
	}
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
		mpVCLFrame->setBounds( nX, nY, nWidth, nHeight );
#endif	// USE_NATIVE_EVENTS

	// Update the cached position immediately
	JavaSalEvent aEvent( SALEVENT_MOVERESIZE, this, NULL );
	aEvent.dispatch();

	mbInSetPosSize = FALSE;
}

// -----------------------------------------------------------------------

void JavaSalFrame::GetWorkArea( Rectangle &rRect )
{
	SalData *pSalData = GetSalData();
	sal_Bool bFullScreenMode = ( pSalData->mpPresentationFrame || ( this == pSalData->mpLastDragFrame ) );
	long nX = rRect.nLeft;
	long nY = rRect.nTop;
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
	USHORT nFlags = 0;
	if ( pState->mnMask & SAL_FRAMESTATE_MASK_X )
		nFlags |= SAL_FRAME_POSSIZE_X;
	if ( pState->mnMask & SAL_FRAMESTATE_MASK_Y )
		nFlags |= SAL_FRAME_POSSIZE_Y;
	if ( pState->mnMask & SAL_FRAMESTATE_MASK_WIDTH )
		nFlags |= SAL_FRAME_POSSIZE_WIDTH;
	if ( pState->mnMask & SAL_FRAMESTATE_MASK_HEIGHT )
		nFlags |= SAL_FRAME_POSSIZE_HEIGHT;
	if ( nFlags )
	{
		JavaSalFrame *pParent = mpParent;
		mpParent = NULL;
		SetPosSize( pState->mnX, pState->mnY, pState->mnWidth, pState->mnHeight, nFlags );
		mpParent = pParent;
	}

	// Fix bug 3078 by setting the state after setting the size
	if ( pState->mnMask & SAL_FRAMESTATE_MASK_STATE )
		SetState( pState->mnState );
}

// -----------------------------------------------------------------------

BOOL JavaSalFrame::GetWindowState( SalFrameState* pState )
{
	Rectangle aBounds( GetBounds( NULL, sal_True ) );
	pState->mnMask = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT | SAL_FRAMESTATE_MASK_STATE;
	pState->mnX = aBounds.Left();
	pState->mnY = aBounds.Top();
	pState->mnWidth = aBounds.GetWidth() - maGeometry.nLeftDecoration - maGeometry.nRightDecoration;
	pState->mnHeight = aBounds.GetHeight() - maGeometry.nTopDecoration - maGeometry.nBottomDecoration;
	pState->mnState = GetState();

	// Fix bug 3012 by returning false if the frame size is not larger than
	// the frame's minimum size
	if ( !maGeometry.nWidth || !maGeometry.nHeight )
		return FALSE;
	else
		return TRUE;
}

// -----------------------------------------------------------------------

void JavaSalFrame::ShowFullScreen( BOOL bFullScreen, sal_Int32 nDisplay )
{
	if ( mbInShowFullScreen || bFullScreen == mbFullScreen )
		return;

	mbInShowFullScreen = TRUE;
	if ( !mbInWindowDidExitFullScreen && !mbInWindowWillEnterFullScreen )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSWindow *pNSWindow = (NSWindow *)GetNativeWindow();
		if ( pNSWindow )
		{
			VCLToggleFullScreen *pVCLToggleFullScreen = [VCLToggleFullScreen createToggleFullScreen:pNSWindow];
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		    ULONG nCount = Application::ReleaseSolarMutex();
			[pVCLToggleFullScreen performSelectorOnMainThread:@selector(toggleFullScreen:) withObject:pVCLToggleFullScreen waitUntilDone:YES modes:pModes];
		    Application::AcquireSolarMutex( nCount );
		}

		[pPool release];
	}

	USHORT nFlags = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT;
	if ( bFullScreen )
	{
		memcpy( &maOriginalGeometry, &maGeometry, sizeof( SalFrameGeometry ) );

		JavaSalSystem *pSalSystem = (JavaSalSystem *)ImplGetSalSystem();
		Rectangle aWorkArea;
		if ( pSalSystem )
			aWorkArea = pSalSystem->GetDisplayWorkAreaPosSizePixel( nDisplay );
		if ( aWorkArea.IsEmpty() )
			aWorkArea = Rectangle( Point( maGeometry.nX - maGeometry.nLeftDecoration, maGeometry.nY - maGeometry.nTopDecoration ), Size( maGeometry.nWidth, maGeometry.nHeight ) );
		GetWorkArea( aWorkArea );
		SetPosSize( aWorkArea.nLeft, aWorkArea.nTop, aWorkArea.GetWidth() - maGeometry.nLeftDecoration - maGeometry.nRightDecoration, aWorkArea.GetHeight() - maGeometry.nTopDecoration - maGeometry.nBottomDecoration, nFlags );
	}
	else
	{
		SetPosSize( maOriginalGeometry.nX - maOriginalGeometry.nLeftDecoration, maOriginalGeometry.nY - maOriginalGeometry.nTopDecoration, maOriginalGeometry.nWidth, maOriginalGeometry.nHeight, nFlags );
		memset( &maOriginalGeometry, 0, sizeof( SalFrameGeometry ) );
	}

#ifdef USE_NATIVE_EVENTS
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		VCLWindowWrapperArgs *pSetFullScreenModeArgs = [VCLWindowWrapperArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithBool:bFullScreen]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(setFullScreenMode:) withObject:pSetFullScreenModeArgs waitUntilDone:YES modes:pModes];

		[pPool release];
	}
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
		mpVCLFrame->setFullScreenMode( bFullScreen );
#endif	// USE_NATIVE_EVENTS
	mbFullScreen = bFullScreen;
	mbInShowFullScreen = FALSE;
}

// -----------------------------------------------------------------------

void JavaSalFrame::StartPresentation( BOOL bStart )
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
		USHORT nFlags = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT;

		Rectangle aWorkArea( Point( maGeometry.nX - maGeometry.nLeftDecoration, maGeometry.nY - maGeometry.nTopDecoration ), Size( maGeometry.nWidth, maGeometry.nHeight ) );
		GetWorkArea( aWorkArea );

		SetPosSize( aWorkArea.nLeft, aWorkArea.nTop, aWorkArea.GetWidth() - maGeometry.nLeftDecoration - maGeometry.nRightDecoration, aWorkArea.GetHeight() - maGeometry.nTopDecoration - maGeometry.nBottomDecoration, nFlags );
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

void JavaSalFrame::SetAlwaysOnTop( BOOL bOnTop )
{
}

// -----------------------------------------------------------------------

void JavaSalFrame::ToTop( USHORT nFlags )
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

		if ( bModal )
			bSuccess = pFrame->ToFront();
		else
			bSuccess = pFrame->RequestFocus();
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
		JavaSalEvent aEvent( SALEVENT_GETFOCUS, pFrame, NULL );
		aEvent.dispatch();
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetPointer( PointerStyle ePointerStyle )
{
#ifdef USE_NATIVE_WINDOW
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	VCLSetCursor *pVCLSetCursor = [VCLSetCursor createWithPointerStyle:ePointerStyle view:maSysData.pView];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pVCLSetCursor performSelectorOnMainThread:@selector(setCursor:) withObject:pVCLSetCursor waitUntilDone:NO modes:pModes];

	[pPool release];
#else	// USE_NATIVE_WINDOW
	if ( mpVCLFrame )
		mpVCLFrame->setPointer( ePointerStyle );
#endif	// USE_NATIVE_WINDOW
}

// -----------------------------------------------------------------------

void JavaSalFrame::CaptureMouse( BOOL bCapture )
{
	SalData *pSalData = GetSalData();
	if ( bCapture )
		pSalData->mpCaptureFrame = this;
	else
		pSalData->mpCaptureFrame = NULL;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetPointerPos( long nX, long nY )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::SetPointerPos not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalFrame::Flush()
{
#ifdef USE_NATIVE_EVENTS
	if ( mpWindow )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpWindow performSelectorOnMainThread:@selector(flush:) withObject:mpWindow waitUntilDone:YES modes:pModes];

		[pPool release];
	}
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
		mpVCLFrame->sync();
#endif	// USE_NATIVE_EVENTS
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
#ifdef USE_NATIVE_EVENTS
	fprintf( stderr, "JavaSalFrame::SetInputContext not implemented\n" );
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
	{
		if ( pContext && pContext->mnOptions & SAL_INPUTCONTEXT_TEXT )
			mpVCLFrame->setAllowKeyBindings( sal_True );
		else
			mpVCLFrame->setAllowKeyBindings( sal_False );
	}
#endif	// USE_NATIVE_EVENTS
}

// -----------------------------------------------------------------------

void JavaSalFrame::EndExtTextInput( USHORT nFlags )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::EndExtTextInput not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

XubString JavaSalFrame::GetKeyName( USHORT nKeyCode )
{
	XubString aRet;

#ifdef USE_NATIVE_EVENTS
	fprintf( stderr, "JavaSalFrame::GetKeyName not implemented\n" );
#else	// USE_NATIVE_EVENTS
	if ( mpVCLFrame )
		aRet = XubString( mpVCLFrame->getKeyName( nKeyCode ) );
#endif	// USE_NATIVE_EVENTS

	return aRet;
}

// -----------------------------------------------------------------------

XubString JavaSalFrame::GetSymbolKeyName( const XubString&, USHORT nKeyCode )
{
	return GetKeyName( nKeyCode );
}

// -----------------------------------------------------------------------

void JavaSalFrame::UpdateSettings( AllSettings& rSettings )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	MouseSettings aMouseSettings = rSettings.GetMouseSettings();
	float fDoubleClickThreshold = 0;
	NSUserDefaults *pDefaults = [NSUserDefaults standardUserDefaults];
	if ( pDefaults )
		fDoubleClickThreshold = [pDefaults floatForKey:@"com.apple.mouse.doubleClickThreshold"];

	if ( fDoubleClickThreshold > 0 )
	{
		if ( fDoubleClickThreshold < 0.25 )
			fDoubleClickThreshold = 0.25;
		aMouseSettings.SetDoubleClickTime( fDoubleClickThreshold * 1000 );
	}
	else
	{
		ULONG nDblTime = 25;
		void *pLib = dlopen( NULL, RTLD_LAZY | RTLD_LOCAL );
		if ( pLib )
		{
			GetDblTime_Type *pGetDblTime = (GetDblTime_Type *)dlsym( pLib, "GetDblTime" );
			if ( pGetDblTime )
			{
				nDblTime = (ULONG)pGetDblTime();
				if ( nDblTime < 25 )
					nDblTime = 25;
			}

			dlclose( pLib );
		}
		aMouseSettings.SetDoubleClickTime( nDblTime * 1000 / CLK_TCK );
	}
	aMouseSettings.SetStartDragWidth( 6 );
	aMouseSettings.SetStartDragHeight( 6 );
	rSettings.SetMouseSettings( aMouseSettings );

	StyleSettings aStyleSettings( rSettings.GetStyleSettings() );

	long nBlinkRate = 500;
	if ( pDefaults )
	{
		nBlinkRate = [pDefaults integerForKey:@"NSTextInsertionPointBlinkPeriod"];
		if ( nBlinkRate < 500 )
			nBlinkRate = 500;
	}
	aStyleSettings.SetCursorBlinkTime( nBlinkRate );

	// Update colors if any system colors have not yet been set
	ResettableGuard< Mutex > aGuard( aSystemColorsMutex );
	if ( !pVCLControlTextColor || !pVCLTextColor || !pVCLHighlightColor || !pVCLHighlightTextColor || !pVCLDisabledControlTextColor || !pVCLBackColor )
	{
		VCLUpdateSystemColors *pVCLUpdateSystemColors = [VCLUpdateSystemColors create];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		aGuard.clear();
		[pVCLUpdateSystemColors performSelectorOnMainThread:@selector(updateSystemColors:) withObject:pVCLUpdateSystemColors waitUntilDone:YES modes:pModes];
		aGuard.reset();
	}

	BOOL useThemeDialogColor = FALSE;
	Color themeDialogColor;
	if ( pVCLControlTextColor )
	{
		themeDialogColor = Color( (unsigned char)( [pVCLControlTextColor redComponent] * 0xff ), (unsigned char)( [pVCLControlTextColor greenComponent] * 0xff ), (unsigned char)( [pVCLControlTextColor blueComponent] * 0xff ) );
		useThemeDialogColor = TRUE;
	}

#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	SalColor nTextTextColor = com_sun_star_vcl_VCLScreen::getTextTextColor();
	Color aTextColor( SALCOLOR_RED( nTextTextColor ), SALCOLOR_GREEN( nTextTextColor ), SALCOLOR_BLUE( nTextTextColor ) );
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	Color aTextColor;
	if ( pVCLTextColor )
		aTextColor = Color( (unsigned char)( [pVCLTextColor redComponent] * 0xff ), (unsigned char)( [pVCLTextColor greenComponent] * 0xff ), (unsigned char)( [pVCLTextColor blueComponent] * 0xff ) );
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	aStyleSettings.SetDialogTextColor( ( useThemeDialogColor ) ? themeDialogColor : aTextColor );
	aStyleSettings.SetMenuTextColor( aTextColor );
	aStyleSettings.SetButtonTextColor( ( useThemeDialogColor) ? themeDialogColor : aTextColor );
	aStyleSettings.SetRadioCheckTextColor( ( useThemeDialogColor ) ? themeDialogColor : aTextColor );
	aStyleSettings.SetGroupTextColor( ( useThemeDialogColor ) ? themeDialogColor : aTextColor );
	aStyleSettings.SetLabelTextColor( ( useThemeDialogColor ) ? themeDialogColor : aTextColor );
	aStyleSettings.SetInfoTextColor( aTextColor );
	aStyleSettings.SetWindowTextColor( aTextColor );
	aStyleSettings.SetFieldTextColor( aTextColor );

#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	SalColor nTextHighlightColor = com_sun_star_vcl_VCLScreen::getTextHighlightColor();
	Color aHighlightColor( SALCOLOR_RED( nTextHighlightColor ), SALCOLOR_GREEN( nTextHighlightColor ), SALCOLOR_BLUE( nTextHighlightColor ) );
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	Color aHighlightColor;
	if ( pVCLHighlightColor )
		aHighlightColor = Color( (unsigned char)( [pVCLHighlightColor redComponent] * 0xff ), (unsigned char)( [pVCLHighlightColor greenComponent] * 0xff ), (unsigned char)( [pVCLHighlightColor blueComponent] * 0xff ) );
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	aStyleSettings.SetActiveBorderColor( aHighlightColor );
	aStyleSettings.SetActiveColor( aHighlightColor );
	aStyleSettings.SetActiveTextColor( aHighlightColor );
	aStyleSettings.SetHighlightColor( aHighlightColor );
	aStyleSettings.SetMenuHighlightColor( aHighlightColor );

#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	SalColor nTextHighlightTextColor = com_sun_star_vcl_VCLScreen::getTextHighlightTextColor();
	Color aHighlightTextColor( SALCOLOR_RED( nTextHighlightTextColor ), SALCOLOR_GREEN( nTextHighlightTextColor ), SALCOLOR_BLUE( nTextHighlightTextColor ) );
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	Color aHighlightTextColor;
	if ( pVCLHighlightTextColor )
		aHighlightTextColor = Color( (unsigned char)( [pVCLHighlightTextColor redComponent] * 0xff ), (unsigned char)( [pVCLHighlightTextColor greenComponent] * 0xff ), (unsigned char)( [pVCLHighlightTextColor blueComponent] * 0xff ) );
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	aStyleSettings.SetHighlightTextColor( aHighlightTextColor );
	aStyleSettings.SetMenuHighlightTextColor( aHighlightTextColor );

	useThemeDialogColor = FALSE;
	if ( pVCLDisabledControlTextColor )
	{
		themeDialogColor = Color( (unsigned char)( [pVCLDisabledControlTextColor redComponent] * 0xff ), (unsigned char)( [pVCLDisabledControlTextColor greenComponent] * 0xff ), (unsigned char)( [pVCLDisabledControlTextColor blueComponent] * 0xff ) );
		useThemeDialogColor = TRUE;
	}

#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	SalColor nControlColor = com_sun_star_vcl_VCLScreen::getControlColor();
	Color aBackColor( SALCOLOR_RED( nControlColor ), SALCOLOR_GREEN( nControlColor ), SALCOLOR_BLUE( nControlColor ) );
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	Color aBackColor;
	if ( pVCLBackColor )
		aBackColor = Color( (unsigned char)( [pVCLBackColor redComponent] * 0xff ), (unsigned char)( [pVCLBackColor greenComponent] * 0xff ), (unsigned char)( [pVCLBackColor blueComponent] * 0xff ) );
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	aStyleSettings.Set3DColors( aBackColor );
	aStyleSettings.SetDeactiveBorderColor( aBackColor );
	aStyleSettings.SetDeactiveColor( aBackColor );
	aStyleSettings.SetDeactiveTextColor( ( useThemeDialogColor ) ? themeDialogColor : aBackColor );
	aStyleSettings.SetDialogColor( aBackColor );
	aStyleSettings.SetDisableColor( aBackColor );
	aStyleSettings.SetFaceColor( aBackColor );
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
		aStyleSettings.SetCheckedColor( Color( (BYTE)( ( (USHORT)aBackColor.GetRed() + (USHORT)aColor2.GetRed() ) / 2 ), (BYTE)( ( (USHORT)aBackColor.GetGreen() + (USHORT)aColor2.GetGreen() ) / 2 ), (BYTE)( ( (USHORT)aBackColor.GetBlue() + (USHORT)aColor2.GetBlue() ) / 2 ) ) );
	}

	rSettings.SetStyleSettings( aStyleSettings );

	[pPool release];
}

// -----------------------------------------------------------------------

SalBitmap* JavaSalFrame::SnapShot()
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::Snapshot not implemented\n" );
#endif
	return NULL;
}

// -----------------------------------------------------------------------

const SystemEnvData* JavaSalFrame::GetSystemData() const
{
	return &maSysData;
}

// -----------------------------------------------------------------------

void JavaSalFrame::Beep( SoundType eSoundType )
{
#ifdef USE_NATIVE_WINDOW
	AudioServicesPlaySystemSound( kUserPreferredAlert );
#else	// USE_NATIVE_WINDOW
#if !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	com_sun_star_vcl_VCLGraphics::beep();
#endif	// !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
#endif	// USE_NATIVE_WINDOW
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
	{
		mpParent->maChildren.remove( this );
#ifndef USE_NATIVE_EVENTS
		if ( mpParent->mpVCLFrame )
			mpParent->mpVCLFrame->removeChild( this );
#endif	// !USE_NATIVE_EVENTS
	}

	mpParent = (JavaSalFrame *)pNewParent;

	// Utility windows should never be attached to a parent window.
	if ( !bUtilityWindow )
	{
		::std::list< JavaSalObject* > aReshowObjects( maVisibleObjects );
		bool bReshow = mbVisible;
		if ( bReshow )
			Show( FALSE );

		// Fix bug 1310 by creating a new native window with the new parent
		maSysData.pView = NULL;
#ifdef USE_NATIVE_EVENTS
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		// Prevent deadlock when opening a document when toolbars are
		// showing and we are in show only menus mode.
		VCLWindow *pParentWindow = nil;
		if ( mpParent && mpParent->mpWindow && !mpParent->mbShowOnlyMenus )
			pParentWindow = [(VCLWindowWrapper *)mpParent->mpWindow window];

		VCLCreateWindow *pVCLCreateWindow = [VCLCreateWindow createWithStyle:mnStyle frame:this parent:pParentWindow showOnlyMenus:mbShowOnlyMenus utility:IsUtilityWindow()];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pVCLCreateWindow performSelectorOnMainThread:@selector(createWindow:) withObject:pVCLCreateWindow waitUntilDone:YES modes:pModes];
		VCLWindowWrapper *pWindow = [pVCLCreateWindow window];
		if ( pWindow )
		{
			// Release old window wrapper
			if ( mpWindow )
				[mpWindow performSelectorOnMainThread:@selector(destroy:) withObject:mpWindow waitUntilDone:YES modes:pModes];

			[pWindow retain];
			mpWindow = pWindow;
		}

		[pPool release];
#else	// USE_NATIVE_EVENTS
		com_sun_star_vcl_VCLFrame *pOldVCLFrame = mpVCLFrame;
#ifndef USE_NATIVE_WINDOW
		com_sun_star_vcl_VCLGraphics *pOldVCLGraphics = mpGraphics->mpVCLGraphics;
#endif	// !USE_NATIVE_WINDOW

		mpVCLFrame = new com_sun_star_vcl_VCLFrame( mnStyle, this, mpParent, mbShowOnlyMenus, bUtilityWindow );
		if ( mpVCLFrame )
		{
#ifndef USE_NATIVE_WINDOW
			mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
#endif	// !USE_NATIVE_WINDOW
			mpVCLFrame->setTitle( maTitle );

#ifndef USE_NATIVE_WINDOW
			mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
			if ( pOldVCLGraphics )
				delete pOldVCLGraphics;
#endif	// !USE_NATIVE_WINDOW

			if ( pOldVCLFrame )
			{
				pOldVCLFrame->dispose();
				delete pOldVCLFrame;
			}
		}
		else
		{
			mpVCLFrame = pOldVCLFrame;
#ifndef USE_NATIVE_WINDOW
			mpGraphics->mpVCLGraphics = pOldVCLGraphics;
#endif	// !USE_NATIVE_WINDOW
		}
#endif	// USE_NATIVE_EVENTS

		if ( mpParent )
			SetPosSize( maGeometry.nX - mpParent->maGeometry.nX - mpParent->maGeometry.nLeftDecoration, maGeometry.nY - mpParent->maGeometry.nY - mpParent->maGeometry.nTopDecoration, maGeometry.nWidth, maGeometry.nHeight, SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
		else
			SetPosSize( maGeometry.nX - maGeometry.nLeftDecoration, maGeometry.nY - maGeometry.nTopDecoration, maGeometry.nWidth, maGeometry.nHeight, SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );

		if ( bReshow )
		{
			Show( TRUE, FALSE );
			for ( ::std::list< JavaSalObject* >::const_iterator it = aReshowObjects.begin(); it != aReshowObjects.end(); ++it )
					(*it)->Show( TRUE );
		}
	}

	if ( mpParent )
	{
#ifndef USE_NATIVE_EVENTS
		if ( mpParent->mpVCLFrame )
			mpParent->mpVCLFrame->addChild( this );
#endif	// !USE_NATIVE_EVENTS
		mpParent->maChildren.push_back( this );
	}

	if ( !bUtilityWindow )
	{
		// Reattach floating children
		::std::list< JavaSalFrame* >aChildren( maChildren );
		for ( ::std::list< JavaSalFrame* >::const_iterator it = aChildren.begin(); it != aChildren.end(); ++it )
			(*it)->SetParent( this );
	}
}

// -----------------------------------------------------------------------

bool JavaSalFrame::SetPluginParent( SystemParentData* pNewParent )
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

BOOL JavaSalFrame::MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, KeyCode& rKeyCode )
{
	return FALSE;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetExtendedFrameStyle( SalExtStyle nExtStyle )
{
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetBackgroundBitmap( SalBitmap* )
{
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetMaxClientSize( long nWidth, long nHeight )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::SetMaxClientSize not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

void JavaSalFrame::ResetClipRegion()
{
#ifdef USE_NATIVE_WINDOW
	if ( maFrameClipPath )
	{
		CGPathRelease( maFrameClipPath );
		maFrameClipPath = NULL;
		mpGraphics->setFrameClipPath( maFrameClipPath );
	}
#else	// USE_NATIVE_WINDOW
#if !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	if ( !mpGraphics->mpVCLGraphics && mpVCLFrame )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
	mpGraphics->mpVCLGraphics->resetClipRegion( sal_True );
#endif	// !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
#endif	// USE_NATIVE_WINDOW
}

// -----------------------------------------------------------------------

void JavaSalFrame::BeginSetClipRegion( ULONG nRects )
{
#ifdef USE_NATIVE_WINDOW
	ResetClipRegion();
#else	// USE_NATIVE_WINDOW
#if !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	if ( !mpGraphics->mpVCLGraphics && mpVCLFrame )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
	mpGraphics->mpVCLGraphics->beginSetClipRegion( sal_True );
#endif	// !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
#endif	// USE_NATIVE_WINDOW
}

// -----------------------------------------------------------------------

void JavaSalFrame::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
#ifdef USE_NATIVE_WINDOW
	CGRect aRect = CGRectStandardize( CGRectMake( nX, nY, nWidth, nHeight ) );
	if ( !CGRectIsEmpty( aRect ) )
	{
		if ( !maFrameClipPath )
			maFrameClipPath = CGPathCreateMutable();

		if ( maFrameClipPath )
			CGPathAddRect( maFrameClipPath, NULL, aRect );
	}
#else	// USE_NATIVE_WINDOW
#if !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	if ( !mpGraphics->mpVCLGraphics && mpVCLFrame )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
	mpGraphics->mpVCLGraphics->unionClipRegion( nX, nY, nWidth, nHeight, sal_True );
#endif	// !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
#endif	// USE_NATIVE_WINDOW
}

// -----------------------------------------------------------------------

void JavaSalFrame::EndSetClipRegion()
{
#ifdef USE_NATIVE_WINDOW
	mpGraphics->setFrameClipPath( maFrameClipPath );
#else	// USE_NATIVE_WINDOW
#if !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	if ( !mpGraphics->mpVCLGraphics && mpVCLFrame )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
	mpGraphics->mpVCLGraphics->endSetClipRegion( sal_True );
#endif	// !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
#endif	// USE_NATIVE_WINDOW
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetScreenNumber( unsigned int nScreen )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalFrame::SetScreenNumber not implemented\n" );
#endif
}
