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
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifdef MACOSX

#ifndef _JAVA_DTRANS_JAVA_LANG_OBJECT_HXX
#include <java/lang/Object.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

static ::osl::Mutex aCarbonEventQueueMutex;
static EventHandlerUPP pEventHandlerUPP = NULL;
static EventHandlerRef pEventHandler = NULL;
static DragTrackingHandlerUPP pDragTrackingHandlerUPP = NULL;
static DragTrackingHandlerUPP pDropTrackingHandlerUPP = NULL;
static DragReceiveHandlerUPP pDragReceiveHandlerUPP = NULL;
static EventQueueRef aTrackingEventQueue = NULL;
static EventRef aLastMouseDraggedEvent = NULL;

static OSStatus CarbonEventHandler( EventHandlerCallRef aNextHandler, EventRef aEvent, void *pData );

#endif	// MACOSX

static ::osl::Mutex aDragMutex;
static oslThread pDragThread = NULL;
static ::java::JavaDragSource *pDragThreadOwner = NULL;
static sal_Int8 nCurrentAction = ::com::sun::star::datatransfer::dnd::DNDConstants::ACTION_NONE;
static bool bNoRejectCursor = false;

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace java;
using namespace java::dtrans;
using namespace osl;
using namespace rtl;
using namespace std;

// ========================================================================

#ifdef MACOSX
static OSStatus CarbonEventHandler( EventHandlerCallRef aNextHandler, EventRef aEvent, void *pData )
{
	if ( Application::IsShutDown() )
		return noErr;

	if ( GetEventClass( aEvent ) == kEventClassMouse )
	{
		ClearableMutexGuard aGuard( aCarbonEventQueueMutex );

		if ( GetEventKind( aEvent ) == kEventMouseDragged )
		{
			if ( aLastMouseDraggedEvent )
				ReleaseEvent( aLastMouseDraggedEvent );
			aLastMouseDraggedEvent = RetainEvent( aEvent );

		}

		// Since we are running the TrackDrag() function outside of the main
		// event loop, we must repost this event to the TrackDrag() event queue
		if ( aTrackingEventQueue )
		{
			EventRef aNewEvent = CopyEvent( aEvent );
			PostEventToQueue( aTrackingEventQueue, aNewEvent, kEventPriorityStandard );
		}

		aGuard.clear();
	}

	// Always execute the next registered handler
	return CallNextEventHandler( aNextHandler, aEvent );
}
#endif	// MACOSX

// ------------------------------------------------------------------------

#ifdef MACOSX
static OSErr ImplDragTrackingHandlerCallback( DragTrackingMessage nMessage, WindowRef aWindow, void *pData, DragRef aDrag )
{
	MacOSPoint aPoint;
	Rect aRect;
	if ( pData && GetDragMouse( aDrag, &aPoint, NULL ) == noErr && GetWindowBounds( aWindow, kWindowContentRgn, &aRect ) == noErr )
	{
		if ( nMessage == kDragTrackingLeaveHandler && !bNoRejectCursor )
			SetThemeCursor( kThemeNotAllowedCursor );

		((JavaDragSource *)pData)->handleDrag( (sal_Int32)( aPoint.h - aRect.left ), (sal_Int32)( aPoint.v - aRect.top ) );
	}

	return noErr;
}
#endif	// MACOSX

// ------------------------------------------------------------------------

#ifdef MACOSX
static OSErr ImplDropTrackingHandlerCallback( DragTrackingMessage nMessage, WindowRef aWindow, void *pData, DragRef aDrag )
{
	MacOSPoint aPoint;
	Rect aRect;
	if ( pData && GetDragMouse( aDrag, &aPoint, NULL ) == noErr && GetWindowBounds( aWindow, kWindowContentRgn, &aRect ) == noErr )
	{
		sal_Int32 nX = (sal_Int32)( aPoint.h - aRect.left );
		sal_Int32 nY = (sal_Int32)( aPoint.v - aRect.top );

		switch ( nMessage )
		{
			case kDragTrackingEnterWindow:
				((JavaDropTarget *)pData)->handleDragEnter( nX, nY );
				break;
			case kDragTrackingInWindow:
				((JavaDropTarget *)pData)->handleDragOver( nX, nY );
				break;
			case kDragTrackingLeaveWindow:
				((JavaDropTarget *)pData)->handleDragExit( nX, nY );
				break;
			default:
				break;
		}
	}

	return noErr;
}
#endif	// MACOSX

// ------------------------------------------------------------------------

#ifdef MACOSX
static OSErr ImplDragReceiveHandlerCallback( WindowRef aWindow, void *pData, DragRef aDrag )
{
	MacOSPoint aPoint;
	Rect aRect;
	if ( pData && GetDragMouse( aDrag, &aPoint, NULL ) == noErr && GetWindowBounds( aWindow, kWindowContentRgn, &aRect ) == noErr && ((JavaDropTarget *)pData)->handleDrop( (sal_Int32)( aPoint.h - aRect.left ), (sal_Int32)( aPoint.v - aRect.top ) ) )
		return noErr;

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
	mnActions( DNDConstants::ACTION_NONE ),
	mpNativeWindow( NULL )
{
}

// ------------------------------------------------------------------------

JavaDragSource::~JavaDragSource()
{
	MutexGuard aDragGuard( aDragMutex );

	// If we own the event loop timer, clean up event loop timer
	if ( pDragThreadOwner == this )
	{
		if ( pDragThread )
		{
			osl_joinWithThread( pDragThread );
			pDragThread = NULL;
		}

		pDragThreadOwner = NULL;
	}
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDragSource::initialize( const Sequence< Any >& arguments ) throw( Exception )
{
	if ( arguments.getLength() > 1 )
	{
		sal_Int32 nWindow;
		arguments.getConstArray()[1] >>= nWindow;
		if ( nWindow )
			mpNativeWindow = (void *)nWindow;
	}

#ifdef MACOSX
	if ( !mpNativeWindow || !IsValidWindowPtr( (WindowRef)mpNativeWindow ) )
	{
		mpNativeWindow = NULL;
		throw RuntimeException();
	}

	MutexGuard aDragGuard( aDragMutex );

	if ( !pEventHandlerUPP )
	{
		// Test the JVM version and if it is 1.4 or higher use Cocoa,
		// otherwise use Carbon
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
					EventTypeSpec aTypes[7];
					aTypes[0].eventClass = kEventClassMouse;
					aTypes[0].eventKind = kEventMouseDown;
					aTypes[1].eventClass = kEventClassMouse;
					aTypes[1].eventKind = kEventMouseUp;
					aTypes[2].eventClass = kEventClassMouse;
					aTypes[2].eventKind = kEventMouseMoved;
					aTypes[3].eventClass = kEventClassMouse;
					aTypes[3].eventKind = kEventMouseDragged;
					aTypes[4].eventClass = kEventClassMouse;
					aTypes[4].eventKind = kEventMouseEntered;
					aTypes[5].eventClass = kEventClassMouse;
					aTypes[5].eventKind = kEventMouseExited;
					aTypes[6].eventClass = kEventClassMouse;
					aTypes[6].eventKind = kEventMouseWheelMoved;
					InstallApplicationEventHandler( pEventHandlerUPP, 7, aTypes, NULL, &pEventHandler );
				}
			}
		}
	}
#endif	// MACOSX
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

	MutexGuard aDragGuard( aDragMutex );

	ClearableMutexGuard aGuard( maMutex );

	if ( pDragThreadOwner )
	{
		aGuard.clear();

		if ( listener.is() )
			listener->dragDropEnd( aDragEvent );

		return;
	}

	mnActions = sourceActions;
	maContents = transferable;
	maListener = listener;

	// Test the JVM version and if it is 1.4 or higher use Cocoa, otherwise
	// use Carbon
	pDragThread = osl_createSuspendedThread( runDragExecute, this );
	if ( pDragThread )
	{
		pDragThreadOwner = this;
		osl_resumeThread( pDragThread );
	}
	else
	{
		mnActions = DNDConstants::ACTION_NONE;
		maContents.clear();
		maListener.clear();

		aGuard.clear();

		if ( listener.is() )
			listener->dragDropEnd( aDragEvent );
	}
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

void JavaDragSource::handleDrag( sal_Int32 nX, sal_Int32 nY )
{
	DragSourceDragEvent aSourceDragEvent;
	aSourceDragEvent.Source = static_cast< OWeakObject* >(this);
	aSourceDragEvent.DragSource = static_cast< XDragSource* >(this);

	ClearableMutexGuard aDragGuard( aDragMutex );

	aSourceDragEvent.DropAction = nCurrentAction;
	aSourceDragEvent.UserAction = nCurrentAction;

	aDragGuard.clear();

	ClearableMutexGuard aGuard( maMutex );

	Reference< XDragSourceListener > xListener( maListener );

	aGuard.clear();

	// Send source drag event
	if ( xListener.is() )
		xListener->dragOver( aSourceDragEvent );
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

	aDragMutex.acquire();

	if ( pDragThreadOwner == pSource )
	{
#ifdef MACOSX
		DragRef aDrag;
		if ( NewDrag( &aDrag ) == noErr )
		{

			EventRecord aEventRecord;
			aEventRecord.what = nullEvent;
			aCarbonEventQueueMutex.acquire();
			if ( aLastMouseDraggedEvent )
				ConvertEventRefToEventRecord( aLastMouseDraggedEvent, &aEventRecord );
			aCarbonEventQueueMutex.release();

			if ( aEventRecord.what != nullEvent )
			{
				RgnHandle aRegion = NewRgn();
				if ( aRegion )
				{
					if ( pSource->mnActions & DNDConstants::ACTION_MOVE )
						nCurrentAction = DNDConstants::ACTION_MOVE;
					else if ( pSource->mnActions & DNDConstants::ACTION_COPY )
						nCurrentAction = DNDConstants::ACTION_COPY;
					else if ( pSource->mnActions & DNDConstants::ACTION_LINK )
						nCurrentAction = DNDConstants::ACTION_LINK;
					else
						nCurrentAction = DNDConstants::ACTION_NONE;

					bNoRejectCursor = false;

					if ( !pDragTrackingHandlerUPP )
						pDragTrackingHandlerUPP = NewDragTrackingHandlerUPP( ImplDragTrackingHandlerCallback );

					if ( pDragTrackingHandlerUPP && pSource->mpNativeWindow )
						InstallTrackingHandler( pDragTrackingHandlerUPP, (WindowRef)pSource->mpNativeWindow, pSource );

					// Release mutex while we are in drag
					aDragMutex.release();

					// Cache the current thread's event loop
					aCarbonEventQueueMutex.acquire();
					aTrackingEventQueue = GetCurrentEventQueue();
					aCarbonEventQueueMutex.release();

					if ( TrackDrag( aDrag, &aEventRecord, aRegion ) == noErr )
					{
						aDragMutex.acquire();

						if ( nCurrentAction != DNDConstants::ACTION_NONE )
						{
							aDragEvent.DropAction = nCurrentAction;
							aDragEvent.DropSuccess = sal_True;
						}

						aDragMutex.release();
					}

					aCarbonEventQueueMutex.acquire();
					aTrackingEventQueue = NULL;
					aCarbonEventQueueMutex.release();

					// Relock mutex after drag
					aDragMutex.acquire();

					if ( pDragTrackingHandlerUPP && pSource->mpNativeWindow )
						RemoveTrackingHandler( pDragTrackingHandlerUPP, (WindowRef)pSource->mpNativeWindow );

					bNoRejectCursor = false;
					nCurrentAction = DNDConstants::ACTION_NONE;

					DisposeRgn( aRegion );
				}
			}

			DisposeDrag( aDrag );
		}
#else	// MACOSX
#ifdef DEBUG
		fprintf( stderr, "DropTargetDropContext::acceptDrop not implemented\n" );
#endif
#endif	// MACOSX
	}

	pDragThreadOwner = NULL;

	if ( pDragThread )
		osl_destroyThread( pDragThread );

	aDragMutex.release();

	ClearableMutexGuard aGuard( pSource->maMutex );

	Reference< XDragSourceListener > xListener( pSource->maListener );

	aGuard.clear();

	if ( xListener.is() )
		xListener->dragDropEnd( aDragEvent );
}

// ========================================================================

JavaDropTarget::JavaDropTarget() :
	WeakComponentImplHelper3< XDropTarget, XInitialization, XServiceInfo >( maMutex ),
	mbActive( sal_True ),
	mnDefaultActions( DNDConstants::ACTION_NONE ),
	mpNativeWindow( NULL )
{
}

// ------------------------------------------------------------------------

JavaDropTarget::~JavaDropTarget()
{
	MutexGuard aDragGuard( aDragMutex );

#ifdef MACOSX
	if ( pDropTrackingHandlerUPP && mpNativeWindow )
		RemoveTrackingHandler( pDropTrackingHandlerUPP, (WindowRef)mpNativeWindow );

	if ( pDragReceiveHandlerUPP && mpNativeWindow )
		RemoveReceiveHandler( pDragReceiveHandlerUPP, (WindowRef)mpNativeWindow );
#endif	// MACOSX
}

// --------------------------------------------------------------------------

void SAL_CALL JavaDropTarget::initialize( const Sequence< Any >& arguments ) throw( Exception )
{
	if ( arguments.getLength() > 0 )
	{
		sal_Int32 nWindow;
		arguments.getConstArray()[0] >>= nWindow;
		if ( nWindow )
			mpNativeWindow = (void *)nWindow;
	}

#ifdef MACOSX
	if ( !mpNativeWindow || !IsValidWindowPtr( (WindowRef)mpNativeWindow ) )
	{
		mpNativeWindow = NULL;
		throw RuntimeException();
	}

	MutexGuard aDragGuard( aDragMutex );

	if ( !pDropTrackingHandlerUPP )
		pDropTrackingHandlerUPP = NewDragTrackingHandlerUPP( ImplDropTrackingHandlerCallback );

	if ( pDropTrackingHandlerUPP )
		InstallTrackingHandler( pDropTrackingHandlerUPP, (WindowRef)mpNativeWindow, this );

	if ( !pDragReceiveHandlerUPP )
		pDragReceiveHandlerUPP = NewDragReceiveHandlerUPP( ImplDragReceiveHandlerCallback );

	if ( pDragReceiveHandlerUPP )
		InstallReceiveHandler( pDragReceiveHandlerUPP, (WindowRef)mpNativeWindow, this );
#endif	// MACOSX
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

// ------------------------------------------------------------------------

void JavaDropTarget::handleDragEnter( sal_Int32 nX, sal_Int32 nY )
{
	DropTargetDragEnterEvent aDragEnterEvent;
	aDragEnterEvent.Source = static_cast< XDropTarget* >(this);
	aDragEnterEvent.LocationX = nX;
	aDragEnterEvent.LocationY = nY;
	aDragEnterEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDragEnterEvent.DropAction = DNDConstants::ACTION_NONE;

	aDragMutex.acquire();

	DropTargetDragContext *pContext = new DropTargetDragContext( nCurrentAction );
	aDragEnterEvent.Context = pContext;

	if ( pDragThreadOwner )
	{
		ClearableMutexGuard aDragSourceGuard( pDragThreadOwner->maMutex );

		aDragEnterEvent.SourceActions = pDragThreadOwner->mnActions;
		aDragEnterEvent.DropAction = nCurrentAction;
		aDragEnterEvent.SupportedDataFlavors = pDragThreadOwner->maContents->getTransferDataFlavors();

		aDragSourceGuard.clear();
	}

	aDragMutex.release();

	ClearableMutexGuard aGuard( maMutex );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	aGuard.clear();

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragEnter( aDragEnterEvent );
	}

	aDragMutex.acquire();

	nCurrentAction = pContext->getDragAction();

	aDragMutex.release();
}

// ------------------------------------------------------------------------

void JavaDropTarget::handleDragExit( sal_Int32 nX, sal_Int32 nY )
{
	DropTargetDragEvent aDragEvent;
	aDragEvent.Source = static_cast< XDropTarget* >(this);
	aDragEvent.LocationX = nX;
	aDragEvent.LocationY = nY;
	aDragEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDragEvent.DropAction = DNDConstants::ACTION_NONE;

	aDragMutex.acquire();

	DropTargetDragContext *pContext = new DropTargetDragContext( nCurrentAction );
	aDragEvent.Context = pContext;

	if ( pDragThreadOwner )
	{
		ClearableMutexGuard aDragSourceGuard( pDragThreadOwner->maMutex );

		aDragEvent.SourceActions = pDragThreadOwner->mnActions;
		aDragEvent.DropAction = nCurrentAction;

		aDragSourceGuard.clear();
	}

	aDragMutex.release();

	ClearableMutexGuard aGuard( maMutex );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	aGuard.clear();

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragExit( aDragEvent );
	}

	aDragMutex.acquire();

	nCurrentAction = pContext->getDragAction();

	aDragMutex.release();
}

// ------------------------------------------------------------------------

void JavaDropTarget::handleDragOver( sal_Int32 nX, sal_Int32 nY )
{
	DropTargetDragEvent aDragEvent;
	aDragEvent.Source = static_cast< XDropTarget* >(this);
	aDragEvent.LocationX = nX;
	aDragEvent.LocationY = nY;
	aDragEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDragEvent.DropAction = DNDConstants::ACTION_NONE;

	aDragMutex.acquire();

	DropTargetDragContext *pContext = new DropTargetDragContext( nCurrentAction );
	aDragEvent.Context = pContext;

	if ( pDragThreadOwner )
	{
		ClearableMutexGuard aDragSourceGuard( pDragThreadOwner->maMutex );

		aDragEvent.SourceActions = pDragThreadOwner->mnActions;
		aDragEvent.DropAction = nCurrentAction;

		aDragSourceGuard.clear();
	}

	aDragMutex.release();

	ClearableMutexGuard aGuard( maMutex );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	aGuard.clear();

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragOver( aDragEvent );
	}

	aDragMutex.acquire();

	nCurrentAction = pContext->getDragAction();

	aDragMutex.release();
}

// ------------------------------------------------------------------------

bool JavaDropTarget::handleDrop( sal_Int32 nX, sal_Int32 nY )
{
	DropTargetDropEvent aDropEvent;
	aDropEvent.Source = static_cast< OWeakObject* >(this);
	aDropEvent.LocationX = nX;
	aDropEvent.LocationY = nY;
	aDropEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDropEvent.DropAction = DNDConstants::ACTION_NONE;

	aDragMutex.acquire();

	DropTargetDropContext *pContext = new DropTargetDropContext( nCurrentAction );
	aDropEvent.Context = pContext;

	if ( pDragThreadOwner )
	{
		ClearableMutexGuard aDragSourceGuard( pDragThreadOwner->maMutex );

		aDropEvent.SourceActions = pDragThreadOwner->mnActions;
		aDropEvent.DropAction = nCurrentAction;
		aDropEvent.Transferable = pDragThreadOwner->maContents;

		// Don't set the cursor to the reject cursor since a drop has occurred
		bNoRejectCursor = true;

		aDragSourceGuard.clear();
	}

	aDragMutex.release();

	ClearableMutexGuard aGuard( maMutex );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	aGuard.clear();

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->drop( aDropEvent );
	}

	aDragMutex.acquire();

	nCurrentAction = pContext->getDropAction();

	aDragMutex.release();

	return pContext->getDropComplete();
}
