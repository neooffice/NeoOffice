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
 *  Copyright 2003 Planamesa Inc.
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

#include <boost/unordered_map.hpp>

#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>

#include "java/salinst.h"

#include "java_clipboard.hxx"
#include "DTransClipboard.hxx"

static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > aSystemClipboard;
static ::boost::unordered_map< OUString, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >, OUStringHash > aClipboardInstancesMap;
static ::osl::Mutex aClipboardInstancesMutex;

using namespace com::sun::star;
using namespace cppu;
using namespace osl;
using namespace rtl;

// ========================================================================

static uno::Sequence< OUString > JavaClipboard_getSupportedServiceNames()
{
	uno::Sequence< OUString > aRet( 1 );
	aRet[0] = "com.sun.star.datatransfer.clipboard.SystemClipboard";
	return aRet;
}

// ========================================================================

JavaClipboard::JavaClipboard( bool bSystemClipboard ) : WeakComponentImplHelper3< datatransfer::clipboard::XSystemClipboard, datatransfer::clipboard::XFlushableClipboard, lang::XServiceInfo >( maMutex ),
	mbSystemClipboard( bSystemClipboard ),
	mbPrivateClipboard( sal_False )
{
}

// ------------------------------------------------------------------------

JavaClipboard::~JavaClipboard()
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaClipboard::flushClipboard( ) throw( uno::RuntimeException, std::exception )
{
	uno::Reference< datatransfer::XTransferable > aContents;

	ClearableMutexGuard aGuard( maMutex );

	if ( mbSystemClipboard )
		aContents = uno::Reference< datatransfer::XTransferable >( mbPrivateClipboard ? maPrivateContents : maContents );

	aGuard.clear();

	if ( aContents.is() )
	{
		DTransTransferable *pTransferable = NULL;
		if ( aContents.is() )
			pTransferable = (DTransTransferable *)aContents.get();

		if ( pTransferable )
			pTransferable->flush();
	}
}

// ------------------------------------------------------------------------

uno::Reference< datatransfer::XTransferable > SAL_CALL JavaClipboard::getContents() throw( uno::RuntimeException, std::exception )
{
	MutexGuard aGuard( maMutex );

	uno::Reference< datatransfer::XTransferable > aContents( maContents );

	// Don't send any changedContents notifications to listeners if this is
	// a private clipboard as it will cause Calc's Edit > Paste menus to be
	// disabled when another application has ownership of the system clipboard
	if ( mbPrivateClipboard )
		return aContents;

	if ( mbSystemClipboard )
	{
		DTransTransferable *pTransferable = NULL;
		if ( maContents.is() )
			pTransferable = (DTransTransferable *)maContents.get();

		if ( pTransferable && pTransferable->hasOwnership() )
		{
			uno::Reference< datatransfer::XTransferable > aLocalContents( pTransferable->getTransferable() );
			if ( aLocalContents.is() )
				aContents = aLocalContents;
			else
				maContents = uno::Reference< datatransfer::XTransferable >( pTransferable );
		}
		else
		{
			uno::Reference< datatransfer::XTransferable > aOldContents( maContents );
			NSInteger nOldChangeCount = -1;
			if ( pTransferable )
			{
				nOldChangeCount = pTransferable->getChangeCount();
				aOldContents = pTransferable->getTransferable();
			}

			NSInteger nChangeCount = -1;
			pTransferable = DTransClipboard::getContents();
			if ( pTransferable )
			{
				nChangeCount = pTransferable->getChangeCount();
				maContents = uno::Reference< datatransfer::XTransferable >( pTransferable );
			}
			else
			{
				maContents = uno::Reference< datatransfer::XTransferable >();
			}

			uno::Reference< datatransfer::clipboard::XClipboardOwner > aOldOwner( maOwner );
			maOwner = uno::Reference< datatransfer::clipboard::XClipboardOwner >();

			aContents = maContents;

			// Fix bug 3650 by not sending lostOwnership notifications to
			// transferables that were never pushed to the system clipboard
			// by our application. Fix Edit menu update failure when the system
			// clipboard is changed in another application by sending
			// changedContents notifications when the system clipboard's change
			// count has changed.
			if ( nOldChangeCount != nChangeCount )
			{
				::std::list< uno::Reference< datatransfer::clipboard::XClipboardListener > > listeners( maListeners );

				maMutex.release();

				if ( aOldOwner.is() )
					aOldOwner->lostOwnership( static_cast< datatransfer::clipboard::XClipboard* >( this ), aOldContents );

				datatransfer::clipboard::ClipboardEvent aEvent( static_cast< OWeakObject* >( this ), aContents );
				while ( listeners.begin() != listeners.end() )
				{
					if( listeners.front().is() )
						listeners.front()->changedContents( aEvent );
					listeners.pop_front();
				}

				maMutex.acquire();
			}
		}
	}

	return aContents;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaClipboard::setContents( const uno::Reference< datatransfer::XTransferable >& xTransferable, const uno::Reference< datatransfer::clipboard::XClipboardOwner >& xClipboardOwner ) throw( uno::RuntimeException, std::exception )
{
	ClearableMutexGuard aGuard( maMutex );

	uno::Reference< datatransfer::XTransferable > aOldContents( maContents );
	maContents = xTransferable;

	uno::Reference< datatransfer::clipboard::XClipboardOwner > aOldOwner( maOwner );
	maOwner = xClipboardOwner;

	// Don't send any changedContents notifications to listeners if this is
	// a private clipboard as it will cause Calc's Edit > Paste menus to be
	// disabled when another application has ownership of the system clipboard
	if ( mbPrivateClipboard )
		return;

	if ( mbSystemClipboard )
	{
		DTransTransferable *pTransferable = NULL;
		if ( aOldContents.is() )
			pTransferable = (DTransTransferable *)aOldContents.get();
		if ( pTransferable )
			aOldContents = pTransferable->getTransferable();
		else
			aOldContents = uno::Reference< datatransfer::XTransferable >();

		pTransferable = DTransClipboard::setContents( xTransferable );
		if ( pTransferable )
			maContents = uno::Reference< datatransfer::XTransferable >( pTransferable );
		else
			maContents = uno::Reference< datatransfer::XTransferable >();
	}

	::std::list< uno::Reference< datatransfer::clipboard::XClipboardListener > > listeners( maListeners );

	aGuard.clear();

	if ( aOldOwner.is() )
		aOldOwner->lostOwnership( static_cast< datatransfer::clipboard::XClipboard* >( this ), aOldContents );

	datatransfer::clipboard::ClipboardEvent aEvent( static_cast< OWeakObject* >( this ), xTransferable );
	while ( listeners.begin() != listeners.end() )
	{
		if( listeners.front().is() )
			listeners.front()->changedContents( aEvent );
		listeners.pop_front();
	}
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaClipboard::getName() throw( uno::RuntimeException, std::exception )
{
	return OUString();
}

// ------------------------------------------------------------------------

sal_Int8 SAL_CALL JavaClipboard::getRenderingCapabilities() throw( uno::RuntimeException, std::exception )
{
	return datatransfer::clipboard::RenderingCapabilities::Delayed;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaClipboard::addClipboardListener( const uno::Reference< datatransfer::clipboard::XClipboardListener >& listener ) throw( uno::RuntimeException, std::exception )
{
	MutexGuard aGuard( maMutex );

	maListeners.push_back( listener );
}

// ------------------------------------------------------------------------

void SAL_CALL JavaClipboard::removeClipboardListener( const uno::Reference< datatransfer::clipboard::XClipboardListener >& listener ) throw( uno::RuntimeException, std::exception )
{
	MutexGuard aGuard( maMutex );

	maListeners.remove( listener );
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaClipboard::getImplementationName() throw( uno::RuntimeException, std::exception )
{
	return "com.sun.star.datatransfer.clipboard.AquaClipboard";
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaClipboard::supportsService( const OUString& ServiceName ) throw( uno::RuntimeException, std::exception )
{
	uno::Sequence < OUString > aSupportedServicesNames = JavaClipboard_getSupportedServiceNames();

	for ( sal_Int32 n = aSupportedServicesNames.getLength(); n--; )
		if ( aSupportedServicesNames[ n ].compareTo( ServiceName ) == 0 )
			return sal_True;

	return sal_False;
}

// ------------------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL JavaClipboard::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
{
	return JavaClipboard_getSupportedServiceNames();
}

// ------------------------------------------------------------------------

void JavaClipboard::setPrivateClipboard( sal_Bool bPrivateClipboard )
{
	ClearableMutexGuard aGuard( maMutex );

	if ( mbSystemClipboard && bPrivateClipboard != mbPrivateClipboard )
	{
		uno::Reference< datatransfer::XTransferable > aOldContents( maContents );
		maContents.clear();

		uno::Reference< datatransfer::clipboard::XClipboardOwner > aOldOwner( maOwner );
		maOwner.clear();

		mbPrivateClipboard = bPrivateClipboard;

		if ( mbPrivateClipboard )
		{
			maPrivateContents = aOldContents;
			maPrivateOwner = aOldOwner;
		}
		else
		{
			maContents = maPrivateContents;
			maOwner = maPrivateOwner;

			maPrivateContents.clear();
			maPrivateOwner.clear();
		}

		::std::list< uno::Reference< datatransfer::clipboard::XClipboardListener > > listeners( maListeners );

		aGuard.clear();

		if ( aOldOwner.is() )
			aOldOwner->lostOwnership( static_cast< datatransfer::clipboard::XClipboard* >( this ), aOldContents );

		datatransfer::clipboard::ClipboardEvent aEvent( static_cast< OWeakObject* >( this ), maContents );
		while ( listeners.begin() != listeners.end() )
		{
			if( listeners.front().is() )
				listeners.front()->changedContents( aEvent );
			listeners.pop_front();
		}
	}
}

// ========================================================================

uno::Reference< uno::XInterface > JavaSalInstance::CreateClipboard( const uno::Sequence< uno::Any >& rArguments )
{
	MutexGuard aGuard( aClipboardInstancesMutex );

	uno::Reference< uno::XInterface > xClipboard;
	OUString aClipboardName;
	if ( rArguments.getLength() > 1 )
	{
		rArguments.getConstArray()[ 1 ] >>= aClipboardName;
		xClipboard = aClipboardInstancesMap[ aClipboardName ];
		if ( !xClipboard.is() )
		{
			xClipboard = uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( new JavaClipboard( false ) ) );
			aClipboardInstancesMap[ aClipboardName ] = xClipboard;
		}
	}
	else
	{
		xClipboard = aSystemClipboard;
		if ( !xClipboard.is() )
		{
			xClipboard = uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( new JavaClipboard( true ) ) );
			aSystemClipboard = xClipboard;
		}
	}

	return xClipboard;
}
