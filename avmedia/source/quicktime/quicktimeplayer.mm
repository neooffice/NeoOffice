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

#import <premac.h>
#import <QTKit/QTKit.h>
#import <postmac.h>
#import "quicktimeplayer.hxx"

#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif

// Redefine Cocoa YES and NO defines types for convenience
#ifdef YES
#undef YES
#define YES (MacOSBOOL)1
#endif
#ifdef NO
#undef NO
#define NO (MacOSBOOL)0
#endif

#define AVMEDIA_QUICKTIME_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_QuickTime"
#define AVMEDIA_QUICKTIME_PLAYER_SERVICENAME "com.sun.star.media.Player_QuickTime"

static const short nAVMediaMinDB = -40;
static const short nAVMediaMaxDB = 0;

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::media;
using namespace ::com::sun::star::uno;

@interface QTMoviePlayerArgs : NSObject
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

@implementation QTMoviePlayerArgs

+ (id)argsWithArgs:(NSArray *)pArgs
{
	QTMoviePlayerArgs *pRet = [[QTMoviePlayerArgs alloc] initWithArgs:pArgs];
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

@interface QTMoviePlayer : NSObject
{
	QTMovie*				mpMovie;
}
- (double)currentTime:(QTMoviePlayerArgs *)pArgs;
- (void)dealloc;
- (double)duration:(QTMoviePlayerArgs *)pArgs;
- (id)init;
- (void)initialize:(NSURL *)pURL;
- (QTMovie *)movie;
- (MacOSBOOL)mute:(QTMoviePlayerArgs *)pArgs;
- (void)play:(id)pObject;
- (double)rate:(QTMoviePlayerArgs *)pArgs;
- (void)release:(id)pObject;
- (void)restartIfFinished:(id)pObject;
- (double)selectionEnd:(QTMoviePlayerArgs *)pArgs;
- (void)setCurrentTime:(QTMoviePlayerArgs *)pArgs;
- (void)setMute:(QTMoviePlayerArgs *)pArgs;
- (void)setRate:(QTMoviePlayerArgs *)pArgs;
- (void)setSelection:(QTMoviePlayerArgs *)pArgs;
- (void)setVolumeDB:(QTMoviePlayerArgs *)pArgs;
- (void)stop:(id)pObject;
- (short)volumeDB:(QTMoviePlayerArgs *)pArgs;
@end

@implementation QTMoviePlayer

- (double)currentTime:(QTMoviePlayerArgs *)pArgs
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
		[mpMovie release];

	[super dealloc];
}

- (double)duration:(QTMoviePlayerArgs *)pArgs
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

	return self;
}

- (void)initialize:(NSURL *)pURL
{
	mpMovie = [QTMovie movieWithURL:pURL error:nil];
	if ( mpMovie )
	{
		[mpMovie setSelection:QTMakeTimeRange( QTMakeTimeWithTimeInterval( 0 ), [mpMovie duration] )];
		[mpMovie retain];
	}
}

- (void)restartIfFinished:(id)pObject
{
	NSTimeInterval aCurrentInterval;
	NSTimeInterval aDurationInterval;
	if ( QTGetTimeInterval( [mpMovie currentTime], &aCurrentInterval ) && QTGetTimeInterval( [mpMovie duration], &aDurationInterval ) && aCurrentInterval >= aDurationInterval )
		[mpMovie gotoBeginning];
}

- (QTMovie *)movie
{
	return mpMovie;
}

- (double)rate:(QTMoviePlayerArgs *)pArgs
{
	double fRet = (double)[mpMovie rate];

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithDouble:fRet]];

	return fRet;
}

- (void)release:(id)pObject
{
	[mpMovie stop];
	[self release];
}

- (MacOSBOOL)mute:(QTMoviePlayerArgs *)pArgs
{
	MacOSBOOL bRet = [mpMovie muted];

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithBool:bRet]];

	return bRet;
}

- (void)play:(id)pObject
{
	[mpMovie play];
}

- (double)selectionEnd:(QTMoviePlayerArgs *)pArgs
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

- (void)setCurrentTime:(QTMoviePlayerArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pTime = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pTime )
		return;

	[mpMovie setCurrentTime:QTMakeTimeWithTimeInterval( [pTime doubleValue] )];
}

- (void)setMute:(QTMoviePlayerArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pMute = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pMute )
		return;

	[mpMovie setMuted:[pMute boolValue]];
}

- (void)setRate:(QTMoviePlayerArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pTime = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pTime )
		return;

	[mpMovie setRate:[pTime floatValue]];
}

- (void)setSelection:(QTMoviePlayerArgs *)pArgs
{
	NSArray *pArgArray = [pArgs args];
	if ( !pArgArray || [pArgArray count] < 1 )
		return;

	NSNumber *pTime = (NSNumber *)[pArgArray objectAtIndex:0];
	if ( !pTime )
		return;

	[mpMovie setSelection:QTMakeTimeRange( QTMakeTimeWithTimeInterval( 0 ), QTMakeTimeWithTimeInterval( [pTime doubleValue] ) )];
}

- (void)setVolumeDB:(QTMoviePlayerArgs *)pArgs
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
}

- (short)volumeDB:(QTMoviePlayerArgs *)pArgs
{
	short nRet = (short)( [mpMovie volume] * ( nAVMediaMaxDB - nAVMediaMinDB ) ) + nAVMediaMinDB;

	if ( pArgs )
		[pArgs setResult:[NSNumber numberWithShort:nRet]];

	return nRet;
}

@end

namespace avmedia
{
namespace quicktime
{

// ============================================================================

Player::Player( const Reference< XMultiServiceFactory >& rxMgr ) :
	mbLooping( false ),
	mxMgr( rxMgr ),
	mpMoviePlayer( NULL ),
	mbRunning( false )
{
}

// ----------------------------------------------------------------------------

Player::~Player()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(release:) withObject:(id)mpMoviePlayer waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

bool Player::create( const ::rtl::OUString& rURL )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( isValid() )
	{
		if ( mpMoviePlayer )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(release:) withObject:(id)mpMoviePlayer waitUntilDone:NO modes:pModes];
			mpMoviePlayer = NULL;
		}

		mbLooping = sal_False;
		mbRunning = sal_False;

		NSString *pString = [NSString stringWithCharacters:rURL.getStr() length:rURL.getLength()];
		if ( pString )
		{
			NSURL *pURL = [NSURL URLWithString:pString];
			if ( pURL )
			{
				// Do not retain as invoking alloc disables autorelease
				QTMoviePlayer *pMoviePlayer = [[QTMoviePlayer alloc] init];
				if ( pMoviePlayer )
				{
					NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
					[pMoviePlayer performSelectorOnMainThread:@selector(initialize:) withObject:pURL waitUntilDone:YES modes:pModes];

					if ( [pMoviePlayer movie ] )
					{
						mpMoviePlayer = pMoviePlayer;
						bRet = true;
					}
				}
			}
		}
	}

	[pPool release];

	return bRet;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::start() throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer && !mbRunning )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(play:) withObject:(id)mpMoviePlayer waitUntilDone:NO modes:pModes];
		mbRunning = sal_True;
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::stop() throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(stop:) withObject:(id)mpMoviePlayer waitUntilDone:NO modes:pModes];
	}

	mbRunning = sal_False;

	[pPool release];
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaying() throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mbRunning && mbLooping && mpMoviePlayer )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(restartIfFinished:) withObject:(id)mpMoviePlayer waitUntilDone:NO modes:pModes];
	}

	[pPool release];

	return mbRunning;
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getDuration() throw( RuntimeException )
{
	double fRet = 0;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		QTMoviePlayerArgs *pArgs = [QTMoviePlayerArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(duration:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSNumber *pRet = (NSNumber *)[pArgs result];
		if ( pRet )
			fRet = [pRet doubleValue];
	}

	[pPool release];

	return fRet;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setMediaTime( double fTime ) throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		QTMoviePlayerArgs *pArgs = [QTMoviePlayerArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithDouble:fTime]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setCurrentTime:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getMediaTime() throw( RuntimeException )
{
	double fRet = 0;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		QTMoviePlayerArgs *pArgs = [QTMoviePlayerArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(currentTime:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSNumber *pRet = (NSNumber *)[pArgs result];
		if ( pRet )
			fRet = [pRet doubleValue];
	}

	[pPool release];

	return fRet;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setStopTime( double fTime ) throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		QTMoviePlayerArgs *pArgs = [QTMoviePlayerArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithDouble:fTime]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setSelection:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getStopTime() throw( RuntimeException )
{
	double fRet = 0;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		QTMoviePlayerArgs *pArgs = [QTMoviePlayerArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(selectionEnd:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSNumber *pRet = (NSNumber *)[pArgs result];
		if ( pRet )
			fRet = [pRet doubleValue];
	}

	[pPool release];

	return fRet;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setRate( double fRate ) throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		QTMoviePlayerArgs *pArgs = [QTMoviePlayerArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithDouble:fRate]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setRate:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getRate() throw( RuntimeException )
{
	double fRet = 0;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		QTMoviePlayerArgs *pArgs = [QTMoviePlayerArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(rate:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSNumber *pRet = (NSNumber *)[pArgs result];
		if ( pRet )
			fRet = [pRet doubleValue];
	}

	[pPool release];

	return fRet;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet ) throw( RuntimeException )
{
	mbLooping = bSet;
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaybackLoop() throw( RuntimeException )
{
	return mbLooping;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setMute( sal_Bool bSet ) throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		QTMoviePlayerArgs *pArgs = [QTMoviePlayerArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithBool:( bSet ? YES : NO )]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setMute:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isMute() throw( RuntimeException )
{
	sal_Bool bRet = sal_False;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		QTMoviePlayerArgs *pArgs = [QTMoviePlayerArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(mute:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSNumber *pRet = (NSNumber *)[pArgs result];
		if ( pRet )
			bRet = [pRet boolValue];
	}

	[pPool release];

	return bRet;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB ) throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		QTMoviePlayerArgs *pArgs = [QTMoviePlayerArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithShort:nVolumeDB]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setVolumeDB:) withObject:pArgs waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

sal_Int16 SAL_CALL Player::getVolumeDB() throw( RuntimeException )
{
	sal_Int16 nRet = 0;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		QTMoviePlayerArgs *pArgs = [QTMoviePlayerArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(QTMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(volumeDB:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSNumber *pRet = (NSNumber *)[pArgs result];
		if ( pRet )
			nRet = (sal_Int16)[pRet shortValue];
	}

	[pPool release];

	return nRet;
}

// ----------------------------------------------------------------------------

Size SAL_CALL Player::getPreferredPlayerWindowSize() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::getPreferredPlayerWindowSize not implemented\n" );
#endif
	Size aSize( 0, 0 );
	return aSize;
}

// ----------------------------------------------------------------------------

Reference< XPlayerWindow > SAL_CALL Player::createPlayerWindow( const Sequence< Any >& aArguments ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::createPlayerWindow not implemented\n" );
#endif
	Reference< XPlayerWindow > xRet;
	return xRet;
}

// ----------------------------------------------------------------------------

Reference< XFrameGrabber > SAL_CALL Player::createFrameGrabber() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::createFrameGrabber not implemented\n" );
#endif
	Reference< XFrameGrabber > xRet;
	return xRet;
}

// ----------------------------------------------------------------------------

::rtl::OUString SAL_CALL Player::getImplementationName() throw( RuntimeException )
{
	return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_QUICKTIME_PLAYER_IMPLEMENTATIONNAME ) );
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::supportsService( const ::rtl::OUString& ServiceName ) throw( RuntimeException )
{
	return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( AVMEDIA_QUICKTIME_PLAYER_SERVICENAME ) );
}

// ----------------------------------------------------------------------------

Sequence< ::rtl::OUString > SAL_CALL Player::getSupportedServiceNames() throw( RuntimeException )
{
	Sequence< ::rtl::OUString > aRet(1);
	aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( AVMEDIA_QUICKTIME_PLAYER_SERVICENAME ) );

	return aRet;
}

// ----------------------------------------------------------------------------

bool Player::isValid()
{
	return true;
}

}	// namespace quicktime
}	// namespace avmedia
