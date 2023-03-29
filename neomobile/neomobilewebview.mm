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

#import <dlfcn.h>

#import "neomobile.hxx"
#import "neomobilei18n.hxx"
#import "neomobilewebview.h"
#import "neomobileflipsideview.h"
#import "NSWindow_Flipr.h"

#import <map>

#include "premac.h"
#import <objc/objc-class.h>
#include "postmac.h"

// Comment out the following line to disaable the native login window
#define USE_NATIVE_LOGIN_WINDOW

#define kNMMaxInZoomHeight ( kNMDefaultBrowserHeight / 2 )
#define kNMBottomViewPadding 2
#define kNMStatusLabelFontHeight 16.0f

typedef id Application_acquireSecurityScopedURLFromNSURL_Type( const id pNonSecurityScopedURL, unsigned char bMustShowDialogIfNoBookmark, const id pDialogTitle );
typedef void Application_releaseSecurityScopedURL_Type( id pSecurityScopedURLs );

static Application_acquireSecurityScopedURLFromNSURL_Type *pApplication_acquireSecurityScopedURLFromNSURL = NULL;
static Application_releaseSecurityScopedURL_Type *pApplication_releaseSecurityScopedURL = NULL;
static const NSTimeInterval kBaseURLIncrementInterval = 5 * 60;
static const NSString *kDownloadURI = @"/neofiles/download";

static const NSString *pDevelopmentBaseURLs[] = {
	@"http://localhost/"
};

static const NSString *pTestBaseURLs[] = {
	// Force automatic server fallback during testing
	@"https://127.0.0.2/",
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

using namespace rtl;

static NSString *GetUploadErrorURI(int error, NSString *uuid) {
	if (!uuid)
		uuid = @"";
	return [NSString stringWithFormat:@"/uploads/clienterror?errorCode=%d&uuid=%@", error, uuid];
}

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

static unsigned long long GetFileSize(NSString *pPath)
{
	long long nRet = 0;

	if (pPath && [pPath length])
	{
		NSFileManager *pFileManager = [NSFileManager defaultManager];
		if(pFileManager)
		{
			NSDictionary *pFileAttrs = [pFileManager attributesOfItemAtPath:pPath error:nil];
			if (pFileAttrs)
			{
				// Downloaded file should never be directory or softlink
				NSString *pFileType = [pFileAttrs fileType];
				if ([pFileType isEqualToString:NSFileTypeRegular])
					nRet = [pFileAttrs fileSize];
			}
		}
	}

	return nRet;
}

@interface NeoMobileDownloadData : NSObject
{
	unsigned long long		mnBytesReceived;
	NSURLDownload*			mpDownload;
	unsigned long long		mnExpectedContentLength;
	NSString*				mpFileName;
	NSDictionary*			mpHeaders;
	NSString*				mpMIMEType;
	NSString*				mpPath;
	NSURL*					mpURL;
}
- (void)addBytesReceived:(unsigned long)nBytesReceived;
- (unsigned long long)bytesReceived;
- (void)dealloc;
- (unsigned long long)expectedContentLength;
- (NSString *)fileName;
- (id)initWithDownload:(NSURLDownload *)pDownload response:(NSURLResponse *)pResponse;
- (NSString *)MIMEType;
- (NSString *)path;
- (void)setPath:(NSString *)pPath;
@end

@implementation NeoMobileDownloadData

- (void)addBytesReceived:(unsigned long)nBytesReceived
{
	mnBytesReceived += nBytesReceived;
}

- (unsigned long long)bytesReceived
{
	return mnBytesReceived;
}

- (void)dealloc
{
	if (mpDownload)
		[mpDownload release];
	if (mpFileName)
		[mpFileName release];
	if (mpHeaders)
		[mpHeaders release];
	if (mpMIMEType)
		[mpMIMEType release];
	if (mpPath)
		[mpPath release];
	if (mpURL)
		[mpURL release];

	[super dealloc];
}

- (unsigned long long)expectedContentLength
{
	return mnExpectedContentLength;
}

- (NSString *)fileName
{
	return mpFileName;
}

- (id)initWithDownload:(NSURLDownload *)pDownload response:(NSURLResponse *)pResponse
{
	[super init];

	mpDownload = pDownload;
	if (mpDownload)
		[mpDownload retain];
	mnBytesReceived = 0;
	long long nExpectedContentLength = [pResponse expectedContentLength];
	if (nExpectedContentLength > 0)
		mnExpectedContentLength = nExpectedContentLength;
	mpFileName = nil;
	mpHeaders = nil;
	if ([pResponse isKindOfClass:[NSHTTPURLResponse class]])
	{
		mpHeaders = [(NSHTTPURLResponse *)pResponse allHeaderFields];
		if (mpHeaders)
			[mpHeaders retain];
	}
	mpMIMEType = [pResponse MIMEType];
	if (mpMIMEType)
		[mpMIMEType retain];
	mpPath = nil;
	mpURL = [pResponse URL];
	if (mpURL)
		[mpURL retain];

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
	if (mpFileName)
		[mpFileName release];
	if (mpPath)
		[mpPath release];

	mpFileName = nil;
	mpPath = pPath;

	if (mpPath)
	{
		[mpPath retain];
		mpFileName = [mpPath lastPathComponent];
		if (mpFileName)
			[mpFileName retain];
	}
}

@end

@interface NeoMobileStatusBarView : NSView
- (void)drawRect:(NSRect)dirtyRect;
@end

@implementation NeoMobileStatusBarView

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

static unsigned int neoMobileBaseURLEntry = 0;
static unsigned int neoMobileBaseURLCount = 0;
static NSArray *neoMobileBaseURLEntries = nil;
static NSString *neoMobileServerType = nil;
static NSTimeInterval lastBaseURLIncrementTime = 0;
static unsigned int baseURLIncrements = 0;
static MacOSBOOL bWebJavaScriptTextInputPanelSwizzeled = NO;
static std::map< NSURLDownload*, NeoMobileDownloadData* > aDownloadDataMap;
static NSMutableDictionary *pRetryDownloadURLs = nil;

@implementation NeoMobileWebView

+ (const NSString *)appendNeoMobileServerNameToString:(const NSString *)pString
{
	const NSString *pRet = ( pString ? pString : @"" );

	[NeoMobileWebView neoMobileURL];
	if (neoMobileServerType && [neoMobileServerType length])
		pRet = [pRet stringByAppendingFormat:@" %@", neoMobileServerType];

	return pRet;
}

		
+ (NSString *)neoMobileURL
{
	if (!neoMobileBaseURLEntries)
	{
		// Determine which server type to use. The default server type can be
		// overridden using the following Terminal command:
		//   defaults write $(PRODUCT_DOMAIN).$(PRODUCT_DIR_NAME) nmServerType development|test
		// To use the default server type, use the following Terminal command:
		//   defaults delete $(PRODUCT_DOMAIN).$(PRODUCT_DIR_NAME) nmServerType
		unsigned int nBaseURLCount = 0;
		const NSString **pBaseURLs = nil;
		NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
		NSString *serverType=[defaults stringForKey:kNeoMobileServerTypePref];
		if ( serverType )
		{
			if ( [serverType caseInsensitiveCompare:@"development"] == NSOrderedSame )
			{
				nBaseURLCount = sizeof( pDevelopmentBaseURLs ) / sizeof( NSString* );
				pBaseURLs = pDevelopmentBaseURLs;
				neoMobileServerType = @"development";
			}
			else if ( [serverType caseInsensitiveCompare:@"test"] == NSOrderedSame )
			{
				nBaseURLCount = sizeof( pTestBaseURLs ) / sizeof( NSString* );
				pBaseURLs = pTestBaseURLs;
				neoMobileServerType = @"test";
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

			neoMobileBaseURLEntries = pNewURLEntries;
			[neoMobileBaseURLEntries retain];
			neoMobileBaseURLCount = [neoMobileBaseURLEntries count];
		}
		else
		{
			neoMobileBaseURLCount = 0;
		}
	}

	if (neoMobileBaseURLEntry >= neoMobileBaseURLCount)
		neoMobileBaseURLEntry = 0;

	return (NSString *)[neoMobileBaseURLEntries objectAtIndex:neoMobileBaseURLEntry];
}

+ (MacOSBOOL)isDownloadURL:(NSURL *)url {
	if (!url || ![url path] || [[url path] length] < [kDownloadURI length])
		return(NO);
	NSRange range = NSMakeRange(0, [kDownloadURI length]);
	return ([[url path] compare:(NSString *)kDownloadURI options:0 range:range]==NSOrderedSame && [NeoMobileWebView isNeoMobileURL:url syncServer:NO]);
}

+ (MacOSBOOL)isLoginURL:(NSURL *)url httpMethod:(NSString *)httpMethod {
	if (!url || ![url path])
		return(NO);
	if (!httpMethod)
		httpMethod = @"GET";
	return ([kNeoMobileLoginURI isEqualToString:[url path]] && [httpMethod caseInsensitiveCompare:@"POST"]!=NSOrderedSame && [NeoMobileWebView isNeoMobileURL:url syncServer:NO]);
}

+ (MacOSBOOL)isNeoMobileURL:(NSURL *)url syncServer:(MacOSBOOL)syncServer
{
	// Make sure that the list of servers has been populated
	[NeoMobileWebView neoMobileURL];
	if(!neoMobileBaseURLEntries)
		return(NO);

	if(!url)
		return(NO);

	NSString *urlHost = [url host];
	if(!urlHost || ![urlHost length])
		return(NO);

	for(unsigned int i = 0; i < neoMobileBaseURLCount; i++)
	{
		NSURL *neoMobileBaseURL = [NSURL URLWithString:(NSString *)[neoMobileBaseURLEntries objectAtIndex:i]];
		if(!neoMobileBaseURL)
			continue;
		NSString *neoMobileBaseHost = [neoMobileBaseURL host];
		if(!neoMobileBaseHost || ![neoMobileBaseHost length])
			continue;
		else if ([neoMobileBaseHost caseInsensitiveCompare:urlHost] == NSOrderedSame)
		{
			if (syncServer)
				neoMobileBaseURLEntry = i;
			return(YES);
		}
	}

	return(NO);
}

+ (MacOSBOOL)incrementNeoMobileBaseEntry
{
	// Make sure that the list of servers has been populated
	[NeoMobileWebView neoMobileURL];
	if(!neoMobileBaseURLEntries)
		return(NO);

	if (neoMobileBaseURLCount < 2)
		return(NO);
	
	if (++neoMobileBaseURLEntry >= neoMobileBaseURLCount)
		neoMobileBaseURLEntry = 0;
	
	NSTimeInterval oldLastBaseURLIncrementTime = lastBaseURLIncrementTime;
	lastBaseURLIncrementTime = [NSDate timeIntervalSinceReferenceDate];
	if (++baseURLIncrements >= neoMobileBaseURLCount)
	{
		baseURLIncrements = 0;
		if (oldLastBaseURLIncrementTime + kBaseURLIncrementInterval > lastBaseURLIncrementTime)
			return(NO);
	}
	
	return(YES);
}

- (id)initWithFrame:(NSRect)aFrame panel:(NeoMobileNonRecursiveResponderWebPanel *)pPanel backButton:(NSButton *)pBackButton cancelButton:(NSButton *)pCancelButton downloadingIndicator:(NSProgressIndicator *)pDownloadingIndicator loadingIndicator:(NSProgressIndicator *)pLoadingIndicator statusLabel:(NSText *)pStatusLabel userAgent:(const NSString *)pUserAgent
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
	
	mpdownloadingIndicator = pDownloadingIndicator;
	[mpdownloadingIndicator retain];
	
	mploadingIndicator = pLoadingIndicator;
	[mploadingIndicator retain];
	
	mpstatusLabel = pStatusLabel;
	[mpstatusLabel retain];

	if ( pUserAgent && [pUserAgent length] )
	{
		mpuserAgent = pUserAgent;
		[mpuserAgent retain];
	}
	else
	{
		mpuserAgent = nil;
	}

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

	// Append custom user agent onto standard user agent
	if ( mpuserAgent )
	{
		NSString *pNewUserAgent = [self userAgentForURL:[NSURL URLWithString:[NeoMobileWebView neoMobileURL]]];
		if ( pNewUserAgent )
		{
			pNewUserAgent = [NSString stringWithFormat:@"%@ %@", pNewUserAgent, mpuserAgent];
			[self setCustomUserAgent:pNewUserAgent];
		}
	}

	mpexportEvent=NULL;

	// Hide downloading indicator by default
	[self setDownloadingIndicatorHidden:YES];

	return self;
}

- (IBAction)backButtonPressed
{
#ifdef DEBUG
	fprintf(stderr, "NeoMobile Back Button Clicked\n");
#endif
	[self stopLoading:self];
	if ([self canGoBack])
		[self goBack];
	else
		[self loadURI:@""];
}

- (IBAction)cancelButtonPressed
{
#ifdef DEBUG
	fprintf(stderr, "NeoMobile Cancel Button Clicked\n");
#endif
	[self stopLoading:self];
}

- (void)loadURI:(NSString *)pURI
{
	if ( !pURI )
		pURI = @"";

	NSURL *pURL = [NSURL URLWithString:pURI relativeToURL:[NSURL URLWithString:[NeoMobileWebView neoMobileURL]]];
	if ( pURL )
	{
		NSMutableURLRequest *pRequest = [NSMutableURLRequest requestWithURL:pURL];
		if ( pRequest )
			[[self mainFrame] loadRequest:pRequest];
	}
}

- (void)setDownloadingIndicatorHidden:(MacOSBOOL)bHidden
{
	if ( bHidden != [mpdownloadingIndicator isHidden] )
	{
		[mpdownloadingIndicator setHidden:bHidden];
		if ( !bHidden )
		{
			[mpdownloadingIndicator startAnimation:self];
			[mploadingIndicator setHidden:YES];
		}

		// Adjust the font size and position of the status label
		NSFont *pFont = [mpstatusLabel font];
		if ( pFont )
		{
			// Shrink the font if the downloading indicator is visible
			NSPoint aPoint = NSMakePoint( [mpstatusLabel frame].origin.x, [mpdownloadingIndicator frame].origin.y );
			float fFontHeight = kNMStatusLabelFontHeight;
			if ( !bHidden )
			{
				float fAdjust = [mpdownloadingIndicator frame].size.height/2;
				aPoint.y += fAdjust;
				fFontHeight -= fAdjust/2;
				if ( fFontHeight < kNMStatusLabelFontHeight/2 )
					fFontHeight = kNMStatusLabelFontHeight/2;
			}
			[mpstatusLabel setFrameOrigin:aPoint];
			[mpstatusLabel setFont:[[NSFontManager sharedFontManager] convertFont:pFont toSize:fFontHeight]];
		}
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
		// Error code NSURLErrorCancelled indicates that the WebKit is doing
		// the Back, Reload, or Forward actions so we don't trigger server
		// fallback. These actions are incompatible with our export event code
		// so cancel the current export event.
		if ( mpexportEvent )
			mpexportEvent->Cancel();
		return;
	}

	if ( !aDownloadDataMap.size() )
	{
		[self setDownloadingIndicatorHidden:YES];
		[mpcancelButton setEnabled:NO];
		[mpstatusLabel setString:@""];
	}

	[mploadingIndicator setHidden:YES];

	if ( pWebFrame )
	{
		MacOSBOOL bShowErrorAfterReload = NO;
		WebDataSource *pDataSource = [pWebFrame provisionalDataSource];
		if ( pDataSource )
		{
			// Don't reload downloads as it will leave a blank screen
			NSMutableURLRequest *pRequest = [pDataSource request];
			if ( !pRequest || [NeoMobileWebView isDownloadURL:[pRequest URL]] )
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
					pURL = [NSURL URLWithString:[NeoMobileWebView neoMobileURL]];
				if ( pURL && [NeoMobileWebView isNeoMobileURL:pURL syncServer:NO] && ![NeoMobileWebView isDownloadURL:pURL] && ( errCode == NSURLErrorTimedOut || errCode == NSURLErrorCannotFindHost || errCode == NSURLErrorCannotConnectToHost ) )
				{
					if ( [NeoMobileWebView incrementNeoMobileBaseEntry] )
					{
						// Reconstruct URL with current NeoOffice Mobile server
						NSMutableString *pURI = [NSMutableString stringWithString:[NeoMobileWebView neoMobileURL]];
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
								[defaults setObject:pURI forKey:[NeoMobileWebView appendNeoMobileServerNameToString:kNeoMobileLastURLPref]];
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

	NSAlert *pAlert = [NSAlert alertWithMessageText:[NSString stringWithFormat:@"%@ %@", NeoMobileGetLocalizedString(NEOMOBILEERROR), [pError localizedDescription]] defaultButton:nil alternateButton:nil otherButton:nil informativeTextWithFormat:@""];
	if ( pAlert )
		[pAlert runModal];
}

- (void)stopLoading:(id)pSender
{
	if(aDownloadDataMap.size())
	{
		// file downloads in progress, so cancel them
		for(std::map< NSURLDownload*, NeoMobileDownloadData* >::const_iterator it = aDownloadDataMap.begin(); it != aDownloadDataMap.end(); ++it)
		{
			[it->first cancel];
			[it->second release];
		}
		aDownloadDataMap.clear();

		if (pRetryDownloadURLs)
			[pRetryDownloadURLs removeAllObjects];

		[mpstatusLabel setString:NeoMobileGetLocalizedString(NEOMOBILEDOWNLOADCANCELED)];
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
		[mpstatusLabel setString:@""];
	}

	[mpcancelButton setEnabled:NO];
	[self setDownloadingIndicatorHidden:YES];
	[mploadingIndicator setHidden:YES];

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
	if ( !aDownloadDataMap.size() )
	{
		[self setDownloadingIndicatorHidden:YES];
		[mpcancelButton setEnabled:NO];
		[mpstatusLabel setString:@""];
	}

	[mploadingIndicator setHidden:YES];

	if ( !pWebView || !pWebFrame )
		return;

	WebDataSource *pDataSource = [pWebFrame dataSource];
	if ( !pDataSource )
		return;

	NSHTTPURLResponse *pResponse = (NSHTTPURLResponse *)[pDataSource response];
	if ( !pResponse || ![(NSURLResponse *)pResponse isKindOfClass:[NSHTTPURLResponse class]] )
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

		NSURL *pSaveURL = [NSURL URLWithString:pSaveURIHeader relativeToURL:[NSURL URLWithString:[NeoMobileWebView neoMobileURL]]];
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

					NeoMobileExportFileAppEvent aEvent( NeoMobileNSStringToOUString( pSaveUUIDHeader ), pFileManager, pPostBody, pMimeTypes );
					mpexportEvent=&aEvent;

					[mploadingIndicator setHidden:NO];
					[mploadingIndicator startAnimation:self];
					[mpcancelButton setEnabled:YES];
					[mpstatusLabel setString:NeoMobileGetLocalizedString(NEOMOBILEEXPORTINGFILE)];

					aEvent.Execute();

					if ( !aDownloadDataMap.size() )
					{
						[self setDownloadingIndicatorHidden:YES];
						[mpcancelButton setEnabled:NO];
						[mpstatusLabel setString:@""];
					}

					[mploadingIndicator setHidden:YES];

					if(aEvent.IsUnsupportedComponentType())
					{
						// display unsupported doc type error to user and
						// pass UUID back to server.
						[self loadURI:GetUploadErrorURI(4, pSaveUUIDHeader)];
					}
					else if(aEvent.IsCanceled())
					{
						// display canceled error to user and pass URI back to
						// server
						
						[self loadURI:GetUploadErrorURI(3, pSaveUUIDHeader)];
					}
					else if(aEvent.GetErrorCode()!=0)
					{
						// other error code;  pass along to the server for
						// display
						
						[self loadURI:GetUploadErrorURI(aEvent.GetErrorCode(), pSaveUUIDHeader)];
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
			[pHistory setCapacity:nCapacity];
	}
	else if ( !mpexportEvent )
	{
		// If we loaded the login URL, clear the history, otherwise save the URL
		if ( [NeoMobileWebView isLoginURL:pURL httpMethod:@"GET"] )
		{
			int nCapacity = 0;
			WebBackForwardList *pHistory = [pWebView backForwardList];
			if ( pHistory )
			{
				nCapacity = [pHistory capacity];
				if ( nCapacity )
				{
					[pHistory setCapacity:0];
					[pHistory setCapacity:nCapacity];
				}
			}
		}
		else if ( ![NeoMobileWebView isDownloadURL:pURL] )
		{
			// If not an upload, save last URL preference
			NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
			[defaults setObject:[pURL absoluteString] forKey:[NeoMobileWebView appendNeoMobileServerNameToString:kNeoMobileLastURLPref]];
			[defaults synchronize];
		}
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
	
	if ( mpexportEvent && pRequest && [[pRequest HTTPMethod] isEqualToString:@"POST"] )
		[mpstatusLabel setString:NeoMobileGetLocalizedString(NEOMOBILEUPLOADINGFILE)];
	else
		[mpstatusLabel setString:NeoMobileGetLocalizedString(NEOMOBILELOADING)];
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

#ifdef USE_NATIVE_LOGIN_WINDOW
	if ( pRequest && [NeoMobileWebView isLoginURL:[pRequest URL] httpMethod:[pRequest HTTPMethod]] )
	{
		[mpPanel showFlipsidePanel];
		return nil;
	}
#endif	// USE_NATIVE_LOGIN_WINDOW

	// Always add a special header with the name and version of the application
	// that this web view is running in
	// TODO: set header value to applications's name and version
	if ( pRequest && [pRequest isKindOfClass:[NSMutableURLRequest class]] )
		[(NSMutableURLRequest *)pRequest addValue:( mpuserAgent ? mpuserAgent : @"Neomobile-Application-Version" ) forHTTPHeaderField:@"Neomobile-Application-Version"];

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

	NSAlert *pAlert = [NSAlert alertWithMessageText:pMessage defaultButton:nil alternateButton:NeoMobileGetLocalizedString(NEOMOBILECANCEL) otherButton:nil informativeTextWithFormat:@""];
	if ( pAlert && [pAlert runModal] == NSAlertDefaultReturn )
		bRet = YES;

	return bRet;
}

- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename
{
#ifdef DEBUG
	fprintf( stderr, "NeoMobile Download downloadRequestReceived: %s\n", [[[[download request] URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
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

	NSMutableArray *downloadPaths = [NSMutableArray arrayWithCapacity:10];
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSString *basePath = nil;
	if (downloadPaths && fileManager)
	{
		// Use NSDownloadsDirectory
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDownloadsDirectory, NSUserDomainMask, YES);
		if (paths)
			[downloadPaths addObjectsFromArray:paths];

		// Use NSDesktopDirectory
		paths = NSSearchPathForDirectoriesInDomains(NSDesktopDirectory, NSUserDomainMask, YES);
		if (paths)
			[downloadPaths addObjectsFromArray:paths];
		
		// Use TMPDIR environment variable
		const char *env = getenv("TMPDIR");
		if (env)
			[downloadPaths addObject:[NSString stringWithUTF8String:env]];

 		unsigned int dirCount = [downloadPaths count];
 		unsigned int i = 0;
		for (; i < dirCount && !basePath; i++)
		{
			MacOSBOOL isDir = NO;
			NSString *downloadPath = (NSString *)[downloadPaths objectAtIndex:i];
			if ([fileManager fileExistsAtPath:downloadPath isDirectory:&isDir] && isDir && [fileManager isWritableFileAtPath:downloadPath])
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
	fprintf( stderr, "NeoMobile Download didCreateDestination: %s\n", [[[[download request] URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif

	std::map< NSURLDownload*, NeoMobileDownloadData* >::const_iterator it = aDownloadDataMap.find(download);
	if(it!=aDownloadDataMap.end())
		[it->second setPath:path];
}

- (void)download: (NSURLDownload *) download didReceiveResponse: (NSURLResponse *) response
{
#ifdef DEBUG
	fprintf( stderr, "NeoMobile Download didReceiveResponse\n");
#endif

	std::map< NSURLDownload*, NeoMobileDownloadData* >::iterator it = aDownloadDataMap.find(download);
	if(it!=aDownloadDataMap.end())
	{
		aDownloadDataMap.erase(it);
		[it->second release];
	}

	NeoMobileDownloadData *pDownloadData=[[NeoMobileDownloadData alloc] initWithDownload:download response:response];
	if(pDownloadData)
		aDownloadDataMap[download]=pDownloadData;
}

- (void)downloadDidBegin: (NSURLDownload *)download
{
#ifdef DEBUG
	fprintf( stderr, "NeoMobile Download File Did Begin: %s\n", [[[[download request] URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif

	std::map< NSURLDownload*, NeoMobileDownloadData* >::iterator it = aDownloadDataMap.find(download);
	if(it!=aDownloadDataMap.end())
	{
		[mploadingIndicator setHidden:NO];
		[mploadingIndicator startAnimation:self];
		[mpcancelButton setEnabled:YES];
	}
}

- (void)download:(NSURLDownload *)download didReceiveDataOfLength:(unsigned long)length
{
#ifdef DEBUG
	fprintf( stderr, "NeoMobile Download didReceiveDataOfLength\n");
#endif

	std::map< NSURLDownload*, NeoMobileDownloadData* >::const_iterator it = aDownloadDataMap.find(download);
	if(it!=aDownloadDataMap.end())
	{
		// Reenabled the cancel button as it may have been cancelled by 
		// clicking on a link
		[mpcancelButton setEnabled:YES];

		[it->second addBytesReceived:length];

		NSString *pDownloadLabel=[it->second fileName];
		if(!pDownloadLabel && ![pDownloadLabel length])
			pDownloadLabel=NeoMobileGetLocalizedString(NEOMOBILEDOWNLOADINGFILE);

		unsigned long long nBytesReceived=[it->second bytesReceived];
		unsigned long long nExpectedContentLength=[it->second expectedContentLength];
		if(nExpectedContentLength > 0)
		{
			// we got a response from the server, so we can compute a percentage
			[mpdownloadingIndicator setDoubleValue:(double)nBytesReceived/(double)nExpectedContentLength*100];
			if([mpdownloadingIndicator isIndeterminate])
				[self setDownloadingIndicatorHidden:YES];
			[mpdownloadingIndicator setIndeterminate:NO];
		}
		else if(![mpdownloadingIndicator isIndeterminate])
		{
			MacOSBOOL bIndeterminate = YES;
			for(std::map< NSURLDownload*, NeoMobileDownloadData* >::const_iterator dit = aDownloadDataMap.begin(); dit != aDownloadDataMap.end(); ++dit)
			{
				if([dit->second expectedContentLength] > 0)
				{
					bIndeterminate = NO;
					break;
				}
			}

			if(bIndeterminate != [mpdownloadingIndicator isIndeterminate])
				[self setDownloadingIndicatorHidden:YES];
			[mpdownloadingIndicator setIndeterminate:bIndeterminate];
		}

		[self setDownloadingIndicatorHidden:NO];

		// add MB downloaded
		float fMBReceived = (float)nBytesReceived/(float)(1024*1024);
		long nMBReceived = (long)fMBReceived;
		long n10thMBReceived = (long)((fMBReceived-nMBReceived)*10);
		[mpstatusLabel setString:[NSString stringWithFormat:@"%@ - %ld%@%ld %@", pDownloadLabel, nMBReceived, NeoMobileGetLocalizedDecimalSeparator(), n10thMBReceived, NeoMobileGetLocalizedString(NEOMOBILEMEGABYTE)]];
	}
}

- (void)downloadDidFinish: (NSURLDownload*)download
{
#ifdef DEBUG
	fprintf( stderr, "NeoMobile Download File Did End: %s\n", [[[[download request] URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
#endif
	std::map< NSURLDownload*, NeoMobileDownloadData* >::iterator it = aDownloadDataMap.find(download);
	if(it!=aDownloadDataMap.end())
	{
		NSString *path = [it->second path];
		if (path)
		{
			id pSecurityScopedURL = nil;
			if ( !pApplication_acquireSecurityScopedURLFromNSURL )
				pApplication_acquireSecurityScopedURLFromNSURL = (Application_acquireSecurityScopedURLFromNSURL_Type *)dlsym( RTLD_DEFAULT, "Application_acquireSecurityScopedURLFromNSURL" );
			if ( !pApplication_releaseSecurityScopedURL )
				pApplication_releaseSecurityScopedURL = (Application_releaseSecurityScopedURL_Type *)dlsym( RTLD_DEFAULT, "Application_releaseSecurityScopedURL" );
			if ( pApplication_acquireSecurityScopedURLFromNSURL && pApplication_releaseSecurityScopedURL )
				pSecurityScopedURL = pApplication_acquireSecurityScopedURLFromNSURL( [NSURL fileURLWithPath:path], sal_True, NULL );

			// Check if downloaded file size matches content length header
			unsigned long long nExpectedContentLength=[it->second expectedContentLength];
			NSFileManager *pFileManager = [NSFileManager defaultManager];
			if(nExpectedContentLength > 0 && pFileManager && GetFileSize(path) != nExpectedContentLength)
			{
				if ( pSecurityScopedURL && pApplication_releaseSecurityScopedURL )
					pApplication_releaseSecurityScopedURL( pSecurityScopedURL );

				NSError *pError = [NSError errorWithDomain:@"NSURLErrorDomain" code:NSURLErrorNetworkConnectionLost userInfo:nil];
				[self download:download didFailWithError:pError];
				return;
			}

			if (pRetryDownloadURLs)
				[pRetryDownloadURLs removeObjectForKey:path];

			NSString *pErrorMessage = nil;
			NSString *MIMEType = [it->second MIMEType];
			if(MIMEType && [MIMEType rangeOfString: @"application/vnd.oasis.opendocument"].location != NSNotFound || [MIMEType rangeOfString: @"application/ms"].location != NSNotFound)
			{
				@try
				{
					NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
					NSArray *pURLs = [NSArray arrayWithObject:[NSURL fileURLWithPath:path]];
					NSString *pBundleID = [[NSBundle mainBundle] bundleIdentifier];
					if (pWorkspace && pURLs && pBundleID)
						[pWorkspace openURLs:pURLs withAppBundleIdentifier:pBundleID options:NSWorkspaceLaunchDefault additionalEventParamDescriptor:nil launchIdentifiers:nil];
				}
				@catch (NSException *pExc)
				{
					if (pExc)
						pErrorMessage = [pExc reason];
					else
						pErrorMessage = @"";
				}
			}
			else
			{
				@try
				{
					NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
					NSURL *pURL = [NSURL fileURLWithPath:path];
					if (pWorkspace && pURL)
						[pWorkspace openURL:pURL];
				}
				@catch (NSException *pExc)
				{
					if (pExc)
						pErrorMessage = [pExc reason];
					else
						pErrorMessage = @"";
				}
			}
#ifdef DEBUG
			fprintf( stderr, "Opening file: %s\n", [path cStringUsingEncoding:NSUTF8StringEncoding] );
#endif

			if (pErrorMessage)
			{
				NSAlert *pAlert = [NSAlert alertWithMessageText:[NSString stringWithFormat:@"%@ %@", NeoMobileGetLocalizedString(NEOMOBILEERROR), pErrorMessage] defaultButton:nil alternateButton:nil otherButton:nil informativeTextWithFormat:@""];
				if (pAlert)
					[pAlert runModal];
			}

			if ( pSecurityScopedURL && pApplication_releaseSecurityScopedURL )
				pApplication_releaseSecurityScopedURL( pSecurityScopedURL );
		}

		[it->second release];
		aDownloadDataMap.erase(it);
	}

	if(!aDownloadDataMap.size())
	{
		[self setDownloadingIndicatorHidden:YES];
		[mpcancelButton setEnabled:NO];
		[mpstatusLabel setString:@""];
	}

	[mploadingIndicator setHidden:YES];
}

- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error
{
#ifdef DEBUG
	NSLog( @"NeoMobile Download didFailWithError: %@", error );
#endif

	std::map< NSURLDownload*, NeoMobileDownloadData* >::iterator it = aDownloadDataMap.find(download);
	if(it!=aDownloadDataMap.end())
	{
		if (!pRetryDownloadURLs)
		{
			pRetryDownloadURLs = [NSMutableDictionary dictionaryWithCapacity:1];
			if (pRetryDownloadURLs)
				[pRetryDownloadURLs retain];
		}

		MacOSBOOL bRetry = NO;
		NSString *pPath = [it->second path];
		if (pPath && pRetryDownloadURLs)
		{
			// Delete the file in case automatic deletion fails
			NSFileManager *pFileManager = [NSFileManager defaultManager];
			if (pFileManager)
				[pFileManager removeItemAtPath:pPath error:nil];

			NSObject *pValue = [pRetryDownloadURLs objectForKey:pPath];
			if (!pValue || ![pValue isKindOfClass:[NSNull class]])
			{
				[[self mainFrame] loadRequest:[download request]];
				[pRetryDownloadURLs setObject:[NSNull null] forKey:pPath];
				bRetry = YES;
			}

			if (!bRetry)
				[pRetryDownloadURLs removeObjectForKey:pPath];
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
		[self setDownloadingIndicatorHidden:YES];
		[mpcancelButton setEnabled:NO];
		[mpstatusLabel setString:NeoMobileGetLocalizedString(NEOMOBILEDOWNLOADFAILED)];
	}

	[mploadingIndicator setHidden:YES];
}

- (void)webView:(WebView *)sender decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id)listener
{
#ifdef DEBUG
	fprintf( stderr, "NeoMobile Loading URL: %s\n", [[[request URL] absoluteString] cStringUsingEncoding:NSUTF8StringEncoding] );
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

	if ( mpdownloadingIndicator )
		[mpdownloadingIndicator release];
	
	if ( mploadingIndicator )
		[mploadingIndicator release];
	
	if ( mpstatusLabel )
		[mpstatusLabel release];
	
	[super dealloc];
}

@end

static NeoMobileNonRecursiveResponderPanel *pCurrentPanel = nil;

@implementation NeoMobileNonRecursiveResponderPanel

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
	[self setMinSize: NSMakeSize(kNMDefaultBrowserWidth, 0)];
	[self setTitle: NeoMobileGetLocalizedString(NEOMOBILEPRODUCTNAME)];
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
		if(aFrame.size.height > kNMMaxInZoomHeight && aFrame.size.height > aZoomFrame.size.height)
		{
			NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
			[defaults setObject:[NSString stringWithFormat:@"%d", (int)aFrame.origin.x] forKey:kNeoMobileXPosPref];
			[defaults setObject:[NSString stringWithFormat:@"%d", (int)aFrame.origin.y] forKey:kNeoMobileYPosPref];
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
		if(aFrame.size.height > kNMMaxInZoomHeight && aFrame.size.height > aZoomFrame.size.height)
		{
			NSView *pContentView = [self contentView];
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
}

- (void)windowWillClose:(NSNotification *)notification
{
	if([notification object]==self)
	{
		if (!mbinZoom)
		{
			NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
			[defaults setBool:NO forKey:kNeoMobileVisiblePref];
			[defaults synchronize];
		}

		if ([self isKindOfClass:[NeoMobileNonRecursiveResponderWebPanel class]])
		{
			NeoMobileWebView *pWebView = [(NeoMobileNonRecursiveResponderWebPanel *)self webView];
			if (pWebView)
				[pWebView stopLoading:self];
		}
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
				// Use the top view instead of the bottom view if this is a
				// flipside panel
				MacOSBOOL bUseTopView = ( [self isKindOfClass:[NeoMobileNonRecursiveResponderFlipsidePanel class]] );

				unsigned int nCount = [pSubviews count];
				unsigned int i = 0;
				for ( ; i < nCount; i++ )
				{
					NSView *pView = (NSView *)[pSubviews objectAtIndex:i];
					if ( pView )
					{
						if ( !pBottomView )
							pBottomView = pView;
						else if ( bUseTopView && [pView frame].origin.y + [pView frame].size.height > [pBottomView frame].origin.y + [pBottomView frame].size.height )
							pBottomView = pView;
						else if ( !bUseTopView && [pView frame].origin.y < [pBottomView frame].origin.y )
							pBottomView = pView;
					}
				}
			}

			NSRect aContentFrame = [pContentView frame];
			aContentFrame.origin.x += aRet.origin.x;
			aContentFrame.origin.y += aRet.origin.y + aContentFrame.size.height;
			if ( pBottomView )
			{
				NSRect aBottomFrame = [pBottomView frame];
				aContentFrame.origin.y -= aBottomFrame.size.height;
				aContentFrame.size.height = aBottomFrame.size.height;
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
	if(aFrame.size.height > kNMMaxInZoomHeight && aFrame.size.height > aZoomFrame.size.height)
		bZoomed = NO;

	[super zoom:aObject];

	// On Mac OS X 10.6 and higher the window can get stuck in a zoomed state
	// after the zoomed window has been moved. In such cases we manually resize
	// to the last saved unzoomed size.
	if (bZoomed)
	{
		aFrame = [self frame];
		if(aFrame.size.height > kNMMaxInZoomHeight && aFrame.size.height > aZoomFrame.size.height)
			bZoomed = NO;
	}

	if (bZoomed)
	{
		NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
		NSString *xPosStr=[defaults stringForKey:kNeoMobileXPosPref];
		NSString *yPosStr=[defaults stringForKey:kNeoMobileYPosPref];
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
			NSString *widthStr=[defaults stringForKey:kNeoMobileWidthPref];
			NSString *heightStr=[defaults stringForKey:kNeoMobileHeightPref];
 			if(widthStr)
				contentSize.width=[widthStr intValue];
			else
				contentSize.width=kNMDefaultBrowserWidth;
 			if(heightStr)
				contentSize.height=[heightStr intValue];
			else
				contentSize.height=kNMDefaultBrowserHeight;
			[self setContentSize:contentSize];
		}
	}

	mbinZoom = NO;
}

@end

@interface NSWindow (NeoMobileNonRecursiveResponderWebPanel)
- (NSRect)_growBoxRect;
@end

static NeoMobileNonRecursiveResponderFlipsidePanel *sharedFlipsidePanel = nil;

@implementation NeoMobileNonRecursiveResponderWebPanel

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

	mpwebView = [[NeoMobileWebView alloc] initWithFrame:NSMakeRect(0, [mpbottomView bounds].size.height, [mpcontentView bounds].size.width, [mpcontentView bounds].size.height-[mpbottomView bounds].size.height) panel:self backButton:mpbackButton cancelButton:mpcancelButton downloadingIndicator:mpdownloadingIndicator loadingIndicator:mploadingIndicator statusLabel:mpstatusLabel userAgent:mpuserAgent];
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

	if ( mpdownloadingIndicator )
		[mpdownloadingIndicator release];
	
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

- (void)dismissFlipsidePanel
{
#ifdef USE_NATIVE_LOGIN_WINDOW
	if ( sharedFlipsidePanel && [sharedFlipsidePanel isVisible] )
	{
		// Clear web view content and history list
		NSString *javaScriptCode = @"document.body.innerHTML = ''";
		[mpwebView stringByEvaluatingJavaScriptFromString:javaScriptCode];

		WebBackForwardList *pHistory = [mpwebView backForwardList];
		if ( pHistory )
		{
			int nCapacity = [pHistory capacity];
			if ( nCapacity )
			{
				[pHistory setCapacity:0];
				[pHistory setCapacity:nCapacity];
			}
		}

		[sharedFlipsidePanel neoMobileFlipToShowWindow:self forward:NO];
	}
#endif	// USE_NATIVE_LOGIN_WINDOW
}

- (id)initWithUserAgent:(NSString *)pUserAgent
{
	[super initWithContentRect:NSMakeRect(0, 0, kNMDefaultBrowserWidth, kNMDefaultBrowserHeight) styleMask:NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSUtilityWindowMask backing:NSBackingStoreBuffered defer:YES];
	
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

	mpcancelButton = [[NSButton alloc] initWithFrame:NSMakeRect(contentSize.width-buttonSize.width-MAX(kNMBottomViewPadding, growBoxSize.width), kNMBottomViewPadding, buttonSize.width, buttonSize.height)];
	[mpcancelButton setToolTip:NeoMobileGetLocalizedString(NEOMOBILECANCEL)];
	[mpcancelButton setEnabled:YES];
	[mpcancelButton setButtonType:NSMomentaryPushInButton];
	[mpcancelButton setBezelStyle:NSRegularSquareBezelStyle];
	[mpcancelButton setImage:cancelImage];
	[mpcancelButton setImagePosition:NSImageOnly];
	[[mpcancelButton cell] setImageScaling:NSImageScaleNone];
	[mpcancelButton setAutoresizingMask:(NSViewMinXMargin)];
	
	mpbackButton = [[NSButton alloc] initWithFrame:NSMakeRect([mpcancelButton frame].origin.x-buttonSize.width-kNMBottomViewPadding, kNMBottomViewPadding, buttonSize.width, buttonSize.height)];
	[mpbackButton setToolTip:NeoMobileGetLocalizedString(NEOMOBILEBACK)];
	[mpbackButton setEnabled:YES];
	[mpbackButton setButtonType:NSMomentaryPushInButton];
	[mpbackButton setBezelStyle:NSRegularSquareBezelStyle];
	[mpbackButton setImage:backImage];
	[mpbackButton setImagePosition:NSImageOnly];
	[[mpbackButton cell] setImageScaling:NSImageScaleNone];
	[mpbackButton setAutoresizingMask:(NSViewMinXMargin)];
	
	mploadingIndicator = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect([mpbackButton frame].origin.x-buttonSize.width-kNMBottomViewPadding, kNMBottomViewPadding, buttonSize.width, buttonSize.height)];
	[mploadingIndicator setStyle:NSProgressIndicatorSpinningStyle];
	[mploadingIndicator setHidden:YES];
	[mploadingIndicator setAutoresizingMask:(NSViewMinXMargin)];
	
	float maxButtonHeight=MAX([mploadingIndicator frame].origin.y+[mploadingIndicator frame].size.height-kNMBottomViewPadding, [mpbackButton frame].origin.y+[mpbackButton frame].size.height-kNMBottomViewPadding);
	mpstatusLabel=[[NSText alloc] initWithFrame:NSMakeRect(kNMBottomViewPadding, kNMBottomViewPadding, [mploadingIndicator frame].origin.x-(kNMBottomViewPadding*2), maxButtonHeight)];
	[mpstatusLabel setEditable:NO];
	[mpstatusLabel setString:@""];
	[mpstatusLabel setAutoresizingMask:(NSViewWidthSizable)];
	[mpstatusLabel setDrawsBackground:NO];

	[mpstatusLabel setFont:[[NSFontManager sharedFontManager] convertFont:[mpstatusLabel font] toSize:kNMStatusLabelFontHeight]];
	[self centerTextInTextView:mpstatusLabel];

	// Have downloading indicator overlaying most of status label
	mpdownloadingIndicator = [[NSProgressIndicator alloc] initWithFrame:[mpstatusLabel frame]];
	[mpdownloadingIndicator setStyle:NSProgressIndicatorBarStyle];
	[mpdownloadingIndicator setControlSize:NSSmallControlSize];
	[mpdownloadingIndicator setIndeterminate:NO];
	[mpdownloadingIndicator setMinValue:0.0];
	[mpdownloadingIndicator setMaxValue:100.0];
	[mpdownloadingIndicator setDoubleValue:0.0];
	[mpdownloadingIndicator sizeToFit];
	[mpdownloadingIndicator setAutoresizingMask:(NSViewWidthSizable)];

	mpbottomView=[[NeoMobileStatusBarView alloc] initWithFrame:NSMakeRect(0, 0, contentSize.width, maxButtonHeight+(kNMBottomViewPadding*2))];
	[mpbottomView setAutoresizesSubviews:YES];
	[mpbottomView setAutoresizingMask:(NSViewWidthSizable)];
	
	[mpbottomView addSubview:mpstatusLabel];
	[mpbottomView addSubview:mploadingIndicator];
	[mpbottomView addSubview:mpbackButton];
	[mpbottomView addSubview:mpcancelButton];
	[mpbottomView addSubview:mpdownloadingIndicator];
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

- (void)showFlipsidePanel
{
	if ( !sharedFlipsidePanel )
		sharedFlipsidePanel = [[NeoMobileNonRecursiveResponderFlipsidePanel alloc] initWithWebPanel:self];

	if ( sharedFlipsidePanel )
	{
		[self neoMobileFlipToShowWindow:sharedFlipsidePanel forward:YES];

		// Clear web view content and history list
		NSString *javaScriptCode = @"document.body.innerHTML = ''";
		[mpwebView stringByEvaluatingJavaScriptFromString:javaScriptCode];

		WebBackForwardList *pHistory = [mpwebView backForwardList];
		if ( pHistory )
		{
			int nCapacity = [pHistory capacity];
			if ( nCapacity )
			{
				[pHistory setCapacity:0];
				[pHistory setCapacity:nCapacity];
			}
		}
	}
}

- (NeoMobileWebView *)webView
{
	return mpwebView;
}

@end
