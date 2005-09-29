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
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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

#define _DTRANSCLIPBOARD_CXX

#ifndef _DTRANSCLIPBOARD_HXX
#include "DTransClipboard.hxx"
#endif
#ifndef _DTRANSTRANSFERABLE_HXX
#include "DTransTransferable.hxx"
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::uno;
using namespace java;
using namespace rtl;

// ============================================================================

DTransTransferable *DTransClipboard::getContents()
{
	DTransTransferable *out = NULL;

#ifdef MACOSX
	ScrapRef aScrap;
	if ( GetCurrentScrap( &aScrap ) == (OSStatus)noErr )
		out = new DTransTransferable( aScrap, TRANSFERABLE_TYPE_CLIPBOARD );
#else	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "DTransClipboard::getContents not implemented\n" );
#endif
#endif	// MACOSX

	return out;
}

// ----------------------------------------------------------------------------

DTransTransferable *DTransClipboard::setContents( const Reference< XTransferable > &xTransferable )
{
	DTransTransferable *out = new DTransTransferable( NULL, TRANSFERABLE_TYPE_CLIPBOARD );

	if ( !xTransferable.is() || !out->setContents( xTransferable ) )
	{
		delete out;
		out = NULL;
	}

	return out;
}
