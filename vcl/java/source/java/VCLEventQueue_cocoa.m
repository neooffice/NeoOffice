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

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#import "VCLEventQueue_cocoa.h"
#import "VCLGraphics_cocoa.h"

static BOOL bFontManagerLocked = NO;
static NSRecursiveLock *pFontManagerLock = nil;
static NSString *pCocoaAppWindowString = @"CocoaAppWindow";
static NSString *pNSWindowViewAWTString = @"NSWindowViewAWT";

inline long Float32ToLong( Float32 f ) { return (long)( f < 0 ? f - 1.0 : f + 0.5 ); }

@interface NSObject (ApplicationHasDelegate)
- (void)cancelTermination;
@end

@interface ApplicationHasDelegate : NSObject
{
	BOOL					mbDelegate;
}
+ (id)create;
- (void)applicationHasDelegate:(id)pObject;
- (BOOL)hasDelegate;
- (id)init;
@end

@implementation ApplicationHasDelegate

+ (id)create
{
	ApplicationHasDelegate *pRet = [[ApplicationHasDelegate alloc] init];
	[pRet autorelease];
	return pRet;
}

- (void)applicationHasDelegate:(id)pObject
{
	// Check that our custom delegate is the delegate
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
		mbDelegate = ( [pApp delegate] && [[pApp delegate] respondsToSelector:@selector(cancelTermination)] );
}

- (BOOL)hasDelegate
{
	return mbDelegate;
}

- (id)init
{
	[super init];

	mbDelegate = NO;

	return self;
}

@end

@interface IsApplicationActive : NSObject
{
	BOOL					mbActive;
}
+ (id)create;
- (id)init;
- (BOOL)isActive;
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

- (BOOL)isActive
{
	return mbActive;
}

- (void)isApplicationActive:(id)pObject
{
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
		mbActive = ( [pApp isActive] && ![pApp modalWindow] );
}

@end

// Fix for bugs 1685, 1694, and 1859. Java 1.5 and higher will arbitrarily
// change the selected font by creating a new font from the font's family
// name and style. We fix these bugs by prepending the font names to the
// list of font family names so that Java will think that each font's
// family name is the same as its font name.

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
		pRet = [NSMutableArray arrayWithArray:[super availableFontFamilies]];
	}
	else
	{
		pRet = [NSMutableArray arrayWithArray:[super availableFonts]];
		if ( pRet )
			[pRet addObjectsFromArray:[super availableFontFamilies]];
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
		pRet = [super availableMembersOfFontFamily:family];
	}
	else
	{
		NSFont *pFont = [NSFont fontWithName:family size:12];
		if ( pFont )
		{
			NSMutableArray *pFontEntries = [NSMutableArray arrayWithCapacity:4];
			if ( pFontEntries )
			{
				[pFontEntries addObject:[pFont fontName]];
				[pFontEntries addObject:@""];
				[pFontEntries addObject:[NSNumber numberWithInt:[self weightOfFont:pFont]]];
				[pFontEntries addObject:[NSNumber numberWithUnsignedInt:[self traitsOfFont:pFont]]];
				pRet = [NSArray arrayWithObject:pFontEntries];
			}
		}
	}

	if ( pFontManagerLock )
		[pFontManagerLock unlock];

	return pRet;
}

@end

static NSString *pCancelInputMethodText = @" ";

@interface NSResponder (VCLResponder)
- (void)abandonInput;
- (void)copy:(id)pSender;
- (void)cut:(id)pSender;
- (BOOL)hasMarkedText;
- (void)paste:(id)pSender;
@end

@interface VCLResponder : NSResponder
{
	NSString*				mpLastText;
	NSView*					mpView;
}
- (void)clearLastText;
- (void)dealloc;
- (void)doCommandBySelector:(SEL)aSelector;
- (id)init;
- (void)insertText:(NSString *)pString;
- (void)interpretKeyEvents:(NSArray *)pEvents view:(NSView *)pView;
- (NSString *)lastText;
@end

@implementation VCLResponder

- (void)clearLastText
{
	if ( mpLastText )
	{
		[mpLastText release];
		mpLastText = nil;
	}
}

- (void)dealloc
{
	[self clearLastText];

	[super dealloc];
}

- (void)doCommandBySelector:(SEL)aSelector
{
	// Fix bugs 2125 and 2167 by not overriding Java's handling of the cancel
	// action
}

- (id)init
{
	[super init];

	mpLastText = nil;
	mpView = nil;

	return self;
}

- (void)insertText:(NSString *)pString
{
	[self clearLastText];

	mpLastText = pString;
	if ( mpLastText )
		[mpLastText retain];
}

- (void)interpretKeyEvents:(NSArray *)pEvents view:(NSView *)pView
{
	[self clearLastText];

	mpView = pView;
	[super interpretKeyEvents:pEvents];
	mpView = nil;
}

- (NSString *)lastText
{
	return mpLastText;
}

@end

static BOOL bUseKeyEntryFix = NO;
static BOOL bUsePartialKeyEntryFix = NO;
static BOOL bUseQuickTimeContentViewHack = NO;
static VCLResponder *pSharedResponder = nil;

@interface VCLView : NSView
- (void)interpretKeyEvents:(NSArray *)pEvents;
@end

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
	BOOL bHandled = NO;

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

@interface VCLWindow : NSWindow
- (void)becomeKeyWindow;
- (void)displayIfNeeded;
- (BOOL)makeFirstResponder:(NSResponder *)pResponder;
- (void)makeKeyAndOrderFront:(id)pSender;
- (void)makeKeyWindow;
- (BOOL)performKeyEquivalent:(NSEvent *)pEvent;
- (void)resignKeyWindow;
- (void)sendEvent:(NSEvent *)pEvent;
- (void)setContentView:(NSView *)pView;
@end

@interface VCLWindow (CocoaAppWindow)
- (jobject)peer;
@end

@implementation VCLWindow

- (void)becomeKeyWindow
{
	[super becomeKeyWindow];

	// Fix bug 1819 by forcing cancellation of the input method
	if ( [self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] )
	{
		NSResponder *pResponder = [self firstResponder];
		if ( pResponder && [pResponder respondsToSelector:@selector(abandonInput)] && [pResponder respondsToSelector:@selector(hasMarkedText)] && [pResponder respondsToSelector:@selector(insertText:)] )
		{
			if ( [pResponder hasMarkedText] )
				[pResponder insertText:pCancelInputMethodText];
			[pResponder abandonInput];
		}
	}
}

- (void)displayIfNeeded
{
	// Fix bug 2151 by not allowing any updates if the window is hidden
	if ( ![self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] )
		return;

	[super displayIfNeeded];
}

- (BOOL)makeFirstResponder:(NSResponder *)pResponder
{
	NSResponder *pOldResponder = [self firstResponder];
	BOOL bRet = [super makeFirstResponder:pResponder];

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

	return bRet;
}

- (void)makeKeyAndOrderFront:(id)pSender
{
	if ( [self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] )
	{
		// Fix bug 2992 by not allowing the key window to change when we are
		// tracking the menubar
		MenuTrackingData aTrackingData;
		if ( GetMenuTrackingData( nil, &aTrackingData ) == noErr )
		{
			[self orderFront:pSender];
			return;
		}
	}

	[super makeKeyAndOrderFront:pSender];
}

- (void)makeKeyWindow
{
	if ( [self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] )
	{
		// Fix bug 2992 by not allowing the key window to change when we are
		// tracking the menubar
		MenuTrackingData aTrackingData;
		if ( GetMenuTrackingData( nil, &aTrackingData ) == noErr )
			return;
	}

	[super makeKeyWindow];
}

- (void)resignKeyWindow
{
	// Fix bug 1819 by forcing cancellation of the input method
	if ( [self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] )
	{
		NSResponder *pResponder = [self firstResponder];
		if ( pResponder && [pResponder respondsToSelector:@selector(abandonInput)] && [pResponder respondsToSelector:@selector(hasMarkedText)] && [pResponder respondsToSelector:@selector(insertText:)] )
		{
			if ( [pResponder hasMarkedText] )
				[pResponder insertText:pCancelInputMethodText];
			[pResponder abandonInput];
		}
	}

	[super resignKeyWindow];
}

- (BOOL)performKeyEquivalent:(NSEvent *)pEvent
{
	BOOL bCommandKeyPressed = ( pEvent && ( [pEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask ) == NSCommandKeyMask );

	if ( bCommandKeyPressed && [self isVisible] && [[self className] isEqualToString:pCocoaAppWindowString] )
	{
		// Fix crashing when using a menu shortcut by forcing cancellation of
		// the input method
		NSResponder *pResponder = [self firstResponder];
		if ( pResponder && [pResponder respondsToSelector:@selector(abandonInput)] && [pResponder respondsToSelector:@selector(hasMarkedText)] && [pResponder respondsToSelector:@selector(insertText:)] )
		{
			// Fix bug 2783 by not cancelling the input method if the command
			// key is pressed, but instead, returning YES to cancel the menu
			// matching process
			if ( [pResponder hasMarkedText] )
				return YES;
		}

		// Implement the standard window minimization behavior with the
		// Command-m event
		if ( [self styleMask] & NSMiniaturizableWindowMask )
		{
			NSString *pChars = [pEvent charactersIgnoringModifiers];
			if ( pChars && [pChars isEqualToString:@"m"] )
			{
				[self miniaturize:self];
				return YES;
			}
		}
	}

	BOOL bRet = [super performKeyEquivalent:pEvent];

	// Fix bug 1751 by responding to Command-c, Command-v, and Command-x keys
	// for non-Java windows
	if ( !bRet && bCommandKeyPressed && [self isVisible] && ![[self className] isEqualToString:pCocoaAppWindowString] )
	{
		NSString *pChars = [pEvent charactersIgnoringModifiers];
		NSResponder *pResponder = [self firstResponder];
		if ( pChars && pResponder )
		{
			if ( [pChars isEqualToString:@"c"] && [pResponder respondsToSelector:@selector(copy:)] )
			{
				[pResponder copy:self];
				bRet = YES;
			}
			else if ( [pChars isEqualToString:@"v"] && [pResponder respondsToSelector:@selector(paste:)] )
			{
				[pResponder paste:self];
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

- (void)sendEvent:(NSEvent *)pEvent
{
	[super sendEvent:pEvent];

	if ( pEvent && [pEvent type] == NSScrollWheel && [[self className] isEqualToString:pCocoaAppWindowString] && [self respondsToSelector:@selector(peer)] )
	{
		// Post flipped coordinates 
		NSRect aFrame = [self frame];
		NSRect aContentFrame = [self contentRectForFrameRect:aFrame];
		float fLeftInset = aFrame.origin.x - aContentFrame.origin.x;
		float fTopInset = aFrame.origin.y + aFrame.size.height - aContentFrame.origin.y - aContentFrame.size.height;
		NSPoint aLocation = [pEvent locationInWindow];
		VCLEventQueue_postMouseWheelEvent( [self peer], (long)( aLocation.x - fLeftInset ), (long)( aFrame.size.height - aLocation.y - fTopInset ), Float32ToLong( [pEvent deltaX] ), Float32ToLong( [pEvent deltaY] ) * -1 );
	}
}

- (void)setContentView:(NSView *)pView
{
	[super setContentView:pView];

	// It was found that with QuickTime 7.4 on G4 systems running ATI RAGE 128
	// graphics cards, QTMovieView will misplace the movie if the window's
	// content view is flipped. Since Java replaces the default content view
	// with a flipped view, we need to push their content view down a level
	// and make the content view unflipped. Note that this approach causes
	// Java to lose window events on 10.3.9 JVMs so don't allow use of this
	// approach on Mac OS X 10.3.x.
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

				[super setContentView:pNewContentView];
				aFrame.origin.x = 0;
				aFrame.origin.y = 0;
				[pContentView setFrame:aFrame];
				[pNewContentView addSubview:pContentView positioned:NSWindowAbove relativeTo:nil];

				[pNewContentView release];
			}
		}
	}
}

@end

@implementation VCLView

- (void)interpretKeyEvents:(NSArray *)pEvents
{
	// Fix bugs 1390 and 1619 by reprocessing any events with more than one
	// character as the JVM only seems to process the first character
	NSWindow *pWindow = [self window];
	if ( pEvents && pWindow && [pWindow isVisible] && [[pWindow className] isEqualToString:pCocoaAppWindowString] )
	{
		NSEvent *pEvent = [pEvents objectAtIndex:0];
		if ( pEvent && pSharedResponder )
		{
			BOOL bNeedKeyEntryFix = bUseKeyEntryFix;

			// We still need the key entry fix if there is marked text
			// otherwise bug 1429 reoccurs
			if ( !bNeedKeyEntryFix && bUsePartialKeyEntryFix && [self respondsToSelector:@selector(hasMarkedText)] )
				bNeedKeyEntryFix = [self hasMarkedText];

			if ( bNeedKeyEntryFix )
			{
				[pSharedResponder interpretKeyEvents:pEvents view:self];

				NSString *pText = [(VCLResponder *)pSharedResponder lastText];
				if ( pText )
				{
					NSApplication *pApp = [NSApplication sharedApplication];
					int nLen = [pText length];
					if ( pApp && nLen > 1 )
					{
						unichar pChars[ nLen ];
						[pText getCharacters:pChars];

						int i = 1;
						for ( ; i < nLen; i++ )
						{
							NSString *pChar = [NSString stringWithCharacters:&pChars[i] length:1];
							if ( pChar )
							{
								NSEvent *pNewEvent = [NSEvent keyEventWithType:[pEvent type] location:[pEvent locationInWindow] modifierFlags:[pEvent modifierFlags] timestamp:[pEvent timestamp] windowNumber:[pEvent windowNumber] context:[pEvent context] characters:pChar charactersIgnoringModifiers:pChar isARepeat:[pEvent isARepeat] keyCode:0];
								if ( pNewEvent )
									[pApp postEvent:pNewEvent atStart:YES];
							}
						}
					}
				}
			}
		}
	}

	[super interpretKeyEvents:pEvents];
}

@end

@interface InstallVCLEventQueueClasses : NSObject
{
	BOOL					mbUseKeyEntryFix;
	BOOL					mbUsePartialKeyEntryFix;
	BOOL					mbUseQuickTimeContentViewHack;
}
+ (id)installWithUseKeyEntryFix:(BOOL)bUseKeyEntryFix usePartialKeyEntryFix:(BOOL)bUsePartialKeyEntryFix useQuickTimeContentViewHack:(BOOL)bUseQuickTimeContentViewHack;
- (id)initWithUseKeyEntryFix:(BOOL)bUseKeyEntryFix usePartialKeyEntryFix:(BOOL)bUsePartialKeyEntryFix useQuickTimeContentViewHack:(BOOL)bUseQuickTimeContentViewHack;
- (void)installVCLEventQueueClasses:(id)pObject;
@end

@implementation InstallVCLEventQueueClasses

+ (id)installWithUseKeyEntryFix:(BOOL)bUseKeyEntryFix usePartialKeyEntryFix:(BOOL)bUsePartialKeyEntryFix useQuickTimeContentViewHack:(BOOL)bUseQuickTimeContentViewHack
{
	InstallVCLEventQueueClasses *pRet = [[InstallVCLEventQueueClasses alloc] initWithUseKeyEntryFix:bUseKeyEntryFix usePartialKeyEntryFix:bUsePartialKeyEntryFix useQuickTimeContentViewHack:bUseQuickTimeContentViewHack];
	[pRet autorelease];
	return pRet;
}

- (id)initWithUseKeyEntryFix:(BOOL)bUseKeyEntryFix usePartialKeyEntryFix:(BOOL)bUsePartialKeyEntryFix useQuickTimeContentViewHack:(BOOL)bUseQuickTimeContentViewHack
{
	[super init];

	mbUseKeyEntryFix = bUseKeyEntryFix;
	mbUsePartialKeyEntryFix = bUsePartialKeyEntryFix;
	mbUseQuickTimeContentViewHack = bUseQuickTimeContentViewHack;

	return self;
}

- (void)installVCLEventQueueClasses:(id)pObject
{
	// Do not retain as invoking alloc disables autorelease
	pFontManagerLock = [[NSRecursiveLock alloc] init];

	// Initialize statics
	bUseKeyEntryFix = mbUseKeyEntryFix;
	bUsePartialKeyEntryFix = mbUsePartialKeyEntryFix;
	bUseQuickTimeContentViewHack = mbUseQuickTimeContentViewHack;
	if ( bUseKeyEntryFix || bUsePartialKeyEntryFix )
	{
		// Do not retain as invoking alloc disables autorelease
		pSharedResponder = [[VCLResponder alloc] init];
	}

	[VCLFontManager poseAsClass:[NSFontManager class]];
	[VCLWindow poseAsClass:[NSWindow class]];
	[VCLView poseAsClass:[NSView class]];
}

@end

BOOL NSApplication_hasDelegate()
{
	BOOL bRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	ApplicationHasDelegate *pApplicationHasDelegate = [ApplicationHasDelegate create];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pApplicationHasDelegate performSelectorOnMainThread:@selector(applicationHasDelegate:) withObject:pApplicationHasDelegate waitUntilDone:YES modes:pModes];
	bRet = [pApplicationHasDelegate hasDelegate];

	[pPool release];

	return bRet;
}

BOOL NSApplication_isActive()
{
	BOOL bRet = YES;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	IsApplicationActive *pIsApplicationActive = [IsApplicationActive create];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pIsApplicationActive performSelectorOnMainThread:@selector(isApplicationActive:) withObject:pIsApplicationActive waitUntilDone:YES modes:pModes];
	bRet = [pIsApplicationActive isActive];

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

void VCLEventQueue_installVCLEventQueueClasses( BOOL bUseKeyEntryFix, BOOL bUsePartialKeyEntryFix, BOOL bUseQuickTimeContentViewHack )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	InstallVCLEventQueueClasses *pInstallVCLEventQueueClasses = [InstallVCLEventQueueClasses installWithUseKeyEntryFix:bUseKeyEntryFix usePartialKeyEntryFix:bUsePartialKeyEntryFix useQuickTimeContentViewHack:bUseQuickTimeContentViewHack];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pInstallVCLEventQueueClasses performSelectorOnMainThread:@selector(installVCLEventQueueClasses:) withObject:pInstallVCLEventQueueClasses waitUntilDone:YES modes:pModes];

	[pPool release];
}
