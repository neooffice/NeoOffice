/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 * This file incorporates work covered by the following license notice:
 * 
 *   Modified March 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 4
 *   of the Apache License, Version 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include <scanner.hxx>
#include <sanedlg.hxx>
#include <vos/thread.hxx>
#include <tools/list.hxx>
#include <boost/shared_ptr.hpp>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#ifdef USE_JAVA

#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

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

    SaneHolder() : m_nError(ScanError_ScanErrorNone), m_bBusy(false) {}
};

namespace
{
    typedef std::vector< boost::shared_ptr<SaneHolder> > sanevec;
    class allSanes
    {
    private:
        int mnRefCount;
    public:
        sanevec m_aSanes;
        allSanes() : mnRefCount(0) {}
        void acquire();
        void release();
    };

    void allSanes::acquire()
    {
        ++mnRefCount;
    }

    void allSanes::release()
    {
        // was unused, now because of i99835: "Scanning interface not SANE API
        // compliant" destroy all SaneHolder to get Sane Dtor called
        --mnRefCount;
        if (!mnRefCount)
            m_aSanes.clear();
    }

    struct theSaneProtector : public rtl::Static<vos::OMutex, theSaneProtector> {}; 
    struct theSanes : public rtl::Static<allSanes, theSanes> {}; 
}

// -----------------
// - ScannerThread -
// -----------------

class ScannerThread : public vos::OThread
{
	boost::shared_ptr<SaneHolder>				m_pHolder;
	REF( com::sun::star::lang::XEventListener )	m_xListener;
	ScannerManager*								m_pManager; // just for the disposing call

public:
	virtual void run();
	virtual void onTerminated() { delete this; }
public:
	ScannerThread( boost::shared_ptr<SaneHolder> pHolder,
				   const REF( com::sun::star::lang::XEventListener )& listener,
				   ScannerManager* pManager );
	virtual ~ScannerThread();
};

// -----------------------------------------------------------------------------

ScannerThread::ScannerThread(
                             boost::shared_ptr<SaneHolder> pHolder,
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
			m_pHolder->m_aSane.SetOptionValue( nOption, (sal_Bool)sal_False );

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

void ScannerManager::AcquireData()
{
    vos::OGuard aGuard( theSaneProtector::get() );
    theSanes::get().acquire();
}

void ScannerManager::ReleaseData()
{
    vos::OGuard aGuard( theSaneProtector::get() );
    theSanes::get().release();
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
	vos::OGuard aGuard( theSaneProtector::get() );
	sanevec &rSanes = theSanes::get().m_aSanes;

	if( rSanes.empty() )
	{
		boost::shared_ptr<SaneHolder> pSaneHolder(new SaneHolder);
		if( Sane::IsSane() )
			rSanes.push_back( pSaneHolder );
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

sal_Bool ScannerManager::configureScanner( ScannerContext& scanner_context ) throw( ScannerException )
{
#ifdef USE_JAVA
	return sal_True;
#else	// USE_JAVA
	vos::OGuard aGuard( theSaneProtector::get() );
	sanevec &rSanes = theSanes::get().m_aSanes;

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "ScannerManager::configureScanner\n" );
#endif

	if( scanner_context.InternalData < 0 || (sal_uLong)scanner_context.InternalData >= rSanes.size() )
		throw ScannerException(
			::rtl::OUString::createFromAscii( "Scanner does not exist" ),
			REF( XScannerManager )( this ),
			ScanError_InvalidContext
			);

    boost::shared_ptr<SaneHolder> pHolder = rSanes[scanner_context.InternalData];
    if( pHolder->m_bBusy )
		throw ScannerException(
			::rtl::OUString::createFromAscii( "Scanner is busy" ),
			REF( XScannerManager )( this ),
			ScanError_ScanInProgress
			);

    pHolder->m_bBusy = true;
	SaneDlg aDlg( NULL, pHolder->m_aSane );
    sal_Bool bRet = (sal_Bool)aDlg.Execute();
    pHolder->m_bBusy = false;
        
	return bRet;
#endif	// USE_JAVA
}

// -----------------------------------------------------------------------------

void ScannerManager::startScan( const ScannerContext& scanner_context,
								const REF( com::sun::star::lang::XEventListener )& listener ) throw( ScannerException )
{
#ifdef USE_JAVA
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
	vos::OGuard aGuard( theSaneProtector::get() );
	sanevec &rSanes = theSanes::get().m_aSanes;

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "ScannerManager::startScan\n" );
#endif

	if( scanner_context.InternalData < 0 || (sal_uLong)scanner_context.InternalData >= rSanes.size() )
		throw ScannerException(
			::rtl::OUString::createFromAscii( "Scanner does not exist" ),
			REF( XScannerManager )( this ),
			ScanError_InvalidContext
			);
	boost::shared_ptr<SaneHolder> pHolder = rSanes[scanner_context.InternalData];
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
	vos::OGuard aGuard( theSaneProtector::get() );
	sanevec &rSanes = theSanes::get().m_aSanes;

	if( scanner_context.InternalData < 0 || (sal_uLong)scanner_context.InternalData >= rSanes.size() )
		throw ScannerException(
			::rtl::OUString::createFromAscii( "Scanner does not exist" ),
			REF( XScannerManager )( this ),
			ScanError_InvalidContext
			);

	boost::shared_ptr<SaneHolder> pHolder = rSanes[scanner_context.InternalData];

	return pHolder->m_nError;
}

// -----------------------------------------------------------------------------

REF( AWT::XBitmap ) ScannerManager::getBitmap( const ScannerContext& scanner_context ) throw( ScannerException )
{
	vos::OGuard aGuard( theSaneProtector::get() );
	sanevec &rSanes = theSanes::get().m_aSanes;

	if( scanner_context.InternalData < 0 || (sal_uLong)scanner_context.InternalData >= rSanes.size() )
		throw ScannerException(
			::rtl::OUString::createFromAscii( "Scanner does not exist" ),
			REF( XScannerManager )( this ),
			ScanError_InvalidContext
			);
	boost::shared_ptr<SaneHolder> pHolder = rSanes[scanner_context.InternalData];

	vos::OGuard aProtGuard( pHolder->m_aProtector );

	REF( AWT::XBitmap ) xRet( pHolder->m_xBitmap );
	pHolder->m_xBitmap = REF( AWT::XBitmap )();

	return xRet;
}
