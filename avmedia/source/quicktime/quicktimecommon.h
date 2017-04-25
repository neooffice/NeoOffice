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
 *  Patrick Luby, January 2008
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2008 Planamesa Inc.
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

#ifndef _COMMON_H
#define _COMMON_H

// Uncomment the following line to enable QuickTime
// #define USE_QUICKTIME

#import <premac.h>
#import <AppKit/AppKit.h>
#ifdef USE_QUICKTIME
#import <objc/objc-class.h>
#else	// USE_QUICKTIME
#import <AVFoundation/AVFoundation.h>
#import <AVKit/AVKit.h>
#endif	// USE_QUICKTIME
#import <postmac.h>

#ifndef QTTime
typedef struct {
	long long		timeValue;
	long			timeScale;
	long			flags;
} QTTime;
#endif

#ifndef QTTimeRange
typedef struct {
	QTTime		time;
	QTTime		duration;
} QTTimeRange;
#endif

@interface AvmediaArgs : NSObject
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

@class AvmediaMovieView;

@interface AvmediaMoviePlayer : NSObject
{
#ifdef USE_QUICKTIME
	NSObject*				mpMovie;
#else	// USE_QUICKTIME
	AVPlayer*				mpAVPlayer;
	BOOL					mbLooping;
	double					mfStopTime;
#endif	// USE_QUICKTIME
	AvmediaMovieView*		mpMovieView;
	NSView*					mpSuperview;
	NSSize					maPreferredSize;
	NSRect					maRealFrame;
	int						mnZoomLevel;
	id						mpSecurityScopedURL;
	NSURL*					mpURL;
}
- (void)bounds:(AvmediaArgs *)pArgs;
- (double)currentTime:(AvmediaArgs *)pArgs;
- (void)destroy:(id)pObject;
- (double)duration:(AvmediaArgs *)pArgs;
- (NSBitmapImageRep *)frameImageAtTime:(AvmediaArgs *)pArgs;
- (id)initWithURL:(NSURL *)pURL;
- (void)initialize:(id)pObject;
- (BOOL)isPlaying:(AvmediaArgs *)pArgs;
#ifdef USE_QUICKTIME
- (NSObject *)movie;
#else	// USE_QUICKTIME
- (AVPlayer *)movie;
#endif	// USE_QUICKTIME
- (AvmediaMovieView *)movieView;
- (BOOL)mute:(AvmediaArgs *)pArgs;
- (void)play:(id)pObject;
#ifndef USE_QUICKTIME
- (void)playerItemDidPlayToEndTime:(NSNotification *)pNotification;
#endif	// !USE_QUICKTIME
- (void)preferredSize:(AvmediaArgs *)pArgs;
- (double)rate:(AvmediaArgs *)pArgs;
#ifdef USE_QUICKTIME
- (double)selectionEnd:(AvmediaArgs *)pArgs;
#endif	// USE_QUICKTIME
- (void)setBounds:(AvmediaArgs *)pArgs;
- (void)setCurrentTime:(AvmediaArgs *)pArgs;
- (void)setFocus:(id)pObject;
- (void)setFrame:(NSRect)aRect;
- (void)setLooping:(AvmediaArgs *)pArgs;
- (void)setMute:(AvmediaArgs *)pArgs;
- (void)setPointer:(AvmediaArgs *)pArgs;
- (void)setRate:(AvmediaArgs *)pArgs;
- (void)setSelection:(AvmediaArgs *)pArgs;
- (void)setSuperview:(AvmediaArgs *)pArgs;
- (void)setVolumeDB:(AvmediaArgs *)pArgs;
- (void)setZoomLevel:(AvmediaArgs *)pArgs;
- (void)stop:(id)pObject;
- (short)volumeDB:(AvmediaArgs *)pArgs;
@end

@interface AvmediaMovieView : NSView
{
	NSCursor*				mpCursor;
	AvmediaMoviePlayer*		mpMoviePlayer;
#ifdef USE_QUICKTIME
	NSView*					mpQTMovieView;
#else	// USE_QUICKTIME
	AVPlayerView*			mpAVPlayerView;
#endif	// USE_QUICKTIME
}
+ (NSMenu *)defaultMenu;
- (BOOL)becomeFirstResponder;
- (void)dealloc;
- (NSView *)hitTest:(NSPoint)aPoint;
- (id)initWithFrame:(NSRect)aFrame;
- (BOOL)isFlipped;
- (NSMenu *)menuForEvent:(NSEvent *)pEvent;
- (void)mouseDown:(NSEvent *)pEvent;
- (void)mouseDragged:(NSEvent *)pEvent;
- (void)mouseEntered:(NSEvent *)pEvent;
- (void)mouseExited:(NSEvent *)pEvent;
- (void)mouseMoved:(NSEvent *)pEvent;
- (void)mouseUp:(NSEvent *)pEvent;
- (void)rightMouseDown:(NSEvent *)pEvent;
- (void)rightMouseDragged:(NSEvent *)pEvent;
- (void)rightMouseUp:(NSEvent *)pEvent;
- (void)otherMouseDown:(NSEvent *)pEvent;
- (void)otherMouseDragged:(NSEvent *)pEvent;
- (void)otherMouseUp:(NSEvent *)pEvent;
- (void)resetCursorRects;
- (void)setCursor:(NSCursor *)pCursor;
- (void)setFrame:(NSRect)aRect;
#ifdef USE_QUICKTIME
- (void)setMovie:(NSObject *)pMovie;
#else	// USE_QUICKTIME
- (void)setMovie:(AVPlayer *)pMovie;
#endif	// USE_QUICKTIME
- (void)setMoviePlayer:(AvmediaMoviePlayer *)pPlayer;
- (void)setPreservesAspectRatio:(BOOL)bPreservesAspectRatio;
@end

#endif	// _COMMON_H
