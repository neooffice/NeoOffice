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
static EventLoopTimerUPP pEventLoopTimerUPP = NULL;
static EventLoopTimerRef pEventLoopTimer = NULL;
static ::java::JavaDragSource *pEventLoopTimerOwner = NULL;

static OSStatus CarbonEventHandler( EventHandlerCallRef aNextHandler, EventRef aEvent, void *pData );
static void CarbonEventLoopTimer( EventLoopTimerRef aTimer, void *pData );

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
static void CarbonEventLoopTimer( EventLoopTimerRef aTimer, void *pData )
{
	((JavaDragSource *)pData)->runDragExecute();
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
	return dragNotAcceptedErr;
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
#ifdef MACOSX
	if ( mpNativeDragHandler )
	{
		RemoveTrackingHandler( (DragTrackingHandlerUPP)mpNativeDragHandler, NULL );
		DisposeDragTrackingHandlerUPP( (DragTrackingHandlerUPP)mpNativeDragHandler );
	}

	if ( mpNativeDragReference )
		DisposeDrag( (DragRef)mpNativeDragReference );

	ClearableMutexGuard aEventHandlerGuard( aEventHandlerMutex );

	// If we are the last instance, clean up event handlers
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

	// If we own the event loop timer, clean up event loop timer
	if ( pEventLoopTimerOwner == this )
	{
		if ( pEventLoopTimer )
		{
			RemoveEventLoopTimer( pEventLoopTimer );
			pEventLoopTimer = NULL;
		}

		if ( pEventLoopTimerUPP )
		{
			DisposeEventLoopTimerUPP( pEventLoopTimerUPP );
			pEventLoopTimerUPP = NULL;
		}

		pEventLoopTimerOwner = NULL;
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

void SAL_CALL JavaDragSource::startDrag( const DragGestureEvent& trigger, sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image, const Reference< XTransferable >& transferable, const Reference< XDragSourceListener >& listener ) throw()
{
	DragSourceDropEvent aDragEvent;
	aDragEvent.Source = static_cast< OWeakObject* >(this);
	aDragEvent.DragSource = static_cast< XDragSource* >(this);
	aDragEvent.DragSourceContext = new DragSourceContext();
	aDragEvent.DropAction = DNDConstants::ACTION_NONE;
	aDragEvent.DropSuccess = sal_False;

	ClearableMutexGuard aGuard( maMutex );

#ifdef MACOSX
	MutexGuard aEventHandlerGuard( aEventHandlerMutex );

	if ( pEventLoopTimerOwner )
	{
		aGuard.clear();

		if ( listener.is() )
			listener->dragDropEnd( aDragEvent );
		return;
	}

	maContents = transferable;
	maListener = listener;

	// Test the JVM version and if it is 1.4 or higher use Cocoa, otherwise
	// use Carbon
	DTransThreadAttach t;
	if ( t.pEnv )
	{
		if ( t.pEnv->GetVersion() < JNI_VERSION_1_4 )
		{
			if ( !pEventLoopTimerUPP )
				pEventLoopTimerUPP = NewEventLoopTimerUPP( CarbonEventLoopTimer );
			if ( pEventLoopTimerUPP )
				InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pEventLoopTimerUPP, this, &pEventLoopTimer );

			if ( !pEventLoopTimer )
			{
				DisposeEventLoopTimerUPP( pEventLoopTimerUPP );

				maContents.clear();
				maListener.clear();

				aGuard.clear();

				if ( listener.is() )
					listener->dragDropEnd( aDragEvent );
			}
			else
			{
				pEventLoopTimerOwner = this;
			}
		}
	}
#else	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "JavaDragSource::startDrag not implemented\n" );
#endif
#endif	// MACOSX
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

// ------------------------------------------------------------------------

void JavaDragSource::runDragExecute()
{
	DragSourceDropEvent aDragEvent;
	aDragEvent.Source = static_cast< OWeakObject* >(this);
	aDragEvent.DragSource = static_cast< XDragSource* >(this);
	aDragEvent.DragSourceContext = new DragSourceContext();
	aDragEvent.DropAction = DNDConstants::ACTION_NONE;
	aDragEvent.DropSuccess = sal_False;

#ifdef MACOSX
	ClearableMutexGuard aTrackingGuard( aEventHandlerMutex );

	if ( pEventLoopTimerOwner == this )
	{
		DragRef aDrag;
		if ( NewDrag( &aDrag ) == noErr )
		{
			EventRecord aEventRecord;
			aEventRecord.what = nullEvent;
			if ( aLastMouseDraggedEvent )
				ConvertEventRefToEventRecord( aLastMouseDraggedEvent, &aEventRecord );

			if ( aEventRecord.what != nullEvent )
			{
				RgnHandle aRegion = NewRgn();
				if ( aRegion )
				{
					if ( TrackDrag( aDrag, &aEventRecord, aRegion ) == noErr )
						;
					DisposeRgn( aRegion );
				}
			}

			DisposeDrag( aDrag );
		}

		if ( pEventLoopTimer )
		{
			RemoveEventLoopTimer( pEventLoopTimer );
			pEventLoopTimer = NULL;
		}

		if ( pEventLoopTimerUPP )
		{
			DisposeEventLoopTimerUPP( pEventLoopTimerUPP );
			pEventLoopTimerUPP = NULL;
		}

		pEventLoopTimerOwner = NULL;
	}

	aTrackingGuard.clear();
#else	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "DropTargetDropContext::acceptDrop not implemented\n" );
#endif
#endif	// MACOSX

	ClearableMutexGuard aGuard( maMutex );

	Reference< XTransferable > xContents( maContents );
	Reference< XDragSourceListener > xListener( maListener );

	aGuard.clear();

	if ( xListener.is() )
	{
		xContents.clear();
		xListener->dragDropEnd( aDragEvent );
	}
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
