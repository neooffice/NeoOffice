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

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxsids.hrc>
#include <tools/rcid.h>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#import <objc/objc-runtime.h>
#include <postmac.h>

#import "objserv_cocoa.h"
#import "objserv_cocoa.hrc"
#import "../../../extensions/source/update/check/updatehdl.hrc"

static ResMgr *pObjServResMgr = NULL;
static ResMgr *pUpdResMgr = NULL;

static XubString GetObjServResString( int nId )
{
	if ( !pObjServResMgr )
	{
		pObjServResMgr = SfxApplication::CreateResManager( "objserv_cocoa" );
		if ( !pObjServResMgr )
			return XubString();
	}

	ResId aResId( nId, *pObjServResMgr );
	aResId.SetRT( RSC_STRING );
	if ( !pObjServResMgr->IsAvailable( aResId ) )
		return XubString();
 
	return XubString( ResId( nId, *pObjServResMgr ) );
}

static XubString GetUpdResString( int nId )
{
	if ( !pUpdResMgr )
	{
		pUpdResMgr = SfxApplication::CreateResManager( "upd" );
		if ( !pUpdResMgr )
			return XubString();
	}

	ResId aResId( nId, *pUpdResMgr );
	aResId.SetRT( RSC_STRING );
	if ( !pUpdResMgr->IsAvailable( aResId ) )
		return XubString();
 
	return XubString( ResId( nId, *pUpdResMgr ) );
}

@interface ShowSaveDisabledDialog : NSObject
{
	NSString*				mpMessageText;
	NSString*				mpDefaultButton;
	NSString*				mpAlternateButton;
	NSString*				mpInformativeText;
}
+ (id)createWithMessageText:(NSString *)pMessageText defaultButton:(NSString *)pDefaultButton alternateButton:(NSString *)pAlternateButton informativeText:(NSString *)pInformativeText;
- (void)dealloc;
- (id)initWithMessageText:(NSString *)pMessageText defaultButton:(NSString *)pDefaultButton alternateButton:(NSString *)pAlternateButton informativeText:(NSString *)pInformativeText;
- (void)showSaveDisabledDialog:(id)pObject;
@end

@implementation ShowSaveDisabledDialog

+ (id)createWithMessageText:(NSString *)pMessageText defaultButton:(NSString *)pDefaultButton alternateButton:(NSString *)pAlternateButton informativeText:(NSString *)pInformativeText
{
	ShowSaveDisabledDialog *pRet = [[ShowSaveDisabledDialog alloc] initWithMessageText:pMessageText defaultButton:pDefaultButton alternateButton:pAlternateButton informativeText:pInformativeText];
	[pRet autorelease];
	return pRet;
}

- (void)dealloc
{
	if ( mpMessageText )
		[mpMessageText release];
	if ( mpDefaultButton )
		[mpDefaultButton release];
	if ( mpAlternateButton )
		[mpAlternateButton release];
	if ( mpInformativeText )
		[mpInformativeText release];
	
	[super dealloc];
}

- (id)initWithMessageText:(NSString *)pMessageText defaultButton:(NSString *)pDefaultButton alternateButton:(NSString *)pAlternateButton informativeText:(NSString *)pInformativeText
{
	[super init];

	mpMessageText = pMessageText;
	if ( mpMessageText )
		[mpMessageText retain];
	mpDefaultButton = pDefaultButton;
	if ( mpDefaultButton )
		[mpDefaultButton retain];
	mpAlternateButton = pAlternateButton;
	if ( mpAlternateButton )
		[mpAlternateButton retain];
	mpInformativeText = pInformativeText;
	if ( mpInformativeText )
		[mpInformativeText retain];

	return self;
}

- (void)showSaveDisabledDialog:(id)pObject
{
	NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
	NSURL *pURL = [NSURL URLWithString:(NSString *)CFSTR( PRODUCT_MAC_APP_STORE_URL )];
	if ( pURL && ![@"macappstores" isEqualToString:[pURL scheme]] && ![@"http" isEqualToString:[pURL scheme]] && ![@"https" isEqualToString:[pURL scheme]] )
		pURL = nil;

	NSAlert *pAlert;
	if ( pWorkspace && pURL )
	{
		NSString *pInformativeText = mpInformativeText;
		if ( !pInformativeText )
			pInformativeText = @"";
		pAlert = [NSAlert alertWithMessageText:mpMessageText defaultButton:mpDefaultButton alternateButton:mpAlternateButton otherButton:nil informativeTextWithFormat:pInformativeText, nil];
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

			if ( [pAlert runModal] == NSAlertDefaultReturn )
			{
				// The OS X sandbox will sometimes fail to open URLs when the
				// default browser is not Safari and the browser is not running
				if ( ![pWorkspace openURL:pURL] )
					[pWorkspace openURLs:[NSArray arrayWithObject:pURL] withAppBundleIdentifier:@"com.apple.Safari" options:NSWorkspaceLaunchDefault additionalEventParamDescriptor:nil launchIdentifiers:nil];
			}
		}
	}
	else
	{
		pAlert = [NSAlert alertWithMessageText:mpMessageText defaultButton:nil alternateButton:nil otherButton:nil informativeTextWithFormat:@""];
		if ( pAlert )
			[pAlert runModal];
	}
}

@end

sal_Bool SfxObjectShell_canSave( SfxObjectShell *pObjShell, USHORT nID )
{
	sal_Bool bRet = sal_True;

	if ( pObjShell && ( nID == SID_DOCTEMPLATE || nID == SID_SAVEDOC || nID == SID_SAVEASDOC ) )
	{
		char *env = getenv( "SAL_ENABLE_MAS" );
		if ( !env || strcmp( env, "1" ) )
		{
			bRet = sal_False;

			NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

			XubString aDesc = GetObjServResString( STR_SAVEDISABLEDCANNOTSAVE );
			aDesc.EraseAllChars( '~' );
			NSString *pMessageText = [NSString stringWithCharacters:aDesc.GetBuffer() length:aDesc.Len()];

			aDesc = GetObjServResString( STR_SAVEDISABLEDDOWNLOADPRODUCTTOSAVE );
			aDesc.EraseAllChars( '~' );
			NSString *pInformativeText = [NSString stringWithCharacters:aDesc.GetBuffer() length:aDesc.Len()];

			aDesc = GetUpdResString( RID_UPDATE_BTN_DOWNLOAD );
			aDesc.EraseAllChars( '~' );
			NSString *pDefaultButton = [NSString stringWithCharacters:aDesc.GetBuffer() length:aDesc.Len()];

			aDesc = GetUpdResString( RID_UPDATE_BTN_CANCEL );
			aDesc.EraseAllChars( '~' );
			NSString *pAlternateButton = [NSString stringWithCharacters:aDesc.GetBuffer() length:aDesc.Len()];

			ShowSaveDisabledDialog *pShowSaveDisabledDialog = [ShowSaveDisabledDialog createWithMessageText:pMessageText defaultButton:pDefaultButton alternateButton:pAlternateButton informativeText:pInformativeText];
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pShowSaveDisabledDialog performSelectorOnMainThread:@selector(showSaveDisabledDialog:) withObject:pShowSaveDisabledDialog waitUntilDone:NO modes:pModes];

			[pPool release];
		}
	}

	return bRet;
}
