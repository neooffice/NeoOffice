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
static EventHandlerUPP pEventHandlerUPP = NULL;
static EventHandlerRef pEventHandler = NULL;
static DragTrackingHandlerUPP pDragTrackingHandlerUPP = NULL;
static DragTrackingHandlerUPP pDropTrackingHandlerUPP = NULL;
static DragReceiveHandlerUPP pDragReceiveHandlerUPP = NULL;
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
using namespace std;

// ========================================================================

#ifdef MACOSX
static OSStatus CarbonEventHandler( EventHandlerCallRef aNextHandler, EventRef aEvent, void *pData )
{
	if ( GetEventClass( aEvent ) == kEventClassMouse && GetEventKind( aEvent ) == kEventMouseDragged )
	{
		ClearableMutexGuard aEventHandlerGuard( aEventHandlerMutex );

		if ( !aLastMouseDraggedEvent )
			aLastMouseDraggedEvent = RetainEvent( aEvent );

		aEventHandlerGuard.clear();
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
static OSErr ImplDragTrackingHandlerCallback( DragTrackingMessage nMessage, WindowRef aWindow, void *pData, DragRef aDrag )
{
	MacOSPoint aPoint;
	Rect aRect;
	if ( pData && GetDragMouse( aDrag, &aPoint, NULL ) == noErr && GetWindowBounds( aWindow, kWindowContentRgn, &aRect ) == noErr )
	{
		switch ( nMessage )
		{
			case kDragTrackingEnterHandler:
			case kDragTrackingEnterWindow:
			case kDragTrackingInWindow:
			case kDragTrackingLeaveWindow:
				SetThemeCursor( kThemeClosedHandCursor );
				break;
			case kDragTrackingLeaveHandler:
				SetThemeCursor( kThemeNotAllowedCursor );
				break;
			default:
				break;
		}

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
#ifdef MACOSX
	MutexGuard aEventHandlerGuard( aEventHandlerMutex );

	// If we own the event loop timer, clean up event loop timer
	if ( pEventLoopTimerOwner == this )
	{
		if ( pEventLoopTimer )
		{
			RemoveEventLoopTimer( pEventLoopTimer );
			pEventLoopTimer = NULL;
		}

		pEventLoopTimerOwner = NULL;
	}
#endif	// MACOSX
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

	MutexGuard aEventHandlerGuard( aEventHandlerMutex );

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
					EventTypeSpec aType;
					aType.eventClass = kEventClassMouse;
					aType.eventKind = kEventMouseDragged;
					InstallApplicationEventHandler( pEventHandlerUPP, 1, &aType, NULL, &pEventHandler );
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

	mnActions = sourceActions;
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

				mnActions = DNDConstants::ACTION_NONE;
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

void JavaDragSource::handleDrag( sal_Int32 nX, sal_Int32 nY )
{
	DragSourceDragEvent aSourceDragEvent;
	aSourceDragEvent.Source = static_cast< OWeakObject* >(this);
	aSourceDragEvent.DragSource = static_cast< XDragSource* >(this);
	aSourceDragEvent.DropAction = ~0;
	aSourceDragEvent.UserAction = mnActions;

	ClearableMutexGuard aGuard( maMutex );

	Reference< XDragSourceListener > xListener( maListener );

	aGuard.clear();

	// Send source drag event
	if ( xListener.is() )
		xListener->dragOver( aSourceDragEvent );
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
					if ( !pDragTrackingHandlerUPP )
						pDragTrackingHandlerUPP = NewDragTrackingHandlerUPP( ImplDragTrackingHandlerCallback );

					if ( pDragTrackingHandlerUPP && mpNativeWindow )
						InstallTrackingHandler( pDragTrackingHandlerUPP, (WindowRef)mpNativeWindow, this );

					if ( TrackDrag( aDrag, &aEventRecord, aRegion ) == noErr )
					{
						aDragEvent.DropAction = ~0;
						aDragEvent.DropSuccess = sal_True;
					}

					if ( pDragTrackingHandlerUPP && mpNativeWindow )
						RemoveTrackingHandler( pDragTrackingHandlerUPP, (WindowRef)mpNativeWindow );

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
		xListener->dragDropEnd( aDragEvent );
}

// ========================================================================

JavaDropTarget::JavaDropTarget() :
	WeakComponentImplHelper3< XDropTarget, XInitialization, XServiceInfo >( maMutex ),
	mbActive( sal_True ),
	mnDefaultActions( 0 ),
	mpNativeWindow( NULL )
{
}

// ------------------------------------------------------------------------

JavaDropTarget::~JavaDropTarget()
{
#ifdef MACOSX
	MutexGuard aEventHandlerGuard( aEventHandlerMutex );

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

	MutexGuard aEventHandlerGuard( aEventHandlerMutex );

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
	aDragEnterEvent.Context = new DropTargetDragContext();
	aDragEnterEvent.LocationX = nX;
	aDragEnterEvent.LocationY = nY;
	aDragEnterEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDragEnterEvent.DropAction = DNDConstants::ACTION_NONE;

#ifdef MACOSX
	ClearableMutexGuard aEventHandlerGuard( aEventHandlerMutex );

	if ( pEventLoopTimerOwner )
	{
		ClearableMutexGuard aDragSourceGuard( pEventLoopTimerOwner->maMutex );

		aDragEnterEvent.SourceActions = pEventLoopTimerOwner->mnActions;
		aDragEnterEvent.DropAction = ~0;
		aDragEnterEvent.SupportedDataFlavors = pEventLoopTimerOwner->maContents->getTransferDataFlavors();

		aDragSourceGuard.clear();
	}

	aEventHandlerGuard.clear();
#else // MACOSX
#ifdef DEBUG
	fprintf( stderr, "JavaDropTarget::handleDragEnter not implemented\n" );
#endif
#endif	// MACOSX

	ClearableMutexGuard aGuard( maMutex );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	aGuard.clear();

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragEnter( aDragEnterEvent );
	}
}

// ------------------------------------------------------------------------

void JavaDropTarget::handleDragExit( sal_Int32 nX, sal_Int32 nY )
{
	DropTargetDragEvent aDragEvent;
	aDragEvent.Source = static_cast< XDropTarget* >(this);
	aDragEvent.Context = new DropTargetDragContext();
	aDragEvent.LocationX = nX;
	aDragEvent.LocationY = nY;
	aDragEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDragEvent.DropAction = DNDConstants::ACTION_NONE;

#ifdef MACOSX
	ClearableMutexGuard aEventHandlerGuard( aEventHandlerMutex );

	if ( pEventLoopTimerOwner )
	{
		ClearableMutexGuard aDragSourceGuard( pEventLoopTimerOwner->maMutex );

		aDragEvent.SourceActions = pEventLoopTimerOwner->mnActions;
		aDragEvent.DropAction = ~0;

		aDragSourceGuard.clear();
	}

	aEventHandlerGuard.clear();
#else // MACOSX
#ifdef DEBUG
	fprintf( stderr, "JavaDropTarget::handleDragExit not implemented\n" );
#endif
#endif	// MACOSX

	ClearableMutexGuard aGuard( maMutex );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	aGuard.clear();

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragOver( aDragEvent );
	}
}

// ------------------------------------------------------------------------

void JavaDropTarget::handleDragOver( sal_Int32 nX, sal_Int32 nY )
{
	DropTargetDragEvent aDragEvent;
	aDragEvent.Source = static_cast< XDropTarget* >(this);
	aDragEvent.Context = new DropTargetDragContext();
	aDragEvent.LocationX = nX;
	aDragEvent.LocationY = nY;
	aDragEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDragEvent.DropAction = DNDConstants::ACTION_NONE;

#ifdef MACOSX
	ClearableMutexGuard aEventHandlerGuard( aEventHandlerMutex );

	if ( pEventLoopTimerOwner )
	{
		ClearableMutexGuard aDragSourceGuard( pEventLoopTimerOwner->maMutex );

		aDragEvent.SourceActions = pEventLoopTimerOwner->mnActions;
		aDragEvent.DropAction = ~0;

		aDragSourceGuard.clear();
	}

	aEventHandlerGuard.clear();
#else // MACOSX
#ifdef DEBUG
	fprintf( stderr, "JavaDropTarget::handleDragOver not implemented\n" );
#endif
#endif	// MACOSX

	ClearableMutexGuard aGuard( maMutex );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	aGuard.clear();

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragOver( aDragEvent );
	}
}

// ------------------------------------------------------------------------

bool JavaDropTarget::handleDrop( sal_Int32 nX, sal_Int32 nY )
{
	DropTargetDropEvent aDropEvent;
	aDropEvent.Source = static_cast< OWeakObject* >(this);
	aDropEvent.Context = new DropTargetDropContext();
	aDropEvent.LocationX = nX;
	aDropEvent.LocationY = nY;
	aDropEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDropEvent.DropAction = DNDConstants::ACTION_NONE;

#ifdef MACOSX
	ClearableMutexGuard aEventHandlerGuard( aEventHandlerMutex );

	if ( pEventLoopTimerOwner )
	{
		ClearableMutexGuard aDragSourceGuard( pEventLoopTimerOwner->maMutex );

		aDropEvent.SourceActions = pEventLoopTimerOwner->mnActions;
		aDropEvent.DropAction = ~0;
		aDropEvent.Transferable = pEventLoopTimerOwner->maContents;

		aDragSourceGuard.clear();
	}

	aEventHandlerGuard.clear();
#else // MACOSX
#ifdef DEBUG
	fprintf( stderr, "JavaDropTarget::handleDrop not implemented\n" );
#endif
#endif	// MACOSX

	ClearableMutexGuard aGuard( maMutex );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	aGuard.clear();

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->drop( aDropEvent );
	}

	// TODO: return false if one of the drop() methods causes the event's
	// DropTargetDropContext.rejectDrop() method to be called
	return true;
}
