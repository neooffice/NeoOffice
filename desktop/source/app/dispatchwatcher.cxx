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
 *    Modified January 2007 by Patrick Luby. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dispatchwatcher.hxx"

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_CLOSEVETOEXCEPTION_HPP_
#include <com/sun/star/util/CloseVetoException.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XPRINTABLE_HPP_
#include <com/sun/star/view/XPrintable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_MACROEXECMODE_HPP_
#include <com/sun/star/document/MacroExecMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif

#include <tools/urlobj.hxx>
#include <comphelper/mediadescriptor.hxx>

#include <vector>

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::view;

namespace desktop
{

String GetURL_Impl( const String& rName );

struct DispatchHolder
{
	DispatchHolder( const URL& rURL, Reference< XDispatch >& rDispatch ) :
		aURL( rURL ), xDispatch( rDispatch ) {}

	URL	aURL;
	Reference< XDispatch > xDispatch;
};

Mutex* DispatchWatcher::pWatcherMutex = NULL;

Mutex& DispatchWatcher::GetMutex()
{
	if ( !pWatcherMutex )
	{
		::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
		if ( !pWatcherMutex )
			pWatcherMutex = new osl::Mutex();
	}

	return *pWatcherMutex;
}

// Create or get the dispatch watcher implementation. This implementation must be 
// a singleton to prevent access to the framework after it wants to terminate.
DispatchWatcher* DispatchWatcher::GetDispatchWatcher()
{
    static Reference< XInterface > xDispatchWatcher;
    static DispatchWatcher*        pDispatchWatcher = NULL;

    if ( !xDispatchWatcher.is() )
	{
		::osl::MutexGuard aGuard( GetMutex() );

		if ( !xDispatchWatcher.is() )
        {
			pDispatchWatcher = new DispatchWatcher();
            
            // We have to hold a reference to ourself forever to prevent our own destruction.
            xDispatchWatcher = static_cast< cppu::OWeakObject *>( pDispatchWatcher );
        }
	}

    return pDispatchWatcher;
}


DispatchWatcher::DispatchWatcher()
    : m_nRequestCount(1)
{
}


DispatchWatcher::~DispatchWatcher()
{
}


sal_Bool DispatchWatcher::executeDispatchRequests( const DispatchList& aDispatchRequestsList )
{
    Reference< XComponentLoader > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
												OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
											UNO_QUERY );

	DispatchList::const_iterator	p;
	std::vector< DispatchHolder >	aDispatches;
	::rtl::OUString					aAsTemplateArg( RTL_CONSTASCII_USTRINGPARAM( "AsTemplate"));

	for ( p = aDispatchRequestsList.begin(); p != aDispatchRequestsList.end(); p++ )
	{
        String					aPrinterName;
		const DispatchRequest&	aDispatchRequest = *p;

        // create parameter array
        sal_Int32 nCount = 4;
        if ( aDispatchRequest.aPreselectedFactory.getLength() )
            nCount++;

		// we need more properties for a print/print to request
		if ( aDispatchRequest.aRequestType == REQUEST_PRINT ||
			 aDispatchRequest.aRequestType == REQUEST_PRINTTO  )
            nCount++;

        Sequence < PropertyValue > aArgs( nCount );

        // mark request as user interaction from outside
        aArgs[0].Name = ::rtl::OUString::createFromAscii("Referer");
        aArgs[0].Value <<= ::rtl::OUString::createFromAscii("private:OpenEvent");

        if ( aDispatchRequest.aRequestType == REQUEST_PRINT ||
			 aDispatchRequest.aRequestType == REQUEST_PRINTTO )
        {
            aArgs[1].Name = ::rtl::OUString::createFromAscii("ReadOnly");
            aArgs[2].Name = ::rtl::OUString::createFromAscii("OpenNewView");
            aArgs[3].Name = ::rtl::OUString::createFromAscii("Hidden");
            aArgs[4].Name = ::rtl::OUString::createFromAscii("Silent");
        }
        else
        {
            Reference < com::sun::star::task::XInteractionHandler > xInteraction(
                ::comphelper::getProcessServiceFactory()->createInstance( OUString::createFromAscii("com.sun.star.task.InteractionHandler") ),
                com::sun::star::uno::UNO_QUERY );

            aArgs[1].Name = OUString::createFromAscii( "InteractionHandler" );
            aArgs[1].Value <<= xInteraction;

			sal_Int16 nMacroExecMode = ::com::sun::star::document::MacroExecMode::USE_CONFIG;
            aArgs[2].Name = OUString::createFromAscii( "MacroExecutionMode" );
            aArgs[2].Value <<= nMacroExecMode;

			sal_Int16 nUpdateDoc = ::com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG;
            aArgs[3].Name = OUString::createFromAscii( "UpdateDocMode" );
            aArgs[3].Value <<= nUpdateDoc;
        }

        if ( aDispatchRequest.aPreselectedFactory.getLength() )
        {
            aArgs[nCount-1].Name = ::comphelper::MediaDescriptor::PROP_DOCUMENTSERVICE();
            aArgs[nCount-1].Value <<= aDispatchRequest.aPreselectedFactory;
        }

        String aName( GetURL_Impl( aDispatchRequest.aURL ) );
        ::rtl::OUString aTarget( RTL_CONSTASCII_USTRINGPARAM("_default") );

        if ( aDispatchRequest.aRequestType == REQUEST_PRINT ||
			 aDispatchRequest.aRequestType == REQUEST_PRINTTO )
        {
            // documents opened for printing are opened readonly because they must be opened as a new document and this
            // document could be open already
            aArgs[1].Value <<= sal_True;

            // always open a new document for printing, because it must be disposed afterwards
            aArgs[2].Value <<= sal_True;

            // printing is done in a hidden view
            aArgs[3].Value <<= sal_True;

            // load document for printing without user interaction
            aArgs[4].Value <<= sal_True;

            // hidden documents should never be put into open tasks
            aTarget = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("_blank") );
        }

        // load the document ... if they are loadable!
        // Otherwise try to dispatch it ...
        Reference < XPrintable > xDoc;
        if(
            ( aName.CompareToAscii( ".uno"  , 4 ) == COMPARE_EQUAL )  ||
            ( aName.CompareToAscii( "slot:" , 5 ) == COMPARE_EQUAL )  ||
            ( aName.CompareToAscii( "macro:", 6 ) == COMPARE_EQUAL )  ||
            ( aName.CompareToAscii("vnd.sun.star.script", 19) == COMPARE_EQUAL)
          )
        {
            // Attention: URL must be parsed full. Otherwise some detections on it will fail!
            // It doesnt matter, if parser isn't available. Because; We try loading of URL then ...
            URL             aURL ;
            aURL.Complete = aName;

            Reference < XDispatch >         xDispatcher ;
            Reference < XDispatchProvider > xProvider   ( xDesktop, UNO_QUERY );
            Reference < XURLTransformer >   xParser     ( ::comphelper::getProcessServiceFactory()->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer")) ), ::com::sun::star::uno::UNO_QUERY );

            if( xParser.is() == sal_True )
                xParser->parseStrict( aURL );

            if( xProvider.is() == sal_True )
                xDispatcher = xProvider->queryDispatch( aURL, ::rtl::OUString(), 0 );

            if( xDispatcher.is() == sal_True )
			{
				{
					::osl::ClearableMutexGuard aGuard( GetMutex() );
					// Remember request so we can find it in statusChanged!
					m_aRequestContainer.insert( DispatchWatcherHashMap::value_type( aURL.Complete, (sal_Int32)1 ) );
                    m_nRequestCount++;
				}

				// Use local vector to store dispatcher because we have to fill our request container before
				// we can dispatch. Otherwise it would be possible that statusChanged is called before we dispatched all requests!!
				aDispatches.push_back( DispatchHolder( aURL, xDispatcher ));
			}
        }
        else if ( ( aName.CompareToAscii( "service:"  , 8 ) == COMPARE_EQUAL ) )
        {
            // TODO: the dispatch has to be done for loadComponentFromURL as well. Please ask AS for more details.
            URL             aURL ;
            aURL.Complete = aName;

            Reference < XDispatch >         xDispatcher ;
            Reference < XDispatchProvider > xProvider   ( xDesktop, UNO_QUERY );
            Reference < XURLTransformer >   xParser     ( ::comphelper::getProcessServiceFactory()->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer")) ), ::com::sun::star::uno::UNO_QUERY );

            if( xParser.is() == sal_True )
                xParser->parseStrict( aURL );

            if( xProvider.is() == sal_True )
                xDispatcher = xProvider->queryDispatch( aURL, ::rtl::OUString(), 0 );

            if( xDispatcher.is() == sal_True )
            {
			    try
			    {
				    // We have to be listener to catch errors during dispatching URLs.
				    // Otherwise it would be possible to have an office running without an open
				    // window!!
                    Sequence < PropertyValue > aArgs2(1);
                    aArgs2[0].Name    = ::rtl::OUString::createFromAscii("SynchronMode");
                    aArgs2[0].Value <<= sal_True;
				    Reference < XNotifyingDispatch > xDisp( xDispatcher, UNO_QUERY );
				    if ( xDisp.is() )
					    xDisp->dispatchWithNotification( aURL, aArgs2, DispatchWatcher::GetDispatchWatcher() );
				    else
					    xDispatcher->dispatch( aURL, aArgs2 );
			    }
			    catch ( ::com::sun::star::uno::Exception& )
			    {
				    OUString aMsg = OUString::createFromAscii(
					    "Desktop::OpenDefault() IllegalArgumentException while calling XNotifyingDispatch: ");
				    OSL_ENSURE( sal_False, OUStringToOString(aMsg, RTL_TEXTENCODING_ASCII_US).getStr());
			    }
            }
        }
        else
        {
            INetURLObject aObj( aName );
            if ( aObj.GetProtocol() == INET_PROT_PRIVATE )
                aTarget = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("_default") );

			// Set "AsTemplate" argument according to request type
            if ( aDispatchRequest.aRequestType == REQUEST_FORCENEW ||
				 aDispatchRequest.aRequestType == REQUEST_FORCEOPEN		)
			{
				sal_Int32 nIndex = aArgs.getLength();
				aArgs.realloc( nIndex+1 );
				aArgs[nIndex].Name = aAsTemplateArg;
				if ( aDispatchRequest.aRequestType == REQUEST_FORCENEW )
					aArgs[nIndex].Value <<= sal_True;
				else
					aArgs[nIndex].Value <<= sal_False;
			}

			// if we are called in viewmode, open document read-only
			// #95425#
			if(aDispatchRequest.aRequestType == REQUEST_VIEW) {
				sal_Int32 nIndex = aArgs.getLength();
				aArgs.realloc(nIndex+1);
				aArgs[nIndex].Name = OUString::createFromAscii("ReadOnly");
				aArgs[nIndex].Value <<= sal_True;
			}

            // if we are called with -start set Start in mediadescriptor
			if(aDispatchRequest.aRequestType == REQUEST_START) {
				sal_Int32 nIndex = aArgs.getLength();
				aArgs.realloc(nIndex+1);
				aArgs[nIndex].Name = OUString::createFromAscii("StartPresentation");
				aArgs[nIndex].Value <<= sal_True;
			}

			// This is a synchron loading of a component so we don't have to deal with our statusChanged listener mechanism.

            try
            {
                xDoc = Reference < XPrintable >( xDesktop->loadComponentFromURL( aName, aTarget, 0, aArgs ), UNO_QUERY );
            }
            catch ( ::com::sun::star::lang::IllegalArgumentException& iae)
            {
                OUString aMsg = OUString::createFromAscii(
                    "Dispatchwatcher IllegalArgumentException while calling loadComponentFromURL: ")
                    + iae.Message;
                OSL_ENSURE( sal_False, OUStringToOString(aMsg, RTL_TEXTENCODING_ASCII_US).getStr());
            }
            catch (com::sun::star::io::IOException& ioe)
            {
                OUString aMsg = OUString::createFromAscii(
                    "Dispatchwatcher IOException while calling loadComponentFromURL: ")
                    + ioe.Message;
                OSL_ENSURE( sal_False, OUStringToOString(aMsg, RTL_TEXTENCODING_ASCII_US).getStr());
            }
#ifdef USE_JAVA
            catch (com::sun::star::uno::Exception& e)
            {
                OUString aMsg = OUString::createFromAscii(
                    "Dispatchwatcher IOException while calling loadComponentFromURL: ")
                    + e.Message;
                OSL_ENSURE( sal_False, OUStringToOString(aMsg, RTL_TEXTENCODING_ASCII_US).getStr());
            }
#endif	// USE_JAVA
			if ( aDispatchRequest.aRequestType == REQUEST_OPEN ||
				 aDispatchRequest.aRequestType == REQUEST_VIEW ||
                 aDispatchRequest.aRequestType == REQUEST_START ||
				 aDispatchRequest.aRequestType == REQUEST_FORCEOPEN ||
				 aDispatchRequest.aRequestType == REQUEST_FORCENEW		)
			{
				// request is completed
				OfficeIPCThread::RequestsCompleted( 1 );
			}
			else if ( aDispatchRequest.aRequestType == REQUEST_PRINT ||
					  aDispatchRequest.aRequestType == REQUEST_PRINTTO )
			{
				if ( xDoc.is() )
				{
					if ( aDispatchRequest.aRequestType == REQUEST_PRINTTO )
					{
						// create the printer
						Sequence < PropertyValue > aPrinterArgs( 1 );
						aPrinterArgs[0].Name = ::rtl::OUString::createFromAscii("Name");
						aPrinterArgs[0].Value <<= ::rtl::OUString( aDispatchRequest.aPrinterName );
						xDoc->setPrinter( aPrinterArgs );
					}

					// print ( also without user interaction )
					Sequence < PropertyValue > aPrinterArgs( 1 );
					aPrinterArgs[0].Name = ::rtl::OUString::createFromAscii("Wait");
					aPrinterArgs[0].Value <<= ( sal_Bool ) sal_True;
					xDoc->print( aPrinterArgs );
				}
				else
				{
					// place error message here ...
				}

				// remove the document
				try
				{
					Reference < XCloseable > xClose( xDoc, UNO_QUERY );
					if ( xClose.is() )
						xClose->close( sal_True );
					else
					{
						Reference < XComponent > xComp( xDoc, UNO_QUERY );
						if ( xComp.is() )
							xComp->dispose();
					}
				}
				catch ( com::sun::star::util::CloseVetoException& )
				{
				}

				// request is completed
				OfficeIPCThread::RequestsCompleted( 1 );
			}
		}
	}

    if ( aDispatches.size() > 0 )
	{
		// Execute all asynchronous dispatches now after we placed them into our request container!
		Sequence < PropertyValue > aArgs( 2 );
		aArgs[0].Name = ::rtl::OUString::createFromAscii("Referer");
        aArgs[0].Value <<= ::rtl::OUString::createFromAscii("private:OpenEvent");
		aArgs[1].Name = ::rtl::OUString::createFromAscii("SynchronMode");
        aArgs[1].Value <<= sal_True;

		for ( sal_uInt32 n = 0; n < aDispatches.size(); n++ )
		{
			Reference< XDispatch > xDispatch = aDispatches[n].xDispatch;
            Reference < XNotifyingDispatch > xDisp( xDispatch, UNO_QUERY );
            if ( xDisp.is() )
                xDisp->dispatchWithNotification( aDispatches[n].aURL, aArgs, this );
            else
            {
                ::osl::ClearableMutexGuard aGuard( GetMutex() );
                m_nRequestCount--;
                aGuard.clear();
                xDispatch->dispatch( aDispatches[n].aURL, aArgs );
            }
		}
	}

	::osl::ClearableMutexGuard aGuard( GetMutex() );
    m_nRequestCount--;

	// No more asynchronous requests?
	// The requests are removed from the request container after they called back to this
	// implementation via statusChanged!!
    if ( !m_nRequestCount /*m_aRequestContainer.empty()*/ )
	{
		// We have to check if we have an open task otherwise we have to shutdown the office.
        Reference< XFramesSupplier > xTasksSupplier( xDesktop, UNO_QUERY );
        aGuard.clear();

        Reference< XElementAccess > xList( xTasksSupplier->getFrames(), UNO_QUERY );

        if ( !xList->hasElements() )
		{
			// We don't have any task open so we have to shutdown ourself!!
			Reference< XDesktop > xDesktop2( xTasksSupplier, UNO_QUERY );
			if ( xDesktop2.is() )
				return xDesktop2->terminate();
		}
	}

	return sal_False;
}


void SAL_CALL DispatchWatcher::disposing( const ::com::sun::star::lang::EventObject& )
throw(::com::sun::star::uno::RuntimeException)
{
}


void SAL_CALL DispatchWatcher::dispatchFinished( const DispatchResultEvent& ) throw( RuntimeException )
{
	osl::ClearableMutexGuard aGuard( GetMutex() );
    sal_Int16 nCount = --m_nRequestCount;
    aGuard.clear();
    OfficeIPCThread::RequestsCompleted( 1 );
/*
	// Find request in our hash map and remove it as a pending request
    DispatchWatcherHashMap::iterator pDispatchEntry = m_aRequestContainer.find( rEvent.FeatureURL.Complete ) ;
    if ( pDispatchEntry != m_aRequestContainer.end() )
	{
        m_aRequestContainer.erase( pDispatchEntry );
        aGuard.clear();
		OfficeIPCThread::RequestsCompleted( 1 );
	}
	else
		aGuard.clear();
*/
    if ( !nCount && !OfficeIPCThread::AreRequestsPending() )
	{
		// We have to check if we have an open task otherwise we have to shutdown the office.
        Reference< XFramesSupplier > xTasksSupplier( ::comphelper::getProcessServiceFactory()->createInstance(
													OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
												UNO_QUERY );
        Reference< XElementAccess > xList( xTasksSupplier->getFrames(), UNO_QUERY );

        if ( !xList->hasElements() )
		{
			// We don't have any task open so we have to shutdown ourself!!
			Reference< XDesktop > xDesktop( xTasksSupplier, UNO_QUERY );
			if ( xDesktop.is() )
				xDesktop->terminate();
		}
	}
}

}








