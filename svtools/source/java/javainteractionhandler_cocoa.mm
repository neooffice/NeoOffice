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
 *         - GNU General Public License Version 2.1
 *
 *  Patrick Luby, July 2015
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

#import "javainteractionhandler_cocoa.h"

@interface OpenJavaDownloadURL : NSObject
{
}
+ (id)create;
- (id)init;
- (void)openJavaDownloadURL:(id)pObject;
@end

@implementation OpenJavaDownloadURL

+ (id)create
{
	OpenJavaDownloadURL *pRet = [[OpenJavaDownloadURL alloc] init];
	[pRet autorelease];
	return pRet;
}

- (id)init
{
	[super init];

	return self;
}

- (void)openJavaDownloadURL:(id)pObject
{
	(void)pObject;

	NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
#ifdef PRODUCT_JAVA_DOWNLOAD_URL
	NSURL *pURL = [NSURL URLWithString:static_cast< NSString* >( CFSTR( PRODUCT_JAVA_DOWNLOAD_URL ) )];
	if ( pWorkspace && pURL && ( [@"macappstores" isEqualToString:[pURL scheme]] || [@"http" isEqualToString:[pURL scheme]] || [@"https" isEqualToString:[pURL scheme]] ) )
		[pWorkspace openURL:pURL];
#endif	// PRODUCT_JAVA_DOWNLOAD_URL
}

@end

#ifdef PRODUCT_JAVA_DOWNLOAD_URL

void JavaInteractionHandler_downloadJava()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	OpenJavaDownloadURL *pOpenJavaDownloadURL = [OpenJavaDownloadURL create];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pOpenJavaDownloadURL performSelectorOnMainThread:@selector(openJavaDownloadURL:) withObject:pOpenJavaDownloadURL waitUntilDone:NO modes:pModes];

	[pPool release];
}

#endif	// PRODUCT_JAVA_DOWNLOAD_URL
