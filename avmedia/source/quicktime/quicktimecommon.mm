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

static const short nAVMediaMinDB = -40;
static const short nAVMediaMaxDB = 0;

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
		[pArgs setResult:[NSValue valueWithRect:[mpMovieView frame]]];
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

- (id)init
{
	[super init];

	mpMovie = nil;
	mpMovieView = nil;
	mbPlaying = NO;
	maPreferredSize = NSMakeSize( 0, 0 );

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
			maPreferredSize = NSMakeSize( 1, 1 );

		NSRect aFrame = NSMakeRect( 0, 0, maPreferredSize.width, maPreferredSize.height );
		mpMovieView = [[AvmediaMovieView alloc] initWithFrame:aFrame];
		[mpMovieView setMoviePlayer:self];
		[mpMovieView setFillColor:[NSColor clearColor]];
		[mpMovieView setControllerVisible:NO];
		[mpMovieView setPreservesAspectRatio:YES];
		[mpMovieView setShowsResizeIndicator:NO];
		[mpMovieView setMovie:mpMovie];
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

	if ( mbPlaying && ![mpMovieView inLiveResize] )
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
	[mpMovie stop];
	mbPlaying = NO;

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
	if ( ![mpMovieView inLiveResize] )
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

	if ( NSEqualRects( aNewFrame, [mpMovieView frame] ) )
		return;

	if ( mbPlaying )
		[mpMovie stop];

	// No need to flip coordinates as the JavaSalObject view is already flipped
	// like our view
	[mpMovieView setFrame:aNewFrame];

	if ( mbPlaying && ![mpMovieView inLiveResize] )
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

	if ( mbPlaying && ![mpMovieView inLiveResize] )
		[mpMovie play];
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

	if ( mbPlaying && ![mpMovieView inLiveResize] )
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

	if ( mbPlaying && ![mpMovieView inLiveResize] )
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

		if ( mbPlaying && ![mpMovieView inLiveResize] )
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

		if ( mbPlaying && ![mpMovieView inLiveResize] )
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

	if ( mbPlaying )
		[mpMovie stop];

	// No need to flip coordinates as the JavaSalObject view is already flipped
	// like our view
	[mpMovieView setFrame:aNewFrame];

	[mpMovieView removeFromSuperview];
	if ( mpSuperview )
	{
		[mpSuperview release];
		mpSuperview = nil;
	}

	mpSuperview = pSuperview;
	[mpSuperview retain];

	// Only attach to superview if we are not live resizing
	if ( ![mpMovieView inLiveResize] )
		[mpSuperview addSubview:mpMovieView positioned:NSWindowAbove relativeTo:nil];

	if ( mbPlaying && ![mpMovieView inLiveResize] )
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

- (void)stop:(id)pObject
{
	[mpMovie stop];
	mbPlaying = NO;
}

- (NSView *)superview
{
	return mpSuperview;
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

- (void)dealloc
{
	[self setMoviePlayer:nil];

	[super dealloc];
}

- (id)initWithFrame:(NSRect)aFrame
{
	[super initWithFrame:aFrame];

	mpMoviePlayer = nil;

	return self;
}

- (MacOSBOOL)isFlipped
{
	return YES;
}

- (void)setMoviePlayer:(AvmediaMoviePlayer *)pPlayer
{
	if ( mpMoviePlayer )
		[mpMoviePlayer release];

	mpMoviePlayer = pPlayer;

	if ( mpMoviePlayer )
		[mpMoviePlayer retain];
}

- (void)viewDidEndLiveResize
{
	// Start playing movie after resizing has ended
	if ( mpMoviePlayer )
	{
		NSView *pSuperview = [mpMoviePlayer superview];
		if ( pSuperview && ![self superview] )
			[pSuperview addSubview:self positioned:NSWindowAbove relativeTo:nil];

		if ( [mpMoviePlayer isPlaying:nil] )
		{
			QTMovie *pMovie = [self movie];
			if ( pMovie )
				[pMovie play];
		}
	}

	[super viewDidEndLiveResize];

	[self display];
}

- (void)viewWillStartLiveResize
{
	[super viewWillStartLiveResize];

	// Prevent deadlocking in Java drawing calls on Mac OS X 10.3.9 by
	// stopping the movie and detaching it from its superview
	if ( mpMoviePlayer )
	{
		QTMovie *pMovie = [self movie];
		if ( pMovie )
			[pMovie stop];
	}

	[self removeFromSuperview];
}

@end
