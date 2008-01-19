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
	if ( mpMovieView )
	{
		[mpMovieView setMovie:nil];
		[mpMovieView release];
	}

	if ( mpMovie )
		[mpMovie release];

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

	return self;
}

- (void)initialize:(NSURL *)pURL
{
	mpMovie = [QTMovie movieWithURL:pURL error:nil];
	if ( mpMovie )
	{
		[mpMovie setAttribute:[NSNumber numberWithBool:NO] forKey:QTMovieLoopsAttribute];
		[mpMovie setSelection:QTMakeTimeRange( QTMakeTimeWithTimeInterval( 0 ), [mpMovie duration] )];
		[mpMovie retain];

		NSRect aFrame;
		NSImage *pImage = [mpMovie currentFrameImage];
		if ( pImage )
		{
			NSSize aSize = [pImage size];
			aFrame = NSMakeRect( 0, 0, aSize.width, aSize.height );
		}
		else
		{
			aFrame = NSMakeRect( 0, 0, 1, 1 );
		}

		mpMovieView = [[QTMovieView alloc] initWithFrame:aFrame];
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
			if ( pLooping || ![pLooping boolValue] )
				mbPlaying = NO;
		}
	}

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
	[mpMovie play];
	mbPlaying = YES;
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

- (void)setCurrentTime:(AvmediaArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pTime = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pTime )
		return;

	[mpMovie setCurrentTime:QTMakeTimeWithTimeInterval( [pTime doubleValue] )];
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

	[mpMovie setSelection:QTMakeTimeRange( QTMakeTimeWithTimeInterval( 0 ), QTMakeTimeWithTimeInterval( [pTime doubleValue] ) )];
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

- (NSSize)size:(AvmediaArgs *)pArgs
{
	NSSize aRet = [mpMovieView frame].size;

	if ( pArgs )
		[pArgs setResult:[NSValue valueWithSize:aRet]];

	return aRet;
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
