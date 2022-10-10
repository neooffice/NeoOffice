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

#ifndef _PLAYER_HXX
#define _PLAYER_HXX

#include <cppuhelper/compbase2.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/media/XPlayer.hpp>

namespace avmedia
{
namespace quicktime
{

// ----------
// - Player -
// ----------

class Player : public ::cppu::WeakImplHelper2< ::com::sun::star::media::XPlayer, ::com::sun::star::lang::XServiceInfo >
{
	double				mfDuration;
	sal_Bool			mbLooping;
	double				mfMediaTime;
	::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >	mxMgr;
	void*				mpMoviePlayer;
	sal_Bool			mbMute;
	::com::sun::star::awt::Size	maPreferredSize;
	double				mfStopTime;
	OUString			maURL;
	sal_Int16			mnVolumeDB;

public:
						Player( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMgr );
						~Player();

	bool				create( const OUString& rURL );

    // XPlayer
    virtual void SAL_CALL start(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL stop(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isPlaying(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual double SAL_CALL getDuration(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL setMediaTime( double fTime ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual double SAL_CALL getMediaTime(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL setPlaybackLoop( sal_Bool bSet ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isPlaybackLoop(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL setMute( sal_Bool bSet ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isMute(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual void SAL_CALL setVolumeDB( sal_Int16 nVolumeDB ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getVolumeDB(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Size SAL_CALL getPreferredPlayerWindowSize(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayerWindow > SAL_CALL createPlayerWindow( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::media::XFrameGrabber > SAL_CALL createFrameGrabber(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

	bool					isValid();
};

} // namespace quicktime
} // namespace avmedia

#endif // _PLAYER_HXX
