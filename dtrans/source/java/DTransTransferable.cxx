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
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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

#define _JAVA_DTRANS_COM_SUN_STAR_DTRANS_DTRANSCLIPBOARD_CXX

#include <list>

#ifndef _JAVA_DTRANS_COM_SUN_STAR_DTRANS_DTRANSCLIPBOARD_HXX
#include <com/sun/star/dtrans/DTransClipboard.hxx>
#endif
#ifndef _JAVA_DTRANS_COM_SUN_STAR_DTRANS_DTRANSTRANSFERABLE_HXX
#include <com/sun/star/dtrans/DTransTransferable.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifdef MACOSX

#ifndef _JAVA_DTRANS_JAVA_LANG_CLASS_HXX
#include <java/lang/Class.hxx>
#endif
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif
#include <premac.h>
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <postmac.h>

using namespace rtl;
using namespace vos;

static UInt32 nSupportedTypes = 4;

// List of supported native types in priority order
static FourCharCode aSupportedNativeTypes[] = {
	'RTF ',
	'utxt',
	'TIFF',
	'PICT'
};

// List of supported types that are text
static BOOL aSupportedTextTypes[] = {
	TRUE,
	TRUE,
	FALSE,
	FALSE
};

// List of supported mime types in priority order
static OUString aSupportedMimeTypes[] = {
	OUString::createFromAscii( "text/richtext" ),
	OUString::createFromAscii( "text/plain;charset=utf-16" ),
	OUString::createFromAscii( "image/bmp" ),
	OUString::createFromAscii( "image/bmp" )
};

// List of supported data types in priority order
static ::com::sun::star::uno::Type aSupportedDataTypes[] = {
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( OUString* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 )
};

static DragSendDataUPP pDragSendDataUPP = NULL;
static ScrapPromiseKeeperUPP pScrapPromiseKeeperUPP = NULL;

#endif	// MACOSX

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace java::dtrans;
using namespace osl;

static Mutex aMutex;
static ::std::list< com_sun_star_dtrans_DTransTransferable* > aTransferableList;

// ============================================================================

#ifdef MACOSX
static OSStatus ImplSetTransferableData( void *pNativeTransferable, int nTransferableType, FlavorType nType, void *pData )
{
	OSStatus nErr;
	if ( nTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_CLIPBOARD )
		nErr = noTypeErr;
	else if ( nTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_DRAG )
		nErr = cantGetFlavorErr;
	else
		nErr = noTypeErr;

	MutexGuard aGuard( aMutex );

	static jclass carbonLockClass = NULL;
	static jmethodID mIDRelease0 = NULL;
	static jmethodID mIDAcquire0 = NULL;
	DTransThreadAttach t;
	if ( t.pEnv && t.pEnv->GetVersion() < JNI_VERSION_1_4 )
	{
		carbonLockClass = t.pEnv->FindClass( "com/apple/mrj/macos/carbon/CarbonLock" );
		if ( carbonLockClass )
		{
			if ( !mIDRelease0 )
			{
				char *cSignature = "()I";
				mIDRelease0 = t.pEnv->GetStaticMethodID( carbonLockClass, "release0", cSignature );
			}
			OSL_ENSURE( mIDRelease0, "Unknown method id!" );
			if ( !mIDAcquire0 )
			{
				char *cSignature = "()I";
				mIDAcquire0 = t.pEnv->GetStaticMethodID( carbonLockClass, "acquire0", cSignature );
			}
			OSL_ENSURE( mIDAcquire0, "Unknown method id!" );
		}
	}

	com_sun_star_dtrans_DTransTransferable *pTransferable = (com_sun_star_dtrans_DTransTransferable *)pData;

	if ( carbonLockClass && mIDRelease0 && mIDRelease0 )
		t.pEnv->CallStaticIntMethod( carbonLockClass, mIDRelease0 );

	BOOL bTransferableFound = FALSE;
	if ( pTransferable )
	{
		for ( ::std::list< com_sun_star_dtrans_DTransTransferable* >::const_iterator it = aTransferableList.begin(); it != aTransferableList.end(); ++it )
		{
			if ( pTransferable == *it )
			{
				bTransferableFound = TRUE;
				break;
			}
		}
	}

	if ( bTransferableFound )
	{
		BOOL bFlavorFound = FALSE;
		DataFlavor aFlavor;
		for ( USHORT i = 0; i < nSupportedTypes; i++ ) {
			if ( nType == aSupportedNativeTypes[ i ] )
			{
				aFlavor.MimeType = aSupportedMimeTypes[ i ];
				aFlavor.DataType = aSupportedDataTypes[ i ];
				if ( pTransferable->isDataFlavorSupported( aFlavor ) )
				{
					bFlavorFound = TRUE;
					break;
				}
			}
		}

		if ( bFlavorFound )
		{
			BOOL bDataFound = FALSE;
			Any aValue;
			try {
				aValue = pTransferable->getTransferData( aFlavor );
				bDataFound = TRUE;
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

					// Replace line feeds with carriage returns
					sal_Unicode *pArray = (sal_Unicode *)aString.getStr();
					sal_Int32 nLen = aString.getLength() * sizeof( sal_Unicode );
					sal_Int32 j = 0;
					for ( j = 0; j < nLen; j++ )
					{
						if ( pArray[ j ] == (sal_Unicode)'\n' )
							pArray[ j ] = (sal_Unicode)'\r';
					}

					if ( nType != 'utxt' )
					{
						OString aEncodedString = OUStringToOString( aString, RTL_TEXTENCODING_ASCII_US );

						if ( nTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_CLIPBOARD )
							nErr = PutScrapFlavor( (ScrapRef)pNativeTransferable, nType, kScrapFlavorMaskNone, aEncodedString.getLength(), (const void *)aEncodedString.getStr() );
						else if ( nTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_DRAG )
							nErr = AddDragItemFlavor( (DragRef)pNativeTransferable, (DragItemRef)pData, nType, (const void *)aEncodedString.getStr(), aEncodedString.getLength(), 0 );
					}
					else
					{
						if ( nTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_CLIPBOARD )
							nErr = PutScrapFlavor( (ScrapRef)pNativeTransferable, nType, kScrapFlavorMaskNone, nLen, (const void *)aString.getStr() );
						else if ( nTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_DRAG )
							nErr = AddDragItemFlavor( (DragRef)pNativeTransferable, (DragItemRef)pData, nType, (const void *)aString.getStr(), nLen, 0 );
					}
				}
				else if ( aValue.getValueType().equals( getCppuType( ( Sequence< sal_Int8 >* )0 ) ) )
				{
					Sequence< sal_Int8 > aData;
					aValue >>= aData;

					if ( nType == 'PICT' )
					{
						// Convert to PICT from our BMP data
						ComponentInstance aImporter;
						if ( OpenADefaultComponent( GraphicsImporterComponentType, 'BMPf', &aImporter ) == noErr )
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
										HLock( (Handle)hPict );
										if ( nTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_CLIPBOARD )
											nErr = PutScrapFlavor( (ScrapRef)pNativeTransferable, nType, kScrapFlavorMaskNone, GetHandleSize( (Handle)hPict ), (const void *)*hPict );
										else if ( nTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_DRAG )
											nErr = AddDragItemFlavor( (DragRef)pNativeTransferable, (DragItemRef)pData, nType, (const void *)*hPict, GetHandleSize( (Handle)hPict ), 0 );
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
														if ( nTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_CLIPBOARD )
															nErr = PutScrapFlavor( (ScrapRef)pNativeTransferable, nType, kScrapFlavorMaskNone, nDataLen, (const void *)*hExportData );
														else if ( nTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_DRAG )
															nErr = AddDragItemFlavor( (DragRef)pNativeTransferable, (DragItemRef)pData, nType, (const void *)*hExportData, nDataLen, 0 );
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
						sal_Int8 *pData = aData.getArray();
						MacOSSize nDataLen = aData.getLength();

						if ( pData && nDataLen )
						if ( nTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_CLIPBOARD )
							nErr = PutScrapFlavor( (ScrapRef)pNativeTransferable, nType, kScrapFlavorMaskNone, aData.getLength(), (const void *)aData.getArray() );
						else if ( nTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_DRAG )
							nErr = AddDragItemFlavor( (DragRef)pNativeTransferable, (DragItemRef)pData, nType, (const void *)aData.getArray(), aData.getLength(), 0 );
					}
				}
			}
		}
	}

	if ( carbonLockClass && mIDRelease0 && mIDAcquire0 )
		t.pEnv->CallStaticIntMethod( carbonLockClass, mIDAcquire0 );

	return nErr;
}
#endif	// MACOSX

// ----------------------------------------------------------------------------

#ifdef MACOSX
static OSErr ImplDragSendDataCallback( FlavorType nType, void *pData, DragItemRef aItem, DragRef aDrag )
{
	return ImplSetTransferableData( (void *)aDrag, JAVA_DTRANS_TRANSFERABLE_TYPE_DRAG, nType, pData );
}
#endif	// MACOSX

// ----------------------------------------------------------------------------

#ifdef MACOSX
static OSStatus ImplScrapPromiseKeeperCallback( ScrapRef aScrap, ScrapFlavorType nType, void *pData )
{
	return ImplSetTransferableData( (void *)aScrap, JAVA_DTRANS_TRANSFERABLE_TYPE_CLIPBOARD, nType, pData );
}
#endif	// MACOSX

// ============================================================================

jclass com_sun_star_dtrans_DTransTransferable::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_dtrans_DTransTransferable::getMyClass()
{
#ifndef MACOSX
	if ( !theClass )
	{
		DTransThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "com/sun/star/dtrans/DTransTransferable" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
#endif	// !MACOSX
	return theClass;
}

// ----------------------------------------------------------------------------

Any SAL_CALL com_sun_star_dtrans_DTransTransferable::getTransferData( const DataFlavor& aFlavor ) throw ( UnsupportedFlavorException, IOException, RuntimeException )
{
	if ( mxTransferable.is() )
		return mxTransferable->getTransferData( aFlavor );

	Any out;

#ifdef MACOSX
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
		if ( mnTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_CLIPBOARD )
		{
			nErr = GetScrapFlavorSize( (ScrapRef)mpNativeTransferable, nRequestedType, &nSize );
		}
		else if ( mnTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_DRAG )
		{
			nErr = GetFlavorDataSize( (DragRef)mpNativeTransferable, (DragItemRef)this, nRequestedType, &nSize );
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
			if ( mnTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_CLIPBOARD )
				bDataFound = ( GetScrapFlavorData( (ScrapRef)mpNativeTransferable, nRequestedType, &nSize, aData.getArray() ) == noErr );
			else if ( mnTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_DRAG )
				bDataFound = ( GetFlavorData( (DragRef)mpNativeTransferable, (DragItemRef)this, nRequestedType, aData.getArray(), &nSize, 0 ) == noErr );

			if ( bDataFound )
			{
				if ( aFlavor.DataType.equals( getCppuType( ( OUString* )0 ) ) )
				{
					OUString aString;
					sal_Int32 nLen;
					if ( nRequestedType != 'utxt' )
					{
						nLen = aData.getLength();
						if ( ( (sal_Char *)aData.getArray() )[ nLen - 1 ] == 0 )
							nLen--;
						aString = OUString( (sal_Char *)aData.getArray(), nLen, RTL_TEXTENCODING_ASCII_US );
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
					else if ( nRequestedType == 'TIFF' )
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
#else // MACOSX
#ifdef DEBUG
	fprintf( stderr, "DTransTransferable::getTransferData not implemented\n" );
#endif
#endif	// MACOSX

	return out;
}

// ----------------------------------------------------------------------------

com_sun_star_dtrans_DTransTransferable::~com_sun_star_dtrans_DTransTransferable()
{
	MutexGuard aGuard( aMutex );

	aTransferableList.remove( this );
}

// ----------------------------------------------------------------------------

Sequence< DataFlavor > SAL_CALL com_sun_star_dtrans_DTransTransferable::getTransferDataFlavors() throw ( RuntimeException )
{
	if ( mxTransferable.is() )
		return mxTransferable->getTransferDataFlavors();

	Sequence< DataFlavor > out;

#ifdef MACOSX
	if ( mnTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_CLIPBOARD )
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
	else if ( mnTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_DRAG )
	{
		UInt16 nCount;
		if ( CountDragItemFlavors( (DragRef)mpNativeTransferable, (DragItemRef)this, &nCount ) == noErr && nCount > 0 )
		{
			for ( USHORT i = 0; i < nSupportedTypes; i++ )
			{
				for ( UInt16 j = 0; j < nCount; j++ )
				{
					FlavorType nFlavor;
					if ( GetFlavorType( (DragRef)mpNativeTransferable, (DragItemRef)this, j, &nFlavor ) == noErr && aSupportedNativeTypes[ i ] == nFlavor )
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
#else // MACOSX
#ifdef DEBUG
	fprintf( stderr, "DTransTransferable::getTransferData not implemented\n" );
#endif
#endif	// MACOSX

	return out;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_dtrans_DTransTransferable::hasOwnership()
{
	sal_Bool out = sal_False;

#ifdef MACOSX
	if ( mnTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_CLIPBOARD )
	{
		ScrapRef aScrap;

		if ( GetCurrentScrap( &aScrap ) == noErr && aScrap == (ScrapRef)mpNativeTransferable )
			out = sal_True;
	}
#else // MACOSX
#ifdef DEBUG
	fprintf( stderr, "DTransTransferable::transferToClipboard not implemented\n" );
#endif
#endif	// MACOSX

	return out;
}

// ----------------------------------------------------------------------------

sal_Bool SAL_CALL com_sun_star_dtrans_DTransTransferable::isDataFlavorSupported( const DataFlavor& aFlavor ) throw ( RuntimeException )
{
	if ( mxTransferable.is() )
		return mxTransferable->isDataFlavorSupported( aFlavor );

	sal_Bool out = sal_False;

#ifdef MACOSX
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
		if ( mnTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_CLIPBOARD )
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
		else if ( mnTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_DRAG )
		{
			UInt16 nCount;
			if ( CountDragItemFlavors( (DragRef)mpNativeTransferable, (DragItemRef)this, &nCount ) == noErr && nCount > 0 )
			{
				for ( UInt16 i = 0; i < nCount; i++ )
				{
					FlavorType nFlavor;
					if ( GetFlavorType( (DragRef)mpNativeTransferable, (DragItemRef)this, i, &nFlavor ) == noErr && nFlavor == nRequestedType && aFlavor.DataType.equals( aRequestedDataType ) )
					{
						out = sal_True;
						break;
					}
				}
			}
		}
	}
#else // MACOSX
#ifdef DEBUG
	fprintf( stderr, "DTransTransferable::isDataFlavorSupported not implemented\n" );
#endif
#endif	// MACOSX

	return out;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_dtrans_DTransTransferable::setContents( const Reference< XTransferable > &xTransferable )
{
	sal_Bool out = sal_False;

	MutexGuard aGuard( aMutex );

	mxTransferable = xTransferable;
	if ( mxTransferable.is() )
	{
#ifdef MACOSX
		if ( mnTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_CLIPBOARD )
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
				BOOL bTextOnly = FALSE;
				sal_Int32 i;
				for ( i = 0; i < nLen; i++ )
				{
					for ( USHORT j = 0; j < nSupportedTypes; j++ )
					{
						if ( xFlavors[ i ].MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ j ] ) && aSupportedTextTypes[ j ] )
						{
							bTextOnly = TRUE;
							break;
						}
					}
				}

				aTransferableList.push_back( this );

				BOOL bRenderImmediately = FALSE;
				if ( !pScrapPromiseKeeperUPP )
					pScrapPromiseKeeperUPP = NewScrapPromiseKeeperUPP( (ScrapPromiseKeeperProcPtr)ImplScrapPromiseKeeperCallback );
				if ( !pScrapPromiseKeeperUPP || SetScrapPromiseKeeper( (ScrapRef)mpNativeTransferable, pScrapPromiseKeeperUPP, (const void *)this ) != noErr )
					bRenderImmediately = TRUE;

				for ( i = 0; i < nLen; i++ )
				{ 
					for ( USHORT j = 0; j < nSupportedTypes; j++ )
					{
						if ( xFlavors[ i ].MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ j ] ) )
						{
							if ( bTextOnly && !aSupportedTextTypes[ j ] )
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
		else if ( mnTransferableType == JAVA_DTRANS_TRANSFERABLE_TYPE_DRAG )
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
			BOOL bTextOnly = FALSE;
			sal_Int32 i;
			for ( i = 0; i < nLen; i++ )
			{
				for ( USHORT j = 0; j < nSupportedTypes; j++ )
				{
					if ( xFlavors[ i ].MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ j ] ) && aSupportedTextTypes[ j ] )
					{
						bTextOnly = TRUE;
						break;
					}
				}
			}

			aTransferableList.push_back( this );

			BOOL bRenderImmediately = FALSE;
			if ( !pDragSendDataUPP )
				pDragSendDataUPP = NewDragSendDataUPP( (DragSendDataProcPtr)ImplDragSendDataCallback );
			if ( !pDragSendDataUPP || SetDragSendProc( (DragRef)mpNativeTransferable, pDragSendDataUPP, (void *)this ) != noErr )
				bRenderImmediately = TRUE;

			for ( i = 0; i < nLen; i++ )
			{ 
				for ( USHORT j = 0; j < nSupportedTypes; j++ )
				{
					if ( xFlavors[ i ].MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ j ] ) )
					{
						if ( bTextOnly && !aSupportedTextTypes[ j ] )
							continue;

						if ( bRenderImmediately )
							ImplDragSendDataCallback( aSupportedNativeTypes[ j ], (void *)this, (DragItemRef)this, (DragRef)mpNativeTransferable );
						else
							AddDragItemFlavor( (DragRef)mpNativeTransferable, (DragItemRef)this, aSupportedNativeTypes[ j ], NULL, 0, 0 );
					}
				}
			}
		}
#else	// MACOSX
#ifdef DEBUG
		fprintf( stderr, "DTransTransferable::setContents not implemented\n" );
#endif
#endif	// MACOSX
	}

	return out;
}
