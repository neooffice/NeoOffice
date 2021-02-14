/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <algorithm>

#include <comphelper/docpasswordhelper.hxx>
#ifndef NO_LIBO_BUG_118639_FIX
#include <comphelper/storagehelper.hxx>
#include <comphelper/sequence.hxx>
#endif	// !NO_LIBO_BUG_118639_FIX
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

#include <osl/time.h>
#include <osl/diagnose.h>
#include <rtl/digest.h>
#include <rtl/random.h>
#include <string.h>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::task::PasswordRequestMode;
using ::com::sun::star::task::PasswordRequestMode_PASSWORD_ENTER;
using ::com::sun::star::task::PasswordRequestMode_PASSWORD_REENTER;
using ::com::sun::star::task::XInteractionHandler;
using ::com::sun::star::task::XInteractionRequest;

using namespace ::com::sun::star;

namespace comphelper {



static uno::Sequence< sal_Int8 > GeneratePBKDF2Hash( const OUString& aPassword, const uno::Sequence< sal_Int8 >& aSalt, sal_Int32 nCount, sal_Int32 nHashLength )
{
    uno::Sequence< sal_Int8 > aResult;

    if ( !aPassword.isEmpty() && aSalt.getLength() && nCount && nHashLength )
    {
        OString aBytePass = OUStringToOString( aPassword, RTL_TEXTENCODING_UTF8 );
        aResult.realloc( 16 );
        rtl_digest_PBKDF2( reinterpret_cast < sal_uInt8 * > ( aResult.getArray() ),
                           aResult.getLength(),
                           reinterpret_cast < const sal_uInt8 * > ( aBytePass.getStr() ),
                           aBytePass.getLength(),
                           reinterpret_cast < const sal_uInt8 * > ( aSalt.getConstArray() ),
                           aSalt.getLength(),
                           nCount );
    }

    return aResult;
}



IDocPasswordVerifier::~IDocPasswordVerifier()
{
}


uno::Sequence< beans::PropertyValue > DocPasswordHelper::GenerateNewModifyPasswordInfo( const OUString& aPassword )
{
    uno::Sequence< beans::PropertyValue > aResult;

    uno::Sequence< sal_Int8 > aSalt = GenerateRandomByteSequence( 16 );
    sal_Int32 nCount = 1024;

    uno::Sequence< sal_Int8 > aNewHash = GeneratePBKDF2Hash( aPassword, aSalt, nCount, 16 );
    if ( aNewHash.getLength() )
    {
        aResult.realloc( 4 );
        aResult[0].Name = "algorithm-name";
        aResult[0].Value <<= OUString( "PBKDF2" );
        aResult[1].Name = "salt";
        aResult[1].Value <<= aSalt;
        aResult[2].Name = "iteration-count";
        aResult[2].Value <<= nCount;
        aResult[3].Name = "hash";
        aResult[3].Value <<= aNewHash;
    }

    return aResult;
}


bool DocPasswordHelper::IsModifyPasswordCorrect( const OUString& aPassword, const uno::Sequence< beans::PropertyValue >& aInfo )
{
    bool bResult = false;
    if ( !aPassword.isEmpty() && aInfo.getLength() )
    {
        OUString sAlgorithm;
        uno::Sequence< sal_Int8 > aSalt;
        uno::Sequence< sal_Int8 > aHash;
        sal_Int32 nCount = 0;

        for ( sal_Int32 nInd = 0; nInd < aInfo.getLength(); nInd++ )
        {
            if ( aInfo[nInd].Name == "algorithm-name" )
                aInfo[nInd].Value >>= sAlgorithm;
            else if ( aInfo[nInd].Name == "salt" )
                aInfo[nInd].Value >>= aSalt;
            else if ( aInfo[nInd].Name == "iteration-count" )
                aInfo[nInd].Value >>= nCount;
            else if ( aInfo[nInd].Name == "hash" )
                aInfo[nInd].Value >>= aHash;
        }

        if ( sAlgorithm == "PBKDF2" && aSalt.getLength() && nCount > 0 && aHash.getLength() )
        {
            uno::Sequence< sal_Int8 > aNewHash = GeneratePBKDF2Hash( aPassword, aSalt, nCount, aHash.getLength() );
            for ( sal_Int32 nInd = 0; nInd < aNewHash.getLength() && nInd < aHash.getLength() && aNewHash[nInd] == aHash[nInd]; nInd ++ )
            {
                if ( nInd == aNewHash.getLength() - 1 && nInd == aHash.getLength() - 1 )
                    bResult = true;
            }
        }
    }

    return bResult;
}


sal_uInt32 DocPasswordHelper::GetWordHashAsUINT32(
                const OUString& aUString )
{
    static const sal_uInt16 pInitialCode[] = {
        0xE1F0, // 1
        0x1D0F, // 2
        0xCC9C, // 3
        0x84C0, // 4
        0x110C, // 5
        0x0E10, // 6
        0xF1CE, // 7
        0x313E, // 8
        0x1872, // 9
        0xE139, // 10
        0xD40F, // 11
        0x84F9, // 12
        0x280C, // 13
        0xA96A, // 14
        0x4EC3  // 15
    };

    static const sal_uInt16 pEncryptionMatrix[15][7] = {
        { 0xAEFC, 0x4DD9, 0x9BB2, 0x2745, 0x4E8A, 0x9D14, 0x2A09}, // last-14
        { 0x7B61, 0xF6C2, 0xFDA5, 0xEB6B, 0xC6F7, 0x9DCF, 0x2BBF}, // last-13
        { 0x4563, 0x8AC6, 0x05AD, 0x0B5A, 0x16B4, 0x2D68, 0x5AD0}, // last-12
        { 0x0375, 0x06EA, 0x0DD4, 0x1BA8, 0x3750, 0x6EA0, 0xDD40}, // last-11
        { 0xD849, 0xA0B3, 0x5147, 0xA28E, 0x553D, 0xAA7A, 0x44D5}, // last-10
        { 0x6F45, 0xDE8A, 0xAD35, 0x4A4B, 0x9496, 0x390D, 0x721A}, // last-9
        { 0xEB23, 0xC667, 0x9CEF, 0x29FF, 0x53FE, 0xA7FC, 0x5FD9}, // last-8
        { 0x47D3, 0x8FA6, 0x8FA6, 0x1EDA, 0x3DB4, 0x7B68, 0xF6D0}, // last-7
        { 0xB861, 0x60E3, 0xC1C6, 0x93AD, 0x377B, 0x6EF6, 0xDDEC}, // last-6
        { 0x45A0, 0x8B40, 0x06A1, 0x0D42, 0x1A84, 0x3508, 0x6A10}, // last-5
        { 0xAA51, 0x4483, 0x8906, 0x022D, 0x045A, 0x08B4, 0x1168}, // last-4
        { 0x76B4, 0xED68, 0xCAF1, 0x85C3, 0x1BA7, 0x374E, 0x6E9C}, // last-3
        { 0x3730, 0x6E60, 0xDCC0, 0xA9A1, 0x4363, 0x86C6, 0x1DAD}, // last-2
        { 0x3331, 0x6662, 0xCCC4, 0x89A9, 0x0373, 0x06E6, 0x0DCC}, // last-1
        { 0x1021, 0x2042, 0x4084, 0x8108, 0x1231, 0x2462, 0x48C4}  // last
    };

    sal_uInt32 nResult = 0;
    sal_uInt32 nLen = aUString.getLength();

    if ( nLen )
    {
        if ( nLen > 15 )
            nLen = 15;

        sal_uInt16 nHighResult = pInitialCode[nLen - 1];
        sal_uInt16 nLowResult = 0;

        const sal_Unicode* pStr = aUString.getStr();
        for ( sal_uInt32 nInd = 0; nInd < nLen; nInd++ )
        {
            // NO Encoding during conversion!
            // The specification says that the low byte should be used in case it is not NULL
            char nHighChar = (char)( pStr[nInd] >> 8 );
            char nLowChar = (char)( pStr[nInd] & 0xFF );
            char nChar = nLowChar ? nLowChar : nHighChar;

            for ( int nMatrixInd = 0; nMatrixInd < 7; ++nMatrixInd )
            {
                if ( ( nChar & ( 1 << nMatrixInd ) ) != 0 )
                    nHighResult = nHighResult ^ pEncryptionMatrix[15 - nLen + nInd][nMatrixInd];
            }

            nLowResult = ( ( ( nLowResult >> 14 ) & 0x0001 ) | ( ( nLowResult << 1 ) & 0x7FFF ) ) ^ nChar;
        }

        nLowResult = (sal_uInt16)( ( ( ( nLowResult >> 14 ) & 0x001 ) | ( ( nLowResult << 1 ) & 0x7FF ) ) ^ nLen ^ 0xCE4B );

        nResult = ( nHighResult << 16 ) | nLowResult;
    }

    return nResult;
}


sal_uInt16 DocPasswordHelper::GetXLHashAsUINT16(
                const OUString& aUString,
                rtl_TextEncoding nEnc )
{
    sal_uInt16 nResult = 0;

    OString aString = OUStringToOString( aUString, nEnc );

    if ( !aString.isEmpty() && aString.getLength() <= SAL_MAX_UINT16 )
    {
        for ( sal_Int32 nInd = aString.getLength() - 1; nInd >= 0; nInd-- )
        {
            nResult = ( ( nResult >> 14 ) & 0x01 ) | ( ( nResult << 1 ) & 0x7FFF );
            nResult ^= aString[nInd];
        }

        nResult = ( ( nResult >> 14 ) & 0x01 ) | ( ( nResult << 1 ) & 0x7FFF );
        nResult ^= ( 0x8000 | ( 'N' << 8 ) | 'K' );
        nResult ^= aString.getLength();
    }

    return nResult;
}


Sequence< sal_Int8 > DocPasswordHelper::GetXLHashAsSequence(
                const OUString& aUString,
                rtl_TextEncoding nEnc )
{
    sal_uInt16 nHash = GetXLHashAsUINT16( aUString, nEnc );
    Sequence< sal_Int8 > aResult( 2 );
    aResult[0] = ( nHash >> 8 );
    aResult[1] = ( nHash & 0xFF );

    return aResult;
}


/*static*/ uno::Sequence< sal_Int8 > DocPasswordHelper::GenerateRandomByteSequence( sal_Int32 nLength )
{
    uno::Sequence< sal_Int8 > aResult( nLength );

    TimeValue aTime;
    osl_getSystemTime( &aTime );
    rtlRandomPool aRandomPool = rtl_random_createPool ();
    rtl_random_addBytes ( aRandomPool, &aTime, 8 );
    rtl_random_getBytes ( aRandomPool, aResult.getArray(), nLength );
    rtl_random_destroyPool ( aRandomPool );

    return aResult;
}



/*static*/ uno::Sequence< sal_Int8 > DocPasswordHelper::GenerateStd97Key( const OUString& aPassword, const uno::Sequence< sal_Int8 >& aDocId )
{
    uno::Sequence< sal_Int8 > aResultKey;
    if ( !aPassword.isEmpty() && aDocId.getLength() == 16 )
    {
        sal_uInt16 pPassData[16];
        memset( pPassData, 0, sizeof(pPassData) );

        sal_Int32 nPassLen = ::std::min< sal_Int32 >( aPassword.getLength(), 15 );
        memcpy( pPassData, aPassword.getStr(), nPassLen * sizeof(pPassData[0]) );

        aResultKey = GenerateStd97Key( pPassData, aDocId );
    }

    return aResultKey;
}


/*static*/ uno::Sequence< sal_Int8 > DocPasswordHelper::GenerateStd97Key( const sal_uInt16 pPassData[16], const uno::Sequence< sal_Int8 >& aDocId )
{
    uno::Sequence< sal_Int8 > aResultKey;

    if ( aDocId.getLength() == 16 )
        aResultKey = GenerateStd97Key(pPassData, (const sal_uInt8*)aDocId.getConstArray());

    return aResultKey;
}


/*static*/ uno::Sequence< sal_Int8 > DocPasswordHelper::GenerateStd97Key( const sal_uInt16 pPassData[16], const sal_uInt8 pDocId[16] )
{
    uno::Sequence< sal_Int8 > aResultKey;
    if ( pPassData[0] )
    {
        sal_uInt8 pKeyData[64];
        memset( pKeyData, 0, sizeof(pKeyData) );

        sal_Int32 nInd = 0;

        // Fill PassData into KeyData.
        for ( nInd = 0; nInd < 16 && pPassData[nInd]; nInd++)
        {
            pKeyData[2*nInd] = sal::static_int_cast< sal_uInt8 >( (pPassData[nInd] >> 0) & 0xff );
            pKeyData[2*nInd + 1] = sal::static_int_cast< sal_uInt8 >( (pPassData[nInd] >> 8) & 0xff );
        }

        pKeyData[2*nInd] = 0x80;
        pKeyData[56] = sal::static_int_cast< sal_uInt8 >( nInd << 4 );

        // Fill raw digest of KeyData into KeyData.
        rtlDigest hDigest = rtl_digest_create ( rtl_Digest_AlgorithmMD5 );
        (void)rtl_digest_updateMD5 (
            hDigest, pKeyData, sizeof(pKeyData));
        (void)rtl_digest_rawMD5 (
            hDigest, pKeyData, RTL_DIGEST_LENGTH_MD5);

        // Update digest with KeyData and Unique.
        for ( nInd = 0; nInd < 16; nInd++ )
        {
            rtl_digest_updateMD5( hDigest, pKeyData, 5 );
            rtl_digest_updateMD5( hDigest, pDocId, 16 );
        }

        // Update digest with padding.
        pKeyData[16] = 0x80;
        memset( pKeyData + 17, 0, sizeof(pKeyData) - 17 );
        pKeyData[56] = 0x80;
        pKeyData[57] = 0x0a;

        rtl_digest_updateMD5( hDigest, &(pKeyData[16]), sizeof(pKeyData) - 16 );

        // Fill raw digest of above updates
        aResultKey.realloc( RTL_DIGEST_LENGTH_MD5 );
        rtl_digest_rawMD5 ( hDigest, (sal_uInt8*)aResultKey.getArray(), aResultKey.getLength() );

        // Erase KeyData array and leave.
        memset( pKeyData, 0, sizeof(pKeyData) );

#ifndef NO_LIBO_DIGEST_LEAK_FIX
        rtl_digest_destroy(hDigest);
#endif	// !NO_LIBO_DIGEST_LEAK_FIX
    }

    return aResultKey;
}




/*static*/ ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > DocPasswordHelper::requestAndVerifyDocPassword(
        IDocPasswordVerifier& rVerifier,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rMediaEncData,
        const OUString& rMediaPassword,
        const Reference< XInteractionHandler >& rxInteractHandler,
        const OUString& rDocumentName,
        DocPasswordRequestType eRequestType,
        const ::std::vector< OUString >* pDefaultPasswords,
        bool* pbIsDefaultPassword )
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > aEncData;
#ifndef NO_LIBO_BUG_118639_FIX
    OUString aPassword;
#endif	// !NO_LIBO_BUG_118639_FIX
    DocPasswordVerifierResult eResult = DocPasswordVerifierResult_WRONG_PASSWORD;

#ifndef NO_LIBO_BUG_93389_FIX
    sal_Int32 nMediaEncDataCount = rMediaEncData.getLength();

    // tdf#93389: if the document is being restored from autorecovery, we need to add encryption
    // data also for real document type.
    // TODO: get real filter name here (from CheckPasswd_Impl), to only add necessary data
    bool bForSalvage = false;
    if (nMediaEncDataCount)
    {
        for (auto& val : rMediaEncData)
        {
            if (val.Name == "ForSalvage")
            {
                --nMediaEncDataCount; // don't consider this element below
                val.Value >>= bForSalvage;
                break;
            }
        }
    }
#endif	// !NO_LIBO_BUG_93389_FIX

    // first, try provided default passwords
    if( pbIsDefaultPassword )
        *pbIsDefaultPassword = false;
    if( pDefaultPasswords )
    {
        for( ::std::vector< OUString >::const_iterator aIt = pDefaultPasswords->begin(), aEnd = pDefaultPasswords->end(); (eResult == DocPasswordVerifierResult_WRONG_PASSWORD) && (aIt != aEnd); ++aIt )
        {
            OSL_ENSURE( !aIt->isEmpty(), "DocPasswordHelper::requestAndVerifyDocPassword - unexpected empty default password" );
            if( !aIt->isEmpty() )
            {
                eResult = rVerifier.verifyPassword( *aIt, aEncData );
#ifdef NO_LIBO_BUG_118639_FIX
                if( pbIsDefaultPassword )
                    *pbIsDefaultPassword = eResult == DocPasswordVerifierResult_OK;
#else	// NO_LIBO_BUG_118639_FIX
                if (eResult == DocPasswordVerifierResult_OK)
                {
                    aPassword = *aIt;
                    if (pbIsDefaultPassword)
                        *pbIsDefaultPassword = true;
                }
#endif	// NO_LIBO_BUG_118639_FIX
            }
        }
    }

    // try media encryption data (skip, if result is OK or ABORT)
    if( eResult == DocPasswordVerifierResult_WRONG_PASSWORD )
    {
#ifdef NO_LIBO_BUG_93389_FIX
        if( rMediaEncData.getLength() > 0 )
#else	// NO_LIBO_BUG_93389_FIX
        if (nMediaEncDataCount)
#endif	// NO_LIBO_BUG_93389_FIX
        {
            eResult = rVerifier.verifyEncryptionData( rMediaEncData );
            if( eResult == DocPasswordVerifierResult_OK )
                aEncData = rMediaEncData;
        }
    }

    // try media password (skip, if result is OK or ABORT)
    if( eResult == DocPasswordVerifierResult_WRONG_PASSWORD )
    {
        if( !rMediaPassword.isEmpty() )
#ifndef NO_LIBO_BUG_118639_FIX
        {
#endif	// !NO_LIBO_BUG_118639_FIX
            eResult = rVerifier.verifyPassword( rMediaPassword, aEncData );
#ifndef NO_LIBO_BUG_118639_FIX
            if (eResult == DocPasswordVerifierResult_OK)
                aPassword = rMediaPassword;
        }
#endif	// !NO_LIBO_BUG_118639_FIX
    }

    // request a password (skip, if result is OK or ABORT)
    if( (eResult == DocPasswordVerifierResult_WRONG_PASSWORD) && rxInteractHandler.is() ) try
    {
        PasswordRequestMode eRequestMode = PasswordRequestMode_PASSWORD_ENTER;
        while( eResult == DocPasswordVerifierResult_WRONG_PASSWORD )
        {
            DocPasswordRequest* pRequest = new DocPasswordRequest( eRequestType, eRequestMode, rDocumentName );
            Reference< XInteractionRequest > xRequest( pRequest );
            rxInteractHandler->handle( xRequest );
            if( pRequest->isPassword() )
            {
                if( !pRequest->getPassword().isEmpty() )
                    eResult = rVerifier.verifyPassword( pRequest->getPassword(), aEncData );
#ifndef NO_LIBO_BUG_118639_FIX
                if (eResult == DocPasswordVerifierResult_OK)
                    aPassword = pRequest->getPassword();
#endif	// !NO_LIBO_BUG_118639_FIX
            }
            else
            {
                eResult = DocPasswordVerifierResult_ABORT;
            }
            eRequestMode = PasswordRequestMode_PASSWORD_REENTER;
        }
    }
    catch( Exception& )
    {
    }

#ifndef NO_LIBO_BUG_118639_FIX
    if (eResult == DocPasswordVerifierResult_OK && !aPassword.isEmpty())
    {
        sal_Int32 nInd = 0;
        for ( ; nInd < aEncData.getLength(); nInd++ )
        {
            if ( aEncData[nInd].Name == PACKAGE_ENCRYPTIONDATA_SHA256UTF8 )
                break;
        }
        if ( nInd == aEncData.getLength() )
        {
            // tdf#118639: We need ODF encryption data for autorecovery, where password
            // will already be unavailable, so generate and append it here
            aEncData = comphelper::concatSequences(
                aEncData, OStorageHelper::CreatePackageEncryptionData(aPassword));
        }

#ifndef NO_LIBO_BUG_93389_FIX
        if (bForSalvage)
        {
            // TODO: add individual methods for different target filter, and only call what's needed

            // 1. Prepare binary MS formats encryption data
            auto aUniqueID = GenerateRandomByteSequence(16);
            auto aEnc97Key = GenerateStd97Key(aPassword.getStr(), aUniqueID);
            // 2. Add MS binary and OOXML encryption data to result
            uno::Sequence< beans::NamedValue > aOOXPassword(3);
            aOOXPassword[0].Name = "STD97EncryptionKey";
            aOOXPassword[0].Value = css::uno::Any(aEnc97Key);
            aOOXPassword[1].Name = "STD97UniqueID";
            aOOXPassword[1].Value = css::uno::Any(aUniqueID);
            aOOXPassword[1].Name = "OOXPassword";
            aOOXPassword[1].Value = css::uno::Any(aPassword );
            aEncData = comphelper::concatSequences(aEncData, aOOXPassword);
        }
#endif	// !NO_LIBO_BUG_93389_FIX
    }
#endif	// !NO_LIBO_BUG_118639_FIX

    return (eResult == DocPasswordVerifierResult_OK) ? aEncData : uno::Sequence< beans::NamedValue >();
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
