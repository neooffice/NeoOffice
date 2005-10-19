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
 *  Patrick Luby, September 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 by Patrick Luby (patrick.luby@planamesa.com)
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

#import <Cocoa/Cocoa.h>
#import "VCLPageFormat_cocoa.h"


@interface VCLPrintInfo : NSPrintInfo
- (id)copyWithZone:(NSZone *)pZone;
@end

@implementation VCLPrintInfo

- (id)copyWithZone:(NSZone *)pZone
{
	// Don't actually make a copy as the JVM keeps making copies of the shared
	// print info and we need all copies to stay in sync whenever the selected
	// printer changes
	return [self retain];
}

@end

@interface InstallVCLPrintInfo : NSObject
- (void)installVCLPrintInfo:(id)pObject;
@end

@implementation InstallVCLPrintInfo

- (void)installVCLPrintInfo:(id)pObject
{
	[VCLPrintInfo poseAsClass:[NSPrintInfo class]];
}

@end

@interface ShowPageLayoutDialog : NSObject
{
	BOOL					mbFinished;
	NSPrintInfo*			mpInfo;
	NSPrintingOrientation	mnOrientation;
	BOOL					mbResult;
	NSWindow*				mpWindow;
}
- (BOOL)finished;
- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow orientation:(NSPrintingOrientation)nOrientation;
- (void)pageLayoutDidEnd:(NSPageLayout *)pLayout returnCode:(int)nCode contextInfo:(void *)pContextInfo;
- (BOOL)result;
- (void)showPageLayoutDialog:(id)pObject;
@end

@implementation ShowPageLayoutDialog

- (BOOL)finished
{
	return mbFinished;
}

- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow orientation:(NSPrintingOrientation)nOrientation
{
	mbFinished = YES;
	mpInfo = pInfo;
	mnOrientation = nOrientation;
	mbResult = NO;
	mpWindow = pWindow;
}

- (void)pageLayoutDidEnd:(NSPageLayout *)pLayout returnCode:(int)nCode contextInfo:(void *)pContextInfo
{
	mbFinished = YES;
	if ( nCode == NSOKButton )
		mbResult = YES;
	else
		mbResult = NO;
}

- (BOOL)result
{
	return mbResult;
}

- (void)showPageLayoutDialog:(id)pObject
{
	NSPageLayout *pLayout = [NSPageLayout pageLayout];
	if ( pLayout )
	{
		if ( [mpInfo orientation] != mnOrientation )
			[mpInfo setOrientation:mnOrientation ];

		// Set the scaling factor to 100% until we can accurately implement
		// scaling in the Java printing code
		NSMutableDictionary *pDict = [mpInfo dictionary];
		if ( pDict )
		{
			NSNumber *pNum = [NSNumber numberWithFloat:1.0];
			if ( pNum )
				[pDict setObject:pNum forKey:NSPrintScalingFactor];
        }

		mbFinished = NO;
		[pLayout beginSheetWithPrintInfo:mpInfo modalForWindow:mpWindow delegate:self didEndSelector:@selector(pageLayoutDidEnd:returnCode:contextInfo:) contextInfo:nil];
	}
}

@end

BOOL NSPageLayout_finished( id pDialog )
{
	BOOL bRet = YES;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		bRet = [(ShowPageLayoutDialog *)pDialog finished];

	[pPool release];

	return bRet;
}

BOOL NSPageLayout_result( id pDialog )
{
	BOOL bRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		bRet = [(ShowPageLayoutDialog *)pDialog result];
		[(ShowPageLayoutDialog *)pDialog release];
	}

	[pPool release];

	return bRet;
}

id NSPrintInfo_create()
{
	VCLPrintInfo *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSPrintInfo *pSharedInfo = [NSPrintInfo sharedPrintInfo];
	if ( pSharedInfo )
	{
		NSDictionary *pDict = [pSharedInfo dictionary];
		if ( pDict )
		{
			pDict = [[NSMutableDictionary alloc] initWithDictionary:pDict];
			if ( pDict )
			{
				pRet = [[VCLPrintInfo alloc] initWithDictionary:pDict];
				if ( pRet )
					pRet = [pRet retain];
			}
		}
	}

	[pPool release];

	return pRet;
}

void NSPrintInfo_installVCLPrintInfo()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	InstallVCLPrintInfo *pInstallVCLPrintInfo = [[InstallVCLPrintInfo alloc] init];
	[pInstallVCLPrintInfo performSelectorOnMainThread:@selector(installVCLPrintInfo:) withObject:pInstallVCLPrintInfo waitUntilDone:YES];

	[pPool release];
}

void NSPrintInfo_setSharedPrintInfo( id pNSPrintInfo )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSPrintInfo )
		[VCLPrintInfo setSharedPrintInfo:pNSPrintInfo];

	[pPool release];
}

id NSPrintInfo_showPageLayoutDialog( id pNSPrintInfo, id pNSWindow, BOOL bLandscape )
{
	ShowPageLayoutDialog *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSPrintInfo && pNSWindow )
	{
		pRet = [[ShowPageLayoutDialog alloc] initWithPrintInfo:(NSPrintInfo *)pNSPrintInfo window:(NSWindow *)pNSWindow orientation:( bLandscape ? NSLandscapeOrientation : NSPortraitOrientation )];
		if ( pRet )
		{
			[pRet performSelectorOnMainThread:@selector(showPageLayoutDialog:) withObject:pRet waitUntilDone:YES];
			[pRet retain];
		}
	}

	[pPool release];

	return pRet;
}
