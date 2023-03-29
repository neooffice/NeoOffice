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

#import "neomobile.hxx"
#import "neomobilei18n.hxx"
#import "neomobileflipsideview.h"

#import <Security/Security.h>

#define kNMFlipsidePanelPadding 20

static const NSString *kCreateAccountURI = @"/signup/planselection";
static const NSString *kForgotPasswordURI = @"/users/forgotpassword";
static const NSString *kKeychainServiceName = @"NeoOffice Mobile";
static const NSString *kSavePasswordPref = @"nmSavePassword";
static const NSString *kUsernamePref = @"nmUsername";

@interface NeoMobileLoginTitleView : NSView
- (void)drawRect:(NSRect)dirtyRect;
@end

@implementation NeoMobileLoginTitleView

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];

	NSGradient *pGradient = [[NSGradient alloc] initWithColors:[NSArray arrayWithObjects:[NSColor darkGrayColor], [NSColor grayColor], nil]];
	[pGradient drawInRect:[self bounds] angle:90];
}

@end

@implementation NeoMobileNonRecursiveResponderFlipsidePanel

- (void)dealloc
{
	NSNotificationCenter *pNotificationCenter = [NSNotificationCenter defaultCenter];
	if ( pNotificationCenter )
		[pNotificationCenter removeObserver:self name:NSApplicationWillTerminateNotification object:nil];

	[self setContentView:nil];

	if ( mpWebPanel )
		[mpWebPanel release];

	if ( mpcontentView )
		[mpcontentView release];

	if ( mploginButton )
		[mploginButton release];

	if ( mppasswordEdit )
		[mppasswordEdit release];

	if ( mppasswordLabel )
		[mppasswordLabel release];

	if ( mpsavePasswordButton )
		[mpsavePasswordButton release];

	if ( mptitleLabel )
		[mptitleLabel release];

	if ( mptitleView )
		[mptitleView release];

	if ( mpusernameEdit )
		[mpusernameEdit release];

	if ( mpusernameLabel )
		[mpusernameLabel release];

	[super dealloc];
}

- (IBAction)doAbout
{
	// Have Mac OS X open the about URL
	NSURL *pURL = [NSURL URLWithString:kNeoMobileAboutURL];
	if ( pURL )
	{
		NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
		if ( pWorkspace )
			[pWorkspace openURL:pURL];
	}
}

- (IBAction)doCreateAccount
{
	// Dismiss our panel
	[mpWebPanel dismissFlipsidePanel];

	// Have web view load URI
	NeoMobileWebView *webView = [mpWebPanel webView];
	if ( webView )
		[webView loadURI:kCreateAccountURI];
}

- (IBAction)doForgotPassword
{
	// Dismiss our panel
	[mpWebPanel dismissFlipsidePanel];

	// Have web view load URI
	NeoMobileWebView *webView = [mpWebPanel webView];
	if ( webView )
		[webView loadURI:kForgotPasswordURI];
}

- (IBAction)doLogin
{
	// Dismiss our panel
	[mpWebPanel dismissFlipsidePanel];

	// Have web view perform a post request
	NeoMobileWebView *webView = [mpWebPanel webView];
	if ( !webView )
		return;

	NSURL *pBaseURL = [NSURL URLWithString:[NeoMobileWebView neoMobileURL]];
	if (!pBaseURL)
		return;

	NSURL *pURL = [NSURL URLWithString:(NSString *)kNeoMobileLoginURI relativeToURL:pBaseURL];
	if (!pURL)
		return;

	NSMutableURLRequest *pURLRequest = [NSMutableURLRequest requestWithURL:pURL];
	if (!pURLRequest)
		return;

	[pURLRequest setHTTPMethod:@"POST"];

	NSData *postData=[[NSString stringWithFormat:@"data[User][username]=%@&data[User][password]=%@", [[mpusernameEdit stringValue] stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding], [[mppasswordEdit stringValue] stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]] dataUsingEncoding:NSUTF8StringEncoding];
	
	[pURLRequest setValue:[NSString stringWithFormat:@"%d", [postData length]] forHTTPHeaderField:@"Content-Length"];
	[pURLRequest setValue:@"application/x-www-form-urlencoded" forHTTPHeaderField:@"Content-Type"];
	[pURLRequest setHTTPBody:postData];
	
	[[webView mainFrame] loadRequest:pURLRequest];
}

- (id)initWithWebPanel:(NeoMobileNonRecursiveResponderWebPanel *)pWebPanel
{
	[super initWithContentRect:[pWebPanel frame] styleMask:[pWebPanel styleMask] backing:NSBackingStoreBuffered defer:YES];
	[self setBackgroundColor:[NSColor whiteColor]];

	mpWebPanel = pWebPanel;
	[mpWebPanel retain];

	mpcontentView=[[NSView alloc] initWithFrame:[[self contentView] frame]];
	[mpcontentView setAutoresizesSubviews:YES];

	NSSize contentSize=[mpcontentView bounds].size;
	float defaultHeight=50;
	float verticalCenterLine=kNMDefaultBrowserWidth*0.5;

	mptitleView=[[NeoMobileLoginTitleView alloc] initWithFrame:NSMakeRect(0, contentSize.height-defaultHeight, contentSize.width, defaultHeight)];
	[mptitleView setAutoresizingMask:(NSViewWidthSizable|NSViewMinYMargin)];

	mptitleLabel=[[NSText alloc] initWithFrame:NSMakeRect(0, 0, [mptitleView frame].size.width, [mptitleView frame].size.height)];
	[mptitleLabel setEditable:NO];
	[mptitleLabel setString:NeoMobileGetLocalizedString(NEOMOBILELOGINTITLE)];
	[mptitleLabel setAutoresizingMask:(NSViewWidthSizable|NSViewMinYMargin)];
	[mptitleLabel setAlignment:NSCenterTextAlignment];
	[mptitleLabel setDrawsBackground:NO];
	[mptitleLabel setTextColor:[NSColor whiteColor]];

	NSFont *titleLabelFont=[[NSFontManager sharedFontManager] convertFont:[mptitleLabel font] toSize:30];
	titleLabelFont=[[NSFontManager sharedFontManager] convertFont:titleLabelFont toHaveTrait:NSBoldFontMask];
	[mptitleLabel setFont:titleLabelFont];
	[self centerTextInTextView:mptitleLabel];

	[mptitleView addSubview:mptitleLabel];

	mpusernameEdit=[[NSTextField alloc] initWithFrame:NSMakeRect(verticalCenterLine+kNMFlipsidePanelPadding, [mptitleView frame].origin.y-defaultHeight-kNMFlipsidePanelPadding, contentSize.width-verticalCenterLine-(kNMFlipsidePanelPadding*2), defaultHeight)];
	[mpusernameEdit setEditable:YES];
	[mpusernameEdit setAutoresizingMask:(NSViewWidthSizable|NSViewMinYMargin)];

	NSFont *usernameEditFont=[[NSFontManager sharedFontManager] convertFont:[mpusernameEdit font] toSize:18];
	usernameEditFont=[[NSFontManager sharedFontManager] convertFont:usernameEditFont toHaveTrait:NSBoldFontMask];
	[mpusernameEdit setFont:usernameEditFont];
	[self adjustBottomOfControlToTextHeight:mpusernameEdit];

	NSRect usernameEditFrame=[mpusernameEdit frame];
	mpusernameLabel=[[NSText alloc] initWithFrame:NSMakeRect(kNMFlipsidePanelPadding, usernameEditFrame.origin.y, verticalCenterLine-kNMFlipsidePanelPadding, usernameEditFrame.size.height)];
	[mpusernameLabel setEditable:NO];
	[mpusernameLabel setString:NeoMobileGetLocalizedString(NEOMOBILEUSERNAME)];
	[mpusernameLabel setAutoresizingMask:NSViewMinYMargin];
	[mpusernameLabel setFont:usernameEditFont];
	[mpusernameLabel setDrawsBackground:NO];
	[self centerTextInTextView:mpusernameLabel];

	mppasswordEdit=[[NSSecureTextField alloc] initWithFrame:NSMakeRect(usernameEditFrame.origin.x, usernameEditFrame.origin.y-usernameEditFrame.size.height-kNMFlipsidePanelPadding, usernameEditFrame.size.width, usernameEditFrame.size.height)];
	[mppasswordEdit setEditable:YES];
	[mppasswordEdit setAutoresizingMask:(NSViewWidthSizable|NSViewMinYMargin)];
	[mppasswordEdit setFont:usernameEditFont];

	NSRect passwordEditFrame=[mppasswordEdit frame];
	mppasswordLabel=[[NSText alloc] initWithFrame:NSMakeRect(kNMFlipsidePanelPadding, passwordEditFrame.origin.y, verticalCenterLine-kNMFlipsidePanelPadding, passwordEditFrame.size.height)];
	[mppasswordLabel setEditable:NO];
	[mppasswordLabel setString:NeoMobileGetLocalizedString(NEOMOBILEPASSWORD)];
	[mppasswordLabel setAutoresizingMask:NSViewMinYMargin];
	[mppasswordLabel setFont:usernameEditFont];
	[mppasswordLabel setDrawsBackground:NO];
	[self centerTextInTextView:mppasswordLabel];

	mpsavePasswordButton=[[NSButton alloc] initWithFrame:NSMakeRect(kNMFlipsidePanelPadding*2, passwordEditFrame.origin.y-defaultHeight-kNMFlipsidePanelPadding, contentSize.width-(kNMFlipsidePanelPadding*4), defaultHeight)];
	[mpsavePasswordButton setTitle:NeoMobileGetLocalizedString(NEOMOBILESAVEPASSWORD)];
	[mpsavePasswordButton setEnabled:YES];
	[mpsavePasswordButton setButtonType:NSSwitchButton];
	[mpsavePasswordButton setState:NSOnState];
	[mploginButton setImagePosition:NSNoImage];
	[mpsavePasswordButton setAutoresizingMask:(NSViewWidthSizable|NSViewMinYMargin)];
	[mpsavePasswordButton setFont:usernameEditFont];
	[self adjustBottomOfControlToTextHeight:mpsavePasswordButton];

	NSRect savePasswordButtonFrame=[mpsavePasswordButton frame];
	mploginButton=[[NSButton alloc] initWithFrame:NSMakeRect(kNMFlipsidePanelPadding, savePasswordButtonFrame.origin.y-defaultHeight-kNMFlipsidePanelPadding, contentSize.width-(kNMFlipsidePanelPadding*2), defaultHeight)];
	[mploginButton setTitle:NeoMobileGetLocalizedString(NEOMOBILELOGIN)];
	[mploginButton setEnabled:YES];
	[mploginButton setButtonType:NSMomentaryPushInButton];
	[mploginButton setBezelStyle:NSRegularSquareBezelStyle];
	[mploginButton setImagePosition:NSNoImage];
	[mploginButton setAutoresizingMask:(NSViewWidthSizable|NSViewMinYMargin)];

	NSFont *loginButtonFont=[[NSFontManager sharedFontManager] convertFont:[mploginButton font] toSize:18];
	loginButtonFont=[[NSFontManager sharedFontManager] convertFont:loginButtonFont toHaveTrait:NSBoldFontMask];
	[mploginButton setFont:loginButtonFont];
	[mploginButton setTarget:self];
	[mploginButton setAction:@selector(doLogin)];

	NSRect loginButtonFrame=[mploginButton frame];
	mpcreateAccountButton=[[NSButton alloc] initWithFrame:NSMakeRect(kNMFlipsidePanelPadding*2, loginButtonFrame.origin.y-defaultHeight-kNMFlipsidePanelPadding, contentSize.width-(kNMFlipsidePanelPadding*4), defaultHeight)];
	[mpcreateAccountButton setTitle:NeoMobileGetLocalizedString(NEOMOBILECREATEACCOUNT)];
	[mpcreateAccountButton setEnabled:YES];
	[mpcreateAccountButton setButtonType:NSMomentaryPushInButton];
	[mpcreateAccountButton setBezelStyle:NSRegularSquareBezelStyle];
	[mpcreateAccountButton setImagePosition:NSNoImage];
	[mpcreateAccountButton setAutoresizingMask:(NSViewWidthSizable|NSViewMinYMargin)];
	[mpcreateAccountButton setFont:loginButtonFont];
	[mpcreateAccountButton setTarget:self];
	[mpcreateAccountButton setAction:@selector(doCreateAccount)];

	NSRect createAccountButtonFrame=[mpcreateAccountButton frame];
	mpforgotPasswordButton=[[NSButton alloc] initWithFrame:NSMakeRect(kNMFlipsidePanelPadding*2, createAccountButtonFrame.origin.y-defaultHeight-kNMFlipsidePanelPadding, contentSize.width-(kNMFlipsidePanelPadding*4), defaultHeight)];
	[mpforgotPasswordButton setTitle:NeoMobileGetLocalizedString(NEOMOBILEFORGOTPASSWORD)];
	[mpforgotPasswordButton setEnabled:YES];
	[mpforgotPasswordButton setButtonType:NSMomentaryPushInButton];
	[mpforgotPasswordButton setBezelStyle:NSRegularSquareBezelStyle];
	[mpforgotPasswordButton setImagePosition:NSNoImage];
	[mpforgotPasswordButton setAutoresizingMask:(NSViewWidthSizable|NSViewMinYMargin)];
	[mpforgotPasswordButton setFont:loginButtonFont];
	[mpforgotPasswordButton setTarget:self];
	[mpforgotPasswordButton setAction:@selector(doForgotPassword)];

	NSRect forgotPasswordButtonFrame=[mpforgotPasswordButton frame];
	mpaboutButton=[[NSButton alloc] initWithFrame:NSMakeRect(kNMFlipsidePanelPadding*2, forgotPasswordButtonFrame.origin.y-defaultHeight-kNMFlipsidePanelPadding, contentSize.width-(kNMFlipsidePanelPadding*4), defaultHeight)];
	[mpaboutButton setTitle:NeoMobileGetLocalizedString(NEOMOBILEABOUT)];
	[mpaboutButton setEnabled:YES];
	[mpaboutButton setButtonType:NSMomentaryPushInButton];
	[mpaboutButton setBezelStyle:NSRegularSquareBezelStyle];
	[mpaboutButton setImagePosition:NSNoImage];
	[mpaboutButton setAutoresizingMask:(NSViewWidthSizable|NSViewMinYMargin)];
	[mpaboutButton setFont:loginButtonFont];
	[mpaboutButton setTarget:self];
	[mpaboutButton setAction:@selector(doAbout)];

	[mpcontentView addSubview:mptitleView];
	[mpcontentView addSubview:mpusernameEdit];
	[mpcontentView addSubview:mpusernameLabel];
	[mpcontentView addSubview:mppasswordEdit];
	[mpcontentView addSubview:mppasswordLabel];
	[mpcontentView addSubview:mpsavePasswordButton];
	[mpcontentView addSubview:mploginButton];
	[mpcontentView addSubview:mpcreateAccountButton];
	[mpcontentView addSubview:mpforgotPasswordButton];
	[mpcontentView addSubview:mpaboutButton];
	[self setContentView:mpcontentView];

	// Limit tabbing to only active controls
	[mpusernameEdit setNextKeyView:mppasswordEdit];
	[mppasswordEdit setNextKeyView:mpsavePasswordButton];
	[mpsavePasswordButton setNextKeyView:mploginButton];
	[mploginButton setNextKeyView:mpcreateAccountButton];
	[mpcreateAccountButton setNextKeyView:mpforgotPasswordButton];
	[mpforgotPasswordButton setNextKeyView:mpaboutButton];
	[mpaboutButton setNextKeyView:mpusernameEdit];

	[self setInitialFirstResponder:mpusernameEdit];
	[self setDefaultButtonCell:[mploginButton cell]];

	return self;
}

- (void)orderWindow:(NSWindowOrderingMode)nOrderingMode relativeTo:(int)nOtherWindowNumber
{
	if (nOrderingMode == NSWindowOut)
	{
		[self storeLoginInfo];
	}
	else if (![self isVisible])
	{
		// Load in the stored username and password into the entry fields
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		if(defaults)
		{
			MacOSBOOL savePassword = [defaults boolForKey:(NSString *)[NeoMobileWebView appendNeoMobileServerNameToString:kSavePasswordPref]];
			NSString *usernamePref = [defaults stringForKey:(NSString *)[NeoMobileWebView appendNeoMobileServerNameToString:kUsernamePref]];
			[mpusernameEdit setStringValue:(usernamePref ? usernamePref : @"")];
			[mpsavePasswordButton setState:(savePassword ? NSOnState : NSOffState)];
;

			// Get password from keychain
			NSString *passwordPref = nil;
			const char *serviceName = [[NeoMobileWebView appendNeoMobileServerNameToString:kKeychainServiceName] UTF8String];
			unsigned int serviceNameLen = strlen(serviceName);
			const char *username = (usernamePref ? [usernamePref UTF8String] : "");
			unsigned int usernameLen = strlen(username);

			SecKeychainAttribute aAttributes[2];
			aAttributes[0].tag = kSecServiceItemAttr;
			aAttributes[0].length = serviceNameLen;
			aAttributes[0].data = (void *)serviceName;
			aAttributes[1].tag = kSecAccountItemAttr;
			aAttributes[1].length = usernameLen;
			aAttributes[1].data = (void *)username;
			SecKeychainAttributeList aAttributeList;
			aAttributeList.count = 2;
			aAttributeList.attr = aAttributes;
			SecKeychainSearchRef aSearch = NULL;
			SecKeychainSearchCreateFromAttributes(NULL, kSecGenericPasswordItemClass, &aAttributeList, &aSearch);
			if (aSearch)
			{
				SecKeychainItemRef aSearchItem = NULL;
				if (SecKeychainSearchCopyNext(aSearch, &aSearchItem) != errSecItemNotFound && aSearchItem)
				{
					UInt32 passwordLen = 0;
					char *password = NULL;
					SecKeychainItemCopyAttributesAndData(aSearchItem, NULL, NULL, NULL, &passwordLen, (void **)&password);
					if (password)
					{
						char buffer[passwordLen+1];
						memcpy(buffer, password, passwordLen);
						buffer[passwordLen] = '\0';
						passwordPref = [NSString stringWithUTF8String:buffer];
						SecKeychainItemFreeAttributesAndData(NULL, password);
					}

					CFRelease(aSearchItem);
				}

				CFRelease(aSearch);
			}

			[mppasswordEdit setStringValue:(passwordPref ? passwordPref : @"")];

			if ([[mpusernameEdit stringValue] length])
				[self makeFirstResponder:mppasswordEdit];
			else
				[self makeFirstResponder:mpusernameEdit];
		}
	}

	[super orderWindow:nOrderingMode relativeTo:nOtherWindowNumber];
}

- (IBAction)storeLoginInfo
{
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	if(defaults)
	{
		MacOSBOOL savePassword = ([mpsavePasswordButton state] == NSOnState);
		[defaults setObject:[mpusernameEdit stringValue] forKey:(NSString *)[NeoMobileWebView appendNeoMobileServerNameToString:kUsernamePref]];
		[defaults setBool:savePassword forKey:(NSString *)[NeoMobileWebView appendNeoMobileServerNameToString:kSavePasswordPref]];
		[defaults synchronize];

		// Update password in keychain
		const char *serviceName = [[NeoMobileWebView appendNeoMobileServerNameToString:kKeychainServiceName] UTF8String];
		unsigned int serviceNameLen = strlen(serviceName);
		const char *username = [[mpusernameEdit stringValue] UTF8String];
		const char *password = (savePassword ? [[mppasswordEdit stringValue] UTF8String] : "");
		unsigned int usernameLen = strlen(username);
		unsigned int passwordLen = strlen(password);

		MacOSBOOL keyChainUpdated = NO;
		SecKeychainAttribute aServiceAttribute;
		aServiceAttribute.tag = kSecServiceItemAttr;
		aServiceAttribute.length = serviceNameLen;
		aServiceAttribute.data = (void *)serviceName;
		SecKeychainAttributeList aServiceAttributeList;
		aServiceAttributeList.count = 1;
		aServiceAttributeList.attr = &aServiceAttribute;
		SecKeychainSearchRef aSearch = NULL;
		SecKeychainSearchCreateFromAttributes(NULL, kSecGenericPasswordItemClass, &aServiceAttributeList, &aSearch);
		if (aSearch)
		{
			SecKeychainItemRef aSearchItem = NULL;
			if (SecKeychainSearchCopyNext(aSearch, &aSearchItem) != errSecItemNotFound && aSearchItem)
			{
				if (savePassword)
				{
					SecKeychainAttribute aAccountAttribute;
					aAccountAttribute.tag = kSecAccountItemAttr;
					aAccountAttribute.length = usernameLen;
					aAccountAttribute.data = (void *)username;
					SecKeychainAttributeList aAccountAttributeList;
					aAccountAttributeList.count = 1;
					aAccountAttributeList.attr = &aAccountAttribute;
					SecKeychainItemModifyAttributesAndData(aSearchItem, &aAccountAttributeList, passwordLen, password);
				}
				else
				{
					SecKeychainItemDelete(aSearchItem);
				}

				keyChainUpdated = YES;
				CFRelease(aSearchItem);
			}

			CFRelease(aSearch);
		}

		if (savePassword && !keyChainUpdated)
			SecKeychainAddGenericPassword(NULL, serviceNameLen, serviceName, usernameLen, username, passwordLen, password, NULL);
	}
}

@end
