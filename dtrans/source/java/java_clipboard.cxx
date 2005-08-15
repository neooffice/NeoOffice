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
#ifndef _DTRANSCLIPBOARD_HXX
#include "DTransClipboard.hxx"
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

JavaClipboard::JavaClipboard( bool bSystemClipboard ) : WeakComponentImplHelper4< XClipboardEx, XClipboardNotifier, XServiceInfo, XInitialization >( maMutex )
{
	mbSystemClipboard = bSystemClipboard;
}

// ------------------------------------------------------------------------

JavaClipboard::~JavaClipboard()
{
}

// ------------------------------------------------------------------------

Reference< XTransferable > SAL_CALL JavaClipboard::getContents() throw( RuntimeException )
{
	ClearableMutexGuard aGuard( maMutex );

	Reference< XTransferable > aContents( maContents );

	if ( mbSystemClipboard )
	{
		DTransTransferable *pTransferable = NULL;
		if ( maContents.is() )
			pTransferable = (DTransTransferable *)maContents.get();

		if ( pTransferable && pTransferable->hasOwnership() )
		{
			Reference< XTransferable > aLocalContents( pTransferable->getTransferable() );
			if ( aLocalContents.is() )
				aContents = aLocalContents;
			else
				maContents = Reference< XTransferable >( pTransferable );
		}
		else
		{
			Reference< XTransferable > aOldContents( maContents );
			if ( pTransferable )
				aOldContents = pTransferable->getTransferable();
			pTransferable = DTransClipboard::getContents();
			if ( pTransferable )
				maContents = Reference< XTransferable >( pTransferable );
			else
				maContents = Reference< XTransferable >();

			Reference< XClipboardOwner > aOldOwner( maOwner );
			maOwner = Reference< XClipboardOwner >();

			aContents = maContents;

			list< Reference< XClipboardListener > > listeners( maListeners );

			aGuard.clear();

			if ( aOldOwner.is() )
				aOldOwner->lostOwnership( static_cast< XClipboard* >( this ), aOldContents );

			ClipboardEvent aEvent( static_cast< OWeakObject* >( this ), aContents );
			while ( listeners.begin() != listeners.end() )
			{
				if( listeners.front().is() )
					listeners.front()->changedContents( aEvent );
				listeners.pop_front();
			}
		}
	}

	return aContents;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaClipboard::setContents( const Reference< XTransferable >& xTransferable, const Reference< XClipboardOwner >& xClipboardOwner ) throw( RuntimeException )
{
	ClearableMutexGuard aGuard( maMutex );

	Reference< XTransferable > aOldContents( maContents );
	maContents = xTransferable;

	Reference< XClipboardOwner > aOldOwner( maOwner );
	maOwner = xClipboardOwner;

	if ( mbSystemClipboard )
	{
		DTransTransferable *pTransferable = NULL;
		if ( aOldContents.is() )
			pTransferable = (DTransTransferable *)aOldContents.get();
		if ( pTransferable )
			aOldContents = pTransferable->getTransferable();
		else
			aOldContents = Reference< XTransferable >();
		pTransferable = DTransClipboard::setContents( xTransferable );
		if ( pTransferable )
			maContents = Reference< XTransferable >( pTransferable );
		else
			maContents = Reference< XTransferable >();
	}

	list< Reference< XClipboardListener > > listeners( maListeners );

	aGuard.clear();

	if ( aOldOwner.is() )
		aOldOwner->lostOwnership( static_cast< XClipboard* >( this ), aOldContents );

	ClipboardEvent aEvent( static_cast< OWeakObject* >( this ), xTransferable );
	while ( listeners.begin() != listeners.end() )
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
	maListeners.push_back( listener );
}

// ------------------------------------------------------------------------

void SAL_CALL JavaClipboard::removeClipboardListener( const Reference< XClipboardListener >& listener ) throw( RuntimeException )
{
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
	return createInstanceWithArguments( Sequence< Any >() );
}

// ------------------------------------------------------------------------

Reference< XInterface > JavaClipboardFactory::createInstanceWithArguments( const Sequence< Any >& arguments ) throw()
{
	bool bSystemClipboard = false;
	OUString aClipboardName;
	if ( arguments.getLength() > 1 )
	{
		arguments.getConstArray()[ 1 ] >>= aClipboardName;
	}
	else
	{
		aClipboardName = OUString::createFromAscii( "CLIPBOARD" );
		bSystemClipboard = true;
	}

	MutexGuard aGuard( maMutex );

	Reference< XInterface > xClipboard = maInstances[ aClipboardName ];
	if ( !xClipboard.is() )
	{
		JavaClipboard *pClipboard = new JavaClipboard( bSystemClipboard );
		xClipboard = Reference< XInterface >( static_cast< OWeakObject* >( pClipboard ) );
		maInstances[ aClipboardName ] = xClipboard;
	}

	return xClipboard;
}
