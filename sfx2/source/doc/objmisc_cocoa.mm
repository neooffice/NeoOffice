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
 *  Edward Peterlin, April 2007
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2007 Planamesa Inc.
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
