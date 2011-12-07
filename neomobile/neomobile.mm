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
#include <unistd.h>

using namespace rtl;

//========================================================================

#ifdef TEST
const NSString *kAboutURL = @"http://www-test.neooffice.org/neomobile/";
#else	// TEST
const NSString *kAboutURL = @"http://www.neooffice.org/neomobile/";
#endif	// TEST
const NSString *kNeoMobileLastURLPref = @"nmLastURL";
const NSString *kNeoMobileXPosPref = @"nmXPos";
const NSString *kNeoMobileYPosPref = @"nmYPos";
const NSString *kNeoMobileWidthPref = @"nmWidth";
const NSString *kNeoMobileHeightPref = @"nmHeight";
const NSString *kNeoMobileVisiblePref = @"nmVisible";
const NSString *kNeoMobileServerTypePref = @"nmServerType";
const NSString *kOpenURI = @"/";

static NonRecursiveResponderWebPanel *pSharedPanel = nil;

@implementation CreateWebViewImpl

+ (id)createWithURI:(const NSString *)pURI userAgent:(const NSString *)pUserAgent
{
	CreateWebViewImpl *pRet = [[CreateWebViewImpl alloc] initWithURI:pURI userAgent:pUserAgent];
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
		pSharedPanel = [[NonRecursiveResponderWebPanel alloc] initWithUserAgent:mpUserAgent];

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
				NSString *loadURLPref = [defaults stringForKey:kNeoMobileLastURLPref];
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

OUString NSStringToOUString( NSString *pString )
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
 * Check if the we have full WebView support available
 */
::sal_Bool IsSupportedMacOSXVersion() 
{
	// we currently need to be running on 10.4 in order to have full WebView
	// support.  Check using our gestalt.
	
	long res=0;
	if(Gestalt(gestaltSystemVersion, &res)==noErr)
	{
		bool isTigerOrHigher = ( ( ( ( res >> 8 ) & 0x00FF ) == 0x10 ) && ( ( ( res >> 4 ) & 0x000F ) >= 0x4 ) );
		if(!isTigerOrHigher)
			return(sal_False);
	}
	
	return(sal_True);
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
::sal_Bool ZipDirectory( const rtl::OUString& dirPath, const rtl::OUString& zipFilePath ) 
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
