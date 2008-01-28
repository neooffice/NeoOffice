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

#define _DTRANSCLIPBOARD_CXX

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

#include <premac.h>
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <postmac.h>

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace java;
using namespace rtl;
using namespace vcl;
using namespace vos;

static UInt32 nSupportedTypes = 6;

// List of supported native types in priority order
static FourCharCode aSupportedNativeTypes[] = {
	'RTF ',
	'utxt',
	'TEXT',
	'PDF ',
	'TIFF',
	'PICT'
};

// List of supported types that are text
static bool aSupportedTextTypes[] = {
	true,
	true,
	true,
	false,
	false,
	false
};

// List of supported mime types in priority order
static OUString aSupportedMimeTypes[] = {
	OUString::createFromAscii( "text/richtext" ),
	OUString::createFromAscii( "text/plain;charset=utf-16" ),
	OUString::createFromAscii( "text/plain;charset=utf-16" ),
	OUString::createFromAscii( "image/bmp" ),
	OUString::createFromAscii( "image/bmp" ),
	OUString::createFromAscii( "image/bmp" )
};

// List of supported data types in priority order
static ::com::sun::star::uno::Type aSupportedDataTypes[] = {
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( OUString* )0 ),
	getCppuType( ( OUString* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 )
};

static ::std::list< DTransTransferable* > aTransferableList;
static DragSendDataUPP pDragSendDataUPP = NULL;
static ScrapPromiseKeeperUPP pScrapPromiseKeeperUPP = NULL;

// ============================================================================

static OSStatus ImplSetTransferableData( void *pNativeTransferable, int nTransferableType, FlavorType nType, void *pData )
{
	OSStatus nErr;
	if ( nTransferableType == TRANSFERABLE_TYPE_CLIPBOARD )
		nErr = noTypeErr;
	else if ( nTransferableType == TRANSFERABLE_TYPE_DRAG )
		nErr = cantGetFlavorErr;
	else
		nErr = noTypeErr;

	if ( !Application::IsShutDown() )
	{
		IMutex& rSolarMutex = Application::GetSolarMutex();
		rSolarMutex.acquire();
		if ( !Application::IsShutDown() )
		{
			DTransTransferable *pTransferable = (DTransTransferable *)pData;

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
				DataFlavor aFlavor;
				for ( USHORT i = 0; i < nSupportedTypes; i++ ) {
					if ( nType == aSupportedNativeTypes[ i ] )
					{
						aFlavor.MimeType = aSupportedMimeTypes[ i ];
						aFlavor.DataType = aSupportedDataTypes[ i ];
						if ( pTransferable->isDataFlavorSupported( aFlavor ) )
						{
							bFlavorFound = true;
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
								// Replace line feeds with carriage returns
								sal_Int32 j = 0;
								for ( j = 0; j < nLen; j++ )
								{
									if ( pArray[ j ] == (sal_Unicode)'\n' )
										pArray[ j ] = (sal_Unicode)'\r';
								}
		
								if ( nType == 'RTF ' )
								{
									OString aEncodedString = OUStringToOString( aString, RTL_TEXTENCODING_ASCII_US );

									if ( nTransferableType == TRANSFERABLE_TYPE_CLIPBOARD )
										nErr = PutScrapFlavor( (ScrapRef)pNativeTransferable, nType, kScrapFlavorMaskNone, aEncodedString.getLength(), (const void *)aEncodedString.getStr() );
									else if ( nTransferableType == TRANSFERABLE_TYPE_DRAG )
										nErr = SetDragItemFlavorData( (DragRef)pNativeTransferable, (DragItemRef)pData, nType, (const void *)aEncodedString.getStr(), aEncodedString.getLength(), 0 );
								}
								else if ( nType == 'TEXT' )
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
											UInt8 aBuf[ nBufLen + 1 ];
											if ( CFStringGetBytes( aCFString, aRange, CFStringGetSystemEncoding(), '?', false, aBuf, nBufLen, &nRealLen ) && nRealLen == nBufLen )
											{
												aBuf[ nBufLen ] = '\0';
												if ( nTransferableType == TRANSFERABLE_TYPE_CLIPBOARD )
													nErr = PutScrapFlavor( (ScrapRef)pNativeTransferable, nType, kScrapFlavorMaskNone, nBufLen, (const void *)aBuf );
												else if ( nTransferableType == TRANSFERABLE_TYPE_DRAG )
													nErr = SetDragItemFlavorData( (DragRef)pNativeTransferable, (DragItemRef)pData, nType, (const void *)aBuf, nBufLen, 0 );
											}
										}

										CFRelease( aCFString );
									}
								}
								else
								{
 									nLen *= sizeof( sal_Unicode );
									if ( nTransferableType == TRANSFERABLE_TYPE_CLIPBOARD )
										nErr = PutScrapFlavor( (ScrapRef)pNativeTransferable, nType, kScrapFlavorMaskNone, nLen, (const void *)aString.getStr() );
									else if ( nTransferableType == TRANSFERABLE_TYPE_DRAG )
										nErr = SetDragItemFlavorData( (DragRef)pNativeTransferable, (DragItemRef)pData, nType, (const void *)aString.getStr(), nLen, 0 );
								}
							}
						}
						else if ( aValue.getValueType().equals( getCppuType( ( Sequence< sal_Int8 >* )0 ) ) )
						{
							Sequence< sal_Int8 > aData;
							aValue >>= aData;
							sal_Int8 *pArray = aData.getArray();
							sal_Int32 nLen = aData.getLength();
							if ( pArray && nLen )
							{
								if ( nType == 'PICT' )
								{
									// Convert to PICT from our BMP data
									ComponentInstance aImporter;
									if ( OpenADefaultComponent( GraphicsImporterComponentType, 'BMPf', &aImporter ) == noErr )
									{
										Handle hData;
										if ( PtrToHand( pArray, &hData, nLen ) == noErr )
										{
											// Free the source data
											aData = Sequence< sal_Int8 >();

											if ( GraphicsImportSetDataHandle( aImporter, hData ) == noErr )
											{
												PicHandle hPict;
												if ( GraphicsImportGetAsPicture( aImporter, &hPict ) == noErr )
												{
													HLock( (Handle)hPict );
													if ( nTransferableType == TRANSFERABLE_TYPE_CLIPBOARD )
														nErr = PutScrapFlavor( (ScrapRef)pNativeTransferable, nType, kScrapFlavorMaskNone, GetHandleSize( (Handle)hPict ), (const void *)*hPict );
													else if ( nTransferableType == TRANSFERABLE_TYPE_DRAG )
														nErr = SetDragItemFlavorData( (DragRef)pNativeTransferable, (DragItemRef)pData, nType, (const void *)*hPict, GetHandleSize( (Handle)hPict ), 0 );
													HUnlock( (Handle)hPict );
													KillPicture( hPict );
												}
												HUnlock( hData );
												DisposeHandle( hData );
											}
											CloseComponent( aImporter );
										}
									}
								}
								else if ( nType == 'TIFF' )
								{
									// Convert to TIFF from our BMP data
									ComponentInstance aImporter;
									if ( OpenADefaultComponent( GraphicsImporterComponentType, 'BMPf', &aImporter ) == noErr )
									{
										Handle hData;
										if ( PtrToHand( pArray, &hData, nLen ) == noErr )
										{
											// Free the source data
											aData = Sequence< sal_Int8 >();

											if ( GraphicsImportSetDataHandle( aImporter, hData ) == noErr )
											{
												PicHandle hPict;
												if ( GraphicsImportGetAsPicture( aImporter, &hPict ) == noErr )
												{
													ComponentInstance aExporter;
													if ( OpenADefaultComponent( GraphicsExporterComponentType, nType, &aExporter ) == noErr );
													{
														if ( GraphicsExportSetInputPicture( aExporter, hPict ) == noErr )
														{
															Handle hExportData = NewHandle( 0 );
															if ( GraphicsExportSetOutputHandle( aExporter, hExportData ) == noErr )
															{
																unsigned long nDataLen;
																if ( GraphicsExportDoExport( aExporter, &nDataLen ) == noErr )
																{
																	HLock( hExportData );
																	if ( nTransferableType == TRANSFERABLE_TYPE_CLIPBOARD )
																		nErr = PutScrapFlavor( (ScrapRef)pNativeTransferable, nType, kScrapFlavorMaskNone, nDataLen, (const void *)*hExportData );
																	else if ( nTransferableType == TRANSFERABLE_TYPE_DRAG )
																		nErr = SetDragItemFlavorData( (DragRef)pNativeTransferable, (DragItemRef)pData, nType, (const void *)*hExportData, nDataLen, 0 );
																	HUnlock( hExportData );
																}
																DisposeHandle( hExportData );
															}
														}
														CloseComponent( aExporter );
													}
													KillPicture( hPict );
												}
												DisposeHandle( hData );
											}
											CloseComponent( aImporter );
										}
									}
								}
								else
								{
									if ( pArray && nLen )
									{
										if ( nTransferableType == TRANSFERABLE_TYPE_CLIPBOARD )
											nErr = PutScrapFlavor( (ScrapRef)pNativeTransferable, nType, kScrapFlavorMaskNone, nLen, (const void *)pArray );
										else if ( nTransferableType == TRANSFERABLE_TYPE_DRAG )
											nErr = SetDragItemFlavorData( (DragRef)pNativeTransferable, (DragItemRef)pData, nType, (const void *)pArray, nLen, 0 );
									}
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

// ----------------------------------------------------------------------------

static OSErr ImplDragSendDataCallback( FlavorType nType, void *pData, DragItemRef aItem, DragRef aDrag )
{
	return ImplSetTransferableData( (void *)aDrag, TRANSFERABLE_TYPE_DRAG, nType, pData );
}

// ----------------------------------------------------------------------------

static OSStatus ImplScrapPromiseKeeperCallback( ScrapRef aScrap, ScrapFlavorType nType, void *pData )
{
	return ImplSetTransferableData( (void *)aScrap, TRANSFERABLE_TYPE_CLIPBOARD, nType, pData );
}

// ============================================================================

void DTransTransferable::flush()
{
	if ( hasOwnership() )
		CallInScrapPromises();
}

// ============================================================================

Any DTransTransferable::getTransferData( const DataFlavor& aFlavor ) throw ( UnsupportedFlavorException, IOException, RuntimeException )
{
	if ( mxTransferable.is() )
		return mxTransferable->getTransferData( aFlavor );

	Any out;

	FourCharCode nRequestedType = NULL;
	OSStatus nErr = noErr;

	// Run a loop so that if data type fails, we can try another
	for ( USHORT i = 0; i < nSupportedTypes; i++ )
	{
		if ( aFlavor.MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ i ] ) )
			nRequestedType = aSupportedNativeTypes[ i ];
		else
			continue;

		MacOSSize nSize;
		if ( mnTransferableType == TRANSFERABLE_TYPE_CLIPBOARD )
		{
			nErr = GetScrapFlavorSize( (ScrapRef)mpNativeTransferable, nRequestedType, &nSize );
		}
		else if ( mnTransferableType == TRANSFERABLE_TYPE_DRAG )
		{
			DragItemRef aItem;
			if ( GetDragItemReferenceNumber( (DragRef)mpNativeTransferable, 1, &aItem ) == noErr )
				nErr = GetFlavorDataSize( (DragRef)mpNativeTransferable, aItem, nRequestedType, &nSize );
		}
		else
		{
			nErr = noTypeErr;
			nSize = 0;
		}

		if ( nErr == noErr && nSize > 0 )
		{
			Sequence< sal_Int8 > aData( nSize );
			bool bDataFound = false;
			if ( mnTransferableType == TRANSFERABLE_TYPE_CLIPBOARD )
			{
				bDataFound = ( GetScrapFlavorData( (ScrapRef)mpNativeTransferable, nRequestedType, &nSize, aData.getArray() ) == noErr );
			}
			else if ( mnTransferableType == TRANSFERABLE_TYPE_DRAG )
			{
				DragItemRef aItem;
				if ( GetDragItemReferenceNumber( (DragRef)mpNativeTransferable, 1, &aItem ) == noErr )
					bDataFound = ( GetFlavorData( (DragRef)mpNativeTransferable, aItem, nRequestedType, aData.getArray(), &nSize, 0 ) == noErr );
			}

			if ( bDataFound )
			{
				if ( aFlavor.DataType.equals( getCppuType( ( OUString* )0 ) ) )
				{
					OUString aString;
					sal_Int32 nLen;
					if ( nRequestedType == 'RTF ' )
					{
						nLen = aData.getLength();
						if ( ( (sal_Char *)aData.getArray() )[ nLen - 1 ] == 0 )
							nLen--;
						aString = OUString( (sal_Char *)aData.getArray(), nLen, RTL_TEXTENCODING_ASCII_US );
					}
					else if ( nRequestedType == 'TEXT' )
					{
						nLen = aData.getLength();
						if ( ( (sal_Char *)aData.getArray() )[ nLen - 1 ] == 0 )
							nLen--;
						CFStringRef aCFString = CFStringCreateWithBytes( kCFAllocatorDefault, (const UInt8 *)aData.getArray(), nLen, CFStringGetSystemEncoding(), false );
						if ( aCFString )
						{
							
							CFRange aRange;
							aRange.location = 0;
							aRange.length = CFStringGetLength( aCFString );
							// [ed 3/24/07 Place on to heap, not stack.  Bug #2171
							UniChar *aBuf = new UniChar[ aRange.length ];
							CFStringGetCharacters( aCFString, aRange, aBuf );
							aString = OUString( (sal_Unicode *)aBuf, aRange.length );
							CFRelease( aCFString );
							delete[] aBuf;
						}
					}
					else
					{
						nLen = aData.getLength() / 2;
						if ( ( (sal_Unicode *)aData.getArray() )[ nLen - 1 ] == 0 )
							nLen--;
						aString = OUString( (sal_Unicode *)aData.getArray(), nLen );
					}

					// Replace carriage returns with line feeds
					sal_Unicode *pArray = (sal_Unicode *)aString.getStr();
					nLen = aString.getLength();
					sal_Int32 j = 0;
					for ( j = 0; j < nLen; j++ )
					{
						if ( pArray[ j ] == (sal_Unicode)'\r' )
							pArray[ j ] = (sal_Unicode)'\n';
					}

					out <<= aString;
				}
				else if ( aFlavor.DataType.equals( getCppuType( ( Sequence< sal_Int8 >* )0 ) ) )
				{
					if ( nRequestedType == 'PICT' )
					{
						// Convert to BMP format
						ComponentInstance aExporter;
						if ( OpenADefaultComponent( GraphicsExporterComponentType, 'BMPf', &aExporter ) == noErr );
						{
							Handle hData;
							if ( PtrToHand( aData.getArray(), &hData, aData.getLength() ) == noErr )
							{
								if ( GraphicsExportSetInputPicture( aExporter, (PicHandle)hData ) == noErr )
								{
									Handle hExportData = NewHandle( 0 );
									if ( GraphicsExportSetOutputHandle( aExporter, hExportData ) == noErr )
									{
										unsigned long nDataLen;
										if ( GraphicsExportDoExport( aExporter, &nDataLen ) == noErr )
										{
											Sequence< sal_Int8 > aExportData( nDataLen );
											HLock( hExportData );
											memcpy( aExportData.getArray(), *hExportData, nDataLen );
											HUnlock( hExportData );
											out <<= aExportData;
										}
										DisposeHandle( hExportData );
									}
								}
								DisposeHandle( hData );
							}
							CloseComponent( aExporter );
						}
					}
					else if ( nRequestedType == 'PDF ' || nRequestedType == 'TIFF' )
					{
						// Convert to BMP format
						ComponentInstance aImporter;
						if ( OpenADefaultComponent( GraphicsImporterComponentType, nRequestedType, &aImporter ) == noErr )
						{
							Handle hData;
							if ( PtrToHand( aData.getArray(), &hData, aData.getLength() ) == noErr )
							{
								// Free the source data
								aData = Sequence< sal_Int8 >();

								if ( GraphicsImportSetDataHandle( aImporter, hData ) == noErr )
								{
									PicHandle hPict;
									if ( GraphicsImportGetAsPicture( aImporter, &hPict ) == noErr )
									{
										ComponentInstance aExporter;
										if ( OpenADefaultComponent( GraphicsExporterComponentType, 'BMPf', &aExporter ) == noErr );
										{
											if ( GraphicsExportSetInputPicture( aExporter, hPict ) == noErr )
											{
												Handle hExportData = NewHandle( 0 );
												if ( GraphicsExportSetOutputHandle( aExporter, hExportData ) == noErr )
												{
													unsigned long nDataLen;
													if ( GraphicsExportDoExport( aExporter, &nDataLen ) == noErr )
													{
														Sequence< sal_Int8 > aExportData( nDataLen );
														HLock( hExportData );
														memcpy( aExportData.getArray(), *hExportData, nDataLen );
														HUnlock( hExportData );
														out <<= aExportData;
													}
													DisposeHandle( hExportData );
												}
											}
											CloseComponent( aExporter );
										}
										KillPicture( hPict );
									}
								}
								DisposeHandle( hData );
							}
							CloseComponent( aImporter );
						}
					}
					else
					{
						out <<= aData;
					}
				}

				// Force a break from the loop
				i = nSupportedTypes;
			}
		}
	}

	if ( !nRequestedType )
	{
		if ( nErr == noTypeErr || nErr == cantGetFlavorErr )
			throw UnsupportedFlavorException( aFlavor.MimeType, static_cast< XTransferable * >( this ) );
		else
			throw IOException( aFlavor.MimeType, static_cast< XTransferable * >( this ) );
	}

	return out;
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
		UInt32 nCount;
		if ( GetScrapFlavorCount( (ScrapRef)mpNativeTransferable, &nCount ) == noErr && nCount > 0 )
		{
			ScrapFlavorInfo *pInfo = new ScrapFlavorInfo[ nCount ];

			if ( GetScrapFlavorInfoList( (ScrapRef)mpNativeTransferable, &nCount, pInfo ) == noErr )
			{
				for ( USHORT i = 0; i < nSupportedTypes; i++ )
				{
					for ( UInt32 j = 0; j < nCount; j++ )
					{
						if ( aSupportedNativeTypes[ i ] == pInfo[ j ].flavorType )
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

			delete[] pInfo;
		}
	}
	else if ( mnTransferableType == TRANSFERABLE_TYPE_DRAG )
	{
		DragItemRef aItem;
		UInt16 nCount;
		if ( GetDragItemReferenceNumber( (DragRef)mpNativeTransferable, 1, &aItem ) == noErr && CountDragItemFlavors( (DragRef)mpNativeTransferable, aItem, &nCount ) == noErr && nCount > 0 )
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

	if ( mnTransferableType == TRANSFERABLE_TYPE_CLIPBOARD )
	{
		ScrapRef aScrap;

		if ( GetCurrentScrap( &aScrap ) == noErr && aScrap == (ScrapRef)mpNativeTransferable )
			out = sal_True;
	}

	return out;
}

// ----------------------------------------------------------------------------

sal_Bool DTransTransferable::isDataFlavorSupported( const DataFlavor& aFlavor ) throw ( RuntimeException )
{
	if ( mxTransferable.is() )
		return mxTransferable->isDataFlavorSupported( aFlavor );

	sal_Bool out = sal_False;

	FourCharCode nRequestedType = NULL;
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
		if ( mnTransferableType == TRANSFERABLE_TYPE_CLIPBOARD )
		{
			UInt32 nCount;
			if ( GetScrapFlavorCount( (ScrapRef)mpNativeTransferable, &nCount ) == noErr && nCount > 0 )
			{
				ScrapFlavorInfo *pInfo = new ScrapFlavorInfo[ nCount ];

				if ( GetScrapFlavorInfoList( (ScrapRef)mpNativeTransferable, &nCount, pInfo ) == noErr )
				{
					for ( UInt32 i = 0; i < nCount; i++ )
					{
						if ( pInfo[ i ].flavorType == nRequestedType && aFlavor.DataType.equals( aRequestedDataType ) )
						{
							out = sal_True;
							break;
						}
					}
				}

				delete[] pInfo;
			}
		}
		else if ( mnTransferableType == TRANSFERABLE_TYPE_DRAG )
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
			ScrapRef aScrap;
			if ( ClearCurrentScrap() == noErr && GetCurrentScrap( &aScrap ) == noErr )
			{
				// We have now cleared the scrap so we now own it
				mpNativeTransferable = aScrap;
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

				aTransferableList.push_back( this );

				bool bRenderImmediately = false;
				if ( !pScrapPromiseKeeperUPP )
					pScrapPromiseKeeperUPP = NewScrapPromiseKeeperUPP( (ScrapPromiseKeeperProcPtr)ImplScrapPromiseKeeperCallback );
				if ( !pScrapPromiseKeeperUPP || SetScrapPromiseKeeper( (ScrapRef)mpNativeTransferable, pScrapPromiseKeeperUPP, (const void *)this ) != noErr )
					bRenderImmediately = true;

				for ( i = 0; i < nLen; i++ )
				{ 
					for ( USHORT j = 0; j < nSupportedTypes; j++ )
					{
						if ( xFlavors[ i ].MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ j ] ) )
						{
							if ( bTextOnly && !aSupportedTextTypes[ j ] )
								continue;

							// Converting to PDF doesn't work (only converting
							// from PDF works) so don't add the PDF flavor
							if ( aSupportedNativeTypes[ j ] == 'PDF ' )
								continue;

							if ( bRenderImmediately )
								ImplScrapPromiseKeeperCallback( (ScrapRef)mpNativeTransferable, aSupportedNativeTypes[ j ], (void *)this );
							else
								 PutScrapFlavor( (ScrapRef)mpNativeTransferable, aSupportedNativeTypes[ j ], kScrapFlavorMaskNone, kScrapFlavorSizeUnknown, NULL );
						}
					}
				}
			}
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

			aTransferableList.push_back( this );

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
						if ( bRenderImmediately )
							ImplDragSendDataCallback( aSupportedNativeTypes[ j ], (void *)this, (DragItemRef)this, (DragRef)mpNativeTransferable );
					}
				}
			}
		}
	}

	return out;
}
