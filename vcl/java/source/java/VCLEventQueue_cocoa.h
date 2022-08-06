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

#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include "java/salframe.h"
#include "osx/a11ywrapper.h"

// Comment out to disable LibreOffice's native accessibility code
#define USE_AQUA_A11Y

#ifdef __OBJC__

#ifndef NSAppearanceNameDarkAqua
#define NSAppearanceNameDarkAqua @"NSAppearanceNameDarkAqua"
#endif	// NSAppearanceNameDarkAqua

@interface NSApplication (VCLApplication)
- (NSAppearance *)appearance;
- (NSAppearance *)effectiveAppearance;
- (void)setAppearance:(NSAppearance *)pAppearance;
@end

@interface NSColor (VCLColor)
+ (NSColor *)controlHighlightColor;
+ (NSColor *)controlShadowColor;
+ (NSColor *)linkColor;
+ (NSColor *)scrollBarColor;
+ (NSColor *)selectedContentBackgroundColor;
+ (NSColor *)selectedMenuItemColor;
+ (NSColor *)separatorColor;
+ (NSColor *)unemphasizedSelectedContentBackgroundColor;
+ (NSColor *)unemphasizedSelectedTextBackgroundColor;
+ (NSColor *)unemphasizedSelectedTextColor;
@end

#ifdef USE_AQUA_A11Y
@interface VCLView : AquaA11yWrapper <NSDraggingDestination, NSDraggingSource, NSTextInputClient>
#else	// USE_AQUA_A11Y
@interface VCLView : NSView <NSDraggingDestination, NSDraggingSource, NSTextInputClient>
#endif	// USE_AQUA_A11Y
{
	JavaSalFrame*			mpFrame;
	BOOL					mbInKeyDown;
	NSObject*				mpInputManager;
	NSEvent*				mpLastKeyDownEvent;
	SalKeyEvent*			mpPendingKeyUpEvent;
	NSRange					maSelectedRange;
	id						mpTextInput;
	NSRange					maTextInputRange;
	BOOL					mbTextInputWantsNonRepeatKeyDown;
}
- (BOOL)acceptsFirstResponder;
- (void)abandonInput;
#ifdef USE_AQUA_A11Y
- (::com::sun::star::accessibility::XAccessibleContext *)accessibleContext;
- (id)parentAttribute;
- (NSWindow *)windowForParent;
#else	// USE_AQUA_A11Y
- (id)accessibilityAttributeValue:(NSAccessibilityAttributeName)aAttribute;
#endif	// USE_AQUA_A11Y
- (void)dealloc;
- (void)keyDown:(NSEvent *)pEvent;
- (void)keyUp:(NSEvent *)pEvent;
- (BOOL)hasMarkedText;
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
- (void)draggingSession:(NSDraggingSession *)pSession endedAtPoint:(NSPoint)aScreenPoint operation:(NSDragOperation)nOperation;
- (void)draggingSession:(NSDraggingSession *)pSession movedToPoint:(NSPoint)aScreenPoint;
- (NSDragOperation)draggingSession:(NSDraggingSession *)pSession sourceOperationMaskForDraggingContext:(NSDraggingContext)nContext;
- (void)draggingSession:(NSDraggingSession *)pSession willBeginAtPoint:(NSPoint)aScreenPoint;
- (id)draggingDestinationDelegate;
- (void)draggingEnded:(id < NSDraggingInfo >)pSender;
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)pSender;
- (void)draggingExited:(id < NSDraggingInfo >)pSender;
- (id)draggingSourceDelegate;
- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)pSender;
- (void)drawRect:(NSRect)aDirtyRect;
- (void)resetCursorRects;
- (BOOL)ignoreModifierKeysForDraggingSession:(NSDraggingSession *)pSession;
- (id)initWithFrame:(NSRect)aFrame;
- (BOOL)isOpaque;
- (BOOL)performDragOperation:(id < NSDraggingInfo >)pSender;
- (BOOL)prepareForDragOperation:(id < NSDraggingInfo >)pSender;
- (BOOL)readSelectionFromPasteboard:(NSPasteboard *)pPasteboard;
- (void)setDraggingDestinationDelegate:(id)pDelegate;
- (void)setDraggingSourceDelegate:(id)pDelegate;
- (void)updateDraggingItemsForDrag:(id <NSDraggingInfo>)pSender;
- (id)validRequestorForSendType:(NSString *)pSendType returnType:(NSString *)pReturnType;
- (BOOL)wantsPeriodicDraggingUpdates;
- (BOOL)writeSelectionToPasteboard:(NSPasteboard *)pPasteboard types:(NSArray *)pTypes;
@end

@interface NSWindow (VCLWindow)
- (void)_clearModalWindowLevel;
- (void)_restoreModalWindowLevel;
- (void)_setModalWindowLevel;
@end

@interface VCLPanel : NSPanel <NSWindowDelegate>
{
	BOOL					mbCanBecomeKeyWindow;
	NSUInteger				mnIgnoreMouseReleasedModifiers;
	JavaSalFrame*			mpFrame;
	sal_uLong				mnLastMetaModifierReleasedTime;
	NSEvent*				mpLastWindowDraggedEvent;
	BOOL					mbInVersionBrowser;
	BOOL					mbCloseOnExitVersionBrowser;
	NSRect					maNonFullScreenFrame;
}
- (void)_init;
#ifdef USE_AQUA_A11Y
- (::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >)accessibleContext;
#endif	// USE_AQUA_A11Y
- (BOOL)canBecomeKeyWindow;
- (void)dealloc;
- (NSRect)nonFullScreenFrame;
- (void)setCanBecomeKeyWindow:(BOOL)bCanBecomeKeyWindow;
- (void)setJavaFrame:(JavaSalFrame *)pFrame;
- (void)setNonFullScreenFrame:(NSRect)aFrame;
@end

@interface VCLWindow : NSWindow <NSWindowDelegate>
{
	BOOL					mbCanBecomeKeyWindow;
	NSUInteger				mnIgnoreMouseReleasedModifiers;
	JavaSalFrame*			mpFrame;
	sal_uLong				mnLastMetaModifierReleasedTime;
	NSEvent*				mpLastWindowDraggedEvent;
	BOOL					mbInVersionBrowser;
	BOOL					mbCloseOnExitVersionBrowser;
	NSRect					maNonFullScreenFrame;
}
+ (void)clearModalWindowLevel;
+ (void)restoreModalWindowLevel;
+ (void)swizzleSelectors:(NSWindow *)pWindow;
- (void)_init;
#ifdef USE_AQUA_A11Y
- (::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >)accessibleContext;
#endif	// USE_AQUA_A11Y
- (void)becomeKeyWindow;
- (BOOL)canBecomeKeyWindow;
- (void)dealloc;
- (void)displayIfNeeded;
- (id)draggingSourceDelegate;
- (id)initWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nStyle backing:(NSBackingStoreType)nBufferingType defer:(BOOL)bDeferCreation;
- (id)initWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nStyle backing:(NSBackingStoreType)nBufferingType defer:(BOOL)bDeferCreation screen:(NSScreen *)pScreen;
- (BOOL)makeFirstResponder:(NSResponder *)pResponder;
- (void)makeKeyWindow;
- (NSRect)nonFullScreenFrame;
- (void)orderWindow:(NSWindowOrderingMode)nOrderingMode relativeTo:(NSInteger)nOtherWindowNumber;
- (BOOL)performKeyEquivalent:(NSEvent *)pEvent;
- (void)resignKeyWindow;
- (void)sendEvent:(NSEvent *)pEvent;
- (void)setCanBecomeKeyWindow:(BOOL)bCanBecomeKeyWindow;
- (void)setJavaFrame:(JavaSalFrame *)pFrame;
- (void)setNonFullScreenFrame:(NSRect)aFrame;
- (void)setDraggingSourceDelegate:(id)pDelegate;
- (IBAction)toggleTabBar:(id)pSender;
- (void)windowDidExitFullScreen:(NSNotification *)pNotification;
- (void)windowDidFailToEnterFullScreen:(NSWindow *)pWindow;
- (void)windowWillEnterFullScreen:(NSNotification *)pNotification;
- (void)windowDidMove:(NSNotification *)pNotification;
- (void)windowDidResize:(NSNotification *)pNotification;
- (void)windowDidDeminiaturize:(NSNotification *)pNotification;
- (void)windowWillMiniaturize:(NSNotification *)pNotification;
- (BOOL)windowShouldClose:(id)pObject;
- (BOOL)windowShouldZoom:(NSWindow *)pWindow toFrame:(NSRect)aNewFrame;
- (void)windowWillEnterVersionBrowser:(NSNotification *)pNotification;
- (void)windowDidExitVersionBrowser:(NSNotification *)pNotification;
- (void)windowWillClose:(NSNotification *)pNotification;
- (void)windowWillExitVersionBrowser:(NSNotification *)pNotification;
@end

#ifndef USE_AQUA_A11Y

@interface VCLPanel (AquaA11yWrapper)
- (::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >)accessibleContext;
@end

@interface VCLWindow (AquaA11yWrapper)
- (::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >)accessibleContext;
@end

#endif	// !USE_AQUA_A11Y

SAL_DLLPRIVATE void JavaSalFrame_drawToNSView( NSView *pView, NSRect aDirtyRect );
SAL_DLLPRIVATE NSCursor *JavaSalFrame_getCursor( NSView *pView );
SAL_DLLPRIVATE sal_Bool NSWindow_hasMarkedText( NSWindow *pWindow );
SAL_DLLPRIVATE CGContextRef NSWindow_cachedCGContext();
SAL_DLLPRIVATE sal_Bool NSWindow_cachedCGContextScaleFactorHasChanged( CGLayerRef aLayer );
SAL_DLLPRIVATE void NSWindow_resetCachedCGContext();

#endif	// __OBJC__

SAL_DLLPRIVATE void VCLEventQueue_cancelTermination();
SAL_DLLPRIVATE void VCLEventQueue_getTextSelection( void *pNSWindow, CFStringRef *pTextSelection, CFDataRef *pRTFSelection );
SAL_DLLPRIVATE sal_Bool VCLEventQueue_paste( void *pNSWindow );
SAL_DLLPRIVATE void VCLEventQueue_removeCachedEvents();
SAL_DLLPRIVATE sal_Bool NSApplication_isActive();
SAL_DLLPRIVATE void VCLEventQueue_installVCLEventQueueClasses();
SAL_DLLPRIVATE void VCLUpdateSystemAppearance_handleAppearanceChange();

#endif
