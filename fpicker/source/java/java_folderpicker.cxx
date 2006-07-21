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

#include <stdio.h>

#ifndef _JAVA_FOLDERPICKER_HXX_
#include "java_folderpicker.hxx"
#endif
#ifndef _JAVA_SERVICE_HXX_
#include "java_service.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HPP_
#include <com/sun/star/lang/NullPointerException.hpp>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include "cocoa_dialog.h"

using namespace cppu;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::ui::dialogs;
using namespace com::sun::star::util;
using namespace osl;
using namespace rtl;
using namespace java;

// ========================================================================

namespace java {

Sequence< OUString > SAL_CALL JavaFolderPicker_getSupportedServiceNames()
{
	Sequence< OUString > aRet( 2 );
	aRet[0] = OUString::createFromAscii( "com.sun.star.ui.dialogs.FolderPicker" );
	aRet[1] = OUString::createFromAscii( FOLDER_PICKER_SERVICE_NAME );
	return aRet;
}
 
// ------------------------------------------------------------------------

Reference< XInterface > SAL_CALL JavaFolderPicker_createInstance( const Reference< XMultiServiceFactory >& xMultiServiceFactory )
{
	return Reference< XInterface >( static_cast< XFolderPicker* >( new JavaFolderPicker( xMultiServiceFactory ) ) );
}

}

// ========================================================================

JavaFolderPicker::JavaFolderPicker( const Reference< XMultiServiceFactory >& xServiceMgr ) : WeakComponentImplHelper3< XFolderPicker, XServiceInfo, XCancellable >( maMutex )
{
	mpDialog = NSFileDialog_create( NULL, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE );
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

void SAL_CALL JavaFolderPicker::setTitle( const OUString& aTitle ) throw( RuntimeException )
{
	CFStringRef aString = CFStringCreateWithCharacters( NULL, aTitle.getStr(), aTitle.getLength() );
	if ( aString )
	{
		NSFileDialog_setTitle( mpDialog, aString );
		CFRelease( aString );
	}
}

// ------------------------------------------------------------------------

sal_Int16 SAL_CALL JavaFolderPicker::execute() throw( RuntimeException )
{
	// Don't lock mutex as we expect callbacks to this object from a
	// a different thread while the dialog is showing
	ULONG nCount = Application::ReleaseSolarMutex();
	sal_Int16 nRet = NSFileDialog_showFileDialog( mpDialog );
	Application::AcquireSolarMutex( nCount );

	return nRet;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::setDisplayDirectory( const OUString& aDirectory ) throw( IllegalArgumentException, RuntimeException )
{
	OUString aPath;
	File::getSystemPathFromFileURL( aDirectory, aPath );
	if ( aPath.getLength() )
	{
		CFStringRef aString = CFStringCreateWithCharacters( NULL, aPath.getStr(), aPath.getLength() );
		if ( aString )
		{
			NSFileDialog_setDirectory( mpDialog, aString );
			CFRelease( aString );
		}
	}
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getDisplayDirectory() throw( RuntimeException )
{
	OUString aRet;

	CFStringRef aString = NSFileDialog_directory( mpDialog );
	if ( aString )
	{
		CFIndex nLen = CFStringGetLength( aString );
		CFRange aRange = CFRangeMake( 0, nLen );
		sal_Unicode pBuffer[ nLen + 1 ];
		CFStringGetCharacters( aString, aRange, pBuffer );
		pBuffer[ nLen ] = 0;
		CFRelease( aString );
		OUString aPath( pBuffer );
		File::getFileURLFromSystemPath( aPath, aRet );
	}

	return aRet;
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getDirectory() throw( RuntimeException )
{
	OUString aRet;

	CFStringRef *pFileNames = NSFileDialog_fileNames( mpDialog );
	if ( pFileNames )
	{
		if ( pFileNames[ 0 ] )
		{
			CFStringRef aString = pFileNames[ 0 ];
			CFIndex nLen = CFStringGetLength( aString );
			CFRange aRange = CFRangeMake( 0, nLen );
			sal_Unicode pBuffer[ nLen + 1 ];
			CFStringGetCharacters( aString, aRange, pBuffer ); 
			pBuffer[ nLen ] = 0;
			OUString aPath( pBuffer );

			OUString aURL;
			File::getFileURLFromSystemPath( aPath, aURL );
			if ( aURL.getLength() )
				aRet = aURL;
		}

		NSFileManager_releaseFileNames( pFileNames );
	}

	return aRet;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::setDescription( const OUString& rDescription ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFolderPicker::setDescription not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getImplementationName() throw( RuntimeException )
{
	return OUString::createFromAscii( FOLDER_PICKER_IMPL_NAME );
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaFolderPicker::supportsService( const OUString& ServiceName ) throw( RuntimeException )
{
	Sequence < OUString > aSupportedServicesNames = JavaFolderPicker_getSupportedServiceNames();
 
	for ( sal_Int32 n = aSupportedServicesNames.getLength(); n--; )
		if ( aSupportedServicesNames[n].compareTo(ServiceName) == 0 )
			return sal_True;
 
	return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL JavaFolderPicker::getSupportedServiceNames() throw( RuntimeException )
{
	return JavaFolderPicker_getSupportedServiceNames();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::cancel() throw( RuntimeException )
{
	NSFileDialog_cancel( mpDialog );
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::disposing( const EventObject& aEvent ) throw( RuntimeException )
{
}
