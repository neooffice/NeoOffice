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

@interface NeoMobileWebFrameLoadDelegate : NSObject
- (void)webView:(WebView *)self didCommitLoadForFrame:(WebFrame *)pWebFrame;
@end

@implementation NeoMobileWebFrameLoadDelegate

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

@end

@implementation NeoMobileWebView

- (void)dealloc
{
	if ( mpPanel )
		[mpPanel release];

	[super dealloc];
}

- (id)initWithFrame:(NSRect)aFrame frameName:(NSString *)pFrameName groupName:(NSString *)pGroupName
{
	[super initWithFrame:aFrame frameName:pFrameName groupName:pGroupName];

	mpPanel = [[NSPanel alloc] initWithContentRect:NSMakeRect(0, 0, 700, 500) styleMask:NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSUtilityWindowMask backing:NSBackingStoreBuffered defer:YES];
	if ( mpPanel )
	{
		[mpPanel setFloatingPanel:YES];
		[mpPanel setContentView:self];
		NeoMobileWebFrameLoadDelegate *pDelegate = [[NeoMobileWebFrameLoadDelegate alloc] init];
		if ( pDelegate )
		{
			WebPreferences *pPrefs = [self preferences];
			if ( pPrefs )
			{
				[pPrefs setJavaScriptEnabled:YES];
				[self setFrameLoadDelegate:pDelegate];
				[[self mainFrame] loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"https://neomobile-test.neooffice.org/neofolders/"]]];
			}
		}
	}

	return self;
}

@end
