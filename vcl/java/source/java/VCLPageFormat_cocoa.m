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
{
}
- (id)copyWithZone:(NSZone *)pZone;
- (void)installVCLPrintInfo:(id)pObject;
@end

@implementation VCLPrintInfo

- (id)copyWithZone:(NSZone *)pZone
{
	// Don't actually make a copy as the JVM keeps making copies of the shared
	// print info and we need all copies to stay in sync whenever the selected
	// printer changes
	return [self retain];
}

- (void)installVCLPrintInfo:(id)pObject;
{
	[VCLPrintInfo poseAsClass:[NSPrintInfo class]];
}

@end

@interface ShowPageLayoutDialog : NSObject
{
	NSPrintInfo*			mpInfo;
	NSLock*					mpLock;
	NSPrintingOrientation	mnOrientation;
	BOOL					mbResult;
	NSWindow*				mpWindow;
}
- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow orientation:(NSPrintingOrientation)nOrientation lock:(NSLock *)pLock;
- (void)pageLayoutDidEnd:(NSPageLayout *)pLayout returnCode:(int)nCode contextInfo:(void *)pContextInfo;
- (BOOL)result;
- (void)showPageLayoutDialog:(id)pObject;
@end

@implementation ShowPageLayoutDialog

- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow orientation:(NSPrintingOrientation)nOrientation lock:(NSLock *)pLock
{
	mpInfo = pInfo;
	mpLock = pLock;
	mnOrientation = nOrientation;
	mbResult = NO;
	mpWindow = pWindow;
}

- (void)pageLayoutDidEnd:(NSPageLayout *)pLayout returnCode:(int)nCode contextInfo:(void *)pContextInfo
{
	if ( nCode == NSOKButton )
		mbResult = YES;
	else
		mbResult = NO;
	[mpLock unlock];
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
		[mpLock lock];
		if ( [mpInfo orientation] != mnOrientation )
			[mpInfo setOrientation:mnOrientation ];
		[pLayout beginSheetWithPrintInfo:mpInfo modalForWindow:mpWindow delegate:self didEndSelector:@selector(pageLayoutDidEnd:returnCode:contextInfo:) contextInfo:nil];
	}
}

@end

id NSPrintInfo_create()
{
	VCLPrintInfo *pRet = nil;

	NSPrintInfo *pSharedInfo = [NSPrintInfo sharedPrintInfo];
	if ( pSharedInfo )
	{
		NSDictionary *pDict = [pSharedInfo dictionary];
		if ( pDict )
		{
			pDict = [[NSMutableDictionary alloc] initWithDictionary:pDict];
			if ( pDict )
				pRet = [[VCLPrintInfo alloc] initWithDictionary:pDict];
		}
	}

	return pRet;
}

void NSPrintInfo_installVCLPrintInfo()
{
	VCLPrintInfo *pVCLPrintInfo = [[VCLPrintInfo alloc] init];
	[pVCLPrintInfo performSelectorOnMainThread:@selector(installVCLPrintInfo:) withObject:pVCLPrintInfo waitUntilDone:YES];
	[pVCLPrintInfo release];
}

void NSPrintInfo_setSharedPrintInfo( id pNSPrintInfo )
{
	if ( pNSPrintInfo )
		[VCLPrintInfo setSharedPrintInfo:pNSPrintInfo];
}

BOOL NSPrintInfo_showPageLayoutDialog( id pNSPrintInfo, id pNSWindow, BOOL bLandscape )
{
	BOOL bRet = NO;

	if ( pNSPrintInfo && pNSWindow )
	{
		NSLock *pLock = [[NSLock alloc] init];
		if ( pLock )
		{
			ShowPageLayoutDialog *pShowPageLayoutDialog = [[ShowPageLayoutDialog alloc] initWithPrintInfo:(NSPrintInfo *)pNSPrintInfo window:(NSWindow *)pNSWindow orientation:( bLandscape ? NSLandscapeOrientation : NSPortraitOrientation ) lock:pLock];
			[pShowPageLayoutDialog performSelectorOnMainThread:@selector(showPageLayoutDialog:) withObject:pShowPageLayoutDialog waitUntilDone:YES];
			[pLock lock];
			bRet = [pShowPageLayoutDialog result];
			[pLock unlock];
			[pShowPageLayoutDialog release];
			[pLock release];
		}
	}

	return bRet;
}
