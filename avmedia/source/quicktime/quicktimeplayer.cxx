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

#define AVMEDIA_QUICKTIME_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_QuickTime"
#define AVMEDIA_QUICKTIME_PLAYER_SERVICENAME "com.sun.star.media.Player_QuickTime"

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::media;
using namespace ::com::sun::star::uno;

namespace avmedia
{
namespace quicktime
{

Player::Player( const Reference< XMultiServiceFactory >& rxMgr ) :
	mxMgr( rxMgr )
{
}

// ----------------------------------------------------------------------------

Player::~Player()
{
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::start() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::start not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::stop() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::stop not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaying() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::isPlaying not implemented\n" );
#endif
	return false;
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getDuration() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::getDuration not implemented\n" );
#endif
	double aRefTime( 0.0 );
	return aRefTime;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setMediaTime( double fTime ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::setMediaTime not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getMediaTime() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::getMediaTime not implemented\n" );
#endif
	double aRefTime( 0.0 );
	return aRefTime; 
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setStopTime( double fTime ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::setStopTime not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getStopTime() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::getStopTime not implemented\n" );
#endif
	double aRefTime( 0.0 );
	return aRefTime; 
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setRate( double fRate ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::setRate not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

double SAL_CALL Player::getRate() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::getRate not implemented\n" );
#endif
	double fRet( 0.0 );
	return fRet;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::setPlaybackLoop not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaybackLoop() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::isPlaybackLoop not implemented\n" );
#endif
	return false;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setMute( sal_Bool bSet ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::setMute not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isMute() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::isMute not implemented\n" );
#endif
	return false;
}

// ----------------------------------------------------------------------------

void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::setVolumeDB not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------
	
sal_Int16 SAL_CALL Player::getVolumeDB() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "Player::getVolumeDB not implemented\n" );
#endif
	return 50;
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

}	// namespace quicktime
}	// namespace avmedia
