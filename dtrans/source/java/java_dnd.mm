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
static sal_Int8 nCurrentAction = DNDConstants::ACTION_NONE;

static Point ImplGetPointFromNSPoint( NSPoint aPoint, NSWindow *pWindow );
static sal_Int8 ImplGetActionsFromDragOperationMask( NSDragOperation nMask );
static NSDragOperation ImplGetOperationFromActions( sal_Int8 nActions );
static sal_Int8 ImplGetDropActionFromOperationMask( NSDragOperation nMask );

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

	Point aPos( ImplGetPointFromNSPoint( [pSender draggedImageLocation], pWindow ) );
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

	Point aPos( ImplGetPointFromNSPoint( [pSender draggedImageLocation], pWindow ) );
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

	Point aPos( ImplGetPointFromNSPoint( [pSender draggedImageLocation], pWindow ) );
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

	Point aPos( ImplGetPointFromNSPoint( [pSender draggedImageLocation], pWindow ) );
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
}

- (void)draggedImage:(NSImage *)pImage endedAt:(NSPoint)aPoint operation:(NSDragOperation)nOperation
{
	if ( !mpSource )
		return;
 
	NSWindow *pWindow = [mpSource window];
	if ( !pWindow )
		return;
 
	Point aPos( ImplGetPointFromNSPoint( aPoint, pWindow ) );
}

- (void)draggedImage:(NSImage *)pImage movedTo:(NSPoint)aPoint
{
	if ( !mpSource )
		return;
 
	NSWindow *pWindow = [mpSource window];
	if ( !pWindow )
		return;
 
	Point aPos( ImplGetPointFromNSPoint( aPoint, pWindow ) );
}

- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)bLocal
{
	NSDragOperation bRet = NSDragOperationNone;
	if ( !mpSource )
		return bRet;
 
	NSWindow *pWindow = [mpSource window];
	if ( !pWindow )
		return bRet;
 
	return bRet;
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
		sal_Int32 nPtr = 0;
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

	if ( maContents.is() && mpPasteboardHelper )
	{
		pTrackDragOwner = this;

		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[(JavaDNDPasteboardHelper *)mpPasteboardHelper performSelectorOnMainThread:@selector(startDrag:) withObject:mpPasteboardHelper waitUntilDone:YES modes:pModes];
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

NSView *JavaDragSource::getNSView()
{
	return ( mpPasteboardHelper ? [mpPasteboardHelper getSource] : nil );
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
		sal_Int32 nPtr = 0;
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
