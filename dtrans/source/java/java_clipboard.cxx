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

#ifndef _JAVA_CLIPBOARD_HXX_
#include "java_clipboard.hxx"
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_RENDERINGCAPABILITIES_HPP_
#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>
#endif

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace java;
using namespace osl;
using namespace rtl;
using namespace std;

// ========================================================================

namespace java
{

Sequence< OUString > SAL_CALL JavaClipboard_getSupportedServiceNames()
{
	Sequence< OUString > aRet( 1 );
	aRet[0] = OUString::createFromAscii( JAVA_CLIPBOARD_SERVICE_NAME );
	return aRet;
}

}

// ========================================================================

Reference< XTransferable > *JavaClipboard::mpContents = NULL;

// ------------------------------------------------------------------------

Reference< XClipboardOwner > *JavaClipboard::mpOwner = NULL;

// ------------------------------------------------------------------------

JavaClipboard *JavaClipboard::mpLastSetter = NULL;

// ------------------------------------------------------------------------

Mutex JavaClipboard::maClipboardMutex;

// ========================================================================

JavaClipboard::JavaClipboard() : WeakComponentImplHelper4< XClipboardEx, XClipboardNotifier, XServiceInfo, XInitialization >( maMutex )
{
}

// ------------------------------------------------------------------------

JavaClipboard::~JavaClipboard()
{
	ClearableMutexGuard aGuard( maClipboardMutex );
	if ( mpLastSetter == this )
	{
		Reference< XTransferable > aOldContents;
		if ( mpContents )
		{
			aOldContents = Reference< XTransferable >( *mpContents );
			delete mpContents;
			mpContents = NULL;
		}

		Reference< XClipboardOwner > aOldOwner;
		if ( mpOwner )
		{
			aOldOwner = Reference< XClipboardOwner >( *mpOwner );
			delete mpOwner;
			mpOwner = NULL;
		}
    
		list< Reference< XClipboardListener > > listeners( maListeners );

		aGuard.clear();

		if ( aOldOwner.is() )
			aOldOwner->lostOwnership( static_cast< XClipboard* >( this ), aOldContents );

		mpLastSetter = NULL;
	}
}

// ------------------------------------------------------------------------

Reference< XTransferable > SAL_CALL JavaClipboard::getContents() throw( RuntimeException )
{
	MutexGuard aGuard( maClipboardMutex );
	Reference< XTransferable > aContents;
	if ( mpContents )
		aContents = Reference< XTransferable >( *mpContents );
	return aContents;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaClipboard::setContents( const Reference< XTransferable >& xTransferable, const Reference< XClipboardOwner >& xClipboardOwner ) throw( RuntimeException )
{
	ClearableMutexGuard aGuard( maClipboardMutex );

	mpLastSetter = this;

	Reference< XTransferable > aOldContents;
	if ( mpContents )
	{
		aOldContents = Reference< XTransferable >( *mpContents );
		delete mpContents;
	}
	mpContents = new Reference< XTransferable >( xTransferable );

	Reference< XClipboardOwner > aOldOwner;
	if ( mpOwner )
	{
		aOldOwner = Reference< XClipboardOwner >( *mpOwner );
		delete mpOwner;
	}
	mpOwner = new Reference< XClipboardOwner >( xClipboardOwner );
    
	list< Reference< XClipboardListener > > listeners( maListeners );

	aGuard.clear();

	if ( aOldOwner.is() )
		aOldOwner->lostOwnership( static_cast< XClipboard* >( this ), aOldContents );

	ClipboardEvent aEvent( static_cast< OWeakObject* >( this ), xTransferable );
	while( listeners.begin() != listeners.end() )
	{
		if( listeners.front().is() )
			listeners.front()->changedContents( aEvent );
		listeners.pop_front();
	}
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaClipboard::getName() throw( RuntimeException )
{
	return OUString();
}

// ------------------------------------------------------------------------

sal_Int8 SAL_CALL JavaClipboard::getRenderingCapabilities() throw( RuntimeException )
{
	return RenderingCapabilities::Delayed;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaClipboard::addClipboardListener( const Reference< XClipboardListener >& listener ) throw( RuntimeException )
{
	MutexGuard aGuard( maMutex );
	maListeners.push_back( listener );
}

// ------------------------------------------------------------------------

void SAL_CALL JavaClipboard::removeClipboardListener( const Reference< XClipboardListener >& listener ) throw( RuntimeException )
{
	MutexGuard aGuard( maMutex );
	maListeners.remove( listener );
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaClipboard::getImplementationName() throw( RuntimeException )
{
	return OUString::createFromAscii( JAVA_CLIPBOARD_IMPL_NAME );
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaClipboard::supportsService( const OUString& ServiceName ) throw( RuntimeException )
{
	Sequence < OUString > aSupportedServicesNames = JavaClipboard_getSupportedServiceNames();

	for ( sal_Int32 n = aSupportedServicesNames.getLength(); n--; )
		if ( aSupportedServicesNames[ n ].compareTo( ServiceName ) == 0 )
			return sal_True;

	return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL JavaClipboard::getSupportedServiceNames() throw( RuntimeException )
{
	return JavaClipboard_getSupportedServiceNames();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaClipboard::initialize( const Sequence< Any >& xAny ) throw( RuntimeException )
{
}

// ========================================================================

JavaClipboardFactory::JavaClipboardFactory() : WeakComponentImplHelper1< XSingleServiceFactory >( maMutex )
{
}

// ------------------------------------------------------------------------

JavaClipboardFactory::~JavaClipboardFactory()
{
}

// ------------------------------------------------------------------------

Reference< XInterface > JavaClipboardFactory::createInstance() throw()
{
	return Reference< XInterface >( static_cast< OWeakObject* >( new JavaClipboard() ) );
}

// ------------------------------------------------------------------------

Reference< XInterface > JavaClipboardFactory::createInstanceWithArguments( const Sequence< Any >& arguments ) throw()
{
	return createInstance();
}
