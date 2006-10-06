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
 *  Copyright 2006 by Patrick Luby (patrick.luby@planamesa.com)
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

@interface VCLResponder : NSResponder
{
	NSString*				mpLastText;
	NSView*					mpView;
}
- (void)clearLastText;
- (void)dealloc;
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

	if ( mpView )
		[mpView release];

	[super dealloc];
}

- (void)doCommandBySelector:(SEL)aSelector
{
	NSString *pSelectorName = NSStringFromSelector( aSelector );
	if ( pSelectorName )
	{
		if ( [pSelectorName compare:@"cancelOperation:"] == NSOrderedSame || [pSelectorName compare:@"deleteBackward:"] == NSOrderedSame )
		{
			if ( mpView )
			{
				NSWindow *pWindow = [mpView window];
				if ( pWindow )
					VCLEventQueue_postInputMethodTextCancelled( [pWindow windowRef] );
			}
		}
	}
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

	if ( mpView )
		[mpView release];
	mpView = pView;
	if ( mpView )
		[mpView retain];

	[super interpretKeyEvents:pEvents];

	if ( mpView )
	{
		[mpView release];
		mpView = nil;
	}
}

- (NSString *)lastText
{
	return mpLastText;
}

@end

static VCLResponder *pResponder = nil;

@interface VCLView : NSView
- (void)interpretKeyEvents:(NSArray *)pEvents;
@end

@implementation VCLView

- (void)interpretKeyEvents:(NSArray *)pEvents
{
	// Fix bugs 1390 and 1619 by reprocessing any events with more than one
	// character as the JVM only seems to process the first character
	if ( pEvents )
	{
		NSEvent *pEvent = [pEvents objectAtIndex:0];
		if ( pEvent )
		{
			NSApplication *pApp = [NSApplication sharedApplication];

			if ( !pResponder )
			{
				pResponder = [[VCLResponder alloc] init];
				if ( pResponder )
					[pResponder retain];
			}

			if ( pApp && pResponder )
			{
				[pResponder interpretKeyEvents:pEvents view:self];
				NSString *pText = [(VCLResponder *)pResponder lastText];
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

	[super interpretKeyEvents:pEvents];
}

@end

@interface InstallVCLEventQueueClasses : NSObject
- (void)installVCLEventQueueClasses:(id)pObject;
@end

@implementation InstallVCLEventQueueClasses

- (void)installVCLEventQueueClasses:(id)pObject
{
	[VCLView poseAsClass:[NSView class]];
}

@end

void VCLEventQueue_installVCLEventQueueClasses()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	InstallVCLEventQueueClasses *pInstallVCLEventQueueClasses = [[InstallVCLEventQueueClasses alloc] init];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pInstallVCLEventQueueClasses performSelectorOnMainThread:@selector(installVCLEventQueueClasses:) withObject:pInstallVCLEventQueueClasses waitUntilDone:YES modes:pModes];

	[pPool release];
}
