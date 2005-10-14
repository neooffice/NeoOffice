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
#import "VCLPrintJob_cocoa.h"

@interface ShowPrintDialog : NSObject
{
	NSPrintInfo*			mpInfo;
	NSLock*					mpLock;
	BOOL					mbResult;
	NSWindow*				mpWindow;
}
- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow lock:(NSLock *)pLock;
- (BOOL)result;
- (void)showPrintDialog:(id)pObject;
@end

@implementation ShowPrintDialog

- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow lock:(NSLock *)pLock
{
	mpInfo = pInfo;
	mpLock = pLock;
	mbResult = NO;
	mpWindow = pWindow;
}

- (void)printPanelDidEnd:(NSPrintPanel *)pPanel returnCode:(int)nCode contextInfo:(void *)pContextInfo
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

- (void)showPrintDialog:(id)pObject
{
	NSPrintPanel *pPanel = [NSPrintPanel printPanel];
	if ( pPanel )
	{
		[mpLock lock];
		[pPanel beginSheetWithPrintInfo:mpInfo modalForWindow:mpWindow delegate:self didEndSelector:@selector(printPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
	}
}

@end

BOOL NSPrintInfo_pageRange( id pNSPrintInfo, int *nFirst, int *nLast )
{
	if ( pNSPrintInfo && nFirst && nLast )
	{
		NSMutableDictionary *pDictionary = [(NSPrintInfo *)pNSPrintInfo dictionary];
		if ( pDictionary )
		{
			NSNumber *pNumber = [pDictionary objectForKey:NSPrintAllPages];
			if ( !pNumber || ![pNumber boolValue] )
			{
				NSNumber *pFirst = [pDictionary objectForKey:NSPrintFirstPage];
				NSNumber *pLast = [pDictionary objectForKey:NSPrintLastPage];
				if ( pFirst )
				{
					*nFirst = [pFirst intValue];
					*nLast = [pLast intValue];
					if ( nFirst > 0 && nLast > nFirst )
						return YES;
				}
			}
		}
	}

	return NO;
}

BOOL NSPrintInfo_showPrintDialog( id pNSPrintInfo, id pNSWindow )
{
	BOOL bRet = NO;

	if ( pNSPrintInfo && pNSWindow )
	{
		NSLock *pLock = [[NSLock alloc] init];
		if ( pLock )
		{
			ShowPrintDialog *pShowPrintDialog = [[ShowPrintDialog alloc] initWithPrintInfo:(NSPrintInfo *)pNSPrintInfo window:(NSWindow *)pNSWindow lock:pLock];
			[pShowPrintDialog performSelectorOnMainThread:@selector(showPrintDialog:) withObject:pShowPrintDialog waitUntilDone:YES];
			[pLock lock];
			bRet = [pShowPrintDialog result];
			[pLock unlock];
			[pShowPrintDialog release];
			[pLock release];
		}
	}

	return bRet;
}
