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
}

// ------------------------------------------------------------------------

sal_Int16 SAL_CALL JavaFolderPicker::execute() throw( RuntimeException )
{
	return 0;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::setDisplayDirectory( const OUString& rDirectory ) throw( IllegalArgumentException, RuntimeException )
{
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getDisplayDirectory() throw( RuntimeException )
{
	return OUString();
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getDirectory() throw( RuntimeException )
{
	return OUString();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::setDescription( const OUString& rDescription ) throw( RuntimeException )
{
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getImplementationName() throw( RuntimeException )
{
	return OUString();
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaFolderPicker::supportsService( const OUString& ServiceName ) throw( RuntimeException )
{
	return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL JavaFolderPicker::getSupportedServiceNames() throw( RuntimeException )
{
	return Sequence< OUString >();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::cancel() throw( RuntimeException )
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::disposing( const EventObject& aEvent ) throw( RuntimeException )
{
}
