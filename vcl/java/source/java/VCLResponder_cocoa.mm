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

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include <com/sun/star/awt/Key.hdl>
#include <vcl/keycod.hxx>

#include "VCLResponder_cocoa.h"

using namespace ::com::sun::star::awt;

static unsigned short GetCurrentKeyChar()
{
	unsigned short nRet = 0;

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSEvent *pEvent = [pApp currentEvent];
		if ( pEvent && [pEvent type] == NSKeyDown )
		{
			NSString *pChars = [pEvent charactersIgnoringModifiers];
			if ( pChars && [pChars length] )
				nRet = [pChars characterAtIndex:0];
		}
	}

	return nRet;
}

static short GetCurrentKeyModifiers()
{
	short nRet = 0;

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSEvent *pEvent = [pApp currentEvent];
		if ( pEvent )
		{
			unsigned int nModifiers = [pEvent modifierFlags];
			if ( nModifiers & NSShiftKeyMask )
				nRet |= KEY_SHIFT;
			if ( nModifiers & NSControlKeyMask )
				nRet |= KEY_MOD1;
			if ( nModifiers & NSAlternateKeyMask )
				nRet |= KEY_MOD2;
			if ( nModifiers & NSCommandKeyMask )
				nRet |= KEY_MOD3;
		}
	}

	return nRet;
}

@implementation VCLResponder

- (void)clearLastText
{
	mnLastCommandKey = 0;
	mnLastModifiers = 0;

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

- (void)cancelOperation:(id)pSender
{
	// Fix bugs 2125 and 2167 by not overriding Java's handling of the cancel
	// action
	mnLastCommandKey = 0;
}

- (void)deleteBackward:(id)pSender
{
	mnLastCommandKey = KEY_BACKSPACE;
}

- (void)deleteBackwardByDecomposingPreviousCharacter:(id)pSender
{
	mnLastCommandKey = KEY_BACKSPACE;
}

- (void)deleteForward:(id)pSender
{
	mnLastCommandKey = KEY_DELETE;
}

- (void)deleteToBeginningOfLine:(id)pSender
{
	mnLastCommandKey = Key::DELETE_TO_BEGIN_OF_LINE;
}

- (void)deleteToBeginningOfParagraph:(id)pSender
{
	mnLastCommandKey = Key::DELETE_TO_BEGIN_OF_PARAGRAPH;
}

- (void)deleteToEndOfLine:(id)pSender
{
	mnLastCommandKey = Key::DELETE_TO_END_OF_LINE;
}

- (void)deleteToEndOfParagraph:(id)pSender
{
	mnLastCommandKey = Key::DELETE_TO_END_OF_PARAGRAPH;
}

- (void)deleteWordBackward:(id)pSender
{
	mnLastCommandKey = Key::DELETE_WORD_BACKWARD;
}

- (void)deleteWordForward:(id)pSender
{
	mnLastCommandKey = Key::DELETE_WORD_FORWARD;
}

- (BOOL)disableServicesMenu
{
	BOOL bDisableServicesMenu = NO;

	CFPropertyListRef aPref = CFPreferencesCopyAppValue( CFSTR( "DisableServicesMenu" ), kCFPreferencesCurrentApplication );
	if( aPref )
	{
		if ( CFGetTypeID( aPref ) == CFBooleanGetTypeID() && (CFBooleanRef)aPref == kCFBooleanTrue )
			bDisableServicesMenu = YES;
		CFRelease( aPref );
	}

	return bDisableServicesMenu;
}

- (void)doCommandBySelector:(SEL)aSelector
{
	// Do not invoke the superclass as it can trigger beeping
	if ( [self respondsToSelector:aSelector] )
		[self performSelector:aSelector withObject:nil];
}

- (BOOL)ignoreTrackpadGestures
{
	BOOL bIgnoreTrackpadGestures = NO;

	CFPropertyListRef aPref = CFPreferencesCopyAppValue( CFSTR( "IgnoreTrackpadGestures" ), kCFPreferencesCurrentApplication );
	if( aPref )
	{
		if ( CFGetTypeID( aPref ) == CFBooleanGetTypeID() && (CFBooleanRef)aPref == kCFBooleanTrue )
			bIgnoreTrackpadGestures = YES;
		CFRelease( aPref );
	}

	return bIgnoreTrackpadGestures;
}

- (id)init
{
	[super init];

	mnLastCommandKey = 0;
	mnLastModifiers = 0;
	mpLastText = nil;

	return self;
}

- (void)insertBacktab:(id)pSender
{
	mnLastCommandKey = KEY_TAB;
	mnLastModifiers = KEY_SHIFT;
}

- (void)insertLineBreak:(id)pSender
{
	mnLastCommandKey = Key::INSERT_LINEBREAK;
}

- (void)insertNewline:(id)pSender
{
	mnLastCommandKey = KEY_RETURN;
	// Fix bug 3350 by using the current event's key modifiers
	mnLastModifiers = GetCurrentKeyModifiers();
}

- (void)insertParagraphSeparator:(id)pSender
{
	mnLastCommandKey = Key::INSERT_PARAGRAPH;
}

- (void)insertTab:(id)pSender
{
	mnLastCommandKey = KEY_TAB;
}

- (void)insertText:(NSString *)pString
{
	[self clearLastText];

	mpLastText = pString;
	if ( mpLastText )
		[mpLastText retain];
}

- (void)interpretKeyEvents:(NSArray *)pEvents
{
	[self clearLastText];

	[super interpretKeyEvents:pEvents];
}

- (short)lastCommandKey
{
	return mnLastCommandKey;
}

- (short)lastModifiers
{
	return mnLastModifiers;
}

- (unsigned short)lastOriginalKeyChar
{
	return GetCurrentKeyChar();
}

- (short)lastOriginalModifiers
{
	return GetCurrentKeyModifiers();
}

- (NSString *)lastText
{
	return mpLastText;
}

- (void)moveBackwardAndModifySelection:(id)pSender
{
	mnLastCommandKey = Key::SELECT_BACKWARD;
}

- (void)moveDown:(id)pSender
{
	mnLastCommandKey = KEY_DOWN;
}

- (void)moveForwardAndModifySelection:(id)pSender
{
	mnLastCommandKey = Key::SELECT_FORWARD;
}

- (void)moveLeft:(id)pSender
{
	mnLastCommandKey = KEY_LEFT;
}

- (void)moveLeftAndModifySelection:(id)pSender
{
	mnLastCommandKey = KEY_LEFT;
	mnLastModifiers = KEY_SHIFT;
}

- (void)moveParagraphBackward:(id)pSender
{
	mnLastCommandKey = Key::MOVE_TO_BEGIN_OF_PARAGRAPH;
}

- (void)moveParagraphBackwardAndModifySelection:(id)pSender
{
	mnLastCommandKey = Key::SELECT_TO_BEGIN_OF_PARAGRAPH;
}

- (void)moveParagraphForward:(id)pSender
{
	mnLastCommandKey = Key::MOVE_TO_END_OF_PARAGRAPH;
}

- (void)moveParagraphForwardAndModifySelection:(id)pSender
{
	mnLastCommandKey = Key::SELECT_TO_END_OF_PARAGRAPH;
}

- (void)moveRight:(id)pSender
{
	mnLastCommandKey = KEY_RIGHT;
}

- (void)moveRightAndModifySelection:(id)pSender
{
	mnLastCommandKey = KEY_RIGHT;
	mnLastModifiers = KEY_SHIFT;
}

- (void)moveToBeginningOfDocument:(id)pSender
{
	mnLastCommandKey = Key::MOVE_TO_BEGIN_OF_DOCUMENT;
}

- (void)moveToBeginningOfDocumentAndModifySelection:(id)pSender
{
	mnLastCommandKey = Key::SELECT_TO_BEGIN_OF_DOCUMENT;
}

- (void)moveToBeginningOfLine:(id)pSender
{
	mnLastCommandKey = Key::MOVE_TO_BEGIN_OF_LINE;
}

- (void)moveToBeginningOfLineAndModifySelection:(id)pSender
{
	mnLastCommandKey = Key::SELECT_TO_BEGIN_OF_LINE;
}

- (void)moveToBeginningOfParagraph:(id)pSender
{
	mnLastCommandKey = Key::MOVE_TO_BEGIN_OF_PARAGRAPH;
}

- (void)moveToBeginningOfParagraphAndModifySelection:(id)pSender
{
	mnLastCommandKey = Key::SELECT_TO_BEGIN_OF_PARAGRAPH;
}

- (void)moveToEndOfDocument:(id)pSender
{
	mnLastCommandKey = Key::MOVE_TO_END_OF_DOCUMENT;
}

- (void)moveToEndOfDocumentAndModifySelection:(id)pSender
{
	mnLastCommandKey = Key::SELECT_TO_END_OF_DOCUMENT;
}

- (void)moveToEndOfLine:(id)pSender
{
	mnLastCommandKey = Key::MOVE_TO_END_OF_LINE;
}

- (void)moveToEndOfLineAndModifySelection:(id)pSender
{
	mnLastCommandKey = Key::SELECT_TO_END_OF_LINE;
}

- (void)moveToEndOfParagraph:(id)pSender
{
	mnLastCommandKey = Key::MOVE_TO_END_OF_PARAGRAPH;
}

- (void)moveToEndOfParagraphAndModifySelection:(id)pSender
{
	mnLastCommandKey = Key::SELECT_TO_END_OF_PARAGRAPH;
}

- (void)moveUp:(id)pSender
{
	mnLastCommandKey = KEY_UP;
}

- (void)moveWordBackward:(id)pSender
{
	mnLastCommandKey = Key::MOVE_WORD_BACKWARD;
}

- (void)moveWordBackwardAndModifySelection:(id)pSender
{
	mnLastCommandKey = Key::SELECT_WORD_BACKWARD;
}

- (void)moveWordForward:(id)pSender
{
	mnLastCommandKey = Key::MOVE_WORD_FORWARD;
}

- (void)moveWordForwardAndModifySelection:(id)pSender
{
	mnLastCommandKey = Key::SELECT_WORD_FORWARD;
}

- (void)moveWordLeft:(id)pSender
{
	mnLastCommandKey = Key::MOVE_WORD_BACKWARD;
}

- (void)moveWordLeftAndModifySelection:(id)pSender
{
	mnLastCommandKey = Key::SELECT_WORD_BACKWARD;
}

- (void)moveWordRight:(id)pSender
{
	mnLastCommandKey = Key::MOVE_WORD_FORWARD;
}

- (void)moveWordRightAndModifySelection:(id)pSender
{
	mnLastCommandKey = Key::SELECT_WORD_FORWARD;
}

- (void)pageDown:(id)pSender
{
	mnLastCommandKey = KEY_PAGEDOWN;
}

- (void)pageUp:(id)pSender
{
	mnLastCommandKey = KEY_PAGEUP;
}

- (void)selectAll:(id)pSender
{
	mnLastCommandKey = Key::SELECT_ALL;
}

- (void)selectLine:(id)pSender
{
	mnLastCommandKey = Key::SELECT_LINE;
}

- (void)selectParagraph:(id)pSender
{
	mnLastCommandKey = Key::SELECT_PARAGRAPH;
}

- (void)selectWord:(id)pSender
{
	mnLastCommandKey = Key::SELECT_WORD;
}

@end
