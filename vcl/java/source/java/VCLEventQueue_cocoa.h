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

#ifdef __cplusplus
typedef void* id;
#else
@interface NSWindow (VCLWindow)
- (void)_clearModalWindowLevel;
- (BOOL)_isUtilityWindow;
- (void)_restoreModalWindowLevel;
- (void)_setModalWindowLevel;
- (void)_setUtilityWindow:(BOOL)bUtilityWindow;
@end

@interface VCLWindow : NSWindow
+ (void)clearModalWindowLevel;
+ (void)restoreModalWindowLevel;
- (void)becomeKeyWindow;
- (void)displayIfNeeded;
- (BOOL)makeFirstResponder:(NSResponder *)pResponder;
- (void)makeKeyWindow;
- (void)orderWindow:(NSWindowOrderingMode)nOrderingMode relativeTo:(int)nOtherWindowNumber;
- (BOOL)performKeyEquivalent:(NSEvent *)pEvent;
- (void)resignKeyWindow;
- (void)sendEvent:(NSEvent *)pEvent;
- (void)setContentView:(NSView *)pView;
- (void)setLevel:(int)nWindowLevel;
@end
#endif

#ifdef __cplusplus
BEGIN_C
#endif
void VCLEventQueue_postMouseWheelEvent( jobject aPeer, long nX, long nY, long nRotationX, long nRotationY, BOOL bShiftDown, BOOL bMetaDown, BOOL bAltDown, BOOL bControlDown );
void VCLEventQueue_postWindowMoveSessionEvent( jobject aPeer, long nX, long nY, BOOL bStartSession );
BOOL NSApplication_hasDelegate();
BOOL NSApplication_hasDelegate();
BOOL NSApplication_isActive();
void NSFontManager_acquire();
void NSFontManager_release();
void VCLEventQueue_installVCLEventQueueClasses( BOOL bUseKeyEntryFix, BOOL bUsePartialKeyEntryFix, BOOL bUseQuickTimeContentViewHack );
#ifdef __cplusplus
END_C
#endif

#endif
