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

@interface IsApplicationActive : NSObject
{
	BOOL					mbActive;
}
- (id)init;
- (BOOL)isActive;
- (void)isApplicationActive:(id)pObject;
@end

@implementation IsApplicationActive

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
- (unsigned)retainCount;
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

- (unsigned)retainCount
{
	return UINT_MAX;
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
	BOOL bRet = [super performKeyEquivalent:pEvent];

	// Fix bug 1751 by responding to Command-c, Command-v, and Command-x keys
	// for non-Java windows
	if ( !bRet && pEvent && ( [pEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask ) == NSCommandKeyMask && [self isVisible] && ![[self className] isEqualToString:pCocoaAppWindowString] )
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
		if ( pEvent )
		{
			NSApplication *pApp = [NSApplication sharedApplication];
			if ( pApp && pSharedResponder )
			{
				[pSharedResponder interpretKeyEvents:pEvents view:self];

				// We still need the key entry fix if there is marked text
				// otherwise bug 1429 reoccurs
				BOOL bNeedKeyEntryFix = bUseKeyEntryFix;
				if ( !bNeedKeyEntryFix && [self respondsToSelector:@selector(hasMarkedText)] )
					bNeedKeyEntryFix = [self hasMarkedText];

				if ( bNeedKeyEntryFix )
				{
					NSString *pText = [(VCLResponder *)pSharedResponder lastText];
					if ( pText )
					{
						int nLen = [pText length];
						if ( nLen > 1 )
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
	}

	[super interpretKeyEvents:pEvents];
}

@end

@interface InstallVCLEventQueueClasses : NSObject
{
	BOOL					mbUseKeyEntryFix;
}
- (id)initWithUseKeyEntryFix:(BOOL)bUseKeyEntryFix;
- (void)installVCLEventQueueClasses:(id)pObject;
@end

@implementation InstallVCLEventQueueClasses

- (id)initWithUseKeyEntryFix:(BOOL)bUseKeyEntryFix
{
	[super init];

	mbUseKeyEntryFix = bUseKeyEntryFix;

	return self;
}

- (void)installVCLEventQueueClasses:(id)pObject
{
	pFontManagerLock = [[NSRecursiveLock alloc] init];

	// Initialize statics
	bUseKeyEntryFix = mbUseKeyEntryFix;
	pSharedResponder = [[VCLResponder alloc] init];
	if ( pSharedResponder )
		[pSharedResponder retain];

	[VCLFontManager poseAsClass:[NSFontManager class]];
	[VCLWindow poseAsClass:[NSWindow class]];
	[VCLView poseAsClass:[NSView class]];
}

@end

BOOL NSApplication_isActive()
{
	BOOL bRet = YES;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	IsApplicationActive *pIsApplicationActive = [[IsApplicationActive alloc] init];
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

void VCLEventQueue_installVCLEventQueueClasses( BOOL bUseKeyEntryFix )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	InstallVCLEventQueueClasses *pInstallVCLEventQueueClasses = [[InstallVCLEventQueueClasses alloc] initWithUseKeyEntryFix:bUseKeyEntryFix];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pInstallVCLEventQueueClasses performSelectorOnMainThread:@selector(installVCLEventQueueClasses:) withObject:pInstallVCLEventQueueClasses waitUntilDone:YES modes:pModes];

	[pPool release];
}
