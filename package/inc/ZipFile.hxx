/*************************************************************************
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of NeoOffice.
 *
 * NeoOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * NeoOffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 3 along with NeoOffice.  If not, see
 * <http://www.gnu.org/licenses/gpl-3.0.txt>
 * for a copy of the GPLv3 License.
 *
 * Modified March 2013 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/
#ifndef _ZIP_FILE_HXX
#define _ZIP_FILE_HXX

#include <com/sun/star/packages/zip/ZipException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/packages/NoEncryptionException.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <ByteGrabber.hxx>
#include <HashMaps.hxx>
#ifndef _INFLATER_HXX
#include <Inflater.hxx>
#endif


namespace com { namespace sun { namespace star {
	namespace lang { class XMultiServiceFactory; }
	namespace ucb  { class XProgressHandler; }
} } }
namespace vos 
{
	template < class T > class ORef;
}
/*  
 * We impose arbitrary but reasonable limit on ZIP files.
 */ 

#define ZIP_MAXNAMELEN 512
#define ZIP_MAXEXTRA 256
#define ZIP_MAXENTRIES (0x10000 - 2)

typedef void* rtlCipher;
class ZipEnumeration;
class EncryptionData;

class ZipFile
{
protected:
    ::rtl::OUString	sName;	  	  	/* zip file name */
    ::rtl::OUString	sComment; 	  	/* zip file comment */
	EntryHash		aEntries;
	ByteGrabber 	aGrabber;
    Inflater        aInflater;
	com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xStream;
	com::sun::star::uno::Reference < com::sun::star::io::XSeekable > xSeek;
	const ::com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xFactory;
	::com::sun::star::uno::Reference < ::com::sun::star::ucb::XProgressHandler > xProgressHandler;

	sal_Bool bRecoveryMode;

	com::sun::star::uno::Reference < com::sun::star::io::XInputStream >  createMemoryStream( 
			ZipEntry & rEntry,
			const vos::ORef < EncryptionData > &rData, 
			sal_Bool bRawStream,
			sal_Bool bDecrypt );

	com::sun::star::uno::Reference < com::sun::star::io::XInputStream >  createFileStream(
			ZipEntry & rEntry,
			const vos::ORef < EncryptionData > &rData, 
			sal_Bool bRawStream,
			sal_Bool bDecrypt );

	// aMediaType parameter is used only for raw stream header creation
	com::sun::star::uno::Reference < com::sun::star::io::XInputStream >  createUnbufferedStream(
			ZipEntry & rEntry,
			const vos::ORef < EncryptionData > &rData, 
			sal_Int8 nStreamMode,
			sal_Bool bDecrypt,
			::rtl::OUString aMediaType = ::rtl::OUString() );

	sal_Bool hasValidPassword ( ZipEntry & rEntry, const vos::ORef < EncryptionData > &rData );

	sal_Bool checkSizeAndCRC( const ZipEntry& aEntry );

	sal_Int32 getCRC( sal_Int32 nOffset, sal_Int32 nSize );

	void getSizeAndCRC( sal_Int32 nOffset, sal_Int32 nCompressedSize, sal_Int32 *nSize, sal_Int32 *nCRC );

public:

	ZipFile( com::sun::star::uno::Reference < com::sun::star::io::XInputStream > &xInput, 
			 const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > &xNewFactory, 
			 sal_Bool bInitialise
			 )
		throw(::com::sun::star::io::IOException, com::sun::star::packages::zip::ZipException, com::sun::star::uno::RuntimeException);

	ZipFile( com::sun::star::uno::Reference < com::sun::star::io::XInputStream > &xInput, 
			 const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > &xNewFactory, 
			 sal_Bool bInitialise,
			 sal_Bool bForceRecover,
			 ::com::sun::star::uno::Reference < ::com::sun::star::ucb::XProgressHandler > xProgress
			 )
		throw(::com::sun::star::io::IOException, com::sun::star::packages::zip::ZipException, com::sun::star::uno::RuntimeException);

	~ZipFile();

	EntryHash& GetEntryHash() { return aEntries; }

	void setInputStream ( com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xNewStream );
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getRawData( 
			ZipEntry& rEntry,
			const vos::ORef < EncryptionData > &rData,
			sal_Bool bDecrypt)
		throw(::com::sun::star::io::IOException, ::com::sun::star::packages::zip::ZipException, ::com::sun::star::uno::RuntimeException);
	
	static void StaticGetCipher ( const vos::ORef < EncryptionData > & xEncryptionData, rtlCipher &rCipher );

#ifndef NO_OOO_3_4_1_AES_ENCRYPTION
	static sal_Bool StaticGetDecryptedData( const ::com::sun::star::uno::Sequence< sal_Int8 > &aReadBuffer, const vos::ORef < EncryptionData > &rData, ::com::sun::star::uno::Sequence< sal_Int8 > &aDecryptedBuffer );
#endif	// !NO_OOO_3_4_1_AES_ENCRYPTION

	static void StaticFillHeader ( const vos::ORef < EncryptionData > & rData,
									sal_Int32 nSize,
									const ::rtl::OUString& aMediaType,
									sal_Int8 * & pHeader );

	static sal_Bool StaticFillData ( vos::ORef < EncryptionData > & rData,
									 sal_Int32 &rSize,
									 ::rtl::OUString& aMediaType,
									 ::com::sun::star::uno::Reference < com::sun::star::io::XInputStream > &rStream );

	static ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > StaticGetDataFromRawStream(
			const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xStream,
			const vos::ORef < EncryptionData > &rData )
		throw ( ::com::sun::star::packages::WrongPasswordException,
				::com::sun::star::packages::zip::ZipIOException,
				::com::sun::star::uno::RuntimeException );
				
	static sal_Bool StaticHasValidPassword ( const ::com::sun::star::uno::Sequence< sal_Int8 > &aReadBuffer,
											 const vos::ORef < EncryptionData > &rData );
	

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream( 
			ZipEntry& rEntry,
			const vos::ORef < EncryptionData > &rData,
			sal_Bool bDecrypt )
		throw(::com::sun::star::io::IOException, ::com::sun::star::packages::zip::ZipException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getDataStream( 
			ZipEntry& rEntry,
			const vos::ORef < EncryptionData > &rData,
			sal_Bool bDecrypt )
		throw ( ::com::sun::star::packages::WrongPasswordException,
				::com::sun::star::io::IOException,
				::com::sun::star::packages::zip::ZipException,
				::com::sun::star::uno::RuntimeException );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getWrappedRawStream( 
			ZipEntry& rEntry,
			const vos::ORef < EncryptionData > &rData,
			const ::rtl::OUString& aMediaType )
		throw ( ::com::sun::star::packages::NoEncryptionException,
				::com::sun::star::io::IOException,
				::com::sun::star::packages::zip::ZipException,
				::com::sun::star::uno::RuntimeException );

    ZipEnumeration * SAL_CALL entries(  );
protected:
	sal_Bool		readLOC ( ZipEntry &rEntry)
		throw(::com::sun::star::io::IOException, com::sun::star::packages::zip::ZipException, com::sun::star::uno::RuntimeException);
	sal_Int32		readCEN()
		throw(::com::sun::star::io::IOException, com::sun::star::packages::zip::ZipException, com::sun::star::uno::RuntimeException);
	sal_Int32		findEND()
		throw(::com::sun::star::io::IOException, com::sun::star::packages::zip::ZipException, com::sun::star::uno::RuntimeException);
	sal_Int32 		recover()
		throw(::com::sun::star::io::IOException, com::sun::star::packages::zip::ZipException, com::sun::star::uno::RuntimeException);

};

#endif
