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
 *  Patrick Luby, September 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 Planamesa Inc.
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
#import "recently_used_file_handler_cocoa.h"

@interface NoteNewRecentDocumentURL : NSObject
{
	NSURL*				mpURL;
}
+ (id)createWithURL:(NSURL *)pURL;
- (void)dealloc;
- (id)initWithURL:(NSURL *)pURL;
- (void)noteNewRecentDocumentURL:(id)pObject;
@end

@implementation NoteNewRecentDocumentURL

+ (id)createWithURL:(NSURL *)pURL
{
	NoteNewRecentDocumentURL *pRet = [[NoteNewRecentDocumentURL alloc] initWithURL:pURL];
	[pRet autorelease];
	return pRet;
}

- (void)dealloc
{
	if ( mpURL )
		[mpURL release];

	[super dealloc];
}

- (id)initWithURL:(NSURL *)pURL
{
	[super init];

	mpURL = pURL;
	if ( mpURL )
		[mpURL retain];
 
	return self;
}

- (void)noteNewRecentDocumentURL:(id)pObject
{
	if ( mpURL )
	{
		NSDocumentController *pController = [NSDocumentController sharedDocumentController];
		if ( pController )
			[pController noteNewRecentDocumentURL:mpURL];
	}
}

@end

void NSDocumentController_noteNewRecentDocumentURL( CFStringRef aString )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( aString )
	{
		NoteNewRecentDocumentURL *pNoteNewRecentDocumentURL = [NoteNewRecentDocumentURL createWithURL:[NSURL URLWithString:(NSString *)aString]];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pNoteNewRecentDocumentURL performSelectorOnMainThread:@selector(noteNewRecentDocumentURL:) withObject:pNoteNewRecentDocumentURL waitUntilDone:NO modes:pModes];
	}

	[pPool release];
}
