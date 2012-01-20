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

#import "update_cocoa.hxx"
#import "updatei18n_cocoa.hxx"
#import "updatewebview_cocoa.h"

#import <map>

#include "premac.h"
#import <objc/objc-class.h>
#include "postmac.h"

#ifndef NSDownloadsDirectory
#define NSDownloadsDirectory ((NSSearchPathDirectory)15)
#endif

#define kUpdateMaxInZoomHeight ( kUpdateDefaultBrowserHeight / 2 )
#define kUpdateBottomViewPadding 2

static const NSTimeInterval kBaseURLIncrementInterval = 5 * 60;
static const NSString *kDownloadURI = @".dmg";

static const NSString *pDevelopmentBaseURLs[] = {
	@"http://localhost/"
};

static const NSString *pTestBaseURLs[] = {
	// Force automatic server fallback during testing
	@"https://127.0.0.2/",
#ifndef DEBUG
	@"https://www-test.neooffice.org/",
#endif	// !DEBUG
	@"https://www-test-primary.neooffice.org/",
	@"https://www-test-backup.neooffice.org/",
	@"https://www-test-backup2.neooffice.org/",
	@"https://www-test-backup3.neooffice.org/"
};

#ifndef TEST
static const NSString *pProductionBaseURLs[] = {
#ifndef DEBUG
	@"https://www.neooffice.org/",
#endif	// !DEBUG
	@"https://www-primary.neooffice.org/",
	@"https://www-backup.neooffice.org/",
	@"https://www-backup2.neooffice.org/",
	@"https://www-backup3.neooffice.org/"
};
#endif	// !TEST

using namespace rtl;

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

@interface UpdateDownloadData : NSObject
{
	unsigned long			mnBytesReceived;
	NSURLDownload*			mpDownload;
	long long				mnExpectedContentLength;
	NSString*				mpMIMEType;
	NSString*				mpPath;
}
- (void)addBytesReceived:(unsigned long)nBytesReceived;
- (unsigned long)bytesReceived;
- (void)dealloc;
- (long long)expectedContentLength;
- (id)initWithDownload:(NSURLDownload *)pDownload expectedContentLength:(long long)nExpectedContentLength MIMEType:(NSString *)pMIMEType;
- (NSString *)MIMEType;
- (NSString *)path;
- (void)setPath:(NSString *)pPath;
@end

@implementation UpdateDownloadData

- (void)addBytesReceived:(unsigned long)nBytesReceived
{
	mnBytesReceived += nBytesReceived;
}

- (unsigned long)bytesReceived
{
	return mnBytesReceived;
}

- (void)dealloc
{
	if (mpDownload)
		[mpDownload release];
	if (mpMIMEType)
		[mpMIMEType release];
	if (mpPath)
		[mpPath release];

	[super dealloc];
}

- (long long)expectedContentLength
{
	return mnExpectedContentLength;
}

- (id)initWithDownload:(NSURLDownload *)pDownload expectedContentLength:(long long)nExpectedContentLength MIMEType:(NSString *)pMIMEType
{
	[super init];

	mpDownload = pDownload;
	if (mpDownload)
		[mpDownload retain];
	mnBytesReceived = 0;
	mnExpectedContentLength = nExpectedContentLength;
	mpMIMEType = pMIMEType;
	if (mpMIMEType)
		[pMIMEType retain];
	mpPath = nil;

	return self;
}

- (NSString *)MIMEType
{
	return mpMIMEType;
}

- (NSString *)path
{
	return mpPath;
}

- (void)setPath:(NSString *)pPath;
{
	mpPath = pPath;
	if (mpPath)
		[mpPath retain];
}

@end

@interface UpdateStatusBarView : NSView
- (void)drawRect:(NSRect)dirtyRect;
@end

@implementation UpdateStatusBarView

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];

	NSRect bounds = [self bounds];
	[[NSColor blackColor] set];
	NSBezierPath *bezierPath = [NSBezierPath bezierPath];
	[bezierPath setLineWidth:0.5];
	[bezierPath moveToPoint:NSMakePoint( bounds.origin.x, bounds.origin.y + bounds.size.height - 0.5 )];
	[bezierPath lineToPoint:NSMakePoint( bounds.origin.x + bounds.size.width, bounds.origin.y + bounds.size.height - 0.5 )];
	[bezierPath stroke];
}

@end

static unsigned int updateBaseURLEntry = 0;
static unsigned int updateBaseURLCount = 0;
static NSArray *updateBaseURLEntries = nil;
static NSString *updateServerType = nil;
static NSTimeInterval lastBaseURLIncrementTime = 0;
static unsigned int baseURLIncrements = 0;
static MacOSBOOL bWebJavaScriptTextInputPanelSwizzeled = NO;
static std::map< NSURLDownload*, UpdateDownloadData* > aDownloadDataMap;
static NSMutableDictionary *pRetryDownloadURLs = nil;

@implementation UpdateWebView

+ (const NSString *)appendUpdateServerNameToString:(const NSString *)pString
{
	const NSString *pRet = ( pString ? pString : @"" );

	[UpdateWebView updateURL];
	if (updateServerType && [updateServerType length])
		pRet = [pRet stringByAppendingFormat:@" %@", updateServerType];

	return pRet;
}

		
+ (NSString *)updateURL
{
	if (!updateBaseURLEntries)
	{
		// Determine which server type to use. The default server type can be
		// overridden using the following Terminal command:
		//   defaults write org.neooffice.NeoOffice nmServerType development|test
		// To use the default server type, use the following Terminal command:
		//   defaults delete org.neooffice.NeoOffice nmServerType
		unsigned int nBaseURLCount = 0;
		const NSString **pBaseURLs = nil;
		NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
		NSString *serverType=[defaults stringForKey:kUpdateServerTypePref];
		if ( serverType )
		{
			if ( [serverType caseInsensitiveCompare:@"development"] == NSOrderedSame )
			{
				nBaseURLCount = sizeof( pDevelopmentBaseURLs ) / sizeof( NSString* );
				pBaseURLs = pDevelopmentBaseURLs;
				updateServerType = @"development";
			}
			else if ( [serverType caseInsensitiveCompare:@"test"] == NSOrderedSame )
			{
				nBaseURLCount = sizeof( pTestBaseURLs ) / sizeof( NSString* );
				pBaseURLs = pTestBaseURLs;
				updateServerType = @"test";
			}
		}
		if ( !pBaseURLs )
		{
#ifdef TEST
			nBaseURLCount = sizeof( pTestBaseURLs ) / sizeof( NSString* );
			pBaseURLs = pTestBaseURLs;
#else	// TEST
			nBaseURLCount = sizeof( pProductionBaseURLs ) / sizeof( NSString* );
			pBaseURLs = pProductionBaseURLs;
#endif	// TEST
		}

		NSMutableArray *pNewURLEntries = [NSMutableArray arrayWithCapacity:nBaseURLCount];
		if ( pNewURLEntries )
		{
			unsigned int i = 0;
			for ( ; i < nBaseURLCount; i++ )
			{
				if ( pBaseURLs[ i ] )
				{
					NSString *pBaseURL = [pBaseURLs[ i ] stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
					unsigned int len = [pBaseURL length];
					if ( len && [pBaseURL characterAtIndex:len - 1] != (unichar)'/' )
						pBaseURL = [pBaseURL stringByAppendingString:@"/"];
					[pNewURLEntries addObject:pBaseURL];
				}
			}

			updateBaseURLEntries = pNewURLEntries;
			[updateBaseURLEntries retain];
			updateBaseURLCount = [updateBaseURLEntries count];
		}
		else
		{
			updateBaseURLCount = 0;
		}
	}

	if (updateBaseURLEntry >= updateBaseURLCount)
		updateBaseURLEntry = 0;

	return (NSString *)[updateBaseURLEntries objectAtIndex:updateBaseURLEntry];
}

+ (MacOSBOOL)isDownloadURL:(NSURL *)url {
	if (!url)
		return(NO);
	NSString *path = [url path];
	if (!path || [path length] < [kDownloadURI length])
		return(NO);
	NSRange range = NSMakeRange([path length] - [kDownloadURI length], [kDownloadURI length]);
	return ([[url path] compare:(NSString *)kDownloadURI options:0 range:range]==NSOrderedSame && [UpdateWebView isUpdateURL:url syncServer:NO]);
}

+ (MacOSBOOL)isUpdateURL:(NSURL *)url syncServer:(MacOSBOOL)syncServer
{
	// Make sure that the list of servers has been populated
	[UpdateWebView updateURL];
	if(!updateBaseURLEntries)
		return(NO);

	if(!url)
		return(NO);

	NSString *urlHost = [url host];
	if(!urlHost || ![urlHost length])
		return(NO);

	for(unsigned int i = 0; i < updateBaseURLCount; i++)
	{
		NSURL *updateBaseURL = [NSURL URLWithString:(NSString *)[updateBaseURLEntries objectAtIndex:i]];
		if(!updateBaseURL)
			continue;
		NSString *updateBaseHost = [updateBaseURL host];
		if(!updateBaseHost || ![updateBaseHost length])
			continue;
		else if ([updateBaseHost caseInsensitiveCompare:urlHost] == NSOrderedSame)
		{
			if (syncServer)
			    updateBaseURLEntry = i;
			return(YES);
		}
	}

	return(NO);
}

+ (MacOSBOOL)incrementUpdateBaseEntry
{
	// Make sure that the list of servers has been populated
	[UpdateWebView updateURL];
	if(!updateBaseURLEntries)
		return(NO);

	if (updateBaseURLCount < 2)
		return(NO);
	
	if (++updateBaseURLEntry >= updateBaseURLCount)
		updateBaseURLEntry = 0;
	
    NSTimeInterval oldLastBaseURLIncrementTime = lastBaseURLIncrementTime;
    lastBaseURLIncrementTime = [NSDate timeIntervalSinceReferenceDate];
	if (++baseURLIncrements >= updateBaseURLCount)
	{
		baseURLIncrements = 0;
		if (oldLastBaseURLIncrementTime + kBaseURLIncrementInterval > lastBaseURLIncrementTime)
			return(NO);
	}
	
	return(YES);
}

- (id)initWithFrame:(NSRect)aFrame panel:(UpdateNonRecursiveResponderWebPanel *)pPanel backButton:(NSButton *)pBackButton cancelButton:(NSButton *)pCancelButton loadingIndicator:(NSProgressIndicator *)pLoadingIndicator statusLabel:(NSText *)pStatusLabel userAgent:(const NSString *)pUserAgent
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
					class_addMethod( aClass, aSelector, WebJavaScriptTextInputPanel_windowDidLoadIMP, [pSignature methodReturnType] );

					bWebJavaScriptTextInputPanelSwizzeled = YES;
				}
			}
		}
	}

	[super initWithFrame:aFrame frameName:nil groupName:nil];
	
	mpPanel = pPanel;
	[mpPanel retain];
	
	mpbackButton = pBackButton;
	[mpbackButton retain];
	
	mpcancelButton = pCancelButton;
	[mpcancelButton retain];
	
	mploadingIndicator = pLoadingIndicator;
	[mploadingIndicator retain];
	
	mpstartingURL = nil;

	mpstatusLabel = pStatusLabel;
	[mpstatusLabel retain];

	NSHTTPCookieStorage *pCookieStorage = [NSHTTPCookieStorage sharedHTTPCookieStorage];
	if ( pCookieStorage )
		[pCookieStorage setCookieAcceptPolicy:NSHTTPCookieAcceptPolicyAlways];

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

	return self;
}

- (IBAction)backButtonPressed
{
#ifdef DEBUG
	fprintf(stderr, "Update Back Button Clicked\n");
#endif
	[self stopLoading:self];
	if ([self canGoBack])
		[self goBack];
	else
		[self loadStartingURL:nil];
}

- (IBAction)cancelButtonPressed
{
#ifdef DEBUG
	fprintf(stderr, "Update Cancel Button Clicked\n");
#endif
	[self stopLoading:self];
}

- (void)loadStartingURL:(NSURL *)pStartingURL
{
	if ( pStartingURL && pStartingURL != mpstartingURL )
	{
		if ( mpstartingURL )
			[mpstartingURL release];
		mpstartingURL = pStartingURL;
		if ( mpstartingURL )
			[mpstartingURL retain];
	}

	if ( mpstartingURL )
	{
		NSMutableURLRequest *loadRequest =(NSMutableURLRequest *)[NSMutableURLRequest requestWithURL:mpstartingURL];
		if(loadRequest)
			[[self mainFrame] loadRequest:loadRequest];
	}
}

- (void)reloadFrameWithNextServer:(WebFrame *)pWebFrame reason:(NSError *)pError
{
	int errCode = pError ? [pError code] : 0;

	if ( !errCode || errCode == WebKitErrorFrameLoadInterruptedByPolicyChange )
	{
		// NOTE: we don't want to trigger the server fallback if we are just
		// processing a data download we've redirected from the web frame
		return;
	}
	else if ( errCode == NSURLErrorCancelled )
	{
		// Error code -999 indicates that the WebKit is doing the Back, Reload,
		// or Forward actions so we don't trigger server fallback. These
		// actions are incompatible with our export event code so cancel
		// the current export event
		return;
	}

	[mploadingIndicator setHidden:YES];
	[mpcancelButton setEnabled:NO];
	[mpstatusLabel setString:@""];

	if ( pWebFrame )
	{
		MacOSBOOL bShowErrorAfterReload = NO;
		WebDataSource *pDataSource = [pWebFrame provisionalDataSource];
		if ( pDataSource )
		{
			// Don't reload downloads as it will leave a blank screen
			NSMutableURLRequest *pRequest = [pDataSource request];
			if ( !pRequest || [UpdateWebView isDownloadURL:[pRequest URL]] )
			{
				bShowErrorAfterReload = YES;
				pDataSource = nil;
			}
		}
		if ( !pDataSource )
			pDataSource = [pWebFrame dataSource];
		if ( pDataSource )
		{
			NSMutableURLRequest *pRequest = [pDataSource request];
			if ( pRequest )
			{
				NSURL *pURL = [pRequest URL];
				// The URL may be empty on first load
				if ( !pURL || ![[pURL absoluteString] length] )
					pURL = [NSURL URLWithString:[UpdateWebView updateURL]];
				if ( pURL && [UpdateWebView isUpdateURL:pURL syncServer:NO] && ![UpdateWebView isDownloadURL:pURL] && ( errCode == NSURLErrorTimedOut || errCode == NSURLErrorCannotFindHost || errCode == NSURLErrorCannotConnectToHost ) )
				{
					if ( [UpdateWebView incrementUpdateBaseEntry] )
					{
						// Reconstruct URL with current NeoOffice Mobile server
						NSMutableString *pURI = [NSMutableString stringWithString:[UpdateWebView updateURL]];
						if ( pURI )
						{
							NSString *pPath = [pURL path];
							if ( pPath )
							{
								while ( [pPath length] && [pPath characterAtIndex:0] == (unichar)'/' )
									pPath = [pPath substringFromIndex:1];
								[pURI appendString:[pPath stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
							}
							NSString *pParams = [pURL parameterString];
							if ( pParams )
							{
								[pURI appendString:@";"];
								[pURI appendString:[pParams stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
							}
							NSString *pQuery = [pURL query];
							if ( pQuery )
							{
								[pURI appendString:@"?"];
								[pURI appendString:[pQuery stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
							}
							NSString *pFragment = [pURL fragment];
							if ( pFragment )
							{
								[pURI appendString:@"#"];
								[pURI appendString:[pFragment stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
							}

							NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
							if ( defaults )
							{
								// Only reload this activity if we were able to
								// save the URL. Otherwise we will be in an
								// endless reload loop.
								[defaults setObject:pURI forKey:[UpdateWebView appendUpdateServerNameToString:kUpdateLastURLPref]];
								[defaults synchronize];
		
								pURL = [NSURL URLWithString:pURI];
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
										[mpPanel createWebView:pNewRequest];
										if ( !bShowErrorAfterReload )
											return;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	NSAlert *pAlert = [NSAlert alertWithMessageText:[NSString stringWithFormat:@"%@ %@", UpdateGetLocalizedString(UPDATEERROR), [pError localizedDescription]] defaultButton:nil alternateButton:nil otherButton:nil informativeTextWithFormat:@""];
	if ( pAlert )
		[pAlert runModal];
}

- (void)stopLoading:(id)pSender
{
	if(aDownloadDataMap.size())
	{
		// file downloads in progress, so cancel them
		
		if (pRetryDownloadURLs)
			[pRetryDownloadURLs removeAllObjects];

		for(std::map< NSURLDownload*, UpdateDownloadData* >::const_iterator it = aDownloadDataMap.begin(); it != aDownloadDataMap.end(); ++it)
		{
			[it->first cancel];
			[it->second release];
		}
		aDownloadDataMap.clear();

		[mploadingIndicator setHidden:YES];
		[mpcancelButton setEnabled:NO];
		[mpstatusLabel setString:UpdateGetLocalizedString(UPDATEDOWNLOADCANCELED)];
	}
	else
	{
		[mploadingIndicator setHidden:YES];
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
	[mploadingIndicator setHidden:YES];
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

	if ( ![UpdateWebView isDownloadURL:pURL] )
	{
		// If not an upload, save last URL preference
		NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
		[defaults setObject:[pURL absoluteString] forKey:[UpdateWebView appendUpdateServerNameToString:kUpdateLastURLPref]];
		[defaults synchronize];
	}
}

- (void)webView:(WebView *)pWebView didStartProvisionalLoadForFrame:(WebFrame *)pFrame
{
	[mploadingIndicator setHidden:NO];
	[mploadingIndicator startAnimation:self];
	[mpcancelButton setEnabled:YES];

	if ( !pWebView || !pFrame )
		return;

	WebDataSource *pDataSource = [pFrame provisionalDataSource];
	
	NSMutableURLRequest *pRequest = nil;
	if ( pDataSource )
		pRequest = [pDataSource request];
	
	[mpstatusLabel setString:UpdateGetLocalizedString(UPDATELOADING)];
}

- (NSURLRequest *)webView:(WebView *)pWebView resource:(id)aIdentifier willSendRequest:(NSURLRequest *)pRequest redirectResponse:(NSURLResponse *)pRedirectResponse fromDataSource:(WebDataSource *)pDataSource
{
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

	NSAlert *pAlert = [NSAlert alertWithMessageText:pMessage defaultButton:nil alternateButton:UpdateGetLocalizedString(UPDATECANCEL) otherButton:nil informativeTextWithFormat:@""];
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

	// Use NSDownloadsDirectory
	downloadPaths = NSSearchPathForDirectoriesInDomains(NSDownloadsDirectory, NSUserDomainMask, YES);
    if (!downloadPaths)
		downloadPaths = NSSearchPathForDirectoriesInDomains(NSDesktopDirectory, NSUserDomainMask, YES);

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
	
	[download setDestination:filePath allowOverwrite:NO];
}

- (void)download:(NSURLDownload *)download didCreateDestination:(NSString *)path
{
#ifdef DEBUG
	fprintf( stderr, "Download didCreateDestination: %s\n", [[[[download request] URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif

	std::map< NSURLDownload*, UpdateDownloadData* >::const_iterator it = aDownloadDataMap.find(download);
	if(it!=aDownloadDataMap.end())
		[it->second setPath:path];
}

- (void)download: (NSURLDownload *) download didReceiveResponse: (NSURLResponse *) response
{
#ifdef DEBUG
	fprintf( stderr, "Update Download didReceiveResponse\n");
#endif

	std::map< NSURLDownload*, UpdateDownloadData* >::const_iterator it = aDownloadDataMap.find(download);
	if(it!=aDownloadDataMap.end())
		[it->second release];
	aDownloadDataMap[download]=[[UpdateDownloadData alloc] initWithDownload:download expectedContentLength:[response expectedContentLength] MIMEType:[response MIMEType]];
}

- (void)downloadDidBegin: (NSURLDownload *)download
{
#ifdef DEBUG
	fprintf( stderr, "Download File Did Begin: %s\n", [[[[download request] URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif

	std::map< NSURLDownload*, UpdateDownloadData* >::iterator it = aDownloadDataMap.find(download);
	if(it!=aDownloadDataMap.end())
	{
		[mploadingIndicator setHidden:NO];
		[mploadingIndicator startAnimation:self];
		[mpcancelButton setEnabled:YES];
		[mpstatusLabel setString:UpdateGetLocalizedString(UPDATEDOWNLOADINGFILE)];
	}
}

- (void)download:(NSURLDownload *)download didReceiveDataOfLength:(unsigned long)length
{
#ifdef DEBUG
	fprintf( stderr, "Update Download didReceiveDataOfLength\n");
#endif

	std::map< NSURLDownload*, UpdateDownloadData* >::iterator it = aDownloadDataMap.find(download);
	if(it!=aDownloadDataMap.end())
	{
		[it->second addBytesReceived:length];

		unsigned long nBytesReceived=[it->second bytesReceived];
		long long nExpectedContentLength=[it->second expectedContentLength];
		if(nExpectedContentLength > 0)
		{
			// we got a response from the server, so we can compute a percentage
			[mpstatusLabel setString:[NSString stringWithFormat:@"%@ %d%%", UpdateGetLocalizedString(UPDATEDOWNLOADINGFILE), (int)((double)nBytesReceived/(double)nExpectedContentLength*100)]];
		}
		else
		{
			// no expected size received from the server, just show Kb download
			[mpstatusLabel setString:[NSString stringWithFormat:@"%@ %ldK", UpdateGetLocalizedString(UPDATEDOWNLOADINGFILE), (long)(nBytesReceived/1024)]];
		}
	}
}

- (void)downloadDidFinish: (NSURLDownload*)download
{
#ifdef DEBUG
	fprintf( stderr, "Download File Did End: %s\n", [[[[download request] URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif
	std::map< NSURLDownload*, UpdateDownloadData* >::iterator it = aDownloadDataMap.find(download);
	if(it!=aDownloadDataMap.end())
	{
		NSString *path = [it->second path];
		if (path)
		{
			// Check if downloaded file size matches content length header
			long long nExpectedContentLength=[it->second expectedContentLength];
			NSFileManager *pFileManager = [NSFileManager defaultManager];
			if(nExpectedContentLength > 0 && pFileManager)
			{
				MacOSBOOL bFileHasAllBits = NO;
				NSDictionary *pFileAttrs = [pFileManager attributesOfItemAtPath:path error:nil];
				if (pFileAttrs)
				{
					// Downloaded file should never be directory or softlink
					NSString *pFileType = [pFileAttrs fileType];
					if ([pFileType isEqualToString:NSFileTypeRegular])
					{
						unsigned long long nFileSize = [pFileAttrs fileSize];
						if (nFileSize == (unsigned long long)nExpectedContentLength)
							bFileHasAllBits = YES;
					}
				}

				NSString *pURLPath = [[[download request] URL] absoluteString];
				if (!bFileHasAllBits)
				{
					[pFileManager removeItemAtPath:path error:nil];
					NSError *pError = [NSError errorWithDomain:@"NSURLErrorDomain" code:NSURLErrorNetworkConnectionLost userInfo:nil];
					[self download:download didFailWithError:pError];
					return;
				}
				else if (pRetryDownloadURLs)
				{
					[pRetryDownloadURLs removeObjectForKey:pURLPath];
				}
			}

			NSString *MIMEType = [it->second MIMEType];
			if([MIMEType rangeOfString: @"application/vnd.oasis.opendocument"].location != NSNotFound || [MIMEType rangeOfString: @"application/ms"].location != NSNotFound)
				[NSTask launchedTaskWithLaunchPath:@"/usr/bin/open" arguments:[NSArray arrayWithObjects:@"-a", [[NSBundle mainBundle] bundlePath], [it->second path], nil]];
			else
				[NSTask launchedTaskWithLaunchPath:@"/usr/bin/open" arguments:[NSArray arrayWithObjects:[it->second path], nil]];
#ifdef DEBUG
			fprintf( stderr, "Opening file: %s\n", [[it->second path] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif
		}

		[it->second release];
		aDownloadDataMap.erase(it);
	}

	if(!aDownloadDataMap.size())
	{
		[mploadingIndicator setHidden:YES];
		[mpcancelButton setEnabled:NO];
		[mpstatusLabel setString:@""];
	}
}

- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error
{
#ifdef DEBUG
	NSLog( @"Download didFailWithError: %@", error );
#endif

	std::map< NSURLDownload*, UpdateDownloadData* >::iterator it = aDownloadDataMap.find(download);
	if(it!=aDownloadDataMap.end())
	{
		if (!pRetryDownloadURLs)
		{
			pRetryDownloadURLs = [NSMutableDictionary dictionaryWithCapacity:1];
			if (pRetryDownloadURLs)
				[pRetryDownloadURLs retain];
		}

		MacOSBOOL bRetry = NO;
		NSString *pURLPath = [[[download request] URL] absoluteString];
		if (pRetryDownloadURLs && ![pRetryDownloadURLs objectForKey:pURLPath])
		{
			[pRetryDownloadURLs setObject:pURLPath forKey:pURLPath];
			[[self mainFrame] loadRequest:[download request]];
			bRetry = YES;
		}
		else
		{
			[pRetryDownloadURLs removeObjectForKey:pURLPath];
		}

		[it->second release];
		aDownloadDataMap.erase(it);

		// Display error dialog
		if (bRetry)
			return;
		else
			[self reloadFrameWithNextServer:[self mainFrame] reason:error];
	}

	if(!aDownloadDataMap.size())
	{
		[mploadingIndicator setHidden:YES];
		[mpcancelButton setEnabled:NO];
		[mpstatusLabel setString:UpdateGetLocalizedString(UPDATEDOWNLOADFAILED)];
	}
}

- (void)webView:(WebView *)sender decidePolicyForNavigationAction:(NSDictionary *)actionInformation
        request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id)listener
{
#ifdef DEBUG
	fprintf( stderr, "Update Loading URL: %s\n", [[[request URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif
	[listener use];
}

- (void)webView:(WebView *)sender decidePolicyForMIMEType:(NSString *)type request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id < WebPolicyDecisionListener >)listener
{
	if([type rangeOfString: @"application/"].location != NSNotFound)
	{
		[listener download];
	}
	else if([WebView canShowMIMEType:type])
	{
		[listener use];
	}
	else
	{
		[listener download];
	}
}

- (void)dealloc
{
	if ( mpPanel )
		[mpPanel release];
	
	if ( mpbackButton )
		[mpbackButton release];
	
	if ( mpcancelButton )
		[mpcancelButton release];
	
	if ( mploadingIndicator )
		[mploadingIndicator release];
	
	if ( mpstartingURL )
		[mpstartingURL release];

	if ( mpstatusLabel )
		[mpstatusLabel release];
	
	[super dealloc];
}

@end

static NonRecursiveResponderPanel *pCurrentPanel = nil;

@implementation NonRecursiveResponderPanel

- (void)adjustBottomOfControlToTextHeight:(NSControl *)pControl
{
	if ( pControl )
	{
		NSRect aFrame = [pControl frame];
		float fHeightAdjustment = aFrame.size.height-[[pControl cell] cellSize].height;
		aFrame.origin.y += fHeightAdjustment;
		aFrame.size.height -= fHeightAdjustment;
		[pControl setFrame:aFrame];
	}
}

- (void)centerTextInTextView:(NSText *)pTextView
{
	if ( pTextView )
	{
		NSFont *pFont = [pTextView font];
		if ( pFont )
		{
			// Count the descender twice as the descender is used as the
			// padding for the first line
			NSRect aFrame = [pTextView frame];
			aFrame.size.height -= ( aFrame.size.height - [pFont ascender] - fabs( [pFont descender] * 2 ) ) / 2;
			[pTextView setFrame:aFrame];
		}
	}
}

- (id)initWithContentRect:(NSRect)aContentRect styleMask:(NSUInteger)nWindowStyle backing:(NSBackingStoreType)nBufferingType defer:(MacOSBOOL)bDeferCreation
{
	[super initWithContentRect:aContentRect styleMask:nWindowStyle backing:nBufferingType defer:bDeferCreation];
	[self setFloatingPanel:YES];
	[self setMinSize: NSMakeSize(kUpdateDefaultBrowserWidth, 0)];
	[self setDelegate:self];

	mbinZoom = NO;

	return self;
}

- (MacOSBOOL)tryToPerform:(SEL)aAction with:(id)aObject
{
	MacOSBOOL bRet = NO;

	// Fix bug 3525 by preventing infinite recursion
	if ( pCurrentPanel == self )
		return bRet;

	pCurrentPanel = self;
	bRet = [super tryToPerform:aAction with:aObject];
	pCurrentPanel = nil;

	return bRet;
}

- (void)windowDidMove:(NSNotification *)notification
{
	if([notification object]==self && !mbinZoom)
	{
		NSRect aFrame = [self frame];
		NSRect aZoomFrame = [self windowWillUseStandardFrame:self defaultFrame:aFrame];
		if(aFrame.size.height > kUpdateMaxInZoomHeight && aFrame.size.height > aZoomFrame.size.height)
		{
			NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
			[defaults setObject:[NSString stringWithFormat:@"%d", (int)aFrame.origin.x] forKey:kUpdateXPosPref];
			[defaults setObject:[NSString stringWithFormat:@"%d", (int)aFrame.origin.y] forKey:kUpdateYPosPref];
			[defaults synchronize];
		}
	}
}

- (void)windowDidResize:(NSNotification *)notification
{
	if([notification object]==self && !mbinZoom)
	{
		NSRect aFrame = [self frame];
		NSRect aZoomFrame = [self windowWillUseStandardFrame:self defaultFrame:aFrame];
		if(aFrame.size.height > kUpdateMaxInZoomHeight && aFrame.size.height > aZoomFrame.size.height)
		{
			NSView *pContentView = [self contentView];
			if(pContentView)
			{
				NSSize contentSize=[pContentView frame].size;
				NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
				[defaults setObject:[NSString stringWithFormat:@"%d", (int)contentSize.width] forKey:kUpdateWidthPref];
				[defaults setObject:[NSString stringWithFormat:@"%d", (int)contentSize.height] forKey:kUpdateHeightPref];
				[defaults synchronize];
			}
		}
	}
}

- (void)windowWillClose:(NSNotification *)notification
{
	if([notification object]==self && !mbinZoom)
	{
		NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
		[defaults setBool:NO forKey:kUpdateVisiblePref];
		[defaults synchronize];
	}
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

- (void)zoom:(id)aObject
{
	if ( mbinZoom )
		return;
	mbinZoom = YES;

	MacOSBOOL bZoomed = YES;
	NSRect aFrame = [self frame];
	NSRect aZoomFrame = [self windowWillUseStandardFrame:self defaultFrame:aFrame];
	if(aFrame.size.height > kUpdateMaxInZoomHeight && aFrame.size.height > aZoomFrame.size.height)
		bZoomed = NO;

	[super zoom:aObject];

	// On Mac OS X 10.6 and higher the window can get stuck in a zoomed state
	// after the zoomed window has been moved. In such cases we manually resize
	// to the last saved unzoomed size.
	if (bZoomed)
	{
		aFrame = [self frame];
		if(aFrame.size.height > kUpdateMaxInZoomHeight && aFrame.size.height > aZoomFrame.size.height)
			bZoomed = NO;
	}

	if (bZoomed)
	{
		NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
		NSString *xPosStr=[defaults stringForKey:kUpdateXPosPref];
		NSString *yPosStr=[defaults stringForKey:kUpdateYPosPref];
		if(xPosStr && yPosStr)
		{
			NSPoint windowPos=[self frame].origin;
			windowPos.x=[xPosStr intValue];
			windowPos.y=[yPosStr intValue];
			[self setFrameOrigin:windowPos];
		}

		NSView *pContentView = [self contentView];
		if(pContentView)
		{
			NSSize contentSize=[pContentView frame].size;
			NSString *widthStr=[defaults stringForKey:kUpdateWidthPref];
			NSString *heightStr=[defaults stringForKey:kUpdateHeightPref];
 			if(widthStr)
				contentSize.width=[widthStr intValue];
			else
				contentSize.width=kUpdateDefaultBrowserWidth;
 			if(heightStr)
				contentSize.height=[heightStr intValue];
			else
				contentSize.height=kUpdateDefaultBrowserHeight;
			[self setContentSize:contentSize];
		}
	}

	mbinZoom = NO;
}

@end

@interface NSWindow (UpdateNonRecursiveResponderWebPanel)
- (NSRect)_growBoxRect;
@end

@implementation UpdateNonRecursiveResponderWebPanel

- (void)createWebView:(NSURLRequest *)pRequest
{
	if ( mpwebView )
	{
		[[mpwebView mainFrame] stopLoading];
		[mpbackButton setTarget:nil];
		[mpcancelButton setTarget:nil];
		[mpwebView removeFromSuperviewWithoutNeedingDisplay];
		[mpwebView release];
		mpwebView = nil;
	}

	mpwebView = [[UpdateWebView alloc] initWithFrame:NSMakeRect(0, [mpbottomView bounds].size.height, [mpcontentView bounds].size.width, [mpcontentView bounds].size.height-[mpbottomView bounds].size.height) panel:self backButton:mpbackButton cancelButton:mpcancelButton loadingIndicator:mploadingIndicator statusLabel:mpstatusLabel userAgent:mpuserAgent];
	[mpwebView setAutoresizingMask:(NSViewHeightSizable | NSViewWidthSizable)];
	[mpcontentView addSubview:mpwebView];
	[mpbackButton setTarget:mpwebView];
	[mpbackButton setAction:@selector(backButtonPressed)];
	[mpcancelButton setTarget:mpwebView];
	[mpcancelButton setAction:@selector(cancelButtonPressed)];

	if ( pRequest )
		[[mpwebView mainFrame] loadRequest:pRequest];
}

- (void)dealloc
{
	[self setContentView:nil];

	if ( mpbackButton )
		[mpbackButton release];
	
	if ( mpbottomView )
		[mpbottomView release];

	if ( mpcancelButton )
		[mpcancelButton release];

	if ( mpcontentView )
		[mpcontentView release];

	if ( mploadingIndicator )
		[mploadingIndicator release];
	
	if ( mpstatusLabel )
		[mpstatusLabel release];

	if ( mpuserAgent )
		[mpuserAgent release];

	if ( mpwebView )
		[mpwebView release];

	[super dealloc];
}

- (id)initWithUserAgent:(NSString *)pUserAgent title:(NSString *)pTitle
{
	[super initWithContentRect:NSMakeRect(0, 0, kUpdateDefaultBrowserWidth, kUpdateDefaultBrowserHeight) styleMask:NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSUtilityWindowMask backing:NSBackingStoreBuffered defer:YES];
	if ( pTitle )
		[self setTitle:pTitle];
	
	mpuserAgent = pUserAgent;
	if ( mpuserAgent )
		[mpuserAgent retain];

	mpcontentView=[[NSView alloc] initWithFrame:[[self contentView] frame]];
	[mpcontentView setAutoresizesSubviews:YES];
	
	NSSize contentSize=[mpcontentView bounds].size;
	NSSize buttonSize = NSMakeSize( 30, 30 );
	NSImage *backImage = [[NSImage alloc] initWithSize:buttonSize];
	NSImage *cancelImage = [[NSImage alloc] initWithSize:buttonSize];

	// Autorelease images as they will be retained by their respective buttons
	[backImage autorelease];
	[cancelImage autorelease];

	NSView *focusView = [NSView focusView];
	if ( focusView )
		[focusView unlockFocus];

	[backImage lockFocus];
	[[NSColor blackColor] set];
	NSBezierPath *bezierPath = [NSBezierPath bezierPath];
	[bezierPath moveToPoint:NSMakePoint( 10, 15 )];
	[bezierPath lineToPoint:NSMakePoint( 20, 10 )];
	[bezierPath lineToPoint:NSMakePoint( 20, 20 )];
	[bezierPath closePath];
	NSAffineTransform *imageTransform = [NSAffineTransform transform];
	[imageTransform translateXBy:-1 yBy:-1];
	[bezierPath transformUsingAffineTransform:imageTransform];
	[bezierPath fill];
	[backImage unlockFocus];

	[cancelImage lockFocus];
	[[NSColor blackColor] set];
	bezierPath = [NSBezierPath bezierPath];
	[bezierPath setLineWidth:2];
	[bezierPath moveToPoint:NSMakePoint( 10, 10 )];
	[bezierPath lineToPoint:NSMakePoint( 20, 20 )];
	[bezierPath moveToPoint:NSMakePoint( 10, 20 )];
	[bezierPath lineToPoint:NSMakePoint( 20, 10 )];
	imageTransform = [NSAffineTransform transform];
	[imageTransform translateXBy:0 yBy:-1];
	[bezierPath transformUsingAffineTransform:imageTransform];
	[bezierPath stroke];
	[cancelImage unlockFocus];

	if ( focusView )
		[focusView lockFocus];

	NSSize growBoxSize=NSMakeSize( 0, 0 );
	if ( [self respondsToSelector:@selector(_growBoxRect)] )
		growBoxSize=[self _growBoxRect].size;
	growBoxSize.width /= 2;

	mpcancelButton = [[NSButton alloc] initWithFrame:NSMakeRect(contentSize.width-buttonSize.width-MAX(kUpdateBottomViewPadding, growBoxSize.width), kUpdateBottomViewPadding, buttonSize.width, buttonSize.height)];
	[mpcancelButton setToolTip:UpdateGetLocalizedString(UPDATECANCEL)];
	[mpcancelButton setEnabled:YES];
	[mpcancelButton setButtonType:NSMomentaryPushInButton];
	[mpcancelButton setBezelStyle:NSRegularSquareBezelStyle];
	[mpcancelButton setImage:cancelImage];
	[mpcancelButton setImagePosition:NSImageOnly];
	[[mpcancelButton cell] setImageScaling:NSImageScaleNone];
	[mpcancelButton setAutoresizingMask:(NSViewMinXMargin)];
	
	mpbackButton = [[NSButton alloc] initWithFrame:NSMakeRect([mpcancelButton frame].origin.x-buttonSize.width-kUpdateBottomViewPadding, kUpdateBottomViewPadding, buttonSize.width, buttonSize.height)];
	[mpbackButton setToolTip:UpdateGetLocalizedString(UPDATEBACK)];
	[mpbackButton setEnabled:YES];
	[mpbackButton setButtonType:NSMomentaryPushInButton];
	[mpbackButton setBezelStyle:NSRegularSquareBezelStyle];
	[mpbackButton setImage:backImage];
	[mpbackButton setImagePosition:NSImageOnly];
	[[mpbackButton cell] setImageScaling:NSImageScaleNone];
	[mpbackButton setAutoresizingMask:(NSViewMinXMargin)];
	
	mploadingIndicator = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect([mpbackButton frame].origin.x-buttonSize.width-kUpdateBottomViewPadding, kUpdateBottomViewPadding, buttonSize.width, buttonSize.height)];
	[mploadingIndicator setStyle:NSProgressIndicatorSpinningStyle];
	[mploadingIndicator setHidden:YES];
	[mploadingIndicator setAutoresizingMask:(NSViewMinXMargin)];
	
	float maxButtonHeight=MAX([mploadingIndicator frame].origin.y+[mploadingIndicator frame].size.height-kUpdateBottomViewPadding, [mpbackButton frame].origin.y+[mpbackButton frame].size.height-kUpdateBottomViewPadding);
	mpstatusLabel=[[NSText alloc] initWithFrame:NSMakeRect(kUpdateBottomViewPadding, kUpdateBottomViewPadding, [mploadingIndicator frame].origin.x-(kUpdateBottomViewPadding*2), maxButtonHeight)];
	[mpstatusLabel setEditable:NO];
	[mpstatusLabel setString:@""];
	[mpstatusLabel setAutoresizingMask:(NSViewWidthSizable)];
	[mpstatusLabel setDrawsBackground:NO];

	NSFont *statusLabelFont=[[NSFontManager sharedFontManager] convertFont:[mpstatusLabel font] toSize:16];
	[mpstatusLabel setFont:statusLabelFont];
	[self centerTextInTextView:mpstatusLabel];

	mpbottomView=[[UpdateStatusBarView alloc] initWithFrame:NSMakeRect(0, 0, contentSize.width, maxButtonHeight+(kUpdateBottomViewPadding*2))];
	[mpbottomView setAutoresizesSubviews:YES];
	[mpbottomView setAutoresizingMask:(NSViewWidthSizable)];
	
	[mpbottomView addSubview:mpstatusLabel];
	[mpbottomView addSubview:mploadingIndicator];
	[mpbottomView addSubview:mpbackButton];
	[mpbottomView addSubview:mpcancelButton];
	[mpcontentView addSubview:mpbottomView];
	
	[self createWebView:nil];
	[self setContentView:mpcontentView];

	// Limit tabbing to only active controls
	[mpwebView setNextKeyView:mpbackButton];
	[mpbackButton setNextKeyView:mpcancelButton];
	[mpcancelButton setNextKeyView:mpwebView];

	[self setInitialFirstResponder:mpwebView];

	return self;
}

- (UpdateWebView *)webView
{
	return mpwebView;
}

@end
