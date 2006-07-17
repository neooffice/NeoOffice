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

using namespace cppu;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::ui::dialogs;
using namespace com::sun::star::util;
using namespace rtl;
using namespace java;

// ========================================================================

namespace java {

Sequence< OUString > SAL_CALL JavaFolderPicker_getSupportedServiceNames()
{
	Sequence< OUString > aRet( 2 );
	aRet[0] = OUString::createFromAscii( "com.sun.star.ui.dialogs.FolderPicker" );
	aRet[1] = OUString::createFromAscii( "com.sun.star.ui.dialogs.SystemFolderPicker" );
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
}

// ------------------------------------------------------------------------

JavaFolderPicker::~JavaFolderPicker()
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::setTitle( const OUString& aTitle ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFolderPicker::setTitle not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

sal_Int16 SAL_CALL JavaFolderPicker::execute() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFolderPicker::execute not implemented\n" );
#endif
	return 0;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::setDisplayDirectory( const OUString& rDirectory ) throw( IllegalArgumentException, RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFolderPicker::setDisplayDirectory not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getDisplayDirectory() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFolderPicker::getDisplayDirectory not implemented\n" );
#endif
	return OUString();
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getDirectory() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFolderPicker::getDirectory not implemented\n" );
#endif
	return OUString();
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
#ifdef DEBUG
	fprintf( stderr, "JavaFolderPicker::getImplementationName not implemented\n" );
#endif
	return OUString();
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
#ifdef DEBUG
	fprintf( stderr, "JavaFolderPicker::cancel not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::disposing( const EventObject& aEvent ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFolderPicker::disposing not implemented\n" );
#endif
}
