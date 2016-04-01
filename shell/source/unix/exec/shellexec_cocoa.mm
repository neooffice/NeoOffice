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

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include <osl/file.hxx>

#import "shellexec.hxx"
#import "shellexec_cocoa.h"

using namespace com::sun::star::uno;

@interface ShellExecOpenURL : NSObject
{
	BOOL				mbResult;
	NSURL*				mpURL;
}
+ (id)createWithURL:(NSURL *)pURL;
- (void)dealloc;
- (id)initWithURL:(NSURL *)pURL;
- (void)openURL:(id)pSender;
- (BOOL)result;
@end

@implementation ShellExecOpenURL

+ (id)createWithURL:(NSURL *)pURL
{
	ShellExecOpenURL *pRet = [[ShellExecOpenURL alloc] initWithURL:pURL];
	[pRet autorelease];
	return pRet;
}

- (void)dealloc
{
	if ( mpURL )
		[mpURL release];

	[super dealloc];
}

- (id)initWithURL:(NSURL *)pURL
{
	[super init];

	mbResult = NO;
	mpURL = pURL;
	if ( mpURL )
		[mpURL retain];

	return self;
}

- (void)openURL:(id)pSender
{
	mbResult = NO;

	NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
	if ( pWorkspace && mpURL )
	{
		NSArray *pURLs = [NSArray arrayWithObject:mpURL];
		if ( pURLs )
		{
			mbResult = [pWorkspace openURLs:pURLs withAppBundleIdentifier:nil options:NSWorkspaceLaunchDefault additionalEventParamDescriptor:nil launchIdentifiers:nil];
			if ( !mbResult )
			{
				NSString *pAppID = nil;
				if ( [@"mailto" isEqualToString:[mpURL scheme]] )
					pAppID = @"com.apple.mail";
				else
					pAppID = @"com.apple.Safari";
				mbResult = [pWorkspace openURLs:pURLs withAppBundleIdentifier:pAppID options:NSWorkspaceLaunchDefault additionalEventParamDescriptor:nil launchIdentifiers:nil];
			}
		}
	}
}

- (BOOL)result
{
	return mbResult;
}

@end

sal_Bool ShellExec_openURL( ::rtl::OUString &rURL )
{
	sal_Bool bRet = sal_False;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSString *pURLString = [NSString stringWithCharacters:rURL.getStr() length:rURL.getLength()];
	if ( pURLString )
	{
		NSURL *pURL = [NSURL URLWithString:pURLString];
		if ( pURL )
		{
			ShellExecOpenURL *pShellExecOpenURL = [ShellExecOpenURL createWithURL:pURL];
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pShellExecOpenURL performSelectorOnMainThread:@selector(openURL:) withObject:pShellExecOpenURL waitUntilDone:YES modes:pModes];
			bRet = [pShellExecOpenURL result];
		}
	}

	[pPool release];

	return bRet;
}
