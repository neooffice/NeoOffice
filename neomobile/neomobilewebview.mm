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

#include "neomobile.hxx"
#include "neomobilewebview.h"
#include "neomobileappevent.hxx"

#include "premac.h"
#include <objc/objc-class.h>
#include "postmac.h"

#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif

#include <map>
#include <string>

using namespace rtl;
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

@interface ZeroHeightDividerSplitView : NSSplitView
- (float)dividerThickness;
@end

@implementation ZeroHeightDividerSplitView

- (float)dividerThickness
{
	return 0;
}

@end

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

	[self setResourceLoadDelegate:self];
	[self setFrameLoadDelegate:self];
	[self setUIDelegate:self];
	[self setDownloadDelegate:self];
	[self setPolicyDelegate:self];

	mpdownload=nil;
	mpexportEvent=NULL;
	
	mpPanel = [[NSPanel alloc] initWithContentRect:NSMakeRect(0, 0, 700, 524) styleMask:NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSUtilityWindowMask backing:NSBackingStoreBuffered defer:YES];
	if ( mpPanel )
	{
		[mpPanel setFloatingPanel:YES];
		
		mpcontentView=[[NSView alloc] initWithFrame:NSMakeRect(0, 0, 700, 524)];
		[mpcontentView setAutoresizesSubviews:YES];
		
		//mpcontentView=[[ZeroHeightDividerSplitView alloc] initWithFrame:NSMakeRect(0, 0, 700, 500)];		
		
		mpcancelButton=[[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 100, 24)];
		[mpcancelButton setTitle:@"Cancel"];
		[mpcancelButton setTarget:self];
		[mpcancelButton setAction:@selector(cancelButtonPressed)];
		[mpcancelButton setAutoresizingMask:(NSViewMaxXMargin)];
		[mpcancelButton setEnabled:NO];
		[mpcancelButton setButtonType:NSMomentaryPushInButton];
		[mpcancelButton setBezelStyle:NSRoundedBezelStyle];
		[mpcancelButton setKeyEquivalent:@"\r"];
		
		float fontSize=[NSFont systemFontSizeForControlSize:NSSmallControlSize];
		NSCell *theCell = [mpcancelButton cell];
		NSFont *theFont = [NSFont fontWithName:[[theCell font] fontName] size:fontSize];
		[theCell setFont:theFont];
		[theCell setControlSize:NSSmallControlSize];
		
		mpstatusLabel=[[NSText alloc] initWithFrame:NSMakeRect(100, 0, 600, 24)];
		[mpstatusLabel setEditable:NO];
		[mpstatusLabel setString:@""];
		[mpstatusLabel setAutoresizingMask:(NSViewWidthSizable)];
		[mpstatusLabel setDrawsBackground:NO];
		
		//ZeroHeightDividerSplitView *bottomView=[[ZeroHeightDividerSplitView alloc] initWithFrame:NSMakeRect(0, 0, 700, 30)];
		//[bottomView setVertical:YES];
		
		NSView *bottomView=[[NSView alloc] initWithFrame:NSMakeRect(0, 0, 700, 24)];
		
		//NSBox *bottomView=[[NSBox alloc] initWithFrame:NSMakeRect(0, 0, 700, 30)];
		//[bottomView setBorderType:NSNoBorder];
		
		//[bottomView setAutoresizesSubviews:YES];
		[bottomView setAutoresizesSubviews:YES];
		
		[bottomView setAutoresizingMask:(NSViewWidthSizable)];
		
		[bottomView addSubview:mpcancelButton];
		[bottomView addSubview:mpstatusLabel];
		
		[mpcontentView addSubview:self];
		
		[self setFrame:NSMakeRect(0, 24, 700, 500)];
		[self setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
		[mpcontentView addSubview:bottomView];
		
		[mpPanel setContentView:mpcontentView];
	}

	return self;
}

- (void)cancelButtonPressed
{
#ifdef DEBUG
	fprintf(stderr, "NeoMobile Cancel Button Clicked\n");
#endif
	if(mpdownload)
	{
		// a file download is in progress, cancel it
		
		[mpdownload cancel];
	}
	else if(mpexportEvent)
	{
		// a file export application event is being processed, set the flag
		// to cancel it
		
		mpexportEvent->Cancel();
	}
	{
		// regular webframe load
		
		[self stopLoading:self];
	}
}

- (void)loadURI:(NSString *)pURI
{
	if ( !pURI )
		pURI = @"/";

	NSURL *pURL = [NSURL URLWithString:pURI relativeToURL:[NSURL URLWithString:(NSString *)[mpBaseURLs objectAtIndex:mnBaseURLEntry]]];
	if ( pURL )
		[[self mainFrame] loadRequest:[NSURLRequest requestWithURL:pURL]];
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
#ifdef DEBUG
	NSLog( @"didFailLoadWithError: %@", pError);
#endif	
	// NOTE: we don't want to trigger the server fallback if we are just
	// processing a data download we've redirected from the web frame
	if([pError code]!=WebKitErrorFrameLoadInterruptedByPolicyChange)
	{
		[mpcancelButton setEnabled:NO];
		[mpstatusLabel setString:@""];

		[self reloadFrameWithNextServer:pWebFrame];
	}
}

- (void)webView:(WebView *)pWebView didFailProvisionalLoadWithError:(NSError *)pError forFrame:(WebFrame *)pWebFrame
{
#ifdef DEBUG
	NSLog( @"didFailProvisionalLoadWithError: %@", pError);
#endif
	// NOTE: we don't want to trigger the server fallback if we are just
	// processing a data download we've redirected from the web frame
	if([pError code]!=WebKitErrorFrameLoadInterruptedByPolicyChange)
	{
		[mpcancelButton setEnabled:NO];
		[mpstatusLabel setString:@""];

		[self reloadFrameWithNextServer:pWebFrame];
	}
}

- (void)webView:(WebView *)pWebView didFinishLoadForFrame:(WebFrame *)pWebFrame
{
	[mpcancelButton setEnabled:NO];
	[mpstatusLabel setString:@""];
	
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
	NSString *pSaveUUIDHeader = (NSString *)[pHeaders objectForKey:@"Neomobile-Save-Uuid"];
	if ( pSaveURIHeader && pSaveUUIDHeader )
	{
		NSURL *pSaveURL = [NSURL URLWithString:pSaveURIHeader relativeToURL:[NSURL URLWithString:(NSString *)[mpBaseURLs objectAtIndex:mnBaseURLEntry]]];
		if ( pSaveURL )
		{
			NSMutableURLRequest *pURLRequest = [NSMutableURLRequest requestWithURL:pSaveURL];
			NSFileManager *pFileManager = [NSFileManager defaultManager];
			NSData *pPostBody = [NSMutableData dataWithLength:0];
			if ( pURLRequest && pFileManager && pPostBody )
			{
				NeoMobilExportFileAppEvent aEvent( NSStringToOUString( pSaveUUIDHeader ), pFileManager, pPostBody );

				vos::IMutex& rSolarMutex = Application::GetSolarMutex();
				rSolarMutex.acquire();

				if ( Application::IsInMain() )
				{
					[mpcancelButton setEnabled:YES];
					[mpstatusLabel setString:@"Exporting file..."];
					mpexportEvent=&aEvent;
					
					Application::PostUserEvent( LINK( &aEvent, NeoMobilExportFileAppEvent, ExportFile ) );
					while ( !aEvent.IsFinished() && Application::IsInMain() )
						Application::Reschedule();
					
					[mpcancelButton setEnabled:NO];
					[mpstatusLabel setString:@""];
					mpexportEvent=NULL;
					
					if(aEvent.IsCanceled())
					{
						// go back to publish page
						
						[self goBack];
					}
					else
					{
						// start the upload
						
						[pURLRequest addValue: @"multipart/form-data; boundary=neomobileupload" forHTTPHeaderField: @"Content-Type"];
						[pURLRequest setHTTPMethod:@"POST"];
						[pURLRequest setHTTPBody:pPostBody];
						[pWebFrame loadRequest:pURLRequest];
					}
				}

				rSolarMutex.release();

			}
		}
	}
}

- (void)webView:(WebView *)pWebView didStartProvisionalLoadForFrame:(WebFrame *)pFrame
{
	[mpcancelButton setEnabled:YES];
	
	WebDataSource *pDataSource = [pFrame provisionalDataSource];
	
	NSMutableURLRequest *pRequest = nil;
	if ( pDataSource )
		pRequest = [pDataSource request];
	
	if ( pRequest && [[pRequest HTTPMethod] isEqualToString:@"POST"] )
		[mpstatusLabel setString:@"Uploading file..."];
	else
		[mpstatusLabel setString:@"Loading..."];
}

- (NSURLRequest *)webView:(WebView *)pWebView resource:(id)aIdentifier willSendRequest:(NSURLRequest *)pRequest redirectResponse:(NSURLResponse *)pRedirectResponse fromDataSource:(WebDataSource *)pDataSource
{
	if ( mnBaseURLEntry >= mnBaseURLCount )
		mnBaseURLEntry = 0;

	// Always add a special header with the name and version of the application
	// that this web view is running in
	// TODO: set header value to applications's name and version
	if ( pRequest && [pRequest isKindOfClass:[NSMutableURLRequest class]] )
		[(NSMutableURLRequest *)pRequest addValue:@"Neomobile-Application-Version" forHTTPHeaderField:@"Neomobile-Application-Version"];

	return pRequest;
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

- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename
{
#ifdef DEBUG
	fprintf( stderr, "Download downloadRequestReceived: %s\n", [[[[download request] URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif
	NSString *basePath = NSTemporaryDirectory();
	NSString *filePath = [basePath stringByAppendingPathComponent:filename];
	int i=0;
	while ([[NSFileManager defaultManager] fileExistsAtPath:filePath]) {
		filePath = [basePath stringByAppendingPathComponent:[NSString stringWithFormat:@"%@ %d.%@", [filename stringByDeletingPathExtension], (++i), [filename pathExtension]]];
	}
	
	[download setDestination:filePath allowOverwrite:YES];
}

static std::map<NSURLDownload *, std::string> gDownloadPathMap;

- (void)download:(NSURLDownload *)download didCreateDestination:(NSString *)path
{
#ifdef DEBUG
	fprintf( stderr, "Download didCreateDestination: %s\n", [[[[download request] URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif
	gDownloadPathMap[download]=[path cStringUsingEncoding:NSUTF8StringEncoding];
}

- (void)downloadDidBegin: (NSURLDownload *)download
{
#ifdef DEBUG
	fprintf( stderr, "Download File Did Begin: %s\n", [[[[download request] URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif
	
	mpdownload=download;
	[mpcancelButton setEnabled:YES];
	[mpstatusLabel setString:@"Downloading file..."];
}

- (void)downloadDidFinish: (NSURLDownload*)download
{
#ifdef DEBUG
	fprintf( stderr, "Download File Did End: %s\n", [[[[download request] URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif
	char outBuf[PATH_MAX];
	if(gDownloadPathMap.count(download)>0)
	{
		sprintf(outBuf, "/usr/bin/open \"%s\"", gDownloadPathMap[download].c_str());
		system(outBuf); // +++ REPLACE WITH APP EVENT
		gDownloadPathMap.erase(download);
	}
	
	mpdownload=nil;
	[mpcancelButton setEnabled:NO];
	[mpstatusLabel setString:@""];
}

- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error
{
#ifdef DEBUG
	NSLog( @"Download didFailWithError: %@", error );
#endif
	mpdownload=nil;
	[mpcancelButton setEnabled:NO];
	[mpstatusLabel setString:@"Download failed!"];
	// +++ ADD SERVER FALLBACK DOWNLOAD HERE
}

- (void)webView:(WebView *)sender decidePolicyForNavigationAction:(NSDictionary *)actionInformation
        request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id)listener
{
#ifdef DEBUG
	fprintf( stderr, "NeoMobile Loading URL: %s\n", [[[request URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif
	[listener use];
}

- (void)webView:(WebView *)sender decidePolicyForMIMEType:(NSString *)type request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id < WebPolicyDecisionListener >)listener
{
	if([type rangeOfString: @"vnd.oasis.opendocument"].location != NSNotFound)
	{
		[listener download];
	}
	else
	{
		if([WebView canShowMIMEType:type]==YES)
			[listener use];
		else
			[listener ignore];
	}
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
