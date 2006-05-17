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
 *  Copyright 2006 by Patrick Luby (patrick.luby@planamesa.com)
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

#include "quicktimeplayer.hxx"

#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#define QUICKTIME_STATE_UNLOADED	0x0
#define QUICKTIME_STATE_INVALID		0x1
#define QUICKTIME_STATE_VALID		0x2

#define DEF_LIB "/System/Library/Frameworks/QuickTime.framework/QuickTime"

#define AVMEDIA_QUICKTIME_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_QuickTime"
#define AVMEDIA_QUICKTIME_PLAYER_SERVICENAME "com.sun.star.media.Player_QuickTime"
#define AVMEDIA_DB_RANGE -40

typedef void DisposeMovieFunc( Movie );
typedef OSErr EnterMoviesFunc( void );
typedef void GetMovieActiveSegmentFunc( Movie, MacOSTimeValue*, MacOSTimeValue* );
typedef MacOSTimeValue GetMovieDurationFunc( Movie );
typedef Fixed GetMovieRateFunc( Movie );
typedef MacOSTimeValue GetMovieTimeFunc( Movie, TimeRecord* );
typedef short GetMovieVolumeFunc( Movie );
typedef MacOSBoolean IsMovieDoneFunc( Movie );
typedef void MoviesTaskFunc( Movie, long );
typedef OSErr NewMovieFromDataRefFunc( Movie*, short, short*, Handle, OSType );
typedef OSErr QTNewDataReferenceFromCFURLFunc( CFURLRef, UInt32, Handle*, OSType* );
typedef void SetMovieActiveSegmentFunc( Movie, MacOSTimeValue, MacOSTimeValue );
typedef void SetMovieGWorldFunc( Movie, CGrafPtr, GDHandle );
typedef void SetMovieRateFunc( Movie, Fixed );
typedef void SetMovieTimeValueFunc( Movie, MacOSTimeValue );
typedef void SetMovieVolumeFunc( Movie, short );
typedef void StartMovieFunc( Movie );
typedef void StopMovieFunc( Movie );

static ::vos::OModule aModule;
DisposeMovieFunc *pDisposeMovie = NULL;
EnterMoviesFunc *pEnterMovies = NULL;
GetMovieActiveSegmentFunc *pGetMovieActiveSegment = NULL;
GetMovieDurationFunc *pGetMovieDuration = NULL;
GetMovieRateFunc *pGetMovieRate = NULL;
GetMovieTimeFunc *pGetMovieTime = NULL;
GetMovieVolumeFunc *pGetMovieVolume = NULL;
IsMovieDoneFunc *pIsMovieDone = NULL;
MoviesTaskFunc *pMoviesTask = NULL;
NewMovieFromDataRefFunc *pNewMovieFromDataRef = NULL;
QTNewDataReferenceFromCFURLFunc *pQTNewDataReferenceFromCFURL = NULL;
SetMovieActiveSegmentFunc *pSetMovieActiveSegment = NULL;
SetMovieGWorldFunc *pSetMovieGWorld = NULL;
SetMovieRateFunc *pSetMovieRate = NULL;
SetMovieTimeValueFunc *pSetMovieTimeValue = NULL;
SetMovieVolumeFunc *pSetMovieVolume = NULL;
StartMovieFunc *pStartMovie = NULL;
StopMovieFunc *pStopMovie = NULL;

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::media;
using namespace ::com::sun::star::uno;

namespace avmedia
{
namespace quicktime
{

// ============================================================================

int Player::mnQuickTimeState = QUICKTIME_STATE_UNLOADED;

// ----------------------------------------------------------------------------

Player::Player( const Reference< XMultiServiceFactory >& rxMgr ) :
	mbLooping( false ),
	mxMgr( rxMgr ),
	maMovie( NULL ),
	mbRunning( false )
{
}

// ----------------------------------------------------------------------------

Player::~Player()
{
	stop();

	if ( maMovie && pDisposeMovie )
		pDisposeMovie( maMovie );
}

// ----------------------------------------------------------------------------

bool Player::create( const ::rtl::OUString& rURL )
{
	bool bRet = false;

	if ( !isValid() )
		return bRet;

	stop();

	if ( maMovie )
	{
		if ( pDisposeMovie )
			pDisposeMovie( maMovie );
		maMovie = NULL;
	}

	mbLooping = sal_False;
	mbRunning = sal_False;

	::rtl::OString aURLBytes( rURL.getStr(), rURL.getLength(), RTL_TEXTENCODING_UTF8 );
	CFURLRef aURL = CFURLCreateWithBytes( NULL, (const UInt8 *)aURLBytes.getStr(), aURLBytes.getLength(), kCFStringEncodingUTF8, NULL );
	if ( aURL )
	{
		Handle hHandle = NULL;
		OSType nType;
		if ( pQTNewDataReferenceFromCFURL && pQTNewDataReferenceFromCFURL( aURL, 0, &hHandle, &nType ) == noErr )
		{
			short nID = movieInDataForkResID;
			if ( pEnterMovies && pMoviesTask && pNewMovieFromDataRef && pSetMovieActiveSegment && pSetMovieGWorld && pEnterMovies() == noErr && pNewMovieFromDataRef( &maMovie, 0, &nID, hHandle, nType ) == noErr && maMovie )
			{
				pSetMovieGWorld( maMovie, NULL, NULL );
				pSetMovieActiveSegment( maMovie, -1, 0 );
				pMoviesTask( maMovie, 0 );
				bRet = true;
			}

			DisposeHandle( hHandle );
		}

		CFRelease( aURL );
	}

	return bRet;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::start() throw( RuntimeException )
{
	stop();

	if ( maMovie && pStartMovie )
	{
		pStartMovie( maMovie );
		mbRunning = sal_True;
	}
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::stop() throw( RuntimeException )
{
	if ( maMovie && pStopMovie )
		pStopMovie( maMovie );

	mbRunning = sal_False;
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaying() throw( RuntimeException )
{
	if ( mbRunning && maMovie && pIsMovieDone && pIsMovieDone( maMovie ) )
	{
		stop();
		if ( mbLooping && pSetMovieTimeValue )
		{
			pSetMovieTimeValue( maMovie, 0 );
			start();
		}
	}

	return mbRunning;
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getDuration() throw( RuntimeException )
{
	double aRefTime = 0.0;

	if ( maMovie && pGetMovieDuration )
		aRefTime = (double)pGetMovieDuration( maMovie ) / 1000;

	return aRefTime;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setMediaTime( double fTime ) throw( RuntimeException )
{
	if ( maMovie && pSetMovieTimeValue )
	{
		sal_Bool bPlaying = isPlaying();
		if ( bPlaying )
			stop();
		pSetMovieTimeValue( maMovie, (MacOSTimeValue)( fTime * 1000 ) );
		if ( bPlaying )
			start();
	}
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getMediaTime() throw( RuntimeException )
{
	double aRefTime = 0.0;

	if ( maMovie && pGetMovieTime )
		aRefTime = (double)pGetMovieTime( maMovie, NULL ) / 1000;

	return aRefTime; 
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setStopTime( double fTime ) throw( RuntimeException )
{
	if ( maMovie && pSetMovieActiveSegment )
		pSetMovieActiveSegment( maMovie, -1, (MacOSTimeValue)( fTime * 1000 ) );
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getStopTime() throw( RuntimeException )
{
	double aRefTime = 0.0;

	if ( maMovie && pGetMovieActiveSegment )
	{
		MacOSTimeValue nStartTime;
		MacOSTimeValue nDuration;
		pGetMovieActiveSegment( maMovie, &nStartTime, &nDuration );
		if ( nDuration == -1 )
			aRefTime = getDuration();
		else
			aRefTime = (double)nDuration / 1000;
	}

	return aRefTime; 
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setRate( double fRate ) throw( RuntimeException )
{
	if ( maMovie && pSetMovieRate )
		pSetMovieRate( maMovie, X2Fix( (float)fRate ) );
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getRate() throw( RuntimeException )
{
	double fRet = 0.0;

	if ( maMovie && pGetMovieRate )
		fRet = (double)Fix2X( pGetMovieRate( maMovie ) );

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
	if ( maMovie && pGetMovieVolume && pSetMovieVolume )
	{
		short nVolume = pGetMovieVolume( maMovie );
		if ( bSet && nVolume > kNoVolume )
			pSetMovieVolume( maMovie, nVolume * -1 );
		else if ( !bSet && nVolume < kNoVolume )
			pSetMovieVolume( maMovie, nVolume * -1 );
	}
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isMute() throw( RuntimeException )
{
	if ( maMovie && pGetMovieVolume && pGetMovieVolume( maMovie ) < kNoVolume )
		return true;
	else
		return false;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB ) throw( RuntimeException )
{
	if ( maMovie && pSetMovieVolume )
		pSetMovieVolume( maMovie, ( ( AVMEDIA_DB_RANGE - nVolumeDB ) * kFullVolume ) / AVMEDIA_DB_RANGE );
}

// ----------------------------------------------------------------------------
	
sal_Int16 SAL_CALL Player::getVolumeDB() throw( RuntimeException )
{
	sal_Int16 nRet = 0;

	if ( maMovie && pGetMovieVolume )
		nRet = AVMEDIA_DB_RANGE - ( ( pGetMovieVolume( maMovie ) * AVMEDIA_DB_RANGE ) / kFullVolume );

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
	if ( mnQuickTimeState == QUICKTIME_STATE_UNLOADED )
	{
		mnQuickTimeState = QUICKTIME_STATE_INVALID;
		if ( aModule.load( ::rtl::OUString::createFromAscii( DEF_LIB ) ) )
		{
			pDisposeMovie = (DisposeMovieFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "DisposeMovie" ) );
			pEnterMovies = (EnterMoviesFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "EnterMovies" ) );
			pGetMovieActiveSegment = (GetMovieActiveSegmentFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "GetMovieActiveSegment" ) );
			pGetMovieDuration = (GetMovieDurationFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "GetMovieDuration" ) );
			pGetMovieRate = (GetMovieRateFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "GetMovieRate" ) );
			pGetMovieTime = (GetMovieTimeFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "GetMovieTime" ) );
			pGetMovieVolume = (GetMovieVolumeFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "GetMovieVolume" ) );
			pIsMovieDone = (IsMovieDoneFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "IsMovieDone" ) );
			pMoviesTask = (MoviesTaskFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "MoviesTask" ) );
			pNewMovieFromDataRef = (NewMovieFromDataRefFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "NewMovieFromDataRef" ) );
			pQTNewDataReferenceFromCFURL = (QTNewDataReferenceFromCFURLFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "QTNewDataReferenceFromCFURL" ) );
			pSetMovieGWorld = (SetMovieGWorldFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "SetMovieGWorld" ) );
			pSetMovieActiveSegment = (SetMovieActiveSegmentFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "SetMovieActiveSegment" ) );
			pSetMovieRate = (SetMovieRateFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "SetMovieRate" ) );
			pSetMovieTimeValue = (SetMovieTimeValueFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "SetMovieTimeValue" ) );
			pSetMovieVolume = (SetMovieVolumeFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "SetMovieVolume" ) );
			pStartMovie = (StartMovieFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "StartMovie" ) );
			pStopMovie = (StopMovieFunc *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "StopMovie" ) );
			if ( pDisposeMovie && pEnterMovies && pGetMovieActiveSegment && pGetMovieDuration && pGetMovieRate && pGetMovieTime && pGetMovieVolume && pIsMovieDone && pMoviesTask && pNewMovieFromDataRef && pQTNewDataReferenceFromCFURL && pSetMovieActiveSegment && pSetMovieGWorld && pSetMovieRate && pSetMovieTimeValue && pSetMovieVolume && pStartMovie && pStopMovie )
				mnQuickTimeState = QUICKTIME_STATE_VALID;
		}
	}

	return ( mnQuickTimeState == QUICKTIME_STATE_VALID ? true : false );
}

}	// namespace quicktime
}	// namespace avmedia
