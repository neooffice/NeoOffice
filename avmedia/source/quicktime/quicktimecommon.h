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

#import <premac.h>
#import <QTKit/QTKit.h>
#import <postmac.h>

// Redefine Cocoa YES and NO defines types for convenience
#ifdef YES
#undef YES
#define YES (MacOSBOOL)1
#endif
#ifdef NO
#undef NO
#define NO (MacOSBOOL)0
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
	QTMovie*				mpMovie;
	AvmediaMovieView*		mpMovieView;
	MacOSBOOL				mbPlaying;
	NSView*					mpSuperview;
	NSSize					maPreferredSize;
	NSRect					maRealFrame;
	int						mnZoomLevel;
}
- (void)bounds:(AvmediaArgs *)pArgs;
- (double)currentTime:(AvmediaArgs *)pArgs;
- (void)dealloc;
- (double)duration:(AvmediaArgs *)pArgs;
- (NSBitmapImageRep *)frameImageAtTime:(AvmediaArgs *)pArgs;
- (id)init;
- (void)initialize:(NSURL *)pURL;
- (MacOSBOOL)isPlaying:(AvmediaArgs *)pArgs;
- (QTMovie *)movie;
- (QTMovieView *)movieView;
- (MacOSBOOL)mute:(AvmediaArgs *)pArgs;
- (void)play:(id)pObject;
- (void)preferredSize:(AvmediaArgs *)pArgs;
- (double)rate:(AvmediaArgs *)pArgs;
- (void)release:(id)pObject;
- (double)selectionEnd:(AvmediaArgs *)pArgs;
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

@interface AvmediaMovieView : QTMovieView
{
	NSCursor*				mpCursor;
	AvmediaMoviePlayer*		mpMoviePlayer;
}
- (MacOSBOOL)becomeFirstResponder;
- (void)dealloc;
- (id)initWithFrame:(NSRect)aFrame;
- (MacOSBOOL)isFlipped;
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
- (void)setMoviePlayer:(AvmediaMoviePlayer *)pPlayer;
@end

#endif	// _COMMON_H
