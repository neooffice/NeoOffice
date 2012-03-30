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
#include <salobj.h>
#include <saldata.hxx>
#include <salmenu.h>
#include <salsys.h>
#include <vcl/dialog.hxx>
#include <vcl/settings.hxx>
#include <vcl/status.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/vcl/VCLEvent.hxx>
#include <com/sun/star/vcl/VCLFrame.hxx>
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
#include <postmac.h>

#ifdef USE_NATIVE_WINDOW
#include "../java/VCLEventQueue_cocoa.h"
#endif	// USE_NATIVE_WINDOW

typedef UInt32 GetDblTime_Type();
typedef OSStatus SetSystemUIMode_Type( SystemUIMode nMode, SystemUIOptions nOptions );

#ifdef USE_NATIVE_WINDOW
static ::std::map< NSWindow*, JavaSalGraphics* > aNativeWindowMap;
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
		// Calculate the total combined scren so that we can flip coordinates
		NSRect aTotalBounds = NSMakeRect( 0, 0, 0, 0 );
		unsigned int nCount = [pScreens count];
		unsigned int i;
		for ( i = 0 ; i < nCount; i++ )
		{
			NSScreen *pScreen = [pScreens objectAtIndex:i];
			if ( pScreen )
				aTotalBounds = NSUnionRect( [pScreen frame], aTotalBounds );
		}

		for ( i = 0 ; i < nCount; i++ )
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
	for ( ::std::map< NSWindow*, JavaSalGraphics* >::iterator it = aNativeWindowMap.begin(); it != aNativeWindowMap.end(); ++it )
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
					it->second->copyToContext( NULL, false, false, aContext, aBounds, aDestRect.origin, aDestRect );
			}
		}
	}
}

#endif	// USE_NATIVE_WINDOW

// =======================================================================

JavaSalFrame::JavaSalFrame() :
#ifdef USE_NATIVE_WINDOW
	mnHiddenBit( 0 ),
    maHiddenContext( NULL ),
    maHiddenLayer( NULL ),
	maFrameLayer( NULL ),
#endif  // USE_NATIVE_WINDOW
	mpVCLFrame( NULL ),
	mpGraphics( new JavaSalGraphics() ),
	mnStyle( 0 ),
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
}

// -----------------------------------------------------------------------

JavaSalFrame::~JavaSalFrame()
{
#ifdef USE_NATIVE_WINDOW
	// Make sure that no native drawing is possible
	maSysData.pView = NULL;
	UpdateLayer();
#endif	// USE_NATIVE_WINDOW

	Show( FALSE );
	StartPresentation( FALSE );
	CaptureMouse( FALSE );

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
	if ( maFrameLayer )
		CGLayerRelease( maFrameLayer );

	if ( maHiddenLayer )
		CGLayerRelease( maHiddenLayer );

	if ( maHiddenContext )
		CGContextRelease( maHiddenContext );
#endif  // USE_NATIVE_WINDOW

	if ( mpVCLFrame )
	{
		mpVCLFrame->dispose();
		delete mpVCLFrame;
	}

	// Delete graphics last as it may be needed by a JavaSalBitmap
	if ( mpGraphics )
		delete mpGraphics;
}

#ifdef USE_NATIVE_WINDOW

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
		VCLUpdateScreens *pVCLUpdateScreens = [VCLUpdateScreens create];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		aGuard.clear();
		[pVCLUpdateScreens performSelectorOnMainThread:@selector(updateScreens:) withObject:pVCLUpdateScreens waitUntilDone:YES modes:pModes];
		aGuard.reset();
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
		VCLUpdateScreens *pVCLUpdateScreens = [VCLUpdateScreens create];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		aGuard.clear();
		[pVCLUpdateScreens performSelectorOnMainThread:@selector(updateScreens:) withObject:pVCLUpdateScreens waitUntilDone:YES modes:pModes];
		aGuard.reset();
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
		VCLUpdateScreens *pVCLUpdateScreens = [VCLUpdateScreens create];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		aGuard.clear();
		[pVCLUpdateScreens performSelectorOnMainThread:@selector(updateScreens:) withObject:pVCLUpdateScreens waitUntilDone:YES modes:pModes];
		aGuard.reset();
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
		VCLUpdateScreens *pVCLUpdateScreens = [VCLUpdateScreens create];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		aGuard.clear();
		[pVCLUpdateScreens performSelectorOnMainThread:@selector(updateScreens:) withObject:pVCLUpdateScreens waitUntilDone:YES modes:pModes];
		aGuard.reset();
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

#ifdef USE_NATIVE_WINDOW

// -----------------------------------------------------------------------

void JavaSalFrame::UpdateLayer()
{
	CGSize aLayerSize = CGSizeMake( maGeometry.nWidth + maGeometry.nLeftDecoration + maGeometry.nRightDecoration, maGeometry.nHeight + maGeometry.nTopDecoration + maGeometry.nBottomDecoration );
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
		com_sun_star_vcl_VCLEvent aEvent( SALEVENT_PAINT, this, new SalPaintEvent( 0, 0, aLayerSize.width, aLayerSize.height ) );
		GetSalData()->mpEventQueue->postCachedEvent( &aEvent );
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
	if ( !mpGraphics->mpVCLGraphics )
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
	com_sun_star_vcl_VCLEvent aEvent( SALEVENT_USEREVENT, this, pData );
	GetSalData()->mpEventQueue->postCachedEvent( &aEvent );
	return TRUE;
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetTitle( const XubString& rTitle )
{
	maTitle = OUString( rTitle );
	mpVCLFrame->setTitle( maTitle );
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
	if ( !mpGraphics->mpVCLGraphics )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
#endif	// !USE_NATIVE_WINDOW

	mpVCLFrame->setVisible( mbVisible, bNoActivate );

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
						mpVCLFrame->makeModal();
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

		maSysData.pView = (NSView *)mpVCLFrame->getNativeWindowContentView( bTopLevelWindow );
		mbCenter = FALSE;

		com_sun_star_vcl_VCLEvent aEvent( SALEVENT_MOVERESIZE, this, NULL );
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
		com_sun_star_vcl_VCLEvent aEvent( SALEVENT_ENDEXTTEXTINPUT, this, NULL );
		aEvent.dispatch();

		// Remove the native window since it is destroyed when hidden
		maSysData.pView = NULL;

		// Unset focus but don't set focus to another frame as it will cause
		// menu shortcuts to be disabled if we go into show only menus mode
		// after closing a window whose child window had focus
		if ( pSalData->mpFocusFrame == this )
		{
			com_sun_star_vcl_VCLEvent aFocusEvent( SALEVENT_LOSEFOCUS, this, NULL );
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
	mpVCLFrame->setMinClientSize( nWidth, nHeight );
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
		Rectangle aParentBounds( mpParent->mpVCLFrame->getBounds() );
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
		if ( mpParent && mpParent->maGeometry.nWidth >= nWidth && mpParent->maGeometry.nHeight > nHeight)
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
			Rectangle aParentBounds( mpParent->mpVCLFrame->getBounds() );
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

	mpVCLFrame->setBounds( nX, nY, nWidth, nHeight );

	// Update the cached position immediately
	com_sun_star_vcl_VCLEvent aEvent( SALEVENT_MOVERESIZE, this, NULL );
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
	{
		if ( pState->mnState & SAL_FRAMESTATE_MINIMIZED )
			mpVCLFrame->setState( SAL_FRAMESTATE_MINIMIZED );
		else
			mpVCLFrame->setState( SAL_FRAMESTATE_NORMAL );
	}
}

// -----------------------------------------------------------------------

BOOL JavaSalFrame::GetWindowState( SalFrameState* pState )
{
	Rectangle aBounds( mpVCLFrame->getBounds( NULL, sal_True ) );
	pState->mnMask = SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y | SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT | SAL_FRAMESTATE_MASK_STATE;
	pState->mnX = aBounds.Left();
	pState->mnY = aBounds.Top();
	pState->mnWidth = aBounds.GetWidth() - maGeometry.nLeftDecoration - maGeometry.nRightDecoration;
	pState->mnHeight = aBounds.GetHeight() - maGeometry.nTopDecoration - maGeometry.nBottomDecoration;
	pState->mnState = mpVCLFrame->getState();

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

		NSWindow *pNSWindow = (NSWindow *)mpVCLFrame->getNativeWindow();
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

	mpVCLFrame->setFullScreenMode( bFullScreen );
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

	bool bSuccess;
	if ( nFlags & SAL_FRAME_TOTOP_GRABFOCUS )
	{
		bSuccess = pFrame->mpVCLFrame->toFront();
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
			bSuccess = pFrame->mpVCLFrame->toFront();
		else
			bSuccess = pFrame->mpVCLFrame->requestFocus();
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
		com_sun_star_vcl_VCLEvent aEvent( SALEVENT_GETFOCUS, pFrame, NULL );
		aEvent.dispatch();
	}
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetPointer( PointerStyle ePointerStyle )
{
	mpVCLFrame->setPointer( ePointerStyle );
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
	mpVCLFrame->sync();
}

// -----------------------------------------------------------------------

void JavaSalFrame::Sync()
{
	mpVCLFrame->sync();
}

// -----------------------------------------------------------------------

void JavaSalFrame::SetInputContext( SalInputContext* pContext )
{
	// Only allow Mac OS X key bindings when the OOo application code says so
	if ( pContext && pContext->mnOptions & SAL_INPUTCONTEXT_TEXT )
		mpVCLFrame->setAllowKeyBindings( sal_True );
	else
		mpVCLFrame->setAllowKeyBindings( sal_False );
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
	OUString aKeyName( mpVCLFrame->getKeyName( nKeyCode ) );
	return XubString( aKeyName );
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
		mpParent->mpVCLFrame->removeChild( this );
	}

	mpParent = (JavaSalFrame *)pNewParent;

	if ( !bUtilityWindow )
	{
		::std::list< JavaSalObject* > aReshowObjects( maVisibleObjects );
		bool bReshow = mbVisible;
		if ( bReshow )
			Show( FALSE );

		// Fix bug 1310 by creating a new native window with the new parent
		maSysData.pView = NULL;
		com_sun_star_vcl_VCLFrame *pOldVCLFrame = mpVCLFrame;
#ifndef USE_NATIVE_WINDOW
		com_sun_star_vcl_VCLGraphics *pOldVCLGraphics = mpGraphics->mpVCLGraphics;
#endif	// USE_NATIVE_WINDOW

		mpVCLFrame = new com_sun_star_vcl_VCLFrame( mnStyle, this, mpParent, mbShowOnlyMenus, bUtilityWindow );
		if ( mpVCLFrame )
		{
#ifndef USE_NATIVE_WINDOW
			mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
#endif	// USE_NATIVE_WINDOW
			mpVCLFrame->setTitle( maTitle );

#ifndef USE_NATIVE_WINDOW
			mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
			if ( pOldVCLGraphics )
				delete pOldVCLGraphics;
#endif	// USE_NATIVE_WINDOW

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
		mpParent->mpVCLFrame->addChild( this );
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
	fprintf( stderr, "JavaSalFrame::ResetClipRegion not implemented\n" );
#else	// USE_NATIVE_WINDOW
#if !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	if ( !mpGraphics->mpVCLGraphics )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
	mpGraphics->mpVCLGraphics->resetClipRegion( sal_True );
#endif	// !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
#endif	// USE_NATIVE_WINDOW
}

// -----------------------------------------------------------------------

void JavaSalFrame::BeginSetClipRegion( ULONG nRects )
{
#ifdef USE_NATIVE_WINDOW
	fprintf( stderr, "JavaSalFrame::BeginSetClipRegion not implemented\n" );
#else	// USE_NATIVE_WINDOW
#if !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	if ( !mpGraphics->mpVCLGraphics )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
	mpGraphics->mpVCLGraphics->beginSetClipRegion( sal_True );
#endif	// !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
#endif	// USE_NATIVE_WINDOW
}

// -----------------------------------------------------------------------

void JavaSalFrame::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
#ifdef USE_NATIVE_WINDOW
	fprintf( stderr, "JavaSalFrame::UnionClipRegion not implemented\n" );
#else	// USE_NATIVE_WINDOW
#if !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	if ( !mpGraphics->mpVCLGraphics )
		mpGraphics->mpVCLGraphics = mpVCLFrame->getGraphics();
	mpGraphics->mpVCLGraphics->unionClipRegion( nX, nY, nWidth, nHeight, sal_True );
#endif	// !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
#endif	// USE_NATIVE_WINDOW
}

// -----------------------------------------------------------------------

void JavaSalFrame::EndSetClipRegion()
{
#ifdef USE_NATIVE_WINDOW
	fprintf( stderr, "JavaSalFrame::EndSetClipRegion not implemented\n" );
#else	// USE_NATIVE_WINDOW
#if !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	if ( !mpGraphics->mpVCLGraphics )
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
