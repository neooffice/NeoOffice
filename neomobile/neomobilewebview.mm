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
#include "neomobileappevent.hxx"
#include <objc/objc-class.h>

#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif

using namespace vos;

static const NSString *pTestBaseURLs[] = { @"https://neomobile-test.neooffice.org/", @"https://neomobile-test-primary.neooffice.org/", @"https://neomobile-test-backup.neooffice.org/" };

/**
 * Overrides WebKit's [WebJavaScriptTextInputPanel windowDidLoad] selector to
 * set the JavaScript prompt dialog to have no title like the other JavaScript
 * dialogs.
 */
static id WebJavaScriptTextInputPanel_windowDidLoadIMP( id pThis, SEL aSelector, ... )
{
	NSWindowController *pController = (NSWindowController *)pThis;
	if ( pController )
	{
		NSWindow *pWindow = [pController window];
		if ( pWindow )
			[pWindow setTitle:@""];
	}

	return pThis;
}

static MacOSBOOL bWebJavaScriptTextInputPanelSwizzeled = NO;

@implementation NeoMobileWebView

- (id)initWithFrame:(NSRect)aFrame frameName:(NSString *)pFrameName groupName:(NSString *)pGroupName
{
	if ( !bWebJavaScriptTextInputPanelSwizzeled )
	{
		// Override [WebJavaScriptTextInputPanel windowDidLoad]
		NSBundle *pBundle = [NSBundle bundleForClass:[WebView class]];
		if ( pBundle )
		{
			Class aClass = [pBundle classNamed:@"WebJavaScriptTextInputPanel"];
			if ( aClass )
			{
				SEL aSelector = @selector(windowDidLoad);
				NSMethodSignature *pSignature = [NSWindowController instanceMethodSignatureForSelector:aSelector];
				if ( pSignature )
				{
					// Do not free method list
					struct objc_method_list *pMethods = (struct objc_method_list *)malloc( sizeof( struct objc_method_list ) );
					pMethods->method_count = 1;
					pMethods->method_list[ 0 ].method_name = aSelector;
					pMethods->method_list[ 0 ].method_types = (char *)[pSignature methodReturnType];
					pMethods->method_list[ 0 ].method_imp = WebJavaScriptTextInputPanel_windowDidLoadIMP;
					class_addMethods( aClass, pMethods );

					bWebJavaScriptTextInputPanelSwizzeled = YES;
				}
			}
		}
	}

	[super initWithFrame:aFrame frameName:pFrameName groupName:pGroupName];

	mnBaseURLEntry = 0;
	mpBaseURLs = [NSArray arrayWithObjects:pTestBaseURLs count:sizeof( pTestBaseURLs ) / sizeof( NSString* )];
	if ( mpBaseURLs )
	{
		[mpBaseURLs retain];
		mnBaseURLCount = [mpBaseURLs count];
	}
	else
	{
		mnBaseURLCount = 0;
	}

	WebPreferences *pPrefs = [self preferences];
	if ( pPrefs )
		[pPrefs setJavaScriptEnabled:YES];

	[self setFrameLoadDelegate:self];
	[self setUIDelegate:self];
	NSURL *pURL = [NSURL URLWithString:@"/" relativeToURL:[NSURL URLWithString:(NSString *)[mpBaseURLs objectAtIndex:mnBaseURLEntry]]];
	if ( pURL )
		[[self mainFrame] loadRequest:[NSURLRequest requestWithURL:pURL]];

	mpPanel = [[NSPanel alloc] initWithContentRect:NSMakeRect(0, 0, 700, 500) styleMask:NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSUtilityWindowMask backing:NSBackingStoreBuffered defer:YES];
	if ( mpPanel )
	{
		[mpPanel setFloatingPanel:YES];
		[mpPanel setContentView:self];
	}

	return self;
}

- (void)reloadFrameWithNextServer:(WebFrame *)pWebFrame
{
	if ( !pWebFrame )
		return;

	WebDataSource *pDataSource = [pWebFrame provisionalDataSource];
	if ( !pDataSource )
	{
		pDataSource = [pWebFrame dataSource];
		if ( !pDataSource )
			return;
	}

	NSMutableURLRequest *pRequest = [pDataSource request];
	if ( !pRequest )
		return;

	NSURL *pURL = [pRequest URL];
	if ( !pURL )
		return;

	NSMutableString *pURI = [NSMutableString stringWithCapacity:512];
	if ( !pURI )
		return;

	mnBaseURLEntry++;
	if ( mnBaseURLEntry >= mnBaseURLCount )
	{
		mnBaseURLEntry = 0;
		return;
	}

	// Try to load next base URL
	NSString *pPath = [pURL path];
	if ( pPath )
		[pURI appendString:pPath];
	NSString *pParams = [pURL parameterString];
	if ( pParams )
	{
		[pURI appendString:@";"];
		[pURI appendString:pParams];
	}
	NSString *pQuery = [pURL query];
	if ( pQuery )
	{
		[pURI appendString:@"?"];
		[pURI appendString:pQuery];
	}
	NSString *pFragment = [pURL fragment];
	if ( pFragment )
	{
		[pURI appendString:@"#"];
		[pURI appendString:pFragment];
	}

	pURL = [NSURL URLWithString:pURI relativeToURL:[NSURL URLWithString:(NSString *)[mpBaseURLs objectAtIndex:mnBaseURLEntry]]];
	if ( pURL )
	{
		NSMutableURLRequest *pNewRequest = [NSMutableURLRequest requestWithURL:pURL cachePolicy:[pRequest cachePolicy] timeoutInterval:[pRequest timeoutInterval]];
		if ( pNewRequest )
		{
			[pNewRequest setAllHTTPHeaderFields:[pRequest allHTTPHeaderFields]];
			NSData *pBody = [pRequest HTTPBody];
			if ( pBody )
				[pNewRequest setHTTPBody:pBody];
			NSInputStream *pBodyStream = [pRequest HTTPBodyStream];
			if ( pBodyStream )
				[pNewRequest setHTTPBodyStream:pBodyStream];
			[pNewRequest setHTTPMethod:[pRequest HTTPMethod]];
			[pWebFrame stopLoading];
			[pWebFrame loadRequest:[NSURLRequest requestWithURL:pURL]];
		}
	}
}

- (void)webView:(WebView *)pWebView didFailLoadWithError:(NSError *)pError forFrame:(WebFrame *)pWebFrame
{
	[self reloadFrameWithNextServer:pWebFrame];
}

- (void)webView:(WebView *)pWebView didFailProvisionalLoadWithError:(NSError *)pError forFrame:(WebFrame *)pWebFrame
{
	[self reloadFrameWithNextServer:pWebFrame];
}

- (void)webView:(WebView *)pWebView didFinishLoadForFrame:(WebFrame *)pWebFrame
{
	if ( !pWebView || !pWebFrame )
		return;

	WebDataSource *pDataSource = [pWebFrame dataSource];
	if ( !pDataSource )
		return;

	NSHTTPURLResponse *pResponse = (NSHTTPURLResponse *)[pDataSource response];
	if ( !pResponse )
		return;

	NSURL *pURL = [pResponse URL];
	if ( !pURL )
		return;

	NSDictionary *pHeaders = [pResponse allHeaderFields];
	if ( !pHeaders )
		return;

#ifdef DEBUG
	fprintf( stderr, "Load Frame URL: %s\n", [[pURL absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
	fprintf( stderr, "Response code: %i\nHeaders:\n", [pResponse statusCode] );
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

	// Post a NeoMobilExportFileAppEvent and have the OOo code execute it
	NSString *pSaveURIHeader = (NSString *)[pHeaders objectForKey:@"Neomobile-Save-Uri"];
	if ( pSaveURIHeader )
	{
		NSURL *pSaveURL = [NSURL URLWithString:pSaveURIHeader relativeToURL:[NSURL URLWithString:(NSString *)[mpBaseURLs objectAtIndex:mnBaseURLEntry]]];
		if ( pSaveURL )
		{
			NeoMobilExportFileAppEvent aEvent;

			vos::IMutex& rSolarMutex = Application::GetSolarMutex();
			rSolarMutex.acquire();

			if ( Application::IsInMain() )
			{
				Application::PostUserEvent( LINK( &aEvent, NeoMobilExportFileAppEvent, ExportFile ) );
				while ( !aEvent.IsFinished() && Application::IsInMain() )
					Application::Reschedule();
				rSolarMutex.release();
			}

			// TODO: Add files generated by the NeoMobilExportFileAppEvent to
			// the load request
			[pWebFrame loadRequest:[NSURLRequest requestWithURL:pSaveURL]];
		}
	}
}

- (void)webView:(WebView *)pWebView didStartProvisionalLoadForFrame:(WebFrame *)pFrame
{
	if ( mnBaseURLEntry >= mnBaseURLCount )
		mnBaseURLEntry = 0;
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

- (void)dealloc
{
	if ( mpBaseURLs )
		[mpBaseURLs release];

	if ( mpPanel )
		[mpPanel release];

	[super dealloc];
}

@end
