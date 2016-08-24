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

#include "java_dnd.hxx"
#include "java_dndcontext.hxx"
#include "DTransTransferable.hxx"
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vos/mutex.hxx>

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

static ::std::list< ::java::JavaDragSource* > aDragSources;
static ::std::list< ::java::JavaDropTarget* > aDropTargets;
static JavaDragSource *pTrackDragOwner = NULL;

static Point ImplGetPointFromNSPoint( NSPoint aPoint, NSWindow *pWindow );
static sal_Int8 ImplGetActionsFromDragOperationMask( NSDragOperation nMask );
static NSDragOperation ImplGetOperationMaskFromActions( sal_Int8 nActions );
static NSDragOperation ImplGetOperationFromActions( sal_Int8 nActions );
static sal_Int8 ImplGetDropActionFromOperationMask( NSDragOperation nMask, bool bSame );
static void ImplSetCursorFromAction( sal_Int8 nAction, Window *pWindow );

// ========================================================================

@interface NSWindow (VCLWindow )
- (void)setDraggingSourceDelegate:(id)pDelegate;
@end

@interface NSView (VCLView)
- (void)setDraggingDestinationDelegate:(id)pDelegate;
- (void)setDraggingSourceDelegate:(id)pDelegate;
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

@interface JavaDNDDraggingSource : NSObject
{
	NSView*						mpSource;
}
- (void)dealloc;
- (void)draggedImage:(NSImage *)pImage beganAt:(NSPoint)aPoint;
- (void)draggedImage:(NSImage *)pImage endedAt:(NSPoint)aPoint operation:(NSDragOperation)nOperation;
- (void)draggedImage:(NSImage *)pImage movedTo:(NSPoint)aPoint;
- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)bLocal;
- (BOOL)ignoreModifierKeysWhileDragging;
- (id)initWithView:(NSView *)pSource;
- (NSArray *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)pDropDestination;
@end

@interface JavaDNDPasteboardHelper : NSObject
{
	NSView*						mpDestination;
	JavaDNDDraggingSource*		mpDraggingSource;
	NSEvent*					mpLastMouseEvent;
	NSArray*					mpNewTypes;
	NSView*						mpSource;
}
- (NSView *)getDestination;
- (NSView *)getSource;
- (id)initWithDraggingDestination:(NSView *)pDestination newTypes:(NSArray *)pNewTypes;
- (id)initWithDraggingSource:(NSView *)pSource;
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
	mpLastMouseEvent = nil;
	mpNewTypes = pNewTypes;
	if ( mpNewTypes )
		[mpNewTypes retain];
	mpSource = nil;

	return self;
}

- (id)initWithDraggingSource:(NSView *)pSource
{
	[super init];

	mpDestination = nil;
	mpDraggingSource = nil;
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
	if ( mpDraggingSource && mpLastMouseEvent && mpSource )
	{
		NSImage *pImage = [[NSImage alloc] initWithSize:NSMakeSize( 1, 1 )];
		if ( pImage )
		{
			[pImage autorelease];

			NSPasteboard *pPasteboard = [NSPasteboard pasteboardWithName:NSDragPboard];
			if ( pPasteboard )
			{
				NSPoint aPoint = [mpSource convertPoint:[mpLastMouseEvent locationInWindow] fromView:nil];
				[mpSource dragImage:pImage at:aPoint offset:NSMakeSize( 0, 0 ) event:mpLastMouseEvent pasteboard:pPasteboard source:mpDraggingSource slideBack:YES];
			}
		}
	}
}

- (void)unregisterDraggedTypesAndReleaseView:(id)pSender
{
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

	if ( !mpDestination || !pSender )
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
	return YES;
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

- (void)draggedImage:(NSImage *)pImage beganAt:(NSPoint)aPoint
{
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

- (void)draggedImage:(NSImage *)pImage endedAt:(NSPoint)aPoint operation:(NSDragOperation)nOperation
{
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
			{
				pSource->handleDrag( aPos.X(), aPos.Y() );

				// Dispatch drop event
				DragSourceDropEvent *pDragEvent = new DragSourceDropEvent();
				pDragEvent->Source = Reference< XInterface >( static_cast< OWeakObject* >( pSource ) );
				pDragEvent->DragSource = Reference< XDragSource >( pSource );
				pDragEvent->DragSourceContext = Reference< XDragSourceContext >( new DragSourceContext() );
				pDragEvent->DropAction = ImplGetDropActionFromOperationMask( nOperation, true );
				pDragEvent->DropSuccess = ( pDragEvent->DropAction == DNDConstants::ACTION_NONE ? sal_False : sal_True );

				// Reset cursor to window's VCL pointer
				ImplSetCursorFromAction( DNDConstants::ACTION_NONE, pTrackDragOwner->mpWindow );

				// Fix bug 1442 by dispatching and deleting the
				// DragSourceDropEvent in the VCL event dispatch thread
				Application::PostUserEvent( STATIC_LINK( NULL, JavaDragSource, dragDropEnd ), pDragEvent );
			}
		}

		rSolarMutex.release();
	}
}

- (void)draggedImage:(NSImage *)pImage movedTo:(NSPoint)aPoint
{
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

- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)bLocal
{
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

- (BOOL)ignoreModifierKeysWhileDragging
{
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

- (NSArray *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)pDropDestination
{
	return nil;
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
	sal_Int8 nRet = DNDConstants::ACTION_NONE;

	if ( nMask & ( NSDragOperationCopy | NSDragOperationGeneric ) )
		nRet |= DNDConstants::ACTION_COPY;
	if ( nMask & ( NSDragOperationMove ) )
		nRet |= DNDConstants::ACTION_MOVE;
	if ( nMask & ( NSDragOperationLink ) )
		nRet |= DNDConstants::ACTION_LINK;

	// If more than one action, add default action to signal that the drop
	// target needs to decide which action to use
	if ( nRet != DNDConstants::ACTION_NONE && nRet != DNDConstants::ACTION_COPY && nRet != DNDConstants::ACTION_MOVE && nRet != DNDConstants::ACTION_LINK )
		nRet |= DNDConstants::ACTION_DEFAULT;

	return nRet;
}

// ------------------------------------------------------------------------

static NSDragOperation ImplGetOperationMaskFromActions( sal_Int8 nActions )
{
	NSDragOperation nRet = NSDragOperationNone;

	if ( nActions & DNDConstants::ACTION_COPY )
		nRet |= NSDragOperationCopy;
	if ( nActions & DNDConstants::ACTION_MOVE )
		nRet |= NSDragOperationMove;
	if ( nActions & DNDConstants::ACTION_LINK )
		nRet |= NSDragOperationLink;

	return nRet;
}

// ------------------------------------------------------------------------

static NSDragOperation ImplGetOperationFromActions( sal_Int8 nActions )
{
	NSDragOperation nRet = NSDragOperationNone;

	if ( nActions & DNDConstants::ACTION_COPY )
		nRet = NSDragOperationCopy;
	if ( nActions & DNDConstants::ACTION_MOVE )
		nRet = NSDragOperationMove;
	if ( nActions & DNDConstants::ACTION_LINK )
		nRet = NSDragOperationLink;

	return nRet;
}

// ------------------------------------------------------------------------

static sal_Int8 ImplGetDropActionFromOperationMask( NSDragOperation nMask, bool bSame )
{
	sal_Int8 nRet = DNDConstants::ACTION_NONE;
	int nActionCount = 0;

	// When the source and destination are the same, moving is preferred over
	// copying
	if ( bSame )
	{
		if ( nMask & NSDragOperationLink )
		{
			nRet = DNDConstants::ACTION_LINK;
			nActionCount++;
		}
		if ( nMask & ( NSDragOperationCopy | NSDragOperationGeneric ) )
		{
			nRet = DNDConstants::ACTION_COPY;
			nActionCount++;
		}
		if ( nMask & NSDragOperationMove )
		{
			nRet = DNDConstants::ACTION_MOVE;
			nActionCount++;
		}
	}
	else
	{
		if ( nMask & NSDragOperationLink )
		{
			nRet = DNDConstants::ACTION_LINK;
			nActionCount++;
		}
		if ( nMask & NSDragOperationMove )
		{
			nRet = DNDConstants::ACTION_MOVE;
			nActionCount++;
		}
		if ( nMask & ( NSDragOperationCopy | NSDragOperationGeneric ) )
		{
			nRet = DNDConstants::ACTION_COPY;
			nActionCount++;
		}
	}

	// If more than one action, add default action to signal that the drop
	// target needs to decide which action to use
	if ( nActionCount > 1 )
		nRet |= DNDConstants::ACTION_DEFAULT;

	return nRet;
}

// ------------------------------------------------------------------------

static void ImplSetCursorFromAction( sal_Int8 nAction, Window *pWindow )
{
	bool bSet = false;

	nAction &= ~DNDConstants::ACTION_DEFAULT;
	if ( nAction == DNDConstants::ACTION_NONE )
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
	else if ( nAction == DNDConstants::ACTION_MOVE )
	{
		NSCursor *pCursor = [NSCursor closedHandCursor];
		if ( pCursor )
		{
			[pCursor set];
			bSet = true;
		}
	}
	else if ( nAction == DNDConstants::ACTION_COPY )
	{
		NSCursor *pCursor = [NSCursor dragCopyCursor];
		if ( pCursor )
		{
			[pCursor set];
			bSet = true;
		}
	}
	else if ( nAction == DNDConstants::ACTION_LINK )
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

void SAL_CALL JavaDragSource::initialize( const Sequence< Any >& arguments ) throw( RuntimeException )
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
		throw RuntimeException();

	aDragSources.push_back( this );

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( !mpPasteboardHelper )
	{
		// Do not retain as invoking alloc disables autorelease
		mpPasteboardHelper = [[JavaDNDPasteboardHelper alloc] initWithDraggingSource:pView];
		if ( mpPasteboardHelper )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[mpPasteboardHelper performSelectorOnMainThread:@selector(registerDragSource:) withObject:mpPasteboardHelper waitUntilDone:YES modes:pModes];
		}
	}

	[pPool release];
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

	bool bDragStarted = false;
	if ( maContents.is() && mpPasteboardHelper )
	{
		DTransTransferable *pTransferable = new DTransTransferable( NSDragPboard );
		if ( pTransferable )
		{
			pTrackDragOwner = this;
			pTransferable->setContents( maContents, sal_True );

			NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

			// Fix bug 3644 by releasing the application mutex so that the drag
			// code can display tooltip windows and dialogs without hanging
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			ULONG nCount = Application::ReleaseSolarMutex();
			[(JavaDNDPasteboardHelper *)mpPasteboardHelper performSelectorOnMainThread:@selector(startDrag:) withObject:mpPasteboardHelper waitUntilDone:YES modes:pModes];
			Application::AcquireSolarMutex( nCount );

			[pPool release];

			// Make sure that we are still the drag owner
			if ( pTrackDragOwner == this )
				bDragStarted = true;
		}
	}

	if ( !bDragStarted )
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

NSView *JavaDragSource::getNSView()
{
	return ( mpPasteboardHelper ? [mpPasteboardHelper getSource] : nil );
}

// ------------------------------------------------------------------------

void JavaDragSource::handleDrag( sal_Int32 nX, sal_Int32 nY )
{
	DragSourceDragEvent aSourceDragEvent;
	aSourceDragEvent.Source = Reference< XInterface >( static_cast< OWeakObject* >( this ) );
	aSourceDragEvent.DragSource = Reference< XDragSource >( this );
	aSourceDragEvent.DropAction = mnActions;
	aSourceDragEvent.UserAction = ImplGetDropActionFromOperationMask( ImplGetOperationMaskFromActions( mnActions ), true );

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

void SAL_CALL JavaDropTarget::initialize( const Sequence< Any >& arguments ) throw( RuntimeException )
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
		throw RuntimeException();

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

NSView *JavaDropTarget::getNSView()
{
	return ( mpPasteboardHelper ? [mpPasteboardHelper getDestination] : nil );
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

	DropTargetDragContext *pContext = new DropTargetDragContext( aDragEnterEvent.DropAction );
	aDragEnterEvent.Context = Reference< XDropTargetDragContext >( pContext );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragEnter( aDragEnterEvent );
	}

	// Fix bug 3647 by allowing the VCL event dispatch thread to run
	Application::Reschedule();

	mbRejected = pContext->isRejected();
	if ( mbRejected )
		return DNDConstants::ACTION_NONE;
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

	DropTargetDragEvent aDragEvent;
	aDragEvent.Source = Reference< XInterface >( static_cast< OWeakObject* >( this ) );
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

	DropTargetDragContext *pContext = new DropTargetDragContext( aDragEvent.DropAction );
	aDragEvent.Context = Reference< XDropTargetDragContext >( pContext );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
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
		return DNDConstants::ACTION_NONE;

	NSPasteboard *pPasteboard = [aInfo draggingPasteboard];
	if ( !pPasteboard )
		return DNDConstants::ACTION_NONE;

	DropTargetDragEvent aDragEvent;
	aDragEvent.Source = Reference< XInterface >( static_cast< OWeakObject* >( this ) );
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

	DropTargetDragContext *pContext = new DropTargetDragContext( aDragEvent.DropAction );
	aDragEvent.Context = Reference< XDropTargetDragContext >( pContext );

	list< Reference< XDropTargetListener > > listeners( maListeners );

	for ( list< Reference< XDropTargetListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
	{
		if ( (*it).is() )
			(*it)->dragOver( aDragEvent );
	}

	// Fix bug 3647 by allowing the VCL event dispatch thread to run
	Application::Reschedule();

	mbRejected = pContext->isRejected();
	if ( mbRejected )
		return DNDConstants::ACTION_NONE;
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

	DropTargetDropEvent aDropEvent;
	aDropEvent.Source = Reference< XInterface >( static_cast< OWeakObject* >( this ) );
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
