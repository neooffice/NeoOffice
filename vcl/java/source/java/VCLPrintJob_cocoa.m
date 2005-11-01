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
	BOOL					mbFinished;
	NSPrintInfo*			mpInfo;
	BOOL					mbResult;
	NSWindow*				mpWindow;
}
- (BOOL)finished;
- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow;
- (BOOL)result;
- (void)showPrintDialog:(id)pObject;
@end

@implementation ShowPrintDialog

- (BOOL)finished
{
	return mbFinished;
}

- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow
{
	mbFinished = YES;
	mpInfo = pInfo;
	mbResult = NO;
	mpWindow = pWindow;
}

- (void)printPanelDidEnd:(NSPrintPanel *)pPanel returnCode:(int)nCode contextInfo:(void *)pContextInfo
{
	NSPrintInfo_setInDialog( NO );
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

- (void)showPrintDialog:(id)pObject
{
	NSPrintPanel *pPanel = [NSPrintPanel printPanel];
	if ( pPanel )
	{
		// Set the job name from the window title
		PMPrintSettings aSettings = (PMPrintSettings)[mpInfo pmPrintSettings];
		if ( aSettings )
		{
			NSString *pTitle = [mpWindow title];
			if ( pTitle )
				pTitle = [pTitle stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
			else
				pTitle = [NSString stringWithString:@"Untitled"];

			if ( pTitle)
				PMSetJobNameCFString( aSettings, (CFStringRef)pTitle );
		}

		mbFinished = NO;
		NSPrintInfo_setInDialog( YES );
		[mpInfo setPrinter:[NSPrintInfo defaultPrinter]];
		[pPanel beginSheetWithPrintInfo:mpInfo modalForWindow:mpWindow delegate:self didEndSelector:@selector(printPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
	}
}

@end

BOOL NSPrintInfo_pageRange( id pNSPrintInfo, int *nFirst, int *nLast )
{
	BOOL bRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

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
						bRet = YES;
				}
			}
		}
	}

	[pPool release];

	return bRet;
}

id NSPrintInfo_showPrintDialog( id pNSPrintInfo, id pNSWindow )
{
	ShowPrintDialog *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSPrintInfo && pNSWindow )
	{
		pRet = [[ShowPrintDialog alloc] initWithPrintInfo:(NSPrintInfo *)pNSPrintInfo window:(NSWindow *)pNSWindow];
		if ( pRet )
		{
			[pRet retain];
			[pRet performSelectorOnMainThread:@selector(showPrintDialog:) withObject:pRet waitUntilDone:YES];
		}
	}

	[pPool release];

	return pRet;
}

BOOL NSPrintPanel_finished( id pDialog )
{
	BOOL bRet = YES;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
		bRet = [(ShowPrintDialog *)pDialog finished];

	[pPool release];

	return bRet;
}

BOOL NSPrintPanel_result( id pDialog )
{
	BOOL bRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pDialog )
	{
		bRet = [(ShowPrintDialog *)pDialog result];
		[(ShowPrintDialog *)pDialog release];
	}

	[pPool release];

	return bRet;
}
