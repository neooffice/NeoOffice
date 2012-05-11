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

#include <dlfcn.h>

#include <premac.h>
#import <Cocoa/Cocoa.h>
// Need to include for virtual key constants but we don't link to it
#import <Carbon/Carbon.h>
#import <objc/objc-class.h>
#include <postmac.h>

#include "VCLApplicationDelegate_cocoa.h"
#include "VCLEventQueue_cocoa.h"
#include "VCLFont_cocoa.h"
#include "VCLFrame_cocoa.h"
#include "VCLGraphics_cocoa.h"
#include "VCLResponder_cocoa.h"
#include "../app/salinst_cocoa.h"

#ifndef NSEventTypeMagnify
#define NSEventTypeMagnify 30
#endif	// !NSEventTypeMagnify

#ifndef NSEventTypeSwipe
#define NSEventTypeSwipe 31
#endif	// !NSEventTypeSwipe

#ifdef USE_NATIVE_EVENTS
#define MODIFIER_RELEASE_INTERVAL 100
#endif	// USE_NATIVE_EVENTS

typedef OSErr Gestalt_Type( OSType selector, long *response );

static MacOSBOOL bFontManagerLocked = NO;
static NSRecursiveLock *pFontManagerLock = nil;
#ifdef USE_NATIVE_WINDOW
static NSString *pCMenuBarString = @"CMenuBar";
#endif	// USE_NATIVE_WINDOW
static NSString *pCocoaAppWindowString = @"CocoaAppWindow";
#ifndef USE_NATIVE_EVENTS
static NSString *pAWTFontString = @"AWTFont";
static NSString *pNSViewAWTString = @"NSViewAWT";
static NSString *pNSWindowViewAWTString = @"NSWindowViewAWT";
#endif	// !USE_NATIVE_EVENTS
#if !defined USE_NATIVE_EVENTS && !defined USE_ROUNDED_BOTTOM_CORNERS_IN_JAVA_FRAMES
static NSString *pNSThemeFrameString = @"NSThemeFrame";
#endif	// !USE_NATIVE_EVENTS && !USE_ROUNDED_BOTTOM_CORNERS_IN_JAVA_FRAMES

inline long Float32ToLong( Float32 f ) { return (long)( f == 0 ? f : f < 0 ? f - 1.0 : f + 1.0 ); }

static MacOSBOOL EventMatchesShortcutKey( NSEvent *pEvent, unsigned int nKey )
{
	MacOSBOOL bRet = NO;

	if ( !pEvent || [pEvent type] != NSKeyDown )
		return bRet;

	CFPreferencesAppSynchronize( CFSTR( "com.apple.symbolichotkeys" ) );
	CFPropertyListRef pPref = CFPreferencesCopyAppValue( CFSTR( "AppleSymbolicHotKeys" ), CFSTR( "com.apple.symbolichotkeys" ) );
	if ( pPref )
	{
		if ( CFGetTypeID( pPref ) == CFDictionaryGetTypeID() )
		{
			NSString *pKey = [[NSNumber numberWithUnsignedInt:nKey] stringValue];
			if ( pKey )
			{
				NSDictionary *pDict = (NSDictionary *)[(NSDictionary *)pPref objectForKey:pKey];
				if ( pDict && CFGetTypeID( pDict ) == CFDictionaryGetTypeID() )
				{
					NSNumber *pEnabled = (NSNumber *)[pDict valueForKey:@"enabled"];
					if ( pEnabled && [pEnabled intValue] )
					{
						NSDictionary *pValue = (NSDictionary *)[pDict objectForKey:@"value"];
						if ( pValue && CFGetTypeID( pValue ) == CFDictionaryGetTypeID() )
						{
							NSArray *pParams = (NSArray *)[pValue objectForKey:@"parameters"];
							if ( pParams && CFGetTypeID( pParams ) == CFArrayGetTypeID() && [pParams count] > 2 )
							{
								NSString *pChars = nil;
								NSNumber *pChar = (NSNumber *)[pParams objectAtIndex:0];
								if ( pChar )
								{
									unichar nChar = [pChar unsignedCharValue];
									pChars = [NSString stringWithCharacters:&nChar length:1];
								}

								NSNumber *pKeyCode = (NSNumber *)[pParams objectAtIndex:1];
								if ( ( pChars && [pChars isEqualToString:[pEvent characters]] ) || ( pKeyCode && [pKeyCode unsignedShortValue] == [pEvent keyCode] ) )
								{
									NSNumber *pModifiers = (NSNumber *)[pParams objectAtIndex:2];
									if ( pModifiers && ( [pModifiers unsignedIntValue] & ( NSCommandKeyMask | NSShiftKeyMask ) ) == ( (unsigned int)[pEvent modifierFlags] & ( NSCommandKeyMask | NSShiftKeyMask ) ) )
										bRet = YES;
								}
							}
						}
					}
				}
			}
		}

		CFRelease( pPref );
	}

	if ( !bRet )
	{
		CFPreferencesAppSynchronize( CFSTR( "com.apple.universalaccess" ) );
		pPref = CFPreferencesCopyAppValue( CFSTR( "UserAssignableHotKeys" ), CFSTR( "com.apple.universalaccess" ) );
		if ( pPref )
		{
			if ( CFGetTypeID( pPref ) == CFArrayGetTypeID() )
			{
				CFIndex nCount = CFArrayGetCount( (CFArrayRef)pPref );
				CFIndex i = 0;
				for ( ; i < nCount; i++ ) {
					NSDictionary *pDict = (NSDictionary *)CFArrayGetValueAtIndex( (CFArrayRef)pPref, i );
					if ( pDict && CFGetTypeID( pDict ) == CFDictionaryGetTypeID() )
					{
						// Note that Apple uses an odd spelling for this key
						NSNumber *pSybmolicHotKey = (NSNumber *)[pDict valueForKey:@"sybmolichotkey"];
						if ( pSybmolicHotKey && [pSybmolicHotKey unsignedIntValue] == nKey )
						{
							NSNumber *pEnabled = (NSNumber *)[pDict valueForKey:@"enabled"];
							if ( pEnabled && [pEnabled intValue] )
							{
								NSNumber *pKeyCode = (NSNumber *)[pDict valueForKey:@"key"];
								if ( pKeyCode && [pKeyCode unsignedShortValue] == [pEvent keyCode] )
								{
									NSNumber *pModifiers = (NSNumber *)[pDict valueForKey:@"modifier"];
									if ( pModifiers && ( [pModifiers unsignedIntValue]  & ~NSHelpKeyMask & ~NSFunctionKeyMask & NSDeviceIndependentModifierFlagsMask ) == ( (unsigned int)[pEvent modifierFlags] & ~NSHelpKeyMask & ~NSFunctionKeyMask & NSDeviceIndependentModifierFlagsMask ) )
{
										bRet = YES;
}
								}
							}

							break;
						}
					}
				}
			}

			CFRelease( pPref );
		}
	}

	return bRet;
}

#ifdef USE_NATIVE_EVENTS

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

#endif	// USE_NATIVE_EVENTS

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
#ifdef USE_NATIVE_EVENTS
		mbActive = ( [pApp isActive] && ![pApp modalWindow] && ( ![pApp keyWindow] || [[pApp keyWindow] isKindOfClass:[VCLPanel class]] || [[pApp keyWindow] isKindOfClass:[VCLWindow class]] ) );
#else	// USE_NATIVE_EVENTS
		mbActive = ( [pApp isActive] && ![pApp modalWindow] && ( ![pApp keyWindow] || [[[pApp keyWindow] className] isEqualToString:pCocoaAppWindowString] ) );
#endif	// USE_NATIVE_EVENTS
}

@end

@interface NSApplication (VCLApplicationPoseAs)
- (void)poseAsSetDelegate:(id)pDelegate;
@end

@interface VCLApplication : NSApplication
- (void)setDelegate:(id)pDelegate;
@end

@implementation VCLApplication

- (void)setDelegate:(id)pDelegate
{
	if ( ![self delegate] )
	{
		VCLApplicationDelegate *pNewDelegate = [VCLApplicationDelegate sharedDelegate];
		if ( pNewDelegate )
		{
			[pNewDelegate setDelegate:pDelegate];
			// NSApplication does not retain delegates so don't release it
			pDelegate = pNewDelegate;
		}
	}

	if ( [super respondsToSelector:@selector(poseAsSetDelegate:)] )
		[super poseAsSetDelegate:pDelegate];
}

@end

@interface VCLBundle : NSBundle
+ (MacOSBOOL)loadNibFile:(NSString *)pFileName externalNameTable:(NSDictionary *)pContext withZone:(NSZone *)pZone;
+ (MacOSBOOL)poseAsLoadNibFile:(NSString *)pFileName externalNameTable:(NSDictionary *)pContext withZone:(NSZone *)pZone;
@end

@implementation VCLBundle

+ (MacOSBOOL)loadNibFile:(NSString *)pFileName externalNameTable:(NSDictionary *)pContext withZone:(NSZone *)pZone
{
	MacOSBOOL bRet = [VCLBundle poseAsLoadNibFile:pFileName externalNameTable:pContext withZone:pZone];

	// Fix bug 3563 by trying to load Java's English nib file if the requested
	// nib file is nil
	if ( !bRet && !pFileName )
		bRet = [VCLBundle poseAsLoadNibFile:@"/System/Library/Frameworks/JavaVM.framework/Versions/Current/Resources/English.lproj/DefaultApp.nib" externalNameTable:pContext withZone:pZone];

	return bRet;
}

+ (MacOSBOOL)poseAsLoadNibFile:(NSString *)pFileName externalNameTable:(NSDictionary *)pContext withZone:(NSZone *)pZone
{
	// This should never be executed and should be swizzled out to superclass
	return NO;
}

@end

// Fix for bugs 1685, 1694, and 1859. Java 1.5 and higher will arbitrarily
// change the selected font by creating a new font from the font's family
// name and style. We fix these bugs by prepending the font names to the
// list of font family names so that Java will think that each font's
// family name is the same as its font name.

@interface NSFontManager (VCLFontManagerPoseAs)
- (NSArray *)poseAsAvailableFontFamilies;
- (NSArray *)poseAsAvailableMembersOfFontFamily:(NSString *)family;
@end

@interface VCLFontManager : NSFontManager
- (NSArray *)availableFontFamilies;
- (NSArray *)availableMembersOfFontFamily:(NSString *)family;
@end

@implementation VCLFontManager

- (NSArray *)availableFontFamilies
{
	NSMutableArray *pRet = nil;

	if ( pFontManagerLock )
		[pFontManagerLock lock];

	if ( bFontManagerLocked )
	{
		if ( [super respondsToSelector:@selector(poseAsAvailableFontFamilies)] )
			pRet = [NSMutableArray arrayWithArray:[super poseAsAvailableFontFamilies]];
	}
	else
	{
		pRet = [NSMutableArray arrayWithArray:[super availableFonts]];
		if ( pRet && [super respondsToSelector:@selector(poseAsAvailableFontFamilies)] )
			[pRet addObjectsFromArray:[super poseAsAvailableFontFamilies]];
	}

	if ( pFontManagerLock )
		[pFontManagerLock unlock];

	return pRet;
}

- (NSArray *)availableMembersOfFontFamily:(NSString *)family
{
	NSArray *pRet = nil;

	if ( pFontManagerLock )
		[pFontManagerLock lock];

	if ( bFontManagerLocked )
	{
		if ( [super respondsToSelector:@selector(poseAsAvailableMembersOfFontFamily:)] )
			pRet = [super poseAsAvailableMembersOfFontFamily:family];
	}
	else
	{
		NSFont *pNSFont = [NSFont fontWithName:family size:12];
		if ( pNSFont )
		{
			NSMutableArray *pFontEntries = [NSMutableArray arrayWithCapacity:4];
			if ( pFontEntries )
			{
				[pFontEntries addObject:[pNSFont fontName]];
				[pFontEntries addObject:@""];
				[pFontEntries addObject:[NSNumber numberWithInt:[self weightOfFont:pNSFont]]];
				[pFontEntries addObject:[NSNumber numberWithUnsignedInt:[self traitsOfFont:pNSFont]]];
				pRet = [NSArray arrayWithObject:pFontEntries];
			}
		}
	}

	if ( pFontManagerLock )
		[pFontManagerLock unlock];

	return pRet;
}

@end

#ifndef USE_NATIVE_EVENTS
static NSString *pCancelInputMethodText = @" ";
#endif	// !USE_NATIVE_EVENTS

@interface NSResponder (VCLResponder)
- (void)abandonInput;
- (void)copy:(id)pSender;
- (void)cut:(id)pSender;
#ifndef USE_NATIVE_EVENTS
- (MacOSBOOL)hasMarkedText;
#endif	// !USE_NATIVE_EVENTS
- (void)paste:(id)pSender;
@end

@interface NSView (VCLViewPoseAs)
- (void)poseAsDragImage:(NSImage *)pImage at:(NSPoint)aImageLocation offset:(NSSize)aMouseOffset event:(NSEvent *)pEvent pasteboard:(NSPasteboard *)pPasteboard source:(id)pSourceObject slideBack:(MacOSBOOL)bSlideBack;
- (id)poseAsInitWithFrame:(NSRect)aFrame;
- (MacOSBOOL)poseAsIsOpaque;
- (NSSize)poseAsBottomCornerSize;
@end

#ifndef USE_NATIVE_EVENTS
static MacOSBOOL bUseQuickTimeContentViewHack = NO;
#endif	// USE_NATIVE_EVENTS

#ifndef USE_NATIVE_EVENTS

// The QuickTime content view hack implemented in [VCLWindow setContentView:]
// break Java's Window.getLocationOnScreen() method so we need to flip the
// points returned by NSView's convertPoint selectors
@interface VCLWindowView : VCLView
{
}
- (NSPoint)convertPoint:(NSPoint)aPoint fromView:(NSView *)pView;
- (NSPoint)convertPoint:(NSPoint)aPoint toView:(NSView *)pView;
- (void)forwardInvocation:(NSInvocation *)pInvocation;
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector;
@end

@implementation VCLWindowView

- (NSPoint)convertPoint:(NSPoint)aPoint fromView:(NSView *)pView
{
	NSPoint aRet = [super convertPoint:aPoint fromView:pView];

	if ( !pView )
	{
		NSWindow *pWindow = [self window];
		if ( pWindow )
			aRet.y += [pWindow frame].size.height;
	}

	return aRet;
}

- (NSPoint)convertPoint:(NSPoint)aPoint toView:(NSView *)pView
{
	NSPoint aRet = [super convertPoint:aPoint toView:pView];

	if ( !pView )
	{
		NSWindow *pWindow = [self window];
		if ( pWindow )
			aRet.y += [pWindow frame].size.height;
	}

	return aRet;
}

- (void)forwardInvocation:(NSInvocation *)pInvocation
{
	MacOSBOOL bHandled = NO;

	SEL aSelector = [pInvocation selector];

	NSArray *pSubviews = [self subviews];
	if ( pSubviews && [pSubviews count] )
	{
		// There should only be one subview and it should be an instance
		// of NSWindowViewAWT
		NSView *pSubview = [pSubviews objectAtIndex:0];
		if ( pSubview )
		{
			if ( [pSubview respondsToSelector:aSelector] )
			{
				[pInvocation invokeWithTarget:pSubview];
				bHandled = YES;
			}
		}
	}

	if ( !bHandled )
		[self doesNotRecognizeSelector:aSelector];
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	NSArray *pSubviews = [self subviews];
	if ( pSubviews && [pSubviews count] )
	{
		// There should only be one subview and it should be an instance
		// of NSWindowViewAWT
		NSView *pSubview = [pSubviews objectAtIndex:0];
		if ( pSubview )
		{
			if ( [pSubview respondsToSelector:aSelector] )
				return [pSubview methodSignatureForSelector:aSelector];
		}
	}

	return nil;
}

@end

#endif	// !USE_NATIVE_EVENTS

#ifdef USE_NATIVE_WINDOW

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

#endif	 // USE_NATIVE_WINDOW

@implementation VCLPanel

#ifdef USE_NATIVE_EVENTS

- (MacOSBOOL)canBecomeKeyOrMainWindow
{
	return ( mbCanBecomeKeyOrMainWindow && ![self becomesKeyOnlyIfNeeded] );
}

- (void)setAllowKeyBindings:(MacOSBOOL)bAllowKeyBindings
{
	mbAllowKeyBindings = bAllowKeyBindings;
}

- (void)setCanBecomeKeyOrMainWindow:(MacOSBOOL)bCanBecomeKeyOrMainWindow
{
	mbCanBecomeKeyOrMainWindow = bCanBecomeKeyOrMainWindow;
}

- (void)setFrame:(JavaSalFrame *)pFrame
{
	mpFrame = pFrame;

	NSView *pContentView = [self contentView];
	if ( pContentView && [pContentView isKindOfClass:[VCLView class]] )
		[(VCLView *)pContentView setFrame:pFrame];
}

#endif	// USE_NATIVE_EVENTS

@end

@interface NSWindow (VCLWindowPoseAs)
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
- (void)poseAsSetContentView:(NSView *)pView;
- (void)poseAsSetLevel:(int)nWindowLevel;
- (void)poseAsSetBackgroundColor:(NSColor *)pColor;
@end

#ifndef USE_NATIVE_EVENTS

@interface VCLWindow (CocoaAppWindow)
- (jobject)peer;
@end

#endif	// !USE_NATIVE_EVENTS

@interface VCLWindow (ICAImageImport)
- (void)setStyleMask:(unsigned int)nStyleMask;
@end

static MacOSBOOL bAWTFontInitialized = NO;
static NSMutableDictionary *pDraggingDestinationDelegates = nil;
static NSMutableArray *pNeedRestoreModalWindows = nil;
static VCLResponder *pSharedResponder = nil;
static NSMutableDictionary *pDraggingSourceDelegates = nil;
#ifdef USE_NATIVE_EVENTS
static NSUInteger nMouseMask = 0;
#endif	// USE_NATIVE_EVENTS

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
#ifdef USE_NATIVE_EVENTS
				if ( pWindow && [pWindow level] == NSModalPanelWindowLevel && [pWindow respondsToSelector:@selector(_clearModalWindowLevel)] && ( [pWindow isKindOfClass:[VCLPanel class]] || [pWindow isKindOfClass:[VCLWindow class]] ) )
#else	// USE_NATIVE_EVENTS
				if ( pWindow && [pWindow level] == NSModalPanelWindowLevel && [pWindow respondsToSelector:@selector(_clearModalWindowLevel)] && [[pWindow className] isEqualToString:pCocoaAppWindowString] )
#endif	// USE_NATIVE_EVENTS
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
	// Load Java's AWTFont class and redirect them to VCLFont's matching
	// selectors
	if ( pWindow && !bAWTFontInitialized && [[pWindow className] isEqualToString:pCocoaAppWindowString] )
	{
		bAWTFontInitialized = YES;

		NSBundle *pBundle = [NSBundle bundleForClass:[pWindow class]];
		if ( pBundle )
		{
			Class aClass;
#ifndef USE_NATIVE_EVENTS
			// AWTFont selectors

			aClass = [pBundle classNamed:pAWTFontString];
			if ( aClass )
			{
				SEL aSelector = @selector(awtFontForName:style:isFakeItalic:);
				Method aOldMethod = class_getClassMethod( aClass, aSelector );
				Method aNewMethod = class_getClassMethod( [VCLFont class], aSelector );
				if ( aOldMethod && aNewMethod )
				{
					IMP aNewIMP = method_getImplementation( aNewMethod );
					if ( aNewIMP )
						method_setImplementation( aOldMethod, aNewIMP );
				}

				aSelector = @selector(nsFontForJavaFont:env:);
				aOldMethod = class_getClassMethod( aClass, aSelector );
				aNewMethod = class_getClassMethod( [VCLFont class], aSelector );
				if ( aOldMethod && aNewMethod )
				{
					IMP aNewIMP = method_getImplementation( aNewMethod );
					if ( aNewIMP )
						method_setImplementation( aOldMethod, aNewIMP );
				}

				aSelector = @selector(initWithFont:isFakeItalic:);
				aOldMethod = class_getInstanceMethod( aClass, aSelector );
				IMP aNewIMP = [[VCLFont class] instanceMethodForSelector:aSelector];
				if ( aOldMethod && aNewIMP )
					method_setImplementation( aOldMethod, aNewIMP );

				aSelector = @selector(dealloc);
				aOldMethod = class_getInstanceMethod( aClass, aSelector );
				aNewIMP = [[VCLFont class] instanceMethodForSelector:aSelector];
				if ( aOldMethod && aNewIMP )
					method_setImplementation( aOldMethod, aNewIMP );

				aSelector = @selector(finalize);
				aOldMethod = class_getInstanceMethod( aClass, aSelector );
				aNewIMP = [[VCLFont class] instanceMethodForSelector:aSelector];
				if ( aOldMethod && aNewIMP )
					method_setImplementation( aOldMethod, aNewIMP );
			}

#endif	// !USE_NATIVE_EVENTS
#ifdef USE_NATIVE_WINDOW
			// CMenuBar selectors

			aClass = [pBundle classNamed:pCMenuBarString];
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
#endif	// USE_NATIVE_WINDOW
		}
	}
}

- (void)becomeKeyWindow
{
	[VCLWindow restoreModalWindowLevel];

	if ( [super respondsToSelector:@selector(poseAsBecomeKeyWindow)] )
		[super poseAsBecomeKeyWindow];

	if ( [self isVisible] )
	{
#ifdef USE_NATIVE_EVENTS
		if ( ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) && mpFrame )
		{
			JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_GETFOCUS, mpFrame, NULL );
			JavaSalEventQueue::postCachedEvent( pEvent );
			pEvent->release();
		}
#else	// USE_NATIVE_EVENTS
		if ( [[self className] isEqualToString:pCocoaAppWindowString] )
		{
			// Fix bug 1819 by forcing cancellation of the input method
			NSResponder *pResponder = [self firstResponder];
			if ( pResponder && [pResponder respondsToSelector:@selector(abandonInput)] && [pResponder respondsToSelector:@selector(hasMarkedText)] && [pResponder respondsToSelector:@selector(insertText:)] )
			{
				if ( [pResponder hasMarkedText] )
					[pResponder insertText:pCancelInputMethodText];
				[pResponder abandonInput];
			}
		}
#endif	// USE_NATIVE_EVENTS
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

#ifdef USE_NATIVE_EVENTS

- (MacOSBOOL)canBecomeKeyOrMainWindow
{
	return mbCanBecomeKeyOrMainWindow;
}

#endif	// USE_NATIVE_EVENTS

- (void)displayIfNeeded
{
	// Fix bug 2151 by not allowing any updates if the window is hidden
#ifdef USE_NATIVE_EVENTS
	if ( ![self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
#else	// USE_NATIVE_EVENTS
	if ( ![self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] )
#endif	// USE_NATIVE_EVENTS
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

	if ( [super respondsToSelector:@selector(poseAsInitWithContentRect:styleMask:backing:defer:)] )
		[super poseAsInitWithContentRect:aContentRect styleMask:nStyle backing:nBufferingType defer:bDeferCreation];

#ifdef USE_NATIVE_EVENTS
	if ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] )
	{
		mbAllowKeyBindings = YES;
		mbCanBecomeKeyOrMainWindow = YES;
		mnIgnoreMouseReleasedModifiers = 0;
		mpFrame = NULL;
		mnLastMetaModifierReleasedTime = 0;

		[self setReleasedWhenClosed:NO];
		[self setDelegate:self];
		[self setAcceptsMouseMovedEvents:YES];
	}
#endif	// USE_NATIVE_EVENTS

	return self;
}

- (id)initWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nStyle backing:(NSBackingStoreType)nBufferingType defer:(MacOSBOOL)bDeferCreation screen:(NSScreen *)pScreen
{
	[VCLWindow swizzleSelectors:self];

	if ( [super respondsToSelector:@selector(poseAsInitWithContentRect:styleMask:backing:defer:screen:)] )
		[super poseAsInitWithContentRect:aContentRect styleMask:nStyle backing:nBufferingType defer:bDeferCreation screen:pScreen];

#ifdef USE_NATIVE_EVENTS
	if ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] )
	{
		mbAllowKeyBindings = YES;
		mbCanBecomeKeyOrMainWindow = YES;
		mnIgnoreMouseReleasedModifiers = 0;
		mpFrame = NULL;
		mnLastMetaModifierReleasedTime = 0;

		[self setReleasedWhenClosed:NO];
		[self setDelegate:self];
		[self setAcceptsMouseMovedEvents:YES];
	}
#endif	// USE_NATIVE_EVENTS

	return self;
}

- (MacOSBOOL)makeFirstResponder:(NSResponder *)pResponder
{
#ifndef USE_NATIVE_EVENTS
	NSResponder *pOldResponder = [self firstResponder];
#endif	// !USE_NATIVE_EVENTS

	MacOSBOOL bRet = NO;
	if ( [super respondsToSelector:@selector(poseAsMakeFirstResponder:)] )
		bRet = [super poseAsMakeFirstResponder:pResponder];

#ifndef USE_NATIVE_EVENTS
	// Fix bug 1819 by forcing cancellation of the input method
	if ( bRet && [self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] )
	{
		if ( pOldResponder && [pOldResponder respondsToSelector:@selector(abandonInput)] && [pOldResponder respondsToSelector:@selector(hasMarkedText)] && [pOldResponder respondsToSelector:@selector(insertText:)] )
		{
			if ( [pOldResponder hasMarkedText] )
				[pOldResponder insertText:pCancelInputMethodText];
			[pOldResponder abandonInput];
		}

		if ( pResponder && [pResponder respondsToSelector:@selector(abandonInput)] && [pResponder respondsToSelector:@selector(hasMarkedText)] && [pResponder respondsToSelector:@selector(insertText:)] )
		{
			if ( [pResponder hasMarkedText] )
				[pResponder insertText:pCancelInputMethodText];
			[pResponder abandonInput];
		}
	}
#endif	// !USE_NATIVE_EVENTS

	return bRet;
}

- (void)makeKeyWindow
{
#ifdef USE_NATIVE_EVENTS
	if ( !mbCanBecomeKeyOrMainWindow )
		return;
#endif	// USE_NATIVE_EVENTS

#ifdef USE_NATIVE_EVENTS
	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
#else	// USE_NATIVE_EVENTS
	if ( [self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] )
#endif	// USE_NATIVE_EVENTS
	{
		MacOSBOOL bTrackingMenuBar = false;
		VCLApplicationDelegate *pAppDelegate = [VCLApplicationDelegate sharedDelegate];
		if ( pAppDelegate )
			bTrackingMenuBar = [pAppDelegate isInTracking];

		// Fix bug 2992 by not allowing the key window to change when we are
		// tracking the menubar and never allow a borderless window to grab
		// focus
		if ( bTrackingMenuBar && ! ( [self styleMask] & NSTitledWindowMask ) )
		{
			return;
		}
		else if ( [self styleMask] & NSUtilityWindowMask )
		{
			// Do not allow utility windows to grab the focus except when the
			// user presses Control-F6
			NSApplication *pApp = [NSApplication sharedApplication];
			if ( pApp && !EventMatchesShortcutKey( [pApp currentEvent], 11 ) )
				return;
		}
	}

	if ( [super respondsToSelector:@selector(poseAsMakeKeyWindow)] )
		[super poseAsMakeKeyWindow];
}

- (void)orderWindow:(NSWindowOrderingMode)nOrderingMode relativeTo:(int)nOtherWindowNumber
{
#ifdef USE_NATIVE_FULL_SCREEN_MODE
#ifdef USE_NATIVE_EVENTS
	if ( nOrderingMode != NSWindowOut && ![self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
#else	// USE_NATIVE_EVENTS
	if ( nOrderingMode != NSWindowOut && ![self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] )
#endif	// USE_NATIVE_EVENTS
	{
		NSNotificationCenter *pNotificationCenter = [NSNotificationCenter defaultCenter];
		if ( pNotificationCenter )
		{
			[pNotificationCenter addObserver:self selector:@selector(windowDidExitFullScreen:) name:@"NSWindowDidExitFullScreenNotification" object:self];
			[pNotificationCenter addObserver:self selector:@selector(windowWillEnterFullScreen:) name:@"NSWindowWillEnterFullScreenNotification" object:self];
		}
	}
#ifdef USE_NATIVE_EVENTS
	else if ( nOrderingMode == NSWindowOut && [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
#else	// USE_NATIVE_EVENTS
	else if ( nOrderingMode == NSWindowOut && [self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] )
#endif	// USE_NATIVE_EVENTS
	{
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
			[pNotificationCenter removeObserver:self name:@"NSWindowDidEnterFullScreenNotification" object:self];
			[pNotificationCenter removeObserver:self name:@"NSWindowWillExitFullScreenNotification" object:self];
		}
	}
#endif	// USE_NATIVE_FULL_SCREEN_MODE

	if ( [super respondsToSelector:@selector(poseAsOrderWindow:relativeTo:)] )
		[super poseAsOrderWindow:nOrderingMode relativeTo:nOtherWindowNumber];

#ifdef USE_NATIVE_EVENTS
	if ( [self styleMask] & NSUtilityWindowMask && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
#else	// USE_NATIVE_EVENTS
	if ( [self level] == NSFloatingWindowLevel && [self styleMask] & NSTitledWindowMask && [[self className] isEqualToString:pCocoaAppWindowString] )
#endif	// USE_NATIVE_EVENTS
	{
		if ( ![self isVisible] )
		{
			// Fix bug 3637 by making the first non-floating, non-utility
			// window have focus after a utility window is closed
			NSApplication *pApp = [NSApplication sharedApplication];
			if ( pApp )
			{
				NSArray *pWindows = [pApp orderedWindows];
				if ( pWindows )
				{
					unsigned int i = 0;
					unsigned int nCount = [pWindows count];
					for ( ; i < nCount; i++ )
					{
						NSWindow *pWindow = [pWindows objectAtIndex:i];
						if ( pWindow && [pWindow isVisible] && [pWindow level] == NSNormalWindowLevel && [pWindow styleMask] & NSTitledWindowMask && ( [pWindow isKindOfClass:[VCLPanel class]] || [pWindow isKindOfClass:[VCLWindow class]] || [[pWindow className] isEqualToString:pCocoaAppWindowString] ) )
						{
							[pWindow makeKeyWindow];
							break;
						}
					}
				}
			}
		}
#ifndef USE_NATIVE_EVENTS
		else
		{
			NSView *pContentView = [self contentView];
			if ( pContentView )
			{
				if ( [super respondsToSelector:@selector(_isUtilityWindow)] && ![super _isUtilityWindow] && [super respondsToSelector:@selector(_setUtilityWindow:)] )
				{
					// Make copy of frame to fix compiler bug that appeared
					// when this source file was renamed from a *.m file to
					// a *.mm file
					NSRect aFrame = [self frame];
					aFrame = NSMakeRect( aFrame.origin.x, aFrame.origin.y, aFrame.size.width, aFrame.size.height );

					[super _setUtilityWindow:YES];

					// We must set the level again after changing the window to
					// a utility window otherwise the resize icon does not
					// display on Mac OS X 10.5.x
					[self setLevel:NSFloatingWindowLevel];

					float fHeightChange = [self frame].size.height - aFrame.size.height;

					[self setFrame:aFrame display:NO];

					// Adjust origin of subviews by height change
					if ( bUseQuickTimeContentViewHack )
					{
						NSArray *pSubviews = [pContentView subviews];
						if ( pSubviews )
						{
							unsigned int nCount = [pSubviews count];
							unsigned int i = 0;
							for ( ; i < nCount; i++ )
							{
								NSView *pSubview = (NSView *)[pSubviews objectAtIndex:i];
								if ( pSubview && [pSubview isFlipped] )
								{
									NSRect aBounds = [pSubview bounds];
									aBounds.origin.y += fHeightChange;
									[pSubview setBounds:aBounds];
								}
							}
						}
					}
					else
					{
						NSRect aBounds = [pContentView bounds];
						aBounds.origin.y += fHeightChange;
						[pContentView setBounds:aBounds];
					}
				}
			}
		}
#endif	// !USE_NATIVE_EVENTS
	}
	else if ( nOrderingMode != NSWindowOut && [self isKindOfClass:[NSPanel class]] && [self isFloatingPanel] && [self respondsToSelector:@selector(setStyleMask:)] )
	{
		// Fix bug in ICAImageImport()'s window by removing its close button.
		// Pressing that button on Mac OS X 10.6 and higher causes that
		// function to never return.
		NSView *pContentView = [self contentView];
		if ( pContentView )
		{
			NSArray *pSubviews = [pContentView subviews];
			if ( pSubviews )
			{
				unsigned int nCount = [pSubviews count];
				unsigned int i = 0;
				for ( ; i < nCount; i++ )
				{
					NSView *pSubview = (NSView *)[pSubviews objectAtIndex:i];
					if ( pSubview && [[pSubview className] isEqualToString:@"IKDeviceBrowserView"] )
					{
						[self setStyleMask:[self styleMask] & ~NSClosableWindowMask];
						break;
					}
				}
			}
		}
	}
}

- (MacOSBOOL)performKeyEquivalent:(NSEvent *)pEvent
{
	MacOSBOOL bCommandKeyPressed = ( pEvent && [pEvent modifierFlags] & NSCommandKeyMask );

#ifdef USE_NATIVE_EVENTS
	if ( bCommandKeyPressed && [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
#else	// USE_NATIVE_EVENTS
	if ( bCommandKeyPressed && [self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] )
#endif	// USE_NATIVE_EVENTS
	{
		[pSharedResponder interpretKeyEvents:[NSArray arrayWithObject:pEvent]];

#ifndef USE_NATIVE_EVENTS
		// Fix crashing when using a menu shortcut by forcing cancellation of
		// the input method
		NSResponder *pResponder = [self firstResponder];
		if ( pResponder && [pResponder respondsToSelector:@selector(hasMarkedText)] )
		{
			// Fix bug 2783 by not cancelling the input method if the command
			// key is pressed, but instead, returning YES to cancel the menu
			// matching process
			if ( [pResponder hasMarkedText] )
				return YES;
		}
#endif	// !USE_NATIVE_EVENTS

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


		// Fix bug 3496 by having any Cocoa commands take precedence over menu
		// shortcuts
		short nCommandKey = [(VCLResponder *)pSharedResponder lastCommandKey];
#ifdef USE_NATIVE_EVENTS
		if ( nCommandKey && mbAllowKeyBindings )
			fprintf( stderr, "VCLEventQueue_postCommandEvent not implemented\n" );
#else	// USE_NATIVE_EVENTS
		if ( nCommandKey && VCLEventQueue_postCommandEvent( [self peer], nCommandKey, [(VCLResponder *)pSharedResponder lastModifiers], [(VCLResponder *)pSharedResponder lastOriginalKeyChar], [(VCLResponder *)pSharedResponder lastOriginalModifiers] ) )
			return YES;
#endif	// USE_NATIVE_EVENTS

		// Fix bug 3357 by updating native menus. Fix bug 3379 by retaining
		// this window as this window may get released while updating.
		[self retain];
		VCLInstance_updateNativeMenus();
		MacOSBOOL bVisible = [self isVisible];
		[self release];
		if ( !bVisible )
			return YES;
	}

	MacOSBOOL bRet = NO;
	if ( [super respondsToSelector:@selector(poseAsPerformKeyEquivalent:)] )
		bRet = [super poseAsPerformKeyEquivalent:pEvent];

	// Fix bug 1751 by responding to Command-c, Command-v, and Command-x keys
	// for non-Java windows. Fix bug 3561 by responding to Command-w keys for
	// closable non-Java windows.
#ifdef USE_NATIVE_EVENTS
	if ( !bRet && bCommandKeyPressed && [self isVisible] && ![self isKindOfClass:[VCLPanel class]] && ![self isKindOfClass:[VCLWindow class]] )
#else	// USE_NATIVE_EVENTS
	if ( !bRet && bCommandKeyPressed && [self isVisible] && ![[self className] isEqualToString:pCocoaAppWindowString] )
#endif	// USE_NATIVE_EVENTS
	{
		NSString *pChars = [pEvent charactersIgnoringModifiers];
		NSResponder *pResponder = [self firstResponder];
		if ( pChars && pResponder )
		{
			if ( [pChars isEqualToString:@"a"] && [pResponder respondsToSelector:@selector(copy:)] )
			{
				[pResponder selectAll:self];
				bRet = YES;
			}
			else if ( [pChars isEqualToString:@"c"] && [pResponder respondsToSelector:@selector(copy:)] )
			{
				[pResponder copy:self];
				bRet = YES;
			}
			else if ( [pChars isEqualToString:@"v"] && [pResponder respondsToSelector:@selector(paste:)] )
			{
				[pResponder paste:self];
				bRet = YES;
			}
			else if ( [pChars isEqualToString:@"w"] && [self styleMask] & NSClosableWindowMask )
			{
				[self performClose:self];
				bRet = YES;
			}
			else if ( [pChars isEqualToString:@"x"] && [pResponder respondsToSelector:@selector(cut:)] )
			{
				[pResponder cut:self];
				bRet = YES;
			}
		}
	}

	return bRet;
}

- (void)resignKeyWindow
{
#ifdef USE_NATIVE_EVENTS
	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) && mpFrame )
	{
		JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_LOSEFOCUS, mpFrame, NULL );
		JavaSalEventQueue::postCachedEvent( pEvent );
		pEvent->release();
#else	// USE_NATIVE_EVENTS
	if ( [self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] )
	{
		// Fix bug 1819 by forcing cancellation of the input method
		NSResponder *pResponder = [self firstResponder];
		if ( pResponder && [pResponder respondsToSelector:@selector(abandonInput)] && [pResponder respondsToSelector:@selector(hasMarkedText)] && [pResponder respondsToSelector:@selector(insertText:)] )
		{
			if ( [pResponder hasMarkedText] )
				[pResponder insertText:pCancelInputMethodText];
			[pResponder abandonInput];
		}
#endif	// USE_NATIVE_EVENTS

		// Fix bug 3557 by forcing any non-utility windows to the back when
		// they lose focus while cycling through windows with the Command-`
		// shortcut. Fix bug 3557 by including the event's device dependent
		// modifiers if the Shift key is pressed and excluding the Control key
		// modifier if the Control key is pressed.
		if ( ! ( [self styleMask] & NSUtilityWindowMask ) )
		{
			NSApplication *pApp = [NSApplication sharedApplication];
			if ( pApp && EventMatchesShortcutKey( [pApp currentEvent], 27 ) )
			{
				NSArray *pWindows = [pApp orderedWindows];
				if ( pWindows )
				{
					unsigned int nCount = [pWindows count];
					if ( nCount )
					{
						NSWindow *pBackWindow = [pWindows objectAtIndex:nCount - 1];
						if ( pBackWindow && pBackWindow != self && [pBackWindow isVisible] )
							[self orderWindow:NSWindowBelow relativeTo:[pBackWindow windowNumber]];
					}
				}
			}
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

	NSEventType nType = [pEvent type];

#ifndef USE_NATIVE_EVENTS
	// Fix bugs 1390 and 1619 by reprocessing any events with more than one
	// character as the JVM only seems to process the first character
	if ( nType == NSKeyDown && pSharedResponder && [self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] && [self respondsToSelector:@selector(peer)] )
	{
		[pSharedResponder interpretKeyEvents:[NSArray arrayWithObject:pEvent]];

		MacOSBOOL bHasMarkedText = NO;
		NSResponder *pResponder = [self firstResponder];
		if ( pResponder && [pResponder respondsToSelector:@selector(hasMarkedText)] )
			bHasMarkedText = [pResponder hasMarkedText];

		// Process any Cocoa commands but ignore when there is marked text
		short nCommandKey = [(VCLResponder *)pSharedResponder lastCommandKey];
		if ( nCommandKey && !bHasMarkedText && VCLEventQueue_postCommandEvent( [self peer], nCommandKey, [(VCLResponder *)pSharedResponder lastModifiers], [(VCLResponder *)pSharedResponder lastOriginalKeyChar], [(VCLResponder *)pSharedResponder lastOriginalModifiers] ) )
			return;
	}
#endif	// !USE_NATIVE_EVENTS

	if ( [super respondsToSelector:@selector(poseAsSendEvent:)] )
		[super poseAsSendEvent:pEvent];

#ifdef USE_NATIVE_EVENTS
	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) && mpFrame )
#else	// USE_NATIVE_EVENTS
	if ( [self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] && [self respondsToSelector:@selector(peer)] )
#endif	// USE_NATIVE_EVENTS
	{
#ifdef USE_NATIVE_EVENTS
		// Handle all mouse events
		if ( ( nType >= NSLeftMouseDown && nType <= NSMouseExited ) || ( nType >= NSOtherMouseDown && nType <= NSOtherMouseDragged ) )
		{
			USHORT nID = 0;
			NSUInteger nModifiers = [pEvent modifierFlags];

			switch ( nType )
			{
				case NSMouseMoved:
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
					if ( [self canBecomeKeyOrMainWindow] && ![self isKeyWindow] && ! ( nModifiers & NSLeftMouseDownMask ) )
					{
						mnIgnoreMouseReleasedModifiers = nModifiers;
						return;
					}
					else
					{
						mnIgnoreMouseReleasedModifiers = 0;
					}
				}
				else if ( nID == SALEVENT_MOUSEBUTTONUP )
				{
					// Fix bug 3453 by adding back any recently released
					// modifiers
					if ( mnLastMetaModifierReleasedTime >= (ULONG)( [pEvent timestamp] * 1000 ) )
						nModifiers |= NSCommandKeyMask;

					if ( mnIgnoreMouseReleasedModifiers && ( mnIgnoreMouseReleasedModifiers & nModifiers ) == nModifiers )
					{
						mnIgnoreMouseReleasedModifiers &= ~nModifiers;
						return;
					}
				}

				// Fix bug 2769 by creating synthetic window moved events when
				// dragging a window's title bar
				if ( nModifiers & NSLeftMouseDownMask && ( nID == SALEVENT_MOUSELEAVE || nID == SALEVENT_MOUSEMOVE ) )
				{
					NSRect aFrame = [self frame];
					NSRect aContentFrame = [self contentRectForFrameRect:aFrame];
					float fLeftInset = aFrame.origin.x - aContentFrame.origin.x;
					float fTopInset = aFrame.origin.y + aFrame.size.height - aContentFrame.origin.y - aContentFrame.size.height;
					NSRect aTitlebarFrame = NSMakeRect( fLeftInset, aContentFrame.origin.y + aContentFrame.size.height - aFrame.origin.y, aFrame.size.width, fTopInset );
					NSPoint aLocation = [pEvent locationInWindow];
					if ( NSPointInRect( aLocation, aTitlebarFrame ) )
						[self windowDidMove:nil];
				}

				USHORT nCode = GetEventCode( nModifiers );

				NSPoint aLocation = GetFlippedContentViewLocation( self, pEvent );
				SalMouseEvent *pMouseEvent = new SalMouseEvent();
				pMouseEvent->mnTime = (ULONG)( [pEvent timestamp] * 1000 );
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
					USHORT nExtraCode = GetEventCode( [pEvent modifierFlags] | nMouseMask );
					pExtraMouseEvent = new SalMouseEvent();
					pExtraMouseEvent->mnTime = (ULONG)( [pEvent timestamp] * 1000 );
					pExtraMouseEvent->mnX = (long)aLocation.x;
					pExtraMouseEvent->mnY = (long)aLocation.y;
					pExtraMouseEvent->mnButton = 0;
					pExtraMouseEvent->mnCode = nExtraCode;
				}

				JavaSalEvent *pEvent = new JavaSalEvent( nID, mpFrame, pMouseEvent );
				JavaSalEventQueue::postCachedEvent( pEvent );
				pEvent->release();

				if ( pExtraMouseEvent )
				{
					JavaSalEvent *pExtraEvent = new JavaSalEvent( SALEVENT_MOUSEMOVE, mpFrame, pExtraMouseEvent );
					JavaSalEventQueue::postCachedEvent( pExtraEvent );
					pExtraEvent->release();
				}
			}
		}
		// Handle key modifier change events
		else if ( nType == NSFlagsChanged )
		{
			NSUInteger nModifiers = [pEvent modifierFlags];
			if ( nModifiers & NSCommandKeyMask )
				mnLastMetaModifierReleasedTime = 0;
			else
				mnLastMetaModifierReleasedTime = (ULONG)( [pEvent timestamp] * 1000 ) + MODIFIER_RELEASE_INTERVAL;

			USHORT nCode = GetEventCode( nModifiers );

			SalKeyModEvent *pKeyModEvent = new SalKeyModEvent();
			pKeyModEvent->mnTime = (ULONG)( [pEvent timestamp] * 1000 );
			pKeyModEvent->mnCode = nCode;
			pKeyModEvent->mnModKeyCode = 0;

			JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_KEYMODCHANGE, mpFrame, pKeyModEvent );
			JavaSalEventQueue::postCachedEvent( pEvent );
			pEvent->release();
		}
#else	// USE_NATIVE_EVENTS
		if ( nType == NSLeftMouseDown || nType == NSLeftMouseUp )
		{
			NSRect aFrame = [self frame];
			NSRect aContentFrame = [self contentRectForFrameRect:aFrame];
			float fLeftInset = aFrame.origin.x - aContentFrame.origin.x;
			float fTopInset = aFrame.origin.y + aFrame.size.height - aContentFrame.origin.y - aContentFrame.size.height;
			NSRect aTitlebarFrame = NSMakeRect( fLeftInset, aContentFrame.origin.y + aContentFrame.size.height - aFrame.origin.y, aFrame.size.width, fTopInset );
			NSPoint aLocation = [pEvent locationInWindow];
			if ( NSPointInRect( aLocation, aTitlebarFrame ) )
			{
				VCLEventQueue_postWindowMoveSessionEvent( [self peer], (long)( aLocation.x - fLeftInset ), (long)( aFrame.size.height - aLocation.y - fTopInset ), nType == NSLeftMouseDown ? YES : NO );
			}
		}
#endif	// USE_NATIVE_EVENTS
		// Handle scroll wheel and magnify
		else if ( nType == NSScrollWheel || ( nType == NSEventTypeMagnify && pSharedResponder && ![pSharedResponder ignoreTrackpadGestures] ) )
		{
			int nModifiers = [pEvent modifierFlags];
			float fDeltaX;
			float fDeltaY;
			if ( nType == NSEventTypeMagnify )
			{
				// Magnify events need to be converted to vertical scrolls with
				// the Command key pressed to force the OOo code to zoom.
				// Fix bug 3284 by reducing the amount of magnification.
				nModifiers |= NSCommandKeyMask;
				fDeltaX = 0;
				fDeltaY = [pEvent deltaZ] / 8;
			}
			else
			{
				fDeltaX = [pEvent deltaX];
				fDeltaY = [pEvent deltaY];
			}

#ifdef USE_NATIVE_EVENTS
			NSPoint aLocation = GetFlippedContentViewLocation( self, pEvent );

	        // Fix bug 3030 by setting the modifiers. Note that we ignore the
			// Shift modifier as using it will disable horizontal scrolling.
			USHORT nCode = GetEventCode( nModifiers ) & ( KEY_MOD1 | KEY_MOD2 | KEY_MOD3 );

			// Note: no matter what buttons we press, mimic the MouseWheelEvents
			// in Apple's JVMs always seem to have the following constant
			// values:
			//   ScrollType == MouseWheelEvent.WHEEL_UNIT_SCROLL
			//   ScrollUnits == 1
			if ( fDeltaX )
			{
				long nScrollAmount = Float32ToLong( fDeltaX );
				SalWheelMouseEvent *pWheelMouseEvent = new SalWheelMouseEvent();
				pWheelMouseEvent->mnTime = (ULONG)( [pEvent timestamp] * 1000 );
				pWheelMouseEvent->mnX = (long)aLocation.x;
				pWheelMouseEvent->mnY = (long)aLocation.y;
				pWheelMouseEvent->mnDelta = nScrollAmount * WHEEL_ROTATION_FACTOR;
				pWheelMouseEvent->mnNotchDelta = nScrollAmount;
				pWheelMouseEvent->mnScrollLines = 1;
				pWheelMouseEvent->mnCode = nCode;
				pWheelMouseEvent->mbHorz = TRUE;

				JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_WHEELMOUSE, mpFrame, pWheelMouseEvent );
				JavaSalEventQueue::postCachedEvent( pEvent );
				pEvent->release();
			}
			if ( fDeltaY )
			{
				long nScrollAmount = Float32ToLong( fDeltaY );
				SalWheelMouseEvent *pWheelMouseEvent = new SalWheelMouseEvent();
				pWheelMouseEvent->mnTime = (ULONG)( [pEvent timestamp] * 1000 );
				pWheelMouseEvent->mnX = (long)aLocation.x;
				pWheelMouseEvent->mnY = (long)aLocation.y;
				pWheelMouseEvent->mnDelta = nScrollAmount * WHEEL_ROTATION_FACTOR;
				pWheelMouseEvent->mnNotchDelta = nScrollAmount;
				pWheelMouseEvent->mnScrollLines = 1;
				pWheelMouseEvent->mnCode = nCode;
				pWheelMouseEvent->mbHorz = FALSE;

				JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_WHEELMOUSE, mpFrame, pWheelMouseEvent );
				JavaSalEventQueue::postCachedEvent( pEvent );
				pEvent->release();
			}
#else	// USE_NATIVE_EVENTS
			// Post flipped coordinates 
			NSRect aFrame = [self frame];
			NSRect aContentFrame = [self contentRectForFrameRect:aFrame];
			float fLeftInset = aFrame.origin.x - aContentFrame.origin.x;
			float fTopInset = aFrame.origin.y + aFrame.size.height - aContentFrame.origin.y - aContentFrame.size.height;
			NSPoint aLocation = [pEvent locationInWindow];
			VCLEventQueue_postMouseWheelEvent( [self peer], (long)( aLocation.x - fLeftInset ), (long)( aFrame.size.height - aLocation.y - fTopInset ), Float32ToLong( fDeltaX ), Float32ToLong( fDeltaY ) * -1, nModifiers & NSShiftKeyMask ? YES : NO, nModifiers & NSCommandKeyMask ? YES : NO, nModifiers & NSAlternateKeyMask ? YES : NO, nModifiers & NSControlKeyMask ? YES : NO );
#endif	// USE_NATIVE_EVENTS
		}
		// Handle swipe
		else if ( nType == NSEventTypeSwipe && pSharedResponder && ![pSharedResponder ignoreTrackpadGestures] )
		{
			NSApplication *pApp = [NSApplication sharedApplication];
			float fDeltaX = [pEvent deltaX] * -1;
			float fDeltaY = [pEvent deltaY] * -1;
			if ( pApp && ( fDeltaX != 0 || fDeltaY != 0 ) )
			{
				unichar pChars[ 1 ];
				pChars[ 0 ] = ( fDeltaY == 0 ? ( fDeltaX < 0 ? NSPageUpFunctionKey : NSPageDownFunctionKey ) : ( fDeltaY < 0 ? NSPageUpFunctionKey : NSPageDownFunctionKey ) );
				unsigned short nKeyCode = ( pChars[ 0 ] == NSPageUpFunctionKey ? kVK_PageUp : kVK_PageDown );
				NSString *pChar = [NSString stringWithCharacters:&pChars[0] length:1];
				if ( pChar )
				{
					NSEvent *pKeyDownEvent = [NSEvent keyEventWithType:NSKeyDown location:[pEvent locationInWindow] modifierFlags:[pEvent modifierFlags] timestamp:[pEvent timestamp] windowNumber:[pEvent windowNumber] context:[pEvent context] characters:pChar charactersIgnoringModifiers:pChar isARepeat:NO keyCode:nKeyCode];
					NSEvent *pKeyUpEvent = [NSEvent keyEventWithType:NSKeyUp location:[pEvent locationInWindow] modifierFlags:[pEvent modifierFlags] timestamp:[pEvent timestamp] windowNumber:[pEvent windowNumber] context:[pEvent context] characters:pChar charactersIgnoringModifiers:pChar isARepeat:NO keyCode:nKeyCode];
					if ( pKeyDownEvent && pKeyUpEvent )
					{
						// Post in reverse order since we are posting to the
						// front
						[pApp postEvent:pKeyUpEvent atStart:YES];
						[pApp postEvent:pKeyDownEvent atStart:YES];
					}
				}
			}
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

#ifdef USE_NATIVE_EVENTS

- (void)setAllowKeyBindings:(MacOSBOOL)bAllowKeyBindings
{
	mbAllowKeyBindings = bAllowKeyBindings;
}

- (void)setCanBecomeKeyOrMainWindow:(MacOSBOOL)bCanBecomeKeyOrMainWindow
{
	mbCanBecomeKeyOrMainWindow = bCanBecomeKeyOrMainWindow;
}

- (void)setFrame:(JavaSalFrame *)pFrame
{
	mpFrame = pFrame;

	NSView *pContentView = [self contentView];
	if ( pContentView && [pContentView isKindOfClass:[VCLView class]] )
		[(VCLView *)pContentView setFrame:pFrame];
}

#endif	// USE_NATIVE_EVENTS

- (void)setContentView:(NSView *)pView
{
	if ( [super respondsToSelector:@selector(poseAsSetContentView:)] )
		[super poseAsSetContentView:pView];

#ifndef USE_NATIVE_EVENTS
	// It was found that with QuickTime 7.4 on G4 systems running ATI RAGE 128
	// graphics cards, QTMovieView will misplace the movie if the window's
	// content view is flipped. Since Java replaces the default content view
	// with a flipped view, we need to push their content view down a level
	// and make the content view unflipped.
	if ( bUseQuickTimeContentViewHack )
	{
		NSView *pContentView = [self contentView];
		if ( pContentView && [pContentView isFlipped] && [[pContentView className] isEqualToString:pNSWindowViewAWTString] )
		{
			NSRect aFrame = [pContentView frame];
			VCLWindowView *pNewContentView = [[VCLWindowView alloc] initWithFrame:aFrame];
			if ( pNewContentView )
			{
				// Retain current content view just to be safe
				[pNewContentView retain];

				if ( [super respondsToSelector:@selector(poseAsSetContentView:)] )
					[super poseAsSetContentView:pNewContentView];
				aFrame.origin.x = 0;
				aFrame.origin.y = 0;
				[pContentView setFrame:aFrame];
				[pNewContentView addSubview:pContentView positioned:NSWindowAbove relativeTo:nil];

				[pNewContentView release];
			}
		}
	}
#endif	// !USE_NATIVE_EVENTS
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

- (void)setLevel:(int)nWindowLevel
{
#ifndef USE_NATIVE_EVENTS
	// Don't let Java unset our window level changes unless it is modal window
	// or the window has been set to the "revert document" window level
	if ( [self level] > nWindowLevel && [self level] != NSModalPanelWindowLevel && [self level] != 2 && [[self className] isEqualToString:pCocoaAppWindowString] )
		return;
#endif	// !USE_NATIVE_EVENTS

	if ( [super respondsToSelector:@selector(poseAsSetLevel:)] )
		[super poseAsSetLevel:nWindowLevel];
}

- (void)windowDidExitFullScreen:(NSNotification *)pNotification
{
#ifdef USE_NATIVE_FULL_SCREEN_MODE
#ifdef USE_NATIVE_EVENTS
	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
#else	// USE_NATIVE_EVENTS
	if ( [self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] )
#endif	// USE_NATIVE_EVENTS
		VCLEventQueue_fullScreen( self, NO );
#endif	// USE_NATIVE_FULL_SCREEN_MODE
}

- (void)windowWillEnterFullScreen:(NSNotification *)pNotification
{
#ifdef USE_NATIVE_FULL_SCREEN_MODE
#ifdef USE_NATIVE_EVENTS
	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
#else	// USE_NATIVE_EVENTS
	if ( [self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] )
#endif	// USE_NATIVE_EVENTS
		VCLEventQueue_fullScreen( self, YES );
#endif	// USE_NATIVE_FULL_SCREEN_MODE
}

#ifdef USE_NATIVE_EVENTS

- (void)windowDidMove:(NSNotification *)pNotification
{
	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) && mpFrame )
	{
		JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_MOVERESIZE, mpFrame, NULL );
		JavaSalEventQueue::postCachedEvent( pEvent );
		pEvent->release();
	}
}

- (void)windowDidResize:(NSNotification *)pNotification
{
	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) && mpFrame )
	{
		JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_MOVERESIZE, mpFrame, NULL );
		JavaSalEventQueue::postCachedEvent( pEvent );
		pEvent->release();
	}
}

- (MacOSBOOL)windowShouldClose:(id)pObject
{
	MacOSBOOL bRet = YES;

	if ( [self isVisible] && ( [self isKindOfClass:[VCLPanel class]] || [self isKindOfClass:[VCLWindow class]] ) )
	{
		if ( mpFrame )
		{
			JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_CLOSE, mpFrame, NULL );
			JavaSalEventQueue::postCachedEvent( pEvent );
			pEvent->release();
			bRet = NO;
		}
	}

	return bRet;
}

#endif	// USE_NATIVE_EVENTS

@end

#ifndef USE_NATIVE_EVENTS
static MacOSBOOL bNSViewAWTInitialized = NO;
#endif	// !USE_NATIVE_EVENTS
static CFStringRef aTextSelection = nil;
static CFDataRef aRTFSelection = nil;

@implementation VCLView

#ifdef USE_NATIVE_EVENTS

- (MacOSBOOL)acceptsFirstResponder
{
	return YES;
}

- (void)dealloc
{
	if ( mpLastKeyDownEvent )
		[mpLastKeyDownEvent release];

	if ( mpPendingKeyUpEventList )
	{
		while ( mpPendingKeyUpEventList->size() )
		{
			SalKeyEvent *pKeyUpEvent = mpPendingKeyUpEventList->front();
			mpPendingKeyUpEventList->pop_front();
			delete pKeyUpEvent;
		}

		delete mpPendingKeyUpEventList;
	}

	[super dealloc];
}

- (void)keyDown:(NSEvent *)pEvent
{
	if ( mpLastKeyDownEvent )
		[mpLastKeyDownEvent release];
	mpLastKeyDownEvent = ( [pEvent type] == NSKeyDown ? pEvent : nil );
	if ( mpLastKeyDownEvent )
		[mpLastKeyDownEvent retain];

	if ( mpPendingKeyUpEventList )
	{
		while ( mpPendingKeyUpEventList->size() )
		{
			SalKeyEvent *pKeyUpEvent = mpPendingKeyUpEventList->front();
			mpPendingKeyUpEventList->pop_front();
			delete pKeyUpEvent;
		}
	}

	[self interpretKeyEvents:[NSArray arrayWithObject:pEvent]];
}

- (void)keyUp:(NSEvent *)pEvent
{
	MacOSBOOL bPostEvents = NO;
	NSWindow *pWindow = [self window];
	if ( pWindow && [pWindow isVisible] && mpFrame && pEvent )
		bPostEvents = YES;

	if ( mpPendingKeyUpEventList )
	{
		while ( mpPendingKeyUpEventList->size() )
		{
			SalKeyEvent *pKeyUpEvent = mpPendingKeyUpEventList->front();
			mpPendingKeyUpEventList->pop_front();
			if ( bPostEvents )
			{
				pKeyUpEvent->mnTime = (ULONG)( [pEvent timestamp] * 1000 );
				JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_KEYUP, mpFrame, pKeyUpEvent );
				JavaSalEventQueue::postCachedEvent( pEvent );
				pEvent->release();
			}
			else
			{
				delete pKeyUpEvent;
			}
		}
	}

}

- (MacOSBOOL)hasMarkedText
{
	fprintf( stderr, "[VCLView hasMarkedText] not implemented\n" );
	return NO;
}

- (NSRange)markedRange
{
	fprintf( stderr, "[VCLView markedRange] not implemented\n" );
	return NSMakeRange( NSNotFound, 0 );
}

- (NSRange)selectedRange
{
	fprintf( stderr, "[VCLView selectedRange] not implemented\n" );
	return NSMakeRange( NSNotFound, 0 );
}

- (void)setMarkedText:(id)aString selectedRange:(NSRange)aSelectedRange replacementRange:(NSRange)aReplacementRange
{
	fprintf( stderr, "[VCLView setMarkedText:selectedRange:replacementRange:] not implemented\n" );
}

- (void)unmarkText
{
	fprintf( stderr, "[VCLView unmarkText] not implemented\n" );
}

- (NSArray *)validAttributesForMarkedText
{
	return [NSArray arrayWithObject:NSUnderlineStyleAttributeName];
}

- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)pActualRange
{
	fprintf( stderr, "[VCLView attributedSubstringForProposedRange:actualRange:] not implemented\n" );
	return nil;
}

- (void)insertText:(id)aString replacementRange:(NSRange)aReplacementRange
{
	NSWindow *pWindow = [self window];
	if ( pWindow && [pWindow isVisible] && mpFrame && mpLastKeyDownEvent )
	{
		NSString *pChars;
		if ( [aString isKindOfClass:[NSAttributedString class]] )
			pChars = [(NSAttributedString *)aString string];
		else
			pChars = (NSString *)aString;
		if ( pChars && [pChars length] )
		{
			// Fix bug 710 by stripping out the Alt modifier. Note that we do
			// it here because we need to let the Alt modifier through for
			// action keys.
			NSUInteger nModifiers = [mpLastKeyDownEvent modifierFlags] | nMouseMask;
			USHORT nCode = GetEventCode( nModifiers & ~NSAlternateKeyMask );

			NSUInteger i = 0;
			NSUInteger nLength = [pChars length];
			for ( ; i < nLength; i++ )
			{
				SalKeyEvent *pKeyDownEvent = new SalKeyEvent();
				pKeyDownEvent->mnTime = (ULONG)( [mpLastKeyDownEvent timestamp] * 1000 );
				pKeyDownEvent->mnCode = nCode;
				pKeyDownEvent->mnCharCode = [pChars characterAtIndex:i];
				pKeyDownEvent->mnRepeat = 0;

				if ( mpPendingKeyUpEventList )
				{
					SalKeyEvent *pKeyUpEvent = new SalKeyEvent();
					memcpy( pKeyUpEvent, pKeyDownEvent, sizeof( SalKeyEvent ) );
					mpPendingKeyUpEventList->push_back( pKeyUpEvent );
				}

				JavaSalEvent *pEvent = new JavaSalEvent( SALEVENT_KEYINPUT, mpFrame, pKeyDownEvent );
				JavaSalEventQueue::postCachedEvent( pEvent );
				pEvent->release();
			}
		}
	}
}

- (NSUInteger)characterIndexForPoint:(NSPoint)aPoint
{
	fprintf( stderr, "[VCLView characterIndexForPoint:] not implemented\n" );
	return NSNotFound;
}

- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)pActualRange
{
	fprintf( stderr, "[VCLView firstRectForCharacterRange:actualRange:] not implemented\n" );
	if ( pActualRange )
		pActualRange = NULL;
	return NSZeroRect;
}

- (void)doCommandBySelector:(SEL)aSelector
{
	fprintf( stderr, "[VCLView doCommandBySelector:] not implemented\n" );
}

- (void)insertText:(id)aString
{
	[self insertText:aString replacementRange:NSMakeRange( NSNotFound, 0 )];
}

- (void)setFrame:(JavaSalFrame *)pFrame
{
	mpFrame = pFrame;
}

#else	// USE_NATIVE_EVENTS

+ (void)swizzleSelectors:(NSView *)pView
{
	// If the NSViewAWT class has its own drag and drop and services selectors,
	// redirect them to VCLView's matching selectors
	if ( pView && !bNSViewAWTInitialized && [[pView className] isEqualToString:pNSViewAWTString] )
	{
		bNSViewAWTInitialized = YES;

		// VCLView drag destination selectors

		SEL aSelector = @selector(draggingDestinationDelegate);
		Method aNewMethod = class_getInstanceMethod( [VCLView class], aSelector );
		if ( aNewMethod )
		{
			IMP aNewIMP = method_getImplementation( aNewMethod );
			if ( aNewIMP )
				class_addMethod( [NSView class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
		}

		aSelector = @selector(setDraggingDestinationDelegate:);
		aNewMethod = class_getInstanceMethod( [VCLView class], aSelector );
		if ( aNewMethod )
		{
			IMP aNewIMP = method_getImplementation( aNewMethod );
			if ( aNewIMP )
				class_addMethod( [NSView class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
		}

		// VCLView drag source selectors

		aSelector = @selector(draggingSourceDelegate);
		aNewMethod = class_getInstanceMethod( [VCLView class], aSelector );
		if ( aNewMethod )
		{
			IMP aNewIMP = method_getImplementation( aNewMethod );
			if ( aNewIMP )
				class_addMethod( [NSView class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
		}

		aSelector = @selector(setDraggingSourceDelegate:);
		aNewMethod = class_getInstanceMethod( [VCLView class], aSelector );
		if ( aNewMethod )
		{
			IMP aNewIMP = method_getImplementation( aNewMethod );
			if ( aNewIMP )
				class_addMethod( [NSView class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
		}

		// NSDraggingDestination selectors

		aSelector = @selector(concludeDragOperation:);
		Method aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		IMP aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		aSelector = @selector(draggingEnded:);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		aSelector = @selector(draggingEntered:);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		aSelector = @selector(draggingExited:);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		aSelector = @selector(draggingUpdated:);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		aSelector = @selector(performDragOperation:);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		aSelector = @selector(prepareForDragOperation:);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		aSelector = @selector(wantsPeriodicDraggingUpdates);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		// NSDraggingSource selectors

		aSelector = @selector(draggedImage:beganAt:);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		aSelector = @selector(draggedImage:endedAt:operation:);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		aSelector = @selector(draggedImage:movedTo:);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		aSelector = @selector(draggingSourceOperationMaskForLocal:);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		aSelector = @selector(ignoreModifierKeysWhileDragging);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		aSelector = @selector(namesOfPromisedFilesDroppedAtDestination:);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		// NSResponder selectors

		aSelector = @selector(readSelectionFromPasteboard:);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		aSelector = @selector(validRequestorForSendType:returnType:);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		aSelector = @selector(writeSelectionToPasteboard:types:);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

#ifdef USE_NATIVE_WINDOW
		// NSViewAWT selectors

		aSelector = @selector(drawRect:);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );

		aSelector = @selector(resetCursorRects);
		aOldMethod = class_getInstanceMethod( [pView class], aSelector );
		aNewIMP = [[VCLView class] instanceMethodForSelector:aSelector];
		if ( aOldMethod && aNewIMP )
			method_setImplementation( aOldMethod, aNewIMP );
#endif	// USE_NATIVE_WINDOW
	}
}

#endif	// USE_NATIVE_EVENTS

- (void)concludeDragOperation:(id < NSDraggingInfo >)pSender
{
	id pDelegate = [self draggingDestinationDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(concludeDragOperation:)])
		[pDelegate concludeDragOperation:pSender];
	else if ( [super respondsToSelector:@selector(concludeDragOperation:)] )
		[super concludeDragOperation:pSender];
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
	else if ( [super respondsToSelector:@selector(draggedImage:beganAt:)] )
		[super draggedImage:pImage beganAt:aPoint];
}

- (void)draggedImage:(NSImage *)pImage endedAt:(NSPoint)aPoint operation:(NSDragOperation)nOperation
{
	id pDelegate = [self draggingSourceDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(draggedImage:endedAt:operation:)])
		[pDelegate draggedImage:pImage endedAt:aPoint operation:nOperation];
	else if ( [super respondsToSelector:@selector(draggedImage:endedAt:operation:)] )
		[super draggedImage:pImage endedAt:aPoint operation:nOperation];
}

- (void)draggedImage:(NSImage *)pImage movedTo:(NSPoint)aPoint
{
	id pDelegate = [self draggingSourceDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(draggedImage:movedTo:)])
		[pDelegate draggedImage:pImage movedTo:aPoint];
	else if ( [super respondsToSelector:@selector(draggedImage:movedTo:)] )
		[super draggedImage:pImage movedTo:aPoint];
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
	else if ( [super respondsToSelector:@selector(draggingEnded:)] )
		[super draggingEnded:pSender];
}

- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)pSender
{
	id pDelegate = [self draggingDestinationDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(draggingEntered:)])
		return [pDelegate draggingEntered:pSender];
	else if ( [super respondsToSelector:@selector(draggingEntered:)] )
		return [super draggingEntered:pSender];
	else
		return NSDragOperationNone;
}

- (void)draggingExited:(id < NSDraggingInfo >)pSender
{
	id pDelegate = [self draggingDestinationDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(draggingExited:)])
		[pDelegate draggingExited:pSender];
	else if ( [super respondsToSelector:@selector(draggingExited:)] )
		[super draggingExited:pSender];
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
	else if ( [super respondsToSelector:@selector(draggingSourceOperationMaskForLocal:)] )
		return [super draggingSourceOperationMaskForLocal:bLocal];
	else
		return NSDragOperationNone;
}

- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)pSender
{
	id pDelegate = [self draggingDestinationDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(draggingUpdated:)])
		return [pDelegate draggingUpdated:pSender];
	else if ( [super respondsToSelector:@selector(draggingUpdated:)] )
		return [super draggingUpdated:pSender];
	else
		return NSDragOperationNone;
}

#ifdef USE_NATIVE_WINDOW

- (void)drawRect:(NSRect)aDirtyRect
{
	NSWindow *pWindow = [self window];
#ifdef USE_NATIVE_EVENTS
	if ( pWindow && [pWindow isVisible] && [self isKindOfClass:[VCLView class]] )
	{
#else	// USE_NATIVE_EVENTS
	if ( pWindow && [pWindow isVisible] && [[self className] isEqualToString:pNSViewAWTString] )
	{
		// For some strange reason, Java will ignore all drawing that we do
		// unless the color is changed in the current graphics context. Also,
		// the new color cannot be clear, white, or black since we use those
		// colors as the window background colors in our Java code so we set
		// the color to red.
		[[NSColor redColor] set];
#endif	// USE_NATIVE_EVENTS

		JavaSalFrame_drawToNSView( self, aDirtyRect );
	}
}

- (void)resetCursorRects
{
	NSWindow *pWindow = [self window];
#ifdef USE_NATIVE_EVENTS
	if ( pWindow && [pWindow isVisible] && [self isKindOfClass:[VCLView class]] )
#else	// USE_NATIVE_EVENTS
	if ( pWindow && [pWindow isVisible] && [[self className] isEqualToString:pNSViewAWTString] )
#endif	// USE_NATIVE_EVENTS
	{
		NSCursor *pCursor = JavaSalFrame_getCursor( self );
		if ( pCursor )
			[self addCursorRect:[self visibleRect] cursor:pCursor];
	}
}

#endif	// USE_NATIVE_WINDOW

- (MacOSBOOL)ignoreModifierKeysWhileDragging
{
	id pDelegate = [self draggingSourceDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(ignoreModifierKeysWhileDragging)])
		return [pDelegate ignoreModifierKeysWhileDragging];
	else if ( [super respondsToSelector:@selector(ignoreModifierKeysWhileDragging)] )
		return [super ignoreModifierKeysWhileDragging];
	else
		return NO;
}

- (id)initWithFrame:(NSRect)aFrame
{
#ifndef USE_NATIVE_EVENTS
	[VCLView swizzleSelectors:self];
#endif	// !USE_NATIVE_EVENTS

	if ( [super respondsToSelector:@selector(poseAsInitWithFrame:)] )
		[super poseAsInitWithFrame:aFrame];

#ifdef USE_NATIVE_EVENTS
	if ( [self isKindOfClass:[VCLView class]] )
	{
		mpFrame = NULL;
		mpLastKeyDownEvent = nil;
		mpPendingKeyUpEventList = new ::std::list< SalKeyEvent* >();
	}
#endif	// USE_NATIVE_EVENTS

	return self;
}

- (MacOSBOOL)isOpaque
{
#ifdef USE_NATIVE_EVENTS
	if ( [self isKindOfClass:[VCLView class]] )
#else	// USE_NATIVE_EVENTS
	if ( [[self className] isEqualToString:pNSViewAWTString] )
#endif	// USE_NATIVE_EVENTS
		return YES;
	else if ( [super respondsToSelector:@selector(poseAsIsOpaque)] )
		return [super poseAsIsOpaque];
	else
		return NO;
}

- (NSArray *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)pDropDestination
{
	id pDelegate = [self draggingSourceDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(namesOfPromisedFilesDroppedAtDestination:)])
		return [pDelegate namesOfPromisedFilesDroppedAtDestination:pDropDestination];
	else if ( [super respondsToSelector:@selector(namesOfPromisedFilesDroppedAtDestination:)] )
		return [super namesOfPromisedFilesDroppedAtDestination:pDropDestination];
	else
		return nil;
}

- (MacOSBOOL)performDragOperation:(id < NSDraggingInfo >)pSender
{
	id pDelegate = [self draggingDestinationDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(performDragOperation:)])
		return [pDelegate performDragOperation:pSender];
	else if ( [super respondsToSelector:@selector(performDragOperation:)] )
		return [super performDragOperation:pSender];
	else
		return NO;
}

- (MacOSBOOL)prepareForDragOperation:(id < NSDraggingInfo >)pSender
{
	id pDelegate = [self draggingDestinationDelegate];
	if ( pDelegate && [pDelegate respondsToSelector:@selector(prepareForDragOperation:)])
		return [pDelegate prepareForDragOperation:pSender];
	else if ( [super respondsToSelector:@selector(prepareForDragOperation:)] )
		return [super prepareForDragOperation:pSender];
	else
		return NO;
}

#if !defined USE_NATIVE_EVENTS && !defined USE_ROUNDED_BOTTOM_CORNERS_IN_JAVA_FRAMES

- (NSSize)_bottomCornerSize
{
	NSWindow *pWindow = [self window];
	if ( pWindow && [[pWindow className] isEqualToString:pCocoaAppWindowString] )
		return NSMakeSize( 0, 0 );
	else if ( [self respondsToSelector:@selector(poseAsBottomCornerSize)] )
		return [self poseAsBottomCornerSize];
	else
		return NSMakeSize( 0, 0 );
}

#endif	// !USE_NATIVE_EVENTS && !USE_ROUNDED_BOTTOM_CORNERS_IN_JAVA_FRAMES

- (MacOSBOOL)readSelectionFromPasteboard:(NSPasteboard *)pPasteboard
{
	MacOSBOOL bRet = NO;

	// Invoke superclass if this is not an NSViewAWT class
#ifdef USE_NATIVE_EVENTS
	if ( ![self isKindOfClass:[VCLView class]] )
#else	// USE_NATIVE_EVENTS
	if ( ![[self className] isEqualToString:pNSViewAWTString] )
#endif	// USE_NATIVE_EVENTS
	{
		if ( [super respondsToSelector:@selector(readSelectionFromPasteboard:)] )
			bRet = (MacOSBOOL)[super readSelectionFromPasteboard:pPasteboard];
		return bRet;
	}

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
	// Invoke superclass if this is not an NSViewAWT class
#ifdef USE_NATIVE_EVENTS
	if ( ![self isKindOfClass:[VCLView class]] )
#else	// USE_NATIVE_EVENTS
	if ( ![[self className] isEqualToString:pNSViewAWTString] )
#endif	// USE_NATIVE_EVENTS
	{
		id pRet = nil;
		if ( [super respondsToSelector:@selector(validRequestorForSendType:returnType:)] )
			pRet = [super validRequestorForSendType:pSendType returnType:pReturnType];
		return pRet;
	}

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
	if ( pDelegate && [pDelegate respondsToSelector:@selector(wantsPeriodicDraggingUpdates)])
		return [pDelegate wantsPeriodicDraggingUpdates];
	else if ( [super respondsToSelector:@selector(wantsPeriodicDraggingUpdates)] )
		return [super wantsPeriodicDraggingUpdates];
	else
		return NO;
}

- (MacOSBOOL)writeSelectionToPasteboard:(NSPasteboard *)pPasteboard types:(NSArray *)pTypes
{
	MacOSBOOL bRet = NO;

	// Invoke superclass if this is not an NSViewAWT class
#ifdef USE_NATIVE_EVENTS
	if ( ![self isKindOfClass:[VCLView class]] )
#else	// USE_NATIVE_EVENTS
	if ( ![[self className] isEqualToString:pNSViewAWTString] )
#endif	// USE_NATIVE_EVENTS
	{
		if ( [super respondsToSelector:@selector(writeSelectionToPasteboard:types:types:)] )
			bRet = (MacOSBOOL)[super writeSelectionToPasteboard:pPasteboard types:pTypes];
		return bRet;
	}

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

@interface InstallVCLEventQueueClasses : NSObject
{
#ifndef USE_NATIVE_EVENTS
	MacOSBOOL					mbUseQuickTimeContentViewHack;
#endif	// !USE_NATIVE_EVENTS
}
+ (id)create;
- (id)init;
- (void)installVCLEventQueueClasses:(id)pObject;
@end

static MacOSBOOL bVCLEventQueueClassesInitialized = NO;

@implementation InstallVCLEventQueueClasses

+ (id)create
{
	InstallVCLEventQueueClasses *pRet = [[InstallVCLEventQueueClasses alloc] init];
	[pRet autorelease];
	return pRet;
}

- (id)init
{
	[super init];

#ifndef USE_NATIVE_EVENTS
	// Fix bug 3159 by only using the QuickTime hack when running QuickTime 7.4
	// or earlier
	mbUseQuickTimeContentViewHack = NO;
	void *pLib = dlopen( NULL, RTLD_LAZY | RTLD_LOCAL );
	if ( pLib )
	{
		Gestalt_Type *pGestalt = (Gestalt_Type *)dlsym( pLib, "Gestalt" );
		if ( pGestalt )
		{
			SInt32 res = 0;
			if ( pGestalt( gestaltQuickTime, &res ) == noErr && res < 0x07500000 )
				mbUseQuickTimeContentViewHack = YES;
		}

		dlclose( pLib );
	}
#endif	// !USE_NATIVE_EVENTS

	return self;
}

- (void)installVCLEventQueueClasses:(id)pObject
{
	if ( bVCLEventQueueClassesInitialized )
		return;

	bVCLEventQueueClassesInitialized = YES;

	// Do not retain as invoking alloc disables autorelease
	pFontManagerLock = [[NSRecursiveLock alloc] init];

#ifndef USE_NATIVE_EVENTS
	// Initialize statics
	bUseQuickTimeContentViewHack = mbUseQuickTimeContentViewHack;
#endif	// !USE_NATIVE_EVENTS

	// Do not retain as invoking alloc disables autorelease
	pSharedResponder = [[VCLResponder alloc] init];

	// VCLApplication selectors

	SEL aSelector = @selector(setDelegate:);
	SEL aPoseAsSelector = @selector(poseAsSetDelegate:);
	Method aOldMethod = class_getInstanceMethod( [NSApplication class], aSelector );
	Method aNewMethod = class_getInstanceMethod( [VCLApplication class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSApplication class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	// VCLBundle selectors

	aSelector = @selector(loadNibFile:externalNameTable:withZone:);
	aPoseAsSelector = @selector(poseAsLoadNibFile:externalNameTable:withZone:);
	aOldMethod = class_getClassMethod( [NSBundle class], aSelector );
	aNewMethod = class_getClassMethod( [VCLBundle class], aSelector );
	Method aPoseAsMethod = class_getClassMethod( [VCLBundle class], aPoseAsSelector );
	if ( aOldMethod && aNewMethod && aPoseAsMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP )
		{
			method_setImplementation( aPoseAsMethod, aOldIMP );
			method_setImplementation( aOldMethod, aNewIMP );
		}
	}

	// VCLFontManager selectors

	aSelector = @selector(availableFontFamilies);
	aPoseAsSelector = @selector(poseAsAvailableFontFamilies);
	aOldMethod = class_getInstanceMethod( [NSFontManager class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLFontManager class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSFontManager class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	aSelector = @selector(availableMembersOfFontFamily:);
	aPoseAsSelector = @selector(poseAsAvailableMembersOfFontFamily:);
	aOldMethod = class_getInstanceMethod( [NSFontManager class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLFontManager class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSFontManager class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	// VCLWindow selectors

	aSelector = @selector(becomeKeyWindow);
	aPoseAsSelector = @selector(poseAsBecomeKeyWindow);
	aOldMethod = class_getInstanceMethod( [NSWindow class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
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

	aSelector = @selector(setContentView:);
	aPoseAsSelector = @selector(poseAsSetContentView:);
	aOldMethod = class_getInstanceMethod( [NSWindow class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSWindow class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	aSelector = @selector(setLevel:);
	aPoseAsSelector = @selector(poseAsSetLevel:);
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

	aSelector = @selector(initWithFrame:);
	aPoseAsSelector = @selector(poseAsInitWithFrame:);
	aOldMethod = class_getInstanceMethod( [NSView class], aSelector );
	aNewMethod = class_getInstanceMethod( [VCLView class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( [NSView class], aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}

	aSelector = @selector(isOpaque);
	aPoseAsSelector = @selector(poseAsIsOpaque);
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

#ifdef USE_NATIVE_EVENTS
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

	aSelector = @selector(windowShouldClose:);
	aNewMethod = class_getInstanceMethod( [VCLWindow class], aSelector );
	if ( aNewMethod )
	{
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aNewIMP )
			class_addMethod( [NSWindow class], aSelector, aNewIMP, method_getTypeEncoding( aNewMethod ) );
	}

#endif	// USE_NATIVE_EVENTS
#if !defined USE_NATIVE_EVENTS && !defined USE_ROUNDED_BOTTOM_CORNERS_IN_JAVA_FRAMES
	// NSThemeFrame selectors

	NSBundle *pBundle = [NSBundle bundleForClass:[NSView class]];
	if ( pBundle )
	{
		Class aClass = [pBundle classNamed:pNSThemeFrameString];
		if ( aClass )
		{
			aSelector = @selector(_bottomCornerSize);
			aPoseAsSelector = @selector(poseAsBottomCornerSize);
			aOldMethod = class_getInstanceMethod( aClass, aSelector );
			aNewMethod = class_getInstanceMethod( [VCLView class], aSelector );
			if ( aOldMethod && aNewMethod )
			{
				IMP aOldIMP = method_getImplementation( aOldMethod );
				IMP aNewIMP = method_getImplementation( aNewMethod );
				if ( aOldIMP && aNewIMP && class_addMethod( aClass, aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
					method_setImplementation( aOldMethod, aNewIMP );
			}
		}
	}

#endif	// !USE_NATIVE_EVENTS && !USE_ROUNDED_BOTTOM_CORNERS_IN_JAVA_FRAMES
#ifdef USE_NATIVE_EVENTS
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		[pApp setDelegate:nil];

		NSMenu *pMainMenu = [pApp mainMenu];
		if ( pMainMenu && [pMainMenu numberOfItems] > 0 )
		{
			NSMenuItem *pItem = [pMainMenu itemAtIndex:0];
			if ( pItem )
			{
				NSMenu *pSubmenu = [pItem submenu];
				if ( pSubmenu )
					[pSubmenu setDelegate:[VCLApplicationDelegate sharedDelegate]];
			}
		}
	}
#endif	// USE_NATIVE_EVENTS
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

void NSFontManager_acquire()
{
	if ( pFontManagerLock )
	{
		[pFontManagerLock lock];
		bFontManagerLocked = YES;
	}
}

void NSFontManager_release()
{
	if ( pFontManagerLock )
	{
		bFontManagerLocked = NO;
		[pFontManagerLock unlock];
	}
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
