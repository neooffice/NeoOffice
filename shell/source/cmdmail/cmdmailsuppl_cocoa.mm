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
 *  Patrick Luby, September 2014
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2014 Planamesa Inc.
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

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include <osl/file.hxx>

#import "cmdmailsuppl.hxx"
#import "cmdmailsuppl_cocoa.h"

using namespace com::sun::star::uno;

@interface CmdMailSupplOpenURLs : NSObject
{
	NSString*			mpAppID;
	BOOL				mbResult;
	NSArray*			mpURLs;
}
+ (id)createWithURLs:(NSArray *)pURLs appID:(NSString *)pAppID;
- (void)dealloc;
- (id)initWithURLs:(NSArray *)pURLs appID:(NSString *)pAppID;
- (void)openURLs:(id)pSender;
- (BOOL)result;
@end

@implementation CmdMailSupplOpenURLs

+ (id)createWithURLs:(NSArray *)pURLs appID:(NSString *)pAppID
{
	CmdMailSupplOpenURLs *pRet = [[CmdMailSupplOpenURLs alloc] initWithURLs:pURLs appID:pAppID];
	[pRet autorelease];
	return pRet;
}

- (void)dealloc
{
	if ( mpAppID )
		[mpAppID release];

	if ( mpURLs )
		[mpURLs release];

	[super dealloc];
}

- (id)initWithURLs:(NSArray *)pURLs appID:(NSString *)pAppID
{
	[super init];

	mpAppID = pAppID;
	if ( mpAppID )
		[mpAppID retain];
	mbResult = NO;
	mpURLs = pURLs;
	if ( mpURLs )
		[mpURLs retain];

	return self;
}

- (void)openURLs:(id)pSender
{
	(void)pSender;

	mbResult = NO;

	NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
	if ( pWorkspace && mpURLs && [mpURLs count] )
	{
		if ( mpAppID && [mpAppID length] )
			mbResult = [pWorkspace openURLs:mpURLs withAppBundleIdentifier:mpAppID options:NSWorkspaceLaunchDefault additionalEventParamDescriptor:nil launchIdentifiers:nil];
		if ( !mbResult )
		{
			NSURL *pDefaultMailApp = [pWorkspace URLForApplicationToOpenURL:[NSURL URLWithString:@"mailto://"]];
			if ( pDefaultMailApp )
			{
				NSBundle *pBundle = [NSBundle bundleWithURL:pDefaultMailApp];
				if ( pBundle )
				{
					NSString *pDefaultMailAppID = [pBundle bundleIdentifier];
					if ( pDefaultMailAppID && [pDefaultMailAppID length] )
						mbResult = [pWorkspace openURLs:mpURLs withAppBundleIdentifier:pDefaultMailAppID options:NSWorkspaceLaunchDefault additionalEventParamDescriptor:nil launchIdentifiers:nil];
				}
			}
		}

		if ( !mbResult )
			mbResult = [pWorkspace openURLs:mpURLs withAppBundleIdentifier:@"com.apple.mail" options:NSWorkspaceLaunchDefault additionalEventParamDescriptor:nil launchIdentifiers:nil];
	}
}

- (BOOL)result
{
	return mbResult;
}

@end

sal_Bool CmdMailSuppl_sendSimpleMailMessage( Sequence< OUString > &rStringList, OUString aMailerPath )
{
	sal_Bool bRet = sal_False;

	sal_Int32 nLen = rStringList.getLength();
	if ( !nLen )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSMutableArray *pURLs = [NSMutableArray arrayWithCapacity:nLen];
	if ( pURLs )
	{
		sal_Int32 i = 0;
		for ( ; i < nLen; i++ )
		{
			OUString aSystemPath;
			if ( ::osl::FileBase::E_None == ::osl::FileBase::getSystemPathFromFileURL( rStringList[ i ], aSystemPath ) )
			{
				NSString *pAttachPath = [NSString stringWithCharacters:aSystemPath.getStr() length:aSystemPath.getLength()];
				if ( pAttachPath )
				{
					NSURL *pAttachURL = [NSURL fileURLWithPath:pAttachPath];
					if ( pAttachURL )
						[pURLs addObject:pAttachURL];
				}
			}
		}

		if ( [pURLs count] )
		{
			NSString *pAppID = nil;
			if ( aMailerPath.getLength() )
			{
				NSString *pMailerPath = [NSString stringWithCharacters:aMailerPath.getStr() length:aMailerPath.getLength()];
				if ( pMailerPath )
				{
					NSBundle *pBundle = [NSBundle bundleWithPath:pMailerPath];
					if ( pBundle )
						pAppID = [pBundle bundleIdentifier];

					// Unquote single quoted paths. In previous versions, the
					// mailer path was stored with single quotes.
					if ( !pAppID && [pMailerPath length] > 2 && [pMailerPath characterAtIndex:0] == '\'' && [pMailerPath characterAtIndex:[pMailerPath length] - 1] == '\'')
					{
						pMailerPath = [pMailerPath substringWithRange:NSMakeRange( 1, [pMailerPath length] - 2)];
						if ( pMailerPath )
						{
							pBundle = [NSBundle bundleWithPath:pMailerPath];
							if ( pBundle )
								pAppID = [pBundle bundleIdentifier];
						}
					}
				}
			}

			CmdMailSupplOpenURLs *pCmdMailSupplOpenURLs = [CmdMailSupplOpenURLs createWithURLs:pURLs appID:pAppID];
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pCmdMailSupplOpenURLs performSelectorOnMainThread:@selector(openURLs:) withObject:pCmdMailSupplOpenURLs waitUntilDone:YES modes:pModes];
			bRet = [pCmdMailSupplOpenURLs result];
		}
	}

	[pPool release];

	return bRet;
}
