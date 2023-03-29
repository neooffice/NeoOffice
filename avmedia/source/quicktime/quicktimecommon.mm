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
 */

#import <dlfcn.h>

#import "quicktimecommon.h"
#include "quicktimewindow.hxx"

#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/media/ZoomLevel.hpp>
#include <vcl/svapp.hxx>

#ifndef USE_QUICKTIME
#define PREFERRED_TIMESCALE 1000
#endif	// !USE_QUICKTIME

typedef id Application_acquireSecurityScopedURLFromNSURL_Type( const id pNonSecurityScopedURL, unsigned char bMustShowDialogIfNoBookmark, const id pDialogTitle );
typedef void Application_releaseSecurityScopedURL_Type( id pSecurityScopedURLs );
#ifdef USE_QUICKTIME
typedef BOOL QTGetTimeInterval_Type( QTTime time, NSTimeInterval *timeInterval );
typedef QTTimeRange QTMakeTimeRange_Type( QTTime time, QTTime duration );
typedef QTTime QTMakeTimeWithTimeInterval_Type( NSTimeInterval timeInterval );
typedef NSString * const QTMovieLoopsAttribute_Type;
#endif	// USE_QUICKTIME

static Application_acquireSecurityScopedURLFromNSURL_Type *pApplication_acquireSecurityScopedURLFromNSURL = NULL;
static Application_releaseSecurityScopedURL_Type *pApplication_releaseSecurityScopedURL = NULL;
#ifdef USE_QUICKTIME
static const short nAVMediaMinDB = -40;
static const short nAVMediaMaxDB = 0;
static BOOL bQTKitInitialized = NO;
static Class aQTMovieClass = nil;
static Class aQTMovieViewClass = nil;
static QTGetTimeInterval_Type *pQTGetTimeInterval = NULL;
static QTMakeTimeRange_Type *pQTMakeTimeRange = NULL;
static QTMakeTimeWithTimeInterval_Type *pQTMakeTimeWithTimeInterval = NULL;
static QTMovieLoopsAttribute_Type *pQTMovieLoopsAttribute = NULL;
#endif	// USE_QUICKTIME

using namespace ::avmedia::quicktime;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::media;

#ifdef USE_QUICKTIME

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

#endif	// USE_QUICKTIME

static void HandleAndFireMouseEvent( NSEvent *pEvent, AvmediaMovieView *pView, AvmediaMoviePlayer *pMoviePlayer )
{
	// Only process the event if both the event and the view are visible
	if ( pEvent && pView && pMoviePlayer && [pEvent window] && [pView window] && [[pView window] isVisible] )
	{
		com::sun::star::awt::MouseEvent aEvt;

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
		if ( nKeyModifiers & NSEventModifierFlagShift )
			aEvt.Modifiers |= KeyModifier::SHIFT;
		if ( nKeyModifiers & NSEventModifierFlagCommand )
			aEvt.Modifiers |= KeyModifier::MOD1;

		NSUInteger nType = [pEvent type];

		if ( !Application::IsShutDown() )
		{
				ACQUIRE_SOLARMUTEX

				// Set buttons
				switch ( nType )
				{
					case NSEventTypeLeftMouseDown:
						if ( nKeyModifiers & NSEventModifierFlagControl )
							aEvt.Buttons = MouseButton::RIGHT;
						else
							aEvt.Buttons = MouseButton::LEFT;
						Application::PostUserEvent( STATIC_LINK( NULL, Window, fireMousePressedEvent ), new MouseEventData( pMoviePlayer, aEvt ) );
						break;
					case NSEventTypeRightMouseDown:
						aEvt.Buttons = MouseButton::RIGHT;
						Application::PostUserEvent( STATIC_LINK( NULL, Window, fireMousePressedEvent ), new MouseEventData( pMoviePlayer, aEvt ) );
						break;
					case NSEventTypeOtherMouseDown:
						aEvt.Buttons = MouseButton::MIDDLE;
						Application::PostUserEvent( STATIC_LINK( NULL, Window, fireMousePressedEvent ), new MouseEventData( pMoviePlayer, aEvt ) );
						break;
					case NSEventTypeLeftMouseDragged:
						if ( nKeyModifiers & NSEventModifierFlagControl )
							aEvt.Buttons = MouseButton::RIGHT;
						else
							aEvt.Buttons = MouseButton::LEFT;
						Application::PostUserEvent( STATIC_LINK( NULL, Window, fireMouseMovedEvent ), new MouseEventData( pMoviePlayer, aEvt ) );
						break;
					case NSEventTypeRightMouseDragged:
						aEvt.Buttons = MouseButton::RIGHT;
						Application::PostUserEvent( STATIC_LINK( NULL, Window, fireMouseMovedEvent ), new MouseEventData( pMoviePlayer, aEvt ) );
						break;
					case NSEventTypeOtherMouseDragged:
						aEvt.Buttons = MouseButton::MIDDLE;
						Application::PostUserEvent( STATIC_LINK( NULL, Window, fireMouseMovedEvent ), new MouseEventData( pMoviePlayer, aEvt ) );
						break;
					case NSEventTypeLeftMouseUp:
						if ( nKeyModifiers & NSEventModifierFlagControl )
							aEvt.Buttons = MouseButton::RIGHT;
						else
							aEvt.Buttons = MouseButton::LEFT;
						Application::PostUserEvent( STATIC_LINK( NULL, Window, fireMouseReleasedEvent ), new MouseEventData( pMoviePlayer, aEvt ) );
						break;
					case NSEventTypeRightMouseUp:
						aEvt.Buttons = MouseButton::RIGHT;
						Application::PostUserEvent( STATIC_LINK( NULL, Window, fireMouseReleasedEvent ), new MouseEventData( pMoviePlayer, aEvt ) );
						break;
					case NSEventTypeOtherMouseUp:
						aEvt.Buttons = MouseButton::MIDDLE;
						Application::PostUserEvent( STATIC_LINK( NULL, Window, fireMouseReleasedEvent ), new MouseEventData( pMoviePlayer, aEvt ) );
						break;
					default:
						aEvt.ClickCount = 0;
						Application::PostUserEvent( STATIC_LINK( NULL, Window, fireMouseMovedEvent ), new MouseEventData( pMoviePlayer, aEvt ) );
						break;
				}
				RELEASE_SOLARMUTEX
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

#ifdef USE_QUICKTIME

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

#endif	// USE_QUICKTIME

@interface NSCursor (AvmediaCursor)
- (void)setOnMouseEntered:(BOOL)bFlag;
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

#ifdef USE_QUICKTIME
	if ( mpMovie && [mpMovie respondsToSelector:@selector(currentTime)] )
	{
		NSTimeInterval aInterval;
		if ( pQTGetTimeInterval && pQTGetTimeInterval( [mpMovie currentTime], &aInterval ) )
			fRet = aInterval;
	}
#else	// USE_QUICKTIME
	if ( mpAVPlayer )
	{
		AVPlayerItem *pAVPlayerItem = mpAVPlayer.currentItem;
		if ( pAVPlayerItem )
		{
			CMTime aTime = pAVPlayerItem.currentTime;
			if ( CMTIME_IS_NUMERIC( aTime ) )
			{
				fRet = CMTimeGetSeconds( aTime );

				if ( fRet > mfStopTime )
					[mpAVPlayer pause];
			}
		}
	}
#endif	// USE_QUICKTIME

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithDouble:fRet]];

	return fRet;
}

- (void)destroy:(id)pObject
{
	(void)pObject;

	[self stop:self];

	if ( mpMovieView )
	{
		[mpMovieView removeFromSuperview];
		[mpMovieView setMoviePlayer:nil];
		[mpMovieView setMovie:nil];
		[mpMovieView release];
		mpMovieView = nil;
	}

#ifdef USE_QUICKTIME
	if ( mpMovie )
	{
		[mpMovie release];
		mpMovie = nil;
	}
#else	// USE_QUICKTIME
	if ( mpAVPlayer )
	{
		AVPlayerItem *pAVPlayerItem = mpAVPlayer.currentItem;
		if ( pAVPlayerItem )
		{
			NSNotificationCenter *pNotificationCenter = [NSNotificationCenter defaultCenter];
			if ( pNotificationCenter )
				[pNotificationCenter removeObserver:self name:AVPlayerItemDidPlayToEndTimeNotification object:pAVPlayerItem];
		}

		[mpAVPlayer release];
		mpAVPlayer = nil;
	}

	mbLooping = NO;
	mfStopTime = DBL_MAX;
#endif	// USE_QUICKTIME

	if ( mpSuperview )
	{
		[mpSuperview release];
		mpSuperview = nil;
	}

	maPreferredSize = NSMakeSize( 0, 0 );
	maRealFrame = NSMakeRect( 0, 0, 0, 0 );
	mnZoomLevel = ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT;
}

- (double)duration:(AvmediaArgs *)pArgs
{
	double fRet = 0;

#ifdef USE_QUICKTIME
	if ( mpMovie && [mpMovie respondsToSelector:@selector(duration)] )
	{
		NSTimeInterval aInterval;
		if ( pQTGetTimeInterval && pQTGetTimeInterval( [mpMovie duration], &aInterval ) )
			fRet = aInterval;
	}
#else	// USE_QUICKTIME
	if ( mpAVPlayer )
	{
		AVPlayerItem *pAVPlayerItem = mpAVPlayer.currentItem;
		if ( pAVPlayerItem )
		{
			CMTime aDuration = pAVPlayerItem.duration;
			if ( CMTIME_IS_NUMERIC( aDuration ) )
			{
				fRet = CMTimeGetSeconds( aDuration );
			}
			else
			{
				AVAsset *pAVAsset = pAVPlayerItem.asset;
				if ( pAVAsset )
				{
					aDuration = pAVAsset.duration;
					if ( CMTIME_IS_NUMERIC( aDuration ) )
						fRet = CMTimeGetSeconds( aDuration );
				}
			}
		}
	}
#endif	// USE_QUICKTIME

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

#ifdef USE_QUICKTIME
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
#else	// USE_QUICKTIME
	if ( mpAVPlayer )
	{
		AVPlayerItem *pAVPlayerItem = mpAVPlayer.currentItem;
		if ( pAVPlayerItem )
		{
			AVAsset *pAVAsset = pAVPlayerItem.asset;
			if ( pAVAsset )
			{
				AVAssetImageGenerator *pAVAssetImageGenerator = [AVAssetImageGenerator assetImageGeneratorWithAsset:pAVAsset];
				if ( pAVAssetImageGenerator )
				{
					pAVAssetImageGenerator.requestedTimeToleranceBefore = kCMTimeZero;
					pAVAssetImageGenerator.requestedTimeToleranceAfter = kCMTimeZero;
					CGImageRef aImage = [pAVAssetImageGenerator copyCGImageAtTime:CMTimeMakeWithSeconds( [pTime doubleValue], PREFERRED_TIMESCALE ) actualTime:NULL error:NULL];
					if ( aImage )
					{
						pRet = [[NSBitmapImageRep alloc] initWithCGImage:aImage];
						CGImageRelease( aImage );
					}
				}
			}
		}
	}
#endif	// USE_QUICKTIME

	if ( pRet )
		[pArgs setResult:pRet];

	return pRet;
}

- (id)initWithURL:(NSURL *)pURL
{
	[super init];

#ifdef USE_QUICKTIME
	mpMovie = nil;
#else	// USE_QUICKTIME
	mpAVPlayer = nil;
	mbLooping = NO;
	mfStopTime = DBL_MAX;
#endif	// USE_QUICKTIME
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
	(void)pObject;

#ifdef USE_QUICKTIME
	InitializeQTKit();
#endif	// USE_QUICKTIME

	[self destroy:self];

	if ( !mpURL )
		return;

#ifdef USE_QUICKTIME
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
#else	// USE_QUICKTIME
	mpAVPlayer = [AVPlayer playerWithURL:mpURL];
	if ( mpAVPlayer )
	{
		[mpAVPlayer retain];

		AVPlayerItem *pAVPlayerItem = mpAVPlayer.currentItem;
		if ( pAVPlayerItem )
		{
			AVAsset *pAVAsset = pAVPlayerItem.asset;
			if ( pAVAsset )
			{
				AVAssetImageGenerator *pAVAssetImageGenerator = [AVAssetImageGenerator assetImageGeneratorWithAsset:pAVAsset];
				if ( pAVAssetImageGenerator )
				{
					CGImageRef aImage = [pAVAssetImageGenerator copyCGImageAtTime:kCMTimeZero actualTime:NULL error:NULL];
					if ( aImage )
					{
						maPreferredSize = NSMakeSize( CGImageGetWidth( aImage ), CGImageGetHeight( aImage ) );
						maRealFrame = NSMakeRect( 0, 0, maPreferredSize.width, maPreferredSize.height );
						CGImageRelease( aImage );
					}
				}
			}

			NSNotificationCenter *pNotificationCenter = [NSNotificationCenter defaultCenter];
			if ( pNotificationCenter )
				[pNotificationCenter addObserver:self selector:@selector(playerItemDidPlayToEndTime:) name:AVPlayerItemDidPlayToEndTimeNotification object:pAVPlayerItem];
		}

		mpMovieView = [[AvmediaMovieView alloc] initWithFrame:maRealFrame];
		[mpMovieView setMoviePlayer:self];
		[mpMovieView setPreservesAspectRatio:( mnZoomLevel == ZoomLevel_FIT_TO_WINDOW ? NO : YES )];
		[mpMovieView setMovie:mpAVPlayer];
	}
#endif	// USE_QUICKTIME
}

#ifdef USE_QUICKTIME

- (NSObject *)movie
{
	return mpMovie;
}

#else	// USE_QUICKTIME

- (AVPlayer *)movie
{
	return mpAVPlayer;
}

#endif	// USE_QUICKTIME

- (AvmediaMovieView *)movieView
{
	return mpMovieView;
}

- (BOOL)isPlaying:(AvmediaArgs *)pArgs
{
#ifdef USE_QUICKTIME
	BOOL bRet = ( mpMovie && [mpMovie respondsToSelector:@selector(rate)] && [mpMovie rate] != 0 );
#else	// USE_QUICKTIME
	BOOL bRet = ( mpAVPlayer && mpAVPlayer.timeControlStatus != AVPlayerTimeControlStatusPaused && mpAVPlayer.rate != 0 );
#endif	// USE_QUICKTIME

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithBool:bRet]];

	return bRet;
}

- (double)rate:(AvmediaArgs *)pArgs
{
#ifdef USE_QUICKTIME
	double fRet = (double)( mpMovie && [mpMovie respondsToSelector:@selector(rate)] ? [mpMovie rate] : 0 );
#else	// USE_QUICKTIME
	double fRet = (double)( mpAVPlayer ? [mpAVPlayer rate] : 0 );
#endif	// USE_QUICKTIME

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithDouble:fRet]];

	return fRet;
}

- (BOOL)mute:(AvmediaArgs *)pArgs
{
#ifdef USE_QUICKTIME
	BOOL bRet = ( mpMovie && [mpMovie respondsToSelector:@selector(muted)] ? [mpMovie muted] : NO );
#else	// USE_QUICKTIME
	BOOL bRet = ( mpAVPlayer ? [mpAVPlayer isMuted] : NO );
#endif	// USE_QUICKTIME

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithBool:bRet]];

	return bRet;
}

- (void)play:(id)pObject
{
	(void)pObject;

#ifdef USE_QUICKTIME
	if ( mpMovie && [mpMovie respondsToSelector:@selector(play)] && [mpMovie respondsToSelector:@selector(rate)] && [mpMovie rate] == 0 )
		[mpMovie play];
#else	// USE_QUICKTIME
	if ( mpAVPlayer )
		[mpAVPlayer play];
#endif	// USE_QUICKTIME
}

#ifndef USE_QUICKTIME

- (void)playerItemDidPlayToEndTime:(NSNotification *)pNotification
{
	(void)pNotification;

	if ( mbLooping && mpAVPlayer )
	{
		AVPlayerItem *pAVPlayerItem = mpAVPlayer.currentItem;
		if ( pAVPlayerItem )
		{
			[pAVPlayerItem seekToTime:kCMTimeZero toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero completionHandler:^(BOOL bFinished) {
				if ( bFinished && mbLooping && mpAVPlayer )
					[mpAVPlayer play];
			}];
		}
	}
}

#endif  // !USE_QUICKTIME

- (void)preferredSize:(AvmediaArgs *)pArgs
{
	if ( pArgs )
		[pArgs setResult:[NSValue valueWithSize:maPreferredSize]];
}

#ifdef USE_QUICKTIME

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

#endif  // USE_QUICKTIME

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

#ifdef USE_QUICKTIME
	if ( mpMovie && [mpMovie respondsToSelector:@selector(setCurrentTime:)] && pQTMakeTimeWithTimeInterval )
		[mpMovie setCurrentTime:pQTMakeTimeWithTimeInterval( [pTime doubleValue] )];
#else	// USE_QUICKTIME
	if ( mpAVPlayer )
	{
		AVPlayerItem *pAVPlayerItem = mpAVPlayer.currentItem;
		if ( pAVPlayerItem )
			[pAVPlayerItem seekToTime:CMTimeMakeWithSeconds( [pTime doubleValue], PREFERRED_TIMESCALE ) toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero completionHandler:nil];
	}
#endif	// USE_QUICKTIME
}

- (void)setFocus:(id)pObject
{
	(void)pObject;

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
		case ZoomLevel_ORIGINAL:
			break;
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

#ifdef USE_QUICKTIME
	if ( mpMovie && [mpMovie respondsToSelector:@selector(setAttribute:forKey:)] && pQTMovieLoopsAttribute )
		[mpMovie setAttribute:pLooping forKey:*pQTMovieLoopsAttribute];
#else	// USE_QUICKTIME
	mbLooping = [pLooping boolValue];
#endif	// USE_QUICKTIME
}

- (void)setMute:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pMute = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pMute )
		return;

#ifdef USE_QUICKTIME
	if ( mpMovie && [mpMovie respondsToSelector:@selector(setMuted:)] )
		[mpMovie setMuted:[pMute boolValue]];
#else	// USE_QUICKTIME
	if ( mpAVPlayer )
		mpAVPlayer.muted = [pMute boolValue];
#endif	// USE_QUICKTIME
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

	float fRate = [pTime floatValue];
#ifdef USE_QUICKTIME
	if ( mpMovie && [mpMovie respondsToSelector:@selector(setRate:)] )
		[mpMovie setRate:fRate];
#else	// USE_QUICKTIME
	if ( mpAVPlayer )
	{
		if ( fRate == 0 || fRate == 1 )
		{
			mpAVPlayer.rate = fRate;
		}
		else
		{
			AVPlayerItem *pAVPlayerItem = mpAVPlayer.currentItem;
			if ( pAVPlayerItem )
			{
				if ( fRate < 0 && ( ( fRate == -1 && pAVPlayerItem.canPlayReverse ) || ( fRate > -1 && pAVPlayerItem.canPlaySlowReverse ) || ( fRate < -1 && pAVPlayerItem.canPlayFastReverse ) ) )
					mpAVPlayer.rate = fRate;
				else if ( fRate > 0 && ( ( fRate < 1 && pAVPlayerItem.canPlaySlowForward ) || ( fRate > 1 && pAVPlayerItem.canPlayFastForward ) ) )
					mpAVPlayer.rate = fRate;
			}
		}
	}
#endif	// USE_QUICKTIME
}

- (void)setSelection:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pTime = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pTime )
		return;

#ifdef USE_QUICKTIME
	if ( mpMovie && [mpMovie respondsToSelector:@selector(setSelection:)] && pQTMakeTimeRange && pQTMakeTimeWithTimeInterval )
		[mpMovie setSelection:pQTMakeTimeRange( pQTMakeTimeWithTimeInterval( 0 ), pQTMakeTimeWithTimeInterval( [pTime doubleValue] ) )];
#else	// USE_QUICKTIME
	mfStopTime = [pTime doubleValue];
#endif	// USE_QUICKTIME
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

#ifdef USE_QUICKTIME
	if ( mpMovie && [mpMovie respondsToSelector:@selector(setVolume:)] )
		[mpMovie setVolume:( (float)( [pDB shortValue] - nAVMediaMinDB ) / (float)( nAVMediaMaxDB - nAVMediaMinDB ) )];
#else	// USE_QUICKTIME
	float fVolume = pow( 10.0f, [pDB shortValue] / 20.0f );
	if ( fVolume < 0 )
		fVolume = 0;
	if ( fVolume > 1 )
		fVolume = 1;

	if ( mpAVPlayer )
		mpAVPlayer.volume = fVolume;
#endif	// USE_QUICKTIME
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
	(void)pObject;

#ifdef USE_QUICKTIME
	if ( mpMovie && [mpMovie respondsToSelector:@selector(rate)] && [mpMovie respondsToSelector:@selector(stop)] && [mpMovie rate] != 0 )
		[mpMovie stop];
#else	// USE_QUICKTIME
	if ( mpAVPlayer )
		[mpAVPlayer pause];
#endif	// USE_QUICKTIME
}

- (short)volumeDB:(AvmediaArgs *)pArgs
{
#ifdef USE_QUICKTIME
	short nRet = (short)( ( mpMovie && [mpMovie respondsToSelector:@selector(volume)] ? [mpMovie volume] : 0 ) * ( nAVMediaMaxDB - nAVMediaMinDB ) ) + nAVMediaMinDB;
#else	// USE_QUICKTIME
	short nRet = ( mpAVPlayer ? (short)( ( 20.0f * log10f( mpAVPlayer.volume ) ) +0.5f ) : 0 );
#endif	// USE_QUICKTIME

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
		com::sun::star::awt::FocusEvent aEvt;
		Application::PostUserEvent( STATIC_LINK( NULL, Window, fireFocusGainedEvent ), new FocusEventData( mpMoviePlayer, aEvt ) );
	}

	return bRet;
}

- (void)dealloc
{
	[self setCursor:nil];
	[self setMoviePlayer:nil];

#ifdef USE_QUICKTIME
	if ( mpQTMovieView )
	{
		[mpQTMovieView removeFromSuperview];
		[mpQTMovieView release];
	}
#else	// USE_QUICKTIME
	if ( mpAVPlayerView )
	{
		[mpAVPlayerView removeFromSuperview];
		[mpAVPlayerView release];
	}
#endif	// USE_QUICKTIME

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

#ifdef USE_QUICKTIME
	InitializeQTKit();
#endif	// USE_QUICKTIME

	mpCursor = nil;
	mpMoviePlayer = nil;
#ifdef USE_QUICKTIME
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
#else	// USE_QUICKTIME
	mpAVPlayerView = [[AVPlayerView alloc] initWithFrame:NSMakeRect( 0, 0, aFrame.size.width, aFrame.size.height )];
	if ( mpAVPlayerView )
	{
		mpAVPlayerView.accessibilityElement = YES;
		mpAVPlayerView.controlsStyle = AVPlayerViewControlsStyleNone;
		[self addSubview:mpAVPlayerView];
	}
#endif	// USE_QUICKTIME

	return self;
}

- (BOOL)isFlipped
{
	return YES;
}

- (NSMenu *)menuForEvent:(NSEvent *)pEvent
{
	(void)pEvent;

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
		if ( [mpCursor respondsToSelector:@selector(setOnMouseEntered:)] )
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

#ifdef USE_QUICKTIME
	if ( mpQTMovieView )
		[mpQTMovieView setFrame:NSMakeRect( 0, 0, aRect.size.width, aRect.size.height )];
#else	// USE_QUICKTIME
	if ( mpAVPlayerView )
		[mpAVPlayerView setFrame:NSMakeRect( 0, 0, aRect.size.width, aRect.size.height )];
#endif	// USE_QUICKTIME
}

#ifdef USE_QUICKTIME

- (void)setMovie:(NSObject *)pMovie
{
	if ( !pMovie || [pMovie isKindOfClass:aQTMovieClass] )
	{
		if ( mpQTMovieView && [mpQTMovieView respondsToSelector:@selector(setMovie:)] )
			[mpQTMovieView setMovie:(QTMovie *)pMovie];
	}
}

#else	// USE_QUICKTIME

- (void)setMovie:(AVPlayer *)pPlayer
{
	if ( mpAVPlayerView )
		mpAVPlayerView.player = pPlayer;
}

#endif	// USE_QUICKTIME

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
#ifdef USE_QUICKTIME
	if ( mpQTMovieView && [mpQTMovieView respondsToSelector:@selector(setPreservesAspectRatio:)] )
		[mpQTMovieView setPreservesAspectRatio:bPreservesAspectRatio];
#else	// USE_QUICKTIME
	if ( mpAVPlayerView )
		mpAVPlayerView.videoGravity = ( bPreservesAspectRatio ? AVLayerVideoGravityResizeAspect : AVLayerVideoGravityResize );
#endif	// USE_QUICKTIME
}

@end
