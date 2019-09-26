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
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfxsids.hrc>
#include <tools/rcid.h>
#include <vcl/layout.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#import <objc/objc-runtime.h>
#include <postmac.h>

#import "objserv_cocoa.h"
#import "objserv_cocoa.hrc"

// LibreOffice headers are found relative to LibreOffice include directory
#import "../desktop/source/app/desktop.hrc"
#import "../extensions/source/update/check/updatehdl.hrc"

#include <dlfcn.h>

typedef sal_Bool Application_canSave_Type();
typedef int Application_validateReceipt_Type();

static Application_canSave_Type *pApplication_canSave = NULL;
static Application_validateReceipt_Type *pApplication_validateReceipt = NULL;
static ResMgr *pDktResMgr = NULL;
static ResMgr *pUpdResMgr = NULL;

static OUString GetDktResString( int nId )
{
	if ( !pDktResMgr )
	{
		pDktResMgr = ResMgr::CreateResMgr( "dkt" );
		if ( !pDktResMgr )
			return "";
	}

	ResId aResId( nId, *pDktResMgr );
	aResId.SetRT( RSC_STRING );
	if ( !pDktResMgr->IsAvailable( aResId ) )
		return "";
 
	return OUString( ResId( nId, *pDktResMgr ) );
}

static OUString GetUpdResString( int nId )
{
	if ( !pUpdResMgr )
	{
		pUpdResMgr = ResMgr::CreateResMgr( "upd" );
		if ( !pUpdResMgr )
			return "";
	}

	ResId aResId( nId, *pUpdResMgr );
	aResId.SetRT( RSC_STRING );
	if ( !pUpdResMgr->IsAvailable( aResId ) )
		return "";
 
	return OUString( ResId( nId, *pUpdResMgr ) );
}

static NSAlert *pSaveDisabledAlert = nil;

@interface ShowSaveDisabledDialog : NSObject
{
	NSString*				mpMessageText;
	NSString*				mpDefaultButton;
	NSString*				mpAlternateButton;
	NSString*				mpInformativeText;
	BOOL					mbTerminate;
}
+ (id)createWithMessageText:(NSString *)pMessageText defaultButton:(NSString *)pDefaultButton alternateButton:(NSString *)pAlternateButton informativeText:(NSString *)pInformativeText terminate:(BOOL)bTerminate;
- (void)dealloc;
- (id)initWithMessageText:(NSString *)pMessageText defaultButton:(NSString *)pDefaultButton alternateButton:(NSString *)pAlternateButton informativeText:(NSString *)pInformativeText terminate:(BOOL)bTerminate;
- (void)showSaveDisabledDialog:(id)pObject;
@end

@implementation ShowSaveDisabledDialog

+ (id)createWithMessageText:(NSString *)pMessageText defaultButton:(NSString *)pDefaultButton alternateButton:(NSString *)pAlternateButton informativeText:(NSString *)pInformativeText terminate:(BOOL)bTerminate
{
	ShowSaveDisabledDialog *pRet = [[ShowSaveDisabledDialog alloc] initWithMessageText:pMessageText defaultButton:pDefaultButton alternateButton:pAlternateButton informativeText:pInformativeText terminate:bTerminate];
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

- (id)initWithMessageText:(NSString *)pMessageText defaultButton:(NSString *)pDefaultButton alternateButton:(NSString *)pAlternateButton informativeText:(NSString *)pInformativeText terminate:(BOOL)bTerminate
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
	mbTerminate = bTerminate;

	return self;
}

- (void)showSaveDisabledDialog:(id)pObject
{
	(void)pObject;

	if ( pSaveDisabledAlert )
		return;

	NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
#ifdef PRODUCT_MAC_APP_STORE_URL
	NSURL *pURL = [NSURL URLWithString:(NSString *)CFSTR( PRODUCT_MAC_APP_STORE_URL )];
	if ( pURL && ![@"macappstores" isEqualToString:[pURL scheme]] && ![@"http" isEqualToString:[pURL scheme]] && ![@"https" isEqualToString:[pURL scheme]] )
		pURL = nil;
#else	// PRODUCT_MAC_APP_STORE_URL
	NSURL *pURL = nil;
#endif	// PRODUCT_MAC_APP_STORE_URL

	@try
	{
		pSaveDisabledAlert = [[NSAlert alloc] init];
		if ( pSaveDisabledAlert )
		{
			if ( !mbTerminate && pWorkspace && pURL )
			{
				if ( mpMessageText )
					pSaveDisabledAlert.messageText = mpMessageText;
				if ( mpInformativeText )
					pSaveDisabledAlert.informativeText = mpInformativeText;
				if ( mpDefaultButton )
					[pSaveDisabledAlert addButtonWithTitle:mpDefaultButton];
				if ( mpAlternateButton )
				{
					[pSaveDisabledAlert addButtonWithTitle:mpAlternateButton];

					NSArray *pButtons = [pSaveDisabledAlert buttons];
					NSUInteger nCount = [pButtons count];
					if ( nCount )
					{
						NSButton *pAlternateButton = [pButtons objectAtIndex:nCount - 1];
						if ( pAlternateButton )
						{
							unichar cEscapeChar = 0x1b;
							NSString *pEscapeKey = [NSString stringWithCharacters:&cEscapeChar length:1];
							if ( pEscapeKey )
								[pAlternateButton setKeyEquivalent:pEscapeKey];
						}
					}
				}

				if ( [pSaveDisabledAlert runModal] == NSAlertFirstButtonReturn )
				{
					// The OS X sandbox will sometimes fail to open URLs when
					// the default browser is not Safari and the browser is not
					// running
					if ( ![pWorkspace openURL:pURL] )
						[pWorkspace openURLs:[NSArray arrayWithObject:pURL] withAppBundleIdentifier:@"com.apple.Safari" options:NSWorkspaceLaunchDefault additionalEventParamDescriptor:nil launchIdentifiers:nil];
				}
			}
			else
			{
				if ( mpMessageText )
					pSaveDisabledAlert.messageText = mpMessageText;
				[pSaveDisabledAlert runModal];
				if ( mbTerminate )
				{
					NSApplication *pApp = [NSApplication sharedApplication];
					if ( pApp )
						[pApp terminate:self];
				}
			}
		}
	}
	@catch ( NSException *pExc )
	{
	}

	if ( pSaveDisabledAlert )
	{
		[pSaveDisabledAlert autorelease];
		pSaveDisabledAlert = nil;
	}
}

@end

sal_Bool SfxObjectShell_canSave( SfxObjectShell *pObjShell, sal_uInt16 nID )
{
	sal_Bool bRet = sal_True;

	if ( pObjShell && ( nID == SID_DOCTEMPLATE || nID == SID_SAVEDOC || nID == SID_SAVEASDOC ) )
	{
		if ( !pApplication_canSave )
			pApplication_canSave = (Application_canSave_Type *)dlsym( RTLD_MAIN_ONLY, "Application_canSave" );
		if ( !pApplication_canSave || !pApplication_canSave() )
		{
			bRet = sal_False;

			NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];
			NSString *pMessageText = nil;
			NSString *pInformativeText = nil;
			NSString *pDefaultButton = nil;
			NSString *pAlternateButton = nil;

			if ( !pApplication_validateReceipt )
				pApplication_validateReceipt = (Application_validateReceipt_Type *)dlsym( RTLD_DEFAULT, "Application_validateReceipt" );
			BOOL bTerminate = ( pApplication_validateReceipt && !pApplication_validateReceipt() );
			if ( bTerminate )
			{
				OUString aDesc = GetDktResString( STR_LO_MUST_BE_RESTARTED );
				aDesc = aDesc.replaceAll( "~", "" );
				pMessageText = [NSString stringWithCharacters:aDesc.getStr() length:aDesc.getLength()];
			}
			else
			{
				OUString aDesc = SfxResId( STR_SAVEDISABLEDCANNOTSAVE );
				aDesc = aDesc.replaceAll( "~", "" );
				pMessageText = [NSString stringWithCharacters:aDesc.getStr() length:aDesc.getLength()];

				aDesc = SfxResId( STR_SAVEDISABLEDDOWNLOADPRODUCTTOSAVE );
				aDesc = aDesc.replaceAll( "~", "" );
				pInformativeText = [NSString stringWithCharacters:aDesc.getStr() length:aDesc.getLength()];

				aDesc = GetUpdResString( RID_UPDATE_BTN_DOWNLOAD );
				aDesc = aDesc.replaceAll( "~", "" );
				pDefaultButton = [NSString stringWithCharacters:aDesc.getStr() length:aDesc.getLength()];

				aDesc = GetUpdResString( RID_UPDATE_BTN_CANCEL );
				aDesc = aDesc.replaceAll( "~", "" );
				pAlternateButton = [NSString stringWithCharacters:aDesc.getStr() length:aDesc.getLength()];
			}

			ShowSaveDisabledDialog *pShowSaveDisabledDialog = [ShowSaveDisabledDialog createWithMessageText:pMessageText defaultButton:pDefaultButton alternateButton:pAlternateButton informativeText:pInformativeText terminate:bTerminate];
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pShowSaveDisabledDialog performSelectorOnMainThread:@selector(showSaveDisabledDialog:) withObject:pShowSaveDisabledDialog waitUntilDone:NO modes:pModes];

			[pPool release];
		}
	}

	return bRet;
}
