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

#include "java_dnd_cocoa.h"

#include <premac.h>
#import <AppKit/AppKit.h>
#include <postmac.h>

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
static ::std::list< ::java::JavaDragSource* > aDragSources;
static ::std::list< ::java::JavaDropTarget* > aDropTargets;
static JavaDragSource *pTrackDragOwner = NULL;
static sal_Int8 nCurrentAction = DNDConstants::ACTION_NONE;
static bool bNoRejectCursor = true;

static Point ImplGetNSPointFromPoint( NSPoint aPoint, NSWindow *pWindow );
static sal_Int8 ImplGetActionsFromDragOperationMask( NSDragOperation nMask );
static NSDragOperation ImplGetOperationFromActions( sal_Int8 nActions );
static sal_Int8 ImplGetDropActionFromOperationMask( NSDragOperation nMask );

// ========================================================================

@interface NSView (VCLView)
- (void)setDraggingDestinationDelegate:(id)pDelegate;
@end

@interface JavaDNDPasteboardHelper : NSObject
{
	NSView*						mpDestination;
	NSArray*					mpNewTypes;
}
+ (id)createWithDraggingDestination:(NSView *)pDestination newTypes:(NSArray *)pNewTypes;
- (id)initWithDraggingDestination:(NSView *)pDestination newTypes:(NSArray *)pNewTypes;
- (void)registerForDraggedTypes:(id)pSender;
- (void)unregisterDraggedTypes:(id)pSender;
@end

@interface JavaDNDDraggingDestination : NSObject
{
	NSView*						mpDestination;
}
+ (id)createWithView:(NSView *)pDestination;
- (void)concludeDragOperation:(id < NSDraggingInfo >)pSender;
- (void)dealloc;
- (void)draggingEnded:(id < NSDraggingInfo >)pSender;
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)pSender;
- (void)draggingExited:(id < NSDraggingInfo >)pSender;
- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)pSender;
- (id)initWithView:(NSView *)pDestination;
- (BOOL)performDragOperation:(id < NSDraggingInfo >)pSender;
- (BOOL)prepareForDragOperation:(id < NSDraggingInfo >)pSender;
- (BOOL)wantsPeriodicDraggingUpdates;
@end

@implementation JavaDNDPasteboardHelper

+ (id)createWithDraggingDestination:(NSView *)pDestination newTypes:(NSArray *)pNewTypes
{
	JavaDNDPasteboardHelper *pRet = [[JavaDNDPasteboardHelper alloc] initWithDraggingDestination:pDestination newTypes:pNewTypes];
	[pRet autorelease];
	return pRet;
}

- (void)dealloc
{
	if ( mpDestination )
		[mpDestination release];

	if ( mpNewTypes )
		[mpNewTypes release];

	[super dealloc];
}

- (id)initWithDraggingDestination:(NSView *)pDestination newTypes:(NSArray *)pNewTypes
{
	[super init];

	mpDestination = pDestination;
	if ( mpDestination )
		[mpDestination retain];
	mpNewTypes = pNewTypes;
	if ( mpNewTypes )
		[mpNewTypes retain];

	return self;
}

- (void)registerForDraggedTypes:(id)pSender
{
	if ( mpDestination && [mpDestination respondsToSelector:@selector(setDraggingDestinationDelegate:)] )
	{
		JavaDNDDraggingDestination *pDragDestination = [JavaDNDDraggingDestination createWithView:mpDestination];
		if ( pDragDestination && mpNewTypes && [mpNewTypes count] )
		{
			// Drag destination object is retained by the view
			[mpDestination setDraggingDestinationDelegate:pDragDestination];
			[mpDestination registerForDraggedTypes:mpNewTypes];
		}
		else
		{
			// Any previous drag destination object wll be released by the view
			[mpDestination unregisterDraggedTypes];
			[mpDestination setDraggingDestinationDelegate:nil];
		}
	}
}

- (void)unregisterDraggedTypes:(id)pSender
{
	if ( mpDestination && [mpDestination respondsToSelector:@selector(setDraggingDestinationDelegate:)] )
	{
		[mpDestination unregisterDraggedTypes];
		[mpDestination setDraggingDestinationDelegate:nil];
	}
}

@end

@implementation JavaDNDDraggingDestination

+ (id)createWithView:(NSView *)pDestination
{
	JavaDNDDraggingDestination *pRet = [[JavaDNDDraggingDestination alloc] initWithView:pDestination];
	[pRet autorelease];
	return pRet;
}

- (void)concludeDragOperation:(id < NSDraggingInfo >)pSender
{
}

- (void)dealloc
{
	if ( mpDestination )
		[mpDestination release];

	[super dealloc];
}

- (void)draggingEnded:(id < NSDraggingInfo >)pSender
{
}

- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)pSender
{
	NSDragOperation nRet = NSDragOperationNone;

	if ( !pSender )
		return nRet;

	NSWindow *pWindow = [pSender draggingDestinationWindow];
	if ( !pWindow )
		return nRet;

	Point aPos( ImplGetNSPointFromPoint( [pSender draggedImageLocation], pWindow ) );
	if ( !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			JavaDropTarget *pTarget = NULL;
			for ( ::std::list< JavaDropTarget* >::const_iterator it = aDropTargets.begin(); it != aDropTargets.end(); ++it )
			{
				if ( (*it)->getNSView() == mpDestination )
				{
					pTarget = *it;
					break;
				}
			}

			if ( pTarget )
				nRet = ImplGetOperationFromActions( pTarget->handleDragEnter( aPos.X(), aPos.Y(), pSender ) );
		}

		rSolarMutex.release();
	}

	return nRet;
}

- (void)draggingExited:(id < NSDraggingInfo >)pSender
{
	if ( !pSender )
		return;

	NSWindow *pWindow = [pSender draggingDestinationWindow];
	if ( !pWindow )
		return;

	Point aPos( ImplGetNSPointFromPoint( [pSender draggedImageLocation], pWindow ) );
	if ( !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			JavaDropTarget *pTarget = NULL;
			for ( ::std::list< JavaDropTarget* >::const_iterator it = aDropTargets.begin(); it != aDropTargets.end(); ++it )
			{
				if ( (*it)->getNSView() == mpDestination )
				{
					pTarget = *it;
					break;
				}
			}

			if ( pTarget )
				pTarget->handleDragExit( aPos.X(), aPos.Y(), pSender );
		}

		rSolarMutex.release();
	}
}

- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)pSender
{
	NSDragOperation nRet = NSDragOperationNone;

	if ( !pSender )
		return nRet;

	NSWindow *pWindow = [pSender draggingDestinationWindow];
	if ( !pWindow )
		return nRet;

	Point aPos( ImplGetNSPointFromPoint( [pSender draggedImageLocation], pWindow ) );
	if ( !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			JavaDropTarget *pTarget = NULL;
			for ( ::std::list< JavaDropTarget* >::const_iterator it = aDropTargets.begin(); it != aDropTargets.end(); ++it )
			{
				if ( (*it)->getNSView() == mpDestination )
				{
					pTarget = *it;
					break;
				}
			}

			if ( pTarget )
				nRet = ImplGetOperationFromActions( pTarget->handleDragOver( aPos.X(), aPos.Y(), pSender ) );
		}

		rSolarMutex.release();
	}

	return nRet;
}

- (id)initWithView:(NSView *)pDestination
{
	[super init];

	mpDestination = pDestination;
	if ( mpDestination )
		[mpDestination retain];

	return self;
}

- (BOOL)performDragOperation:(id < NSDraggingInfo >)pSender
{
	BOOL bRet = NO;

	if ( !pSender )
		return bRet;

	NSWindow *pWindow = [pSender draggingDestinationWindow];
	if ( !pWindow )
		return bRet;

	Point aPos( ImplGetNSPointFromPoint( [pSender draggedImageLocation], pWindow ) );
	if ( !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			JavaDropTarget *pTarget = NULL;
			for ( ::std::list< JavaDropTarget* >::const_iterator it = aDropTargets.begin(); it != aDropTargets.end(); ++it )
			{
				if ( (*it)->getNSView() == mpDestination )
				{
					pTarget = *it;
					break;
				}
			}

			if ( pTarget )
				bRet = pTarget->handleDrop( aPos.X(), aPos.Y(), pSender );
		}

		rSolarMutex.release();
	}

	return bRet;
}

- (BOOL)prepareForDragOperation:(id < NSDraggingInfo >)pSender
{
	return YES;
}

- (BOOL)wantsPeriodicDraggingUpdates
{
	return NO;
}


@end

// ========================================================================

static Point ImplGetNSPointFromPoint( NSPoint aPoint, NSWindow *pWindow )
{
	Point aRet;

	if ( pWindow )
	{
		NSRect aFrameRect = [pWindow frame];
		aFrameRect.origin = NSMakePoint( 0, 0 );
		NSRect aContentRect = [pWindow contentRectForFrameRect:aFrameRect];
		aRet = Point( (long)( aPoint.x - aContentRect.origin.x ), (long)( aContentRect.origin.y + aContentRect.size.height - aPoint.y ) );
	}
	
	return aRet;
}

// ------------------------------------------------------------------------

static sal_Int8 ImplGetActionsFromDragOperationMask( NSDragOperation nMask )
{
	sal_Int8 nRet = DNDConstants::ACTION_NONE;

	if ( nMask & ( NSDragOperationMove | NSDragOperationGeneric ) )
		nRet |= DNDConstants::ACTION_MOVE;
	if ( nMask & ( NSDragOperationCopy | NSDragOperationGeneric ) )
		nRet |= DNDConstants::ACTION_COPY;
	if ( nMask & ( NSDragOperationLink | NSDragOperationGeneric ) )
		nRet |= DNDConstants::ACTION_LINK;

	// If more than one action, add default action to signal that the drop
	// target needs to decide which action to use
	if ( nRet != DNDConstants::ACTION_NONE && nRet != DNDConstants::ACTION_MOVE && nRet != DNDConstants::ACTION_COPY && nRet != DNDConstants::ACTION_LINK )
		nRet |= DNDConstants::ACTION_DEFAULT;

	return nRet;
}

// ------------------------------------------------------------------------

static NSDragOperation ImplGetOperationFromActions( sal_Int8 nActions )
{
	NSDragOperation nRet = NSDragOperationNone;

	if ( nActions & DNDConstants::ACTION_MOVE )
		nRet = NSDragOperationMove;
	if ( nActions & DNDConstants::ACTION_COPY )
		nRet = NSDragOperationCopy;
	if ( nActions & DNDConstants::ACTION_LINK )
		nRet = NSDragOperationLink;

	return nRet;
}

// ------------------------------------------------------------------------

static sal_Int8 ImplGetDropActionFromOperationMask( NSDragOperation nMask )
{
	sal_Int8 nRet = DNDConstants::ACTION_NONE;

	int nActions = 0;
	if ( nMask & NSDragOperationMove )
	{
		nRet = DNDConstants::ACTION_MOVE;
		nActions++;
	}
	else if ( nMask & ( NSDragOperationCopy | NSDragOperationGeneric ) )
	{
		nRet = DNDConstants::ACTION_COPY;
		nActions++;
	}
	else if ( nMask & NSDragOperationLink )
	{
		nRet = DNDConstants::ACTION_LINK;
		nActions++;
	}

	// If more than one action, add default action to signal that the drop
	// target needs to decide which action to use
	if ( nActions > 1 )
		nRet |= DNDConstants::ACTION_DEFAULT;

	return nRet;
}

// ------------------------------------------------------------------------

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

static void ImplUpdateCurrentAction( DragRef aDrag )
{
	if ( pTrackDragOwner )
	{
		sal_Int8 nOldAction = nCurrentAction;

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
		else if ( pTrackDragOwner->mnActions & DNDConstants::ACTION_MOVE )
			nCurrentAction = DNDConstants::ACTION_MOVE;
		else if ( pTrackDragOwner->mnActions & DNDConstants::ACTION_COPY )
			nCurrentAction = DNDConstants::ACTION_COPY;
		else if ( pTrackDragOwner->mnActions & DNDConstants::ACTION_LINK )
			nCurrentAction = DNDConstants::ACTION_LINK;
		else 
			nCurrentAction = DNDConstants::ACTION_NONE;

		if ( ! ( aEventRecord.modifiers & ( shiftKey | cmdKey ) ) )
			nCurrentAction |= DNDConstants::ACTION_DEFAULT;

		if ( nCurrentAction != nOldAction )
		{
			ImplSetDragAllowableActions( aDrag, nCurrentAction );
			pTrackDragOwner->handleActionChange();
		}
	}
}

// ------------------------------------------------------------------------

static OSErr ImplDragTrackingHandlerCallback( DragTrackingMessage nMessage, WindowRef aWindow, void *pData, DragRef aDrag )
{
	if ( !IsValidWindowPtr( aWindow ) )
		return noErr;

	if ( !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
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
					{
						ImplUpdateCurrentAction( aDrag );
						pTrackDragOwner->handleDrag( (sal_Int32)( aPoint.h - aRect.left ), (sal_Int32)( aPoint.v - aRect.top ) );
					}
				}
			}
		}

		rSolarMutex.release();
	}
	
	return noErr;
}

// ------------------------------------------------------------------------

void TrackDragTimerCallback( EventLoopTimerRef aTimer, void *pData )
{
	JavaDragSource *pSource = (JavaDragSource *)pData;

	IMutex& rSolarMutex = Application::GetSolarMutex();
	rSolarMutex.acquire();

	if ( pSource && pTrackDragOwner == pSource )
	{
		DragRef aDrag;
		if ( NewDrag( &aDrag ) == noErr )
		{
			RgnHandle aRegion = NewRgn();
			if ( aRegion )
			{
				bNoRejectCursor = false;

				ImplUpdateCurrentAction( aDrag );
				ImplSetThemeCursor( nCurrentAction, true );

				aDragSources.push_back( pSource );

				// Set the drag's transferable
				DTransTransferable *pTransferable = new DTransTransferable( aDrag, TRANSFERABLE_TYPE_DRAG );

				bool bContentsSet = ( pSource->maContents.is() && pTransferable->setContents( pSource->maContents ) );

				// Unlock application mutex while we are in the drag
				rSolarMutex.release();

				EventRecord aEventRecord;
				aEventRecord.what = mouseDown;
				aEventRecord.message = mouseMovedMessage;
				GetGlobalMouse( &aEventRecord.where );
				aEventRecord.modifiers = GetCurrentKeyModifiers();
				bool bTrackDrag = ( bContentsSet && TrackDrag( aDrag, &aEventRecord, aRegion ) == noErr );

				// Relock application mutex. Note that we don't check for
				// application shutdown as we are now on the hook to clean up
				// this thread
				rSolarMutex.acquire();

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

	return 0;
}

// ------------------------------------------------------------------------

JavaDragSource::JavaDragSource() :
	WeakComponentImplHelper3< XDragSource, XInitialization, XServiceInfo >( maMutex ),
	mnActions( DNDConstants::ACTION_NONE ),
	mpEnvData( NULL ),
	mpWindow( NULL ),
	maWindowRef( NULL )
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
		sal_Int32 nPtr = 0;
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
	if ( !maWindowRef && mpEnvData )
		maWindowRef = (WindowRef)NSView_windowRef( mpEnvData->pView );

	return maWindowRef;
}

// ------------------------------------------------------------------------

void JavaDragSource::handleActionChange()
{
	DragSourceDragEvent aSourceDragEvent;
	aSourceDragEvent.Source = Reference< XInterface >( static_cast< OWeakObject* >( this ) );
	aSourceDragEvent.DragSource = Reference< XDragSource >( this );

	aSourceDragEvent.DropAction = mnActions;
	aSourceDragEvent.UserAction = nCurrentAction;

	Reference< XDragSourceListener > xListener( maListener );

	// Send source drag event
	if ( xListener.is() )
		xListener->dropActionChanged( aSourceDragEvent );
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
	mpView( NULL ),
	mpWindow( NULL ),
	maWindowRef( NULL )
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
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpView )
	{
		JavaDNDPasteboardHelper *pHelper = [JavaDNDPasteboardHelper createWithDraggingDestination:mpView newTypes:nil];
		if ( pHelper )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pHelper performSelectorOnMainThread:@selector(unregisterDraggedTypes:) withObject:pHelper waitUntilDone:YES modes:pModes];
		}

		[mpView release];
		mpView = NULL;
	}

	[pPool release];

	mpWindow = NULL;
	aDropTargets.remove( this );
}

// --------------------------------------------------------------------------

void SAL_CALL JavaDropTarget::initialize( const Sequence< Any >& arguments ) throw( RuntimeException )
{
	if ( arguments.getLength() > 1 )
	{
		sal_Int32 nPtr = 0;
		arguments.getConstArray()[0] >>= nPtr;
		if ( nPtr )
			mpView = (NSView *)nPtr;
		arguments.getConstArray()[1] >>= nPtr;
		if ( nPtr )
			mpWindow = (Window *)nPtr;
	}

	if ( !mpView || !mpWindow )
		throw RuntimeException();

	aDropTargets.push_back( this );

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpView )
	{
		[mpView retain];

		JavaDNDPasteboardHelper *pHelper = [JavaDNDPasteboardHelper createWithDraggingDestination:mpView newTypes:DTransTransferable::getSupportedPasteboardTypes()];
		if ( pHelper )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pHelper performSelectorOnMainThread:@selector(registerForDraggedTypes:) withObject:pHelper waitUntilDone:YES modes:pModes];
		}
	}

	[pPool release];
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
	if ( !maWindowRef && mpView )
		maWindowRef = (WindowRef)NSView_windowRef( mpView );

	return maWindowRef;
}

// ------------------------------------------------------------------------

sal_Int8 JavaDropTarget::handleDragEnter( sal_Int32 nX, sal_Int32 nY, id aInfo )
{
	mbRejected = false;

	if ( !aInfo || ![aInfo conformsToProtocol:@protocol(NSDraggingInfo)] )
		return DNDConstants::ACTION_NONE;

	NSPasteboard *pPasteboard = [aInfo draggingPasteboard];
	if ( !pPasteboard )
		return DNDConstants::ACTION_NONE;

	DropTargetDragEnterEvent aDragEnterEvent;
	aDragEnterEvent.Source = Reference< XInterface >( static_cast< OWeakObject* >( this ) );
	aDragEnterEvent.LocationX = nX;
	aDragEnterEvent.LocationY = nY;
	aDragEnterEvent.SourceActions = DNDConstants::ACTION_NONE;
	aDragEnterEvent.DropAction = DNDConstants::ACTION_NONE;

	if ( pTrackDragOwner )
	{
		aDragEnterEvent.SourceActions = pTrackDragOwner->mnActions;
		aDragEnterEvent.DropAction = nCurrentAction;
		aDragEnterEvent.SupportedDataFlavors = pTrackDragOwner->maContents->getTransferDataFlavors();
	}
	else
	{
		NSDragOperation nMask = [aInfo draggingSourceOperationMask];
		aDragEnterEvent.SourceActions = ImplGetActionsFromDragOperationMask( nMask );
		aDragEnterEvent.DropAction = ImplGetDropActionFromOperationMask( nMask );

		DTransTransferable *pTransferable = new DTransTransferable( [pPasteboard name] );
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

	mbRejected = pContext->isRejected();
	if ( mbRejected )
		return DNDConstants::ACTION_NONE;
	else
		return aDragEnterEvent.DropAction;
}

// ------------------------------------------------------------------------

void JavaDropTarget::handleDragEnter( sal_Int32 nX, sal_Int32 nY, DragRef aNativeTransferable, sal_uInt16 nItem )
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
		aDragEnterEvent.DropAction = nCurrentAction;
		aDragEnterEvent.SupportedDataFlavors = pTrackDragOwner->maContents->getTransferDataFlavors();
	}
	else if ( aNativeTransferable )
	{
		aDragEnterEvent.SourceActions = ImplGetDragAllowableActions( aNativeTransferable );
		aDragEnterEvent.DropAction = ImplGetDragDropAction( aNativeTransferable );

		DTransTransferable *pTransferable = new DTransTransferable( aNativeTransferable, TRANSFERABLE_TYPE_DRAG, nItem );
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

	mbRejected = pContext->isRejected();
}

// ------------------------------------------------------------------------

void JavaDropTarget::handleDragExit( sal_Int32 nX, sal_Int32 nY, id aInfo )
{
	mbRejected = false;

	if ( !aInfo || ![aInfo conformsToProtocol:@protocol(NSDraggingInfo)] )
		return;

	NSPasteboard *pPasteboard = [aInfo draggingPasteboard];
	if ( !pPasteboard )
		return;

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
	else
	{
		NSDragOperation nMask = [aInfo draggingSourceOperationMask];
		aDragEvent.SourceActions = ImplGetActionsFromDragOperationMask( nMask );
		aDragEvent.DropAction = ImplGetDropActionFromOperationMask( nMask );
	}

	DropTargetDragContext *pContext = new DropTargetDragContext( aDragEvent.DropAction );
	aDragEvent.Context = Reference< XDropTargetDragContext >( pContext );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragExit( aDragEvent );
	}
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
		aDragEvent.DropAction = ImplGetDragDropAction( aNativeTransferable );
	}

	DropTargetDragContext *pContext = new DropTargetDragContext( aDragEvent.DropAction );
	aDragEvent.Context = Reference< XDropTargetDragContext >( pContext );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragExit( aDragEvent );
	}

	mbRejected = pContext->isRejected();
}

// ------------------------------------------------------------------------

sal_Int8 JavaDropTarget::handleDragOver( sal_Int32 nX, sal_Int32 nY, id aInfo )
{
	mbRejected = false;

	if ( !aInfo || ![aInfo conformsToProtocol:@protocol(NSDraggingInfo)] )
		return DNDConstants::ACTION_NONE;

	NSPasteboard *pPasteboard = [aInfo draggingPasteboard];
	if ( !pPasteboard )
		return DNDConstants::ACTION_NONE;

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
	else
	{
		NSDragOperation nMask = [aInfo draggingSourceOperationMask];
		aDragEvent.SourceActions = ImplGetActionsFromDragOperationMask( nMask );
		aDragEvent.DropAction = ImplGetDropActionFromOperationMask( nMask );
	}

	DropTargetDragContext *pContext = new DropTargetDragContext( aDragEvent.DropAction );
	aDragEvent.Context = Reference< XDropTargetDragContext >( pContext );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragOver( aDragEvent );
	}

	mbRejected = pContext->isRejected();
	if ( mbRejected )
		return DNDConstants::ACTION_NONE;
	else
		return aDragEvent.DropAction;
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
		aDragEvent.DropAction = ImplGetDragDropAction( aNativeTransferable );
	}

	DropTargetDragContext *pContext = new DropTargetDragContext( aDragEvent.DropAction );
	aDragEvent.Context = Reference< XDropTargetDragContext >( pContext );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragOver( aDragEvent );
	}

	mbRejected = pContext->isRejected();
}

// ------------------------------------------------------------------------

bool JavaDropTarget::handleDrop( sal_Int32 nX, sal_Int32 nY, id aInfo )
{
	bool bRet = false;

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

	if ( mbRejected || !aInfo || ![aInfo conformsToProtocol:@protocol(NSDraggingInfo)] )
		return bRet;

	NSPasteboard *pPasteboard = [aInfo draggingPasteboard];
	if ( !pPasteboard )
		return bRet;

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
	else
	{
		NSDragOperation nMask = [aInfo draggingSourceOperationMask];
		aDropEvent.SourceActions = ImplGetActionsFromDragOperationMask( nMask );
		aDropEvent.DropAction = ImplGetDropActionFromOperationMask( nMask );

		DTransTransferable *pTransferable = new DTransTransferable( [pPasteboard name] );
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

	// One of the listeners may have rejected the drop so use the rejected
	// flag instead the context's getDropComplete() method
	bRet = !mbRejected;

	return bRet;
}

// ------------------------------------------------------------------------

bool JavaDropTarget::handleDrop( sal_Int32 nX, sal_Int32 nY, DragRef aNativeTransferable, sal_uInt16 nItem )
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
		aDropEvent.DropAction = ImplGetDragDropAction( aNativeTransferable );

		DTransTransferable *pTransferable = new DTransTransferable( aNativeTransferable, TRANSFERABLE_TYPE_DRAG, nItem );
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

	// One of the listeners may have rejected the drop so use the rejected
	// flag instead the context's getDropComplete() method
	return !mbRejected;
}
