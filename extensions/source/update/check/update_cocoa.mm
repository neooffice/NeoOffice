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
 *		 - GNU General Public License Version 2.1
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2012 by Planamesa Inc.
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
 *************************************************************************/

#include "update_cocoa.hxx"
#include "update_java.hxx"
#include "updatei18n_cocoa.hxx"
#include "updatewebview_cocoa.h"

using namespace rtl;

//========================================================================

const NSString *kUpdateLastURLPref = @"updateLastURL";
const NSString *kUpdateXPosPref = @"updateXPos";
const NSString *kUpdateYPosPref = @"updateYPos";
const NSString *kUpdateWidthPref = @"updateWidth";
const NSString *kUpdateHeightPref = @"updateHeight";
const NSString *kUpdateVisiblePref = @"updateVisible";
const NSString *kUpdateServerTypePref = @"updateServerType";

static UpdateNonRecursiveResponderWebPanel *pSharedPanel = nil;

@interface UpdateCreateWebViewImpl : NSObject
{
	const NSString*				mpTitle;
	const NSString*				mpURL;
	const NSString*				mpUserAgent;
	MacOSBOOL					mbWebViewShowing;
}
+ (id)createWithURL:(const NSString *)pURL userAgent:(const NSString *)pUserAgent title:(NSString *)pTitle;
- (id)initWithURL:(const NSString *)pURL userAgent:(const NSString *)pUserAgent title:(NSString *)pTitle;
- (MacOSBOOL)isWebViewShowing;
- (void)showWebView:(id)obj;
@end

@implementation UpdateCreateWebViewImpl

+ (id)createWithURL:(const NSString *)pURL userAgent:(const NSString *)pUserAgent title:(NSString *)pTitle
{
	UpdateCreateWebViewImpl *pRet = [[UpdateCreateWebViewImpl alloc] initWithURL:pURL userAgent:pUserAgent title:pTitle];
	[pRet autorelease];
	return pRet;
}

- (id)initWithURL:(const NSString *)pURL userAgent:(const NSString *)pUserAgent title:(NSString *)pTitle
{
	self = [super init];

	mpTitle = pTitle;
	mpURL = pURL;
	mpUserAgent = pUserAgent;
	mbWebViewShowing = NO;

	return(self);
}

- (MacOSBOOL)isWebViewShowing
{
	return mbWebViewShowing;
}

- (void)showWebView:(id)obj
{
	// If the OOo update check URL is out of sync with the server type that
	// the web view is using, do not use the web view to handle updates
	NSURL *pURL = [NSURL URLWithString:mpURL];
	if ( !pURL || ![UpdateWebView isUpdateURL:pURL syncServer:NO] )
		return;

	if ( !pSharedPanel )
		pSharedPanel = [[UpdateNonRecursiveResponderWebPanel alloc] initWithUserAgent:mpUserAgent];

	if(pSharedPanel)
	{
		if ( mpTitle )
			[pSharedPanel setTitle:mpTitle];
		else
			[pSharedPanel setTitle:@""];

		NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
		if(![pSharedPanel isVisible])
		{
			// Check for retained user position. If not available, make
			// relative to the primary frame.
			NSString *widthStr=[defaults stringForKey:kUpdateWidthPref];
			NSString *heightStr=[defaults stringForKey:kUpdateHeightPref];
			if(widthStr && heightStr)
			{
				NSSize contentSize={0, 0};
				contentSize.width=[widthStr intValue];
				contentSize.height=[heightStr intValue];
				[pSharedPanel setContentSize:contentSize];
			}

			NSPoint windowPos={75, 75};
			NSString *xPosStr=[defaults stringForKey:kUpdateXPosPref];
			NSString *yPosStr=[defaults stringForKey:kUpdateYPosPref];
			if(xPosStr && yPosStr)
			{
				windowPos.x=[xPosStr intValue];
				windowPos.y=[yPosStr intValue];
			}
			else
			{
				// Center on top of main window
				NSWindow *mainWindow=[NSApp mainWindow];
				if(mainWindow)
				{
					NSRect mainWindowFrame=[mainWindow frame];
					NSSize sharedPanelSize=[pSharedPanel frame].size;
					windowPos.x=mainWindowFrame.origin.x+((mainWindowFrame.size.width-sharedPanelSize.width)/2);
					windowPos.y=mainWindowFrame.origin.y+((mainWindowFrame.size.height-sharedPanelSize.height)/2);
				}
			}
			[pSharedPanel setFrameOrigin:windowPos];

			// Make sure window is visible
			[pSharedPanel makeKeyAndOrderFront:self];

			[defaults setBool:YES forKey:kUpdateVisiblePref];
			[defaults synchronize];
		}

		UpdateWebView *pWebView = [pSharedPanel webView];
		if(pWebView)
			[pWebView loadStartingURL:pURL];

		mbWebViewShowing = YES;
	}
}
@end

@interface UpdateQuitWebViewImpl : NSObject
{
	MacOSBOOL					mbWebViewRequestedQuitApp;
}
+ (id)create;
- (id)init;
- (void)quitWebView:(id)obj;
- (MacOSBOOL)webViewRequestedQuitApp;
@end

@implementation UpdateQuitWebViewImpl

+ (id)create
{
	UpdateQuitWebViewImpl *pRet = [[UpdateQuitWebViewImpl alloc] init];
	[pRet autorelease];
	return pRet;
}

- (id)init
{
	self = [super init];

	mbWebViewRequestedQuitApp = NO;

	return(self);
}

- (void)quitWebView:(id)obj
{
	if (pSharedPanel)
	{
 		if ([pSharedPanel isVisible])
			[pSharedPanel orderOut:self];

		UpdateWebView *pWebView = [pSharedPanel webView];
		if(pWebView)
			mbWebViewRequestedQuitApp = [pWebView requestedQuitApp];
	}

	if (!mbWebViewRequestedQuitApp && UpdateHasPackagePaths())
	{
		NSAlert *pAlert = [NSAlert alertWithMessageText:UpdateGetLocalizedString(UPDATEINSTALLUPDATES) defaultButton:UpdateGetVCLResString(SV_BUTTONTEXT_YES) alternateButton:UpdateGetVCLResString(SV_BUTTONTEXT_NO) otherButton:nil informativeTextWithFormat:@""];
		if ( pAlert && [pAlert runModal] == NSAlertDefaultReturn )
		{
			mbWebViewRequestedQuitApp = YES;
			CFPreferencesSetAppValue( kUpdateSuppressLaunchAfterInstallationPref
, kCFBooleanTrue, kCFPreferencesCurrentApplication );
			CFPreferencesAppSynchronize( kCFPreferencesCurrentApplication );
		}
	}
}

- (MacOSBOOL)webViewRequestedQuitApp
{
	return mbWebViewRequestedQuitApp;
}

@end

OUString UpdateNSStringToOUString( NSString *pString )
{
	if ( !pString )
		return OUString();

	unsigned int nLen = [pString length];
	if ( !nLen )
		return OUString();

	sal_Unicode aBuf[ nLen + 1 ];
	[pString getCharacters:aBuf];
	aBuf[ nLen ] = 0;

	return OUString( aBuf );
}

sal_Bool UpdateQuitNativeDownloadWebView()
{
	sal_Bool bRet = sal_False;

	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	UpdateQuitWebViewImpl *pImp = [UpdateQuitWebViewImpl create];
	[pImp performSelectorOnMainThread:@selector(quitWebView:) withObject:pImp waitUntilDone:YES modes:pModes];
	bRet = (sal_Bool)[pImp webViewRequestedQuitApp];

	[pool release];

	return bRet;
}

sal_Bool UpdateShowNativeDownloadWebView( ::rtl::OUString aURL, ::rtl::OUString aUserAgent, ::rtl::OUString aTitle )
{
	sal_Bool bRet = sal_False;

	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	NSString *pURL = [NSString stringWithCharacters:aURL.getStr() length:aURL.getLength()];
	NSString *pUserAgent = [NSString stringWithCharacters:aUserAgent.getStr() length:aUserAgent.getLength()];
	NSString *pTitle = [NSString stringWithCharacters:aTitle.getStr() length:aTitle.getLength()];
	if ( pURL )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		UpdateCreateWebViewImpl *pImp = [UpdateCreateWebViewImpl createWithURL:pURL userAgent:pUserAgent title:pTitle];
		[pImp performSelectorOnMainThread:@selector(showWebView:) withObject:pImp waitUntilDone:YES modes:pModes];
		bRet = (sal_Bool)[pImp isWebViewShowing];
	}

	[pool release];

	return bRet;
}
