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

static UInt32 nSupportedTypes = 5;

// List of supported native types in priority order
static FourCharCode aSupportedNativeTypes[] = {
	'RTF ',
	'utxt',
	'TEXT',
	'TIFF',
	'PICT'
};

// List of supported types that are text
static BOOL aSupportedTextTypes[] = {
	TRUE,
	TRUE,
	TRUE,
	FALSE,
	FALSE
};

// List of supported mime types in priority order
static OUString aSupportedMimeTypes[] = {
	OUString::createFromAscii( "text/richtext" ),
	OUString::createFromAscii( "text/plain;charset=utf-16" ),
	OUString::createFromAscii( "text/plain;charset=utf-16" ),
	OUString::createFromAscii( "image/bmp" ),
	OUString::createFromAscii( "image/bmp" )
};

// List of supported data types in priority order
static ::com::sun::star::uno::Type aSupportedDataTypes[] = {
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( OUString* )0 ),
	getCppuType( ( OUString* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 ),
	getCppuType( ( ::com::sun::star::uno::Sequence< sal_Int8 >* )0 )
};

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
static OSStatus ImplScrapPromiseKeeperCallback( ScrapRef aScrap, ScrapFlavorType nType, void *pData )
{
	OSStatus nErr = noTypeErr;

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
					sal_Int32 nLen = aString.getLength();
					sal_Int32 j = 0;
					for ( j = 0; j < nLen; j++ )
					{
						if ( pArray[ j ] == (sal_Unicode)'\n' )
							pArray[ j ] = (sal_Unicode)'\r';
					}

					if ( nType != 'utxt' )
					{
						OString aEncodedString = OUStringToOString( aString, gsl_getSystemTextEncoding() );
						sal_Int8 *pData = (sal_Int8 *)aEncodedString.getStr();
						MacOSSize nDataLen = aEncodedString.getLength();

						if ( pData && nDataLen )
							nErr = PutScrapFlavor( (ScrapRef)pTransferable->getNativeTransferable(), nType, kScrapFlavorMaskNone, nDataLen, (const void *)pData );
					}
					else
					{
						sal_Int8 *pData = (sal_Int8 *)aString.getStr();
						MacOSSize nDataLen = aString.getLength() * sizeof( sal_Unicode );

						if ( pData && nDataLen )
							nErr = PutScrapFlavor( (ScrapRef)pTransferable->getNativeTransferable(), nType, kScrapFlavorMaskNone, nDataLen, (const void *)pData );
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
										nErr = PutScrapFlavor( (ScrapRef)pTransferable->getNativeTransferable(), nType, kScrapFlavorMaskNone, GetHandleSize( (Handle)hPict ), (const void *)*hPict );
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
														nErr = PutScrapFlavor( (ScrapRef)pTransferable->getNativeTransferable(), nType, kScrapFlavorMaskNone, nDataLen, (const void *)*hExportData );
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
							nErr = PutScrapFlavor( (ScrapRef)pTransferable->getNativeTransferable(), nType, kScrapFlavorMaskNone, nDataLen, (const void *)pData );
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

		MacOSSize aSize;

		nErr = GetScrapFlavorSize( (ScrapRef)mpNativeTransferable, nRequestedType, &aSize );
		if ( nErr == noErr )
		{
			Sequence< sal_Int8 > aData( aSize );
			if ( GetScrapFlavorData( (ScrapRef)mpNativeTransferable, nRequestedType, &aSize, aData.getArray() ) == noErr )
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
						aString = OUString( (sal_Char *)aData.getArray(), nLen, gsl_getSystemTextEncoding() );
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
		if ( nErr == noTypeErr )
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
	ScrapRef aScrap;

	if ( GetCurrentScrap( &aScrap ) == noErr && aScrap == (ScrapRef)mpNativeTransferable )
		out = sal_True;
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

			// Test the JVM version and if it is below 1.4, render immediately
			// since some data flavors require Java to render which, if done
			// in the callback, will cause the application to crash when the
			// next AEEvent is dispatched so we can't risk using delayed
			// rendering
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
#else	// MACOSX
#ifdef DEBUG
		fprintf( stderr, "DTransTransferable::setContents not implemented\n" );
#endif
#endif	// MACOSX
	}

	return out;
}
