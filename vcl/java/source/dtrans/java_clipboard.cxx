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

#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>

#include "java/saldata.hxx"
#include "java/salinst.h"

#include "java_clipboard.hxx"
#include "DTransClipboard.hxx"

using namespace com::sun::star;
using namespace cppu;
using namespace osl;
using namespace rtl;

// ========================================================================

static uno::Sequence< OUString > JavaClipboard_getSupportedServiceNames()
{
	uno::Sequence< OUString > aRet( 1 );
	aRet[0] = OUString::createFromAscii( "com.sun.star.datatransfer.clipboard.SystemClipboard" );
	return aRet;
}

// ========================================================================

JavaClipboard::JavaClipboard( bool bSystemClipboard ) : WeakComponentImplHelper4< datatransfer::clipboard::XClipboardEx, XFlushableClipboard, datatransfer::clipboard::XClipboardNotifier, XServiceInfo >( maMutex ),
	mbSystemClipboard( bSystemClipboard ),
	mbPrivateClipboard( sal_False )
{
}

// ------------------------------------------------------------------------

JavaClipboard::~JavaClipboard()
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaClipboard::flushClipboard( ) throw( uno::RuntimeException )
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

uno::Reference< datatransfer::XTransferable > SAL_CALL JavaClipboard::getContents() throw( uno::RuntimeException )
{
	MutexGuard aGuard( maMutex );

	uno::Reference< datatransfer::XTransferable > aContents( maContents );

	if ( mbSystemClipboard && !mbPrivateClipboard )
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
			if ( pTransferable )
				aOldContents = pTransferable->getTransferable();
			pTransferable = DTransClipboard::getContents();
			if ( pTransferable )
				maContents = uno::Reference< datatransfer::XTransferable >( pTransferable );
			else
				maContents = uno::Reference< datatransfer::XTransferable >();

			uno::Reference< datatransfer::clipboard::XClipboardOwner > aOldOwner( maOwner );
			maOwner = uno::Reference< datatransfer::clipboard::XClipboardOwner >();

			aContents = maContents;

			// Fix bug 3650 by not sending lost ownership notifications to
			// transferables that were never pushed to the system clipboard
			// by our application
			if ( aOldContents.is() )
			{
				pTransferable = (DTransTransferable *)aOldContents.get();
				if ( pTransferable && pTransferable->getChangeCount() >= 0 )
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
	}

	return aContents;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaClipboard::setContents( const uno::Reference< datatransfer::XTransferable >& xTransferable, const uno::Reference< datatransfer::clipboard::XClipboardOwner >& xClipboardOwner ) throw( uno::RuntimeException )
{
	ClearableMutexGuard aGuard( maMutex );

	uno::Reference< datatransfer::XTransferable > aOldContents( maContents );
	maContents = xTransferable;

	uno::Reference< datatransfer::clipboard::XClipboardOwner > aOldOwner( maOwner );
	maOwner = xClipboardOwner;

	if ( mbSystemClipboard && !mbPrivateClipboard )
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

OUString SAL_CALL JavaClipboard::getName() throw( uno::RuntimeException )
{
	return OUString();
}

// ------------------------------------------------------------------------

sal_Int8 SAL_CALL JavaClipboard::getRenderingCapabilities() throw( uno::RuntimeException )
{
	return datatransfer::clipboard::RenderingCapabilities::Delayed;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaClipboard::addClipboardListener( const uno::Reference< datatransfer::clipboard::XClipboardListener >& listener ) throw( uno::RuntimeException )
{
	MutexGuard aGuard( maMutex );

	maListeners.push_back( listener );
}

// ------------------------------------------------------------------------

void SAL_CALL JavaClipboard::removeClipboardListener( const uno::Reference< datatransfer::clipboard::XClipboardListener >& listener ) throw( uno::RuntimeException )
{
	MutexGuard aGuard( maMutex );

	maListeners.remove( listener );
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaClipboard::getImplementationName() throw( uno::RuntimeException )
{
	return OUString::createFromAscii( "com.sun.star.datatransfer.clipboard.JavaClipboard" );
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaClipboard::supportsService( const OUString& ServiceName ) throw( uno::RuntimeException )
{
	uno::Sequence < OUString > aSupportedServicesNames = JavaClipboard_getSupportedServiceNames();

	for ( sal_Int32 n = aSupportedServicesNames.getLength(); n--; )
		if ( aSupportedServicesNames[ n ].compareTo( ServiceName ) == 0 )
			return sal_True;

	return sal_False;
}

// ------------------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL JavaClipboard::getSupportedServiceNames() throw( uno::RuntimeException )
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
			DTransTransferable *pTransferable = NULL;
			if ( maPrivateContents.is() )
				pTransferable = (DTransTransferable *)maPrivateContents.get();

			if ( pTransferable && pTransferable->hasOwnership() )
			{
				maContents = maPrivateContents;
				maOwner = maPrivateOwner;
			}

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

uno::Reference< uno::XInterface > JavaSalInstance::CreateClipboard( const uno::Sequence< uno::Any >& /* rArguments */ )
{
	SalData *pSalData = GetSalData();

	if ( !pSalData->mxClipboard.is() )
		pSalData->mxClipboard = uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( new JavaClipboard() ) );

	return pSalData->mxClipboard;
}

// ========================================================================

extern "C" void Application_setPrivateClipboard( uno::Reference< datatransfer::clipboard::XClipboard > *pClipboard, sal_Bool bPrivateClipboard )
{
	if ( !pClipboard )
		return;

	uno::Reference< datatransfer::clipboard::XClipboard > aClipboard = *pClipboard;
	if ( aClipboard.is() )
	{
		JavaClipboard *pJavaClipboard = dynamic_cast< JavaClipboard* >( aClipboard.get() );
		if ( pJavaClipboard )
			pJavaClipboard->setPrivateClipboard( bPrivateClipboard );
	}
}
