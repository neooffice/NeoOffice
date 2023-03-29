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

#include "quicktimecommon.hxx"
#include "quicktimemanager.hxx"
#include "quicktimeplayer.hxx"

#include <tools/urlobj.hxx>

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

Reference< XPlayer > SAL_CALL Manager::createPlayer( const OUString& rURL ) throw( RuntimeException )
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

OUString SAL_CALL Manager::getImplementationName() throw( RuntimeException )
{
	return OUString( AVMEDIA_QUICKTIME_MANAGER_IMPLEMENTATIONNAME );
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL Manager::supportsService( const OUString& ServiceName ) throw( RuntimeException )
{
	return ServiceName == AVMEDIA_QUICKTIME_MANAGER_SERVICENAME;
}

// ----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL Manager::getSupportedServiceNames() throw( RuntimeException )
{
	Sequence< OUString > aRet(1);
	aRet[0] = OUString( AVMEDIA_QUICKTIME_MANAGER_SERVICENAME );

	return aRet;
}

}	// namespace quicktime
}	// namespace avmedia
