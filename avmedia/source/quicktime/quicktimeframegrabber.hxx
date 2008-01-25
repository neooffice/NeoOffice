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
 *		 - GNU General Public License Version 2.1
 *
 *  Patrick Luby, January 2008
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2008 Planamesa Inc.
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

#ifndef _FRAMEGRABBER_HXX
#define _FRAMEGRABBER_HXX

#include <list>

#include "quicktimeplayer.hxx"

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_MEDIA_XFRAMEGRABBER_HDL_
#include <com/sun/star/media/XFrameGrabber.hpp>
#endif

namespace avmedia
{
namespace quicktime
{

// ----------------
// - FrameGrabber -
// ----------------

class FrameGrabber : public ::cppu::WeakImplHelper2 < ::com::sun::star::media::XFrameGrabber, ::com::sun::star::lang::XServiceInfo >
{
	::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >	mxMgr;
	void*				mpMoviePlayer;

public:
						FrameGrabber( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMgr );
						~FrameGrabber();

	// XFrameGrabber
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL	grabFrame( double fMediaTime ) throw( ::com::sun::star::uno::RuntimeException );

	// XServiceInfo
	virtual ::rtl::OUString SAL_CALL	getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
	virtual sal_Bool SAL_CALL	supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL	getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

	bool					create( void *pMoviePlayerView );
};

} // namespace quicktime
} // namespace avmedia

#endif // _FRAMEGRABBER_HXX
