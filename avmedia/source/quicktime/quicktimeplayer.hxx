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

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_MEDIA_XPLAYER_HPP_
#include <com/sun/star/media/XPlayer.hpp>
#endif

namespace avmedia
{
namespace quicktime
{

// ----------
// - Player -
// ----------

class Player : public ::cppu::WeakImplHelper2< ::com::sun::star::media::XPlayer, ::com::sun::star::lang::XServiceInfo >
{
	sal_Bool			mbLooping;
	::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >	mxMgr;
	void*				mpMoviePlayer;
	::rtl::OUString		maURL;

public:
						Player( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMgr );
						~Player();

	bool				create( const ::rtl::OUString& rURL );

	// XPlayer
	virtual void SAL_CALL	start() throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	stop() throw( ::com::sun::star::uno::RuntimeException );
	virtual sal_Bool SAL_CALL	isPlaying() throw( ::com::sun::star::uno::RuntimeException );
	virtual double SAL_CALL	getDuration() throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	setMediaTime( double fTime ) throw( ::com::sun::star::uno::RuntimeException );
	virtual double SAL_CALL	getMediaTime() throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	setStopTime( double fTime ) throw( ::com::sun::star::uno::RuntimeException );
	virtual double SAL_CALL	getStopTime() throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	setRate( double fRate ) throw( ::com::sun::star::uno::RuntimeException );
	virtual double SAL_CALL	getRate() throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	setPlaybackLoop( sal_Bool bSet ) throw( ::com::sun::star::uno::RuntimeException );
	virtual sal_Bool SAL_CALL	isPlaybackLoop() throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	setMute( sal_Bool bSet ) throw( ::com::sun::star::uno::RuntimeException );
	virtual sal_Bool SAL_CALL	isMute() throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL	setVolumeDB( sal_Int16 nVolumeDB ) throw( ::com::sun::star::uno::RuntimeException );
	virtual sal_Int16 SAL_CALL	getVolumeDB() throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::awt::Size SAL_CALL	getPreferredPlayerWindowSize() throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayerWindow > SAL_CALL	createPlayerWindow( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rArguments ) throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::media::XFrameGrabber > SAL_CALL	createFrameGrabber() throw( ::com::sun::star::uno::RuntimeException );

	// XServiceInfo
	virtual ::rtl::OUString SAL_CALL	getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
	virtual sal_Bool SAL_CALL	supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL	getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

	bool					isValid();
};

} // namespace quicktime
} // namespace avmedia

#endif // _PLAYER_HXX
