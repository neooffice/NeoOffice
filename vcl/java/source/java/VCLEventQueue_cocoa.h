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

#include <salframe.h>

// Comment out the following line to disable full screen mode
#define USE_NATIVE_FULL_SCREEN_MODE

#ifdef __OBJC__

@interface VCLView : NSView <NSTextInputClient>
{
	JavaSalFrame*			mpFrame;
	MacOSBOOL				mbInKeyDown;
	NSObject*				mpInputManager;
	NSEvent*				mpLastKeyDownEvent;
	SalKeyEvent*			mpPendingKeyUpEvent;
	NSRange					maSelectedRange;
	id						mpTextInput;
	NSRange					maTextInputRange;
	MacOSBOOL				mbTextInputWantsNonRepeatKeyDown;
}
- (MacOSBOOL)acceptsFirstResponder;
- (void)abandonInput;
- (id)accessibilityAttributeValue:(NSString *)aAttribute;
- (MacOSBOOL)accessibilityIsIgnored;
- (void)dealloc;
- (void)keyDown:(NSEvent *)pEvent;
- (void)keyUp:(NSEvent *)pEvent;
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
- (void)setJavaFrame:(JavaSalFrame *)pFrame;
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
- (void)drawRect:(NSRect)aDirtyRect;
- (void)resetCursorRects;
- (MacOSBOOL)ignoreModifierKeysWhileDragging;
- (id)initWithFrame:(NSRect)aFrame;
- (MacOSBOOL)isOpaque;
- (NSArray *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)pDropDestination;
- (MacOSBOOL)performDragOperation:(id < NSDraggingInfo >)pSender;
- (MacOSBOOL)prepareForDragOperation:(id < NSDraggingInfo >)pSender;
- (MacOSBOOL)readSelectionFromPasteboard:(NSPasteboard *)pPasteboard;
- (void)setDraggingDestinationDelegate:(id)pDelegate;
- (void)setDraggingSourceDelegate:(id)pDelegate;
- (id)validRequestorForSendType:(NSString *)pSendType returnType:(NSString *)pReturnType;
- (MacOSBOOL)wantsPeriodicDraggingUpdates;
- (MacOSBOOL)writeSelectionToPasteboard:(NSPasteboard *)pPasteboard types:(NSArray *)pTypes;
@end

@interface NSWindow (VCLWindow)
+ (void)setAllowsAutomaticWindowTabbing:(MacOSBOOL)bAllowsAutomaticWindowTabbing;
- (void)_clearModalWindowLevel;
- (NSRect)_frameOnExitFromFullScreen;
- (void)_restoreModalWindowLevel;
- (void)_setModalWindowLevel;
@end

@interface VCLPanel : NSPanel
{
	MacOSBOOL				mbCanBecomeKeyWindow;
	NSUInteger				mnIgnoreMouseReleasedModifiers;
	JavaSalFrame*			mpFrame;
	ULONG					mnLastMetaModifierReleasedTime;
	NSEvent*				mpLastWindowDraggedEvent;
	MacOSBOOL				mbInVersionBrowser;
	MacOSBOOL				mbCloseOnExitVersionBrowser;
}
- (void)_init;
- (MacOSBOOL)canBecomeKeyWindow;
- (void)dealloc;
- (void)setCanBecomeKeyWindow:(MacOSBOOL)bCanBecomeKeyWindow;
- (void)setJavaFrame:(JavaSalFrame *)pFrame;
@end

@interface VCLWindow : NSWindow
{
	MacOSBOOL				mbCanBecomeKeyWindow;
	NSUInteger				mnIgnoreMouseReleasedModifiers;
	JavaSalFrame*			mpFrame;
	ULONG					mnLastMetaModifierReleasedTime;
	NSEvent*				mpLastWindowDraggedEvent;
	MacOSBOOL				mbInVersionBrowser;
	MacOSBOOL				mbCloseOnExitVersionBrowser;
}
+ (void)clearModalWindowLevel;
+ (void)restoreModalWindowLevel;
+ (void)swizzleSelectors:(NSWindow *)pWindow;
- (void)_init;
- (void)becomeKeyWindow;
- (MacOSBOOL)canBecomeKeyWindow;
- (void)dealloc;
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
- (void)setCanBecomeKeyWindow:(MacOSBOOL)bCanBecomeKeyWindow;
- (void)setJavaFrame:(JavaSalFrame *)pFrame;
- (void)setDraggingSourceDelegate:(id)pDelegate;
- (void)windowDidExitFullScreen:(NSNotification *)pNotification;
- (void)windowWillEnterFullScreen:(NSNotification *)pNotification;
- (void)windowDidMove:(NSNotification *)pNotification;
- (void)windowDidResize:(NSNotification *)pNotification;
- (void)windowDidDeminiaturize:(NSNotification *)pNotification;
- (void)windowWillMiniaturize:(NSNotification *)pNotification;
- (MacOSBOOL)windowShouldClose:(id)pObject;
- (MacOSBOOL)windowShouldZoom:(NSWindow *)pWindow toFrame:(NSRect)aNewFrame;
- (void)windowWillEnterVersionBrowser:(NSNotification *)pNotification;
- (void)windowDidExitVersionBrowser:(NSNotification *)pNotification;
- (void)windowWillClose:(NSNotification *)pNotification;
- (void)windowWillExitVersionBrowser:(NSNotification *)pNotification;
@end

SAL_DLLPRIVATE void JavaSalFrame_drawToNSView( NSView *pView, NSRect aDirtyRect );
SAL_DLLPRIVATE NSCursor *JavaSalFrame_getCursor( NSView *pView );

SAL_DLLPRIVATE MacOSBOOL NSWindow_hasMarkedText( NSWindow *pWindow );

#endif	// __OBJC__

SAL_DLLPRIVATE void VCLEventQueue_cancelTermination();
SAL_DLLPRIVATE void VCLEventQueue_getTextSelection( void *pNSWindow, CFStringRef *pTextSelection, CFDataRef *pRTFSelection );
SAL_DLLPRIVATE BOOL VCLEventQueue_paste( void *pNSWindow );
SAL_DLLPRIVATE void VCLEventQueue_removeCachedEvents();
SAL_DLLPRIVATE sal_Bool NSApplication_isActive();
SAL_DLLPRIVATE void VCLEventQueue_installVCLEventQueueClasses();

#endif
