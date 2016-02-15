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
 *  Patrick Luby, May 2006
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

#import <dlfcn.h>

#import "quicktimecommon.h"
#include "quicktimewindow.hxx"

#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/media/ZoomLevel.hpp>

typedef id Application_acquireSecurityScopedURLFromNSURL_Type( const id pNonSecurityScopedURL, unsigned char bMustShowDialogIfNoBookmark, const id pDialogTitle );
typedef void Application_releaseSecurityScopedURL_Type( id pSecurityScopedURLs );
typedef BOOL QTGetTimeInterval_Type( QTTime time, NSTimeInterval *timeInterval );
typedef QTTimeRange QTMakeTimeRange_Type( QTTime time, QTTime duration );
typedef QTTime QTMakeTimeWithTimeInterval_Type( NSTimeInterval timeInterval );
typedef NSString * const QTMovieLoopsAttribute_Type;

static Application_acquireSecurityScopedURLFromNSURL_Type *pApplication_acquireSecurityScopedURLFromNSURL = NULL;
static Application_releaseSecurityScopedURL_Type *pApplication_releaseSecurityScopedURL = NULL;
static const short nAVMediaMinDB = -40;
static const short nAVMediaMaxDB = 0;
#if __x86_64__
static BOOL bAVKitInitialized = NO;
static Class aAVAssetClass = nil;
static Class aAVPlayerClass = nil;
static Class aAVPlayerViewClass = nil;
#endif	// __x86_64__
static BOOL bQTKitInitialized = NO;
static Class aQTMovieClass = nil;
static Class aQTMovieViewClass = nil;
static QTGetTimeInterval_Type *pQTGetTimeInterval = NULL;
static QTMakeTimeRange_Type *pQTMakeTimeRange = NULL;
static QTMakeTimeWithTimeInterval_Type *pQTMakeTimeWithTimeInterval = NULL;
static QTMovieLoopsAttribute_Type *pQTMovieLoopsAttribute = NULL;

using namespace ::avmedia::quicktime;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::media;

static void InitializeAVKit()
{
#if __x86_64__
	if ( !bAVKitInitialized )
	{
		NSBundle *pAVKitBundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/AVKit.framework"];
		if ( pAVKitBundle )
		{
			Class aClass = [pAVKitBundle classNamed:@"AVPlayerView"];
			if ( [aClass isSubclassOfClass:[NSView class]] )
			{
				aAVPlayerViewClass = aClass;
				aAVAssetClass = [pAVKitBundle classNamed:@"AVAsset"];
				aAVPlayerClass = [pAVKitBundle classNamed:@"AVPlayer"];
			}
		}

		bAVKitInitialized = YES;
	}
#endif // __x86_64__
}

static void InitializeQTKit()
{
	if ( !bQTKitInitialized )
	{
		NSBundle *pQTKitBundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/QTKit.framework"];
		if ( pQTKitBundle )
		{
			Class aClass = [pQTKitBundle classNamed:@"QTMovieView"];
			if ( [aClass isSubclassOfClass:[NSView class]] )
			{
				aQTMovieViewClass = aClass;
				aQTMovieClass = [pQTKitBundle classNamed:@"QTMovie"];
				if ( aQTMovieClass && aQTMovieViewClass )
				{
					void *pLib = dlopen( NULL, RTLD_LAZY | RTLD_LOCAL );
					if ( pLib )
					{
						pQTGetTimeInterval = (QTGetTimeInterval_Type *)dlsym( pLib, "QTGetTimeInterval" );
						pQTMakeTimeRange = (QTMakeTimeRange_Type *)dlsym( pLib, "QTMakeTimeRange" );
						pQTMakeTimeWithTimeInterval = (QTMakeTimeWithTimeInterval_Type *)dlsym( pLib, "QTMakeTimeWithTimeInterval" );
						pQTMovieLoopsAttribute = (QTMovieLoopsAttribute_Type *)dlsym( pLib, "QTMovieLoopsAttribute" );

						dlclose( pLib );
					}
				}
			}
		}

		bQTKitInitialized = YES;
	}
}

static void HandleAndFireMouseEvent( NSEvent *pEvent, AvmediaMovieView *pView, AvmediaMoviePlayer *pMoviePlayer )
{
	// Only process the event if both the event and the view are visible
	if ( pEvent && pView && pMoviePlayer && [pEvent window] && [pView window] && [[pView window] isVisible] )
	{
		MouseEvent aEvt;

		NSPoint aPoint = [pView convertPoint:[pEvent locationInWindow] fromView:nil];
		aEvt.Modifiers = 0;
		aEvt.Buttons = 0;
		aEvt.X = (sal_Int32)aPoint.x;
		aEvt.Y = (sal_Int32)aPoint.y;
		aEvt.ClickCount = 1;
		aEvt.PopupTrigger = sal_False;

		// Set modifiers. Note that we only care about the Shift and Command
		// modifiers like the Windows code.
		unsigned int nKeyModifiers = [pEvent modifierFlags];
		if ( nKeyModifiers & NSShiftKeyMask )
			aEvt.Modifiers |= KeyModifier::SHIFT;
		if ( nKeyModifiers & NSCommandKeyMask )
			aEvt.Modifiers |= KeyModifier::MOD1;

		// Set buttons
		switch ( [pEvent type] )
		{
			case NSLeftMouseDown:
				if ( nKeyModifiers & NSControlKeyMask )
					aEvt.Buttons = MouseButton::RIGHT;
				else
					aEvt.Buttons = MouseButton::LEFT;
				Window::fireMousePressedEvent( pMoviePlayer, aEvt );
				break;
			case NSRightMouseDown:
				aEvt.Buttons = MouseButton::RIGHT;
				Window::fireMousePressedEvent( pMoviePlayer, aEvt );
				break;
			case NSOtherMouseDown:
				aEvt.Buttons = MouseButton::MIDDLE;
				Window::fireMousePressedEvent( pMoviePlayer, aEvt );
				break;
			case NSLeftMouseDragged:
				if ( nKeyModifiers & NSControlKeyMask )
					aEvt.Buttons = MouseButton::RIGHT;
				else
					aEvt.Buttons = MouseButton::LEFT;
				Window::fireMouseMovedEvent( pMoviePlayer, aEvt );
				break;
			case NSRightMouseDragged:
				aEvt.Buttons = MouseButton::RIGHT;
				Window::fireMouseMovedEvent( pMoviePlayer, aEvt );
				break;
			case NSOtherMouseDragged:
				aEvt.Buttons = MouseButton::MIDDLE;
				Window::fireMouseMovedEvent( pMoviePlayer, aEvt );
				break;
			case NSLeftMouseUp:
				if ( nKeyModifiers & NSControlKeyMask )
					aEvt.Buttons = MouseButton::RIGHT;
				else
					aEvt.Buttons = MouseButton::LEFT;
				Window::fireMouseReleasedEvent( pMoviePlayer, aEvt );
				break;
			case NSRightMouseUp:
				aEvt.Buttons = MouseButton::RIGHT;
				Window::fireMouseReleasedEvent( pMoviePlayer, aEvt );
				break;
			case NSOtherMouseUp:
				aEvt.Buttons = MouseButton::MIDDLE;
				Window::fireMouseReleasedEvent( pMoviePlayer, aEvt );
				break;
			default:
				aEvt.ClickCount = 0;
				Window::fireMouseMovedEvent( pMoviePlayer, aEvt );
				break;
		}
	}
}

@implementation AvmediaArgs

+ (id)argsWithArgs:(NSArray *)pArgs
{
	AvmediaArgs *pRet = [[AvmediaArgs alloc] initWithArgs:pArgs];
	if ( pRet )
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

@interface NSObject (QTMovie)
+ (id)movieWithURL:(NSURL *)pURL error:(NSError **)ppError;
- (QTTime)currentTime;
- (QTTime)duration;
- (NSImage *)frameImageAtTime:(QTTime)aTime;
- (BOOL)muted;
- (float)rate;
- (QTTime)selectionEnd;
- (void)setAttribute:(id)pValue forKey:(NSString *)pAttributeKey;
- (void)setCurrentTime:(QTTime)aTime;
- (void)setMuted:(BOOL)bMute;
- (void)setRate:(float)fRate;
- (void)setSelection:(QTTimeRange)aSelection;
- (void)setVolume:(float)fVolume;
- (void)play;
- (void)stop;
- (float)volume;
@end

@interface NSView (QTMovieView)
- (void)setControllerVisible:(BOOL)bControllerVisible;
- (void)setEditable:(BOOL)bEditable;
- (void)setMovie:(QTMovie *)pMovie;
- (void)setPreservesAspectRatio:(BOOL)bPreservesAspectRatio;
- (void)setShowsResizeIndicator:(BOOL)bShow;
@end

@implementation AvmediaMoviePlayer

- (void)bounds:(AvmediaArgs *)pArgs
{
	if ( pArgs )
		[pArgs setResult:[NSValue valueWithRect:[mpMovieView frame]]];
}

- (double)currentTime:(AvmediaArgs *)pArgs
{
	double fRet = 0;

	if ( mpMovie && [mpMovie respondsToSelector:@selector(currentTime)] )
	{
		NSTimeInterval aInterval;
		if ( pQTGetTimeInterval && pQTGetTimeInterval( [mpMovie currentTime], &aInterval ) )
			fRet = aInterval;
	}

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithDouble:fRet]];

	return fRet;
}

- (void)destroy:(id)pObject
{
	[self stop:self];

	if ( mpMovieView )
	{
		[mpMovieView removeFromSuperview];
		[mpMovieView setMoviePlayer:nil];
		[mpMovieView setMovie:nil];
		[mpMovieView release];
		mpMovieView = nil;
	}

	if ( mpMovie )
	{
		[mpMovie release];
		mpMovie = nil;
	}

	if ( mpSuperview )
	{
		[mpSuperview release];
		mpSuperview = nil;
	}
}

- (double)duration:(AvmediaArgs *)pArgs
{
	double fRet = 0;

	if ( mpMovie && [mpMovie respondsToSelector:@selector(duration)] )
	{
		NSTimeInterval aInterval;
		if ( pQTGetTimeInterval && pQTGetTimeInterval( [mpMovie duration], &aInterval ) )
			fRet = aInterval;
	}

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithDouble:fRet]];

	return fRet;
}

- (void)dealloc
{
	[self destroy:self];

	if ( mpURL )
		[mpURL release];

	if ( mpSecurityScopedURL )
	{
		if ( pApplication_releaseSecurityScopedURL )
			pApplication_releaseSecurityScopedURL( mpSecurityScopedURL );
		mpSecurityScopedURL = nil;
	}

	[super dealloc];
}

- (NSBitmapImageRep *)frameImageAtTime:(AvmediaArgs *)pArgs
{
	NSBitmapImageRep *pRet = nil;

	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return pRet;

	NSNumber *pTime = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pTime )
		return pRet;

	NSImage *pImage = ( mpMovie && [mpMovie respondsToSelector:@selector(frameImageAtTime:)] && pQTMakeTimeWithTimeInterval ? [mpMovie frameImageAtTime:pQTMakeTimeWithTimeInterval( [pTime doubleValue] )] : nil );
	if ( pImage )
	{
		NSSize aSize = [pImage size];
		if ( aSize.width > 0 && aSize.height > 0 )
		{
			NSView *pFocusView = [NSView focusView];
			if ( pFocusView )
				[pFocusView unlockFocus];

			[pImage lockFocus];

			NSBitmapImageRep *pBitmapImageRep = [[NSBitmapImageRep alloc] initWithFocusedViewRect:NSMakeRect( 0, 0, aSize.width, aSize.height )];
			if ( pBitmapImageRep )
			{
				// Add to autorelease pool as invoking alloc disables autorelease
				[pBitmapImageRep autorelease];
				pRet = pBitmapImageRep;
			}

			[pImage unlockFocus];

			if ( pFocusView )
				[pFocusView lockFocus];
		}
	}

	if ( pRet )
		[pArgs setResult:pRet];

	return pRet;
}

- (id)initWithURL:(NSURL *)pURL
{
	[super init];

	mpMovie = nil;
	mpMovieView = nil;
	maPreferredSize = NSMakeSize( 0, 0 );
	maRealFrame = NSMakeRect( 0, 0, 0, 0 );
	mnZoomLevel = ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT;
	mpURL = pURL;
	mpSecurityScopedURL = nil;

	if ( mpURL )
	{
		[mpURL retain];

		// Fix crash when loading URL that needs a security scoped URL by
		// acquiring the URL immediately instead of waiting to do it on the
		// main thread. This works because releasing the application mutex on
		// the main thread does nothing in the vcl/java/source/app/salinst.mm
		// code.
		if ( !pApplication_acquireSecurityScopedURLFromNSURL )
			pApplication_acquireSecurityScopedURLFromNSURL = (Application_acquireSecurityScopedURLFromNSURL_Type *)dlsym( RTLD_DEFAULT, "Application_acquireSecurityScopedURLFromNSURL" );
		if ( !pApplication_releaseSecurityScopedURL )
			pApplication_releaseSecurityScopedURL = (Application_releaseSecurityScopedURL_Type *)dlsym( RTLD_DEFAULT, "Application_releaseSecurityScopedURL" );
		if ( pApplication_acquireSecurityScopedURLFromNSURL && pApplication_releaseSecurityScopedURL )
			mpSecurityScopedURL = pApplication_acquireSecurityScopedURLFromNSURL( mpURL, sal_True, nil );
	}

	return self;
}

- (void)initialize:(id)pObject
{
	InitializeAVKit();
	InitializeQTKit();

	[self destroy:self];

	if ( !mpURL )
		return;

	mpMovie = nil;
	mpMovieView  = nil;
	if ( aQTMovieClass && class_getClassMethod( aQTMovieClass, @selector(movieWithURL:error:) ) && pQTGetTimeInterval && pQTMakeTimeRange && pQTMakeTimeWithTimeInterval && pQTMovieLoopsAttribute )
	{
		NSError *pError = nil;
		mpMovie = [aQTMovieClass movieWithURL:mpURL error:&pError];
		if ( mpMovie && !pError )
		{
			[mpMovie retain];
			if ( [mpMovie respondsToSelector:@selector(setAttribute:forKey:)] )
				[mpMovie setAttribute:[NSNumber numberWithBool:NO] forKey:*pQTMovieLoopsAttribute];
			if ( [mpMovie respondsToSelector:@selector(setSelection:)] && [mpMovie respondsToSelector:@selector(duration)] )
				[mpMovie setSelection:pQTMakeTimeRange( pQTMakeTimeWithTimeInterval( 0 ), [mpMovie duration] )];

			NSImage *pImage = nil;
			if ( [mpMovie respondsToSelector:@selector(frameImageAtTime:)] )
				pImage = [mpMovie frameImageAtTime:pQTMakeTimeWithTimeInterval( 0 )];
			if ( pImage )
				maPreferredSize = [pImage size];
			else
				maPreferredSize = NSMakeSize( 0, 0 );
			maRealFrame = NSMakeRect( 0, 0, maPreferredSize.width, maPreferredSize.height );

			mpMovieView = [[AvmediaMovieView alloc] initWithFrame:maRealFrame];
			[mpMovieView setMoviePlayer:self];
			[mpMovieView setPreservesAspectRatio:( mnZoomLevel == ZoomLevel_FIT_TO_WINDOW ? NO : YES )];
			[mpMovieView setMovie:mpMovie];
		}
	}
}

- (NSObject *)movie
{
	return mpMovie;
}

- (AvmediaMovieView *)movieView
{
	return mpMovieView;
}

- (BOOL)isPlaying:(AvmediaArgs *)pArgs
{
	BOOL bRet = ( mpMovie && [mpMovie respondsToSelector:@selector(rate)] && [mpMovie rate] != 0 );

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithBool:bRet]];

	return bRet;
}

- (double)rate:(AvmediaArgs *)pArgs
{
	double fRet = (double)( mpMovie && [mpMovie respondsToSelector:@selector(rate)] ? [mpMovie rate] : 0 );

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithDouble:fRet]];

	return fRet;
}

- (BOOL)mute:(AvmediaArgs *)pArgs
{
	BOOL bRet = ( mpMovie && [mpMovie respondsToSelector:@selector(muted)] ? [mpMovie muted] : NO );

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithBool:bRet]];

	return bRet;
}

- (void)play:(id)pObject
{
	if ( mpMovie && [mpMovie respondsToSelector:@selector(play)] && [mpMovie respondsToSelector:@selector(rate)] && [mpMovie rate] == 0 )
		[mpMovie play];
}

- (void)preferredSize:(AvmediaArgs *)pArgs
{
	if ( pArgs )
		[pArgs setResult:[NSValue valueWithSize:maPreferredSize]];
}

- (double)selectionEnd:(AvmediaArgs *)pArgs
{
	double fRet = 0;

	if ( mpMovie && pQTGetTimeInterval )
	{
		NSTimeInterval aInterval;
		if ( [mpMovie respondsToSelector:@selector(selectionEnd)] && pQTGetTimeInterval( [mpMovie selectionEnd], &aInterval ) )
		{
			fRet = aInterval;
		}
		else if ( [mpMovie respondsToSelector:@selector(duration)] && pQTGetTimeInterval( [mpMovie duration], &aInterval ) )
		{
			fRet = aInterval;
		}
	}

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithDouble:fRet]];

	return fRet;
}

- (void)setBounds:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
		return;

	NSView *pSuperview = (NSView *)[pArgArray objectAtIndex:0];
	if ( !pSuperview )
		return;

	NSValue *pRect = (NSValue *)[pArgArray objectAtIndex:1];
	if ( !pRect )
		return;

	NSRect aNewFrame = [pRect rectValue];
	if ( aNewFrame.size.width < 1 )
		aNewFrame.size.width = 1;
	if ( aNewFrame.size.height < 1 )
		aNewFrame.size.height = 1;

	if ( NSEqualRects( aNewFrame, maRealFrame ) )
		return;

	// No need to flip coordinates as the JavaSalObject view is already flipped
	// like our view
	[self setFrame:aNewFrame];
}

- (void)setCurrentTime:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pTime = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pTime )
		return;

	if ( mpMovie && [mpMovie respondsToSelector:@selector(setCurrentTime:)] && pQTMakeTimeWithTimeInterval )
		[mpMovie setCurrentTime:pQTMakeTimeWithTimeInterval( [pTime doubleValue] )];
}

- (void)setFocus:(id)pObject
{
	NSWindow *pWindow = [mpMovieView window];
	if ( pWindow )
		[pWindow makeFirstResponder:mpMovieView];
}

- (void)setFrame:(NSRect)aRect
{
	NSRect aOldRealFrame = maRealFrame;
	maRealFrame = aRect;

	// Copy rectangle so that we can edit it
	NSRect aZoomFrame = NSMakeRect( aRect.origin.x, aRect.origin.y, maPreferredSize.width, maPreferredSize.height );

	switch ( mnZoomLevel )
	{
		case ZoomLevel_ZOOM_1_TO_4:
			aZoomFrame.size.width /= 4;
			aZoomFrame.size.height /= 4;
			break;
		case ZoomLevel_ZOOM_1_TO_2:
			aZoomFrame.size.width /= 2;
			aZoomFrame.size.height /= 2;
			break;
		case ZoomLevel_ZOOM_2_TO_1:
			aZoomFrame.size.width *= 2;
			aZoomFrame.size.height *= 2;
			break;
		case ZoomLevel_ZOOM_4_TO_1:
			aZoomFrame.size.width *= 4;
			aZoomFrame.size.height *= 4;
			break;
		case ZoomLevel_FIT_TO_WINDOW:
		case ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT:
		case ZoomLevel_ORIGINAL:
		default:
			// Use the unadjusted preferred size to fit in window
			aZoomFrame = aRect;
			break;
	}

	[mpMovieView setPreservesAspectRatio:( mnZoomLevel == ZoomLevel_FIT_TO_WINDOW ? NO : YES )];

	// Center zoom bounds with real bounds
	aZoomFrame.origin.x += ( maRealFrame.size.width - aZoomFrame.size.width ) / 2;
	aZoomFrame.origin.y += ( maRealFrame.size.height - aZoomFrame.size.height ) / 2;

	if ( NSEqualRects( maRealFrame, aOldRealFrame ) && NSEqualRects( aZoomFrame, [mpMovieView frame] ) )
		return;

	[mpMovieView setFrame:aZoomFrame];
}

- (void)setLooping:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pLooping = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pLooping )
		return;

	if ( mpMovie && [mpMovie respondsToSelector:@selector(setAttribute:forKey:)] && pQTMovieLoopsAttribute )
		[mpMovie setAttribute:pLooping forKey:*pQTMovieLoopsAttribute];
}

- (void)setMute:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pMute = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pMute )
		return;

	if ( mpMovie && [mpMovie respondsToSelector:@selector(setMuted:)] )
		[mpMovie setMuted:[pMute boolValue]];
}

- (void)setPointer:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pPointer = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pPointer )
		return;

	// Make sure cursor is visible
	[NSCursor unhide];

	NSCursor *pCursor;

	switch ( [pPointer intValue] )
	{
		case SystemPointer::CROSS:
		case SystemPointer::MOVE:
			pCursor = [NSCursor crosshairCursor];
			break;
		case SystemPointer::INVISIBLE:
			pCursor = [NSCursor arrowCursor];
			[NSCursor hide];
			break;
		case SystemPointer::HAND:
			pCursor = [NSCursor openHandCursor];
			break;
		default:
			pCursor = [NSCursor arrowCursor];
			break;
	}

	[mpMovieView setCursor:pCursor];
}

- (void)setRate:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pTime = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pTime )
		return;

	if ( mpMovie && [mpMovie respondsToSelector:@selector(setRate:)] )
		[mpMovie setRate:[pTime floatValue]];
}

- (void)setSelection:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pTime = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pTime )
		return;

	if ( mpMovie && [mpMovie respondsToSelector:@selector(setSelection:)] && pQTMakeTimeRange && pQTMakeTimeWithTimeInterval )
		[mpMovie setSelection:pQTMakeTimeRange( pQTMakeTimeWithTimeInterval( 0 ), pQTMakeTimeWithTimeInterval( [pTime doubleValue] ) )];
}

- (void)setSuperview:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
	{
		[mpMovieView removeFromSuperview];
		if ( mpSuperview )
		{
			[mpSuperview release];
			mpSuperview = nil;
		}

		return;
	}

	NSView *pSuperview = (NSView *)[pArgArray objectAtIndex:0];
	if ( !pSuperview )
	{
		[mpMovieView removeFromSuperview];
		if ( mpSuperview )
		{
			[mpSuperview release];
			mpSuperview = nil;
		}

		return;
	}

	NSValue *pRect = (NSValue *)[pArgArray objectAtIndex:1];
	if ( !pRect )
		return;

	NSRect aNewFrame = [pRect rectValue];
	if ( aNewFrame.size.width < 1 )
		aNewFrame.size.width = 1;
	if ( aNewFrame.size.height < 1 )
		aNewFrame.size.height = 1;

	if ( NSEqualRects( aNewFrame, maRealFrame ) && pSuperview == mpSuperview )
		return;

	// No need to flip coordinates as the JavaSalObject view is already flipped
	// like our view
	[self setFrame:aNewFrame];

	[mpMovieView removeFromSuperview];
	if ( mpSuperview )
	{
		[mpSuperview release];
		mpSuperview = nil;
	}

	mpSuperview = pSuperview;
	[mpSuperview retain];

	[mpSuperview addSubview:mpMovieView positioned:NSWindowAbove relativeTo:nil];
}

- (void)setVolumeDB:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pDB = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pDB )
		return;

	if ( mpMovie && [mpMovie respondsToSelector:@selector(setVolume:)] )
		[mpMovie setVolume:( (float)( [pDB shortValue] - nAVMediaMinDB ) / (float)( nAVMediaMaxDB - nAVMediaMinDB ) )];
}

- (void)setZoomLevel:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pZoomLevel = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pZoomLevel )
		return;

	int nOldZoomLevel = mnZoomLevel;

	switch ( [pZoomLevel intValue] )
	{
		case ZoomLevel_ORIGINAL:
		case ZoomLevel_FIT_TO_WINDOW:
		case ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT:
		case ZoomLevel_ZOOM_1_TO_4:
		case ZoomLevel_ZOOM_1_TO_2:
		case ZoomLevel_ZOOM_2_TO_1:
		case ZoomLevel_ZOOM_4_TO_1:
			mnZoomLevel = [pZoomLevel intValue];
			break;
		default:
			mnZoomLevel = ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT;
			break;
	}

	if ( mnZoomLevel == nOldZoomLevel )
		return;

	// No need to flip coordinates as the JavaSalObject view is already flipped
	// like our view
	[self setFrame:maRealFrame];
}

- (void)stop:(id)pObject
{
	if ( mpMovie && [mpMovie respondsToSelector:@selector(rate)] && [mpMovie respondsToSelector:@selector(stop)] && [mpMovie rate] != 0 )
		[mpMovie stop];
}

- (short)volumeDB:(AvmediaArgs *)pArgs
{
	short nRet = (short)( ( mpMovie && [mpMovie respondsToSelector:@selector(volume)] ? [mpMovie volume] : 0 ) * ( nAVMediaMaxDB - nAVMediaMinDB ) ) + nAVMediaMinDB;

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithShort:nRet]];

	return nRet;
}

@end

@implementation AvmediaMovieView

+ (NSMenu *)defaultMenu
{
	return nil;
}

- (BOOL)becomeFirstResponder
{
	BOOL bRet = [super becomeFirstResponder];

	// Only process the event if both the event and the view are visible
	if ( bRet && mpMoviePlayer && [self window] && [[self window] isVisible] )
	{
		FocusEvent aEvt;
		Window::fireFocusGainedEvent( mpMoviePlayer, aEvt );
	}

	return bRet;
}

- (void)dealloc
{
	[self setCursor:nil];
	[self setMoviePlayer:nil];

	if ( mpQTMovieView )
	{
		[mpQTMovieView removeFromSuperview];
		[mpQTMovieView release];
	}

	[super dealloc];
}

- (NSView *)hitTest:(NSPoint)aPoint
{
	// Don't allow subview to get any mouse events
	if ( [super hitTest:aPoint] )
		return self;
	else
		return nil;
}

- (id)initWithFrame:(NSRect)aFrame
{
	[super initWithFrame:aFrame];

	InitializeAVKit();
	InitializeQTKit();

	mpCursor = nil;
	mpMoviePlayer = nil;
	mpQTMovieView = nil;
	if ( aQTMovieViewClass )
	{
		mpQTMovieView = [aQTMovieViewClass alloc];
		if ( mpQTMovieView )
		{
			if ( [mpQTMovieView isKindOfClass:[NSView class]] )
			{
				[(NSView *)mpQTMovieView initWithFrame:NSMakeRect( 0, 0, aFrame.size.width, aFrame.size.height )];
				if ( [mpQTMovieView respondsToSelector:@selector(setControllerVisible:)] )
					[mpQTMovieView setControllerVisible:NO];
				if ( [mpQTMovieView respondsToSelector:@selector(setShowsResizeIndicator:)] )
					[mpQTMovieView setShowsResizeIndicator:NO];
				if ( [mpQTMovieView respondsToSelector:@selector(setEditable:)] )
					[mpQTMovieView setEditable:NO];
				[self addSubview:mpQTMovieView];
			}
			else
			{
				[mpQTMovieView release];
				mpQTMovieView = nil;
			}
		}
	}

	return self;
}

- (BOOL)isFlipped
{
	return YES;
}

- (NSMenu *)menuForEvent:(NSEvent *)pEvent
{
	// Suppress display of native popup menu
	return nil;
}

- (void)mouseDown:(NSEvent *)pEvent
{
	HandleAndFireMouseEvent( pEvent, self, mpMoviePlayer );
}

- (void)mouseDragged:(NSEvent *)pEvent
{
	HandleAndFireMouseEvent( pEvent, self, mpMoviePlayer );
}

- (void)mouseEntered:(NSEvent *)pEvent
{
	HandleAndFireMouseEvent( pEvent, self, mpMoviePlayer );
}

- (void)mouseExited:(NSEvent *)pEvent
{
	HandleAndFireMouseEvent( pEvent, self, mpMoviePlayer );
}

- (void)mouseMoved:(NSEvent *)pEvent
{
	HandleAndFireMouseEvent( pEvent, self, mpMoviePlayer );
}

- (void)mouseUp:(NSEvent *)pEvent
{
	HandleAndFireMouseEvent( pEvent, self, mpMoviePlayer );
}

- (void)rightMouseDown:(NSEvent *)pEvent
{
	HandleAndFireMouseEvent( pEvent, self, mpMoviePlayer );
}

- (void)rightMouseDragged:(NSEvent *)pEvent
{
	HandleAndFireMouseEvent( pEvent, self, mpMoviePlayer );
}

- (void)rightMouseUp:(NSEvent *)pEvent
{
	HandleAndFireMouseEvent( pEvent, self, mpMoviePlayer );
}

- (void)otherMouseDown:(NSEvent *)pEvent
{
	HandleAndFireMouseEvent( pEvent, self, mpMoviePlayer );
}

- (void)otherMouseDragged:(NSEvent *)pEvent
{
	HandleAndFireMouseEvent( pEvent, self, mpMoviePlayer );
}

- (void)otherMouseUp:(NSEvent *)pEvent
{
	HandleAndFireMouseEvent( pEvent, self, mpMoviePlayer );
}

- (void)resetCursorRects
{
	[super resetCursorRects];

	if ( mpCursor )
	{
		[self addCursorRect:[self visibleRect] cursor:mpCursor];
		[mpCursor setOnMouseEntered:YES];
	}
}

- (void)setCursor:(NSCursor *)pCursor
{
	if ( mpCursor )
		[mpCursor release];

	mpCursor = pCursor;

	if ( mpCursor )
	{
		[mpCursor retain];

		NSWindow *pWindow = [self window];
		if ( pWindow )
			[pWindow invalidateCursorRectsForView:self];
	}
}

- (void)setFrame:(NSRect)aRect
{
	[super setFrame:aRect];

	if ( mpQTMovieView )
		[mpQTMovieView setFrame:NSMakeRect( 0, 0, aRect.size.width, aRect.size.height )];
}

- (void)setMovie:(NSObject *)pMovie
{
	if ( !pMovie || [pMovie isKindOfClass:aQTMovieClass] )
	{
		if ( mpQTMovieView && [mpQTMovieView respondsToSelector:@selector(setMovie:)] )
			[mpQTMovieView setMovie:(QTMovie *)pMovie];
	}
}

- (void)setMoviePlayer:(AvmediaMoviePlayer *)pPlayer
{
	if ( mpMoviePlayer )
		[mpMoviePlayer release];

	mpMoviePlayer = pPlayer;

	if ( mpMoviePlayer )
		[mpMoviePlayer retain];
}

- (void)setPreservesAspectRatio:(BOOL)bPreservesAspectRatio
{
	if ( mpQTMovieView && [mpQTMovieView respondsToSelector:@selector(setPreservesAspectRatio:)] )
		[mpQTMovieView setPreservesAspectRatio:bPreservesAspectRatio];
}

@end
