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

#include "quicktimemanager.hxx"
#include "quicktimeplayer.hxx"

#include <tools/urlobj.hxx>

#define AVMEDIA_QUICKTIME_MANAGER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Manager_QuickTime"
#define AVMEDIA_QUICKTIME_MANAGER_SERVICENAME "com.sun.star.media.Manager"

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::media;
using namespace ::com::sun::star::uno;

namespace avmedia
{
namespace quicktime
{

// ============================================================================

Manager::Manager( const Reference< XMultiServiceFactory >& rxMgr ) :
	mxMgr( rxMgr )
{
}

// ----------------------------------------------------------------------------

Manager::~Manager()
{
}

// ----------------------------------------------------------------------------

Reference< XPlayer > SAL_CALL Manager::createPlayer( const ::rtl::OUString& rURL ) throw( RuntimeException )
{
	Reference< XPlayer > xRet;

	const INetURLObject aURL( rURL );
	Player *pPlayer = new Player( mxMgr );
	if ( pPlayer )
	{
		if ( pPlayer->create( aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) ) )
			xRet = Reference< XPlayer >( pPlayer );
		else
			delete pPlayer;
	}

	return xRet;
}

// ----------------------------------------------------------------------------

::rtl::OUString SAL_CALL Manager::getImplementationName() throw( RuntimeException )
{
	return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_QUICKTIME_MANAGER_IMPLEMENTATIONNAME ) );
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Manager::supportsService( const ::rtl::OUString& ServiceName ) throw( RuntimeException )
{
	return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( AVMEDIA_QUICKTIME_MANAGER_SERVICENAME ) );
}

// ----------------------------------------------------------------------------

Sequence< ::rtl::OUString > SAL_CALL Manager::getSupportedServiceNames() throw( RuntimeException )
{
	Sequence< ::rtl::OUString > aRet(1);
	aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( AVMEDIA_QUICKTIME_MANAGER_SERVICENAME ) );

	return aRet;
}

}	// namespace quicktime
}	// namespace avmedia
