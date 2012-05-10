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
 *  Patrick Luby, July 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 Planamesa Inc.
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

#ifndef __VCLEVENTQUEUE_COCOA_H__
#define __VCLEVENTQUEUE_COCOA_H__

#include <com/sun/star/vcl/VCLEventQueue.hxx>
#include <jni.h>

#ifdef __OBJC__

#ifdef USE_NATIVE_EVENTS
@interface VCLView : NSView <NSTextInputClient>
- (MacOSBOOL)acceptsFirstResponder;
- (MacOSBOOL)hasMarkedText;
- (NSRange)markedRange;
- (NSRange)selectedRange;
- (void)setMarkedText:(id)aString selectedRange:(NSRange)aSelectedRange replacementRange:(NSRange)aReplacementRange;
- (void)unmarkText;
- (NSArray *)validAttributesForMarkedText;
- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)pActualRange;
- (void)insertText:(id)aString replacementRange:(NSRange)aReplacementRange;
- (NSUInteger)characterIndexForPoint:(NSPoint)aPoint;
- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)pActualRange;
- (void)doCommandBySelector:(SEL)aSelector;
- (void)insertText:(id)aString;
#else	// USE_NATIVE_EVENTS
@interface VCLView : NSView
+ (void)swizzleSelectors:(NSView *)pView;
#endif	// USE_NATIVE_EVENTS
- (void)concludeDragOperation:(id < NSDraggingInfo >)pSender;
- (void)dragImage:(NSImage *)pImage at:(NSPoint)aImageLocation offset:(NSSize)aMouseOffset event:(NSEvent *)pEvent pasteboard:(NSPasteboard *)pPasteboard source:(id)pSourceObject slideBack:(MacOSBOOL)bSlideBack;
- (void)draggedImage:(NSImage *)pImage beganAt:(NSPoint)aPoint;
- (void)draggedImage:(NSImage *)pImage endedAt:(NSPoint)aPoint operation:(NSDragOperation)nOperation;
- (void)draggedImage:(NSImage *)pImage movedTo:(NSPoint)aPoint;
- (id)draggingDestinationDelegate;
- (void)draggingEnded:(id < NSDraggingInfo >)pSender;
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)pSender;
- (void)draggingExited:(id < NSDraggingInfo >)pSender;
- (id)draggingSourceDelegate;
- (NSDragOperation)draggingSourceOperationMaskForLocal:(MacOSBOOL)bLocal;
- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)pSender;
#ifdef USE_NATIVE_WINDOW
- (void)drawRect:(NSRect)aDirtyRect;
- (void)resetCursorRects;
#endif	// USE_NATIVE_WINDOW
- (MacOSBOOL)ignoreModifierKeysWhileDragging;
- (id)initWithFrame:(NSRect)aFrame;
- (MacOSBOOL)isOpaque;
- (NSArray *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)pDropDestination;
- (MacOSBOOL)performDragOperation:(id < NSDraggingInfo >)pSender;
- (MacOSBOOL)prepareForDragOperation:(id < NSDraggingInfo >)pSender;
- (MacOSBOOL)readSelectionFromPasteboard:(NSPasteboard *)pPasteboard;
#if !defined USE_NATIVE_EVENTS && !defined USE_ROUNDED_BOTTOM_CORNERS_IN_JAVA_FRAMES
- (NSSize)_bottomCornerSize;
#endif	// !USE_NATIVE_EVENTS && !USE_ROUNDED_BOTTOM_CORNERS_IN_JAVA_FRAMES
- (void)setDraggingDestinationDelegate:(id)pDelegate;
- (void)setDraggingSourceDelegate:(id)pDelegate;
- (id)validRequestorForSendType:(NSString *)pSendType returnType:(NSString *)pReturnType;
- (MacOSBOOL)wantsPeriodicDraggingUpdates;
- (MacOSBOOL)writeSelectionToPasteboard:(NSPasteboard *)pPasteboard types:(NSArray *)pTypes;
@end

@interface NSWindow (VCLWindow)
- (void)_clearModalWindowLevel;
- (NSRect)_frameOnExitFromFullScreen;
#ifndef USE_NATIVE_EVENTS
- (MacOSBOOL)_isUtilityWindow;
#endif	// !USE_NATIVE_EVENTS
- (void)_restoreModalWindowLevel;
- (void)_setModalWindowLevel;
#ifndef USE_NATIVE_EVENTS
- (void)_setUtilityWindow:(MacOSBOOL)bUtilityWindow;
#endif	// !USE_NATIVE_EVENTS
- (MacOSBOOL)inLiveResize;
@end

@interface VCLPanel : NSPanel
{
#ifdef USE_NATIVE_EVENTS
	MacOSBOOL				mbAllowKeyBindings;
	MacOSBOOL				mbCanBecomeKeyOrMainWindow;
	NSUInteger				mnIgnoreMouseReleasedModifiers;
	JavaSalFrame*			mpFrame;
	ULONG					mnLastMetaModifierReleasedTime;
#endif	// USE_NATIVE_EVENTS
}
#ifdef USE_NATIVE_EVENTS
- (MacOSBOOL)canBecomeKeyOrMainWindow;
- (void)setAllowKeyBindings:(MacOSBOOL)bAllowKeyBindings;
- (void)setCanBecomeKeyOrMainWindow:(MacOSBOOL)bCanBecomeKeyOrMainWindow;
- (void)setFrame:(JavaSalFrame *)pFrame;
#endif	// USE_NATIVE_EVENTS
@end

@interface VCLWindow : NSWindow
{
#ifdef USE_NATIVE_EVENTS
	MacOSBOOL				mbAllowKeyBindings;
	MacOSBOOL				mbCanBecomeKeyOrMainWindow;
	NSUInteger				mnIgnoreMouseReleasedModifiers;
	JavaSalFrame*			mpFrame;
	ULONG					mnLastMetaModifierReleasedTime;
#endif	// USE_NATIVE_EVENTS
}
+ (void)clearModalWindowLevel;
+ (void)restoreModalWindowLevel;
+ (void)swizzleSelectors:(NSWindow *)pWindow;
- (void)becomeKeyWindow;
#ifdef USE_NATIVE_EVENTS
- (MacOSBOOL)canBecomeKeyOrMainWindow;
#endif	// USE_NATIVE_EVENTS
- (void)displayIfNeeded;
- (id)draggingSourceDelegate;
- (id)initWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nStyle backing:(NSBackingStoreType)nBufferingType defer:(MacOSBOOL)bDeferCreation;
- (id)initWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nStyle backing:(NSBackingStoreType)nBufferingType defer:(MacOSBOOL)bDeferCreation screen:(NSScreen *)pScreen;
- (MacOSBOOL)makeFirstResponder:(NSResponder *)pResponder;
- (void)makeKeyWindow;
- (void)orderWindow:(NSWindowOrderingMode)nOrderingMode relativeTo:(int)nOtherWindowNumber;
- (MacOSBOOL)performKeyEquivalent:(NSEvent *)pEvent;
- (void)resignKeyWindow;
- (void)sendEvent:(NSEvent *)pEvent;
#ifdef USE_NATIVE_EVENTS
- (void)setAllowKeyBindings:(MacOSBOOL)bAllowKeyBindings;
- (void)setCanBecomeKeyOrMainWindow:(MacOSBOOL)bCanBecomeKeyOrMainWindow;
- (void)setFrame:(JavaSalFrame *)pFrame;
#endif	// USE_NATIVE_EVENTS
- (void)setContentView:(NSView *)pView;
- (void)setDraggingSourceDelegate:(id)pDelegate;
- (void)setLevel:(int)nWindowLevel;
- (void)windowDidExitFullScreen:(NSNotification *)pNotification;
- (void)windowWillEnterFullScreen:(NSNotification *)pNotification;
#ifdef USE_NATIVE_EVENTS
- (void)windowDidMove:(NSNotification *)pNotification;
- (void)windowDidResize:(NSNotification *)pNotification;
- (MacOSBOOL)windowShouldClose:(id)pObject;
#endif	// USE_NATIVE_EVENTS
@end

#ifdef USE_NATIVE_WINDOW
SAL_DLLPRIVATE void JavaSalFrame_drawToNSView( NSView *pView, NSRect aDirtyRect );
SAL_DLLPRIVATE NSCursor *JavaSalFrame_getCursor( NSView *pView );
#endif	// USE_NATIVE_WINDOW

#endif	// __OBJC__

SAL_DLLPRIVATE void VCLEventQueue_cancelTermination();
SAL_DLLPRIVATE void VCLEventQueue_fullScreen( void *pNSWindow, BOOL bFullScreen );
SAL_DLLPRIVATE void VCLEventQueue_getTextSelection( void *pNSWindow, CFStringRef *pTextSelection, CFDataRef *pRTFSelection );
SAL_DLLPRIVATE BOOL VCLEventQueue_paste( void *pNSWindow );
#ifndef USE_NATIVE_EVENTS
SAL_DLLPRIVATE BOOL VCLEventQueue_postCommandEvent( jobject aPeer, short nKey, short nModifiers, jchar nOriginalKeyChar, short nOriginalModifiers );
SAL_DLLPRIVATE void VCLEventQueue_postMouseWheelEvent( jobject aPeer, long nX, long nY, long nRotationX, long nRotationY, BOOL bShiftDown, BOOL bMetaDown, BOOL bAltDown, BOOL bControlDown );
SAL_DLLPRIVATE void VCLEventQueue_postWindowMoveSessionEvent( jobject aPeer, long nX, long nY, BOOL bStartSession );
#endif	// !USE_NATIVE_EVENTS
SAL_DLLPRIVATE void VCLEventQueue_removeCachedEvents();
SAL_DLLPRIVATE sal_Bool NSApplication_isActive();
SAL_DLLPRIVATE void NSFontManager_acquire();
SAL_DLLPRIVATE void NSFontManager_release();
SAL_DLLPRIVATE void VCLEventQueue_installVCLEventQueueClasses();

#endif
