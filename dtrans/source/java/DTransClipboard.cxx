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

#define _JAVA_DTRANS_COM_SUN_STAR_DTRANS_DTRANSCLIPBOARD_CXX

#ifndef _JAVA_DTRANS_COM_SUN_STAR_DTRANS_DTRANSCLIPBOARD_HXX
#include <com/sun/star/dtrans/DTransClipboard.hxx>
#endif
#ifndef _JAVA_DTRANS_COM_SUN_STAR_DTRANS_DTRANSTRANSFERABLE_HXX
#include <com/sun/star/dtrans/DTransTransferable.hxx>
#endif

#ifdef MACOSX

#ifndef _JAVA_DTRANS_JAVA_LANG_CLASS_HXX
#include <java/lang/Class.hxx>
#endif
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif
#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

using namespace rtl;
using namespace vos;

#endif	// MACOSX

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::uno;
using namespace java::dtrans;

// ============================================================================

jclass com_sun_star_dtrans_DTransClipboard::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_dtrans_DTransClipboard::getMyClass()
{
#ifndef MACOSX
	if ( !theClass )
	{
		DTransThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "com/sun/star/dtrans/DTransClipboard" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
#endif	// !MACOSX
	return theClass;
}

// ----------------------------------------------------------------------------

com_sun_star_dtrans_DTransTransferable *com_sun_star_dtrans_DTransClipboard::getContents()
{
	com_sun_star_dtrans_DTransTransferable *out = NULL;

#ifdef MACOSX
	ScrapRef aScrap;
	if ( GetCurrentScrap( &aScrap ) == noErr )
		out = new com_sun_star_dtrans_DTransTransferable( aScrap );
#else	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "DTransClipboard::getContents not implemented\n" );
#endif
#endif	// MACOSX

	return out;
}

// ----------------------------------------------------------------------------

com_sun_star_dtrans_DTransTransferable *com_sun_star_dtrans_DTransClipboard::setContents( const Reference< XTransferable > &xTransferable )
{
	com_sun_star_dtrans_DTransTransferable *out = new com_sun_star_dtrans_DTransTransferable();

	if ( !xTransferable.is() || !out->setContents( xTransferable ) )
	{
		delete out;
		out = NULL;
	}

	return out;
}
