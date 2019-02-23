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

#include "quicktimecommon.h"
#include "quicktimecommon.hxx"
#include "quicktimeframegrabber.hxx"
#include "quicktimeplayer.hxx"
#include "quicktimewindow.hxx"

#include <rtl/uri.hxx>

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::media;
using namespace ::com::sun::star::uno;

namespace avmedia
{
namespace quicktime
{

// ============================================================================

Player::Player( const Reference< XMultiServiceFactory >& rxMgr ) :
	mfDuration( 0 ),
	mbLooping( sal_False ),
	mfMediaTime( 0 ),
	mxMgr( rxMgr ),
	mpMoviePlayer( nullptr ),
	mbMute( sal_False ),
	mfStopTime( 0 ),
	mnVolumeDB( 0 )
{
}

// ----------------------------------------------------------------------------

Player::~Player()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(destroy:) withObject:static_cast< id >( mpMoviePlayer ) waitUntilDone:YES modes:pModes];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) release];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

bool Player::create( const OUString& rURL )
{
	bool bRet = false;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( isValid() )
	{
		// The NSURL class requires URLs to be encoded
		OUString aURL = ::rtl::Uri::encode( rURL, rtl_UriCharClassUric, rtl_UriEncodeStrict, RTL_TEXTENCODING_UTF8 );

		if ( mpMoviePlayer )
		{
			if ( aURL.getLength() && aURL == maURL )
			{
				stop();
				setPlaybackLoop( sal_False );
				setMediaTime( 0 );
				bRet = true;
			}
			else
			{
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(destroy:) withObject:static_cast< id >( mpMoviePlayer ) waitUntilDone:YES modes:pModes];
				[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) release];
				mpMoviePlayer = nullptr;
			}
		}

		if ( !bRet )
		{
			mfDuration = 0;
			mbLooping = sal_False;
			mfMediaTime = 0;
			mbMute = sal_False;
			maPreferredSize = Size( 0, 0 );
			mfStopTime = 0;
			maURL = OUString();
			mnVolumeDB = 0;

			if ( aURL.getLength() )
			{
				NSString *pString = [NSString stringWithCharacters:reinterpret_cast< const unichar* >( aURL.getStr() ) length:aURL.getLength()];
				if ( pString )
				{
					NSURL *pURL = [NSURL URLWithString:pString];
					if ( pURL )
					{
						// Do not retain as invoking alloc disables autorelease
						mpMoviePlayer = [[AvmediaMoviePlayer alloc] initWithURL:pURL];
						if ( mpMoviePlayer )
						{
							NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
							[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(initialize:) withObject:static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) waitUntilDone:YES modes:pModes];

							if ( ![static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) movie] || ![static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) movieView] )
							{
								[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(destroy:) withObject:static_cast< id >( mpMoviePlayer ) waitUntilDone:YES modes:pModes];
								[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) release];
								mpMoviePlayer = nullptr;
							}
							else
							{
								// Cache duration
								AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
								[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(duration:) withObject:pArgs waitUntilDone:YES modes:pModes];
								NSNumber *pDuration = static_cast< NSNumber* >( [pArgs result] );
								if ( pDuration )
									mfDuration = [pDuration doubleValue];

								// Cache preferred size
								pArgs = [AvmediaArgs argsWithArgs:nil];
								[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(preferredSize:) withObject:pArgs waitUntilDone:YES modes:pModes];
								NSValue *pPreferredSize = static_cast< NSValue* >( [pArgs result] );
								if ( pPreferredSize )
								{
									NSSize aSize = [pPreferredSize sizeValue];
									if ( aSize.width > 0 && aSize.height > 0 )
										maPreferredSize = Size( static_cast< long >( aSize.width ), static_cast< long >( aSize.height ) );
								}

								maURL = aURL;

								// Push default settings to movie
								setPlaybackLoop( mbLooping );
								setMediaTime( mfMediaTime );
								setMute( mbMute );
#ifdef USE_OLD_XPLAYER_INTERFACE
								setStopTime( mfDuration );
#endif	 // USE_OLD_XPLAYER_INTERFACE
								setVolumeDB( mnVolumeDB );

								bRet = true;
							}
						}
					}
				}
			}
		}
	}

	[pPool release];

	return bRet;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::start()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(play:) withObject:static_cast< id >( mpMoviePlayer ) waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::stop()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(stop:) withObject:static_cast< id >( mpMoviePlayer ) waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaying()
{
	sal_Bool bRet = sal_False;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(isPlaying:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSNumber *pRet = static_cast< NSNumber* >( [pArgs result] );
		if ( pRet )
			bRet = static_cast< sal_Bool >( [pRet boolValue] );
	}

	[pPool release];

	return bRet;
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getDuration()
{
	return mfDuration;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setMediaTime( double fTime )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		if ( fTime < 0 )
			fTime = 0;
		else if ( fTime > mfDuration )
			fTime = mfDuration;

		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithDouble:fTime]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(setCurrentTime:) withObject:pArgs waitUntilDone:YES modes:pModes];
		mfMediaTime = fTime;
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getMediaTime()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(currentTime:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSNumber *pRet = static_cast< NSNumber* >( [pArgs result] );
		if ( pRet )
			mfMediaTime = [pRet doubleValue];
	}

	[pPool release];

	return mfMediaTime;
}

#ifdef USE_OLD_XPLAYER_INTERFACE

// ----------------------------------------------------------------------------

void SAL_CALL Player::setStopTime( double fTime )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		if ( fTime < 0 )
			fTime = 0;
		else if ( fTime > mfDuration )
			fTime = mfDuration;

		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithDouble:fTime]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(setSelection:) withObject:pArgs waitUntilDone:YES modes:pModes];
		mfStopTime = fTime;
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getStopTime()
{
	return mfStopTime;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setRate( double fRate )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithDouble:fRate]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(setRate:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getRate()
{
	double fRet = 1.0f;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(rate:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSNumber *pRet = static_cast< NSNumber* >( [pArgs result] );
		if ( pRet )
			fRet = [pRet doubleValue];
	}

	[pPool release];

	return fRet;
}

#endif	// USE_OLD_XPLAYER_INTERFACE

// ----------------------------------------------------------------------------

void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	mbLooping = bSet;

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithBool:static_cast< BOOL >( bSet )], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(setLooping:) withObject:pArgs waitUntilDone:YES modes:pModes];
	}

	[pPool release];

}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaybackLoop()
{
	return mbLooping;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setMute( sal_Bool bSet )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithBool:( bSet ? YES : NO )]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(setMute:) withObject:pArgs waitUntilDone:YES modes:pModes];
		mbMute = bSet;
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isMute()
{
	return mbMute;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithShort:nVolumeDB]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[static_cast< AvmediaMoviePlayer* >( mpMoviePlayer ) performSelectorOnMainThread:@selector(setVolumeDB:) withObject:pArgs waitUntilDone:YES modes:pModes];
		mnVolumeDB = nVolumeDB;
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

sal_Int16 SAL_CALL Player::getVolumeDB()
{
	return mnVolumeDB;
}

// ----------------------------------------------------------------------------

Size SAL_CALL Player::getPreferredPlayerWindowSize()
{
	return maPreferredSize;
}

// ----------------------------------------------------------------------------

Reference< XPlayerWindow > SAL_CALL Player::createPlayerWindow( const Sequence< Any >& rArguments )
{
	Reference< XPlayerWindow > xRet;

	Size aSize = getPreferredPlayerWindowSize();
	if ( aSize.Width > 0 && aSize.Height > 0 )
	{
		Window *pWindow = new Window( mxMgr );
		if ( pWindow )
		{
			xRet = pWindow;
			if ( !pWindow->create( mpMoviePlayer, rArguments ) )
				xRet.clear();
		}
	}

	return xRet;
}

// ----------------------------------------------------------------------------

Reference< XFrameGrabber > SAL_CALL Player::createFrameGrabber()
{
	Reference< XFrameGrabber > xRet;

	if ( maURL.getLength() )
	{
		FrameGrabber *pFrameGrabber = new FrameGrabber( mxMgr );
		if ( pFrameGrabber )
		{
			xRet = pFrameGrabber;
			if ( !pFrameGrabber->create( mpMoviePlayer ) )
				xRet.clear();
		}
	}

	return xRet;
}

// ----------------------------------------------------------------------------

OUString SAL_CALL Player::getImplementationName()
{
	return OUString( AVMEDIA_QUICKTIME_PLAYER_IMPLEMENTATIONNAME );
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::supportsService( const OUString& ServiceName )
{
	return ServiceName == AVMEDIA_QUICKTIME_PLAYER_SERVICENAME;
}

// ----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL Player::getSupportedServiceNames()
{
	Sequence< OUString > aRet(1);
	aRet[0] = OUString( AVMEDIA_QUICKTIME_PLAYER_SERVICENAME );

	return aRet;
}

// ----------------------------------------------------------------------------

bool Player::isValid()
{
	return true;
}

}	// namespace quicktime
}	// namespace avmedia
