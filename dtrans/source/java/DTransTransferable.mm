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

#ifndef _DTRANSCLIPBOARD_HXX
#include "DTransClipboard.hxx"
#endif
#ifndef _DTRANSTRANSFERABLE_HXX
#include "DTransTransferable.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include "HtmlFmtFlt.hxx"

#include <premac.h>
#import <AppKit/AppKit.h>
#import <QuickTime/QuickTime.h>
#include <postmac.h>

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace java;
using namespace rtl;
using namespace vcl;
using namespace vos;

static UInt32 nSupportedTypes = 9;

// List of supported native types in priority order
static const NSString *aSupportedPasteboardTypes[] = {
	// Mark NSURLPboardType as text to ensure that it is the preferred flavor
	NSURLPboardType,
	NSRTFPboardType,
	NSHTMLPboardType,
	NSStringPboardType,
	nil, // NSPasteboard has no matching non-unicode text type
	NSPDFPboardType,
	nil, // NSPasteboard has no matching PNG image format until Mac OS X 10.6
	NSTIFFPboardType,
	NSPICTPboardType
};

// List of supported native types in priority order
static FourCharCode aSupportedNativeTypes[] = {
	// Mark 'furl' as text to ensure that it is the preferred flavor
	'furl',
	'RTF ',
	'HTML',
	'utxt',
	kQTFileTypeText,
	kQTFileTypePDF,
	kQTFileTypePNG,
	kQTFileTypeTIFF,
	kQTFileTypePicture
};

// List of supported types that are text
static bool aSupportedTextTypes[] = {
	// Mark NSURLPboardType as text to ensure that it is the preferred flavor
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	false,
	false
};

// List of supported mime types in priority order
static OUString aSupportedMimeTypes[] = {
	OUString::createFromAscii( "application/x-openoffice-file;windows_formatname=\"FileName\"" ),
	OUString::createFromAscii( "text/richtext" ),
	OUString::createFromAscii( "text/html" ),
	OUString::createFromAscii( "text/plain;charset=utf-16" ),
	OUString::createFromAscii( "text/plain;charset=utf-16" ),
	OUString::createFromAscii( "image/bmp" ),
	OUString::createFromAscii( "image/bmp" ),
	OUString::createFromAscii( "image/bmp" ),
	OUString::createFromAscii( "image/bmp" )
};

// List of supported data types in priority order
static ::com::sun::star::uno::Type aSupportedDataTypes[] = {
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( OUString* )0 ),
	getCppuType( ( OUString* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
};

static ::std::list< DTransTransferable* > aTransferableList;
static DragSendDataUPP pDragSendDataUPP = NULL;

static id ImplGetDataForType( DTransTransferable *pTransferable, const NSString *pType );

// ============================================================================

@interface DTransPasteboardHelper : NSObject
{
	NSData*							mpBMPData;
	int								mnChangeCount;
	NSData*							mpData;
	const NSString*					mpPasteboardName;
	NSString*						mpString;
	BOOL							mbTypeAvailable;
	const NSArray*					mpTypes;
}
- (NSData *)BMPDataForType;
- (int)changeCount;
- (NSData *)dataForType;
- (void)dealloc;
- (void)destroyData;
- (void)flush:(NSNumber *)pChangeCount;
- (void)getBMPDataForType:(NSString *)pType;
- (void)getChangeCount:(id)pObject;
- (void)getDataForType:(NSString *)pType;
- (void)getStringForType:(NSString *)pType;
- (void)getTypeAvailable:(NSString *)pType;
- (void)getTypes:(id)pObject;
- (id)initWithPasteboardType:(int)nTransferableType;
- (BOOL)isTypeAvailable;
- (NSString *)stringForType;
- (const NSArray *)types;
@end

@implementation DTransPasteboardHelper

- (NSData *)BMPDataForType
{
	return mpBMPData;
}

- (int)changeCount
{
	return mnChangeCount;
}

- (NSData *)dataForType
{
	return mpData;
}

- (void)dealloc
{
	[self destroyData];

	if ( mpTypes )
		[mpTypes release];

	[super dealloc];
}

- (void)flush:(NSNumber *)pChangeCount;
{
	if ( mpPasteboardName && pChangeCount )
	{
		NSPasteboard *pPasteboard = [NSPasteboard pasteboardWithName:mpPasteboardName];
		if ( pPasteboard )
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
}

- (void)destroyData
{
	mbTypeAvailable = NO;

	if ( mpBMPData )
	{
		[mpBMPData release];
		mpBMPData = nil;
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

- (void)getBMPDataForType:(NSString *)pType
{
	[self destroyData];

	if ( mpPasteboardName && pType )
	{
		NSPasteboard *pPasteboard = [NSPasteboard pasteboardWithName:mpPasteboardName];
		if ( pPasteboard )
		{
			@try
			{
				if ( [pPasteboard availableTypeFromArray:[NSArray arrayWithObject:pType]] )
				{
					mbTypeAvailable = YES;

					NSData *pPasteboardData = [pPasteboard dataForType:pType];
					if ( pPasteboardData )
					{
						NSImage *pImage = [[NSImage alloc] initWithData:pPasteboardData];
						if ( pImage )
						{
							[pImage autorelease];

							NSData *pTIFFData = [pImage TIFFRepresentation];
							if ( pTIFFData )
							{
								NSBitmapImageRep *pImageRep = [NSBitmapImageRep imageRepWithData:pTIFFData];
								if ( pImageRep )
									mpBMPData = [pImageRep representationUsingType:NSBMPFileType properties:nil];
							}
						}
					}
				}
			}
			@catch ( NSException *pExc )
			{
				NSLog( @"%@", [pExc reason] );
			}

			if ( mpBMPData )
			{
				if ( mbTypeAvailable )
					[mpBMPData retain];
				else
					mpBMPData = nil;
			}
		}
	}
}

- (void)getChangeCount:(id)pObject
{
	if ( mpPasteboardName )
	{
		NSPasteboard *pPasteboard = [NSPasteboard pasteboardWithName:mpPasteboardName];
		if ( pPasteboard )
			mnChangeCount = [pPasteboard changeCount];
	}
}

- (void)getDataForType:(NSString *)pType
{
	[self destroyData];

	if ( mpPasteboardName && pType )
	{
		NSPasteboard *pPasteboard = [NSPasteboard pasteboardWithName:mpPasteboardName];
		if ( pPasteboard )
		{
			@try
			{
				if ( [pPasteboard availableTypeFromArray:[NSArray arrayWithObject:pType]] )
				{
					mbTypeAvailable = YES;
					mpData = [pPasteboard dataForType:pType];
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
}

- (void)getStringForType:(NSString *)pType
{
	[self destroyData];

	if ( mpPasteboardName && pType )
	{
		NSPasteboard *pPasteboard = [NSPasteboard pasteboardWithName:mpPasteboardName];
		if ( pPasteboard )
		{
			@try
			{
				if ( [pPasteboard availableTypeFromArray:[NSArray arrayWithObject:pType]] )
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
}

- (void)getTypeAvailable:(NSString *)pType
{
	mbTypeAvailable = NO;

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

	if ( mpPasteboardName && pType )
	{
		NSPasteboard *pPasteboard = [NSPasteboard pasteboardWithName:mpPasteboardName];
		if ( pPasteboard && [pPasteboard availableTypeFromArray:[NSArray arrayWithObject:pType]] )
			mbTypeAvailable = YES;
	}
}

- (void)getTypes:(id)pObject
{
	if ( mpTypes )
	{
		[mpTypes release];
		mpTypes = nil;
	}

	if ( mpPasteboardName )
	{
		NSPasteboard *pPasteboard = [NSPasteboard pasteboardWithName:mpPasteboardName];
		if ( pPasteboard )
		{
			mpTypes = [pPasteboard types];
			if ( mpTypes )
				[mpTypes retain];
		}
	}
}

- (id)initWithPasteboardType:(int)nTransferableType
{
	[super init];

	mnChangeCount = -1;
	mpBMPData = nil;
	mpData = nil;
	if ( nTransferableType == TRANSFERABLE_TYPE_DRAG )
		mpPasteboardName = NSDragPboard;
	else
		mpPasteboardName = NSGeneralPboard;
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

- (const NSArray*)types
{
	return mpTypes;
}

@end

@interface DTransPasteboardOwner : NSObject
{
	int								mnChangeCount;
	DTransTransferable*				mpTransferable;
	const NSString*					mpPasteboardName;
	const NSArray*					mpTypes;
}
- (int)changeCount;
- (void)dealloc;
- (id)initWithTransferable:(DTransTransferable *)pTransferable pasteboardType:(int)nTransferableType types:(const NSArray *)pTypes;
- (void)pasteboard:(NSPasteboard *)pSender provideDataForType:(NSString *)pType;
- (void)pasteboardChangedOwner:(NSPasteboard *)pSender;
- (void)setContents:(id)pObject;
@end

@implementation DTransPasteboardOwner

- (int)changeCount
{
	return mnChangeCount;
}

- (void)dealloc
{
	if ( mpTypes )
		[mpTypes release];
	
	[super dealloc];
}

- (id)initWithTransferable:(DTransTransferable *)pTransferable pasteboardType:(int)nTransferableType types:(const NSArray *)pTypes
{
	[super init];

	mnChangeCount = -1;
	mpTransferable = pTransferable;
	if ( nTransferableType == TRANSFERABLE_TYPE_DRAG )
		mpPasteboardName = NSDragPboard;
	else
		mpPasteboardName = NSGeneralPboard;
	mpTypes = pTypes;
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
	[self release];
}

- (void)setContents:(id)pObject
{
	if ( mpPasteboardName && mpTypes )
	{
		NSPasteboard *pPasteboard = [NSPasteboard pasteboardWithName:mpPasteboardName];
		if ( pPasteboard )
			mnChangeCount = [pPasteboard declareTypes:mpTypes owner:self];
	}
}

@end

// ============================================================================

static id ImplGetDataForType( DTransTransferable *pTransferable, const NSString *pType )
{
	id pRet = nil;

	if ( pTransferable && pType && !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
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
				for ( USHORT i = 0; i < nSupportedTypes; i++ ) {
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
										Class aClass = [NSImageRep imageRepClassForPasteboardType:pType];
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

			rSolarMutex.release();
		}
	}

	return pRet;
}

// ----------------------------------------------------------------------------

static OSErr ImplDragSendDataCallback( FlavorType nType, void *pData, DragItemRef aItem, DragRef aDrag )
{
	OSStatus nErr = cantGetFlavorErr;

	DTransTransferable *pTransferable = (DTransTransferable *)pData;

	if ( pTransferable && !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
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
				for ( USHORT i = 0; i < nSupportedTypes; i++ ) {
					if ( nType == aSupportedNativeTypes[ i ] )
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
								if ( nType == kQTFileTypeText )
								{
									CFStringRef aCFString = CFStringCreateWithCharactersNoCopy( kCFAllocatorDefault, pArray, nLen, kCFAllocatorNull );
									if ( aCFString )
									{
										CFIndex nBufLen;
										CFRange aRange;
										aRange.location = 0;
										aRange.length = CFStringGetLength( aCFString );
										if ( CFStringGetBytes( aCFString, aRange, CFStringGetSystemEncoding(), '?', false, NULL, 0, &nBufLen ) )
										{
											CFIndex nRealLen = nBufLen;
											// Place on to heap, not stack.
											UInt8 *aBuf = new UInt8[ nBufLen + 1 ];
											if ( aBuf && CFStringGetBytes( aCFString, aRange, CFStringGetSystemEncoding(), '?', false, aBuf, nBufLen, &nRealLen ) && nRealLen == nBufLen )
											{
												aBuf[ nBufLen ] = '\0';

												// Replace line feeds with
												// carriage returns but for only
												// very old applications
												for ( int j = 0; j < nBufLen; j++ )
												{
													if ( aBuf[ j ] == '\n' )
														aBuf[ j ] = '\r';
												}

												nErr = SetDragItemFlavorData( aDrag, (DragItemRef)pData, nType, (const void *)aBuf, nBufLen, 0 );
												delete aBuf;
											}
										}

										CFRelease( aCFString );
									}
								}
								else
								{
 									nLen *= sizeof( sal_Unicode );
									nErr = SetDragItemFlavorData( aDrag, (DragItemRef)pData, nType, (const void *)aString.getStr(), nLen, 0 );
								}
							}
						}
						else if ( aValue.getValueType().equals( getCppuType( ( Sequence< sal_Int8 >* )0 ) ) )
						{
							Sequence< sal_Int8 > aData;
							aValue >>= aData;

							// Fix bug 3640 by not adding the Microsoft Office
							// HTML headers. Microsoft Office 2004 and 2008 do
							// appear to need those headers and the headers are
							// not understood by other applications.
							sal_Int8 *pArray = aData.getArray();
							sal_Int32 nLen = aData.getLength();
							if ( pArray && nLen )
							{
								if ( !bFlavorIsText )
								{
									// Convert from our BMP data
									ComponentInstance aImporter;
									if ( OpenADefaultComponent( GraphicsImporterComponentType, kQTFileTypeBMP, &aImporter ) == noErr )
									{
										Handle hData;
										if ( PtrToHand( pArray, &hData, nLen ) == noErr )
										{
											// Free the source data
											aData = Sequence< sal_Int8 >();

											Rect aBounds;
											if ( GetHandleSize( hData ) == nLen && GraphicsImportSetDataHandle( aImporter, hData ) == noErr && GraphicsImportGetNaturalBounds( aImporter, &aBounds ) == noErr )
											{
												if ( nType == kQTFileTypePicture )
												{
													Handle hPict;
													if ( GraphicsImportGetAsPicture( aImporter, (PicHandle *)&hPict ) == noErr )
													{
														HLock( hPict );
														nErr = SetDragItemFlavorData( aDrag, (DragItemRef)pData, nType, (const void *)*hPict, GetHandleSize( (Handle)hPict ), 0 );
														HUnlock( hPict );

														DisposeHandle( hPict );
													}
												}
												else
												{
													GWorldPtr aGWorld;
													if ( QTNewGWorld( &aGWorld, k32ARGBPixelFormat, &aBounds, NULL, NULL, 0 ) == noErr )
													{
														if ( GraphicsImportSetGWorld( aImporter, aGWorld, NULL ) == noErr && GraphicsImportDraw( aImporter ) == noErr )
														{
															ComponentInstance aExporter;
															if ( OpenADefaultComponent( GraphicsExporterComponentType, nType, &aExporter ) == noErr )
															{
																if ( GraphicsExportSetInputGWorld( aExporter, aGWorld ) == noErr )
																{
																	Handle hExportData = NewHandle( 0 );
																	if ( GraphicsExportSetOutputHandle( aExporter, hExportData ) == noErr )
																	{
																		ULONG nDataLen;
																		if ( GraphicsExportDoExport( aExporter, &nDataLen ) == noErr )
																		{
																			Sequence< sal_Int8 > aExportData( nDataLen );
																			HLock( hExportData );
																			nErr = SetDragItemFlavorData( aDrag, (DragItemRef)pData, nType, (const void *)*hExportData, nDataLen, 0 );
																			HUnlock( hExportData );
																		}
																	}

																	DisposeHandle( hExportData );
																}

																CloseComponent( aExporter );
															}
														}

														DisposeGWorld( aGWorld );
													}
												}
											}

											DisposeHandle( hData );
										}

										CloseComponent( aImporter );
									}
								}
								else if ( pArray && nLen )
								{
									nErr = SetDragItemFlavorData( aDrag, (DragItemRef)pData, nType, (const void *)pArray, nLen, 0 );
								}
							}
						}
					}
				}
			}

			rSolarMutex.release();
		}
	}

	return nErr;
}

// ============================================================================

void DTransTransferable::flush()
{
	// Force pasteboard to render data if we still have ownership
	if ( mnTransferableType == TRANSFERABLE_TYPE_CLIPBOARD && mnChangeCount >= 0 )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		DTransPasteboardHelper *pHelper = [[DTransPasteboardHelper alloc] initWithPasteboardType:mnTransferableType];
		if ( pHelper )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pHelper performSelectorOnMainThread:@selector(flush:) withObject:[NSNumber numberWithInt:mnChangeCount] waitUntilDone:YES modes:pModes];
		}

		[pPool release];
	}
}

// ============================================================================

Any DTransTransferable::getTransferData( const DataFlavor& aFlavor ) throw ( UnsupportedFlavorException, IOException, RuntimeException )
{
	if ( mxTransferable.is() )
		return mxTransferable->getTransferData( aFlavor );

	Any out;

	if ( mnTransferableType == TRANSFERABLE_TYPE_CLIPBOARD )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		bool bDataAvailable = false;
		bool bDataRetrieved = false;
		DTransPasteboardHelper *pHelper = [[DTransPasteboardHelper alloc] initWithPasteboardType:mnTransferableType];
		if ( pHelper )
		{
			NSString *pRequestedType = nil;
			bool bRequestedTypeIsText = false;
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];

			// Run a loop so that if data type fails, we can try another
			for ( USHORT i = 0; !bDataRetrieved && i < nSupportedTypes; i++ )
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
					[pHelper performSelectorOnMainThread:@selector(getStringForType:) withObject:pRequestedType waitUntilDone:YES modes:pModes];
					bDataAvailable = [pHelper isTypeAvailable];
					NSString *pString = [pHelper stringForType];
					if ( pString )
					{
						const char *pUTF8String = [pString UTF8String];
						if ( pUTF8String )
						{
							OUString aString( pUTF8String, strlen( pUTF8String ), RTL_TEXTENCODING_UTF8 );

							// Replace carriage returns with line feeds
							aString = aString.replace( (sal_Unicode)'\r', (sal_Unicode)'\n' );

							out <<= aString;
							bDataRetrieved = true;
						}
					}
				}
				else if ( aFlavor.DataType.equals( getCppuType( ( Sequence< sal_Int8 >* )0 ) ) )
				{
					if ( bRequestedTypeIsText )
					{
						[pHelper performSelectorOnMainThread:@selector(getDataForType:) withObject:pRequestedType waitUntilDone:YES modes:pModes];
						bDataAvailable = [pHelper isTypeAvailable];
						NSData *pData = [pHelper dataForType];
						if ( pData )
						{
							const void *pArray = [pData bytes];
							unsigned int nLen = [pData length];

							if ( pArray && nLen )
							{
								Sequence< sal_Int8 > aExportData( nLen );
								memcpy( aExportData.getArray(), pArray, nLen );

								// Replace carriage returns with line feeds
								sal_Char *pCharArray = (sal_Char *)aExportData.getArray();
								if ( pCharArray )
								{
									for ( unsigned int j = 0; j < nLen; j++ )
									{
										if ( pCharArray[ j ] == '\r' )
											pCharArray[ j ] = '\n';
									}
								}

								// Strip out HTML Microsoft Office headers
								if ( [NSHTMLPboardType isEqualToString:pRequestedType] && isHTMLFormat( aExportData ) )
									aExportData = HTMLFormatToTextHtml( aExportData );

								out <<= aExportData;
								bDataRetrieved = true;
							}
						}
					}
					else
					{
						[pHelper performSelectorOnMainThread:@selector(getBMPDataForType:) withObject:pRequestedType waitUntilDone:YES modes:pModes];
						bDataAvailable = [pHelper isTypeAvailable];
						NSData *pData = [pHelper BMPDataForType];
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

		if ( !bDataRetrieved )
		{
			if ( bDataAvailable )
				throw IOException( aFlavor.MimeType, static_cast< XTransferable * >( this ) );
			else
				throw UnsupportedFlavorException( aFlavor.MimeType, static_cast< XTransferable * >( this ) );
		}

		[pPool release];
	}
	else if ( mnTransferableType == TRANSFERABLE_TYPE_DRAG && mnItem )
	{
		FourCharCode nRequestedType;
		memset( &nRequestedType, 0, sizeof( FourCharCode ) );
		bool bRequestedTypeIsText = false;
		OSStatus nErr = noErr;

		// Run a loop so that if data type fails, we can try another
		for ( USHORT i = 0; i < nSupportedTypes; i++ )
		{
			if ( aFlavor.MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ i ] ) )
			{
				nRequestedType = aSupportedNativeTypes[ i ];
				bRequestedTypeIsText = aSupportedTextTypes[ i ];
			}
			else
			{
				continue;
			}

			MacOSSize nSize = 0;
			DragItemRef aItem;
			if ( GetDragItemReferenceNumber( (DragRef)mpNativeTransferable, mnItem, &aItem ) == noErr )
				nErr = GetFlavorDataSize( (DragRef)mpNativeTransferable, aItem, nRequestedType, &nSize );

			if ( nErr == noErr && nSize > 0 )
			{
				bool bDataFound = false;
				Handle hData = NewHandle( nSize );

				HLock( hData );
				DragItemRef aItem;
				if ( GetDragItemReferenceNumber( (DragRef)mpNativeTransferable, mnItem, &aItem ) == noErr )
					bDataFound = ( GetFlavorData( (DragRef)mpNativeTransferable, aItem, nRequestedType, *hData, &nSize, 0 ) == noErr );
				HUnlock( hData );

				if ( bDataFound )
				{
					if ( aFlavor.DataType.equals( getCppuType( ( OUString* )0 ) ) )
					{
						OUString aString;
						sal_Int32 nLen = nSize;
						if ( nRequestedType == kQTFileTypeText )
						{
							HLock( hData );
							if ( ( (sal_Char *)*hData )[ nLen - 1 ] == 0 )
								nLen--;
							CFStringRef aCFString = CFStringCreateWithBytes( kCFAllocatorDefault, (const UInt8 *)*hData, nLen, CFStringGetSystemEncoding(), false );
							HUnlock( hData );

							if ( aCFString )
							{
								CFRange aRange;
								aRange.location = 0;
								aRange.length = CFStringGetLength( aCFString );
								// [ed 3/24/07 Place on to heap, not stack.  Bug #2171
								UniChar *aBuf = new UniChar[ aRange.length ];
								if ( aBuf )
								{
									CFStringGetCharacters( aCFString, aRange, aBuf );
									aString = OUString( (sal_Unicode *)aBuf, aRange.length );
									CFRelease( aCFString );
									delete[] aBuf;
								}
							}
						}
						else
						{
							HLock( hData );
							nLen = nSize / 2;
							if ( ( (sal_Unicode *)*hData )[ nLen - 1 ] == 0 )
								nLen--;
							aString = OUString( (sal_Unicode *)*hData, nLen );
							HUnlock( hData );
						}

						// Replace carriage returns with line feeds
						aString = aString.replace( (sal_Unicode)'\r', (sal_Unicode)'\n' );

						out <<= aString;
					}
					else if ( aFlavor.DataType.equals( getCppuType( ( Sequence< sal_Int8 >* )0 ) ) )
					{
						if ( !bRequestedTypeIsText )
						{
							// Convert to PNG format
							if ( nRequestedType == kQTFileTypePNG )
							{
								HLock( hData );
								Sequence< sal_Int8 > aExportData( nSize );
								memcpy( aExportData.getArray(), *hData, nSize );
								HUnlock( hData );
								out <<= aExportData;
							}
							else if ( nRequestedType == kQTFileTypePicture )
							{
								ComponentInstance aExporter;
								if ( OpenADefaultComponent( GraphicsExporterComponentType, kQTFileTypePNG, &aExporter ) == noErr )
								{
									if ( GraphicsExportSetInputPicture( aExporter, (PicHandle)hData ) == noErr )
									{
										Handle hExportData = NewHandle( 0 );
										if ( GraphicsExportSetOutputHandle( aExporter, hExportData ) == noErr )
										{
											ULONG nDataLen;
											if ( GraphicsExportDoExport( aExporter, &nDataLen ) == noErr )
											{
												Sequence< sal_Int8 > aExportData( nDataLen );
												HLock( hExportData );
												memcpy( aExportData.getArray(), *hExportData, nDataLen );
												HUnlock( hExportData );
												out <<= aExportData;
											}
										}

										DisposeHandle( hExportData );
									}

									CloseComponent( aExporter );
								}
							}
							else
							{
								ComponentInstance aImporter;
								if ( OpenADefaultComponent( GraphicsImporterComponentType, nRequestedType, &aImporter ) == noErr )
								{
									Rect aBounds;
									if ( GraphicsImportSetDataHandle( aImporter, hData ) == noErr && GraphicsImportGetNaturalBounds( aImporter, &aBounds ) == noErr )
									{
										GWorldPtr aGWorld;
										if ( QTNewGWorld( &aGWorld, k32ARGBPixelFormat, &aBounds, NULL, NULL, 0 ) == noErr )
										{
											if ( GraphicsImportSetGWorld( aImporter, aGWorld, NULL ) == noErr && GraphicsImportDraw( aImporter ) == noErr )
											{
												ComponentInstance aExporter;
												if ( OpenADefaultComponent( GraphicsExporterComponentType, kQTFileTypePNG, &aExporter ) == noErr )
												{
													if ( GraphicsExportSetInputGWorld( aExporter, aGWorld ) == noErr )
													{
														Handle hExportData = NewHandle( 0 );
														if ( GraphicsExportSetOutputHandle( aExporter, hExportData ) == noErr )
														{
															ULONG nDataLen;
															if ( GraphicsExportDoExport( aExporter, &nDataLen ) == noErr )
															{
																Sequence< sal_Int8 > aExportData( nDataLen );
																HLock( hExportData );
																memcpy( aExportData.getArray(), *hExportData, nDataLen );
																HUnlock( hExportData );
																out <<= aExportData;
															}
														}

														DisposeHandle( hExportData );
													}

													CloseComponent( aExporter );
												}
											}

											DisposeGWorld( aGWorld );
										}
									}

									CloseComponent( aImporter );
								}
							}
						}
						else
						{
							HLock( hData );
							sal_Int32 nLen = nSize;
							if ( ( (sal_Char *)*hData )[ nLen - 1 ] == 0 )
								nLen--;
							Sequence< sal_Int8 > aExportData( nLen );
							memcpy( aExportData.getArray(), *hData, nLen );
							HUnlock( hData );

							// Replace carriage returns with line feeds
							sal_Char *pArray = (sal_Char *)aExportData.getArray();
							if ( pArray )
							{
								for ( int j = 0; j < nLen; j++ )
								{
									if ( pArray[ j ] == '\r' )
										pArray[ j ] = '\n';
								}
							}

							// Strip out HTML Microsoft Office headers
							if ( nRequestedType == 'HTML' && isHTMLFormat( aExportData ) )
								aExportData = HTMLFormatToTextHtml( aExportData );

							out <<= aExportData;
						}
					}

					// Force a break from the loop
					i = nSupportedTypes;
				}

				DisposeHandle( hData );
			}
		}

		if ( !nRequestedType )
		{
			if ( nErr == noTypeErr || nErr == cantGetFlavorErr )
				throw UnsupportedFlavorException( aFlavor.MimeType, static_cast< XTransferable * >( this ) );
			else
				throw IOException( aFlavor.MimeType, static_cast< XTransferable * >( this ) );
		}
	}
	else
	{
		throw UnsupportedFlavorException( aFlavor.MimeType, static_cast< XTransferable * >( this ) );
	}

	return out;
}

// ----------------------------------------------------------------------------

DTransTransferable::DTransTransferable( int nTransferableType ) :
	mnChangeCount( -1 ),
	mpNativeTransferable( NULL ),
	mnTransferableType( nTransferableType ),
	mnItem( 0 )
{
	aTransferableList.push_back( this );
}

// ----------------------------------------------------------------------------

DTransTransferable::DTransTransferable( void *pNativeTransferable, int nTransferableType, sal_uInt16 nItem ) :
	mnChangeCount( -1 ),
	mpNativeTransferable( pNativeTransferable ),
	mnTransferableType( nTransferableType ),
	mnItem( 0 )
{
	aTransferableList.push_back( this );

	if ( mnTransferableType == TRANSFERABLE_TYPE_DRAG && nItem )
	{
		UInt16 nCount = 0;
		if ( CountDragItems( (DragRef)mpNativeTransferable, &nCount ) == noErr && nItem <= nCount )
			mnItem = nItem;
	}
}

// ----------------------------------------------------------------------------

DTransTransferable::~DTransTransferable()
{
	aTransferableList.remove( this );
}

// ----------------------------------------------------------------------------

Sequence< DataFlavor > DTransTransferable::getTransferDataFlavors() throw ( RuntimeException )
{
	if ( mxTransferable.is() )
		return mxTransferable->getTransferDataFlavors();

	Sequence< DataFlavor > out;

	if ( mnTransferableType == TRANSFERABLE_TYPE_CLIPBOARD )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		DTransPasteboardHelper *pHelper = [[DTransPasteboardHelper alloc] initWithPasteboardType:mnTransferableType];
		if ( pHelper )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pHelper performSelectorOnMainThread:@selector(getTypes:) withObject:pHelper waitUntilDone:YES modes:pModes];
			NSArray *pTypes = [pHelper types];
			if ( pTypes )
			{
				for ( USHORT i = 0; i < nSupportedTypes; i++ )
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
	}
	else if ( mnTransferableType == TRANSFERABLE_TYPE_DRAG && mnItem )
	{
		DragItemRef aItem;
		UInt16 nCount;
		if ( GetDragItemReferenceNumber( (DragRef)mpNativeTransferable, mnItem, &aItem ) == noErr && CountDragItemFlavors( (DragRef)mpNativeTransferable, aItem, &nCount ) == noErr && nCount > 0 )
		{
			for ( USHORT i = 0; i < nSupportedTypes; i++ )
			{
				for ( UInt16 j = 0; j < nCount; j++ )
				{
					FlavorType nFlavor;
					if ( GetFlavorType( (DragRef)mpNativeTransferable, aItem, j, &nFlavor ) == noErr && aSupportedNativeTypes[ i ] == nFlavor )
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

	return out;
}

// ----------------------------------------------------------------------------

sal_Bool DTransTransferable::hasOwnership()
{
	sal_Bool out = sal_False;

	if ( mnTransferableType == TRANSFERABLE_TYPE_CLIPBOARD && mnChangeCount >= 0 )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		DTransPasteboardHelper *pHelper = [[DTransPasteboardHelper alloc] initWithPasteboardType:mnTransferableType];
		if ( pHelper )
		{
			NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
			[pHelper performSelectorOnMainThread:@selector(getChangeCount:) withObject:pHelper waitUntilDone:YES modes:pModes];
			if ( [pHelper changeCount] == mnChangeCount )
				out = sal_True;
		}

		[pPool release];
	}

	return out;
}

// ----------------------------------------------------------------------------

sal_Bool DTransTransferable::isDataFlavorSupported( const DataFlavor& aFlavor ) throw ( RuntimeException )
{
	if ( mxTransferable.is() )
		return mxTransferable->isDataFlavorSupported( aFlavor );

	sal_Bool out = sal_False;

	if ( mnTransferableType == TRANSFERABLE_TYPE_CLIPBOARD )
	{
		NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

		const NSString *pRequestedType = nil;
		Type aRequestedDataType;
		for ( USHORT i = 0; i < nSupportedTypes; i++ )
		{
			if ( aFlavor.MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ i ] ) )
			{
				pRequestedType = aSupportedPasteboardTypes[ i ];
				aRequestedDataType = aSupportedDataTypes[ i ];
				break;
			}
		}

		if ( pRequestedType )
		{
			DTransPasteboardHelper *pHelper = [[DTransPasteboardHelper alloc] initWithPasteboardType:mnTransferableType];
			if ( pHelper )
			{
				NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
				[pHelper performSelectorOnMainThread:@selector(getTypeAvailable:) withObject:pRequestedType waitUntilDone:YES modes:pModes];
				if ( [pHelper isTypeAvailable] )
					out = sal_True;
			}
		}

		[pPool release];
	}
	else if ( mnTransferableType == TRANSFERABLE_TYPE_DRAG )
	{
		FourCharCode nRequestedType;
		memset( &nRequestedType, 0, sizeof( FourCharCode ) );
		Type aRequestedDataType;
		for ( USHORT i = 0; i < nSupportedTypes; i++ )
		{
			if ( aFlavor.MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ i ] ) )
			{
				nRequestedType = aSupportedNativeTypes[ i ];
				aRequestedDataType = aSupportedDataTypes[ i ];
				break;
			}
		}

		if ( nRequestedType )
		{
			DragItemRef aItem;
			UInt16 nCount;
			if ( GetDragItemReferenceNumber( (DragRef)mpNativeTransferable, 1, &aItem ) == noErr && CountDragItemFlavors( (DragRef)mpNativeTransferable, aItem, &nCount ) == noErr && nCount > 0 )
			{
				for ( UInt16 i = 0; i < nCount; i++ )
				{
					FlavorType nFlavor;
					if ( GetFlavorType( (DragRef)mpNativeTransferable, aItem, i, &nFlavor ) == noErr && nFlavor == nRequestedType && aFlavor.DataType.equals( aRequestedDataType ) )
					{
						out = sal_True;
						break;
					}
				}
			}
		}
	}

	return out;
}

// ----------------------------------------------------------------------------

sal_Bool DTransTransferable::setContents( const Reference< XTransferable > &xTransferable )
{
	sal_Bool out = sal_False;

	mxTransferable = xTransferable;
	if ( mxTransferable.is() )
	{
		if ( mnTransferableType == TRANSFERABLE_TYPE_CLIPBOARD )
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
				for ( USHORT j = 0; j < nSupportedTypes; j++ )
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
					for ( USHORT j = 0; j < nSupportedTypes; j++ )
					{
						if ( xFlavors[ i ].MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ j ] ) )
						{
							if ( bTextOnly && !aSupportedTextTypes[ j ] )
								continue;

							// Converting to PDF does not work (only converting
							// from PDF works) so don't add the PDF flavor
							if ( !aSupportedPasteboardTypes[ j ] || [NSPDFPboardType isEqualToString:aSupportedPasteboardTypes[ j ]] )
								continue;

							[pTypes addObject:aSupportedPasteboardTypes[ j ]];
						}
					}
				}

				if ( [pTypes count] )
				{
					// Do not retain as invoking alloc disables autorelease
					// and the object will be released in the object's
					// pasteboardChangedOwner: selector
					DTransPasteboardOwner *pOwner = [[DTransPasteboardOwner alloc] initWithTransferable:this pasteboardType:mnTransferableType types:pTypes];
					if ( pOwner )
					{
						NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
						[pOwner performSelectorOnMainThread:@selector(setContents:) withObject:pOwner waitUntilDone:YES modes:pModes];
						mnChangeCount = [pOwner changeCount];
						if ( mnChangeCount >= 0 )
							out = sal_True;
					}
				}
			}

			[pPool release];
		}
		else if ( mnTransferableType == TRANSFERABLE_TYPE_DRAG )
		{
			out = sal_True;

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
				for ( USHORT j = 0; j < nSupportedTypes; j++ )
				{
					if ( xFlavors[ i ].MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ j ] ) && aSupportedTextTypes[ j ] )
					{
						bTextOnly = true;
						break;
					}
				}
			}

			bool bRenderImmediately = false;
			if ( !pDragSendDataUPP )
				pDragSendDataUPP = NewDragSendDataUPP( (DragSendDataProcPtr)ImplDragSendDataCallback );
			if ( !pDragSendDataUPP || SetDragSendProc( (DragRef)mpNativeTransferable, pDragSendDataUPP, (void *)this ) != noErr )
				bRenderImmediately = true;

			for ( i = 0; i < nLen; i++ )
			{ 
				for ( USHORT j = 0; j < nSupportedTypes; j++ )
				{
					if ( xFlavors[ i ].MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ j ] ) )
					{
						if ( bTextOnly && !aSupportedTextTypes[ j ] )
							continue;

						AddDragItemFlavor( (DragRef)mpNativeTransferable, (DragItemRef)this, aSupportedNativeTypes[ j ], NULL, 0, 0 );

						// Set item to the last item
						UInt16 nCount = 0;
						if ( CountDragItems( (DragRef)mpNativeTransferable, &nCount ) == noErr && nCount )
							mnItem = nCount;

						if ( bRenderImmediately )
							ImplDragSendDataCallback( aSupportedNativeTypes[ j ], (void *)this, (DragItemRef)this, (DragRef)mpNativeTransferable );
					}
				}
			}
		}
	}

	return out;
}
