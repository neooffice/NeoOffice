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

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include <vcl/keycod.hxx>
#include <com/sun/star/awt/Key.hdl>

#include "VCLResponder_cocoa.h"

using namespace ::com::sun::star::awt;

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
			if ( nModifiers & NSEventModifierFlagShift )
				nRet |= KEY_SHIFT;
			if ( nModifiers & NSEventModifierFlagControl )
				nRet |= KEY_MOD1;
			if ( nModifiers & NSEventModifierFlagOption )
				nRet |= KEY_MOD2;
			if ( nModifiers & NSEventModifierFlagCommand )
				nRet |= KEY_MOD3;
		}
	}

	return nRet;
}

@implementation VCLResponder

- (void)clear
{
	mnLastCommandKey = 0;
	mnLastModifiers = 0;
}

- (void)cancelOperation:(id)pSender
{
	(void)pSender;

	// Fix bugs 2125 and 2167 by not overriding Java's handling of the cancel
	// action
	mnLastCommandKey = 0;
}

- (void)deleteBackward:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = KEY_BACKSPACE;
}

- (void)deleteBackwardByDecomposingPreviousCharacter:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = KEY_BACKSPACE;
}

- (void)deleteForward:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = KEY_DELETE;
}

- (void)deleteToBeginningOfLine:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::DELETE_TO_BEGIN_OF_LINE;
}

- (void)deleteToBeginningOfParagraph:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::DELETE_TO_BEGIN_OF_PARAGRAPH;
}

- (void)deleteToEndOfLine:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::DELETE_TO_END_OF_LINE;
}

- (void)deleteToEndOfParagraph:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::DELETE_TO_END_OF_PARAGRAPH;
}

- (void)deleteWordBackward:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::DELETE_WORD_BACKWARD;
}

- (void)deleteWordForward:(id)pSender
{
	(void)pSender;

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
	[self clear];

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

	return self;
}

- (void)insertBacktab:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = KEY_TAB;
	mnLastModifiers = KEY_SHIFT;
}

- (void)insertLineBreak:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::INSERT_LINEBREAK;
}

- (void)insertNewline:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = KEY_RETURN;

	// Fix bug 3350 by using the current event's key modifiers
	mnLastModifiers = GetCurrentKeyModifiers();
}

- (void)insertParagraphSeparator:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::INSERT_PARAGRAPH;
}

- (void)insertTab:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = KEY_TAB;
}

- (short)lastCommandKey
{
	return mnLastCommandKey;
}

- (short)lastModifiers
{
	return mnLastModifiers;
}

- (void)moveBackwardAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_BACKWARD;
}

- (void)moveDown:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = KEY_DOWN;
}

- (void)moveForwardAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_FORWARD;
}

- (void)moveLeft:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = KEY_LEFT;
}

- (void)moveLeftAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = KEY_LEFT;
	mnLastModifiers = KEY_SHIFT;
}

- (void)moveParagraphBackward:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_TO_BEGIN_OF_PARAGRAPH;
}

- (void)moveParagraphBackwardAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_TO_BEGIN_OF_PARAGRAPH;
}

- (void)moveParagraphForward:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_TO_END_OF_PARAGRAPH;
}

- (void)moveParagraphForwardAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_TO_END_OF_PARAGRAPH;
}

- (void)moveRight:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = KEY_RIGHT;
}

- (void)moveRightAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = KEY_RIGHT;
	mnLastModifiers = KEY_SHIFT;
}

- (void)moveToBeginningOfDocument:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_TO_BEGIN_OF_DOCUMENT;
}

- (void)moveToBeginningOfDocumentAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_TO_BEGIN_OF_DOCUMENT;
}

- (void)moveToBeginningOfLine:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_TO_BEGIN_OF_LINE;
}

- (void)moveToBeginningOfLineAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_TO_BEGIN_OF_LINE;
}

- (void)moveToBeginningOfParagraph:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_TO_BEGIN_OF_PARAGRAPH;
}

- (void)moveToBeginningOfParagraphAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_TO_BEGIN_OF_PARAGRAPH;
}

- (void)moveToEndOfDocument:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_TO_END_OF_DOCUMENT;
}

- (void)moveToEndOfDocumentAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_TO_END_OF_DOCUMENT;
}

- (void)moveToEndOfLine:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_TO_END_OF_LINE;
}

- (void)moveToEndOfLineAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_TO_END_OF_LINE;
}

- (void)moveToEndOfParagraph:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_TO_END_OF_PARAGRAPH;
}

- (void)moveToEndOfParagraphAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_TO_END_OF_PARAGRAPH;
}

- (void)moveToLeftEndOfLine:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_TO_BEGIN_OF_LINE;
}

- (void)moveToLeftEndOfLineAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_TO_BEGIN_OF_LINE;
}

- (void)moveToRightEndOfLine:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_TO_END_OF_LINE;
}

- (void)moveToRightEndOfLineAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_TO_END_OF_LINE;
}

- (void)moveUp:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = KEY_UP;
}

- (void)moveWordBackward:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_WORD_BACKWARD;
}

- (void)moveWordBackwardAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_WORD_BACKWARD;
}

- (void)moveWordForward:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_WORD_FORWARD;
}

- (void)moveWordForwardAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_WORD_FORWARD;
}

- (void)moveWordLeft:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_WORD_BACKWARD;
}

- (void)moveWordLeftAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_WORD_BACKWARD;
}

- (void)moveWordRight:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_WORD_FORWARD;
}

- (void)moveWordRightAndModifySelection:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_WORD_FORWARD;
}

- (void)pageDown:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = KEY_PAGEDOWN;
}

- (void)pageUp:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = KEY_PAGEUP;
}

- (void)scrollToBeginningOfDocument:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_TO_BEGIN_OF_DOCUMENT;
}

- (void)scrollToEndOfDocument:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::MOVE_TO_END_OF_DOCUMENT;
}

- (void)selectAll:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_ALL;
}

- (void)selectLine:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_LINE;
}

- (void)selectParagraph:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_PARAGRAPH;
}

- (void)selectWord:(id)pSender
{
	(void)pSender;

	mnLastCommandKey = Key::SELECT_WORD;
}

@end
