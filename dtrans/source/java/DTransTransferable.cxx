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

typedef OSStatus GetCurrentScrap_Type( ScrapRef * );
typedef OSStatus GetScrapFlavorData_Type( ScrapRef, ScrapFlavorType, MacOSSize *, void * );
typedef OSStatus GetScrapFlavorSize_Type( ScrapRef, ScrapFlavorType, MacOSSize * );

using namespace rtl;
using namespace vos;

#endif	// MACOSX

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace java::dtrans;

// ============================================================================

jclass com_sun_star_dtrans_DTransTransferable::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_dtrans_DTransTransferable::getMyClass()
{
#ifndef MACOSX
	if ( !theClass )
	{
		DTransThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "com/sun/star/dtrans/DTransTransferable" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
#endif	// !MACOSX
	return theClass;
}

// ----------------------------------------------------------------------------

Any SAL_CALL com_sun_star_dtrans_DTransTransferable::getTransferData( const DataFlavor& aFlavor ) throw ( UnsupportedFlavorException, IOException, RuntimeException )
{
	Any out;

#ifdef MACOSX
	// Test the JVM version and if it is below 1.4, use Carbon APIs or else
	// use Cocoa APIs
	java_lang_Class* pClass = java_lang_Class::forName( OUString::createFromAscii( "java/lang/CharSequence" ) );
	if ( !pClass )
	{
		// Load Carbon
		OModule aModule;
		if ( aModule.load( OUString::createFromAscii( "/System/Library/Frameworks/Carbon.framework/Carbon" ) ) )
		{
			GetScrapFlavorData_Type *pGetScrapFlavorData = (GetScrapFlavorData_Type *)aModule.getSymbol( OUString::createFromAscii( "GetScrapFlavorData" ) );
			GetScrapFlavorSize_Type *pGetScrapFlavorSize = (GetScrapFlavorSize_Type *)aModule.getSymbol( OUString::createFromAscii( "GetScrapFlavorSize" ) );
			if ( pGetScrapFlavorData && pGetScrapFlavorSize )
			{
				ScrapFlavorType aType;
				MacOSSize aSize;

				OSStatus nErr = pGetScrapFlavorSize( *(ScrapRef *)mpNativeObj, aType, &aSize );
				if ( nErr == noTypeErr )
					throw UnsupportedFlavorException( aFlavor.MimeType, static_cast< XTransferable * >( this ) );
				else if ( nErr != noErr )
					throw IOException( aFlavor.MimeType, static_cast< XTransferable * >( this ) );

				if ( pGetScrapFlavorData( *(ScrapRef *)mpNativeObj, aType, &aSize, NULL ) != nErr )
					throw IOException( aFlavor.MimeType, static_cast< XTransferable * >( this ) );
			}

			aModule.unload();
		}
	}
	else
	{
		delete pClass;
#ifdef DEBUG
		fprintf( stderr, "DTransTransferable::getTransferData not implemented\n" );
#endif
	}
#else // MACOSX
#ifdef DEBUG
	fprintf( stderr, "DTransTransferable::getTransferData not implemented\n" );
#endif
#endif	// MACOSX

	return out;
}

// ----------------------------------------------------------------------------

Sequence< DataFlavor > SAL_CALL com_sun_star_dtrans_DTransTransferable::getTransferDataFlavors() throw ( RuntimeException )
{
	Sequence< DataFlavor > out;
	return out;
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL com_sun_star_dtrans_DTransTransferable::isDataFlavorSupported( const DataFlavor& aFlavor ) throw ( RuntimeException )
{
	sal_Bool out = FALSE;
	return out;
}
