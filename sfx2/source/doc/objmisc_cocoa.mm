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

#include <osl/objcutils.h>

#include "objmisc_cocoa.h"

@interface DoSetModified : NSObject
{
	NSView*	theView;
	BOOL theState;
}
+ (id)createWithState:(BOOL)state view:(NSView *)r;
- (id)initWithState:(BOOL)state view:(NSView *)r;
- (void)setModified:(id)pObject;
@end

@implementation DoSetModified
+ (id)createWithState:(BOOL)state view:(NSView *)r
{
	DoSetModified *pRet = [[DoSetModified alloc] initWithState:state view:r];
	[pRet autorelease];
	return pRet;
}

- (id)initWithState:(BOOL)state view:(NSView *)r
{
	[super init];

	theView=r;
	theState=state;

	return(self);
}

- (void)setModified:(id)pObject
{
	(void)pObject;

	NSWindow *theWin = [theView window];
	if (theWin )
		[theWin setDocumentEdited: theState];
}
@end

/**
 * Perform a SetWindowModified on an NSView that has been extracted from
 * a Cocoa window.
 */
void DoCocoaSetWindowModifiedBit( void *pView, bool isModified )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pView )
	{
		DoSetModified *pDoSetModified = [DoSetModified createWithState:((isModified) ? YES : NO) view:(NSView *)pView ];
		osl_performSelectorOnMainThread( pDoSetModified, @selector(setModified:), pDoSetModified, YES );
	}

	[pPool release];
}
