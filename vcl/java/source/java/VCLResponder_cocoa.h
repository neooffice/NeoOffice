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

#ifndef __VCLRESPONDER_COCOA_H__
#define __VCLRESPONDER_COCOA_H__

@interface VCLResponder : NSResponder
{
	short					mnLastCommandKey;
	short					mnLastModifiers;
	BOOL					mbNoGestures;
}
- (void)clear;
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
- (short)lastCommandKey;
- (short)lastModifiers;
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
