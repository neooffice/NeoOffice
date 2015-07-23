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
 *  Patrick Luby, August 2006
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2006 Planamesa Inc.
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

#include <saldata.hxx>
#include <vos/mutex.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
// Need to include for virtual key constants but we don't link to it
#import <Carbon/Carbon.h>
#import <objc/objc-class.h>
#include <postmac.h>

#include "VCLApplicationDelegate_cocoa.h"
#include "VCLEventQueue_cocoa.h"
#include "VCLResponder_cocoa.h"
#include "../app/salinst_cocoa.h"

#define MODIFIER_RELEASE_INTERVAL 100
#define UNDEFINED_KEY_CODE 0xffff

inline long FloatToLong( float f ) { return (long)( f == 0 ? f : f < 0 ? f - 0.5 : f + 0.5 ); }

static NSPoint GetFlippedContentViewLocation( NSWindow *pWindow, NSEvent *pEvent )
{
	NSPoint aRet = NSZeroPoint;

	if ( pWindow && pEvent )
	{
		NSRect aFrame = [pWindow frame];
		aRet = [pEvent locationInWindow];

		NSWindow *pEventWindow = [pEvent window];
		if ( !pEventWindow )
		{
			aRet.x -= aFrame.origin.x;
			aRet.y -= aFrame.origin.y;
		}
		else if ( pEventWindow != pWindow )
		{
			NSRect aEventFrame = [pEventWindow frame];
			aRet.x += aFrame.origin.x - aEventFrame.origin.x;
			aRet.y += aFrame.origin.y - aEventFrame.origin.y;
		}

		// Translate and flip coordinates within content frame
		NSRect aContentFrame = [pWindow contentRectForFrameRect:aFrame];
		aRet.x += aFrame.origin.x - aContentFrame.origin.x;
		aRet.y += aFrame.origin.y - aContentFrame.origin.y;
		aRet.y = aContentFrame.size.height - aRet.y;
	}

	return aRet;
}

static USHORT GetEventCode( NSUInteger nModifiers )
{
	USHORT nRet = 0;

	if ( nModifiers & NSLeftMouseDownMask )
		nRet |= MOUSE_LEFT;
	if ( nModifiers & NSRightMouseDownMask )
		nRet |= MOUSE_RIGHT;
	if ( nModifiers & NSOtherMouseDownMask )
		nRet |= MOUSE_MIDDLE;

	// Treat the Mac OS X command key as a control key and the control key as
	// the meta key
	if ( nModifiers & NSCommandKeyMask )
		nRet |= KEY_MOD1;
	if ( nModifiers & NSAlternateKeyMask )
		nRet |= KEY_MOD2;
	if ( nModifiers & NSControlKeyMask )
		nRet |= KEY_MOD3;
	if ( nModifiers & NSShiftKeyMask )
		nRet |= KEY_SHIFT;

	// If command plus left or middle button is pressed, Cocoa will add the
	// right button so we need to strip it out
	if ( nRet & MOUSE_RIGHT && nRet & KEY_MOD1 && nRet & ( MOUSE_LEFT | MOUSE_MIDDLE ) )
		nRet &= ~MOUSE_RIGHT;

	// Convert control plus left button events to right button events since
	// one button mice have no right button
	if ( nRet & MOUSE_LEFT && nRet & KEY_MOD3 )
		nRet = ( nRet & ~( KEY_MOD3 | MOUSE_LEFT ) ) | MOUSE_RIGHT;

	return nRet;
}

static USHORT GetKeyCode( USHORT nKey, USHORT nChar )
{
	USHORT nRet = 0;

	switch ( nKey )
	{
		case UNDEFINED_KEY_CODE:
			if (nChar >= '0' && nChar <= '9')
				nRet = KEYGROUP_NUM + nChar - '0';
			else if (nChar >= 'A' && nChar <= 'Z')
				nRet = KEYGROUP_ALPHA + nChar - 'A';
			else if (nChar >= 'a' && nChar <= 'z')
				nRet = KEYGROUP_ALPHA + nChar - 'a';
			else if (nChar == 0x08)
				nRet = KEY_BACKSPACE;
			else if (nChar == 0x09)
				nRet = KEY_TAB;
			else if (nChar == 0x03 || nChar == 0x0a || nChar == 0x0d)
				nRet = KEY_RETURN;
			else if (nChar == 0x1b)
				nRet = KEY_ESCAPE;
			else if (nChar == 0x20)
				nRet = KEY_SPACE;
			else if (nChar == 0x7f)
				nRet = KEY_DELETE;
			else if (nChar == 0x2b)
				nRet = KEY_ADD;
			else if (nChar == 0x2d)
				nRet = KEY_SUBTRACT;
			else if (nChar == 0x2e)
				nRet = KEY_POINT;
			else if (nChar == 0x2f)
				nRet = KEY_DIVIDE;
			else if (nChar == 0x3d)
				nRet = KEY_EQUAL;
			// 0xf700 and higher are NSEvent action constants
			else if (nChar == 0xf700)
				nRet = KEY_UP;
			else if (nChar == 0xf701)
				nRet = KEY_DOWN;
			else if (nChar == 0xf702)
				nRet = KEY_LEFT;
			else if (nChar == 0xf703)
				nRet = KEY_RIGHT;
			else if (nChar == 0xf704)
				nRet = KEY_F1;
			else if (nChar == 0xf705)
				nRet = KEY_F2;
			else if (nChar == 0xf706)
				nRet = KEY_F3;
			else if (nChar == 0xf707)
				nRet = KEY_F4;
			else if (nChar == 0xf708)
				nRet = KEY_F5;
			else if (nChar == 0xf709)
				nRet = KEY_F6;
			else if (nChar == 0xf70a)
				nRet = KEY_F7;
			else if (nChar == 0xf70b)
				nRet = KEY_F8;
			else if (nChar == 0xf70c)
				nRet = KEY_F9;
			else if (nChar == 0xf70d)
				nRet = KEY_F10;
			else if (nChar == 0xf70e)
				nRet = KEY_F11;
			else if (nChar == 0xf70f)
				nRet = KEY_F12;
			else if (nChar == 0xf710)
				nRet = KEY_F13;
			else if (nChar == 0xf711)
				nRet = KEY_F14;
			else if (nChar == 0xf712)
				nRet = KEY_F15;
			else if (nChar == 0xf713)
				nRet = KEY_F16;
			else if (nChar == 0xf714)
				nRet = KEY_F17;
			else if (nChar == 0xf715)
				nRet = KEY_F18;
			else if (nChar == 0xf716)
				nRet = KEY_F19;
			else if (nChar == 0xf717)
				nRet = KEY_F20;
			else if (nChar == 0xf718)
				nRet = KEY_F21;
			else if (nChar == 0xf719)
				nRet = KEY_F22;
			else if (nChar == 0xf71a)
				nRet = KEY_F23;
			else if (nChar == 0xf71b)
				nRet = KEY_F24;
			else if (nChar == 0xf71c)
				nRet = KEY_F25;
			else if (nChar == 0xf71d)
				nRet = KEY_F26;
			else if (nChar == 0xf727)
				nRet = KEY_INSERT;
			else if (nChar == 0xf728)
				nRet = KEY_DELETE;
			else if (nChar == 0xf729)
				nRet = KEY_HOME;
			else if (nChar == 0xf72b)
				nRet = KEY_END;
			else if (nChar == 0xf72c)
				nRet = KEY_PAGEUP;
			else if (nChar == 0xf72d)
				nRet = KEY_PAGEDOWN;
			else if (nChar == 0xf743)
				nRet = KEY_UNDO;
			else if (nChar == 0xf745)
				nRet = KEY_FIND;
			else if (nChar == 0xf746)
				nRet = KEY_HELP;
			else
				nRet = 0;
			break;
		case kVK_ANSI_0:
		case kVK_ANSI_Keypad0:
			nRet = KEY_0;
			break;
		case kVK_ANSI_1:
		case kVK_ANSI_Keypad1:
			nRet = KEY_1;
			break;
		case kVK_ANSI_2:
		case kVK_ANSI_Keypad2:
			nRet = KEY_2;
			break;
		case kVK_ANSI_3:
		case kVK_ANSI_Keypad3:
			nRet = KEY_3;
			break;
		case kVK_ANSI_4:
		case kVK_ANSI_Keypad4:
			nRet = KEY_4;
			break;
		case kVK_ANSI_5:
		case kVK_ANSI_Keypad5:
			nRet = KEY_5;
			break;
		case kVK_ANSI_6:
		case kVK_ANSI_Keypad6:
			nRet = KEY_6;
			break;
		case kVK_ANSI_7:
		case kVK_ANSI_Keypad7:
			nRet = KEY_7;
			break;
		case kVK_ANSI_8:
		case kVK_ANSI_Keypad8:
			nRet = KEY_8;
			break;
		case kVK_ANSI_9:
		case kVK_ANSI_Keypad9:
			nRet = KEY_9;
			break;
		case kVK_ANSI_A:
			nRet = KEY_A;
			break;
		case kVK_ANSI_B:
			nRet = KEY_B;
			break;
		case kVK_ANSI_C:
			nRet = KEY_C;
			break;
		case kVK_ANSI_D:
			nRet = KEY_D;
			break;
		case kVK_ANSI_E:
			nRet = KEY_E;
			break;
		case kVK_ANSI_F:
			nRet = KEY_F;
			break;
		case kVK_ANSI_G:
			nRet = KEY_G;
			break;
		case kVK_ANSI_H:
			nRet = KEY_H;
			break;
		case kVK_ANSI_I:
			nRet = KEY_I;
			break;
		case kVK_ANSI_J:
			nRet = KEY_J;
			break;
		case kVK_ANSI_K:
			nRet = KEY_K;
			break;
		case kVK_ANSI_L:
			nRet = KEY_L;
			break;
		case kVK_ANSI_M:
			nRet = KEY_M;
			break;
		case kVK_ANSI_N:
			nRet = KEY_N;
			break;
		case kVK_ANSI_O:
			nRet = KEY_O;
			break;
		case kVK_ANSI_P:
			nRet = KEY_P;
			break;
		case kVK_ANSI_Q:
			nRet = KEY_Q;
			break;
		case kVK_ANSI_R:
			nRet = KEY_R;
			break;
		case kVK_ANSI_S:
			nRet = KEY_S;
			break;
		case kVK_ANSI_T:
			nRet = KEY_T;
			break;
		case kVK_ANSI_U:
			nRet = KEY_U;
			break;
		case kVK_ANSI_V:
			nRet = KEY_V;
			break;
		case kVK_ANSI_W:
			nRet = KEY_W;
			break;
		case kVK_ANSI_X:
			nRet = KEY_X;
			break;
		case kVK_ANSI_Y:
			nRet = KEY_Y;
			break;
		case kVK_ANSI_Z:
			nRet = KEY_Z;
			break;
		case kVK_ANSI_KeypadDecimal:
			nRet = KEY_DECIMAL;
			break;
		case kVK_ANSI_KeypadDivide:
			nRet = KEY_DIVIDE;
			break;
		case kVK_ANSI_KeypadEnter:
		case kVK_Return:
			nRet = KEY_RETURN;
			break;
		case kVK_ANSI_KeypadEquals:
		case kVK_ANSI_Equal:
			nRet = KEY_EQUAL;
			break;
		case kVK_ANSI_KeypadMinus:
		case kVK_ANSI_Minus:
			nRet = KEY_SUBTRACT;
			break;
		case kVK_ANSI_KeypadMultiply:
			nRet = KEY_MULTIPLY;
			break;
		case kVK_ANSI_KeypadPlus:
			nRet = KEY_ADD;
			break;
		case kVK_ANSI_Comma:
			nRet = KEY_COMMA;
			break;
		case kVK_ANSI_LeftBracket:
			nRet = KEY_BRACKETLEFT;
			break;
		case kVK_ANSI_Period:
			nRet = KEY_POINT;
			break;
		case kVK_ANSI_RightBracket:
			nRet = KEY_BRACKETRIGHT;
			break;
		case kVK_F1:
			nRet = KEY_F1;
			break;
		case kVK_F2:
			nRet = KEY_F2;
			break;
		case kVK_F3:
			nRet = KEY_F3;
			break;
		case kVK_F4:
			nRet = KEY_F4;
			break;
		case kVK_F5:
			nRet = KEY_F5;
			break;
		case kVK_F6:
			nRet = KEY_F6;
			break;
		case kVK_F7:
			nRet = KEY_F7;
			break;
		case kVK_F8:
			nRet = KEY_F8;
			break;
		case kVK_F9:
			nRet = KEY_F9;
			break;
		case kVK_F10:
			nRet = KEY_F10;
			break;
		case kVK_F11:
			nRet = KEY_F11;
			break;
		case kVK_F12:
			nRet = KEY_F12;
			break;
		case kVK_F13:
			nRet = KEY_F13;
			break;
		case kVK_F14:
			nRet = KEY_F14;
			break;
		case kVK_F15:
			nRet = KEY_F15;
			break;
		case kVK_F16:
			nRet = KEY_F16;
			break;
		case kVK_F17:
			nRet = KEY_F17;
			break;
		case kVK_F18:
			nRet = KEY_F18;
			break;
		case kVK_F19:
			nRet = KEY_F19;
			break;
		case kVK_F20:
			nRet = KEY_F20;
			break;
		case kVK_Delete:
			nRet = KEY_BACKSPACE;
			break;
		case kVK_DownArrow:
			nRet = KEY_DOWN;
			break;
		case kVK_End:
			nRet = KEY_END;
			break;
		case kVK_Escape:
			nRet = KEY_ESCAPE;
			break;
		case kVK_ForwardDelete:
			nRet = KEY_DELETE;
			break;
		case kVK_Help:
			nRet = KEY_HELP;
			break;
		case kVK_Home:
			nRet = KEY_HOME;
			break;
		case kVK_LeftArrow:
			nRet = KEY_LEFT;
			break;
		case kVK_PageDown:
			nRet = KEY_PAGEDOWN;
			break;
		case kVK_PageUp:
			nRet = KEY_PAGEUP;
			break;
		case kVK_RightArrow:
			nRet = KEY_RIGHT;
			break;
		case kVK_Space:
			nRet = KEY_SPACE;
			break;
		case kVK_Tab:
			nRet = KEY_TAB;
			break;
		case kVK_UpArrow:
			nRet = KEY_UP;
			break;
		case kVK_ANSI_KeypadClear:
		case kVK_ANSI_Backslash:
		case kVK_ANSI_Grave:
		case kVK_ANSI_Quote:
		case kVK_ANSI_Semicolon:
		case kVK_ANSI_Slash:
		case kVK_CapsLock:
		case kVK_Command:
		case kVK_Control:
		case kVK_Function:
		case kVK_ISO_Section:
		case kVK_JIS_Eisu:
		case kVK_JIS_Kana:
		case kVK_JIS_KeypadComma:
		case kVK_JIS_Underscore:
		case kVK_JIS_Yen:
		case kVK_Mute:
		case kVK_Option:
		case kVK_RightControl:
		case kVK_RightOption:
		case kVK_RightShift:
		case kVK_Shift:
		case kVK_VolumeDown:
		case kVK_VolumeUp:
		default:
			break;
	}

	return nRet;
}

static void RegisterMainBundleWithLaunchServices()
{
	// Make sure our application is registered in launch services database as
	// it can become disconnected after displaying the versions browser when
	// running the Mac app sandbox
	NSBundle *pBundle = [NSBundle mainBundle];
	if ( pBundle )
	{
		NSURL *pBundleURL = [pBundle bundleURL];
		if ( pBundleURL )
			LSRegisterURL( (CFURLRef)pBundleURL, false );
	}
}

@interface IsApplicationActive : NSObject
{
	MacOSBOOL					mbActive;
}
+ (id)create;
- (id)init;
- (MacOSBOOL)isActive;
- (void)isApplicationActive:(id)pObject;
@end

@implementation IsApplicationActive

+ (id)create
{
	IsApplicationActive *pRet = [[IsApplicationActive alloc] init];
	[pRet autorelease];
	return pRet;
}

- (id)init
{
	[super init];

	mbActive = YES;

	return self;
}

- (MacOSBOOL)isActive
{
	return mbActive;
}

- (void)isApplicationActive:(id)pObject
{
	// Fix bug 3491 by returning YES only if there is no key window or
	// the key window is a Java window
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
		mbActive = ( [pApp isActive] && ![pApp modalWindow] && ( ![pApp keyWindow] || [[pApp keyWindow] isKindOfClass:[VCLPanel class]] || [[pApp keyWindow] isKindOfClass:[VCLWindow class]] ) );
}

@end

@interface NSResponder (VCLResponder)
- (void)abandonInput;
- (void)copy:(id)pSender;
- (void)cut:(id)pSender;
- (void)paste:(id)pSender;
- (void)redo:(id)pSender;
- (void)undo:(id)pSender;
@end

@interface NSView (VCLViewPoseAs)
- (void)poseAsDragImage:(NSImage *)pImage at:(NSPoint)aImageLocation offset:(NSSize)aMouseOffset event:(NSEvent *)pEvent pasteboard:(NSPasteboard *)pPasteboard source:(id)pSourceObject slideBack:(MacOSBOOL)bSlideBack;
@end

@interface VCLCMenuBar : NSObject
{
}
+ (void)activate:(id)pObject modallyDisabled:(MacOSBOOL)bModallyDisabled;
+ (void)addDefaultHelpMenu;
+ (void)clearMenuBar:(MacOSBOOL)bClear;
+ (id)getDefaultMenuBar;
+ (MacOSBOOL)isActiveMenuBar:(id)pObject;
@end

@implementation VCLCMenuBar

+ (void)activate:(id)pObject modallyDisabled:(MacOSBOOL)bModallyDisabled
{
}

+ (void)addDefaultHelpMenu
{
}

+ (void)clearMenuBar:(MacOSBOOL)bClear
{
}

+ (id)getDefaultMenuBar
{
	return nil;
}

+ (MacOSBOOL)isActiveMenuBar:(id)pObject
{
	return NO;
}

@end

@implementation VCLPanel

- (void)_init
{
	mbCanBecomeKeyWindow = YES;
	mnIgnoreMouseReleasedModifiers = 0;
	mpFrame = NULL;
	mnLastMetaModifierReleasedTime = 0;
	mpLastWindowDraggedEvent = nil;
	mbInVersionBrowser = NO;
	mbCloseOnExitVersionBrowser = NO;

	[self setReleasedWhenClosed:NO];
	[self setDelegate:self];
	[self setAcceptsMouseMovedEvents:YES];
}

- (MacOSBOOL)canBecomeKeyWindow
{
	return ( mbCanBecomeKeyWindow && ![self becomesKeyOnlyIfNeeded] );
}

- (void)dealloc
{
	if ( mpLastWindowDraggedEvent )
		[mpLastWindowDraggedEvent release];

	[super dealloc];
}

- (void)setCanBecomeKeyWindow:(MacOSBOOL)bCanBecomeKeyWindow
{
	mbCanBecomeKeyWindow = bCanBecomeKeyWindow;
}

- (void)setFrame:(JavaSalFrame *)pFrame
{
	mpFrame = pFrame;

	NSView *pContentView = [self contentView];
	if ( pContentView && [pContentView isKindOfClass:[VCLView class]] )
		[(VCLView *)pContentView setFrame:pFrame];
}

@end

@interface NSWindow (VCLWindowPoseAs)
- (void)_init;
- (void)poseAsBecomeKeyWindow;
- (void)poseAsDisplayIfNeeded;
- (id)poseAsInitWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nStyle backing:(NSBackingStoreType)nBufferingType defer:(MacOSBOOL)bDeferCreation;
- (id)poseAsInitWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nStyle backing:(NSBackingStoreType)nBufferingType defer:(MacOSBOOL)bDeferCreation screen:(NSScreen *)pScreen;
- (MacOSBOOL)poseAsMakeFirstResponder:(NSResponder *)pResponder;
- (void)poseAsMakeKeyWindow;
- (void)poseAsOrderWindow:(NSWindowOrderingMode)nOrderingMode relativeTo:(int)nOtherWindowNumber;
- (MacOSBOOL)poseAsPerformKeyEquivalent:(NSEvent *)pEvent;
- (void)poseAsResignKeyWindow;
- (void)poseAsSendEvent:(NSEvent *)pEvent;
- (void)poseAsSetBackgroundColor:(NSColor *)pColor;
@end

static MacOSBOOL bJavaAWTInitialized = NO;
static NSMutableDictionary *pDraggingDestinationDelegates = nil;
static NSMutableArray *pNeedRestoreModalWindows = nil;
static VCLResponder *pSharedResponder = nil;
static NSMutableDictionary *pDraggingSourceDelegates = nil;
static NSUInteger nMouseMask = 0;

@implementation VCLWindow

+ (void)clearModalWindowLevel
{
	if ( !pNeedRestoreModalWindows )
	{
		// Do not retain as invoking alloc disables autorelease
		pNeedRestoreModalWindows = [[NSMutableArray alloc] initWithCapacity:4];
		if ( !pNeedRestoreModalWindows )
			return;
	}

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp && ![pApp isActive] )
	{
		NSArray *pWindows = [pApp windows];
		if ( pWindows )
		{
			unsigned int nCount = [pWindows count];
			unsigned int i = 0;
			for ( ; i < nCount; i++ )
			{
				NSWindow *pWindow = (NSWindow *)[pWindows objectAtIndex:i];
				if ( pWindow && [pWindow level] == NSModalPanelWindowLevel && [pWindow respondsToSelector:@selector(_clearModalWindowLevel)] && ( [pWindow isKindOfClass:[VCLPanel class]] || [pWindow isKindOfClass:[VCLWindow class]] ) )
				{
					[pNeedRestoreModalWindows removeObject:pWindow];
					[pWindow _clearModalWindowLevel];

					// Make sure that that the current window is at the
					// back of the array
					[pNeedRestoreModalWindows addObject:pWindow];
				}
			}
		}
	}
}

+ (void)restoreModalWindowLevel
{
	if ( !pNeedRestoreModalWindows )
		return;

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp && [pApp isActive] )
	{
		unsigned int nCount = [pNeedRestoreModalWindows count];
		unsigned int i = 0;
		for ( ; i < nCount; i++ )
		{
			NSWindow *pWindow = (NSWindow *)[pNeedRestoreModalWindows objectAtIndex:i];
			if ( pWindow && [pWindow level] != NSModalPanelWindowLevel && [pWindow respondsToSelector:@selector(_restoreModalWindowLevel)] )
			{
				if ( [pWindow isVisible] )
					[pWindow _restoreModalWindowLevel];
			}
		}
	}

	// Make sure that all windows are purged from the array
	[pNeedRestoreModalWindows removeAllObjects];
}

+ (void)swizzleSelectors:(NSWindow *)pWindow
{
	// If a Java window is loaded, swizzle Java's menubar management class so
	// that Java will not be able to change the menubar
	if ( pWindow && !bJavaAWTInitialized && [[pWindow className] isEqualToString:@"CocoaAppWindow"] )
	{
		bJavaAWTInitialized = YES;

		NSBundle *pBundle = [NSBundle bundleForClass:[pWindow class]];
		if ( pBundle )
		{
			// CMenuBar selectors

			Class aClass = [pBundle classNamed:@"CMenuBar"];
			if ( aClass )
			{
				SEL aSelector = @selector(activate:modallyDisabled:);
				Method aOldMethod = class_getClassMethod( aClass, aSelector );
				Method aNewMethod = class_getClassMethod( [VCLCMenuBar class], aSelector );
				if ( aOldMethod && aNewMethod )
				{
					IMP aNewIMP = method_getImplementation( aNewMethod );
					if ( aNewIMP )
						method_setImplementation( aOldMethod, aNewIMP );
				}

				aSelector = @selector(addDefaultHelpMenu);
				aOldMethod = class_getClassMethod( aClass, aSelector );
				aNewMethod = class_getClassMethod( [VCLCMenuBar class], aSelector );
				if ( aOldMethod && aNewMethod )
				{
					IMP aNewIMP = method_getImplementation( aNewMethod );
					if ( aNewIMP )
						method_setImplementation( aOldMethod, aNewIMP );
				}

				aSelector = @selector(clearMenuBar:);
				aOldMethod = class_getClassMethod( aClass, aSelector );
				aNewMethod = class_getClassMethod( [VCLCMenuBar class], aSelector );
				if ( aOldMethod && aNewMethod )
				{
					IMP aNewIMP = method_getImplementation( aNewMethod );
					if ( aNewIMP )
						method_setImplementation( aOldMethod, aNewIMP );
				}

				aSelector = @selector(getDefaultMenuBar);
				aOldMethod = class_getClassMethod( aClass, aSelector );
				aNewMethod = class_getClassMethod( [VCLCMenuBar class], aSelector );
				if ( aOldMethod && aNewMethod )
				{
					IMP aNewIMP = method_getImplementation( aNewMethod );
					if ( aNewIMP )
						method_setImplementation( aOldMethod, aNewIMP );
				}

				aSelector = @selector(isActiveMenuBar:);
				aOldMethod = class_getClassMethod( aClass, aSelector );
				aNewMethod = class_getClassMethod( [VCLCMenuBar class], aSelector );
				if ( aOldMethod && aNewMethod )
				{
					IMP aNewIMP = method_getImplementation( aNewMethod );
					if ( aNewIMP )
						method_setImplementation( aOldMethod, aNewIMP );
				}
			}
		}
	}
}

- (void)_init
{
	mbCanBecomeKeyWindow = YES;
	mnIgnoreMouseReleasedModifiers = 0;
	mpFrame = NULL;
	mnLastMetaModifierReleasedTime = 0;
	mpLastWindowDraggedEvent = nil;
	mbInVersionBrowser = NO;
	mbCloseOnExitVersionBrowser = NO;

	[self setReleasedWhenClosed:NO];
	[self setDelegate:self];
	[self setAcceptsMouseMovedEvents:YES];
}

- (void)becomeKeyWindow
{
	[VCLWindow restoreModalWindowLevel];

	if ( [super respondsToSelector:@selector(poseAsBecomeKeyWindow)] )
		[super poseAsBecomeKeyWindow];

	if ( [self isVisible] )
	{
		if ( ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) && mpFrame )
		{
			// Fix bug 1819 by forcing cancellation of the input method
			NSResponder *pResponder = [self firstResponder];
			if ( pResponder && [pResponder isKindOfClass:[VCLView class]] )
				[(VCLView *)pResponder abandonInput];

			JavaSalEvent *pFocusEvent = new JavaSalEvent( SALEVENT_GETFOCUS, mpFrame, NULL );
			JavaSalEventQueue::postCachedEvent( pFocusEvent );
			pFocusEvent->release();
		}
		else
		{
			// Fix bug 3327 by removing any cached events when a non-Java
			// window obtains focus
			NSApplication *pApp = [NSApplication sharedApplication];
			if ( pApp && [pApp modalWindow] == self )
			{
				VCLEventQueue_removeCachedEvents();
				[self orderFront:self];
			}
		}
	}
}

- (MacOSBOOL)canBecomeKeyWindow
{
	return mbCanBecomeKeyWindow;
}

- (void)dealloc
{
	if ( mpLastWindowDraggedEvent )
		[mpLastWindowDraggedEvent release];

	[super dealloc];
}

- (void)displayIfNeeded
{
	// Fix bug 2151 by not allowing any updates if the window is hidden
	if ( ![self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
		return;

	if ( [super respondsToSelector:@selector(poseAsDisplayIfNeeded)] )
		[super poseAsDisplayIfNeeded];
}

- (id)draggingSourceDelegate
{
	id pRet = nil;

	NSNumber *pKey = [NSNumber numberWithUnsignedLong:(unsigned long)self];
	if ( pKey && pDraggingSourceDelegates )
	{
		id pDelegate = [pDraggingSourceDelegates objectForKey:pKey];
		if ( pDelegate )
			pRet = pDelegate;
	}

	return pRet;
}

- (id)initWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nStyle backing:(NSBackingStoreType)nBufferingType defer:(MacOSBOOL)bDeferCreation
{
	[VCLWindow swizzleSelectors:self];

	id pRet = self;

	if ( [super respondsToSelector:@selector(poseAsInitWithContentRect:styleMask:backing:defer:)] )
		pRet = [super poseAsInitWithContentRect:aContentRect styleMask:nStyle backing:nBufferingType defer:bDeferCreation];

	if ( ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) && [self respondsToSelector:@selector(_init)] )
		[self _init];

	return pRet;
}

- (id)initWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nStyle backing:(NSBackingStoreType)nBufferingType defer:(MacOSBOOL)bDeferCreation screen:(NSScreen *)pScreen
{
	[VCLWindow swizzleSelectors:self];

	id pRet = self;

	if ( [super respondsToSelector:@selector(poseAsInitWithContentRect:styleMask:backing:defer:screen:)] )
		pRet = [super poseAsInitWithContentRect:aContentRect styleMask:nStyle backing:nBufferingType defer:bDeferCreation screen:pScreen];

	if ( ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) && [self respondsToSelector:@selector(_init)] )
		[self _init];

	return pRet;
}

- (MacOSBOOL)makeFirstResponder:(NSResponder *)pResponder
{
	NSResponder *pOldResponder = [self firstResponder];

	MacOSBOOL bRet = NO;
	if ( [super respondsToSelector:@selector(poseAsMakeFirstResponder:)] )
		bRet = [super poseAsMakeFirstResponder:pResponder];

	// Fix bug 1819 by forcing cancellation of the input method
	if ( bRet && [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
	{
		if ( pOldResponder && [pOldResponder isKindOfClass:[VCLView class]] )
			[(VCLView *)pOldResponder abandonInput];

		if ( pResponder && [pResponder isKindOfClass:[VCLView class]] )
			[(VCLView *)pResponder abandonInput];
	}

	return bRet;
}

- (void)makeKeyWindow
{
	if ( ![self canBecomeKeyWindow] )
		return;

	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
	{
		MacOSBOOL bTrackingMenuBar = false;
		VCLApplicationDelegate *pSharedDelegate = [VCLApplicationDelegate sharedDelegate];
		if ( pSharedDelegate )
			bTrackingMenuBar = [pSharedDelegate isInTracking];

		// Fix bug 2992 by not allowing the key window to change when we are
		// tracking the menubar and never allow a borderless window to grab
		// focus
		if ( bTrackingMenuBar && ! ( [self styleMask] & NSTitledWindowMask ) )
			return;
	}

	if ( [super respondsToSelector:@selector(poseAsMakeKeyWindow)] )
		[super poseAsMakeKeyWindow];
}

- (void)orderWindow:(NSWindowOrderingMode)nOrderingMode relativeTo:(int)nOtherWindowNumber
{
#ifdef USE_NATIVE_FULL_SCREEN_MODE
	if ( nOrderingMode != NSWindowOut && ![self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
	{
		NSNotificationCenter *pNotificationCenter = [NSNotificationCenter defaultCenter];
		if ( pNotificationCenter )
		{
			[pNotificationCenter addObserver:self selector:@selector(windowDidExitFullScreen:) name:@"NSWindowDidExitFullScreenNotification" object:self];
			[pNotificationCenter addObserver:self selector:@selector(windowWillEnterFullScreen:) name:@"NSWindowWillEnterFullScreenNotification" object:self];
		}
	}
	else if ( nOrderingMode == NSWindowOut && [self isVisible] )
	{
		if ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] )
		{
			if ( mbInVersionBrowser )
			{
				mbCloseOnExitVersionBrowser = YES;

				// Force version browser to exit
				NSButton *pCloseButton = [self standardWindowButton:NSWindowCloseButton];
				if ( pCloseButton )
					[pCloseButton performClick:self];
			}

			if ( mpLastWindowDraggedEvent )
			{
				[mpLastWindowDraggedEvent release];
				mpLastWindowDraggedEvent = nil;
			}

			if ( [self level] == NSModalPanelWindowLevel && [self respondsToSelector:@selector(_clearModalWindowLevel)] )
			{
				// Clear modal level while window is visible or else the window
				// will reappear when focus changes to another application on
				// Mac OS X 10.5
				[pNeedRestoreModalWindows removeObject:self];
				[self _clearModalWindowLevel];
			}

			NSNotificationCenter *pNotificationCenter = [NSNotificationCenter defaultCenter];
			if ( pNotificationCenter )
			{
				[pNotificationCenter removeObserver:self name:@"NSWindowDidExitFullScreenNotification" object:self];
				[pNotificationCenter removeObserver:self name:@"NSWindowWillEnterFullScreenNotification" object:self];
			}
		}
		else if ( [[self className] isEqualToString:@"_NSPopoverWindow"] )
		{
			// Fix crashing on OS X 10.10 when displaying the Save dialog while
			// the titlebar popover window is displayed by removing the
			// titlebar popover window's content view before ordering it out
			[self setContentView:nil];
		}
	}
#endif	// USE_NATIVE_FULL_SCREEN_MODE

	if ( [super respondsToSelector:@selector(poseAsOrderWindow:relativeTo:)] )
		[super poseAsOrderWindow:nOrderingMode relativeTo:nOtherWindowNumber];
}

- (MacOSBOOL)performKeyEquivalent:(NSEvent *)pEvent
{
	MacOSBOOL bCommandKeyPressed = ( pEvent && [pEvent modifierFlags] & NSCommandKeyMask );

	if ( bCommandKeyPressed && [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
	{
		// Fix bug 2783 by cancelling menu actions if the input method if the
		// there is any marked text in this window
		if ( NSWindow_hasMarkedText( self ) )
			return YES;

		// Implement the standard window minimization behavior with the
		// Command-m event
		if ( [self styleMask] & NSMiniaturizableWindowMask )
		{
			NSString *pChars = [pEvent charactersIgnoringModifiers];
			if ( pChars && [pChars isEqualToString:@"m"] )
			{
				// Fix bug 3562 by not allowing utility windows to be minimized
				if ( ! ( [self styleMask] & NSUtilityWindowMask ) )
					[self miniaturize:self];
				return YES;
			}
		}
	}

	MacOSBOOL bRet = NO;
	if ( [super respondsToSelector:@selector(poseAsPerformKeyEquivalent:)] )
		bRet = [super poseAsPerformKeyEquivalent:pEvent];

	// Fix bug 1751 by responding to Command-c, Command-v, and Command-x keys
	// for non-Java windows. Fix bug 3561 by responding to Command-w keys for
	// closable non-Java windows.
	if ( !bRet && bCommandKeyPressed && [self isVisible] && ![self isKindOfClass:[VCLPanel class]] && ![self isKindOfClass:[VCLWindow class]] )
	{
		NSApplication *pApp = [NSApplication sharedApplication];
		NSString *pChars = [pEvent charactersIgnoringModifiers];
		NSResponder *pResponder = [self firstResponder];
		if ( pApp && pChars && pResponder )
		{
			// Fix broken key shortcuts when running in the sandbox by having
			// NSApplication send find the applicable responder that can handle
			// the key shortcut
			if ( [pChars isEqualToString:@"a"] )
			{
				bRet = [pApp sendAction:@selector(selectAll:) to:nil from:self];
			}
			else if ( [pChars isEqualToString:@"c"] )
			{
				bRet = [pApp sendAction:@selector(copy:) to:nil from:self];
			}
			else if ( [pChars isEqualToString:@"v"] )
			{
				bRet = [pApp sendAction:@selector(paste:) to:nil from:self];
			}
			else if ( [pChars isEqualToString:@"w"] && [self styleMask] & NSClosableWindowMask )
			{
				[self performClose:self];
				bRet = YES;
			}
			else if ( [pChars isEqualToString:@"x"] )
			{
				bRet = [pApp sendAction:@selector(cut:) to:nil from:self];
			}
			else if ( [pChars isEqualToString:@"z"] )
			{
				bRet = [pApp sendAction:@selector(undo:) to:nil from:self];
			}
			else if ( [pChars isEqualToString:@"Z"] )
			{
				bRet = [pApp sendAction:@selector(redo:) to:nil from:self];
			}
		}
	}

	return bRet;
}

- (void)resignKeyWindow
{
	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) && mpFrame )
	{
		// Fix bug 1819 by forcing cancellation of the input method
		NSResponder *pResponder = [self firstResponder];
		if ( pResponder && [pResponder isKindOfClass:[VCLView class]] )
			[(VCLView *)pResponder abandonInput];

		// Have content view post its pending key up event
		NSView *pContentView = [self contentView];
		if ( pContentView && [pContentView isKindOfClass:[VCLView class]] )
			[(VCLView *)pContentView keyUp:nil];

		// Do not post a lost focus event if we are tracking the menubar as
		// it will cause the menubar to disappear when then help menu is opened
		VCLApplicationDelegate *pSharedDelegate = [VCLApplicationDelegate sharedDelegate];
		if ( pSharedDelegate && ![pSharedDelegate isInTracking] )
		{
			JavaSalEvent *pFocusEvent = new JavaSalEvent( SALEVENT_LOSEFOCUS, mpFrame, NULL );
			JavaSalEventQueue::postCachedEvent( pFocusEvent );
			pFocusEvent->release();
		}
	}

	if ( [super respondsToSelector:@selector(poseAsResignKeyWindow)] )
		[super poseAsResignKeyWindow];

	[VCLWindow clearModalWindowLevel];
}

- (void)sendEvent:(NSEvent *)pEvent
{
	if ( !pEvent )
		return;

	// Cache the event time
	JavaSalEventQueue::setLastNativeEventTime( [pEvent timestamp] );

	MacOSBOOL bIsVCLWindow = ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] );
	NSEventType nType = [pEvent type];
	NSRect aOldFrame = [self frame];

	// Fix bug 3357 by updating when we are not in a menu tracking session.
	// Fix bug 3379 by retaining this window as this window may get released
	// while updating.
	if ( bIsVCLWindow && [self isVisible] && nType == NSFlagsChanged && [pEvent modifierFlags] & NSCommandKeyMask )
	{
		[self retain];

		if ( VCLInstance_updateNativeMenus() )
		{
			// Fix bug reported in the following NeoOffice forum topic by
			// forcing any pending menu changes to be done before any key 
			// shortcuts are matched to menu items:
			// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8532
			[VCLMainMenuDidEndTracking mainMenuDidEndTracking:YES];
		}

		MacOSBOOL bVisible = [self isVisible];
		[self release];
		if ( !bVisible )
			return;
	}

	// Fix crashing when rapidly closing several windows by retaining this
	// window as this window may get released by the parent class
	[self retain];

	if ( [super respondsToSelector:@selector(poseAsSendEvent:)] )
		[super poseAsSendEvent:pEvent];

	// Fix bug reported in the following NeoOffice forum post by not checking
	// if a window is visible until after we are sure that it is one of our
	// custom windows as some of the native modal panels will dealloc windows
	// during [NSWindow sendEvent:]:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=62851#62851
	MacOSBOOL bVisible = [self isVisible];
	[self release];
	if ( !bVisible )
		return;

	if ( bIsVCLWindow && mpFrame )
	{
		// Handle all mouse events
		if ( ( nType >= NSLeftMouseDown && nType <= NSMouseExited ) || ( nType >= NSOtherMouseDown && nType <= NSOtherMouseDragged ) )
		{
			// If the frame changed after the superclass handled the event,
			// then we likely clicked on one of the titlebar buttons so
			// ignore the event to prevent unexpected dragging events
			NSRect aFrame = [self frame];
			if ( aFrame.size.width != aOldFrame.size.width || aFrame.size.height != aOldFrame.size.height )
				return;

			USHORT nID = 0;
			NSUInteger nModifiers = [pEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask;

			switch ( nType )
			{
				case NSMouseMoved:
					// Ignore mouse move events generated by tracking areas
					if ( ![self isKeyWindow] )
						return;
					nID = SALEVENT_MOUSEMOVE;
					nMouseMask = 0;
					break;
				case NSMouseEntered:
				case NSLeftMouseDragged:
				case NSRightMouseDragged:
				case NSOtherMouseDragged:
					nID = SALEVENT_MOUSEMOVE;
					nModifiers |= nMouseMask;
					break;
				case NSMouseExited:
					nID = SALEVENT_MOUSELEAVE;
					nModifiers |= nMouseMask;
					break;
				case NSLeftMouseDown:
				case NSRightMouseDown:
				case NSOtherMouseDown:
					nID = SALEVENT_MOUSEBUTTONDOWN;
					nMouseMask |= NSEventMaskFromType( nType );
					nModifiers |= nMouseMask;
					break;
				case NSLeftMouseUp:
				case NSRightMouseUp:
				case NSOtherMouseUp:
					// Remove matching mouse down mask
					nID = SALEVENT_MOUSEBUTTONUP;
					nModifiers |= NSEventMaskFromType( nType - 1 );
					nMouseMask &= ~NSEventMaskFromType( nType - 1 );
					break;
				default:
					break;
			}

			if ( nID )
			{
				// The OOo code can get confused when we click on a non-focused
				// window. In these cases, we will receive no mouse move events
				// so if the OOo code displays a popup menu, the popup menu
				// will receive no mouse move events.
				if ( nID == SALEVENT_MOUSEBUTTONDOWN )
				{
					if ( [self canBecomeKeyWindow] && ![self isKeyWindow] && ! ( nModifiers & NSLeftMouseDownMask ) )
					{
						mnIgnoreMouseReleasedModifiers = nModifiers;
						return;
					}

					// For events where the event is in a window's titlebar,
					// use the first drag event as the coordinates should not
					// change but they do sometimes change
					NSRect aContentFrame = [self contentRectForFrameRect:aFrame];
					float fLeftInset = aFrame.origin.x - aContentFrame.origin.x;
					float fTopInset = aFrame.origin.y + aFrame.size.height - aContentFrame.origin.y - aContentFrame.size.height;
					NSRect aTitlebarFrame = NSMakeRect( fLeftInset, aContentFrame.origin.y + aContentFrame.size.height - aFrame.origin.y, aFrame.size.width, fTopInset );
					NSPoint aLocation = [pEvent locationInWindow];
					if ( NSPointInRect( aLocation, aTitlebarFrame ) )
					{
						if ( nModifiers & NSLeftMouseDownMask )
						{
							if ( mpLastWindowDraggedEvent )
								[mpLastWindowDraggedEvent release];
							mpLastWindowDraggedEvent = pEvent;
							if ( mpLastWindowDraggedEvent )
								[mpLastWindowDraggedEvent retain];
						}
					}
					else if ( mpLastWindowDraggedEvent )
					{
						[mpLastWindowDraggedEvent release];
						mpLastWindowDraggedEvent = nil;
					}

					mnIgnoreMouseReleasedModifiers = 0;
				}
				else if ( nID == SALEVENT_MOUSEBUTTONUP )
				{
					// Fix bug 3453 by adding back any recently released
					// modifiers
					if ( mnLastMetaModifierReleasedTime >= (ULONG)( JavaSalEventQueue::getLastNativeEventTime() * 1000 ) )
						nModifiers |= NSCommandKeyMask;

					if ( mpLastWindowDraggedEvent && nModifiers & NSLeftMouseDownMask )
					{
						[mpLastWindowDraggedEvent release];
						mpLastWindowDraggedEvent = nil;
					}

					if ( mnIgnoreMouseReleasedModifiers && ( mnIgnoreMouseReleasedModifiers & nModifiers ) == nModifiers )
					{
						mnIgnoreMouseReleasedModifiers &= ~nModifiers;
						return;
					}
				}

				USHORT nCode = GetEventCode( nModifiers );

				// Fix bug 2769 by creating synthetic window moved events
				// when dragging a window's title bar
				NSEvent *pPositionEvent = pEvent;
				if ( mpLastWindowDraggedEvent )
				{
					pPositionEvent = mpLastWindowDraggedEvent;
					[self windowDidMove:nil];
				}
				NSPoint aLocation = GetFlippedContentViewLocation( self, pPositionEvent );
				SalMouseEvent *pMouseEvent = new SalMouseEvent();
				pMouseEvent->mnTime = (ULONG)( JavaSalEventQueue::getLastNativeEventTime() * 1000 );
				pMouseEvent->mnX = (long)aLocation.x;
				pMouseEvent->mnY = (long)aLocation.y;
				if ( nID == SALEVENT_MOUSEMOVE || nID == SALEVENT_MOUSELEAVE )
					pMouseEvent->mnButton = 0;
				else
					pMouseEvent->mnButton = nCode & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT );
				pMouseEvent->mnCode = nCode;

				SalMouseEvent *pExtraMouseEvent = NULL;
				if ( nID == SALEVENT_MOUSEBUTTONUP )
				{
					// Strange but true, fix bug 2157 by posting a synthetic
					// mouse moved event
					USHORT nExtraCode = GetEventCode( ( [pEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask ) | nMouseMask );
					pExtraMouseEvent = new SalMouseEvent();
					pExtraMouseEvent->mnTime = (ULONG)( JavaSalEventQueue::getLastNativeEventTime() * 1000 );
					pExtraMouseEvent->mnX = (long)aLocation.x;
					pExtraMouseEvent->mnY = (long)aLocation.y;
					pExtraMouseEvent->mnButton = 0;
					pExtraMouseEvent->mnCode = nExtraCode;
				}

				JavaSalEvent *pSalMouseEvent = new JavaSalEvent( nID, mpFrame, pMouseEvent );
				JavaSalEventQueue::postCachedEvent( pSalMouseEvent );
				pSalMouseEvent->release();

				if ( pExtraMouseEvent )
				{
					JavaSalEvent *pSalExtraMouseEvent = new JavaSalEvent( SALEVENT_MOUSEMOVE, mpFrame, pExtraMouseEvent );
					JavaSalEventQueue::postCachedEvent( pSalExtraMouseEvent );
					pSalExtraMouseEvent->release();
				}
			}
		}
		// Handle key modifier change events
		else if ( nType == NSFlagsChanged )
		{
			NSUInteger nModifiers = [pEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask;
			if ( nModifiers & NSCommandKeyMask )
				mnLastMetaModifierReleasedTime = 0;
			else
				mnLastMetaModifierReleasedTime = (ULONG)( JavaSalEventQueue::getLastNativeEventTime() * 1000 ) + MODIFIER_RELEASE_INTERVAL;

			USHORT nCode = GetEventCode( nModifiers );

			SalKeyModEvent *pKeyModEvent = new SalKeyModEvent();
			pKeyModEvent->mnTime = (ULONG)( JavaSalEventQueue::getLastNativeEventTime() * 1000 );
			pKeyModEvent->mnCode = nCode;
			pKeyModEvent->mnModKeyCode = 0;

			JavaSalEvent *pSalKeyModEvent = new JavaSalEvent( SALEVENT_KEYMODCHANGE, mpFrame, pKeyModEvent );
			JavaSalEventQueue::postCachedEvent( pSalKeyModEvent );
			pSalKeyModEvent->release();
		}
		// Handle scroll wheel, magnify, and swipe
		else if ( nType == NSScrollWheel || ( ( nType == NSEventTypeMagnify || nType == NSEventTypeSwipe ) && pSharedResponder && ![pSharedResponder ignoreTrackpadGestures] ) )
		{
			static float fUnpostedMagnification = 0;
			static float fUnpostedVerticalScrollWheel = 0;

			NSApplication *pApp = [NSApplication sharedApplication];
			int nModifiers = [pEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask;
			float fDeltaX = 0;
			float fDeltaY = 0;
			if ( nType == NSEventTypeMagnify )
			{
				fUnpostedVerticalScrollWheel = 0;

				// Magnify events need to be converted to vertical scrolls with
				// the Command key pressed to force the OOo code to zoom.
				nModifiers |= NSCommandKeyMask;
				fDeltaY = [pEvent magnification];

				if ( pApp )
				{
					NSEvent *pPendingEvent;
					while ( ( pPendingEvent = [pApp nextEventMatchingMask:NSEventMaskFromType( nType ) untilDate:[NSDate date] inMode:( [pApp modalWindow] ? NSModalPanelRunLoopMode : NSDefaultRunLoopMode ) dequeue:YES] ) != nil )
						fDeltaY += [pPendingEvent magnification];
				}

				fUnpostedMagnification += fDeltaY;
				if ( FloatToLong( fUnpostedMagnification ) )
				{
					fDeltaY = fUnpostedMagnification;
					fUnpostedMagnification = 0;
				}
				else
				{
					fDeltaY = 0;
				}
			}
			else
			{
				fUnpostedMagnification = 0;

				// Fix bug 3284 by not rounding tiny scroll wheel and swipe
				// amounts to a non-zero integer and, instead, set the
				// amount to zero until there are no more pending events of
				// this type. Fix the unresponsive veritical scrollwheel
				// events reported in the following NeoOffice forum topic
				// by only reducing horizontal events:
				// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8609
				fDeltaX = [pEvent deltaX];
				fDeltaY = [pEvent deltaY];

				if ( nModifiers & NSCommandKeyMask )
				{
					// Only allow horizontal scroll when the Command key is not
					// pressed
					fDeltaX = 0;

					if ( pApp )
					{
						NSEvent *pPendingEvent;
						while ( ( pPendingEvent = [pApp nextEventMatchingMask:NSEventMaskFromType( nType ) untilDate:[NSDate date] inMode:( [pApp modalWindow] ? NSModalPanelRunLoopMode : NSDefaultRunLoopMode ) dequeue:YES] ) != nil )
							fDeltaY += [pEvent deltaY];
					}

					// Precise scrolling devices have excessively large
					// deltas so apply a much larger reduction factor when
					// zooming
					if ( [pEvent hasPreciseScrollingDeltas] )
						fDeltaY /= 10.0f;
					else
						fDeltaY /= 5.0f;

					fUnpostedVerticalScrollWheel += fDeltaY;
					if ( FloatToLong( fUnpostedVerticalScrollWheel ) )
					{
						fDeltaY = fUnpostedVerticalScrollWheel;
						fUnpostedVerticalScrollWheel = 0;
					}
					else
					{
						fDeltaY = 0;
					}
				}
				else
				{
					fUnpostedVerticalScrollWheel = 0;

					// Precise scrolling devices have excessively large
					// deltas so apply a small reduction factor when scrolling
					// so that scrolling can be done down to a single line
					if ( [pEvent hasPreciseScrollingDeltas] )
					{
						fDeltaX /= 2.0f;
						fDeltaY /= 2.0f;
					}
				}
			}

			NSPoint aLocation = GetFlippedContentViewLocation( self, pEvent );

	        // Fix bug 3030 by setting the modifiers. Note that we ignore the
			// Shift modifier as using it will disable horizontal scrolling.
			USHORT nCode = GetEventCode( nModifiers ) & ( KEY_MOD1 | KEY_MOD2 | KEY_MOD3 );
			MacOSBOOL bScrollPages = ( nType == NSEventTypeSwipe && ! ( nModifiers & NSCommandKeyMask ) );

			long nDeltaX = FloatToLong( fDeltaX );
			if ( !nDeltaX )
			{
				// Don't ignore tiny, non-zero amounts
				if ( fDeltaX > 0 )
					nDeltaX = 1;
				else if ( fDeltaX < 0 )
					nDeltaX = -1;
			}
			if ( nDeltaX )
			{
				SalWheelMouseEvent *pWheelMouseEvent = new SalWheelMouseEvent();
				pWheelMouseEvent->mnTime = (ULONG)( JavaSalEventQueue::getLastNativeEventTime() * 1000 );
				pWheelMouseEvent->mnX = (long)aLocation.x;
				pWheelMouseEvent->mnY = (long)aLocation.y;
				pWheelMouseEvent->mnDelta = nDeltaX;
				pWheelMouseEvent->mnNotchDelta = ( nDeltaX < 0 ? -1 : 1 );
				pWheelMouseEvent->mnScrollLines = ( bScrollPages ? SAL_WHEELMOUSE_EVENT_PAGESCROLL : abs( nDeltaX ) );
				pWheelMouseEvent->mnCode = nCode;
				pWheelMouseEvent->mbHorz = TRUE;

				JavaSalEvent *pSalWheelMouseEvent = new JavaSalEvent( SALEVENT_WHEELMOUSE, mpFrame, pWheelMouseEvent );
				JavaSalEventQueue::postCachedEvent( pSalWheelMouseEvent );
				pSalWheelMouseEvent->release();
			}
			long nDeltaY = FloatToLong( fDeltaY );
			if ( !nDeltaY )
			{
				// Don't ignore tiny, non-zero amounts
				if ( fDeltaY > 0 )
					nDeltaY = 1;
				else if ( fDeltaY < 0 )
					nDeltaY = -1;
			}
			if ( nDeltaY )
			{
				SalWheelMouseEvent *pWheelMouseEvent = new SalWheelMouseEvent();
				pWheelMouseEvent->mnTime = (ULONG)( JavaSalEventQueue::getLastNativeEventTime() * 1000 );
				pWheelMouseEvent->mnX = (long)aLocation.x;
				pWheelMouseEvent->mnY = (long)aLocation.y;
				pWheelMouseEvent->mnDelta = nDeltaY;
				pWheelMouseEvent->mnNotchDelta = ( nDeltaY < 0 ? -1 : 1 );
				pWheelMouseEvent->mnScrollLines = ( bScrollPages ? SAL_WHEELMOUSE_EVENT_PAGESCROLL : abs( nDeltaY ) );
				pWheelMouseEvent->mnCode = nCode;
				pWheelMouseEvent->mbHorz = FALSE;

				JavaSalEvent *pSalWheelMouseEvent = new JavaSalEvent( SALEVENT_WHEELMOUSE, mpFrame, pWheelMouseEvent );
				JavaSalEventQueue::postCachedEvent( pSalWheelMouseEvent );
				pSalWheelMouseEvent->release();
			}
		}

		// Cache mouse event in dragging source
		if ( [self respondsToSelector:@selector(draggingSourceDelegate)] )
		{
			id pDelegate = [self draggingSourceDelegate];
			if ( pDelegate )
			{
				switch ( nType )
				{
					case NSLeftMouseDown:
						if ( [pDelegate respondsToSelector:@selector(mouseDown:)] )
							[pDelegate mouseDown:pEvent];
						break;
					case NSLeftMouseDragged:
						if ( [pDelegate respondsToSelector:@selector(mouseDragged:)] )
							[pDelegate mouseDragged:pEvent];
						break;
					default:
						break;
				}
			}
		}
	}
}

- (void)setCanBecomeKeyWindow:(MacOSBOOL)bCanBecomeKeyWindow
{
	mbCanBecomeKeyWindow = bCanBecomeKeyWindow;
}

- (void)setFrame:(JavaSalFrame *)pFrame
{
	mpFrame = pFrame;

	NSView *pContentView = [self contentView];
	if ( pContentView && [pContentView isKindOfClass:[VCLView class]] )
		[(VCLView *)pContentView setFrame:pFrame];
}

- (void)setDraggingSourceDelegate:(id)pDelegate
{
	if ( !pDraggingSourceDelegates )
	{
		pDraggingSourceDelegates = [NSMutableDictionary dictionaryWithCapacity:10];
		if ( pDraggingSourceDelegates )
			[pDraggingSourceDelegates retain];
	}

	if ( pDraggingSourceDelegates )
	{
		NSNumber *pKey = [NSNumber numberWithUnsignedLong:(unsigned long)self];
		if ( pKey )
		{
			if ( pDelegate )
				[pDraggingSourceDelegates setObject:pDelegate forKey:pKey];
			else
				[pDraggingSourceDelegates removeObjectForKey:pKey];
		}
	}
}

- (void)windowDidExitFullScreen:(NSNotification *)pNotification
{
#ifdef USE_NATIVE_FULL_SCREEN_MODE
	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
		VCLEventQueue_fullScreen( self, NO );
#endif	// USE_NATIVE_FULL_SCREEN_MODE
}

- (void)windowWillEnterFullScreen:(NSNotification *)pNotification
{
#ifdef USE_NATIVE_FULL_SCREEN_MODE
	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
		VCLEventQueue_fullScreen( self, YES );
#endif	// USE_NATIVE_FULL_SCREEN_MODE
}

- (void)windowDidMove:(NSNotification *)pNotification
{
	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) && mpFrame )
	{
		JavaSalEvent *pMoveResizeEvent = new JavaSalEvent( SALEVENT_MOVERESIZE, mpFrame, NULL );
		JavaSalEventQueue::postCachedEvent( pMoveResizeEvent );
		pMoveResizeEvent->release();
	}
}

- (void)windowDidResize:(NSNotification *)pNotification
{
	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) && mpFrame )
	{
		JavaSalEvent *pMoveResizeEvent = new JavaSalEvent( SALEVENT_MOVERESIZE, mpFrame, NULL );
		JavaSalEventQueue::postCachedEvent( pMoveResizeEvent );
		pMoveResizeEvent->release();
	}
}

- (void)windowDidDeminiaturize:(NSNotification *)pNotification
{
	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) && mpFrame )
	{
		JavaSalEvent *pDeminimizedEvent = new JavaSalEvent( SALEVENT_DEMINIMIZED, mpFrame, NULL );
		JavaSalEventQueue::postCachedEvent( pDeminimizedEvent );
		pDeminimizedEvent->release();
	}
}

- (void)windowWillMiniaturize:(NSNotification *)pNotification
{
	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) && mpFrame )
	{
		JavaSalEvent *pMinimizedEvent = new JavaSalEvent( SALEVENT_MINIMIZED, mpFrame, NULL );
		JavaSalEventQueue::postCachedEvent( pMinimizedEvent );
		pMinimizedEvent->release();
	}
}

- (MacOSBOOL)windowShouldClose:(id)pObject
{
	MacOSBOOL bRet = YES;

	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
	{
		if ( mpFrame )
		{
			JavaSalEvent *pCloseEvent = new JavaSalEvent( SALEVENT_CLOSE, mpFrame, NULL );
			JavaSalEventQueue::postCachedEvent( pCloseEvent );
			pCloseEvent->release();
			bRet = NO;
		}
	}

	return bRet;
}

- (MacOSBOOL)windowShouldZoom:(NSWindow *)pWindow toFrame:(NSRect)aNewFrame
{
	MacOSBOOL bRet = YES;

	if ( pWindow == self && [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] )
	{
		if ( ![self isVisible] )
		{
			bRet = NO;
		}
		else
		{
			// If the window's frame is roughly the same is the screen's
			// visible frame and the new frame's height has very little room
			// for content, disable zooming to prevent the window from being
			// unexpectedly shrunk to its minimum size
			NSRect aContentRect = [self contentRectForFrameRect:aNewFrame];
			if ( aContentRect.size.height < 25.0 )
			{
				NSScreen *pScreen = [self screen];
				if ( pScreen )
				{
					NSRect aFrame = [self frame];
					NSRect aVisibleFrame = [pScreen visibleFrame];
					if ( fabs( aFrame.origin.x - aVisibleFrame.origin.x ) < 25.0f &&
						fabs( aFrame.origin.y - aVisibleFrame.origin.y ) < 25.0f &&
						fabs( aFrame.size.width - aVisibleFrame.size.width ) < 50.0f &&
						fabs( aFrame.size.height - aVisibleFrame.size.height ) < 50.0f )
							bRet = NO;
				}
			}
		}
	}

	return bRet;
}

- (void)windowWillClose:(NSNotification *)pNotification
{
	RegisterMainBundleWithLaunchServices();
}

- (void)windowWillEnterVersionBrowser:(NSNotification *)notification
{
	if ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] )
		mbInVersionBrowser = YES;
}

- (void)windowWillExitVersionBrowser:(NSNotification *)pNotification
{
	RegisterMainBundleWithLaunchServices();
}

- (void)windowDidExitVersionBrowser:(NSNotification *)pNotification
{
	if ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] )
	{
		mbInVersionBrowser = NO;

		// Stop reappearance of phantom document window when the user has
		// close the window while in the version browser using the File :: Close
		// menu or the Command-W key shortcut
		if ( mbCloseOnExitVersionBrowser )
		{
			mbCloseOnExitVersionBrowser = NO;
			[self close];
		}

		// Force menubar to be visible as it sometimes is disabled or hidden
		NSApplication *pApp = [NSApplication sharedApplication];
		if ( pApp )
			[pApp setPresentationOptions:[pApp presentationOptions]];
	}
}

@end

static CFStringRef aTextSelection = nil;
static CFDataRef aRTFSelection = nil;

@implementation VCLView

- (MacOSBOOL)acceptsFirstResponder
{
	return YES;
}

- (void)abandonInput
{
	if ( mpInputManager && [self hasMarkedText] )
	{
		[mpInputManager markedTextAbandoned:self];
		[self unmarkText];
	}
}

- (id)accessibilityAttributeValue:(NSString *)aAttribute
{
	if ( [NSAccessibilityRoleAttribute isEqualToString:aAttribute] )
	{
		return NSAccessibilityTextAreaRole;
	}
	else if ( [NSAccessibilitySelectedTextAttribute isEqualToString:aAttribute] )
	{
		NSWindow *pWindow = [self window];
		if ( pWindow && [pWindow isVisible] )
		{
			if ( aTextSelection )
			{
				CFRelease( aTextSelection );
				aTextSelection = nil;
			}

			VCLEventQueue_getTextSelection( pWindow, &aTextSelection, NULL );
			if ( aTextSelection )
				return (NSString *)aTextSelection;
		}

		return @"";
	}
	else
	{
		return [super accessibilityAttributeValue:aAttribute];
	}
}

- (MacOSBOOL)accessibilityIsIgnored
{
	return NO;
}

- (void)dealloc
{
	if ( mpInputManager )
		[mpInputManager release];

	if ( mpLastKeyDownEvent )
		[mpLastKeyDownEvent release];

	if ( mpPendingKeyUpEvent )
		delete mpPendingKeyUpEvent;

	if ( mpTextInput )
		[mpTextInput release];

	[super dealloc];
}

- (void)keyDown:(NSEvent *)pEvent
{
	if ( mpLastKeyDownEvent )
		[mpLastKeyDownEvent release];
	mpLastKeyDownEvent = ( [pEvent type] == NSKeyDown ? pEvent : nil );
	if ( mpLastKeyDownEvent )
		[mpLastKeyDownEvent retain];

	if ( mpPendingKeyUpEvent )
	{
		delete mpPendingKeyUpEvent;
		mpPendingKeyUpEvent = NULL;
	}

	[self interpretKeyEvents:[NSArray arrayWithObject:pEvent]];

	// Fix broken repeat key events on Mac OS X 10.8 by explicitly posting
	// the key down event if the interpretKeyEvents: selector does not post
	// anything. Do not do this step if there is uncommitted text.
	if ( !mpTextInput && mpLastKeyDownEvent && [mpLastKeyDownEvent isARepeat] )
		[self insertText:[mpLastKeyDownEvent characters] replacementRange:NSMakeRange( NSNotFound, 0 )];
}

- (void)keyUp:(NSEvent *)pEvent
{
	if ( mpLastKeyDownEvent )
	{
		[mpLastKeyDownEvent release];
		mpLastKeyDownEvent = nil;
	}

	if ( mpPendingKeyUpEvent )
	{
		NSWindow *pWindow = [self window];
		if ( pWindow && [pWindow isVisible] && mpFrame )
		{
			mpPendingKeyUpEvent->mnTime = (ULONG)( JavaSalEventQueue::getLastNativeEventTime() * 1000 );

			JavaSalEvent *pKeyEvent = new JavaSalEvent( SALEVENT_KEYUP, mpFrame, mpPendingKeyUpEvent );
			JavaSalEventQueue::postCachedEvent( pKeyEvent );
			pKeyEvent->release();
		}
		else
		{
			delete mpPendingKeyUpEvent;
		}

		mpPendingKeyUpEvent = NULL;
	}
}

- (MacOSBOOL)hasMarkedText
{
	return ( mpTextInput ? YES : NO );
}

- (NSRange)markedRange
{
	return maTextInputRange;
}

- (NSRange)selectedRange
{
	// Fix bug when using the Mac OS X 10.8 dictation input method reported in
	// the following NeoOffice forum topic by never returning NSNotFound in the
	// selected range's location:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8514
	return ( maSelectedRange.location == NSNotFound ? NSMakeRange( 0, 0 ) : maSelectedRange );
}

- (void)setMarkedText:(id)aString selectedRange:(NSRange)aSelectedRange replacementRange:(NSRange)aReplacementRange
{
	maSelectedRange = NSMakeRange( NSNotFound, 0 );
	maTextInputRange = NSMakeRange( NSNotFound, 0 );

	if ( mpPendingKeyUpEvent )
	{
		delete mpPendingKeyUpEvent;
		mpPendingKeyUpEvent = NULL;
	}

	if ( mpInputManager )
		[mpInputManager release];
	mpInputManager = [NSInputManager currentInputManager];
	if ( mpInputManager )
		[mpInputManager retain];

	if ( mpTextInput )
	{
		[mpTextInput release];
		mpTextInput = nil;
	}

	if ( aString )
	{
		NSUInteger nLen = 0;
		if ( [aString isKindOfClass:[NSAttributedString class]] )
			nLen = [(NSAttributedString *)aString length];
		else if ( [aString isKindOfClass:[NSString class]] )
			nLen = [(NSString *)aString length];

		if ( nLen )
		{
			maTextInputRange = NSMakeRange( 0, nLen );
			maSelectedRange = aSelectedRange;

			mpTextInput = aString;
			[mpTextInput retain];
		}
	}

	NSWindow *pWindow = [self window];
	if ( pWindow && [pWindow isVisible] && mpFrame )
	{
		XubString aText;
		NSString *pChars = nil;
		if ( mpTextInput )
		{
			if ( [mpTextInput isKindOfClass:[NSAttributedString class]] )
				pChars = [(NSAttributedString *)mpTextInput string];
			else if ( [mpTextInput isKindOfClass:[NSString class]] )
				pChars = (NSString *)mpTextInput;

			if ( pChars && [pChars length] )
			{
				NSUInteger nLen = [pChars length];
				sal_Unicode aBuf[ nLen + 1 ];
				[pChars getCharacters:aBuf];
				aBuf[ nLen ] = 0;
				aText = XubString( aBuf );
			}
		}

		ULONG nLen = aText.Len();
		ULONG nCursorPos = ( maSelectedRange.location == NSNotFound ? nLen : maSelectedRange.location );
		if ( nCursorPos > nLen )
			nCursorPos = nLen;

		USHORT *pAttr = NULL;
		if ( nLen )
		{
			pAttr = (USHORT *)rtl_allocateMemory( sizeof( USHORT* ) * nLen );
			if ( pAttr )
			{
				// If no characters are selected, highlight all of them
				ULONG nStartHighlightPos = 0;
				ULONG nEndHighlightPos = nLen;
				if ( maSelectedRange.location != NSNotFound && maSelectedRange.location < nLen && maSelectedRange.length )
				{
					nStartHighlightPos = maSelectedRange.location;
					nEndHighlightPos = maSelectedRange.location + maSelectedRange.length;
				}

				for ( USHORT i = 0; i < nLen; i++ )
				{
					if ( i >= nStartHighlightPos && i < nEndHighlightPos )
						pAttr[ i ] = SAL_EXTTEXTINPUT_ATTR_HIGHLIGHT;
					else
						pAttr[ i ] = SAL_EXTTEXTINPUT_ATTR_UNDERLINE;
				}
			}
		}

		SalExtTextInputEvent *pInputEvent = new SalExtTextInputEvent();
		pInputEvent->mnTime = (ULONG)( JavaSalEventQueue::getLastNativeEventTime() * 1000 );
		pInputEvent->maText = aText;
		pInputEvent->mpTextAttr = pAttr;
		pInputEvent->mnCursorPos = nCursorPos;
		pInputEvent->mnDeltaStart = 0;
		pInputEvent->mbOnlyCursor = FALSE;
		pInputEvent->mnCursorFlags = 0;

		JavaSalEvent *pSalInputEvent = new JavaSalEvent( SALEVENT_EXTTEXTINPUT, mpFrame, (void *)pInputEvent, OString(), 0, nCursorPos );
		JavaSalEventQueue::postCachedEvent( pSalInputEvent );
		pSalInputEvent->release();
	}
}

- (void)unmarkText
{
	maSelectedRange = NSMakeRange( NSNotFound, 0 );
	maTextInputRange = NSMakeRange( NSNotFound, 0 );

	if ( mpPendingKeyUpEvent )
	{
		delete mpPendingKeyUpEvent;
		mpPendingKeyUpEvent = NULL;
	}

	if ( mpInputManager )
	{
		[mpInputManager release];
		mpInputManager = nil;
	}

	if ( mpTextInput )
	{
		[mpTextInput release];
		mpTextInput = nil;

		NSWindow *pWindow = [self window];
		if ( pWindow && [pWindow isVisible] && mpFrame )
		{
			SalExtTextInputEvent *pInputEvent = new SalExtTextInputEvent();
			pInputEvent->mnTime = (ULONG)( JavaSalEventQueue::getLastNativeEventTime() * 1000 );
			pInputEvent->maText = XubString();
			pInputEvent->mpTextAttr = NULL;
			pInputEvent->mnCursorPos = 0;
			pInputEvent->mnDeltaStart = 0;
			pInputEvent->mbOnlyCursor = FALSE;
			pInputEvent->mnCursorFlags = 0;

			JavaSalEvent *pSalInputEvent = new JavaSalEvent( SALEVENT_EXTTEXTINPUT, mpFrame, (void *)pInputEvent, OString(), 0, 0 );
			JavaSalEventQueue::postCachedEvent( pSalInputEvent );
			pSalInputEvent->release();
		}
	}
}

- (NSArray *)validAttributesForMarkedText
{
	return [NSArray arrayWithObject:NSUnderlineStyleAttributeName];
}

- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)pActualRange
{
	NSAttributedString *pRet = nil;

	if ( mpTextInput && aRange.location != NSNotFound )
	{
		NSUInteger nLen = [mpTextInput length];
		if ( nLen )
		{
			maTextInputRange = NSMakeRange( 0, nLen );
			NSRange aSubstringRange = NSIntersectionRange( aRange, maTextInputRange );
			if ( aSubstringRange.location != NSNotFound && aSubstringRange.location < nLen - 1 && aSubstringRange.length )
			{
				if ( [mpTextInput isKindOfClass:[NSAttributedString class]] )
					pRet = [(NSAttributedString *)mpTextInput attributedSubstringFromRange:aSubstringRange];
				else if ( [mpTextInput isKindOfClass:[NSString class]] )
				{
					NSString *pSubstring = [(NSString *)mpTextInput substringWithRange:aSubstringRange];
					if ( pSubstring )
						pRet = [[NSAttributedString alloc] initWithString:pSubstring];
				}
			}
		}
	}

	return pRet;
}

- (void)insertText:(id)aString replacementRange:(NSRange)aReplacementRange
{
	maSelectedRange = NSMakeRange( NSNotFound, 0 );
	maTextInputRange = NSMakeRange( NSNotFound, 0 );

	if ( mpPendingKeyUpEvent )
	{
		delete mpPendingKeyUpEvent;
		mpPendingKeyUpEvent = NULL;
	}

	if ( mpInputManager )
	{
		[mpInputManager release];
		mpInputManager = nil;
	}

	if ( mpTextInput )
	{
		[mpTextInput release];
		mpTextInput = nil;

		NSWindow *pWindow = [self window];
		if ( pWindow && [pWindow isVisible] && mpFrame )
		{
			NSString *pChars = nil;
			if ( [aString isKindOfClass:[NSAttributedString class]] )
				pChars = [(NSAttributedString *)aString string];
			else if ( [aString isKindOfClass:[NSString class]] )
				pChars = (NSString *)aString;

			XubString aText;
			if ( pChars && [pChars length] )
			{
				NSUInteger nLen = [pChars length];
				sal_Unicode aBuf[ nLen + 1 ];
				[pChars getCharacters:aBuf];
				aBuf[ nLen ] = 0;
				aText = XubString( aBuf );
			}

			ULONG nLen = aText.Len();
			SalExtTextInputEvent *pInputEvent = new SalExtTextInputEvent();
			pInputEvent->mnTime = (ULONG)( JavaSalEventQueue::getLastNativeEventTime() * 1000 );
			pInputEvent->maText = aText;
			pInputEvent->mpTextAttr = NULL;
			pInputEvent->mnCursorPos = nLen;
			pInputEvent->mnDeltaStart = 0;
			pInputEvent->mbOnlyCursor = FALSE;
			pInputEvent->mnCursorFlags = 0;

			JavaSalEvent *pSalInputEvent = new JavaSalEvent( SALEVENT_EXTTEXTINPUT, mpFrame, (void *)pInputEvent, OString(), nLen, nLen );
			JavaSalEventQueue::postCachedEvent( pSalInputEvent );
			pSalInputEvent->release();
		}
	}
	else
	{
		NSWindow *pWindow = [self window];
		if ( pWindow && [pWindow isVisible] && mpFrame )
		{
			NSString *pChars = nil;
			if ( [aString isKindOfClass:[NSAttributedString class]] )
				pChars = [(NSAttributedString *)aString string];
			else if ( [aString isKindOfClass:[NSString class]] )
				pChars = (NSString *)aString;
			if ( pChars && [pChars length] )
			{
				// Fix bug 710 by stripping out the Alt modifier. Note that we
				// do it here because we need to let the Alt modifier through
				// for action keys.
				NSUInteger nModifiers = ( ( mpLastKeyDownEvent ? [mpLastKeyDownEvent modifierFlags] : 0 ) & NSDeviceIndependentModifierFlagsMask ) | nMouseMask;

				NSUInteger i = 0;
				NSUInteger nLength = [pChars length];
				for ( ; i < nLength; i++ )
				{
					USHORT nChar = (USHORT)[pChars characterAtIndex:i];
					USHORT nCode = GetKeyCode( mpLastKeyDownEvent ? [mpLastKeyDownEvent keyCode] : UNDEFINED_KEY_CODE, nChar ) | GetEventCode( nModifiers & ~NSAlternateKeyMask );

					SalKeyEvent *pKeyDownEvent = new SalKeyEvent();
					pKeyDownEvent->mnTime = (ULONG)( JavaSalEventQueue::getLastNativeEventTime() * 1000 );
					pKeyDownEvent->mnCode = nCode;
					pKeyDownEvent->mnCharCode = nChar;
					pKeyDownEvent->mnRepeat = 0;

					SalKeyEvent *pKeyUpEvent = new SalKeyEvent();
					memcpy( pKeyUpEvent, pKeyDownEvent, sizeof( SalKeyEvent ) );
	
					JavaSalEvent *pSalKeyDownEvent = new JavaSalEvent( SALEVENT_KEYINPUT, mpFrame, pKeyDownEvent );
					JavaSalEventQueue::postCachedEvent( pSalKeyDownEvent );
					pSalKeyDownEvent->release();

					if ( i == nLength - 1 )
					{
						mpPendingKeyUpEvent = pKeyUpEvent;
					}
					else
					{
						JavaSalEvent *pSalKeyUpEvent = new JavaSalEvent( SALEVENT_KEYUP, mpFrame, pKeyUpEvent );
						JavaSalEventQueue::postCachedEvent( pSalKeyUpEvent );
						pSalKeyUpEvent->release();
					}
				}
			}
		}
	}

	// Do not clear the cached last key down event as it will cause an
	// uncommitted accent character to be committed when the delete key is
	// pressed
}

- (NSUInteger)characterIndexForPoint:(NSPoint)aPoint
{
#ifdef DEBUG
	fprintf( stderr, "[VCLView characterIndexForPoint:] not implemented\n" );
#endif
	return NSNotFound;
}

- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)pActualRange
{
	NSRect aRet = NSZeroRect;

	if ( pActualRange )
		pActualRange = NULL;

	NSWindow *pWindow = [self window];
	if ( pWindow && [pWindow isVisible] && mpFrame )
	{
		::vos::IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			// Fix bug 2426 by checking the frame pointer before any use
			SalData *pSalData = GetSalData();
			for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
			{
				if ( *it == mpFrame )
				{
					SalExtTextInputPosEvent *pInputPosEvent = new SalExtTextInputPosEvent();
					memset( pInputPosEvent, 0, sizeof( SalExtTextInputPosEvent ) );

					JavaSalEvent *pSalInputPosEvent = new JavaSalEvent( SALEVENT_EXTTEXTINPUTPOS, mpFrame, pInputPosEvent );
					pSalInputPosEvent->dispatch();

					if ( pInputPosEvent )
					{
						if ( pInputPosEvent->mbVertical )
							aRet = NSMakeRect( pInputPosEvent->mnX + pInputPosEvent->mnWidth + 25, pInputPosEvent->mnY - pInputPosEvent->mnHeight + 15, pInputPosEvent->mnWidth, pInputPosEvent->mnHeight );
						else
							aRet = NSMakeRect( pInputPosEvent->mnX, pInputPosEvent->mnY + 20, pInputPosEvent->mnWidth, pInputPosEvent->mnHeight );
					}

					pSalInputPosEvent->release();

					// Translate, flip coordinates within content frame, and
					// adjust to screen coordinates
					NSRect aFrame = [pWindow frame];
					NSRect aContentFrame = [pWindow contentRectForFrameRect:aFrame];
					aRet.origin.y = aContentFrame.size.height - aRet.origin.y;
					aRet.origin.x += aContentFrame.origin.x;
					aRet.origin.y += aContentFrame.origin.y;

					break;
				}
			}
		}

		rSolarMutex.release();
	}

	return aRet;
}

- (void)doCommandBySelector:(SEL)aSelector
{
	// Cancel any uncommitted input just to be safe
	[self unmarkText];

	NSWindow *pWindow = [self window];
	if ( pWindow && [pWindow isVisible] && mpFrame && mpLastKeyDownEvent )
	{
		// Get key binding if one exists
		JavaSalEvent *pKeyBindingDownEvent = NULL;
		JavaSalEvent *pKeyBindingUpEvent = NULL;
		if ( pSharedResponder )
		{
			[pSharedResponder doCommandBySelector:aSelector];
			short nCommandKey = [pSharedResponder lastCommandKey];
			if ( nCommandKey )
			{
				USHORT nCode = nCommandKey | [pSharedResponder lastModifiers];

				SalKeyEvent *pKeyDownEvent = new SalKeyEvent();
				pKeyDownEvent->mnTime = (ULONG)( JavaSalEventQueue::getLastNativeEventTime() * 1000 );
				pKeyDownEvent->mnCode = nCode;
				pKeyDownEvent->mnCharCode = 0;
				pKeyDownEvent->mnRepeat = 0;

				SalKeyEvent *pKeyUpEvent = new SalKeyEvent();
				memcpy( pKeyUpEvent, pKeyDownEvent, sizeof( SalKeyEvent ) );

				pKeyBindingDownEvent = new JavaSalEvent( SALEVENT_KEYINPUT, mpFrame, pKeyDownEvent );
				pKeyBindingUpEvent = new JavaSalEvent( SALEVENT_KEYUP, mpFrame, pKeyUpEvent );
			}
		}

		// Post original key events
		NSString *pChars = [mpLastKeyDownEvent charactersIgnoringModifiers];
		if ( pChars && [pChars length] )
		{
			NSUInteger nModifiers = ( [mpLastKeyDownEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask ) | nMouseMask;

			NSUInteger i = 0;
			NSUInteger nLength = [pChars length];
			for ( ; i < nLength; i++ )
			{
				USHORT nChar = (USHORT)[pChars characterAtIndex:i];
				USHORT nCode = GetKeyCode( [mpLastKeyDownEvent keyCode], nChar ) | GetEventCode( nModifiers );

				SalKeyEvent *pKeyDownEvent = new SalKeyEvent();
				pKeyDownEvent->mnTime = (ULONG)( JavaSalEventQueue::getLastNativeEventTime() * 1000 );
				pKeyDownEvent->mnCode = nCode;
				pKeyDownEvent->mnCharCode = nChar;
				pKeyDownEvent->mnRepeat = 0;

				SalKeyEvent *pKeyUpEvent = new SalKeyEvent();
				memcpy( pKeyUpEvent, pKeyDownEvent, sizeof( SalKeyEvent ) );

				JavaSalEvent *pSalKeyUpEvent = new JavaSalEvent( SALEVENT_KEYINPUT, mpFrame, pKeyDownEvent );
				if ( pKeyBindingDownEvent )
					pKeyBindingDownEvent->addOriginalKeyEvent( pSalKeyUpEvent );
				else
					JavaSalEventQueue::postCachedEvent( pSalKeyUpEvent );
				pSalKeyUpEvent->release();

				JavaSalEvent *pSalExtraKeyUpEvent = new JavaSalEvent( SALEVENT_KEYUP, mpFrame, pKeyUpEvent );
				if ( pKeyBindingUpEvent )
					pKeyBindingUpEvent->addOriginalKeyEvent( pSalExtraKeyUpEvent );
				else
					JavaSalEventQueue::postCachedEvent( pSalExtraKeyUpEvent );
				pSalExtraKeyUpEvent->release();
			}
		}

		if ( pKeyBindingDownEvent )
		{
			JavaSalEventQueue::postCachedEvent( pKeyBindingDownEvent );
			pKeyBindingDownEvent->release();
		}
		if ( pKeyBindingUpEvent )
		{
			JavaSalEventQueue::postCachedEvent( pKeyBindingUpEvent );
			pKeyBindingUpEvent->release();
		}
	}
}

- (void)insertText:(id)aString
{
	[self insertText:aString replacementRange:NSMakeRange( NSNotFound, 0 )];
}

- (void)setFrame:(JavaSalFrame *)pFrame
{
	mpFrame = pFrame;
}

- (void)concludeDragOperation:(id < NSDraggingInfo >)pSender
{
	id pDelegate = [self draggingDestinationDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(concludeDragOperation:)])
		[pDelegate concludeDragOperation:pSender];
}

- (void)dragImage:(NSImage *)pImage at:(NSPoint)aImageLocation offset:(NSSize)aMouseOffset event:(NSEvent *)pEvent pasteboard:(NSPasteboard *)pPasteboard source:(id)pSourceObject slideBack:(MacOSBOOL)bSlideBack
{
	// Fix bug 3652 by locking the application mutex and never letting it get
	// released during a native drag session. This prevents drag events from
	// getting dispatched out of order when we release and reacquire the mutex.
	if ( VCLInstance_setDragLock( YES ) )
	{
		if ( [super respondsToSelector:@selector(poseAsDragImage:at:offset:event:pasteboard:source:slideBack:)] )
			[super poseAsDragImage:pImage at:aImageLocation offset:aMouseOffset event:pEvent pasteboard:pPasteboard source:pSourceObject slideBack:bSlideBack];
		VCLInstance_setDragLock( NO );
	}
}

- (void)draggedImage:(NSImage *)pImage beganAt:(NSPoint)aPoint
{
	id pDelegate = [self draggingSourceDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(draggedImage:beganAt:)])
		[pDelegate draggedImage:pImage beganAt:aPoint];
}

- (void)draggedImage:(NSImage *)pImage endedAt:(NSPoint)aPoint operation:(NSDragOperation)nOperation
{
	id pDelegate = [self draggingSourceDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(draggedImage:endedAt:operation:)])
		[pDelegate draggedImage:pImage endedAt:aPoint operation:nOperation];
}

- (void)draggedImage:(NSImage *)pImage movedTo:(NSPoint)aPoint
{
	id pDelegate = [self draggingSourceDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(draggedImage:movedTo:)])
		[pDelegate draggedImage:pImage movedTo:aPoint];
}

- (id)draggingDestinationDelegate
{
	id pRet = nil;

	NSNumber *pKey = [NSNumber numberWithUnsignedLong:(unsigned long)self];
	if ( pKey && pDraggingDestinationDelegates )
	{
		// If the dragging is occurring while a native modal window or sheet
		// is visible, ignore all drag destination events to prevent deadlock
		id pDelegate = [pDraggingDestinationDelegates objectForKey:pKey];
		if ( pDelegate && !NSApplication_getModalWindow() )
			pRet = pDelegate;
	}

	return pRet;
}

- (void)draggingEnded:(id < NSDraggingInfo >)pSender
{
	id pDelegate = [self draggingDestinationDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(draggingEnded:)])
		[pDelegate draggingEnded:pSender];

	// Redispatch current mouse event as native dragging will not forward such
	// events to the application windows
	NSApplication *pApp = [NSApplication sharedApplication];
	NSWindow *pWindow = [self window];
	if ( pApp && pWindow )
	{
		NSEvent *pEvent = [pApp currentEvent];
		if ( pEvent )
		{
			NSEventType nType = [pEvent type];
			if ( ( nType >= NSLeftMouseDown && nType <= NSMouseExited ) || ( nType >= NSOtherMouseDown && nType <= NSOtherMouseDragged ) )
				[pWindow sendEvent:pEvent];
		}
	}
}

- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)pSender
{
	NSDragOperation nRet = NSDragOperationNone;

	id pDelegate = [self draggingDestinationDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(draggingEntered:)])
		nRet = [pDelegate draggingEntered:pSender];

	// Redispatch current mouse event as native dragging will not forward such
	// events to the application windows
	NSApplication *pApp = [NSApplication sharedApplication];
	NSWindow *pWindow = [self window];
	if ( pApp && pWindow )
	{
		NSEvent *pEvent = [pApp currentEvent];
		if ( pEvent )
		{
			NSEventType nType = [pEvent type];
			if ( ( nType >= NSLeftMouseDown && nType <= NSMouseExited ) || ( nType >= NSOtherMouseDown && nType <= NSOtherMouseDragged ) )
				[pWindow sendEvent:pEvent];
		}
	}

	return nRet;
}

- (void)draggingExited:(id < NSDraggingInfo >)pSender
{
	id pDelegate = [self draggingDestinationDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(draggingExited:)])
		[pDelegate draggingExited:pSender];

	// Redispatch current mouse event as native dragging will not forward such
	// events to the application windows
	NSApplication *pApp = [NSApplication sharedApplication];
	NSWindow *pWindow = [self window];
	if ( pApp && pWindow )
	{
		NSEvent *pEvent = [pApp currentEvent];
		if ( pEvent )
		{
			NSEventType nType = [pEvent type];
			if ( ( nType >= NSLeftMouseDown && nType <= NSMouseExited ) || ( nType >= NSOtherMouseDown && nType <= NSOtherMouseDragged ) )
				[pWindow sendEvent:pEvent];
		}
	}
}

- (id)draggingSourceDelegate
{
	id pRet = nil;

	NSNumber *pKey = [NSNumber numberWithUnsignedLong:(unsigned long)self];
	if ( pKey && pDraggingSourceDelegates )
	{
		id pDelegate = [pDraggingSourceDelegates objectForKey:pKey];
		if ( pDelegate )
			pRet = pDelegate;
	}

	return pRet;
}

- (NSDragOperation)draggingSourceOperationMaskForLocal:(MacOSBOOL)bLocal
{
	id pDelegate = [self draggingSourceDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(draggingSourceOperationMaskForLocal:)])
		return [pDelegate draggingSourceOperationMaskForLocal:bLocal];
	else
		return NSDragOperationNone;
}

- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)pSender
{
	NSDragOperation nRet = NSDragOperationNone;

	id pDelegate = [self draggingDestinationDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(draggingUpdated:)])
		nRet = [pDelegate draggingUpdated:pSender];

	// Redispatch current mouse event as native dragging will not forward such
	// events to the application windows
	NSApplication *pApp = [NSApplication sharedApplication];
	NSWindow *pWindow = [self window];
	if ( pApp && pWindow )
	{
		NSEvent *pEvent = [pApp currentEvent];
		if ( pEvent )
		{
			NSEventType nType = [pEvent type];
			if ( ( nType >= NSLeftMouseDown && nType <= NSMouseExited ) || ( nType >= NSOtherMouseDown && nType <= NSOtherMouseDragged ) )
				[pWindow sendEvent:pEvent];
		}
	}

	return nRet;
}

- (void)drawRect:(NSRect)aDirtyRect
{
	NSWindow *pWindow = [self window];
	if ( pWindow && [pWindow isVisible] && [self isKindOfClass:[VCLView class]] )
	{
		[pWindow disableFlushWindow];
		JavaSalFrame_drawToNSView( self, aDirtyRect );
		[pWindow enableFlushWindow];
	}
}

- (void)resetCursorRects
{
	NSWindow *pWindow = [self window];
	if ( pWindow && [pWindow isVisible] && [self isKindOfClass:[VCLView class]] )
	{
		NSCursor *pCursor = JavaSalFrame_getCursor( self );
		if ( pCursor )
			[self addCursorRect:[self visibleRect] cursor:pCursor];
	}
}

- (MacOSBOOL)ignoreModifierKeysWhileDragging
{
	id pDelegate = [self draggingSourceDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(ignoreModifierKeysWhileDragging)])
		return [pDelegate ignoreModifierKeysWhileDragging];
	else
		return NO;
}

- (id)initWithFrame:(NSRect)aFrame
{
	[super initWithFrame:aFrame];

	mpFrame = NULL;
	mpInputManager = nil;
	mpLastKeyDownEvent = nil;
	mpPendingKeyUpEvent = NULL;
	maSelectedRange = NSMakeRange( NSNotFound, 0 );
	mpTextInput = nil;
	maTextInputRange = NSMakeRange( NSNotFound, 0 );

	return self;
}

- (MacOSBOOL)isOpaque
{
	return YES;
}

- (NSArray *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)pDropDestination
{
	id pDelegate = [self draggingSourceDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(namesOfPromisedFilesDroppedAtDestination:)])
		return [pDelegate namesOfPromisedFilesDroppedAtDestination:pDropDestination];
	else
		return nil;
}

- (MacOSBOOL)performDragOperation:(id < NSDraggingInfo >)pSender
{
	if ( pSender )
	{
		NSPasteboard *pPasteboard = [pSender draggingPasteboard];
		if ( pPasteboard )
			Application_cacheSecurityScopedURL( [NSURL URLFromPasteboard:pPasteboard] );
	}

	id pDelegate = [self draggingDestinationDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(performDragOperation:)])
		return [pDelegate performDragOperation:pSender];
	else
		return NO;
}

- (MacOSBOOL)prepareForDragOperation:(id < NSDraggingInfo >)pSender
{
	id pDelegate = [self draggingDestinationDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(prepareForDragOperation:)])
		return [pDelegate prepareForDragOperation:pSender];
	else
		return NO;
}

- (MacOSBOOL)readSelectionFromPasteboard:(NSPasteboard *)pPasteboard
{
	MacOSBOOL bRet = NO;

	NSWindow *pWindow = [self window];
	if ( pPasteboard && pWindow && [pWindow isVisible] )
	{
		NSArray *pTypes = [pPasteboard types];
		if ( pTypes && [pTypes count] )
		{
			NSPasteboard *pGeneralPasteboard = [NSPasteboard generalPasteboard];
			if ( pGeneralPasteboard )
			{
				[pGeneralPasteboard declareTypes:pTypes owner:nil];

				unsigned int nCount = [pTypes count];
				unsigned int i = 0;
				for ( ; i < nCount; i++ )
				{
					NSString *pType = (NSString *)[pTypes objectAtIndex:i];
					if ( pType )
					{
						NSData *pData = [pPasteboard dataForType:pType];
						if ( pData )
							[pGeneralPasteboard setData:pData forType:pType];
					}
				}
				
				bRet = VCLEventQueue_paste( pWindow );
			}
		}
	}

	return bRet;
}

- (void)setDraggingDestinationDelegate:(id)pDelegate
{
	if ( !pDraggingDestinationDelegates )
	{
		pDraggingDestinationDelegates = [NSMutableDictionary dictionaryWithCapacity:10];
		if ( pDraggingDestinationDelegates )
			[pDraggingDestinationDelegates retain];
	}

	if ( pDraggingDestinationDelegates )
	{
		NSNumber *pKey = [NSNumber numberWithUnsignedLong:(unsigned long)self];
		if ( pKey )
		{
			if ( pDelegate )
				[pDraggingDestinationDelegates setObject:pDelegate forKey:pKey];
			else
				[pDraggingDestinationDelegates removeObjectForKey:pKey];
		}
	}
}

- (void)setDraggingSourceDelegate:(id)pDelegate
{
	if ( !pDraggingDestinationDelegates )
	{
		pDraggingSourceDelegates = [NSMutableDictionary dictionaryWithCapacity:10];
		if ( pDraggingSourceDelegates )
			[pDraggingSourceDelegates retain];
	}

	if ( pDraggingSourceDelegates )
	{
		NSNumber *pKey = [NSNumber numberWithUnsignedLong:(unsigned long)self];
		if ( pKey )
		{
			if ( pDelegate )
				[pDraggingSourceDelegates setObject:pDelegate forKey:pKey];
			else
				[pDraggingSourceDelegates removeObjectForKey:pKey];
		}
	}
}

- (id)validRequestorForSendType:(NSString *)pSendType returnType:(NSString *)pReturnType
{
	NSWindow *pWindow = [self window];
	if ( pWindow && [pWindow isVisible] && pSharedResponder && ![pSharedResponder disableServicesMenu] && pSendType && ( !pReturnType || [pReturnType isEqual:NSRTFPboardType] || [pReturnType isEqual:NSStringPboardType] ) )
	{
		if ( [pSendType isEqual:NSRTFPboardType] )
		{
			if ( aRTFSelection )
			{
				CFRelease( aRTFSelection );
				aRTFSelection = nil;
			}

			VCLEventQueue_getTextSelection( pWindow, NULL, &aRTFSelection );
			if ( aRTFSelection )
				return self;
		}
		else if ( [pSendType isEqual:NSStringPboardType] )
		{
			if ( aTextSelection )
			{
				CFRelease( aTextSelection );
				aTextSelection = nil;
			}

			VCLEventQueue_getTextSelection( pWindow, &aTextSelection, NULL );
			if ( aTextSelection )
				return self;
		}
	}

	return nil;
}

- (MacOSBOOL)wantsPeriodicDraggingUpdates
{
	id pDelegate = [self draggingDestinationDelegate];
	// Ignore the delegate's selector as always returns NO and we need to return
	// YES for OOo cursor state to keep in sync with the native pointer position
	if ( pDelegate && [self isKindOfClass:[VCLView class]] )
		return YES;
	else
		return NO;
}

- (MacOSBOOL)writeSelectionToPasteboard:(NSPasteboard *)pPasteboard types:(NSArray *)pTypes
{
	MacOSBOOL bRet = NO;

	if ( pPasteboard && pTypes )
	{
		NSMutableArray *pTypesDeclared = [NSMutableArray arrayWithCapacity:2];
		if ( pTypesDeclared )
		{
			if ( aRTFSelection && [pTypes containsObject:NSRTFPboardType] )
				[pTypesDeclared addObject:NSRTFPboardType];
			if ( aTextSelection && [pTypes containsObject:NSStringPboardType] )
				[pTypesDeclared addObject:NSStringPboardType];

			[pPasteboard declareTypes:pTypesDeclared owner:nil];
			if ( [pTypesDeclared count] )
			{
				if ( aRTFSelection && [pTypesDeclared containsObject:NSRTFPboardType] && [pPasteboard setData:(NSData *)aRTFSelection forType:NSRTFPboardType] )
					bRet = YES;
				if ( aTextSelection && [pTypesDeclared containsObject:NSStringPboardType] && [pPasteboard setString:(NSString *)aTextSelection forType:NSStringPboardType] )
					bRet = YES;
			}
		}

		if ( aTextSelection )
		{
			CFRelease( aTextSelection );
			aTextSelection = nil;
		}

		if ( aRTFSelection )
		{
			CFRelease( aRTFSelection );
			aRTFSelection = nil;
		}
	}

	return bRet;
}

@end

@interface NSObject (VCLObjectPoseAs)
- (void)poseAsPerformSelectorOnMainThread:(SEL)aSelector withObject:(id)aArg waitUntilDone:(MacOSBOOL)bWait modes:(NSArray *)pModes;
@end

@interface VCLObject : NSObject
- (void)performSelectorOnMainThread:(SEL)aSelector withObject:(id)aArg waitUntilDone:(MacOSBOOL)bWait modes:(NSArray *)pModes;
@end

@implementation VCLObject

- (void)performSelectorOnMainThread:(SEL)aSelector withObject:(id)aArg waitUntilDone:(MacOSBOOL)bWait modes:(NSArray *)pModes
{
	if ( [super respondsToSelector:@selector(poseAsPerformSelectorOnMainThread:withObject:waitUntilDone:modes:)] )
	{
		// Fix hanging when opening a new window in full screen mode while
		// running on OS X 10.11 by releasing the application mutex if we are
		// waiting until done
		ULONG nCount = bWait ? Application::ReleaseSolarMutex() : 0;

		@try
		{
			[super poseAsPerformSelectorOnMainThread:aSelector withObject:aArg waitUntilDone:bWait modes:pModes];
		}
		@catch ( NSException *pExc )
		{
		}

		Application::AcquireSolarMutex( nCount );
	}
}

@end

static MacOSBOOL bVCLEventQueueClassesInitialized = NO;

@interface InstallVCLEventQueueClasses : NSObject
{
}
+ (id)create;
- (void)installVCLEventQueueClasses:(id)pObject;
@end

@implementation InstallVCLEventQueueClasses

+ (id)create
{
	InstallVCLEventQueueClasses *pRet = [[InstallVCLEventQueueClasses alloc] init];
	[pRet autorelease];
	return pRet;
}

- (void)installVCLEventQueueClasses:(id)pObject
{
	if ( bVCLEventQueueClassesInitialized )
		return;

	bVCLEventQueueClassesInitialized = YES;

	// Do not retain as invoking alloc disables autorelease
	pSharedResponder = [[VCLResponder alloc] init];

	// VCLWindow selectors

	SEL aSelector = @selector(becomeKeyWindow);
	SEL aPoseAsSelector = @selector(poseAsBecomeKeyWindow);
	Method aOldMethod = class_getInstanceMethod( [NSWindow class], aSelector );
	Method aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSWindow class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	aSelector = @selector(displayIfNeeded);
	aPoseAsSelector = @selector(poseAsDisplayIfNeeded);
	aOldMethod = class_getInstanceMethod( [NSWindow class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSWindow class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	aSelector = @selector(initWithContentRect:styleMask:backing:defer:);
	aPoseAsSelector = @selector(poseAsInitWithContentRect:styleMask:backing:defer:);
	aOldMethod = class_getInstanceMethod( [NSWindow class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSWindow class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	aSelector = @selector(initWithContentRect:styleMask:backing:defer:screen:);
	aPoseAsSelector = @selector(poseAsInitWithContentRect:styleMask:backing:defer:screen:);
	aOldMethod = class_getInstanceMethod( [NSWindow class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSWindow class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	aSelector = @selector(makeFirstResponder:);
	aPoseAsSelector = @selector(poseAsMakeFirstResponder:);
	aOldMethod = class_getInstanceMethod( [NSWindow class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSWindow class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	aSelector = @selector(makeKeyWindow);
	aPoseAsSelector = @selector(poseAsMakeKeyWindow);
	aOldMethod = class_getInstanceMethod( [NSWindow class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSWindow class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	aSelector = @selector(orderWindow:relativeTo:);
	aPoseAsSelector = @selector(poseAsOrderWindow:relativeTo:);
	aOldMethod = class_getInstanceMethod( [NSWindow class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSWindow class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	aSelector = @selector(performKeyEquivalent:);
	aPoseAsSelector = @selector(poseAsPerformKeyEquivalent:);
	aOldMethod = class_getInstanceMethod( [NSWindow class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSWindow class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	aSelector = @selector(resignKeyWindow);
	aPoseAsSelector = @selector(poseAsResignKeyWindow);
	aOldMethod = class_getInstanceMethod( [NSWindow class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSWindow class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	aSelector = @selector(sendEvent:);
	aPoseAsSelector = @selector(poseAsSendEvent:);
	aOldMethod = class_getInstanceMethod( [NSWindow class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSWindow class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	// VCLView selectors

	aSelector = @selector(dragImage:at:offset:event:pasteboard:source:slideBack:);
	aPoseAsSelector = @selector(poseAsDragImage:at:offset:event:pasteboard:source:slideBack:);
	aOldMethod = class_getInstanceMethod( [NSView class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLView class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSView class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	// VCLWindow selectors

	aSelector = @selector(draggingSourceDelegate);
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aNewMethod )
	{
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aNewIMP )
			class_addMethod( [NSWindow class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
	}

	aSelector = @selector(setDraggingSourceDelegate:);
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aNewMethod )
	{
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aNewIMP )
			class_addMethod( [NSWindow class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
	}

	aSelector = @selector(windowWillEnterFullScreen:);
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aNewMethod )
	{
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aNewIMP )
			class_addMethod( [NSWindow class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
	}

	aSelector = @selector(windowDidExitFullScreen:);
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aNewMethod )
	{
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aNewIMP )
			class_addMethod( [NSWindow class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
	}

	aSelector = @selector(windowDidMove:);
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aNewMethod )
	{
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aNewIMP )
			class_addMethod( [NSWindow class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
	}

	aSelector = @selector(windowDidResize:);
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aNewMethod )
	{
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aNewIMP )
			class_addMethod( [NSWindow class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
	}

	aSelector = @selector(windowDidDeminiaturize:);
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aNewMethod )
	{
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aNewIMP )
			class_addMethod( [NSWindow class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
	}

	aSelector = @selector(windowDidMiniaturize:);
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aNewMethod )
	{
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aNewIMP )
			class_addMethod( [NSWindow class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
	}

	aSelector = @selector(windowShouldClose:);
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aNewMethod )
	{
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aNewIMP )
			class_addMethod( [NSWindow class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
	}

	aSelector = @selector(windowShouldZoom:toFrame:);
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aNewMethod )
	{
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aNewIMP )
			class_addMethod( [NSWindow class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
	}

	aSelector = @selector(windowWillEnterVersionBrowser:);
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aNewMethod )
	{
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aNewIMP )
			class_addMethod( [NSWindow class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
	}

	aSelector = @selector(windowDidExitVersionBrowser:);
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aNewMethod )
	{
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aNewIMP )
			class_addMethod( [NSWindow class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
	}

	// VCLObject selectors

	aSelector = @selector(performSelectorOnMainThread:withObject:waitUntilDone:modes:);
	aPoseAsSelector = @selector(poseAsPerformSelectorOnMainThread:withObject:waitUntilDone:modes:);
	aOldMethod = class_getInstanceMethod( [NSObject class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLObject class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSObject class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	NSApplication *pApp = [NSApplication sharedApplication];
	VCLApplicationDelegate *pSharedDelegate = [VCLApplicationDelegate sharedDelegate];
	if ( pApp && pSharedDelegate )
	{
		[pApp setDelegate:pSharedDelegate];

		NSMenu *pMainMenu = [pApp mainMenu];
		if ( pMainMenu && [pMainMenu numberOfItems] > 0 )
		{
			NSMenuItem *pItem = [pMainMenu itemAtIndex:0];
			if ( pItem )
			{
				NSMenu *pSubmenu = [pItem submenu];
				if ( pSubmenu )
					[pSubmenu setDelegate:pSharedDelegate];
			}
		}
	}
}

@end

sal_Bool NSApplication_isActive()
{
	sal_Bool bRet = sal_True;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	IsApplicationActive *pIsApplicationActive = [IsApplicationActive create];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pIsApplicationActive performSelectorOnMainThread:@selector(isApplicationActive:) withObject:pIsApplicationActive waitUntilDone:YES modes:pModes];
	bRet = (sal_Bool)[pIsApplicationActive isActive];

	[pPool release];

	return bRet;
}

@interface CancelTermination : NSObject
+ (id)create;
- (void)cancelTermination:(id)pObject;
@end

@implementation CancelTermination

+ (id)create
{
	CancelTermination *pRet = [[CancelTermination alloc] init];
	[pRet autorelease];
	return pRet;
}

- (void)cancelTermination:(id)pObject;
{
	VCLApplicationDelegate *pDelegate = [VCLApplicationDelegate sharedDelegate];
	if ( pDelegate )
		[pDelegate cancelTermination];
}
@end

MacOSBOOL NSWindow_hasMarkedText( NSWindow *pWindow )
{
	MacOSBOOL bRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( !pWindow )
	{
		NSApplication *pApp = [NSApplication sharedApplication];
		if ( pApp )
			pWindow = [pApp keyWindow];
	}

	if ( pWindow )
	{
		NSResponder *pResponder = [pWindow firstResponder];
		if ( pResponder && [pResponder isKindOfClass:[VCLView class]] && [(VCLView *)pResponder hasMarkedText] )
			bRet = YES;
	}

	[pPool release];

	return bRet;
}

void VCLEventQueue_cancelTermination()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	CancelTermination *pCancelTermination = [CancelTermination create];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pCancelTermination performSelectorOnMainThread:@selector(cancelTermination:) withObject:pCancelTermination waitUntilDone:YES modes:pModes];

	[pPool release];
}

void VCLEventQueue_installVCLEventQueueClasses()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	InstallVCLEventQueueClasses *pInstallVCLEventQueueClasses = [InstallVCLEventQueueClasses create];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pInstallVCLEventQueueClasses performSelectorOnMainThread:@selector(installVCLEventQueueClasses:) withObject:pInstallVCLEventQueueClasses waitUntilDone:YES modes:pModes];

	[pPool release];
}
