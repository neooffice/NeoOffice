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

#include "neomobilewebview.h"

@interface NeoMobileWebViewDelegate : NSObject
- (void)webView:(WebView *)self didCommitLoadForFrame:(WebFrame *)pWebFrame;
- (void)webView:(WebView *)pWebView runJavaScriptAlertPanelWithMessage:(NSString *)pMessage initiatedByFrame:(WebFrame *)pWebFame;
- (MacOSBOOL)webView:(WebView *)pWebView runJavaScriptConfirmPanelWithMessage:(NSString *)pMessage initiatedByFrame:(WebFrame *)pWebFrame;
@end

@implementation NeoMobileWebViewDelegate

- (void)webView:(WebView *)pWebView didCommitLoadForFrame:(WebFrame *)pWebFrame
{
	if ( !pWebView || !pWebFrame )
		return;

	WebDataSource *pDataSource = [pWebFrame dataSource];
	if ( !pDataSource )
		return;

	NSHTTPURLResponse *pResponse = (NSHTTPURLResponse *)[pDataSource response];
	if ( !pResponse )
		return;

#ifdef DEBUG
	fprintf( stderr, "Load Frame URL: %s\n", [[[pResponse URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
	fprintf( stderr, "Response code: %i\nHeaders:\n", [pResponse statusCode] );
	NSDictionary *pHeaders = [pResponse allHeaderFields];
	if ( pHeaders )
	{
		NSString *pKey;
		NSEnumerator *pEnum = [pHeaders keyEnumerator];
		while ( ( pKey = (NSString *)[pEnum nextObject] ) ) {
			NSString *pValue = (NSString *)[pHeaders objectForKey:pKey];
			fprintf( stderr, "    %s: %s\n", [pKey cStringUsingEncoding:NSUTF8StringEncoding], pValue ? [pValue cStringUsingEncoding:NSUTF8StringEncoding] : "" );
		}
	}
#endif	// DEBUG

	NSWindow *pWindow = [pWebView window];
	if ( pWindow )
		[pWindow orderFront:self];
}

- (void)webView:(WebView *)pWebView runJavaScriptAlertPanelWithMessage:(NSString *)pMessage initiatedByFrame:(WebFrame *)pWebFame
{
	if ( !pWebView )
		return;

	NSWindow *pWindow = [pWebView window];
	if ( !pWindow || ![pWindow isVisible] )
		return;

	NSAlert *pAlert = [NSAlert alertWithMessageText:pMessage defaultButton:nil alternateButton:nil otherButton:nil informativeTextWithFormat:@""];
	if ( pAlert )
		[pAlert runModal];
}

- (MacOSBOOL)webView:(WebView *)pWebView runJavaScriptConfirmPanelWithMessage:(NSString *)pMessage initiatedByFrame:(WebFrame *)pWebFrame
{
	MacOSBOOL bRet = NO;

	if ( !pWebView )
		return bRet;

	NSWindow *pWindow = [pWebView window];
	if ( !pWindow || ![pWindow isVisible] )
		return bRet;

	NSAlert *pAlert = [NSAlert alertWithMessageText:pMessage defaultButton:nil alternateButton:@"Cancel" otherButton:nil informativeTextWithFormat:@""];
	if ( pAlert && [pAlert runModal] == NSAlertDefaultReturn )
		bRet = YES;

	return bRet;
}

@end

@implementation NeoMobileWebView

- (void)dealloc
{
	if ( mpDelegate )
		[mpDelegate release];

	if ( mpPanel )
		[mpPanel release];

	[super dealloc];
}

- (id)initWithFrame:(NSRect)aFrame frameName:(NSString *)pFrameName groupName:(NSString *)pGroupName
{
	[super initWithFrame:aFrame frameName:pFrameName groupName:pGroupName];

	mpDelegate = [[NeoMobileWebViewDelegate alloc] init];
	if ( mpDelegate )
	{
		WebPreferences *pPrefs = [self preferences];
		if ( pPrefs )
			[pPrefs setJavaScriptEnabled:YES];

		[self setFrameLoadDelegate:mpDelegate];
		[self setUIDelegate:mpDelegate];
		[[self mainFrame] loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"https://neomobile-test.neooffice.org/neofolders/"]]];
	}

	mpPanel = [[NSPanel alloc] initWithContentRect:NSMakeRect(0, 0, 700, 500) styleMask:NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSUtilityWindowMask backing:NSBackingStoreBuffered defer:YES];
	if ( mpPanel )
	{
		[mpPanel setFloatingPanel:YES];
		[mpPanel setContentView:self];
	}

	return self;
}

@end
