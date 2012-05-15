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
 *  Patrick Luby, December 2008
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2008 Planamesa Inc.
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

#ifndef __VCLRESPONDER_COCOA_H__
#define __VCLRESPONDER_COCOA_H__

#include <salframe.h>

@interface VCLResponder : NSResponder
{
	short					mnLastCommandKey;
	short					mnLastModifiers;
#ifndef USE_NATIVE_EVENTS
	NSString*				mpLastText;
#endif	// !USE_NATIVE_EVENTS
	BOOL					mbNoGestures;
}
- (void)clear;
#ifndef USE_NATIVE_EVENTS
- (void)dealloc;
#endif	// !USE_NATIVE_EVENTS
- (void)deleteBackward:(id)pSender;
- (void)deleteBackwardByDecomposingPreviousCharacter:(id)pSender;
- (void)deleteForward:(id)pSender;
- (void)deleteToBeginningOfLine:(id)pSender;
- (void)deleteToBeginningOfParagraph:(id)pSender;
- (void)deleteToEndOfLine:(id)pSender;
- (void)deleteToEndOfParagraph:(id)pSender;
- (void)deleteWordBackward:(id)pSender;
- (void)deleteWordForward:(id)pSender;
- (BOOL)disableServicesMenu;
- (void)doCommandBySelector:(SEL)aSelector;
- (BOOL)ignoreTrackpadGestures;
- (id)init;
- (void)insertBacktab:(id)pSender;
- (void)insertLineBreak:(id)pSender;
- (void)insertNewline:(id)pSender;
- (void)insertParagraphSeparator:(id)pSender;
- (void)insertTab:(id)pSender;
#ifndef USE_NATIVE_EVENTS
- (void)insertText:(NSString *)pString;
- (void)interpretKeyEvents:(NSArray *)pEvents;
#endif	// !USE_NATIVE_EVENTS
- (short)lastCommandKey;
- (short)lastModifiers;
#ifndef USE_NATIVE_EVENTS
- (unsigned short)lastOriginalKeyChar;
- (short)lastOriginalModifiers;
- (NSString *)lastText;
#endif	// !USE_NATIVE_EVENTS
- (void)moveBackwardAndModifySelection:(id)pSender;
- (void)moveDown:(id)pSender;
- (void)moveForwardAndModifySelection:(id)pSender;
- (void)moveLeft:(id)pSender;
- (void)moveLeftAndModifySelection:(id)pSender;
- (void)moveRight:(id)pSender;
- (void)moveRightAndModifySelection:(id)pSender;
- (void)moveToBeginningOfLine:(id)pSender;
- (void)moveToBeginningOfParagraph:(id)pSender;
- (void)moveToEndOfLine:(id)pSender;
- (void)moveToEndOfParagraph:(id)pSender;
- (void)moveToLeftEndOfLine:(id)pSender;
- (void)moveToLeftEndOfLineAndModifySelection:(id)pSender;
- (void)moveToRightEndOfLine:(id)pSender;
- (void)moveToRightEndOfLineAndModifySelection:(id)pSender;
- (void)moveUp:(id)pSender;
- (void)moveWordBackward:(id)pSender;
- (void)moveWordBackwardAndModifySelection:(id)pSender;
- (void)moveWordForward:(id)pSender;
- (void)moveWordForwardAndModifySelection:(id)pSender;
- (void)moveWordLeft:(id)pSender;
- (void)moveWordRight:(id)pSender;
- (void)pageDown:(id)pSender;
- (void)pageUp:(id)pSender;
- (void)scrollToBeginningOfDocument:(id)pSender;
- (void)scrollToEndOfDocument:(id)pSender;
- (void)selectAll:(id)pSender;
- (void)selectLine:(id)pSender;
- (void)selectParagraph:(id)pSender;
- (void)selectWord:(id)pSender;
@end

#endif
