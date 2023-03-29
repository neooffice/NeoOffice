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

#include "quicktimecommon.h"
#include "quicktimecommon.hxx"
#include "quicktimeframegrabber.hxx"
#include "quicktimeplayer.hxx"
#include "quicktimewindow.hxx"

#include <osl/objcutils.h>
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
	mpMoviePlayer( NULL ),
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
		osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(destroy:), (AvmediaMoviePlayer *)mpMoviePlayer, YES );
		[(AvmediaMoviePlayer *)mpMoviePlayer release];
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
				osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(destroy:), (AvmediaMoviePlayer *)mpMoviePlayer, YES );
				[(AvmediaMoviePlayer *)mpMoviePlayer release];
				mpMoviePlayer = NULL;
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
				NSString *pString = [NSString stringWithCharacters:aURL.getStr() length:aURL.getLength()];
				if ( pString )
				{
					NSURL *pURL = [NSURL URLWithString:pString];
					if ( pURL )
					{
						// Do not retain as invoking alloc disables autorelease
						mpMoviePlayer = [[AvmediaMoviePlayer alloc] initWithURL:pURL];
						if ( mpMoviePlayer )
						{
							osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(initialize:), (AvmediaMoviePlayer *)mpMoviePlayer, YES );
							if ( ![(AvmediaMoviePlayer *)mpMoviePlayer movie] || ![(AvmediaMoviePlayer *)mpMoviePlayer movieView] )
							{
								osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(destroy:), (AvmediaMoviePlayer *)mpMoviePlayer, YES );
								[(AvmediaMoviePlayer *)mpMoviePlayer release];
								mpMoviePlayer = NULL;
							}
							else
							{
								// Cache duration
								AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
								osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(duration:), pArgs, YES );
								NSNumber *pDuration = (NSNumber *)[pArgs result];
								if ( pDuration )
									mfDuration = [pDuration doubleValue];

								// Cache preferred size
								pArgs = [AvmediaArgs argsWithArgs:nil];
								osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(preferredSize:), pArgs, YES );
								NSValue *pPreferredSize = (NSValue *)[pArgs result];
								if ( pPreferredSize )
								{
									NSSize aSize = [pPreferredSize sizeValue];
									if ( aSize.width > 0 && aSize.height > 0 )
										maPreferredSize = Size( (long)aSize.width, (long)aSize.height );
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

void SAL_CALL Player::start() throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
		osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(play:), (AvmediaMoviePlayer *)mpMoviePlayer, YES );

	[pPool release];
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::stop() throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
		osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(stop:), (AvmediaMoviePlayer *)mpMoviePlayer, YES );

	[pPool release];
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaying() throw( RuntimeException )
{
	sal_Bool bRet = sal_False;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
		osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(isPlaying:), pArgs, YES );
		NSNumber *pRet = (NSNumber *)[pArgs result];
		if ( pRet )
			bRet = (sal_Bool)[pRet boolValue];
	}

	[pPool release];

	return bRet;
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getDuration() throw( RuntimeException )
{
	return mfDuration;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setMediaTime( double fTime ) throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		if ( fTime < 0 )
			fTime = 0;
		else if ( fTime > mfDuration )
			fTime = mfDuration;

		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithDouble:fTime]]];
		osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(setCurrentTime:), pArgs, YES );
		mfMediaTime = fTime;
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getMediaTime() throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
		osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(currentTime:), pArgs, YES );
		NSNumber *pRet = (NSNumber *)[pArgs result];
		if ( pRet )
			mfMediaTime = [pRet doubleValue];
	}

	[pPool release];

	return mfMediaTime;
}

#ifdef USE_OLD_XPLAYER_INTERFACE

// ----------------------------------------------------------------------------

void SAL_CALL Player::setStopTime( double fTime ) throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		if ( fTime < 0 )
			fTime = 0;
		else if ( fTime > mfDuration )
			fTime = mfDuration;

		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithDouble:fTime]]];
		osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(setSelection:), pArgs, YES );
		mfStopTime = fTime;
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getStopTime() throw( RuntimeException )
{
	return mfStopTime;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setRate( double fRate ) throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithDouble:fRate]]];
		osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(setRate:), pArgs, YES );
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getRate() throw( RuntimeException )
{
	double fRet = 1.0f;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
		osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(rate:), pArgs, YES );
		NSNumber *pRet = (NSNumber *)[pArgs result];
		if ( pRet )
			fRet = [pRet doubleValue];
	}

	[pPool release];

	return fRet;
}

#endif	// USE_OLD_XPLAYER_INTERFACE

// ----------------------------------------------------------------------------

void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet ) throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	mbLooping = bSet;

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithBool:(BOOL)bSet], nil]];
		osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(setLooping:), pArgs, YES );
	}

	[pPool release];

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
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithBool:( bSet ? YES : NO )]]];
		osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(setMute:), pArgs, YES );
		mbMute = bSet;
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isMute() throw( RuntimeException )
{
	return mbMute;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB ) throw( RuntimeException )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithShort:nVolumeDB]]];
		osl_performSelectorOnMainThread( (AvmediaMoviePlayer *)mpMoviePlayer, @selector(setVolumeDB:), pArgs, YES );
		mnVolumeDB = nVolumeDB;
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

sal_Int16 SAL_CALL Player::getVolumeDB() throw( RuntimeException )
{
	return mnVolumeDB;
}

// ----------------------------------------------------------------------------

Size SAL_CALL Player::getPreferredPlayerWindowSize() throw( RuntimeException )
{
	return maPreferredSize;
}

// ----------------------------------------------------------------------------

Reference< XPlayerWindow > SAL_CALL Player::createPlayerWindow( const Sequence< Any >& rArguments ) throw( RuntimeException )
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

Reference< XFrameGrabber > SAL_CALL Player::createFrameGrabber() throw( RuntimeException )
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

OUString SAL_CALL Player::getImplementationName() throw( RuntimeException )
{
	return OUString( AVMEDIA_QUICKTIME_PLAYER_IMPLEMENTATIONNAME );
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::supportsService( const OUString& ServiceName ) throw( RuntimeException )
{
	return ServiceName == AVMEDIA_QUICKTIME_PLAYER_SERVICENAME;
}

// ----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL Player::getSupportedServiceNames() throw( RuntimeException )
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
