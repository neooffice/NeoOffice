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

#import "neomobile.hxx"
#import "neomobilewebview.h"
#import <unistd.h>

using namespace rtl;

//========================================================================

#ifdef TEST
SAL_DLLPRIVATE const NSString *kNeoMobileAboutURL = @"http://www-test.neooffice.org/neomobile/";
#else	// TEST
SAL_DLLPRIVATE const NSString *kNeoMobileAboutURL = @"http://www.neooffice.org/neomobile/";
#endif	// TEST
SAL_DLLPRIVATE const NSString *kNeoMobileLoginURI = @"/users/login";
SAL_DLLPRIVATE const NSString *kNeoMobileLastURLPref = @"nmLastURL";
SAL_DLLPRIVATE const NSString *kNeoMobileXPosPref = @"nmXPos";
SAL_DLLPRIVATE const NSString *kNeoMobileYPosPref = @"nmYPos";
SAL_DLLPRIVATE const NSString *kNeoMobileWidthPref = @"nmWidth";
SAL_DLLPRIVATE const NSString *kNeoMobileHeightPref = @"nmHeight";
SAL_DLLPRIVATE const NSString *kNeoMobileVisiblePref = @"nmVisible";
SAL_DLLPRIVATE const NSString *kNeoMobileServerTypePref = @"nmServerType";

static NeoMobileNonRecursiveResponderWebPanel *pSharedPanel = nil;

@implementation NeoMobileCreateWebViewImpl

+ (id)createWithURI:(const NSString *)pURI userAgent:(const NSString *)pUserAgent
{
	NeoMobileCreateWebViewImpl *pRet = [[NeoMobileCreateWebViewImpl alloc] initWithURI:pURI userAgent:pUserAgent];
	[pRet autorelease];
	return pRet;
}

- (id)initWithURI:(const NSString *)pURI userAgent:(const NSString *)pUserAgent
{
	self = [super init];

	mpURI = pURI;
	mpUserAgent = pUserAgent;

	return(self);
}

- (void)showWebView:(id)obj
{
	if ( !pSharedPanel )
		pSharedPanel = [[NeoMobileNonRecursiveResponderWebPanel alloc] initWithUserAgent:mpUserAgent];

	if(pSharedPanel)
	{
		NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
		if(![pSharedPanel isVisible])
		{
			// Check for retained user position. If not available, make
			// relative to the primary frame.
			NSPoint windowPos={75, 75};
			NSString *xPosStr=[defaults stringForKey:kNeoMobileXPosPref];
			NSString *yPosStr=[defaults stringForKey:kNeoMobileYPosPref];
			if(xPosStr && yPosStr)
			{
				windowPos.x=[xPosStr intValue];
				windowPos.y=[yPosStr intValue];
			}
			else
			{
				NSWindow *mainWindow=[NSApp mainWindow];
				if(mainWindow)
				{
					NSPoint mainWindowPos=[mainWindow frame].origin;
					windowPos.x+=mainWindowPos.x;
					windowPos.y+=mainWindowPos.y;
				}
			}
			[pSharedPanel setFrameOrigin:windowPos];

			NSString *widthStr=[defaults stringForKey:kNeoMobileWidthPref];
			NSString *heightStr=[defaults stringForKey:kNeoMobileHeightPref];
			if(widthStr && heightStr)
			{
				NSSize contentSize={0, 0};
				contentSize.width=[widthStr intValue];
				contentSize.height=[heightStr intValue];
				[pSharedPanel setContentSize:contentSize];
			}

			// Make sure window is visible
			[pSharedPanel orderFront:self];

			[defaults setBool:YES forKey:kNeoMobileVisiblePref];
			[defaults synchronize];
		}

		NeoMobileWebView *pWebView = [pSharedPanel webView];
		if(pWebView)
		{
			// Use kNeoMobileLastURLPref preference if it is a valid
			// application URL otherwise use the default application URL
			NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
			if(defaults)
			{
				NSString *loadURLPref = [defaults stringForKey:[NeoMobileWebView appendNeoMobileServerNameToString:kNeoMobileLastURLPref]];
				if(loadURLPref && [loadURLPref length])
				{
					NSURL *lastLoadURL = [NSURL URLWithString:loadURLPref];
					if (lastLoadURL && [NeoMobileWebView isNeoMobileURL:lastLoadURL syncServer:YES] && ![NeoMobileWebView isLoginURL:lastLoadURL httpMethod:@"GET"] && ![NeoMobileWebView isDownloadURL:lastLoadURL])
					{
						NSMutableURLRequest *loadRequest =(NSMutableURLRequest *)[NSMutableURLRequest requestWithURL:lastLoadURL];
						if(loadRequest)
						{
							[[pWebView mainFrame] loadRequest:loadRequest];
							return;
						}
		 			}
				}
			}

			// Load fallback URI
			[pWebView loadURI:mpURI]; 
		}
	}
}

- (void)showWebViewOnlyIfVisible:(id)obj
{
	NSUserDefaults *defaults=[NSUserDefaults standardUserDefaults];
	if ( [defaults boolForKey:kNeoMobileVisiblePref] )
		[self showWebView:obj];
}
@end

OUString NeoMobileNSStringToOUString( NSString *pString )
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

/**
 * Zip the contents of a directory into new selfcontained zip file.
 *
 * @param dirPath	absolute path to the directory whose contents should be
 *					compressed.  Trailing path separator is optional.
 * @param zipFilePath	absolute path to the output ZIP file.  This should
 *						include the ".zip" suffix.  If not present, the
 *						suffix will be added.
 * @return sal_True if the zip operation succeeded, sal_False on error.
 */
::sal_Bool NeoMobileZipDirectory( const rtl::OUString& dirPath, const rtl::OUString& zipFilePath ) 
{
	try
	{
		OString asciiDirPath = OUStringToOString(dirPath,RTL_TEXTENCODING_UTF8);
		if (!asciiDirPath.getLength())
			return(sal_False);
		
		OString asciiZipFilePath = OUStringToOString(zipFilePath,RTL_TEXTENCODING_UTF8);
		if (!asciiZipFilePath.getLength())
			return(sal_False);
			
		char oldWD[2048];
		
		getcwd(oldWD, sizeof(oldWD));
		chdir(asciiDirPath.getStr());
		OString zipCmd("/usr/bin/zip -q \"");
		zipCmd+=asciiZipFilePath;
		zipCmd+=OString("\" *");
		short outVal=system(zipCmd.getStr());
		chdir(oldWD);
		return((outVal==0));
	}
	catch (...)
	{
	}
	
	return(sal_False);
}
