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

#include <jni.h>
#include <salframe.h>
#include <sal/types.h>

#ifndef __OBJC__
typedef void* id;
#else	// __OBJC__

@interface VCLWindow : NSWindow
+ (void)clearModalWindowLevel;
+ (void)restoreModalWindowLevel;
+ (void)swizzleSelectors:(NSWindow *)pWindow;
- (void)becomeKeyWindow;
- (void)displayIfNeeded;
- (id)draggingSourceDelegate;
- (id)initWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nStyle backing:(NSBackingStoreType)nBufferingType defer:(BOOL)bDeferCreation;
- (id)initWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nStyle backing:(NSBackingStoreType)nBufferingType defer:(BOOL)bDeferCreation screen:(NSScreen *)pScreen;
- (BOOL)makeFirstResponder:(NSResponder *)pResponder;
- (void)makeKeyWindow;
- (void)orderWindow:(NSWindowOrderingMode)nOrderingMode relativeTo:(int)nOtherWindowNumber;
- (BOOL)performKeyEquivalent:(NSEvent *)pEvent;
- (void)resignKeyWindow;
- (void)sendEvent:(NSEvent *)pEvent;
- (void)setContentView:(NSView *)pView;
- (void)setDraggingSourceDelegate:(id)pDelegate;
- (void)setLevel:(int)nWindowLevel;
- (void)windowDidExitFullScreen:(NSNotification *)pNotification;
- (void)windowWillEnterFullScreen:(NSNotification *)pNotification;
@end

#ifdef USE_NATIVE_WINDOW
#ifdef __cplusplus
BEGIN_C
#endif
SAL_DLLPRIVATE void JavaSalFrame_drawToNSView( NSView *pView, NSRect aDirtyRect );
#ifdef __cplusplus
END_C
#endif
#endif	// USE_NATIVE_WINDOW

#endif	// __OBJC__

#ifdef __cplusplus
BEGIN_C
#endif
SAL_DLLPRIVATE void VCLEventQueue_cancelTermination();
SAL_DLLPRIVATE void VCLEventQueue_fullScreen( void *pNSWindow, BOOL bFullScreen );
SAL_DLLPRIVATE void VCLEventQueue_getTextSelection( void *pNSWindow, CFStringRef *pTextSelection, CFDataRef *pRTFSelection );
SAL_DLLPRIVATE BOOL VCLEventQueue_paste( void *pNSWindow );
SAL_DLLPRIVATE BOOL VCLEventQueue_postCommandEvent( jobject aPeer, short nKey, short nModifiers, jchar nOriginalKeyChar, short nOriginalModifiers );
SAL_DLLPRIVATE void VCLEventQueue_postMouseWheelEvent( jobject aPeer, long nX, long nY, long nRotationX, long nRotationY, BOOL bShiftDown, BOOL bMetaDown, BOOL bAltDown, BOOL bControlDown );
SAL_DLLPRIVATE void VCLEventQueue_postWindowMoveSessionEvent( jobject aPeer, long nX, long nY, BOOL bStartSession );
SAL_DLLPRIVATE void VCLEventQueue_removeCachedEvents();
SAL_DLLPRIVATE BOOL NSApplication_isActive();
SAL_DLLPRIVATE void NSFontManager_acquire();
SAL_DLLPRIVATE void NSFontManager_release();
SAL_DLLPRIVATE void VCLEventQueue_installVCLEventQueueClasses();
#ifdef __cplusplus
END_C
#endif

#endif
