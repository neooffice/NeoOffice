/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU General Public License Version 2.1.
 *
 *
 *    GNU General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *    Modified March 2008 by Patrick Luby. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include <scanner.hxx>
#include <sanedlg.hxx>
#include <vos/thread.hxx>
#include <tools/list.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#ifdef USE_JAVA

#include <comphelper/processfactory.hxx>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

using namespace ::com::sun::star;

#endif	// USE_JAVA

BitmapTransporter::BitmapTransporter()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "BitmapTransporter\n" );
#endif
}

BitmapTransporter::~BitmapTransporter()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "~BitmapTransporter\n" );
#endif
}

// -----------------------------------------------------------------------------

ANY SAL_CALL BitmapTransporter::queryInterface( const Type& rType ) throw( RuntimeException )
{
	const ANY aRet( cppu::queryInterface( rType, static_cast< AWT::XBitmap* >( this ) ) );

	return( aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ) );
}

// -----------------------------------------------------------------------------

AWT::Size BitmapTransporter::getSize() throw()
{
	vos::OGuard	aGuard( m_aProtector );
	int			nPreviousPos = m_aStream.Tell();
	AWT::Size	aRet;

	// ensure that there is at least a header
	m_aStream.Seek( STREAM_SEEK_TO_END );
	int nLen = m_aStream.Tell();
	if( nLen > 15 )
	{
		m_aStream.Seek( 4 );
		m_aStream >> aRet.Width >> aRet.Height;
	}
	else
		aRet.Width = aRet.Height = 0;

	m_aStream.Seek( nPreviousPos );

	return aRet;
}

// -----------------------------------------------------------------------------

SEQ( sal_Int8 ) BitmapTransporter::getDIB() throw()
{
	vos::OGuard aGuard( m_aProtector );
	int			nPreviousPos = m_aStream.Tell();

	// create return value
	m_aStream.Seek( STREAM_SEEK_TO_END );
	int nBytes = m_aStream.Tell();
	m_aStream.Seek( 0 );

	SEQ( sal_Int8 ) aValue( nBytes );
	m_aStream.Read( aValue.getArray(), nBytes );
	m_aStream.Seek( nPreviousPos );

	return aValue;
}

// --------------
// - SaneHolder -
// --------------

struct SaneHolder
{
	Sane				m_aSane;
	REF( AWT::XBitmap )	m_xBitmap;
	vos::OMutex			m_aProtector;
	ScanError			m_nError;
    bool				m_bBusy;
};

DECLARE_LIST( SaneHolderList, SaneHolder* )

static SaneHolderList	allSanes;
static vos::OMutex		aSaneProtector;

// -----------------
// - ScannerThread -
// -----------------

class ScannerThread : public vos::OThread
{
	SaneHolder*									m_pHolder;
	REF( com::sun::star::lang::XEventListener )	m_xListener;
	ScannerManager*								m_pManager; // just for the disposing call

public:
	virtual void run();
	virtual void onTerminated() { delete this; }
public:
	ScannerThread( SaneHolder* pHolder,
				   const REF( com::sun::star::lang::XEventListener )& listener,
				   ScannerManager* pManager );
	virtual ~ScannerThread();
};

// -----------------------------------------------------------------------------

ScannerThread::ScannerThread(
                             SaneHolder* pHolder,
                             const REF( com::sun::star::lang::XEventListener )& listener,
                             ScannerManager* pManager )
        : m_pHolder( pHolder ), m_xListener( listener ), m_pManager( pManager )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "ScannerThread\n" );
#endif
}

ScannerThread::~ScannerThread()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "~ScannerThread\n" );
#endif
}

void ScannerThread::run()
{
	vos::OGuard			aGuard( m_pHolder->m_aProtector );
    BitmapTransporter*	pTransporter = new BitmapTransporter;
	REF( XInterface )	aIf( static_cast< OWeakObject* >( pTransporter ) );

	m_pHolder->m_xBitmap = REF( AWT::XBitmap )( aIf, UNO_QUERY );

    m_pHolder->m_bBusy = true;
	if( m_pHolder->m_aSane.IsOpen() )
	{
		int nOption = m_pHolder->m_aSane.GetOptionByName( "preview" );
		if( nOption != -1 )
			m_pHolder->m_aSane.SetOptionValue( nOption, (BOOL)FALSE );

		m_pHolder->m_nError =
			m_pHolder->m_aSane.Start( *pTransporter ) ?
			ScanError_ScanErrorNone : ScanError_ScanCanceled;
	}
	else
		m_pHolder->m_nError = ScanError_ScannerNotAvailable;


    REF( XInterface ) xXInterface( static_cast< OWeakObject* >( m_pManager ) );
	m_xListener->disposing( com::sun::star::lang::EventObject(xXInterface) );
    m_pHolder->m_bBusy = false;
}

// ------------------
// - ScannerManager -
// ------------------

void ScannerManager::DestroyData()
{
	// unused
}

// -----------------------------------------------------------------------------

AWT::Size ScannerManager::getSize() throw()
{
	AWT::Size aRet;
	aRet.Width = aRet.Height = 0;
	return aRet;
}

// -----------------------------------------------------------------------------

SEQ( sal_Int8 ) ScannerManager::getDIB() throw()
{
	return SEQ( sal_Int8 )();
}

// -----------------------------------------------------------------------------

SEQ( ScannerContext ) ScannerManager::getAvailableScanners() throw()
{
#ifdef USE_JAVA
	SEQ( ScannerContext ) aRet(1);
	aRet.getArray()[0].ScannerName = ::rtl::OUString::createFromAscii( "ImageCapture" );
	aRet.getArray()[0].InternalData = 0;
	return aRet;
#else	// USE_JAVA
	vos::OGuard aGuard( aSaneProtector );

	if( ! allSanes.Count() )
	{
		SaneHolder* pSaneHolder = new SaneHolder;
		pSaneHolder->m_nError = ScanError_ScanErrorNone;
        pSaneHolder->m_bBusy = false;
		if( Sane::IsSane() )
			allSanes.Insert( pSaneHolder );
		else
			delete pSaneHolder;
	}

	if( Sane::IsSane() )
	{
		SEQ( ScannerContext ) aRet(1);
		aRet.getArray()[0].ScannerName		= ::rtl::OUString::createFromAscii( "SANE" );
		aRet.getArray()[0].InternalData		= 0;
		return aRet;
	}

	return SEQ( ScannerContext )();
#endif	// USE_JAVA
}

// -----------------------------------------------------------------------------

BOOL ScannerManager::configureScanner( ScannerContext& scanner_context ) throw( ScannerException )
{
#ifdef USE_JAVA
fprintf( stderr, "Here 0\n" );
	return TRUE;
#else	// USE_JAVA
	vos::OGuard aGuard( aSaneProtector );

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "ScannerManager::configureScanner\n" );
#endif

	if( scanner_context.InternalData < 0 || (ULONG)scanner_context.InternalData >= allSanes.Count() )
		throw ScannerException(
			::rtl::OUString::createFromAscii( "Scanner does not exist" ),
			REF( XScannerManager )( this ),
			ScanError_InvalidContext
			);

    SaneHolder* pHolder = allSanes.GetObject( scanner_context.InternalData );
    if( pHolder->m_bBusy )
		throw ScannerException(
			::rtl::OUString::createFromAscii( "Scanner is busy" ),
			REF( XScannerManager )( this ),
			ScanError_ScanInProgress
			);

    pHolder->m_bBusy = true;
	SaneDlg aDlg( NULL, pHolder->m_aSane );
    BOOL bRet = (BOOL)aDlg.Execute();
    pHolder->m_bBusy = false;
        
	return bRet;
#endif	// USE_JAVA
}

// -----------------------------------------------------------------------------

void ScannerManager::startScan( const ScannerContext& scanner_context,
								const REF( com::sun::star::lang::XEventListener )& listener ) throw( ScannerException )
{
#ifdef USE_JAVA
fprintf( stderr, "Here 1\n" );
	uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

	// Invoke the ImageCapture UNO component's BASIC script
	try
	{
		util::URL aURL;
		aURL.Complete = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "macro:///ImageCapture.Main.Main" ) );
		uno::Reference< util::XURLTransformer > xTransformer( xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ) ) ), uno::UNO_QUERY_THROW );
		xTransformer->parseStrict( aURL );
		uno::Reference< frame::XDesktop > xDesktop( xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ) ), uno::UNO_QUERY_THROW );
		uno::Reference< frame::XDispatchProvider > xDispatchProvider( xDesktop->getCurrentFrame(), uno::UNO_QUERY );
		if( xDispatchProvider.is() )
		{
			uno::Reference< frame::XDispatch > xDispatch = xDispatchProvider->queryDispatch( aURL, rtl::OUString(), 0 );
			if( xDispatch.is() )
				xDispatch->dispatch( aURL, uno::Sequence< beans::PropertyValue >() );
		}
	}
	catch ( ... )
	{
	}
#else	// USE_JAVA
	vos::OGuard aGuard( aSaneProtector );

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "ScannerManager::startScan\n" );
#endif

	if( scanner_context.InternalData < 0 || (ULONG)scanner_context.InternalData >= allSanes.Count() )
		throw ScannerException(
			::rtl::OUString::createFromAscii( "Scanner does not exist" ),
			REF( XScannerManager )( this ),
			ScanError_InvalidContext
			);
	SaneHolder* pHolder = allSanes.GetObject( scanner_context.InternalData );
    if( pHolder->m_bBusy )
		throw ScannerException(
			::rtl::OUString::createFromAscii( "Scanner is busy" ),
			REF( XScannerManager )( this ),
			ScanError_ScanInProgress
			);
    pHolder->m_bBusy = true;

	ScannerThread* pThread = new ScannerThread( pHolder, listener, this );
	pThread->create();
#endif	// USE_JAVA
}

// -----------------------------------------------------------------------------

ScanError ScannerManager::getError( const ScannerContext& scanner_context ) throw( ScannerException )
{
	vos::OGuard aGuard( aSaneProtector );

	if( scanner_context.InternalData < 0 || (ULONG)scanner_context.InternalData >= allSanes.Count() )
		throw ScannerException(
			::rtl::OUString::createFromAscii( "Scanner does not exist" ),
			REF( XScannerManager )( this ),
			ScanError_InvalidContext
			);

	SaneHolder* pHolder = allSanes.GetObject( scanner_context.InternalData );

	return pHolder->m_nError;
}

// -----------------------------------------------------------------------------

REF( AWT::XBitmap ) ScannerManager::getBitmap( const ScannerContext& scanner_context ) throw( ScannerException )
{
	vos::OGuard aGuard( aSaneProtector );

	if( scanner_context.InternalData < 0 || (ULONG)scanner_context.InternalData >= allSanes.Count() )
		throw ScannerException(
			::rtl::OUString::createFromAscii( "Scanner does not exist" ),
			REF( XScannerManager )( this ),
			ScanError_InvalidContext
			);
	SaneHolder* pHolder = allSanes.GetObject( scanner_context.InternalData );

	vos::OGuard aProtGuard( pHolder->m_aProtector );

	REF( AWT::XBitmap ) xRet( pHolder->m_xBitmap );
	pHolder->m_xBitmap = REF( AWT::XBitmap )();

	return xRet;
}
