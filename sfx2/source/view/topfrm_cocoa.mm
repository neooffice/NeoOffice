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

#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#import <objc/objc-runtime.h>
#include <postmac.h>
#import "topfrm_cocoa.h"

// Uncomment out the following line to enable native versions support
// #define USE_NATIVE_VERSIONS

@class NSIBObjectData;
@class NSNibConnector;

static NSString *pRevertDocumentToSavedLabel = @"revertDocumentToSaved:";

using namespace rtl;
using namespace vos;

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

@interface SFXDocument : NSDocument
{
	SfxTopViewFrame*		mpFrame;
	NSWindowController*		mpWinController;
	NSWindow*				mpWindow;
}
+ (BOOL)autosavesInPlace;
- (void)dealloc;
- (void)encodeRestorableStateWithCoder:(NSCoder *)pCoder;
- (id)initWithContentsOfURL:(NSURL *)pURL frame:(SfxTopViewFrame *)pFrame window:(NSWindow *)pWindow ofType:(NSString *)pTypeName error:(NSError **)pError;
- (BOOL)readFromURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)pError;
- (BOOL)revertToContentsOfURL:(NSURL *)pURL ofType:(NSString *)pTypeName error:(NSError **)pError;
- (NSArray *)writableTypesForSaveOperation:(NSSaveOperationType)nSaveOperation;
@end

static NSMutableDictionary *pFrameDict = nil;

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
		// Do not retain as invoking alloc disables autorelease
		pFrameDict = [NSMutableDictionary dictionaryWithCapacity:10];
		if ( pFrameDict )
			[pFrameDict retain];
	}

	if ( pFrameDict )
	{
		NSNumber *pKey = [NSNumber numberWithUnsignedLong:(unsigned long)pFrame];
		if ( pKey )
		{
			if ( pDoc )
				[pFrameDict setObject:pDoc forKey:pKey];
			else
				[pFrameDict removeObjectForKey:pKey];
		}
	}
}

@implementation SFXDocument

+ (BOOL)autosavesInPlace
{
	return NSDocument_versionsSupported();
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

- (id)initWithContentsOfURL:(NSURL *)pURL frame:(SfxTopViewFrame *)pFrame window:(NSWindow *)pWindow ofType:(NSString *)pTypeName error:(NSError **)pError
{
	[super initWithContentsOfURL:pURL ofType:pTypeName error:pError];

	mpFrame = pFrame;
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
	if ( pError )
		pError = nil;

	if ( NSDocument_versionsSupported() && !Application::IsShutDown() )
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

@interface NSObject (NSIBObjectData)
- (NSArray *)allKeys;
- (NSArray *)connections;
- (NSMapTable *)objectTable;
@end

@interface NSObject (NSNibConnector)
- (void)establishConnection;
- (NSString *)label;
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
- (void)getRevertToSavedLocalizedString:(id)pObject;
- (id)initWithFrame:(SfxTopViewFrame *)pFrame view:(NSView *)pView URL:(NSURL *)pURL readOnly:(BOOL)bReadOnly;
- (void)revertDocumentToSaved:(id)pObject;
- (NSString *)revertToSavedLocalizedString;
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
	if ( mpFrame && mpView && mpURL )
	{
		NSWindow *pWindow = [mpView window];
		if ( pWindow )
		{
			if ( mbReadOnly )
			{
				[pWindow setRepresentedURL:mpURL];
			}
			else if ( [pWindow isVisible] )
			{
				NSError *pError = nil;
				SFXDocument *pDoc = [[SFXDocument alloc] initWithContentsOfURL:mpURL frame:mpFrame window:pWindow ofType:@"" error:&pError];
				if ( pDoc )
					SetDocumentForFrame( mpFrame, pDoc );
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

- (void)getRevertToSavedLocalizedString:(id)pObject
{
	if ( !mpRevertToSavedLocalizedString && class_getClassMethod( [NSBundle class], @selector(bundleWithURL:) ) )
	{
		CFURLRef aURL = NULL;
		if ( LSFindApplicationForInfo( kLSUnknownCreator, CFSTR( "com.apple.TextEdit" ), nil, nil, &aURL ) == noErr && aURL )
		{
			NSBundle *pBundle = [NSBundle bundleWithURL:(NSURL *)aURL];
			CFRelease( aURL );

			if ( pBundle )
			{
				NSString *pNibPath = [pBundle pathForResource:@"Edit" ofType:@"nib"];
				if ( pNibPath )
				{
					NSData *pNibData = [[NSData alloc] initWithContentsOfFile:pNibPath];
					if ( pNibData )
					{
						NSKeyedUnarchiver *pUnarchiver = [[NSKeyedUnarchiver alloc] initForReadingWithData:pNibData];
						[pUnarchiver autorelease];
						NSIBObjectData *pObjData = [pUnarchiver decodeObjectForKey:@"IB.objectdata"];
						if ( pObjData && [pObjData respondsToSelector:@selector(objectTable)] )
						{
							NSArray *pArray = [pObjData connections];
							if ( pArray )
							{
								NSUInteger i = 0;
								NSUInteger nCount = [pArray count];
								for ( ; i < nCount; i++ )
								{
									NSObject *pObject = [pArray objectAtIndex:i];
									if ( pObject && [pObject respondsToSelector:@selector(establishConnection)] && [pRevertDocumentToSavedLabel isEqualToString:[pObject label]] )
									{
										[pObject establishConnection];
										break;
									}
								}
							}

							NSMapTable *pTable = [pObjData objectTable];
							if ( pTable)
							{
								SEL aRevertDocumentToSavedSelector = NSSelectorFromString( pRevertDocumentToSavedLabel );
								NSArray *pArray = [pTable allKeys];
								NSUInteger i = 0;
								NSUInteger nCount = [pArray count];
								for ( ; i < nCount; i++ )
								{
									NSObject *pObject = [pArray objectAtIndex:i];
									if ( pObject && [pObject isKindOfClass:[NSMenuItem class]] )
									{
										NSMenuItem *pItem = (NSMenuItem *)pObject;
										if ( pItem && [pItem action] == aRevertDocumentToSavedSelector )
										{
											mpRevertToSavedLocalizedString = [pItem title];
											if ( mpRevertToSavedLocalizedString )
												[mpRevertToSavedLocalizedString retain];
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

- (id)initWithFrame:(SfxTopViewFrame *)pFrame view:(NSView *)pView URL:(NSURL *)pURL readOnly:(BOOL)bReadOnly
{
	[super init];

	mpFrame = pFrame;
	mpURL = pURL;
	if ( mpURL )
		[mpURL retain];
	mbReadOnly = bReadOnly;
	mpRevertToSavedLocalizedString = nil;
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
CFShow( mpRevertToSavedLocalizedString );
	return mpRevertToSavedLocalizedString;
}

@end

OUString NSDocument_revertToSavedLocalizedString()
{
	if ( !aRevertToSavedLocalizedString.getLength() )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		RunSFXDocument *pRunSFXDocument = [RunSFXDocument create];
		[pRunSFXDocument performSelectorOnMainThread:@selector(getRevertToSavedLocalizedString:) withObject:pRunSFXDocument waitUntilDone:YES modes:pModes];
		NSString *pLocalizedString = [pRunSFXDocument revertToSavedLocalizedString];
		if ( !pLocalizedString )
			pLocalizedString = @"Revert to Saved";

		NSUInteger nLen = [pLocalizedString length];
		if ( nLen )
		{
			sal_Unicode aBuf[ nLen + 1 ];
			[pLocalizedString getCharacters:aBuf];
			aBuf[ nLen ] = 0;
			aRevertToSavedLocalizedString = OUString( aBuf );
		}

		[pPool release];
	}

	return aRevertToSavedLocalizedString;
}

OUString NSDocument_saveAVersionLocalizedString()
{
	if ( !aSaveAVersionLocalizedString.getLength() )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSString *pKey = @"Save a Version";
		NSString *pLocalizedString = nil;
		NSBundle *pBundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/AppKit.framework"];
		if ( pBundle )
			pLocalizedString = [pBundle localizedStringForKey:pKey value:pKey table:@"Document"];
		if ( !pLocalizedString )
			pLocalizedString = pKey;

		NSUInteger nLen = [pLocalizedString length];
		if ( nLen )
		{
			sal_Unicode aBuf[ nLen + 1 ];
			[pLocalizedString getCharacters:aBuf];
			aBuf[ nLen ] = 0;
			aSaveAVersionLocalizedString = OUString( aBuf );
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
	return ( class_getClassMethod( [NSDocument class], @selector(restorableStateKeyPaths) ) ? YES : NO );
#else	// USE_NATIVE_VERSIONS
	return NO;
#endif	// USE_NATIVE_VERSIONS
}

void SFXDocument_createDocument( SfxTopViewFrame *pFrame, NSView *pView, CFURLRef aURL, BOOL bReadOnly )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pFrame && pView && aURL )
	{
		NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
		RunSFXDocument *pRunSFXDocument = [RunSFXDocument createWithFrame:pFrame view:pView URL:(NSURL *)aURL readOnly:bReadOnly];
		[pRunSFXDocument performSelectorOnMainThread:@selector(createDocument:) withObject:pRunSFXDocument waitUntilDone:YES modes:pModes];
	}

	[pPool release];
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
