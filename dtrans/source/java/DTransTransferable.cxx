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

typedef OSStatus ClearCurrentScrap_Type( void );
typedef OSErr CloseComponent_Type( ComponentInstance );
typedef void DisposeHandle_Type( Handle );
typedef OSStatus GetCurrentScrap_Type( ScrapRef * );
typedef OSStatus GetScrapFlavorData_Type( ScrapRef, ScrapFlavorType, MacOSSize *, void * );
typedef OSStatus GetScrapFlavorSize_Type( ScrapRef, ScrapFlavorType, MacOSSize * );
typedef OSStatus GetScrapFlavorCount_Type( ScrapRef, UInt32 * );
typedef OSStatus GetScrapFlavorInfoList_Type( ScrapRef, UInt32 *, ScrapFlavorInfo[] );
typedef void KillPicture_Type( PicHandle );
typedef Handle NewHandle_Type( MacOSSize );
typedef ScrapPromiseKeeperUPP NewScrapPromiseKeeperUPP_Type( ScrapPromiseKeeperProcPtr );
typedef OSErr OpenADefaultComponent_Type( OSType, OSType, ComponentInstance * );
typedef OSErr PtrToHand_Type( const void *, Handle *, long );
typedef OSStatus PutScrapFlavor_Type( ScrapRef, ScrapFlavorType, ScrapFlavorFlags, MacOSSize, const void * );
typedef OSStatus SetScrapPromiseKeeper_Type( ScrapRef, ScrapPromiseKeeperUPP, const void * );

using namespace rtl;
using namespace vos;

static UInt32 nSupportedTypes = 5;

// List of support native types in priority order
static FourCharCode aSupportedNativeTypes[] = {
	'RTF ',
	'utxt',
	'TEXT',
	'TIFF',
	'PICT'
};

// List of support mime types in priority order
static OUString aSupportedMimeTypes[] = {
	OUString::createFromAscii( "text/richtext" ),
	OUString::createFromAscii( "text/plain;charset=utf-16" ),
	OUString::createFromAscii( "text/plain;charset=utf-16" ),
	OUString::createFromAscii( "application/x-openoffice;windows_formatname=\"Bitmap\"" ),
	OUString::createFromAscii( "application/x-openoffice;windows_formatname=\"Bitmap\"" )
};

// List of support data types in priority order
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

	com_sun_star_dtrans_DTransTransferable *pTransferable = (com_sun_star_dtrans_DTransTransferable *)pData;

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
		// Load Carbon
		OModule aModule;
		if ( aModule.load( OUString::createFromAscii( "/System/Library/Frameworks/Carbon.framework/Carbon" ) ) )
		{
			CloseComponent_Type *pCloseComponent = (CloseComponent_Type *)aModule.getSymbol( OUString::createFromAscii( "CloseComponent" ) );
			DisposeHandle_Type *pDisposeHandle = (DisposeHandle_Type *)aModule.getSymbol( OUString::createFromAscii( "DisposeHandle" ) );
			KillPicture_Type *pKillPicture = (KillPicture_Type *)aModule.getSymbol( OUString::createFromAscii( "KillPicture" ) );
			NewHandle_Type *pNewHandle = (NewHandle_Type *)aModule.getSymbol( OUString::createFromAscii( "NewHandle" ) );
			OpenADefaultComponent_Type *pOpenADefaultComponent = (OpenADefaultComponent_Type *)aModule.getSymbol( OUString::createFromAscii( "OpenADefaultComponent" ) );
			PtrToHand_Type *pPtrToHand = (PtrToHand_Type *)aModule.getSymbol( OUString::createFromAscii( "PtrToHand" ) );
			PutScrapFlavor_Type *pPutScrapFlavor = (PutScrapFlavor_Type *)aModule.getSymbol( OUString::createFromAscii( "PutScrapFlavor" ) );
			if ( pCloseComponent && pDisposeHandle && pKillPicture && pNewHandle && pOpenADefaultComponent && pPtrToHand && pPutScrapFlavor )
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
					Any aValue( pTransferable->getTransferData( aFlavor ) );

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

						if ( nType == 'TEXT' )
						{
							OString aEncodedString = OUStringToOString( aString, gsl_getSystemTextEncoding() );
							sal_Int8 *pData = (sal_Int8 *)aEncodedString.getStr();
							MacOSSize nDataLen = aEncodedString.getLength();

							if ( pData && nDataLen )
								nErr = pPutScrapFlavor( (ScrapRef)pTransferable->getNativeTransferable(), nType, kScrapFlavorMaskNone, nDataLen, (const void *)pData );
						}
						else
						{
							sal_Int8 *pData = (sal_Int8 *)aString.getStr();
							MacOSSize nDataLen = aString.getLength() * sizeof( sal_Unicode );

							if ( pData && nDataLen )
								nErr = pPutScrapFlavor( (ScrapRef)pTransferable->getNativeTransferable(), nType, kScrapFlavorMaskNone, nDataLen, (const void *)pData );
						}
					}
					else if ( aValue.getValueType().equals( getCppuType( ( Sequence< sal_Int8 >* )0 ) ) )
					{
						Sequence< sal_Int8 > aData;
						aValue >>= aData;

						if ( nType == 'PICT' || nType == 'TIFF' )
						{
							// Convert to PICT or TIFF from our BMP data
							ComponentInstance aImporter;
							if ( pOpenADefaultComponent( GraphicsImporterComponentType, 'BMPf', &aImporter ) == noErr )
							{
								Handle hData;
								if ( pPtrToHand( aData.getArray(), &hData, aData.getLength() ) == noErr )
								{
									// Free the source data
									aData = Sequence< sal_Int8 >();

									if ( GraphicsImportSetDataHandle( aImporter, hData ) == noErr )
									{
										PicHandle hPict;
										if ( GraphicsImportGetAsPicture( aImporter, &hPict ) == noErr )
										{
											ComponentInstance aExporter;
											if ( pOpenADefaultComponent( GraphicsExporterComponentType, nType, &aExporter ) == noErr );
											{
												if ( GraphicsExportSetInputPicture( aExporter, hPict ) == noErr )
												{
													Handle hExportData = pNewHandle( 0 );
													if ( GraphicsExportSetOutputHandle( aExporter, hExportData ) == noErr )
													{
														unsigned long nDataLen;
														if ( GraphicsExportDoExport( aExporter, &nDataLen ) == noErr )
															nErr = pPutScrapFlavor( (ScrapRef)pTransferable->getNativeTransferable(), nType, kScrapFlavorMaskNone, nDataLen, (const void *)*hExportData );
														pDisposeHandle( hExportData );
													}
												}
												pCloseComponent( aExporter );
											}
											pKillPicture( hPict );
										}
										pDisposeHandle( hData );
									}
									pCloseComponent( aImporter );
								}
							}
						}
						else
						{
							sal_Int8 *pData = aData.getArray();
							MacOSSize nDataLen = aData.getLength();

							if ( pData && nDataLen )
								nErr = pPutScrapFlavor( (ScrapRef)pTransferable->getNativeTransferable(), nType, kScrapFlavorMaskNone, nDataLen, (const void *)pData );
						}
					}
				}
			}
	
			aModule.unload();
		}
	}

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

		// Test the JVM version and if it is below 1.4, use Carbon APIs or else
		// use Cocoa APIs
		java_lang_Class* pClass = java_lang_Class::forName( OUString::createFromAscii( "java/lang/CharSequence" ) );
		if ( !pClass )
		{
			// Load Carbon
			OModule aModule;
			if ( aModule.load( OUString::createFromAscii( "/System/Library/Frameworks/Carbon.framework/Carbon" ) ) )
			{
				CloseComponent_Type *pCloseComponent = (CloseComponent_Type *)aModule.getSymbol( OUString::createFromAscii( "CloseComponent" ) );
				DisposeHandle_Type *pDisposeHandle = (DisposeHandle_Type *)aModule.getSymbol( OUString::createFromAscii( "DisposeHandle" ) );
				GetScrapFlavorData_Type *pGetScrapFlavorData = (GetScrapFlavorData_Type *)aModule.getSymbol( OUString::createFromAscii( "GetScrapFlavorData" ) );
				GetScrapFlavorSize_Type *pGetScrapFlavorSize = (GetScrapFlavorSize_Type *)aModule.getSymbol( OUString::createFromAscii( "GetScrapFlavorSize" ) );
				KillPicture_Type *pKillPicture = (KillPicture_Type *)aModule.getSymbol( OUString::createFromAscii( "KillPicture" ) );
				NewHandle_Type *pNewHandle = (NewHandle_Type *)aModule.getSymbol( OUString::createFromAscii( "NewHandle" ) );
				OpenADefaultComponent_Type *pOpenADefaultComponent = (OpenADefaultComponent_Type *)aModule.getSymbol( OUString::createFromAscii( "OpenADefaultComponent" ) );
				PtrToHand_Type *pPtrToHand = (PtrToHand_Type *)aModule.getSymbol( OUString::createFromAscii( "PtrToHand" ) );
				if ( pCloseComponent && pDisposeHandle && pGetScrapFlavorData && pGetScrapFlavorSize && pKillPicture && pNewHandle && pOpenADefaultComponent && pPtrToHand )
				{
					MacOSSize aSize;

					nErr = pGetScrapFlavorSize( (ScrapRef)mpNativeTransferable, nRequestedType, &aSize );
					if ( nErr == noErr )
					{
						Sequence< sal_Int8 > aData( aSize );
						if ( pGetScrapFlavorData( (ScrapRef)mpNativeTransferable, nRequestedType, &aSize, aData.getArray() ) == noErr )
						{
							if ( aFlavor.DataType.equals( getCppuType( ( OUString* )0 ) ) )
							{
								OUString aString;
								sal_Int32 nLen;
								if ( nRequestedType == 'TEXT' )
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
								if ( nRequestedType == 'PICT' || nRequestedType == 'TIFF' )
								{
									// Convert to BMP format
									ComponentInstance aImporter;
									if ( pOpenADefaultComponent( GraphicsImporterComponentType, nRequestedType, &aImporter ) == noErr )
									{
										Handle hData;
										if ( pPtrToHand( aData.getArray(), &hData, aData.getLength() ) == noErr )
										{
											// Free the source data
											aData = Sequence< sal_Int8 >();

											if ( GraphicsImportSetDataHandle( aImporter, hData ) == noErr )
											{
												PicHandle hPict;
												if ( GraphicsImportGetAsPicture( aImporter, &hPict ) == noErr )
												{
													ComponentInstance aExporter;
													if ( pOpenADefaultComponent( GraphicsExporterComponentType, 'BMPf', &aExporter ) == noErr );
													{
														if ( GraphicsExportSetInputPicture( aExporter, hPict ) == noErr )
														{
															Handle hExportData = pNewHandle( 0 );
															if ( GraphicsExportSetOutputHandle( aExporter, hExportData ) == noErr )
															{
																unsigned long nDataLen;
																if ( GraphicsExportDoExport( aExporter, &nDataLen ) == noErr )
																{
																	Sequence< sal_Int8 > aExportData( nDataLen );
																	memcpy( aExportData.getArray(), *hExportData, aExportData.getLength() );
																	out <<= aExportData;
																}
																pDisposeHandle( hExportData );
															}
														}
														pCloseComponent( aExporter );
													}
													pKillPicture( hPict );
												}
											}
											pDisposeHandle( hData );
										}
										pCloseComponent( aImporter );
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

				aModule.unload();
			}
		}
		else
		{
			delete pClass;
#ifdef DEBUG
			fprintf( stderr, "DTransTransferable::getTransferData not implemented\n" );
#endif
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
	// Test the JVM version and if it is below 1.4, use Carbon APIs or else
	// use Cocoa APIs
	java_lang_Class* pClass = java_lang_Class::forName( OUString::createFromAscii( "java/lang/CharSequence" ) );
	if ( !pClass )
	{
		// Load Carbon
		OModule aModule;
		if ( aModule.load( OUString::createFromAscii( "/System/Library/Frameworks/Carbon.framework/Carbon" ) ) )
		{
			GetScrapFlavorCount_Type *pGetScrapFlavorCount = (GetScrapFlavorCount_Type *)aModule.getSymbol( OUString::createFromAscii( "GetScrapFlavorCount" ) );
			GetScrapFlavorInfoList_Type *pGetScrapFlavorInfoList = (GetScrapFlavorInfoList_Type *)aModule.getSymbol( OUString::createFromAscii( "GetScrapFlavorInfoList" ) );
			if ( pGetScrapFlavorCount && pGetScrapFlavorInfoList )
			{
				UInt32 nCount;

				if ( pGetScrapFlavorCount( (ScrapRef)mpNativeTransferable, &nCount ) == noErr && nCount > 0 )
				{
					ScrapFlavorInfo *pInfo = new ScrapFlavorInfo[ nCount ];

					if ( pGetScrapFlavorInfoList( (ScrapRef)mpNativeTransferable, &nCount, pInfo ) == noErr )
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

			aModule.unload();
		}
	}
	else
	{
		delete pClass;
#ifdef DEBUG
		fprintf( stderr, "DTransTransferable::getTransferData not implemented\n" );
#endif
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
	sal_Bool out = FALSE;

#ifdef MACOSX
	// Test the JVM version and if it is below 1.4, use Carbon APIs or else
	// use Cocoa APIs
	java_lang_Class* pClass = java_lang_Class::forName( OUString::createFromAscii( "java/lang/CharSequence" ) );
	if ( !pClass )
	{
		// Load Carbon
		OModule aModule;
		if ( aModule.load( OUString::createFromAscii( "/System/Library/Frameworks/Carbon.framework/Carbon" ) ) )
		{
			GetCurrentScrap_Type *pGetCurrentScrap = (GetCurrentScrap_Type *)aModule.getSymbol( OUString::createFromAscii( "GetCurrentScrap" ) );
			if ( pGetCurrentScrap )
			{
				ScrapRef aScrap;

				if ( pGetCurrentScrap( &aScrap ) == noErr && aScrap == (ScrapRef)mpNativeTransferable )
					out = TRUE;
			}
			aModule.unload();
		}
	}
	else
	{
		delete pClass;
#ifdef DEBUG
		fprintf( stderr, "DTransTransferable::transferToClipboard not implemented\n" );
#endif
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

	sal_Bool out = FALSE;

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
		// Test the JVM version and if it is below 1.4, use Carbon APIs or else
		// use Cocoa APIs
		java_lang_Class* pClass = java_lang_Class::forName( OUString::createFromAscii( "java/lang/CharSequence" ) );
		if ( !pClass )
		{
			// Load Carbon
			OModule aModule;
			if ( aModule.load( OUString::createFromAscii( "/System/Library/Frameworks/Carbon.framework/Carbon" ) ) )
			{
				GetScrapFlavorCount_Type *pGetScrapFlavorCount = (GetScrapFlavorCount_Type *)aModule.getSymbol( OUString::createFromAscii( "GetScrapFlavorCount" ) );
				GetScrapFlavorInfoList_Type *pGetScrapFlavorInfoList = (GetScrapFlavorInfoList_Type *)aModule.getSymbol( OUString::createFromAscii( "GetScrapFlavorInfoList" ) );
				if ( pGetScrapFlavorCount && pGetScrapFlavorInfoList )
				{
					UInt32 nCount;

					if ( pGetScrapFlavorCount( (ScrapRef)mpNativeTransferable, &nCount ) == noErr && nCount > 0 )
					{
						ScrapFlavorInfo *pInfo = new ScrapFlavorInfo[ nCount ];

						if ( pGetScrapFlavorInfoList( (ScrapRef)mpNativeTransferable, &nCount, pInfo ) == noErr )
						{
							for ( UInt32 i = 0; i < nCount; i++ )
							{
								if ( pInfo[ i ].flavorType == nRequestedType && aFlavor.DataType.equals( aRequestedDataType ) )
								{
									out = TRUE;
									break;
								}
							}
						}

						delete[] pInfo;
					}
				}

				aModule.unload();
			}
		}
		else
		{
			delete pClass;
#ifdef DEBUG
			fprintf( stderr, "DTransTransferable::isDataFlavorSupported not implemented\n" );
#endif
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
	sal_Bool out = FALSE;

	MutexGuard aGuard( aMutex );

	mxTransferable = xTransferable;
	if ( mxTransferable.is() )
	{
#ifdef MACOSX
		// Test the JVM version and if it is below 1.4, use Carbon APIs or else
		// use Cocoa APIs
		java_lang_Class* pClass = java_lang_Class::forName( OUString::createFromAscii( "java/lang/CharSequence" ) );
		if ( !pClass )
		{
			// Load Carbon
			OModule aModule;
			if ( aModule.load( OUString::createFromAscii( "/System/Library/Frameworks/Carbon.framework/Carbon" ) ) )
			{
				ClearCurrentScrap_Type *pClearCurrentScrap = (ClearCurrentScrap_Type *)aModule.getSymbol( OUString::createFromAscii( "ClearCurrentScrap" ) );
				GetCurrentScrap_Type *pGetCurrentScrap = (GetCurrentScrap_Type *)aModule.getSymbol( OUString::createFromAscii( "GetCurrentScrap" ) );
				NewScrapPromiseKeeperUPP_Type *pNewScrapPromiseKeeperUPP = (NewScrapPromiseKeeperUPP_Type *)aModule.getSymbol( OUString::createFromAscii( "NewScrapPromiseKeeperUPP" ) );
				PutScrapFlavor_Type *pPutScrapFlavor = (PutScrapFlavor_Type *)aModule.getSymbol( OUString::createFromAscii( "PutScrapFlavor" ) );
				SetScrapPromiseKeeper_Type *pSetScrapPromiseKeeper = (SetScrapPromiseKeeper_Type *)aModule.getSymbol( OUString::createFromAscii( "SetScrapPromiseKeeper" ) );
				if ( pClearCurrentScrap && pGetCurrentScrap && pNewScrapPromiseKeeperUPP && pPutScrapFlavor && pSetScrapPromiseKeeper )
				{
					ScrapRef aScrap;
					if ( pClearCurrentScrap() == noErr && pGetCurrentScrap( &aScrap ) == noErr )
					{
						// We have now cleared the scrap so we now own it
						mpNativeTransferable = aScrap;
						out = TRUE;

						if ( !pScrapPromiseKeeperUPP )
							pScrapPromiseKeeperUPP = pNewScrapPromiseKeeperUPP( (ScrapPromiseKeeperProcPtr)ImplScrapPromiseKeeperCallback );

						if ( pScrapPromiseKeeperUPP && pSetScrapPromiseKeeper( (ScrapRef)mpNativeTransferable, pScrapPromiseKeeperUPP, (const void *)this ) == noErr )
						{
							Sequence< DataFlavor > xFlavors;
							try
							{
								xFlavors = mxTransferable->getTransferDataFlavors();
							}
							catch ( Exception )
							{
							}

							for ( sal_Int32 i = 0; i < xFlavors.getLength(); i++ )
							{ 
								for ( USHORT j = 0; j < nSupportedTypes; j++ )
								{
									if ( xFlavors[ i ].MimeType.equalsIgnoreAsciiCase( aSupportedMimeTypes[ j ] ) )
										pPutScrapFlavor( (ScrapRef)mpNativeTransferable, aSupportedNativeTypes[ j ], kScrapFlavorMaskNone, kScrapFlavorSizeUnknown, NULL );
								}
							}

							aTransferableList.push_back( this );
						}
					}
				}
	
				aModule.unload();
			}
		}
		else
		{
			delete pClass;
#ifdef DEBUG
			fprintf( stderr, "DTransClipboard::setContents not implemented\n" );
#endif
		}
#else	// MACOSX
#ifdef DEBUG
		fprintf( stderr, "DTransTransferable::setContents not implemented\n" );
#endif
#endif	// MACOSX
	}

	return out;
}
