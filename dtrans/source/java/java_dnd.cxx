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
#ifndef _OSL_CONDITN_HXX_
#undef check
#include <osl/conditn.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
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

static Mutex aDragMutex;
static EventLoopTimerUPP pTrackDragTimerUPP = NULL;
static DragTrackingHandlerUPP pDragTrackingHandlerUPP = NULL;
static DragTrackingHandlerUPP pDropTrackingHandlerUPP = NULL;
static DragReceiveHandlerUPP pDragReceiveHandlerUPP = NULL;
static ::std::list< ::java::JavaDragSource* > aDragSources;
static ::std::list< ::java::JavaDropTarget* > aDropTargets;
static JavaDragSource *pTrackDragOwner = NULL;
static Condition aTrackDragCondition;
static sal_Int8 nCurrentAction = DNDConstants::ACTION_NONE;
static sal_Int8 nStartingAction = DNDConstants::ACTION_NONE;
static bool bNoRejectCursor = true;

// ========================================================================

static void ImplSetThemeCursor( sal_Int8 nAction, bool bPointInWindow )
{
	MutexGuard aDragGuard( aDragMutex );

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
	MutexGuard aDragGuard( aDragMutex );

	if ( !pTrackDragOwner )
		return noErr;

	OGuard aSolarGuard( Application::GetSolarMutex() );

	JavaDragSource *pSource = NULL;
	for ( ::std::list< JavaDragSource* >::const_iterator it = aDragSources.begin(); it != aDragSources.end(); ++it )
	{
		if ( (*it)->getNativeWindow() == aWindow )
		{
			pSource = *it;
			break;
		}
	}

	if ( !pSource || pSource == pTrackDragOwner )
		return noErr;

	MacOSPoint aPoint;
	Rect aRect;
	if ( GetDragMouse( aDrag, &aPoint, NULL ) == noErr && GetWindowBounds( pTrackDragOwner->mpNativeWindow, kWindowContentRgn, &aRect ) == noErr )
		pTrackDragOwner->handleDrag( (sal_Int32)( aPoint.h - aRect.left ), (sal_Int32)( aPoint.v - aRect.top ) );

	return noErr;
}

// ------------------------------------------------------------------------

static OSErr ImplDropTrackingHandlerCallback( DragTrackingMessage nMessage, WindowRef aWindow, void *pData, DragRef aDrag )
{
	MutexGuard aDragGuard( aDragMutex );

	OGuard aSolarGuard( Application::GetSolarMutex() );

	JavaDropTarget *pTarget = NULL;
	for ( ::std::list< JavaDropTarget* >::const_iterator it = aDropTargets.begin(); it != aDropTargets.end(); ++it )
	{
		if ( (*it)->getNativeWindow() == aWindow )
		{
			pTarget = *it;
			break;
		}
	}

	if ( !pTarget )
		return noErr;

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

	return noErr;
}

// ------------------------------------------------------------------------

static OSErr ImplDragReceiveHandlerCallback( WindowRef aWindow, void *pData, DragRef aDrag )
{
	MutexGuard aDragGuard( aDragMutex );

	OGuard aSolarGuard( Application::GetSolarMutex() );

	JavaDropTarget *pTarget = NULL;
	for ( ::std::list< JavaDropTarget* >::const_iterator it = aDropTargets.begin(); it != aDropTargets.end(); ++it )
	{
		if ( (*it)->getNativeWindow() == aWindow )
		{
			pTarget = *it;
			break;
		}
	}

	if ( !pTarget )
		return dragNotAcceptedErr;

	MacOSPoint aPoint;
	Rect aRect;
	if ( GetDragMouse( aDrag, &aPoint, NULL ) == noErr && GetWindowBounds( aWindow, kWindowContentRgn, &aRect ) == noErr && pTarget->handleDrop( (sal_Int32)( aPoint.h - aRect.left ), (sal_Int32)( aPoint.v - aRect.top ), aDrag ) )
	{
		// Update actions
		ImplSetDragDropAction( aDrag, nCurrentAction );
		ImplSetThemeCursor( pTarget->isRejected() ? DNDConstants::ACTION_NONE : nCurrentAction, false );
		return noErr;
	}

	return dragNotAcceptedErr;
}

// ------------------------------------------------------------------------

void TrackDragTimerCallback( EventLoopTimerRef aTimer, void *pData )
{
	MutexGuard aDragGuard( aDragMutex );

	JavaDragSource *pSource = (JavaDragSource *)pData;

	DragSourceDropEvent aDragEvent;
	aDragEvent.Source = static_cast< OWeakObject* >(pSource);
	aDragEvent.DragSource = static_cast< XDragSource* >(pSource);
	aDragEvent.DragSourceContext = new DragSourceContext();
	aDragEvent.DropAction = DNDConstants::ACTION_NONE;
	aDragEvent.DropSuccess = sal_False;

	if ( pTrackDragOwner == pSource )
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

				// Release mutex while we are in drag
				aDragMutex.release();

				ImplSetDragAllowableActions( aDrag, nCurrentAction );
				ImplSetThemeCursor( nCurrentAction, true );

				aDragSources.push_back( pSource );

				// Set the drag's transferable
				DTransTransferable *pTransferable = new DTransTransferable( aDrag, TRANSFERABLE_TYPE_DRAG );

				if ( pSource->maContents.is() && pTransferable->setContents( pSource->maContents ) && TrackDrag( aDrag, &aEventRecord, aRegion ) == noErr )
				{
					aDragMutex.acquire();

					nCurrentAction = ImplGetDragDropAction( aDrag );
					if ( nCurrentAction != DNDConstants::ACTION_NONE )
					{
						aDragEvent.DropAction = nCurrentAction;
						aDragEvent.DropSuccess = sal_True;
					}

					aDragMutex.release();
				}

				delete pTransferable;

				aDragSources.remove( pSource );

				// Relock mutex after drag
				aDragMutex.acquire();

				DisposeRgn( aRegion );
			}

			DisposeDrag( aDrag );
		}
	}

	ClearableMutexGuard aGuard( pSource->maMutex );

	Reference< XDragSourceListener > xListener( pSource->maListener );

	aGuard.clear();

	if ( xListener.is() )
		xListener->dragDropEnd( aDragEvent );

	pTrackDragOwner = NULL;
	aTrackDragCondition.set();
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

	// If we own the event loop timer, wait for the timer to finish
	if ( pTrackDragOwner == this )
		aTrackDragCondition.wait();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaDragSource::initialize( const Sequence< Any >& arguments ) throw( Exception )
{
	if ( arguments.getLength() > 0 )
	{
		sal_Int32 nWindow;
		arguments.getConstArray()[0] >>= nWindow;
		if ( nWindow )
			mpNativeWindow = (WindowRef)nWindow;
	}

	if ( !mpNativeWindow || !IsValidWindowPtr( mpNativeWindow ) )
	{
		mpNativeWindow = NULL;
		throw RuntimeException();
	}

	if ( !pDragTrackingHandlerUPP )
	{
		pDragTrackingHandlerUPP = NewDragTrackingHandlerUPP( ImplDragTrackingHandlerCallback );
		if ( pDragTrackingHandlerUPP )
			InstallTrackingHandler( pDragTrackingHandlerUPP, NULL, NULL );
	}
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
	MutexGuard aDragGuard( aDragMutex );

	DragSourceDropEvent aDragEvent;
	aDragEvent.Source = static_cast< OWeakObject* >(this);
	aDragEvent.DragSource = static_cast< XDragSource* >(this);
	aDragEvent.DragSourceContext = Reference< XDragSourceContext >( new DragSourceContext() );
	aDragEvent.DropAction = DNDConstants::ACTION_NONE;
	aDragEvent.DropSuccess = sal_False;

	ClearableMutexGuard aGuard( maMutex );

	if ( pTrackDragOwner )
	{
		aGuard.clear();

		if ( listener.is() )
			listener->dragDropEnd( aDragEvent );

		return;
	}

	mnActions = sourceActions;
	maContents = transferable;
	maListener = listener;

	if ( !pTrackDragTimerUPP )
		pTrackDragTimerUPP = NewEventLoopTimerUPP( TrackDragTimerCallback );

	if ( maContents.is() && pTrackDragTimerUPP && mpNativeWindow )
	{
		pTrackDragOwner = this;
		aTrackDragCondition.reset();
		InstallEventLoopTimer( GetMainEventLoop(), 0, 0, pTrackDragTimerUPP, (void *)this, NULL );
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
	MutexGuard aDragGuard( aDragMutex );

	DragSourceDragEvent aSourceDragEvent;
	aSourceDragEvent.Source = static_cast< OWeakObject* >(this);
	aSourceDragEvent.DragSource = static_cast< XDragSource* >(this);

	aSourceDragEvent.DropAction = mnActions;
	aSourceDragEvent.UserAction = nCurrentAction;

	ClearableMutexGuard aGuard( maMutex );

	Reference< XDragSourceListener > xListener( maListener );

	aGuard.clear();

	// Send source drag event
	if ( xListener.is() )
		xListener->dragOver( aSourceDragEvent );
}

// ========================================================================

JavaDropTarget::JavaDropTarget() :
	WeakComponentImplHelper3< XDropTarget, XInitialization, XServiceInfo >( maMutex ),
	mbActive( sal_True ),
	mnDefaultActions( DNDConstants::ACTION_NONE ),
	mpNativeWindow( NULL ),
	mbRejected( false )
{
}

// ------------------------------------------------------------------------

JavaDropTarget::~JavaDropTarget()
{
	MutexGuard aDragGuard( aDragMutex );

	aDropTargets.remove( this );
}

// --------------------------------------------------------------------------

void SAL_CALL JavaDropTarget::initialize( const Sequence< Any >& arguments ) throw( Exception )
{
	if ( arguments.getLength() > 0 )
	{
		sal_Int32 nWindow;
		arguments.getConstArray()[0] >>= nWindow;
		if ( nWindow )
			mpNativeWindow = (WindowRef)nWindow;
	}

	if ( !mpNativeWindow || !IsValidWindowPtr( mpNativeWindow ) )
	{
		mpNativeWindow = NULL;
		throw RuntimeException();
	}

	MutexGuard aDragGuard( aDragMutex );

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

void JavaDropTarget::handleDragEnter( sal_Int32 nX, sal_Int32 nY, DragRef aNativeTransferable )
{
	MutexGuard aDragGuard( aDragMutex );

	mbRejected = false;

	DropTargetDragEnterEvent aDragEnterEvent;
	aDragEnterEvent.Source = static_cast< XDropTarget* >(this);
	aDragEnterEvent.LocationX = nX;
	aDragEnterEvent.LocationY = nY;
	aDragEnterEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDragEnterEvent.DropAction = DNDConstants::ACTION_NONE;

	if ( pTrackDragOwner )
	{
		ClearableMutexGuard aDragSourceGuard( pTrackDragOwner->maMutex );

		aDragEnterEvent.SourceActions = pTrackDragOwner->mnActions;
		aDragEnterEvent.DropAction = nStartingAction;
		aDragEnterEvent.SupportedDataFlavors = pTrackDragOwner->maContents->getTransferDataFlavors();

		aDragSourceGuard.clear();
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

	ClearableMutexGuard aGuard( maMutex );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	aGuard.clear();

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
	MutexGuard aDragGuard( aDragMutex );

	DropTargetDragEvent aDragEvent;
	aDragEvent.Source = static_cast< XDropTarget* >(this);
	aDragEvent.LocationX = nX;
	aDragEvent.LocationY = nY;
	aDragEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDragEvent.DropAction = DNDConstants::ACTION_NONE;

	if ( pTrackDragOwner )
	{
		ClearableMutexGuard aDragSourceGuard( pTrackDragOwner->maMutex );

		aDragEvent.SourceActions = pTrackDragOwner->mnActions;
		aDragEvent.DropAction = nCurrentAction;

		aDragSourceGuard.clear();
	}
	else if ( aNativeTransferable )
	{
		aDragEvent.SourceActions = ImplGetDragAllowableActions( aNativeTransferable );
		aDragEvent.DropAction = nCurrentAction;
	}

	DropTargetDragContext *pContext = new DropTargetDragContext( aDragEvent.DropAction );
	aDragEvent.Context = Reference< XDropTargetDragContext >( pContext );

	ClearableMutexGuard aGuard( maMutex );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	aGuard.clear();

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
	MutexGuard aDragGuard( aDragMutex );

	DropTargetDragEvent aDragEvent;
	aDragEvent.Source = static_cast< XDropTarget* >(this);
	aDragEvent.LocationX = nX;
	aDragEvent.LocationY = nY;
	aDragEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDragEvent.DropAction = DNDConstants::ACTION_NONE;

	if ( pTrackDragOwner )
	{
		ClearableMutexGuard aDragSourceGuard( pTrackDragOwner->maMutex );

		aDragEvent.SourceActions = pTrackDragOwner->mnActions;
		aDragEvent.DropAction = nCurrentAction;

		aDragSourceGuard.clear();
	}
	else if ( aNativeTransferable )
	{
		aDragEvent.SourceActions = ImplGetDragAllowableActions( aNativeTransferable );
		aDragEvent.DropAction = nCurrentAction;
	}

	DropTargetDragContext *pContext = new DropTargetDragContext( aDragEvent.DropAction );
	aDragEvent.Context = Reference< XDropTargetDragContext >( pContext );

	ClearableMutexGuard aGuard( maMutex );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	aGuard.clear();

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
	MutexGuard aDragGuard( aDragMutex );

	// Don't set the cursor to the reject cursor since a drop has occurred
	bNoRejectCursor = true;

	if ( mbRejected )
		return false;

	DropTargetDropEvent aDropEvent;
	aDropEvent.Source = static_cast< OWeakObject* >(this);
	aDropEvent.LocationX = nX;
	aDropEvent.LocationY = nY;
	aDropEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDropEvent.DropAction = DNDConstants::ACTION_NONE;

	if ( pTrackDragOwner )
	{
		ClearableMutexGuard aDragSourceGuard( pTrackDragOwner->maMutex );

		aDropEvent.SourceActions = pTrackDragOwner->mnActions;
		aDropEvent.DropAction = nCurrentAction;
		aDropEvent.Transferable = pTrackDragOwner->maContents;

		aDragSourceGuard.clear();
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

	ClearableMutexGuard aGuard( maMutex );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	aGuard.clear();

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->drop( aDropEvent );
	}

	nCurrentAction = pContext->getDropAction();

	return pContext->getDropComplete();
}
