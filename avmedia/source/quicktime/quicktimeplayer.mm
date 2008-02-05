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
#import "quicktimeframegrabber.hxx"
#import "quicktimeplayer.hxx"
#import "quicktimewindow.hxx"

#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif

#define AVMEDIA_QUICKTIME_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_QuickTime"
#define AVMEDIA_QUICKTIME_PLAYER_SERVICENAME "com.sun.star.media.Player_QuickTime"

// Undefine this to disable playing of video
#define ALLOW_QUICKTIME_VIDEO

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::media;
using namespace ::com::sun::star::uno;

static MacOSBOOL isRunningPanther()
{
	static bool initializedOnce = NO;
	static bool isPanther = NO;
	
	if ( ! initializedOnce )
	{
		long res = 0;
		Gestalt( gestaltSystemVersion, &res );
		isPanther = ( ( ( ( res >> 8 ) & 0x00FF ) == 0x10 ) && ( ( ( res >> 4 ) & 0x000F ) == 0x3 ) );
		initializedOnce = YES;
	}

	return isPanther;
}

namespace avmedia
{
namespace quicktime
{

// ============================================================================

Player::Player( const Reference< XMultiServiceFactory >& rxMgr ) :
	mbLooping( false ),
	mxMgr( rxMgr ),
	mpMoviePlayer( NULL )
{
}

// ----------------------------------------------------------------------------

Player::~Player()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(release:) withObject:(id)mpMoviePlayer waitUntilDone:YES modes:pModes];
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
		// The NSURL class requires URLs to be encoded
		::rtl::OUString aURL = ::rtl::Uri::encode( rURL, rtl_UriCharClassUric, rtl_UriEncodeStrict, RTL_TEXTENCODING_UTF8 );

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
				[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(release:) withObject:(id)mpMoviePlayer waitUntilDone:YES modes:pModes];
				mpMoviePlayer = NULL;
			}
		}

		if ( !bRet )
		{
			mbLooping = sal_False;
			maURL = ::rtl::OUString();

			if ( aURL.getLength() )
			{
				NSString *pString = [NSString stringWithCharacters:aURL.getStr() length:aURL.getLength()];
				if ( pString )
				{
					NSURL *pURL = [NSURL URLWithString:pString];
					if ( pURL )
					{
						// Do not retain as invoking alloc disables autorelease
						mpMoviePlayer = [[AvmediaMoviePlayer alloc] init];
						if ( mpMoviePlayer )
						{
							NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
							[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(initialize:) withObject:pURL waitUntilDone:YES modes:pModes];

							if ( ![(AvmediaMoviePlayer *)mpMoviePlayer movie] || ![(AvmediaMoviePlayer *)mpMoviePlayer movieView] )
							{
								[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(release:) withObject:(id)mpMoviePlayer waitUntilDone:YES modes:pModes];
								mpMoviePlayer = NULL;
							}
							else
							{
								maURL = aURL;
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
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(play:) withObject:(id)mpMoviePlayer waitUntilDone:YES modes:pModes];
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
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(stop:) withObject:(id)mpMoviePlayer waitUntilDone:YES modes:pModes];
	}

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
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(isPlaying:) withObject:pArgs waitUntilDone:YES modes:pModes];
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
	double fRet = 0;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(duration:) withObject:pArgs waitUntilDone:YES modes:pModes];
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
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithDouble:fTime]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setCurrentTime:) withObject:pArgs waitUntilDone:YES modes:pModes];
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
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(currentTime:) withObject:pArgs waitUntilDone:YES modes:pModes];
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
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithDouble:fTime]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setSelection:) withObject:pArgs waitUntilDone:YES modes:pModes];
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
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(selectionEnd:) withObject:pArgs waitUntilDone:YES modes:pModes];
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
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithDouble:fRate]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setRate:) withObject:pArgs waitUntilDone:YES modes:pModes];
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
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(rate:) withObject:pArgs waitUntilDone:YES modes:pModes];
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
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	mbLooping = bSet;

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObjects:[NSNumber numberWithBool:(MacOSBOOL)bSet], nil]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setLooping:) withObject:pArgs waitUntilDone:YES modes:pModes];
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
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setMute:) withObject:pArgs waitUntilDone:YES modes:pModes];
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
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(mute:) withObject:pArgs waitUntilDone:YES modes:pModes];
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
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:[NSArray arrayWithObject:[NSNumber numberWithShort:nVolumeDB]]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(setVolumeDB:) withObject:pArgs waitUntilDone:YES modes:pModes];
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
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(volumeDB:) withObject:pArgs waitUntilDone:YES modes:pModes];
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
	Size aRet( 0, 0 );

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpMoviePlayer )
	{
		AvmediaArgs *pArgs = [AvmediaArgs argsWithArgs:nil];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(AvmediaMoviePlayer *)mpMoviePlayer performSelectorOnMainThread:@selector(preferredSize:) withObject:pArgs waitUntilDone:YES modes:pModes];
		NSValue *pRet = (NSValue *)[pArgs result];
		if ( pRet )
		{
			NSSize aSize = [pRet sizeValue];
			if ( aSize.width > 0 && aSize.height > 0 )
				aRet = Size( (long)aSize.width, (long)aSize.height );
		}
	}

	[pPool release];

	return aRet;
}

// ----------------------------------------------------------------------------

Reference< XPlayerWindow > SAL_CALL Player::createPlayerWindow( const Sequence< Any >& rArguments ) throw( RuntimeException )
{
	Reference< XPlayerWindow > xRet;

#ifdef ALLOW_QUICKTIME_VIDEO
	// Don't allow running with Panther as our RAGE 128 graphics card hack in
	// the vcl module is incompatible with the JVMs on Mac OS X 10.3.9
	if ( !isRunningPanther() )
	{
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
	}
#endif	// ALLOW_QUICKTIME_VIDEO
	
	return xRet;
}

// ----------------------------------------------------------------------------

Reference< XFrameGrabber > SAL_CALL Player::createFrameGrabber() throw( RuntimeException )
{
	Reference< XFrameGrabber > xRet;

#ifdef ALLOW_QUICKTIME_VIDEO
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
#endif	// ALLOW_QUICKTIME_VIDEO

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
