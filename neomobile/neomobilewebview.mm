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
#include "neomobilei18n.hxx"
#include "neomobilewebview.h"

#include <map>

#include "premac.h"
#include <objc/objc-class.h>
#include "postmac.h"

#ifndef NSDownloadsDirectory
#define NSDownloadsDirectory ((NSSearchPathDirectory)15)
#endif

#define kNMDefaultBrowserWidth	430
#define kNMDefaultBrowserHeight	620
#define kNMBottomViewHeight 24

using namespace rtl;

static const NSString *pDevelopmentBaseURLs[] = {
	@"http://localhost/"
};

static const NSString *pTestBaseURLs[] = {
#ifndef DEBUG
	@"https://neomobile-test.neooffice.org/",
#endif	// !DEBUG
	@"https://neomobile-test-primary.neooffice.org/",
	@"https://neomobile-test-backup.neooffice.org/",
	@"https://neomobile-test-backup2.neooffice.org/",
	@"https://neomobile-test-backup3.neooffice.org/"
};

#ifndef TEST
static const NSString *pProductionBaseURLs[] = {
#ifndef DEBUG
	@"https://neomobile.neooffice.org/",
#endif	// !DEBUG
	@"https://neomobile-primary.neooffice.org/",
	@"https://neomobile-backup.neooffice.org/",
	@"https://neomobile-backup2.neooffice.org/",
	@"https://neomobile-backup3.neooffice.org/"
};
#endif	// !TEST

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

- (id)initWithFrame:(NSRect)aFrame cancelButton:(NSButton *)pCancelButton statusLabel:(NSText *)pStatusLabel userAgent:(const NSString *)pUserAgent
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

	[super initWithFrame:aFrame frameName:nil groupName:nil];
	
	mpcancelButton = pCancelButton;
	[mpcancelButton retain];
	
	mpstatusLabel = pStatusLabel;
	[mpstatusLabel retain];

	// Determine which server type to use. The default server type can be
	// overridden using the following Terminal command:
	//   defaults write org.neooffice.NeoOffice nmServerType development|test
	// To use the default server type, use the following Terminal command:
	//   defaults delete org.neooffice.NeoOffice nmServerType
	mnBaseURLCount = 0;
	const NSString **pBaseURLs = nil;
	NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
	NSString *serverType=[defaults stringForKey:kNeoMobileServerTypePref];
	if ( serverType )
	{
		if ( [serverType caseInsensitiveCompare:@"development"] == NSOrderedSame )
		{
			mnBaseURLCount = sizeof( pDevelopmentBaseURLs ) / sizeof( NSString* );
			pBaseURLs = pDevelopmentBaseURLs;
		}
		else if ( [serverType caseInsensitiveCompare:@"test"] == NSOrderedSame )
		{
			mnBaseURLCount = sizeof( pTestBaseURLs ) / sizeof( NSString* );
			pBaseURLs = pTestBaseURLs;
		}
	}
	if ( !pBaseURLs )
	{
#ifdef TEST
		mnBaseURLCount = sizeof( pTestBaseURLs ) / sizeof( NSString* );
		pBaseURLs = pTestBaseURLs;
#else	// TEST
		mnBaseURLCount = sizeof( pProductionBaseURLs ) / sizeof( NSString* );
		pBaseURLs = pProductionBaseURLs;
#endif	// TEST
	}

	mnBaseURLEntry = 0;
	mpBaseURLs = [NSArray arrayWithObjects:pBaseURLs count:mnBaseURLCount];
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
	{
		[pPrefs setPrivateBrowsingEnabled:YES];
		[pPrefs setJavaScriptEnabled:YES];
	}

	[self setMaintainsBackForwardList:YES];
	[self setResourceLoadDelegate:self];
	[self setFrameLoadDelegate:self];
	[self setUIDelegate:self];
	[self setDownloadDelegate:self];
	[self setPolicyDelegate:self];

	// Set custom user agent
	if ( pUserAgent && [pUserAgent length] )
		[self setCustomUserAgent:pUserAgent];

	mpdownload=nil;
	mndownloadSize=0;
	mndownloadBytesReceived=0;
	mpexportEvent=NULL;

	return self;
}

- (void)cancelButtonPressed
{
#ifdef DEBUG
	fprintf(stderr, "NeoMobile Cancel Button Clicked\n");
#endif
	[self stopLoading:self];
}

- (void)loadURI:(NSString *)pURI
{
	if ( !pURI || ![pURI length] )
		pURI = @"/";

	NSURL *pURL = [NSURL URLWithString:pURI relativeToURL:[NSURL URLWithString:(NSString *)[mpBaseURLs objectAtIndex:mnBaseURLEntry]]];
	if ( pURL )
	{
		NSMutableURLRequest *pRequest = [NSMutableURLRequest requestWithURL:pURL];
		if ( pRequest )
			[[self mainFrame] loadRequest:pRequest];
	}
}

- (void)reloadFrameWithNextServer:(WebFrame *)pWebFrame reason:(NSError *)pError
{
	int nErr = pError ? [pError code] : 0;

	if ( !nErr || nErr == WebKitErrorFrameLoadInterruptedByPolicyChange )
	{
		// NOTE: we don't want to trigger the server fallback if we are just
		// processing a data download we've redirected from the web frame
		return;
	}
	else if ( nErr == NSURLErrorCancelled )
	{
		// Error code -999 indicates that the WebKit is doing the Back, Reload,
		// or Forward actions so we don't trigger server fallback. These
		// actions are incompatible with our export event code so cancel
		// the current export event
		if ( mpexportEvent )
			mpexportEvent->Cancel();
		return;
	}

	[mpcancelButton setEnabled:NO];
	[mpstatusLabel setString:@""];

	if ( nErr != NSURLErrorTimedOut && nErr != NSURLErrorCannotFindHost && nErr != NSURLErrorCannotConnectToHost )
	{
		NSAlert *pAlert = [NSAlert alertWithMessageText:[NSString stringWithFormat:@"%@ %@", GetLocalizedString(NEOMOBILEERROR), [pError localizedDescription]] defaultButton:nil alternateButton:nil otherButton:nil informativeTextWithFormat:@""];
		if ( pAlert )
            [pAlert runModal];
        return;
    }

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
	if ( !pURL || ![self isNeoMobileURL:pURL] )
		return;

	NSMutableString *pURI = [NSMutableString stringWithCapacity:512];
	if ( !pURI )
		return;

	NSURL *pRelativeURL = nil;
	while ( !pRelativeURL && ++mnBaseURLEntry < mnBaseURLCount )
	{
		pRelativeURL = [NSURL URLWithString:(NSString *)[mpBaseURLs objectAtIndex:mnBaseURLEntry]];

		// Do not reload from the same IP address as the first host
		if ( pRelativeURL && [[[NSHost hostWithName:[pURL host]] address] isEqualToString:[[NSHost hostWithName:[pRelativeURL host]] address]] )
			pRelativeURL = nil;
	}

	if ( !pRelativeURL )
	{
		mnBaseURLEntry = 0;
		return;
	}

	// Clear all history since we are going to switch to a different server
	WebView *pWebView = [pWebFrame webView];
	if ( pWebView )
	{
		WebBackForwardList *pHistory = [pWebView backForwardList];
		if ( pHistory )
		{
			int nCapacity = [pHistory capacity];
			if ( nCapacity )
			{
				[pHistory setCapacity:0];
				[pHistory setCapacity:nCapacity];

				// Put the new server in the history list
				WebHistoryItem *pItem = [[WebHistoryItem alloc] initWithURLString:[pRelativeURL absoluteString] title:@"" lastVisitedTimeInterval:0];
				if ( pItem )
				{
					[pItem autorelease];
					[pHistory addItem:pItem];
				}
			}
		}
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

	pURL = [NSURL URLWithString:pURI relativeToURL:pRelativeURL];
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
			[pWebFrame loadRequest:pNewRequest];
		}
	}
}

- (void)stopLoading:(id)pSender
{
	if(mpdownload)
	{
		// a file download is in progress, cancel it
		
		[mpdownload cancel];
		mpdownload=nil;
		[mpcancelButton setEnabled:NO];
		[mpstatusLabel setString:GetLocalizedString(NEOMOBILEDOWNLOADCANCELED)];
	}
	else if(mpexportEvent)
	{
		// a file export application event is being processed, set the flag
		// to cancel it but do not stop the loading as we want the export
		// event to finish normally
		
		mpexportEvent->Cancel();
		return;
	}
	else
	{
		[mpcancelButton setEnabled:NO];
		[mpstatusLabel setString:@""];
	}

	[super stopLoading:pSender];
}

- (void)webView:(WebView *)pWebView decidePolicyForNewWindowAction:(NSDictionary *)pActionInformation request:(NSURLRequest *)pRequest newFrameName:(NSString *)pFrameName decisionListener:(id < WebPolicyDecisionListener >)pListener
{
	// Have Mac OS X open the URL
	if ( pRequest )
	{
		NSURL *pURL = [pRequest URL];
		if ( pURL )
		{
			NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
			if ( pWorkspace )
				[pWorkspace openURL:pURL];
		}
	}

	// Tell the listener to ignore the request
	if ( pListener )
		[pListener ignore];
}

- (void)webView:(WebView *)pWebView didFailLoadWithError:(NSError *)pError forFrame:(WebFrame *)pWebFrame
{
#ifdef DEBUG
	NSLog( @"didFailLoadWithError: %@", pError);
#endif
	[self reloadFrameWithNextServer:pWebFrame reason:pError];
}

- (void)webView:(WebView *)pWebView didFailProvisionalLoadWithError:(NSError *)pError forFrame:(WebFrame *)pWebFrame
{
#ifdef DEBUG
	NSLog( @"didFailProvisionalLoadWithError: %@", pError);
#endif
	[self reloadFrameWithNextServer:pWebFrame reason:pError];
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

	NSData *pData = [pDataSource data];
	if ( pData && [pData bytes] )
		fprintf( stderr, "Content:\n%s\n\n", (const char *)[pData bytes] );
#endif	// DEBUG

	// Post a NeoMobileExportFileAppEvent and have the OOo code execute it
	NSString *pSaveURIHeader = (NSString *)[pHeaders objectForKey:@"Neomobile-Save-Uri"];
	NSString *pSaveUUIDHeader = (NSString *)[pHeaders objectForKey:@"Neomobile-Save-Uuid"];
	if ( pSaveURIHeader && pSaveUUIDHeader )
	{
		// Disable history completely once an upload starts. Note that once we
		// enter the upload process, we cannot reliably go back so we set the
		// history list capacity to zero.
		int nCapacity = 0;
		WebBackForwardList *pHistory = [pWebView backForwardList];
		if ( pHistory )
		{
			nCapacity = [pHistory capacity];
			if ( nCapacity )
				[pHistory setCapacity:0];
		}

		NSURL *pSaveURL = [NSURL URLWithString:pSaveURIHeader relativeToURL:[NSURL URLWithString:(NSString *)[mpBaseURLs objectAtIndex:mnBaseURLEntry]]];
		if ( pSaveURL )
		{
			NSMutableURLRequest *pURLRequest = [NSMutableURLRequest requestWithURL:pSaveURL];
			NSFileManager *pFileManager = [NSFileManager defaultManager];
			NSData *pPostBody = [NSMutableData dataWithLength:0];
			if ( pURLRequest && pFileManager && pPostBody )
			{
				if ( !mpexportEvent )
				{
					// Get list of supported mime types if any
					NSArray *pMimeTypes = nil;
					NSString *pSaveMimeTypesHeader = (NSString *)[pHeaders objectForKey:@"Neomobile-Save-Mime-Types"];
					if ( pSaveMimeTypesHeader )
						pMimeTypes = [pSaveMimeTypesHeader componentsSeparatedByString:@", "];

					NeoMobileExportFileAppEvent aEvent( NSStringToOUString( pSaveUUIDHeader ), pFileManager, pPostBody, pMimeTypes );
					mpexportEvent=&aEvent;

					[mpcancelButton setEnabled:YES];
					[mpstatusLabel setString:GetLocalizedString(NEOMOBILEEXPORTINGFILE)];

					aEvent.Execute();

					[mpcancelButton setEnabled:NO];
					[mpstatusLabel setString:@""];

					if(aEvent.IsUnsupportedComponentType())
					{
						// display unsupported doc type error to user and
						// pass UUID back to server.
						
						[self loadURI:[NSString stringWithFormat: @"/uploads/clienterror?errorCode=4&uuid=%@", pSaveUUIDHeader]];						
					}
					else if(aEvent.IsCanceled())
					{
						// display canceled error to user and pass URI back to
						// server
						
						[self loadURI:[NSString stringWithFormat: @"/uploads/clienterror?errorCode=3&uuid=%@", pSaveUUIDHeader]];
					}
					else if(aEvent.GetErrorCode()!=0)
					{
						// other error code;  pass along to the server for
						// display
						
						[self loadURI:[NSString stringWithFormat: @"/uploads/clienterror?errorCode=%d&uuid=%@", aEvent.GetErrorCode(), pSaveUUIDHeader]];
					}
					else
					{
						// start the upload
						
						[pURLRequest addValue: @"multipart/form-data; boundary=neomobileupload" forHTTPHeaderField: @"Content-Type"];
						[pURLRequest setHTTPMethod:@"POST"];
						[pURLRequest setHTTPBody:pPostBody];
						[pWebFrame loadRequest:pURLRequest];
					}
					mpexportEvent=NULL;
				}
			}
		}

		if ( nCapacity && pHistory )
		{
			[pHistory setCapacity:nCapacity];

			// Put the new server in the history list
			NSURL *pRelativeURL = [NSURL URLWithString:(NSString *)[mpBaseURLs objectAtIndex:mnBaseURLEntry]];
			if ( pRelativeURL )
			{
				WebHistoryItem *pItem = [[WebHistoryItem alloc] initWithURLString:[pRelativeURL absoluteString] title:@"" lastVisitedTimeInterval:0];
				if ( pItem )
				{
					[pItem autorelease];
					[pHistory addItem:pItem];
				}
			}
		}
	}
	else if ( !mpexportEvent )
	{
		// If not an upload, save last URL preference
		NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
		[defaults setObject:[pURL absoluteString] forKey:kNeoMobileLastURLPref];
		[defaults synchronize];
	}
}

- (void)webView:(WebView *)pWebView didStartProvisionalLoadForFrame:(WebFrame *)pFrame
{
	[mpcancelButton setEnabled:YES];

	if ( !pWebView || !pFrame )
		return;

	WebDataSource *pDataSource = [pFrame provisionalDataSource];
	
	NSMutableURLRequest *pRequest = nil;
	if ( pDataSource )
		pRequest = [pDataSource request];
	
	if ( mpexportEvent && pRequest && [[pRequest HTTPMethod] isEqualToString:@"POST"] )
		[mpstatusLabel setString:GetLocalizedString(NEOMOBILEUPLOADINGFILE)];
	else
		[mpstatusLabel setString:GetLocalizedString(NEOMOBILELOADING)];
}

- (NSURLRequest *)webView:(WebView *)pWebView resource:(id)aIdentifier willSendRequest:(NSURLRequest *)pRequest redirectResponse:(NSURLResponse *)pRedirectResponse fromDataSource:(WebDataSource *)pDataSource
{
	if ( mnBaseURLEntry >= mnBaseURLCount )
		mnBaseURLEntry = 0;

	// Clear the forward history
	WebBackForwardList *pHistory = [pWebView backForwardList];
	if ( pHistory )
	{
		int nCapacity = [pHistory capacity];
		if ( nCapacity )
		{
			int nBackListCount = [pHistory backListCount];
			[pHistory setCapacity:nBackListCount + 1];
			[pHistory setCapacity:nCapacity];

			// Put the new server in the history list
			if ( !nBackListCount )
			{
				NSURL *pRelativeURL = [NSURL URLWithString:(NSString *)[mpBaseURLs objectAtIndex:mnBaseURLEntry]];
				if ( pRelativeURL )
				{
					WebHistoryItem *pItem = [[WebHistoryItem alloc] initWithURLString:[pRelativeURL absoluteString] title:@"" lastVisitedTimeInterval:0];
					if ( pItem )
					{
						[pItem autorelease];
						[pHistory addItem:pItem];
					}
				}
			}
		}
	}

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

	NSAlert *pAlert = [NSAlert alertWithMessageText:pMessage defaultButton:nil alternateButton:GetLocalizedString(NEOMOBILECANCEL) otherButton:nil informativeTextWithFormat:@""];
	if ( pAlert && [pAlert runModal] == NSAlertDefaultReturn )
		bRet = YES;

	return bRet;
}

- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename
{
#ifdef DEBUG
	fprintf( stderr, "Download downloadRequestReceived: %s\n", [[[[download request] URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif

	// Fix broken WebKit handling of the Content-Disposition header by assuming
	// that our server has URL encoded the file name
	NSString *decodedFilename = filename;
	NSMutableString *mutableDecodedFilename = [NSMutableString stringWithString:filename];
	if (mutableDecodedFilename)
	{
		[mutableDecodedFilename replaceOccurrencesOfString:@"+" withString:@" " options:0 range:NSMakeRange(0, [filename length])];
		decodedFilename = mutableDecodedFilename;
	}
	decodedFilename = [decodedFilename stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
	if (!decodedFilename)
		decodedFilename = filename;

	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSString *basePath = nil;
	NSArray *downloadPaths = nil;

	// Use NSDownloadsDirectory only if we are running Leopard or higher,
	// otherwise use NSDesktopDirectory
	long res = 0;
	if (Gestalt(gestaltSystemVersion, &res) == noErr)
	{
		bool isLeopardOrHigher = ( ( ( ( res >> 8 ) & 0x00FF ) == 0x10 ) && ( ( ( res >> 4 ) & 0x000F ) >= 0x5 ) );
		if (isLeopardOrHigher)
			downloadPaths = NSSearchPathForDirectoriesInDomains(NSDownloadsDirectory, NSUserDomainMask, YES);
		else
			downloadPaths = NSSearchPathForDirectoriesInDomains(NSDesktopDirectory, NSUserDomainMask, YES);
	}

	if (downloadPaths && fileManager)
	{
 		unsigned int dirCount = [downloadPaths count];
 		unsigned int i = 0;
		for (; i < dirCount && !basePath; i++)
		{
			MacOSBOOL isDir = NO;
			NSString *downloadPath = (NSString *)[downloadPaths objectAtIndex:i];
			if ([fileManager fileExistsAtPath:downloadPath isDirectory:&isDir] && isDir)
			{
				basePath = downloadPath;
				break;
			}
		}
	}

	if (!basePath)
	{
		basePath = NSTemporaryDirectory();
		if (!basePath)
			basePath = @"/tmp";
	}

	NSString *filePath = [basePath stringByAppendingPathComponent:decodedFilename];
	if (fileManager)
	{
		int i=0;
		while ([fileManager fileExistsAtPath:filePath])
			filePath = [basePath stringByAppendingPathComponent:[NSString stringWithFormat:@"%@ %d.%@", [decodedFilename stringByDeletingPathExtension], (++i), [decodedFilename pathExtension]]];
	}
	
	[download setDestination:filePath allowOverwrite:YES];
	mndownloadSize=0;	// initialize only if we receive a response
}

static std::map< NSURLDownload *, OString > gDownloadPathMap;

- (void)download:(NSURLDownload *)download didCreateDestination:(NSString *)path
{
#ifdef DEBUG
	fprintf( stderr, "Download didCreateDestination: %s\n", [[[[download request] URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif
	gDownloadPathMap[download]=OString([path cStringUsingEncoding:NSUTF8StringEncoding]);
}

- (void) download: (NSURLDownload *) download didReceiveResponse: (NSURLResponse *) response
{
#ifdef DEBUG
	fprintf( stderr, "NeoMobile Download didReceiveResponse\n");
#endif

	mndownloadSize=[response expectedContentLength];
	mndownloadBytesReceived=0;
}

- (void)downloadDidBegin: (NSURLDownload *)download
{
#ifdef DEBUG
	fprintf( stderr, "Download File Did Begin: %s\n", [[[[download request] URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif

	mpdownload=download;
	[mpcancelButton setEnabled:YES];
	[mpstatusLabel setString:GetLocalizedString(NEOMOBILEDOWNLOADINGFILE)];
}

- (void)download:(NSURLDownload *)download didReceiveDataOfLength:(unsigned long)length
{
#ifdef DEBUG
	fprintf( stderr, "NeoMobile Download didReiveDataOfLength\n");
#endif

	mndownloadBytesReceived+=length;
	
	if(mndownloadSize > 0)
	{
		// we got a response from the server, so we can compute a percentage
		[mpstatusLabel setString:[NSString stringWithFormat:@"%@ %d%%", GetLocalizedString(NEOMOBILEDOWNLOADINGFILE), (int)((double)mndownloadBytesReceived/(double)mndownloadSize*100)]];
	}
	else
	{
		// no expected size received from the server, just show Kb download
		[mpstatusLabel setString:[NSString stringWithFormat:@"%@ %ldK", GetLocalizedString(NEOMOBILEDOWNLOADINGFILE), (long)(mndownloadBytesReceived/1024)]];
	}
}

- (void)downloadDidFinish: (NSURLDownload*)download
{
#ifdef DEBUG
	fprintf( stderr, "Download File Did End: %s\n", [[[[download request] URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif
	char outBuf[2*PATH_MAX];
	if(gDownloadPathMap.count(download)>0)
	{
		sprintf(outBuf, "/usr/bin/open -a \"%s\" \"%s\"", [[[NSBundle mainBundle] bundlePath] cStringUsingEncoding:NSUTF8StringEncoding], gDownloadPathMap[download].getStr());
#ifdef DEBUG
		fprintf( stderr, "Opening using: %s\n", outBuf );
#endif
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
	[mpstatusLabel setString:GetLocalizedString(NEOMOBILEDOWNLOADFAILED)];
	// +++ ADD SERVER FALLBACK DOWNLOAD HERE
}

- (void)windowDidMove:(NSNotification *)notification
{
	NSWindow *window=[notification object];
	if(window && (window==mpPanel) && ![window isZoomed])
	{
		NSPoint windowPos=[window frame].origin;
		NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
		[defaults setObject:[NSString stringWithFormat:@"%d", (int)windowPos.x] forKey:kNeoMobileXPosPref];
		[defaults setObject:[NSString stringWithFormat:@"%d", (int)windowPos.y] forKey:kNeoMobileYPosPref];
		[defaults synchronize];
	}
}

- (void)windowDidResize:(NSNotification *)notification
{
	NSWindow *window=[notification object];
	if(window && (window==mpPanel) && ![window isZoomed])
	{
		NSView *pContentView = [window contentView];
		if(pContentView)
		{
			NSSize contentSize=[pContentView frame].size;
			NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
			[defaults setObject:[NSString stringWithFormat:@"%d", (int)contentSize.width] forKey:kNeoMobileWidthPref];
			[defaults setObject:[NSString stringWithFormat:@"%d", (int)contentSize.height] forKey:kNeoMobileHeightPref];
			[defaults synchronize];
		}
	}
}

- (void)windowWillClose:(NSNotification *)notification
{
	NSWindow *window=[notification object];
	if(window && (window==mpPanel))
	{
		NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
		[defaults setBool:NO forKey:kNeoMobileVisiblePref];
		[defaults synchronize];
	}
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

- (MacOSBOOL)isNeoMobileURL:(NSURL *)pURL
{
	// Make sure that the list of servers has been populated
	if ( !mpBaseURLs )
		return NO;

	if ( !pURL )
		return NO;

	NSString *pURLHost = [pURL host];
	if ( !pURLHost || ![pURLHost length] )
		return NO;

	for ( unsigned int i = 0; i < mnBaseURLCount; i++ )
	{
		NSURL *pBaseURL = [NSURL URLWithString:(NSString *)[mpBaseURLs objectAtIndex:i]];
		if ( !pBaseURL )
			continue;
		NSString *pBaseHost = [pBaseURL host];
		if(!pBaseHost || ![pBaseHost length])
			continue;
		else if ([pBaseHost caseInsensitiveCompare:pURLHost] == NSOrderedSame)
			return YES;
	}

	return NO;
}

@end

static NonRecursiveResponderPanel *pCurrentPanel = nil;

@implementation NonRecursiveResponderPanel

- (void)dealloc
{
	[self setContentView:nil];

	if ( mpcontentView )
		[mpcontentView release];

	if ( mpcancelButton )
		[mpcancelButton release];

	if ( mpstatusLabel )
		[mpstatusLabel release];

	if ( mpwebView )
		[mpwebView release];

	[super dealloc];
}

- (id)initWithUserAgent:(NSString *)pUserAgent
{
	[super initWithContentRect:NSMakeRect(0, 0, kNMDefaultBrowserWidth, kNMDefaultBrowserHeight+kNMBottomViewHeight) styleMask:NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSUtilityWindowMask backing:NSBackingStoreBuffered defer:YES];
	[self setFloatingPanel:YES];
	[self setMinSize: NSMakeSize(kNMDefaultBrowserWidth, 90)];
	[self setTitle: GetLocalizedString(NEOMOBILEPRODUCTNAME)];
	
	mpcontentView=[[NSView alloc] initWithFrame:NSMakeRect(0, 0, kNMDefaultBrowserWidth, kNMDefaultBrowserHeight+kNMBottomViewHeight)];
	[mpcontentView setAutoresizesSubviews:YES];
	
	mpcancelButton=[[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 100, kNMBottomViewHeight)];
	[mpcancelButton setTitle:GetLocalizedString(NEOMOBILECANCEL)];
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
	
	mpstatusLabel=[[NSText alloc] initWithFrame:NSMakeRect([mpcancelButton bounds].size.width, 0, kNMDefaultBrowserWidth-[mpcancelButton bounds].size.width, [mpcancelButton bounds].size.height)];
	[mpstatusLabel setEditable:NO];
	[mpstatusLabel setString:@""];
	[mpstatusLabel setAutoresizingMask:(NSViewWidthSizable)];
	[mpstatusLabel setDrawsBackground:NO];
	
	fontSize=[NSFont systemFontSize];
	theFont = [NSFont fontWithName:[theFont fontName] size:fontSize];
	[mpstatusLabel setFont:theFont];
	
	NSView *bottomView=[[NSView alloc] initWithFrame:NSMakeRect(0, 0, kNMDefaultBrowserWidth, [mpcancelButton bounds].size.height)];
	[bottomView setAutoresizesSubviews:YES];
	[bottomView setAutoresizingMask:(NSViewWidthSizable)];
	
	[bottomView addSubview:mpcancelButton];
	[bottomView addSubview:mpstatusLabel];
	[mpcontentView addSubview:bottomView];
	
	mpwebView = [[NeoMobileWebView alloc] initWithFrame:NSMakeRect(0, kNMBottomViewHeight, kNMDefaultBrowserWidth, kNMDefaultBrowserHeight) cancelButton:mpcancelButton statusLabel:mpstatusLabel userAgent:pUserAgent];
	[mpwebView setAutoresizingMask:(NSViewHeightSizable | NSViewWidthSizable)];
	[mpcontentView addSubview:mpwebView];
	[mpcancelButton setTarget:mpwebView];
	
	[self setContentView:mpcontentView];
	[self setDelegate:mpwebView];
	
	return self;
}

- (BOOL)tryToPerform:(SEL)aAction with:(id)aObject
{
	BOOL bRet = NO;

	// Fix bug 3525 by preventing infinite recursion
	if ( pCurrentPanel == self )
		return bRet;

	pCurrentPanel = self;
	bRet = [super tryToPerform:aAction with:aObject];
	pCurrentPanel = nil;

	return bRet;
}

- (NeoMobileWebView *)webView
{
	return mpwebView;
}

- (NSRect)windowWillUseStandardFrame:(NSWindow *)pWindow defaultFrame:(NSRect)aFrame
{
	NSRect aRet = aFrame;

	if ( pWindow == self )
	{
		aRet = [pWindow frame];

		NSView *pContentView = [pWindow contentView];
		if ( pContentView )
		{
			// Make the window only big enough to show the bottom view
			NSView *pBottomView = nil;
			NSArray *pSubviews = [pContentView subviews];
			if ( pSubviews )
			{
				unsigned int nCount = [pSubviews count];
				unsigned int i = 0;
				for ( ; i < nCount; i++ )
				{
					NSView *pView = (NSView *)[pSubviews objectAtIndex:i];
					if ( pView && ( !pBottomView || [pView frame].origin.y < [pBottomView frame].origin.y ) )
						pBottomView = pView;
				}
			}

			NSRect aContentFrame = [pContentView frame];
			aContentFrame.origin.x += aRet.origin.x;
			aContentFrame.origin.y += aRet.origin.y + aContentFrame.size.height;
			if ( pBottomView )
			{
				NSRect aBottomFrame = [pBottomView frame];
				aContentFrame.origin.y -= aBottomFrame.size.height;
				aContentFrame.size.height = aBottomFrame.origin.y + aBottomFrame.size.height;
			}
			else
			{
				aContentFrame.size.height = 0;
			}
				
			aRet = [NSWindow frameRectForContentRect:aContentFrame styleMask:[pWindow styleMask]];

			
		}
	}

	return aRet;
}

@end
