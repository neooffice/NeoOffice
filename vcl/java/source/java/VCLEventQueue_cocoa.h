/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __VCLEVENTQUEUE_COCOA_H__
#define __VCLEVENTQUEUE_COCOA_H__

#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include "java/salframe.h"
#include "osx/a11ywrapper.h"

// Uncomment to disable LibreOffice's native accessibility code
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

@class VCLView;

@interface VCLA11yWrapper : AquaA11yWrapper
{
	VCLView*				mpParentView;
}
- (id)initWithParent:(VCLView *)pParentView accessibleContext:(::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >&)rxAccessibleContext;
- (void)dealloc;
@end

#endif	// USE_AQUA_A11Y

@interface VCLView : NSView <NSDraggingDestination, NSDraggingSource, NSTextInputClient>
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
#ifdef USE_AQUA_A11Y
	VCLA11yWrapper*			mpChildWrapper;
	BOOL					mbNeedChildWrapper;
#endif	// USE_AQUA_A11Y
}
- (BOOL)acceptsFirstResponder;
- (void)abandonInput;
#ifdef USE_AQUA_A11Y
- (id)accessibilityAttributeValue:(NSString *)pAttribute;
- (BOOL)accessibilityIsIgnored;
- (NSArray *)accessibilityAttributeNames;
- (BOOL)accessibilityIsAttributeSettable:(NSString *)pAttribute;
- (NSArray *)accessibilityParameterizedAttributeNames;
- (BOOL)accessibilitySetOverrideValue:(id)pValue forAttribute:(NSString *)pAttribute;
- (void)accessibilitySetValue:(id)pValue forAttribute:(NSString *)pAttribute;
- (id)accessibilityAttributeValue:(NSString *)pAttribute forParameter:(id)pParameter;
- (id)accessibilityFocusedUIElement;
- (NSString *)accessibilityActionDescription:(NSString *)pAction;
- (void)accessibilityPerformAction:(NSString *)pAction;
- (NSArray *)accessibilityActionNames;
- (id)accessibilityHitTest:(NSPoint)aPoint;
- (NSArray *)accessibilityVisibleChildren;
- (NSArray *)accessibilitySelectedChildren;
- (NSArray *)accessibilityChildren;
- (NSArray <id<NSAccessibilityElement>> *)accessibilityChildrenInNavigationOrder;
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
#ifdef USE_AQUA_A11Y
- (void)insertRegisteredViewIntoWrapperRepository;
- (void)registerView;
- (void)revokeView;
#endif	// USE_AQUA_A11Y
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
#ifdef USE_AQUA_A11Y
	BOOL					mbShowOnlyMenus;
	sal_uLong				mnStyle;
#endif	// USE_AQUA_A11Y
}
- (void)_init;
#ifdef USE_AQUA_A11Y
- (::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >)accessibleContext;
- (id)accessibilityApplicationFocusedUIElement;
- (id)accessibilityFocusedUIElement;
- (BOOL)accessibilityIsIgnored;
- (BOOL)isAccessibilityElement;
#endif	// USE_AQUA_A11Y
- (BOOL)canBecomeKeyWindow;
- (void)dealloc;
- (NSRect)nonFullScreenFrame;
- (void)setCanBecomeKeyWindow:(BOOL)bCanBecomeKeyWindow;
- (void)setJavaFrame:(JavaSalFrame *)pFrame;
#ifdef USE_AQUA_A11Y
- (void)setJavaShowOnlyMenus:(BOOL)bShowOnlyMenus;
- (void)setJavaStyle:(sal_uLong)nStyle;
- (BOOL)isIgnoredWindow;
- (void)registerWindow;
- (void)revokeWindow;
#endif	// USE_AQUA_A11Y
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
#ifdef USE_AQUA_A11Y
	BOOL					mbShowOnlyMenus;
	sal_uLong				mnStyle;
#endif	// USE_AQUA_A11Y
}
+ (void)clearModalWindowLevel;
+ (void)restoreModalWindowLevel;
+ (void)swizzleSelectors:(NSWindow *)pWindow;
- (void)_init;
#ifdef USE_AQUA_A11Y
- (::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >)accessibleContext;
- (id)accessibilityApplicationFocusedUIElement;
- (id)accessibilityFocusedUIElement;
- (BOOL)accessibilityIsIgnored;
- (BOOL)isAccessibilityElement;
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
#ifdef USE_AQUA_A11Y
- (void)setJavaShowOnlyMenus:(BOOL)bShowOnlyMenus;
- (void)setJavaStyle:(sal_uLong)nStyle;
- (BOOL)isIgnoredWindow;
- (void)registerWindow;
- (void)revokeWindow;
#endif	// USE_AQUA_A11Y
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
