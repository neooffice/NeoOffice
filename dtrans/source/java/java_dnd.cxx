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
#include "java_dnd.hxx"
#endif
#ifndef _DTRANS_JAVA_DNDCONTEXT_HXX
#include "java_dndcontext.hxx"
#endif
#ifndef _DTRANSTRANSFERABLE_HXX
#include "DTransTransferable.hxx"
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_DNDCONSTANTS_HPP_
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_SYSDATA_HXX
#include <vcl/sysdata.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace java;
using namespace osl;
using namespace rtl;
using namespace std;
using namespace vos;

static EventLoopTimerUPP pTrackDragTimerUPP = NULL;
static DragTrackingHandlerUPP pDragTrackingHandlerUPP = NULL;
static DragTrackingHandlerUPP pDropTrackingHandlerUPP = NULL;
static DragReceiveHandlerUPP pDragReceiveHandlerUPP = NULL;
static ::std::list< ::java::JavaDragSource* > aDragSources;
static ::std::list< ::java::JavaDropTarget* > aDropTargets;
static JavaDragSource *pTrackDragOwner = NULL;
static sal_Int8 nCurrentAction = DNDConstants::ACTION_NONE;
static sal_Int8 nStartingAction = DNDConstants::ACTION_NONE;
static bool bNoRejectCursor = true;

// ========================================================================

static void ImplSetThemeCursor( sal_Int8 nAction, bool bPointInWindow )
{
	if ( bPointInWindow )
	{
		if ( !bNoRejectCursor )
		{
			if ( nAction & DNDConstants::ACTION_MOVE )
				SetThemeCursor( kThemeClosedHandCursor );
			else if ( nAction & DNDConstants::ACTION_COPY )
				SetThemeCursor( kThemeCopyArrowCursor );
			else if ( nAction & DNDConstants::ACTION_LINK )
				SetThemeCursor( kThemeAliasArrowCursor );
			else
				SetThemeCursor( kThemeNotAllowedCursor );
		}
	}
	else
	{
		SetThemeCursor( kThemeArrowCursor );
	}
}

// ------------------------------------------------------------------------

static sal_Int8 ImplGetDragDropAction( DragRef aDrag )
{
	sal_Int8 nActions = DNDConstants::ACTION_NONE;

	DragActions nDragActions;
	if ( GetDragDropAction( aDrag, &nDragActions ) == noErr )
	{
		if ( nDragActions & kDragActionMove )
			nActions = DNDConstants::ACTION_MOVE;
		else if ( nDragActions & ( kDragActionCopy | kDragActionGeneric ) )
			nActions = DNDConstants::ACTION_COPY;
		else if ( nDragActions & kDragActionAlias )
			nActions = DNDConstants::ACTION_LINK;
	}

	SInt16 nKeyModifiers;
	if ( GetDragModifiers( aDrag, &nKeyModifiers, NULL, NULL ) != noErr || ! ( nKeyModifiers & ( shiftKey | cmdKey ) ) )
		nActions |= DNDConstants::ACTION_DEFAULT;

	return nActions;
}

// ------------------------------------------------------------------------

static void ImplSetDragDropAction( DragRef aDrag, sal_Int8 nActions )
{
	if ( nActions & DNDConstants::ACTION_MOVE )
		SetDragDropAction( aDrag, kDragActionMove );
	else if ( nActions & DNDConstants::ACTION_COPY )
		SetDragDropAction( aDrag, kDragActionCopy );
	else if ( nActions & DNDConstants::ACTION_LINK )
		SetDragDropAction( aDrag, kDragActionAlias );
	else
		SetDragDropAction( aDrag, kDragActionNothing );
}

// ------------------------------------------------------------------------

static sal_Int8 ImplGetDragAllowableActions( DragRef aDrag )
{
	sal_Int8 nActions = DNDConstants::ACTION_NONE;

	DragActions nDragActions;
	if ( GetDragAllowableActions( aDrag, &nDragActions ) == noErr )
	{
		if ( nDragActions & ( kDragActionMove | kDragActionGeneric ) )
			nActions |= DNDConstants::ACTION_MOVE;
		if ( nDragActions & ( kDragActionCopy | kDragActionGeneric ) )
			nActions |= DNDConstants::ACTION_COPY;
		if ( nDragActions & ( kDragActionAlias | kDragActionGeneric ) )
			nActions |= DNDConstants::ACTION_LINK;
	}

	SInt16 nKeyModifiers;
	if ( GetDragModifiers( aDrag, &nKeyModifiers, NULL, NULL ) != noErr || ! ( nKeyModifiers & ( shiftKey | cmdKey ) ) )
		nActions |= DNDConstants::ACTION_DEFAULT;

	return nActions;
}

// ------------------------------------------------------------------------

static void ImplSetDragAllowableActions( DragRef aDrag, sal_Int8 nActions )
{
	DragActions nDragActions = kDragActionGeneric;
	if ( nActions & DNDConstants::ACTION_MOVE )
		nDragActions |= kDragActionMove;
	if ( nActions & DNDConstants::ACTION_COPY )
		nDragActions |= kDragActionCopy;
	if ( nActions & DNDConstants::ACTION_LINK )
		nDragActions |= kDragActionAlias;
	SetDragAllowableActions( aDrag, nDragActions, false );
}

// ------------------------------------------------------------------------

static OSErr ImplDragTrackingHandlerCallback( DragTrackingMessage nMessage, WindowRef aWindow, void *pData, DragRef aDrag )
{
	if ( !IsValidWindowPtr( aWindow ) )
		return noErr;

	if ( !Application::IsShutDown() )
	{
		// We need to let any pending timers run so that we don't deadlock
		IMutex& rSolarMutex = Application::GetSolarMutex();
		bool bAcquired = false;
		TimeValue aDelay;
		aDelay.Seconds = 0;
		aDelay.Nanosec = 10;
		while ( !Application::IsShutDown() )
		{
			if ( rSolarMutex.tryToAcquire() )
			{
				if ( IsValidWindowPtr( aWindow ) && !Application::IsShutDown() )
					bAcquired = true;
				else
					rSolarMutex.release();
				break;
			}

			ReceiveNextEvent( 0, NULL, 0, false, NULL );
			OThread::wait( aDelay );
		}

		if ( bAcquired )
		{
			if ( pTrackDragOwner )
			{
				JavaDragSource *pSource = NULL;
				for ( ::std::list< JavaDragSource* >::const_iterator it = aDragSources.begin(); it != aDragSources.end(); ++it )
				{
					if ( (*it)->getNativeWindow() == aWindow )
					{
						pSource = *it;
						break;
					}
				}

				if ( pSource && pSource != pTrackDragOwner )
				{
					MacOSPoint aPoint;
					Rect aRect;
					WindowRef aTrackDragOwnerWindow = pTrackDragOwner->getNativeWindow();
					if ( aTrackDragOwnerWindow && GetDragMouse( aDrag, &aPoint, NULL ) == noErr && GetWindowBounds( aTrackDragOwnerWindow, kWindowContentRgn, &aRect ) == noErr )
						pTrackDragOwner->handleDrag( (sal_Int32)( aPoint.h - aRect.left ), (sal_Int32)( aPoint.v - aRect.top ) );
				}
			}

			rSolarMutex.release();
		}
	}
	
	return noErr;
}

// ------------------------------------------------------------------------

static OSErr ImplDropTrackingHandlerCallback( DragTrackingMessage nMessage, WindowRef aWindow, void *pData, DragRef aDrag )
{
	if ( !IsValidWindowPtr( aWindow ) )
		return noErr;

	if ( !Application::IsShutDown() )
	{
		// We need to let any pending timers run so that we don't deadlock
		IMutex& rSolarMutex = Application::GetSolarMutex();
		bool bAcquired = false;
		TimeValue aDelay;
		aDelay.Seconds = 0;
		aDelay.Nanosec = 10;
		while ( !Application::IsShutDown() )
		{
			if ( rSolarMutex.tryToAcquire() )
			{
				if ( IsValidWindowPtr( aWindow ) && !Application::IsShutDown() )
					bAcquired = true;
				else
					rSolarMutex.release();
				break;
			}

			ReceiveNextEvent( 0, NULL, 0, false, NULL );
			OThread::wait( aDelay );
		}

		if ( bAcquired )
		{
			JavaDropTarget *pTarget = NULL;
			for ( ::std::list< JavaDropTarget* >::const_iterator it = aDropTargets.begin(); it != aDropTargets.end(); ++it )
			{
				if ( (*it)->getNativeWindow() == aWindow )
				{
					pTarget = *it;
					break;
				}
			}

			if ( pTarget )
			{
				MacOSPoint aPoint;
				Rect aRect;
				if ( GetDragMouse( aDrag, &aPoint, NULL ) == noErr && GetWindowBounds( aWindow, kWindowContentRgn, &aRect ) == noErr )
				{
					sal_Int32 nX = (sal_Int32)( aPoint.h - aRect.left );
					sal_Int32 nY = (sal_Int32)( aPoint.v - aRect.top );

					switch ( nMessage )
					{
						case kDragTrackingEnterWindow:
							pTarget->handleDragEnter( nX, nY, aDrag );
							break;
						case kDragTrackingInWindow:
							pTarget->handleDragOver( nX, nY, aDrag );
							break;
						case kDragTrackingLeaveWindow:
							pTarget->handleDragExit( nX, nY, aDrag );
							break;
						default:
							break;
					}

					ImplSetThemeCursor( pTarget->isRejected() ? DNDConstants::ACTION_NONE : nCurrentAction, PtInRect( aPoint, &aRect ) );
				}
			}

			rSolarMutex.release();
		}
	}

	return noErr;
}

// ------------------------------------------------------------------------

static OSErr ImplDragReceiveHandlerCallback( WindowRef aWindow, void *pData, DragRef aDrag )
{
	OSErr nRet = dragNotAcceptedErr;

	if ( !IsValidWindowPtr( aWindow ) )
		return nRet;

	if ( !Application::IsShutDown() )
	{
		// We need to let any pending timers run so that we don't deadlock
		IMutex& rSolarMutex = Application::GetSolarMutex();
		bool bAcquired = false;
		TimeValue aDelay;
		aDelay.Seconds = 0;
		aDelay.Nanosec = 10;
		while ( !Application::IsShutDown() )
		{
			if ( rSolarMutex.tryToAcquire() )
			{
				if ( IsValidWindowPtr( aWindow ) && !Application::IsShutDown() )
					bAcquired = true;
				else
					rSolarMutex.release();
				break;
			}

			ReceiveNextEvent( 0, NULL, 0, false, NULL );
			OThread::wait( aDelay );
		}

		if ( bAcquired )
		{
			JavaDropTarget *pTarget = NULL;
			for ( ::std::list< JavaDropTarget* >::const_iterator it = aDropTargets.begin(); it != aDropTargets.end(); ++it )
			{
				if ( (*it)->getNativeWindow() == aWindow )
				{
					pTarget = *it;
					break;
				}
			}

			if ( pTarget )
			{
				MacOSPoint aPoint;
				Rect aRect;
				if ( GetDragMouse( aDrag, &aPoint, NULL ) == noErr && GetWindowBounds( aWindow, kWindowContentRgn, &aRect ) == noErr && pTarget->handleDrop( (sal_Int32)( aPoint.h - aRect.left ), (sal_Int32)( aPoint.v - aRect.top ), aDrag ) )
				{
					// Update actions
					ImplSetDragDropAction( aDrag, nCurrentAction );
					ImplSetThemeCursor( pTarget->isRejected() ? DNDConstants::ACTION_NONE : nCurrentAction, false );
					nRet = noErr;
				}
			}

			rSolarMutex.release();
		}
	}

	return nRet;
}

// ------------------------------------------------------------------------

void TrackDragTimerCallback( EventLoopTimerRef aTimer, void *pData )
{
	JavaDragSource *pSource = (JavaDragSource *)pData;

	IMutex& rSolarMutex = Application::GetSolarMutex();
	TimeValue aDelay;
	aDelay.Seconds = 0;
	aDelay.Nanosec = 10;
	while ( !rSolarMutex.tryToAcquire() )
	{
		ReceiveNextEvent( 0, NULL, 0, false, NULL );
		OThread::wait( aDelay );
	}

	if ( pSource && pTrackDragOwner == pSource )
	{
		DragRef aDrag;
		if ( NewDrag( &aDrag ) == noErr )
		{
			EventRecord aEventRecord;
			aEventRecord.what = mouseDown;
			aEventRecord.message = mouseMovedMessage;
			GetGlobalMouse( &aEventRecord.where );
			aEventRecord.modifiers = GetCurrentKeyModifiers();

			if ( ( aEventRecord.modifiers & shiftKey ) && ! ( aEventRecord.modifiers & cmdKey ) )
				nCurrentAction = DNDConstants::ACTION_MOVE;
			else if ( ! ( aEventRecord.modifiers & shiftKey ) && ( aEventRecord.modifiers & cmdKey ) )
				nCurrentAction = DNDConstants::ACTION_COPY;
			else if ( aEventRecord.modifiers & ( shiftKey | cmdKey ) )
				nCurrentAction = DNDConstants::ACTION_LINK;
			else if ( pSource->mnActions & DNDConstants::ACTION_MOVE )
				nCurrentAction = DNDConstants::ACTION_MOVE;
			else if ( pSource->mnActions & DNDConstants::ACTION_COPY )
				nCurrentAction = DNDConstants::ACTION_COPY;
			else if ( pSource->mnActions & DNDConstants::ACTION_LINK )
				nCurrentAction = DNDConstants::ACTION_LINK;
			else 
				nCurrentAction = DNDConstants::ACTION_NONE;
 
			if ( ! ( aEventRecord.modifiers & ( shiftKey | cmdKey ) ) )
				nCurrentAction |= DNDConstants::ACTION_DEFAULT;

			nStartingAction = nCurrentAction;

			RgnHandle aRegion = NewRgn();
			if ( aRegion )
			{
				bNoRejectCursor = false;

				ImplSetDragAllowableActions( aDrag, nCurrentAction );
				ImplSetThemeCursor( nCurrentAction, true );

				aDragSources.push_back( pSource );

				// Set the drag's transferable
				DTransTransferable *pTransferable = new DTransTransferable( aDrag, TRANSFERABLE_TYPE_DRAG );

				bool bContentsSet = ( pSource->maContents.is() && pTransferable->setContents( pSource->maContents ) );

				// Unlock application mutex while we are in the drag
				rSolarMutex.release();

				bool bTrackDrag = ( bContentsSet && TrackDrag( aDrag, &aEventRecord, aRegion ) == noErr );

				// Relock application mutex. Note that we don't check for
				// application shutdown as we are noew on the hook to clean up
				// this thread
				while ( !rSolarMutex.tryToAcquire() )
				{
					ReceiveNextEvent( 0, NULL, 0, false, NULL );
					OThread::wait( aDelay );
				}

				if ( pSource && pTrackDragOwner == pSource )
				{
					DragSourceDropEvent *pDragEvent = new DragSourceDropEvent();
					pDragEvent->Source = Reference< XInterface >( static_cast< OWeakObject* >( pSource ) );
					pDragEvent->DragSource = Reference< XDragSource >( pSource );
					pDragEvent->DragSourceContext = Reference< XDragSourceContext >( new DragSourceContext() );
					pDragEvent->DropAction = DNDConstants::ACTION_NONE;
					pDragEvent->DropSuccess = sal_False;

					if ( bTrackDrag )
					{
						nCurrentAction = ImplGetDragDropAction( aDrag );
						if ( nCurrentAction != DNDConstants::ACTION_NONE )
						{
							pDragEvent->DropAction = nCurrentAction;
							pDragEvent->DropSuccess = sal_True;
						}
					}

					// Fix bug 1442 by dispatching and deleting the
					// DragSourceDropEvent in the VCL event dispatch thread
					Application::PostUserEvent( STATIC_LINK( NULL, JavaDragSource, dragDropEnd ), pDragEvent );
				}

				delete pTransferable;

				aDragSources.remove( pSource );

				DisposeRgn( aRegion );
			}

			DisposeDrag( aDrag );
		}
	}

	rSolarMutex.release();
}

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

IMPL_STATIC_LINK( JavaDragSource, dragDropEnd, void*, pData )
{
	DragSourceDropEvent *pDragEvent = (DragSourceDropEvent *)pData;

	if ( pDragEvent )
	{
		JavaDragSource *pSource = (JavaDragSource*)pDragEvent->DragSource.get();

		if ( pSource && pTrackDragOwner == pSource )
		{
			Reference< XDragSourceListener > xListener( pSource->maListener );
			if ( xListener.is() )
				xListener->dragDropEnd( *pDragEvent );

			pTrackDragOwner = NULL;
		}

		delete pDragEvent;
	}
}

// ------------------------------------------------------------------------

JavaDragSource::JavaDragSource() :
	WeakComponentImplHelper3< XDragSource, XInitialization, XServiceInfo >( maMutex ),
	mnActions( DNDConstants::ACTION_NONE ),
	mpEnvData( NULL ),
	mpWindow( NULL )
{
}

// ------------------------------------------------------------------------

JavaDragSource::~JavaDragSource()
{
	// If we own the event loop timer, wait for the timer to finish
	if ( pTrackDragOwner == this )
		pTrackDragOwner = NULL;

	aDragSources.remove( this );
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDragSource::initialize( const Sequence< Any >& arguments ) throw( RuntimeException )
{
	if ( arguments.getLength() > 1 )
	{
		sal_Int32 nPtr;
		arguments.getConstArray()[0] >>= nPtr;
		if ( nPtr )
			mpEnvData = (SystemEnvData *)nPtr;
		arguments.getConstArray()[1] >>= nPtr;
		if ( nPtr )
			mpWindow = (Window *)nPtr;
	}

	if ( !mpEnvData || !mpWindow )
		throw RuntimeException();

	if ( !pDragTrackingHandlerUPP )
	{
		pDragTrackingHandlerUPP = NewDragTrackingHandlerUPP( ImplDragTrackingHandlerCallback );
		if ( pDragTrackingHandlerUPP )
			InstallTrackingHandler( pDragTrackingHandlerUPP, NULL, NULL );
	}
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaDragSource::isDragImageSupported() throw( com::sun::star::uno::RuntimeException )
{
	return sal_False;
}

// ------------------------------------------------------------------------

sal_Int32 SAL_CALL JavaDragSource::getDefaultCursor( sal_Int8 dragAction ) throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException )
{
	return 0;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDragSource::startDrag( const DragGestureEvent& trigger, sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image, const Reference< XTransferable >& transferable, const Reference< XDragSourceListener >& listener ) throw( com::sun::star::uno::RuntimeException )
{
	DragSourceDropEvent aDragEvent;
	aDragEvent.Source = Reference< XInterface >( static_cast< OWeakObject* >( this ) );
	aDragEvent.DragSource = Reference< XDragSource >( this );
	aDragEvent.DragSourceContext = Reference< XDragSourceContext >( new DragSourceContext() );
	aDragEvent.DropAction = DNDConstants::ACTION_NONE;
	aDragEvent.DropSuccess = sal_False;

	if ( pTrackDragOwner )
	{
		if ( listener.is() )
			listener->dragDropEnd( aDragEvent );

		return;
	}

	mnActions = sourceActions;
	maContents = transferable;
	maListener = listener;

	if ( !pTrackDragTimerUPP )
		pTrackDragTimerUPP = NewEventLoopTimerUPP( TrackDragTimerCallback );

	if ( maContents.is() && pTrackDragTimerUPP )
	{
		pTrackDragOwner = this;
		InstallEventLoopTimer( GetMainEventLoop(), 0.001, kEventDurationForever, pTrackDragTimerUPP, (void *)this, NULL );
	}
	else
	{
		mnActions = DNDConstants::ACTION_NONE;
		maContents.clear();
		maListener.clear();

		if ( listener.is() )
			listener->dragDropEnd( aDragEvent );
	}
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaDragSource::getImplementationName() throw( RuntimeException )
{
	return OUString::createFromAscii( JAVA_DRAGSOURCE_IMPL_NAME );
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaDragSource::supportsService( const OUString& serviceName ) throw( RuntimeException )
{
	Sequence < OUString > aSupportedServicesNames = JavaDragSource_getSupportedServiceNames();

	for ( sal_Int32 n = aSupportedServicesNames.getLength(); n--; )
		if ( aSupportedServicesNames[n].compareTo(serviceName) == 0 )
			return sal_True;

	return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL JavaDragSource::getSupportedServiceNames() throw( RuntimeException )
{
	return JavaDragSource_getSupportedServiceNames();
}

// ------------------------------------------------------------------------

WindowRef JavaDragSource::getNativeWindow()
{
	if ( mpEnvData )
		return (WindowRef)mpEnvData->aWindow;
	else
		return NULL;
}

// ------------------------------------------------------------------------

void JavaDragSource::handleDrag( sal_Int32 nX, sal_Int32 nY )
{
	DragSourceDragEvent aSourceDragEvent;
	aSourceDragEvent.Source = Reference< XInterface >( static_cast< OWeakObject* >( this ) );
	aSourceDragEvent.DragSource = Reference< XDragSource >( this );

	aSourceDragEvent.DropAction = mnActions;
	aSourceDragEvent.UserAction = nCurrentAction;

	Reference< XDragSourceListener > xListener( maListener );

	// Send source drag event
	if ( xListener.is() )
		xListener->dragOver( aSourceDragEvent );
}

// ========================================================================

JavaDropTarget::JavaDropTarget() :
	WeakComponentImplHelper3< XDropTarget, XInitialization, XServiceInfo >( maMutex ),
	mbActive( sal_True ),
	mnDefaultActions( DNDConstants::ACTION_NONE ),
	mbRejected( false ),
	mpEnvData( NULL ),
	mpWindow( NULL )
{
}

// ------------------------------------------------------------------------

JavaDropTarget::~JavaDropTarget()
{
	disposing();
}

// --------------------------------------------------------------------------

void JavaDropTarget::disposing()
{
	mpEnvData = NULL;
	mpWindow = NULL;
	aDropTargets.remove( this );
}

// --------------------------------------------------------------------------

void SAL_CALL JavaDropTarget::initialize( const Sequence< Any >& arguments ) throw( RuntimeException )
{
	if ( arguments.getLength() > 1 )
	{
		sal_Int32 nPtr;
		arguments.getConstArray()[0] >>= nPtr;
		if ( nPtr )
			mpEnvData = (SystemEnvData *)nPtr;
		arguments.getConstArray()[1] >>= nPtr;
		if ( nPtr )
			mpWindow = (Window *)nPtr;
	}

	if ( !mpEnvData || !mpWindow )
		throw RuntimeException();

	aDropTargets.push_back( this );

	if ( !pDropTrackingHandlerUPP )
	{
		pDropTrackingHandlerUPP = NewDragTrackingHandlerUPP( ImplDropTrackingHandlerCallback );
		if ( pDropTrackingHandlerUPP )
			InstallTrackingHandler( pDropTrackingHandlerUPP, NULL, NULL );
	}

	if ( !pDragReceiveHandlerUPP )
	{
		pDragReceiveHandlerUPP = NewDragReceiveHandlerUPP( ImplDragReceiveHandlerCallback );
		if ( pDragReceiveHandlerUPP )
			InstallReceiveHandler( pDragReceiveHandlerUPP, NULL, NULL );
	}
}

// --------------------------------------------------------------------------

void SAL_CALL JavaDropTarget::addDropTargetListener( const Reference< XDropTargetListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException )
{
	maListeners.push_back( xListener );
}

// --------------------------------------------------------------------------

void SAL_CALL JavaDropTarget::removeDropTargetListener( const Reference< XDropTargetListener >& xListener ) throw( ::com::sun::star::uno::RuntimeException )
{
	maListeners.remove( xListener );
}

// --------------------------------------------------------------------------

sal_Bool SAL_CALL JavaDropTarget::isActive() throw( ::com::sun::star::uno::RuntimeException )
{
	return mbActive;
}

// --------------------------------------------------------------------------

void SAL_CALL JavaDropTarget::setActive( sal_Bool active ) throw( ::com::sun::star::uno::RuntimeException )
{
	mbActive = active;
}

// --------------------------------------------------------------------------

sal_Int8 SAL_CALL JavaDropTarget::getDefaultActions() throw( ::com::sun::star::uno::RuntimeException )
{
	return mnDefaultActions;
}

// --------------------------------------------------------------------------

void SAL_CALL JavaDropTarget::setDefaultActions( sal_Int8 actions ) throw( ::com::sun::star::uno::RuntimeException )
{
	mnDefaultActions = actions;
}

// --------------------------------------------------------------------------

OUString SAL_CALL JavaDropTarget::getImplementationName() throw( RuntimeException )
{
	return OUString::createFromAscii( JAVA_DROPTARGET_IMPL_NAME );
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaDropTarget::supportsService( const OUString& ServiceName ) throw( RuntimeException )
{
	Sequence < OUString > aSupportedServicesNames = JavaDropTarget_getSupportedServiceNames();

	for ( sal_Int32 n = aSupportedServicesNames.getLength(); n--; )
		if ( aSupportedServicesNames[n].compareTo(ServiceName) == 0 )
			return sal_True;

	return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL JavaDropTarget::getSupportedServiceNames() throw( RuntimeException )
{
	return JavaDropTarget_getSupportedServiceNames();
}

// ------------------------------------------------------------------------

WindowRef JavaDropTarget::getNativeWindow()
{
	if ( mpEnvData )
		return (WindowRef)mpEnvData->aWindow;
	else
		return NULL;
}

// ------------------------------------------------------------------------

void JavaDropTarget::handleDragEnter( sal_Int32 nX, sal_Int32 nY, DragRef aNativeTransferable )
{
	mbRejected = false;

	DropTargetDragEnterEvent aDragEnterEvent;
	aDragEnterEvent.Source = Reference< XInterface >( static_cast< OWeakObject* >( this ) );
	aDragEnterEvent.LocationX = nX;
	aDragEnterEvent.LocationY = nY;
	aDragEnterEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDragEnterEvent.DropAction = DNDConstants::ACTION_NONE;

	if ( pTrackDragOwner )
	{
		aDragEnterEvent.SourceActions = pTrackDragOwner->mnActions;
		aDragEnterEvent.DropAction = nStartingAction;
		aDragEnterEvent.SupportedDataFlavors = pTrackDragOwner->maContents->getTransferDataFlavors();
	}
	else if ( aNativeTransferable )
	{
		aDragEnterEvent.SourceActions = ImplGetDragAllowableActions( aNativeTransferable );
		aDragEnterEvent.DropAction = ImplGetDragDropAction( aNativeTransferable );

		DTransTransferable *pTransferable = new DTransTransferable( aNativeTransferable, TRANSFERABLE_TYPE_DRAG );
		if ( pTransferable )
		{
			aDragEnterEvent.SupportedDataFlavors = pTransferable->getTransferDataFlavors();
			delete pTransferable;
		}
	}

	DropTargetDragContext *pContext = new DropTargetDragContext( aDragEnterEvent.DropAction );
	aDragEnterEvent.Context = Reference< XDropTargetDragContext >( pContext );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragEnter( aDragEnterEvent );
	}

	nCurrentAction = pContext->getDragAction();
	mbRejected = pContext->isRejected();
}

// ------------------------------------------------------------------------

void JavaDropTarget::handleDragExit( sal_Int32 nX, sal_Int32 nY, DragRef aNativeTransferable )
{
	DropTargetDragEvent aDragEvent;
	aDragEvent.Source = Reference< XInterface >( static_cast< OWeakObject* >( this ) );
	aDragEvent.LocationX = nX;
	aDragEvent.LocationY = nY;
	aDragEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDragEvent.DropAction = DNDConstants::ACTION_NONE;

	if ( pTrackDragOwner )
	{
		aDragEvent.SourceActions = pTrackDragOwner->mnActions;
		aDragEvent.DropAction = nCurrentAction;
	}
	else if ( aNativeTransferable )
	{
		aDragEvent.SourceActions = ImplGetDragAllowableActions( aNativeTransferable );
		aDragEvent.DropAction = nCurrentAction;
	}

	DropTargetDragContext *pContext = new DropTargetDragContext( aDragEvent.DropAction );
	aDragEvent.Context = Reference< XDropTargetDragContext >( pContext );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragExit( aDragEvent );
	}

	nCurrentAction = pContext->getDragAction();
	mbRejected = pContext->isRejected();
}

// ------------------------------------------------------------------------

void JavaDropTarget::handleDragOver( sal_Int32 nX, sal_Int32 nY, DragRef aNativeTransferable )
{
	DropTargetDragEvent aDragEvent;
	aDragEvent.Source = Reference< XInterface >( static_cast< OWeakObject* >( this ) );
	aDragEvent.LocationX = nX;
	aDragEvent.LocationY = nY;
	aDragEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDragEvent.DropAction = DNDConstants::ACTION_NONE;

	if ( pTrackDragOwner )
	{
		aDragEvent.SourceActions = pTrackDragOwner->mnActions;
		aDragEvent.DropAction = nCurrentAction;
	}
	else if ( aNativeTransferable )
	{
		aDragEvent.SourceActions = ImplGetDragAllowableActions( aNativeTransferable );
		aDragEvent.DropAction = nCurrentAction;
	}

	DropTargetDragContext *pContext = new DropTargetDragContext( aDragEvent.DropAction );
	aDragEvent.Context = Reference< XDropTargetDragContext >( pContext );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragOver( aDragEvent );
	}

	nCurrentAction = pContext->getDragAction();
	mbRejected = pContext->isRejected();
}

// ------------------------------------------------------------------------

bool JavaDropTarget::handleDrop( sal_Int32 nX, sal_Int32 nY, DragRef aNativeTransferable )
{
	// Don't set the cursor to the reject cursor since a drop has occurred
	bNoRejectCursor = true;

	// Reset the pointer to the last pointer set in VCL window
	if ( mpWindow && mpWindow->IsVisible() )
	{
		// We need to toggle the style to make sure that VCL resets the
		// pointer
		PointerStyle nStyle = mpWindow->GetPointer().GetStyle();
		if ( nStyle == POINTER_ARROW )
			mpWindow->SetPointer( Pointer( POINTER_NULL ) );
		else
			mpWindow->SetPointer( Pointer( POINTER_ARROW ) );
		mpWindow->SetPointer( Pointer( nStyle ) );
	}

	if ( mbRejected )
		return false;

	DropTargetDropEvent aDropEvent;
	aDropEvent.Source = Reference< XInterface >( static_cast< OWeakObject* >( this ) );
	aDropEvent.LocationX = nX;
	aDropEvent.LocationY = nY;
	aDropEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDropEvent.DropAction = DNDConstants::ACTION_NONE;

	if ( pTrackDragOwner )
	{
		aDropEvent.SourceActions = pTrackDragOwner->mnActions;
		aDropEvent.DropAction = nCurrentAction;
		aDropEvent.Transferable = pTrackDragOwner->maContents;
	}
	else if ( aNativeTransferable )
	{
		aDropEvent.SourceActions = ImplGetDragAllowableActions( aNativeTransferable );
		aDropEvent.DropAction = nCurrentAction;

		DTransTransferable *pTransferable = new DTransTransferable( aNativeTransferable, TRANSFERABLE_TYPE_DRAG );
		if ( pTransferable )
			aDropEvent.Transferable = Reference< XTransferable >( pTransferable );
	}

	DropTargetDropContext *pContext = new DropTargetDropContext( aDropEvent.DropAction );
	aDropEvent.Context = Reference< XDropTargetDropContext >( pContext );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->drop( aDropEvent );
	}

	nCurrentAction = pContext->getDropAction();

	return pContext->getDropComplete();
}
