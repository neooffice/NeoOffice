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
 *  Copyright 2005 Planamesa Inc.
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

@interface VCLPrintOperation : NSPrintOperation
+ (NSPrintOperation *)printOperationWithView:(NSView *)pView;
+ (NSPrintOperation *)printOperationWithView:(NSView *)pView printInfo:(NSPrintInfo *)pPrintInfo;
@end

@implementation VCLPrintOperation

+ (NSPrintOperation *)printOperationWithView:(NSView *)pView
{
	return [VCLPrintOperation printOperationWithView:pView printInfo:[NSPrintInfo sharedPrintInfo]];
}

+ (NSPrintOperation *)printOperationWithView:(NSView *)pView printInfo:(NSPrintInfo *)pPrintInfo
{
	// Fix bugs 1688 and 2517 by restoring the print info's dictionary values
	// set by the native print dialog
	if ( pPrintInfo )
	{
		NSMutableDictionary *pDictionary = [(NSPrintInfo *)pPrintInfo dictionary];
		if ( pDictionary )
		{
			NSPrintInfo *pRealPrintInfo = [pDictionary objectForKey:(NSString *)VCLPrintInfo_getVCLPrintInfoDictionaryKey()];
			if ( pRealPrintInfo )
			{
				// Fix bug 2900 by synching the printers between print info
				// instances. Fix bug 2908 by setting the Java print info's
				// printer to the native print dialog's printer.
				[pPrintInfo setPrinter:[pRealPrintInfo printer]];
				pPrintInfo = pRealPrintInfo;
			}
		}
	}

	return [[VCLPrintOperation superclass] printOperationWithView:pView printInfo:pPrintInfo];
}

@end

@interface InstallVCLPrintClasses : NSObject
+ (id)create;
- (void)installVCLPrintClasses:(id)pObject;
@end

@implementation InstallVCLPrintClasses

+ (id)create
{
	InstallVCLPrintClasses *pRet = [[InstallVCLPrintClasses alloc] init];
	[pRet autorelease];
	return pRet;
}

- (void)installVCLPrintClasses:(id)pObject
{
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
	NSPrintInfo *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSPrintInfo *pSharedInfo = [NSPrintInfo sharedPrintInfo];
	if ( pSharedInfo )
	{
		NSMutableDictionary *pDictionary = [pSharedInfo dictionary];
		if ( pDictionary )
		{
			// Some users seem to get scaling values other than 100% so
			// force the scaling factor here
			NSNumber *pValue = [NSNumber numberWithFloat:1.0f];
			if ( pValue )
				[pDictionary setObject:pValue forKey:NSPrintScalingFactor];

			// Fix bug 2573 by not cloning the dictionary as that will cause
			// querying of the printer which, in turn, will cause hanging if
			// the printer is an unavailable network printer
			// Do not retain as invoking alloc disables autorelease
			pRet = [[NSPrintInfo alloc] initWithDictionary:pDictionary];
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
		NSMutableDictionary *pDictionary = [pInfo dictionary];
		if ( pDictionary )
		{
			NSValue *pValue = [pDictionary objectForKey:NSPrintPaperSize];
			if ( pValue )
			{
				NSSize aSize = [pValue sizeValue];
				*pWidth = aSize.width;
				*pHeight = aSize.height;
				*pImageableX = 0;
				*pImageableY = 0;
				*pImageableWidth = aSize.width;
				*pImageableHeight = aSize.height;
			}
		}
	}

	[pPool release];
}

BOOL NSPrintInfo_setPaperSize( id pNSPrintInfo, long nWidth, long nHeight )
{
	BOOL bRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSPrintInfo *pInfo = (NSPrintInfo *)pNSPrintInfo;
	if ( pNSPrintInfo && nWidth > 0 && nHeight > 0 )
	{
		NSMutableDictionary *pDictionary = [pInfo dictionary];
		if ( pDictionary )
		{
			NSSize aSize = NSMakeSize( (float)nWidth, (float)nHeight );
			[pInfo setOrientation:NSPortraitOrientation];
			NSValue *pValue = [NSValue valueWithSize:aSize];
			if ( pValue )
			{
				[pDictionary setObject:pValue forKey:NSPrintPaperSize];

				// Fix bugs 543, 1678, and 2202 by detecting when the paper
				// should be rotated determining the minimum unmatched area
				double fDiff = pow( (double)aSize.width - nWidth, 2 ) + pow( (double)aSize.height - nHeight, 2 );
				double fRotatedDiff = pow( (double)aSize.width - nHeight, 2 ) + pow( (double)aSize.height - nWidth, 2 );
				if ( fDiff > fRotatedDiff )
					bRet = YES;
			}

			NSPrintInfo *pRealInfo = [pDictionary objectForKey:(NSString *)VCLPrintInfo_getVCLPrintInfoDictionaryKey()];
			if ( pRealInfo )
				NSPrintInfo_setPaperSize( pRealInfo, nWidth, nHeight );
		}
	}

	[pPool release];

	return bRet;
}

void NSPrintInfo_setSharedPrintInfo( id pNSPrintInfo )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSPrintInfo )
		[NSPrintInfo setSharedPrintInfo:(NSPrintInfo *)pNSPrintInfo];

	[pPool release];
}

id NSPrintInfo_showPageLayoutDialog( id pNSPrintInfo, id pNSWindow, BOOL bLandscape )
{
	ShowPageLayoutDialog *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSPrintInfo && pNSWindow )
	{
		// Do not retain as invoking alloc disables autorelease
		pRet = [[ShowPageLayoutDialog alloc] initWithPrintInfo:(NSPrintInfo *)pNSPrintInfo window:(NSWindow *)pNSWindow orientation:( bLandscape ? NSLandscapeOrientation : NSPortraitOrientation )];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pRet performSelectorOnMainThread:@selector(showPageLayoutDialog:) withObject:pRet waitUntilDone:YES modes:pModes];
	}

	[pPool release];

	return pRet;
}

CFStringRef VCLPrintInfo_getVCLPrintInfoDictionaryKey()
{
	return CFSTR( "VCLPrintInfoDictionaryKey" );
}

void VCLPrintInfo_installVCLPrintClasses()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	InstallVCLPrintClasses *pInstallVCLPrintClasses = [InstallVCLPrintClasses create];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pInstallVCLPrintClasses performSelectorOnMainThread:@selector(installVCLPrintClasses:) withObject:pInstallVCLPrintClasses waitUntilDone:YES modes:pModes];

	[pPool release];
}
