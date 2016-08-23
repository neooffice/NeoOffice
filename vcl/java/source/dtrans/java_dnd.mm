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

#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vos/mutex.hxx>

#include <premac.h>
#import <AppKit/AppKit.h>
#include <postmac.h>
#undef check

#include "java/salinst.h"

#include "java_dnd.hxx"
#include "java_dndcontext.hxx"
#include "DTransTransferable.hxx"
#include "../app/salinst_cocoa.h"

using namespace com::sun::star;
using namespace cppu;
using namespace osl;
using namespace rtl;
using namespace vos;

static ::std::list< JavaDragSource* > aDragSources;
static ::std::list< JavaDropTarget* > aDropTargets;
static JavaDragSource *pTrackDragOwner = NULL;

static Point ImplGetPointFromNSPoint( NSPoint aPoint, NSWindow *pWindow );
static sal_Int8 ImplGetActionsFromDragOperationMask( NSDragOperation nMask );
static NSDragOperation ImplGetOperationMaskFromActions( sal_Int8 nActions );
static NSDragOperation ImplGetOperationFromActions( sal_Int8 nActions );
static sal_Int8 ImplGetDropActionFromOperationMask( NSDragOperation nMask, bool bSame );
static void ImplSetCursorFromAction( sal_Int8 nAction, Window *pWindow );


// ========================================================================

@interface NSWindow (JavaDNDPasteboardHelper)
- (void)setDraggingSourceDelegate:(id)pDelegate;
@end

@interface NSView (JavaDNDPasteboardHelper)
- (void)setDraggingDestinationDelegate:(id)pDelegate;
- (void)setDraggingSourceDelegate:(id)pDelegate;
@end

@interface JavaDNDDraggingDestination : NSObject <NSDraggingDestination>
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
- (void)updateDraggingItemsForDrag:(id <NSDraggingInfo>)pSender;
- (BOOL)wantsPeriodicDraggingUpdates;
@end

@interface JavaDNDDraggingSource : NSObject <NSDraggingSource>
{
	NSView*						mpSource;
}
- (void)dealloc;
- (void)draggingSession:(NSDraggingSession *)pSession endedAtPoint:(NSPoint)aPoint operation:(NSDragOperation)nOperation;
- (void)draggingSession:(NSDraggingSession *)pSession movedToPoint:(NSPoint)aPoint;
- (NSDragOperation)draggingSession:(NSDraggingSession *)pSession sourceOperationMaskForDraggingContext:(NSDraggingContext)nContext;
- (void)draggingSession:(NSDraggingSession *)pSession willBeginAtPoint:(NSPoint)aPoint;
- (BOOL)ignoreModifierKeysForDraggingSession:(NSDraggingSession *)pSession;
- (id)initWithView:(NSView *)pSource;
@end

static NSDraggingSession *pCurrentDraggingSession = nil;
static DTransTransferable *pCurrentTransferable = NULL;

@interface JavaDNDPasteboardHelper : NSObject
{
	NSView*						mpDestination;
	JavaDNDDraggingSource*		mpDraggingSource;
	JavaDragSource*				mpDragOwner;
	BOOL						mbDragStarted;
	NSEvent*					mpLastMouseEvent;
	NSArray*					mpNewTypes;
	NSView*						mpSource;
}
+ (void)releaseCurrentDraggingSession:(NSDraggingSession *)pSession releaseDragLock:(BOOL)bUnlock;
- (void)dealloc;
- (BOOL)dragStarted;
- (NSView *)getDestination;
- (NSView *)getSource;
- (id)initWithDraggingDestination:(NSView *)pDestination newTypes:(NSArray *)pNewTypes;
- (id)initWithDraggingSource:(NSView *)pSource dragOwner:(JavaDragSource *)pDragOwner;
- (void)mouseDown:(NSEvent *)pEvent;
- (void)mouseDragged:(NSEvent *)pEvent;
- (void)registerDragSource:(id)pSender;
- (void)registerForDraggedTypes:(id)pSender;
- (void)setLastMouseEvent:(NSEvent *)pEvent;
- (void)startDrag:(id)pSender;
- (void)unregisterDraggedTypesAndReleaseView:(id)pSender;
- (void)unregisterDragSourceAndReleaseView:(id)pSender;
@end

@implementation JavaDNDPasteboardHelper

+ (void)releaseCurrentDraggingSession:(NSDraggingSession *)pSession releaseDragLock:(BOOL)bUnlock
{
	if ( pSession == pCurrentDraggingSession )
	{
		pCurrentDraggingSession = nil;
		if ( pCurrentTransferable )
		{
			delete pCurrentTransferable;
			pCurrentTransferable = NULL;
		}
	}

	// Prevent risk of hanging by unlocking drag lock even if the dragging
	// session does not match
	if ( bUnlock )
		VCLInstance_setDragLock( NO );
}

- (void)dealloc
{
	if ( mpDestination )
		[mpDestination release];

	if ( mpDraggingSource )
		[mpDraggingSource release];

	if ( mpLastMouseEvent )
		[mpLastMouseEvent release];

	if ( mpNewTypes )
		[mpNewTypes release];

	if ( mpSource )
		[mpSource release];

	[super dealloc];
}

- (BOOL)dragStarted
{
	return mbDragStarted;
}

- (NSView *)getDestination
{
	return mpDestination;
}

- (NSView *)getSource
{
	return mpSource;
}

- (id)initWithDraggingDestination:(NSView *)pDestination newTypes:(NSArray *)pNewTypes
{
	[super init];

	mpDestination = pDestination;
	if ( mpDestination )
		[mpDestination retain];
	mpDraggingSource = nil;
	mpDragOwner = NULL;
	mbDragStarted = NO;
	mpLastMouseEvent = nil;
	mpNewTypes = pNewTypes;
	if ( mpNewTypes )
		[mpNewTypes retain];
	mpSource = nil;

	return self;
}

- (id)initWithDraggingSource:(NSView *)pSource dragOwner:(JavaDragSource *)pDragOwner
{
	[super init];

	mpDestination = nil;
	mpDraggingSource = nil;
	mpDragOwner = pDragOwner;
	mbDragStarted = NO;
	mpLastMouseEvent = nil;
	mpNewTypes = nil;
	mpSource = pSource;
	if ( mpSource )
		[mpSource retain];

	return self;
}

- (void)mouseDown:(NSEvent *)pEvent
{
	[self setLastMouseEvent:pEvent];
}

- (void)mouseDragged:(NSEvent *)pEvent
{
	[self setLastMouseEvent:pEvent];
}

- (void)registerDragSource:(id)pSender
{
	(void)pSender;

	if ( mpSource )
	{
		if ( [mpSource respondsToSelector:@selector(setDraggingSourceDelegate:)] )
		{
			if ( !mpDraggingSource )
			{
				// Do not retain as invoking alloc disables autorelease
				mpDraggingSource = [[JavaDNDDraggingSource alloc] initWithView:mpSource];
			}

			if ( mpDraggingSource )
			{
				// Drag source object is retained by the view
				[mpSource setDraggingSourceDelegate:mpDraggingSource];
			}
			else
			{
				// Any previous drag source object wll be released by the view
				[mpSource setDraggingSourceDelegate:nil];
			}
		}

		NSWindow *pWindow = [mpSource window];
		if ( pWindow && [pWindow respondsToSelector:@selector(setDraggingSourceDelegate:)] )
		{
			// Drag source object is retained by the view
			[pWindow setDraggingSourceDelegate:self];
		}
	}
}

- (void)registerForDraggedTypes:(id)pSender
{
	(void)pSender;

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

- (void)setLastMouseEvent:(NSEvent *)pEvent
{
	if ( !pEvent )
		return;

	if ( mpLastMouseEvent )
	{
		[mpLastMouseEvent release];
		mpLastMouseEvent = nil;
	}

	mpLastMouseEvent = pEvent;
	[mpLastMouseEvent retain];
}

- (void)startDrag:(id)pSender
{
	(void)pSender;

	mbDragStarted = NO;

	if ( mpDraggingSource && mpDragOwner && mpLastMouseEvent && mpSource )
	{
		// Fix bug 3652 by locking the application mutex and never letting it
		// get released during a native drag session. This prevents drag events
		// from getting dispatched out of order when we release and reacquire
		// the mutex.
		if ( VCLInstance_setDragLock( YES ) )
		{
			NSDraggingSession *pDraggingSession = nil;
			DTransTransferable *pTransferable = NULL;
			if ( pTrackDragOwner == mpDragOwner && pTrackDragOwner->maContents.is() )
			{
				pTransferable = new DTransTransferable( @"JavaDNDPasteboardHelper" );
				if ( pTransferable )
				{
					id pPasteboardWriter = nil;
					pTransferable->setContents( pTrackDragOwner->maContents, &pPasteboardWriter );
					if ( pPasteboardWriter )
					{
						if ( [[pPasteboardWriter class] conformsToProtocol:@protocol(NSPasteboardWriting)] )
						{
							NSDraggingItem *pDraggingItem = [[NSDraggingItem alloc] initWithPasteboardWriter:pPasteboardWriter];
							if ( pDraggingItem )
							{
								[pDraggingItem autorelease];

								pDraggingSession = [mpSource beginDraggingSessionWithItems:[NSArray arrayWithObject:pDraggingItem] event:mpLastMouseEvent source:mpDraggingSource];
							}
						}

						[pPasteboardWriter release];
					}
				}
			}

			if ( pDraggingSession )
			{
				[JavaDNDPasteboardHelper releaseCurrentDraggingSession:pCurrentDraggingSession releaseDragLock:NO];
				mbDragStarted = YES;
				pCurrentDraggingSession = pDraggingSession;
				pCurrentTransferable = pTransferable;
			}
			else
			{
				VCLInstance_setDragLock( NO );
				if ( pTransferable )
					delete pTransferable;
			}
		}
	}
}

- (void)unregisterDraggedTypesAndReleaseView:(id)pSender
{
	(void)pSender;

	if ( mpDestination )
	{
		if ( [mpDestination respondsToSelector:@selector(setDraggingDestinationDelegate:)] )
		{
			[mpDestination unregisterDraggedTypes];
			[mpDestination setDraggingDestinationDelegate:nil];
		}

		[mpDestination release];
		mpDestination = nil;
	}
}

- (void)unregisterDragSourceAndReleaseView:(id)pSender
{
	(void)pSender;

	if ( mpSource )
	{
		if ( [mpSource respondsToSelector:@selector(setDraggingSourceDelegate:)] )
			[mpSource setDraggingSourceDelegate:nil];

		NSWindow *pWindow = [mpSource window];
		if ( pWindow && [pWindow respondsToSelector:@selector(setDraggingSourceDelegate:)] )
			[pWindow setDraggingSourceDelegate:nil];

		[mpSource release];
		mpSource = nil;
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
	(void)pSender;
}

- (void)dealloc
{
	if ( mpDestination )
		[mpDestination release];

	[super dealloc];
}

- (void)draggingEnded:(id < NSDraggingInfo >)pSender
{
	(void)pSender;
}

- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)pSender
{
	NSDragOperation nRet = NSDragOperationNone;

	if ( !mpDestination || !pSender )
		return nRet;

	NSWindow *pWindow = [pSender draggingDestinationWindow];
	if ( !pWindow )
		return nRet;

	Point aPos( ImplGetPointFromNSPoint( [pSender draggingLocation], pWindow ) );
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
	if ( !mpDestination || !pSender )
		return;

	NSWindow *pWindow = [pSender draggingDestinationWindow];
	if ( !pWindow )
		return;

	Point aPos( ImplGetPointFromNSPoint( [pSender draggingLocation], pWindow ) );
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

	if ( !mpDestination || !pSender )
		return nRet;

	NSWindow *pWindow = [pSender draggingDestinationWindow];
	if ( !pWindow )
		return nRet;

	Point aPos( ImplGetPointFromNSPoint( [pSender draggingLocation], pWindow ) );
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

	// Fix hanging when dragging a pivot table onto a chart reported in
	// 04/25/2016 e-mail to elcapitanbugs@neooffice.org by releasing the
	// native drag lock so that the OOo code can display any dialogs in the
	// drop event
	if ( [pSender draggingSource] )
		VCLInstance_setDragLock( NO );

	if ( !mpDestination )
		return bRet;

	NSWindow *pWindow = [pSender draggingDestinationWindow];
	if ( !pWindow )
		return bRet;

	Point aPos( ImplGetPointFromNSPoint( [pSender draggingLocation], pWindow ) );
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
	(void)pSender;

	return YES;
}

- (void)updateDraggingItemsForDrag:(id <NSDraggingInfo>)pSender
{
	(void)pSender;
}

- (BOOL)wantsPeriodicDraggingUpdates
{
	return NO;
}


@end

@implementation JavaDNDDraggingSource

- (void)dealloc
{
	if ( mpSource )
		[mpSource release];

	[super dealloc];
}

- (void)draggingSession:(NSDraggingSession *)pSession endedAtPoint:(NSPoint)aPoint operation:(NSDragOperation)nOperation
{
	(void)pSession;

	if ( !mpSource )
	{
		[JavaDNDPasteboardHelper releaseCurrentDraggingSession:pCurrentDraggingSession releaseDragLock:YES];
		return;
	}
 
	NSWindow *pWindow = [mpSource window];
	if ( !pWindow )
	{
		[JavaDNDPasteboardHelper releaseCurrentDraggingSession:pCurrentDraggingSession releaseDragLock:YES];
		return;
	}
 
	Point aPos( ImplGetPointFromNSPoint( aPoint, pWindow ) );
	if ( !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			JavaDragSource *pSource = NULL;
			for ( ::std::list< JavaDragSource* >::const_iterator it = aDragSources.begin(); it != aDragSources.end(); ++it )
			{
				if ( (*it)->getNSView() == mpSource )
				{
					pSource = *it;
					break;
				}
			}

			if ( pSource && pTrackDragOwner == pSource )
			{
				pSource->handleDrag( aPos.X(), aPos.Y() );

				// Dispatch drop event
				datatransfer::dnd::DragSourceDropEvent *pDragEvent = new datatransfer::dnd::DragSourceDropEvent();
				pDragEvent->Source = uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( pSource ) );
				pDragEvent->DragSource = uno::Reference< datatransfer::dnd::XDragSource >( pSource );
				pDragEvent->DragSourceContext = uno::Reference< datatransfer::dnd::XDragSourceContext >( new JavaDragSourceContext() );
				pDragEvent->DropAction = ImplGetDropActionFromOperationMask( nOperation, true );
				pDragEvent->DropSuccess = ( pDragEvent->DropAction == datatransfer::dnd::DNDConstants::ACTION_NONE ? sal_False : sal_True );

				// Reset cursor to window's VCL pointer
				ImplSetCursorFromAction( datatransfer::dnd::DNDConstants::ACTION_NONE, pTrackDragOwner->mpWindow );

				// Fix bug 1442 by dispatching and deleting the
				// DragSourceDropEvent in the VCL event dispatch thread
				Application::PostUserEvent( STATIC_LINK( NULL, JavaDragSource, dragDropEnd ), pDragEvent );
			}
		}

		rSolarMutex.release();
	}

	[JavaDNDPasteboardHelper releaseCurrentDraggingSession:pCurrentDraggingSession releaseDragLock:YES];
}

- (void)draggingSession:(NSDraggingSession *)pSession movedToPoint:(NSPoint)aPoint
{
	(void)pSession;

	if ( !mpSource )
		return;
 
	NSWindow *pWindow = [mpSource window];
	if ( !pWindow )
		return;
 
	Point aPos( ImplGetPointFromNSPoint( aPoint, pWindow ) );
	if ( !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			JavaDragSource *pSource = NULL;
			for ( ::std::list< JavaDragSource* >::const_iterator it = aDragSources.begin(); it != aDragSources.end(); ++it )
			{
				if ( (*it)->getNSView() == mpSource )
				{
					pSource = *it;
					break;
				}
			}

			if ( pSource && pTrackDragOwner == pSource )
				pSource->handleDrag( aPos.X(), aPos.Y() );
		}

		rSolarMutex.release();
	}
}

- (NSDragOperation)draggingSession:(NSDraggingSession *)pSession sourceOperationMaskForDraggingContext:(NSDraggingContext)nContext
{
	(void)pSession;
	(void)nContext;

	NSDragOperation nRet = NSDragOperationNone;
	if ( !mpSource )
		return nRet;
 
	if ( !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( pTrackDragOwner && !Application::IsShutDown() )
		{
			JavaDragSource *pSource = NULL;
			for ( ::std::list< JavaDragSource* >::const_iterator it = aDragSources.begin(); it != aDragSources.end(); ++it )
			{
				if ( (*it)->getNSView() == mpSource )
				{
					pSource = *it;
					break;
				}
			}

			if ( pSource && pSource == pTrackDragOwner )
				nRet = ImplGetOperationMaskFromActions( pSource->mnActions );
		}

		rSolarMutex.release();
	}

	return nRet;
}

- (void)draggingSession:(NSDraggingSession *)pSession willBeginAtPoint:(NSPoint)aPoint
{
	(void)pSession;

	if ( !mpSource )
		return;

	NSWindow *pWindow = [mpSource window];
	if ( !pWindow )
		return;
 
	Point aPos( ImplGetPointFromNSPoint( aPoint, pWindow ) );
	if ( !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			JavaDragSource *pSource = NULL;
			for ( ::std::list< JavaDragSource* >::const_iterator it = aDragSources.begin(); it != aDragSources.end(); ++it )
			{
				if ( (*it)->getNSView() == mpSource )
				{
					pSource = *it;
					break;
				}
			}

			if ( pSource && pTrackDragOwner == pSource )
				pSource->handleDrag( aPos.X(), aPos.Y() );
		}

		rSolarMutex.release();
	}
}

- (BOOL)ignoreModifierKeysForDraggingSession:(NSDraggingSession *)pSession
{
	(void)pSession;

	return NO;
}

- (id)initWithView:(NSView *)pSource
{
	[super init];

	mpSource = pSource;
	if ( mpSource )
		[mpSource retain];

	return self;
}

@end

// ========================================================================

static Point ImplGetPointFromNSPoint( NSPoint aPoint, NSWindow *pWindow )
{
	Point aRet;

	if ( pWindow )
	{
		NSRect aFrameRect = [pWindow frame];
		aFrameRect.origin = NSMakePoint( 0, 0 );
		NSRect aContentRect = [pWindow contentRectForFrameRect:aFrameRect];
		aRet = Point( (long)aPoint.x, (long)( aContentRect.size.height - aPoint.y ) );
	}
	
	return aRet;
}

// ------------------------------------------------------------------------

static sal_Int8 ImplGetActionsFromDragOperationMask( NSDragOperation nMask )
{
	sal_Int8 nRet = datatransfer::dnd::DNDConstants::ACTION_NONE;

	if ( nMask & ( NSDragOperationCopy | NSDragOperationGeneric ) )
		nRet |= datatransfer::dnd::DNDConstants::ACTION_COPY;
	if ( nMask & ( NSDragOperationMove ) )
		nRet |= datatransfer::dnd::DNDConstants::ACTION_MOVE;
	if ( nMask & ( NSDragOperationLink ) )
		nRet |= datatransfer::dnd::DNDConstants::ACTION_LINK;

	// If more than one action, add default action to signal that the drop
	// target needs to decide which action to use
	if ( nRet != datatransfer::dnd::DNDConstants::ACTION_NONE && nRet != datatransfer::dnd::DNDConstants::ACTION_COPY && nRet != datatransfer::dnd::DNDConstants::ACTION_MOVE && nRet != datatransfer::dnd::DNDConstants::ACTION_LINK )
		nRet |= datatransfer::dnd::DNDConstants::ACTION_DEFAULT;

	return nRet;
}

// ------------------------------------------------------------------------

static NSDragOperation ImplGetOperationMaskFromActions( sal_Int8 nActions )
{
	NSDragOperation nRet = NSDragOperationNone;

	if ( nActions & datatransfer::dnd::DNDConstants::ACTION_COPY )
		nRet |= NSDragOperationCopy;
	if ( nActions & datatransfer::dnd::DNDConstants::ACTION_MOVE )
		nRet |= NSDragOperationMove;
	if ( nActions & datatransfer::dnd::DNDConstants::ACTION_LINK )
		nRet |= NSDragOperationLink;

	return nRet;
}

// ------------------------------------------------------------------------

static NSDragOperation ImplGetOperationFromActions( sal_Int8 nActions )
{
	NSDragOperation nRet = NSDragOperationNone;

	if ( nActions & datatransfer::dnd::DNDConstants::ACTION_COPY )
		nRet = NSDragOperationCopy;
	if ( nActions & datatransfer::dnd::DNDConstants::ACTION_MOVE )
		nRet = NSDragOperationMove;
	if ( nActions & datatransfer::dnd::DNDConstants::ACTION_LINK )
		nRet = NSDragOperationLink;

	return nRet;
}

// ------------------------------------------------------------------------

static sal_Int8 ImplGetDropActionFromOperationMask( NSDragOperation nMask, bool bSame )
{
	sal_Int8 nRet = datatransfer::dnd::DNDConstants::ACTION_NONE;
	int nActionCount = 0;

	// When the source and destination are the same, moving is preferred over
	// copying
	if ( bSame )
	{
		if ( nMask & NSDragOperationLink )
		{
			nRet = datatransfer::dnd::DNDConstants::ACTION_LINK;
			nActionCount++;
		}
		if ( nMask & ( NSDragOperationCopy | NSDragOperationGeneric ) )
		{
			nRet = datatransfer::dnd::DNDConstants::ACTION_COPY;
			nActionCount++;
		}
		if ( nMask & NSDragOperationMove )
		{
			nRet = datatransfer::dnd::DNDConstants::ACTION_MOVE;
			nActionCount++;
		}
	}
	else
	{
		if ( nMask & NSDragOperationLink )
		{
			nRet = datatransfer::dnd::DNDConstants::ACTION_LINK;
			nActionCount++;
		}
		if ( nMask & NSDragOperationMove )
		{
			nRet = datatransfer::dnd::DNDConstants::ACTION_MOVE;
			nActionCount++;
		}
		if ( nMask & ( NSDragOperationCopy | NSDragOperationGeneric ) )
		{
			nRet = datatransfer::dnd::DNDConstants::ACTION_COPY;
			nActionCount++;
		}
	}

	// If more than one action, add default action to signal that the drop
	// target needs to decide which action to use
	if ( nActionCount > 1 )
		nRet |= datatransfer::dnd::DNDConstants::ACTION_DEFAULT;

	return nRet;
}

// ------------------------------------------------------------------------

static void ImplSetCursorFromAction( sal_Int8 nAction, Window *pWindow )
{
	bool bSet = false;

	nAction &= ~datatransfer::dnd::DNDConstants::ACTION_DEFAULT;
	if ( nAction == datatransfer::dnd::DNDConstants::ACTION_NONE )
	{
		// Reset the pointer to the last pointer set in VCL window
		if ( pWindow && pWindow->IsVisible() )
		{
			// We need to toggle the style to make sure that VCL resets the
			// pointer
			PointerStyle nStyle = pWindow->GetPointer().GetStyle();
			if ( nStyle == POINTER_ARROW )
				pWindow->SetPointer( Pointer( POINTER_NULL ) );
			else
				pWindow->SetPointer( Pointer( POINTER_ARROW ) );
			pWindow->SetPointer( Pointer( nStyle ) );
			bSet = true;
		}
	}
	else if ( nAction == datatransfer::dnd::DNDConstants::ACTION_MOVE )
	{
		NSCursor *pCursor = [NSCursor closedHandCursor];
		if ( pCursor )
		{
			[pCursor set];
			bSet = true;
		}
	}
	else if ( nAction == datatransfer::dnd::DNDConstants::ACTION_COPY )
	{
		NSCursor *pCursor = [NSCursor dragCopyCursor];
		if ( pCursor )
		{
			[pCursor set];
			bSet = true;
		}
	}
	else if ( nAction == datatransfer::dnd::DNDConstants::ACTION_LINK )
	{
		NSCursor *pCursor = [NSCursor dragLinkCursor];
		if ( pCursor )
		{
			[pCursor set];
			bSet = true;
		}
	}

	if ( !bSet )
	{
		NSCursor *pCursor = [NSCursor closedHandCursor];
		if ( pCursor )
			[pCursor set];
	}
}

// ========================================================================

static uno::Sequence< OUString > JavaDragSource_getSupportedServiceNames()
{
    uno::Sequence< OUString > aRet( 1 );
    aRet[0] = OUString::createFromAscii( "com.sun.star.datatransfer.dnd.OleDragSource" );
    return aRet;
}
 
// ------------------------------------------------------------------------
 
static uno::Sequence< OUString > JavaDropTarget_getSupportedServiceNames()
{
    uno::Sequence< OUString > aRet( 1 );
    aRet[0] = OUString::createFromAscii( "com.sun.star.datatransfer.dnd.OleDropTarget" );
    return aRet;
}
 
// ========================================================================

IMPL_STATIC_LINK( JavaDragSource, dragDropEnd, void*, pData )
{
	datatransfer::dnd::DragSourceDropEvent *pDragEvent = (datatransfer::dnd::DragSourceDropEvent *)pData;

	if ( pDragEvent )
	{
		JavaDragSource *pSource = (JavaDragSource*)pDragEvent->DragSource.get();

		if ( pSource && pTrackDragOwner == pSource )
		{
			uno::Reference< datatransfer::dnd::XDragSourceListener > xListener( pSource->maListener );
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
	WeakComponentImplHelper3< datatransfer::dnd::XDragSource, lang::XInitialization, lang::XServiceInfo >( maMutex ),
	mnActions( datatransfer::dnd::DNDConstants::ACTION_NONE ),
	mpDraggingSource( nil ),
	mpPasteboardHelper( nil ),
	mpWindow( NULL )
{
}

// ------------------------------------------------------------------------

JavaDragSource::~JavaDragSource()
{
	aDragSources.remove( this );

	if ( pTrackDragOwner == this )
		pTrackDragOwner = NULL;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpPasteboardHelper )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpPasteboardHelper performSelectorOnMainThread:@selector(unregisterDragSourceAndReleaseView:) withObject:mpPasteboardHelper waitUntilDone:YES modes:pModes];

		[mpPasteboardHelper release];
	}

	[pPool release];
}

// ------------------------------------------------------------------------

void JavaDragSource::initialize( const uno::Sequence< uno::Any >& arguments ) throw( uno::RuntimeException )
{
	NSView *pView = nil;
	if ( arguments.getLength() > 1 )
	{
		sal_IntPtr nPtr = 0;
		arguments.getConstArray()[0] >>= nPtr;
		if ( nPtr )
			pView = (NSView *)nPtr;
		arguments.getConstArray()[1] >>= nPtr;
		if ( nPtr )
			mpWindow = (Window *)nPtr;
	}

	if ( !pView || !mpWindow )
		throw uno::RuntimeException();

	aDragSources.push_back( this );

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( !mpPasteboardHelper )
	{
		// Do not retain as invoking alloc disables autorelease
		mpPasteboardHelper = [[JavaDNDPasteboardHelper alloc] initWithDraggingSource:pView dragOwner:this];
		if ( mpPasteboardHelper )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[mpPasteboardHelper performSelectorOnMainThread:@selector(registerDragSource:) withObject:mpPasteboardHelper waitUntilDone:YES modes:pModes];
		}
	}

	[pPool release];
}

// ------------------------------------------------------------------------

sal_Bool JavaDragSource::isDragImageSupported() throw( uno::RuntimeException )
{
	return sal_False;
}

// ------------------------------------------------------------------------

sal_Int32 JavaDragSource::getDefaultCursor( sal_Int8 /* dragAction */ ) throw( lang::IllegalArgumentException, uno::RuntimeException )
{
	return 0;
}

// ------------------------------------------------------------------------

void JavaDragSource::startDrag( const datatransfer::dnd::DragGestureEvent& /* trigger */, sal_Int8 sourceActions, sal_Int32 /* cursor */, sal_Int32 /* image */, const uno::Reference< datatransfer::XTransferable >& transferable, const uno::Reference< datatransfer::dnd::XDragSourceListener >& listener ) throw( uno::RuntimeException )
{
	datatransfer::dnd::DragSourceDropEvent aDragEvent;
	aDragEvent.Source = uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( this ) );
	aDragEvent.DragSource = uno::Reference< datatransfer::dnd::XDragSource >( this );
	aDragEvent.DragSourceContext = uno::Reference< datatransfer::dnd::XDragSourceContext >( new JavaDragSourceContext() );
	aDragEvent.DropAction = datatransfer::dnd::DNDConstants::ACTION_NONE;
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

	bool bDragStarted = false;
	if ( maContents.is() && mpPasteboardHelper )
	{
		pTrackDragOwner = this;

		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		// Fix bug 3644 by releasing the application mutex so that the drag
		// code can display tooltip windows and dialogs without hanging
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		sal_uLong nCount = Application::ReleaseSolarMutex();
		[(JavaDNDPasteboardHelper *)mpPasteboardHelper performSelectorOnMainThread:@selector(startDrag:) withObject:mpPasteboardHelper waitUntilDone:YES modes:pModes];
		bDragStarted = [(JavaDNDPasteboardHelper *)mpPasteboardHelper dragStarted];
		Application::AcquireSolarMutex( nCount );

		[pPool release];

		// Make sure that we are still the drag owner
		if ( bDragStarted && pTrackDragOwner != this )
			bDragStarted = false;
	}

	if ( !bDragStarted )
	{
		mnActions = datatransfer::dnd::DNDConstants::ACTION_NONE;
		maContents.clear();
		maListener.clear();

		if ( listener.is() )
			listener->dragDropEnd( aDragEvent );
	}
}

// ------------------------------------------------------------------------

OUString JavaDragSource::getImplementationName() throw( uno::RuntimeException )
{
	return OUString::createFromAscii( "com.sun.star.datatransfer.dnd.JavaDragSource" );
}

// ------------------------------------------------------------------------

sal_Bool JavaDragSource::supportsService( const OUString& serviceName ) throw( uno::RuntimeException )
{
	uno::Sequence < OUString > aSupportedServicesNames = JavaDragSource_getSupportedServiceNames();

	for ( sal_Int32 n = aSupportedServicesNames.getLength(); n--; )
		if ( aSupportedServicesNames[n].compareTo(serviceName) == 0 )
			return sal_True;

	return sal_False;
}

// ------------------------------------------------------------------------

uno::Sequence< OUString > JavaDragSource::getSupportedServiceNames() throw( uno::RuntimeException )
{
	return JavaDragSource_getSupportedServiceNames();
}

// ------------------------------------------------------------------------

NSView *JavaDragSource::getNSView()
{
	return ( mpPasteboardHelper ? [mpPasteboardHelper getSource] : nil );
}

// ------------------------------------------------------------------------

void JavaDragSource::handleDrag( sal_Int32 /* nX */, sal_Int32 /* nY */ )
{
	datatransfer::dnd::DragSourceDragEvent aSourceDragEvent;
	aSourceDragEvent.Source = uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( this ) );
	aSourceDragEvent.DragSource = uno::Reference< datatransfer::dnd::XDragSource >( this );
	aSourceDragEvent.DropAction = mnActions;
	aSourceDragEvent.UserAction = ImplGetDropActionFromOperationMask( ImplGetOperationMaskFromActions( mnActions ), true );

	uno::Reference< datatransfer::dnd::XDragSourceListener > xListener( maListener );

	// Send source drag event
	if ( xListener.is() )
		xListener->dragOver( aSourceDragEvent );
}

// ========================================================================

JavaDropTarget::JavaDropTarget() :
	WeakComponentImplHelper3< datatransfer::dnd::XDropTarget, lang::XInitialization, lang::XServiceInfo >( maMutex ),
	mbActive( sal_True ),
	mnDefaultActions( datatransfer::dnd::DNDConstants::ACTION_NONE ),
	mpPasteboardHelper( nil ),
	mbRejected( false ),
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
	aDropTargets.remove( this );

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpPasteboardHelper )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(JavaDNDPasteboardHelper *)mpPasteboardHelper performSelectorOnMainThread:@selector(unregisterDraggedTypesAndReleaseView:) withObject:mpPasteboardHelper waitUntilDone:YES modes:pModes];

		[mpPasteboardHelper release];
		mpPasteboardHelper = nil;
	}

	[pPool release];

	mpWindow = NULL;
}

// --------------------------------------------------------------------------

void JavaDropTarget::initialize( const uno::Sequence< uno::Any >& arguments ) throw( uno::RuntimeException )
{
	NSView *pView = nil;

	if ( arguments.getLength() > 1 )
	{
		sal_uInt64 nPtr = 0;
		arguments.getConstArray()[0] >>= nPtr;
		if ( nPtr )
			pView = (NSView *)nPtr;
		arguments.getConstArray()[1] >>= nPtr;
		if ( nPtr )
			mpWindow = (Window *)nPtr;
	}

	if ( !pView || !mpWindow )
		throw uno::RuntimeException();

	aDropTargets.push_back( this );

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( !mpPasteboardHelper )
	{
		// Do not retain as invoking alloc disables autorelease
		mpPasteboardHelper = [[JavaDNDPasteboardHelper alloc] initWithDraggingDestination:pView newTypes:DTransTransferable::getSupportedPasteboardTypes()];
		if ( mpPasteboardHelper )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[(JavaDNDPasteboardHelper *)mpPasteboardHelper performSelectorOnMainThread:@selector(registerForDraggedTypes:) withObject:mpPasteboardHelper waitUntilDone:YES modes:pModes];
		}
	}

	[pPool release];
}

// --------------------------------------------------------------------------

void JavaDropTarget::addDropTargetListener( const uno::Reference< datatransfer::dnd::XDropTargetListener >& xListener ) throw( uno::RuntimeException )
{
	maListeners.push_back( xListener );
}

// --------------------------------------------------------------------------

void JavaDropTarget::removeDropTargetListener( const uno::Reference< datatransfer::dnd::XDropTargetListener >& xListener ) throw( uno::RuntimeException )
{
	maListeners.remove( xListener );
}

// --------------------------------------------------------------------------

sal_Bool JavaDropTarget::isActive() throw( uno::RuntimeException )
{
	return mbActive;
}

// --------------------------------------------------------------------------

void JavaDropTarget::setActive( sal_Bool active ) throw( uno::RuntimeException )
{
	mbActive = active;
}

// --------------------------------------------------------------------------

sal_Int8 JavaDropTarget::getDefaultActions() throw( uno::RuntimeException )
{
	return mnDefaultActions;
}

// --------------------------------------------------------------------------

void JavaDropTarget::setDefaultActions( sal_Int8 actions ) throw( uno::RuntimeException )
{
	mnDefaultActions = actions;
}

// --------------------------------------------------------------------------

OUString JavaDropTarget::getImplementationName() throw( uno::RuntimeException )
{
	return OUString::createFromAscii( "com.sun.star.datatransfer.dnd.JavaDropTarget"
 );
}

// ------------------------------------------------------------------------

sal_Bool JavaDropTarget::supportsService( const OUString& ServiceName ) throw( uno::RuntimeException )
{
	uno::Sequence < OUString > aSupportedServicesNames = JavaDropTarget_getSupportedServiceNames();

	for ( sal_Int32 n = aSupportedServicesNames.getLength(); n--; )
		if ( aSupportedServicesNames[n].compareTo(ServiceName) == 0 )
			return sal_True;

	return sal_False;
}

// ------------------------------------------------------------------------

uno::Sequence< OUString > JavaDropTarget::getSupportedServiceNames() throw( uno::RuntimeException )
{
	return JavaDropTarget_getSupportedServiceNames();
}

// ------------------------------------------------------------------------

NSView *JavaDropTarget::getNSView()
{
	return ( mpPasteboardHelper ? [mpPasteboardHelper getDestination] : nil );
}

// ------------------------------------------------------------------------

sal_Int8 JavaDropTarget::handleDragEnter( sal_Int32 nX, sal_Int32 nY, id aInfo )
{
	mbRejected = false;

	if ( !aInfo || ![aInfo conformsToProtocol:@protocol(NSDraggingInfo)] )
		return datatransfer::dnd::DNDConstants::ACTION_NONE;

	NSPasteboard *pPasteboard = [aInfo draggingPasteboard];
	if ( !pPasteboard )
		return datatransfer::dnd::DNDConstants::ACTION_NONE;

	datatransfer::dnd::DropTargetDragEnterEvent aDragEnterEvent;
	aDragEnterEvent.Source = uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( this ) );
	aDragEnterEvent.LocationX = nX;
	aDragEnterEvent.LocationY = nY;

	NSDragOperation nMask = [aInfo draggingSourceOperationMask];
	if ( pTrackDragOwner )
	{
		aDragEnterEvent.SourceActions = pTrackDragOwner->mnActions;
		aDragEnterEvent.DropAction = ImplGetDropActionFromOperationMask( nMask, pTrackDragOwner->getNSView() == getNSView() );
		aDragEnterEvent.SupportedDataFlavors = pTrackDragOwner->maContents->getTransferDataFlavors();
	}
	else
	{
		aDragEnterEvent.SourceActions = ImplGetActionsFromDragOperationMask( nMask );
		aDragEnterEvent.DropAction = ImplGetDropActionFromOperationMask( nMask, false );

		DTransTransferable *pTransferable = new DTransTransferable( [pPasteboard name] );
		if ( pTransferable )
		{
			aDragEnterEvent.SupportedDataFlavors = pTransferable->getTransferDataFlavors();
			delete pTransferable;
		}
	}

	// Set the cursor
	ImplSetCursorFromAction( aDragEnterEvent.DropAction, mpWindow );

	JavaDropTargetDragContext *pContext = new JavaDropTargetDragContext( aDragEnterEvent.DropAction );
	aDragEnterEvent.Context = uno::Reference< datatransfer::dnd::XDropTargetDragContext >( pContext );

	::std::list< uno::Reference< datatransfer::dnd::XDropTargetListener > > listeners( maListeners );

	for ( ::std::list< uno::Reference< datatransfer::dnd::XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragEnter( aDragEnterEvent );
	}

	// Fix bug 3647 by allowing the VCL event dispatch thread to run
	Application::Reschedule();

	mbRejected = pContext->isRejected();
	if ( mbRejected )
		return datatransfer::dnd::DNDConstants::ACTION_NONE;
	else
		return aDragEnterEvent.DropAction;
}

// ------------------------------------------------------------------------

void JavaDropTarget::handleDragExit( sal_Int32 nX, sal_Int32 nY, id aInfo )
{
	if ( !aInfo || ![aInfo conformsToProtocol:@protocol(NSDraggingInfo)] )
		return;

	NSPasteboard *pPasteboard = [aInfo draggingPasteboard];
	if ( !pPasteboard )
		return;

	datatransfer::dnd::DropTargetDragEvent aDragEvent;
	aDragEvent.Source = uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( this ) );
	aDragEvent.LocationX = nX;
	aDragEvent.LocationY = nY;

	NSDragOperation nMask = [aInfo draggingSourceOperationMask];
	if ( pTrackDragOwner )
	{
		aDragEvent.SourceActions = pTrackDragOwner->mnActions;
		aDragEvent.DropAction = ImplGetDropActionFromOperationMask( nMask, pTrackDragOwner->getNSView() == getNSView() );
	}
	else
	{
		aDragEvent.SourceActions = ImplGetActionsFromDragOperationMask( nMask );
		aDragEvent.DropAction = ImplGetDropActionFromOperationMask( nMask, false );
	}

	// Set the cursor
	ImplSetCursorFromAction( aDragEvent.DropAction, mpWindow );

	JavaDropTargetDragContext *pContext = new JavaDropTargetDragContext( aDragEvent.DropAction );
	aDragEvent.Context = uno::Reference< datatransfer::dnd::XDropTargetDragContext >( pContext );

	::std::list< uno::Reference< datatransfer::dnd::XDropTargetListener > > listeners( maListeners );

	for ( ::std::list< uno::Reference< datatransfer::dnd::XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragExit( aDragEvent );
	}

	// Fix bug 3647 by allowing the VCL event dispatch thread to run
	Application::Reschedule();
}

// ------------------------------------------------------------------------

sal_Int8 JavaDropTarget::handleDragOver( sal_Int32 nX, sal_Int32 nY, id aInfo )
{
	if ( !aInfo || ![aInfo conformsToProtocol:@protocol(NSDraggingInfo)] )
		return datatransfer::dnd::DNDConstants::ACTION_NONE;

	NSPasteboard *pPasteboard = [aInfo draggingPasteboard];
	if ( !pPasteboard )
		return datatransfer::dnd::DNDConstants::ACTION_NONE;

	datatransfer::dnd::DropTargetDragEvent aDragEvent;
	aDragEvent.Source = uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( this ) );
	aDragEvent.LocationX = nX;
	aDragEvent.LocationY = nY;

	NSDragOperation nMask = [aInfo draggingSourceOperationMask];
	if ( pTrackDragOwner )
	{
		aDragEvent.SourceActions = pTrackDragOwner->mnActions;
		aDragEvent.DropAction = ImplGetDropActionFromOperationMask( nMask, pTrackDragOwner->getNSView() == getNSView() );
	}
	else
	{
		aDragEvent.SourceActions = ImplGetActionsFromDragOperationMask( nMask );
		aDragEvent.DropAction = ImplGetDropActionFromOperationMask( nMask, false);
	}

	// Set the cursor
	ImplSetCursorFromAction( aDragEvent.DropAction, mpWindow );

	JavaDropTargetDragContext *pContext = new JavaDropTargetDragContext( aDragEvent.DropAction );
	aDragEvent.Context = uno::Reference< datatransfer::dnd::XDropTargetDragContext >( pContext );

	::std::list< uno::Reference< datatransfer::dnd::XDropTargetListener > > listeners( maListeners );

	for ( ::std::list< uno::Reference< datatransfer::dnd::XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragOver( aDragEvent );
	}

	// Fix bug 3647 by allowing the VCL event dispatch thread to run
	Application::Reschedule();

	mbRejected = pContext->isRejected();
	if ( mbRejected )
		return datatransfer::dnd::DNDConstants::ACTION_NONE;
	else
		return aDragEvent.DropAction;
}

// ------------------------------------------------------------------------

bool JavaDropTarget::handleDrop( sal_Int32 nX, sal_Int32 nY, id aInfo )
{
	bool bRet = false;

	if ( mbRejected || !aInfo || ![aInfo conformsToProtocol:@protocol(NSDraggingInfo)] )
		return bRet;

	NSPasteboard *pPasteboard = [aInfo draggingPasteboard];
	if ( !pPasteboard )
		return bRet;

	datatransfer::dnd::DropTargetDropEvent aDropEvent;
	aDropEvent.Source = uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( this ) );
	aDropEvent.LocationX = nX;
	aDropEvent.LocationY = nY;

	NSDragOperation nMask = [aInfo draggingSourceOperationMask];
	if ( pTrackDragOwner )
	{
		aDropEvent.SourceActions = pTrackDragOwner->mnActions;
		aDropEvent.DropAction = ImplGetDropActionFromOperationMask( nMask, pTrackDragOwner->getNSView() == getNSView() );
		aDropEvent.Transferable = pTrackDragOwner->maContents;
	}
	else
	{
		aDropEvent.SourceActions = ImplGetActionsFromDragOperationMask( nMask );
		aDropEvent.DropAction = ImplGetDropActionFromOperationMask( nMask, false );

		DTransTransferable *pTransferable = new DTransTransferable( [pPasteboard name] );
		if ( pTransferable )
			aDropEvent.Transferable = uno::Reference< datatransfer::XTransferable >( pTransferable );
	}

	JavaDropTargetDropContext *pContext = new JavaDropTargetDropContext( aDropEvent.DropAction );
	aDropEvent.Context = uno::Reference< datatransfer::dnd::XDropTargetDropContext >( pContext );

	::std::list< uno::Reference< datatransfer::dnd::XDropTargetListener > > listeners( maListeners );

	for ( ::std::list< uno::Reference< datatransfer::dnd::XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->drop( aDropEvent );
	}

	// Fix bug 3647 by allowing the VCL event dispatch thread to run
	Application::Reschedule();

	// Fix bug reported in the following NeoOffice forum topic by only
	// returning true if none of the listeners have rejected the drop and
	// one of the listeners set the drop as successfully completed:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8457
	mbRejected = pContext->isRejected();
	bRet = ( !mbRejected && pContext->getDropComplete() );

	return bRet;
}

// ========================================================================

uno::Reference< uno::XInterface > JavaSalInstance::CreateDragSource()
{
	return uno::Reference< uno::XInterface >( static_cast< lang::XInitialization* >( new JavaDragSource() ) );
}

// ------------------------------------------------------------------------

uno::Reference< uno::XInterface > JavaSalInstance::CreateDropTarget()
{
	return uno::Reference< uno::XInterface >( static_cast< lang::XInitialization* >( new JavaDropTarget() ) );
}
