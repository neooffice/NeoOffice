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

static BOOL bInDialog = NO;

void HidePageSetupControls( NSView *pView )
{
	if ( pView )
	{
		NSArray *pArray = [pView subviews];
		if ( pArray )
		{
			NSView *pScaleView = nil;
			unsigned nCount = [pArray count];
			unsigned i = 0;
			for ( ; i < nCount; i++ )
			{
				NSView *pSubview = (NSView *)[pArray objectAtIndex:i];
				HidePageSetupControls( pSubview );

				if ( [pSubview isKindOfClass:[NSControl class]] )
				{
					if ( ![pSubview isKindOfClass:[NSButton class]] || [pSubview isKindOfClass:[NSPopUpButton class]] )
						[pSubview setHidden:YES];

					if ( [pSubview isKindOfClass:[NSTextField class]] && [(NSTextField *)pSubview isEditable] )
						pScaleView = pSubview;
				}
			}

			// Show the label that matches the scale view
			if ( pScaleView )
			{
				NSRect aScaleFrame = [pScaleView frame];
				i = 0;
				for ( ; i < nCount; i++ )
				{
					NSView *pSubview = (NSView *)[pArray objectAtIndex:i];
					if ( pSubview != pScaleView )
					{
						if ( [pSubview isKindOfClass:[NSTextField class]] && ![(NSTextField *)pSubview isEditable] )
						{
							NSRect aSubviewFrame = [pSubview frame];
							if ( aSubviewFrame.origin.y < aScaleFrame.origin.y + aScaleFrame.size.height && aSubviewFrame.origin.y + aSubviewFrame.size.height > aScaleFrame.origin.y )
							{
								[pSubview setHidden:NO];
								[pScaleView setHidden:NO];
								break;
							}
						}
					}
				}
			}
		}
	}
}

@interface VCLPrintInfo : NSPrintInfo
+ (void)setInDialog:(BOOL)bIn;
- (id)copyWithZone:(NSZone *)pZone;
- (void)setPrinter:(NSPrinter *)pPrinter;
@end

@implementation VCLPrintInfo

+ (void)setInDialog:(BOOL)bIn
{
	bInDialog = bIn;
}

- (id)copyWithZone:(NSZone *)pZone
{
	// Don't actually make a copy as the JVM keeps making copies of the shared
	// print info and we need all copies to stay in sync whenever the selected
	// printer changes
	return [self retain];
}

- (void)setPrinter:(NSPrinter *)pPrinter
{
	// Only allow the native Cocoa dialogs to change the printer
	if ( bInDialog && pPrinter )
		[super setPrinter:pPrinter];
}

@end

@interface VCLPrintOperation : NSPrintOperation
+ (NSPrintOperation *)printOperationWithView:(NSView *)aView printInfo:(NSPrintInfo *)aPrintInfo;
@end

@implementation VCLPrintOperation

+ (NSPrintOperation *)printOperationWithView:(NSView *)aView printInfo:(NSPrintInfo *)aPrintInfo
{
	NSPrintOperation *pOperation = [[VCLPrintOperation superclass] printOperationWithView:aView printInfo:aPrintInfo];

	[NSPrintOperation setCurrentOperation:pOperation];

	return pOperation;
}

@end

@interface InstallVCLPrintClasses : NSObject
- (void)installVCLPrintClasses:(id)pObject;
@end

@implementation InstallVCLPrintClasses

- (void)installVCLPrintClasses:(id)pObject
{
	[VCLPrintInfo poseAsClass:[NSPrintInfo class]];
	[VCLPrintOperation poseAsClass:[NSPrintOperation class]];
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
	[super init];

	mbFinished = YES;
	mpInfo = pInfo;
	mnOrientation = nOrientation;
	mbResult = NO;
	mpWindow = pWindow;

	return self;
}

- (void)pageLayoutDidEnd:(NSPageLayout *)pLayout returnCode:(int)nCode contextInfo:(void *)pContextInfo
{
	[VCLPrintInfo setInDialog:NO];
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

		mbFinished = NO;
		[VCLPrintInfo setInDialog:YES];
		[pLayout beginSheetWithPrintInfo:mpInfo modalForWindow:mpWindow delegate:self didEndSelector:@selector(pageLayoutDidEnd:returnCode:contextInfo:) contextInfo:nil];

		// Disable all but the scale text field since the other controls have
		// no effect
		NSWindow *pSheet = (NSWindow *)[mpWindow attachedSheet];
		if ( pSheet )
			HidePageSetupControls( [pSheet contentView] );
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

void NSPrintInfo_getPrintInfoDimensions( id pNSPrintInfo, float *pWidth, float *pHeight, float *pImageableX, float *pImageableY, float *pImageableWidth, float *pImageableHeight )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSPrintInfo *pInfo = (NSPrintInfo *)pNSPrintInfo;
	if ( pInfo && pWidth && pHeight && pImageableX && pImageableY && pImageableWidth && pImageableHeight )
	{
		// Fix bug 2333 by setting the imageable bounds to the page bounds as
		// using the true imageable bounds does cause uncontrollable shifting
		// and clipping in the OOo code. The downside of this fix is that the
		// user will never get a warning about too small margins in the
		// Format :: Page dialog, but this seems to be a necessary tradeoff.
		NSSize aSize = [pInfo paperSize];
		*pWidth = aSize.width;
		*pHeight = aSize.height;
		*pImageableX = 0;
		*pImageableY = 0;
		*pImageableWidth = aSize.width;
		*pImageableHeight = aSize.height;
	}

	[pPool release];
}

void NSPrintInfo_setInDialog( BOOL bIn )
{
	[VCLPrintInfo setInDialog:bIn];
}

BOOL NSPrintInfo_setPaperSize( id pNSPrintInfo, long nWidth, long nHeight )
{
	BOOL bRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSPrintInfo *pInfo = (NSPrintInfo *)pNSPrintInfo;
	if ( pNSPrintInfo && nWidth > 0 && nHeight > 0 )
	{
		NSPrintingOrientation nOldOrientation = [pInfo orientation];
		NSSize aOldSize = [pInfo paperSize];

		[pInfo setOrientation:NSPortraitOrientation];
		[pInfo setPaperSize:NSMakeSize((float)nWidth, (float)nHeight)];

		// Fix bug 2202 by setting the orientation back to the original state
		NSSize aSize = [pInfo paperSize];
		if ( aSize.width < 1.0 || aSize.height < 1.0 )
		{
			[pInfo setPaperSize:aOldSize];
		}
		else
		{
			// Fix bugs 543, 1678, and 2202 by detecting when the paper should
			// be rotated determining the minimum unmatched area
			double fDiff = pow( (double)aSize.width - nWidth, 2 ) + pow( (double)aSize.height - nHeight, 2 );
			double fRotatedDiff = pow( (double)aSize.width - nHeight, 2 ) + pow( (double)aSize.height - nWidth, 2 );
			if ( fDiff > fRotatedDiff )
				bRet = YES;
		}

		[pInfo setOrientation:nOldOrientation];
	}

	[pPool release];

	return bRet;
}

void NSPrintInfo_setSharedPrintInfo( id pNSPrintInfo )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSPrintInfo )
		[VCLPrintInfo setSharedPrintInfo:(NSPrintInfo *)pNSPrintInfo];

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
			[pRet retain];
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pRet performSelectorOnMainThread:@selector(showPageLayoutDialog:) withObject:pRet waitUntilDone:YES modes:pModes];
		}
	}

	[pPool release];

	return pRet;
}

void VCLPrintInfo_installVCLPrintClasses()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	InstallVCLPrintClasses *pInstallVCLPrintClasses = [[InstallVCLPrintClasses alloc] init];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pInstallVCLPrintClasses performSelectorOnMainThread:@selector(installVCLPrintClasses:) withObject:pInstallVCLPrintClasses waitUntilDone:YES modes:pModes];

	[pPool release];
}
