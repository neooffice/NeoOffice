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
#import "VCLEventQueue_cocoa.h"
#import "VCLGraphics_cocoa.h"

static BOOL bFontManagerLocked = NO;
static NSRecursiveLock *pFontManagerLock = nil;
static NSString *pCocoaAppWindowString = @"CocoaAppWindow";

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

const static NSString *pCancelInputMethodText = @" ";

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

@interface VCLWindow : NSWindow
- (void)becomeKeyWindow;
- (void)displayIfNeeded;
- (BOOL)makeFirstResponder:(NSResponder *)pResponder;
- (BOOL)performKeyEquivalent:(NSEvent *)pEvent;
- (void)resignKeyWindow;
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

@end

static BOOL bUseKeyEntryFix = NO;
static BOOL bUsePartialKeyEntryFix = NO;
static VCLResponder *pSharedResponder = nil;

@interface VCLView : NSView
- (void)interpretKeyEvents:(NSArray *)pEvents;
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
}
+ (id)installWithUseKeyEntryFix:(BOOL)bUseKeyEntryFix usePartialKeyEntryFix:(BOOL)bUsePartialKeyEntryFix;
- (id)initWithUseKeyEntryFix:(BOOL)bUseKeyEntryFix usePartialKeyEntryFix:(BOOL)bUsePartialKeyEntryFix;
- (void)installVCLEventQueueClasses:(id)pObject;
@end

@implementation InstallVCLEventQueueClasses

+ (id)installWithUseKeyEntryFix:(BOOL)bUseKeyEntryFix usePartialKeyEntryFix:(BOOL)bUsePartialKeyEntryFix
{
	InstallVCLEventQueueClasses *pRet = [[InstallVCLEventQueueClasses alloc] initWithUseKeyEntryFix:bUseKeyEntryFix usePartialKeyEntryFix:bUsePartialKeyEntryFix];
	[pRet autorelease];
	return pRet;
}

- (id)initWithUseKeyEntryFix:(BOOL)bUseKeyEntryFix usePartialKeyEntryFix:(BOOL)bUsePartialKeyEntryFix
{
	[super init];

	mbUseKeyEntryFix = bUseKeyEntryFix;
	mbUsePartialKeyEntryFix = bUsePartialKeyEntryFix;

	return self;
}

- (void)installVCLEventQueueClasses:(id)pObject
{
	// Do not retain as invoking alloc disables autorelease
	pFontManagerLock = [[NSRecursiveLock alloc] init];

	// Initialize statics
	bUseKeyEntryFix = mbUseKeyEntryFix;
	bUsePartialKeyEntryFix = mbUsePartialKeyEntryFix;
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

void VCLEventQueue_installVCLEventQueueClasses( BOOL bUseKeyEntryFix, BOOL bUsePartialKeyEntryFix )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	InstallVCLEventQueueClasses *pInstallVCLEventQueueClasses = [InstallVCLEventQueueClasses installWithUseKeyEntryFix:bUseKeyEntryFix usePartialKeyEntryFix:bUsePartialKeyEntryFix];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pInstallVCLEventQueueClasses performSelectorOnMainThread:@selector(installVCLEventQueueClasses:) withObject:pInstallVCLEventQueueClasses waitUntilDone:YES modes:pModes];

	[pPool release];
}
