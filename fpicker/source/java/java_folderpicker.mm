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
 *  Patrick Luby, July 2006
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

#include <stdio.h>

#include "java_folderpicker.hxx"
#include "../aqua/FPServiceInfo.hxx"
#include <com/sun/star/lang/NullPointerException.hpp>

#include "cocoa_dialog.h"
#include "../aqua/CFStringUtilities.hxx"

using namespace cppu;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::ui::dialogs;
using namespace com::sun::star::util;
using namespace java;

// ========================================================================

static Sequence< OUString > SAL_CALL JavaFolderPicker_getSupportedServiceNames()
{
	Sequence< OUString > aRet( 3 );
	aRet[0] = "com.sun.star.ui.dialogs.FolderPicker";
	aRet[1] = "com.sun.star.ui.dialogs.SystemFolderPicker";
	aRet[2] = FOLDER_PICKER_SERVICE_NAME;
	return aRet;
}

// ========================================================================

JavaFolderPicker::JavaFolderPicker( const Reference< XMultiServiceFactory >& /* xServiceMgr */ ) : WeakImplHelper3< XFolderPicker2, XServiceInfo, XEventListener >()
{
	mpDialog = NSFileDialog_create( nullptr, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE );
	if ( !mpDialog )
		throw NullPointerException();
}

// ------------------------------------------------------------------------

JavaFolderPicker::~JavaFolderPicker()
{
	if ( mpDialog )
		NSFileDialog_release( mpDialog );
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::setTitle( const OUString& aTitle )
{
	CFStringRef aString = CFStringCreateWithCharacters( nullptr, reinterpret_cast< const UniChar* >( aTitle.getStr() ), aTitle.getLength() );
	if ( aString )
	{
		NSFileDialog_setTitle( mpDialog, aString );
		CFRelease( aString );
	}
}

// ------------------------------------------------------------------------

sal_Int16 SAL_CALL JavaFolderPicker::execute()
{
	return NSFileDialog_showFileDialog( mpDialog );
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::setDisplayDirectory( const OUString& aDirectory )
{
	if ( aDirectory.getLength() )
	{
		CFStringRef aString = CFStringCreateWithCharacters( nullptr, reinterpret_cast< const UniChar* >( aDirectory.getStr() ), aDirectory.getLength() );
		if ( aString )
		{
			NSFileDialog_setDirectory( mpDialog, aString );
			CFRelease( aString );
		}
	}
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getDisplayDirectory()
{
	OUString aRet;

	CFStringRef aString = NSFileDialog_directory( mpDialog );
	if ( aString )
	{
		aRet = CFStringToOUString( aString );
		CFRelease( aString );
	}

	return aRet;
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getDirectory()
{
	OUString aRet;

	CFStringRef *pURLs = NSFileDialog_URLs( mpDialog );
	if ( pURLs )
	{
		if ( pURLs[ 0 ] )
			aRet = CFStringToOUString( pURLs[ 0 ] );

		NSFileManager_releaseURLs( pURLs );
	}

	return aRet;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::setDescription( const OUString& /* rDescription */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFolderPicker::setDescription not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getImplementationName()
{
	return FOLDER_PICKER_IMPL_NAME;
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaFolderPicker::supportsService( const OUString& ServiceName )
{
	Sequence < OUString > aSupportedServicesNames = JavaFolderPicker_getSupportedServiceNames();
 
	for ( sal_Int32 n = aSupportedServicesNames.getLength(); n--; )
		if ( aSupportedServicesNames[n].compareTo(ServiceName) == 0 )
			return sal_True;
 
	return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL JavaFolderPicker::getSupportedServiceNames()
{
	return JavaFolderPicker_getSupportedServiceNames();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::cancel()
{
	NSFileDialog_cancel( mpDialog );
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::disposing( const EventObject& /* aEvent */ )
{
}
