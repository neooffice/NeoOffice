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
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 Planamesa Inc.
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

#include <list>
#include <mach-o/dyld.h>

#include <osl/objcutils.h>
#include <vcl/svapp.hxx>

#include "DTransClipboard.hxx"
#include "DTransTransferable.hxx"

#include "../../../osx/HtmlFmtFlt.hxx"

#define HTML_TYPE_TAG @"HTML"
#define PDF_TYPE_TAG @"PDF"
#define STRING_TYPE_TAG @"STRING"
#define URL_TYPE_TAG @"URL"
#define FILE_TYPE_TAG @"FILE"

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace vcl;

CFStringRef kNeoOfficeInternalPboardType = CFSTR( "NeoOfficeInternalPboardType" );

static sal_uInt16 nSupportedTypes = 8;

// List of supported native type symbol names in priority order. The first
// item on each line is the most current symbol name and the second item is a
// pointer to a local static NSString that the symbol address should be
// assigned to.
static NSString *aSupportedPasteboardTypeSymbolNames[] = {
	NSPasteboardTypeURL, URL_TYPE_TAG,
	NSPasteboardTypeFileURL, FILE_TYPE_TAG,
	NSPasteboardTypeRTF, nil,
	NSPasteboardTypeHTML, HTML_TYPE_TAG,
	NSPasteboardTypeString, STRING_TYPE_TAG,
	NSPasteboardTypePDF, PDF_TYPE_TAG,
	NSPasteboardTypePNG, nil,
	NSPasteboardTypeTIFF, nil
};

static NSArray *pSupportedPasteboardTypes = nil;

// Special native symbols for conditional checking
static const NSString *pFilePasteboardType = nil;
static const NSString *pHTMLPasteboardType = nil;
static const NSString *pPDFPasteboardType = nil;
static const NSString *pStringPasteboardType = nil;
static const NSString *pURLPasteboardType = nil;

// List of supported native types in priority order
static NSString *aSupportedPasteboardTypes[] = {
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil,
	nil
};

// List of supported types that are text
static bool aSupportedTextTypes[] = {
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	false
};

// List of supported mime types in priority order
static OUString aSupportedMimeTypes[] = {
	"application/x-openoffice-file;windows_formatname=\"FileName\"",
	"application/x-openoffice-file;windows_formatname=\"FileName\"",
	"text/richtext",
	"text/html",
	"text/plain;charset=utf-16",
	"image/bmp",
	"image/bmp",
	"image/bmp"
};

// List of supported data types in priority order
static ::com::sun::star::uno::Type aSupportedDataTypes[] = {
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( OUString* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 )
};

static ::std::list< DTransTransferable* > aTransferableList;

static id ImplGetDataForType( DTransTransferable *pTransferable, NSString *pType );

// ============================================================================

@interface DTransPasteboardHelper : NSObject
{
	NSData*							mpPNGData;
	NSInteger						mnChangeCount;
	NSData*							mpData;
	NSString*						mpPasteboardName;
	NSString*						mpString;
	BOOL							mbTypeAvailable;
	NSArray*						mpTypes;
}
+ (id)createWithPasteboardName:(NSString *)pPasteboardName;
- (NSInteger)changeCount;
- (void)clearContentsWithChangeCount:(NSNumber *)pChangeCount;
- (NSData *)dataForType;
- (void)dealloc;
- (void)destroyData;
- (NSArray *)filteredTypes;
- (void)flush:(NSNumber *)pChangeCount;
- (void)getPNGDataForType:(NSString *)pType;
- (void)getChangeCount:(id)pObject;
- (void)getDataForType:(NSString *)pType;
- (void)getStringForType:(NSString *)pType;
- (void)getTypeAvailable:(NSString *)pType;
- (void)getTypes:(id)pObject;
- (id)initWithPasteboardName:(NSString *)pPasteboardName;
- (BOOL)isTypeAvailable;
- (NSString *)stringForType;
- (NSArray *)types;
- (NSData *)PNGDataForType;
@end

@interface DTransPasteboardOwner : NSObject <NSPasteboardWriting>
{
	NSInteger						mnChangeCount;
	DTransTransferable*				mpTransferable;
	BOOL							mbTransferableOwner;
	NSString*						mpPasteboardName;
	NSArray*						mpTypes;
}
+ (id)createWithTransferable:(DTransTransferable *)pTransferable pasteboardName:(NSString *)pPasteboardName types:(NSArray *)pTypes;
- (NSInteger)changeCount;
- (void)dealloc;
- (id)initWithTransferable:(DTransTransferable *)pTransferable pasteboardName:(NSString *)pPasteboardName types:(NSArray *)pTypes;
- (void)pasteboard:(NSPasteboard *)pSender provideDataForType:(NSString *)pType;
- (void)pasteboardChangedOwner:(NSPasteboard *)pSender;
- (id)pasteboardPropertyListForType:(NSString *)pType;
- (void)setContents:(NSNumber *)pNumber;
- (NSArray *)writableTypesForPasteboard:(NSPasteboard *)pPasteboard;
- (NSPasteboardWritingOptions)writingOptionsForType:(NSString *)pType pasteboard:(NSPasteboard *)pPasteboard;
@end

@interface DTransTransferableDeletor : NSObject
{
	DTransTransferable*				mpTransferable;
}
+ (id)createWithTransferable:(DTransTransferable *)pTransferable;
- (void)deleteTransferable:(id)pSender;
- (id)initWithTransferable:(DTransTransferable *)pTransferable;
@end

@implementation DTransPasteboardHelper

+ (id)createWithPasteboardName:(NSString *)pPasteboardName
{
	DTransPasteboardHelper *pRet = [[DTransPasteboardHelper alloc] initWithPasteboardName:pPasteboardName];
	[pRet autorelease];
	return pRet;
}

- (NSInteger)changeCount
{
	return mnChangeCount;
}

- (void)clearContentsWithChangeCount:(NSNumber *)pChangeCount
{
	if ( !pChangeCount )
		return;

	NSPasteboard *pPasteboard = ( mpPasteboardName ? [NSPasteboard pasteboardWithName:mpPasteboardName] : [NSPasteboard generalPasteboard] );
	if ( pPasteboard && [pChangeCount intValue] == [pPasteboard changeCount] )
		[pPasteboard clearContents];
}

- (NSData *)dataForType
{
	return mpData;
}

- (void)dealloc
{
	[self destroyData];

	if ( mpPasteboardName )
		[mpPasteboardName release];

	if ( mpTypes )
		[mpTypes release];

	[super dealloc];
}

- (void)destroyData
{
	mbTypeAvailable = NO;

	if ( mpPNGData )
	{
		[mpPNGData release];
		mpPNGData = nil;
	}

	if ( mpData )
	{
		[mpData release];
		mpData = nil;
	}

	if ( mpString )
	{
		[mpString release];
		mpString = nil;
	}
}

- (NSArray *)filteredTypes
{
	NSMutableArray *pRet = nil;

	NSPasteboard *pPasteboard = ( mpPasteboardName ? [NSPasteboard pasteboardWithName:mpPasteboardName] : [NSPasteboard generalPasteboard] );
	if ( pPasteboard )
	{
		NSArray *pTypes = [pPasteboard types];
		if ( pTypes )
		{
			unsigned int nCount = [pTypes count];
			NSMutableArray *pImageTypes = [NSMutableArray arrayWithCapacity:nCount];

			pRet = [NSMutableArray arrayWithArray:pTypes];
			if ( pImageTypes && pRet )
			{
				bool bHTMLTypeFound = false;
				bool bImageTypeFound = false;
				bool bTextTypeFound = false;
				bool bStringTypeFound = false;
				bool bURLTypeFound = false;
				bool bURLTypeIsFile = false;

				unsigned int i = 0;
				for ( ; i < nCount; i++ )
				{
					NSString *pType = [pTypes objectAtIndex:i];
					if ( pType )
					{
						if ( pHTMLPasteboardType && [pHTMLPasteboardType isEqualToString:pType] )
						{
							bHTMLTypeFound = true;
						}
						else if ( pStringPasteboardType && [pStringPasteboardType isEqualToString:pType] )
						{
							bStringTypeFound = true;
						}
						else if ( ( pFilePasteboardType && [pFilePasteboardType isEqualToString:pType] ) || ( pURLPasteboardType && [pURLPasteboardType isEqualToString:pType] ) )
						{
							bURLTypeFound = true;
							NSURL *pURL = [NSURL URLFromPasteboard:pPasteboard];
							if ( pURL && [pURL isFileURL] )
								bURLTypeIsFile = true;
						}
						else
						{
							// Determine if this is a text or image type
							for ( sal_uInt16 j = 0; j < nSupportedTypes; j++ )
							{
								if ( aSupportedPasteboardTypes[ j ] && [aSupportedPasteboardTypes[ j ] isEqualToString:pType] )
								{
									if ( aSupportedTextTypes[ j ] )
									{
										bTextTypeFound = true;
									}
									else
									{
										bImageTypeFound = true;
										[pImageTypes addObject:pType];
									}
									break;
								}
							}
						}
					}

					// Safari sends images as a URL type so filter out the
					// URL type if the only text type is the URL type. Mozilla
					// sends images as the URL type and HTML when dragging so
					// so filter that as well.
					if ( bURLTypeFound && ( bHTMLTypeFound || bStringTypeFound ) && bImageTypeFound && !bTextTypeFound )
						pRet = pImageTypes;
					else if ( bURLTypeFound && !bURLTypeIsFile && pURLPasteboardType)
						[pRet removeObject:pURLPasteboardType];
				}
			}
		}
	}

	return pRet;
}

- (void)flush:(NSNumber *)pChangeCount
{
	NSPasteboard *pPasteboard = ( mpPasteboardName ? [NSPasteboard pasteboardWithName:mpPasteboardName] : [NSPasteboard generalPasteboard] );
	if ( pPasteboard && pChangeCount )
	{
		// While we have ownership, force each type to be rendered
		NSArray *pTypes = [pPasteboard types];
		if ( pTypes )
		{
			unsigned int nCount = [pTypes count];
			unsigned int i = 0;
			for ( ; i < nCount ; i++ )
			{
				if ( [pChangeCount intValue] != [pPasteboard changeCount] )
					break;

				NSString *pType = (NSString *)[pTypes objectAtIndex:i];
				if ( pType )
					[pPasteboard dataForType:pType];
			}
		}
	}
}

- (void)getPNGDataForType:(NSString *)pType
{
	[self destroyData];

	NSPasteboard *pPasteboard = ( mpPasteboardName ? [NSPasteboard pasteboardWithName:mpPasteboardName] : [NSPasteboard generalPasteboard] );
	if ( pPasteboard && pType )
	{
		@try
		{
			NSArray *pFilteredTypes = [self filteredTypes];
			if ( pFilteredTypes && [pFilteredTypes containsObject:pType] )
			{
				mbTypeAvailable = YES;

				NSData *pPasteboardData = [pPasteboard dataForType:pType];
				if ( pPasteboardData )
				{
					NSBitmapImageRep *pPasteboardImageRep = [NSBitmapImageRep imageRepWithData:pPasteboardData];
					if ( pPasteboardImageRep )
						mpPNGData = [pPasteboardImageRep representationUsingType:NSBitmapImageFileTypePNG properties:[NSDictionary dictionary]];

					if ( !mpPNGData )
					{
						NSImage *pImage = [[NSImage alloc] initWithData:pPasteboardData];
						if ( pImage )
						{
							[pImage autorelease];

							NSData *pTIFFData = [pImage TIFFRepresentation];
							if ( pTIFFData )
							{
								NSBitmapImageRep *pTIFFImageRep = [NSBitmapImageRep imageRepWithData:pTIFFData];
								if ( pTIFFImageRep )
									mpPNGData = [pTIFFImageRep representationUsingType:NSBitmapImageFileTypePNG properties:[NSDictionary dictionary]];
							}
						}
					}
				}
			}
		}
		@catch ( NSException *pExc )
		{
			NSLog( @"%@", [pExc reason] );
		}

		if ( mpPNGData )
		{
			if ( mbTypeAvailable )
				[mpPNGData retain];
			else
				mpPNGData = nil;
		}
	}
}

- (void)getChangeCount:(id)pObject
{
	(void)pObject;

	NSPasteboard *pPasteboard = ( mpPasteboardName ? [NSPasteboard pasteboardWithName:mpPasteboardName] : [NSPasteboard generalPasteboard] );
	if ( pPasteboard )
		mnChangeCount = [pPasteboard changeCount];
}

- (void)getDataForType:(NSString *)pType
{
	[self destroyData];

	NSPasteboard *pPasteboard = ( mpPasteboardName ? [NSPasteboard pasteboardWithName:mpPasteboardName] : [NSPasteboard generalPasteboard] );
	if ( pPasteboard && pType )
	{
		@try
		{
			NSArray *pFilteredTypes = [self filteredTypes];
			if ( pFilteredTypes && [pFilteredTypes containsObject:pType] )
			{
				mbTypeAvailable = YES;
				if ( ( pFilePasteboardType && [pFilePasteboardType isEqualToString:pType] ) || ( pURLPasteboardType && [pURLPasteboardType isEqualToString:pType] ) )
				{
					NSURL *pURL = [NSURL URLFromPasteboard:pPasteboard];
					if ( pURL && [pURL isFileURL] )
					{
						NSString *pPath = [pURL path];
						NSFileManager *pFileManager = [NSFileManager defaultManager];
						if ( pPath && pFileManager )
						{
							const char *pBytes = [pFileManager fileSystemRepresentationWithPath:pPath];
							if ( pBytes )
								mpData = [NSData dataWithBytes:pBytes length:strlen( pBytes )];
						}
					}
				}
				else
				{
					mpData = [pPasteboard dataForType:pType];
				}
			}
		}
		@catch ( NSException *pExc )
		{
			NSLog( @"%@", [pExc reason] );
		}

		if ( mpData )
		{
			if ( mbTypeAvailable )
				[mpData retain];
			else
				mpData = nil;
		}
	}
}

- (void)getStringForType:(NSString *)pType
{
	[self destroyData];

	NSPasteboard *pPasteboard = ( mpPasteboardName ? [NSPasteboard pasteboardWithName:mpPasteboardName] : [NSPasteboard generalPasteboard] );
	if ( pPasteboard && pType )
	{
		@try
		{
			NSArray *pFilteredTypes = [self filteredTypes];
			if ( pFilteredTypes && [pFilteredTypes containsObject:pType] )
			{
				mbTypeAvailable = YES;
				mpString = [pPasteboard stringForType:pType];
			}
		}
		@catch ( NSException *pExc )
		{
			NSLog( @"%@", [pExc reason] );
		}

		if ( mpString )
		{
			if ( mbTypeAvailable )
				[mpString retain];
			else
				mpString = nil;
		}
	}
}

- (void)getTypeAvailable:(NSString *)pType
{
	[self destroyData];

	if ( pType )
	{
		NSArray *pTypes = [self filteredTypes];
		if ( pTypes && [pTypes indexOfObject:pType] != NSNotFound )
			mbTypeAvailable = YES;
	}
}

- (void)getTypes:(id)pObject
{
	(void)pObject;

	if ( mpTypes )
	{
		[mpTypes release];
		mpTypes = nil;
	}

	mpTypes = [self filteredTypes];
	if ( mpTypes )
		[mpTypes retain];
	else
		mpTypes = nil;
}

- (id)initWithPasteboardName:(NSString *)pPasteboardName
{
	[super init];

	mnChangeCount = -1;
	mpPNGData = nil;
	mpData = nil;
	mpPasteboardName = pPasteboardName;
	if ( mpPasteboardName )
		[mpPasteboardName retain];
	mpString = nil;
	mbTypeAvailable = NO;
	mpTypes = nil;

	return self;
}

- (BOOL)isTypeAvailable
{
	return mbTypeAvailable;
}

- (NSString *)stringForType
{
	return mpString;
}

- (NSArray *)types
{
	return mpTypes;
}

- (NSData *)PNGDataForType
{
	return mpPNGData;
}

@end

@implementation DTransPasteboardOwner

+ (id)createWithTransferable:(DTransTransferable *)pTransferable pasteboardName:(NSString *)pPasteboardName types:(NSArray *)pTypes
{
	DTransPasteboardOwner *pRet = [[DTransPasteboardOwner alloc] initWithTransferable:pTransferable pasteboardName:pPasteboardName types:pTypes];
	[pRet autorelease];
	return pRet;
}

- (NSInteger)changeCount
{
	return mnChangeCount;
}

- (void)dealloc
{
	if ( mpPasteboardName )
		[mpPasteboardName release];

	if ( mpTypes )
		[mpTypes release];

	if ( mpTransferable && mbTransferableOwner )
	{
		// Fix hanging when starting a new drag on OS X 10.10 and higher by
		// deleting the transferable asynchronously
		DTransTransferableDeletor *pDeletor = [DTransTransferableDeletor createWithTransferable:mpTransferable];
		if ( pDeletor )
			osl_performSelectorOnMainThread( pDeletor, @selector(deleteTransferable:), pDeletor, NO );
	}

	[super dealloc];
}

- (id)initWithTransferable:(DTransTransferable *)pTransferable pasteboardName:(NSString *)pPasteboardName types:(NSArray *)pTypes
{
	[super init];

	mnChangeCount = -1;
	mpTransferable = pTransferable;
	mbTransferableOwner = NO;
	mpPasteboardName = pPasteboardName;
	if ( mpPasteboardName )
		[mpPasteboardName retain];
	mpTypes = pTypes;
	// Fix bug 3673 by adding a custom type when the list of native drag types
	// is empty. Fix failure to drag slides in presentation sidebar by using
	// the UTTypeCreatePreferredIdentifierForTag() function to avoid "invalid
	// UTI" errors in [DTransPasteboardOwner writeableTypesForPasteboard:].
	if ( ( !mpTypes || ![mpTypes count] ) && mpPasteboardName && [mpPasteboardName isEqualToString:@"JavaDNDPasteboardHelper"] )
		mpTypes = [NSArray arrayWithObject:(NSString *)UTTypeCreatePreferredIdentifierForTag( kUTTagClassNSPboardType, kNeoOfficeInternalPboardType, kUTTypeData )];
	if ( mpTypes )
		[mpTypes retain];

	return self;
}

- (void)pasteboard:(NSPasteboard *)pSender provideDataForType:(NSString *)pType
{
	if ( pSender && pType )
	{
		id pData = nil;
		if ( mpTransferable )
			pData = ImplGetDataForType( mpTransferable, pType );
		if ( !pData )
			pData = [NSData data];
		if ( pData )
		{
			if ( [pData isKindOfClass:[NSString class]] )
				[pSender setString:(NSString *)pData forType:pType];
			else if ( [pData isKindOfClass:[NSData class]] )
				[pSender setData:(NSData *)pData forType:pType];
		}
	}
}

- (void)pasteboardChangedOwner:(NSPasteboard *)pSender
{
	// Fix crash due to multiple calls to this selector that occur when this
	// object is used as an NSDraggingItem by only releasing when the sender
	// is the same pasteboard used to create this object. Attempt to fix Mac
	// App Store crash by delaying the release of the clipboard owner.
	NSPasteboard *pPasteboard = ( mpPasteboardName ? [NSPasteboard pasteboardWithName:mpPasteboardName] : [NSPasteboard generalPasteboard] );
	if ( pPasteboard == pSender )
		[self performSelector:@selector(release) withObject:nil afterDelay:0];
}

- (id)pasteboardPropertyListForType:(NSString *)pType
{
	id pRet = nil;

	if ( pType )
	{
		if ( mpTransferable )
			pRet = ImplGetDataForType( mpTransferable, pType );
		if ( !pRet )
			pRet = [NSData data];
	}

	return pRet;
}

- (void)setContents:(NSNumber *)pNumber
{
	mnChangeCount = -1;

	NSPasteboard *pPasteboard = ( mpPasteboardName ? [NSPasteboard pasteboardWithName:mpPasteboardName] : [NSPasteboard generalPasteboard] );
	if ( pPasteboard )
	{
		// Retain this object as once it becomes the pasteboard owner, we
		// must keep this object alive until the pasteboardChangedOwner:
		// selector is called
		[self retain];
		mnChangeCount = [pPasteboard declareTypes:mpTypes owner:self];
		if ( mnChangeCount >= 0 )
		{
			if ( pNumber )
				mbTransferableOwner = [pNumber boolValue];
		}
		else
		{
			// Attempt to fix Mac App Store crash by delaying the release of
			// the clipboard owner
			[self performSelector:@selector(release) withObject:nil afterDelay:0];
		}
	}
}

- (NSArray *)writableTypesForPasteboard:(NSPasteboard *)pPasteboard
{
	(void)pPasteboard;

	return mpTypes;
}

- (NSPasteboardWritingOptions)writingOptionsForType:(NSString *)pType pasteboard:(NSPasteboard *)pPasteboard
{
	(void)pType;
	(void)pPasteboard;

	return NSPasteboardWritingPromised;
}

@end

@implementation DTransTransferableDeletor

+ (id)createWithTransferable:(DTransTransferable *)pTransferable
{
	DTransTransferableDeletor *pRet = [[DTransTransferableDeletor alloc] initWithTransferable:pTransferable];
	[pRet autorelease];
	return pRet;
}

- (void)deleteTransferable:(id)pSender
{
	(void)pSender;

	if ( mpTransferable )
	{
		comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			delete mpTransferable;
			mpTransferable = nil;
		}
		rSolarMutex.release();
	}
}

- (id)initWithTransferable:(DTransTransferable *)pTransferable
{
	[super init];

	mpTransferable = pTransferable;

	return self;
}

@end

// ============================================================================

static void ImplInitializeSupportedPasteboardTypes()
{
	if ( pSupportedPasteboardTypes )
		return;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSMutableArray *pTypes = [NSMutableArray arrayWithCapacity:nSupportedTypes];
	if ( pTypes )
	{
		NSBundle *pBundle = [NSBundle bundleForClass:[NSPasteboard class]];
		if ( pBundle )
		{
			CFBundleRef aBundle = CFBundleGetBundleWithIdentifier( (CFStringRef)[pBundle bundleIdentifier] );
			if ( aBundle )
			{
				for ( sal_uInt16 i = 0; i < nSupportedTypes; i++ )
				{
					aSupportedPasteboardTypes[ i ] = aSupportedPasteboardTypeSymbolNames[ i * 2 ];

					// Assign the symbol to the matching local static NSString
					NSString *pLocalName = aSupportedPasteboardTypeSymbolNames[ ( i * 2 ) + 1 ];
					if ( pLocalName )
					{
						if ( [HTML_TYPE_TAG isEqualToString:pLocalName] )
							pHTMLPasteboardType = aSupportedPasteboardTypes[ i ];
						else if ( [PDF_TYPE_TAG isEqualToString:pLocalName] )
							pPDFPasteboardType = aSupportedPasteboardTypes[ i ];
						else if ( [STRING_TYPE_TAG isEqualToString:pLocalName] )
							pStringPasteboardType = aSupportedPasteboardTypes[ i ];
						else if ( [URL_TYPE_TAG isEqualToString:pLocalName] )
							pURLPasteboardType = aSupportedPasteboardTypes[ i ];
						else if ( [FILE_TYPE_TAG isEqualToString:pLocalName] )
							pFilePasteboardType = aSupportedPasteboardTypes[ i ];
					}

					// Add to supported types array
					if ( aSupportedPasteboardTypes[ i ] )
						[pTypes addObject:aSupportedPasteboardTypes[ i ]];
				}
			}
		}

		// Fix bug 3673 by adding a custom type to the list of native types.
		// Fix failure to drag slides in presentation sidebar by using the
		// UTTypeCreatePreferredIdentifierForTag() function to avoid "invalid
		// UTI" errors in [DTransPasteboardOwner writeableTypesForPasteboard:].
		[pTypes addObject:(NSString *)UTTypeCreatePreferredIdentifierForTag( kUTTagClassNSPboardType, kNeoOfficeInternalPboardType, kUTTypeData )];

		[pTypes retain];
		pSupportedPasteboardTypes = pTypes;
	}

	[pPool release];
}

// ----------------------------------------------------------------------------

static id ImplGetDataForType( DTransTransferable *pTransferable, NSString *pType )
{
	id pRet = nil;

	if ( pTransferable && pType && !Application::IsShutDown() )
	{
		comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			bool bTransferableFound = false;
			if ( pTransferable )
			{
				for ( ::std::list< DTransTransferable* >::const_iterator it = aTransferableList.begin(); it != aTransferableList.end(); ++it )
				{
					if ( pTransferable == *it )
					{
						bTransferableFound = true;
						break;
					}
				}
			}

			if ( bTransferableFound )
			{
				bool bFlavorFound = false;
				bool bFlavorIsText = false;
				DataFlavor aFlavor;
				for ( sal_uInt16 i = 0; i < nSupportedTypes; i++ )
				{
					if ( aSupportedPasteboardTypes[ i ] && [aSupportedPasteboardTypes[ i ] isEqualToString:pType] )
					{
						aFlavor.MimeType = aSupportedMimeTypes[ i ];
						aFlavor.DataType = aSupportedDataTypes[ i ];
						if ( pTransferable->isDataFlavorSupported( aFlavor ) )
						{
							bFlavorFound = true;
							bFlavorIsText = aSupportedTextTypes[ i ];
							break;
						}
					}
				}

				if ( bFlavorFound )
				{
					bool bDataFound = false;
					Any aValue;
					try {
						aValue = pTransferable->getTransferData( aFlavor );
						bDataFound = true;
					}
					catch ( ... )
					{
					}

					if ( bDataFound )
					{
						if ( aValue.getValueType().equals( getCppuType( ( OUString* )0 ) ) )
						{
							OUString aString;
							aValue >>= aString;
							sal_Unicode *pArray = (sal_Unicode *)aString.getStr();
							sal_Int32 nLen = aString.getLength();
							if ( pArray && nLen )
							{
								NSString *pData = [NSString stringWithCharacters:pArray length:nLen];
								if ( pData )
									pRet = pData;
							}
						}
						else if ( aValue.getValueType().equals( getCppuType( ( Sequence< sal_Int8 >* )0 ) ) )
						{
							Sequence< sal_Int8 > aData;
							aValue >>= aData;

							// Fix bug 3640 by not adding the Microsoft Office
							// HTML headers. Microsoft Office 2004 and 2008 do
							// not appear to need those headers and the headers
							// are not understood by other applications.
							sal_Int8 *pArray = aData.getArray();
							sal_Int32 nLen = aData.getLength();
							if ( pArray && nLen )
							{
								NSData *pData = [NSData dataWithBytes:pArray length:nLen];
								if ( pData )
								{
									if ( !bFlavorIsText )
									{
										// Convert from our BMP data
										Class aClass = [NSImageRep imageRepClassForType:pType];
										if ( aClass && [aClass canInitWithData:pData] )
										{
											NSImageRep *pImageRep = [aClass imageRepWithData:pData];
											if ( pImageRep )
											{
												if ( [pImageRep isKindOfClass:[NSPDFImageRep class]] )
													pData = [(NSPDFImageRep *)pImageRep PDFRepresentation];
												else if ( [pImageRep isKindOfClass:[NSEPSImageRep class]] )
													pData = [(NSEPSImageRep *)pImageRep EPSRepresentation];
												else if ( [pImageRep isKindOfClass:[NSBitmapImageRep class]] )
													pData = [(NSBitmapImageRep *)pImageRep TIFFRepresentation];
												else
													pData = nil;
											}
										}
									}

									if ( pData )
										pRet = pData;
								}
							}
						}
					}
				}
			}
		}

		rSolarMutex.release();
	}

	return pRet;
}

// ============================================================================

NSArray *DTransTransferable::getSupportedPasteboardTypes()
{
	ImplInitializeSupportedPasteboardTypes();

	return pSupportedPasteboardTypes;
}

// ----------------------------------------------------------------------------

void DTransTransferable::flush()
{
	// Force transferable to render data if we still have ownership
	if ( mxTransferable.is() && mnChangeCount >= 0 )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		DTransPasteboardHelper *pHelper = [DTransPasteboardHelper createWithPasteboardName:mpPasteboardName];
		if ( pHelper )
			osl_performSelectorOnMainThread( pHelper, @selector(flush:), [NSNumber numberWithInt:mnChangeCount], YES );

		[pPool release];
	}
}

// ----------------------------------------------------------------------------

NSInteger DTransTransferable::getChangeCount()
{
	return mnChangeCount;
}

// ----------------------------------------------------------------------------

Any DTransTransferable::getTransferData( const DataFlavor& aFlavor ) throw ( UnsupportedFlavorException, IOException, RuntimeException, std::exception )
{
	if ( mxTransferable.is() )
		return mxTransferable->getTransferData( aFlavor );

	Any out;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	bool bDataAvailable = false;
	bool bDataRetrieved = false;
	DTransPasteboardHelper *pHelper = [DTransPasteboardHelper createWithPasteboardName:mpPasteboardName];
	if ( pHelper )
	{
		NSString *pRequestedType = nil;
		bool bRequestedTypeIsText = false;

		// Run a loop so that if data type fails, we can try another
		for ( sal_uInt16 i = 0; !bDataRetrieved && i < nSupportedTypes; i++ )
		{
			if ( aSupportedPasteboardTypes[ i ] && aFlavor.MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ i ] ) )
			{
				pRequestedType = aSupportedPasteboardTypes[ i ];
				bRequestedTypeIsText = aSupportedTextTypes[ i ];
			}
			else
			{
				continue;
			}

			if ( aFlavor.DataType.equals( getCppuType( ( OUString* )0 ) ) )
			{
				osl_performSelectorOnMainThread( pHelper, @selector(getStringForType:), pRequestedType, YES );
				bDataAvailable = [pHelper isTypeAvailable];
				NSString *pString = [pHelper stringForType];
				if ( pString )
				{
					NSUInteger nLen = [pString length];
					OUStringBuffer aExportData( nLen );

					// Replace carriage returns with line feeds
					unsigned int nCopiedChars = 0;
					for ( NSUInteger j = 0; j < nLen; j++, nCopiedChars++ )
					{
						unichar nChar = [pString characterAtIndex:j];
						if ( nChar == (unichar)'\r' )
						{
							aExportData.append( (sal_Unicode)'\n' );

							// Replace \r\n with a single newline
							if ( j < nLen - 1 && [pString characterAtIndex:j + 1] == (unichar)'\n' )
								j++;
						}
						else
						{
								aExportData.append( (sal_Unicode)nChar );
						}
					}

					OUString aString( aExportData.makeStringAndClear() );
					out <<= aString;
					bDataRetrieved = true;
				}
			}
			else if ( aFlavor.DataType.equals( getCppuType( ( Sequence< sal_Int8 >* )0 ) ) )
			{
				if ( bRequestedTypeIsText )
				{
					osl_performSelectorOnMainThread( pHelper, @selector(getDataForType:), pRequestedType, YES );
					bDataAvailable = [pHelper isTypeAvailable];
					NSData *pData = [pHelper dataForType];
					if ( pData )
					{
						const char *pArray = (const char *)[pData bytes];
						unsigned int nLen = [pData length];

						if ( pArray && nLen )
						{
							Sequence< sal_Int8 > aExportData( nLen );

							// Replace carriage returns with line feeds
							sal_Char *pCharArray = (sal_Char *)aExportData.getArray();
							if ( pCharArray )
							{
								memset( pCharArray, 0, nLen );

								unsigned int nCopiedChars = 0;
								for ( unsigned int j = 0; j < nLen; j++, nCopiedChars++ )
								{
									if ( pArray[ j ] == '\r' )
									{
										pCharArray[ nCopiedChars ] = '\n';

										// Replace \r\n with a single newline
										if ( j < nLen - 1 && pArray[ j + 1 ] == '\n' )
											j++;
									}
									else
									{
										pCharArray[ nCopiedChars ] = pArray[ j ];
									}
								}

								aExportData.realloc( nCopiedChars );
							}

							// Strip out HTML Microsoft Office headers
							if ( pHTMLPasteboardType && [pHTMLPasteboardType isEqualToString:pRequestedType] && isHTMLFormat( aExportData ) )
								aExportData = HTMLFormatToTextHtml( aExportData );

							out <<= aExportData;
							bDataRetrieved = true;
						}
					}
				}
				else
				{
					// Convert to PNG data even though the OpenOffice.org
					// code asks for BMP data as we have made some changes
					// to the vcl/source/gdi/bitmap2.cxx code to accept
					// PNG data
					osl_performSelectorOnMainThread( pHelper, @selector(getPNGDataForType:), pRequestedType, YES );
					bDataAvailable = [pHelper isTypeAvailable];
					NSData *pData = [pHelper PNGDataForType];
					if ( pData )
					{
						const void *pArray = [pData bytes];
						unsigned int nLen = [pData length];

						if ( pArray && nLen )
						{
							Sequence< sal_Int8 > aExportData( nLen );
							memcpy( aExportData.getArray(), pArray, nLen );

							out <<= aExportData;
							bDataRetrieved = true;
						}
					}
				}
			}
		}
	}

	[pPool release];

	if ( !bDataRetrieved )
	{
		if ( bDataAvailable )
			throw IOException( aFlavor.MimeType, static_cast< XTransferable * >( this ) );
		else
			throw UnsupportedFlavorException( aFlavor.MimeType, static_cast< XTransferable * >( this ) );
	}

	return out;
}

// ----------------------------------------------------------------------------

DTransTransferable::DTransTransferable( NSString *pPasteboardName ) :
	mnChangeCount( -1 ),
	mpPasteboardName( pPasteboardName )
{
	ImplInitializeSupportedPasteboardTypes();

	if ( mpPasteboardName )
		[mpPasteboardName retain];

	aTransferableList.push_back( this );
}

// ----------------------------------------------------------------------------

DTransTransferable::~DTransTransferable()
{
	aTransferableList.remove( this );

	// If this object is the pasteboard owner, clear the pasteboard's contents
	// Avoid deadlock reported in the following NeoOffice forum topic
	// by clearing the pasteboard's contents is this object is the pasteboard
	// owner:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8508
	if ( mxTransferable.is() && mnChangeCount >= 0 )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		DTransPasteboardHelper *pHelper = [DTransPasteboardHelper createWithPasteboardName:mpPasteboardName];
		if ( pHelper )
			osl_performSelectorOnMainThread( pHelper, @selector(clearContentsWithChangeCount:), [NSNumber numberWithInt:mnChangeCount], YES );

		[pPool release];
	}

	if ( mpPasteboardName )
		[mpPasteboardName release];
}

// ----------------------------------------------------------------------------

Sequence< DataFlavor > DTransTransferable::getTransferDataFlavors() throw ( RuntimeException, std::exception )
{
	if ( mxTransferable.is() )
		return mxTransferable->getTransferDataFlavors();

	Sequence< DataFlavor > out;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	DTransPasteboardHelper *pHelper = [DTransPasteboardHelper createWithPasteboardName:mpPasteboardName];
	if ( pHelper )
	{
		osl_performSelectorOnMainThread( pHelper, @selector(getTypes:), pHelper, YES );
		NSArray *pTypes = [pHelper types];
		if ( pTypes )
		{
			for ( sal_uInt16 i = 0; i < nSupportedTypes; i++ )
			{
				unsigned int nCount = [pTypes count];
				unsigned int j = 0;
				for ( ; j < nCount; j++ )
				{
					NSString *pType = (NSString *)[pTypes objectAtIndex:j];
					if ( pType && aSupportedPasteboardTypes[ i ] && [aSupportedPasteboardTypes[ i ] isEqualToString:pType] )
					{
						DataFlavor aFlavor;
						aFlavor.MimeType = aSupportedMimeTypes[ i ];
						aFlavor.DataType = aSupportedDataTypes[ i ];
						sal_Int32 nLen = out.getLength();
						out.realloc( nLen + 1 );
						out[ nLen ] = aFlavor;
					}
				}
			}
		}
	}

	[pPool release];

	return out;
}

// ----------------------------------------------------------------------------

sal_Bool DTransTransferable::hasOwnership()
{
	sal_Bool out = sal_False;

	if ( mxTransferable.is() && mnChangeCount >= 0 )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		DTransPasteboardHelper *pHelper = [DTransPasteboardHelper createWithPasteboardName:mpPasteboardName];
		if ( pHelper )
		{
			osl_performSelectorOnMainThread( pHelper, @selector(getChangeCount:), pHelper, YES );
			if ( [pHelper changeCount] == mnChangeCount )
				out = sal_True;
		}

		[pPool release];
	}

	return out;
}

// ----------------------------------------------------------------------------

sal_Bool DTransTransferable::isDataFlavorSupported( const DataFlavor& aFlavor ) throw ( RuntimeException, std::exception )
{
	if ( mxTransferable.is() )
		return mxTransferable->isDataFlavorSupported( aFlavor );

	sal_Bool out = sal_False;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSString *pRequestedType = nil;
	for ( sal_uInt16 i = 0; i < nSupportedTypes; i++ )
	{
		if ( aFlavor.MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ i ] ) )
		{
			pRequestedType = aSupportedPasteboardTypes[ i ];
			break;
		}
	}

	if ( pRequestedType )
	{
		DTransPasteboardHelper *pHelper = [DTransPasteboardHelper createWithPasteboardName:mpPasteboardName];
		if ( pHelper )
		{
			osl_performSelectorOnMainThread( pHelper, @selector(getTypeAvailable:), pRequestedType, YES );
			if ( [pHelper isTypeAvailable] )
				out = sal_True;
		}
	}

	[pPool release];

	return out;
}

// ----------------------------------------------------------------------------

sal_Bool DTransTransferable::setContents( const Reference< XTransferable > &xTransferable, id *pPasteboardWriter )
{
	sal_Bool out = sal_False;

	if ( pPasteboardWriter )
		*pPasteboardWriter = nil;

	mxTransferable = xTransferable;
	if ( mxTransferable.is() )
	{
		Sequence< DataFlavor > xFlavors;
		try
		{
			xFlavors = mxTransferable->getTransferDataFlavors();
		}
		catch ( ... )
		{
		}

		// Check if text flavors are supported, if so, exclude any
		// image flavors since we would be just passing a picture
		// of text
		sal_Int32 nLen = xFlavors.getLength();
		bool bTextOnly = false;
		sal_Int32 i;
		for ( i = 0; i < nLen; i++ )
		{
			for ( sal_uInt16 j = 0; j < nSupportedTypes; j++ )
			{
				if ( xFlavors[ i ].MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ j ] ) && aSupportedTextTypes[ j ] )
				{
					bTextOnly = true;
					break;
				}
			}
		}

		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		NSMutableArray *pTypes = [NSMutableArray arrayWithCapacity:nLen];
		if ( pTypes )
		{
			for ( i = 0; i < nLen; i++ )
			{ 
				for ( sal_uInt16 j = 0; j < nSupportedTypes; j++ )
				{
					if ( xFlavors[ i ].MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ j ] ) )
					{
						if ( bTextOnly && !aSupportedTextTypes[ j ] )
							continue;

						// Converting to PDF does not work (only converting
						// from PDF works) so don't add the PDF flavor
						if ( !aSupportedPasteboardTypes[ j ] || ( pPDFPasteboardType && [pPDFPasteboardType isEqualToString:aSupportedPasteboardTypes[ j ]]  ) )
							continue;

						[pTypes addObject:aSupportedPasteboardTypes[ j ]];
					}
				}
			}

			// Do not retain the object as invoking the setContents: selector
			// retains it and it will be released in the object's
			// pasteboardChangedOwner: selector
			DTransPasteboardOwner *pOwner = [DTransPasteboardOwner createWithTransferable:this pasteboardName:mpPasteboardName types:pTypes];
			if ( pOwner )
			{
				osl_performSelectorOnMainThread( pOwner, @selector(setContents:), [NSNumber numberWithBool:( pPasteboardWriter ? YES : NO )], YES );
				mnChangeCount = [pOwner changeCount];
				if ( mnChangeCount >= 0 )
				{
					out = sal_True;
					if ( pPasteboardWriter )
					{
						// Retain before it released by the autorelease pool
						[pOwner retain];
						*pPasteboardWriter = pOwner;
					}
				}
			}
		}

		[pPool release];
	}

	return out;
}

// ----------------------------------------------------------------------------

void DTransTransferable::updateChangeCount()
{
	if ( !mxTransferable.is() && mnChangeCount < 0 )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		DTransPasteboardHelper *pHelper = [DTransPasteboardHelper createWithPasteboardName:mpPasteboardName];
		if ( pHelper )
		{
			osl_performSelectorOnMainThread( pHelper, @selector(getChangeCount:), pHelper, YES );
			mnChangeCount = [pHelper changeCount];
		}

		[pPool release];
	}
}

