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

#include <salprn.h>
#include <salframe.h>
#include <salgdi.h>
#include <salinst.h>
#include <salvd.h>
#include <osl/mutex.hxx>
#include <sfx2/sfx.hrc>
#include <tools/rcid.h>
#include <vcl/jobset.h>
#include <vcl/salptype.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include "../app/salinst_cocoa.h"
#include "../../../../sfx2/source/doc/doc.hrc"

static rtl::OUString aPageScalingFactorKey( RTL_CONSTASCII_USTRINGPARAM( "PAGE_SCALING_FACTOR" ) );
static ResMgr *pSfxResMgr = NULL;

using namespace osl;
using namespace rtl;
using namespace vcl;
using namespace vos;

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

Paper ImplPrintInfoGetPaperType( NSPrintInfo *pInfo, sal_Bool bPaperRotated )
{
	Paper nRet = PAPER_USER;

	if ( pInfo )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		// Paper sizes are always portrait dimensions
		NSSize aPaperSize = [pInfo paperSize];
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

sal_Bool ImplPrintInfoSetPaperType( NSPrintInfo *pInfo, Paper nPaper, Orientation nOrientation, float fWidth, float fHeight )
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
	MacOSBOOL				mbCancelled;
	MacOSBOOL				mbFinished;
	NSPrintInfo*			mpInfo;
	MacOSBOOL				mbResult;
	NSWindow*				mpWindow;
}
+ (id)createWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow;
- (void)cancel:(id)pObject;
- (void)checkForErrors:(id)pObject;
- (void)dealloc;
- (void)destroy:(id)pObject;
- (MacOSBOOL)finished;
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
	// Force the panel to end its modal session
	if ( !mbCancelled && !mbFinished && mpAttachedSheet )
	{
		// Prevent crashing by only allowing cancellation to be requested once
		mbCancelled = YES;

		NSApplication *pApp = [NSApplication sharedApplication];
		if ( pApp )
			[pApp endSheet:mpAttachedSheet returnCode:NSCancelButton];
	}
}

- (void)checkForErrors:(id)pObject
{
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

- (MacOSBOOL)finished
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
	[self setResult:nCode pageLayout:pLayout];
	[self release];
}

- (void)setResult:(int)nResult pageLayout:(NSPageLayout *)pLayout
{
	if ( mbFinished )
		return;

	if ( nResult == NSOKButton )
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
					MacOSBOOL bFlipped = [self isFlipped];
					NSRect aBounds = [self bounds];
					NSPrintOperation *pPrintOperation = [NSPrintOperation currentOperation];
					if ( pPrintOperation )
					{
						NSPrintInfo *pInfo = [pPrintOperation printInfo];
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
						if ( ( pGraphics->mbPaperRotated && pGraphics->meOrientation == ORIENTATION_PORTRAIT ) || ( !pGraphics->mbPaperRotated && pGraphics->meOrientation == ORIENTATION_LANDSCAPE ) )
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
	NSWindow*				mpAttachedSheet;
	MacOSBOOL				mbCancelled;
	MacOSBOOL				mbFinished;
	NSPrintInfo*			mpInfo;
	NSString*				mpJobName;
	MacOSBOOL				mbResult;
	NSWindow*				mpWindow;
}
+ (id)createWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow jobName:(NSString *)pJobName;
- (void)cancel:(id)pObject;
- (void)checkForErrors:(id)pObject;
- (void)dealloc;
- (void)destroy:(id)pObject;
- (MacOSBOOL)finished;
- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow jobName:(NSString *)pJobName;
- (NSPrintInfo *)printInfo;
- (void)printPanelDidEnd:(NSPrintPanel *)pPanel returnCode:(NSInteger)nCode contextInfo:(void *)pContextInfo;
- (void)setResult:(int)nResult printPanel:(NSPrintPanel *)pPanel;
- (void)showPrintDialog:(id)pObject;
@end

@implementation JavaSalPrinterShowPrintDialog

+ (id)createWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow jobName:(NSString *)pJobName
{
	JavaSalPrinterShowPrintDialog *pRet = [[JavaSalPrinterShowPrintDialog alloc] initWithPrintInfo:pInfo window:pWindow jobName:pJobName];
	[pRet autorelease];
	return pRet;
}

- (void)cancel:(id)pObject
{
	// Force the panel to end its modal session
	if ( !mbCancelled && !mbFinished && mpAttachedSheet )
	{
		// Prevent crashing by only allowing cancellation to be requested once
		mbCancelled = YES;

		NSApplication *pApp = [NSApplication sharedApplication];
		if ( pApp )
			[pApp endSheet:mpAttachedSheet returnCode:NSCancelButton];
	}
}

- (void)checkForErrors:(id)pObject
{
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

- (MacOSBOOL)finished
{
	return mbFinished;
}

- (id)initWithPrintInfo:(NSPrintInfo *)pInfo window:(NSWindow *)pWindow jobName:(NSString *)pJobName
{
	[super init];

	mpAttachedSheet = nil;
	mbCancelled = NO;
	mbFinished = NO;
	mpInfo = pInfo;
	if ( mpInfo )
		[mpInfo retain];
	mpJobName = pJobName;
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
- (void)printPanelDidEnd:(NSPrintPanel *)pPanel returnCode:(NSInteger)nCode contextInfo:(void *)pContextInfo
{
	[self setResult:nCode printPanel:pPanel];
	[self release];
}

- (void)setResult:(int)nResult printPanel:(NSPrintPanel *)pPanel
{
	if ( mbFinished )
		return;

	if ( nResult == NSOKButton )
		mbResult = YES;
	else
		mbResult = NO;

	mbFinished = YES;

	if ( mpAttachedSheet )
	{
		[mpAttachedSheet release];
		mpAttachedSheet = nil;
	}

	if ( pPanel && mbResult )
	{
		NSPrintInfo *pInfo = nil;

		@try
		{
			// When running in the sandbox, native file dialog calls may
			// throw exceptions if the PowerBox daemon process is killed
			pInfo = [pPanel printInfo];
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

- (void)showPrintDialog:(id)pObject
{
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

	// Also set the job name via the PMPrintSettings so that the Save As
	// dialog gets the job name
	PMPrintSettings aSettings = (PMPrintSettings)[mpInfo PMPrintSettings];
	if ( aSettings )
		PMPrintSettingsSetJobName( aSettings, (CFStringRef)mpJobName );

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

	@try
	{
		// When running in the sandbox, native file dialog calls may
		// throw exceptions if the PowerBox daemon process is killed
		NSPrintPanel *pPanel = [NSPrintPanel printPanel];
		if ( pPanel )
		{
			if ( mpWindow )
			{
				NSWindow *pOldAttachedSheet = [mpWindow attachedSheet];

				// Retain self to ensure that we don't release it before the
				// completion handler executes
				[self retain];
				[pPanel beginSheetWithPrintInfo:mpInfo modalForWindow:mpWindow delegate:self didEndSelector:@selector(printPanelDidEnd:returnCode:contextInfo:) contextInfo:nil];

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
				[self setResult:[pPanel runModalWithPrintInfo:mpInfo] printPanel:pPanel];
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

BOOL JavaSalInfoPrinter::Setup( SalFrame* pFrame, ImplJobSetup* pSetupData )
{
	BOOL bRet = FALSE;

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
		ULONG nCount = Application::ReleaseSolarMutex();

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

			bRet = TRUE;
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
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( ! ( nFlags & SAL_JOBSET_ORIENTATION ) )
	{
		NSPrintingOrientation nOrientation = ( mpInfo ? [mpInfo orientation] : NSPortraitOrientation );
		if ( ( mbPaperRotated && nOrientation == NSPortraitOrientation ) || ( !mbPaperRotated && nOrientation == NSLandscapeOrientation ) )
			pSetupData->meOrientation = ORIENTATION_LANDSCAPE;
		else
			pSetupData->meOrientation = ORIENTATION_PORTRAIT;
	}
	else if ( mpInfo )
	{
		if ( ( mbPaperRotated && pSetupData->meOrientation == ORIENTATION_PORTRAIT ) || ( !mbPaperRotated && pSetupData->meOrientation == ORIENTATION_LANDSCAPE ) )
			[mpInfo setOrientation:NSLandscapeOrientation];
		else
			[mpInfo setOrientation:NSPortraitOrientation];
	}

	if ( ! ( nFlags & SAL_JOBSET_PAPERBIN ) )
		pSetupData->mnPaperBin = 0;

	if ( ! ( nFlags & SAL_JOBSET_PAPERSIZE ) )
	{
		pSetupData->mePaperFormat = ImplPrintInfoGetPaperType( mpInfo, mbPaperRotated );
		if ( pSetupData->mePaperFormat == PAPER_USER && mpInfo )
		{
			NSSize aPaperSize = [mpInfo paperSize];
			NSPrintingOrientation nOrientation = ( mpInfo ? [mpInfo orientation] : NSPortraitOrientation );
			if ( ( mbPaperRotated && nOrientation == NSPortraitOrientation ) || ( !mbPaperRotated && nOrientation == NSLandscapeOrientation ) )
			{
				pSetupData->mnPaperWidth = (long)( aPaperSize.height * 2540 / 72 );
				pSetupData->mnPaperHeight = (long)( aPaperSize.width * 2540 / 72 );
			}
			else
			{
				pSetupData->mnPaperWidth = (long)( aPaperSize.width * 2540 / 72 );
				pSetupData->mnPaperHeight = (long)( aPaperSize.height * 2540 / 72 );
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
		mbPaperRotated = ImplPrintInfoSetPaperType( mpInfo, pSetupData->mePaperFormat, pSetupData->meOrientation, (float)pSetupData->mnPaperWidth * 72 / 2540, (float)pSetupData->mnPaperHeight * 72 / 2540 );
		if ( ( mbPaperRotated && pSetupData->meOrientation == ORIENTATION_PORTRAIT ) || ( !mbPaperRotated && pSetupData->meOrientation == ORIENTATION_LANDSCAPE ) )
			[mpInfo setOrientation:NSLandscapeOrientation];
		else
			[mpInfo setOrientation:NSPortraitOrientation];
	}

	[pPool release];

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
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	Size aSize;
	Rectangle aRect;
	if ( mpInfo )
	{
		NSSize aPaperSize = [mpInfo paperSize];
		NSRect aPageBounds = [mpInfo imageablePageBounds];

		// Flip page bounds
		aPageBounds.origin.y = aPaperSize.height - aPageBounds.origin.y - aPageBounds.size.height;

		NSPrintingOrientation nOrientation = [mpInfo orientation];
		if ( ( mbPaperRotated && nOrientation == NSPortraitOrientation ) || ( !mbPaperRotated && nOrientation == NSLandscapeOrientation ) )
		{
			aSize = Size( (long)( aPaperSize.height * MIN_PRINTER_RESOLUTION / 72 ), (long)( aPaperSize.width * MIN_PRINTER_RESOLUTION / 72 ) );
			aRect = Rectangle( Point( (long)( ( aPaperSize.height - aPageBounds.origin.y - aPageBounds.size.height ) * MIN_PRINTER_RESOLUTION / 72  ), (long)( aPageBounds.origin.x * MIN_PRINTER_RESOLUTION / 72  ) ), Size( (long)( aPageBounds.size.height * MIN_PRINTER_RESOLUTION / 72 ), (long)( aPageBounds.size.width * MIN_PRINTER_RESOLUTION / 72 ) ) );
		}
		else
		{
			aSize = Size( (long)( aPaperSize.width * MIN_PRINTER_RESOLUTION / 72 ), (long)( aPaperSize.height * MIN_PRINTER_RESOLUTION / 72 ) );
			aRect = Rectangle( Point( (long)( aPageBounds.origin.x * MIN_PRINTER_RESOLUTION / 72  ), (long)( ( aPaperSize.height - aPageBounds.origin.y - aPageBounds.size.height ) * MIN_PRINTER_RESOLUTION / 72  ) ), Size( (long)( aPageBounds.size.width * MIN_PRINTER_RESOLUTION / 72 ), (long)( aPageBounds.size.height * MIN_PRINTER_RESOLUTION / 72 ) ) );
		}
	}

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

	[pPool release];
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
	mpInfoPrinter( pInfoPrinter ),
	mbGraphics( FALSE ),
	mePaperFormat( PAPER_USER ),
	mnPaperWidth( 0 ),
	mnPaperHeight( 0 ),
	mbStarted( FALSE ),
	mpInfo( nil ),
	mbPaperRotated( sal_False ),
	mpPrintOperation( nil ),
	maPrintThread( NULL ),
	mpPrintView( nil )
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
	// Call EndJob() to join and destroy the print thread
	EndJob();

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpInfo )
		[mpInfo release];
	if ( mpPrintOperation )
		[mpPrintOperation release];
	if ( mpPrintView )
		[mpPrintView release];

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

BOOL JavaSalPrinter::StartJob( const XubString* pFileName,
							   const XubString& rJobName,
							   const XubString& rAppName,
							   ULONG nCopies, BOOL bCollate,
							   ImplJobSetup* pSetupData, BOOL bFirstPass )
{
	if ( !mpInfo )
		return FALSE;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	// Set paper type
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

	if ( !mpPrintOperation )
	{
		mbStarted = FALSE;

		if ( mpPrintView )
		{
			[mpPrintView release];
			mpPrintView = nil;
		}

		NSString *pJobName = [NSString stringWithCharacters:maJobName.GetBuffer() length:maJobName.Len()];
		if ( bFirstPass )
		{
			// Ignore any AWT events while the page layout dialog is showing
			// to emulate a modal dialog
			NSWindow *pNSWindow = nil;
			if ( Application_beginModalSheet( &pNSWindow ) )
			{
				// Don't lock mutex as we expect callbacks to this object from
				// a different thread while the dialog is showing
				ULONG nCount = Application::ReleaseSolarMutex();

				JavaSalPrinterShowPrintDialog *pJavaSalPrinterShowPrintDialog = [JavaSalPrinterShowPrintDialog createWithPrintInfo:mpInfo window:pNSWindow jobName:pJobName];
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				[pJavaSalPrinterShowPrintDialog performSelectorOnMainThread:@selector(showPrintDialog:) withObject:pJavaSalPrinterShowPrintDialog waitUntilDone:YES modes:pModes];

				while ( ![pJavaSalPrinterShowPrintDialog finished] && !Application::IsShutDown() )
				{
					[pJavaSalPrinterShowPrintDialog performSelectorOnMainThread:@selector(checkForErrors:) withObject:pJavaSalPrinterShowPrintDialog waitUntilDone:YES modes:pModes];
					if ( Application::IsShutDown() )
						break;

					IMutex &rSolarMutex = Application::GetSolarMutex();
					rSolarMutex.acquire();
					if ( !Application::IsShutDown() )
						Application::Yield();
					rSolarMutex.release();
				}

				NSPrintInfo *pInfo = [pJavaSalPrinterShowPrintDialog printInfo];
				if ( pInfo )
				{
					if ( pInfo != mpInfo )
					{
						if ( mpInfo )
							[mpInfo release];
						mpInfo = pInfo;
						[mpInfo retain];
					}

					mbStarted = TRUE;
				}

				[pJavaSalPrinterShowPrintDialog performSelectorOnMainThread:@selector(destroy:) withObject:pJavaSalPrinterShowPrintDialog waitUntilDone:YES modes:pModes];

				Application::AcquireSolarMutex( nCount );
				Application_endModalSheet();
			}
		}
		else
		{
			// Update print info settings
			mbPaperRotated = ImplPrintInfoSetPaperType( mpInfo, pSetupData->mePaperFormat, pSetupData->meOrientation, (float)pSetupData->mnPaperWidth * 72 / 2540, (float)pSetupData->mnPaperHeight * 72 / 2540 );
			if ( ( mbPaperRotated && pSetupData->meOrientation == ORIENTATION_PORTRAIT ) || ( !mbPaperRotated && pSetupData->meOrientation == ORIENTATION_LANDSCAPE ) )
				[mpInfo setOrientation:NSLandscapeOrientation];
			else
				[mpInfo setOrientation:NSPortraitOrientation];
			mePaperFormat = pSetupData->mePaperFormat;
			mnPaperWidth = pSetupData->mnPaperWidth;
			mnPaperHeight = pSetupData->mnPaperHeight;

			// Set scaling factor
			NSNumber *pValue = [NSNumber numberWithFloat:fScaleFactor];
			if ( pValue )
			{
				NSMutableDictionary *pDict = [mpInfo dictionary];
				if ( pDict )
					[pDict setObject:pValue forKey:NSPrintScalingFactor];
			}

			NSSize aPaperSize = [mpInfo paperSize];

			// Do not retain as invoking alloc disables autorelease
			mpPrintView = [[VCLPrintView alloc] initWithFrame:NSMakeRect( 0, 0, aPaperSize.width, aPaperSize.height )];
			if ( mpPrintView )
			{
				mpPrintOperation = [NSPrintOperation printOperationWithView:mpPrintView printInfo:mpInfo];
				if ( mpPrintOperation )
				{
					[mpPrintOperation retain];
					[mpPrintOperation setJobTitle:pJobName];
					[mpPrintOperation setShowsPrintPanel:NO];
					[mpPrintOperation setShowsProgressPanel:NO];

					mbStarted = TRUE;
				}
			}
		}
	}

	[pPool release];

	return mbStarted;
}

// -----------------------------------------------------------------------

BOOL JavaSalPrinter::EndJob()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( maPrintThread )
	{
		TimeValue aDelay;
		aDelay.Seconds = 0;
		aDelay.Nanosec = 10000;
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
	mbStarted = FALSE;

	JavaSalEventQueue::setShutdownDisabled( sal_False );
	return TRUE;
}

// -----------------------------------------------------------------------

BOOL JavaSalPrinter::AbortJob()
{
	if ( mpPrintView )
		[mpPrintView abortPrintOperation];

	EndJob();

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

			NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

			if ( mpPrintOperation )
			{
				[mpPrintOperation release];
				mpPrintOperation = nil;
			}

			if ( mpPrintView )
			{
				[mpPrintView release];
				mpPrintView = nil;
			}

			[pPool release];

			if ( !StartJob( NULL, maJobName, XubString(), 1, TRUE, pSetupData, FALSE ) )
				return NULL;
		}
	}

	if ( !maPrintThread )
	{
		// Do not retain as invoking alloc disables autorelease
		maPrintThread = osl_createSuspendedThread( ImplPrintOperationRun, this );
		if ( maPrintThread )
			osl_resumeThread( maPrintThread );
	}

	// The OOo code does not call Printer::EndJob() if the page range is
	// results in no pages to print so do not disable shutdown until a page
	// is successfully started
	JavaSalEventQueue::setShutdownDisabled( sal_True );

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
		mpInfoPrinter->GetPageInfo( pSetupData, nOutWidth, nOutHeight, nPageOffX, nPageOffY, nPageWidth, nPageHeight );

		mpGraphics->mfPageTranslateX = (float)nPageOffX * 72 / MIN_PRINTER_RESOLUTION;
		mpGraphics->mfPageTranslateY = (float)nPageOffY * 72 / MIN_PRINTER_RESOLUTION;
		mpGraphics->maNativeBounds = CGRectMake( 0, 0, nPageWidth, nPageHeight );
	}

	mbGraphics = TRUE;

	return mpGraphics;
}

// -----------------------------------------------------------------------

BOOL JavaSalPrinter::EndPage()
{
	if ( mpGraphics )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		// Give ownership of graphics to print view
		if ( !mpPrintView || ![mpPrintView addUnprintedGraphics:mpGraphics] )
			delete mpGraphics;

		[pPool release];
	}
	mpGraphics = NULL;
	mbGraphics = FALSE;
	return TRUE;
}

// -----------------------------------------------------------------------

ULONG JavaSalPrinter::GetErrorCode()
{
	if ( !mbStarted || !mpPrintOperation || !maPrintThread || !osl_isThreadRunning( maPrintThread ) )
		return SAL_PRINTER_ERROR_ABORT;
	else
		return 0;
}

// -----------------------------------------------------------------------

XubString JavaSalPrinter::GetJobDisposition()
{
	XubString aRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpInfo )
	{
		NSString *pJobDisposition = [mpInfo jobDisposition];
		if ( pJobDisposition && ![pJobDisposition isEqualToString:NSPrintCancelJob] )
		{
			unsigned int nLen = [pJobDisposition length];
			if ( nLen )
			{
				sal_Unicode aBuf[ nLen + 1 ];
				[pJobDisposition getCharacters:aBuf];
				aBuf[ nLen ] = 0;
				aRet = XubString( aBuf );
			}
		}
	}

	[pPool release];

	return aRet;
}

// -----------------------------------------------------------------------

XubString JavaSalPrinter::GetJobSavingPath()
{
	XubString aRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpInfo )
	{
		NSMutableDictionary *pDictionary = [mpInfo dictionary];
		if ( pDictionary )
		{
			NSURL *pJobSavingURL = [pDictionary objectForKey:NSPrintJobSavingURL];
			if ( pJobSavingURL )
			{
				pJobSavingURL = [pJobSavingURL filePathURL];
				if ( pJobSavingURL )
				{
					Application_cacheSecurityScopedURL( pJobSavingURL );

					NSString *pJobSavingPath = [pJobSavingURL path];
					if ( pJobSavingPath )
					{
						unsigned int nLen = [pJobSavingPath length];
						if ( nLen )
						{
							sal_Unicode aBuf[ nLen + 1 ];
							[pJobSavingPath getCharacters:aBuf];
							aBuf[ nLen ] = 0;
							aRet = XubString( aBuf );
						}
					}
				}
			}
		}
	}

	[pPool release];

	return aRet;
}

// -----------------------------------------------------------------------

XubString JavaSalPrinter::GetPageRange()
{
	XubString aRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( mpInfo )
	{
		NSMutableDictionary *pDictionary = [mpInfo dictionary];
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

	return aRet;
}

// -----------------------------------------------------------------------

void JavaSalPrinter::SetJobDisposition( const XubString *pJobDisposition )
{
	if ( !mpInfo || !pJobDisposition || !pJobDisposition->Len() )
		return;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSString *pString = [NSString stringWithCharacters:pJobDisposition->GetBuffer() length:pJobDisposition->Len()];
	if ( pString && ![pString isEqualToString:NSPrintCancelJob] )
		[mpInfo setJobDisposition:pString];

	[pPool release];
}

// -----------------------------------------------------------------------

void JavaSalPrinter::SetJobSavingPath( const XubString *pJobSavingPath, sal_Int32 nIteration )
{
	if ( !mpInfo || !pJobSavingPath || !pJobSavingPath->Len() )
		return;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSMutableDictionary *pDictionary = [mpInfo dictionary];
	if ( pDictionary )
	{
		NSString *pString = [NSString stringWithCharacters:pJobSavingPath->GetBuffer() length:pJobSavingPath->Len()];
		if ( pString )
		{
			pString = [NSString stringWithFormat:@"%@ %d.%@", [pString stringByDeletingPathExtension], nIteration, [pString pathExtension]];
			if ( pString )
			{
				NSURL *pURL = [NSURL fileURLWithPath:pString];
				if ( pURL )
				{
					NSString *pPath = [pURL path];
					if ( pPath )
					{
						NSString *pTitle = nil;
						XubString aTitle( GetSfxResString( STR_SAVEDOC ) );
						aTitle.EraseAllChars( '~' );
						if ( aTitle.Len() )
							pTitle = [NSString stringWithCharacters:aTitle.GetBuffer() length:aTitle.Len()];

						id pSecurityScopedURLs = Application_acquireSecurityScopedURLFromNSURL( pURL, sal_True, pTitle );
						if ( pSecurityScopedURLs )
							maSecurityScopeURLList.push_back( pSecurityScopedURLs );
					}

					[pDictionary setObject:pURL forKey:NSPrintJobSavingURL];
				}
			}
		}
	}

	[pPool release];
}

// -----------------------------------------------------------------------

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
