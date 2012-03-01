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

#include <dlfcn.h>
#include <list>

#include <salprn.h>
#include <saldata.hxx>
#include <salframe.h>
#include <salgdi.h>
#include <osl/mutex.hxx>
#include <sfx2/sfx.hrc>
#include <tools/rcid.h>
#include <vcl/jobset.h>
#include <vcl/salptype.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <com/sun/star/vcl/VCLFrame.hxx>
#include <com/sun/star/vcl/VCLGraphics.hxx>
#include <com/sun/star/vcl/VCLPageFormat.hxx>
#ifndef USE_NATIVE_PRINTING
#include <com/sun/star/vcl/VCLPrintJob.hxx>
#endif	// !USE_NATIVE_PRINTING

#ifdef USE_NATIVE_PRINTING

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

typedef OSStatus PMSetJobNameCFString_Type( PMPrintSettings aSettings, CFStringRef aName );

#endif	// USE_NATIVE_PRINTING

static rtl::OUString aPageScalingFactorKey( RTL_CONSTASCII_USTRINGPARAM( "PAGE_SCALING_FACTOR" ) );
static ResMgr *pSfxResMgr = NULL;

using namespace osl;
using namespace rtl;
using namespace vcl;

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

#ifdef USE_NATIVE_PRINTING

Paper ImplPrintInfoGetPaperType( NSPrintInfo *pInfo, sal_Bool bPaperRotated )
{
	Paper nRet = PAPER_USER;

	if ( pInfo )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		// Paper sizes are always portrait dimensions
		NSSize aPaperSize = [pInfo paperSize];
		NSPrintingOrientation nOrientation = [pInfo orientation];
		if ( ( bPaperRotated && nOrientation == NSPortraitOrientation ) || nOrientation == NSLandscapeOrientation )
			aPaperSize = NSMakeSize( aPaperSize.height, aPaperSize.width );

        if (fabs(aPaperSize.width - 842) < 2 && fabs(aPaperSize.height - 1191) < 2)
            return PAPER_A3;
        else if (fabs(aPaperSize.width - 595) < 2 && fabs(aPaperSize.height - 842) < 2)
            return PAPER_A4;
        else if (fabs(aPaperSize.width - 420) < 2 && fabs(aPaperSize.height - 595) < 2)
            return PAPER_A5;
        else if (fabs(aPaperSize.width - 709) < 2 && fabs(aPaperSize.height - 1001) < 2)
            return PAPER_B4;
        else if (fabs(aPaperSize.width - 499) < 2 && fabs(aPaperSize.height - 709) < 2)
            return PAPER_B5;
        else if (fabs(aPaperSize.width - 612) < 2 && fabs(aPaperSize.height - 792) < 2)
            return PAPER_LETTER;
        else if (fabs(aPaperSize.width - 612) < 2 && fabs(aPaperSize.height - 1008) < 2)
            return PAPER_LEGAL;
        else if (fabs(aPaperSize.width - 792) < 2 && fabs(aPaperSize.height - 1224) < 2)
            return PAPER_TABLOID;

		[pPool release];
	}

	return nRet;
}

sal_Bool ImplPrintInfoSetPaperType( NSPrintInfo *pInfo, Paper nPaper, float fWidth, float fHeight )
{
	sal_Bool bRet = sal_False;

	if ( nPaper == PAPER_A3 )
	{
		fWidth = 842;
		fHeight = 1191;
	}
	else if ( nPaper == PAPER_A4 )
	{
		fWidth = 595;
		fHeight = 842;
	}
	else if ( nPaper == PAPER_A5 )
	{
		fWidth = 420;
		fHeight = 595;
	}
	else if ( nPaper == PAPER_B4 )
	{
		fWidth = 709;
		fHeight = 1001;
	}
	else if ( nPaper == PAPER_B5 )
	{
		fWidth = 499;
		fHeight = 709;
	}
	else if ( nPaper == PAPER_LETTER )
	{
		fWidth = 612;
		fHeight = 792;
	}
	else if ( nPaper == PAPER_LEGAL )
	{
		fWidth = 612;
		fHeight = 1008;
	}
	else if ( nPaper == PAPER_TABLOID )
	{
		fWidth = 792;
		fHeight = 1224;
	}

	if ( pInfo && fWidth > 0 && fHeight > 0 )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		// Paper sizes are always portrait dimensions
		[pInfo setOrientation:NSPortraitOrientation];

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
				pValue = (NSValue *)[pDictionary objectForKey:NSPrintPaperSize];
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

		[pPool release];
	}

	return bRet;
}

static void SAL_CALL ImplPrintOperationRun( void *pJavaSalPrinter )
{
	if ( pJavaSalPrinter )
		((JavaSalPrinter *)pJavaSalPrinter)->RunPrintOperation();
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
	if ( !mpInfo )
	{
		NSPrintInfo *pInfo = mpSourceInfo;
		if ( !pInfo )
			pInfo = [NSPrintInfo sharedPrintInfo];
		if ( pInfo )
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
					[[mpInfo dictionary] setObject:pValue forKey:NSPrintScalingFactor];
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
	MacOSBOOL				mbFinished;
	NSPrintInfo*			mpInfo;
	MacOSBOOL				mbResult;
	NSWindow*				mpWindow;
}
+ (id)createWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow;
- (MacOSBOOL)finished;
- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow;
- (void)pageLayoutDidEnd:(NSPageLayout *)pLayout returnCode:(int)nCode contextInfo:(void *)pContextInfo;
- (MacOSBOOL)result;
- (void)showPageLayoutDialog:(id)pObject;
@end

@implementation JavaSalInfoPrinterShowPageLayoutDialog

+ (id)createWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow
{
	JavaSalInfoPrinterShowPageLayoutDialog *pRet = [[JavaSalInfoPrinterShowPageLayoutDialog alloc] initWithPrintInfo:pInfo window:pWindow];
	[pRet autorelease];
	return pRet;
}

- (MacOSBOOL)finished
{
	return mbFinished;
}

- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow
{
	[super init];

	mbFinished = NO;
	mpInfo = pInfo;
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

- (MacOSBOOL)result
{
	return mbResult;
}

- (void)showPageLayoutDialog:(id)pObject
{
	if ( mbFinished )
		return;

	NSPageLayout *pLayout = [NSPageLayout pageLayout];
	if ( pLayout )
		[pLayout beginSheetWithPrintInfo:mpInfo modalForWindow:mpWindow delegate:self didEndSelector:@selector(pageLayoutDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

@end

@interface VCLPrintView : NSView
{
	MacOSBOOL				mbPrintOperationAborted;
	MacOSBOOL				mbPrintOperationEnded;
	std::list< JavaSalGraphics* >*	mpUnprintedGraphicsList;
	Condition*				mpUnprintedGraphicsCondition;
	Mutex*					mpUnprintedGraphicsMutex;
}
- (void)abortPrintOperation;
- (MacOSBOOL)addUnprintedGraphics:(JavaSalGraphics *)pGraphics;
- (void)dealloc;
- (void)drawRect:(NSRect)aRect;
- (void)endPrintOperation;
- (id)initWithFrame:(NSRect)aFrame;
- (MacOSBOOL)isFlipped;
- (MacOSBOOL)knowsPageRange:(NSRangePointer)pRange;
- (NSPoint)locationOfPrintRect:(NSRect)aRect;
- (NSRect)rectForPage:(NSInteger)nPageNumber;
@end

@implementation VCLPrintView

- (void)abortPrintOperation
{
	if ( mpUnprintedGraphicsCondition && mpUnprintedGraphicsMutex )
	{
		mpUnprintedGraphicsMutex->acquire();
		mbPrintOperationAborted = YES;
		mpUnprintedGraphicsCondition->set();
		mpUnprintedGraphicsMutex->release();
	}
}

- (MacOSBOOL)addUnprintedGraphics:(JavaSalGraphics *)pGraphics
{
	MacOSBOOL bRet = NO;

	if ( pGraphics && mpUnprintedGraphicsCondition && mpUnprintedGraphicsList && mpUnprintedGraphicsMutex )
	{
		mpUnprintedGraphicsMutex->acquire();
		mpUnprintedGraphicsList->push_back( pGraphics );
		mpUnprintedGraphicsCondition->set();
		mpUnprintedGraphicsMutex->release();
		bRet = YES;
	}

	return bRet;
}

- (void)dealloc
{
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

	[super dealloc];
}

- (void)drawRect:(NSRect)aRect
{
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
					NSRect aPageBounds = NSZeroRect;
					NSPrintOperation *pPrintOperation = [NSPrintOperation currentOperation];
					if ( pPrintOperation )
					{
						NSPrintInfo *pInfo = [pPrintOperation printInfo];
						if ( pInfo )
						{
							NSNumber *pValue = [[pInfo dictionary] objectForKey:NSPrintScalingFactor];
							if ( pValue )
								fScaleFactor = [pValue floatValue];

							NSSize aPaperSize = [pInfo paperSize];
							aPageBounds = [pInfo imageablePageBounds];

							// Flip page bounds
							aPageBounds.origin.y = aPaperSize.height - aPageBounds.origin.y - aPageBounds.size.height;
						}
					}

					CGContextSaveGState( aContext );
					CGContextTranslateCTM( aContext, aPageBounds.origin.x, aPageBounds.origin.y );
					CGContextScaleCTM( aContext, fScaleFactor, fScaleFactor );
					pGraphics->drawUndrawnNativeOps( aContext );
					CGContextRestoreGState( aContext );
				}
			}

			delete pGraphics;
		}
	}
}

- (void)endPrintOperation
{
	if ( mpUnprintedGraphicsCondition && mpUnprintedGraphicsMutex )
	{
		mpUnprintedGraphicsMutex->acquire();
		mbPrintOperationEnded = YES;
		mpUnprintedGraphicsCondition->set();
		mpUnprintedGraphicsMutex->release();
	}
}

- (id)initWithFrame:(NSRect)aFrame
{
	[super initWithFrame:aFrame];

	mbPrintOperationAborted = NO;
	mbPrintOperationEnded = NO;
	mpUnprintedGraphicsCondition = new Condition();
	if ( mpUnprintedGraphicsCondition )
		mpUnprintedGraphicsCondition->set();
	mpUnprintedGraphicsList = new std::list< JavaSalGraphics* >();
	mpUnprintedGraphicsMutex = new Mutex();

	return self;
}

- (MacOSBOOL)isFlipped
{
	return YES;
}

- (MacOSBOOL)knowsPageRange:(NSRangePointer)pRange
{
	MacOSBOOL bRet = NO;

	if ( pRange )
	{
		pRange->location = 1;
		pRange->length = NSIntegerMax;
		bRet = YES;
	}

	return bRet;
}

- (NSPoint)locationOfPrintRect:(NSRect)aRect
{
	return NSMakePoint( 0, 0 );
}

- (NSRect)rectForPage:(NSInteger)nPageNumber
{
	NSRect aRet = NSZeroRect;

	if ( mpUnprintedGraphicsCondition && mpUnprintedGraphicsList && mpUnprintedGraphicsMutex )
	{
		MacOSBOOL bContinue = YES;
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
				NSPrintOperation *pPrintOperation = [NSPrintOperation currentOperation];
				if ( pPrintOperation )
				{
					NSPrintInfo *pInfo = [pPrintOperation printInfo];
					if ( pInfo )
						[pInfo setJobDisposition:NSPrintCancelJob];
				}
			}
			else if ( mpUnprintedGraphicsList && mpUnprintedGraphicsList->size() )
			{
				bContinue = NO;

				// Set page orientation and adjust view frame
				NSPrintOperation *pPrintOperation = [NSPrintOperation currentOperation];
				if ( pPrintOperation )
				{
					NSPrintInfo *pInfo = [pPrintOperation printInfo];
					if ( pInfo )
					{
						JavaSalGraphics *pGraphics = mpUnprintedGraphicsList->front();
						if ( ( pGraphics->mbPaperRotated && pGraphics->meOrientation == ORIENTATION_PORTRAIT ) || pGraphics->meOrientation == ORIENTATION_LANDSCAPE )
							[pInfo setOrientation:NSLandscapeOrientation];
						else
							[pInfo setOrientation:NSPortraitOrientation];

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

	return aRet;
}

@end
 
@interface JavaSalPrinterShowPrintDialog : NSObject
{
	MacOSBOOL				mbFinished;
	NSPrintInfo*			mpInfo;
	NSString*				mpJobName;
	MacOSBOOL				mbResult;
	NSWindow*				mpWindow;
}
+ (id)createWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow jobName:(NSString *)pJobName;
- (MacOSBOOL)finished;
- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow jobName:(NSString *)pJobName;
- (NSPrintInfo *)printInfo;
- (void)printPanelDidEnd:(NSPrintPanel *)pPanel returnCode:(NSInteger)nCode contextInfo:(void *)pContextInfo;
- (void)setJobTitle;
- (void)showPrintDialog:(id)pObject;
@end

@implementation JavaSalPrinterShowPrintDialog

+ (id)createWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow jobName:(NSString *)pJobName
{
	JavaSalPrinterShowPrintDialog *pRet = [[JavaSalPrinterShowPrintDialog alloc] initWithPrintInfo:pInfo window:pWindow jobName:pJobName];
	[pRet autorelease];
	return pRet;
}

- (MacOSBOOL)finished
{
	return mbFinished;
}

- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow jobName:(NSString *)pJobName
{
	[super init];

	mbFinished = NO;
	mpInfo = pInfo;
	mpJobName = pJobName;
	mbResult = NO;
	mpWindow = pWindow;

	return self;
}

- (NSPrintInfo *)printInfo
{
	if ( mbResult )
		return mpInfo;
	else
		return nil;
}

- (void)printPanelDidEnd:(NSPrintPanel *)pPanel returnCode:(NSInteger)nCode contextInfo:(void *)pContextInfo
{
	mbFinished = YES;

	if ( nCode == NSOKButton )
		mbResult = YES;
	else
		mbResult = NO;
}

- (void)setJobTitle
{
	// Also set the job name via the PMPrintSettings so that the Save As
	// dialog gets the job name
	void *pLib = dlopen( NULL, RTLD_LAZY | RTLD_LOCAL );
	if ( pLib )
	{
		PMSetJobNameCFString_Type *pPMSetJobNameCFString = (PMSetJobNameCFString_Type *)dlsym( pLib, "PMSetJobNameCFString" );
		if ( pPMSetJobNameCFString )
		{
			PMPrintSettings aSettings = (PMPrintSettings)[mpInfo PMPrintSettings];
			if ( aSettings )
				pPMSetJobNameCFString( aSettings, (CFStringRef)mpJobName );
		}

		dlclose( pLib );
	}
}

- (void)showPrintDialog:(id)pObject
{
	if ( mbFinished )
		return;

	// Set job title
	[self setJobTitle];

	NSPrintPanel *pPanel = [NSPrintPanel printPanel];
	if ( pPanel )
	{
		// Fix bug 1548 by setting to print all pages
		NSMutableDictionary *pDictionary = [mpInfo dictionary];
		if ( pDictionary )
		{
			[pDictionary setObject:[NSNumber numberWithBool:YES] forKey:NSPrintAllPages];
			[pDictionary setObject:[NSNumber numberWithBool:YES] forKey:NSPrintMustCollate];
			[pDictionary removeObjectForKey:NSPrintCopies];
			[pDictionary removeObjectForKey:NSPrintFirstPage];
			[pDictionary removeObjectForKey:NSPrintLastPage];

			// Fix bug 2030 by resetting the layout
			[pDictionary setObject:[NSNumber numberWithUnsignedInt:1] forKey:@"NSPagesAcross"];
			[pDictionary setObject:[NSNumber numberWithUnsignedInt:1] forKey:@"NSPagesDown"];
		}

		NSPrinter *pPrinter = [NSPrintInfo defaultPrinter];
		if ( pPrinter )
			[mpInfo setPrinter:pPrinter];

		// Fix bug 3614 by displaying a modal print dialog if there is no
		// window to attach a sheet to
		if ( !mpWindow )
		{
			NSView *pView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 1, 1)];
			if ( pView )
			{
				[pView autorelease];

				NSPrintOperation *pOperation = [NSPrintOperation printOperationWithView:pView printInfo:mpInfo];
				if ( pOperation )
				{
					[pOperation setJobTitle:(NSString *)mpJobName];
					[pOperation setShowsPrintPanel:NO];
					[pOperation setShowsProgressPanel:NO];

					NSPrintOperation *pOldOperation = [NSPrintOperation currentOperation];
					[NSPrintOperation setCurrentOperation:pOperation];

					NSInteger nButton = [pPanel runModalWithPrintInfo:mpInfo];
					[self printPanelDidEnd:pPanel returnCode:nButton contextInfo:nil];
					[pOperation cleanUpOperation];
					[NSPrintOperation setCurrentOperation:pOldOperation];
				}
			}
		}
		else
		{
			[pPanel beginSheetWithPrintInfo:mpInfo modalForWindow:mpWindow delegate:self didEndSelector:@selector(printPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
		}
	}
}

@end

#endif	// USE_NATIVE_PRINTING

// =======================================================================

JavaSalInfoPrinter::JavaSalInfoPrinter( ImplJobSetup* pSetupData ) :
	mpGraphics( new JavaSalGraphics() ),
	mbGraphics( FALSE ),
#ifdef USE_NATIVE_PRINTING
	mpInfo( nil ),
	mbPaperRotated( sal_False )
#else	// USE_NATIVE_PRINTING
	mpVCLPageFormat( new com_sun_star_vcl_VCLPageFormat() )
#endif	// USE_NATIVE_PRINTING
{
#ifdef USE_NATIVE_PRINTING
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	JavaSalInfoPrinterCreatePrintInfo *pJavaSalInfoPrinterCreatePrintInfo = [JavaSalInfoPrinterCreatePrintInfo createWithPrintInfo:nil];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pJavaSalInfoPrinterCreatePrintInfo performSelectorOnMainThread:@selector(createPrintInfo:) withObject:pJavaSalInfoPrinterCreatePrintInfo waitUntilDone:YES modes:pModes];
	mpInfo = [pJavaSalInfoPrinterCreatePrintInfo printInfo];
	if ( mpInfo )
		[mpInfo retain];

	[pPool release];
#endif	// USE_NATIVE_PRINTING

	SetData( 0, pSetupData );
}

// -----------------------------------------------------------------------

JavaSalInfoPrinter::~JavaSalInfoPrinter()
{
	if ( mpGraphics )
		delete mpGraphics;

#ifdef USE_NATIVE_PRINTING
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpInfo )
		[mpInfo release];

	[pPool release];
#else	// USE_NATIVE_PRINTING
	if ( mpVCLPageFormat )
		delete mpVCLPageFormat;
#endif	// USE_NATIVE_PRINTING
}

// -----------------------------------------------------------------------

SalGraphics* JavaSalInfoPrinter::GetGraphics()
{
	if ( mbGraphics )
		return NULL;

#ifdef USE_NATIVE_PRINTING
	mpGraphics->mpInfoPrinter = this;
#else	// USE_NATIVE_PRINTING
	mpGraphics->mpVCLGraphics = mpVCLPageFormat->getGraphics();
#endif	// USE_NATIVE_PRINTING
	mbGraphics = TRUE;

	return mpGraphics;
}

// -----------------------------------------------------------------------

void JavaSalInfoPrinter::ReleaseGraphics( SalGraphics* pGraphics )
{
	if ( pGraphics != mpGraphics )
		return;

	if ( mpGraphics && mpGraphics->mpVCLGraphics )
	{
		delete mpGraphics->mpVCLGraphics;
		mpGraphics->mpVCLGraphics = NULL;
	}
	mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL JavaSalInfoPrinter::Setup( SalFrame* pFrame, ImplJobSetup* pSetupData )
{
	BOOL bRet = FALSE;

	// Display a native page setup dialog
#ifdef USE_NATIVE_PRINTING
	if ( !mpInfo )
		return bRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	SalData *pSalData = GetSalData();
	JavaSalFrame *pFocusFrame = NULL;

	// Get the active document window
	Window *pWindow = Application::GetActiveTopWindow();
	if ( pWindow )
		pFocusFrame = (JavaSalFrame *)pWindow->ImplGetFrame();

	if ( !pFocusFrame )
		pFocusFrame = pSalData->mpFocusFrame;

	// Fix bug 3294 by not attaching to utility windows
	while ( pFocusFrame && ( pFocusFrame->IsFloatingFrame() || pFocusFrame->IsUtilityWindow() ) )
		pFocusFrame = pFocusFrame->mpParent;

	// Fix bug 1106 If the focus frame is not set or is not visible, find
	// the first visible non-floating, non-utility frame
	if ( !pFocusFrame || !pFocusFrame->mbVisible )
	{
		pFocusFrame = NULL;
		for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
		{
			if ( (*it)->mbVisible && !(*it)->IsFloatingFrame() && !(*it)->IsUtilityWindow() )
			{
				pFocusFrame = *it;
				break;
			}
		}
	}

	if ( pFocusFrame )
	{
		pSalData->mpNativeModalSheetFrame = pFocusFrame;
		pSalData->mbInNativeModalSheet = true;

		// Ignore any AWT events while the page layout dialog is showing to
		// emulate a modal dialog
		JavaSalInfoPrinterShowPageLayoutDialog *pJavaSalInfoPrinterShowPageLayoutDialog = [JavaSalInfoPrinterShowPageLayoutDialog createWithPrintInfo:mpInfo window:(NSWindow *)pFocusFrame->mpVCLFrame->getNativeWindow()];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pJavaSalInfoPrinterShowPageLayoutDialog performSelectorOnMainThread:@selector(showPageLayoutDialog:) withObject:pJavaSalInfoPrinterShowPageLayoutDialog waitUntilDone:YES modes:pModes];
		while ( ![pJavaSalInfoPrinterShowPageLayoutDialog finished] )
			Application::Yield();
		pSalData->mbInNativeModalSheet = false;
		pSalData->mpNativeModalSheetFrame = NULL;
		bRet = (BOOL)[pJavaSalInfoPrinterShowPageLayoutDialog result];
	}
#else	// USE_NATIVE_PRINTING
	bRet = mpVCLPageFormat->setup();
#endif	// USE_NATIVE_PRINTING
	if ( bRet )
	{
#ifdef USE_NATIVE_PRINTING
		mbPaperRotated = sal_False;
#endif	// USE_NATIVE_PRINTING

		// Update values
		SetData( 0, pSetupData );

		// Fix bug 2777 by caching the scaling factor
#ifdef USE_NATIVE_PRINTING
		float fScaleFactor = 1.0f;
		NSNumber *pValue = [[mpInfo dictionary] objectForKey:NSPrintScalingFactor];
		if ( pValue )
			fScaleFactor = [pValue floatValue];
		pSetupData->maValueMap[ aPageScalingFactorKey ] = OUString::valueOf( fScaleFactor );
#else	// USE_NATIVE_PRINTING
		pSetupData->maValueMap[ aPageScalingFactorKey ] = OUString::valueOf( mpVCLPageFormat->getScaleFactor() );
#endif	// USE_NATIVE_PRINTING
	}

#ifdef USE_NATIVE_PRINTING
	[pPool release];
#endif	// USE_NATIVE_PRINTING

	return bRet;
}

// -----------------------------------------------------------------------

BOOL JavaSalInfoPrinter::SetPrinterData( ImplJobSetup* pSetupData )
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

	return TRUE;
}

// -----------------------------------------------------------------------

BOOL JavaSalInfoPrinter::SetData( ULONG nFlags, ImplJobSetup* pSetupData )
{
	// Set or update values
#ifdef USE_NATIVE_PRINTING
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( ! ( nFlags & SAL_JOBSET_ORIENTATION ) )
	{
		NSPrintingOrientation nOrientation = ( mpInfo ? [mpInfo orientation] : NSPortraitOrientation );
		if ( mbPaperRotated )
			pSetupData->meOrientation = ( nOrientation == NSLandscapeOrientation ? ORIENTATION_PORTRAIT : ORIENTATION_LANDSCAPE );
		else
			pSetupData->meOrientation = ( nOrientation == NSLandscapeOrientation ? ORIENTATION_LANDSCAPE : ORIENTATION_PORTRAIT );
	}
	else if ( mpInfo )
	{
		if ( mbPaperRotated )
			[mpInfo setOrientation:( pSetupData->meOrientation == ORIENTATION_LANDSCAPE ? NSPortraitOrientation : NSLandscapeOrientation )];
		else
			[mpInfo setOrientation:( pSetupData->meOrientation == ORIENTATION_LANDSCAPE ? NSLandscapeOrientation : NSPortraitOrientation )];
	}
#else	// USE_NATIVE_PRINTING
	if ( ! ( nFlags & SAL_JOBSET_ORIENTATION ) )
		pSetupData->meOrientation = mpVCLPageFormat->getOrientation();
	else
		mpVCLPageFormat->setOrientation( pSetupData->meOrientation );
#endif	// USE_NATIVE_PRINTING

	if ( ! ( nFlags & SAL_JOBSET_PAPERBIN ) )
		pSetupData->mnPaperBin = 0;

	if ( ! ( nFlags & SAL_JOBSET_PAPERSIZE ) )
	{
#ifdef USE_NATIVE_PRINTING
		pSetupData->mePaperFormat = ImplPrintInfoGetPaperType( mpInfo, mbPaperRotated );
		if ( pSetupData->mePaperFormat == PAPER_USER && mpInfo )
		{
			NSSize aPaperSize = [mpInfo paperSize];
			if ( mbPaperRotated )
			{
				pSetupData->mnPaperWidth = (long)( aPaperSize.height * 2540 / MIN_PRINTER_RESOLUTION );
				pSetupData->mnPaperHeight = (long)( aPaperSize.width * 2540 / MIN_PRINTER_RESOLUTION );
			}
			else
			{
				pSetupData->mnPaperWidth = (long)( aPaperSize.width * 2540 / MIN_PRINTER_RESOLUTION );
				pSetupData->mnPaperHeight = (long)( aPaperSize.height * 2540 / MIN_PRINTER_RESOLUTION );
			}
		}
#else	// USE_NATIVE_PRINTING
		pSetupData->mePaperFormat = mpVCLPageFormat->getPaperType();
		if ( pSetupData->mePaperFormat == PAPER_USER )
		{
			Size aSize( mpVCLPageFormat->getPageSize() );
			Size aResolution( mpVCLPageFormat->getResolution() );
			pSetupData->mnPaperWidth = aSize.Width() * 2540 / aResolution.Width();
			pSetupData->mnPaperHeight = aSize.Height() * 2540 / aResolution.Height();
		}
#endif	// USE_NATIVE_PRINTING
		else
		{
			pSetupData->mnPaperWidth = 0;
			pSetupData->mnPaperHeight = 0;
		}
	}
#ifdef USE_NATIVE_PRINTING
	else if ( mpInfo )
	{
		mbPaperRotated = ImplPrintInfoSetPaperType( mpInfo, pSetupData->mePaperFormat, (float)pSetupData->mnPaperWidth * 72 / 2540, (float)pSetupData->mnPaperHeight * 72 / 2540 );
		if ( mbPaperRotated )
			[mpInfo setOrientation:( pSetupData->meOrientation == ORIENTATION_LANDSCAPE ? NSPortraitOrientation : NSLandscapeOrientation )];
		else
			[mpInfo setOrientation:( pSetupData->meOrientation == ORIENTATION_LANDSCAPE ? NSLandscapeOrientation : NSPortraitOrientation )];
	}
#else	// USE_NATIVE_PRINTING
	else
	{
		mpVCLPageFormat->setOrientation( pSetupData->meOrientation );
		mpVCLPageFormat->setPaperType( pSetupData->mePaperFormat, pSetupData->mnPaperWidth * 72 / 2540, pSetupData->mnPaperHeight * 72 / 2540 );
	}
#endif	// USE_NATIVE_PRINTING

#ifdef USE_NATIVE_PRINTING
	[pPool release];
#endif	// USE_NATIVE_PRINTING

	return TRUE;
}

// -----------------------------------------------------------------------

ULONG JavaSalInfoPrinter::GetPaperBinCount( const ImplJobSetup* pSetupData )
{
	// Return a dummy value
	return 1;
}

// -----------------------------------------------------------------------

XubString JavaSalInfoPrinter::GetPaperBinName( const ImplJobSetup* pSetupData, ULONG nPaperBin )
{
	// Return a dummy value
	return XubString();
}

// -----------------------------------------------------------------------

ULONG JavaSalInfoPrinter::GetCapabilities( const ImplJobSetup* pSetupData, USHORT nType )
{
	ULONG nRet = 0;

	switch ( nType )
	{
		case PRINTER_CAPABILITIES_SETORIENTATION:
		case PRINTER_CAPABILITIES_SETPAPER:
		case PRINTER_CAPABILITIES_SETPAPERSIZE:
			nRet = 1;
			break;
		default:
			break;
	}

	return nRet;
}

// -----------------------------------------------------------------------

void JavaSalInfoPrinter::GetPageInfo( const ImplJobSetup* pSetupData,
								  long& rOutWidth, long& rOutHeight,
								  long& rPageOffX, long& rPageOffY,
								  long& rPageWidth, long& rPageHeight )
{
#ifdef USE_NATIVE_PRINTING
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	Size aSize;
	Rectangle aRect;
	if ( mpInfo )
	{
		NSSize aPaperSize = [mpInfo paperSize];
		NSRect aPageBounds = [mpInfo imageablePageBounds];

		// Flip page bounds
		aPageBounds.origin.y = aPaperSize.height - aPageBounds.origin.y - aPageBounds.size.height;

		if ( mbPaperRotated )
		{
			aSize = Size( (long)( aPaperSize.height * MIN_PRINTER_RESOLUTION / 72 ), (long)( aPaperSize.width * MIN_PRINTER_RESOLUTION / 72 ) );
			aRect = Rectangle( Point( (long)( aPageBounds.origin.y * MIN_PRINTER_RESOLUTION / 72  ), (long)( aPageBounds.origin.x * MIN_PRINTER_RESOLUTION / 72  ) ), Size( (long)( aPageBounds.size.height * MIN_PRINTER_RESOLUTION / 72 ), (long)( aPageBounds.size.width * MIN_PRINTER_RESOLUTION / 72 ) ) );
		}
		else
		{
			aSize = Size( (long)( aPaperSize.width * MIN_PRINTER_RESOLUTION / 72 ), (long)( aPaperSize.height * MIN_PRINTER_RESOLUTION / 72 ) );
			aRect = Rectangle( Point( (long)( aPageBounds.origin.x * MIN_PRINTER_RESOLUTION / 72  ), (long)( aPageBounds.origin.y * MIN_PRINTER_RESOLUTION / 72  ) ), Size( (long)( aPageBounds.size.width * MIN_PRINTER_RESOLUTION / 72 ), (long)( aPageBounds.size.height * MIN_PRINTER_RESOLUTION / 72 ) ) );
		}
	}
#else	// USE_NATIVE_PRINTING
	Size aSize( mpVCLPageFormat->getPageSize() );
	Rectangle aRect( mpVCLPageFormat->getImageableBounds() );
#endif	// USE_NATIVE_PRINTING

	// Fix bug 2278 by detecting if the OOo code wants rotated bounds
	if ( pSetupData->meOrientation != ORIENTATION_PORTRAIT )
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

#ifdef USE_NATIVE_PRINTING
	[pPool release];
#endif	// USE_NATIVE_PRINTING
}

// -----------------------------------------------------------------------

void JavaSalInfoPrinter::InitPaperFormats( const ImplJobSetup* pSetupData )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalInfoPrinter::InitPaperFormats not implemented\n" );
#endif
}

// -----------------------------------------------------------------------

int JavaSalInfoPrinter::GetLandscapeAngle( const ImplJobSetup* pSetupData )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalInfoPrinter::GetLandscapeAngle not implemented\n" );
#endif
	return 900;
}

// -----------------------------------------------------------------------

DuplexMode JavaSalInfoPrinter::GetDuplexMode( const ImplJobSetup* pJobSetup )
{
#ifdef DEBUG
	fprintf( stderr, "JavaSalInfoPrinter::GetDuplexMode not implemented\n" );
#endif
	return DUPLEX_UNKNOWN;
}

// =======================================================================

JavaSalPrinter::JavaSalPrinter( JavaSalInfoPrinter *pInfoPrinter ) :
	mpGraphics( NULL ),
	mbGraphics( FALSE ),
	mePaperFormat( PAPER_USER ),
	mnPaperWidth( 0 ),
	mnPaperHeight( 0 ),
#ifdef USE_NATIVE_PRINTING
	mpInfo( nil ),
	mbPaperRotated( sal_False ),
	mpPrintOperation( nil ),
	maPrintThread( NULL ),
	mpPrintView( nil )
#else	// USE_NATIVE_PRINTING
	mbStarted( FALSE ),
	mpVCLPageFormat( NULL ),
	mpVCLPrintJob( new com_sun_star_vcl_VCLPrintJob() )
#endif	// USE_NATIVE_PRINTING
{
#ifdef USE_NATIVE_PRINTING
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	// Create a copy of the info printer's print info to any isolate changes
	// made by the print job
	JavaSalInfoPrinterCreatePrintInfo *pJavaSalInfoPrinterCreatePrintInfo = [JavaSalInfoPrinterCreatePrintInfo createWithPrintInfo:pInfoPrinter->GetPrintInfo()];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pJavaSalInfoPrinterCreatePrintInfo performSelectorOnMainThread:@selector(createPrintInfo:) withObject:pJavaSalInfoPrinterCreatePrintInfo waitUntilDone:YES modes:pModes];
	mpInfo = [pJavaSalInfoPrinterCreatePrintInfo printInfo];
	if ( mpInfo )
		[mpInfo retain];

	[pPool release];
#else	// USE_NATIVE_PRINTING
	const com_sun_star_vcl_VCLPageFormat *pVCLPageFormat = pInfoPrinter->GetVCLPageFormat();
	if ( pVCLPageFormat )
		mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat( pVCLPageFormat->getJavaObject() );
	else
		mpVCLPageFormat = new com_sun_star_vcl_VCLPageFormat();
#endif	// USE_NATIVE_PRINTING
}

// -----------------------------------------------------------------------

JavaSalPrinter::~JavaSalPrinter()
{
	// Call EndJob() to join and destroy the print thread
	EndJob();

	if ( mpGraphics )
		delete mpGraphics;

#ifdef USE_NATIVE_PRINTING
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpInfo )
		[mpInfo release];
	if ( mpPrintOperation )
		[mpPrintOperation release];
	if ( mpPrintView )
		[mpPrintView release];

	[pPool release];
#else	// USE_NATIVE_PRINTING
	if ( mpVCLPageFormat )
		delete mpVCLPageFormat;
	if ( mpVCLPrintJob )
	{
		mpVCLPrintJob->dispose();
		delete mpVCLPrintJob;
	}
#endif	// USE_NATIVE_PRINTING
}

// -----------------------------------------------------------------------

BOOL JavaSalPrinter::StartJob( const XubString* pFileName,
							   const XubString& rJobName,
							   const XubString& rAppName,
							   ULONG nCopies, BOOL bCollate,
							   ImplJobSetup* pSetupData, BOOL bFirstPass )
{
	// Set paper type
#ifdef USE_NATIVE_PRINTING
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( !mpInfo )
		return FALSE;
#endif	// USE_NATIVE_PRINTING

#ifdef USE_NATIVE_PRINTING
	if ( !bFirstPass )
	{
		mbPaperRotated = ImplPrintInfoSetPaperType( mpInfo, pSetupData->mePaperFormat, (float)pSetupData->mnPaperWidth * 72 / 2540, (float)pSetupData->mnPaperHeight * 72 / 2540 );
		if ( mbPaperRotated )
			[mpInfo setOrientation:( pSetupData->meOrientation == ORIENTATION_LANDSCAPE ? NSPortraitOrientation : NSLandscapeOrientation )];
		else
			[mpInfo setOrientation:( pSetupData->meOrientation == ORIENTATION_LANDSCAPE ? NSLandscapeOrientation : NSPortraitOrientation )];
		mePaperFormat = pSetupData->mePaperFormat;
		mnPaperWidth = pSetupData->mnPaperWidth;
		mnPaperHeight = pSetupData->mnPaperHeight;
	}
#else	// USE_NATIVE_PRINTING
	if ( !bFirstPass )
	{
		mpVCLPageFormat->setOrientation( pSetupData->meOrientation );
		mePaperFormat = pSetupData->mePaperFormat;
		mnPaperWidth = pSetupData->mnPaperWidth;
		mnPaperHeight = pSetupData->mnPaperHeight;
		mpVCLPageFormat->setPaperType( pSetupData->mePaperFormat, pSetupData->mnPaperWidth * 72 / 2540, pSetupData->mnPaperHeight * 72 / 2540 );
	}
#endif	// USE_NATIVE_PRINTING

	float fScaleFactor = 1.0f;
	::std::hash_map< OUString, OUString, OUStringHash >::const_iterator it = pSetupData->maValueMap.find( aPageScalingFactorKey );
	if ( it != pSetupData->maValueMap.end() )
	{
		fScaleFactor = it->second.toFloat();
		if ( fScaleFactor <= 0.0f )
			fScaleFactor = 1.0f;
	}

	// Fix bug by detecting when an OOo printer job is being reused for serial
	// print jobs
	if ( rJobName.Len() )
		maJobName = XubString( rJobName );
	else if ( !maJobName.Len() )
		maJobName = GetSfxResString( STR_NONAME );

#ifdef USE_NATIVE_PRINTING
	if ( !mpPrintOperation )
	{
		if ( mpPrintView )
		{
			[mpPrintView release];
			mpPrintView = nil;
		}

		// Set scaling factor
		NSNumber *pValue = [NSNumber numberWithFloat:fScaleFactor];
		if ( pValue )
			[[mpInfo dictionary] setObject:pValue forKey:NSPrintScalingFactor];

		SalData *pSalData = GetSalData();
		JavaSalFrame *pFocusFrame = NULL;

		// Get the active document window
		Window *pWindow = Application::GetActiveTopWindow();
		if ( pWindow )
			pFocusFrame = (JavaSalFrame *)pWindow->ImplGetFrame();

		if ( !pFocusFrame )
			pFocusFrame = pSalData->mpFocusFrame;

		// Fix bug 3294 by not attaching to utility windows
		while ( pFocusFrame && ( pFocusFrame->IsFloatingFrame() || pFocusFrame->IsUtilityWindow() || pFocusFrame->mbShowOnlyMenus ) )
			pFocusFrame = pFocusFrame->mpParent;

		// Fix bug 1106 If the focus frame is not set or is not visible,
		// find the first visible non-floating, non-utility frame
		if ( !pFocusFrame || !pFocusFrame->mbVisible || pFocusFrame->IsFloatingFrame() || pFocusFrame->IsUtilityWindow() || pFocusFrame->mbShowOnlyMenus )
		{
			pFocusFrame = NULL;
			for ( ::std::list< JavaSalFrame* >::const_iterator it = pSalData->maFrameList.begin(); it != pSalData->maFrameList.end(); ++it )
			{
				if ( (*it)->mbVisible && !(*it)->IsFloatingFrame() && !(*it)->IsUtilityWindow() && !(*it)->mbShowOnlyMenus )
				{
					pFocusFrame = *it;
					break;
				}
			}
		}

		// Ignore any AWT events while the print dialog is showing to
		// emulate a modal dialog
		pSalData->mpNativeModalSheetFrame = pFocusFrame;
		pSalData->mbInNativeModalSheet = true;

		NSWindow *pNSWindow = ( pFocusFrame ? (NSWindow *)pFocusFrame->mpVCLFrame->getNativeWindow() : NULL );
		NSString *pJobName = [NSString stringWithCharacters:maJobName.GetBuffer() length:maJobName.Len()];

		JavaSalPrinterShowPrintDialog *pJavaSalPrinterShowPrintDialog = [JavaSalPrinterShowPrintDialog createWithPrintInfo:mpInfo window:pNSWindow jobName:pJobName];
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		[pJavaSalPrinterShowPrintDialog performSelectorOnMainThread:@selector(showPrintDialog:) withObject:pJavaSalPrinterShowPrintDialog waitUntilDone:YES modes:pModes];

		while ( ![pJavaSalPrinterShowPrintDialog finished] )
			Application::Yield();
		pSalData->mbInNativeModalSheet = false;
		pSalData->mpNativeModalSheetFrame = NULL;
		NSPrintInfo *pInfo = [pJavaSalPrinterShowPrintDialog printInfo];
		if ( pInfo )
		{
			NSSize aPaperSize = [pInfo paperSize];

			// Do not retain as invoking alloc disables autorelease
			mpPrintView = [[VCLPrintView alloc] initWithFrame:NSMakeRect( 0, 0, aPaperSize.width, aPaperSize.height )];
			if ( mpPrintView )
			{
				mpPrintOperation = [NSPrintOperation printOperationWithView:mpPrintView printInfo:pInfo];
				if ( mpPrintOperation )
				{
					[mpPrintOperation retain];
					[mpPrintOperation setJobTitle:pJobName];
					[mpPrintOperation setShowsPrintPanel:NO];
					[mpPrintOperation setShowsProgressPanel:NO];
				}
			}
		}
	}

	[pPool release];

	return ( mpPrintOperation ? TRUE : FALSE );
#else	// USE_NATIVE_PRINTING
	mbStarted = mpVCLPrintJob->startJob( mpVCLPageFormat, OUString( rJobName ), fScaleFactor, !bFirstPass ? sal_True : mbStarted );

	return mbStarted;
#endif	// USE_NATIVE_PRINTING
}

// -----------------------------------------------------------------------

BOOL JavaSalPrinter::EndJob()
{
#ifdef USE_NATIVE_PRINTING
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( maPrintThread )
	{
		TimeValue aDelay;
		aDelay.Seconds = 0;
		aDelay.Nanosec = 50;
		MacOSBOOL bMainEventLoop = ( CFRunLoopGetCurrent() == CFRunLoopGetMain() );
		while ( osl_isThreadRunning( maPrintThread ) )
		{
			// Invoke on each pass to ensure that the print view's wait
			// condition is unblocked
			if ( mpPrintView )
				[mpPrintView endPrintOperation];

			osl_waitThread( &aDelay );
			if ( bMainEventLoop )
				CFRunLoopRunInMode( CFSTR( "AWTRunLoopMode" ), 0, false );
		}

		osl_destroyThread( maPrintThread );
		maPrintThread = NULL;
	}

	[pPool release];
#else	// USE_NATIVE_PRINTING
	mpVCLPrintJob->endJob();
	mbStarted = FALSE;
#endif	// USE_NATIVE_PRINTING

	GetSalData()->mpEventQueue->setShutdownDisabled( sal_False );
	return TRUE;
}

// -----------------------------------------------------------------------

BOOL JavaSalPrinter::AbortJob()
{
#ifdef USE_NATIVE_PRINTING
	if ( mpPrintView )
		[mpPrintView abortPrintOperation];
	EndJob();
#else	// USE_NATIVE_PRINTING
	mpVCLPrintJob->abortJob();
#endif	// USE_NATIVE_PRINTING
	return TRUE;
}

// -----------------------------------------------------------------------

SalGraphics* JavaSalPrinter::StartPage( ImplJobSetup* pSetupData, BOOL bNewJobData )
{
	if ( mbGraphics )
		return NULL;

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

#ifdef USE_NATIVE_PRINTING
			NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

			if ( mpPrintView )
			{
				[mpPrintView release];
				mpPrintView = nil;
			}

			// Create print operation with same print info
			if ( mpPrintOperation )
			{
				NSPrintOperation *pNewPrintOperation = nil;
				NSPrintInfo *pInfo = [mpPrintOperation printInfo];
				if ( pInfo )
				{
					NSSize aPaperSize = [pInfo paperSize];

					// Do not retain as invoking alloc disables autorelease
					mpPrintView = [[VCLPrintView alloc] initWithFrame:NSMakeRect( 0, 0, aPaperSize.width, aPaperSize.height )];
					if ( mpPrintView )
					{
						[mpPrintView autorelease];

						pNewPrintOperation = [NSPrintOperation printOperationWithView:mpPrintView printInfo:pInfo];
						if ( pNewPrintOperation )
						{
							[pNewPrintOperation retain];
							[pNewPrintOperation setJobTitle:[mpPrintOperation jobTitle]];
							[pNewPrintOperation setShowsPrintPanel:NO];
							[pNewPrintOperation setShowsProgressPanel:NO];
						}
					}
				}

				[mpPrintOperation release];
				mpPrintOperation = pNewPrintOperation;
				if ( mpPrintOperation )
					[mpPrintOperation retain];
			}

			[pPool release];
#else	// USE_NATIVE_PRINTING
			delete mpVCLPrintJob;
			mpVCLPrintJob = new com_sun_star_vcl_VCLPrintJob();
#endif	// USE_NATIVE_PRINTING
			if ( !StartJob( NULL, maJobName, XubString(), 1, TRUE, pSetupData, FALSE ) )
				return NULL;
		}
	}

#ifdef USE_NATIVE_PRINTING
	if ( !maPrintThread )
	{
		// Do not retain as invoking alloc disables autorelease
		maPrintThread = osl_createSuspendedThread( ImplPrintOperationRun, this );
		if ( maPrintThread )
			osl_resumeThread( maPrintThread );
	}
#else	// USE_NATIVE_PRINTING
	com_sun_star_vcl_VCLGraphics *pVCLGraphics = mpVCLPrintJob->startPage( pSetupData->meOrientation );
	if ( !pVCLGraphics )
		return NULL;
#endif	// USE_NATIVE_PRINTING

	// The OOo code does not call Printer::EndJob() if the page range is
	// results in no pages to print so do not disable shutdown until a page
	// is successfully started
	GetSalData()->mpEventQueue->setShutdownDisabled( sal_True );

	mpGraphics = new JavaSalGraphics();
#ifdef USE_NATIVE_PRINTING
	mpGraphics->meOrientation = pSetupData->meOrientation;
	mpGraphics->mbPaperRotated = mbPaperRotated;
#else	// USE_NATIVE_PRINTING
	mpGraphics->mpVCLGraphics = pVCLGraphics;
#endif	// !USE_NATIVE_PRINTING
	mpGraphics->mpPrinter = this;
	mbGraphics = TRUE;

	return mpGraphics;
}

// -----------------------------------------------------------------------

BOOL JavaSalPrinter::EndPage()
{
	if ( mpGraphics )
	{
#ifdef USE_NATIVE_PRINTING
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		// Give ownership of graphics to print view
		if ( !mpPrintView || ![mpPrintView addUnprintedGraphics:mpGraphics] )
			delete mpGraphics;

		[pPool release];
#else	// USE_NATIVE_PRINTING
		if ( mpGraphics->mpVCLGraphics )
			delete mpGraphics->mpVCLGraphics;
		delete mpGraphics;
#endif	// USE_NATIVE_PRINTING
	}
	mpGraphics = NULL;
	mbGraphics = FALSE;
#ifndef USE_NATIVE_PRINTING
	mpVCLPrintJob->endPage();
#endif	// !USE_NATIVE_PRINTING
	return TRUE;
}

// -----------------------------------------------------------------------

ULONG JavaSalPrinter::GetErrorCode()
{
#ifdef USE_NATIVE_PRINTING
	if ( !mpPrintOperation || !maPrintThread || !osl_isThreadRunning( maPrintThread ) )
#else	// USE_NATIVE_PRINTING
	if ( !mbStarted || mpVCLPrintJob->isFinished() )
#endif	// USE_NATIVE_PRINTING
		return SAL_PRINTER_ERROR_ABORT;
	else
		return 0;
}

// -----------------------------------------------------------------------

XubString JavaSalPrinter::GetPageRange()
{
#ifdef USE_NATIVE_PRINTING
	XubString aRet;

	if ( mpPrintOperation )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSPrintInfo *pInfo = [mpPrintOperation printInfo];
		if ( pInfo )
		{
			NSMutableDictionary *pDictionary = [pInfo dictionary];
			if ( pDictionary )
			{
				NSNumber *pNumber = [pDictionary objectForKey:NSPrintAllPages];
				if ( !pNumber || ![pNumber boolValue] )
				{
					NSNumber *pFirst = [pDictionary objectForKey:NSPrintFirstPage];
					NSNumber *pLast = [pDictionary objectForKey:NSPrintLastPage];
					if ( pFirst && pLast )
					{
						int nFirst = [pFirst intValue];
						int nLast = [pLast intValue];
						if ( nFirst > 0 )
						{
							if ( nLast < nFirst )
								nLast = nFirst;

							aRet = XubString::CreateFromInt32( nFirst );
							aRet += '-';
							aRet += XubString::CreateFromInt32( nLast );
						}
					}
				}
			}
		}

		[pPool release];
	}

	return aRet;
#else	// USE_NATIVE_PRINTING
	return mpVCLPrintJob->getPageRange( mpVCLPageFormat );
#endif	// USE_NATIVE_PRINTING
}

#ifdef USE_NATIVE_PRINTING

void JavaSalPrinter::RunPrintOperation()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpPrintOperation )
	{
		[mpPrintOperation runOperation];
		[mpPrintOperation cleanUpOperation];
	}

	[pPool release];
}

#endif	// USE_NATIVE_PRINTING
