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
 *  Patrick Luby, May 2011
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2011 Planamesa Inc.
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

#import <dlfcn.h>

#include <com/sun/star/embed/ElementModes.hpp>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/topfrm.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#import <objc/objc-runtime.h>
#include <postmac.h>
#import "topfrm_cocoa.h"

#define PDF_BUF_SIZE ( 128 * 1024 )

typedef void Application_cacheSecurityScopedURL_Type( id pURL );

static Application_cacheSecurityScopedURL_Type *pApplication_cacheSecurityScopedURL = NULL;
static NSString *pNoTranslationValue = @" ";
static NSString *pNSQuickLookWrapperDocument = @"NSQuickLookWrapperDocument";

using namespace com::sun::star;
using namespace rtl;
using namespace vos;

static BOOL HasNativeVersion( Window *pWindow )
{
	SfxViewFrame *pFrame = SfxViewFrame::GetFirst();
	while ( pFrame )
	{
		SfxTopViewFrame *pTopViewFrame = (SfxTopViewFrame *)pFrame->GetTopViewFrame();
		if ( pTopViewFrame && pTopViewFrame->GetTopFrame_Impl()->GetTopWindow_Impl() == pWindow )
		{
			SfxObjectShell *pDoc = pTopViewFrame->GetObjectShell();
			if ( pDoc )
			{
				SfxMedium *pMedium = pDoc->GetMedium();
				if ( pMedium )
				{
					if ( !pMedium->GetBaseURL( true ).getLength() )
						return NO;
				}
			}

			return YES;
		}

		pFrame = SfxViewFrame::GetNext( *pFrame );
	}

	return NO;
}

static const NSString *pWritableTypeEntries[] = {
	#ifdef PRODUCT_NAME
	@PRODUCT_NAME" Chart Document",
	@PRODUCT_NAME" Database Document",
	@PRODUCT_NAME" Document",
	@PRODUCT_NAME" Drawing Document",
	@PRODUCT_NAME" Drawing Template",
	@PRODUCT_NAME" Extension",
	@PRODUCT_NAME" Formula Document",
	@PRODUCT_NAME" HTML Document",
	@PRODUCT_NAME" HTML Template",
	@PRODUCT_NAME" Image Document",
	@PRODUCT_NAME" Image Template",
	@PRODUCT_NAME" Master Document",
	@PRODUCT_NAME" MathML Document",
	@PRODUCT_NAME" Presentation Document",
	@PRODUCT_NAME" Presentation Template",
	@PRODUCT_NAME" Spreadsheet Document",
	@PRODUCT_NAME" Spreadsheet Template",
	@PRODUCT_NAME" Text Document",
	@PRODUCT_NAME" Text Template"
#endif	// PRODUCT_NAME
};

static NSArray *pWritableTypes = nil;
static OUString aRevertToSavedLocalizedString;
static OUString aSaveAVersionLocalizedString;

@class NSDocumentVersion;

@interface NSObject (NSDocumentRevisionsController)
+ (id)sharedController;
- (BOOL)isVisualizing;
@end

@interface NSDocument (SFXDocument)
- (void)_checkAutosavingThenUpdateChangeCount:(NSDocumentChangeType)nChangeType;
- (BOOL)_preserveContentsIfNecessaryAfterWriting:(BOOL)bAfter toURL:(NSURL *)pURL forSaveOperation:(NSUInteger)nSaveOperation version:(NSDocumentVersion **)ppVersion error:(NSError **)ppError;
- (void)browseDocumentVersions:(id)pSender;
- (void)poseAsMakeWindowControllers;
@end

@interface SFXDocument : NSDocument
{
	SfxTopViewFrame*		mpFrame;
	BOOL					mbInSetDocumentModified;
	NSWindowController*		mpWinController;
	NSWindow*				mpWindow;
}
+ (BOOL)autosavesInPlace;
+ (void)initialize;
- (void)dealloc;
- (NSDocument *)duplicateAndReturnError:(NSError **)ppError;
- (void)duplicateDocument:(id)pObject;
- (void)duplicateDocumentAndWaitForRevertCall:(BOOL)bWait;
- (BOOL)hasUnautosavedChanges;
- (id)initWithContentsOfURL:(NSURL *)pURL frame:(SfxTopViewFrame *)pFrame window:(NSWindow *)pWindow ofType:(NSString *)pTypeName error:(NSError **)ppError;
- (BOOL)isInVersionBrowser;
- (void)makeWindowControllers;
- (BOOL)readFromURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)ppError;
- (void)reloadFrame;
- (void)restoreStateWithCoder:(NSCoder *)pCoder;
- (void)revertDocumentToSaved:(id)pObject;
- (BOOL)revertToContentsOfURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)ppError;
- (void)setDocumentModified:(BOOL)bModified;
- (void)updateChangeCount:(NSDocumentChangeType)nChangeType;
- (NSArray *)writableTypesForSaveOperation:(NSSaveOperationType)nSaveOperation;
- (BOOL)writeToURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)ppError;
@end

@interface SFXUndoManager : NSUndoManager
{
	SFXDocument*			mpDoc;
}
+ (id)createWithDocument:(SFXDocument *)pDoc;
- (void)dealloc;
- (id)initWithDocument:(SFXDocument *)pDoc;
- (void)undo;
@end

static NSMutableDictionary *pFrameDict = nil;

static OUString NSStringToOUString( NSString *pString )
{
	OUString aRet;

	if ( pString )
	{
		NSUInteger nLen = [pString length];
		if ( nLen )
		{
			sal_Unicode aBuf[ nLen + 1 ];
			[pString getCharacters:aBuf];
			aBuf[ nLen ] = 0;
			aRet = OUString( aBuf );
		}
	}

	return aRet;
}

static SFXDocument *GetDocumentForFrame( SfxTopViewFrame *pFrame )
{
	SFXDocument *pRet = nil;

	if ( pFrame && pFrameDict )
	{
		NSNumber *pKey = [NSNumber numberWithUnsignedLong:(unsigned long)pFrame];
		if ( pKey )
			pRet = [pFrameDict objectForKey:pKey];
	}

	return pRet;
}

static void SetDocumentForFrame( SfxTopViewFrame *pFrame, SFXDocument *pDoc )
{
	if ( !pFrame )
		return;

	if ( !pFrameDict )
	{
		pFrameDict = [NSMutableDictionary dictionaryWithCapacity:10];
		if ( pFrameDict )
			[pFrameDict retain];
	}

	if ( pFrameDict )
	{
		NSNumber *pKey = [NSNumber numberWithUnsignedLong:(unsigned long)pFrame];
		if ( pKey )
		{
			// If we are replacing an existing document, close the old document
			// otherwise it will never get released by its controllers
			SFXDocument *pOldDoc = [pFrameDict objectForKey:pKey];
			if ( pOldDoc )
				[pOldDoc close];

			if ( pDoc )
			{

				[pFrameDict setObject:pDoc forKey:pKey];
			}
			else
			{
				[pFrameDict removeObjectForKey:pKey];
			}
		}
	}
}

#include <premac.h>
#import <Quartz/Quartz.h>
#include <postmac.h>

@implementation SFXDocument

+ (BOOL)autosavesInPlace
{
	return NSDocument_versionsSupported();
}

+ (void)initialize
{
	Class aNSQuickLookWrapperDocumentClass = NSClassFromString( pNSQuickLookWrapperDocument );
	SEL aSelector = @selector(makeWindowControllers);
	SEL aPoseAsSelector = @selector(poseAsMakeWindowControllers);
	Method aOldMethod = class_getInstanceMethod( aNSQuickLookWrapperDocumentClass, aSelector );
	Method aNewMethod = class_getInstanceMethod( [SFXDocument class], aSelector );
	if ( aOldMethod && aNewMethod )
	{
		IMP aOldIMP = method_getImplementation( aOldMethod );
		IMP aNewIMP = method_getImplementation( aNewMethod );
		if ( aOldIMP && aNewIMP && class_addMethod( aNSQuickLookWrapperDocumentClass, aPoseAsSelector, aOldIMP, method_getTypeEncoding( aOldMethod ) ) )
			method_setImplementation( aOldMethod, aNewIMP );
	}
}

- (void)dealloc
{
	// Release our custom undo manager
	[self setUndoManager:nil];

	if ( mpWinController )
	{
		[self removeWindowController:mpWinController];
		[mpWinController release];
	}

	if ( mpWindow )
		[mpWindow release];

	NSDocumentController *pDocController = [NSDocumentController sharedDocumentController];
	if ( pDocController )
		[pDocController removeDocument:self];

	[super dealloc];
}

- (NSDocument *)duplicateAndReturnError:(NSError **)ppError
{
	if ( ppError )
		*ppError = nil;

	// This selector gets invoked by the duplicate button in the "this document
	// is locked" alert that appears when you change content in a locked
	// document so wait for a revert call
	[self duplicateDocumentAndWaitForRevertCall:YES];

	return self;
}

- (void)duplicateDocument:(id)pObject
{
	// This selector gets invoked by the duplicate menu item in the titlebar's
	// menu so don't wait for a revert call and execute immediately
	[self duplicateDocumentAndWaitForRevertCall:NO];
}

- (void)duplicateDocumentAndWaitForRevertCall:(BOOL)bWait
{
	if ( NSDocument_versionsSupported() && !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			SFXDocument *pDoc = GetDocumentForFrame( mpFrame );
			if ( pDoc == self )
				SFXDocument_duplicate( mpFrame, bWait );
		}
		rSolarMutex.release();
	}
}

- (BOOL)hasUnautosavedChanges
{
	// Don't allow NSDocument to do the autosaving
	return NO;
}

- (id)initWithContentsOfURL:(NSURL *)pURL frame:(SfxTopViewFrame *)pFrame window:(NSWindow *)pWindow ofType:(NSString *)pTypeName error:(NSError **)ppError
{
	[super initWithContentsOfURL:pURL ofType:pTypeName error:ppError];

	mpFrame = pFrame;
	mbInSetDocumentModified = NO;
	mpWinController = nil;
	mpWindow = pWindow;
	if ( mpWindow )
	{
		[mpWindow retain];

		NSDocumentController *pDocController = [NSDocumentController sharedDocumentController];
		if ( pDocController )
		{
			mpWinController = [[NSWindowController alloc] initWithWindow:mpWindow];
			if ( mpWinController )
			{
				[self addWindowController:mpWinController];
				[pDocController addDocument:self];
			}
		}
	}

	// Set our own custom undo manager
	[self setUndoManager:[SFXUndoManager createWithDocument:self]];

	return self;
}

- (BOOL)isInVersionBrowser
{
	NSBundle *pBundle = [NSBundle bundleForClass:[NSDocument class]];
	if ( pBundle )
	{
		Class aClass = [pBundle classNamed:@"NSDocumentRevisionsController"];
		if ( aClass && class_getClassMethod( aClass, @selector(sharedController) ) )
		{
			id pController = [aClass sharedController];
			if ( pController && [pController respondsToSelector:@selector(isVisualizing)] )
				return [pController isVisualizing];
		}
	}

	return NO;
}

- (void)makeWindowControllers
{
	if ( [[self className] isEqualToString:pNSQuickLookWrapperDocument] && [self respondsToSelector:@selector(poseAsMakeWindowControllers)] )
	{
		[self poseAsMakeWindowControllers];

		NSMutableData *pPDFData = nil;
		NSURL *pFileURL = [self fileURL];
		if ( pFileURL )
		{
			if ( !pApplication_cacheSecurityScopedURL )
				pApplication_cacheSecurityScopedURL = (Application_cacheSecurityScopedURL_Type *)dlsym( RTLD_DEFAULT, "Application_cacheSecurityScopedURL" );
			if ( pApplication_cacheSecurityScopedURL )
				pApplication_cacheSecurityScopedURL( pFileURL );

			OUString aFileURL( NSStringToOUString( [pFileURL absoluteString] ) );
			if ( aFileURL.getLength() )
			{
				SfxMedium aMedium( aFileURL, STREAM_STD_READ );
				uno::Reference< io::XInputStream > xPDFInputStream;
				try
				{
					uno::Reference< embed::XStorage > xStorage( aMedium.GetStorage() );
					if ( xStorage.is() )
					{
						uno::Reference< embed::XStorage > xThumbnails = xStorage->openStorageElement( OUString( RTL_CONSTASCII_USTRINGPARAM( "Thumbnails" ) ), embed::ElementModes::READ );
						if ( xThumbnails.is() )
						{
							uno::Reference< io::XStream > xPDFStream = xThumbnails->openStreamElement( OUString( RTL_CONSTASCII_USTRINGPARAM( "thumbnail.pdf" ) ), embed::ElementModes::READ );
							if ( xPDFStream.is() )
							{
								uno::Reference< io::XInputStream > xPDFInputStream = xPDFStream->getInputStream();
								if ( xPDFInputStream.is() )
								{
									pPDFData = [NSMutableData dataWithCapacity:PDF_BUF_SIZE];
									if ( pPDFData )
									{
										static const sal_uInt32 nBytes = 4096;
										sal_Int32 nBytesRead;
										uno::Sequence< ::sal_Int8 > aBytes( nBytes );
										while ( ( nBytesRead = xPDFInputStream->readBytes( aBytes, nBytes ) ) > 0 )
											[pPDFData appendBytes:aBytes.getConstArray() length:nBytesRead];
									}
								}
							}
						}
					}
				}
				catch ( ... )
				{
				}

				if ( xPDFInputStream.is() )
				{
					try
					{
						xPDFInputStream->closeInput();
					}
					catch ( ... )
					{
					}
				}

				aMedium.CloseAndRelease();
			}
		}

		if ( pPDFData )
		{
			PDFDocument *pPDFDoc = [[PDFDocument alloc] initWithData:pPDFData];
			if ( pPDFDoc )
			{
				[pPDFDoc autorelease];

				NSArray *pWindowControllers = [self windowControllers];
				if ( pWindowControllers )
				{
					NSUInteger nCount = [pWindowControllers count];
					NSUInteger i = 0;
					for ( ; i < nCount; i++ )
					{
						NSWindowController *pWindowController = [pWindowControllers objectAtIndex:i];
						if ( pWindowController && [pWindowController isKindOfClass:[NSWindowController class]] )
						{
							NSWindow *pWindow = [pWindowController window];
							if ( pWindow )
							{
								PDFView *pPDFView = [[PDFView alloc] initWithFrame:[[pWindow contentView] frame]];
								if ( pPDFView )
								{
									[pPDFView autorelease];

									[pPDFView setDocument:pPDFDoc];
									[pPDFView setAllowsDragging:NO];
									[pPDFView setAutoScales:YES];
									[pPDFView setDisplaysPageBreaks:NO];
									[pWindow setContentView:pPDFView];
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		[super makeWindowControllers];
	}
}

- (BOOL)readFromURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)ppError
{
	if ( ppError )
		*ppError = nil;

	return YES;
}

- (void)reloadFrame
{
	if ( NSDocument_versionsSupported() && !Application::IsShutDown() )
	{
		if ( [self isInVersionBrowser] )
		{
			[self performSelector:@selector(reloadFrame) withObject:nil afterDelay:0];
		}
		else
		{
			IMutex& rSolarMutex = Application::GetSolarMutex();
			rSolarMutex.acquire();
			if ( !Application::IsShutDown() )
			{
				SFXDocument *pDoc = GetDocumentForFrame( mpFrame );
				if ( pDoc == self )
					SFXDocument_reload( mpFrame );
			}
			rSolarMutex.release();
		}
	}
}

- (void)restoreStateWithCoder:(NSCoder *)pCoder
{
	// Don't allow NSDocument to do the restoration
}

- (void)revertDocumentToSaved:(id)pObject
{
	if ( [self isInVersionBrowser] )
		return;

	if ( [super respondsToSelector:@selector(browseDocumentVersions:)] )
		[super browseDocumentVersions:pObject];
}

- (BOOL)revertToContentsOfURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)ppError
{
	if ( ppError )
		*ppError = nil;

	[self reloadFrame];

	return YES;
}

- (void)setDocumentModified:(BOOL)bModified
{
	if ( mbInSetDocumentModified )
		return;

	mbInSetDocumentModified = YES;

	if ( bModified )
	{
		if ( [self respondsToSelector:@selector(_checkAutosavingThenUpdateChangeCount:)] )
			[self _checkAutosavingThenUpdateChangeCount:NSChangeDone];
		else
			[self updateChangeCount:NSChangeDone];
	}
	else
	{
		[self updateChangeCount:NSChangeCleared];
	}

	mbInSetDocumentModified = NO;
}

- (void)updateChangeCount:(NSDocumentChangeType)nChangeType
{
	BOOL bIsEdited = [self isDocumentEdited];

	[super updateChangeCount:nChangeType];

	if ( !mbInSetDocumentModified && nChangeType == NSChangeDone && !bIsEdited && [self isDocumentEdited] )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			SFXDocument *pDoc = GetDocumentForFrame( mpFrame );
			if ( pDoc == self )
				SFXDocument_documentHasBeenModified( mpFrame );
		}
		rSolarMutex.release();
	}
}

- (NSArray *)writableTypesForSaveOperation:(NSSaveOperationType)nSaveOperation
{
	if ( !pWritableTypes )
	{
		unsigned int nCount = sizeof( pWritableTypeEntries ) / sizeof( NSString* );
		if ( nCount )
			pWritableTypes = [NSArray arrayWithObjects:pWritableTypeEntries count:nCount];
		else
			pWritableTypes = [NSArray array];

		if ( pWritableTypes )
			[pWritableTypes retain];
	}

	return pWritableTypes;
}

- (BOOL)writeToURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)ppError
{
	if ( ppError )
		*ppError = nil;

	return NO;
}

@end

@implementation SFXUndoManager

+ (id)createWithDocument:(SFXDocument *)pDoc
{
	SFXUndoManager *pRet = [[SFXUndoManager alloc] initWithDocument:pDoc];
	[pRet autorelease];
	return pRet;
}

- (void)dealloc
{
	if ( mpDoc )
		[mpDoc release];

	[super dealloc];
}

- (id)initWithDocument:(SFXDocument *)pDoc
{
	[super init];

	mpDoc = pDoc;
	if ( mpDoc )
		[mpDoc retain];

	return self;
}

- (void)undo
{
	[super undo];

	// This selector is called when the document is locked and the user cancels
	// unlocking so revert any changes
	if ( mpDoc )
		[mpDoc reloadFrame];
}

@end

@interface NSBundle (RunSFXDocument)
+ (NSBundle *)bundleWithURL:(NSURL *)pURL;
@end

@interface RunSFXDocument : NSObject
{
	SFXDocument*			mpDoc;
	SfxTopViewFrame*		mpFrame;
	BOOL					mbReadOnly;
	NSString*				mpRevertToSavedLocalizedString;
	BOOL					mbSaved;
	NSString*				mpTitle;
	NSURL*					mpURL;
	NSView*					mpView;
}
+ (id)create;
+ (id)createWithFrame:(SfxTopViewFrame *)pFrame;
+ (id)createWithFrame:(SfxTopViewFrame *)pFrame view:(NSView *)pView URL:(NSURL *)pURL readOnly:(BOOL)bReadOnly;
- (void)createDocument:(id)pObject;
- (void)dealloc;
- (SFXDocument *)document;
- (void)getDocument:(id)pObject;
- (void)getTitle:(id)pObject;
- (id)initWithFrame:(SfxTopViewFrame *)pFrame view:(NSView *)pView URL:(NSURL *)pURL readOnly:(BOOL)bReadOnly;
- (void)revertDocumentToSaved:(id)pObject;
- (NSString *)revertToSavedLocalizedString;
- (void)saveVersionOfDocument:(id)pObject;
- (void)setDocumentModified:(id)pObject;
- (NSString *)title;
@end

@implementation RunSFXDocument

+ (id)create
{
	RunSFXDocument *pRet = [[RunSFXDocument alloc] initWithFrame:nil view:nil URL:nil readOnly:YES];
	[pRet autorelease];
	return pRet;
}

+ (id)createWithFrame:(SfxTopViewFrame *)pFrame
{
	RunSFXDocument *pRet = [[RunSFXDocument alloc] initWithFrame:pFrame view:nil URL:nil readOnly:YES];
	[pRet autorelease];
	return pRet;
}

+ (id)createWithFrame:(SfxTopViewFrame *)pFrame view:(NSView *)pView URL:(NSURL *)pURL readOnly:(BOOL)bReadOnly
{
	RunSFXDocument *pRet = [[RunSFXDocument alloc] initWithFrame:pFrame view:pView URL:pURL readOnly:bReadOnly];
	[pRet autorelease];
	return pRet;
}

- (void)createDocument:(id)pObject
{
	if ( mpFrame && mpView )
	{
		NSWindow *pWindow = [mpView window];
		if ( pWindow )
		{
			if ( mbReadOnly || !NSDocument_versionsSupported() )
			{
				[pWindow setRepresentedURL:mpURL];
			}
			else if ( [pWindow isVisible] )
			{
				SFXDocument *pOldDoc = GetDocumentForFrame( mpFrame );
				if ( pOldDoc )
				{
					NSURL *pOldURL = [pOldDoc fileURL];
					[pOldDoc setFileURL:mpURL];

					// If the URL has changed, disconnect the document from
					// the old URL's version history
					if ( !mpURL || !pOldURL || ![pOldURL isEqual:mpURL] )
						[pOldDoc updateChangeCount:NSChangeCleared];
				}
				else if ( mpURL )
				{
					NSError *pError = nil;
					SFXDocument *pDoc = [[SFXDocument alloc] initWithContentsOfURL:mpURL frame:mpFrame window:pWindow ofType:@"" error:&pError];
					SetDocumentForFrame( mpFrame, pDoc );
					// The document will be retained by SetDocumentForFrame()
					[pDoc release];
				}
			}
		}
	}
}

- (void)dealloc
{
	if ( mpDoc )
		[mpDoc release];
	if ( mpRevertToSavedLocalizedString )
		[mpRevertToSavedLocalizedString release];
	if ( mpTitle )
		[mpTitle release];
	if ( mpURL )
		[mpURL release];
	if ( mpView )
		[mpView release];

	[super dealloc];
}

- (SFXDocument *)document
{
	return mpDoc;
}

- (void)getDocument:(id)pObject
{
	if ( !mpDoc )
	{
		mpDoc = GetDocumentForFrame( mpFrame );
		if ( mpDoc )
			[mpDoc retain];
	}
}

- (void)getTitle:(id)pObject
{
	if ( !mpTitle )
	{
		SFXDocument *pDoc = GetDocumentForFrame( mpFrame );
		if ( pDoc )
		{
			mpTitle = [pDoc displayName];
			if ( mpTitle )
				[mpTitle retain];
		}
	}
}

- (id)initWithFrame:(SfxTopViewFrame *)pFrame view:(NSView *)pView URL:(NSURL *)pURL readOnly:(BOOL)bReadOnly
{
	[super init];

	mpDoc = nil;
	mpFrame = pFrame;
	mbReadOnly = bReadOnly;
	mpRevertToSavedLocalizedString = nil;
	mbSaved = NO;
	mpTitle = nil;
	mpURL = pURL;
	if ( mpURL )
		[mpURL retain];
	mpView = pView;
	if ( mpView )
		[mpView retain];

	return self;
}

- (void)release:(id)pObject
{
	SetDocumentForFrame( mpFrame, nil );
}

- (void)revertDocumentToSaved:(id)pObject
{
	SFXDocument *pDoc = GetDocumentForFrame( mpFrame );
	if ( pDoc )
		[pDoc revertDocumentToSaved:self];
}

- (NSString *)revertToSavedLocalizedString
{
	return mpRevertToSavedLocalizedString;
}

- (void)saveVersionOfDocument:(id)pObject
{
	SFXDocument *pDoc = GetDocumentForFrame( mpFrame );

	// Fix crashing bug reported in the following NeoOffice forum post by
	// checking for nil file URLs:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=64664#64664
	if ( pDoc && [pDoc respondsToSelector:@selector(_preserveContentsIfNecessaryAfterWriting:toURL:forSaveOperation:version:error:)] && [pDoc fileURL] )
	{
		NSDocumentVersion *pNewVersion = nil;
		NSError *pError = nil;
		mbSaved = [pDoc _preserveContentsIfNecessaryAfterWriting:YES toURL:[pDoc fileURL] forSaveOperation:NSSaveOperation version:&pNewVersion error:&pError];
	}
}

- (void)setDocumentModified:(id)pObject
{
	SFXDocument *pDoc = GetDocumentForFrame( mpFrame );
	if ( pDoc )
	{
		if ( pObject && [pObject isKindOfClass:[NSNumber class]] && [(NSNumber *)pObject boolValue] )
			[pDoc setDocumentModified:YES];
		else
			[pDoc setDocumentModified:NO];
	}
}

- (NSString *)title
{
	return mpTitle;
}

@end

OUString NSDocument_revertToSavedLocalizedString( Window *pWindow )
{
	if ( !pWindow || !NSDocument_versionsEnabled() || !HasNativeVersion( pWindow ) )
		return OUString();

	if ( !aRevertToSavedLocalizedString.getLength() )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSString *pKey = @"Browse All Versions\\U2026";
		NSString *pTable = @"Document";
		NSString *pLocalizedString = nil;
		NSBundle *pBundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/AppKit.framework"];
		if ( pBundle )
		{
			pLocalizedString = [pBundle localizedStringForKey:pKey value:pNoTranslationValue table:pTable];
			if ( pLocalizedString && [pLocalizedString length] && ![pLocalizedString isEqualToString:pNoTranslationValue] )
				aRevertToSavedLocalizedString = NSStringToOUString( pLocalizedString );
		}

		[pPool release];
	}

	return aRevertToSavedLocalizedString;
}

OUString NSDocument_saveAVersionLocalizedString( Window *pWindow )
{
	if ( !pWindow || !NSDocument_versionsEnabled() || !HasNativeVersion( pWindow ) )
		return OUString();

	if ( !aSaveAVersionLocalizedString.getLength() )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSString *pKey = @"Save a Version";
		NSString *pAltKey = @"Save";
		NSString *pTable = @"Document";
		NSString *pLocalizedString = nil;
		NSBundle *pBundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/AppKit.framework"];
		if ( pBundle )
		{
			pLocalizedString = [pBundle localizedStringForKey:pKey value:pNoTranslationValue table:pTable];
			if ( pLocalizedString && [pLocalizedString length] && ![pLocalizedString isEqualToString:pNoTranslationValue] )
			{
				aSaveAVersionLocalizedString = NSStringToOUString( pLocalizedString );
			}
			else
			{
				pLocalizedString = [pBundle localizedStringForKey:pAltKey value:pNoTranslationValue table:pTable];
				if ( pLocalizedString && [pLocalizedString length] && ![pLocalizedString isEqualToString:pNoTranslationValue] )
					aSaveAVersionLocalizedString = NSStringToOUString( pLocalizedString );
			}
		}

		[pPool release];
	}

	return aSaveAVersionLocalizedString;
}

BOOL NSDocument_versionsEnabled()
{
	BOOL bRet = NSDocument_versionsSupported();

	// Check if user has explicitly disabled versions
	if ( bRet )
	{
		CFPropertyListRef aPref = CFPreferencesCopyAppValue( CFSTR( "DisableVersions" ), kCFPreferencesCurrentApplication );
		if ( aPref )
		{
			if ( CFGetTypeID( aPref ) == CFBooleanGetTypeID() && (CFBooleanRef)aPref == kCFBooleanTrue )
				bRet = NO;
			CFRelease( aPref );
		}
	}

	return bRet;

}

BOOL NSDocument_versionsSupported()
{
#ifdef USE_NATIVE_VERSIONS
	return ( class_getInstanceMethod( [NSDocument class], @selector(browseDocumentVersions:) ) && class_getInstanceMethod( [NSDocument class], @selector(_checkAutosavingThenUpdateChangeCount:) ) && class_getInstanceMethod( [NSDocument class], @selector(_preserveContentsIfNecessaryAfterWriting:toURL:forSaveOperation:version:error:) ) ? YES : NO );
#else	// USE_NATIVE_VERSIONS
	return NO;
#endif	// USE_NATIVE_VERSIONS
}

void SFXDocument_createDocument( SfxTopViewFrame *pFrame, NSView *pView, CFURLRef aURL, BOOL bReadOnly )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pFrame && pView )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		RunSFXDocument *pRunSFXDocument = [RunSFXDocument createWithFrame:pFrame view:pView URL:(NSURL *)aURL readOnly:bReadOnly];
		[pRunSFXDocument performSelectorOnMainThread:@selector(createDocument:) withObject:pRunSFXDocument waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

OUString SFXDocument_documentTitle( SfxTopViewFrame *pFrame )
{
	OUString aRet;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pFrame )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		RunSFXDocument *pRunSFXDocument = [RunSFXDocument createWithFrame:pFrame];
		[pRunSFXDocument performSelectorOnMainThread:@selector(getTitle:) withObject:pRunSFXDocument waitUntilDone:YES modes:pModes];
		aRet = NSStringToOUString( [pRunSFXDocument title] );
	}

	[pPool release];

	return aRet;
}

BOOL SFXDocument_hasDocument( SfxTopViewFrame *pFrame )
{
	BOOL bRet = NO;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pFrame )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		RunSFXDocument *pRunSFXDocument = [RunSFXDocument createWithFrame:pFrame];
		[pRunSFXDocument performSelectorOnMainThread:@selector(getDocument:) withObject:pRunSFXDocument waitUntilDone:YES modes:pModes];
		bRet = ( [pRunSFXDocument document] ? YES : NO );
	}

	[pPool release];

	return bRet;
}

void SFXDocument_releaseDocument( SfxTopViewFrame *pFrame )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pFrame )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		RunSFXDocument *pRunSFXDocument = [RunSFXDocument createWithFrame:pFrame];
		[pRunSFXDocument performSelectorOnMainThread:@selector(release:) withObject:pRunSFXDocument waitUntilDone:YES modes:pModes];
	}

	[pPool release];
}

void SFXDocument_revertDocumentToSaved( SfxTopViewFrame *pFrame )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	RunSFXDocument *pRunSFXDocument = [RunSFXDocument createWithFrame:pFrame];
	[pRunSFXDocument performSelectorOnMainThread:@selector(revertDocumentToSaved:) withObject:pRunSFXDocument waitUntilDone:YES modes:pModes];

	[pPool release];
}

void SFXDocument_saveVersionOfDocument( SfxTopViewFrame *pFrame )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	RunSFXDocument *pRunSFXDocument = [RunSFXDocument createWithFrame:pFrame];
	[pRunSFXDocument performSelectorOnMainThread:@selector(saveVersionOfDocument:) withObject:pRunSFXDocument waitUntilDone:YES modes:pModes];

	[pPool release];
}

void SFXDocument_setDocumentModified( SfxTopViewFrame *pFrame, BOOL bModified )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSNumber *pNumber = [NSNumber numberWithBool:bModified];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	RunSFXDocument *pRunSFXDocument = [RunSFXDocument createWithFrame:pFrame];
	[pRunSFXDocument performSelectorOnMainThread:@selector(setDocumentModified:) withObject:pNumber waitUntilDone:YES modes:pModes];
	[pPool release];
}
