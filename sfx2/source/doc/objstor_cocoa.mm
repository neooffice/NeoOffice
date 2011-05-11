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

#import <Cocoa/Cocoa.h>
#import <objc/objc-class.h>
#import "objstor_cocoa.h"

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

@interface SFXDocument : NSDocument
{
	NSWindowController*		mpWinController;
	NSWindow*				mpWindow;
}
+ (BOOL)autosavesInPlace;
- (void)dealloc;
- (void)encodeRestorableStateWithCoder:(NSCoder *)pCoder;
- (id)initWithContentsOfURL:(NSURL *)pURL window:(NSWindow *)pWindow ofType:(NSString *)pTypeName error:(NSError **)pError;
- (BOOL)readFromURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)pError;
- (BOOL)revertToContentsOfURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)pError;
- (NSArray *)writableTypesForSaveOperation:(NSSaveOperationType)nSaveOperation;
@end

@implementation SFXDocument

+ (BOOL)autosavesInPlace
{
	return YES;
}

- (void)dealloc
{
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

- (void)encodeRestorableStateWithCoder:(NSCoder *)pCoder
{
	// Do not trigger automatic restoration of the document when launching
}

- (id)initWithContentsOfURL:(NSURL *)pURL window:(NSWindow *)pWindow ofType:(NSString *)pTypeName error:(NSError **)pError
{
	[super initWithContentsOfURL:pURL ofType:pTypeName error:pError];

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

	return self;
}

- (BOOL)readFromURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)pError
{
	if ( pError )
		pError = nil;

	return YES;
}

- (BOOL)revertToContentsOfURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)pError
{
	CFShow( pURL );
	if ( pError )
		pError = nil;

	return YES;
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

@end

@interface RunDocumentVersions : NSObject
{
	NSURL*					mpURL;
	NSView*					mpView;
}
+ (id)createWithView:(NSView *)pView URL:(NSURL *)pURL;
- (id)initWithView:(NSView *)pView URL:(NSURL *)pURL;
- (void)revertDocumentToSaved:(id)pObject;
@end

@implementation RunDocumentVersions

+ (id)createWithView:(NSView *)pView URL:(NSURL *)pURL
{
	RunDocumentVersions *pRet = [[RunDocumentVersions alloc] initWithView:pView URL:pURL];
	[pRet autorelease];
	return pRet;
}

- (id)initWithView:(NSView *)pView URL:(NSURL *)pURL
{
	[super init];

	mpURL = pURL;
	mpView = pView;

	return self;
}

- (void)revertDocumentToSaved:(id)pObject
{
	if ( mpView )
	{
		NSWindow *pWindow = [mpView window];
		if ( pWindow && [pWindow isVisible] )
		{
			NSError *pError = nil;
			SFXDocument *pDoc = [[SFXDocument alloc] initWithContentsOfURL:mpURL window:pWindow ofType:@"" error:&pError];
			if ( pDoc )
			{
				[pDoc revertDocumentToSaved:self];
				[pDoc release];
			}
		}
	}
}

@end

void NSDocument_revertDocumentToSaved( NSView *pView, CFURLRef aURL )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pView && aURL )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		RunDocumentVersions *pRunDocumentVersions = [RunDocumentVersions createWithView:pView URL:(NSURL *)aURL];
		[pRunDocumentVersions performSelectorOnMainThread:@selector(revertDocumentToSaved:) withObject:pRunDocumentVersions waitUntilDone:YES modes:pModes];
	}

	[pPool release];
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
	return ( class_getClassMethod( [NSDocument class], @selector(restorableStateKeyPaths) ) ? YES : NO );
#else	// USE_NATIVE_VERSIONS
	return NO;
#endif	// USE_NATIVE_VERSIONS
}
