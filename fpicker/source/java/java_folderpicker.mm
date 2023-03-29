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
	return NSFileDialog_showFileDialog( mpDialog );
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::setDisplayDirectory( const OUString& aDirectory ) throw( IllegalArgumentException, RuntimeException )
{
	if ( aDirectory.getLength() )
	{
		CFStringRef aString = CFStringCreateWithCharacters( NULL, aDirectory.getStr(), aDirectory.getLength() );
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
		aRet = CFStringToOUString( aString );
		CFRelease( aString );
	}

	return aRet;
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getDirectory() throw( RuntimeException )
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

void SAL_CALL JavaFolderPicker::setDescription( const OUString& /* rDescription */ ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFolderPicker::setDescription not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getImplementationName() throw( RuntimeException )
{
	return FOLDER_PICKER_IMPL_NAME;
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

void SAL_CALL JavaFolderPicker::disposing( const EventObject& /* aEvent */ ) throw( RuntimeException )
{
}
