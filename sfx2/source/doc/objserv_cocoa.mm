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
 *  Patrick Luby, May 2014
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2014 Planamesa Inc.
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

#include <sfx2/objsh.hxx>
#include <sfx2/sfxsids.hrc>
#include <vcl/svapp.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#import <objc/objc-runtime.h>
#include <postmac.h>

#import "objserv_cocoa.h"

@interface ShowSaveDisabledDialog : NSObject
{
}
+ (id)create;
- (id)init;
- (void)showSaveDisabledDialog:(id)pObject;
@end

@implementation ShowSaveDisabledDialog

+ (id)create
{
	ShowSaveDisabledDialog *pRet = [[ShowSaveDisabledDialog alloc] init];
	[pRet autorelease];
	return pRet;
}

- (id)init
{
	[super init];

	return self;
}

- (void)showSaveDisabledDialog:(id)pObject
{
	NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
	NSURL *pURL = [NSURL URLWithString:(NSString *)CFSTR( PRODUCT_MAC_APP_STORE_URL )];
	if ( pURL && ![@"macappstores" isEqualToString:[pURL scheme]] )
		pURL = nil;

	NSString *pMessage = @"Free Edition cannot save documents.";
	NSAlert *pAlert;
	if ( pWorkspace && pURL )
		pAlert = [NSAlert alertWithMessageText:pMessage defaultButton:@"Download" alternateButton:@"Cancel" otherButton:nil informativeTextWithFormat:@"To save documents, download on the Mac\u00A0App\u00A0Store."];
	else
		pAlert = [NSAlert alertWithMessageText:pMessage defaultButton:@"OK" alternateButton:nil otherButton:nil informativeTextWithFormat:@""];

	if ( pAlert )
	{
		NSArray *pButtons = [pAlert buttons];
		if ( [pButtons count] > 1 )
		{
			NSButton *pAlternateButton = [pButtons objectAtIndex:1];
			if ( pAlternateButton )
			{
				unichar cEscapeChar = 0x1b;
				NSString *pEscapeKey = [NSString stringWithCharacters:&cEscapeChar length:1];
				if ( pEscapeKey )
					[pAlternateButton setKeyEquivalent:pEscapeKey];
			}
		}

		if ( [pAlert runModal] == NSAlertDefaultReturn && pWorkspace && pURL )
			[pWorkspace openURL:pURL];
	}
}

@end

sal_Bool SfxObjectShell_canSave( SfxObjectShell *pObjShell, USHORT nID )
{
	sal_Bool bRet = sal_True;

	if ( pObjShell && ( nID == SID_SAVEDOC || nID == SID_SAVEASDOC ) )
	{
		char *env = getenv( "SAL_ENABLE_MAS" );
		if ( !env || strcmp( env, "1" ) )
		{
			bRet = sal_False;

			NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

			ShowSaveDisabledDialog *pShowSaveDisabledDialog = [ShowSaveDisabledDialog create];
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			ULONG nCount = Application::ReleaseSolarMutex();
			[pShowSaveDisabledDialog performSelectorOnMainThread:@selector(showSaveDisabledDialog:) withObject:pShowSaveDisabledDialog waitUntilDone:YES modes:pModes];
			Application::AcquireSolarMutex( nCount );

			[pPool release];
		}
	}

	return bRet;
}
