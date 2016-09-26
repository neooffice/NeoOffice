/*************************************************************************
 *
 * $RCSfile$
 *
 * $Revision$
 *
 * last change: $Author$ $Date$
 *
 * The Contents of this file are made available subject to the terms of
 * either of the following licenses
 *
 *        - GNU General Public License Version 2.1
 *
 * Patrick Luby, June 2003
 *
 * GNU General Public License Version 2.1
 * =============================================
 * Copyright 2003 Planamesa Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2.1, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307  USA
 *
 ************************************************************************/

#include <list>

#ifdef TODO
#include <osl/mutex.hxx>
#endif	// TODO
#include <sfx2/sfx.hrc>
#include <tools/rcid.h>
#include <vcl/print.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include "jobset.h"
#include "salptype.hxx"
#include "java/salframe.h"
#include "java/salgdi.h"
#include "java/salinst.h"
#include "java/salprn.h"
#include "java/salvd.h"

#include "../app/salinst_cocoa.h"
#include "../../../../sfx2/source/doc/doc.hrc"

static rtl::OUString aPageScalingFactorKey( RTL_CONSTASCII_USTRINGPARAM( "PAGE_SCALING_FACTOR" ) );
static ResMgr *pSfxResMgr = NULL;

using namespace com::sun::star;
using namespace osl;
using namespace rtl;
using namespace vcl;
using namespace vos;

inline float Impl100thMMToPixel( long n ) { return (float)n * 72 / 2540; }

inline long ImplPixelTo100thMM( float n ) { return (long)( n * 2540 / 72 ); }

inline float ImplPrinterToPixel( long n ) { return (float)n * 72 / MIN_PRINTER_RESOLUTION; }

inline long ImplPixelToPrinter( float n ) { return (long)( n * MIN_PRINTER_RESOLUTION / 72 ); }

static XubString GetSfxResString( int nId )
{
	if ( !pSfxResMgr )
	{
		::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();
        pSfxResMgr = ResMgr::SearchCreateResMgr( "sfx", aLocale );
		if ( !pSfxResMgr )
			return OUString();
	}

	ResId aResId( nId, *pSfxResMgr );
	aResId.SetRT( RSC_STRING );
	if ( !pSfxResMgr->IsAvailable( aResId ) )
		return OUString();

	return XubString( ResId( nId, *pSfxResMgr ) );
}

static Paper ImplPrintInfoGetPaperType( NSPrintInfo *pInfo )
{
	Paper nRet = PAPER_USER;

	if ( pInfo )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		// Paper sizes are always portrait dimensions
		NSSize aPaperSize = [pInfo paperSize];
		long nWidth = ImplPixelTo100thMM( aPaperSize.width );
		long nHeight = ImplPixelTo100thMM( aPaperSize.height );

		PaperInfo aPaperInfo( nWidth, nHeight );
		aPaperInfo.doSloppyFit();
		nRet = aPaperInfo.getPaper();

		[pPool release];
	}

	return nRet;
}

static sal_Bool ImplPrintInfoSetPaperType( NSPrintInfo *pInfo, Paper nPaper, Orientation nOrientation, float fWidth, float fHeight )
{
	(void)nOrientation;

	sal_Bool bRet = sal_False;

	if ( nPaper != PAPER_USER )
	{
		PaperInfo aPaperInfo( nPaper );
		fWidth = Impl100thMMToPixel( aPaperInfo.getWidth() );
		fHeight = Impl100thMMToPixel( aPaperInfo.getHeight() );
	}

	if ( pInfo && fWidth > 0 && fHeight > 0 )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		@try
		{
			// When running in the sandbox, native file dialog calls may
			// throw exceptions if the PowerBox daemon process is killed

			// Paper sizes are always portrait dimensions
			[pInfo setOrientation:NSPaperOrientationPortrait];

			NSMutableDictionary *pDictionary = [pInfo dictionary];
			if ( pDictionary )
			{
				NSSize aSize = NSMakeSize( fWidth, fHeight );
				NSValue *pValue = [NSValue valueWithSize:aSize];
				if ( pValue )
				{
					[pDictionary setObject:pValue forKey:NSPrintPaperSize];

					// Fix bugs 543, 1678, 2202, and 2913 by detecting when the
					// paper should be rotated determining the minimum unmatched
					// area
					NSValue *pValue = [pDictionary objectForKey:NSPrintPaperSize];
					if ( pValue )
					{
						aSize = [pValue sizeValue];
						double fDiff = pow( (double)aSize.width - fWidth, 2 ) + pow( (double)aSize.height - fHeight, 2 );
						double fRotatedDiff = pow( (double)aSize.width - fHeight, 2 ) + pow( (double)aSize.height - fWidth, 2 );
						if ( fDiff > fRotatedDiff )
							bRet = sal_True;
					}
				}
			}
		}
		@catch ( NSException *pExc )
		{
			if ( pExc )
				NSLog( @"%@", [pExc callStackSymbols] );
		}

		[pPool release];
	}

	return bRet;
}

static void ImplGetPageInfo( NSPrintInfo *pInfo, const ImplJobSetup* pSetupData, sal_Bool bPaperRotated, long& rOutWidth, long& rOutHeight, long& rPageOffX, long& rPageOffY, long& rPageWidth, long& rPageHeight )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	Size aSize;
	Rectangle aRect;
	if ( pInfo )
	{
		NSSize aPaperSize = [pInfo paperSize];
		NSRect aPageBounds = [pInfo imageablePageBounds];

		// Flip page bounds
		aPageBounds.origin.y = aPaperSize.height - aPageBounds.origin.y - aPageBounds.size.height;

		NSPaperOrientation nOrientation = [pInfo orientation];
		if ( ( bPaperRotated && nOrientation == NSPaperOrientationPortrait ) || ( !bPaperRotated && nOrientation == NSPaperOrientationLandscape ) )
		{
			aSize = Size( ImplPixelToPrinter( aPaperSize.height ), ImplPixelToPrinter( aPaperSize.width ) );
			aRect = Rectangle( Point( ImplPixelToPrinter( aPaperSize.height - aPageBounds.origin.y - aPageBounds.size.height ), ImplPixelToPrinter( aPageBounds.origin.x ) ), Size( ImplPixelToPrinter( aPageBounds.size.height ), ImplPixelToPrinter( aPageBounds.size.width ) ) );
		}
		else
		{
			aSize = Size( ImplPixelToPrinter( aPaperSize.width ), ImplPixelToPrinter( aPaperSize.height ) );
			aRect = Rectangle( Point( ImplPixelToPrinter( aPageBounds.origin.x ), ImplPixelToPrinter( aPaperSize.height - aPageBounds.origin.y - aPageBounds.size.height ) ), Size( ImplPixelToPrinter( aPageBounds.size.width ), ImplPixelToPrinter( aPageBounds.size.height ) ) );
		}
	}

	// Fix bug 2278 by detecting if the OOo code wants rotated bounds
	if ( pSetupData && pSetupData->meOrientation != ORIENTATION_PORTRAIT )
	{
		rPageWidth = aSize.Height();
		rPageHeight = aSize.Width();
		rPageOffX = aRect.nTop;
		rPageOffY = aRect.nLeft;
		rOutWidth = aRect.nBottom - aRect.nTop + 1;
		rOutHeight = aRect.nRight - aRect.nLeft + 1;
	}
	else
	{
		rPageWidth = aSize.Width();
		rPageHeight = aSize.Height();
		rPageOffX = aRect.nLeft;
		rPageOffY = aRect.nTop;
		rOutWidth = aRect.nRight - aRect.nLeft + 1;
		rOutHeight = aRect.nBottom - aRect.nTop + 1;
	}

	[pPool release];
}

@interface JavaSalInfoPrinterCreatePrintInfo : NSObject
{
	NSPrintInfo*			mpInfo;
	NSPrintInfo*			mpSourceInfo;
}
+ (id)createWithPrintInfo:(NSPrintInfo *)pSourceInfo;
- (void)dealloc;
- (id)initWithPrintInfo:(NSPrintInfo *)pSourceInfo;
- (NSPrintInfo *)printInfo;
@end

@implementation JavaSalInfoPrinterCreatePrintInfo

+ (id)createWithPrintInfo:(NSPrintInfo *)pSourceInfo
{
	JavaSalInfoPrinterCreatePrintInfo *pRet = [[JavaSalInfoPrinterCreatePrintInfo alloc] initWithPrintInfo:pSourceInfo];
	[pRet autorelease];
	return pRet;
}

- (void)createPrintInfo:(id)pObject
{
	(void)pObject;

	if ( !mpInfo )
	{
		NSPrintInfo *pInfo = mpSourceInfo;
		if ( !pInfo )
			pInfo = [NSPrintInfo sharedPrintInfo];
		if ( pInfo && [pInfo dictionary] )
		{
			// Fix bug 2573 by not cloning the dictionary as that will cause
			// querying of the printer which, in turn, will cause hanging if
			// the printer is an unavailable network printer
			// Do not retain as invoking alloc disables autorelease
			mpInfo = [[NSPrintInfo alloc] initWithDictionary:[pInfo dictionary]];
			if ( mpInfo )
			{
				// Some users seem to get scaling values other than 100% so
				// force the scaling factor here
				NSNumber *pValue = [NSNumber numberWithFloat:1.0f];
				if ( pValue )
				{
					NSMutableDictionary *pDict = [mpInfo dictionary];
					if ( pDict )
						[pDict setObject:pValue forKey:NSPrintScalingFactor];
				}
			}
		}
	}
}

- (void)dealloc
{
	if ( mpInfo )
		[mpInfo release];

	[super dealloc];
}

- (id)initWithPrintInfo:(NSPrintInfo *)pSourceInfo
{
	[super init];

	mpInfo = nil;
	mpSourceInfo = pSourceInfo;

	return self;
}

- (NSPrintInfo *)printInfo
{
	return mpInfo;
}

@end

@interface JavaSalInfoPrinterShowPageLayoutDialog : NSObject
{
	NSWindow*				mpAttachedSheet;
	BOOL					mbCancelled;
	BOOL					mbFinished;
	NSPrintInfo*			mpInfo;
	BOOL					mbResult;
	NSWindow*				mpWindow;
}
+ (id)createWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow;
- (void)cancel:(id)pObject;
- (void)checkForErrors:(id)pObject;
- (void)dealloc;
- (void)destroy:(id)pObject;
- (BOOL)finished;
- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow;
- (NSPrintInfo *)printInfo;
- (void)pageLayoutDidEnd:(NSPageLayout *)pLayout returnCode:(int)nCode contextInfo:(void *)pContextInfo;
- (void)setResult:(int)nResult pageLayout:(NSPageLayout *)pLayout;
- (void)showPageLayoutDialog:(id)pObject;
@end

@implementation JavaSalInfoPrinterShowPageLayoutDialog

+ (id)createWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow
{
	JavaSalInfoPrinterShowPageLayoutDialog *pRet = [[JavaSalInfoPrinterShowPageLayoutDialog alloc] initWithPrintInfo:pInfo window:pWindow];
	[pRet autorelease];
	return pRet;
}

- (void)cancel:(id)pObject
{
	(void)pObject;

	// Force the panel to end its modal session
	if ( !mbCancelled && !mbFinished && mpAttachedSheet )
	{
		// Prevent crashing by only allowing cancellation to be requested once
		mbCancelled = YES;

		NSWindow *pParent = [mpAttachedSheet sheetParent];
		if ( pParent )
			[pParent endSheet:mpAttachedSheet returnCode:NSModalResponseCancel];
	}
}

- (void)checkForErrors:(id)pObject
{
	(void)pObject;

	// Detect if the sheet window has been closed without any call to the
	// completion handler
	if ( !mbFinished && ( !mpAttachedSheet || !mpWindow || [mpWindow attachedSheet] != mpAttachedSheet ) )
		[self cancel:self];
}

- (void)dealloc
{
	[self destroy:self];

	[super dealloc];
}

- (void)destroy:(id)pObject
{
	(void)pObject;

	if ( !mbFinished )
		[self cancel:self];

	if ( mpAttachedSheet )
	{
		[mpAttachedSheet release];
		mpAttachedSheet = nil;
	}

	if ( mpInfo )
	{
		[mpInfo release];
		mpInfo = nil;
	}

	if ( mpWindow )
	{
		[mpWindow release];
		mpWindow = nil;
	}
}

- (BOOL)finished
{
	return mbFinished;
}

- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow
{
	[super init];

	mpAttachedSheet = nil;
	mbCancelled = NO;
	mbFinished = NO;
	mpInfo = pInfo;
	if ( mpInfo )
		[mpInfo retain];
	mbResult = NO;
	mpWindow = pWindow;
	if ( mpWindow )
		[mpWindow retain];

	return self;
}

- (NSPrintInfo *)printInfo
{
	if ( mbResult )
		return mpInfo;
	else
		return nil;
}

// Never call this selector directly since it releases self
- (void)pageLayoutDidEnd:(NSPageLayout *)pLayout returnCode:(int)nCode contextInfo:(void *)pContextInfo
{
	(void)pContextInfo;

	[self setResult:nCode pageLayout:pLayout];
	[self release];
}

- (void)setResult:(int)nResult pageLayout:(NSPageLayout *)pLayout
{
	if ( mbFinished )
		return;

	if ( nResult == NSModalResponseOK )
		mbResult = YES;
	else
		mbResult = NO;

	mbFinished = YES;

	if ( mpAttachedSheet )
	{
		[mpAttachedSheet release];
		mpAttachedSheet = nil;
	}

	if ( pLayout && mbResult )
	{
		NSPrintInfo *pInfo = nil;

		@try
		{
			// When running in the sandbox, native file dialog calls may
			// throw exceptions if the PowerBox daemon process is killed
			pInfo = [pLayout printInfo];
		}
		@catch ( NSException *pExc )
		{
			mbFinished = YES;
			if ( pExc )
				NSLog( @"%@", [pExc callStackSymbols] );
		}
		
		if ( pInfo && pInfo != mpInfo )
		{
			if ( mpInfo )
				[mpInfo release];
			mpInfo = pInfo;
			[mpInfo retain];
		}
	}

	// Post an event to wakeup the VCL event thread if the VCL
	// event dispatch thread is in a potentially long wait
	Application_postWakeUpEvent();
}

- (void)showPageLayoutDialog:(id)pObject
{
	(void)pObject;

	// Do not allow recursion or reuse
	if ( mpAttachedSheet || mbCancelled || mbFinished || !mpInfo || mbResult )
		return;

	// Fix crashing bug reported in the following NeoOffice forum topic by
	// using a separate NSPrintInfo instance for the modal sheet:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8568
	NSPrintInfo *pInfo = [[NSPrintInfo alloc] initWithDictionary:[mpInfo dictionary]];
	if ( mpInfo )
		[mpInfo release];
	mpInfo = pInfo;
	if ( mpInfo )
		[mpInfo retain];
	else
		return;

	// Don't use sheet if it is an open dialog or there is no window to attach
	// a sheet to
	if ( !mpWindow || [mpWindow attachedSheet] || ![mpWindow canBecomeKeyWindow] || ( ![mpWindow isVisible] && ![mpWindow isMiniaturized] ) )
	{
		if ( mpWindow )
			[mpWindow release];
		mpWindow = nil;
	}

	@try
	{
		// When running in the sandbox, native file dialog calls may
		// throw exceptions if the PowerBox daemon process is killed
		NSPageLayout *pLayout = [NSPageLayout pageLayout];
		if ( pLayout )
		{
			if ( mpWindow )
			{
				NSWindow *pOldAttachedSheet = [mpWindow attachedSheet];

				// Retain self to ensure that we don't release it before the
				// completion handler executes
				[self retain];
				[pLayout beginSheetWithPrintInfo:mpInfo modalForWindow:mpWindow delegate:self didEndSelector:@selector(pageLayoutDidEnd:returnCode:contextInfo:) contextInfo:nil];

				NSWindow *pAttachedSheet = [mpWindow attachedSheet];
				if ( pAttachedSheet && pAttachedSheet != pOldAttachedSheet )
				{
					mpAttachedSheet = pAttachedSheet;
					if ( mpAttachedSheet )
						[mpAttachedSheet retain];
				}
				else
				{
					mbFinished = YES;
				}
			}
			else
			{
				[self setResult:[pLayout runModalWithPrintInfo:mpInfo] pageLayout:pLayout];
			}
		}
	}
	@catch ( NSException *pExc )
	{
		mbFinished = YES;
		if ( pExc )
			NSLog( @"%@", [pExc callStackSymbols] );
	}
}

@end

@class VCLPrintView;

@interface JavaSalPrinterPrintJob : NSObject
{
	BOOL					mbAborted;
	sal_Bool				mbCollate;
	sal_uInt16				mnCopies;
	BOOL					mbFinished;
	BOOL					mbInDealloc;
	NSPrintInfo*			mpInfo;
	JavaSalInfoPrinter*		mpInfoPrinter;
	NSString*				mpJobName;
	sal_Bool				mbMonitorVisible;
	NSUInteger				mnPageCount;
	vcl::PrinterController*	mpPrinterController;
	sal_uInt32				mnPrintJobCounter;
	NSPrintOperation*		mpPrintOperation;
	VCLPrintView*			mpPrintView;
	BOOL					mbResult;
	float					mfScaleFactor;
	::std::map< NSUInteger, ImplJobSetup >*	mpSetupDataMap;
	NSWindow*				mpWindow;
}
- (BOOL)aborted;
- (void)cancel:(id)pObject;
- (void)checkForErrors:(id)pObject;
- (void)dealloc;
- (void)destroy:(id)pObject;
- (void)end:(id)pObject;
- (BOOL)finished;
- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow jobName:(NSString *)pJobName infoPrinter:(JavaSalInfoPrinter *)pInfoPrinter printerController:(vcl::PrinterController *)pPrinterController scaleFactor:(float)fScaleFactor;
- (const ImplJobSetup *)jobSetupForPage:(NSInteger)nPageNumber;
- (NSPrintInfo *)printInfo;
- (void)printOperationDidRun:(NSPrintOperation *)pPrintOperation success:(BOOL)bSuccess contextInfo:(void *)pContextInfo;
- (BOOL)result;
- (BOOL)startNextPrintOperation:(BOOL)bFirstPrintOperation;
- (void)startPrintJob:(id)pObject;
@end

@interface VCLPrintView : NSView
{
	NSUInteger				mnFirstPage;
	NSUInteger				mnLastPagePrinted;
	BOOL					mbNewPrintOperationNeeded;
	NSUInteger				mnPageCount;
	BOOL					mbPaperRotated;
	vcl::PrinterController*	mpPrinterController;
	BOOL					mbPrintingStarted;
	JavaSalPrinterPrintJob*	mpPrintJob;
	NSPrintOperation*		mpPrintOperation;
	BOOL					mbPrintOperationAborted;
	BOOL					mbPrintOperationEnded;
#ifdef TODO
	std::list< JavaSalGraphics* >*	mpUnprintedGraphicsList;
	Condition*				mpUnprintedGraphicsCondition;
	Mutex*					mpUnprintedGraphicsMutex;
#endif	// TODO
}
- (void)abortPrintOperation;
- (void)dealloc;
- (void)drawRect:(NSRect)aRect;
- (void)endPrintOperation;
- (NSUInteger)firstPage;
- (id)initWithFrame:(NSRect)aFrame printJob:(JavaSalPrinterPrintJob *)pPrintJob printerController:(vcl::PrinterController *)pPrinterController pageCount:(NSUInteger)nPageCount lastPagePrinted:(NSUInteger)nLastPagePrinted;
- (BOOL)knowsPageRange:(NSRangePointer)pRange;
- (NSUInteger)lastPagePrinted;
- (NSPoint)locationOfPrintRect:(NSRect)aRect;
- (BOOL)newPrintOperationNeeded;
- (NSRect)rectForPage:(NSInteger)nPageNumber;
- (void)setPrintOperation:(NSPrintOperation *)pPrintOperation;
- (void)updatePaper:(NSInteger)nPageNumber;
@end

@implementation VCLPrintView

- (void)abortPrintOperation
{
	// Don't abort immediately. Instead, abort the print operation before the
	// next page is printed in the rectForPage: selector
	mbPrintOperationAborted = YES;
}

- (void)dealloc
{
	[self destroy:self];

#ifdef TODO
	if ( mpUnprintedGraphicsCondition )
		delete mpUnprintedGraphicsCondition;

	if ( mpUnprintedGraphicsList )
	{
		while ( mpUnprintedGraphicsList->size() )
		{
			delete mpUnprintedGraphicsList->front();
			mpUnprintedGraphicsList->pop_front();
		}

		delete mpUnprintedGraphicsList;
	}

	if ( mpUnprintedGraphicsMutex )
		delete mpUnprintedGraphicsMutex;
#endif	// TODO

	[super dealloc];
}

- (void)destroy:(id)pObject
{
	(void)pObject;

	[self abortPrintOperation];

	mpPrinterController = NULL;

	if ( mpPrintJob )
	{
		[mpPrintJob release];
		mpPrintJob = nil;
	}

	if ( mpPrintOperation )
	{
		[mpPrintOperation release];
		mpPrintOperation = nil;
	}
}

- (void)drawRect:(NSRect)aRect
{
	if ( !mpPrintOperation )
		return;

	NSUInteger nPageNumber = [mpPrintOperation currentPage];
	[self updatePaper:[mpPrintOperation currentPage]];
	if ( mbPrintingStarted && mnLastPagePrinted < nPageNumber )
		mnLastPagePrinted = nPageNumber;

	if ( !mbPrintOperationAborted && !mbPrintOperationEnded && !mbNewPrintOperationNeeded && nPageNumber > 0 && nPageNumber <= mnPageCount )
	{
#ifdef TODO
	if ( mpUnprintedGraphicsList && mpUnprintedGraphicsMutex )
	{
		JavaSalGraphics *pGraphics = NULL;

		mpUnprintedGraphicsMutex->acquire();
		if ( mpUnprintedGraphicsList->size() )
		{
			pGraphics = mpUnprintedGraphicsList->front();
			mpUnprintedGraphicsList->pop_front();
		}
		else
		{
			mbPrintOperationEnded = YES;
		}
		mpUnprintedGraphicsMutex->release();

		if ( pGraphics )
		{
			NSGraphicsContext *pContext = [NSGraphicsContext currentContext];
			if ( pContext )
			{
				// Draw undrawn graphics ops to the print context
				CGContextRef aContext = (CGContextRef)[pContext graphicsPort];
				if ( aContext )
				{
					float fScaleFactor = 1.0f;
					BOOL bFlipped = [self isFlipped];
					NSRect aBounds = [self bounds];
					if ( mpPrintOperation )
					{
						NSPrintInfo *pInfo = [mpPrintOperation printInfo];
						if ( pInfo )
						{
							NSDictionary *pDict = [pInfo dictionary];
							if ( pDict )
							{
								NSNumber *pValue = [pDict objectForKey:NSPrintScalingFactor];
								if ( pValue )
									fScaleFactor = [pValue floatValue];
							}
						}
					}

					CGContextSaveGState( aContext );

					if ( bFlipped )
					{
						CGContextTranslateCTM( aContext, 0, aBounds.size.height );
						CGContextScaleCTM( aContext, 1.0, -1.0f );
						aRect.origin.y = aBounds.origin.y + aBounds.size.height - aRect.origin.y - aRect.size.height;
					}

					// Fix bug reported in the following NeoOffice forum topic
					// by translating using the margins that were set before
					// the native print dialog was displayed:
					// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8468
					aRect.origin.x -= pGraphics->mfPageTranslateX;
					aRect.origin.y += pGraphics->mfPageTranslateY;
					CGContextTranslateCTM( aContext, pGraphics->mfPageTranslateX, pGraphics->mfPageTranslateY * -1 );

					CGContextScaleCTM( aContext, fScaleFactor, fScaleFactor );
					pGraphics->drawUndrawnNativeOps( aContext, NSRectToCGRect( aRect ) );
					CGContextRestoreGState( aContext );
				}
			}

			delete pGraphics;
		}
	}
#endif	// TODO
	}

	// The print operation will set the printer's paper size to the size of the
	// page that is in the print dialog's preview pane so reset to the first
	// page's paper size
	if ( !mbPrintingStarted && [mpPrintOperation showsPrintPanel] )
	{
		NSUInteger nFirstPage = mnFirstPage;
		NSPrintInfo *pInfo = [mpPrintOperation printInfo];
		if ( pInfo )
		{
			NSDictionary *pDict = [pInfo dictionary];
			if ( pDict )
			{
				NSNumber *pAllPages = [pDict objectForKey:NSPrintAllPages];
				if ( !pAllPages || ![pAllPages boolValue] )
				{
					NSNumber *pFirstPage = [pDict objectForKey:NSPrintFirstPage];
					if ( pFirstPage )
					{
						nFirstPage = [pFirstPage unsignedIntegerValue];
						if ( nFirstPage < mnFirstPage )
							nFirstPage = mnFirstPage;
					}
				}
			}
		}

		[self updatePaper:nFirstPage];
	}
}

- (void)endPrintOperation
{
	// Don't end immediately. Instead, end the print operation before the
	// next page is printed in the rectForPage: selector
	mbPrintOperationEnded = YES;
}

- (NSUInteger)firstPage
{
	return mnFirstPage;
}

- (id)initWithFrame:(NSRect)aFrame printJob:(JavaSalPrinterPrintJob *)pPrintJob printerController:(vcl::PrinterController *)pPrinterController pageCount:(NSUInteger)nPageCount lastPagePrinted:(NSUInteger)nLastPagePrinted
{
	[super initWithFrame:aFrame];

	mnFirstPage = nLastPagePrinted + 1;
	mnLastPagePrinted = nLastPagePrinted;
	mbNewPrintOperationNeeded = NO;
	mnPageCount = nPageCount >= mnFirstPage ? nPageCount : mnFirstPage;
	mbPaperRotated = NO;
	mpPrinterController = pPrinterController;
	mbPrintingStarted = NO;
	mpPrintJob = pPrintJob;
	if ( mpPrintJob )
		[mpPrintJob retain];
	mpPrintOperation = nil;
	mbPrintOperationAborted = NO;
	mbPrintOperationEnded = NO;
#ifdef TODO
	mpUnprintedGraphicsCondition = new Condition();
	if ( mpUnprintedGraphicsCondition )
		mpUnprintedGraphicsCondition->set();
	mpUnprintedGraphicsList = new std::list< JavaSalGraphics* >();
	mpUnprintedGraphicsMutex = new Mutex();
#endif	// TODO

	return self;
}

- (BOOL)knowsPageRange:(NSRangePointer)pRange
{
	BOOL bRet = NO;

	if ( pRange )
	{
		// First page and page count should have already been sanity checked
		*pRange = NSMakeRange( mnFirstPage, mnPageCount - mnFirstPage + 1 );
		bRet = YES;
	}

	return bRet;
}

- (NSUInteger)lastPagePrinted
{
	return mnLastPagePrinted;
}

- (NSPoint)locationOfPrintRect:(NSRect)aRect
{
	(void)aRect;

	return NSMakePoint( 0, 0 );
}

- (BOOL)newPrintOperationNeeded
{
	return mbNewPrintOperationNeeded;
}

- (NSRect)rectForPage:(NSInteger)nPageNumber
{
	(void)nPageNumber;

	NSRect aRet = NSZeroRect;

	[self updatePaper:nPageNumber];

	if ( mpPrintOperation )
	{
		NSPrintInfo *pInfo = [mpPrintOperation printInfo];
		if ( pInfo )
		{
			// We cannot force the print operation to abort until printing has
			// started
			if ( mbPrintingStarted && mbPrintOperationAborted )
			{
				[pInfo setJobDisposition:NSPrintCancelJob];
			}
			else if ( !mbPrintOperationEnded && !mbNewPrintOperationNeeded )
			{
				NSSize aPaperSize = [pInfo paperSize];
				aRet = NSMakeRect( 0, 0, aPaperSize.width, aPaperSize.height );
			}
		}
	}

#ifdef TODO
	if ( mpUnprintedGraphicsCondition && mpUnprintedGraphicsList && mpUnprintedGraphicsMutex )
	{
		BOOL bContinue = YES;
		while ( bContinue )
		{
			mpUnprintedGraphicsMutex->acquire();

			// Aborting has higher priority than ending
			if ( mbPrintOperationAborted )
			{
				bContinue = NO;

				while ( mpUnprintedGraphicsList->size() )
				{
					delete mpUnprintedGraphicsList->front();
					mpUnprintedGraphicsList->pop_front();
				}

				// Cancel all print output
				if ( mpPrintOperation )
				{
					NSPrintInfo *pInfo = [mpPrintOperation printInfo];
					if ( pInfo )
						[pInfo setJobDisposition:NSPrintCancelJob];
				}
			}
			else if ( mpUnprintedGraphicsList && mpUnprintedGraphicsList->size() )
			{
				bContinue = NO;

				// Set page orientation and adjust view frame
				if ( mpPrintOperation )
				{
					NSPrintInfo *pInfo = [mpPrintOperation printInfo];
					if ( pInfo )
					{
						JavaSalGraphics *pGraphics = mpUnprintedGraphicsList->front();
						if ( ( pGraphics->mbPaperRotated && pGraphics->meOrientation == ORIENTATION_PORTRAIT ) || ( !pGraphics->mbPaperRotated && pGraphics->meOrientation == ORIENTATION_LANDSCAPE ) )
							[pInfo setOrientation:NSPaperOrientationLandscape];
						else
							[pInfo setOrientation:NSPaperOrientationPortrait];

						NSSize aPaperSize = [pInfo paperSize];
						[self setFrame:NSMakeRect( 0, 0, aPaperSize.width, aPaperSize.height )];
					}
				}

				aRet = [self bounds];
			}
			else if ( mbPrintOperationEnded )
			{
				bContinue = NO;
			}
			else
			{
				// Wait until the JavaSalPrinter instance aborts, ends, or
				// adds another unprinted graphics
				mpUnprintedGraphicsCondition->reset();
				mpUnprintedGraphicsMutex->release();
				mpUnprintedGraphicsCondition->wait();
				mpUnprintedGraphicsMutex->acquire();
			}

			mpUnprintedGraphicsMutex->release();
		}
	}
#endif // TODO

	return aRet;
}

- (void)setPrintOperation:(NSPrintOperation *)pPrintOperation
{
	if ( pPrintOperation && pPrintOperation != mpPrintOperation )
	{
		if ( mpPrintOperation)
			[mpPrintOperation release];
		mpPrintOperation = pPrintOperation;
		[mpPrintOperation retain];
	}
}

- (void)updatePaper:(NSInteger)nPageNumber
{
	BOOL bOldPrintingStarted = mbPrintingStarted;
	if ( !mbPrintingStarted )
	{
		// Check if we are actually spooling to printer
		NSGraphicsContext *pContext = [NSGraphicsContext currentContext];
		if ( pContext )
		{
			NSDictionary *pDict = [pContext attributes];
			if ( pDict )
			{
				NSString *pFormat = [pDict objectForKey:NSGraphicsContextRepresentationFormatAttributeName];
				if ( pFormat && [pFormat isEqualToString:NSGraphicsContextPDFFormat] )
					mbPrintingStarted = YES;
			}
		}
	}

	if ( !mbNewPrintOperationNeeded && mpPrintJob && mpPrintOperation )
	{
		NSPrintInfo *pInfo = [mpPrintOperation printInfo];
		const ImplJobSetup *pSetupData = [mpPrintJob jobSetupForPage:nPageNumber];
		const ImplJobSetup *pPrevSetupData = ( nPageNumber > 0 && nPageNumber > (NSInteger)mnFirstPage ? [mpPrintJob jobSetupForPage:nPageNumber - 1] : NULL );
		if ( pInfo && pSetupData )
		{
			if ( bOldPrintingStarted && pPrevSetupData )
			{
				long nWidth = pSetupData->mnPaperWidth;
				long nHeight = pSetupData->mnPaperHeight;
				long nPrevWidth = pPrevSetupData->mnPaperWidth;
				long nPrevHeight = pPrevSetupData->mnPaperHeight;
				if ( pSetupData->mePaperFormat != PAPER_USER )
				{
					PaperInfo aPaperInfo( pSetupData->mePaperFormat );
					nWidth = aPaperInfo.getWidth();
					nHeight = aPaperInfo.getHeight();
				}
				if ( pPrevSetupData->mePaperFormat != PAPER_USER )
				{
					PaperInfo aPrevPaperInfo( pPrevSetupData->mePaperFormat );
					nPrevWidth = aPrevPaperInfo.getWidth();
					nPrevHeight = aPrevPaperInfo.getHeight();
				}

				if ( ( nWidth != nPrevWidth || nHeight != nPrevHeight ) && ( nHeight != nPrevWidth || nWidth != nPrevHeight ) )
				{
					// Determine if the previous page is below the page range
					// set by the user
					BOOL bPreviousPageInPageRange = YES;
					NSDictionary *pDict = [pInfo dictionary];
					if ( pDict )
					{
						NSNumber *pAllPages = [pDict objectForKey:NSPrintAllPages];
						if ( !pAllPages || ![pAllPages boolValue] )
						{
							NSNumber *pFirstPage = [pDict objectForKey:NSPrintFirstPage];
							if ( pFirstPage )
							{
								NSInteger nFirstPage = [pFirstPage integerValue];
								if ( nFirstPage >= nPageNumber )
									bPreviousPageInPageRange = NO;
							}
						}
					}

					if ( bPreviousPageInPageRange )
						mbNewPrintOperationNeeded = YES;
				}
			}

			if ( !mbNewPrintOperationNeeded )
			{
				mbPaperRotated = ImplPrintInfoSetPaperType( pInfo, pSetupData->mePaperFormat, pSetupData->meOrientation, Impl100thMMToPixel( pSetupData->mnPaperWidth ), Impl100thMMToPixel( pSetupData->mnPaperHeight ) );
				if ( ( mbPaperRotated && pSetupData->meOrientation == ORIENTATION_PORTRAIT ) || ( !mbPaperRotated && pSetupData->meOrientation == ORIENTATION_LANDSCAPE ) )
					[pInfo setOrientation:NSPaperOrientationLandscape];
				else
					[pInfo setOrientation:NSPaperOrientationPortrait];

				// The print operation will set the printer's paper size from
				// the NSConcretePrintOperation._copyingBounds property and the
				// print dialog may set this property directly so ensure that
				// this property matches the print info's paper size
				NSSize aPaperSize = [pInfo paperSize];
				[mpPrintOperation setValue:[NSValue valueWithRect:NSMakeRect( 0, 0, aPaperSize.width, aPaperSize.height )] forKey:@"_copyingBounds"];
			}
		}
	}
}

@end

@implementation JavaSalPrinterPrintJob

- (BOOL)aborted
{
	return mbAborted;
}

- (void)cancel:(id)pObject
{
	(void)pObject;

	if ( mpPrintView )
		[mpPrintView abortPrintOperation];
}

- (void)checkForErrors:(id)pObject
{
	(void)pObject;

	// Detect if the print operation has finished without any call to the
	// completion handler
	if ( !mbFinished && ( mbAborted || !mnPageCount || !mpPrintOperation || mbResult || !mpSetupDataMap || mpPrintOperation != [NSPrintOperation currentOperation] ) )
		[self cancel:self];
}

- (void)dealloc
{
	mbInDealloc = YES;
	[self destroy:self];

	[super dealloc];
}

- (void)destroy:(id)pObject
{
	(void)pObject;

	// Avoid callbacks into the JavaSalPrintJob instance by destroying the
	// print view before destroying any other print objects
	if ( mpPrintView )
	{
		[mpPrintView destroy:self];
		[mpPrintView release];
		mpPrintView = nil;
	}

	if ( mpInfo )
	{
		[mpInfo release];
		mpInfo = nil;
	}

	mpInfoPrinter = NULL;

	if ( mpJobName )
	{
		[mpJobName release];
		mpJobName = nil;
	}

	mpPrinterController = NULL;

	if ( mpPrintOperation )
	{
		// Wait until the print operation's completion handler is called to
		// prevent the print operation from leaking memory or hanging. Avoid
		// blocking if this instance is not in dealloc: by waiting
		// asynchronously for the completion handler to be called.
		while ( [NSPrintOperation currentOperation] == mpPrintOperation )
		{
			if ( mbInDealloc )
			{
				NSApplication_dispatchPendingEvents( sal_False, sal_True );
			}
			else
			{
				[self performSelector:@selector(destroy:) withObject:self afterDelay:0.5f];
				return;
			}
		}

		[mpPrintOperation release];
		mpPrintOperation = nil;
	}

	if ( mpSetupDataMap )
	{
		delete mpSetupDataMap;
		mpSetupDataMap = NULL;
	}

	if ( mpWindow )
	{
		[mpWindow release];
		mpWindow = nil;
	}
}

- (void)end:(id)pObject
{
	(void)pObject;

	if ( mpPrintView )
		[mpPrintView endPrintOperation];
}

- (BOOL)finished
{
	return mbFinished;
}

- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow jobName:(NSString *)pJobName infoPrinter:(JavaSalInfoPrinter *)pInfoPrinter printerController:(vcl::PrinterController *)pPrinterController scaleFactor:(float)fScaleFactor
{
	[super init];

	mbAborted = NO;
	mbCollate = sal_False;
	mnCopies = 1;
	mbFinished = NO;
	mbInDealloc = NO;
	mpInfo = pInfo;
	if ( mpInfo )
		[mpInfo retain];
	mpInfoPrinter = pInfoPrinter;
	mpJobName = pJobName;
	if ( mpJobName )
		[mpJobName retain];
	mbMonitorVisible = sal_True;
	mnPageCount = 0;
	mpPrinterController = pPrinterController;
	mnPrintJobCounter = 0;
	mpPrintOperation = nil;
	mpPrintView = nil;
	mbResult = NO;
	mfScaleFactor = fScaleFactor;
	if ( mfScaleFactor <= 0.0f )
		mfScaleFactor = 1.0f;
	mpSetupDataMap = NULL;
	mpWindow = pWindow;
	if ( mpWindow )
		[mpWindow retain];

	// Cache paper details for each page here as process may be time consuming
	// and this selector is usually called from the OOo event dispatch thread
	// and not from the main thread
	if ( mpInfoPrinter && mpPrinterController )
	{
		IMutex &rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			mbMonitorVisible = mpPrinterController->isShowDialogs();
			if ( mbMonitorVisible )
			{
				beans::PropertyValue* pMonitorVisible = mpPrinterController->getValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "MonitorVisible" ) ) );
				if ( pMonitorVisible )
					pMonitorVisible->Value >>= mbMonitorVisible;
			}

			boost::shared_ptr< Printer > aPrinter( mpPrinterController->getPrinter() );
			if ( aPrinter.get() )
			{
				mbCollate = aPrinter->IsCollateCopy();
				mnCopies = aPrinter->GetCopyCount();
				if ( !mnCopies )
					mnCopies = 1;

				mpSetupDataMap = new ::std::map< NSUInteger, ImplJobSetup >();
				if ( mpSetupDataMap )
				{
					ImplJobSetup aSetupData;
					int nPages = mpPrinterController->getFilteredPageCount();	
					mnPageCount = nPages > 0 ? nPages : 0;
					for ( NSUInteger i = 0; i < mnPageCount; i++ )
					{
						mpPrinterController->getFilteredPageSize( i );

						// Invoking PrinterController::getFilteredPageSize()
						// calls JavaSalInfoPrinter::SetData() so retrieve page
						// info from the info printer. Note: use one-based
						// page numbers to match the native page numbers.
						NSPrintInfo *pInfo = (NSPrintInfo *)mpInfoPrinter->GetPrintInfo();
						if ( pInfo && mpInfoPrinter->SetData( SAL_JOBSET_EXACTPAPERSIZE, &aSetupData ) )
							(*mpSetupDataMap)[ i + 1 ] = aSetupData;
					}
				}
			}
		}
		rSolarMutex.release();
	}

	return self;
}

- (const ImplJobSetup *)jobSetupForPage:(NSInteger)nPageNumber
{
	const ImplJobSetup *pRet = NULL;

	if ( mpSetupDataMap )
	{
		::std::map< NSUInteger, ImplJobSetup >::const_iterator it = mpSetupDataMap->find( nPageNumber );
		if ( it != mpSetupDataMap->end() )
			pRet = &it->second;
	}

	return pRet;
}

- (NSPrintInfo *)printInfo
{
	if ( mbResult )
		return mpInfo;
	else
		return nil;
}

- (void)printOperationDidRun:(NSPrintOperation *)pPrintOperation success:(BOOL)bSuccess contextInfo:(void *)pContextInfo
{
	(void)pContextInfo;

	if ( !mpPrintOperation || pPrintOperation != mpPrintOperation )
		return;

	if ( [NSPrintOperation currentOperation] == pPrintOperation )
		[NSPrintOperation setCurrentOperation:nil];

	if ( mbAborted || mbFinished || mbResult )
		return;

	@try
	{
		// When running in the sandbox, native file dialog calls may
		// throw exceptions if the PowerBox daemon process is killed
		NSPrintInfo *pInfo = [pPrintOperation printInfo];
		if ( pInfo )
		{
			if ( pInfo != mpInfo )
			{
				if ( mpInfo )
					[mpInfo release];
				mpInfo = pInfo;
				[mpInfo retain];
			}

			NSString *pDisposition = [pInfo jobDisposition];
			if ( pDisposition && [pDisposition isEqualToString:NSPrintCancelJob] )
				mbAborted = YES;
		}

		VCLPrintView *pPrintView = (VCLPrintView *)[pPrintOperation view];
		if ( pPrintView )
		{
			if ( [pPrintView isKindOfClass:[VCLPrintView class]] )
				[pPrintView destroy:self];

			if ( pPrintView == mpPrintView )
			{
				NSUInteger nLastPagePrinted = [mpPrintView lastPagePrinted];
				BOOL bNewPrintOperationNeeded = [mpPrintView newPrintOperationNeeded];
				[mpPrintView release];

				// Prepare to run after print operation for the unprinted pages
				if ( !mbAborted && bNewPrintOperationNeeded && nLastPagePrinted < mnPageCount )
					mpPrintView = [[VCLPrintView alloc] initWithFrame:NSMakeRect( 0, 0, 1, 1 ) printJob:self printerController:mpPrinterController pageCount:mnPageCount lastPagePrinted:nLastPagePrinted];
				else
					mpPrintView = nil;

			}
		}
	}
	@catch ( NSException *pExc )
	{
		if ( pExc )
			NSLog( @"%@", [pExc callStackSymbols] );

		mbAborted = YES;
	}

	// If not aborted, start the next print operation
	if ( !mbAborted && [self startNextPrintOperation:NO] )
		return;

	// Don't mark the print job as finished until there are no more views left
	// to print
	mbResult = bSuccess;
	mbFinished = YES;

	// Post an event to wakeup the VCL event thread if the VCL
	// event dispatch thread is in a potentially long wait
	Application_postWakeUpEvent();
}

- (BOOL)result
{
	return mbResult;
}

- (void)runModalPrintOperation
{
	if ( mbAborted || mbFinished || !mpInfoPrinter || !mpPrinterController || !mpPrintOperation || mpPrintOperation != [NSPrintOperation currentOperation] || mbResult )
		return;

	@try
	{
		// When running in the sandbox, native file dialog calls may
		// throw exceptions if the PowerBox daemon process is killed
		[self printOperationDidRun:mpPrintOperation success:[mpPrintOperation runOperation] contextInfo:nil];
	}
	@catch ( NSException *pExc )
	{
		mbAborted = YES;
		mbFinished = YES;
		if ( [NSPrintOperation currentOperation] == mpPrintOperation )
			[NSPrintOperation setCurrentOperation:nil];
		if ( pExc )
			NSLog( @"%@", [pExc callStackSymbols] );
	}
}

- (BOOL)startNextPrintOperation:(BOOL)bFirstPrintOperation
{
	BOOL bRet = NO;

	if ( mbAborted || mbFinished || !mpInfo || !mpPrintView || mbResult || [NSPrintOperation currentOperation] )
		return bRet;

	const ImplJobSetup *pSetupData = [self jobSetupForPage:[mpPrintView firstPage]];
	if ( !pSetupData )
		return bRet;

	// Set the paper size to the first page as the print operation will ignore
	// any paper size changes made after the print operation is created
	ImplPrintInfoSetPaperType( mpInfo, pSetupData->mePaperFormat, pSetupData->meOrientation, Impl100thMMToPixel( pSetupData->mnPaperWidth ), Impl100thMMToPixel( pSetupData->mnPaperHeight ) );

	@try
	{
		// When running in the sandbox, native file dialog calls may
		// throw exceptions if the PowerBox daemon process is killed
		NSMutableDictionary *pDictionary = [mpInfo dictionary];
		if ( pDictionary )
		{
			NSURL *pURL = [pDictionary objectForKey:NSPrintJobSavingURL];
			if ( pURL )
			{
				Application_cacheSecurityScopedURL( pURL );

				// Cache save URL from first print operation
				if ( mnPrintJobCounter == 1 )
				{
					NSString *pJobName = [pURL lastPathComponent];
					if ( pJobName && [pJobName length] )
					{
						pJobName = [pJobName stringByDeletingPathExtension];
						if ( pJobName && [pJobName length] )
						{
							if ( mpJobName )
								[mpJobName release];
							mpJobName = pJobName;
							[mpJobName retain];
						}
					}
				}

				// Remove save URL so that the print job will display a save
				// dialog if the user chose to save to PDF in the print dialog
				[pDictionary removeObjectForKey:NSPrintJobSavingURL];
			}
		}
	}
	@catch ( NSException *pExc )
	{
		if ( pExc )
			NSLog( @"%@", [pExc callStackSymbols] );
	}

	NSPrintOperation *pPrintOperation = [NSPrintOperation printOperationWithView:mpPrintView printInfo:mpInfo];
	if ( pPrintOperation )
	{
		bRet = YES;

		mnPrintJobCounter++;
		if ( mpPrintOperation )
			[mpPrintOperation release];
		mpPrintOperation = pPrintOperation;
		[mpPrintOperation retain];

		@try
		{
			// When running in the sandbox, native file dialog calls may
			// throw exceptions if the PowerBox daemon process is killed

			// Set job name from save URL if there is one
			NSString *pJobName = nil;
			if ( mnPrintJobCounter == 1 )
			{
				if ( mpJobName && [mpJobName length] )
					pJobName = mpJobName;
			}
			else
			{
				if ( mpJobName && [mpJobName length] )
					pJobName = [NSString stringWithFormat:@"%@-%u", mpJobName, mnPrintJobCounter];
				if ( !pJobName )
					pJobName = [NSString stringWithFormat:@"%u", mnPrintJobCounter];
			}
			if ( pJobName )
				[mpPrintOperation setJobTitle:pJobName];

			[mpPrintOperation setShowsPrintPanel:bFirstPrintOperation];
			[mpPrintOperation setShowsProgressPanel:mbMonitorVisible];

			// [NSPrintOperation runOperation] will not spawn a separate
			// thread so disable using a separate thread in all cases
			[mpPrintOperation setCanSpawnSeparateThread:NO];

			[NSPrintOperation setCurrentOperation:mpPrintOperation];

			[mpPrintView setPrintOperation:mpPrintOperation];
			if ( mpWindow )
			{
				[mpPrintOperation runOperationModalForWindow:mpWindow delegate:self didRunSelector:@selector(printOperationDidRun:success:contextInfo:) contextInfo:nil];
			}
			else
			{
				[self performSelector:@selector(runModalPrintOperation) withObject:nil afterDelay:0];
			}
		}
		@catch ( NSException *pExc )
		{
			mbAborted = YES;
			mbFinished = YES;
			if ( [NSPrintOperation currentOperation] == mpPrintOperation )
				[NSPrintOperation setCurrentOperation:nil];
			if ( pExc )
				NSLog( @"%@", [pExc callStackSymbols] );
		}
	}

	return bRet;
}

- (void)startPrintJob:(id)pObject
{
	(void)pObject;

	if ( !mpInfo || !mpInfoPrinter || mnPageCount <= 0 || !mpPrinterController || !mpSetupDataMap )
		return;

	// Do not allow recursion or reuse
	if ( mbAborted || mbFinished || !mpInfo || !mpPrinterController || mpPrintOperation || mpPrintView || mbResult || [NSPrintOperation currentOperation] )
		return;

	// Fix crashing bug reported in the following NeoOffice forum topic by
	// using a separate NSPrintInfo instance for the modal sheet:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8568
	NSPrintInfo *pInfo = [[NSPrintInfo alloc] initWithDictionary:[mpInfo dictionary]];
	if ( mpInfo )
		[mpInfo release];
	mpInfo = pInfo;
	if ( mpInfo )
		[mpInfo retain];
	else
		return;

	// Don't use sheet if it is an open dialog or there is no window to attach
	// a sheet to
	if ( !mpWindow || [mpWindow attachedSheet] || ![mpWindow canBecomeKeyWindow] || ( ![mpWindow isVisible] && ![mpWindow isMiniaturized] ) )
	{
		if ( mpWindow )
			[mpWindow release];
		mpWindow = nil;
	}

	// Fix bug 1548 by setting to print all pages
	NSMutableDictionary *pDictionary = [mpInfo dictionary];
	if ( pDictionary )
	{
		[pDictionary setObject:[NSNumber numberWithBool:YES] forKey:NSPrintAllPages];
		[pDictionary setObject:[NSNumber numberWithBool:mbCollate] forKey:NSPrintMustCollate];
		[pDictionary setObject:[NSNumber numberWithUnsignedShort:mnCopies] forKey:NSPrintCopies];
		[pDictionary removeObjectForKey:NSPrintFirstPage];
		[pDictionary removeObjectForKey:NSPrintLastPage];

		// Fix bug 2030 by resetting the layout
		[pDictionary setObject:[NSNumber numberWithUnsignedInt:1] forKey:@"NSPagesAcross"];
		[pDictionary setObject:[NSNumber numberWithUnsignedInt:1] forKey:@"NSPagesDown"];

		// Set scaling factor
		[pDictionary setObject:[NSNumber numberWithFloat:mfScaleFactor] forKey:NSPrintScalingFactor];
	}

	NSPrinter *pPrinter = [NSPrintInfo defaultPrinter];
	if ( pPrinter )
		[mpInfo setPrinter:pPrinter];

	mpPrintView = [[VCLPrintView alloc] initWithFrame:NSMakeRect( 0, 0, 1, 1 ) printJob:self printerController:mpPrinterController pageCount:mnPageCount lastPagePrinted:0];

	// Start first print operation
	[self startNextPrintOperation:YES];
}

@end

// =======================================================================

JavaSalInfoPrinter::JavaSalInfoPrinter( ImplJobSetup* pSetupData ) :
	mpInfo( nil ),
	mbPaperRotated( sal_False ),
	mpVirDev( new JavaSalVirtualDevice() )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	JavaSalInfoPrinterCreatePrintInfo *pJavaSalInfoPrinterCreatePrintInfo = [JavaSalInfoPrinterCreatePrintInfo createWithPrintInfo:nil];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pJavaSalInfoPrinterCreatePrintInfo performSelectorOnMainThread:@selector(createPrintInfo:) withObject:pJavaSalInfoPrinterCreatePrintInfo waitUntilDone:YES modes:pModes];
	mpInfo = [pJavaSalInfoPrinterCreatePrintInfo printInfo];
	if ( mpInfo )
		[mpInfo retain];

	[pPool release];

	mpVirDev->SetSize( 1, 1 );

	// Set graphics resolution to match printer resolution
	JavaSalGraphics *pGraphics = (JavaSalGraphics *)GetGraphics();
	if ( pGraphics )
	{
		pGraphics->mnDPIX = MIN_PRINTER_RESOLUTION;
		pGraphics->mnDPIY = MIN_PRINTER_RESOLUTION;
		ReleaseGraphics( pGraphics );
	}

	SetData( 0, pSetupData );
}

// -----------------------------------------------------------------------

JavaSalInfoPrinter::~JavaSalInfoPrinter()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpInfo )
		[mpInfo release];

	[pPool release];

	if ( mpVirDev )
		delete mpVirDev;
}

// -----------------------------------------------------------------------

SalGraphics* JavaSalInfoPrinter::GetGraphics()
{
	return mpVirDev->GetGraphics();
}

// -----------------------------------------------------------------------

void JavaSalInfoPrinter::ReleaseGraphics( SalGraphics* pGraphics )
{
	mpVirDev->ReleaseGraphics( pGraphics );
}

// -----------------------------------------------------------------------

sal_Bool JavaSalInfoPrinter::Setup( SalFrame* /* pFrame */, ImplJobSetup* pSetupData )
{
	sal_Bool bRet = sal_False;

	// Display a native page setup dialog
	if ( !mpInfo )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	// Ignore any AWT events while the page layout dialog is showing
	// to emulate a modal dialog
	NSWindow *pNSWindow = nil;
	if ( Application_beginModalSheet( &pNSWindow ) )
	{
		// Don't lock mutex as we expect callbacks to this object from
		// a different thread while the dialog is showing
		sal_uLong nCount = Application::ReleaseSolarMutex();

		JavaSalInfoPrinterShowPageLayoutDialog *pJavaSalInfoPrinterShowPageLayoutDialog = [JavaSalInfoPrinterShowPageLayoutDialog createWithPrintInfo:mpInfo window:pNSWindow];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pJavaSalInfoPrinterShowPageLayoutDialog performSelectorOnMainThread:@selector(showPageLayoutDialog:) withObject:pJavaSalInfoPrinterShowPageLayoutDialog waitUntilDone:YES modes:pModes];
		while ( ![pJavaSalInfoPrinterShowPageLayoutDialog finished] && !Application::IsShutDown() )
		{
			[pJavaSalInfoPrinterShowPageLayoutDialog performSelectorOnMainThread:@selector(checkForErrors:) withObject:pJavaSalInfoPrinterShowPageLayoutDialog waitUntilDone:YES modes:pModes];
			if ( Application::IsShutDown() )
				break;

			IMutex &rSolarMutex = Application::GetSolarMutex();
			rSolarMutex.acquire();
			if ( !Application::IsShutDown() )
				Application::Yield();
			rSolarMutex.release();
		}

		NSPrintInfo *pInfo = [pJavaSalInfoPrinterShowPageLayoutDialog printInfo];
		if ( pInfo )
		{
			if ( pInfo != mpInfo )
			{
				if ( mpInfo )
					[mpInfo release];
				mpInfo = pInfo;
				[mpInfo retain];
			}

			bRet = sal_True;
		}

		[pJavaSalInfoPrinterShowPageLayoutDialog performSelectorOnMainThread:@selector(destroy:) withObject:pJavaSalInfoPrinterShowPageLayoutDialog waitUntilDone:YES modes:pModes];

		Application::AcquireSolarMutex( nCount );
		Application_endModalSheet();
	}

	if ( bRet )
	{
		mbPaperRotated = sal_False;

		// Update values
		SetData( 0, pSetupData );

		// Fix bug 2777 by caching the scaling factor
		float fScaleFactor = 1.0f;
		NSDictionary *pDict = [mpInfo dictionary];
		if ( pDict )
		{
			NSNumber *pValue = [pDict objectForKey:NSPrintScalingFactor];
			if ( pValue )
				fScaleFactor = [pValue floatValue];
		}
		pSetupData->maValueMap[ aPageScalingFactorKey ] = OUString::valueOf( fScaleFactor );
	}

	[pPool release];

	return bRet;
}

// -----------------------------------------------------------------------

sal_Bool JavaSalInfoPrinter::SetPrinterData( ImplJobSetup* pSetupData )
{
	// Clear driver data
	if ( pSetupData->mpDriverData )
	{
		rtl_freeMemory( pSetupData->mpDriverData );
		pSetupData->mpDriverData = NULL;
		pSetupData->mnDriverDataLen = 0;
	}

	// Set but don't update values
	SetData( SAL_JOBSET_ALL, pSetupData );

	return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool JavaSalInfoPrinter::SetData( sal_uLong nFlags, ImplJobSetup* pSetupData )
{
	// Set or update values
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( ! ( nFlags & SAL_JOBSET_ORIENTATION ) )
	{
		NSPaperOrientation nOrientation = ( mpInfo ? [mpInfo orientation] : NSPaperOrientationPortrait );
		if ( ( mbPaperRotated && nOrientation == NSPaperOrientationPortrait ) || ( !mbPaperRotated && nOrientation == NSPaperOrientationLandscape ) )
			pSetupData->meOrientation = ORIENTATION_LANDSCAPE;
		else
			pSetupData->meOrientation = ORIENTATION_PORTRAIT;
	}
	else if ( mpInfo )
	{
		if ( ( mbPaperRotated && pSetupData->meOrientation == ORIENTATION_PORTRAIT ) || ( !mbPaperRotated && pSetupData->meOrientation == ORIENTATION_LANDSCAPE ) )
			[mpInfo setOrientation:NSPaperOrientationLandscape];
		else
			[mpInfo setOrientation:NSPaperOrientationPortrait];
	}

	if ( ! ( nFlags & SAL_JOBSET_PAPERBIN ) )
		pSetupData->mnPaperBin = 0;

	if ( ! ( nFlags & SAL_JOBSET_PAPERSIZE ) )
	{
		if ( nFlags & SAL_JOBSET_EXACTPAPERSIZE )
			pSetupData->mePaperFormat = PAPER_USER;
		else
			pSetupData->mePaperFormat = ImplPrintInfoGetPaperType( mpInfo );
		if ( pSetupData->mePaperFormat == PAPER_USER && mpInfo )
		{
			NSSize aPaperSize = [mpInfo paperSize];
			NSPaperOrientation nOrientation = ( mpInfo ? [mpInfo orientation] : NSPaperOrientationPortrait );
			if ( ( mbPaperRotated && nOrientation == NSPaperOrientationPortrait ) || ( !mbPaperRotated && nOrientation == NSPaperOrientationLandscape ) )
			{
				pSetupData->mnPaperWidth = ImplPixelTo100thMM( aPaperSize.height );
				pSetupData->mnPaperHeight = ImplPixelTo100thMM( aPaperSize.width );
			}
			else
			{
				pSetupData->mnPaperWidth = ImplPixelTo100thMM( aPaperSize.width );
				pSetupData->mnPaperHeight = ImplPixelTo100thMM( aPaperSize.height );
			}
		}
		else
		{
			pSetupData->mnPaperWidth = 0;
			pSetupData->mnPaperHeight = 0;
		}
	}
	else if ( mpInfo )
	{
		mbPaperRotated = ImplPrintInfoSetPaperType( mpInfo, pSetupData->mePaperFormat, pSetupData->meOrientation, Impl100thMMToPixel( pSetupData->mnPaperWidth ), Impl100thMMToPixel( pSetupData->mnPaperHeight ) );
		if ( ( mbPaperRotated && pSetupData->meOrientation == ORIENTATION_PORTRAIT ) || ( !mbPaperRotated && pSetupData->meOrientation == ORIENTATION_LANDSCAPE ) )
			[mpInfo setOrientation:NSPaperOrientationLandscape];
		else
			[mpInfo setOrientation:NSPaperOrientationPortrait];

	}

	[pPool release];

	return sal_True;
}

// -----------------------------------------------------------------------

sal_uLong JavaSalInfoPrinter::GetPaperBinCount( const ImplJobSetup* /* pSetupData */ )
{
	// Return a dummy value
	return 1;
}

// -----------------------------------------------------------------------

XubString JavaSalInfoPrinter::GetPaperBinName( const ImplJobSetup* /* pSetupData */, sal_uLong /* nPaperBin */ )
{
	// Return a dummy value
	return XubString();
}

// -----------------------------------------------------------------------

sal_uLong JavaSalInfoPrinter::GetCapabilities( const ImplJobSetup* /* pSetupData */, sal_uInt16 nType )
{
	sal_uLong nRet = 0;

	switch ( nType )
	{
		case PRINTER_CAPABILITIES_COPIES:
		case PRINTER_CAPABILITIES_COLLATECOPIES:
			nRet = ULONG_MAX;
		case PRINTER_CAPABILITIES_EXTERNALDIALOG:
		case PRINTER_CAPABILITIES_PDF:
		case PRINTER_CAPABILITIES_SETORIENTATION:
		case PRINTER_CAPABILITIES_SETPAPER:
		case PRINTER_CAPABILITIES_SETPAPERSIZE:
		case PRINTER_CAPABILITIES_USEPULLMODEL:
			nRet = 1;
			break;
		default:
			break;
	}

	return nRet;
}

// -----------------------------------------------------------------------

void JavaSalInfoPrinter::GetPageInfo( const ImplJobSetup* pSetupData, long& rOutWidth, long& rOutHeight, long& rPageOffX, long& rPageOffY, long& rPageWidth, long& rPageHeight )
{
	ImplGetPageInfo( mpInfo, pSetupData, mbPaperRotated, rOutWidth, rOutHeight, rPageOffX, rPageOffY, rPageWidth, rPageHeight );
}

// -----------------------------------------------------------------------

void JavaSalInfoPrinter::InitPaperFormats( const ImplJobSetup* /* pSetupData */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalInfoPrinter::InitPaperFormats not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

int JavaSalInfoPrinter::GetLandscapeAngle( const ImplJobSetup* /* pSetupData */ )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalInfoPrinter::GetLandscapeAngle not implemented\n" );
#endif
	return 900;
}

// =======================================================================

JavaSalPrinter::JavaSalPrinter( JavaSalInfoPrinter *pInfoPrinter ) :
	mpGraphics( NULL ),
	mpInfoPrinter( pInfoPrinter ),
	mbGraphics( sal_False ),
	mePaperFormat( PAPER_USER ),
	mnPaperWidth( 0 ),
	mnPaperHeight( 0 ),
	mpInfo( nil ),
	mbPaperRotated( sal_False ),
	mpPrintJob( nil )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	// Create a copy of the info printer's print info to any isolate changes
	// made by the print job
	JavaSalInfoPrinterCreatePrintInfo *pJavaSalInfoPrinterCreatePrintInfo = [JavaSalInfoPrinterCreatePrintInfo createWithPrintInfo:( mpInfoPrinter ? mpInfoPrinter->GetPrintInfo() : nil )];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pJavaSalInfoPrinterCreatePrintInfo performSelectorOnMainThread:@selector(createPrintInfo:) withObject:pJavaSalInfoPrinterCreatePrintInfo waitUntilDone:YES modes:pModes];
	mpInfo = [pJavaSalInfoPrinterCreatePrintInfo printInfo];
	if ( mpInfo )
		[mpInfo retain];

	[pPool release];
}

// -----------------------------------------------------------------------

JavaSalPrinter::~JavaSalPrinter()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpInfo )
		[mpInfo release];

	if ( mpPrintJob )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpPrintJob performSelectorOnMainThread:@selector(destroy:) withObject:mpPrintJob waitUntilDone:YES modes:pModes];
		[mpPrintJob release];
	}

	while ( maSecurityScopeURLList.size() )
	{
		Application_releaseSecurityScopedURL( maSecurityScopeURLList.front() );
		maSecurityScopeURLList.pop_front();
	}

	[pPool release];

	// Delete graphics last as it may be needed by a JavaSalBitmap
	if ( mpGraphics )
		delete mpGraphics;
}

// -----------------------------------------------------------------------

sal_Bool JavaSalPrinter::StartJob( const String* /* pFileName */, const String& /* rJobName */, const String& /* rAppName */, sal_uLong /* nCopies */, bool /* bCollate */, bool /* bDirect */, ImplJobSetup* /* pSetupData */ )
{
	return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool JavaSalPrinter::StartJob( const String* /* pFileName */, const String& rJobName, const String& /* rAppName */, ImplJobSetup* pSetupData, vcl::PrinterController& rController )
{
	sal_Bool bRet = sal_False;
	BOOL bAborted = NO;

	if ( mpInfo && !mpPrintJob )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		// Set paper type
		float fScaleFactor = 1.0f;
		::std::hash_map< OUString, OUString, OUStringHash >::const_iterator it = pSetupData->maValueMap.find( aPageScalingFactorKey );
		if ( it != pSetupData->maValueMap.end() )
			fScaleFactor = it->second.toFloat();

		// Fix bug by detecting when an OOo printer job is being reused for
		// serial print jobs
		String aJobName = rJobName;
		if ( !aJobName.Len() )
			aJobName = GetSfxResString( STR_NONAME );

		// Update print info settings
		UpdatePageInfo( pSetupData );

		// Ignore any AWT events while the page layout dialog is showing
		// to emulate a modal dialog
		NSWindow *pNSWindow = nil;
		if ( Application_beginModalSheet( &pNSWindow ) )
		{
			// Don't lock mutex as we expect callbacks to this object from
			// a different thread while the dialog is showing
			sal_uLong nCount = Application::ReleaseSolarMutex();

			mpPrintJob = [[JavaSalPrinterPrintJob alloc] initWithPrintInfo:mpInfo window:pNSWindow jobName:[NSString stringWithCharacters:aJobName.GetBuffer() length:aJobName.Len()] infoPrinter:mpInfoPrinter printerController:&rController scaleFactor:fScaleFactor];
			if ( mpPrintJob )
			{
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				[mpPrintJob performSelectorOnMainThread:@selector(startPrintJob:) withObject:mpPrintJob waitUntilDone:YES modes:pModes];

				rController.jobStarted();

				while ( ![mpPrintJob finished] && !Application::IsShutDown() )
				{
					[mpPrintJob performSelectorOnMainThread:@selector(checkForErrors:) withObject:mpPrintJob waitUntilDone:YES modes:pModes];
					if ( Application::IsShutDown() )
						break;

					IMutex &rSolarMutex = Application::GetSolarMutex();
					rSolarMutex.acquire();
					if ( !Application::IsShutDown() )
						Application::Yield();
					rSolarMutex.release();
				}

				bRet = [(JavaSalPrinterPrintJob *)mpPrintJob result];
				if ( bRet )
				{
					bAborted = [(JavaSalPrinterPrintJob *)mpPrintJob aborted];

					NSPrintInfo *pInfo = [mpPrintJob printInfo];
					if ( pInfo )
					{
						if ( pInfo != mpInfo )
						{
							if ( mpInfo )
								[mpInfo release];
							mpInfo = pInfo;
							[mpInfo retain];
						}
					}
				}

				[mpPrintJob performSelectorOnMainThread:@selector(destroy:) withObject:mpPrintJob waitUntilDone:YES modes:pModes];
				[mpPrintJob release];
				mpPrintJob = nil;
			}

			Application::AcquireSolarMutex( nCount );
			Application_endModalSheet();
		}

		[pPool release];

		JavaSalEventQueue::setShutdownDisabled( sal_False );
	}

	// Per the comments at the end of the AquaSalInfoPrinter::StartJob() method
	// in vcl/aqua/source/gdi/salprn.cxx, we need to set the controller's last
	// page property and fetch the meta file for the last page. Otherwise,
	// closing a Writer window will cause a crash.
	rController.setLastPage( sal_True );
	GDIMetaFile aPageFile;
	rController.getFilteredPageFile( 0, aPageFile );
	rController.setJobState( bRet && !bAborted ? view::PrintableState_JOB_SPOOLED : view::PrintableState_JOB_ABORTED );

	return bRet;
}

// -----------------------------------------------------------------------

sal_Bool JavaSalPrinter::EndJob()
{
	if ( mpPrintJob )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpPrintJob performSelectorOnMainThread:@selector(end:) withObject:mpPrintJob waitUntilDone:YES modes:pModes];

		[pPool release];
	}


	return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool JavaSalPrinter::AbortJob()
{
	if ( mpPrintJob )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[mpPrintJob performSelectorOnMainThread:@selector(cancel:) withObject:mpPrintJob waitUntilDone:YES modes:pModes];

		[pPool release];
	}

	return sal_True;
}

// -----------------------------------------------------------------------

SalGraphics* JavaSalPrinter::StartPage( ImplJobSetup* pSetupData, sal_Bool bNewJobData )
{
	if ( mbGraphics )
		return NULL;

#ifdef TODO
	// Fix bug 2060 by creating a new print job with the same printer if a
	// change in paper size is requested. Change in orientation does not
	// require a new print job.
	if ( bNewJobData )
	{
		bool bEndJob = false;
		if ( pSetupData->mePaperFormat == PAPER_USER && ( !pSetupData->mnPaperWidth || !pSetupData->mnPaperHeight ) )
		{
			// Fix bug 3660 by ignoring custom paper size with no width or
			// height as that indicates that the OpenOffice.org code is
			// automatically inserting a page
		}
		else if ( pSetupData->mePaperFormat != mePaperFormat )
		{
			bEndJob = true;
		}
		else if ( pSetupData->mePaperFormat == PAPER_USER && ( pSetupData->mnPaperWidth != mnPaperWidth || pSetupData->mnPaperHeight != mnPaperHeight ) )
		{
			bEndJob = true;
		}

		if ( bEndJob )
		{
			EndJob();

			if ( !StartJob( NULL, maJobName, XubString(), 1, sal_True, sal_False, pSetupData ) )
				return NULL;
		}
	}
#endif	// TODO

	// Update print info settings
	UpdatePageInfo( pSetupData );

	mpGraphics = new JavaSalGraphics();
	mpGraphics->meOrientation = pSetupData->meOrientation;
	mpGraphics->mbPaperRotated = mbPaperRotated;
	mpGraphics->mnDPIX = MIN_PRINTER_RESOLUTION;
	mpGraphics->mnDPIY = MIN_PRINTER_RESOLUTION;
	mpGraphics->mpPrinter = this;

	if ( mpInfoPrinter )
	{
		long nOutWidth = 0;
		long nOutHeight = 0;
		long nPageOffX = 0;
		long nPageOffY = 0;
		long nPageWidth = 0;
		long nPageHeight = 0;
		ImplGetPageInfo( mpInfo, pSetupData, mbPaperRotated, nOutWidth, nOutHeight, nPageOffX, nPageOffY, nPageWidth, nPageHeight );

		mpGraphics->mfPageTranslateX = ImplPrinterToPixel( nPageOffX );
		mpGraphics->mfPageTranslateY = ImplPrinterToPixel( nPageOffY );
		mpGraphics->maNativeBounds = CGRectMake( 0, 0, nPageWidth, nPageHeight );
	}

	mbGraphics = sal_True;

	return mpGraphics;
}

// -----------------------------------------------------------------------

sal_Bool JavaSalPrinter::EndPage()
{
	if ( mpGraphics )
	{
		delete mpGraphics;
		mpGraphics = NULL;
	}

	mbGraphics = sal_False;
	return sal_True;
}

// -----------------------------------------------------------------------

sal_uLong JavaSalPrinter::GetErrorCode()
{
	if ( !mpPrintJob )
		return SAL_PRINTER_ERROR_ABORT;
	else
		return 0;
}

// -----------------------------------------------------------------------

void JavaSalPrinter::UpdatePageInfo( const ImplJobSetup* pSetupData )
{
	if ( pSetupData && mpInfo )
	{
		mbPaperRotated = ImplPrintInfoSetPaperType( mpInfo, pSetupData->mePaperFormat, pSetupData->meOrientation, Impl100thMMToPixel( pSetupData->mnPaperWidth ), Impl100thMMToPixel( pSetupData->mnPaperHeight ) );
		if ( ( mbPaperRotated && pSetupData->meOrientation == ORIENTATION_PORTRAIT ) || ( !mbPaperRotated && pSetupData->meOrientation == ORIENTATION_LANDSCAPE ) )
			[mpInfo setOrientation:NSPaperOrientationLandscape];
		else
			[mpInfo setOrientation:NSPaperOrientationPortrait];
		mePaperFormat = pSetupData->mePaperFormat;
		mnPaperWidth = pSetupData->mnPaperWidth;
		mnPaperHeight = pSetupData->mnPaperHeight;
	}
}
