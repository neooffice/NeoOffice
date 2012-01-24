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
 *  Copyright 2008 by Planamesa Inc.
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

#include "premac.h"
#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#include "postmac.h"
#import "neomobileappevent.hxx"

// Redefine Cocoa YES and NO defines types for convenience
#ifdef YES
#undef YES
#define YES (MacOSBOOL)1
#endif
#ifdef NO
#undef NO
#define NO (MacOSBOOL)0
#endif

#define kNMDefaultBrowserWidth	430
#define kNMDefaultBrowserHeight	620

@class NeoMobileNonRecursiveResponderWebPanel;

@interface NeoMobileWebView : WebView
{
	NSObject*				mpDelegate;
	NeoMobileNonRecursiveResponderWebPanel*	mpPanel;
	NSButton*				mpbackButton;
	NSButton*				mpcancelButton;
	NSProgressIndicator*	mpdownloadingIndicator;
	NSProgressIndicator*	mploadingIndicator;
	NSText*					mpstatusLabel;
	NSString*				mpuserAgent;
	NeoMobileExportFileAppEvent*	mpexportEvent;
}
+ (const NSString *)appendNeoMobileServerNameToString:(const NSString *)pString;
+ (NSString *)neoMobileURL;
+ (MacOSBOOL)isDownloadURL:(NSURL *)url;
+ (MacOSBOOL)isLoginURL:(NSURL *)url httpMethod:(NSString *)httpMethod;
+ (MacOSBOOL)isNeoMobileURL:(NSURL *)pURL syncServer:(MacOSBOOL)syncServer;
+ (MacOSBOOL)incrementNeoMobileBaseEntry;
- (void)dealloc;
- (id)initWithFrame:(NSRect)aFrame panel:(NeoMobileNonRecursiveResponderWebPanel *)pPanel backButton:(NSButton *)pBackButton cancelButton:(NSButton *)pCancelButton downloadingIndicator:(NSProgressIndicator *)pDownloadingIndicator loadingIndicator:(NSProgressIndicator *)pLoadingIndicator statusLabel:(NSText *)pStatusLabel userAgent:(const NSString *)pUserAgent;
- (void)loadURI:(NSString *)pURI;
- (void)reloadFrameWithNextServer:(WebFrame *)pWebFrame reason:(NSError *)pError;
- (void)setDownloadingIndicatorHidden:(MacOSBOOL)bHidden;
- (void)webView:(WebView *)pWebView decidePolicyForNewWindowAction:(NSDictionary *)pActionInformation request:(NSURLRequest *)pRequest newFrameName:(NSString *)pFrameName decisionListener:(id < WebPolicyDecisionListener >)pListener;
- (void)webView:(WebView *)pWebView didFailLoadWithError:(NSError *)pError forFrame:(WebFrame *)pWebFrame;
- (void)webView:(WebView *)pWebView didFailProvisionalLoadWithError:(NSError *)pError forFrame:(WebFrame *)pWebFrame;
- (void)webView:(WebView *)pWebView didFinishLoadForFrame:(WebFrame *)pWebFrame;
- (void)webView:(WebView *)pWebView didStartProvisionalLoadForFrame:(WebFrame *)pFrame;
- (NSURLRequest *)webView:(WebView *)pWebView resource:(id)aIdentifier willSendRequest:(NSURLRequest *)pRequest redirectResponse:(NSURLResponse *)pRedirectResponse fromDataSource:(WebDataSource *)pDataSource;
- (void)webView:(WebView *)pWebView runJavaScriptAlertPanelWithMessage:(NSString *)pMessage initiatedByFrame:(WebFrame *)pWebFame;
- (MacOSBOOL)webView:(WebView *)pWebView runJavaScriptConfirmPanelWithMessage:(NSString *)pMessage initiatedByFrame:(WebFrame *)pWebFrame;
- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename;
- (void)download: (NSURLDownload *)download didReceiveResponse:(NSURLResponse *) response;
- (void)downloadDidBegin: (NSURLDownload *)download;
- (void)download:(NSURLDownload *)download didReceiveDataOfLength:(unsigned long)length;
- (void)downloadDidFinish: (NSURLDownload*)download;
- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error;
- (void)webView:(WebView *)sender decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id)listener;
- (void)webView:(WebView *)sender decidePolicyForMIMEType:(NSString *)type request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id < WebPolicyDecisionListener >)listener;
- (IBAction)backButtonPressed;
- (IBAction)cancelButtonPressed;
@end

@interface NeoMobileNonRecursiveResponderPanel : NSPanel
{
	MacOSBOOL				mbinZoom;
}
- (void)adjustBottomOfControlToTextHeight:(NSControl *)pControl;
- (void)centerTextInTextView:(NSText *)pTextView;
- (id)initWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nWindowStyle backing:(NSBackingStoreType)nBufferingType defer:(MacOSBOOL)bDeferCreation;
- (MacOSBOOL)tryToPerform:(SEL)aAction with:(id)aObject;
- (void)windowDidMove:(NSNotification *)notification;
- (void)windowDidResize:(NSNotification *)notification;
- (void)windowWillClose:(NSNotification *)notification;
- (NSRect)windowWillUseStandardFrame:(NSWindow *)pWindow defaultFrame:(NSRect)aFrame;
- (void)zoom:(id)aObject;
@end

@interface NeoMobileNonRecursiveResponderWebPanel : NeoMobileNonRecursiveResponderPanel
{
	NSButton*				mpbackButton;
	NSView*					mpbottomView;
	NSView*					mpcontentView;
	NSButton*				mpcancelButton;
	NSProgressIndicator*	mpdownloadingIndicator;
	NSProgressIndicator*	mploadingIndicator;
	NSText*					mpstatusLabel;
	NSString*				mpuserAgent;
	NeoMobileWebView*		mpwebView;
}
- (void)createWebView:(NSURLRequest *)pRequest;
- (void)dealloc;
- (void)dismissFlipsidePanel;
- (id)initWithUserAgent:(NSString *)pUserAgent;
- (void)showFlipsidePanel;
- (NeoMobileWebView *)webView;
@end
