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

#include <stdio.h>

#ifndef _DTRANS_JAVA_DND_HXX_
#include <java_dnd.hxx>
#endif
#ifndef _DTRANS_JAVA_DNDCONTEXT_HXX
#include <java_dndcontext.hxx>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_DNDCONSTANTS_HPP_
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#endif

#ifdef MACOSX

#ifndef _JAVA_DTRANS_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

static ::osl::Mutex aEventHandlerMutex;
static ULONG nEventHandlerCount = 0;
static EventHandlerUPP pEventHandlerUPP  = NULL;
static EventHandlerRef pEventHandler = NULL;
static EventRef aLastMouseDraggedEvent = NULL;

static OSStatus CarbonEventHandler( EventHandlerCallRef aNextHandler, EventRef aEvent, void *pData );

#endif	// MACOSX

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace java;
using namespace java::dtrans;
using namespace osl;
using namespace rtl;

// ========================================================================

#ifdef MACOSX
static OSStatus CarbonEventHandler( EventHandlerCallRef aNextHandler, EventRef aEvent, void *pData )
{
	EventClass nClass = GetEventClass( aEvent );
	if ( nClass == kEventClassMouse )
	{
		EventKind nKind = GetEventKind( aEvent );
		if ( nKind == kEventMouseDragged )
		{
			ClearableMutexGuard aEventHandlerGuard( aEventHandlerMutex );

			if ( !aLastMouseDraggedEvent )
				aLastMouseDraggedEvent = RetainEvent( aEvent );

			aEventHandlerGuard.clear();
		}
 		else if ( nKind == kEventMouseUp )
		{
			ClearableMutexGuard aEventHandlerGuard( aEventHandlerMutex );

			if ( aLastMouseDraggedEvent )
				ReleaseEvent( aLastMouseDraggedEvent );
			aLastMouseDraggedEvent = NULL;

			aEventHandlerGuard.clear();
		}
	}

	// Always execute the next registered handler
	return CallNextEventHandler( aNextHandler, aEvent );
}
#endif	// MACOSX

// ------------------------------------------------------------------------

#ifdef MACOSX
static OSErr ImplTrackingHandlerCallback( DragTrackingMessage nMessage, WindowRef aWindow, void *pData, DragRef aDrag )
{
#ifdef DEBUG
	fprintf( stderr, "ImplTrackingHandlerCallback not implemented\n" );
#endif
	return noErr;
}
#endif	// MACOSX

// ------------------------------------------------------------------------

#ifdef MACOSX
static OSErr ImplDragReceiveHandlerCallback( WindowRef aWindow, void *pData, DragRef aDrag )
{
#ifdef DEBUG
	fprintf( stderr, "ImplDragReceiveHandlerCallback not implemented\n" );
#endif
	return noErr;
}
#endif	// MACOSX

// ========================================================================
  
namespace java
{

Sequence< OUString > SAL_CALL JavaDragSource_getSupportedServiceNames()
{
    Sequence< OUString > aRet( 1 );
    aRet[0] = OUString::createFromAscii( JAVA_DRAGSOURCE_SERVICE_NAME );
    return aRet;
}
 
// ------------------------------------------------------------------------
 
Reference< XInterface > SAL_CALL JavaDragSource_createInstance( const Reference<
XMultiServiceFactory >& xMultiServiceFactory )
{
	return Reference< XInterface >( static_cast< OWeakObject* >( new JavaDragSource() ) );
}

// ------------------------------------------------------------------------
  
Sequence< OUString > SAL_CALL JavaDropTarget_getSupportedServiceNames()
{
    Sequence< OUString > aRet( 1 );
    aRet[0] = OUString::createFromAscii( JAVA_DROPTARGET_SERVICE_NAME );
    return aRet;
}
 
// ------------------------------------------------------------------------
 
Reference< XInterface > SAL_CALL JavaDropTarget_createInstance( const Reference<
XMultiServiceFactory >& xMultiServiceFactory )
{
	return Reference< XInterface >( static_cast< OWeakObject* >( new JavaDropTarget() ) );
}

}

// ========================================================================

JavaDragSource::JavaDragSource() :
	WeakComponentImplHelper3< XDragSource, XInitialization, XServiceInfo >( maMutex ),
	maDragExecuteThread( NULL ),
	mpNativeDragHandler( NULL ),
	mpNativeDragReference( NULL )
{
#ifdef MACOSX
	ClearableMutexGuard aEventHandlerGuard( aEventHandlerMutex );

	if ( !nEventHandlerCount )
	{
		// Test the JVM version and if it is 1.4 or higher use Cocoa, otherwise
		// use Carbon
		DTransThreadAttach t;
		if ( t.pEnv )
		{
			if ( t.pEnv->GetVersion() < JNI_VERSION_1_4 )
			{
				if ( !pEventHandlerUPP )
					pEventHandlerUPP = NewEventHandlerUPP( CarbonEventHandler );

				if ( pEventHandlerUPP )
				{
					// Set up native event handler
					EventTypeSpec aTypes[2];
					aTypes[0].eventClass = kEventClassMouse;
					aTypes[0].eventKind = kEventMouseDragged;
					aTypes[1].eventClass = kEventClassMouse;
					aTypes[1].eventKind = kEventMouseUp;
					InstallApplicationEventHandler( pEventHandlerUPP, 2, aTypes, NULL, &pEventHandler );
				}
			}
		}
	}

	if ( pEventHandler )
		nEventHandlerCount++;

	aEventHandlerGuard.clear();

	mpNativeDragHandler = (void *)NewDragTrackingHandlerUPP( ImplTrackingHandlerCallback );
	if ( mpNativeDragHandler )
		InstallTrackingHandler( (DragTrackingHandlerUPP)mpNativeDragHandler, NULL, this );
#endif	// MACOSX
}

// ------------------------------------------------------------------------

JavaDragSource::~JavaDragSource()
{
	ClearableMutexGuard aGuard( maMutex );

	if ( maDragExecuteThread )
	{
		osl_terminateThread( maDragExecuteThread );
		osl_joinWithThread( maDragExecuteThread );
		osl_destroyThread( maDragExecuteThread );
	}

	aGuard.clear();

#ifdef MACOSX
	if ( mpNativeDragHandler )
	{
		RemoveTrackingHandler( (DragTrackingHandlerUPP)mpNativeDragHandler, NULL );
		DisposeDragTrackingHandlerUPP( (DragTrackingHandlerUPP)mpNativeDragHandler );
	}

	if ( mpNativeDragReference )
		DisposeDrag( (DragRef)mpNativeDragReference );

	ClearableMutexGuard aEventHandlerGuard( aEventHandlerMutex );

	if ( !--nEventHandlerCount )
	{
		if ( pEventHandler )
			RemoveEventHandler( pEventHandler );

		if ( pEventHandlerUPP )
			DisposeEventHandlerUPP( pEventHandlerUPP );

		if ( aLastMouseDraggedEvent )
		{
			ReleaseEvent( aLastMouseDraggedEvent );
			aLastMouseDraggedEvent = NULL;
		}
	}

	aEventHandlerGuard.clear();
#endif	// MACOSX
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDragSource::initialize( const Sequence< Any >& arguments ) throw( Exception )
{
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaDragSource::isDragImageSupported() throw()
{
	return sal_False;
}

// ------------------------------------------------------------------------

sal_Int32 SAL_CALL JavaDragSource::getDefaultCursor( sal_Int8 dragAction ) throw()
{
	return 0;
}

// ------------------------------------------------------------------------

void JavaDragSource::runDragExecute( void *pData )
{
	JavaDragSource *pSource = (JavaDragSource *)pData;

	DragSourceDropEvent aDragEvent;
	aDragEvent.Source = static_cast< OWeakObject* >(pSource);
	aDragEvent.DragSource = static_cast< XDragSource* >(pSource);
	aDragEvent.DragSourceContext = new DragSourceContext();
	aDragEvent.DropAction = DNDConstants::ACTION_NONE;
	aDragEvent.DropSuccess = sal_False;

#ifdef MACOSX
	if ( !pSource->mpNativeDragReference )
	{
		ClearableMutexGuard aNewDragGuard( pSource->maMutex );

		DragRef aDrag = (DragRef)pSource->mpNativeDragReference;
		if ( NewDrag( &aDrag ) == noErr )
		{
			aNewDragGuard.clear();

			EventRef aEvent = NULL;
			ClearableMutexGuard aEventHandlerGuard( aEventHandlerMutex );
			EventRecord aEventRecord;
			aEventRecord.what = nullEvent;
			if ( aLastMouseDraggedEvent )
				ConvertEventRefToEventRecord( aLastMouseDraggedEvent, &aEventRecord );
			aEventHandlerGuard.clear();

			if ( aEventRecord.what != nullEvent )
			{
				RgnHandle aRegion = NewRgn();
				if ( aRegion )
				{
					TrackDrag( aDrag, &aEventRecord, aRegion );
					DisposeRgn( aRegion );
				}
				ReleaseEvent( aEvent );
			}

			ClearableMutexGuard aDisposeDragGuard( pSource->maMutex );

			DisposeDrag( aDrag );
			pSource->mpNativeDragReference = NULL;

			aDisposeDragGuard.clear();
		}
	}
#else	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "DropTargetDropContext::acceptDrop not implemented\n" );
#endif
#endif	// MACOSX

	ClearableMutexGuard aGuard( pSource->maMutex );

	Reference< XTransferable > xContents( pSource->maContents );
	Reference< XDragSourceListener > xListener( pSource->maListener );

	oslThread aThread = pSource->maDragExecuteThread;
	pSource->maDragExecuteThread = NULL;

	aGuard.clear();

	if ( xListener.is() )
	{
		xContents.clear();
		xListener->dragDropEnd( aDragEvent );
	}

	osl_destroyThread( aThread );
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDragSource::startDrag( const DragGestureEvent& trigger, sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image, const Reference< XTransferable >& transferable, const Reference< XDragSourceListener >& listener ) throw()
{
	ClearableMutexGuard aGuard( maMutex );

	DragSourceDropEvent aDragEvent;
	aDragEvent.Source = static_cast< OWeakObject* >(this);
	aDragEvent.DragSource = static_cast< XDragSource* >(this);
	aDragEvent.DragSourceContext = new DragSourceContext();
	aDragEvent.DropAction = DNDConstants::ACTION_NONE;
	aDragEvent.DropSuccess = sal_False;

	if ( maDragExecuteThread )
	{
		aGuard.clear();

		if ( listener.is() )
			listener->dragDropEnd( aDragEvent );
		return;
	}

	maContents = transferable;
	maListener = listener;

	maDragExecuteThread = osl_createSuspendedThread( JavaDragSource::runDragExecute, this );
	if ( !maDragExecuteThread )
	{
		maContents.clear();
		maListener.clear();

		aGuard.clear();

		if ( listener.is() )
			listener->dragDropEnd( aDragEvent );
	}

	osl_resumeThread( maDragExecuteThread );
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaDragSource::getImplementationName() throw()
{
	return OUString::createFromAscii( JAVA_DRAGSOURCE_IMPL_NAME );
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaDragSource::supportsService( const OUString& serviceName ) throw()
{
	Sequence < OUString > aSupportedServicesNames = JavaDragSource_getSupportedServiceNames();

	for ( sal_Int32 n = aSupportedServicesNames.getLength(); n--; )
		if ( aSupportedServicesNames[n].compareTo(serviceName) == 0 )
			return sal_True;

	return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL JavaDragSource::getSupportedServiceNames() throw()
{
	return JavaDragSource_getSupportedServiceNames();
}

// ========================================================================

JavaDropTarget::JavaDropTarget() :
	WeakComponentImplHelper3< XDropTarget, XInitialization, XServiceInfo >( maMutex ),
	mbActive( sal_False ),
	mnDefaultActions( 0 ),
	mpNativeDropHandler( NULL )
{
#ifdef MACOSX
	mpNativeDropHandler = (void *)NewDragReceiveHandlerUPP( ImplDragReceiveHandlerCallback );
	if ( mpNativeDropHandler )
		InstallReceiveHandler( (DragReceiveHandlerUPP)mpNativeDropHandler, NULL, this );
#endif	// MACOSX
}

// ------------------------------------------------------------------------

JavaDropTarget::~JavaDropTarget()
{
#ifdef MACOSX
	if ( mpNativeDropHandler )
	{
		RemoveReceiveHandler( (DragReceiveHandlerUPP)mpNativeDropHandler, NULL );
		DisposeDragReceiveHandlerUPP( (DragReceiveHandlerUPP)mpNativeDropHandler );
	}
#endif	// MACOSX
}

// --------------------------------------------------------------------------

void SAL_CALL JavaDropTarget::initialize( const Sequence< Any >& arguments ) throw( Exception )
{
}

// --------------------------------------------------------------------------

void SAL_CALL JavaDropTarget::addDropTargetListener( const Reference< XDropTargetListener >& xListener ) throw()
{
	MutexGuard aGuard( maMutex );
	maListeners.push_back( xListener );
}

// --------------------------------------------------------------------------

void SAL_CALL JavaDropTarget::removeDropTargetListener( const Reference< XDropTargetListener >& xListener ) throw()
{
	MutexGuard aGuard( maMutex );
	maListeners.remove( xListener );
}

// --------------------------------------------------------------------------

sal_Bool SAL_CALL JavaDropTarget::isActive() throw()
{
	return mbActive;
}

// --------------------------------------------------------------------------

void SAL_CALL JavaDropTarget::setActive( sal_Bool active ) throw()
{
	MutexGuard aGuard( maMutex );
	mbActive = active;
}

// --------------------------------------------------------------------------

sal_Int8 SAL_CALL JavaDropTarget::getDefaultActions() throw()
{
	return mnDefaultActions;
}

// --------------------------------------------------------------------------

void SAL_CALL JavaDropTarget::setDefaultActions( sal_Int8 actions ) throw()
{
	MutexGuard aGuard( maMutex );
	mnDefaultActions = actions;
}

// --------------------------------------------------------------------------

OUString SAL_CALL JavaDropTarget::getImplementationName() throw()
{
	return OUString::createFromAscii( JAVA_DROPTARGET_IMPL_NAME );
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaDropTarget::supportsService( const OUString& ServiceName ) throw()
{
	Sequence < OUString > aSupportedServicesNames = JavaDropTarget_getSupportedServiceNames();

	for ( sal_Int32 n = aSupportedServicesNames.getLength(); n--; )
		if ( aSupportedServicesNames[n].compareTo(ServiceName) == 0 )
			return sal_True;

	return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL JavaDropTarget::getSupportedServiceNames() throw()
{
	return JavaDropTarget_getSupportedServiceNames();
}
