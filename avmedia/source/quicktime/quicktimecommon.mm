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

#import "quicktimecommon.h"
#import "quicktimewindow.hxx"

#ifndef _COM_SUN_STAR_AWT_KEYMODIFIER_HDL_
#include <com/sun/star/awt/KeyModifier.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MOUSEBUTTON_HDL_
#include <com/sun/star/awt/MouseButton.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SYSTEMPOINTER_HDL_
#include <com/sun/star/awt/SystemPointer.hpp>
#endif
#ifndef _COM_SUN_STAR_MEDIA_ZOOMLEVEL_HDL_
#include <com/sun/star/media/ZoomLevel.hpp>
#endif

static const short nAVMediaMinDB = -40;
static const short nAVMediaMaxDB = 0;

using namespace ::avmedia::quicktime;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::media;

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

@implementation AvmediaMoviePlayer

- (void)bounds:(AvmediaArgs *)pArgs
{
	if ( pArgs )
		[pArgs setResult:[NSValue valueWithRect:maRealFrame]];
}

- (double)currentTime:(AvmediaArgs *)pArgs
{
	double fRet = 0;

	NSTimeInterval aInterval;
	if ( QTGetTimeInterval( [mpMovie currentTime], &aInterval ) )
		fRet = aInterval;

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithDouble:fRet]];

	return fRet;
}

- (void)dealloc
{
	if ( mpMovie )
		[mpMovie stop];

	if ( mpMovieView )
	{
		[mpMovieView removeFromSuperview];
		[mpMovieView setMoviePlayer:nil];
		[mpMovieView setMovie:nil];
		[mpMovieView release];
	}

	if ( mpMovie )
		[mpMovie release];

	if ( mpSuperview )
		[mpSuperview release];

	[super dealloc];
}

- (double)duration:(AvmediaArgs *)pArgs
{
	double fRet = 0;

	NSTimeInterval aInterval;
	if ( QTGetTimeInterval( [mpMovie duration], &aInterval ) )
		fRet = aInterval;

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithDouble:fRet]];

	return fRet;
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

	NSImage *pImage = [mpMovie frameImageAtTime:QTMakeTimeWithTimeInterval( [pTime doubleValue] )];
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

- (id)init
{
	[super init];

	mpMovie = nil;
	mpMovieView = nil;
	mbPlaying = NO;
	maPreferredSize = NSMakeSize( 0, 0 );
	maRealFrame = NSMakeRect( 0, 0, 0, 0 );
	mnZoomLevel = ZoomLevel_FIT_TO_WINDOW;

	return self;
}

- (void)initialize:(NSURL *)pURL
{
	NSError *pError = nil;
	mpMovie = [QTMovie movieWithURL:pURL error:&pError];
	if ( mpMovie && !pError )
	{
		[mpMovie retain];
		[mpMovie setAttribute:[NSNumber numberWithBool:NO] forKey:QTMovieLoopsAttribute];
		[mpMovie setSelection:QTMakeTimeRange( QTMakeTimeWithTimeInterval( 0 ), [mpMovie duration] )];

		NSImage *pImage = [mpMovie frameImageAtTime:QTMakeTimeWithTimeInterval( 0 )];
		if ( pImage )
			maPreferredSize = [pImage size];
		else
			maPreferredSize = NSMakeSize( 0, 0 );
		maRealFrame = NSMakeRect( 0, 0, maPreferredSize.width, maPreferredSize.height );

		mpMovieView = [[AvmediaMovieView alloc] initWithFrame:maRealFrame];
		[mpMovieView setMoviePlayer:self];
		[mpMovieView setFillColor:[NSColor clearColor]];
		[mpMovieView setControllerVisible:NO];
		[mpMovieView setPreservesAspectRatio:YES];
		[mpMovieView setShowsResizeIndicator:NO];
		[mpMovieView setMovie:mpMovie];
		[mpMovieView setEditable:NO];
	}
}

- (QTMovie *)movie
{
	return mpMovie;
}

- (QTMovieView *)movieView
{
	return mpMovieView;
}

- (MacOSBOOL)isPlaying:(AvmediaArgs *)pArgs
{
	// Check if we are at the end of the movie
	if ( mbPlaying )
	{
		NSTimeInterval aCurrentInterval;
		NSTimeInterval aDurationInterval;
		if ( QTGetTimeInterval( [mpMovie currentTime], &aCurrentInterval ) && QTGetTimeInterval( [mpMovie duration], &aDurationInterval ) && aCurrentInterval >= aDurationInterval )
		{
			NSNumber *pLooping = [mpMovie attributeForKey:QTMovieLoopsAttribute];
			if ( pLooping && ![pLooping boolValue] )
				mbPlaying = NO;
		}
	}

	if ( mbPlaying )
		[mpMovie play];
	else
		[mpMovie stop];

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithBool:mbPlaying]];

	return mbPlaying;
}

- (double)rate:(AvmediaArgs *)pArgs
{
	double fRet = (double)[mpMovie rate];

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithDouble:fRet]];

	return fRet;
}

- (void)release:(id)pObject
{
	[self release];
}

- (MacOSBOOL)mute:(AvmediaArgs *)pArgs
{
	MacOSBOOL bRet = [mpMovie muted];

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithBool:bRet]];

	return bRet;
}

- (void)play:(id)pObject
{
	[mpMovie play];
	mbPlaying = YES;
}

- (void)preferredSize:(AvmediaArgs *)pArgs
{
	if ( pArgs )
		[pArgs setResult:[NSValue valueWithSize:maPreferredSize]];
}

- (double)selectionEnd:(AvmediaArgs *)pArgs
{
	double fRet = 0;

	NSTimeInterval aInterval;
	if ( QTGetTimeInterval( [mpMovie selectionEnd], &aInterval ) )
	{
		fRet = aInterval;
	}
	else if ( QTGetTimeInterval( [mpMovie duration], &aInterval ) )
	{
		fRet = aInterval;
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

	if ( mbPlaying )
		[mpMovie stop];

	// No need to flip coordinates as the JavaSalObject view is already flipped
	// like our view
	[self setFrame:aNewFrame];

	if ( mbPlaying )
		[mpMovie play];
}

- (void)setCurrentTime:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pTime = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pTime )
		return;

	if ( mbPlaying )
		[mpMovie stop];

	[mpMovie setCurrentTime:QTMakeTimeWithTimeInterval( [pTime doubleValue] )];

	if ( mbPlaying )
		[mpMovie play];
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

	NSRect aZoomFrame = NSMakeRect( aRect.origin.x, aRect.origin.y, aRect.size.width, aRect.size.height );

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
		default:
			break;
	}

	// Center zoom bounds with real bounds
	aZoomFrame.origin.x += ( maRealFrame.size.width - aZoomFrame.size.width ) / 2;
	aZoomFrame.origin.y += ( maRealFrame.size.height - aZoomFrame.size.height ) / 2;

	if ( NSEqualRects( maRealFrame, aOldRealFrame ) && NSEqualRects( aZoomFrame, [mpMovieView frame] ) )
		return;

	[mpMovieView setFrame:aZoomFrame];
	[mpMovieView setNeedsDisplay:YES];
}

- (void)setLooping:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pLooping = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pLooping )
		return;

	[mpMovie setAttribute:pLooping forKey:QTMovieLoopsAttribute];
}

- (void)setMute:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pMute = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pMute )
		return;

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

	if ( mbPlaying )
		[mpMovie stop];

	[mpMovie setRate:[pTime floatValue]];

	if ( mbPlaying )
		[mpMovie play];
}

- (void)setSelection:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pTime = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pTime )
		return;

	if ( mbPlaying )
		[mpMovie stop];

	[mpMovie setSelection:QTMakeTimeRange( QTMakeTimeWithTimeInterval( 0 ), QTMakeTimeWithTimeInterval( [pTime doubleValue] ) )];

	if ( mbPlaying )
		[mpMovie play];
}

- (void)setSuperview:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 2 )
	{
		if ( mbPlaying )
			[mpMovie stop];

		[mpMovieView removeFromSuperview];
		if ( mpSuperview )
		{
			[mpSuperview release];
			mpSuperview = nil;
		}

		if ( mbPlaying )
			[mpMovie play];

		return;
	}

	NSView *pSuperview = (NSView *)[pArgArray objectAtIndex:0];
	if ( !pSuperview )
	{
		if ( mbPlaying )
			[mpMovie stop];

		[mpMovieView removeFromSuperview];
		if ( mpSuperview )
		{
			[mpSuperview release];
			mpSuperview = nil;
		}

		if ( mbPlaying )
			[mpMovie play];

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

	if ( mbPlaying )
		[mpMovie stop];

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

	if ( mbPlaying )
		[mpMovie play];
}

- (void)setVolumeDB:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pDB = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pDB )
		return;

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
		case ZoomLevel_FIT_TO_WINDOW:
		case ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT:
		case ZoomLevel_ZOOM_1_TO_4:
		case ZoomLevel_ZOOM_1_TO_2:
		case ZoomLevel_ZOOM_2_TO_1:
		case ZoomLevel_ZOOM_4_TO_1:
			mnZoomLevel = [pZoomLevel intValue];
			break;
		default:
			break;
	}

	if ( mnZoomLevel == nOldZoomLevel )
		return;

	if ( mbPlaying )
		[mpMovie stop];

	// No need to flip coordinates as the JavaSalObject view is already flipped
	// like our view
	[self setFrame:maRealFrame];

	if ( mbPlaying )
		[mpMovie play];
}

- (void)stop:(id)pObject
{
	[mpMovie stop];
	mbPlaying = NO;
}

- (short)volumeDB:(AvmediaArgs *)pArgs
{
	short nRet = (short)( [mpMovie volume] * ( nAVMediaMaxDB - nAVMediaMinDB ) ) + nAVMediaMinDB;

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

- (MacOSBOOL)becomeFirstResponder
{
	MacOSBOOL bRet = [super becomeFirstResponder];

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

	[super dealloc];
}

- (id)initWithFrame:(NSRect)aFrame
{
	[super initWithFrame:aFrame];

	mpCursor = nil;
	mpMoviePlayer = nil;

	return self;
}

- (MacOSBOOL)isFlipped
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

- (void)setMoviePlayer:(AvmediaMoviePlayer *)pPlayer
{
	if ( mpMoviePlayer )
		[mpMoviePlayer release];

	mpMoviePlayer = pPlayer;

	if ( mpMoviePlayer )
		[mpMoviePlayer retain];
}

@end
