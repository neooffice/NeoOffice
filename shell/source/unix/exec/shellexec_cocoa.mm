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
 *  Patrick Luby, August 2015
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2015 Planamesa Inc.
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
 ************************************************************************/

#include <osl/file.hxx>
#include <osl/objcutils.h>

#import "shellexec.hxx"
#import "shellexec_cocoa.h"

using namespace com::sun::star::uno;

@interface ShellExecOpenURL : NSObject
{
	BOOL				mbSelectInFinder;
	BOOL				mbResult;
	NSURL*				mpURL;
}
+ (id)createWithURL:(NSURL *)pURL selectInFinder:(BOOL)bSelectInFinder;
- (void)dealloc;
- (id)initWithURL:(NSURL *)pURL selectInFinder:(BOOL)bSelectInFinder;
- (void)openURL:(id)pSender;
- (BOOL)result;
@end

@implementation ShellExecOpenURL

+ (id)createWithURL:(NSURL *)pURL selectInFinder:(BOOL)bSelectInFinder
{
	ShellExecOpenURL *pRet = [[ShellExecOpenURL alloc] initWithURL:pURL selectInFinder:bSelectInFinder];
	[pRet autorelease];
	return pRet;
}

- (void)dealloc
{
	if ( mpURL )
		[mpURL release];

	[super dealloc];
}

- (id)initWithURL:(NSURL *)pURL selectInFinder:(BOOL)bSelectInFinder
{
	[super init];

	mbSelectInFinder = bSelectInFinder;
	mbResult = NO;
	mpURL = pURL;
	if ( mpURL )
		[mpURL retain];

	return self;
}

- (void)openURL:(id)pSender
{
	(void)pSender;

	mbResult = NO;

	NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
	if ( pWorkspace && mpURL )
	{
		if ( mbSelectInFinder )
		{
			[pWorkspace activateFileViewerSelectingURLs:[NSArray arrayWithObject:mpURL]];
			mbResult = YES;
		}
		else
		{
			mbResult = [pWorkspace openURL:mpURL];
			if ( !mbResult )
			{
				NSURL *pAppURL = nil;
				if ( [@"mailto" isEqualToString:[mpURL scheme]] )
					pAppURL = [pWorkspace URLForApplicationWithBundleIdentifier:@"com.apple.mail"];
				else
					pAppURL = [pWorkspace URLForApplicationWithBundleIdentifier:@"com.apple.Safari"];
				NSWorkspaceOpenConfiguration *pConfiguration = [NSWorkspaceOpenConfiguration configuration];
				if ( pAppURL && pConfiguration )
				{
					[pWorkspace openURLs:[NSArray arrayWithObject:mpURL] withApplicationAtURL:pAppURL configuration:pConfiguration completionHandler:nil];
					mbResult = YES;
				}
			}
		}
	}
}

- (BOOL)result
{
	return mbResult;
}

@end

sal_Bool ShellExec_openURL( OUString &rURL, sal_Bool bSelectInFinder )
{
	sal_Bool bRet = sal_False;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSString *pURLString = [NSString stringWithCharacters:rURL.getStr() length:rURL.getLength()];
	if ( pURLString )
	{
		NSURL *pURL = [NSURL URLWithString:pURLString];
		if ( pURL )
		{
			// If the file is an alias file, open the directory in the Finder
			// like LibO does for softlinks and other insecure extensions
			NSNumber *pAlias = nil;
			if ( !bSelectInFinder && [pURL getResourceValue:&pAlias forKey:NSURLIsAliasFileKey error:nil] && pAlias && [pAlias boolValue] )
				bSelectInFinder = sal_True;

			ShellExecOpenURL *pShellExecOpenURL = [ShellExecOpenURL createWithURL:pURL selectInFinder:bSelectInFinder];
			osl_performSelectorOnMainThread( pShellExecOpenURL, @selector(openURL:), pShellExecOpenURL, YES );
			bRet = [pShellExecOpenURL result];
		}
	}

	[pPool release];

	return bRet;
}
